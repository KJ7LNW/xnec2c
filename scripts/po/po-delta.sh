#!/bin/bash
#
# Report work-tree catalog entries that are newly fuzzy or newly untranslated
# relative to a base revision. Entries are keyed by identity (msgctxt
# plus msgid), so po-refresh line-number churn never registers as a delta and a
# pre-existing fuzzy or untranslated entry present at the base never reports.
#
# Usage: scripts/po/po-delta.sh <base-ref> <po-path>
#
# The tip catalog is the work-tree file; the base catalog is the blob at
# <base-ref>, treated as empty when absent there. Reads git objects and files
# only; never mutates.
#
# Exit status: 1 when any new entry is reported, 0 when none, above 1 on a
# usage error or a failed read.

set -euo pipefail

usage() {
	echo "usage: $0 <base-ref> <po-path>" >&2
	exit 2
}

base=${1:-}
[ -n "$base" ] || usage
shift

po=${1:-}
[ -n "$po" ] || usage

# Reject a base revision resolving to no tree, so a mistyped revision never
# reads as a base lacking the catalog.
if ! git rev-parse --verify --quiet "$base^{tree}" > /dev/null; then
	echo "$0: err: $base: resolves to no tree" >&2
	exit 2
fi

classifier="$(dirname "$0")/po-classify.awk"

# Classify the base blob; an absent blob classifies to no records.
if git rev-parse --verify --quiet "$base:$po" > /dev/null; then
	base_records=$(git show "$base:$po" | awk -f "$classifier")
else
	# Report every tip entry of a catalog the base lacks as new.
	base_records=""
fi

# Classify the tip: the work-tree file.
tip_records=$(awk -f "$classifier" "$po")

# A tip record is new when its key is absent from the base key set; the key is
# field 3, identity-stable across line-number churn. An exempt record is the
# catalog's recorded outcome rather than pending work, so neither set reads it.
new=$(awk -F'\t' '
	NR == FNR { if (NF && $1 != "exempt") base[$3] = 1; next }
	NF && $1 != "exempt" && !($3 in base) { print }
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
