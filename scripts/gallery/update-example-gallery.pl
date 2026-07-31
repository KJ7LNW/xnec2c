#!/usr/bin/env perl

use strict;
use warnings;

use Cwd qw(abs_path);
use File::Basename qw(basename dirname);
use File::Path qw(make_path);
use File::Spec;
use File::Temp qw(tempfile);
use Getopt::Long qw(Configure GetOptions);
use IPC::Cmd qw(can_run);
use JSON::PP qw(decode_json);
use POSIX ();

use FindBin qw($RealBin);
use lib $RealBin;
use GalleryBundle qw(
	load_file
	build_deck_bundle
	build_manifest_bundle
	build_browser_bundle
	load_browser_assignment
	validate_membership
);

my @SUPPORTED_FORMATS = qw(x y z iso quad);
my %SUPPORTED_FORMATS = map { $_ => 1 } @SUPPORTED_FORMATS;

use constant RECOMPRESS_QUALITY => '90-100';
use constant RECOMPRESS_NICE    => 10;

exit main() unless caller;
1;

# Print command-line usage.
sub usage
{
	print <<'USAGE';
Usage: update-example-gallery.pl [options] <command> <deck.nec>...

Commands:
  render    render every supplied deck and rewrite gallery data
  refresh   render only missing image sets and rewrite gallery data
  check     validate committed gallery data without writing or rendering

Options:
  --binary <path>                xnec2c executable (default: src/xnec2c; render only)
  --config <path>                render configuration file (render only)
                                 (default: scripts/gallery/gallery-render.conf)
  --output-dir <dir>             gallery output directory (default: doc/images/gallery)
  --rdpat-png-format <formats>   comma-separated x, y, z, iso, or quad values
                                 (default: x,y,z,iso)
  --force                        re-render image sets that already exist
  --help                         print this usage and exit
USAGE
}

# Parse a comma-separated radiation-pattern format list.
sub parse_formats
{
	my ($format_text) = @_;
	my @formats       = split /,/, $format_text, -1;

	for my $format (@formats) {
		if (!exists $SUPPORTED_FORMATS{$format}) {
			die "error: invalid radiation-pattern PNG format: $format\n";
		}
	}

	return @formats;
}

# Derive one relative gallery image path from its output cardinality.
sub image_path
{
	my ($name, $format, $format_count) = @_;
	my $filename = $format_count > 1 ? "$name-$format.png" : "$name.png";

	return "$name/$filename";
}

# Derive one relative gallery image record per requested format.
sub image_records
{
	my ($name, $formats_ref) = @_;
	my @images;

	for my $format (@{$formats_ref}) {
		push @images,
			{
			format => $format,
			path   => image_path($name, $format, scalar @{$formats_ref}),
			};
	}

	return \@images;
}

# Replace one textual gallery artifact atomically.
sub write_file
{
	my ($path, $temporary_template, $text) = @_;

	make_path(dirname($path));
	my ($file, $temporary_path) = tempfile(
		$temporary_template,
		DIR    => dirname($path),
		UNLINK => 1,
	);
	binmode $file, ':raw'
		or die "error: $path: unable to set raw output mode: $!\n";
	print {$file} $text
		or die "error: $path: unable to write temporary artifact: $!\n";
	close $file
		or die "error: $path: unable to close temporary artifact: $!\n";
	rename $temporary_path, $path
		or die "error: $path: unable to replace artifact: $!\n";
}

# Publish the manifest and both browser projections from prepared data.
sub write_gallery_artifacts
{
	my ($context, $manifest) = @_;
	my @decks       = map { $_->{deck} } @{ $manifest->{examples} };
	my $deck_bundle = build_deck_bundle($context->{repo_root}, \@decks);

	write_file($context->{manifest_path}, '.manifest-XXXXXX', build_manifest_bundle($manifest));
	write_file($context->{manifest_js_path},
		'.manifest-js-XXXXXX', build_browser_bundle('XNEC2C_GALLERY', $manifest));
	write_file($context->{decks_js_path},
		'.decks-js-XXXXXX', build_browser_bundle('XNEC2C_DECKS', $deck_bundle));
}

