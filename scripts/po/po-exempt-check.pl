#!/usr/bin/env perl
#
# Validate exemption agreement across every catalog in po/.
#
# Usage: scripts/po/po-exempt-check.pl [--help]
#
# An "xnec2c-exempt:" translator comment records that a source holds no
# distinct target form, so the catalog writes no translation for it and its
# msgstr stays empty. That claim rests on grounds no single catalog can
# validate, so the gates below read every catalog together:
#
#   1. every exemption comment reads the recorded form and names one
#      documented exemption
#   2. one exemption comment per entry
#   3. an exempted entry holds no translation of its own, the state its
#      exemption records
#   4. one source carries one reason across every catalog claiming it, except
#      loanword, which rests on the adopting language's own vocabulary and so
#      reaches no other catalog
#
# Run from the repository root; catalog paths resolve against po/.
#
# This command reports and never writes. A fault names the entry and the
# correction it takes, which a translator applies to that catalog directly.
#
# Exit status: 0 all gates pass, 1 any gate fails, 2 usage error.

use strict;
use warnings;
use utf8;

use File::Basename qw(basename);
use FindBin qw($RealBin);
use Getopt::Long qw(GetOptions);
use lib "$RealBin/lib";

use Xnec2c::PO::ExemptionAudit qw(audit_exemptions);

binmode STDOUT, ':encoding(UTF-8)';
binmode STDERR, ':encoding(UTF-8)';

my $help = 0;

usage(2) if !GetOptions('help' => \$help);
usage(0) if $help;
usage(2) if @ARGV;

exit(audit_exemptions() == 0 ? 0 : 1);

# Report command syntax and terminate with the caller's status.
sub usage
{
	my ($status) = @_;
	my $text = 'usage: ' . basename($0) . " [--help]\n";

	if ($status == 0)
	{
		print $text;
	}
	else
	{
		print STDERR $text;
	}

	exit $status;
}
