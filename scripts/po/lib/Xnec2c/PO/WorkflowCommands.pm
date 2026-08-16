package Xnec2c::PO::WorkflowCommands;

use strict;
use warnings;
use utf8;

use Exporter qw(import);
use File::Path qw(make_path);

use Xnec2c::PO::CatalogState qw(catalog_path);
use Xnec2c::PO::CatalogTransaction qw(
	commit_language_output verify_language_output
);
use Xnec2c::PO::LanguageWorkflow qw(
	apply_language assert_language_outcome prepare_language_manifest
	report_commit report_outcome
);
use Xnec2c::PO::LanguageWorkspace qw(
	catalog_languages clear_language_manifest validate_language
);
use Xnec2c::PO::ManifestWriter qw(write_language_manifest);
use Xnec2c::PO::OutputJoin qw(join_output_records);
use Xnec2c::PO::TranslationMap qw(AI_DIR output_path);

our @EXPORT_OK = qw(cmd_apply cmd_generate cmd_inject cmd_lint cmd_verify);

# Present every map fault and exemption warning for language review.
sub report_translation_notices
{
	my ($result) = @_;
	print "$_\n" for @{$result->{faults}};
	print "$_\n" for @{$result->{warnings}};
}

# Resolve one command argument naming a language, either as the bare name or as
# a workspace map path such as po/ai/es.out.map.
sub argument_language
{
	my ($argument) = @_;
	my ($lang) = $argument =~ m{\A(?:.*/)?([A-Za-z_]+)(?:\.out)?\.map\z};

	$lang = $argument if !defined $lang;
	validate_language($lang);

	return $lang;
}

# Refresh every catalog and generate the manifest its current state requires.
sub cmd_generate
{
	make_path(AI_DIR);
	for my $lang (catalog_languages())
	{
		my ($records, $copied) = prepare_language_manifest($lang);
		if (@{$records})
		{
			write_language_manifest($lang, $records, $copied);
		}
		else
		{
			clear_language_manifest($lang);
			print "$lang: no fuzzy, untranslated, or self-translated entries;"
				. " existing output retained\n";
		}
	}
}

# Complete one language before starting the next.
sub cmd_apply
{
	make_path(AI_DIR);
	for my $lang (catalog_languages())
	{
		apply_language($lang);
	}
}

# Report the map faults and exemption warnings of one output map.
sub lint_language
{
	my ($lang) = @_;
	my $result = join_output_records($lang);
	my $passed = @{$result->{faults}} ? 0 : 1;

	report_translation_notices($result);
	if ($passed)
	{
		print "$lang: output map passes map checks\n";
	}
	else
	{
		print output_path($lang) . ': FAIL ' . scalar(@{$result->{faults}})
			. ' faults; correct each with Edit at the line it names, run no'
			. " program of your own, then run this lint command again\n";
	}

	return $passed;
}

# Lint every named output map, reporting each language before failing.
sub cmd_lint
{
	my (@arguments) = @_;
	my @failed;

	for my $argument (@arguments)
	{
		my $lang = argument_language($argument);
		push @failed, $lang if !lint_language($lang);
	}

	die 'output map lint failed: ' . join(', ', @failed) . "\n" if @failed;
}

# Apply one output map to a staged catalog, leaving the authoritative one
# untouched.
sub cmd_verify
{
	my ($argument) = @_;
	my $lang = argument_language($argument);
	my $outcome = verify_language_output($lang);

	report_outcome($outcome);
	assert_language_outcome($lang, $outcome,
		'output map does not apply to its catalog');
	print "$lang: $outcome->{applied_count} records apply cleanly to "
		. catalog_path($lang) . "\n";
}

# Commit one output map to its authoritative catalog.
sub cmd_inject
{
	my ($argument) = @_;
	my $lang = argument_language($argument);
	my $outcome = commit_language_output($lang);

	report_outcome($outcome);
	assert_language_outcome($lang, $outcome,
		'output map does not apply to its catalog');
	report_commit($lang, $outcome);
}

1;
