#!/usr/bin/env perl

# Fetch the published gallery browser bundles and report drift against the
# local examples tree, so a maintainer knows whether the website needs a
# resync before a release. Compares deck membership and deck source and exits
# nonzero when the live site diverges. HTTPS fetching requires IO::Socket::SSL.

use strict;
use warnings;

use Cwd qw(abs_path);
use File::Basename qw(dirname);
use Getopt::Long qw(Configure GetOptions);
use HTTP::Tiny;

use FindBin qw($RealBin);
use lib $RealBin;
use GalleryBundle qw(
	build_deck_bundle
	parse_browser_assignment
	validate_membership
);

use constant DEFAULT_BASE_URL => 'https://www.xnec2c.org/';

exit main() unless caller;
1;

# Print command-line usage.
sub usage
{
	print <<'USAGE';
Usage: check-gallery-website.pl [--base-url <url>] <deck.nec>...

Fetch the published gallery bundles and report drift against the local
examples tree. Exits nonzero when the website needs to be resynchronized.

Options:
  --base-url <url>   published site root (default: https://www.xnec2c.org/)
  --help             print this usage and exit
USAGE
}

# Fetch one published artifact as text, dying on any transport failure.
sub fetch_text
{
	my ($http, $url) = @_;
	my $response = $http->get($url);

	if (!$response->{success}) {
		die "error: $url: fetch failed: $response->{status} $response->{reason}\n";
	}

	return $response->{content};
}

# Join the base site root and a gallery-relative artifact name.
sub artifact_url
{
	my ($base_url, $name) = @_;
	(my $root = $base_url) =~ s{/+\z}{};

	return "$root/images/gallery/$name";
}

# Compare the published deck-source bundle against the local examples tree.
sub check_deck_bundle
{
	my ($defects_ref, $url, $remote, $local, $decks_ref) = @_;

	if (ref($remote) ne 'HASH') {
		push @{$defects_ref},
			{ artifact => $url, deck => '-', defect => 'deck bundle is not a map' };
		return;
	}

	validate_membership($defects_ref, $url, $decks_ref, [sort keys %{$remote}]);

	for my $deck (@{$decks_ref}) {
		if (exists $remote->{$deck} && $remote->{$deck} ne $local->{$deck}) {
			push @{$defects_ref},
				{
				artifact => $url,
				deck     => $deck,
				defect   => 'source differs from local examples tree',
				};
		}
	}
}

# Compare the published manifest membership against the local examples tree.
sub check_manifest_bundle
{
	my ($defects_ref, $url, $remote, $decks_ref) = @_;

	if (ref($remote) ne 'HASH' || ref($remote->{examples}) ne 'ARRAY') {
		push @{$defects_ref},
			{ artifact => $url, deck => '-', defect => 'invalid manifest root' };
		return;
	}

	my @manifest_decks = map { $_->{deck} } @{ $remote->{examples} };
	validate_membership($defects_ref, $url, $decks_ref, \@manifest_decks);
}

sub main
{
	my $repo_root = dirname(dirname(abs_path($0)));
	my $base_url  = DEFAULT_BASE_URL;
	my $help      = 0;

	Configure(qw(no_auto_abbrev no_ignore_case));
	if (!GetOptions('base-url=s' => \$base_url, 'help' => \$help)) {
		usage();
		return 1;
	}

	if ($help) {
		usage();
		return 0;
	}

	my @decks = @ARGV;
	if (!@decks) {
		usage();
		die "error: no NEC deck paths supplied\n";
	}

	my $http          = HTTP::Tiny->new;
	my $decks_url     = artifact_url($base_url, 'decks.js');
	my $manifest_url  = artifact_url($base_url, 'manifest.js');
	my $remote_decks  =
		parse_browser_assignment(fetch_text($http, $decks_url), 'XNEC2C_DECKS', $decks_url);
	my $remote_manifest =
		parse_browser_assignment(fetch_text($http, $manifest_url), 'XNEC2C_GALLERY', $manifest_url);
	my $local_decks = build_deck_bundle($repo_root, \@decks);

	my @defects;
	check_deck_bundle(\@defects, $decks_url, $remote_decks, $local_decks, \@decks);
	check_manifest_bundle(\@defects, $manifest_url, $remote_manifest, \@decks);

	for my $defect (@defects) {
		print STDERR "error: $defect->{artifact}: $defect->{deck}: "
			. "$defect->{defect}\n";
	}

	if (@defects) {
		print STDERR "website gallery is out of sync; run 'make gallery' and publish\n";
		return 1;
	}

	print "website gallery is synchronized with the local examples tree\n";
	return 0;
}
