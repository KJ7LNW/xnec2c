# Translating xnec2c

xnec2c is an electromagnetic simulation program for antenna and radio-frequency
design, built on the NEC2 method-of-moments engine. Its interface addresses
electrical and RF engineers, antenna designers, and amateur-radio operators, so
every string reads as engineering language rather than general prose.

This guide holds only the considerations shared by every language.
`po/rules/<lang>.md` decides every wording question for one language: script
and orthography, numerals, punctuation, retained tokens, format-specifier
handling, capitalization, interface register, formality and address,
accelerator mnemonics, the locked domain lexicon, disambiguation, grammatical
agreement, whitespace fidelity, and developer-string policy. Where this guide
and a rule file disagree, the rule file governs.

A source string is fixed. Translate it as written; never correct, shorten, or
reformat one.

## Translation priority

Order the work by how directly the string reaches a user:

1. Menu items, button labels, window titles, and dialog messages.
2. Status messages, confirmations, and file-type descriptions.
3. Diagnostics, NEC2 geometry errors, and developer messages.

## Plural forms

The catalog header declares how many plural forms the language has and the
expression selecting among them. Take both from the plural-forms table in the
GNU gettext manual rather than deriving them; the declared count binds every
plural entry in the catalog, and a wrong count invalidates the whole file.

## Terminology sources

Derive a term absent from the language's locked lexicon from the IEEE Standard
Dictionary of Electrical and Electronics Terms, the ITU Radio Regulations, the
ARRL Handbook, or the national electrical-engineering standard for that
language. Record the decision in the rule file's lexicon before using it, so
the term stays fixed for every later string.

## Contact

Translation questions and terminology corrections: xnec2c-bugs@z.ewheeler.org
