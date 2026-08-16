package Xnec2c::PO::ExemptionEntryAudit;

use strict;
use warnings;
use utf8;

use Exporter qw(import);

use Xnec2c::PO::CatalogState qw(exempt_marker);
use Xnec2c::PO::TranslationRules qw(exemption_names);

our @EXPORT_OK = qw(accepted_exemption entry_faults);

my %DOCUMENTED_NAME_MAP = map { $_ => 1 } exemption_names();

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
				&& $_[0]{translation} ne '';
		},
		report => sub {
			return 'exemption ' . accepted_exemption($_[0])
				. ' stands beside a non-empty msgstr; clear msgstr to keep the'
				. ' exemption or drop the comment to keep the translation';
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

# Report every fault one catalog entry's exemption comment holds, in rule
# order.
sub entry_faults
{
	my ($record) = @_;

	return map { $_->{report}->($record) }
		grep { $_->{holds}->($record) } @ENTRY_RULE_ROWS;
}

1;
