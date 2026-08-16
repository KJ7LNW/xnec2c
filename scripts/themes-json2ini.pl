#!/usr/bin/perl

# Convert the upstream daisyUI color export (themes.json) into the committed
# GKeyFile theme palette (resources/themes.ini) consumed by the freqplots
# color-theme system.
#
# themes.json is the canonical upstream source and is never committed; this
# script is the only bridge to the committed format. Each daisyUI theme yields
# two registry groups: [<name>] from its "normal" variant and
# [<name>-inverted] from its "inverted" variant. background reads
# colors.base-100 (a surface, never contrast-adjusted); every other role reads
# a --tw-prose-* typography key already WCAG-solved against that same variant's
# base-100, so foreground and background are always paired within one variant.
# The two data-series roles are the exception: they are not read from a fixed
# key but chosen per variant so the left and right traces never collapse to one
# color (see resolve_secondary).
#
# The hand-authored legacy and smith-derived palettes are not present in the
# daisyUI export; they are appended verbatim from the literal block below so a
# single emitter produces one output file. A role whose source value is absent
# is omitted, letting the C loader inherit that role's legacy default (single
# source of defaults).
#
# Usage: perl scripts/themes-json2ini.pl [--input themes.json]
#                                        [--output resources/themes.ini]

use strict;
use warnings;

use JSON::PP qw(decode_json);
use Getopt::Long qw(GetOptions);
use File::Basename qw(basename);

# Role emission order mirrors the C theme_t field order (src/theme.h). Each row
# binds an INI role key to its daisyUI source: 'color' for a colors.* surface
# key, 'typo' for a --tw-prose-* foreground key, or 'family' for the primary or
# secondary data-series color resolved per variant (see resolve_secondary).
# background is the only surface; all others are foregrounds over it.
my @roles = (
	{ role => 'background',        src => 'color',  key => 'base-100' },
	{ role => 'grid',              src => 'typo',   key => '--tw-prose-hr' },
	{ role => 'grid_primary',      src => 'family', key => 'primary' },
	{ role => 'grid_secondary',    src => 'family', key => 'secondary' },
	{ role => 'grid_emphasis',     src => 'typo',   key => '--tw-prose-kbd' },
	{ role => 'grid_perimeter',    src => 'typo',   key => '--tw-prose-td-borders' },
	{ role => 'grid_scale',        src => 'typo',   key => '--tw-prose-counters' },
	{ role => 'series_primary',    src => 'family', key => 'primary' },
	{ role => 'series_secondary',  src => 'family', key => 'secondary' },
	{ role => 'axis',              src => 'typo',   key => '--tw-prose-th-borders' },
	{ role => 'label_primary',     src => 'family', key => 'primary' },
	{ role => 'label_secondary',   src => 'family', key => 'secondary' },
	{ role => 'label_axis',        src => 'typo',   key => '--tw-prose-captions' },
	{ role => 'marker_extreme',    src => 'typo',   key => '--tw-prose-body' },
	{ role => 'cursor',            src => 'typo',   key => '--tw-prose-bold' },
);

# How the upstream contrast tool produced these colors, and what it dictates for
# choosing the two data-series colors:
#   semantic token (primary, secondary, accent, neutral, base-content)
#     -> typography variable (TYPOGRAPHY_COLOR_MAPPINGS)
#       -> OKLCH lightness adjusted to 4.5:1 against base-100 (contrast-adjust)
# The adjustment preserves hue and chroma, so two series stay distinguishable
# exactly when their semantic tokens differ; where a theme's palette collapses
# toward one hue (monochrome phosphor), every typography output collapses with
# it and no choice can separate them. The primary series anchors to the primary
# family; the secondary series takes the first family below whose emitted color
# is distinct from the primary, preferring the natural secondary, then accent,
# base-content, and neutral. Each family emits its already-corrected typography
# value, never the raw semantic color (only typography carries the 4.5:1 solve).
my $primary_key = '--tw-prose-headings';
my @secondary_keys = (
	'--tw-prose-quotes',    # secondary semantic family (natural second series)
	'--tw-prose-bold',      # accent
	'--tw-prose-body',      # base-content
	'--tw-prose-counters',  # neutral
);

