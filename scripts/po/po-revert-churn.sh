#!/bin/bash
#
# Restore one catalog or template from the git index when its sole working-tree
# change is reference-line or header-date churn. Single point of truth for that
# comparison; msgmerge and xgettext each call this on the file they just wrote.
#
# Usage: scripts/po/po-revert-churn.sh <po-or-pot-path>
#
# Exits 0 leaving the file untouched when no work tree covers it, when it is
# untracked, or when it carries a genuine change.

set -euo pipefail

file=$1
dir=$(dirname "$file")
base=$(basename "$file")

# Collapse the line number in every "#:" source-reference token to a fixed
# placeholder and blank the generated POT-Creation-Date, so two catalogs
# compare equal when they differ only by where a string now sits in the
# sources. Referenced filenames, fuzzy flags, and all msgid/msgstr content
# stay intact, so a genuine reference, flag, or translation change still shows.
normalize_po_refs() {
    sed -e '/^#:/ s/:[0-9][0-9]*/:LINE/g' \
        -e '/^"POT-Creation-Date:/ s/.*/"POT-Creation-Date: DATE"/' "$1"
    return $?
}

# The index holds the pre-merge state, since msgmerge and xgettext write the
# working tree only. A catalog unpacked from a dist tarball carries no work
# tree, so the comparison has no baseline and the file stands as written.
root=$(git -C "$dir" rev-parse --show-toplevel 2> /dev/null) || exit 0
rel=$(git -C "$dir" ls-files --full-name --error-unmatch "$base" 2> /dev/null) || exit 0

if diff -q \
    <(git -C "$root" show ":$rel" | normalize_po_refs /dev/stdin) \
    <(normalize_po_refs "$file") > /dev/null; then
    git -C "$root" checkout -- "$rel"
    echo "  reverted (line-only): $rel"
fi

exit 0
