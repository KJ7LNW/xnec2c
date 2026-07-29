# sl translation rules

## 1. Script and orthography
- Scope: contemporary standard Slovenian for Slovenia, written in Latin script under the SSKJ orthographic norm, for professional electrical and RF engineers, antenna designers, and amateur-radio operators.
- Preserve č, š, ž and Č, Š, Ž exactly; never strip their diacritics or fold them to c, s, or z.
- Use precomposed Unicode letters for Slovenian diacritics; do not substitute combining sequences.
- Use U+2019 RIGHT SINGLE QUOTATION MARK for a Slovenian typographic apostrophe; do not substitute U+0027 or a modifier-letter look-alike.
- Not applicable: Slovenian has no textual joiners or positional letter forms.
- Write left-to-right; embedded retained tokens remain left-to-right and are never mirrored.
- Slovenian distinguishes letter case; apply topic 6 to translated literals.
- Separate words with one space; use one space between Slovenian text and an embedded numeric or retained token where ordinary word separation applies.
- Form established compounds in their standard fused or hyphenated form; otherwise use a spaced technical phrase, eg "sevalni diagram".

## 2. Numerals in literals
- Use digits 0-9 for technical values written in translated prose.
- Use a comma as the decimal separator and a dot as the grouping separator, eg `1.000,5`.
- Preserve the source spelling of formulas, examples, fixed defaults, and named mathematical or standards constants, including π and e.
- Form an ordinal with a digit plus period, eg `1.`; keep an index marker such as `#1` distinct from the decimal comma.

## 3. Punctuation and quotation
- Use Slovenian quotation marks „…“ for quoted prose; retain source straight quotes inside technical tokens whose spelling includes them, for token integrity.
- Use the shared comma, question mark, and exclamation mark forms; not applicable: Slovenian has no inverted opening punctuation.
- Put no space before a colon, semicolon, or terminal mark and one space after a colon or semicolon when text follows.
- Use U+2026 HORIZONTAL ELLIPSIS for an ellipsis; preserve a source hyphen or dash when it carries a technical distinction.
- End full sentences with a period; omit terminal punctuation from short labels and fragments.
- Preserve source punctuation inside an embedded identifier, formula, filename, or other retained technical run.

## 4. Never-translate tokens
- Preserve every retained token exactly, with zero translation, transliteration, case change, character substitution, or internal-space change.
- Keep NEC2 card mnemonics verbatim: `GW GA GH EX LD FR RP GE EN SP SM SC NE NH NT TL GN GD GC GX GR GS GM GF CP CM SY XQ EK KH PQ PT`.
- Keep unit symbols verbatim, including `Hz`, `kHz`, `MHz`, `GHz`, `dB`, `dBi`, `Ω`, `W`, `%`, `°`, `K`, and `S/m`.
- Keep RF figure-of-merit and parameter tokens verbatim, including `VSWR`, S-parameter forms, `Z`, `Z0`, `F/B`, and `G/T`.
- Keep every file extension and printf-style format specifier in its exact source form.
- Keep embedded function names, variable names, identifiers, and configuration keys verbatim.
- Keep physical and mathematical symbol letters verbatim, including `E`, `H`, `φ`, and `θ`.
- Keep proper product, library, toolkit, and chart-name elements verbatim, including `xnec2c`, `NEC2`, `GTK`, `Cairo`, `OpenGL`, and `Smith`.
- Keep named transfer and algorithm terms verbatim, including `μ-law`, `Reinhard`, `Sigmoid`, and `Asinh`; translate the descriptive `Power` family as `potenčna`.
- Keep `segment`, `tag`, `patch`, and process `fork` as loanwords; decline masculine `patch` as `patch`, `patcha`, `patchi`, or `patchev` where grammar requires it.
- Translate ordinary geometry and domain words not listed as retained tokens according to topic 10.
- Apply topic 1 directionality to every retained token.

## 5. Format-specifier integrity
- Preserve exactly the source set of format specifiers; zero missing, added, translated, or altered specifiers are permitted.
- Keep specifiers in source order by default; use a positional specifier only where the source already supplies it.
- Restructure Slovenian wording around fixed specifier positions instead of moving a specifier.
- Preserve every digit and marker inside a specifier without numeral localization.

## 6. Capitalization and title-case
- Use sentence case for labels, menu items, and titles: capitalize the first word and proper nouns, not every lexical word.
- Preserve lowercase mathematical and coordinate variables, including `x`, `y`, and `z`, even at the start of a label.
- Capitalize each element of a coordinated option name only when each element is a retained uppercase symbol, eg `X-Y-Z pogled`.
- Write generic `kartica` lowercase within a sentence and `Kartica` at the start of a title or sentence.
- Preserve acronym and proper-name casing, including `NEC2` and `GTK`.

