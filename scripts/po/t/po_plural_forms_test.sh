#!/bin/sh
# Regression test: scripts/po/po-plural-check.sh passes a catalog whose
# Plural-Forms header holds the value the GNU gettext or Translate House plural
# table publishes for its language, and fails a header whose text differs,
# lacks the field, or belongs to a language no row in the check lists.
#
# Each case writes a header-only fixture catalog and compares the check's exit
# status, and the header line its fault names, with the case's expectation.

top="$(dirname "$0")/../../.."
check="$top/scripts/po/po-plural-check.sh"
work=$(mktemp -d "${PWD}/po_plural_forms_test.XXXXXX") || exit 99
trap 'rm -rf "$work"' EXIT

# Header lines at which write_catalog places msgid "" and the case's
# Plural-Forms line.
msgid_line=1
plural_forms_line=11

passed=0
failed=""

# Write fixture catalog $1 in language $2 whose header ends with line $3; an
# empty $3 ends the header without a Plural-Forms field.
write_catalog()
{
	printf '%s\n' \
		'msgid ""' \
		'msgstr ""' \
		'"Project-Id-Version: xnec2c\n"' \
		'"PO-Revision-Date: 2026-11-25 00:00+0000\n"' \
		'"Last-Translator: Fixture <fixture@example.invalid>\n"' \
		'"Language-Team: Fixture <fixture@example.invalid>\n"' \
		"\"Language: $2\\n\"" \
		'"MIME-Version: 1.0\n"' \
		'"Content-Type: text/plain; charset=UTF-8\n"' \
		'"Content-Transfer-Encoding: 8bit\n"' \
		"$3" > "$1"
	return $?
}

# Record case $1 as failed for reason $2, printing the check's output $3.
report_failure()
{
	echo "FAIL: $1: $2:"
	echo "$3"
	failed="$failed $1"
	return 0
}

# Run the check on case $1's fixture in language $2 with header line $3, and
# require exit status $4 and, for a failure, a fault naming header line $5.
run_case()
{
	fixture="$work/$1.po"
	write_catalog "$fixture" "$2" "$3" || exit 99
	out=$("$check" "$fixture" 2>&1)
	status=$?

	if [ "$status" -ne "$4" ]; then
		report_failure "$1" "po-plural-check.sh exited $status, expected $4" "$out"
	elif [ "$4" -eq 0 ]; then
		passed=$((passed + 1))
	else
		# The case expects a fault; require it to name header line $5.
		case $out in
		*"at line $5"[!0-9]*|*"at line $5")
			passed=$((passed + 1))
			;;
		*)
			report_failure "$1" "fault names no header line $5" "$out"
			;;
		esac
	fi

	return 0
}

# Headers holding the published text: a Translate House row, a GNU row wrapped
# across two continuation lines, a GNU row of one form, and a region code
# taking its language's row.
run_case mk_published mk '"Plural-Forms: nplurals=2; plural= n==1 || n%10==1 ? 0 : 1;\n"' 0
run_case lv_wrapped lv '"Plural-Forms: nplurals=3; plural=(n%10==1 && n%100!=11 ? 0 : n != 0 ? 1 : "
"2);\n"' 0
run_case vi_published vi '"Plural-Forms: nplurals=1; plural=0;\n"' 0
run_case zh_TW_language zh_TW '"Plural-Forms: nplurals=1; plural=0;\n"' 0

# Headers departing from the published text: the mk formula with added
# parentheses, Hebrew with a form for two, Vietnamese with two forms, Turkish
# with the Translate House value the GNU row sets aside, an absent field, and a
# language no row lists.
run_case mk_parenthesized mk '"Plural-Forms: nplurals=2; plural=(n==1 || n%10==1 ? 0 : 1);\n"' 1 "$plural_forms_line"
run_case he_three_forms he '"Plural-Forms: nplurals=3; plural=(n==1 ? 0 : n==2 ? 1 : 2);\n"' 1 "$plural_forms_line"
run_case vi_two_forms vi '"Plural-Forms: nplurals=2; plural=(n > 1);\n"' 1 "$plural_forms_line"
run_case tr_th_value tr '"Plural-Forms: nplurals=2; plural=(n > 1);\n"' 1 "$plural_forms_line"
run_case ky_absent ky '' 1 "$msgid_line"
run_case xx_unlisted xx '"Plural-Forms: nplurals=2; plural=(n != 1);\n"' 1 "$plural_forms_line"

if [ -n "$failed" ]; then
	echo "FAIL: po-plural-check.sh misjudged:$failed"
	exit 1
else
	echo "PASS: po-plural-check.sh judged $passed cases against the published tables"
	exit 0
fi
