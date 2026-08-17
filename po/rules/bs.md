# bs translation rules

## 1. Script and orthography

- Scope: contemporary Bosnian for Bosnia and Herzegovina, written in Latin Gajica with the ijekavian standard, for professional electrical and RF engineers, antenna designers, and amateur-radio operators.
- Use č, ć, š, ž, đ and the digraphs dž, lj, nj; retain every diacritic and never fold a letter to an ASCII look-alike.
- Use precomposed letters, not combining-mark sequences.
- Use U+0027 for an apostrophe where required; omit look-alike apostrophe and modifier-letter code points.
- Script-specific textual joiners do not apply.
- Write left-to-right; bidirectional handling does not apply.
- Bosnian has letter case; apply topic 6.
- Use one space between words and between Bosnian text and embedded foreign, numeric, or unit tokens.
- Form established compounds as fused words, coordinated loanword pairs with a hyphen, and other technical phrases as spaced words; keep lexicalized forms unchanged.

## 2. Numerals in literals

- Use Arabic digits for literal technical values.
- Use a comma as the decimal separator and a space as the thousands separator in translated prose.
- Keep formulas, examples, fixed defaults, and named mathematical or standards constants in source form.
- Form literal ordinals and indices with a digit followed immediately by a period, eg 1.; distinguish this index marker from the decimal comma.

## 3. Punctuation and quotation

- Native quotation marks: „ " (low-high) for prose quotes; embedded technical tokens (filenames, identifiers) keep plain straight quotes for consistency with never-translate tokens.
- Native punctuation replaces source marks in prose: comma, question mark, exclamation mark, opening parenthesis are the same glyphs as source (Latin-script shared set); no separate glyph substitution required.
- No space before colon/semicolon/terminal punctuation; one space after, matching source convention.
- Ellipsis: single "…" character, not three periods, in prose; dash: en dash "–" for ranges, hyphen "-" for compounds, preserved from source where used as a range marker.
- Sentence terminator: full stop for complete sentences (dialogs, tooltips, status messages); short labels, field names, and menu items omit the terminal period.
- Punctuation inside embedded technical runs (format specifiers, code, paths) stays in source form, never localized.

## 4. Never-translate tokens

- Preserve every retained token byte-for-byte with zero omissions, translations, transliterations, or character substitutions.
- NEC2 card mnemonics kept verbatim, unchanged: GW GA GH EX LD FR RP GE EN, and extended SP SM SC NE NH NT TL GN GD GC GX GR GS GM GF CP CM SY XQ EK KH PQ PT.
- Unit symbols kept verbatim: MHz, dBi, Ω, VSWR, %, K, S/m, deg (as a parenthetical unit tag "(deg)"; freestanding prose "degrees" translates, see topic 10).
- Figure-of-merit/parameter tokens kept verbatim: VSWR, S-parameter labels (S11, etc.), Z0, front/back-ratio and G/T tokens.
- File extensions kept verbatim: .nec, .csv, .s1p, .s2p, .png.
- Format specifiers kept verbatim per topic 5: %s, %d, %f, %c, %%, %llu, %lu, %g.
- Embedded identifiers (function names, variable names, config keys) inside developer strings kept verbatim; they name code, not prose.
- Physical/mathematical symbol letters (E, H field letters; θ, φ angle letters) kept verbatim; no native look-alike substitution.
- Product/library/tool names kept verbatim: xnec2c, GTK, Cairo, OpenGL, and the Smith chart proper name ("Smith chart", not "Smitov dijagram").
- Named transfer-function/algorithm terms kept verbatim: Log, Asinh, μ-law, Reinhard, Sigmoid; Power is the descriptive exception and translates to Stepena to denote a power-law curve without merging with electrical snaga.
- Conditional loanwords, decided file-wide: segment stays as loanword segment; patch translates to zakrpa; tag translates to oznaka; fork (process) stays verbatim fork.
- No directionality override applies; topic 1 states left-to-right only.

## 5. Format-specifier integrity