## 7. Interface register by string type
- Commands, buttons, and menu actions use a second-person singular imperative without an explicit subject; place the verb first and its object or complement after it, eg `Shrani datoteko`.
- Field labels use the shortest complete noun phrase in normal head-before-dependent Slovenian order and retain a source colon, eg `Frekvenca:`.
- Dialog statements use complete declarative sentences in subject-verb-complement order where a subject is natural; confirmation questions place known context before the requested decision and use topic 8's formal address.
- Tooltips use a complete instructional imperative or descriptive sentence matching the source function; when the source explains unavailability, retain the reason after the affected action or state.
- Status and error messages use terse impersonal, passive, or declarative sentences; state the affected entity before the condition when this improves immediate recognition, eg `Datoteke ni mogoče odpreti.`
- Name user-visible domain entities as a generic Slovenian noun followed by a retained designator where applicable, eg `Kartica GW`; apply topic 18 to card names.
- Prefer the shortest natural complete wording and spaced technical phrases; preserve all meaning, avoid unnatural abbreviations, and do not imitate source length.
- Keep developer and debug strings under topic 24.

## 8. Formality and address
- Slovenian distinguishes formal and informal address; use no explicit pronoun in commands and labels.
- Use a second-person singular imperative without a pronoun for commands and instructional tooltips; use third-person indicative wording for descriptive text.
- Use formal plural `vi` and its matching verb ending only when a confirmation or dialog directly addresses the user; never use informal `ti` or `si` there.
- Phrase confirmations as a complete formal-plural question that states the proposed action before requesting confirmation.
- Prefer impersonal or passive wording where direct address or a participle would impose unnecessary gender marking.
- Keep required grammatical gender, number, and animacy agreement natural while referring to roles and users without an exclusionary gender label.
- Not applicable: interface address uses no honorific titles.
- Use grammatical second person only under the command and confirmation rules above; use Slovenian personal-name order as given name followed by family name when a literal presents a person.
- Exclude casual, slang, commercial, archaic, and ceremonially over-formal register.

## 9. Accelerator/hotkey mnemonics
- Preserve a source mnemonic marker as one underscore immediately before a typable letter within the translated term, eg `_Shrani`.
- Choose the mnemonic letter from the translated Slovenian term, never from a transliteration of the source term.
- If that letter conflicts within the same source-defined mnemonic group, choose another distinctive letter from the translated term.
- Not applicable: Slovenian Latin script requires no separate parenthetical mnemonic letter.
- Add no mnemonic marker when the source literal has none.
- Prefer letters available on a standard Slovenian keyboard; avoid č, š, and ž when an equally distinctive plain letter exists.

## 10. Domain lexicon

- Lock each row to its single listed Slovenian term; every row serves correct technical sense and catalog consistency, while its hazard cell records any additional false-friend or retention boundary.

