package Xnec2c::PO::CatalogTransaction;

use strict;
use warnings;
use utf8;

use Exporter qw(import);
use File::Temp;
use Locale::PO;

use Xnec2c::PO::CatalogState qw(
	apply_translation_records catalog_line_index catalog_path load_catalog
	mapped_catalog_projection
);
use Xnec2c::PO::MapFile qw(read_utf8_file source_excerpt);
use Xnec2c::PO::Outcome qw(
	OUTCOME_FATAL OUTCOME_OK OUTCOME_REPAIR command_outcome new_outcome
);
use Xnec2c::PO::TranslationMap qw(join_output_records output_path);

our @EXPORT_OK = qw(
	commit_language_output refresh_language_catalog run_full_catalog_gate
	verify_language_output
);

# Construct one staged candidate, requiring every field its release needs.
sub new_candidate
{
	my (%fields) = @_;

	for my $field (qw(lang full_path gate_path line_index))
	{
		die "candidate requires $field\n" if !defined $fields{$field};
	}

	return { %fields };
}

# Name the map record owning one projected catalog line.
sub gate_line_origin
{
	my ($candidate, $line_number) = @_;
	my $out_path = output_path($candidate->{lang});

	# The index runs in ascending start order, so the last entry beginning at
	# or before the reported line owns it.
	my $line_index = $candidate->{line_index};
	my $index = 0;
	my $found;
	while (($index < @{$line_index})
		&& ($line_index->[$index]{start} <= $line_number))
	{
		$found = $line_index->[$index];
		$index++;
	}

	return $out_path if !defined $found;
	return "$out_path:$found->{record}{line}: K $found->{record}{key}"
		if defined $found->{record};

	my $source = source_excerpt($found->{S} // '');

	return "$out_path (no record; catalog msgid \"$source\")";
}

# Recast each gate line number as the output-map record that produced it.
sub localize_gate_faults
{
	my ($candidate, $lines) = @_;
	my $gate_path = $candidate->{gate_path};
	my $out_path = output_path($candidate->{lang});
	my @localized;

	for my $line (@{$lines})
	{
		my $text = $line;
		$text =~ s{\Q$gate_path\E:([0-9]+):}
			{gate_line_origin($candidate, $1) . ':'}ge;
		$text =~ s{at line ([0-9]+)}
			{'at ' . gate_line_origin($candidate, $1)}ge;
		$text =~ s/\Q$gate_path\E/$out_path/g;
		$text =~ s/\n\z//;
		push @localized, $text;
	}

	return \@localized;
}

# Join the output, apply its answered records to a staged full catalog, and
# derive the mapped projection the gate reads. The authoritative catalog is
# untouched until commit_candidate renames the staged file over it.
sub prepare_candidate
{
	my ($lang) = @_;
	my $join = join_output_records($lang);

	return (new_outcome(kind => OUTCOME_REPAIR, faults => $join->{faults},
		warnings => $join->{warnings}), undef) if @{$join->{faults}};

	my $path = catalog_path($lang);
	my $catalog = load_catalog($path);
	my $application = apply_translation_records($path, $catalog,
		$join->{translations});

	# Keep automatic cleanup through serialization and indexing; transfer both
	# files to the candidate only after those preparation steps pass.
	my $full_file = File::Temp->new(
		TEMPLATE => ".$lang.inject.XXXXXX", DIR => 'po', UNLINK => 1);
	my $full_path = $full_file->filename;
	close $full_file
		or die "$full_path: unable to close staged catalog: $!\n";
	my $gate_file = File::Temp->new(
		TEMPLATE => ".$lang.gate.XXXXXX", DIR => 'po', UNLINK => 1);
	my $gate_path = $gate_file->filename;
	close $gate_file
		or die "$gate_path: unable to close staged projection: $!\n";

	Locale::PO->save_file_fromarray($full_path, $catalog, 'utf8');
	Locale::PO->save_file_fromarray($gate_path,
		mapped_catalog_projection($catalog, $join->{translations}), 'utf8');

	my $candidate = new_candidate(
		lang => $lang,
		full_path => $full_path,
		gate_path => $gate_path,
		line_index => catalog_line_index(read_utf8_file($gate_path),
			$join->{translations}),
	);
	my $outcome = new_outcome(kind => OUTCOME_OK,
		warnings => [@{$join->{warnings}}, @{$application->{retired}}],
		applied_count => $application->{applied});

	$gate_file->unlink_on_destroy(0);
	$full_file->unlink_on_destroy(0);

	return ($outcome, $candidate);
}

# Run the shared gate against the mapped projection alone, so entries no
# current manifest record answers cannot block already-answered work.
sub gate_candidate
{
	my ($candidate) = @_;
	my $gate_path = $candidate->{gate_path};
	my $pid = open my $output, '-|';

	return new_outcome(kind => OUTCOME_FATAL,
		faults => ["$gate_path: unable to fork scripts/po/po-check.sh: $!"])
		if !defined $pid;

	if ($pid == 0)
	{
		open STDERR, '>&', \*STDOUT
			or die "$gate_path: unable to merge check output: $!\n";
		# po-check.sh reserves exit 1 for gate failures, so a failed exec
		# leaves through the code the parent treats as unrepairable.
		exec 'scripts/po/po-check.sh', '--review', $gate_path or do {
			print "$gate_path: unable to run scripts/po/po-check.sh: $!\n";
			exit 127;
		};
	}

	my @lines = <$output>;
	close $output;
	my $status = $?;
	my $localized = localize_gate_faults($candidate, \@lines);

	return new_outcome(kind => OUTCOME_OK) if $status == 0;
	return new_outcome(kind => OUTCOME_FATAL,
		faults => ["$gate_path: gate terminated by signal " . ($status & 127)])
		if ($status & 127) != 0;

	# Only the documented gate-failure status on mapped entries describes
	# faults an output-map edit can answer. Header faults retain the explicit
	# no-record origin and terminate because the output map cannot repair them.
	my @reported = grep { /FAIL/ } @{$localized};
	my @unmapped = grep { /\(no record; catalog msgid/ } @{$localized};
	return new_outcome(kind => OUTCOME_FATAL,
		faults => ["$gate_path: gate exited with status " . ($status >> 8),
			@{$localized}])
		if ($status >> 8) != 1 || !@reported || @unmapped;

	return new_outcome(kind => OUTCOME_REPAIR, faults => $localized);
}

# Release both staged files in reverse acquisition order.
sub release_candidate
{
	my ($candidate) = @_;
	my @faults;

	for my $path ($candidate->{gate_path}, $candidate->{full_path})
	{
		push @faults, "$path: unable to remove staged catalog: $!"
			if !unlink $path;
	}
	die join("\n", @faults) . "\n" if @faults;
}

# Build one staged candidate and gate it. A candidate returns only when its
# mapped gate passed; every other path releases the stage first.
sub gated_candidate
{
	my ($lang) = @_;
	my ($outcome, $candidate) = prepare_candidate($lang);

	return ($outcome, undef) if !defined $candidate;

	my $gate = gate_candidate($candidate);
	return ($outcome, $candidate) if $gate->{kind} eq OUTCOME_OK;

	release_candidate($candidate);

	return (new_outcome(kind => $gate->{kind}, faults => $gate->{faults},
		warnings => $outcome->{warnings}), undef);
}

# Apply the output map to a staged catalog without mutating the authoritative
# one.
sub verify_language_output
{
	my ($lang) = @_;
	my ($outcome, $candidate) = gated_candidate($lang);

	release_candidate($candidate) if defined $candidate;

	return $outcome;
}

# Remove an uncommitted full catalog, then report the fault that prevented its
# commit.
sub abort_commit
{
	my ($candidate, $fault) = @_;

	unlink $candidate->{full_path}
		or die "$candidate->{full_path}: unable to remove uncommitted"
			. " staged catalog: $!\n";
	die "$fault\n";
}

# Commit the gated candidate over the authoritative catalog with one rename.
sub commit_language_output
{
	my ($lang) = @_;
	my ($outcome, $candidate) = gated_candidate($lang);

	return $outcome if !defined $candidate;

	my $path = catalog_path($lang);
	abort_commit($candidate, "$candidate->{gate_path}: unable to remove"
		. " staged projection: $!")
		if !unlink $candidate->{gate_path};
	abort_commit($candidate, "$path: unable to commit staged catalog: $!")
		if !rename $candidate->{full_path}, $path;

	return $outcome;
}

# Canonicalize one committed catalog through the shared refresh transaction.
sub refresh_language_catalog
{
	my ($lang) = @_;
	my $path = catalog_path($lang);
	my $status = system('scripts/po/po-refresh.sh', $path);

	return command_outcome("$path: scripts/po/po-refresh.sh", $status);
}

# Run the complete release gate against one authoritative catalog.
sub run_full_catalog_gate
{
	my ($lang) = @_;
	my $path = catalog_path($lang);
	my $status = system('scripts/po/po-check.sh', $path);

	return command_outcome("$path: scripts/po/po-check.sh", $status);
}

1;
