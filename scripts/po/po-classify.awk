# Classify gettext catalog entries as fuzzy, untranslated, and/or identical.
#
# Reads a catalog (file argument or stdin) and emits one tab-separated record
# per flagged entry:
#
#   state <TAB> report_line <TAB> key
#
# state is the literal "fuzzy", "untranslated", "exempt", or "identical".
# report_line is the 1-based catalog line reported for that state: the
# flags line naming fuzzy for a fuzzy record, the msgid line for an untranslated or
# exempt record, the msgstr line, or a plural entry's first msgstr[N] line, for
# an identical record. An entry carrying a sole "xnec2c-exempt:" translator comment
# records that this catalog writes no translation, so its empty msgstr, or a
# plural entry whose every form is empty, emits an exempt record in place of an
# untranslated record and a repeated msgid stays identical. A plural entry is
# untranslated when any form is empty, and identical when msgstr[0] repeats its
# msgid and every later form its msgid_plural, the English pair in English
# order. key is
# the entry identity used for delta set difference: msgctxt content, a U+0004
# separator, then the msgid content concatenated across continuation lines. The
# header entry (empty msgid with no msgctxt) is excluded.
#
# Owned by scripts/po/po-check.sh (absolute reporting), scripts/po/po-delta.sh
# (base-vs-tip set difference), scripts/po/po-stats.sh (per-language counts),
# and Xnec2c::PO::CatalogState (output-map work selection); each reads catalog
# entry state from this classifier alone.

# Return the text between the first and last double quote on a catalog line.
function qcontent(s) {
	sub(/^[^"]*"/, "", s)
	sub(/"[^"]*$/, "", s)
	return s
}

# Emit records for the accumulated entry, then clear it for the next one.
function emit(   key, i, have_translation, missing, blank, copied, exempted, translation_line) {
	if (have_msgid && !(msgid == "" && !have_msgctxt)) {
		key = msgctxt "\004" msgid
		if (have_plural) {
			# Judge the forms together: missing when any is empty, blank when
			# all are, copied when they repeat msgid then msgid_plural.
			have_translation = (nforms > 0)
			translation_line = form_line
			missing = 0; blank = 1; copied = (forms[0] == msgid)
			for (i = 0; i < nforms; i++) {
				if (forms[i] == "") missing = 1
				else blank = 0
				if (i > 0 && forms[i] != msgid_plural) copied = 0
			}
			copied = (copied && !missing)
		} else {
			# One msgstr: missing and blank coincide.
			have_translation = have_msgstr
			translation_line = msgstr_line
			missing = blank = (msgstr == "")
			copied = (!missing && msgstr == msgid)
		}

		# A sole exemption marker records an all-empty translation as exempt.
		exempted = (blank && (exempt_markers == 1))
		if (fuzzy)
			print "fuzzy\t" fuzzy_line "\t" key
		if (have_translation && missing && !exempted)
			print "untranslated\t" msgid_line "\t" key
		if (have_translation && exempted)
			print "exempt\t" msgid_line "\t" key
		if (have_translation && copied)
			print "identical\t" translation_line "\t" key
	}
	fuzzy = 0; fuzzy_line = 0
	exempt_markers = 0
	have_msgctxt = 0; msgctxt = ""
	have_msgid = 0; msgid = ""; msgid_line = 0
	have_msgstr = 0; msgstr = ""; msgstr_line = 0
	have_plural = 0; msgid_plural = ""; nforms = 0; form_line = 0
	sect = ""
}

# Blank line closes the current entry; entries are blank-separated.
/^[[:space:]]*$/ { emit(); next }

# Flags line naming fuzzy at any position; report this line for the entry's
# fuzzy state.
/^#,([^,]*,)*[ \t]*fuzzy[ \t]*(,|$)/ { fuzzy = 1; fuzzy_line = NR; next }

# Exemption marker; a sole marker records that this catalog writes no
# translation, so its empty msgstr is the recorded outcome rather than a missing
# one, while several markers record no single state.
/^#[ \t]*xnec2c-exempt:[ \t]*[a-z]+[ \t]*$/ { exempt_markers++; next }

# Remaining comments, including obsolete "#~" entries, carry no state.
/^#/ { next }

/^msgctxt "/ { have_msgctxt = 1; msgctxt = qcontent($0); sect = "ctxt"; next }
/^msgid "/ { have_msgid = 1; msgid = qcontent($0); msgid_line = NR; sect = "id"; next }
/^msgid_plural "/ { have_plural = 1; msgid_plural = qcontent($0); sect = "plural"; next }
/^msgstr "/ { have_msgstr = 1; msgstr = qcontent($0); msgstr_line = NR; sect = "str"; next }

# Plural form msgstr[N]; collect each form's text in order and the first
# form's line, which an identical record reports.
/^msgstr\[/ {
	if (nforms == 0) form_line = NR
	forms[nforms++] = qcontent($0)
	sect = "strn"
	next
}

# Continuation line; append its content to the section in progress.
/^"/ {
	if (sect == "ctxt") msgctxt = msgctxt qcontent($0)
	else if (sect == "id") msgid = msgid qcontent($0)
	else if (sect == "str") msgstr = msgstr qcontent($0)
	else if (sect == "plural") msgid_plural = msgid_plural qcontent($0)
	else if (sect == "strn") forms[nforms - 1] = forms[nforms - 1] qcontent($0)
	next
}

END { emit() }
