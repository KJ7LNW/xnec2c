package Xnec2c::PO::OutputJoin;

use strict;
use warnings;
use utf8;

use Exporter qw(import);

use Xnec2c::PO::MapFile qw(
	OWNER_PROGRAM decode_map_value read_manifest_records read_map_records
	record_line record_owner_tags
);
use Xnec2c::PO::TranslationMap qw(manifest_identity map_path output_path);
use Xnec2c::PO::TranslationRules qw(exemption_name resolved_target);

our @EXPORT_OK = qw(join_output_records);

# Name every way one positional output record departs from its manifest record.
sub record_contract_faults
{
	my ($path, $entry, $output) = @_;
	my @faults;

	for my $tag (record_owner_tags(OWNER_PROGRAM))
	{
		next if ($output->{$tag} // '') eq ($entry->{$tag} // '');
		push @faults, "$path:" . record_line($output, $tag)
			. ": K $output->{K} holds $tag " . ($output->{$tag} // '')
			. ' where the manifest writes ' . ($entry->{$tag} // '')
			. '; restore it';
	}

	return @faults;
}

# Construct one answer result; an unresolved answer contributes no translation.
sub answer_outcome
{
	my (%fields) = @_;

	return {
		faults => $fields{faults} // [],
		warnings => $fields{warnings} // [],
		translation => $fields{translation},
	};
}

# Resolve one structurally matched output answer against its manifest baseline.
sub answer_result
{
	my ($map_path, $out_path, $entry, $output) = @_;
	my @faults = record_contract_faults($out_path, $entry, $output);

	return answer_outcome(faults => \@faults) if @faults;

	my $named = manifest_identity($map_path, $entry);
	my $target_line = record_line($output, 'T');
	my $target_result = decode_map_value($out_path, $output->{K},
		$target_line, $output->{T} // '');

	return answer_outcome(faults => $target_result->{faults})
		if @{$target_result->{faults}};

	my $answered = exemption_name($output->{X} // '');
	my $inherited = $entry->{X} // '';
	my $line = ($output->{X} // '') ne ''
		? record_line($output, 'X') : $target_line;
	my @warnings;
	my $translation;

	push @faults, "$out_path:$line: K $output->{K} replaces inherited"
		. " exemption $inherited with $answered; keep $inherited or"
		. ' write a translation in T without X'
		if $inherited ne '' && defined $answered
			&& $answered ne $inherited;

	my $resolution = resolved_target($named->{source},
		$target_result->{value}, $output->{X} // '');
	push @faults, "$out_path:$line: K $output->{K}:"
		. " $resolution->{error}" if exists $resolution->{error};
	push @warnings, "$out_path:$line: K $output->{K}:"
		. " WARN $resolution->{warning}"
		if exists $resolution->{warning};
	$translation = {
		identity => $named->{identity},
		record => {
			key => $entry->{K},
			line => $target_line,
			%{$resolution},
		},
	} if !@faults;

	return answer_outcome(faults => \@faults, warnings => \@warnings,
		translation => $translation);
}

# Construct one join result; a fault set leaves every answer unresolved.
sub join_result
{
	my (%fields) = @_;

	return {
		translations => $fields{translations} // {},
		faults => $fields{faults} // [],
		warnings => $fields{warnings} // [],
	};
}

# Join one output map to its authoritative manifest in manifest order.
sub join_output_records
{
	my ($lang) = @_;
	my $map_path = map_path($lang);
	my $out_path = output_path($lang);
	my $manifest = read_manifest_records($map_path);
	my $output_result = read_map_records($out_path);

	# Parser faults already name malformed records; no structural comparison
	# can add a reliable location until those records parse.
	return join_result(faults => $output_result->{faults})
		if @{$output_result->{faults}};

	my @faults;
	my @warnings;
	my %translations;

	if (@{$output_result->{records}} != @{$manifest})
	{
		push @faults, "$out_path holds "
			. scalar(@{$output_result->{records}})
			. ' records where ' . $map_path . ' holds '
			. scalar(@{$manifest})
			. '; restore the manifest record set and order';
	}
	else
	{
		for my $index (0 .. $#{$manifest})
		{
			my $answer = answer_result($map_path, $out_path,
				$manifest->[$index], $output_result->{records}[$index]);
			push @faults, @{$answer->{faults}};
			push @warnings, @{$answer->{warnings}};
			next if !defined $answer->{translation};

			my $identity = $answer->{translation}{identity};
			die "$map_path: duplicate catalog identity at K"
				. " $manifest->[$index]{K}\n"
				if exists $translations{$identity};
			$translations{$identity} = $answer->{translation}{record};
		}
	}

	return join_result(translations => \%translations, faults => \@faults,
		warnings => \@warnings);
}

1;
