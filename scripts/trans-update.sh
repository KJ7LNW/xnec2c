#!/bin/bash
#
# Synchronize po/POTFILES.in with git-tracked source files and
# regenerate translation catalogs.
#
# Usage:
#   ./scripts/trans-update.sh          # regenerate POTFILES.in, make update-po, show stats
#   ./scripts/trans-update.sh --check  # report differences without modifying files
#   ./scripts/trans-update.sh --sync   # regenerate POTFILES.in only when it drifted
#   ./scripts/trans-update.sh --revert-line-only  # restore catalogs whose only
#                                                  # change is reference line
#                                                  # numbers or the header date
#

set -euo pipefail

cd "$(dirname "$0")/.."

POTFILES_IN="po/POTFILES.in"

check_only=false
sync_only=false
revert_line_only=false
case "${1:-}" in
    --check) check_only=true ;;
    --sync)  sync_only=true ;;
    --revert-line-only) revert_line_only=true ;;
    "")      ;;
    *)
        echo "Usage: $(basename "$0") [--check|--sync|--revert-line-only]"
        echo ""
        echo "  --check             Report differences without modifying files"
        echo "  --sync              Regenerate POTFILES.in only when it drifted; skip update-po"
        echo "  --revert-line-only  Restore catalogs whose only change since the index is"
        echo "                      reference line numbers or the header date (needs update-po first)"
        echo "  (none)              Regenerate POTFILES.in, run make update-po, show stats"
        exit 1
        ;;
esac

# All git-tracked C source, header, glade, and desktop entry files.
# xgettext keywords in po/Makevars control which strings are extracted;
# listing files without translatable strings is harmless.
scan_source_files() {
    git ls-files \
        'src/*.c' 'src/**/*.c' \
        'src/*.h' 'src/**/*.h' \
        'resources/*.glade' 'resources/**/*.glade' \
        'files/*.desktop.in' \
        | sort
}

# Compare scanned files against current POTFILES.in
check_potfiles() {
    local scanned current stale missing
    scanned=$(scan_source_files)
    current=$(grep -v '^#' "$POTFILES_IN" | grep -v '^[[:space:]]*$')

    stale=$(comm -23 <(echo "$current" | sort) <(echo "$scanned"))
    missing=$(comm -13 <(echo "$current" | sort) <(echo "$scanned"))

    local rc=0

    if [ -n "$stale" ]; then
        echo "Stale entries in $POTFILES_IN (no longer git-tracked):"
        echo "$stale" | while IFS= read -r f; do echo "  - $f"; done
        rc=1
    fi

    if [ -n "$missing" ]; then
        echo "Missing from $POTFILES_IN (git-tracked but not listed):"
        echo "$missing" | while IFS= read -r f; do echo "  + $f"; done
        rc=1
    fi

    # Same set but different byte order means the list is unsorted.
    if [ "$rc" -eq 0 ] && [ "$current" != "$scanned" ]; then
        echo "Entries in $POTFILES_IN are not sorted."
        rc=1
    fi

    if [ "$rc" -eq 0 ]; then
        echo "$POTFILES_IN is up to date."
    fi

    return $rc
}

# Point core.hooksPath at the tree-carried hooks in scripts/githooks;
# preserve and report a foreign hooksPath instead of clobbering it.
install_git_hooks() {
    local hooks_path
    hooks_path=$(git config --get core.hooksPath || true)
    if [ -z "$hooks_path" ]; then
        git config core.hooksPath scripts/githooks
        echo "Installed git hooks: core.hooksPath=scripts/githooks"
    elif [ "$hooks_path" = "scripts/githooks" ]; then
        : # already installed
    else
        echo "Note: core.hooksPath=$hooks_path already set;" \
             "tree hooks in scripts/githooks not installed." >&2
    fi
    return 0
}

# Regenerate POTFILES.in from git ls-files
regenerate_potfiles() {
    {
        echo "# List of source files containing translatable strings."
        scan_source_files
    } > "$POTFILES_IN"
    echo "Regenerated $POTFILES_IN"
}

# Collapse the line number in every "#:" source-reference token to a fixed
# placeholder and blank the generated POT-Creation-Date, so two catalogs
# compare equal when they differ only by where a string now sits in the
# sources. Referenced filenames, fuzzy flags, and all msgid/msgstr content
# stay intact, so a genuine reference, flag, or translation change still shows.
normalize_po_refs() {
    sed -e '/^#:/ s/:[0-9][0-9]*/:LINE/g' \
        -e '/^"POT-Creation-Date:/ s/.*/"POT-Creation-Date: DATE"/' "$1"
}

# Restore any catalog whose sole working-tree change against the index is that
# reference-line and header-date churn. The index holds the pre-update state,
# since update-po writes the working tree only.
revert_line_only_catalogs() {
    local f count=0

    for f in $(git diff --name-only -- 'po/*.po' 'po/*.pot'); do
        if diff -u \
            <(git show ":$f" | normalize_po_refs /dev/stdin) \
            <(normalize_po_refs "$f") > /dev/null; then
            git checkout -- "$f"
            echo "  reverted (line-only): $f"
            count=$((count + 1))
        fi
    done

    echo "Reverted $count catalog(s) with only reference/date churn."
    return 0
}

# ---------------------------------------------------------------------------

# Reference-churn reversion needs git as the pre-update baseline; a catalog
# unpacked from a dist tarball carries no .git, so the mode is a no-op there.
# update-po must already have run; this mode performs no merge of its own.
if $revert_line_only; then
    if [ ! -d .git ]; then
        echo "No .git directory; skipping line-only reversion."
        exit 0
    fi
    revert_line_only_catalogs
    exit 0
fi

if $check_only; then
    check_potfiles
    exit $?
fi

# Sync mode rewrites the list only on drift so the file's mtime stays
# stable across clean builds and po/ sees no spurious rebuilds.
if $sync_only; then
    install_git_hooks
    if ! check_potfiles > /dev/null; then
        regenerate_potfiles
    fi
    exit 0
fi

install_git_hooks

if ! check_potfiles; then
    echo ""
    echo "Regenerating $POTFILES_IN..."
    regenerate_potfiles
    echo ""
fi

echo "Running make update-po..."
make update-po
echo ""

# Display translation statistics
"$(dirname "$0")/trans-stats.sh"