# Validate and normalize the complete authoritative deck inventory.
sub validate_deck_arguments
{
	my ($repo_root, $decks_ref) = @_;
	my %seen;

	if (!@{$decks_ref}) {
		die "error: no NEC deck paths supplied\n";
	}

	for my $deck (@{$decks_ref}) {
		if ($deck !~ /\Aexamples\/.+\.nec\z/) {
			die "error: $deck: expected an examples/*.nec path\n";
		}
		elsif ($seen{$deck}++) {
			die "error: $deck: duplicate authoritative inventory member\n";
		}
		elsif (!-f File::Spec->catfile($repo_root, split m{/}, $deck)) {
			die "error: $deck: NEC deck not found or not a regular file\n";
		}
	}
}

# Validate manifest records and return their unique deck paths.
sub validate_manifest_records
{
	my ($artifact, $manifest, $defects_ref) = @_;
	my @decks;
	my %seen_decks;

	if (ref($manifest) ne 'HASH' || ref($manifest->{examples}) ne 'ARRAY') {
		push @{$defects_ref},
			{
			artifact => $artifact,
			deck     => '-',
			defect   => 'invalid manifest root',
			};
		return \@decks;
	}

	for my $entry (@{ $manifest->{examples} }) {
		my $deck =
			   ref($entry) eq 'HASH'
			&& defined($entry->{deck})
			&& !ref($entry->{deck}) ? $entry->{deck} : '-';

		if (       ref($entry) ne 'HASH'
			|| !defined($entry->{deck})
			|| ref($entry->{deck})
			|| !defined($entry->{name})
			|| ref($entry->{name})
			|| ref($entry->{images}) ne 'ARRAY')
		{
			push @{$defects_ref},
				{
				artifact => $artifact,
				deck     => $deck,
				defect   => 'invalid manifest entry',
				};
			next;
		}

		if ($seen_decks{$deck}++) {
			push @{$defects_ref},
				{
				artifact => $artifact,
				deck     => $deck,
				defect   => 'duplicate gallery member',
				};
		}
		else {
			push @decks, $deck;
		}

		my $expected_name = basename($deck);
		$expected_name =~ s/\.nec\z//;
		if ($entry->{name} ne $expected_name) {
			push @{$defects_ref},
				{
				artifact => $artifact,
				deck     => $deck,
				defect   => "name differs from $expected_name",
				};
		}

		my %seen_formats;
		for my $image (@{ $entry->{images} }) {
			if (       ref($image) ne 'HASH'
				|| !defined($image->{format})
				|| ref($image->{format})
				|| !defined($image->{path})
				|| ref($image->{path}))
			{
				push @{$defects_ref},
					{
					artifact => $artifact,
					deck     => $deck,
					defect   => 'invalid image record',
					};
				next;
			}

			my $format = $image->{format};
			if (!exists $SUPPORTED_FORMATS{$format}) {
				push @{$defects_ref},
					{
					artifact => $artifact,
					deck     => $deck,
					defect   => "invalid image format $format",
					};
				next;
			}
			elsif ($seen_formats{$format}++) {
				push @{$defects_ref},
					{
					artifact => $artifact,
					deck     => $deck,
					defect   => "duplicate image format $format",
					};
				next;
			}

			my $expected_path =
				image_path($expected_name, $format, scalar @{ $entry->{images} });
			if ($image->{path} ne $expected_path) {
				push @{$defects_ref},
					{
					artifact => $artifact,
					deck     => $deck,
					defect   => "image path differs from $expected_path",
					};
			}
		}
	}

	return \@decks;
}

