package Xnec2c::PO::TranslationMap;

use strict;
use warnings;
use utf8;

use Exporter qw(import);

use Xnec2c::PO::CatalogState qw(catalog_identity);
use Xnec2c::PO::MapFile qw(
	decode_map_value encode_map_value read_manifest_records read_map_records
	record_line serialize_records write_utf8_file
);
use Xnec2c::PO::TranslationRules qw(exemption_propagates);

our @EXPORT_OK = qw(
	AI_DIR manifest_identity manifest_records map_path output_path
	prompt_path rebase_output_records
);

use constant AI_DIR => 'po/ai';

# Name the workspace artifacts one language owns.
sub map_path
{
	my ($lang) = @_;
	return AI_DIR . "/$lang.map";
}

# Name one language's model output artifact.
sub output_path
{
	my ($lang) = @_;
	return AI_DIR . "/$lang.out.map";
}

# Name one language's generated prompt artifact.
sub prompt_path
{
	my ($lang) = @_;
	return AI_DIR . "/$lang.prompt.md";
}

# Build the tagged manifest records answering one catalog's current changes,
# keying them by their position in that catalog. A source a preceding catalog
# already exempted carries that reason here, so the program owns the inherited
# answer and every later reader compares against this one baseline.
sub manifest_records
{
	my ($lang, $changes, $exemptions) = @_;
	my $inherited = propagated_answers($exemptions);
	my @records;

	for my $change (@{$changes})
	{
		my $identity = catalog_identity($change->{context},
			$change->{source});
		push @records, {
			K => scalar(@records) + 1,
			L => $lang,
			C => encode_map_value($change->{context}),
			S => encode_map_value($change->{source}),
			T => '',
			X => $inherited->{$identity} // '',
		};
	}

	return \@records;
}

# Decode the catalog entry one manifest record names.
sub manifest_identity
{
	my ($path, $record) = @_;
	my $context = decode_map_value($path, $record->{K},
		record_line($record, 'C'), $record->{C} // '');
	my $source = decode_map_value($path, $record->{K},
		record_line($record, 'S'), $record->{S});

	die join("\n", @{$context->{faults}}, @{$source->{faults}}) . "\n"
		if @{$context->{faults}} || @{$source->{faults}};

	return {
		source => $source->{value},
		identity => catalog_identity($context->{value}, $source->{value}),
	};
}

# Index output records by decoded C and S identity, retaining their original
# line metadata so every fault points back to the model-owned file.
sub index_answers_by_identity
{
	my ($path, $records) = @_;
	my (%answers, @faults);

	for my $record (@{$records})
	{
		my $context_result = decode_map_value($path, $record->{K},
			record_line($record, 'C'), $record->{C} // '');
		my $source_result = decode_map_value($path, $record->{K},
			record_line($record, 'S'), $record->{S} // '');
		push @faults, @{$context_result->{faults}}, @{$source_result->{faults}};
		next if @{$context_result->{faults}} || @{$source_result->{faults}};

		my $identity = catalog_identity($context_result->{value},
			$source_result->{value});
		if (exists $answers{$identity})
		{
			push @faults, "$path:" . record_line($record, 'S')
				. ": K $record->{K} repeats the S of K $answers{$identity}{K};"
				. ' keep one record per entry';
			next;
		}
		$answers{$identity} = $record;
	}

	return { answers => \%answers, faults => \@faults };
}

# Reduce the exemptions preceding catalogs already accept to the answer each
# source carries forward. A reason resting on the source alone holds in every
# language, so the acceptance nearest the reading language names the reason
# every unanswered record of that source takes.
sub propagated_answers
{
	my ($exemptions) = @_;
	my %answers;

	for my $exemption (@{$exemptions})
	{
		next if !exemption_propagates($exemption->{exempt});
		my $identity = catalog_identity($exemption->{context},
			$exemption->{source});
		$answers{$identity} = $exemption->{exempt};
	}

	return \%answers;
}

# Construct one rebase result; a fault set leaves every count at zero.
sub rebase_result
{
	my (%fields) = @_;

	return {
		faults => $fields{faults} // [],
		warnings => $fields{warnings} // [],
		carried => $fields{carried} // 0,
		open => $fields{open} // 0,
		propagated => $fields{propagated} // 0,
		changed => $fields{changed} // 0,
	};
}

# Return answers the current manifest no longer asks for in record-key order.
sub retired_answers
{
	my ($answers) = @_;
	return map { $answers->{$_} }
		sort { $answers->{$a}{K} <=> $answers->{$b}{K} } keys %{$answers};
}

# Rewrite one output map in the current manifest's order and numbering,
# carrying every answer across by the identity its C and S name. The program
# owns the K, C, S, and L structure; the model owns only T and X, so record
# drift costs no model session. An identity the manifest no longer holds is
# reported and dropped. An unanswered record takes the inherited exemption its
# manifest record carries, and arrives empty where none did. A language holding
# no output map yet takes the whole manifest, so every answer the program owns
# stands in the file before any session opens it.
sub rebase_output_records
{
	my ($lang) = @_;
	my $map_path = map_path($lang);
	my $out_path = output_path($lang);
	my $manifest = read_manifest_records($map_path);
	my $output_result = -e $out_path
		? read_map_records($out_path)
		: { records => [], faults => [], content => undef };
	my $existing = $output_result->{content};

	return rebase_result(faults => $output_result->{faults})
		if @{$output_result->{faults}};

	my $index = index_answers_by_identity($out_path,
		$output_result->{records});

	return rebase_result(faults => $index->{faults}) if @{$index->{faults}};

	my %answers = %{$index->{answers}};
	my @records;
	my $carried = 0;
	my $open = 0;
	my $propagated = 0;

	for my $entry (@{$manifest})
	{
		my $identity = manifest_identity($map_path, $entry)->{identity};
		my $output = delete $answers{$identity};
		my $record = {
			K => $entry->{K},
			C => $entry->{C} // '',
			S => $entry->{S},
			L => $entry->{L},
			T => defined $output ? ($output->{T} // '') : '',
			X => defined $output ? ($output->{X} // '') : '',
		};
		my $unanswered = $record->{T} eq '' && $record->{X} eq '';
		$carried++ if defined $output;
		if ($unanswered && ($entry->{X} // '') ne '')
		{
			$record->{X} = $entry->{X};
			$propagated++;
		}
		else
		{
			$open++ if $unanswered;
		}
		push @records, $record;
	}

	my $rebased = serialize_records(\@records);
	my $changed = !defined $existing || $rebased ne $existing ? 1 : 0;
	write_utf8_file($out_path, $rebased) if $changed;

	my @warnings = map {
		"$out_path: K $_->{K}: WARN answers no current manifest entry; dropped"
	} retired_answers(\%answers);

	return rebase_result(warnings => \@warnings, carried => $carried,
		open => $open, propagated => $propagated, changed => $changed);
}

1;
