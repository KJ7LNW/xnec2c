package Xnec2c::PO::LanguageWorkflow;

use strict;
use warnings;
use utf8;

use Exporter qw(import);

use Xnec2c::PO::CatalogState qw(catalog_change_records catalog_path);
use Xnec2c::PO::CatalogTransaction qw(
	commit_language_output refresh_language_catalog run_full_catalog_gate
);
use Xnec2c::PO::Outcome qw(OUTCOME_FATAL OUTCOME_OK OUTCOME_REPAIR);
use Xnec2c::PO::LanguageWorkspace qw(
	clear_language_workspace preceding_exemptions validate_language
);
use Xnec2c::PO::ManifestWriter qw(write_language_manifest);
use Xnec2c::PO::ModelSession qw(new_model_session run_model_session);
use Xnec2c::PO::TranslationMap qw(
	manifest_records output_path rebase_output_records
);

our @EXPORT_OK = qw(
	apply_language assert_language_outcome prepare_language_manifest
	report_commit report_outcome
);

use constant STATE_APPLY_OUTPUT => 'apply_output';
use constant STATE_PREPARE_WORK => 'prepare_work';
use constant STATE_RUN_MODEL => 'run_model';
use constant STATE_DONE => 'done';

# Bound the model sessions one language receives. A map still invalid after
# these attempts wants an operator rather than another session, so apply stops
# at that language instead of spending sessions on the ones behind it.
use constant MODEL_ATTEMPT_MAX => 3;

# Name the handler advancing each state; STATE_DONE closes the language and so
# names none.
my %STATE_HANDLER_MAP = (
	STATE_APPLY_OUTPUT() => \&apply_output_state,
	STATE_PREPARE_WORK() => \&prepare_work_state,
	STATE_RUN_MODEL() => \&run_model_state,
);

# Report the warnings and faults one outcome carries.
sub report_outcome
{
	my ($outcome) = @_;

	print "$_\n" for @{$outcome->{warnings}};
	print "$_\n" for @{$outcome->{faults}};
}

# Terminate at one language whose outcome no output-map edit can repair.
sub assert_language_outcome
{
	my ($lang, $outcome, $subject) = @_;

	return if $outcome->{kind} eq OUTCOME_OK;
	die "$lang: FAIL $subject\n";
}

# Report the records one commit wrote to its authoritative catalog.
sub report_commit
{
	my ($lang, $outcome) = @_;

	print "$lang: committed $outcome->{applied_count} records to "
		. catalog_path($lang) . "\n";
}

# Refresh one catalog against the template and derive the manifest records its
# current state requires.
sub prepare_language_manifest
{
	my ($lang) = @_;
	my $refresh = refresh_language_catalog($lang);

	report_outcome($refresh);
	assert_language_outcome($lang, $refresh, 'catalog refresh failed');

	my $changes = catalog_change_records($lang);

	return (manifest_records($lang, $changes, preceding_exemptions($lang)),
		scalar grep { $_->{copied} } @{$changes});
}

# Open every language at regeneration. The work set follows from current
# catalog content, and rebase carries each existing answer onto that set by
# identity, so an entry the catalog answered between runs leaves the set and
# its stale answer is reported as dropped rather than written back over it.
sub new_language_work
{
	my ($lang) = @_;

	validate_language($lang);

	return {
		lang => $lang,
		state => STATE_PREPARE_WORK,
		attempts => 0,
		feedback => [],
		signature => undef,
	};
}

# Return repairable faults to the same language while retaining its current
# manifest and output map for line-targeted edits.
sub enter_edit_state
{
	my ($work, $faults) = @_;
	my $lang = $work->{lang};

	print "$lang: " . scalar(@{$faults}) . " faults returned to the model\n";
	$work->{state} = STATE_RUN_MODEL;
	$work->{feedback} = $faults;
}

