# Polish (`pl_PL`) translation rules

These rules govern modern standard Polish for Poland, written in the Latin script, for professional electrical and radio-frequency engineers, antenna designers, and amateur-radio operators using electromagnetic simulation software.

## 1. Script and orthography

- Use the Polish Latin alphabet with mandatory `ą ć ę ł ń ó ś ź ż` and uppercase `Ą Ć Ę Ł Ń Ó Ś Ź Ż`; never strip diacritics or replace letters with ASCII look-alikes.
- Use precomposed Unicode letters for Polish diacritics; do not construct them from a base letter and combining mark.
- Apostrophe, modifier-letter, and textual-joiner distinctions are not applicable to native Polish orthography.
- Script-specific joining and positional letter forms are not applicable.
- Write left to right; retain embedded technical tokens in their source order without mirroring or manual direction marks.
- Apply Polish uppercase and lowercase distinctions; topic 6 fixes interface casing.
- Use modern standard Polish orthography for Poland without historical or regional variants.
- Separate words, numbers, and embedded tokens with one ordinary space; use a non-breaking space only for numeric grouping or where Polish typography binds a value to its unit.
- Follow Polish compound spelling: use an established fused or hyphenated form where prescribed; otherwise use a spaced technical phrase, eg `punkt zasilania`.

## 2. Numerals in literals

- Use European digits `0-9` for technical values written directly in translated prose.
- Use comma `,` as the decimal separator and a non-breaking space as the thousands separator in prose literals.
- Retain formulas, NEC2 data examples, fixed card-field defaults, and named mathematical or standards constants exactly in source form, including their digits and separators.
- Form ordinals from literal digits with a following period, eg `1.`; treat that period as an ordinal marker, never a decimal or index separator.

## 3. Punctuation and quotation

- Use Polish quotation marks `„…”` for quoted prose; retain plain or source quotation marks inside identifiers, code, paths, card data, and other technical tokens.
- Use Polish comma, question mark, exclamation mark, colon, semicolon, and terminal period glyphs; Polish requires no inverted opening punctuation.
- Put no space before a colon, semicolon, comma, question mark, exclamation mark, or period; put one space after it when text follows.
- Use the single ellipsis character `…` in prose; preserve a source hyphen, en dash, or minus sign when it carries a technical distinction.
- End full dialog, tooltip, status, and error sentences with their required terminal mark; omit a terminator from short labels, buttons, menu items, and fragments.
- Keep punctuation within an embedded technical run exactly in source form.

## 4. Never-translate tokens

Retain every token below verbatim, without translation or transliteration; exact spelling preserves file, engineering, mathematical, and program identity.

- NEC2 card mnemonics: `GW GA GH EX LD FR RP GE EN SP SM SC NE NH NT TL GN GD GC GX GR GS GM GF CP CM SY XQ EK KH PQ PT`.
- Unit symbols: `Hz kHz MHz GHz dB dBi dBd Ω V A W K S/m ° %`.
- Figure-of-merit and parameter tokens: `VSWR`, S-parameter forms, `Z`, `Z0`, front/back-ratio tokens, and gain/temperature tokens.
- File extensions: `.nec .csv .s1p .s2p .png`.
- Format specifiers: every printf-style placeholder governed by topic 5.
- Embedded identifiers: function names, variable names, configuration keys, and other code identifiers.
- Physical and mathematical symbols: `E H Z φ θ μ` and other source symbol letters used as notation.
- Product, library, tool, and chart names: `xnec2c`, `NEC2`, `GTK`, `Cairo`, `OpenGL`, and `Smith` where it names the chart.
- Named transfer functions and algorithms: `Log`, `Asinh`, `μ-law`, `Reinhard`, `Sigmoid`, and `Identity`; translate descriptive `Power` as `Potęgowa`.
- Conditional loanwords: retain `segment` and process `fork`; translate `patch` as `płat` and `tag` as `znacznik` in every literal.

## 5. Format-specifier integrity

- Preserve exactly the source set, count, spelling, and order of format specifiers.
- Restructure Polish wording around fixed specifier positions; do not introduce positional forms absent from the source.
- Keep every flag, width, precision, length modifier, conversion letter, and digit inside a specifier unchanged.

