# Classify gettext catalog entries as fuzzy, untranslated, and/or identical.
#
# Reads a catalog (file argument or stdin) and emits one tab-separated record
# per flagged entry:
#
#   state <TAB> report_line <TAB> key
#
# state is the literal "fuzzy", "untranslated", or "identical". report_line is
# the 1-based catalog line reported for that state: the "#, fuzzy" flag line for
# a fuzzy record, the msgid line for an untranslated record, the msgstr line for
# an identical record. An entry carrying an "xnec2c-exempt:" translator comment
# holds a reviewed identity translation and emits no identical record. key is
# the entry identity used for delta set difference: msgctxt content, a U+0004
# separator, then the msgid content concatenated across continuation lines. The
# header entry (empty msgid with no msgctxt) is excluded.
#
# Owned by scripts/po/po-check.sh (absolute reporting) and scripts/po/po-delta.sh
# (base-vs-tip set difference); the single source of catalog state detection.

# Return the text between the first and last double quote on a catalog line.
function qcontent(s) {
	sub(/^[^"]*"/, "", s)
	sub(/"[^"]*$/, "", s)
	return s
}

# Emit records for the accumulated entry, then clear it for the next one.
function emit(   key) {
	if (have_msgid && !(msgid == "" && !have_msgctxt)) {
		key = msgctxt "\004" msgid
		if (fuzzy)
			print "fuzzy\t" fuzzy_line "\t" key
		if (have_msgstr && msgstr == "")
			print "untranslated\t" msgid_line "\t" key
		if (have_msgstr && msgstr != "" && msgstr == msgid && !exempt)
			print "identical\t" msgstr_line "\t" key
	}
	fuzzy = 0; fuzzy_line = 0
	exempt = 0
	have_msgctxt = 0; msgctxt = ""
	have_msgid = 0; msgid = ""; msgid_line = 0
	have_msgstr = 0; msgstr = ""; msgstr_line = 0
	sect = ""
}

# Blank line closes the current entry; entries are blank-separated.
/^[[:space:]]*$/ { emit(); next }

# Fuzzy flag; report this line for the entry's fuzzy state.
/^#, *fuzzy/ { fuzzy = 1; fuzzy_line = NR; next }

# Exemption marker; the entry records that its source form is its target form,
# so repeating the source is the reviewed outcome rather than a missing one.
/^#[ \t]*xnec2c-exempt:[ \t]*[a-z]+[ \t]*$/ { exempt = 1; next }

# Remaining comments, including obsolete "#~" entries, carry no state.
/^#/ { next }

/^msgctxt "/ { have_msgctxt = 1; msgctxt = qcontent($0); sect = "ctxt"; next }
/^msgid "/ { have_msgid = 1; msgid = qcontent($0); msgid_line = NR; sect = "id"; next }
/^msgid_plural "/ { sect = "plural"; next }
/^msgstr "/ { have_msgstr = 1; msgstr = qcontent($0); msgstr_line = NR; sect = "str"; next }

# Plural msgstr[N] mirrors gettext: no plain msgstr, so no untranslated state.
/^msgstr\[/ { sect = "strn"; next }

# Continuation line; append its content to the section in progress.
/^"/ {
	if (sect == "ctxt") msgctxt = msgctxt qcontent($0)
	else if (sect == "id") msgid = msgid qcontent($0)
	else if (sect == "str") msgstr = msgstr qcontent($0)
	next
}

END { emit() }
