# Print the failure notice sections the git hooks share: the failed headline,
# explanation lines, the fixing commands, and the bypass command. Source this
# file once from the top of the tree, naming the git verb the hook gates and
# the variable forcing that verb past every gate:
#
#   . scripts/githooks/hook-notice.sh commit FORCE_COMMIT

if [ $# -ne 2 ]; then
	echo "usage: . ${BASH_SOURCE[0]} <git-verb> <bypass-variable>" >&2
	return 2
fi

readonly hook_notice_verb=$1
readonly hook_notice_bypass_var=$2

# Print each argument as one line, then the blank separator line.
hook_notice_print_section() {
	printf '%s\n' "$@" ""
	return $?
}

# Print the failed headline, naming the failed subject when one is passed.
hook_notice_print_failed() {
	if [ $# -gt 1 ]; then
		echo "usage: hook_notice_print_failed [<subject>]" >&2
		return 2
	fi

	local headline="❌ Pre-$hook_notice_verb validation failed"

	if [ $# -eq 0 ]; then
		hook_notice_print_section "" "$headline"
	else
		# Append the one subject the guard above admits.
		hook_notice_print_section "" "$headline: $1"
	fi
	return $?
}

# Print the commands fixing the failure, each indented under a "Fix with:" lead.
hook_notice_print_fix() {
	hook_notice_print_section "Fix with:"
	hook_notice_print_section "${@/#/  }"
	return $?
}

# Print the command forcing the gated git verb past every gate.
hook_notice_print_bypass() {
	hook_notice_print_section "If you have determined this $hook_notice_verb must proceed anyway:"
	hook_notice_print_section "  $hook_notice_bypass_var=true git $hook_notice_verb"
	return $?
}
