#!/bin/sh

# run xnec2c with a specific language

if [ -z "$1" ]; then
	echo "usage: $0 <language> [xnec2c args]"
	exit 1
fi

lang=$1
shift

XNEC2C_LOCALEDIR=./locale LANGUAGE=$lang ./src/xnec2c "$@"
