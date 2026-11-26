# Translating xnec2c

xnec2c is an electromagnetic simulation program for antenna and radio-frequency design, built on the NEC2 method-of-moments engine, for electrical and RF engineers, antenna designers, and amateur-radio operators; write every string as engineering language, never general prose.

Treat this file as holding only the considerations shared by every language; `po/rules/<lang>.md` decides every wording question for one language: script and orthography, numerals, punctuation, retained tokens, format-specifier handling, capitalization, interface register, formality and address, accelerator mnemonics, the locked domain lexicon, disambiguation, grammatical agreement, whitespace fidelity, and developer-string policy. A rule file governs a conflict with this file, except under Retained tokens.

- Translate each source string as written; never correct, shorten, or reformat it.

## Retained tokens

Treat each token below as source text in every language, beside the tokens a rule file lists: copy it exactly, in spelling, case, punctuation, digits, and edition year; translate only the words around it. This section governs over a rule file.

- Call signs: any station call sign, eg `G4CQM`
- Standard designations and edition years: eg `ITU-R`, `P.372`, `(2022)`; translate a qualifier beside one, eg `pre` in `(pre-2016)`
- Tokens a control shows untranslated, or a file format or measurement key stores: eg `avg`, `gain_max`
- Markup tags and formulas: keep every byte; translate the text between them

eg `Galactic P.372 (2022)`: translate `Galactic`; keep `P.372 (2022)`.
eg `<b>avg:</b> Average penalty across band`: keep `<b>avg:</b>`; translate `Average penalty across band`.

## Plural forms

- Take the plural count and the selection expression from the Plural forms section the translation request supplies.
- The declared count binds every plural entry in the catalog; a wrong count invalidates the whole file.

## Terminology sources

- Derive a term absent from the language's locked lexicon from the IEEE Standard Dictionary of Electrical and Electronics Terms, the ITU Radio Regulations, the ARRL Handbook, or the national electrical-engineering standard for that language.
- Record the decision in the rule file's lexicon before use, so the term stays fixed for every later string.

## Translation priority

Order work by how directly a string reaches a user:

1. Menu items, button labels, window titles, and dialog messages
2. Status messages, confirmations, and file-type descriptions
3. Diagnostics, NEC2 geometry errors, and developer messages

## Contact

Send translation questions and terminology corrections to xnec2c-bugs@z.ewheeler.org.
