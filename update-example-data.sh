#!/bin/sh

for i in examples/*.nec; do
	echo
	echo ===== $i =====
	output="examples/data/`basename $i .nec`.csv"
	if [ -s "$output" ]; then
		echo "$output already exists, skipping"
	else
		./src/xnec2c -j $(grep -c ^processor /proc/cpuinfo) --write-csv "$output" --batch $i
	fi
done

