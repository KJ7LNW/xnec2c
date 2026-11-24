# Source this file to stage a scripts/po make check in a scratch directory
# under the build tree and to run make there with the calling make's state
# cleared.
#
# Usage: . scripts/po/po-stage.sh

. "$(dirname "${BASH_SOURCE[0]}")/../pr-message.sh"

# Create a scratch directory named for stem $1 under the build tree and store
# its path in scratch; the EXIT trap removes the directory mktemp created, and
# the signal traps exit through it. Return 1 after reporting a stage directory
# mktemp could not create.
create_scratch_stage() {
	local stem=$1

	if ! scratch=$(mktemp --directory "$PWD/$stem.XXXXXX"); then
		pr_err "$PWD" "stage directory not created"
		return 1
	fi
	trap 'rm --recursive --force -- "$scratch"' EXIT
	trap 'exit 130' INT
	trap 'exit 143' TERM
	return 0
}

# Run command $@ with the make state a parent make exports cleared, so no job
# server, flag, or recursion level of the make running the check reaches it.
run_without_make_state() {
	env --unset=MAKEFLAGS --unset=MFLAGS --unset=MAKELEVEL "$@"
	return $?
}
