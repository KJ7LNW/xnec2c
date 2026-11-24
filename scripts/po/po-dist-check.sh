#!/bin/bash
#
# Run make dist of a generated po/ makefile from a staged copy and fail when a
# make compiles a catalog both directly and through a make nested inside it.
# Parallel make runs those two compiles at once, and they collide on the
# catalog's temporary output.
#
# Usage: scripts/po/po-dist-check.sh <make> <po-makefile> <po-srcdir>
#
# The stage holds copies of the source po directory and the makefile's
# directory, so the run writes nothing outside it. A make wrapper numbers every
# make it starts and extends a chain of those numbers, and a msgfmt stub logs
# each compile under that chain and dates its output before every catalog, so
# each make reaching a catalog compiles it again and a serial run shows every
# pairing.
#
# Exit status: 0 no catalog compiles both directly and in a nested make; 1 a
# nested compile, a failed dist, or no compile logged; 2 a usage or staging
# error.

set -o pipefail

. "$(dirname "$0")/../pr-message.sh"

if [ $# -ne 3 ]; then
	echo "usage: ${0##*/} <make> <po-makefile> <po-srcdir>" >&2
	exit 2
fi

make_program=$1
po_makefile=$2
po_srcdir=$3

# Date every stale stage file at this POSIX touch time, older than any catalog;
# the second day of 1970 stays past the epoch in every time zone.
readonly stale_time=197001020000

# Copy each regular file of directory $1 into the staged po directory, leaving
# out the makefile, the stamp, and the compiled catalogs the stage writes for
# itself.
stage_inputs() {
	local dir=$1
	local file

	for file in "$dir"/*; do
		if [ ! -f "$file" ] || [[ ${file##*/} =~ ^(Makefile|stamp-poT?|t-.*|.*\.gmo)$ ]]; then
			continue
		fi
		cp --preserve=timestamps -- "$file" "$stage_po/" || return 1
	done
	return 0
}

# Lay out the stage: the inputs of source po directory $2, then the generated
# files beside makefile $1, a config.status and stamp-po older than every input
# so nothing regenerates the makefile and dist rebuilds the stamp, and the
# makefile copied last so it is the newest file.
stage_tree() {
	local po_makefile=$1
	local po_srcdir=$2

	mkdir "$stage_po" "$stage_dist" || return 1
	stage_inputs "$po_srcdir" || return 1
	stage_inputs "$(dirname "$po_makefile")" || return 1
	touch -t "$stale_time" "$stage_config_status" "$stage_po/stamp-po" || return 1
	cp -- "$po_makefile" "$stage_po/Makefile"
	return $?
}

# Write the make wrapper extending the chain of make numbers, the msgfmt stub
# logging each compile under that chain, and their empty logs.
write_tools() {
	cat > "$make_wrapper" <<'EOF' || return 1
#!/bin/sh
# Number this make by the makes started before it, append the number to the
# chain its recipes inherit, then run it. The serial run starts one make at a
# time, so each number is unique.
echo >> "$PO_DIST_CHECK_MAKES"
PO_DIST_CHECK_CHAIN="$PO_DIST_CHECK_CHAIN/$(wc --lines < "$PO_DIST_CHECK_MAKES")"
export PO_DIST_CHECK_CHAIN
exec "$PO_DIST_CHECK_MAKE" "$@"
EOF

	cat > "$msgfmt_stub" <<'EOF' || return 1
#!/bin/sh
# Log the compiled catalog under the chain of makes that reached it, then write
# the output dated before every catalog so each later make compiles it again.
# Options other than the output name take no part in the log.
command_line="$*"
output=
catalog=
while [ $# -gt 0 ]; do
	case $1 in
	-o) output=$2; shift ;;
	--output-file=*) output=${1#--output-file=} ;;
	-*) ;;
	*) catalog=$1 ;;
	esac
	shift
done
if [ -z "$output" ] || [ -z "$catalog" ]; then
	echo "msgfmt-log: err: compile command: no output or catalog ($command_line)" >&2
	exit 2
