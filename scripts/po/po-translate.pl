#!/usr/bin/env perl

use strict;
use warnings;
use utf8;

use Cwd qw(abs_path);
use File::Path qw(make_path);
use FindBin qw($RealBin);
use Getopt::Long qw(GetOptionsFromArray);
use Locale::PO;

binmode STDOUT, ':encoding(UTF-8)';
binmode STDERR, ':encoding(UTF-8)';

my $AI_DIR = 'po/ai';
my $ROOT = abs_path("$RealBin/../..");
my $MODEL = 'gpt-5.6-terra(low)[1m]';
my @RECORD_FORMAT = (
	{ tag => 'K', optional => 0 },
	{ tag => 'L', optional => 0 },
	{ tag => 'C', optional => 1 },
	{ tag => 'S', optional => 0 },
	{ tag => 'T', optional => 0 },
);
my %RECORD_TAG_MAP = map { $_->{tag} => $_ } @RECORD_FORMAT;

my %COMMAND_MAP = (
	generate => { arg_count => 0, handler => \&cmd_generate },
	apply => { arg_count => 0, handler => \&cmd_apply },
	inject => { arg_count => 0, handler => \&cmd_inject },
	lint => { arg_count => 1, handler => \&cmd_lint },
);

my $options_ok = GetOptionsFromArray(
	\@ARGV,
	'model|m=s' => \$MODEL,
);
usage() if !$options_ok;

my $command = shift @ARGV // '';
my $command_spec = $COMMAND_MAP{$command};
usage() if !defined $command_spec;
usage() if scalar(@ARGV) != $command_spec->{arg_count};
$command_spec->{handler}->(@ARGV);

exit 0;

# Report command syntax and terminate with a usage failure.
sub usage
{
	die "usage: $0 [--model NAME] generate|apply|inject\n"
		. "       $0 [--model NAME] lint <lang>\n";
}

# Read a complete UTF-8 text file.
sub read_utf8_file
{
	my ($path) = @_;

	open my $file, '<:encoding(UTF-8)', $path
		or die "$path: unable to read: $!\n";
	local $/;
	my $content = <$file>;
	close $file or die "$path: unable to close after reading: $!\n";

	return $content;
}

# Write a complete UTF-8 text file.
sub write_utf8_file
{
	my ($path, $content) = @_;

	open my $file, '>:encoding(UTF-8)', $path
		or die "$path: unable to write: $!\n";
	print {$file} $content
		or die "$path: unable to write content: $!\n";
	close $file or die "$path: unable to close after writing: $!\n";
}

# Encode one map value without raw record delimiters.
sub encode_map_value
{
	my ($value) = @_;

	$value =~ s/\\/\\\\/g;
	$value =~ s/\n/\\n/g;
	$value =~ s/\t/\\t/g;
	$value =~ s/\r/\\r/g;
	$value =~ s/"/\\"/g;

	return $value;
}

# Decode one map value and reject unsupported escape sequences.
sub decode_map_value
{
	my ($path, $record_key, $value) = @_;
	my %escape_map = (
		n => "\n",
		t => "\t",
		r => "\r",
		'"' => '"',
		'\\' => '\\',
	);

	$value =~ s/\\(.)/
		exists $escape_map{$1}
			? $escape_map{$1}
			: die "$path: record $record_key has unsupported escape \\$1\n"
	/gex;

	return $value;
}

# Serialize records in the sole authoritative tag order.
sub serialize_records
{
	my ($records) = @_;
	my @blocks;

	for my $record (@{$records})
	{
		my @lines;
		for my $field (@RECORD_FORMAT)
		{
			my $tag = $field->{tag};
			next if $field->{optional} && $record->{$tag} eq '';
			push @lines, "$tag\t$record->{$tag}";
		}
		push @blocks, join("\n", @lines);
	}

	return @blocks ? join("\n\n", @blocks) . "\n" : '';
}

