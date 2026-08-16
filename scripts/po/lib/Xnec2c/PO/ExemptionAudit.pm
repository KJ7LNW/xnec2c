package Xnec2c::PO::ExemptionAudit;

use strict;
use warnings;
use utf8;

use Exporter qw(import);

use Xnec2c::PO::CatalogState qw(catalog_annotation_records catalog_path);
use Xnec2c::PO::ExemptionEntryAudit qw(accepted_exemption entry_faults);
use Xnec2c::PO::LanguageWorkspace qw(catalog_languages);
use Xnec2c::PO::MapFile qw(source_excerpt);
use Xnec2c::PO::TranslationRules qw(exemption_conflict exemption_propagates);

our @EXPORT_OK = qw(audit_exemptions);

# Bound the catalogs one cross-catalog report names before it summarizes the
# rest, so a fault every catalog holds still reads on one line.
use constant LANGUAGE_SAMPLE_MAX => 8;

# Pluralize one counted noun for compact report text.
sub counted
{
	my ($count, $noun) = @_;
	my $result;

	if ($count == 1)
	{
		$result = "$count $noun";
	}
	else
	{
		$result = "$count ${noun}s";
	}

	return $result;
}

# Name the catalogs one fault concerns, summarizing the tail beyond the sample
# bound.
sub language_list
{
	my ($languages) = @_;
	my $total = scalar @{$languages};
	my $result;

	if ($total <= LANGUAGE_SAMPLE_MAX)
	{
		$result = counted($total, 'catalog') . ': '
			. join(', ', @{$languages});
	}
	else
	{
		$result = counted($total, 'catalog') . ': '
			. join(', ', @{$languages}[0 .. LANGUAGE_SAMPLE_MAX - 1])
			. ' and ' . ($total - LANGUAGE_SAMPLE_MAX) . ' more';
	}

	return $result;
}

# Name the catalog entry one fault concerns. A context distinguishes entries
# sharing one source, so it reads beside the source wherever an entry holds it.
sub entry_subject
{
	my ($entry) = @_;
	my $subject = 'source "' . source_excerpt($entry->{source}) . '"';
	my $result;

	if ($entry->{context} eq '')
	{
		$result = $subject;
	}
	else
	{
		$result = $subject . ' in context "'
			. source_excerpt($entry->{context}) . '"';
	}

	return $result;
}

# Write one cross-catalog fault naming the entry it concerns. The fault rests
# on no single catalog, so it reports against the catalog directory.
sub cross_fault
{
	my ($state, $text) = @_;

	return 'po: FAIL ' . entry_subject($state) . " $text";
}

# Record one structurally valid, documented exemption claim for comparison.
sub record_claim
{
	my ($states, $record, $lang) = @_;
	my $accepted = accepted_exemption($record);

	return if !defined $accepted;

	my $state = $states->{$record->{identity}} //= {
		context => $record->{context},
		source => $record->{source},
		claims => {},
	};
	push @{$state->{claims}{$accepted}}, $lang;
}

# Reduce every catalog to the reasons each source identity carries across them,
# naming the catalogs claiming each. Entry faults surface here because one pass
# reads every catalog.
sub source_states
{
	my ($languages) = @_;
	my %states;
	my @faults;

	for my $lang (@{$languages})
	{
		my $path = catalog_path($lang);
		for my $record (@{catalog_annotation_records($lang)})
		{
			push @faults, map {
				"$path:$record->{line}: FAIL $_ on " . entry_subject($record)
			} entry_faults($record);
			record_claim(\%states, $record, $lang);
		}
	}

	return { states => \%states, faults => \@faults };
}

# Name the fault two source-level reasons on one source raise, and nothing
# where at most one of them stands.
sub propagating_divergence
{
	my ($propagating) = @_;

	return () if scalar(@{$propagating}) <= 1;

	return ('carries ' . scalar(@{$propagating}) . ' deviating exemptions;'
		. ' one source holds one reason');
}

# Name the fault a source-level reason standing beside a language-local one
# raises, and nothing where the two never meet.
sub local_divergence
{
	my ($propagating, $local) = @_;

	return () if scalar(@{$propagating}) != 1 || scalar(@{$local}) == 0;

	return ("carries $propagating->[0], which rests on the source"
		. ' alone, beside ' . join(', ', @{$local}) . ', which rests on'
		. " one language's own vocabulary; one of them describes this"
		. ' source wrongly');
}

# Judge one source identity against the reasons every catalog records for it. A
# reason resting on the source alone holds in every language, so two catalogs
# naming different such reasons describe one source two ways and one of them is
# wrong, and a language-local reason standing beside one contradicts it the
# same way. Each reason also states the content it covers, so a source its own
# reason excludes reports here once beside the catalogs claiming it.
sub cross_catalog_faults
{
	my ($state) = @_;
	my @claimed = sort keys %{$state->{claims}};
	my (@propagating, @local, @reports);

	for my $reason (@claimed)
	{
		if (exemption_propagates($reason))
		{
			push @propagating, $reason;
		}
		else
		{
			push @local, $reason;
		}

		my $conflict = exemption_conflict($reason, $state->{source});
		next if !defined $conflict;

		push @reports, "is $reason in "
			. language_list($state->{claims}{$reason})
			. " yet $conflict; write the reason its content takes";
	}

	my @divergence = propagating_divergence(\@propagating);
	push @divergence, local_divergence(\@propagating, \@local);
	push @divergence, map {
		"is $_ in " . language_list($state->{claims}{$_})
	} (scalar(@divergence) != 0 ? @claimed : ());

	return map { cross_fault($state, $_) } @reports, @divergence;
}

# Audit every catalog for exemption agreement, reporting each fault and
# returning their total.
sub audit_exemptions
{
	my @languages = catalog_languages();
	my $reduction = source_states(\@languages);
	my $states = $reduction->{states};
	my @faults = @{$reduction->{faults}};

	push @faults, cross_catalog_faults($states->{$_})
		for sort {
			$states->{$a}{source} cmp $states->{$b}{source}
				|| $states->{$a}{context} cmp $states->{$b}{context}
				|| $a cmp $b
		} keys %{$states};

	my $fault_count = scalar @faults;
	print "$_\n" for @faults;
	if ($fault_count == 0)
	{
		print 'po: exemption comments agree across '
			. counted(scalar @languages, 'catalog') . "\n";
	}
	else
	{
		print 'po: FAIL ' . counted($fault_count, 'exemption fault')
			. ' across ' . counted(scalar @languages, 'catalog') . "\n";
	}

	return $fault_count;
}

1;
