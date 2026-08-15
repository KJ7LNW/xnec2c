package Xnec2c::PO::ExemptionAudit;

use strict;
use warnings;
use utf8;

use Exporter qw(import);

use Xnec2c::PO::CatalogState qw(
	catalog_annotation_records catalog_path exempt_marker
);
use Xnec2c::PO::LanguageWorkspace qw(catalog_languages);
use Xnec2c::PO::MapFile qw(source_excerpt);
use Xnec2c::PO::TranslationRules qw(exemption_names propagating_exemptions);

our @EXPORT_OK = qw(audit_exemptions);

# Bound the catalogs one cross-catalog report names before it summarizes the
# rest, so a fault every catalog holds still reads on one line.
use constant LANGUAGE_SAMPLE_MAX => 8;

my %DOCUMENTED_NAME_MAP = map { $_ => 1 } exemption_names();
my %PROPAGATING_NAME_MAP = map { $_ => 1 } propagating_exemptions();

sub accepted_exemption;
sub documented_names;
sub undocumented_names;

# Each row names one fault a single catalog entry holds: the condition raising
# it and the report it writes. Adding a rule adds a row.
my @ENTRY_RULE_ROWS = (
	{
		holds => sub {
			return scalar(@{$_[0]{markers}}) > scalar(@{$_[0]{names}});
		},
		report => sub {
			return 'malformed exemption comment; write "' . exempt_marker()
				. ': <name>" naming one of ' . documented_names();
		},
	},
	{
		holds => sub { return scalar(@{$_[0]{markers}}) > 1; },
		report => sub {
			return scalar(@{$_[0]{markers}})
				. ' exemption comments on one entry; write one';
		},
	},
	{
		holds => sub { return scalar(undocumented_names($_[0])) != 0; },
		report => sub {
			return 'undocumented exemption '
				. join(', ', undocumented_names($_[0])) . '; write one of '
				. documented_names();
		},
	},
	{
		holds => sub {
			return defined accepted_exemption($_[0])
				&& $_[0]{translation} eq '';
		},
		report => sub {
			return 'exemption ' . accepted_exemption($_[0])
				. ' stands on an untranslated entry; write the source form in'
				. ' msgstr or drop the comment';
		},
	},
	{
		holds => sub {
			return defined accepted_exemption($_[0])
				&& $_[0]{translation} ne ''
				&& $_[0]{translation} ne $_[0]{source};
		},
		report => sub {
			return 'exemption ' . accepted_exemption($_[0])
				. ' stands on a translated entry; drop the comment or restore'
				. ' the source form';
		},
	},
);

# Return the documented exemption one structurally valid comment claims.
sub accepted_exemption
{
	my ($record) = @_;
	my $result;

	if ((@{$record->{markers}} == 1) && (@{$record->{names}} == 1)
		&& exists $DOCUMENTED_NAME_MAP{$record->{names}[0]})
	{
		$result = $record->{names}[0];
	}
	else
	{
		# Leave malformed, repeated, and undocumented comments unresolved
		# after their entry rules report them.
		$result = undef;
	}

	return $result;
}

# Return every syntactically valid name outside the documented vocabulary.
sub undocumented_names
{
	my ($record) = @_;

	return grep { !exists $DOCUMENTED_NAME_MAP{$_} } @{$record->{names}};
}

# Name the documented exemptions one message offers as its correction.
sub documented_names
{
	return join(', ', exemption_names());
}

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
			push @faults, "$path:$record->{line}: FAIL "
				. $_->{report}->($record) . ' on ' . entry_subject($record)
				for grep { $_->{holds}->($record) } @ENTRY_RULE_ROWS;
			record_claim(\%states, $record, $lang);
		}
	}

	return { states => \%states, faults => \@faults };
}

# Judge one source identity against the reasons every catalog records for it. A
# reason resting on the source alone holds in every language, so two catalogs
# naming different such reasons describe one source two ways and one of them is
# wrong. A loanword rests on the adopting language's own vocabulary, so it
# enters no comparison.
sub cross_catalog_faults
{
	my ($state) = @_;
	my @reasons = grep { exists $PROPAGATING_NAME_MAP{$_} }
		sort keys %{$state->{claims}};
	my @faults;

	return @faults if @reasons < 2;

	push @faults, cross_fault($state, 'carries ' . scalar(@reasons)
		. ' deviating exemptions; one source holds one reason');
	push @faults, cross_fault($state,
		"is $_ in " . language_list($state->{claims}{$_})) for @reasons;

	return @faults;
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
