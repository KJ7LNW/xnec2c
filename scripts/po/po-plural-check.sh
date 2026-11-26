#!/bin/sh
#
# Check that one .po catalog's Plural-Forms header holds the value a published
# gettext plural table lists for its language, copied verbatim.
#
# Usage: scripts/po/po-plural-check.sh <po-file>
#
# The GNU gettext table, gettext-tools/src/plural-table.c, publishes a
# language's value first; the Translate House localization guide's Plural
# forms table publishes each language the GNU table lists none for. A catalog
# code holding a region, eg zh_TW, takes its own row, else its language's row.
# The language is the header's Language field. The header's Plural-Forms
# value, joined across its continuation lines, must equal the published text.
# Each fault names the header's Plural-Forms line, or its msgid line when that
# field is absent, so po-translate.pl attributes it to the header, which no
# output-map record owns.
#
# Exit status: 0 the header holds the published value, 1 it differs, lacks a
# Language or Plural-Forms value, or names a language no row below lists, 2
# usage error.

po="$1"

if [ -z "$po" ]; then
	echo "usage: $0 <po-file>" >&2
	exit 2
fi

if [ ! -f "$po" ]; then
	echo "$po: no such file" >&2
	exit 2
fi

# Print header field $2 of catalog $1. The header entry's msgstr strings are
# joined, then split at each \n escape, so a value wrapped across continuation
# lines reads whole.
header_field()
{
	sed -n '1,/^$/{/^"/{s/^"//;s/"$//;p;}}' "$1" | tr -d '\n' \
		| sed 's/\\n/\n/g' | sed -n "s/^$2: //p"
	return $?
}

# Set published_table and published_value to the table publishing language
# $1's Plural-Forms value and that value, copied verbatim from the table; return
# 1 when no row lists the language. The zh row is the one whose footnote
# extends it to zh_CN, zh_HK, and zh_TW; a second zh row's footnote confines it
# to rare cases. The mk row copies the formula its cell holds, leaving out the
# emphasized remark that follows it in the cell.
published_plural_forms()
{
	gnu='GNU gettext plural-table.c'
	th='Translate House Plural forms table'

	case $1 in
	bg|da|de|el|es|et|fi|he|hu|it|nl|no|pt|sv|tr)
		published_table=$gnu
		published_value='nplurals=2; plural=(n != 1);'
		;;
	cs|sk)
		published_table=$gnu
		published_value='nplurals=3; plural=(n==1) ? 0 : (n>=2 && n<=4) ? 1 : 2;'
		;;
	fr)
		published_table=$gnu
		published_value='nplurals=2; plural=(n > 1);'
		;;
	hr|ru|sr|uk)
		published_table=$gnu
		published_value='nplurals=3; plural=(n%10==1 && n%100!=11 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2);'
		;;
	ja|ko|vi)
		published_table=$gnu
		published_value='nplurals=1; plural=0;'
		;;
	lt)
		published_table=$gnu
		published_value='nplurals=3; plural=(n%10==1 && n%100!=11 ? 0 : n%10>=2 && (n%100<10 || n%100>=20) ? 1 : 2);'
		;;
	lv)
		published_table=$gnu
		published_value='nplurals=3; plural=(n%10==1 && n%100!=11 ? 0 : n != 0 ? 1 : 2);'
		;;
	pl)
		published_table=$gnu
		published_value='nplurals=3; plural=(n==1 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2);'
		;;
	ro)
		published_table=$gnu
		published_value='nplurals=3; plural=n==1 ? 0 : (n==0 || (n%100 > 0 && n%100 < 20)) ? 1 : 2;'
		;;
	sl)
		published_table=$gnu
		published_value='nplurals=4; plural=(n%100==1 ? 0 : n%100==2 ? 1 : n%100==3 || n%100==4 ? 2 : 3);'
		;;
	af|bn|hi|ky|ps|ta|ur)
		published_table=$th
		published_value='nplurals=2; plural=(n != 1);'
		;;
	ar)
		published_table=$th
		published_value='nplurals=6; plural=(n==0 ? 0 : n==1 ? 1 : n==2 ? 2 : n%100>=3 && n%100<=10 ? 3 : n%100>=11 ? 4 : 5);'
		;;
	bs)
		published_table=$th
		published_value='nplurals=3; plural=(n%10==1 && n%100!=11 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2);'
		;;
	fa|uz)
		published_table=$th
		published_value='nplurals=2; plural=(n > 1);'
		;;
	id|ms|th|zh)
		published_table=$th
		published_value='nplurals=1; plural=0;'
		;;
	mk)
		published_table=$th
		published_value='nplurals=2; plural= n==1 || n%10==1 ? 0 : 1;'
		;;
	*)
		# No row copies a published value for this language.
		return 1
		;;
	esac

	return 0
}

plural_line=$(sed -n '1,/^$/{/^"Plural-Forms: /=}' "$po")
if [ -n "$plural_line" ]; then
	# Locate faults at the field they concern.
	header_line=$plural_line
else
	# The header lacks the field; locate faults at the header entry itself.
	header_line=$(sed -n '1,/^$/{/^msgid ""$/=}' "$po")
fi

lang=$(header_field "$po" Language)
if [ -z "$lang" ]; then
	echo "$po: FAIL header entry holds no Language value at line $header_line"
	exit 1
fi

if ! published_plural_forms "$lang" && ! published_plural_forms "${lang%%_*}"; then
	echo "$po: FAIL language $lang at line $header_line has no published Plural-Forms row in $0"
	exit 1
fi

header_value=$(header_field "$po" Plural-Forms)
if [ -z "$header_value" ]; then
	echo "$po: FAIL header entry holds no Plural-Forms value at line $header_line; $published_table publishes '$published_value'"
	status=1
elif [ "$header_value" != "$published_value" ]; then
	echo "$po: FAIL Plural-Forms at line $header_line: header '$header_value' differs from '$published_value' published by $published_table"
	status=1
else
	# The header holds the published text.
	status=0
fi

exit $status
