package Xnec2c::PO::ExemptionSession;

use strict;
use warnings;
use utf8;

use Exporter qw(import);

use Xnec2c::PO::ModelSession qw(run_injection);
use Xnec2c::PO::TranslationRules qw(cross_catalog_rules);

our @EXPORT_OK = qw(run_exemption_session);

# Name the audit this session answers and repeats until it passes.
use constant AUDIT_COMMAND => 'scripts/po/po-exempt-check.pl';

# Name the make target proving every per-catalog gate still passes behind the
# audit. The full check target also proves the tree carries no uncommitted
# catalog, a state every session leaves behind, so the stage stops here.
use constant GATE_COMMAND => 'make check-po-exemptions';

# State the files this session reads and writes, the corrections it applies,
# and the commands it runs.
sub workspace_rules
{
	return join("\n",
		'- Edit only the po/<lang>.po catalogs the report names.',
		'- Read only po/<lang>.po catalogs, po/rules/<lang>.md, and'
			. ' doc/TRANSLATING.md; the injected context supplies the rest.',
		'- Open no program source; act on what the audit reports, never on how'
			. ' it works.',
		'- Apply every correction with Edit, at the entry the report names.',
		'- Retain every other entry; never rewrite a whole catalog.',
		'- Write and run no program that rewrites a catalog.',
		'- Run no Bash command except ' . AUDIT_COMMAND . ' and '
			. GATE_COMMAND . '.');
}

# State the ordered steps closing the session.
sub session_contract
{
	return join("\n",
		'# Procedure',
		'',
		'1. Read the source each fault names in every catalog, both the'
			. ' catalogs it names and the catalogs it does not.',
		'2. Edit each catalog entry the rules above resolve.',
		'3. Run ' . AUDIT_COMMAND . ' until it reports no FAIL. Repeat steps 1'
			. ' and 2 against each report.',
		'4. Run ' . GATE_COMMAND . ' until it passes. Repeat steps 1 through 3'
			. ' against each FAIL it reports.',
		'5. Stop once both commands pass.',
		'',
		'',
		'Perform item 1 now.');
}

# Direct correction of the catalog entries the audit rejected.
sub exemption_instruction
{
	my ($faults) = @_;
	my $report = join("\n", map { "- $_" } @{$faults});

	return join("\n\n",
		'Correct the exemption comments the audit below rejects. The catalogs'
			. ' it does not name already agree; keep them that way.',
		"# Reported faults\n\n$report",
		cross_catalog_rules(),
		"# Workspace\n\n" . workspace_rules(),
		session_contract());
}

# Seat one session against the catalogs the audit faults name. The audit
# injects last so its report reads the catalogs as they stand on disk.
sub run_exemption_session
{
	my ($faults) = @_;

	die "exemption session requires the faults it answers\n"
		if !defined $faults || !@{$faults};

	run_injection('po', ['doc/TRANSLATING.md', '!' . AUDIT_COMMAND],
		exemption_instruction($faults));
}

1;
