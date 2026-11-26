#!/bin/sh
#
# Validate a single .po catalog for release/commit readiness.
#
# Usage: scripts/po/po-check.sh <po-file>
#
# Gates, each reported by name on failure:
#   1. msgfmt -c --check-format: parses cleanly, valid header, and no
#      format-specifier type losses (%d vs %s) or count drift. msgfmt
#      reports its own file:line locations on stderr.
#   2. zero fuzzy entries; each remaining one reported with the po-file
#      line number of its flags line naming fuzzy.
#   3. zero untranslated entries; each remaining
#      one reported with the po-file line number of its msgid. An entry
#      carrying a sole "xnec2c-exempt:" translator comment records that this
#      catalog writes no translation, so its empty msgstr is the recorded
#      outcome rather than a missing one.
#   4. zero entries whose msgstr repeats their own msgid; an exemption
#      comment grants no exception, because an exemption records an absent
#      translation rather than a copied one. Each remaining one is reported
#      with the po-file line number of its msgstr.
#   5. the Plural-Forms header holds the value the GNU gettext or Translate
#      House plural table publishes for the catalog's language
#      (scripts/po/po-plural-check.sh), reported at the header line.
#
# Exit status: 0 all gates pass, 1 any gate fails, 2 usage error or
# classifier failure.

l="$1"

if [ -z "$l" ]; then
	echo "usage: $0 <po-file>" >&2
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
# records carry the flags line naming fuzzy, untranslated records the msgid line,
# and identical records the msgstr line, matching the per-gate reporting below.
if ! records=$(awk -f "$(dirname "$0")/po-classify.awk" "$l"); then
	echo "$l: entry classifier failed" >&2
	exit 2
fi

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

# Gate 2: fuzzy entries, reported at the flags line naming fuzzy of each entry.
report_state fuzzy 'fuzzy entries remain' 'fuzzy entry' || rc=1

# Gate 3: untranslated entries, reported at each entry's msgid line.
report_state untranslated 'untranslated entries remain' 'untranslated entry' || rc=1

# Gate 4: entries whose translation repeats their own source, reported at each
# entry's msgstr line; such an entry claims a translation gettext would
# otherwise fall back to.
report_state identical 'entries translate to their own source' 'identical translation' || rc=1

# Gate 5: Plural-Forms header against the value its language's published plural
# table row holds.
"$(dirname "$0")/po-plural-check.sh" "$l"
plural_status=$?
case $plural_status in
0)
	# The header holds the published value.
	;;
1)
	echo "$l: FAIL Plural-Forms check reported errors"
	rc=1
	;;
*)
	echo "$l: plural check failed with status $plural_status" >&2
	exit 2
	;;
esac

if [ "$rc" -eq 0 ]; then
	echo "$l: mechanical checks pass; language accuracy still requires review"
fi

exit $rc
