package Xnec2c::PO::ManifestWriter;

use strict;
use warnings;
use utf8;

use Exporter qw(import);

use Xnec2c::PO::MapFile qw(
	read_utf8_file serialize_records write_utf8_file
);
use Xnec2c::PO::TranslationMap qw(map_path prompt_path);
use Xnec2c::PO::TranslationRules qw(
	exemption_rules record_rules repair_rules
);

our @EXPORT_OK = qw(write_language_manifest);

# Generate the authoritative map and model-facing prompt for one language.
sub write_language_manifest
{
	my ($lang, $records, $copied) = @_;
	my $serialized = serialize_records($records);
	my $directions = "# Output format\n\n" . record_rules()
		. "\n- Translate each T under the preceding rules.\n"
		. repair_rules() . "\n\n" . exemption_rules();
	my $prompt = read_utf8_file('doc/TRANSLATING.md') . "\n\n"
		. read_utf8_file("po/rules/$lang.md") . "\n\n$directions\n$serialized";

	write_utf8_file(map_path($lang), $serialized);
	write_utf8_file(prompt_path($lang), $prompt);
	print "$lang: generated " . scalar(@{$records})
		. " records, $copied repeating their source\n";
}

1;
