#!/usr/bin/env perl
# Regression test: scripts/po/po-check.sh, the gate make check-po and the
# po-translate.pl gate run, and catalog_change_records, the selection of
# output-map work, judge each singular entry state po-classify.awk records
# alike. An untranslated, fuzzy, or self-translated entry fails the gate at its
# report line and enters the work set, a self-translated one flagged copied; a
# translated entry and an empty entry under a sole exemption marker pass the
# gate and stay out of it. A fuzzy flag after another flag and an empty entry
# under two exemption markers fail the gate and enter the work set.
#
# The test writes one catalog holding the fixture header and every case entry
# into a scratch tree whose scripts directory links to the production scripts,
# numbering each case line as it writes it.

use strict;
use warnings;
use utf8;

use Cwd qw(abs_path getcwd);
use File::Temp;
use FindBin qw($RealBin);
use lib "$RealBin/../lib";

use Xnec2c::PO::CatalogState qw(catalog_change_records catalog_path);

binmode STDOUT, ':encoding(UTF-8)';

my $LANG = 'mk';
my $CATALOG = catalog_path($LANG);

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

# Fault text po-check.sh prints before " at line N" for each reported state.
my %FAULT_TEXT = (
	fuzzy => 'fuzzy entry',
	untranslated => 'untranslated entry',
	identical => 'identical translation',
);

# One singular entry per case, as [role, text] lines; a role names a line a
# fault reports. report maps each state po-check.sh names to the role of its
# report line; copied holds the flag the entry's work record carries, and is
# absent for an entry outside the work set.
my @CASES = (
	{ name => 'translated', context => '', source => 'segment', report => {},
		lines => [[msgid => 'msgid "segment"'], [msgstr => 'msgstr "сегмент"']] },
	{ name => 'untranslated', context => '', source => 'wire', copied => 0,
		report => { untranslated => 'msgid' },
		lines => [[msgid => 'msgid "wire"'], [msgstr => 'msgstr ""']] },
	{ name => 'exempt_sole', context => '', source => 'Hz', report => {},
		lines => [[undef, '# xnec2c-exempt: symbol'], [msgid => 'msgid "Hz"'],
			[msgstr => 'msgstr ""']] },
	{ name => 'exempt_two_markers', context => '', source => 'dBi', copied => 0,
		report => { untranslated => 'msgid' },
		lines => [[undef, '# xnec2c-exempt: symbol'], [undef, '# xnec2c-exempt: name'],
			[msgid => 'msgid "dBi"'], [msgstr => 'msgstr ""']] },
	{ name => 'fuzzy_first_flag', context => '', source => 'card', copied => 0,
		report => { fuzzy => 'flags' },
		lines => [[flags => '#, fuzzy'], [msgid => 'msgid "card"'],
			[msgstr => 'msgstr "картичка"']] },
	{ name => 'fuzzy_later_flag', context => '', source => '%d steps', copied => 0,
		report => { fuzzy => 'flags' },
		lines => [[flags => '#, c-format, fuzzy'], [msgid => 'msgid "%d steps"'],
			[msgstr => 'msgstr "%d чекори"']] },
	{ name => 'copied', context => '', source => 'node', copied => 1,
		report => { identical => 'msgstr' },
		lines => [[msgid => 'msgid "node"'], [msgstr => 'msgstr "node"']] },
	{ name => 'copied_escaped', context => '', source => 'say "go"', copied => 1,
		report => { identical => 'msgstr' },
		lines => [[msgid => 'msgid "say \"go\""'], [msgstr => 'msgstr "say \"go\""']] },
	{ name => 'multiline_untranslated', context => '', source => 'multi line',
		copied => 0, report => { untranslated => 'msgid' },
		lines => [[msgid => 'msgid ""'], [undef, '"multi "'], [undef, '"line"'],
			[msgstr => 'msgstr ""']] },
	{ name => 'context_untranslated', context => 'unit', source => 'wire',
		copied => 0, report => { untranslated => 'msgid' },
		lines => [[undef, 'msgctxt "unit"'], [msgid => 'msgid "wire"'],
			[msgstr => 'msgstr ""']] },
);

