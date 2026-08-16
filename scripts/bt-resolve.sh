#!/bin/sh
#
# bt-resolve.sh - resolve backtrace hex addresses to file:line
#
# Reads xnec2c log output from stdin or file arguments.
# Detects binary path from backtrace lines and resolves [0xADDR]
# to source locations via addr2line.
#
# Usage:
#   bt-resolve.sh < logfile
#   bt-resolve.sh logfile [logfile ...]

BINARY=""

resolve()
{
	addr="$1"
	if [ -z "$BINARY" ]; then
		echo "$addr"
		return
	fi
	line=$(addr2line -e "$BINARY" -f -p "$addr" 2>/dev/null)
	if [ -n "$line" ] && ! echo "$line" | grep -q '??'; then
		echo "$line"
	else
		echo "$addr"
	fi
}

process_line()
{
	line="$1"

	# Extract binary path from backtrace lines on first encounter
	if [ -z "$BINARY" ]; then
		candidate=$(echo "$line" | grep -oP '/[^\s(]+/xnec2c(?=[\(])' | head -1)
		if [ -n "$candidate" ] && [ -x "$candidate" ]; then
			BINARY="$candidate"
		fi
	fi

	# Match backtrace frame lines: N. /path/binary(sym+0xoff) [0xADDR]
	addr=$(echo "$line" | grep -oP '\[0x[0-9a-fA-F]+\]$' | tr -d '[]')
	if [ -n "$addr" ]; then
		prefix=$(echo "$line" | sed 's/\[0x[0-9a-fA-F]*\]$//')
		resolved=$(resolve "$addr")
		echo "${prefix}${resolved}"
	else
		echo "$line"
	fi
}

if [ $# -eq 0 ]; then
	while IFS= read -r line; do
		process_line "$line"
	done
else
	for f in "$@"; do
		while IFS= read -r line; do
			process_line "$line"
		done < "$f"
	done
fi