| Concept | Slovenian | Sense | Purpose/hazard |
|---|---|---|---|
| current | tok | electrical current, Amperes | exclude temporal `trenutni` and `nedavni` |
| charge | naboj | electrical charge, Coulombs | exclude billing, fee, and cargo senses |
| voltage | napetost | electric potential | lock the electrical-potential sense |
| power (electrical) | moč | radiated or dissipated watts, gain, and power flow | distinguish `potenčna` transfer family and mathematical `potenca` |
| impedance | impedanca | complex Z | distinguish resistance and reactance |
| resistance | upornost | real part of Z | distinguish impedance and load |
| reactance | reaktanca | imaginary part of Z | distinguish resistance |
| inductance | induktivnost | magnetic energy-storage property | lock the electrical quantity |
| capacitance | kapacitivnost | electric energy-storage property | lock the electrical quantity |
| conductivity | prevodnost | material conductivity in S/m | use the established native term |
| admittance | admitanca | admittance-matrix quantity | distinguish impedance |
| load | obremenitev | LD-card impedance load | exclude physical weight and electrical charge |
| gain | ojačitev | antenna directivity ratio in dB | exclude amplifier `ojačanje` and profit `dobiček` |
| excitation | vzbujanje | EM energy input or source | exclude emotional `vznemirjenje` |
| feedpoint | napajalna točka | antenna feed point | lock the antenna-feed sense |
| port | vrata | excitation or S-parameter port | exclude an ordinary entrance sense |
| radials | radialni vodniki | ground-plane radial wires | distinguish adjective `radialen` |
| ground / ground plane | masa | RF reference plane and GN/GD ground concepts | distinguish physical `zemlja`; use one term across ground sub-senses |
| earth (physical medium) | zemlja | terrain or noise-model medium | distinguish electrical `masa` |
| ground wave | talni val | propagation mode | distinguish the RF reference plane |
| wire | žica | thin conductor / GW element | not cable/cord; unified across uses |
| segment | segment | NEC2 geometry subdivision | untranslated (Appendix B) |
| patch | patch | NEC2 surface patch (SP/SM) | untranslated loanword, masculine declension; never "ploskev" |
| tag | tag | NEC2 geometry identifier | untranslated; not a UI label or card |
| card | kartica | NEC2 input record | register per topic 18 |
| kernel | jedro | integral-equation/thin-wire kernel | not OS kernel |
| cliff | prepad | two-medium ground-boundary type | not fracture/break |
| structure | struktura | antenna model geometry | not "gradnja" (construction) |
| model | model | NEC model or noise-temperature model | |
| geometry | geometrija | model geometry | |
| crossed | prekrižan | transmission-line conductors reversed | not cut/severed |
| field (EM) | polje | near/total/E/H field | distinct from a data/config field |
| near field | bližnje polje | near spatial region | existing catalog usage |
| far field | daljno polje | far spatial region | antonym pair with near field |
| far-field contribution | prispevek daljnega polja | per-direction contribution | not near-field animation |
| radiation | sevanje | radiated emission | |
| radiation pattern | sevalni diagram | plotted directional response | not template/design; corrected from retired calque "vzorec sevanja" |
| gain pattern | diagram ojačitve | gain radiation pattern | |
| polarization | polarizacija | antenna/wave field orientation | |
| polarity | polariteta | sign (+/-) of a quantity | false friend of polarizacija |
| phase | faza | | |
| reference phase | referenčna faza | | existing catalog usage |
| frequency | frekvenca | | |
| wave / wavelength | val / valovna dolžina | | |
| standing wave / traveling wave | stoječi val / potujoči val | opposed pair | |
| node / antinode | vozel / trebuh | standing-wave zero/maximum | not generic "ničla" (numeric null) |
| crest | greben | instantaneous wave apex (comet-head) | distinct from "vrh" (curve/step peak) |
| magnitude | velikost | modulus of a quantity (|Z|, scalar) | distinct from amplituda |
| amplitude | amplituda | oscillating-quantity peak | distinct from velikost |
| peak value | vršna vrednost | UI option | distinct label from vrhovna amplituda, must not collapse |
| peak magnitude | vrhovna amplituda | UI option | existing catalog usage, distinct from vršna vrednost |
| instantaneous | trenutni | projection mode | "(φ=0)" qualifier added only where source carries it |
| Poynting vector | Poyntingov vektor | | proper-name compound |
| solid angle | prostorski kot | | |
| net gain | neto ojačitev | total-minus-mismatch gain | not "realni del ojačitve" |
| viewer | pregledovalnik | observation direction / 3D view widget | not "opazovalec"/govorec/predogled |
| flow / flow direction | smer toka | patch/current flow | existing catalog usage |
| total field | skupno polje | | |
| color | barva | | |
| color projection | barvna projekcija | which quantity drives hue | |
| hue | odtenek | color-wheel angle | chroma subsystem |
| brightness | svetlost | luminance channel | |
| hue encoding / brightness encoding | kodiranje odtenka / kodiranje svetlosti | distinct internal enums | neither collapses to barvna projekcija |
| color scale | barvna lestvica | magnitude-to-color scale | |
| scale family / color tone | družina lestvice | transfer-curve family | map both source spellings to one term; distinguish hue, palette kind, and color projection |
| palette / palette kind | paleta / vrsta palete | palette-layout enum | distinct from scale family and color projection |
| ramp / gradient | lestvica / preliv | palette kind / linear color strip | |
| gamma | gama | power-law exponent | |
| knee | koleno | soft-knee bend point | |
| softening | glajenje | | |
| compression | stiskanje | dynamic-range | |
| contrast | kontrast | | |
| dynamic range | dinamično območje | | |
| floor | spodnja meja | minimum/lower clamp (brightness/dB floor) | not a room floor |
| envelope | ovojnica | magnitude/amplitude envelope | |
| comet | komet | moving-crest overlay effect | not geometry; stale-inheritance hazard |
| overlay (noun) | prekritje | added visual layer | distinct from verb "prekriti" |
| animate / animation | animirati / animacija | | |
| animated / static | animirano / statično | category-header adjectives | |
| projection | projekcija | color or geometry projection | |
| scale | lestvica (noun) / spremeniti merilo (verb) | verb and noun senses | |
| wireframe | žičnati prikaz | wire-mesh render mode | existing catalog usage |
| identity | identiteta | no-op/passthrough transfer | distinct from "enota" (unity, Smith-chart) |
| sentinel | stražar | unreachable-case guard value | |
| bins | koši | discretization buckets | |
| companding | kompandiranje | bounded log curve (μ-law) | |
| tone mapping | tonsko preslikavanje | photographic tone-map | |
| renderer | izrisovalnik | drawing backend | not "render engine" calque |
| shader | senčilnik | | |
| allocation (memory) / managed allocator | dodelitev pomnilnika / upravljani dodeljevalnik | allocation and allocator/report | |
| thread | nit | compute thread | homonym-free from žica (wire); context disambiguates |
| widget | gradnik | UI element | |
| validation | validacija | validation-tree feature | distinct from "preverjanje" (verification checks) |
| batch mode | paketni način | | |
| fork (process) | fork | process fork | kept verbatim (Appendix B) |
| deadlock | zastoj | | |
| notifier | obvestilnik | | |
| token / operand / operator / arity | token / operand / operator / aritnost | expression-parser terms | |
| override | preglasitev | supersede a value (SY symbol) | not "prepisati" (overwrite) |
| swap | zamenjava | exchange | |
| theme | tema | UI/color theme | accepted homonym with "tema" (topic/subject); context disambiguates (topic 11) |
| noise / noise temperature | šum / šumna temperatura | electronic/thermal noise | not acoustic racket |
| efficiency | izkoristek | | |
| interpolation | interpolacija | | |
| mnemonic | mnemonik | card's code descriptor | not "opomnik" (memo/note) |
| degrees / deg | stopinje / (deg) | freestanding prose vs parenthetical unit tag | "(deg)" kept verbatim like other unit tags (topic 4) |
| diameter | premer | canonical native term | not loanword "diameter" |
| reflect | zrcaliti / zrcali / odbijati | geometry mirror / behavioral tracking / physics reflection | three distinct senses, never merged |
| default(s) | privzeta vrednost | fallback value | |
| normalize / normalization | normalizirati / normalizacija | | native-formed, not transliterated |

