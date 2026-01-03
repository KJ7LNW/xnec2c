#!/bin/sh

for i in examples/*.nec; do
	echo
	echo ===== $i =====
	output="examples/data/`basename $i .nec`.csv"
	if [ -s "$output" ]; then
		echo "$output already exists, skipping"
	else
		# there is a correctness bug edge case somewhere with SMP, so for data references, use -j1.
		# -j`nproc` is usually correct, but lets j1 for this stable test.
		./src/xnec2c -j 1 --write-csv "$output" --batch $i
	fi
done