# Commit the current output map, returning a repairable fault to the same
# language and terminating on one no edit can answer.
sub apply_output_state
{
	my ($work) = @_;
	my $lang = $work->{lang};
	my $rebase = rebase_output_records($lang);

	print "$_\n" for @{$rebase->{warnings}};
	if (@{$rebase->{faults}})
	{
		print "$_\n" for @{$rebase->{faults}};
		enter_edit_state($work, $rebase->{faults});
	}
	else
	{
		print "$lang: rebased $rebase->{carried} answers; applied "
			. "$rebase->{propagated} prior exemptions; $rebase->{open} current"
			. " records remain open\n"
			if $rebase->{changed};
		my $outcome = commit_language_output($lang);

		report_outcome($outcome);
		if ($outcome->{kind} eq OUTCOME_FATAL)
		{
			die "$lang: FAIL catalog transaction cannot proceed\n";
		}
		elsif ($outcome->{kind} eq OUTCOME_REPAIR)
		{
			enter_edit_state($work, $outcome->{faults});
		}
		elsif ($outcome->{kind} eq OUTCOME_OK)
		{
			report_commit($lang, $outcome);
			$work->{state} = STATE_PREPARE_WORK;
			$work->{feedback} = [];
		}
		else
		{
			die "$lang: unsupported outcome kind $outcome->{kind}\n";
		}
	}
}

# Terminate one language whose regeneration returns the work a preceding commit
# already answered. Commit, refresh, and regeneration advance without a model
# session, so the session bound alone leaves those transitions unbounded. A
# commit clears the fuzzy, untranslated, and self-translated state that selects
# a record, so identical consecutive work proves the catalog stopped advancing.
# The map codec escapes every tab and newline, so neither delimiter can occur
# inside an encoded field.
sub assert_regeneration_progress
{
	my ($work, $records) = @_;
	my $lang = $work->{lang};
	my $signature = join("\n", map { "$_->{C}\t$_->{S}" } @{$records});
	my $prior = $work->{signature};

	die "$lang: FAIL regeneration returns the same " . scalar(@{$records})
		. ' records a commit already answered; correct '
		. catalog_path($lang) . ' or ' . output_path($lang)
		. ", then run apply again\n"
		if defined $prior && $prior eq $signature;

	$work->{signature} = $signature;
}

# Refresh the committed catalog and regenerate its manifest, completing the
# language only when that regeneration finds no work and the full gate passes.
sub prepare_work_state
{
	my ($work) = @_;
	my $lang = $work->{lang};
	my ($records, $copied) = prepare_language_manifest($lang);

	if (@{$records})
	{
		assert_regeneration_progress($work, $records);

		# Rebase materializes the complete output before any session. It carries
		# existing answers and fills accepted source exemptions, so the model
		# modifies only records the program leaves unanswered.
		write_language_manifest($lang, $records, $copied);
		$work->{feedback} = [];
		$work->{state} = STATE_APPLY_OUTPUT;
	}
	else
	{
		my $gate = run_full_catalog_gate($lang);
		report_outcome($gate);
		assert_language_outcome($lang, $gate, 'catalog fails the release gate');
		clear_language_workspace($lang);
		$work->{state} = STATE_DONE;
	}
}

# Seat one model session, then return immediately to the commit attempt. Work
# outstanding once the bound is spent wants an operator, not another session.
sub run_model_state
{
	my ($work) = @_;
	my $lang = $work->{lang};

	die "$lang: FAIL work outstanding after " . MODEL_ATTEMPT_MAX
		. ' model sessions; correct ' . output_path($lang)
		. " at the lines reported above, then run apply again\n"
		if $work->{attempts} >= MODEL_ATTEMPT_MAX;

	my $session = new_model_session($lang, $work->{feedback});

	$work->{attempts}++;
	print "$lang: model session $work->{attempts} of " . MODEL_ATTEMPT_MAX
		. "\n";
	run_model_session($session);
	$work->{state} = STATE_APPLY_OUTPUT;
}

# Drive one language to a complete catalog, spending at most the bounded number
# of model sessions on it. Every non-model transition still runs after the last
# session, so a final commit reaches refresh, regeneration, and the full gate.
sub apply_language
{
	my ($lang) = @_;
	my $work = new_language_work($lang);

	while ($work->{state} ne STATE_DONE)
	{
		my $handler = $STATE_HANDLER_MAP{$work->{state}};
		die "$lang: unsupported workflow state $work->{state}\n"
			if !defined $handler;
		$handler->($work);
	}

	print "$lang: catalog complete after $work->{attempts} model sessions\n";
}

1;