- Preserve the exact source set of format specifiers with zero additions, omissions, type changes, or malformed tokens.
- Keep source order by default; use positional forms such as %1$s only where natural Bosnian grammar requires reordering.
- Restructure the sentence around fixed specifier positions before using positional reordering.
- Keep every number and punctuation character inside a specifier unchanged.

## 6. Capitalization and title-case

- Default casing: sentence case for labels, menu items, dialog titles, and tooltips; title-case-per-word is forbidden.
- Axis letters X, Y, Z stay uppercase as in source; the noun os (axis) itself follows normal sentence casing.
- Lowercase math/coordinate variables (eg x in a formula) stay lowercase; this is distinct from the axis-letter rule above.
- No coordinated-option all-word capitalization exception; "Peak Value / Peak Magnitude" style renders as single-cap "Vršna vrijednost" / "Vršna amplituda".
- Generic card/record noun (kartica) is lowercase in prose, capitalized only when first word of a title or sentence; acronyms (VSWR, NEC2) and proper nouns (topic 4) keep their fixed casing regardless of position.

## 7. Interface register by string type

- Commands, buttons, and menu actions use a formal second-person plural imperative, omit the subject pronoun, place the verb first, and follow it with the object or complement.
- Field labels use the shortest complete noun phrase in normal modifier-head order and retain a source colon.
- Dialogs and confirmations use full sentences in normal subject-verb-object order; omit the subject where the verb form identifies it naturally.
- Tooltips use concise declarative sentences and retain the source reason when a control is unavailable.
- Status and error messages use terse impersonal or declarative sentences, place the result or fault before supporting detail, and omit first-person voice.
- User-visible domain entities use the locked topic-10 noun followed by its distinguishing modifier unless established Bosnian technical usage fixes another order.
- Prefer the shortest natural complete form; use spaced multiword phrases when compounding obscures meaning, retain every source meaning, and omit forced abbreviations and source-length imitation.
- Developer and debug strings follow topic 24.

## 8. Formality and address

- Use formal Vi agreement in direct address and formal imperative endings without an explicit pronoun; omit informal ti forms.
- Use impersonal constructions where no user action is addressed.
- Avoid gendered reference to an unknown user through present-tense, imperative, or impersonal forms; retain grammatical gender, number, and animacy required by the named entity.
- Use neutral professional role terms where available; do not add gender marking absent from the source.
- Honorifics do not apply to interface text; do not add one absent from the source.
- Keep personal names in source order and spelling unless the source supplies a localized form.
- Commands use formal imperatives; dialogs use full formal sentences; confirmations use a full formal question, eg Da li ste sigurni da želite obrisati…?
- Omit casual, slang, commercial, archaic, and ceremonially over-formal register.

## 9. Accelerator/hotkey mnemonics

- Place U+005F immediately before the mnemonic letter inside the translated literal, eg _Otvori.
- Choose a typable letter from the translated term, not a transliteration of the source mnemonic; Bosnian Latin script requires no separate parenthetical mnemonic.
- Add a mnemonic only where the source literal contains one.
- Diacritic letters č, ć, š, ž, and đ are valid mnemonic letters; do not treat the two-letter digraphs dž, lj, or nj as one keystroke.

## 10. Domain lexicon

