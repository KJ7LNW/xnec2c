package Xnec2c::PO::ExemptionWorkflow;

use strict;
use warnings;
use utf8;

use Exporter qw(import);

use Xnec2c::PO::ExemptionAudit qw(audit_exemptions);
use Xnec2c::PO::ExemptionSession qw(run_exemption_session);
use Xnec2c::PO::LanguageWorkflow qw(refresh_language);
use Xnec2c::PO::LanguageWorkspace qw(catalog_languages);

our @EXPORT_OK = qw(apply_exemptions);

# Bound the sessions this stage seats. A fault surviving these attempts wants
# an operator rather than another session, so the stage stops instead of
# spending sessions that carry the catalogs no further.
use constant SESSION_ATTEMPT_MAX => 3;

# Terminate the stage whose faults outlive its session budget.
sub assert_session_budget
{
	my ($attempts, $faults) = @_;

	die 'po: FAIL ' . scalar(@{$faults}) . ' exemption faults outstanding'
		. ' after ' . SESSION_ATTEMPT_MAX . ' sessions; correct the catalogs'
		. " the report above names, then run apply again\n"
		if $attempts >= SESSION_ATTEMPT_MAX;
}

# Drive every catalog to exemption agreement once the per-language loop drains.
# A reason resting on the source alone binds every catalog, so the state it is
# judged against exists only after the last language commits, and no language
# session holds it.
sub apply_exemptions
{
	my $attempts = 0;
	my $faults = audit_exemptions();

	while (@{$faults})
	{
		assert_session_budget($attempts, $faults);
		$attempts++;
		print "po: exemption session $attempts of " . SESSION_ATTEMPT_MAX
			. "\n";
		run_exemption_session($faults);
		$faults = audit_exemptions();
	}

	# Rewrite every catalog through the single regeneration point. The refresh
	# opening each per-language iteration already normalizes what that language
	# commits, leaving this stage the one writer without one behind it: its
	# sessions edit catalogs by hand, in a form the gettext tools do not write.
	# The sweep closes apply with one form across every catalog, so a later
	# refresh carries no reformatting into an unrelated commit.
	refresh_language($_) for catalog_languages();
	print "po: exemption agreement reached after $attempts sessions\n";
}

1;
