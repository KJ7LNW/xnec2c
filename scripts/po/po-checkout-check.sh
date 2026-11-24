#!/bin/bash
#
# Run the git-checkout gates of a generated top-level makefile, potfiles-sync
# and check-po-clean, in staged layouts and fail unless both run in every git
# checkout, whose .git is a directory or the gitfile of a linked worktree or a
# submodule, and both stand down in a tree holding no .git.
#
# Usage: scripts/po/po-checkout-check.sh <make> <makefile> <current-targets>
#
# Each layout holds a po-update.sh stub recording that potfiles-sync reached
# the sync, and an untracked catalog check-po-clean reports as po/ residue.
# Make runs from each layout with srcdir and top_srcdir naming it and with the
# targets <current-targets> lists, check-po-clean's prerequisite gates, taken
# as current, so only the two gates run. Git runs apart from the caller's repository and configuration, since a
# git hook exports the repository it runs in.
#
# Exit status: 0 every layout meets its expectation; 1 a gate runs or stands
# down against its layout; 2 a usage or staging error.

set -o pipefail

. "$(dirname "$0")/../pr-message.sh"

if [ $# -ne 3 ]; then
	echo "usage: ${0##*/} <make> <makefile> <current-targets>" >&2
	exit 2
fi

make_program=$1

# Take each target $3 lists as current, so make runs no prerequisite of the
# gate a layout names.
read -r -a current_targets <<< "$3"
old_file_options=()
for target in "${current_targets[@]}"; do
	old_file_options+=("--old-file=$target")
done

# Name each layout with the outcome potfiles-sync and check-po-clean reach
# there: a git checkout syncs and fails on its po/ residue; a tree holding no
# .git, as an unpacked tarball, skips both.
readonly layout_rows=(
	'directory ran failed'
	'worktree ran failed'
	'separate ran failed'
	'tarball skipped passed'
)

# Name the file the po-update.sh stub creates in the directory make runs from
# when potfiles-sync reaches the sync.
readonly sync_marker=po-update.ran

# Create layout tree $1 as a checkout whose .git is a directory.
create_layout_directory() {
	git init "$1"
	return $?
}

# Create layout tree $1 as a linked worktree, whose .git is a gitfile, of a
# repository created beside it. The worktree records an empty commit and
# checks out nothing, so no checkout hook runs.
create_layout_worktree() {
	local tree=$1
	local repository=$tree.repository
	local empty_tree commit

	git init "$repository" || return 1
	empty_tree=$(git -C "$repository" mktree < /dev/null) || return 1
	commit=$(git -C "$repository" -c user.name=po-checkout-check \
		-c user.email=po-checkout-check@localhost.invalid \
		commit-tree -m 'checkout layout' "$empty_tree") || return 1
	git -C "$repository" worktree add --detach --no-checkout "$tree" "$commit"
	return $?
}

# Create layout tree $1 as a checkout whose .git is a gitfile naming a
# repository outside it, the layout a submodule carries.
create_layout_separate() {
	git init --separate-git-dir="$1.git" "$1"
	return $?
}

# Create layout tree $1 holding no .git, standing for an unpacked tarball.
create_layout_tarball() {
	mkdir -- "$1"
	return $?
}

# Write into layout tree $1 the po-update.sh stub, which records a sync by
# creating the sync marker in the directory make runs from, and the untracked
# catalog check-po-clean reports as po/ residue.
stage_layout() {
	local tree=$1

	mkdir --parents -- "$tree/po" "$tree/scripts/po" || return 1
	printf '#!/bin/sh\n: > %s\n' "$sync_marker" > "$tree/scripts/po/po-update.sh" || return 1
	chmod +x -- "$tree/scripts/po/po-update.sh" || return 1
	printf 'msgid ""\nmsgstr ""\n' > "$tree/po/residue.po"
	return $?
}

# Run gate $2 of the makefile from layout tree $1 with srcdir and top_srcdir
# naming the layout, check-po-clean's prerequisite gates taken as current, and
# the caller's make state cleared, printing make's output on stdout.
run_gate() {
	local tree=$1
	local gate=$2

	env --unset=MAKEFLAGS --unset=MFLAGS --unset=MAKELEVEL "$make_program" \
		--no-print-directory --directory="$tree" --makefile="$makefile" \
		"${old_file_options[@]}" srcdir=. top_srcdir=. "$gate" 2>&1
	return $?
}

# Print captured command output $1 on stderr; empty output prints nothing.
print_captured_output() {
	local output=$1

	if [ -z "$output" ]; then
		return 0
	fi
	printf '%s\n' "$output" >&2
	return $?
}

# Report gate $2 of layout tree $1 when its observed outcome $4 differs from
# its expected outcome $3, followed by make's output $5 for that run.
report_gate() {
	local tree=$1
	local gate=$2
	local expected=$3
	local observed=$4
	local output=$5

	if [ "$observed" = "$expected" ]; then
		return 0
	fi
	pr_err "${tree##*/}" "$gate $observed (expected $expected)"
	print_captured_output "$output"
	return 1
}

# Run both gates in staged layout tree $1 and report each whose outcome
# differs from expected potfiles-sync outcome $2 or check-po-clean outcome $3.
check_layout() {
	local tree=$1
	local sync_expected=$2
	local clean_expected=$3
	local sync_output clean_output sync_observed clean_observed
	local rc=0

	if ! sync_output=$(run_gate "$tree" potfiles-sync); then
		sync_observed=failed
	elif [ -e "$tree/$sync_marker" ]; then
		sync_observed=ran
	else
		# The recipe exited 0 without reaching the stub, so its guard held.
		sync_observed=skipped
	fi

	if clean_output=$(run_gate "$tree" check-po-clean); then
		clean_observed=passed
	else
		clean_observed=failed
	fi

	report_gate "$tree" potfiles-sync "$sync_expected" "$sync_observed" "$sync_output" || rc=1
	report_gate "$tree" check-po-clean "$clean_expected" "$clean_observed" "$clean_output" || rc=1
	return $rc
}

if ! makefile=$(realpath --canonicalize-existing -- "$2"); then
	pr_err "$2" "makefile not found"
	exit 2
fi

# Clear the repository variables a git hook exports, so every git command and
# check-po-clean's status read the staged layout, and read no global or system
# configuration, which can omit untracked files from git status.
if ! git_local_env=$(git rev-parse --local-env-vars); then
	pr_err git "repository variables not listed"
	exit 2
fi
mapfile -t git_local_vars <<< "$git_local_env"
unset "${git_local_vars[@]}"
export GIT_CONFIG_GLOBAL=/dev/null GIT_CONFIG_NOSYSTEM=1

# Stage under the build tree; the EXIT trap removes the directory mktemp
# created, and the signal traps exit through it.
if ! scratch=$(mktemp --directory "$PWD/po-checkout-check.XXXXXX"); then
	pr_err "$PWD" "stage directory not created"
	exit 2
fi
trap 'rm --recursive --force -- "$scratch"' EXIT
trap 'exit 130' INT
trap 'exit 143' TERM

failed=0
for row in "${layout_rows[@]}"; do
	read -r name sync_expected clean_expected <<< "$row"
	tree=$scratch/$name
	if ! layout_output=$({ "create_layout_$name" "$tree" && stage_layout "$tree"; } 2>&1); then
		pr_err "$name" "layout not staged; its output follows"
		print_captured_output "$layout_output"
		exit 2
	fi
	check_layout "$tree" "$sync_expected" "$clean_expected" || failed=$((failed + 1))
done

if [ "$failed" -ne 0 ]; then
	pr_err "$makefile" "a git-checkout gate misreads .git in $failed of ${#layout_rows[@]} layouts"
	rc=1
else
	pr_info "$makefile" "both git-checkout gates run in each git checkout and stand down without .git (${#layout_rows[@]} layouts)"
	rc=0
fi
exit $rc
