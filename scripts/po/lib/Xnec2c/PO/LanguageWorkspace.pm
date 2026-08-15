package Xnec2c::PO::LanguageWorkspace;

use strict;
use warnings;
use utf8;

use Exporter qw(import);

use Xnec2c::PO::CatalogState qw(catalog_exemption_records);
use Xnec2c::PO::TranslationMap qw(map_path output_path prompt_path);

our @EXPORT_OK = qw(
	catalog_languages clear_language_manifest clear_language_workspace
	preceding_exemptions validate_language
);

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

# Validate a language before deriving repository paths from it.
sub validate_language
{
	my ($lang) = @_;

	die "invalid language: $lang\n" if $lang !~ /\A[A-Za-z_]+\z/;
}

# Remove one generated workspace artifact, accepting one the workspace never
# held.
sub remove_artifact
{
	my ($path, $subject) = @_;

	return if !-e $path;
	unlink $path or die "$path: unable to remove $subject: $!\n";
}

# Remove one language's generated manifest and prompt while retaining any
# model output that has not crossed a successful catalog gate.
sub clear_language_manifest
{
	my ($lang) = @_;

	remove_artifact($_, 'stale artifact')
		for map_path($lang), prompt_path($lang);
}

# Remove all workspace artifacts only after the catalog reaches the complete
# release gate.
sub clear_language_workspace
{
	my ($lang) = @_;

	clear_language_manifest($lang);
	remove_artifact(output_path($lang), 'consumed output map');
}

# Read the exemptions the catalogs ahead of one language in catalog order
# already accept. Acceptance travels forward through that order alone, so the
# same predecessors answer a language on every run and the catalogs holding
# them stay the only record.
sub preceding_exemptions
{
	my ($lang) = @_;
	my @languages = catalog_languages();
	my $position = 0;

	$position++ while $position < @languages && $languages[$position] ne $lang;

	return catalog_exemption_records([@languages[0 .. $position - 1]]);
}

1;