## 6. Capitalization and title-case

- Use sentence case for labels, menu items, buttons, and titles; do not imitate source title case.
- Keep axis letters `X Y Z` uppercase.
- Keep lowercase mathematical and coordinate variables `x y z` lowercase.
- Capitalizing every element of a coordinated option name is not applicable; use sentence case for the whole name.
- Keep generic `karta` lowercase except at sentence start; retain acronym and proper-name casing.

## 7. Interface register by string type

Treat interface literals as concise professional Polish; preserve complete meaning and use the shortest natural grammatical form for each family.

- Commands, buttons, and menu actions use a second-person singular imperative with the subject omitted and the object or complement after the verb, eg `Zapisz plik`; a stable entity selector may use a noun phrase.
- Field labels use a concise noun phrase in nominative case followed by the retained colon; put the head noun before its genitive or adjectival qualifier according to established technical usage.
- Dialogs and confirmations use full sentences; confirmations begin with `Czy`, omit the explicit subject, place the finite verb before its object, and present the requested action before consequences.
- Tooltips use a complete declarative or imperative sentence; when the source explains an unavailable control, state the unavailable state first and its reason second.
- Status messages use impersonal or declarative sentences with the affected domain entity before its state when that order aids recognition.
- Error messages use `Nie można` plus an infinitive or `Błąd` plus a genitive noun phrase; state the failed action before its cause or remedy.
- Prefer compact Polish noun phrases over source gerunds; use multi-word structure where a compound would be unnatural, without abbreviating technical meaning.
- Name user-visible domain entities as a Polish head noun followed by the distinguishing technical qualifier; retain proper names and card mnemonics in their topic-4 form.
- Apply topic 24 separately to developer and diagnostic families.

## 8. Formality and address

- Use informal second-person singular only where direct address is required; forbid `Pan`, `Pani`, slang, commercial, archaic, and ceremonially formal wording.
- Carry direct address through the verb ending and omit `ty`; commands use the person-neutral imperative surface form, while dialogs use informal second-person singular.
- Prefer impersonal construction where direct address adds no meaning.
- Avoid gendered references to the user; where grammatical gender, number, or animacy is required for a named entity, agree naturally with that entity rather than a person.
- Honorifics are not applicable to interface address; do not add them.
- Use second person only for user actions and third-person or impersonal forms for program and model states.
- Personal names, when present, retain their source order and spelling; Polish interface text adds no honorific.
- Use `Czy na pewno chcesz …?` for confirmation dialogs requiring direct address.

## 9. Accelerator/hotkey mnemonics

- Preserve `_` as the mnemonic marker immediately before one letter inside the translated literal.
- Select the mnemonic letter from the translated Polish term, never from a source-word transliteration.
- A separate parenthetical mnemonic letter is not applicable because Polish uses the Latin script.
- Add no mnemonic marker when the source literal has none.
- Select a directly typable letter and prefer a base Latin letter over a diacritic letter where the translated term provides one.

## 10. Domain lexicon

Lock each concept to the listed Polish term. Every row protects the stated engineering sense and catalog consistency; an explicit warning names the additional false friend or inheritance hazard. Add no qualifier absent from the source unless the unqualified Polish term remains ambiguous in its supplied context.

### Electrical primitives

| Concept | Polish term | Sense / purpose |
|---|---|---|
| current | prąd | electrical current; established |
| charge | ładunek | electrical charge; distinct from `load` |
| voltage | napięcie | electric potential |
| power (electrical) | moc | radiated/dissipated watts, power gain, power flow; distinct from the `Power` scale-family name |
| impedance | impedancja | complex Z; distinct from resistance/reactance |
| resistance | rezystancja | real part of Z; distinct from impedancja and obciążenie |
| reactance | reaktancja | imaginary part of Z |
| inductance | indukcyjność | |
| capacitance | pojemność | |
| conductivity | przewodność | native term preferred over the loanword `konduktywność` |
| admittance | admitancja | admittance-matrix sense; distinct from impedancja |
| load | obciążenie | LD-card impedance load; not physical weight; accepted homonym with `ładunek` does not occur in Polish, so no note needed |
| gain | zysk | antenna directivity ratio (dB); reserved for this sense only, never financial profit, which never occurs in this catalog |
| excitation | wzbudzenie | EM energy input/source; not emotional excitement |
| feedpoint | punkt zasilania | antenna feed point |
| port | port | excitation/S-parameter port; kept as an established technical loanword |
| radials | promienie (uziemiające) | ground-plane radial wires (noun); distinct from the adjective `promieniowy` |

