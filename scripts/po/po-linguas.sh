#!/bin/sh
#
# Print each language code a LINGUAS file registers, one per line, parsed as
# gettext's po.m4 parses it: strip every comment from # to line end, then split
# the remainder into whitespace-separated codes.
#
# Usage: scripts/po/po-linguas.sh <linguas-file>
#
# Exit status: 0 every code printed, a file registering none included;
# non-zero a usage error or an unreadable LINGUAS file.

if [ $# -ne 1 ]; then
	echo "usage: $0 <linguas-file>" >&2
	exit 2
fi

awk '{ sub(/#.*/, ""); for (i = 1; i <= NF; i++) print $i }' "$1"
