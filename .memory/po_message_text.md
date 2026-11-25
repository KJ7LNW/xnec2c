# Message text in C source

Apply to authoring strings in C; exclude catalog and msgstr editing.

## User-given

### extraction

- Treat every extracted string as a translated interface: `po/Makevars` extracts `_`, `N_`, `Notice:2`, `Notice:3`, `Stop:2`, `pr_*:1`; `BUG` and `BUG_ON` messages stay unextracted
- Flag each printf-style message function `c-format` in `po/Makevars` at the format argument its `format(printf, N, M)` attribute names; a new one adds its `--flag` line
- Mark with `_()` each string a user reads: widget text, tooltips, window titles, dialog titles and bodies, `Notice` and `Stop` arguments; a glade `label`, `tooltip_text`, or `title` property takes `translatable="yes"`
- Mark a static table's display text `N_()` in its row and `_()` at the site that shows it
- Leave bare: config keys, file tokens, option ids, `BUG()` and `BUG_ON()` text, assertion text, product and library names (eg OpenGL, Cairo, math libraries); a string whose descriptive words surround a call sign or standard designation stays marked, since `doc/TRANSLATING.md` retains the designation

### msgid stability

- Move code between files, split modules, and renumber lines freely; hold msgid bytes identical across every refactor
- Reword only when the meaning a translator conveys changes; accept retranslation for that change alone
- Let `BUG()` and `pr_*` supply file, function, and line; omit all three from message text
- Exclude from message text every name a rename can change: function, variable, type, macro
- Repair an unintended msgid change by restoring the source string 