### Ground and earth

| Concept | Polish term | Sense / purpose |
|---|---|---|
| ground / ground plane | ziemia / płaszczyzna ziemi | RF electrical reference plane, GN/GD ground cards; unify on the `ziemia` root across every sub-use, never `masa` (chassis-ground) nor `uziemienie` (act of earthing) |
| earth (physical medium) | grunt | terrain/noise-model earth, "below ground" geometry; distinct from the electrical reference `ziemia` |
| ground wave | fala przyziemna | propagation term; distinct from the ground reference |

### Geometry primitives

| Concept | Polish term | Sense / purpose |
|---|---|---|
| wire | drut | thin conductor/GW element; not `kabel`/`przewód` (cable/cord) |
| segment | segment | NEC2 geometry subdivision; kept per topic 4 |
| patch | płat | NEC2 surface patch (SP/SM); established translated form |
| tag | znacznik | NEC2 geometry identifier; not a UI label (`etykieta`) or a card (`karta`) |
| card | karta | NEC2 input record; register per topic 18 |
| kernel | jądro (całkowe) | integral-equation/thin-wire kernel; not an OS kernel |
| cliff | urwisko | two-medium ground-boundary type; not a fracture |
| structure | struktura | the antenna model geometry; never `budowa`/`konstrukcja` (construction) |
| model | model | NEC model or noise-temperature model |
| geometry | geometria | the model geometry |
| crossed | skrzyżowane | transmission-line conductors crossed/reversed; not cut/severed |

### Field, pattern, viewer

| Concept | Polish term | Sense / purpose |
|---|---|---|
| field (EM) | pole | near/total/E/H field; accepted homonym with the data/config-field sense of `pole`, disambiguated by context (topic 11) |
| near field / far field | pole bliskie / pole dalekie | opposed spatial regions; kept symmetric |
| far-field contribution | wkład w pole dalekie | per-direction contribution; not near-field animation (`Animacja pola bliskiego`), a distinct feature |
| radiation | promieniowanie | radiated emission |
| radiation pattern | charakterystyka promieniowania | plotted directional response; not `szablon`/`projekt` (template/design) |
| gain pattern | charakterystyka zysku | the gain radiation pattern |
| polarization | polaryzacja | antenna/wave field orientation |
| polarity | biegunowość | sign (+/-) of a quantity; false friend of polaryzacja, kept distinct |
| phase | faza | |
| reference phase | faza odniesienia | |
| frequency | częstotliwość | |
| wave / wavelength | fala / długość fali | |
| standing wave / traveling wave | fala stojąca / fala bieżąca | opposed pair; established |
| node / antinode | węzeł / strzałka (fali) | standing-wave zero/maximum; `strzałka` is the standard Polish physics term for antinode, never `przeciwwęzeł` |
| crest | grzbiet (fali) | instantaneous wave apex; distinct from a curve/step peak (`szczyt`) |
| magnitude | moduł | modulus of a quantity (\|Z\|, scalar); distinct from `amplituda` |
| amplitude | amplituda | oscillating-quantity peak; distinct from `moduł` |
| peak value | wartość szczytowa | distinct UI option from peak magnitude |
| peak magnitude | moduł szczytowy | distinct UI option from peak value |
| instantaneous | chwilowy | projection mode; add a `(φ=0)` qualifier only where the source carries it |
| Poynting vector | wektor Poyntinga | |
| solid angle | kąt bryłowy | |
| net gain | zysk netto | total-minus-mismatch gain; established (`Zysk _netto`), reserved for this sense only |
| viewer | widok / kierunek obserwacji | observation direction and/or the 3D view widget; never `obserwator` (implies a person), `mówca` (speaker), or `podgląd` (preview) |
| flow / flow direction | przepływ / kierunek przepływu | patch/current flow |
| total field | pole całkowite | |