| Concept | Bosnian term | Sense | Hazard guarded |
|---|---|---|---|
| current | struja | electrical current, A | not "trenutno" (temporal) |
| charge | naboj | electrical charge, C | not "naplata" (billing) |
| voltage | napon | electric potential | catalog consistency |
| power (electrical) | snaga | radiated or dissipated watts and power flow | distinct from the Power transfer-family name |
| Power (transfer family) | Stepena | power-law transfer curve | distinct from electrical snaga |
| impedance | impedansa | complex Z | distinct from otpor, reaktansa |
| resistance | otpor | real part of Z | distinct from impedansa |
| reactance | reaktansa | imaginary part of Z | distinct from otpor |
| inductance | induktivnost | magnetic energy storage, H | distinct electrical primitive |
| capacitance | kapacitivnost | electric energy storage, F | distinct electrical primitive |
| conductivity | provodljivost | material S/m | native term; never "konduktivnost" |
| admittance | admitansa | admittance matrix | distinct from impedansa |
| load | opterećenje | LD-card impedance load | not physical teret; accepted homonym boundary with naboj declared, none merged |
| gain | dobitak (masc.) | antenna directivity, dB | not pojačanje (amplifier), not dobit/profit |
| excitation | pobuda | EM energy input/source | not uzbuđenje (emotional) |
| feedpoint | napojna tačka | antenna feed point | native compound, distinct from port |
| port | port | excitation/S-parameter port | kept as standard RF loanword, distinct from napojna tačka |
| radials | radijali | ground-plane radial wires (noun) | distinct from radijalni/radijalna (adjective) |
| ground / ground plane | uzemljenje | GN/GD ground cards, type/conductivity/effects/model | one term catalog-wide; never tlo |
| earth (physical medium) | tlo | terrain/noise-model earth and below-ground geometry | distinct from uzemljenje; one concrete term |
| ground wave | prizemni talas | propagation term | distinct from uzemljenje |
| wire | žica | thin conductor / GW element | not kabl (cable/cord) |
| segment | segment | NEC2 geometry subdivision | kept loanword, topic 4 |
| patch | zakrpa | NEC2 surface patch (SP/SM) | translated geometry term; one form file-wide |
| tag | oznaka | NEC2 geometry identifier | not a UI label or a card |
| card | kartica | NEC2 input record | register in topic 18 |
| kernel | jezgro | integral-equation/thin-wire kernel | accepted homonym with OS kernel; context disambiguates |
| cliff | litica | two-medium ground-boundary type | not a fracture/break |
| structure | struktura | antenna model geometry | not konstrukcija |
| model | model | NEC model or noise-temperature model | catalog consistency |
| geometry | geometrija | the model geometry | catalog consistency |
| crossed | ukršten | transmission-line conductors crossed/reversed | not "cut/severed" |
| field (EM) | polje | near/total/E/H field | accepted homonym with data/config polje; context disambiguates |
| near field / far field | bliže polje / dalje polje | opposed spatial regions | kept symmetric |
| far-field contribution | doprinos dalekog polja | per-direction contribution | not near-field animation |
| radiation | zračenje | radiated emission | catalog consistency |
| radiation pattern | dijagram zračenja | plotted directional response | not šablon (template/design); one term catalog-wide |
| gain pattern | dijagram dobitka | gain radiation pattern | catalog consistency |
| polarization | polarizacija | antenna/wave field orientation | see Polarization axis below |
| polarity | polaritet | sign (+/-) of a quantity | false friend of polarizacija, never merged |
| phase | faza | angular position of an oscillation | canonical electromagnetic sense |
| reference phase | referentna faza | phase used as the comparison origin | distinct from unqualified faza |
| frequency | frekvencija | oscillation rate | canonical electromagnetic sense |
| wave / wavelength | talas / talasna dužina | propagating oscillation / its spatial period | keep quantity distinct from phenomenon |
| standing wave / traveling wave | stojeći talas / putujući talas | opposed pair | kept symmetric |
| node / antinode | čvor / trbuh | standing-wave zero/maximum | not a generic numeric null/zero |
| crest | greben (talasa) | instantaneous wave apex (comet-head) | distinct from vrh (curve/step peak) |
| magnitude | iznos | modulus of a quantity (|Z|, scalar) | distinct from amplituda; see topic 13 override for peak magnitude |
| amplitude | amplituda | oscillating-quantity peak | distinct from iznos |
| peak value / peak magnitude | Vršna vrijednost / Vršna amplituda | two distinct UI options | must not collapse to one label; locked exception, topic 13 |
| instantaneous | trenutačno | projection mode | "(φ=0)" qualifier only where source carries it |
| Poynting vector | Pojntingov vektor | electromagnetic power-flow vector | established named quantity |
| solid angle | prostorni ugao | steradian angular measure | distinct from planar angle |
| net gain | neto dobitak | total-minus-mismatch gain | not "real (part) gain" (no such catalog term exists) |
| viewer | prikazivač | observation direction or 3D view | not posmatrač, govornik, or pregled |
| flow / flow direction | tok / smjer toka | patch/current flow | catalog consistency |
| total field | ukupno polje | combined electromagnetic field | distinct from an individual field contribution |
| color | boja | visual chromatic attribute | parent concept for the color subsystem |
| color projection | projekcija boje | which quantity drives hue | catalog consistency |
| hue | nijansa | color-wheel angle | distinct from boja itself |
| brightness | svjetlina | luminance channel | catalog consistency |
| hue encoding / brightness encoding | kodiranje nijanse / kodiranje svjetline | distinct internal enums | neither collapses to projekcija boje |
| color scale | skala boja | magnitude-to-color scale | catalog consistency |
| scale family / color tone | ton boje | transfer-curve family | one concept, two source spellings, one term |
| palette / palette kind | paleta / tip palete | palette-layout enum | distinct from ton boje and projekcija boje |
| ramp / gradient | rampa / gradijent | a palette kind / linear color strip | distinct terms, not merged |
| gamma | gama | power-law exponent | loanword |
| knee | koljeno | soft-knee bend point | catalog consistency |
| softening / compression / contrast / dynamic range | omekšavanje / kompresija / kontrast / dinamički raspon | dynamic-range terms | catalog consistency |
| floor | donja granica | minimum/lower clamp (brightness/dB floor) | not pod (room floor) |
| envelope | omotač | magnitude/amplitude envelope | catalog consistency |
| comet | komet | moving-crest overlay effect | not geometrija; fuzzy-inheritance hazard, topic 20 |
| overlay (noun) | preklop / preklopi | an added visual layer | distinct from verb preklopiti |
| animate / animation | animirati / animacija | vary a visual state over time / that process | keep verb and noun forms consistent |
| animated / static | animirano / statično | category-header adjectives | dynamic vs phase-invariant |
| projection | projekcija | color or geometry projection | catalog consistency |
| scale | skalirati (verb) / skala (noun) | resize or map / the mapping range | keep action distinct from object |
| wireframe | žičani okvir | wire-mesh render mode | catalog consistency |
| identity | identitet | no-op/passthrough transfer | distinct from jedinica (Smith-chart unity) |
| sentinel | stražarska vrijednost | unreachable-case guard value | catalog consistency |
| bins | razredi | discretization buckets | catalog consistency |
| companding | kompandiranje | bounded log curve (μ-law) | loanword adaptation |
| tone mapping | mapiranje tonova | photographic tone-map | catalog consistency |
| renderer | renderer | drawing backend | not "mašina za renderovanje" (render engine) |
| shader | šejder | programmable rendering stage | established technical loanword |
| allocation (memory) / managed allocator | alokacija / upravljani alokator | reserved memory / its managing allocator | distinguish operation from component |
| thread | nit | compute thread | no clash with žica (wire), distinct native words |
| widget | widget | UI element | kept untranslated |
| validation | validacija | the validation-tree feature | distinct from verifikacija (verification checks), topic 12 |
| batch mode | serijski način rada | non-interactive grouped processing | distinct from an interactive session |
| fork (process) | fork | process fork | kept verbatim, topic 4 |
| deadlock | deadlock | mutual waiting that prevents progress | kept verbatim as the established technical term |
| notifier | notifikator | component that signals an event | canonical compute term |
| token / operand / operator / arity | token / operand / operator / aritet | expression-parser terms | catalog consistency |
| override | nadjačati | supersede a value (SY symbol) | not prepisati (overwrite) |
| swap | zamijeni | exchange | catalog consistency |
| theme | tema | UI or color theme | distinct from predmet for topic or subject |
| noise / noise temperature | šum / temperatura šuma | electronic/thermal noise | not buka (acoustic) |
| efficiency | efikasnost | useful-output to input-power ratio | canonical engineering metric |
| interpolation | interpolacija | estimation between sampled values | canonical numerical method |
| mnemonic | mnemonik | a card's code descriptor | not bilješka (memo/note) |
| degrees / deg | stepeni / (deg) | freestanding prose vs parenthetical unit tag | (deg) tag kept verbatim per topic 4 |
| diameter | prečnik | canonical term | promjer forbidden as competing form |
| reflect | reflektuj (geometry mirror) / prati (mirrors, tracking) / refleksija (physics, noun) | three distinct senses | never collapsed to one term |
| default(s) | podrazumijevano | fallback value | catalog consistency |
| normalize / normalization | normalizirati / normaliziranje | rescale to a defined reference / that process | native grammatical derivation, topic 16 |