fi
printf '%s %s\n' "$PO_DIST_CHECK_CHAIN" "${catalog##*/}" >> "$PO_DIST_CHECK_LOG" &&
	: > "$output" &&
	touch -t "$PO_DIST_CHECK_STALE" "$output"
EOF

	chmod +x "$make_wrapper" "$msgfmt_stub" || return 1
	: > "$make_log" && : > "$compile_log"
	return $?
}

# Run make dist serially in the stage through make program $1, every make
# started through the wrapper and every compile through the stub, with the
# caller's make state cleared so no inherited job or variable reaches it.
run_dist() {
	local make_program=$1

	PO_DIST_CHECK_MAKE=$make_program PO_DIST_CHECK_MAKES=$make_log \
		PO_DIST_CHECK_LOG=$compile_log PO_DIST_CHECK_STALE=$stale_time \
		PO_DIST_CHECK_CHAIN= env --unset=MAKEFLAGS --unset=MFLAGS \
		--unset=MAKELEVEL "$make_wrapper" --no-print-directory --jobs=1 \
		--directory="$stage_po" dist distdir="$stage_dist" srcdir=. \
		MAKE="$make_wrapper" GMSGFMT="$msgfmt_stub" > "$make_output" 2>&1
	return $?
}

# Print each catalog compile log $1 records under two chains where one extends
# the other: a make compiling the catalog directly and through a make nested
# inside it. Sort in byte order so the report reads the same in every locale.
nested_catalogs() {
	local log=$1

	awk '
		{ n[$2]++; chain[$2, n[$2]] = $1 }
		END {
			for (catalog in n)
				for (i = 1; i <= n[catalog]; i++)
					for (j = 1; j <= n[catalog]; j++) {
						if (index(chain[catalog, j], chain[catalog, i] "/") != 1)
							continue
						nested[catalog] = 1
					}
			for (catalog in nested)
				print catalog
		}
	' "$log" | LC_ALL=C sort
	return $?
}

# Stage under the build tree; the EXIT trap removes the directory mktemp
# created, and the signal traps exit through it.
if ! scratch=$(mktemp --directory "$PWD/po-dist-check.XXXXXX"); then
	pr_err "$PWD" "stage directory not created"
	exit 2
fi
trap 'rm --recursive --force -- "$scratch"' EXIT
trap 'exit 130' INT
trap 'exit 143' TERM

# Place every stage file under the scratch directory; the staged makefile's
# top_builddir, "..", reaches the config.status beside the staged po directory.
readonly stage_po=$scratch/po
readonly stage_dist=$scratch/dist
readonly stage_config_status=$scratch/config.status
readonly make_wrapper=$scratch/make-chain
readonly msgfmt_stub=$scratch/msgfmt-log
readonly make_log=$scratch/makes.log
readonly compile_log=$scratch/compiles.log
readonly make_output=$scratch/make.out

if ! stage_tree "$po_makefile" "$po_srcdir" || ! write_tools; then
	pr_err "$po_makefile" "dist stage not written"
	exit 2
fi

if ! run_dist "$make_program"; then
	pr_err "$po_makefile" "make dist failed in the stage; its output follows"
	cat -- "$make_output" >&2
	exit 1
fi

if ! catalogs=$(awk '!seen[$2]++' "$compile_log" | wc --lines) ||
	! nested=$(nested_catalogs "$compile_log"); then
	pr_err "$po_makefile" "compile log not read"
	exit 2
fi

if [ "$catalogs" -eq 0 ]; then
	pr_err "$po_makefile" "make dist compiled no catalog through GMSGFMT"
	exit 1
fi

if [ -n "$nested" ]; then
	pr_err "$po_makefile" "$(printf '%s\n' "$nested" | wc --lines) of $catalogs catalogs compile both directly and in a nested make; parallel make dist runs both compiles at once"
	while IFS= read -r catalog; do
		pr_err "$po_srcdir/$catalog" "compiled directly and in a nested make"
	done <<< "$nested"
	rc=1
else
	pr_info "$po_makefile" "no catalog compiles both directly and in a nested make ($catalogs catalogs)"
	rc=0
fi
exit $rc
