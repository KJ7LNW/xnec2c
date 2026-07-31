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
# tab-separated "state<TAB>line<TAB>key" record per fuzzy or untranslated
# entry. Fuzzy records carry the "#, fuzzy" flag line and untranslated records
# the msgid line, matching the per-gate reporting below.
records=$(awk -f "$(dirname "$0")/po-classify.awk" "$l")

# Gate 2: fuzzy entries, reported at the "#, fuzzy" flag line of each entry.
fuzzy_lines=$(printf '%s\n' "$records" | awk -F'\t' '$1 == "fuzzy" { print $2 }')

if [ -n "$fuzzy_lines" ]; then
	fuzzy_count=$(printf '%s\n' "$fuzzy_lines" | wc -l)
	echo "$l: FAIL $fuzzy_count fuzzy entries remain"
	printf '%s\n' "$fuzzy_lines" | while IFS= read -r n; do
		echo "$l: FAIL fuzzy entry at line $n"
	done
	rc=1
fi

# Gate 3: untranslated entries, reported at each entry's msgid line and
# skipped under --review for staged partial catalogs.
if [ "$review" = false ]; then
	untranslated_lines=$(printf '%s\n' "$records" | awk -F'\t' '$1 == "untranslated" { print $2 }')

	if [ -n "$untranslated_lines" ]; then
		untranslated_count=$(printf '%s\n' "$untranslated_lines" | wc -l)
		echo "$l: FAIL $untranslated_count untranslated entries remain"
		printf '%s\n' "$untranslated_lines" | while IFS= read -r n; do
			echo "$l: FAIL untranslated entry at line $n"
		done
		rc=1
	fi
fi

if [ "$rc" -eq 0 ]; then
	echo "$l: mechanical checks pass; language accuracy still requires review"
fi

exit $rc