## 11. Disambiguation policy

- Correct technical sense chosen per entry above for every ambiguous term.
- No qualifier is added beyond the source; program context already disambiguates (eg plain dobitak, not "dobitak antene" unless source itself qualifies).
- A qualifier is added only where the Bosnian term would otherwise be genuinely ambiguous.
- Accepted intra-domain homonyms: jezgro for integral-equation and operating-system kernels, and polje for electromagnetic and data/config fields; context disambiguates each. Keep uzemljenje/opterećenje distinct from naboj, tema distinct from predmet, and snaga distinct from Stepena.
- No locative homonym collision recorded beyond the above.
- Gerund vs noun: skaliranje (verbal noun, the act of scaling) vs skala (a scale, the object); kept distinct.

## 12. Cross-catalog consistency

- One term per concept; reuse the lexicon in topic 10 across the whole catalog, never introduce a synonym for an already-mapped concept.
- Keep every Appendix C distinction: polaritet/polarizacija; iznos/amplituda; Vršna vrijednost/Vršna amplituda; uzemljenje/tlo; opterećenje/naboj; dobitak/pojačanje/dobit; struja/trenutno; naboj/naplata; žica/kabl/nit; dijagram zračenja/šablon/dalje polje; pobuda/uzbuđenje; čvor/trbuh/nula; ton boje/nijansa/tip palete/projekcija boje; komet/geometrija; identitet/jedinica; renderer/mašina za renderovanje; nadjačati/prepisati; prikazivač/posmatrač/govornik/pregled; reflektuj/prati/refleksija; struktura/konstrukcija; tema/predmet; validacija/verifikacija; neto dobitak/dobitak realnog dijela; snaga/Stepena. Never use one member's term for another member of the same group.
- Loanword-vs-native locked spellings: diameter → prečnik (native, not promjer); conductivity → provodljivost (native, not konduktivnost); segment, port, fork, deadlock, widget stay loanwords.
- Minority-outlier spellings (eg any -ovati/-isati verb variant) unify to the -irati canonical form, topic 16.
- Consistency priority can outrank locale-form preference: segment stays the loanword catalog-wide even where a native synonym (dio) exists, to hold parity with established NEC2 terminology.