# Parse and validate tagged map records.
sub read_records
{
	my ($path) = @_;
	my $content = read_utf8_file($path);
	$content =~ s/\n+\z//;
	return [] if $content eq '';
	my @blocks = split(/\n\n+/, $content, -1);
	my @records;
	my %seen_keys;

	for my $block (@blocks)
	{
		my %record;
		my @actual_tags;

		for my $line (split(/\n/, $block, -1))
		{
			my ($tag, $value) = $line =~ /\A([A-Z])\t(.*)\z/s;
			die "$path: malformed record line: $line\n" if !defined $tag;
			die "$path: record value contains a raw delimiter\n"
				if $value =~ /[\t\r]/;
			die "$path: unsupported $tag tag\n" if !exists $RECORD_TAG_MAP{$tag};
			die "$path: duplicate $tag tag in one record\n" if exists $record{$tag};
			$record{$tag} = $value;
			push @actual_tags, $tag;
		}

		die "$path: record is missing K\n" if !exists $record{K};
		die "$path: duplicate K $record{K}\n" if $seen_keys{$record{K}}++;
		die "$path: record $record{K} has invalid K\n"
			if $record{K} !~ /\A[1-9][0-9]*\z/;

		my @expected_tags = map { $_->{tag} }
			grep { !$_->{optional} || ($record{$_->{tag}} // '') ne '' }
			@RECORD_FORMAT;
		die "$path: record $record{K} tags are not in the required order\n"
			if join("\x1f", @actual_tags) ne join("\x1f", @expected_tags);

		push @records, \%record;
	}

	return \@records;
}

# Derive the catalog identity fields through Locale::PO.
sub po_identity
{
	my ($po) = @_;
	my $context = Locale::PO->dequote($po->msgctxt);
	my $source = Locale::PO->dequote($po->msgid);

	$context = '' if !defined $context;
	die "catalog entry has no msgid\n" if !defined $source;

	return ($context, $source, join("\x04", $context, $source));
}

# Load one catalog with the required encoding contract.
sub load_catalog
{
	my ($path) = @_;
	my $catalog = Locale::PO->load_file_asarray($path, 'utf8');

	die "$path: Locale::PO failed to load catalog\n" if !defined $catalog;

	# po-refresh.sh strips obsolete entries from every catalog it writes, so
	# one reaching here means the catalog came from an unstripped path.
	my $obsolete = grep { $_->obsolete } @{$catalog};
	die "$path: $obsolete obsolete entries; run make po-refresh\n" if $obsolete != 0;

	return $catalog;
}

# Build map records for fuzzy and untranslated catalog entries.
sub change_set
{
	my ($lang) = @_;
	my $path = "po/$lang.po";
	my $catalog = load_catalog($path);
	my @records;
	my %seen_identities;

	for my $po (@{$catalog})
	{
		my $translation = Locale::PO->dequote($po->msgstr);
		my $selected = $po->fuzzy || !defined $translation || $translation eq '';
		next if !$selected;

		my ($context, $source, $identity) = po_identity($po);
		die "$path: duplicate catalog identity for msgid $source\n"
			if $seen_identities{$identity}++;

		push @records, {
			K => scalar(@records) + 1,
			L => $lang,
			C => encode_map_value($context),
			S => encode_map_value($source),
			T => '',
		};
	}

	return \@records;
}

# Compose model directions with the language-specific output path.
sub output_directions
{
	my ($lang) = @_;

	return <<"DIRECTIONS";
# Output format

- Reproduce every K, L, C, and S line verbatim and in order.
- Fill only each T value.
- Preserve every PO escape and printf placeholder from S exactly.
- Translate each T value under the preceding global and language rules.
- Leave no T value empty.
- Write only the complete mapping to po/ai/$lang.out.map.
DIRECTIONS
}

# Generate the authoritative map and model-facing prompt for one language.
sub generate_language
{
	my ($lang, $records) = @_;
	my $serialized_records = serialize_records($records);
	my $map_path = "$AI_DIR/$lang.map";
	my $prompt_path = "$AI_DIR/$lang.prompt.md";
	my $global_rules = read_utf8_file('doc/TRANSLATING.md');
	my $language_rules = read_utf8_file("po/rules/$lang.md");
	my $prompt = $global_rules . "\n\n" . $language_rules . "\n\n"
		. output_directions($lang) . "\n" . $serialized_records;

	write_utf8_file($map_path, $serialized_records);
	write_utf8_file($prompt_path, $prompt);
	print "$lang: generated " . scalar(@{$records}) . " records\n";
}

# Join an output map to its authoritative manifest by record key.
sub joined_translations
{
	my ($lang) = @_;
	my $map_path = "$AI_DIR/$lang.map";
	my $out_path = "$AI_DIR/$lang.out.map";
	my $manifest_records = read_records($map_path);
	my $output_records = read_records($out_path);
	my %output_by_key = map { $_->{K} => $_ } @{$output_records};
	my %translations;

	for my $manifest (@{$manifest_records})
	{
		my $key = $manifest->{K};
		my $output = delete $output_by_key{$key};
		die "$out_path: missing K $key\n" if !defined $output;

		for my $tag (qw(L C S))
		{
			my $expected = $manifest->{$tag} // '';
			my $actual = $output->{$tag} // '';
			die "$out_path: record $key has altered $tag\n"
				if $actual ne $expected;
		}

		die "$out_path: record $key has an empty T\n" if $output->{T} eq '';
		my $context = decode_map_value($map_path, $key, $manifest->{C} // '');
		my $source = decode_map_value($map_path, $key, $manifest->{S});
		my $target = decode_map_value($out_path, $key, $output->{T});
		my $identity = join("\x04", $context, $source);
		die "$out_path: duplicate catalog identity at record $key\n"
			if exists $translations{$identity};
		$translations{$identity} = {
			key => $key,
			target => $target,
		};
	}

	my @extra_keys = sort { $a <=> $b } keys %output_by_key;
	die "$out_path: unexpected K $extra_keys[0]\n" if @extra_keys;

	return \%translations;
}

# Set an unquoted translation while preserving literal backslashes.
sub set_msgstr
{
	my ($po, $target) = @_;

	# Double each backslash so Locale::PO stores literal backslashes in msgstr.
	$target =~ s/\\/\\\\/g;
	$po->msgstr($target);
	$po->fuzzy(0);
}

# Prepare a translated catalog without choosing its output destination.
sub translated_catalog
{
	my ($lang, $translations) = @_;
	my $path = "po/$lang.po";
	my $catalog = load_catalog($path);
	my %catalog_by_identity;

	for my $po (@{$catalog})
	{
		my (undef, $source, $identity) = po_identity($po);
		die "$path: duplicate catalog identity for msgid $source\n"
			if exists $catalog_by_identity{$identity};
		$catalog_by_identity{$identity} = $po;
	}

	for my $identity (keys %{$translations})
	{
		my $record = $translations->{$identity};
		my $po = $catalog_by_identity{$identity};
		die "$path: no catalog identity for map record $record->{key}\n"
			if !defined $po;
		set_msgstr($po, $record->{target});
	}

	return $catalog;
}

# Canonicalize one translated catalog through the shared refresh transaction.
sub refresh_catalog
{
	my ($path) = @_;
	my $status = system('scripts/po/po-refresh.sh', $path);

	assert_command_status("$path: scripts/po/po-refresh.sh", $status);
}

# Save and canonicalize joined translations in their authoritative catalog.
sub inject_language
{
	my ($lang, $translations) = @_;
	my $path = "po/$lang.po";
	my $catalog = translated_catalog($lang, $translations);

	Locale::PO->save_file_fromarray($path, $catalog, 'utf8');
	refresh_catalog($path);
}

# Propagate a child command failure with its process outcome.
sub assert_command_status
{
	my ($name, $status) = @_;

	die "$name: unable to run: $!\n" if $status == -1;
	die "$name: terminated by signal " . ($status & 127) . "\n"
		if ($status & 127) != 0;
	die "$name: failed with status " . ($status >> 8) . "\n"
		if ($status >> 8) != 0;
}

# Accept the stop-on-done exit code while propagating other launch failures.
sub assert_claude_inject_status
{
	my ($lang, $status) = @_;

	if (($status & 127) == 0 && ($status >> 8) == 143)
	{
		print "$lang: claude-inject stopped after done\n";
	}
	else
	{
		assert_command_status("$lang: claude-inject", $status);
	}
}

# Run the shared translation gate against one catalog path.
sub trans_check
{
	my ($path) = @_;
	my $status = system('scripts/po/po-check.sh', $path);

	assert_command_status("$path: scripts/po/po-check.sh", $status);
}

# Remove generated workspace artifacts before creating a new manifest set.
sub clear_ai_workspace
{
	opendir my $directory, $AI_DIR
		or die "$AI_DIR: unable to read directory: $!\n";
	my @paths = map { "$AI_DIR/$_" }
		grep { /\A[A-Za-z_]+\.(?:map|out\.map|prompt\.md|lint\.po)\z/ }
		readdir $directory;
	closedir $directory
		or die "$AI_DIR: unable to close directory: $!\n";

	for my $path (@paths)
	{
		unlink $path or die "$path: unable to remove stale artifact: $!\n";
	}
}

# Enumerate catalog languages from repository filenames.
sub catalog_languages
{
	opendir my $directory, 'po' or die "po: unable to read directory: $!\n";
	my @languages = sort map { /\A([A-Za-z_]+)\.po\z/ ? $1 : () }
		readdir $directory;
	closedir $directory or die "po: unable to close directory: $!\n";
	die "po: no translation catalogs found\n" if !@languages;

	return @languages;
}

# Enumerate authoritative map languages from workspace filenames.
sub map_languages
{
	opendir my $directory, $AI_DIR
		or die "$AI_DIR: unable to read directory: $!\n";
	my @languages = sort map { /\A([A-Za-z_]+)\.map\z/ ? $1 : () }
		readdir $directory;
	closedir $directory
		or die "$AI_DIR: unable to close directory: $!\n";
	die "$AI_DIR: no authoritative map files found\n" if !@languages;

	return @languages;
}

# Validate a language before deriving repository paths from it.
sub validate_language
{
	my ($lang) = @_;

	die "invalid language: $lang\n" if $lang !~ /\A[A-Za-z_]+\z/;
}

# Generate work files for every catalog requiring translation.
sub cmd_generate
{
	make_path($AI_DIR);
	clear_ai_workspace();
	for my $lang (catalog_languages())
	{
		validate_language($lang);
		my $records = change_set($lang);
		if (@{$records})
		{
			generate_language($lang, $records);
		}
		else
		{
			print "$lang: no fuzzy or untranslated entries\n";
		}
	}
}

# Decode one quoted PO string line through the catalog's own dequoting, so a
# parsed identity matches the identity Locale::PO derives for the same entry.
sub unquote_po_line
{
	my ($quoted) = @_;

	return Locale::PO->dequote($quoted);
}

# Index every generated catalog entry by its starting line and map record key.
sub lint_key_index
{
	my ($path, $translations) = @_;
	my %key_by_identity = map { $_ => $translations->{$_}{key} }
		keys %{$translations};
	my @entries;
	my %entry;
	my $line_number = 0;

	for my $line (split /\n/, read_utf8_file($path), -1)
	{
		$line_number++;
		if ($line =~ /\A\s*\z/)
		{
			push @entries, {%entry} if exists $entry{start};
			%entry = ();
			next;
		}

		$entry{start} = $line_number if !exists $entry{start};
		if ($line =~ /\Amsgctxt[ \t]+(".*")[ \t]*\z/)
		{
			$entry{field} = 'C';
			$entry{C} = unquote_po_line($1);
		}
		elsif ($line =~ /\Amsgid[ \t]+(".*")[ \t]*\z/)
		{
			$entry{field} = 'S';
			$entry{S} = unquote_po_line($1);
		}
		elsif ($line =~ /\A(".*")[ \t]*\z/ && defined $entry{field})
		{
			$entry{$entry{field}} .= unquote_po_line($1);
		}
		else
		{
			delete $entry{field};
		}
	}
	push @entries, {%entry} if exists $entry{start};

	for my $found (@entries)
	{
		my $identity = join("\x04", $found->{C} // '', $found->{S} // '');
		$found->{key} = $key_by_identity{$identity};
	}

	return [sort { $a->{start} <=> $b->{start} } @entries];
}

# Name the map record owning one generated catalog line.
sub lint_line_origin
{
	my ($index, $out_path, $line_number) = @_;
	my $found;

	for my $entry (@{$index})
	{
		last if $entry->{start} > $line_number;
		$found = $entry;
	}

	return $out_path if !defined $found;
	return "$out_path K $found->{key}" if defined $found->{key};

	my $source = substr($found->{S} // '', 0, 60);
	utf8::encode($source);

	return "$out_path (no record; catalog msgid \"$source\")";
}

# Run the shared gate on a generated catalog, recasting each reported line
# number as the output-map record that produced it.
sub lint_check
{
	my ($lint_path, $out_path, $index) = @_;
	my $pid = open my $output, '-|';

	die "$lint_path: unable to fork scripts/po/po-check.sh: $!\n"
		if !defined $pid;

	if ($pid == 0)
	{
		open STDERR, '>&', \*STDOUT
			or die "$lint_path: unable to merge check output: $!\n";
		exec 'scripts/po/po-check.sh', $lint_path
			or die "$lint_path: unable to run scripts/po/po-check.sh: $!\n";
	}

	my @lines = <$output>;
	close $output;
	my $status = $?;

	for my $line (@lines)
	{
		$line =~ s{\Q$lint_path\E:([0-9]+):}
			{lint_line_origin($index, $out_path, $1) . ':'}ge;
		$line =~ s{at line ([0-9]+)}
			{'at ' . lint_line_origin($index, $out_path, $1)}ge;
		$line =~ s/\Q$lint_path\E/$out_path/g;
		print $line;
	}

	return $status;
}

# Lint one output map through a temporary translated catalog.
sub cmd_lint
{
	my ($lang) = @_;
	validate_language($lang);
	my $translations = joined_translations($lang);
	my $catalog = translated_catalog($lang, $translations);
	my $lint_path = "$AI_DIR/$lang.lint.po";
	my $out_path = "$AI_DIR/$lang.out.map";

	Locale::PO->save_file_fromarray($lint_path, $catalog, 'utf8');
	my $status = lint_check($lint_path, $out_path,
		lint_key_index($lint_path, $translations));
	unlink $lint_path or die "$lint_path: unable to remove lint catalog: $!\n";
	assert_command_status("$out_path: scripts/po/po-check.sh", $status);
	print "$lang: output map passes translation checks\n";
}

# Run the public linter as the sole output-map validity test.
sub run_output_lint
{
	my ($lang) = @_;
	my $status = system($^X, "$ROOT/scripts/po/po-translate.pl", 'lint', $lang);

	die "$lang: unable to launch output-map lint: $!\n" if $status == -1;
	die "$lang: output-map lint terminated by signal " . ($status & 127) . "\n"
		if ($status & 127) != 0;

	return ($status >> 8) == 0;
}

# Define the complete model contract for producing one output map.
sub apply_instructions
{
	my ($lang) = @_;

	return <<"INSTRUCTIONS";
Create $AI_DIR/$lang.out.map from $AI_DIR/$lang.map.
Write plain K, L, C, S, and T tagged text only.
Do not use XML, <invoke>, <content>, or tool-call markup.
Apply doc/TRANSLATING.md and po/rules/$lang.md to each translation.
Reproduce every K, L, C, and S line verbatim and in order.
Fill only each T value.
Preserve every PO escape and printf placeholder from S exactly.
Leave no T value empty.
Edit only $AI_DIR/$lang.out.map.
Use only the injected context and lint command result.
Correct every error reported by the injected lint command result.
Use Read, Write, or Edit only on $AI_DIR/$lang.out.map when correcting lint errors.
Access no other workspace path through file or inspection tools.
Do not read or modify scripts/po/po-translate.pl.
Do not use Bash for any command except scripts/po/po-translate.pl lint $lang.
Run scripts/po/po-translate.pl lint $lang until it exits 0.
INSTRUCTIONS
}

# Launch one model session per invalid language and apply all validated maps.
sub cmd_apply
{
	my @pending_languages;

	for my $lang (map_languages())
	{
		validate_language($lang);
		if (run_output_lint($lang))
		{
			print "$lang: valid output map; skipping model session\n";
		}
		else
		{
			push @pending_languages, $lang;
		}
	}

	for my $index (0 .. $#pending_languages)
	{
		my $lang = $pending_languages[$index];
		my $lint_command = "!scripts/po/po-translate.pl lint $lang";
		my $status;
		{
			local $ENV{CLAUDE_HOOKS_KILL_ON_STOP} = 'done';
			$status = system(
				'claude-inject',
				'--claude', 'claude-raw',
				'--cwd', $ROOT,
				'--permission-mode', 'acceptEdits',
				'--model', $MODEL,
				'doc/TRANSLATING.md',
				"po/rules/$lang.md",
				"$AI_DIR/$lang.map",
				$lint_command,
				'--', apply_instructions($lang),
			);
		}
		assert_claude_inject_status($lang, $status);
		cmd_lint($lang);

		if ($index < $#pending_languages)
		{
			print "$lang: next language starts in 3 seconds; press Ctrl-C to stop\n";
			sleep 3;
		}
		else
		{
			print "$lang: all required model sessions complete\n";
		}
	}

	cmd_inject();
}

# Apply every authoritative workspace mapping and validate each result.
sub cmd_inject
{
	for my $lang (map_languages())
	{
		validate_language($lang);
		my $translations = joined_translations($lang);
		inject_language($lang, $translations);
		trans_check("po/$lang.po");
		print "$lang: applied " . scalar(keys %{$translations}) . " records\n";
	}
}