### Color, tone, animation subsystem

| Concept | Polish term | Sense / purpose |
|---|---|---|
| color | kolor | |
| color projection | projekcja koloru | which quantity drives hue |
| hue | odcień | color-wheel angle |
| brightness | jasność | luminance channel |
| hue encoding / brightness encoding | kodowanie odcienia / kodowanie jasności | distinct internal enums; neither collapses to `projekcja koloru` |
| color scale | skala koloru | magnitude-to-color scale; distinct from `rodzina skali` |
| scale family / color tone | rodzina skali | the transfer-curve family (Power/Log/Asinh/μ-law/Reinhard/Sigmoid/Identity); one concept, two source spellings, one Polish term |
| palette / palette kind | paleta / rodzaj palety | palette-layout enum; distinct from `rodzina skali` and `projekcja koloru` |
| ramp / gradient | rampa / gradient | a palette kind (`rampa`) vs the linear color strip (`gradient`); kept distinct |
| gamma | gamma | power-law exponent; math term, kept as-is |
| knee | próg (miękkiego) załamania | soft-knee bend point |
| softening | łagodzenie | dynamic-range softening |
| compression | kompresja | dynamic-range compression |
| contrast | kontrast | |
| dynamic range | zakres dynamiki | |
| floor | dolny próg | minimum/lower clamp (brightness/dB floor); never `podłoga` (a room floor) |
| envelope | obwiednia | magnitude/amplitude envelope |
| comet | kometa | moving-crest overlay effect; not a geometry term (flagged fuzzy-inheritance hazard, topic 20) |
| overlay (noun) | nakładka | an added visual layer; distinct from the verb `nakładać` |
| animate / animation | animować / animacja | |
| animated / static | animowany / statyczny | category-header adjectives (dynamic vs phase-invariant) |
| projection | projekcja | color or geometry projection |
| scale | skala (noun) / skalować (verb) | |
| wireframe | siatka szkieletowa | wire-mesh render mode |
| identity | (funkcja) tożsamościowa | no-op/passthrough transfer; distinct from `jednostkowy` (unity, Smith-chart) |
| sentinel | wartownik | unreachable-case guard value |
| bins | przedziały | discretization buckets |
| companding | kompandowanie | bounded log curve (μ-law) |
| tone mapping | mapowanie tonalne | photographic tone-map |

### Render and compute

| Concept | Polish term | Sense / purpose |
|---|---|---|
| renderer | renderer | drawing backend; never `silnik renderowania` (render engine) |
| shader | shader | kept as an established technical loanword |
| allocation (memory) / managed allocator | alokacja (pamięci) / zarządzany alokator | allocation and the allocator/report |
| thread | wątek | compute thread; no homonym collision with `drut` (wire) exists in Polish |
| widget | widżet | UI element |
| validation | walidacja | the validation-tree feature; distinct from `weryfikacja` (verification checks) |
| batch mode | tryb wsadowy | |
| fork (process) | fork | kept verbatim (topic 4) |
| deadlock | zakleszczenie | |
| notifier | mechanizm powiadamiania | |
| token / operand / operator / arity | token / operand / operator / arność | expression-parser terms; `token` and `operator` kept as established loanwords |
| override | przesłonięcie | supersede a value (SY symbol); distinct from `overwrite` |
| swap | zamiana | exchange |
| theme | motyw | UI/color theme; never `temat` (topic/subject) |

### Metrics and miscellaneous

| Concept | Polish term | Sense / purpose |
|---|---|---|
| noise / noise temperature | szum / temperatura szumów | electronic/thermal noise; not acoustic racket |
| efficiency | sprawność | |
| interpolation | interpolacja | |
| mnemonic | mnemonik | a card's code descriptor; not a memo/note (`notatka`) |
| degrees / deg | stopnie / `(deg)` | freestanding prose form vs the parenthetical unit tag, which is kept verbatim like other unit tags (topic 4) |
| diameter | średnica | canonical native term; the loanword form is not used |
| reflect | odbicie lustrzane (geometry) / odzwierciedla (mirrors, behavioral tracking) / odbicie (physics) | three distinct senses, three distinct renderings |
| default(s) | (wartości) domyślne | fallback value |
| normalize / normalization | normalizować / normalizacja | translated, established native-suffixed form, not a differently transliterated variant |