# Background-contrast violations found while converting the daisyUI variants.
# Accumulated across every group, reported together, and fatal: a palette with
# an unreadable foreground never overwrites the committed output. Series
# distinctness shortfalls are warned during resolution, not collected here,
# because a monochrome source cannot satisfy them yet must still convert.
my @violations;

# Hand-authored palettes absent from the daisyUI export. legacy reproduces the
# framebuffer-exact CGA/digital colors that keep both XY and Smith rendering
# pixel-identical to the pre-theme program. smith-derived re-skins both plot
# types in the Smith CGA octant: dim chromatic grid families with bright data
# traces sitting above them.
my $literal_groups = <<'INI';
[legacy]
display = Legacy
background = #000000
grid = #808080
grid_primary = #0000AA
grid_secondary = #AA0000
grid_emphasis = #AAAAAA
grid_perimeter = #555555
grid_scale = #AA5500
axis = #AAAAAA
series_primary = #FF00FF
series_secondary = #00FFFF
label_primary = #FF00FF
label_secondary = #00FFFF
label_axis = #FFFF00
marker_extreme = #FFFFFF
cursor = #00FF00

[smith-derived]
display = Smith Derived
background = #000000
grid = #555555
grid_primary = #0000AA
grid_secondary = #AA0000
grid_emphasis = #AAAAAA
grid_perimeter = #555555
grid_scale = #AA5500
axis = #AAAAAA
series_primary = #5555FF
series_secondary = #FF5555
label_primary = #5555FF
label_secondary = #FF5555
label_axis = #AA5500
marker_extreme = #FFFFFF
cursor = #008000
INI

my $input        = 'themes.json';
my $output       = 'resources/themes.ini';
my $min_contrast = 4.5;
my $min_distinct = 10.0;
my $help         = 0;

GetOptions(
	'input=s'        => \$input,
	'output=s'       => \$output,
	'min-contrast=f' => \$min_contrast,
	'min-distinct=f' => \$min_distinct,
	'help'           => \$help,
) or die usage();

if ($help)
{
	print usage();
	exit 0;
}

my $json = decode_json(slurp($input));

my @out;
push @out, "# Generated by scripts/themes-json2ini.pl from $input.\n";
push @out, "# Do not edit; regenerate from the upstream daisyUI export instead.\n";
push @out, "# daisyUI groups carry WCAG-solved typography foregrounds over each\n";
push @out, "# variant's own base-100 surface. legacy and smith-derived are\n";
push @out, "# hand-authored. Missing role keys inherit the legacy default in the\n";
push @out, "# C loader.\n";

my @variants = (
	{ suffix => '',          key => 'normal' },
	{ suffix => '-inverted', key => 'inverted' },
);

for my $name (sort keys %$json)
{
	for my $v (@variants)
	{
		my $group   = "$name$v->{suffix}";
		my $variant = $json->{$name}{ $v->{key} };

		my $colors = resolve_variant($group, $variant);
		next if !defined $colors;

		validate_variant($group, $colors);
		push @out, emit_group($group, $variant, $colors);
	}
}

push @out, "\n", $literal_groups;

# Surface every contrast failure at once, then refuse to write: a palette with
# an unreadable foreground must not replace the committed file.
if (@violations)
{
	warn "themes-json2ini: $_\n" for @violations;
	die sprintf("themes-json2ini: %d contrast violation(s); %s not "
		. "regenerated\n", scalar @violations, $output);
}

spew($output, join('', @out));

exit 0;

# Resolve one daisyUI variant to a role -> #RRGGBB map. The primary and
# secondary data-series families are resolved first (see resolve_secondary) so
# the two series stay distinct; 'family' roles read those, 'color' and 'typo'
# roles read their fixed source key. An absent or malformed source value is
# warned and omitted so the C loader inherits the legacy default for that role.
# Returns undef when the variant itself is missing.
sub resolve_variant
{
	my ($group, $variant) = @_;

	if (!defined $variant)
	{
		warn "themes-json2ini: group '$group' has no variant data\n";
		return undef;
	}

	my %family = (
		primary => lookup_hex($variant->{typography}, $primary_key),
	);
	$family{secondary} = resolve_secondary($group, $variant, $family{primary});

	my %colors;
	for my $r (@roles)
	{
		my $hex;
		if ($r->{src} eq 'family')
		{
			$hex = $family{ $r->{key} };
		}
		else
		{
			my $block = $r->{src} eq 'color'
				? $variant->{colors}
				: $variant->{typography};
			my $raw = $block->{ $r->{key} };
			next if !defined $raw;

			$hex = normalize_hex($raw);
			if (!defined $hex)
			{
				warn "themes-json2ini: group '$group' role '$r->{role}' has "
					. "non-#RRGGBB value '$raw'; omitting (inherits legacy)\n";
				next;
			}
		}

		next if !defined $hex;
		$colors{ $r->{role} } = $hex;
	}

	return \%colors;
}

