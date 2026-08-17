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
		use => 'a product, a person, an algorithm, or a source composed only'
			. ' of names, declared identifiers, constants, format specifiers,'
			. ' and nonlinguistic syntax',
	},
	2 => {
		name => 'notation',
		propagates => 1,
		use => 'a source composed only of tokens fixed by the NEC deck,'
			. ' engineering convention, or a program-written file or diagnostic'
			. ' record format, plus nonlinguistic syntax: units, axis or field'
			. ' labels, Greek letters, card mnemonics, impedance tokens, or'
			. ' S-parameter tokens',
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
		fault => 'contains a word-bearing token; a unit takes notation, an'
			. ' identifier naming a program entity takes name, and ordinary prose'
			. ' is translated',
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
		'- Write an empty T as bare T. Write each nonempty field as its tag,'
			. ' one tab, then its value.',
		'- Leave every ' . join(', ', record_owner_tags(OWNER_PROGRAM))
			. ' line as written.',
		'- Change only ' . join(' and ', record_owner_tags(OWNER_MODEL))
			. ' values.',
		'- Preserve every PO escape and printf placeholder from S exactly.',
		'- Keep an identifier S binds to a placeholder through = joined to it,'
			. ' as in name=%d; word every qualifier outside that pair.',
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

- Apply the first fitting reason in the listed precedence.
- Classify each word-bearing token by its role in S; use name only where the
  token names a program entity. Treat matching identifier spelling as
  role-neutral.
- Translate diagnostic labels, eg BUG, ERROR, and WARNING, even when uppercase.
- Use X only where one reason describes the whole of S. Ignore punctuation,
  digits, format specifiers, and markup when deciding whether its word-bearing
  tokens fit that reason.
- Translate S when any word-bearing token is ordinary prose; retain embedded
  names, identifiers, notation, constants, format specifiers, and syntax as
  required by the language rules.
- For no distinct target form, write bare T and an X reason.
- Apply name, notation, and symbol across every language and catalog; apply
  loanword only to the language whose vocabulary adopts S.
- Preserve an existing X reason where this language has no distinct form;
  otherwise replace it with a translation in T and drop X.
- Review every lint exemption warning; translate each record with a distinct
  target form, then run lint again.
RULES
}

1;