# Validate every committed gallery projection without writing or rendering.
sub validate_gallery
{
	my ($context, $decks_ref) = @_;
	my @defects;
	my $manifest_text = load_file($context->{manifest_path});
	my $manifest      = eval { decode_json($manifest_text) };

	if ($@ ne '') {
		chomp $@;
		push @defects,
			{
			artifact => $context->{manifest_label},
			deck     => '-',
			defect   => "invalid JSON: $@",
			};
	}
	else {
		my $manifest_decks =
			validate_manifest_records($context->{manifest_label}, $manifest, \@defects);
		validate_membership(\@defects, $context->{manifest_label},
			$decks_ref, $manifest_decks);

		if ($manifest_text ne build_manifest_bundle($manifest)) {
			push @defects,
				{
				artifact => $context->{manifest_label},
				deck     => '-',
				defect   => 'serialization is not canonical',
				};
		}

		my ($browser_manifest, $browser_manifest_text) =
			load_browser_assignment($context->{manifest_js_path}, 'XNEC2C_GALLERY');
		my $browser_manifest_decks =
			validate_manifest_records($context->{manifest_js_label},
			$browser_manifest, \@defects);
		validate_membership(
			\@defects,  $context->{manifest_js_label},
			$decks_ref, $browser_manifest_decks
		);

		if (build_manifest_bundle($browser_manifest) ne build_manifest_bundle($manifest)) {
			push @defects,
				{
				artifact => $context->{manifest_js_label},
				deck     => '-',
				defect   => 'data differs from JSON manifest',
				};
		}
		elsif ($browser_manifest_text ne build_browser_bundle('XNEC2C_GALLERY', $manifest))
		{
			push @defects,
				{
				artifact => $context->{manifest_js_label},
				deck     => '-',
				defect   => 'serialization is not canonical',
				};
		}

		my ($deck_bundle, $deck_bundle_text) =
			load_browser_assignment($context->{decks_js_path}, 'XNEC2C_DECKS');
		if (ref($deck_bundle) ne 'HASH') {
			push @defects,
				{
				artifact => $context->{decks_js_label},
				deck     => '-',
				defect   => 'deck bundle is not a map',
				};
		}
		else {
			my @bundle_decks = sort keys %{$deck_bundle};
			validate_membership(\@defects, $context->{decks_js_label},
				$decks_ref, \@bundle_decks);
			my $expected_decks = build_deck_bundle($context->{repo_root}, $decks_ref);

			for my $deck (@{$decks_ref}) {
				if (exists($deck_bundle->{$deck})
					&& $deck_bundle->{$deck} ne $expected_decks->{$deck})
				{
					push @defects,
						{
						artifact => $context->{decks_js_label},
						deck     => $deck,
						defect   => 'content differs from source deck',
						};
				}
			}

			if ($deck_bundle_text ne build_browser_bundle('XNEC2C_DECKS', $deck_bundle))
			{
				push @defects,
					{
					artifact => $context->{decks_js_label},
					deck     => '-',
					defect   => 'serialization is not canonical',
					};
			}
		}
	}

	for my $defect (@defects) {
		print STDERR "error: $defect->{artifact}: $defect->{deck}: "
			. "$defect->{defect}\n";
	}

	return @defects == 0 ? 0 : 1;
}

# Report the online CPU count for the recompressor semaphore, defaulting to one.
sub cpu_count
{
	my $count = `getconf _NPROCESSORS_ONLN 2>/dev/null`;
	if (defined($count) && $count =~ /\A\s*([1-9][0-9]*)\s*\z/) {
		return $1;
	}
	return 1;
}

# Recompress one PNG in place with pngquant, keeping the smaller output and the
# original's permissions; a not-smaller or below-quality result is discarded.
sub recompress_png
{
	my ($pngquant, $png_path) = @_;
	my @stat = stat $png_path;
	if (!@stat) {
		return;
	}

	my $before = $stat[7];
	my $mode   = $stat[2] & 07777;
	my ($tmp_fh, $tmp_path) = tempfile('.recompress-XXXXXX',
		DIR => dirname($png_path), SUFFIX => '.png', UNLINK => 1);
	close $tmp_fh;

	my $status = system($pngquant,
		'--quality', RECOMPRESS_QUALITY, '--speed', '1', '--strip',
		'--skip-if-larger', '--force', '--output', $tmp_path, '--', $png_path);
	my $exit  = (($status == -1) || (($status & 127) != 0)) ? -1 : ($status >> 8);
	my $after = ($exit == 0) ? -s $tmp_path : undef;

	if (defined($after) && ($after > 0) && ($after < $before)) {
		chmod $mode, $tmp_path;
		rename $tmp_path, $png_path;
	}
	elsif (-e $tmp_path) {
		unlink $tmp_path;
	}

	return;
}