# Fetch one key from a daisyUI sub-block and canonicalize it. Returns undef when
# the block, the key, or its format is absent or not a #RRGGBB triple.
sub lookup_hex
{
	my ($block, $key) = @_;

	return undef if !defined $block;

	my $raw = $block->{$key};
	return undef if !defined $raw;

	return normalize_hex($raw);
}

# Choose the secondary data-series color for one variant. Walks the candidate
# families in preference order and returns the first whose emitted color is at
# least $min_distinct from the primary, so the two series never collapse to one
# color where the palette can avoid it. Distinctness is measured on the emitted
# (already contrast-corrected) typography values, the colors actually drawn:
# because the adjustment shifts only lightness, this difference tracks the
# semantic families' own hue and chroma. When no candidate reaches the threshold
# the most-distinct one is used as a best effort and the shortfall is warned;
# truly monochrome sources, where even that coincides with the primary, are
# reported as indistinguishable.
sub resolve_secondary
{
	my ($group, $variant, $primary_hex) = @_;

	my $best_hex;
	my $best_de = -1.0;
	my $first_ok;

	for my $key (@secondary_keys)
	{
		my $hex = lookup_hex($variant->{typography}, $key);
		next if !defined $hex;

		my $de = defined $primary_hex ? delta_e76($primary_hex, $hex) : 1e9;

		if (!defined $first_ok && $de >= $min_distinct - 1e-9)
		{
			$first_ok = $hex;
		}

		if ($de > $best_de)
		{
			$best_de = $de;
			$best_hex = $hex;
		}
	}

	return $first_ok if defined $first_ok;

	if (!defined $best_hex)
	{
		warn "themes-json2ini: group '$group' has no secondary foreground; the "
			. "second data series inherits the legacy default\n";
		return undef;
	}

	if ($best_de <= 1e-9)
	{
		warn "themes-json2ini: group '$group' is monochrome: every foreground "
			. "collapses onto the primary, so both data series share "
			. "$best_hex\n";
	}
	else
	{
		warn sprintf("themes-json2ini: group '%s' second data series is only "
			. "deltaE %.1f from the first (below %.1f); the two may be hard to "
			. "tell apart\n", $group, $best_de, $min_distinct);
	}

	return $best_hex;
}

# Emit one INI group from a resolved color map, in canonical role order. Roles
# omitted by resolve_variant are skipped so the C loader inherits the legacy
# default for each.
sub emit_group
{
	my ($group, $variant, $colors) = @_;

	my @lines = ("\n[$group]\n");

	my $display = $variant->{displayName};
	push @lines, "display = $display\n" if defined $display;

	for my $r (@roles)
	{
		my $hex = $colors->{ $r->{role} };
		next if !defined $hex;
		push @lines, "$r->{role} = $hex\n";
	}

	return @lines;
}

# Verify one resolved daisyUI variant's foregrounds against WCAG-AA 4.5:1, the
# pairing the upstream contrast solve already targeted; any shortfall is a fatal
# violation. Series distinctness is decided during resolution, not here. The
# hand-authored literal groups are deliberately CGA-dim structural colors
# outside the solver's scope and are never passed here.
sub validate_variant
{
	my ($group, $colors) = @_;

	my $bg = $colors->{background};
	if (!defined $bg)
	{
		push @violations,
			"group '$group' has no background; cannot check contrast";
		return;
	}

	for my $r (@roles)
	{
		next if $r->{role} eq 'background';

		my $hex = $colors->{ $r->{role} };
		next if !defined $hex;

		my $ratio = contrast_ratio($hex, $bg);
		if ($ratio < $min_contrast - 1e-9)
		{
			push @violations, sprintf(
				"group '%s' role '%s' (%s) contrast %.2f:1 below %.2f:1 "
				. "against background %s",
				$group, $r->{role}, $hex, $ratio, $min_contrast, $bg);
		}
	}

	return;
}