## 13. Priority ordering

- Precedence chain: correct meaning, then interface convention, then catalog consistency, then disambiguation, then locale numeral form.
- Override ruling: Peak magnitude renders as Vršna amplituda (using amplituda) rather than the topic-10 default iznos, because catalog consistency with the already-established Peak value/Peak magnitude UI pair outranks the generic magnitude mapping; the general concept magnitude keeps iznos everywhere else.

## 14. Grammatical number

- Use singular, paucal, and plural forms in translated literals.
- After a literal count ending in 1 but not 11, use singular agreement; after 2-4 but not 12-14, use paucal agreement; use plural agreement otherwise.
- Inflect nouns, adjectives, participles, and verbs for the selected number; topic 15 governs case and concord.

## 15. Grammatical agreement

- Adjectives and participles agree in gender and number with the head noun, including standalone labels whose head noun is implied (eg "Uključen" agrees with masculine unos when the head noun is elided).
- Declension: os (axis) is i-declension only (N os, G/L osi); never a-declension osa/ose.
- Partitive-after-count: paucal 2-4 takes genitive singular (2 segmenta), 5+ takes genitive plural (5 segmenata).
- Reflexive particle se is retained where the source verb is reflexive in Bosnian usage (izračunava se, postavlja se).
- Standalone label with no explicit head noun defaults to masculine gender agreement.

## 16. Morphological derivation

