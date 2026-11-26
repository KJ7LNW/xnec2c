#!/bin/sh
# Defect test: po-stats.sh reports an entry carrying an xnec2c-exempt marker
# apart from the untranslated count.
#
# po-stats.sh resolves its catalogs relative to its own location, so the
# production scripts are staged into a scratch tree beside a one-language
# LINGUAS file and the fixture catalog. The fixture holds a translated entry,
# an untranslated entry, an exempt entry, a filled fuzzy entry, an empty fuzzy
# entry, and an empty fuzzy exempt entry. msgfmt counts every empty msgstr as
# untranslated, so the exempt pair must leave the untranslated count and
# report as exempt.

top="$(dirname "$0")/../../.."
work=$(mktemp -d "${PWD}/po_stats_exempt.XXXXXX") || exit 99
trap 'rm -rf "$work"' EXIT

mkdir -p "$work/scripts/po" "$work/po" || exit 99
for script in po-stats.sh po-classify.awk po-linguas.sh; do
	cp "$top/scripts/po/$script" "$work/scripts/po/$script" || exit 99
done
cp "$top/scripts/po/t/fixtures/po_stats_exempt.po" "$work/po/xx.po" || exit 99
echo xx > "$work/po/LINGUAS" || exit 99

if ! out=$("$work/scripts/po/po-stats.sh" -v 2>&1); then
	echo "FAIL: po-stats.sh exited non-zero:"
	echo "$out"
	exit 1
fi

case "$out" in
	*"  1 translated,   1 fuzzy,   2 untranslated,   2 exempt)"*) ;;
	*)
		echo "FAIL: language line lacks 1 translated, 1 fuzzy, 2 untranslated, 2 exempt:"
		echo "$out"
		exit 1
		;;
esac

case "$out" in
	*"Total untranslated messages: 2"*"Total exempt messages: 2"*)
		echo "PASS: exempt entries report apart from untranslated ones"
		exit 0
		;;
	*)
		echo "FAIL: summary lacks 2 untranslated and 2 exempt totals:"
		echo "$out"
		exit 1
		;;
esac
