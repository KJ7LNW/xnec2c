package GalleryBundle;

# Shared, side-effect-free gallery bundle helpers: read deck sources, build the
# canonical manifest and browser projections, decode a browser global
# assignment, and compare artifact membership. Consumed by the gallery
# generator and the website-drift checker.

use strict;
use warnings;

use Exporter qw(import);
use File::Spec;
use JSON::PP qw(decode_json);

our @EXPORT_OK = qw(
	load_file
	build_deck_bundle
	build_manifest_bundle
	build_browser_bundle
	parse_browser_assignment
	load_browser_assignment
	validate_membership
);

# Read a complete file without changing its bytes.
sub load_file
{
	my ($path) = @_;

	open my $file, '<:raw', $path
		or die "error: $path: unable to read: $!\n";
	local $/;
	my $text = <$file>;
	close $file
		or die "error: $path: unable to close: $!\n";

	return $text;
}

# Build the browser deck map from the authoritative source files.
sub build_deck_bundle
{
	my ($repo_root, $decks_ref) = @_;
	my %decks;

	for my $deck (@{$decks_ref}) {
		$decks{$deck} =
			load_file(File::Spec->catfile($repo_root, split m{/}, $deck));
	}

	return \%decks;
}

# Build the canonical JSON representation of a gallery manifest.
sub build_manifest_bundle
{
	my ($manifest) = @_;

	return JSON::PP->new->canonical->pretty->encode($manifest);
}

# Build the canonical browser assignment for a JSON value.
sub build_browser_bundle
{
	my ($global_name, $value) = @_;

	return "window.$global_name = " . JSON::PP->new->canonical->pretty->encode($value) . ";\n";
}

# Decode one canonical window.<global> = <json>; assignment from in-memory text.
sub parse_browser_assignment
{
	my ($text, $global_name, $label) = @_;

	if ($text !~ /\Awindow\.\Q$global_name\E\s*=\s*(.*?)\s*;\s*\z/s) {
		die "error: $label: invalid window.$global_name assignment\n";
	}

	my $value = eval { decode_json($1) };
	if ($@ ne '') {
		chomp $@;
		die "error: $label: invalid JSON: $@\n";
	}

	return $value;
}

# Decode one canonical browser global assignment from a file.
sub load_browser_assignment
{
	my ($path, $global_name) = @_;
	my $text = load_file($path);

	return (parse_browser_assignment($text, $global_name, $path), $text);
}

# Record bidirectional set differences for one gallery artifact.
sub validate_membership
{
	my ($defects_ref, $artifact, $expected_ref, $actual_ref) = @_;
	my %expected = map { $_ => 1 } @{$expected_ref};
	my %actual   = map { $_ => 1 } @{$actual_ref};

	for my $deck (@{$expected_ref}) {
		if (!exists $actual{$deck}) {
			push @{$defects_ref},
				{
				artifact => $artifact,
				deck     => $deck,
				defect   => 'missing gallery member',
				};
		}
	}

	for my $deck (@{$actual_ref}) {
		if (!exists $expected{$deck}) {
			push @{$defects_ref},
				{
				artifact => $artifact,
				deck     => $deck,
				defect   => 'extra gallery member',
				};
		}
	}
}

1;
