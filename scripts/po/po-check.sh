#!/bin/sh
#
# Validate a single .po catalog for release/commit readiness.
#
# Usage: scripts/po/po-check.sh [--review] <po-file>
#
# Gates, each reported by name on failure:
#   1. msgfmt -c --check-format: parses cleanly, valid header, and no
#      format-specifier type losses (%d vs %s) or count drift. msgfmt
#      reports its own file:line locations on stderr.
#   2. zero fuzzy entries; each remaining one reported with the po-file
#      line number of its msgid.
#   3. zero untranslated entries (skipped with --review); each remaining
#      one reported with the po-file line number of its msgid.
#   4. zero entries whose msgstr repeats their own msgid, except those
#      carrying an "xnec2c-exempt:" translator comment; each remaining one
#      reported with the po-file line number of its msgstr.
#
# Exit status: 0 all gates pass, 1 any gate fails, 2 usage error.

# Review mode gates staged partial catalogs: keep syntax, format, and fuzzy
# gates, drop the untranslated-completeness gate that a partial catalog fails.
review=false
case "${1:-}" in
	--review) review=true; shift ;;
esac

l="$1"

if [ -z "$l" ]; then
	echo "usage: $0 [--review] <po-file>" >&2
	exit 2
fi

if [ ! -f "$l" ]; then
	echo "$l: no such file" >&2
	exit 2
fi

rc=0

# Gate 1: syntax, header, and format-specifier checks.
if ! msgfmt -c --check-format -o /dev/null "$l"; then
	echo "$l: FAIL msgfmt -c --check-format reported errors"
	rc=1
fi

# Classify entries once through the shared awk classifier; it emits one
# tab-separated "state<TAB>line<TAB>key" record per flagged entry. Fuzzy
# records carry the "#, fuzzy" flag line, untranslated records the msgid line,
# and identical records the msgstr line, matching the per-gate reporting below.
records=$(awk -f "$(dirname "$0")/po-classify.awk" "$l")

# Report every entry one classifier state names, summarizing the state and then
# naming each entry at its own line. Returns non-zero when the state holds any.
report_state() (
	state=$1
	summary=$2
	detail=$3
	state_lines=$(printf '%s\n' "$records" | awk -F'\t' -v state="$state" '$1 == state { print $2 }')

	if [ -z "$state_lines" ]; then
		return 0
	fi

	state_count=$(printf '%s\n' "$state_lines" | wc -l)
	echo "$l: FAIL $state_count $summary"
	printf '%s\n' "$state_lines" | while IFS= read -r n; do
		echo "$l: FAIL $detail at line $n"
	done

	return 1
)

# Gate 2: fuzzy entries, reported at the "#, fuzzy" flag line of each entry.
report_state fuzzy 'fuzzy entries remain' 'fuzzy entry' || rc=1

# Gate 3: untranslated entries, reported at each entry's msgid line and
# skipped under --review for staged partial catalogs.
if [ "$review" = false ]; then
	report_state untranslated 'untranslated entries remain' 'untranslated entry' || rc=1
fi

# Gate 4: entries whose translation repeats their own source, reported at each
# entry's msgstr line; such an entry claims a translation gettext would
# otherwise fall back to, so it is rejected in review mode too.
report_state identical 'entries translate to their own source' 'identical translation' || rc=1

if [ "$rc" -eq 0 ]; then
	echo "$l: mechanical checks pass; language accuracy still requires review"
fi

exit $rc