# Wait until fewer than the CPU-count limit of background recompressors remain.
sub throttle_recompressors
{
	my ($pids_ref, $limit) = @_;
	while (scalar(keys %{$pids_ref}) >= $limit) {
		my $pid = waitpid(-1, 0);
		if ($pid <= 0) {
			last;
		}
		delete $pids_ref->{$pid};
	}
}

# Reap every outstanding background recompressor before returning.
sub reap_recompressors
{
	my ($pids_ref) = @_;
	while (scalar(keys %{$pids_ref}) > 0) {
		my $pid = waitpid(-1, 0);
		if ($pid <= 0) {
			last;
		}
		delete $pids_ref->{$pid};
	}
}

# Fork a niced background recompressor for one deck's freshly rendered images,
# bounded by the CPU-count semaphore so concurrent recompressors never exceed it.
sub spawn_recompressor
{
	my ($pids_ref, $limit, $pngquant, $outputs_ref) = @_;
	if (!defined $pngquant) {
		return;
	}

	throttle_recompressors($pids_ref, $limit);
	my $pid = fork();
	if (!defined $pid) {
		warn "warning: unable to fork recompressor: $!\n";
	}
	elsif ($pid == 0) {
		POSIX::nice(RECOMPRESS_NICE);
		for my $png (@{$outputs_ref}) {
			recompress_png($pngquant, $png);
		}
		POSIX::_exit(0);
	}
	else {
		$pids_ref->{$pid} = 1;
	}
}

# Publish the supplied decks' gallery data atomically. A deck renders only when
# an image output is missing or force is set, so existing sets are reused and a
# complete tree rewrites the data files without invoking the xnec2c binary.
# Each freshly rendered deck is recompressed by a niced background worker while
# the next deck renders, capped by a CPU-count semaphore.
sub render_gallery
{
	my ($context, $decks_ref) = @_;
	my @formats  = parse_formats($context->{format_text});
	my $total    = scalar @{$decks_ref};
	my $rendered = 0;
	my $fail     = 0;
	my $index    = 0;
	my @entries;
	my @plan;
	my %recompress_pids;
	my $pngquant  = can_run('pngquant');
	my $cpu_limit = cpu_count();

	for my $deck (@{$decks_ref}) {
		my $name = basename($deck);
		$name =~ s/\.nec\z//;
		my $images  = image_records($name, \@formats);
		my @outputs =
			map { File::Spec->catfile($context->{output_dir}, split m{/}, $_->{path}); }
			@{$images};
		push @plan,
			{
			deck    => $deck,
			images  => $images,
			name    => $name,
			outputs => \@outputs,
			needs   => ($context->{force} || grep { !-s $_ } @outputs) ? 1 : 0,
			};
	}

	make_path($context->{output_dir});
	my $to_render = grep { $_->{needs} } @plan;
	if (($to_render != 0) && (!-x $context->{binary})) {
		die "error: xnec2c binary not found or not executable: $context->{binary}\n";
	}
	if (($to_render != 0) && (!-r $context->{config})) {
		die "error: render configuration not readable: $context->{config}\n";
	}
	if (($to_render != 0) && !defined($pngquant)) {
		warn "warning: pngquant not found; rendering without background recompression\n";
	}

	print "Publishing $total gallery image sets into $context->{output_dir}\n\n";

	for my $item (@plan) {
		$index++;
		printf '%3d/%-3d  %-40s ', $index, $total, $item->{name};

		if (!$item->{needs}) {
			print "cached\n";
			push @entries,
				{ deck => $item->{deck}, images => $item->{images}, name => $item->{name} };
			next;
		}

		my $deck_dir = File::Spec->catdir($context->{output_dir}, $item->{name});
		make_path($deck_dir);
		my $output_base = File::Spec->catfile($deck_dir, "$item->{name}.png");
		my @outputs = @{ $item->{outputs} };

		my $status = system($context->{binary},
			'--batch',
			'-c',
			$context->{config},
			'--rdpat-png-format',
			$context->{format_text},
			'--freq-select',
			'center',
			'--write-rdpat-png',
			$output_base,
			'--',
			File::Spec->catfile($context->{repo_root}, split m{/}, $item->{deck}));
		my $exit = (($status == -1) || (($status & 127) != 0)) ? -1 : ($status >> 8);

		# Success is a complete, non-empty output set; a nonzero xnec2c exit on a
		# fully rendered deck is a diagnostic, not a failure.
		if (!grep { !-s $_ } @outputs) {
			if ($exit != 0) {
				print "ok (xnec2c exit $exit)\n";
			}
			else {
				print "ok\n";
			}
			$rendered++;
			push @entries,
				{ deck => $item->{deck}, images => $item->{images}, name => $item->{name} };
			spawn_recompressor(\%recompress_pids, $cpu_limit, $pngquant, \@outputs);
		}
		else {
			print "FAILED\n";
			$fail++;
		}
	}

	reap_recompressors(\%recompress_pids);

	if ($fail != 0) {
		print "\nGallery failed: $fail of $total image sets failed\n";
		return 1;
	}

	write_gallery_artifacts($context,
		{ examples => [ sort { $a->{deck} cmp $b->{deck} } @entries ] });
	print "\nGallery complete: $rendered rendered, " . ($total - $rendered) . " cached\n";

	return 0;
}