## 11. Disambiguation policy
- Choose the technical sense fixed in topic 10 for every ambiguous source term.
- Add no qualifier absent from the source when program context identifies the technical sense, eg translate `Currents` as `Tokovi`, not `Električni tokovi`.
- Add a qualifier only where the unqualified Slovenian term remains genuinely ambiguous in its supplied context.
- Accept `tema` for both a user-interface theme and a topic; context distinguishes the senses without an invented qualifier.
- Not applicable: no locative homonym requires a separate Slovenian domain term.
- Use `sevanje` for both the process noun and the emitted-quantity noun; grammatical context distinguishes them.

## 12. Cross-catalog consistency
- Use one topic-10 term per concept throughout the catalog; replace minority spellings with the locked form.
- Keep these pairs distinct: `polariteta` / `polarizacija`; `velikost` / `amplituda`; `vršna vrednost` / `vrhovna amplituda`; `masa` / `zemlja`; `obremenitev` / `naboj`.
- Keep these electrical senses distinct: `ojačitev` / `ojačanje` / `dobiček`; `tok` / `trenutni`; `naboj` / `pristojbina`; `moč` / `potenčna`.
- Keep these geometry and compute senses distinct: `žica` / `kabel` / `nit`; `struktura` / `gradnja`; `zrcaliti` / `slediti` / `odbijati`; `preglasitev` / `prepis`.
- Keep these field and viewer senses distinct: `sevalni diagram` / `predloga` / `daljno polje`; `vzbujanje` / `vznemirjenje`; `vozel` and `trebuh` / `ničla`; `pregledovalnik` / `opazovalec` / `govorec` / `predogled`; `neto ojačitev` / `realni del ojačitve`.
- Keep these color and render senses distinct: `družina lestvice` / `odtenek` / `vrsta palete` / `barvna projekcija`; `komet` / `geometrija`; `identiteta` / `enota`; `izrisovalnik` / `izrisovalni pogon`; `tema` / `predmet`; `validacija` / `preverjanje`.
- Keep `patch` as the canonical masculine loanword and `premer` as the canonical native term.
- Use `ojačitev`, never `ojačenje` or `dobitek`; use `sevalni diagram`, never `vzorec sevanja`.
- Catalog consistency overrides the general native-term preference only for retained `patch`.

