#!/usr/bin/env perl
# Regression test: every mechanically detectable plural-entry translation
# failure fails scripts/po/po-check.sh, the gate make check-po runs, and
# terminates the po-translate.pl gate, which runs the same script on its
# mapped projection; complete and exempt plural entries pass both, and no
# plural entry becomes output-map work.
#
# Each case writes a catalog holding the fixture header and one plural entry
# into a scratch tree whose scripts directory links to the production scripts,
# then runs po-check.sh on it, catalog_change_records over it, and
# gate_candidate on the projection mapped_catalog_projection derives when no
# output-map record answers an entry.

use strict;
use warnings;
use utf8;

use Cwd qw(abs_path getcwd);
use File::Temp;
use FindBin qw($RealBin);
use lib "$RealBin/../lib";

use Locale::PO;

use Xnec2c::PO::CatalogState qw(catalog_change_records catalog_path load_catalog);
use Xnec2c::PO::CatalogTransaction;
use Xnec2c::PO::GateProjection qw(catalog_line_index mapped_catalog_projection);
use Xnec2c::PO::MapFile qw(read_utf8_file);

binmode STDOUT, ':encoding(UTF-8)';

my $LANG = 'mk';
my $CATALOG = catalog_path($LANG);

# Header lines; write_catalog follows them with a blank line and the case entry.
my @HEADER = (
	'msgid ""',
	'msgstr ""',
	'"Project-Id-Version: xnec2c\n"',
	'"PO-Revision-Date: 2026-11-25 00:00+0000\n"',
	'"Last-Translator: Fixture <fixture@example.invalid>\n"',
	'"Language-Team: Fixture <fixture@example.invalid>\n"',
	'"Language: mk\n"',
	'"MIME-Version: 1.0\n"',
	'"Content-Type: text/plain; charset=UTF-8\n"',
	'"Content-Transfer-Encoding: 8bit\n"',
	'"Plural-Forms: nplurals=2; plural= n==1 || n%10==1 ? 0 : 1;\n"',
);

# One plural entry per case; fault names the keyword opening the entry line
# po-check.sh reports for a failing case and is absent for a passing one.
my @CASES = (
	{ name => 'complete', msgid => '%d segment', entry => [
		'msgid "%d segment"', 'msgid_plural "%d segments"',
		'msgstr[0] "%d сегмент"', 'msgstr[1] "%d сегменти"'] },
	{ name => 'exempt_blank', msgid => '%d', entry => [
		'# xnec2c-exempt: symbol', 'msgid "%d"', 'msgid_plural "%d"',
		'msgstr[0] ""', 'msgstr[1] ""'] },
	{ name => 'form_empty', msgid => '%d wire', fault => 'msgid', entry => [
		'msgid "%d wire"', 'msgid_plural "%d wires"',
		'msgstr[0] "%d жица"', 'msgstr[1] ""'] },
	{ name => 'forms_empty', msgid => '%d card', fault => 'msgid', entry => [
		'msgid "%d card"', 'msgid_plural "%d cards"',
		'msgstr[0] ""', 'msgstr[1] ""'] },
	{ name => 'exempt_partial', msgid => '%d port', fault => 'msgid', entry => [
		'# xnec2c-exempt: symbol', 'msgid "%d port"', 'msgid_plural "%d ports"',
		'msgstr[0] "%d порта"', 'msgstr[1] ""'] },
	{ name => 'copied', msgid => '%d job', fault => 'msgstr[0]', entry => [
		'msgid "%d job"', 'msgid_plural "%d jobs"',
		'msgstr[0] "%d job"', 'msgstr[1] "%d jobs"'] },
	{ name => 'fuzzy', msgid => '%d step', fault => '#,', entry => [
		'#, fuzzy', 'msgid "%d step"', 'msgid_plural "%d steps"',
		'msgstr[0] "%d чекор"', 'msgstr[1] "%d чекори"'] },
	{ name => 'forms_short', msgid => '%d node', fault => 'msgstr[0]', entry => [
		'msgid "%d node"', 'msgid_plural "%d nodes"',
		'msgstr[0] "%d јазол"'] },
);

my $start = getcwd();
my $scratch = File::Temp->newdir('po_plural_gate.XXXXXX', DIR => $start);
my $scripts = abs_path("$RealBin/../..");
mkdir "$scratch/po" or abort_setup("$scratch/po: unable to create: $!");
symlink $scripts, "$scratch/scripts"
	or abort_setup("$scratch/scripts: unable to link $scripts: $!");