- Form borrowed technical verbs with -irati: normalizirati, definirati, komprimirati, resetirati, kontrolirati, inicijalizirati, konfigurirati, optimizirati, and specificirati; omit competing -ovati and -isati forms. Use matching present-tense forms, eg definira, kontrolira, and komprimira.
- Verbal nouns take the -iranje suffix: resetiranje (not resetovanje), normaliziranje.
- Native-affix preference where an established native term exists (uzemljenje, provodljivost, prečnik); -irati loanword adaptation is the standard fallback otherwise.

## 17. Preposition and sandhi selection

- Preposition s/sa alternates by following sound: sa before words starting with s, z, š, ž, or a consonant cluster (sa mnom); s otherwise (s tobom).
- No further elision/contraction rules apply beyond this alternation.

## 18. Card/record-label register

- Dialog/editor titles use the fixed designator form: "<mnemonic> kartica" (eg GW kartica).
- Running prose uses the hyphenated form: "<mnemonic>-kartica" (eg GW-kartica), or genitive "kartice GW" where grammar requires.
- Generic noun kartica is lowercase except at title/sentence start (topic 6); no short-vs-long form distinction beyond the two registers above.
- Each register stays internally consistent; titles never borrow the hyphenated prose form and vice versa.

## 19. Multi-paragraph and whitespace fidelity

- Mirror source paragraph breaks at the same positions and preserve whether each break is blank-line or single-line.
- Drop clauses absent from the current source rather than retaining inherited text.
- Preserve source trailing newlines and punctuation.
- Preserve semantic line breaks; add no line break for visual wrapping.
- Retain complete natural meaning; do not truncate or abbreviate for an assumed display limit.

## 20. Current-source fidelity

- Derive each translation from the current source literal and its supplied context.
- Reuse an inherited translation only when its complete meaning matches the current source.
- Do not inherit komet as geometrija, merge uzemljenje with tlo, use a forbidden -ovati or -isati form, or exchange iznos with amplituda outside the topic-13 exception.

## 21. Script hygiene

- Require zero Cyrillic or Latin homoglyph substitutions in translated prose; allow a non-Bosnian character only inside a retained token whose source spelling requires it.
- Translate plain foreign words; retain genuine identifiers, units, symbols, and proper technical names under topic 4.

## 22. Rule-file scope hygiene

- Keep only current decisions that can alter wording, Unicode characters, punctuation, capitalization, embedded tokens, mnemonic markers, or semantic whitespace in a translated literal.
- Omit catalog representation, serialization, headers, flags, validation procedure, rendering, fonts, shaping, layout, widget behavior, runtime formatting, sorting, search, display sizing, audit history, completion state, review metadata, provenance, and citations.
- State each decision directly; include no implementation procedure, bibliography, correction history, or alternative policy source.

## 23. Section-disjointness declaration

- Script mechanics in topics 1-3 and 21 govern character, punctuation, direction, and spacing form only.
- Token integrity in topics 4-5 and 9 governs retained tokens, specifiers, and mnemonic markers only.
- Phrasing and structure in topics 6-7 and 14-20 govern sentence, label, inflection, and semantic-whitespace construction only.
- Address register in topic 8 governs formality, person, honorifics, and gender-neutral address only.
- Lexical policy in topics 10-13 governs concept choice, disambiguation, consistency, and precedence only.

## 24. Developer/debug-string policy

- Translate user-facing actions, labels, dialogs, tooltips, statuses, and errors into Bosnian; review priority never permits an applicable user-facing literal to remain untranslated.
- Translate informational notices and reports into Bosnian in a concise professional register.
- Translate developer-facing assertions, parser diagnostics, allocation reports, and debug messages into Bosnian in a terse technical register unless a source family has an explicit established Bosnian precedent, which governs that family.
- Keep identifiers, function names, format specifiers, units, and other topic-4 tokens verbatim in every family regardless of review priority.
- Render the `BUG:` diagnostic prefix as "DEFEKT:", held distinct from the "GREŠKA:" prefix that carries ordinary failures; the two severities never share a word.
- Keep an identifier-plus-value diagnostic pair contiguous (eg `sweep_state=%d`); a qualifier precedes the whole pair and never stands between the identifier and its `=`.
