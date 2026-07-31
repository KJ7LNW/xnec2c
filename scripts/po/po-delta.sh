#!/bin/bash
#
# Report catalog entries that are newly fuzzy or newly untranslated at a tip
# revision relative to a base revision. Entries are keyed by identity (msgctxt
# plus msgid), so update-po line-number churn never registers as a delta and a
# pre-existing fuzzy or untranslated entry present at the base never reports.
#
# Usage: scripts/po/po-delta.sh <base-ref> [--tip <tip-ref>] <po-path>
#
# With --tip the tip catalog is the blob at <tip-ref>; without it the tip is
# the work-tree file. The base catalog is the blob at <base-ref>, treated as
# empty when absent there. Reads git objects and files only; never mutates.
#
# Exit status: 1 when any new entry is reported, 0 when none, 2 usage error.

set -euo pipefail

usage() {
	echo "usage: $0 <base-ref> [--tip <tip-ref>] <po-path>" >&2
	exit 2
}

base=${1:-}
[ -n "$base" ] || usage
shift

tip=""
tip_given=false
if [ "${1:-}" = "--tip" ]; then
	tip=${2:-}
	[ -n "$tip" ] || usage
	tip_given=true
	shift 2
fi

po=${1:-}
[ -n "$po" ] || usage

classifier="$(dirname "$0")/po-classify.awk"

# Classify the base blob; an absent blob classifies to no records.
base_records=$(git show "$base:$po" 2>/dev/null | awk -f "$classifier" || true)

# Classify the tip: the blob at --tip when given, else the work-tree file.
if $tip_given; then
	tip_records=$(git show "$tip:$po" 2>/dev/null | awk -f "$classifier" || true)
else
	tip_records=$(awk -f "$classifier" "$po")
fi

# A tip record is new when its key is absent from the base key set; the key is
# field 3, identity-stable across line-number churn.
new=$(awk -F'\t' '
	NR == FNR { if (NF) base[$3] = 1; next }
	NF && !($3 in base) { print }
' <(printf '%s\n' "$base_records") <(printf '%s\n' "$tip_records"))

if [ -z "$new" ]; then
	exit 0
fi

# Report each new entry at its state line; the msgid excerpt is the key text
# after the U+0004 msgctxt separator.
printf '%s\n' "$new" | while IFS=$'\t' read -r state line key; do
	printf '%s:%s: NEW %s: %s\n' "$po" "$line" "$state" "${key#*$'\004'}"
done

exit 1
