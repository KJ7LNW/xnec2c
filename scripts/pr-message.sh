# Source this file to define pr_<severity> helpers, each printing one
# diagnostic line as program: severity: subject: state, the program named from
# the argv0 of the script sourcing it.
#
# Usage: . scripts/pr-message.sh

# Print one diagnostic line from severity $1, subject $2, and state $3.
pr_print() {
	local severity=$1
	local subject=$2
	local state=$3

	printf '%s: %s: %s: %s\n' "${0##*/}" "$severity" "$subject" "$state"
	return $?
}

# Print an error line for subject $1 in state $2 on stderr.
pr_err() {
	pr_print err "$1" "$2" >&2
	return $?
}

# Print an informational line for subject $1 in state $2 on stdout.
pr_info() {
	pr_print info "$1" "$2"
	return $?
}