## 13. Priority ordering
- Resolve conflicts in this order: correct technical meaning, interface convention, catalog consistency, disambiguation, then literal numeral form.
- Keep `patch` under the higher catalog-consistency ruling even though topic 16 otherwise prefers an established native term.

## 14. Grammatical number
- Slovenian translated literals distinguish singular, dual, paucal forms for counts ending in 3 or 4 where grammar requires them, and plural.
- Inflect nouns, adjectives, participles, and verbs for the grammatical number selected by the explicit literal count.
- Use singular after a count ending in 1 except 11, dual after a count ending in 2 except 12, the 3-or-4 form except 13 or 14, and plural otherwise.

## 15. Grammatical agreement
- Make adjectives and participles agree in gender and number with the head noun, including a head noun implied by a standalone label.
- Decline masculine `patch` as `patch`, `patcha`, `patchi`, or `patchev`; use genitive plural after counts of 5 or more, eg `5 žic`.
- Retain reflexive `se` where the Slovenian verb requires it, eg `spreminja se`.
- Use neuter agreement for a standalone process or state label unless its established implied head noun has another gender, eg neuter `sevanje` and feminine `ojačitev`.

## 16. Morphological derivation
- Form borrowed technical verbs with established `-irati`, eg `animirati`; do not coin competing `-ovati` or `-isati` forms.
- Form verbal nouns with the established `-anje` or `-enje` family, eg `vzbujanje`.
- Prefer an established native affix and term, eg `premer`; use spaced technical phrases unless Slovenian has an established fused or hyphenated compound.

## 17. Preposition and sandhi selection
- Use `s` before a word beginning with `p, t, k, s, š, c, č, f, h`; use `z` before a vowel or other consonant, eg `s spodnjo mejo` and `z animacijo`.
- Treat retained mathematical `z` as a token, not as the preposition.
- Not applicable: Slovenian requires no other elision or contraction in these literals.

## 18. Card/record-label register
- Use `Kartica <oznaka>` in a dialog or editor title, eg `Kartica GW`.
- Use `<oznaka>-kartica` in running prose, eg `GW-kartica`.
- Write generic `kartica` lowercase within prose and `Kartica` at the start of a title or sentence.
- Keep the title and running-prose forms distinct and internally consistent; no separate long form applies.

## 19. Multi-paragraph and whitespace fidelity
- Mirror each source paragraph break at the same position, preserving the distinction between a blank line and a single line break.
- Drop a trailing clause removed from the current source instead of retaining inherited text.
- Preserve a source trailing newline and terminal punctuation exactly.
- Preserve semantic line breaks; add no line break for visual wrapping.
- Preserve complete natural meaning without truncation or an unnatural abbreviation based on assumed display space.

## 20. Current-source fidelity
- Derive every translation from the current source literal and its supplied context.
- Reuse an inherited translation only when its complete meaning agrees with the current source.
- Do not inherit `komet` into a geometry sense or replace canonical `sevalni diagram` with `vzorec sevanja`.

## 21. Script hygiene
- Permit zero wrong-script or Latin-homoglyph substitutions in Slovenian prose; retained tokens are the only exception when their exact spelling requires another character.
- Preserve Slovenian letters and diacritics under topic 1; reject look-alike substitutions for `č`, `š`, and `ž`.
- Translate an ordinary foreign word; preserve only a genuine identifier, unit, symbol, proper name, or other topic-4 token.

## 22. Rule-file scope hygiene
- Keep only current decisions that can alter wording, Unicode characters, punctuation, capitalization, embedded tokens, mnemonic markers, or semantic whitespace in a translated literal.
- Omit catalog representation, serialization, headers, flags, review procedure, rendering, fonts, shaping, layout, widget behavior, runtime formatting, sorting, search, display sizing, audit history, completion state, provenance, and citations.
- State each retained decision directly without implementation procedure, correction history, or alternative policy sources.

## 23. Section-disjointness declaration
- Topics 1-3 govern script, orthography, numerals, and punctuation; topics 5-7, 9, and 13-19 govern literal structure and phrasing; topic 8 governs address register.
- Apply each decision on only its stated axis so no concept receives competing rules across these sections.

## 24. Developer/debug-string policy
- Translate every user-facing command, label, dialog, tooltip, status, and error string; review priority never permits an applicable user-facing string to remain untranslated.
- Translate informational diagnostics intended for users in terse technical Slovenian.
- Keep developer-only debug and trace strings in their source form; no subsystem-specific precedent overrides this rule.
- Preserve every identifier, function name, format specifier, and topic-4 token inside any string family regardless of translation policy or review priority.