## 11. Disambiguation policy

- Select the topic-10 engineering sense for every ambiguous source concept.
- Add no qualifier absent from the source when context resolves the Polish term.
- Add the shortest qualifier when the unqualified Polish term would retain a genuine competing technical sense, eg `jądro całkowe` for the thin-wire kernel.
- Accept `pole` for both electromagnetic field and data field; the surrounding domain noun or operation resolves the intra-domain homonym.
- Reserve `ziemia` for the electrical reference and `grunt` for physical terrain or medium to prevent the locative collision.
- Use `skalowanie` for the action and `skala` for the noun; do not collapse source gerund and noun senses.

## 12. Cross-catalog consistency

Use one locked term per concept throughout the catalog; do not introduce synonyms or outlier spellings.

- Keep `biegunowość` distinct from `polaryzacja`, `moduł` from `amplituda`, and `wartość szczytowa` from `moduł szczytowy`.
- Keep `ziemia` distinct from `grunt`, `obciążenie` from `ładunek`, and antenna `zysk` from amplifier `wzmocnienie` and financial `zysk finansowy`.
- Keep electrical `prąd` distinct from temporal `bieżący`; keep electrical `ładunek` distinct from fee `opłata`.
- Keep `drut` distinct from `kabel` and `przewód`, and from compute `wątek`.
- Keep `charakterystyka promieniowania` distinct from `szablon`, `projekt`, and `pole dalekie`.
- Keep electromagnetic `wzbudzenie` distinct from emotional `podniecenie`; keep standing-wave `węzeł` and `strzałka` distinct from generic `zero` and `maksimum`.
- Keep `rodzina skali`, `odcień`, `rodzaj palety`, and `projekcja koloru` as four distinct color-system concepts.
- Keep overlay `kometa` distinct from `geometria`; keep transfer `funkcja tożsamościowa` distinct from Smith-chart `jednostkowy`.
- Keep backend `renderer` distinct from `silnik renderowania`; keep superseding `przesłonięcie` distinct from overwrite `nadpisanie`.
- Keep view `widok` and `kierunek obserwacji` distinct from person `obserwator`, speaker `mówca`, and preview `podgląd`.
- Render geometry mirror as `odbicie lustrzane`, behavioral tracking as `odzwierciedla`, and physical reflection as `odbicie`.
- Keep model `struktura` distinct from `budowa` and `konstrukcja`; keep interface `motyw` distinct from subject `temat`.
- Keep feature `walidacja` distinct from check `weryfikacja`; keep `zysk netto` distinct from gain of a real component.
- Keep electrical `moc` distinct from transfer-family `Potęgowa`.
- Lock established engineering loanwords to their canonical spelling: `impedancja`, `reaktancja`, and `admitancja`; use native `przewodność`.
- Normalize spelling variants to the topic-10 form, eg `impedancja`, never `impedansja`.
- Preserve `zysk netto` and `charakterystyka promieniowania` where a more literal variant would break catalog consistency.

## 13. Priority ordering

- Resolve conflicts in this order: correct technical meaning, Polish interface convention, catalog consistency, disambiguation, then literal numeral form.
- Correct meaning keeps `moduł` distinct from `amplituda` even where general prose uses them loosely.
- Catalog consistency maps both `scale family` and `color tone` to `rodzina skali` because they name one transfer-curve concept.

## 14. Grammatical number

- Use Polish singular, paucal plural, or general plural in every translated literal: singular after `1`; paucal nominative plural after counts ending in `2-4` except `12-14`; genitive plural after `0`, `5-21`, counts ending in `5-9`, and counts ending in `11-14`.
- Inflect the noun, adjective, participle, and finite verb for the selected number; do not copy source singular or plural mechanically.
- With a literal count, use the counted construction required by that count, including genitive plural after `5` and higher forms governed by the rule above.

## 15. Grammatical agreement

