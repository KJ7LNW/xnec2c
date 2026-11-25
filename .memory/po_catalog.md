# PO catalog tooling

Apply each line when changing xgettext flags or keywords, refreshing catalogs, or running format gates.

## User-given

- Leave newly marked msgids untranslated for the single translation pass.

## Model-derived

- Write doubles as `%f` in extracted format strings.
- Register a message function whose callers wrap every argument in `_()` through `--flag` alone.
- Prove a catalog change by a passing format check, an unchanged `c-format` count, and intact msgstrs.
- Extract templates with their charset header.
- Compare template statistics only against a template holding the theme descriptions.
- One classifier owns po entry state classification, `exempt` included; the po scripts filter its records by state.
- Refresh catalogs through `make -s po-refresh`.