# Parse one command and dispatch it to its operation handler.
sub main
{
	my $script_dir = dirname(abs_path($0));
	my $repo_root  = dirname(dirname($script_dir));
	my $command    = shift @ARGV;
	my $help       = 0;
	my $context    = {
		binary         => File::Spec->catfile($repo_root, 'src',     'xnec2c'),
		config         => File::Spec->catfile($repo_root, 'scripts', 'gallery', 'gallery-render.conf'),
		decks_js_label => File::Spec->catfile('doc', 'images', 'gallery', 'decks.js'),
		force          => 0,
		format_text    => 'x,y,z,iso',
		manifest_js_label => File::Spec->catfile('doc', 'images', 'gallery', 'manifest.js'),
		manifest_label => File::Spec->catfile('doc', 'images', 'gallery', 'manifest.json'),
		output_dir     => File::Spec->catdir($repo_root, 'doc', 'images', 'gallery'),
		repo_root      => $repo_root,
	};

	Configure(qw(no_auto_abbrev no_ignore_case));
	GetOptions(
		'binary=s'           => \$context->{binary},
		'config=s'           => \$context->{config},
		'output-dir=s'       => \$context->{output_dir},
		'rdpat-png-format=s' => \$context->{format_text},
		'force'              => \$context->{force},
		'help'               => \$help,
		)
		or do {
		usage();
		die "error: invalid command-line options\n";
		};

	if ($help) {
		usage();
		return 0;
	}
	elsif (!defined($command)
		|| $command !~ /\A(?:render|refresh|check)\z/)
	{
		usage();
		die "error: expected render, refresh, or check command\n";
	}

	$context->{manifest_path}    = File::Spec->catfile($context->{output_dir}, 'manifest.json');
	$context->{decks_js_path}    = File::Spec->catfile($context->{output_dir}, 'decks.js');
	$context->{manifest_js_path} = File::Spec->catfile($context->{output_dir}, 'manifest.js');
	my @decks = @ARGV;
	validate_deck_arguments($repo_root, \@decks);
	$context->{force} = ($context->{force} || $command eq 'render') ? 1 : 0;
	my %handlers = (
		check   => \&validate_gallery,
		refresh => \&render_gallery,
		render  => \&render_gallery,
	);

	return $handlers{$command}->($context, \@decks);
}
