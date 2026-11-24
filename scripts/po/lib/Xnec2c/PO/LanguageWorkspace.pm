package Xnec2c::PO::LanguageWorkspace;

use strict;
use warnings;
use utf8;

use Exporter qw(import);

use Xnec2c::PO::CatalogState qw(catalog_exemption_records catalog_path);
use Xnec2c::PO::TranslationMap qw(map_path output_path prompt_path);

our @EXPORT_OK = qw(
	catalog_languages clear_language_manifest clear_language_workspace
	preceding_exemptions validate_language
);

my $LINGUAS_PATH = 'po/LINGUAS';

# Read the language codes po/LINGUAS registers, in file order, through the
# LINGUAS reader the catalog gates share.
sub registered_languages
{
	my $reader_path = 'scripts/po/po-linguas.sh';

	open my $reader, '-|', $reader_path, $LINGUAS_PATH
		or die "$LINGUAS_PATH: unable to run $reader_path: $!\n";
	chomp(my @languages = <$reader>);
	close $reader;
	die "$LINGUAS_PATH: $reader_path failed (wait status $?)\n" if $? != 0;

	return @languages;
}

# Enumerate the languages po/LINGUAS registers in sorted order, holding each to
# a valid code whose catalog exists, since consumers derive catalog paths from
# every returned language.
sub catalog_languages
{
	my @languages = registered_languages();

	die "$LINGUAS_PATH: registers no language\n" if scalar(@languages) == 0;

	for my $lang (@languages)
	{
		validate_language($lang);
		my $catalog_path = catalog_path($lang);
		die "$catalog_path: catalog absent (registered in $LINGUAS_PATH)\n"
			if !-f $catalog_path;
	}

	return sort @languages;
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