- Make adjectives and participles agree in gender, number, and case with their head noun, including standalone labels with an implied head.
- Apply the noun's established declension class; counts governed by general plural take genitive plural, eg `5 segmentów`.
- Retain `się` where the Polish verb requires the reflexive particle, eg `zmienia się`.
- For a standalone label without an established head noun, use masculine as the unmarked technical-interface default; otherwise follow the established noun, eg feminine `charakterystyka`.

## 16. Morphological derivation

- Form borrowed technical verbs with the established `-ować` family, eg `skalować`; use another family only where the technical verb is already lexicalized in that form.
- Form verbal nouns with the productive `-anie` or `-enie` suffix selected by the verb, eg `skalowanie`.
- Prefer an established native root, eg `przewodność`; retain a standardized engineering loanword where it is the professional term, eg `impedancja`.
- Form technical concepts as spaced multi-word phrases unless Polish orthography establishes a fused or hyphenated form.

## 17. Preposition and sandhi selection

- Select `w` or `we` and `z` or `ze` by the initial sound cluster of the following word; use the vowel-augmented form where the base form creates a disfavored consonant cluster.
- Elision and contraction beyond these preposition variants are not applicable to standard Polish interface text.

## 18. Card/record-label register

- In dialog and editor titles, use the fixed designator `Karta GW`, substituting the retained card mnemonic.
- In running prose, inflect `karta` in a separate phrase with the retained mnemonic, eg `błąd karty GW`; do not form a hyphenated compound.
- Keep generic `karta` lowercase except at sentence start; use the same full noun in short and long contexts.
- Keep title-designator and running-prose forms within their own registers; do not cross-convert them.

## 19. Multi-paragraph and whitespace fidelity

- Mirror every source paragraph boundary at the same position, preserving the distinction between blank-line and single-line breaks.
- Preserve semantic line breaks; add no line break solely for visual wrapping.
- Drop a trailing clause removed from the current source instead of retaining inherited text.
- Carry source trailing newlines and punctuation exactly.
- Preserve the complete meaning; do not truncate or abbreviate a translation to fit an assumed display width.

## 20. Current-source fidelity

- Derive each translation from the complete current source literal and its supplied context.
- Reuse an inherited translation only when its complete meaning agrees with the current source.
- Treat inherited mappings of `comet`, `magnitude`, `scale family`, and `color tone` as unsafe; apply the locked topic-10 terms instead.

## 21. Script hygiene

- Use Polish Latin letters in prose; forbid Cyrillic, Greek-look-alike, and non-Polish Latin homoglyphs that imitate Polish or ASCII letters, except inside a retained token whose exact spelling requires them.
- Keep required diacritics and code-point distinctions from topic 1; adjacent native letters and retained tokens remain separated by topic-1 spacing.
- Translate ordinary foreign prose words; retain only genuine identifiers, units, symbols, and proper technical names listed in topic 4.

## 22. Rule-file scope hygiene

- Keep only current decisions that can alter wording, Unicode characters, punctuation, capitalization, embedded tokens, mnemonic markers, or semantic whitespace in a translated literal.
- Omit catalog representation, serialization, headers, flags, validation procedure, rendering, fonts, shaping, layout, widget behavior, runtime formatting, sorting, search, display sizing, audit state, review metadata, provenance, and citations.
- State each decision directly; include no implementation procedure, bibliography, correction history, status report, or alternative policy source.

## 23. Section-disjointness declaration

- Treat script mechanics, phrasing and structure, and address register as disjoint axes: topics 1, 3-5, 9, and 21 govern characters, marks, and retained tokens; topics 2, 6-7, 10-20, and 22 govern literal wording and structure; topic 8 alone governs person and formality.
- Place each decision on one axis only; a cross-reference does not duplicate governance.

## 24. Developer/debug-string policy

- Translate user-facing controls, dialogs, tooltips, errors, and status messages into Polish; their visibility makes them user-facing regardless of review priority.
- Translate informational diagnostics intended for operators into Polish in a terse, technical register.
- Translate developer-facing and low-priority debug strings into Polish in the same terse, technical register; no subsystem family overrides this policy.
- Keep identifiers, function names, format specifiers, units, and topic-4 retained tokens verbatim in every string family.
- Use priority only to order review; never leave an applicable user-facing, informational, or developer-facing literal untranslated because of its priority.