chdir "$scratch" or abort_setup("$scratch: unable to enter: $!");

my @failed;
for my $case (@CASES)
{
	my $faults = case_faults($case);
	if (@{$faults})
	{
		print "FAIL: $case->{name}: $_\n" for @{$faults};
		push @failed, $case->{name};
	}
	else
	{
		print "PASS: $case->{name}\n";
	}
}

chdir $start or abort_setup("$start: unable to return: $!");
if (@failed)
{
	print "FAIL: plural cases misjudged: @failed\n";
	exit 1;
}
else
{
	print 'PASS: ' . scalar(@CASES)
		. " plural cases judged by po-check.sh and the po-translate.pl gate\n";
	exit 0;
}

# Name every way po-check.sh, catalog_change_records, and gate_candidate
# misjudge one case's catalog.
sub case_faults
{
	my ($case) = @_;
	my $fails = defined $case->{fault};
	my @faults;

	my $entry_line = write_catalog($case);

	my $output = qx(scripts/po/po-check.sh $CATALOG 2>&1);
	my $status = $? >> 8;
	if ($fails)
	{
		my $fault_line = case_fault_line($case, $entry_line);

		push @faults, "po-check.sh exited $status, expected 1" if $status != 1;
		push @faults, "po-check.sh names no fault at line $fault_line:\n$output"
			if $output !~ /(?:at line |\.po:)\Q$fault_line\E\b/;
	}
	else
	{
		push @faults, "po-check.sh exited $status, expected 0:\n$output"
			if $status != 0;
	}

	push @faults, 'catalog_change_records selected the plural entry as'
		. ' output-map work'
		if grep { $_->{source} eq $case->{msgid} }
			@{catalog_change_records($LANG)};

	my $outcome = gate_outcome();
	my $gate_text = join("\n", @{$outcome->{faults}});
	if ($fails)
	{
		push @faults, "gate_candidate returned $outcome->{kind}, expected fatal"
			if $outcome->{kind} ne 'fatal';
		push @faults, 'gate_candidate names no unanswered fault for msgid'
			. " $case->{msgid}:\n$gate_text"
			if $gate_text !~ /\(no record; catalog msgid "\Q$case->{msgid}\E"\)/;
	}
	else
	{
		push @faults, "gate_candidate returned $outcome->{kind}, expected ok:"
			. "\n$gate_text"
			if $outcome->{kind} ne 'ok';
	}

	return \@faults;
}

# Number the catalog line a failing case's fault names: the first line of its
# entry, which starts at catalog line $entry_line, opening with its fault
# keyword.
sub case_fault_line
{
	my ($case, $entry_line) = @_;
	my ($offset) = grep { $case->{entry}[$_] =~ /\A\Q$case->{fault}\E / }
		0 .. $#{$case->{entry}};

	abort_setup("$case->{name}: entry holds no line opening with"
		. " $case->{fault}")
		if !defined $offset;

	return $entry_line + $offset;
}

# Gate the projection prepare_candidate derives when no output-map record
# answers an entry, as po-translate.pl verify and inject gate it.
sub gate_outcome
{
	my $gate_path = "$CATALOG.gate";

	Locale::PO->save_file_fromarray($gate_path,
		mapped_catalog_projection(load_catalog($CATALOG), {}), 'utf8');

	return Xnec2c::PO::CatalogTransaction::gate_candidate(
		Xnec2c::PO::CatalogTransaction::new_candidate(
			lang => $LANG,
			gate_path => $gate_path,
			full_path => $CATALOG,
			line_index => catalog_line_index(read_utf8_file($gate_path), {}),
		));
}

# Write the fixture header and one case's plural entry as the scratch catalog,
# and return the catalog line on which the entry starts.
sub write_catalog
{
	my ($case) = @_;
	my @header_block = (@HEADER, '');

	open my $fh, '>:encoding(UTF-8)', $CATALOG
		or abort_setup("$CATALOG: unable to write: $!");
	print {$fh} join("\n", @header_block, @{$case->{entry}}) . "\n"
		or abort_setup("$CATALOG: unable to write: $!");
	close $fh or abort_setup("$CATALOG: unable to close: $!");

	return scalar(@header_block) + 1;
}

# Report a scratch-tree setup failure through the hard-error status.
sub abort_setup
{
	my ($reason) = @_;

	print "ERROR: $reason\n";
	exit 99;
}
