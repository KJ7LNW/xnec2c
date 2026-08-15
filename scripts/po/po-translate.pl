#!/usr/bin/env perl

use strict;
use warnings;
use utf8;

use FindBin qw($RealBin);
use Getopt::Long qw(GetOptionsFromArray);
use lib "$RealBin/lib";

use Xnec2c::PO::ModelSession qw(set_model);
use Xnec2c::PO::WorkflowCommands qw(
	cmd_apply cmd_generate cmd_inject cmd_lint cmd_verify
);

binmode STDOUT, ':encoding(UTF-8)';
binmode STDERR, ':encoding(UTF-8)';

# Keep report lines in the order they are written; a model reads this output
# interleaved with the gate's own stderr.
$| = 1;

my $model;
my %command_map = (
	generate => { min_args => 0, max_args => 0, handler => \&cmd_generate },
	apply => { min_args => 0, max_args => 0, handler => \&cmd_apply },
	lint => { min_args => 1, max_args => undef, handler => \&cmd_lint },
	verify => { min_args => 1, max_args => 1, handler => \&cmd_verify },
	inject => { min_args => 1, max_args => 1, handler => \&cmd_inject },
);

my $options_ok = GetOptionsFromArray(
	\@ARGV,
	'model|m=s' => \$model,
);
usage() if !$options_ok;
set_model($model) if defined $model;

my $command = shift @ARGV // '';
my $command_spec = $command_map{$command};
usage() if !defined $command_spec;
usage() if scalar(@ARGV) < $command_spec->{min_args};
usage() if defined $command_spec->{max_args}
	&& scalar(@ARGV) > $command_spec->{max_args};
$command_spec->{handler}->(@ARGV);

exit 0;

# Report command syntax and terminate with a usage failure.
sub usage
{
	die "usage: $0 [--model NAME] generate|apply\n"
		. "       $0 [--model NAME] lint <lang|map-path> [<lang|map-path>...]\n"
		. "       $0 [--model NAME] verify|inject <lang|map-path>\n";
}
