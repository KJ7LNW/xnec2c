package Xnec2c::PO::TranslationRules;

use strict;
use warnings;
use utf8;

use Exporter qw(import);

use Xnec2c::PO::MapFile qw(record_tag_order source_excerpt);

our @EXPORT_OK = qw(
	exemption_names exemption_rules propagating_exemptions record_rules
	repair_rules resolved_target
);

# Each exemption states why a source carries no distinct target form. A reason
# resting on the source alone holds in every language, so an acceptance
# recorded by one catalog reaches the manifests generated after it; a loanword
# rests on the adopting language's own vocabulary and reaches no other.
my %EXEMPTION_MAP = (
	1 => {
		name => 'symbol',
		propagates => 1,
		use => 'a source holding no word: units, numbers, punctuation, format'
			. ' specifiers, and identifier text such as function, variable,'
			. ' and constant names',
	},
	2 => {
		name => 'name',
		propagates => 1,
		use => 'a name: a product, a person, an algorithm, or an identifier'
			. ' the source code declares',
	},
	3 => {
		name => 'notation',
		propagates => 1,
		use => 'notation fixed by the NEC deck, by engineering convention, or'
			. ' by a file format this program writes: a Greek letter, an axis'
			. ' label, a unit, a card mnemonic',
	},
	4 => {
		name => 'loanword',
		propagates => 0,
		use => 'the English word this language adopts unchanged',
	},
);

# Resolve either spelling an X line carries, its documented code or its name,
# to the one row that documents it.
my %EXEMPTION_ROW_MAP = map {
	($_ => $EXEMPTION_MAP{$_}, $EXEMPTION_MAP{$_}{name} => $EXEMPTION_MAP{$_})
} keys %EXEMPTION_MAP;

# Name every documented exemption in its presentation order.
sub exemption_names
{
	return map { $EXEMPTION_MAP{$_}{name} } sort keys %EXEMPTION_MAP;
}

# Name the exemptions one language's acceptance carries to the languages
# generated after it.
sub propagating_exemptions
{
	return map { $EXEMPTION_MAP{$_}{name} }
		grep { $EXEMPTION_MAP{$_}{propagates} } sort keys %EXEMPTION_MAP;
}

# Resolve what one output record contributes: a translation, a target held by
# an exemption, or the rejection that clears it.
sub resolved_target
{
	my ($source, $target, $code) = @_;
	my $excerpt = source_excerpt($source);
	my $result;

	if ($code eq '')
	{
		if ($target eq '')
		{
			$result = { error => "no answer; write this language's form of S"
				. " \"$excerpt\" in T, or an X line naming the reason S has no"
				. ' distinct form' };
		}
		elsif ($target ne $source)
		{
			$result = { target => $target };
		}
		else
		{
			$result = { error => "T repeats S \"$excerpt\"; write a target"
				. ' differing from S, or an X line naming the reason S has no'
				. ' distinct form' };
		}
	}
	else
	{
		my $row = $EXEMPTION_ROW_MAP{$code};
		if (!defined $row)
		{
			$result = { error => "X $code names no documented exemption; write"
				. " the code or name of one documented reason for S \"$excerpt\"" };
		}
		elsif ($target ne '' && $target ne $source)
		{
			$result = { error => "X $code stands beside a translated T; drop"
				. ' the X line to keep that translation, or empty T to take the'
				. " source form of S \"$excerpt\"" };
		}
		else
		{
			$result = {
				target => $source,
				exempt => $row->{name},
				warning => "exemption $row->{name} claimed for S \"$excerpt\"",
			};
		}
	}

	return $result;
}

# State the tagged-record contract both prompts impose on an output map.
sub record_rules
{
	return join("\n",
		'- A record reads ' . record_tag_order() . ', one tag per line,'
			. ' omitting C and X when they hold nothing.',
		'- Leave every K, C, S, and L line as written.',
		'- Change only T and X values.',
		'- Preserve every PO escape and printf placeholder from S exactly.',
		'- Copy every \\s in S as written; write \\s in T wherever the target'
			. ' needs an edge space.');
}

# State how a reported fault is repaired, the editing contract both prompts
# impose.
sub repair_rules
{
	return join("\n",
		'- Apply every correction with Edit, at the line the report names.',
		'- Retain every valid record; never replace the whole output file.',
		'- Write and run no program that rewrites this map.');
}

# State the answer every record carries, and the warning an exemption draws.
sub exemption_rules
{
	my $guide = join("\n", map {
		"    $_ $EXEMPTION_MAP{$_}{name}: $EXEMPTION_MAP{$_}{use}"
	} sort keys %EXEMPTION_MAP);

	return <<"RULES" =~ s/\n\z//r;
# The X line

- Answer every record with a translation in T.
- Use X only where this language holds no distinct form for S:

$guide

- An X already standing in a record names an exemption a preceding catalog
  accepted for the same source. Keep it where this language also holds no
  distinct form; replace it with a translation in T where this language does.
- Lint warns on every accepted X. Read every warning, translate each record
  that has a distinct target form, then run lint again.
- Write no X on a translated record.
RULES
}

1;
