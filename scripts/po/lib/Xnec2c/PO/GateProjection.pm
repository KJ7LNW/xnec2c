package Xnec2c::PO::GateProjection;

use strict;
use warnings;
use utf8;

use Exporter qw(import);
use Locale::PO;

use Xnec2c::PO::CatalogState qw(catalog_identity po_identity);

our @EXPORT_OK = qw(catalog_line_index mapped_catalog_projection);

# Derive the gate projection: the catalog header plus only the entries the
# current output map answers.
sub mapped_catalog_projection
{
	my ($catalog, $translations) = @_;
	my @projection;

	for my $po (@{$catalog})
	{
		my $identity = po_identity($po);
		next if $identity->{source} ne ''
			&& !exists $translations->{$identity->{identity}};

		push @projection, $po;
	}

	return \@projection;
}

# Index every projected catalog entry by its starting line and map record. Each
# quoted line passes through the catalog's own dequoting, so a parsed identity
# matches the identity Locale::PO derives for the same entry.
sub catalog_line_index
{
	my ($content, $translations) = @_;
	my @entries;
	my %entry;
	my $line_number = 0;

	# Append one delimiter so the final entry closes through the same blank-line
	# path whether or not the serialized projection ends with a newline.
	for my $line (split /\n/, $content . "\n", -1)
	{
		$line_number++;
		if ($line =~ /\A\s*\z/)
		{
			push @entries, { %entry } if exists $entry{start};
			%entry = ();
			next;
		}

		$entry{start} = $line_number if !exists $entry{start};
		if ($line =~ /\Amsgctxt[ \t]+(".*")[ \t]*\z/)
		{
			$entry{field} = 'C';
			$entry{C} = Locale::PO->dequote($1);
		}
		elsif ($line =~ /\Amsgid[ \t]+(".*")[ \t]*\z/)
		{
			$entry{field} = 'S';
			$entry{S} = Locale::PO->dequote($1);
		}
		elsif ($line =~ /\A(".*")[ \t]*\z/ && defined $entry{field})
		{
			$entry{$entry{field}} .= Locale::PO->dequote($1);
		}
		else
		{
			delete $entry{field};
		}
	}

	for my $found (@entries)
	{
		my $identity = catalog_identity($found->{C} // '', $found->{S} // '');
		$found->{record} = $translations->{$identity};
	}

	return \@entries;
}

1;
