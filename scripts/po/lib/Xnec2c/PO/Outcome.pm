package Xnec2c::PO::Outcome;

use strict;
use warnings;
use utf8;

use Exporter qw(import);

our @EXPORT_OK = qw(
	OUTCOME_FATAL OUTCOME_OK OUTCOME_REPAIR command_outcome new_outcome
);

# Enumerate the closed set every workflow operation returns.
use constant OUTCOME_OK => 'ok';
use constant OUTCOME_REPAIR => 'repair';
use constant OUTCOME_FATAL => 'fatal';

my %OUTCOME_KIND_MAP = map { $_ => 1 }
	(OUTCOME_OK, OUTCOME_REPAIR, OUTCOME_FATAL);

# Construct one operation outcome, rejecting any kind outside the closed set.
sub new_outcome
{
	my (%fields) = @_;
	my $kind = $fields{kind};

	die "outcome requires a kind\n" if !defined $kind;
	die "unsupported outcome kind $kind\n" if !exists $OUTCOME_KIND_MAP{$kind};

	return {
		kind => $kind,
		faults => $fields{faults} // [],
		warnings => $fields{warnings} // [],
		applied_count => $fields{applied_count} // 0,
	};
}

# Resolve one child command's process outcome.
sub command_outcome
{
	my ($name, $status) = @_;

	return new_outcome(kind => OUTCOME_FATAL,
		faults => ["$name: unable to run: $!"]) if $status == -1;
	return new_outcome(kind => OUTCOME_FATAL,
		faults => ["$name: terminated by signal " . ($status & 127)])
		if ($status & 127) != 0;
	return new_outcome(kind => OUTCOME_FATAL,
		faults => ["$name: failed with status " . ($status >> 8)])
		if ($status >> 8) != 0;

	return new_outcome(kind => OUTCOME_OK);
}

1;
