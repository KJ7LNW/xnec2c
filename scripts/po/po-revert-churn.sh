#!/bin/bash
#
# Restore one catalog or template from the git index when its sole working-tree
# change is entry-order, reference-line, or header-date churn. Single point of
# truth for that comparison; msgmerge and xgettext each call this on the file
# they just wrote.
#
# Usage: scripts/po/po-revert-churn.sh <po-or-pot-path>
#
# Exits 0 leaving the file untouched when no work tree covers it, when it is
# untracked, or when it carries a genuine change.

set -euo pipefail

file=$1
dir=$(dirname "$file")
base=$(basename "$file")

# Fold the three generated axes so two catalogs compare equal when they differ
# only in how a regeneration arranged them: msgcat --sort-output imposes one
# entry order, so a template reordering never registers; the line number in
# every "#:" source-reference token collapses to a fixed placeholder; and the
# generated POT-Creation-Date blanks. Referenced filenames, fuzzy flags, and
# all msgid/msgstr content stay intact, so a genuine reference, flag, or
# translation change still shows.
normalize_po_refs() {
    msgcat --sort-output --no-wrap "$1" \
        | sed -e '/^#:/ s/:[0-9][0-9]*/:LINE/g' \
              -e '/^"POT-Creation-Date:/ s/.*/"POT-Creation-Date: DATE"/'
    return $?
}

# The index holds the pre-merge state, since msgmerge and xgettext write the
# working tree only. A catalog unpacked from a dist tarball carries no work
# tree, so the comparison has no baseline and the file stands as written.
root=$(git -C "$dir" rev-parse --show-toplevel 2> /dev/null) || exit 0
rel=$(git -C "$dir" ls-files --full-name --error-unmatch "$base" 2> /dev/null) || exit 0

# Capture both normalizations before comparing. A msgcat failure aborts here
# under set -o pipefail, rather than yielding an empty form on both sides that
# would compare equal and check out a file holding real work.
index_form=$(git -C "$root" show ":$rel" | normalize_po_refs /dev/stdin)
tree_form=$(normalize_po_refs "$file")

if [ "$index_form" = "$tree_form" ]; then
    git -C "$root" checkout -- "$rel"
    echo "  reverted (churn-only): $rel"
fi

exit 0
