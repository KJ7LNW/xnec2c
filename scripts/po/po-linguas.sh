#!/bin/sh
#
# Print the language entries a LINGUAS file registers, one line per entry,
# omitting comment lines and blank lines.
#
# Usage: scripts/po/po-linguas.sh <linguas-file>
#
# Exit status: 0 an entry printed, 1 no entry printed, 2 usage error.

if [ $# -ne 1 ]; then
	echo "usage: $0 <linguas-file>" >&2
	exit 2
fi

grep -v '^[[:space:]]*#' "$1" | grep -v '^[[:space:]]*$'
