#!/bin/bash
#
# Synchronize po/POTFILES.in with git-tracked source files and
# regenerate translation catalogs.
#
# Usage:
#   ./scripts/po/po-update.sh          # regenerate POTFILES.in, make po-refresh, show stats
#   ./scripts/po/po-update.sh --check  # report differences without modifying files
#   ./scripts/po/po-update.sh --sync   # regenerate POTFILES.in only when it drifted
#

set -euo pipefail

cd "$(dirname "$0")/../.."

POTFILES_IN="po/POTFILES.in"

check_only=false
sync_only=false
case "${1:-}" in
    --check) check_only=true ;;
    --sync)  sync_only=true ;;
    "")      ;;
    *)
        echo "Usage: $(basename "$0") [--check|--sync]"
        echo ""
        echo "  --check             Report differences without modifying files"
        echo "  --sync              Regenerate POTFILES.in only when it drifted; skip po-refresh"
        echo "  (none)              Regenerate POTFILES.in, run make po-refresh, show stats"
        exit 1
        ;;
esac

# All git-tracked C source, header, glade, and desktop entry files.
# xgettext keywords in po/Makevars control which strings are extracted;
# listing files without translatable strings is harmless.
# git ls-files already emits byte order, so this holds that order rather than
# imposing another.
sort_bytes() {
    LC_ALL=C sort
    return $?
}

scan_source_files() {
    git ls-files \
        'src/*.c' 'src/**/*.c' \
        'src/*.h' 'src/**/*.h' \
        'resources/*.glade' 'resources/**/*.glade' \
        'files/*.desktop.in' \
        | sort_bytes
    return $?
}

# Compare scanned files against current POTFILES.in
check_potfiles() {
    local scanned current stale missing
    scanned=$(scan_source_files)
    current=$(grep -v '^#' "$POTFILES_IN" | grep -v '^[[:space:]]*$')

    # comm requires both sides in one collation; scan_source_files supplies the other.
    stale=$(comm -23 <(echo "$current" | sort_bytes) <(echo "$scanned"))
    missing=$(comm -13 <(echo "$current" | sort_bytes) <(echo "$scanned"))

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

# ---------------------------------------------------------------------------

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

echo "Running make po-refresh..."
make po-refresh
echo ""

# Display translation statistics
"$(dirname "$0")/po-stats.sh"