my $start = getcwd();
my $scratch = File::Temp->newdir('po_entry_state.XXXXXX', DIR => $start);
my $scripts = abs_path("$RealBin/../..");
mkdir "$scratch/po" or abort_setup("$scratch/po: unable to create: $!");
symlink $scripts, "$scratch/scripts"
	or abort_setup("$scratch/scripts: unable to link $scripts: $!");
chdir "$scratch" or abort_setup("$scratch: unable to enter: $!");

my $lines = write_catalog();
my @faults = (@{gate_faults($lines)}, @{selection_faults()});

chdir $start or abort_setup("$start: unable to return: $!");
if (@faults)
{
	print "FAIL: $_\n" for @faults;
	exit 1;
}
else
{
	print 'PASS: ' . scalar(@CASES)
		. " singular entry states judged alike by po-check.sh and"
		. " catalog_change_records\n";
	exit 0;
}

# Name every way po-check.sh misjudges the catalog: a wrong exit status, an
# expected fault it omits, or an entry fault no case expects.
sub gate_faults
{
	my ($case_lines) = @_;
	my @expected;
	my @faults;

	for my $case (@CASES)
	{
		for my $state (sort keys %{$case->{report}})
		{
			my $role = $case->{report}{$state};
			push @expected,
				"$FAULT_TEXT{$state} at line $case_lines->{$case->{name}}{$role}";
		}
	}

	my $output = qx(scripts/po/po-check.sh $CATALOG 2>&1);
	my $status = $? >> 8;
	push @faults, "po-check.sh exited $status, expected 1" if $status != 1;
	for my $fault (@expected)
	{
		push @faults, "po-check.sh names no '$fault':\n$output"
			if $output !~ /\Q$fault\E\b/;
	}

	my $fault_pattern = join('|', map { quotemeta } values %FAULT_TEXT);
	my $named = () = $output =~ /(?:$fault_pattern) at line \d+/g;
	push @faults, "po-check.sh names $named entry faults, expected "
		. scalar(@expected) . ":\n$output"
		if $named != scalar(@expected);

	return \@faults;
}

# Name the difference between the work set catalog_change_records selects and
# the cases holding a copied flag, in catalog order.
sub selection_faults
{
	my @wanted = map { record_text($_) } grep { defined $_->{copied} } @CASES;
	my @selected = map { record_text($_) } @{catalog_change_records($LANG)};
	my $wanted_text = join("\n", @wanted);
	my $selected_text = join("\n", @selected);

	my @faults;

	push @faults, "catalog_change_records selected:\n$selected_text\nexpected:\n"
		. $wanted_text
		if $selected_text ne $wanted_text;

	return \@faults;
}

# Render one work record or case as its identity and copied flag.
sub record_text
{
	my ($record) = @_;

	return "[$record->{context}] $record->{source} copied=$record->{copied}";
}

# Write the fixture header and every case entry as the scratch catalog, and
# return the line number of each case line holding a role, by case and role.
sub write_catalog
{
	my @text = (@HEADER, '');
	my %case_lines;

	for my $case (@CASES)
	{
		for my $line (@{$case->{lines}})
		{
			push @text, $line->[1];
			$case_lines{$case->{name}}{$line->[0]} = scalar(@text)
				if defined $line->[0];
		}
		push @text, '';
	}

	open my $fh, '>:encoding(UTF-8)', $CATALOG
		or abort_setup("$CATALOG: unable to write: $!");
	print {$fh} join("\n", @text) . "\n"
		or abort_setup("$CATALOG: unable to write: $!");
	close $fh or abort_setup("$CATALOG: unable to close: $!");

	return \%case_lines;
}

# Report a scratch-tree setup failure through the hard-error status.
sub abort_setup
{
	my ($reason) = @_;

	print "ERROR: $reason\n";
	exit 99;
}
