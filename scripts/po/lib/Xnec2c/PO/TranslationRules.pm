package Xnec2c::PO::TranslationRules;

use strict;
use warnings;
use utf8;

use Exporter qw(import);

use Xnec2c::PO::MapFile qw(
	OWNER_MODEL OWNER_PROGRAM record_owner_tags record_tag_order source_excerpt
);

our @EXPORT_OK = qw(
	exemption_conflict exemption_name exemption_names exemption_propagates
	exemption_rules record_rules repair_rules resolved_target
);

# Each exemption states why a source carries no distinct target form. A reason
# resting on the source alone holds in every language, so an acceptance
# recorded by one catalog reaches the manifests generated after it; a loanword
# rests on the adopting language's own vocabulary and reaches no other.
my %EXEMPTION_MAP = (
	1 => {
		name => 'name',
		propagates => 1,
		use => 'a product, a person, an algorithm, a declared identifier,'
			. ' or a source composed only of declared identifiers, constants,'
			. ' format specifiers, and nonlinguistic syntax',
	},
	2 => {
		name => 'notation',
		propagates => 1,
		use => 'a token fixed by the NEC deck, engineering convention, or a'
			. ' program-written file or diagnostic record format: a unit, an'
			. ' axis or field label, a Greek letter, a card mnemonic, an'
			. ' impedance token, or an S-parameter token',
	},
	3 => {
		name => 'loanword',
		propagates => 0,
		use => 'a source whose form this language adopts as written, one'
			. ' another language does translate',
	},
	4 => {
		name => 'symbol',
		propagates => 1,
		use => 'a source holding no word at all: punctuation, digits, format'
			. ' specifiers, and markup',
		excludes => \&source_has_word_token,
		fault => 'contains a word-bearing token; a unit takes notation, a'
			. ' declared identifier takes name, and ordinary prose is translated',
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

# Name the documented exemption one X value spells, or undef where it names
# none. A documented code and a documented name both resolve here, so every
# caller reads one spelling.
sub exemption_name
{
	my ($code) = @_;
	my $row = $EXEMPTION_ROW_MAP{$code};

	return defined $row ? $row->{name} : undef;
}

# Report whether one documented reason rests on the source alone, so the
# catalog accepting it answers every catalog generated after it.
sub exemption_propagates
{
	my ($name) = @_;
	my $row = $EXEMPTION_ROW_MAP{$name};

	return defined $row && $row->{propagates} ? 1 : 0;
}

# Report whether a source contains a word-bearing token once its format
# specifiers and markup tags are set aside. Such a source is not wordless, so
# it cannot take the residual symbol reason.
sub source_has_word_token
{
	my ($source) = @_;
	my $residue = $source;

	$residue =~ s/%[-+ #0-9.*']*(?:hh|h|ll|l|L|q|j|z|t)?[diouxXeEfFgGaAcspn%]//g;
	$residue =~ s/<[^<>]*>//g;

	return $residue =~ /[\p{L}_]/ ? 1 : 0;
}

# Name the fault one documented reason draws from the source it annotates, or
# undef where that source suits it. A reason resting on knowledge the source
# text does not carry states no rule and answers undef, taking its review from
# the catalogs instead.
sub exemption_conflict
{
	my ($name, $source) = @_;
	my $row = $EXEMPTION_ROW_MAP{$name};
	my $result;

	if (!defined $row || !exists $row->{excludes})
	{
		# An undocumented name and a reason without a content rule each draw
		# their report elsewhere.
		$result = undef;
	}
	elsif ($row->{excludes}->($source))
	{
		$result = $row->{fault};
	}
	else
	{
		$result = undef;
	}

	return $result;
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
		elsif ($target ne '' && $target eq $source)
		{
			$result = { error => "T repeats S \"$excerpt\"; empty T beside an X"
				. ' line, which records that this language writes no'
				. ' translation for S' };
		}
		elsif ($target ne '')
		{
			$result = { error => "X $code stands beside a translated T; drop"
				. ' the X line to keep that translation, or empty T to record'
				. " that S \"$excerpt\" has no distinct form" };
		}
		else
		{
			$result = {
				target => '',
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
		'- Leave every ' . join(', ', record_owner_tags(OWNER_PROGRAM))
			. ' line as written.',
		'- Change only ' . join(' and ', record_owner_tags(OWNER_MODEL))
			. ' values.',
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

- The reasons read in precedence order. Write the first reason whose
  description fits S.
- A reason describes the whole of S. A sentence containing ordinary prose is
  translated even when it also contains a unit, identifier, or format
  specifier. A source composed entirely of fixed tokens takes the first reason
  that covers those tokens.
- Write X beside an empty T. An X records that this language writes no
  translation for S, so the catalog stores none and never repeats S.
- Every reason but loanword rests on S alone and so holds in every language,
  and every catalog names that one reason for S. A loanword rests on this
  language's own vocabulary and answers for this language alone; another
  catalog translating S calls for that reading, and this language's own usage
  settles it.
- An X already standing in a record names the reason every catalog holds for
  that source. Keep it beside an empty T where this language also holds no
  distinct form; replace it with a translation in T, dropping the X line, where
  this language does. Write no other reason in its place.
- Lint warns on every accepted X. Read every warning, translate each record
  that has a distinct target form, then run lint again.
- Write no X on a translated record.
RULES
}

1;
