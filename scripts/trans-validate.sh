#!/bin/sh

export MAX_THINKING_TOKENS=31999

for l in "$@"; do
	echo
	echo
	echo
	echo ================================== $l ===============================
	scripts/trans-validate-1.sh "$l"
	stty echo cooked
	reset
	sleep 3
done