# Split #RRGGBB into 0..255 channels. The caller guarantees the format via
# normalize_hex.
sub hex_channels
{
	my ($hex) = @_;
	$hex =~ /^#([0-9A-Fa-f]{2})([0-9A-Fa-f]{2})([0-9A-Fa-f]{2})$/;
	return (hex($1), hex($2), hex($3));
}

# sRGB gamma-expand one 0..255 channel to linear-light 0..1 (IEC 61966-2-1).
sub srgb_to_linear
{
	my ($c) = @_;
	$c /= 255.0;
	return $c <= 0.04045 ? $c / 12.92 : (($c + 0.055) / 1.055) ** 2.4;
}

# WCAG 2.x relative luminance of an #RRGGBB color.
sub rel_luminance
{
	my ($hex) = @_;
	my ($r, $g, $b) = map { srgb_to_linear($_) } hex_channels($hex);
	return 0.2126 * $r + 0.7152 * $g + 0.0722 * $b;
}

# WCAG 2.x contrast ratio between two colors, lighter over darker.
sub contrast_ratio
{
	my ($hex1, $hex2) = @_;
	my ($l1, $l2) = (rel_luminance($hex1), rel_luminance($hex2));
	($l1, $l2) = ($l2, $l1) if $l2 > $l1;
	return ($l1 + 0.05) / ($l2 + 0.05);
}

# CIELAB nonlinearity used in the XYZ -> L*a*b* transform.
sub lab_f
{
	my ($t) = @_;
	return $t > 0.008856 ? $t ** (1.0 / 3.0) : (7.787 * $t) + (16.0 / 116.0);
}

# Convert an #RRGGBB color to CIELAB (D65 reference white).
sub rgb_to_lab
{
	my ($hex) = @_;
	my ($r, $g, $b) = map { srgb_to_linear($_) } hex_channels($hex);

	my $x = (($r * 0.4124) + ($g * 0.3576) + ($b * 0.1805)) / 0.95047;
	my $y = (($r * 0.2126) + ($g * 0.7152) + ($b * 0.0722)) / 1.00000;
	my $z = (($r * 0.0193) + ($g * 0.1192) + ($b * 0.9505)) / 1.08883;

	my $fx = lab_f($x);
	my $fy = lab_f($y);
	my $fz = lab_f($z);

	return ((116.0 * $fy) - 16.0, 500.0 * ($fx - $fy), 200.0 * ($fy - $fz));
}

# Perceptual color difference (CIE76 deltaE*ab) between two #RRGGBB colors.
sub delta_e76
{
	my ($hex1, $hex2) = @_;
	my ($l1, $a1, $b1) = rgb_to_lab($hex1);
	my ($l2, $a2, $b2) = rgb_to_lab($hex2);
	return sqrt((($l1 - $l2) ** 2) + (($a1 - $a2) ** 2) + (($b1 - $b2) ** 2));
}

# Validate and canonicalize a color to uppercase #RRGGBB. Returns undef for any
# value that is not a six-digit hex triple.
sub normalize_hex
{
	my ($value) = @_;

	if ($value =~ /^#([0-9A-Fa-f]{6})$/)
	{
		return '#' . uc($1);
	}

	return undef;
}

sub slurp
{
	my ($path) = @_;

	open(my $fh, '<', $path) or die "themes-json2ini: cannot read $path: $!\n";
	my $data = do { local $/; <$fh> };
	close($fh);

	return $data;
}

sub spew
{
	my ($path, $data) = @_;

	open(my $fh, '>', $path) or die "themes-json2ini: cannot write $path: $!\n";
	print {$fh} $data;
	close($fh);

	return;
}

sub usage
{
	my $self = basename($0);
	return "Usage: $self [--input themes.json] [--output resources/themes.ini]\n"
		. "       [--min-contrast 4.5] [--min-distinct 10.0]\n";
}
