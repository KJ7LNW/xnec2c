package Xnec2c::PO::ModelSession;

use strict;
use warnings;
use utf8;

use Cwd qw(abs_path);
use Exporter qw(import);
use FindBin qw($RealBin);

use Xnec2c::PO::Outcome qw(OUTCOME_OK command_outcome);
use Xnec2c::PO::TranslationMap qw(map_path output_path);
use Xnec2c::PO::TranslationRules qw(
	exemption_rules record_rules repair_rules
);

our @EXPORT_OK = qw(new_model_session run_model_session set_model);

my $ROOT = abs_path("$RealBin/../..");

# The model every session seats. This module holds the sole default; the entry
# point names one only when a caller asks for it.
my $MODEL = 'gpt-5.6-terra(low)[1m]';

# Seat the model every later session runs against, once, at the composition
# boundary.
sub set_model
{
	my ($model) = @_;
	$MODEL = $model;
}

# Construct one immutable session specification.
sub new_model_session
{
	my ($lang, $feedback) = @_;

	die "session requires a language\n" if !defined $lang;
	die "session requires the faults it answers\n"
		if !defined $feedback || !@{$feedback};

	return {
		lang => $lang,
		output_path => output_path($lang),
		feedback => $feedback,
	};
}

# State the workspace and command restrictions every session carries.
sub workspace_rules
{
	my ($session) = @_;
	my $lang = $session->{lang};

	return join("\n",
		"- Edit only $session->{output_path}.",
		"- Read no file other than $session->{output_path}; the injected"
			. ' context supplies every file this task needs.',
		'- Search no directory and open no program source; act on what lint'
			. ' and verify report, never on how they work.',
		"- Run no Bash command except scripts/po/po-translate.pl lint $lang"
			. " and scripts/po/po-translate.pl verify $lang.");
}

# State the ordered steps closing every session.
sub session_contract
{
	my ($session) = @_;
	my $lang = $session->{lang};

	return join("\n",
		'# Procedure',
		'',
		"1. Before any Edit, you must read $session->{output_path} with offset"
			. ' 1 and limit 1.',
		'2. Change each reported record at its named line.',
		"3. Run scripts/po/po-translate.pl lint $lang until it reports no FAIL."
			. ' Repeat steps 1 and 2 against each report.',
		'4. Review every exemption WARN; translate each record that holds a'
			. ' distinct target form through steps 1 and 2.',
		"5. Run scripts/po/po-translate.pl lint $lang again.",
		"6. Run scripts/po/po-translate.pl verify $lang until it succeeds."
			. ' Repeat steps 1 and 2 against each report.',
		'7. Stop once verify succeeds.',
		'',
		'',
		'Perform item 1 now.');
}

# Direct line-targeted correction of the records the preceding report named.
sub edit_output_instruction
{
	my ($session) = @_;
	my $lang = $session->{lang};
	my $feedback = join("\n", map { "- $_" } @{$session->{feedback}});

	return join("\n\n",
		"Correct $session->{output_path}. It exists and holds valid records;"
			. ' keep every record the report below does not name.',
		"# Reported faults\n\n$feedback",
		'# Output format',
		record_rules() . "\n- Translate each T under doc/TRANSLATING.md and"
			. " po/rules/$lang.md.",
		repair_rules(),
		exemption_rules(),
		"# Workspace\n\n" . workspace_rules($session),
		session_contract($session));
}

# Accept the stop-on-done exit code while propagating other launch failures.
sub assert_session_status
{
	my ($lang, $status) = @_;

	if (($status & 127) == 0 && ($status >> 8) == 143)
	{
		print "$lang: claude-inject stopped after done\n";
	}
	else
	{
		my $outcome = command_outcome("$lang: claude-inject", $status);
		die join("\n", @{$outcome->{faults}}) . "\n"
			if $outcome->{kind} ne OUTCOME_OK;
	}
}

# Run one model session against one language, seating the instruction its
# action names.
sub run_model_session
{
	my ($session) = @_;
	my $lang = $session->{lang};
	my $status;

	# The output map injects last. Each reference ahead of it runs first, and a
	# command touching the file after it was read leaves the first edit
	# rejected against newer content on disk.
	my @context = (
		'doc/TRANSLATING.md',
		"po/rules/$lang.md",
		map_path($lang),
		"!scripts/po/po-translate.pl lint $lang",
		"!scripts/po/po-translate.pl verify $lang",
		$session->{output_path},
	);

	{
		local $ENV{CLAUDE_HOOKS_KILL_ON_STOP} = 'done';
		$status = system(
			'claude-inject',
			'--claude', 'claude-raw',
			'--cwd', $ROOT,
			'--permission-mode', 'acceptEdits',
			'--model', $MODEL,
			@context,
			'--', edit_output_instruction($session),
		);
	}
	assert_session_status($lang, $status);
}

1;
