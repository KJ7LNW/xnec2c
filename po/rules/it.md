# it translation rules

## 1. Script and orthography

- Writing system: Latin, with diacritics à è é ì ò ù (grave dominant: è à ù; acute in perché-type words and word-final é); ç rare, loanword-only.
- Diacritics mandatory: never strip or fold to unmarked vowels.
- Precomposed Unicode vowel+diacritic only (à, not a + combining grave); no combining-mark sequences.
- Apostrophe: straight ASCII U+0027 for elision, required (l'ampiezza, dell'ottimizzatore, sull'antenna); never omit (no "la ampiezza"); forbid curly '.
- No script joining/shaping mechanics: Latin script, not applicable.
- Directionality: left-to-right; no bidi handling needed.
- Case distinction: upper/lower case exists; casing rules at topic 6 apply.
- Locale scope: standard Italian for Italy, written in the Latin script under modern Italian orthography, for professional electrical and radio-frequency engineers, antenna designers, and amateur-radio operators; this scope fixes technical sense and interface usage.
- Spacing: single space between words and between Italian text and embedded retained or numeric tokens (eg "50 Ω"); this preserves readable token boundaries.
- Compound formation: use spaced multiword noun phrases (eg diagramma di radiazione); fuse only established lexical compounds and omit compound hyphens, preserving standard technical orthography.

## 2. Numerals in literals

- Digit set: use Western Arabic digits for technical values in translated prose; this preserves Italian engineering convention.
- Decimal separator: use a comma in literal prose values; retain the source form inside formulas, examples, fixed defaults, named mathematical or standards constants, and format specifiers so fixed technical tokens remain unchanged.
- Thousands separator: use a period in literal prose values; this distinguishes grouping from the decimal comma.
- Ordinal/index notation: attach º or ª to a literal digit according to the grammatical gender of the indexed noun; treat the ordinal mark as an affix, never as a decimal separator.

## 3. Punctuation and quotation

- Quotation marks: use straight double quotes and U+0027 apostrophes in interface literals; retain the same marks around embedded technical tokens to keep catalog punctuation consistent.
- Native punctuation: use the comma, colon, semicolon, question mark, and exclamation mark in their standard Italian forms; Italian has no opening question or exclamation mark.
- Spacing: place no space before a comma, colon, semicolon, question mark, exclamation mark, or period, and one space after it when text follows; this preserves Italian punctuation syntax.
- Ellipsis and dash: use U+2026 for an ellipsis and U+2013 for a parenthetical or range dash; retain ASCII hyphen-minus only inside a fixed technical token.
- Sentence terminator: end complete dialogs, tooltips, status messages, and errors with a period; omit a terminator from short labels, menu items, and fragments so each string type remains consistent.
- Embedded technical runs: retain their source punctuation unchanged so identifiers, paths, formulas, and format tokens remain exact.

## 4. Never-translate tokens

- NEC2 card mnemonics: retain `GW GA GH EX LD FR RP GE EN SP SM SC NE NH NT TL GN GD GC GX GR GS GM GF CP CM SY XQ EK KH PQ PT` verbatim; never translate or transliterate these file-format identifiers.
- Unit symbols: retain `Hz kHz MHz GHz dB dBi dBd Ω W K S/m deg %` verbatim; never translate or transliterate standard electrical and radio-frequency notation.
- Figure-of-merit and parameter tokens: retain `VSWR S11 S1P S2P Z Z0 F/B G/T` and every S-parameter token verbatim; never translate or transliterate radio-frequency notation.
- File extensions: retain `.nec .csv .s1p .s2p .png` verbatim; never translate or alter literal filename suffixes.
- Format specifiers: retain every printf-style placeholder verbatim under topic 5; never localize any character within one.
- Embedded code tokens: retain identifier, function, variable, and configuration-key names verbatim in developer strings because they name program elements.
- Physical and mathematical symbols: retain `E H Z φ γ η μ` and other source symbol letters verbatim; never replace them with look-alikes because notation identity is significant.
- Proper technical names: retain `xnec2c NEC2 GSL OpenGL GTK Smith` verbatim; never translate or transliterate product, library, toolkit, or chart names.
- Transfer-function and algorithm names: retain `Log Asinh μ-law Reinhard Sigmoid Identity` verbatim; translate the descriptive `Power` family as `Potenza` to distinguish it from proper names.
- Conditional loanwords: retain `patch`, `tag`, and process `fork` verbatim; translate `segment` as `segmento`; apply each decision catalog-wide to prevent synonym drift.
- Geometry terms outside the conditional set translate according to topic 10; never extend token retention by analogy.
- All retained tokens remain left-to-right under topic 1 and receive no manual direction marker.

## 5. Format-specifier integrity

- Preserve exactly the source set of format specifiers in every translated literal; omission, addition, or mutation is forbidden because substitution depends on token identity.
- Keep source order by default; use positional forms such as `%1$s` only when Italian syntax requires a different argument order.
- Restructure Italian wording around fixed bare specifiers rather than moving them, preserving argument order where positional syntax is absent.
- Preserve every digit and punctuation character inside each specifier; numeral localization applies only outside the token.

## 6. Capitalization and title-case

- Menu items, button labels, and column/field titles: Title Case (established: Guadagno Netto, Diagramma di Radiazione, Impedenza Caratteristica, Scala Struttura); sentence case for full sentences, tooltips, dialogs, error messages.
- Axis-letter labels (X, Y, Z): preserve source casing, no lowercase override.
- Lowercase math/coordinate variables (x, y, z as variables, not axis labels) stay lowercase.
- Coordinated option names: each element Title-Cased consistently (eg "Corrente + Carica").
- Generic card/record noun: lowercase in running prose, Title Case in fixed dialog/editor titles (topic 18); acronyms and proper nouns (NEC2, VSWR) retain fixed casing.

## 7. Interface register by string type

- Commands, buttons, and menu actions: use the shortest complete imperative label with the verb first and the object after it; omit the subject and avoid nominalization so actions read as direct interface commands.
- Field labels: use a terse noun phrase in head-before-modifier order and retain a source colon; omit subjects and verbs because the label names a value rather than asserting a sentence.
- Dialogs and confirmations: use complete sentences in the impersonal patterns fixed by topic 8; present the condition before the requested decision so context precedes action.
- Tooltips: use complete declarative sentences in subject-verb-object order; explain function first and, when the source states it, the reason a disabled control is unavailable.
- Status and error messages: use impersonal declarative clauses with the affected entity before its state or failure; omit direct address and commands so the message reports rather than instructs.
- User-visible domain entities: name the head concept before its qualifying complement or adjective according to standard Italian technical order (eg diagramma di radiazione); retain proper names under topics 4 and 8.
- Terseness: use the shortest natural complete form for each string type; preserve all source meaning, avoid abbreviations absent from established technical usage, and never imitate source length at the cost of Italian syntax.
- Developer and debug strings follow topic 24.

## 8. Formality and address

- Italian distinguishes formal `Lei` and informal `tu`; use neither as direct address, and use impersonal infinitives or `si` constructions to keep a neutral professional register.
- Instructions: use the infinitive with omitted subject and verb-object order (eg Selezionare la riga); this avoids direct second-person address.
- Questions and confirmations: use a `si`-impersonal full question with the decision last (eg Si desidera uscire da xnec2c?); this is the fixed confirmation pattern.
- Descriptive control text: use third-person indicative with the controlled function as subject; this distinguishes description from command.
- Gender and inclusion: avoid addressee gender through impersonal construction; where grammatical gender or number is required, agree with the named entity rather than assigning a gender to a person.
- Honorifics: not applicable because the interface neither addresses nor titles people.
- Personal-name order: preserve the source order of a person's name because the interface defines no locale-specific naming field.
- Grammatical person: commands omit a person-bearing subject; dialogs use impersonal forms; no verb ending encodes honorific formality.
- Prohibit direct `tu` or `Lei` address, casual language, slang, commercial language, archaic language, and ceremonial over-formality; this preserves a neutral engineering register.

## 9. Accelerator/hotkey mnemonics

- Marker: retain one source underscore immediately before the mnemonic letter inside the translated literal; this preserves GTK mnemonic syntax.
- Letter choice: choose a typable letter present in the translated term, preferring an unaccented letter; never reuse the source letter when it is absent from the translation.
- Collision rule: within a shared menu or dialog, select another available letter from the translated term when the first choice collides; this keeps each mnemonic distinct.
- Separate-letter presentation is not applicable because Italian uses the same Latin letters for text and mnemonics.
- Source-presence rule: preserve a mnemonic if and only if the source literal contains one; never invent one or retain one in a string whose source has none.

## 10. Domain lexicon

concept | target term | sense | purpose/hazard guarded
---|---|---|---
current | corrente | electrical current (A) | never attuale/presente (temporal false friend)
charge | carica | electrical charge (C) | never addebito/costo (billing false friend)
voltage | tensione | electric potential | -
power (electrical) | potenza | radiated/dissipated watts, power gain/flow | distinct from Power transfer-family name and math power-law
impedance | impedenza | complex Z | distinct from resistenza/reattanza
resistance | resistenza | real part of Z | distinct from impedenza and carico
reactance | reattanza | imaginary part of Z | -
inductance | induttanza | - | -
capacitance | capacità | - | -
conductivity | conduttività | material S/m | native term
admittance | ammettenza | admittance-matrix sense | distinct from impedenza
load | carico | LD-card impedance load | never physical weight; accepted homonym boundary with carica declared distinct
gain | guadagno | antenna directivity ratio (dB) | never profitto or amplifier amplification
excitation | eccitazione | EM energy input/source | never emotional sense
feedpoint | punto di alimentazione | antenna feed point | -
port | porta | excitation/S-parameter port | -
radials | radiali | ground-plane radial wires (noun) | distinct from adjective radiale
ground / ground plane | massa / piano di massa | RF reference plane, GN/GD cards | never terreno/suolo; one term across sub-uses
earth (physical medium) | terreno | terrain/noise-model earth, below-ground geometry | distinct from massa
ground wave | onda di superficie | propagation term | distinct from massa
wire | filo | thin conductor / GW element | never cavo
segment | segmento | NEC2 geometry subdivision | translated, not kept as loanword (topic 4)
patch | patch | NEC2 surface patch (SP/SM) | kept as loanword catalog-wide
tag | tag | NEC2 geometry identifier | kept as loanword; never a UI label/card
card | scheda | NEC2 input record | register at topic 18
kernel | kernel | integral-equation/thin-wire kernel | never OS kernel; kept as loanword
cliff | dislivello | two-medium ground-boundary type | never scogliera/frattura
structure | struttura | antenna model geometry | never costruzione
model | modello | NEC model or noise-temperature model | -
geometry | geometria | model geometry | -
crossed | incrociati | transmission-line conductors reversed | never tagliati/recisi
field (EM) | campo | near/total/E/H field | disambiguated from a data/config field by context
near field / far field | campo vicino / campo lontano | opposed spatial regions | kept symmetric
far-field contribution | contributo al campo lontano | per-direction contribution | never near-field animation
radiation | radiazione | radiated emission | -
radiation pattern | diagramma di radiazione | plotted directional response | never modello/template
gain pattern | diagramma di guadagno | the gain radiation pattern | -
polarization | polarizzazione | antenna/wave field orientation | see Appendix C, vs polarità
polarity | polarità | sign (+/-) of a quantity | false friend of polarizzazione
phase | fase | - | -
reference phase | fase di riferimento | - | -
frequency | frequenza | - | -
wave / wavelength | onda / lunghezza d'onda | - | -
standing wave / traveling wave | onda stazionaria / onda progressiva | opposed pair | viaggiante avoided as non-standard
node / antinode | nodo / antinodo | standing-wave zero/maximum | also the null/peak overlay sense
crest | cresta | instantaneous wave apex (comet-head) | distinct from picco (curve/step peak)
magnitude | modulo | modulus of a quantity (\|Z\|, scalar) | distinct from ampiezza
amplitude | ampiezza | oscillating-quantity peak | distinct from modulo
peak value vs peak magnitude | valore di picco vs modulo di picco | two distinct UI options | must not collapse to one label
instantaneous | istantaneo | projection mode | "(φ=0)" qualifier only where source carries it
Poynting vector | vettore di Poynting | - | -
solid angle | angolo solido | - | -
net gain | guadagno netto | total-minus-mismatch gain | never guadagno reale/parte reale
viewer | visualizzatore | observation direction / 3D view widget | never osservatore/altoparlante/anteprima
flow / flow direction | flusso / direzione del flusso | patch/current flow | -
total field | campo totale | - | -
color | colore | - | -
color projection | proiezione colore | which quantity drives hue | -
hue | tonalità | color-wheel angle | -
brightness | luminosità | luminance channel | -
hue encoding / brightness encoding | codifica tonalità / codifica luminosità | distinct internal enums | neither collapses to proiezione colore
color scale | scala colore | magnitude-to-color scale | -
scale family / color tone | curva tonale | transfer-curve family (Power/Log/Asinh/μ-law/Reinhard/Sigmoid/Identity) | one target for both source spellings; distinct from tonalità, tipo di palette, and proiezione colore
palette / palette kind | palette / tipo di palette | palette-layout enum | distinct from curva tonale and proiezione colore
ramp / gradient | rampa / gradiente | palette kind / linear color strip | keeps layout and interpolation senses distinct
gamma | gamma | power-law exponent | retained standard notation
knee | ginocchio | soft-knee bend point | distinguishes the curve bend from a body-part reading by technical context
softening | attenuazione | dynamic-range softening | distinguishes gradual range reduction from compression
compression | compressione | dynamic-range compression | distinct from attenuazione
contrast | contrasto | luminance separation | fixes the image-control sense
dynamic range | gamma dinamica | span between usable extrema | fixes the signal-processing sense
floor | soglia minima | minimum/lower clamp (brightness/dB floor) | never pavimento
envelope | inviluppo | magnitude/amplitude envelope | fixes the signal-envelope sense
comet | cometa | moving-crest overlay effect | never geometry
overlay (noun) | sovrapposizione | added visual layer | distinct from verb sovrapporre
animate / animation | animare / animazione | phase-varying display action/process | keeps verb and noun forms consistent
animated / static | animato / statico | dynamic vs phase-invariant category adjectives | keeps the opposed category pair symmetric
projection | proiezione | color or geometry projection | fixes the transformation sense
scale | scalare / scala | verb / noun | keeps grammatical senses distinct
wireframe | struttura a fili | wire-mesh render mode | never antenna wire geometry
identity | identità | no-op/passthrough transfer | distinct from unità in the Smith chart
sentinel | sentinella | unreachable-case guard value | fixes the programming sense
bins | bin | discretization buckets | retained technical loanword
companding | companding | bounded logarithmic curve (μ-law) | retained standard electrical-engineering term
tone mapping | mappatura tonale | photographic tone map | fixes the image-processing sense
renderer | backend grafico | drawing backend | never motore di rendering
shader | shader | graphics program | retained technical loanword
allocation (memory) / managed allocator | allocazione / allocatore gestito | memory allocation and allocator/report | fixes the memory-management sense
thread | thread | compute thread | distinct from filo (wire)
widget | widget | interface element | retained standard toolkit term
validation | validazione | validation-tree feature | distinct from verifica for checks
batch mode | modalità batch | noninteractive processing mode | fixes the processing-mode sense
fork (process) | fork | process creation | retained technical loanword
deadlock | deadlock | blocked concurrency state | retained technical loanword
notifier | notificatore | event notification component | fixes the software-component sense
token / operand / operator / arity | token / operando / operatore / arità | expression-parser terms | keeps parser roles distinct
override (supersede a value) | sostituzione | SY symbol supersession | distinct from sovrascrittura
overwrite | sovrascrittura | replacement of stored data | distinct from sostituzione
swap | scambio | exchange | fixes the bidirectional-exchange sense
theme | tema | interface/color theme | never argomento or soggetto
noise / noise temperature | rumore / temperatura di rumore | electronic/thermal noise | never acoustic disturbance
efficiency | efficienza | electrical or radiation efficiency | fixes the engineering ratio
interpolation | interpolazione | numerical estimation between samples | fixes the numerical sense
mnemonic | mnemonico | card code descriptor | never memo or nota
degrees / deg | gradi / (deg) | prose or axis term / parenthetical unit tag | keeps unit tags verbatim
diameter | diametro | geometric diameter | locks the canonical native term
reflect (geometry) | riflettere | geometry mirror operation | distinct from behavioral tracking and physical reflection
reflect (behavioral) | rispecchiare | control tracking another | distinct from geometry operation and physical reflection
reflect (physics) | riflessione | wave reflection | distinct from geometry operation and behavioral tracking
default(s) | predefinito / predefiniti | fallback value or values | fixes the configuration sense
normalize / normalization | normalizzare / normalizzazione | numerical scaling verb and noun | uses established native derivation

- Reuse each locked target for its concept throughout the catalog; introduce no competing synonym.

## 11. Disambiguation policy

- Choose the technical sense fixed in topic 10 for every ambiguous source concept; this prevents general-language substitutions.
- Add no qualifier absent from the source when program context already identifies the technical sense.
- Add a qualifier only when the unqualified Italian target remains genuinely ambiguous in that context.
- Accepted intra-domain homonym: none; `carico` and `carica` remain distinct despite their related general senses.
- Avoid the wire/thread collision with `filo` for antenna wire and retained `thread` for compute concurrency.
- Render source `scale` as `scalare` for the verb and `scala` for the noun; grammatical role selects the target.

## 12. Cross-catalog consistency

- Use one locked target per concept from topic 10; unify every outlier spelling to that target.
- Keep these Appendix C terms distinct: `polarità` / `polarizzazione`; `modulo` / `ampiezza`; `valore di picco` / `modulo di picco`; `massa` / `terreno`; `carico` / `carica`; `guadagno` / `amplificazione` / `profitto`; `corrente` / `attuale`; `carica` / `addebito`; `filo` / `cavo` / `thread`; `diagramma di radiazione` / `modello` / `campo lontano`; `eccitazione` / `entusiasmo`; `nodo` or `antinodo` / `zero numerico`; `curva tonale` / `tonalità` / `tipo di palette` / `proiezione colore`; `cometa` / `geometria`; `identità` / `unità`; `backend grafico` / `motore di rendering`; `sostituzione` / `sovrascrittura`; `visualizzatore` / `osservatore` / `altoparlante` / `anteprima`; `riflettere` / `rispecchiare` / `riflessione`; `struttura` / `costruzione`; `tema` / `argomento`; `validazione` / `verifica`; `guadagno netto` / `guadagno reale`; `potenza elettrica` / `curva di potenza`.
- Retain the canonical loanwords `patch tag kernel thread shader widget bin companding gamma fork deadlock`; use the native targets in topic 10 for all other mapped concepts.
- Prefer catalog consistency over a locale variant for `patch`, which remains the established NEC2 surface-element term.

## 13. Priority ordering

- Apply this precedence: correct technical meaning, interface convention, catalog consistency, disambiguation, then literal numeral form.
- Use the impersonal register in topic 8 even when a general interface convention favors direct formal address; register consistency outranks that lower-level preference.

## 14. Grammatical number

- Use singular for exactly one and plural for zero or any count other than one; this matches Italian number selection in translated literals.
- Inflect countable nouns, adjectives, participles, and finite verbs for singular or plural agreement; keep invariant loanwords unchanged where established usage does not mark number.
- After an explicit literal count, use the corresponding singular or plural noun phrase (eg `1 segmento`, `2 segmenti`); never infer number from a nearby unrelated value.

## 15. Grammatical agreement

- Make adjectives and participles agree in gender and number with the head noun, including standalone labels whose head is implied.
- Select the established declension of each topic-10 noun; after a literal count, use ordinary Italian agreement rather than a partitive construction.
- Retain reflexive or impersonal `si` wherever the construction in topic 8 requires it.
- Resolve a standalone label's gender from its established implied head noun (eg `Massa` feminine, `Diagramma` masculine); this keeps omitted heads grammatically recoverable.

## 16. Morphological derivation

- Form borrowed technical verbs with established `-are` derivation (eg `normalizzare`); omit improvised or colloquial affixes.
- Form verbal nouns with the established `-zione` family (eg `normalizzazione`); this keeps verb/noun families recognizable.
- Prefer a native affix and compound where established; keep the locked loanwords in topics 10 and 12 unaffixed unless normal Italian inflection is established.

## 17. Preposition and sandhi selection

- Apply mandatory article-preposition contractions (`del`, `della`, `nel`, `sul`) according to the following article; this preserves standard Italian syntax.
- Elide an article or preposition before the vowel-initial form that requires it, using U+0027 (eg `dell'antenna`); retain the full form where Italian does not license elision.
- Other sound-conditioned sandhi selection is not applicable to Italian literals in this catalog.

## 18. Card/record-label register

- Use Title Case `Scheda` plus its designator in dialog or editor titles (eg `Scheda Geometria`); this marks the fixed title register.
- Use lowercase `scheda` plus its qualifier or retained mnemonic in running prose (eg `la scheda GW`); this marks the sentence register.
- No short and long noun forms exist for `scheda`; vary only casing and surrounding grammar by register.
- Never cross-convert the title and prose forms; keep each register internally consistent.

## 19. Multi-paragraph and whitespace fidelity

- Mirror every source paragraph break at the same position, preserving the distinction between a single newline and a blank line.
- Remove any translated trailing clause absent from the current source; never preserve stale inherited meaning.
- Preserve source trailing newlines and terminal punctuation exactly where they carry semantic separation.
- Preserve semantic line breaks; add no line break solely for visual wrapping.
- Preserve complete meaning and natural wording; never truncate or abbreviate a literal to fit an assumed display width.

## 20. Current-source fidelity

- Derive every translation from the complete current source literal and its supplied context.
- Reuse an inherited translation only when its complete meaning agrees with the current source and context.
- Treat inherited `cometa` as unsafe for geometry strings and inherited `soglia` as unsafe outside a confirmed lower-clamp sense; these known collisions require re-derivation from context.

## 21. Script hygiene

- Forbid non-Latin letters and Latin homoglyph substitutions in Italian prose; allow a non-Italian character only inside a retained token whose exact spelling requires it.
- Translate ordinary foreign prose words; retain only the identifiers, units, symbols, and proper technical names enumerated in topic 4.
- Mixed-script adjacency requires no separate Italian rule beyond exact retained-token boundaries.

## 22. Rule-file scope hygiene

- Keep only decisions that can alter wording, Unicode characters, punctuation, capitalization, embedded tokens, mnemonic markers, or semantic whitespace in a translated literal.
- Omit catalog representation, serialization, headers, flags, workflow, rendering, fonts, shaping, layout, widget behavior, runtime formatting, sorting, search, display sizing, audit history, completion state, review metadata, provenance, and citations.
- State each current decision directly; include no implementation procedure, correction history, alternative policy source, or bibliography.

## 23. Section-disjointness declaration

- Topic 1 governs script and orthographic characters; topics 2-5 govern literal numerals, punctuation, retained tokens, and format-token integrity.
- Topics 6, 7, 9, and 18 govern casing, string-type phrasing, mnemonics, and card-label structure; topic 8 alone governs address register.
- Topics 14-17 govern number, agreement, derivation, and contraction without redefining address; each decision belongs to one axis only.

## 24. Developer/debug-string policy

- Translate user-facing controls, dialogs, tooltips, status messages, and errors into complete natural Italian; user visibility determines translation, not review priority.
- Translate informational notices and reports into terse technical Italian while preserving their full meaning.
- Translate developer-facing diagnostics, debug messages, memory reports, configuration diagnostics, theme diagnostics, and validation diagnostics into terse technical Italian; no subsystem family remains in the source language.
- Preserve every embedded identifier, function name, configuration key, format specifier, unit, and retained token verbatim in every translated family under topics 4 and 5.
- Use impersonal declarative wording without direct user address for informational and developer-facing families.
- Review priority affects ordering only; it never permits an applicable string family to remain untranslated.
