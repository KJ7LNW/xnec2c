# mk translation rules

These rules govern standard Macedonian for North Macedonia, written in the modern Macedonian Cyrillic orthography, for professional electrical and radio-frequency engineers, antenna designers, and amateur-radio operators.

## 1. Script and orthography

- Use Macedonian Cyrillic with the required letters Ѓ ѓ, Ѕ ѕ, Ј ј, Љ љ, Њ њ, Ќ ќ, and Џ џ; preserve every required diacritic and never fold a letter to a look-alike.
- Use precomposed Cyrillic code points only; forbid Latin look-alike substitution and combining-diacritic composition.
- No ambiguous joiner/apostrophe mark occurs in standard Macedonian orthography; this sub-concept does not apply.
- No positional letter-shaping or ligature mechanics; Cyrillic here is non-joining; not applicable.
- Left-to-right only; not bidirectional; the opposite-direction/mirroring rule does not apply.
- Has full letter-case distinction (uppercase/lowercase); casing rules follow at topic 6.
- Orthography follows the modern codified standard (1945 codification, current normative spelling); no dialectal variant.
- Standard single space between words and between Macedonian text and embedded Latin/numeric tokens; no special narrow-space convention.
- Compound formation: fused (слитно) for established single-word technical compounds (eg жичен), hyphenated for coordinate/paired terms, spaced multi-word noun phrase for most domain terms (eg дијаграм на зрачење); pick per established lexicon entry, never invent a new pattern for an already-mapped concept.

## 2. Numerals in literals

- Use Western Arabic digits (0-9) for technical values in translated prose; Macedonian has no separate native digit set.
- Use a decimal comma and a grouping period in numbers physically present in translated prose.
- Keep formulas, examples, fixed defaults, and named mathematical or standards constants in source form, including their digits and separators.
- Form literal ordinals and indices as digit + hyphen + native suffix, eg 1-ви; treat the hyphen as an index separator, distinct from the decimal comma.

## 3. Punctuation and quotation

- Native quotation marks are „ “ (low-open, high-close); use them for prose quotations. Embedded technical tokens (code, CLI text) keep plain source quotes for display consistency.
- Comma, question mark, exclamation mark, and opening-quote glyphs are shape-identical to source punctuation; no substitution needed beyond the quotation-mark swap above.
- No space before colon or semicolon; one space after; matches source spacing.
- Ellipsis is the single character …, never three periods. Dashes (em/en) carry over unchanged for ranges.
- Full sentences take a terminal period; short labels and menu/button fragments omit it.
- Punctuation inside an embedded technical run (format specifier, code snippet) stays in source form.

## 4. Never-translate tokens

Treat every retained token as an exact zero-failure boundary: preserve its characters, case, punctuation, and internal spacing; never translate or transliterate it.

- Keep all NEC2 card mnemonics verbatim: GW GA GH EX LD FR RP GE EN SP SM SC NE NH NT TL GN GD GC GX GR GS GM GF CP CM SY XQ EK KH PQ PT.
- Keep source unit symbols verbatim, including Hz, kHz, MHz, GHz, dB, dBi, Ω, W, K, S/m, deg, and %.
- Keep figure-of-merit and parameter tokens verbatim, including VSWR, S11, S21, Z, Z0, F/B, and G/T.
- Keep source file extensions verbatim, including .nec, .csv, .s1p, .s2p, and .png.
- Keep every printf-style format specifier verbatim; topic 5 governs set and order integrity.
- Keep embedded identifiers, function names, variable names, and configuration keys verbatim in developer strings.
- Keep physical and mathematical symbol letters verbatim, including E, H, Z, φ, and μ; a Cyrillic look-alike changes the notation.
- Keep product, library, toolkit, and chart names verbatim: xnec2c, NEC2, GTK, Cairo, OpenGL, and Smith.
- Keep named transfer functions and algorithms verbatim: Reinhard, Sigmoid, Asinh, and μ-law. Translate descriptive Power as степенска to distinguish it from electrical моќност.
- Translate the conditional geometry loanwords as сегмент, плочка, and ознака; transliterate process `fork` consistently as форк.
- Apply topic 1's left-to-right direction to every retained token.

## 5. Format-specifier integrity

- Every specifier present in the source string is preserved, same set, none added or dropped.
- Preserve source order by default; where Macedonian grammar requires a different argument order, use numbered positional forms such as %1$s without changing the represented argument set.
- When numbering is absent or fixed, restructure the sentence around the source specifier positions rather than moving them.
- Numbers rendered through a format specifier are never localized by hand; see topic 2.

## 6. Capitalization and title-case

- Labels, menu items, and titles use sentence case; no forced title-case capitalization of every word.
- Axis letters (X, Y, Z) keep source uppercase Latin casing; never lowercase them.
- A lowercase math/coordinate variable (x, y, z in a formula) stays lowercase; this is not overridden by the axis-letter rule above.
- No coordinated-option-name exception exists in this catalog; only the first word of a multi-word option name capitalizes.
- Generic record noun картичка stays lowercase in running text, capitalized only at sentence start; acronyms and proper nouns (NEC2, Smith) retain source casing.

## 7. Interface register by string type

- Commands/buttons/menu actions: terse singular imperative (-и, -ај), eg Ресетирај, Зачувај, Избери, Прикажи, Избриши, Скалирај; established catalog convention for labels.
- Field labels: noun phrase plus retained colon, eg Фреквенција:.
- Dialogs and confirmations: full grammatical sentences in the formal-neutral plural imperative (-ете, -ајте), eg Изберете, Прикажете ја, Анимирајте ја, Овозможете го, Вратете ги; no explicit Вие pronoun needed.
- Tooltips: full-sentence form; when the source states why a disabled control is unavailable, the translation states the same reason.
- Status/error messages: use an impersonal declarative clause, preserve cause before consequence, and use passive voice where the source foregrounds the affected object; omit an explicit subject unless contrast requires it.
- Commands place the imperative first and its object after it; field labels use a head-first noun phrase; dialogs and tooltips use normal subject-verb-object order with omitted recoverable subjects; confirmations place Дали first, then the formal predicate, action, and consequence.
- Name user-visible domain entities as a specific modifier plus the topic-10 head term, eg референтна фаза; retain proper-name and personal-name rules from topic 8.
- Use the shortest complete natural form: prefer established compounds or spaced phrases, preserve every meaning-bearing element, and avoid invented abbreviations or mechanical imitation of source length.
- Developer/debug-string policy is topic 24.

## 8. Formality and address

- Macedonian distinguishes formal Вие from informal ти; full-sentence dialogs/tooltips/status text use the formal-neutral plural verb form; informal ти-singular imperative is forbidden in those strings.
- Short command labels use the terse singular imperative form as a separate, established catalog register (topic 7); this is not the informal ти-address, it is the standard label convention and coexists with the formal register above without contradiction.
- Subject pronouns drop by default; use second-person plural only through the formal verb ending when addressing the user, and use third-person impersonal constructions for status and error text.
- Imperative forms carry no gender marking; where agreement requires gender, follow the explicit referent and otherwise use the natural generic masculine without naming or implying a user gender.
- Use inclusive role and participant nouns where established; preserve required grammatical gender, number, and animacy agreement without adding gender not present in the source.
- Macedonian interface address uses no honorific; personal names retain their supplied order and spelling. Honorific morphology and alternate personal-name order are not applicable.
- Realize register through the imperative ending (-ете/-ајте formal, -и/-ај terse label), not a separate pronoun.
- Confirmation dialogs use a full formal-plural interrogative sentence, eg Дали сте сигурни дека сакате да ...?
- Forbid casual, slang, over-formal, commercial, and archaic register.

## 9. Accelerator/hotkey mnemonics

- Mnemonic marker is an underscore before the chosen letter, as GTK requires.
- Keep mnemonic letters unique within their source-defined menu or dialog; on collision, choose the next unused eligible letter from the translated term.
- Because the translated label uses Cyrillic and the accelerator key uses Latin, append a parenthetical Latin key corresponding to a letter in the translated term, eg Зачувај (_Z); never inherit or transliterate the source term's mnemonic.
- Preserve source presence exactly: add the marker only where the source literal carries one and never invent one elsewhere.
- Use a directly typable standard Latin key; avoid a key requiring a dead key or alternate input layer.

## 10. Domain lexicon

Concept key, chosen Macedonian term, intended sense, and the hazard the mapping guards, per row.

### Electrical primitives

| Concept | Термин | Смисла | Причина |
|---|---|---|---|
| current | струја / струи | electrical current, A | not the temporal "recent" sense |
| charge | полнеж / полнежи | electrical charge, C | not billing/fee/cargo |
| voltage | напон | electric potential | keeps potential distinct from current and power |
| power (electrical) | моќност | radiated/dissipated watts, power gain/flow | distinct from Power (scale family), see topic 4 |
| impedance | импеданса | complex Z | distinct from resistance, reactance |
| resistance | отпор | real part of Z | distinct from impedance, load |
| reactance | реактанса | imaginary part of Z | keeps the imaginary component distinct from resistance |
| inductance | индуктивност | magnetic energy-storage property | keeps it distinct from capacitance |
| capacitance | капацитивност | electric energy-storage property | keeps it distinct from inductance |
| conductivity | спроводливост | material S/m | native term preferred over loanword |
| admittance | адмитанса | admittance matrix | distinct from impedance |
| load | оптоварување | LD-card impedance load | not physical weight; distinct from charge |
| gain (antenna) | добиток | directivity ratio, dB | not profit (добивка), not amplifier засилување |
| excitation | возбуда | EM energy input/source | not возбуденост (emotional excitement) |
| feedpoint | точка на напојување | antenna feed point | identifies the excitation connection, not a generic point |
| port | порт | excitation/S-parameter port | not порта (gate/door) |
| radials | радијали | ground-plane radial wires, noun | distinct from adjective радијален |

### Ground and earth

| Concept | Термин | Смисла | Причина |
|---|---|---|---|
| ground / ground plane | маса | RF electrical reference, GN/GD cards | not soil; one term across all sub-uses |
| earth (physical medium) | земја | terrain/noise-model earth, "below ground" | distinct from electrical маса |
| ground wave | приземен бран | propagation term | distinct from маса |

### Geometry primitives

| Concept | Термин | Смисла | Причина |
|---|---|---|---|
| wire | жица / жици | thin conductor, GW element | not кабел (cable/cord) |
| segment | сегмент | NEC2 geometry subdivision | conditional loanword, translated (topic 4) |
| patch | плочка | NEC2 surface patch, SP/SM | one translated geometry term; never the generic површина |
| tag | ознака | NEC2 geometry identifier | not a UI label or a card |
| card | картичка | NEC2 input record | register at topic 18 |
| kernel | јадро | integral-equation/thin-wire kernel | accepted homonym with OS sense; domain never exposes the OS sense |
| cliff | карпа | two-medium ground-boundary type | not пресек/пукнатина (fracture/break) |
| structure | структура | antenna model geometry | not конструкција (construction) |
| model | модел | NEC model or noise-temperature model | keeps the represented system distinct from its geometry |
| geometry | геометрија | spatial definition of the model | keeps geometry distinct from structure and construction |
| crossed | вкрстени | transmission-line conductors crossed/reversed | not пресечени (cut/severed) |

### Field, pattern, viewer

| Concept | Термин | Смисла | Причина |
|---|---|---|---|
| field (EM) | поле | near/total/E/H field | accepted homonym with data/config field; context disambiguates |
| near field / far field | блиско поле / далечно поле | opposed spatial regions | kept symmetric |
| far-field contribution | придонес во далечно поле | per-direction contribution | not near-field animation |
| radiation | зрачење | radiated electromagnetic emission | keeps emission distinct from its plotted pattern |
| radiation pattern | дијаграм на зрачење | plotted directional response | not шаблон/дизајн (template/design) |
| gain pattern | дијаграм на добиток | gain-valued radiation pattern | keeps gain content distinct from the generic radiation pattern |
| polarization | поларизација | antenna/wave field orientation | see Appendix C pair with polarity |
| polarity | поларитет | sign (+/-) of a quantity | never conflated with поларизација |
| phase | фаза | angular position in a periodic quantity | keeps phase distinct from polarity and amplitude |
| reference phase | референтна фаза | phase used as the comparison origin | keeps the reference quantity distinct from phase generally |
| frequency | фреквенција | cycles per unit time | keeps frequency distinct from phase and wavelength |
| wave / wavelength | бран / бранова должина | propagating oscillation / its spatial period | keeps the quantity distinct from the phenomenon |
| standing wave / traveling wave | стоечки бран / патувачки бран | stationary / propagating wave forms | preserves the opposed pair |
| node / antinode | јазол / антијазол | standing-wave zero/maximum | also the null/peak overlay sense |
| crest | гребен | instantaneous wave apex (comet-head) | distinct from врв (peak of a curve/step) |
| magnitude | модул | modulus of a quantity (\|Z\|, scalar) | distinct from amplitude; single term across all magnitude contexts, replaces prior split usage |
| amplitude | амплитуда | oscillating-quantity peak | distinct from модул |
| peak value | врвна вредност | maximum value option | distinct UI option from peak magnitude |
| peak magnitude | врвен модул | maximum modulus option | must not collapse to peak value; kept consistent with модул |
| instantaneous | моментален (φ=0) | projection mode | qualifier only where source carries it |
| Poynting vector | Појнтингов вектор | electromagnetic energy-flux vector | preserves the established proper-name term |
| solid angle | просторен агол | three-dimensional angular measure | keeps it distinct from a planar angle |
| net gain | нето добиток | total-minus-mismatch gain | not "реален дел на добиток" |
| viewer | прегледувач | observation direction / 3D view widget | not набљудувач/говорник/преглед |
| flow / flow direction | тек / насока на тек | patch or current flow and its direction | keeps the quantity distinct from its directional label |
| total field | вкупно поле | combined electromagnetic field | keeps the aggregate distinct from individual field components |

### Color, tone, animation subsystem

| Concept | Термин | Смисла | Причина |
|---|---|---|---|
| color | боја | visible color generally | parent term for hue, brightness, palette, and projection concepts |
| color projection | проекција на боја | which quantity drives hue | parallels проекција |
| hue | нијанса | color-wheel angle | standard color-theory term |
| brightness | осветленост | luminance channel | keeps luminance distinct from hue |
| hue encoding | кодирање на нијанса | internal enum | distinct from color projection |
| brightness encoding | кодирање на осветленост | internal enum | distinct from color projection |
| color scale | скала на бои | magnitude-to-color mapping | keeps the mapping distinct from scale family and palette kind |
| scale family / color tone | фамилија на скали | transfer-curve family (Power/Log/Asinh/μ-law/Reinhard/Sigmoid/Identity) | one concept, one term; color tone collapses onto this entry |
| palette / palette kind | палета / вид на палета | palette-layout enum | distinct from scale family and color projection; values линеарна (ramp), дивергентна (diverging), циклична (cyclic) |
| ramp / gradient | рампа / градиент | palette kind / linear color strip | preserves the two source concepts without collapsing them into palette |
| gamma | гама | power-law exponent | standard EE/photography transliteration |
| knee | колено | soft-knee bend point | literal EE term |
| softening | омекнување | dynamic-range softening | native formation from мек |
| compression | компресија | dynamic-range compression | established loanword |
| contrast | контраст | separation between light and dark values | preserves the established technical loanword |
| dynamic range | динамички опсег | ratio or span between limiting levels | preserves the standard electrical-engineering term |
| floor | долна граница | minimum/lower clamp, brightness/dB floor | avoids the room-floor sense под |
| envelope | обвивка | magnitude/amplitude envelope | established EE signal-processing term |
| comet | комета | moving-crest overlay effect | not geometry; direct loanword matches visual metaphor |
| overlay (noun) | преклоп | added visual layer | distinct from verb преклопува |
| animate / animation | анимира / анимација | phase-varying display action / state | keeps verb and noun forms consistent |
| animated / static | анимиран / статичен | category-header adjectives | dynamic vs phase-invariant |
| projection | проекција | mapping into color or geometry | parent term kept distinct from color projection |
| scale | скала / скалира | scale noun / scaling verb | preserves the grammatical sense of each source use |
| wireframe | жичен приказ | wire-mesh drawing mode | keeps the display mode distinct from antenna wire geometry |
| identity | идентитет | no-op/passthrough transfer | distinct from единица (unity, Smith chart) |
| sentinel | сентинел | unreachable-case guard value | CS loanword |
| bins | интервали | discretization buckets | native, matches histogram-bucket sense |
| companding | компандирање | bounded logarithmic μ-law curve | keeps the operation distinct from generic compression |
| tone mapping | тон-мапирање | photographic dynamic-range mapping | keeps the operation distinct from color tone and scale family |

### Render and compute

| Concept | Термин | Смисла | Причина |
|---|---|---|---|
| renderer | рендерер | drawing backend | never called "render engine" |
| shader | шејдер | graphics-stage program | keeps the program distinct from the renderer backend |
| allocation (memory) / managed allocator | алокација / управувач на алокации | memory allocation / its manager | keeps the operation distinct from the managing component |
| thread | нишка | compute thread | not homonymous with жица (wire) in Macedonian; no collision |
| widget | виџет | interface element | locks the established toolkit term |
| validation | валидација | validation-tree feature | distinct from проверка (verification/checks) |
| batch mode | пакетен режим | non-interactive grouped processing mode | keeps the mode distinct from an individual package |
| fork (process) | форк | process creation by fork | locked transliteration; never use a branch/fork utensil sense |
| deadlock | мртва брава | mutual waiting with no progress | preserves the established native computing compound |
| notifier | известувач | component that emits notifications | keeps the component distinct from a notification message |
| token / operand / operator / arity | токен / операнд / оператор / аритет | lexical item / value / operation / operand count | keeps all parser roles distinct |
| override | надместува | supersede a value, SY symbol | not презапишува (overwrite) |
| swap | размена | exchange two values | keeps exchange distinct from overwrite and override |
| theme | изглед | UI/color theme | never тема, which is reserved for topic/subject to avoid the false-friend collision |

### Metrics and miscellaneous

| Concept | Термин | Смисла | Причина |
|---|---|---|---|
| noise / noise temperature | шум / температура на шум | electronic/thermal noise | not acoustic racket |
| efficiency | ефикасност | useful output divided by input | keeps the metric distinct from gain and power |
| interpolation | интерполација | estimation between sampled values | keeps interpolation distinct from extrapolation |
| mnemonic | мнемоник | a card's code descriptor | not белешка (memo/note) |
| degrees / deg | степени / (deg) | freestanding prose vs parenthetical unit tag | tag kept verbatim like other unit tags |
| diameter | дијаметар | canonical loanword | minority пречник unifies to дијаметар |
| reflect (geometry mirror op) | отсликува | geometry mirror operation | distinct from mirrors and reflection below |
| mirrors (behavioral tracking) | следи | a control tracking another | distinct from reflect and reflection |
| reflection (physics) | рефлексија | physics reflection | distinct from reflect and mirrors |
| default(s) | стандардна вредност / стандардни вредности | fallback value or values | keeps the fallback sense distinct from a general standard |
| normalize / normalization | нормализира / нормализација | rescale to a reference / that operation | use the established translated forms consistently |

### File-specific additions beyond Appendix A

| Concept | Термин | Смисла | Причина |
|---|---|---|---|
| optimizer | оптимизатор | optimization component | distinguishes the component from the act of optimization |
| hue wheel | тркало на нијанси | circular hue selector | keeps hue distinct from palette and color projection |
| cold/hot (color-temperature sign metaphor) | ладно/топло | color-temperature sign metaphor | avoids the literal physical-temperature wording студено |

## 11. Disambiguation policy

- Correct technical sense chosen per ambiguous term; see topic 10 rows for the resolved sense of each.
- No qualifier is added that the source omits; program context already disambiguates (eg струи for "Currents", not електрични струи).
- A qualifier is added only where the target term would otherwise be genuinely ambiguous, eg модул на импеданса when модул alone risks a software-module reading.
- Accepted intra-domain homonym: поле serves both EM field and data/config field; context disambiguates.
- Accepted homonym: јадро serves both integral-equation kernel and, generically, OS kernel; this catalog never exposes the OS sense, so no collision occurs in practice.
- Gerund vs noun kept distinct: скалирање (act of scaling) vs скала (a scale).

## 12. Cross-catalog consistency

- One term per concept, reused from topic 10 exactly; no synonym introduced for an already-mapped concept.
- Appendix C false-friend pairs, each resolved to two distinct terms:

| Pair | Term A | Term B |
|---|---|---|
| polarity / polarization | поларитет | поларизација |
| magnitude / amplitude | модул | амплитуда |
| peak value / peak magnitude | врвна вредност | врвен модул |
| ground / earth | маса | земја |
| load / charge | оптоварување | полнеж |
| gain / amplification / profit | добиток | засилување / добивка |
| current / present-recent | струја | сегашен / неодамнешен |
| charge / billing-fee | полнеж | наплата / такса |
| wire / cable-cord | жица | кабел |
| wire / thread (compute) | жица | нишка |
| radiation pattern / template-design | дијаграм на зрачење | шаблон / дизајн |
| excitation / emotional excitement | возбуда | возбуденост |
| node-antinode / generic zero | јазол / антијазол | нула |
| scale family vs hue vs palette kind vs color projection | фамилија на скали | нијанса / вид на палета / проекција на боја |
| comet / geometry | комета | геометрија |
| identity / unity | идентитет | единица |
| renderer / render engine | рендерер | рендерирачки механизам |
| override / overwrite | надместува | презапишува |
| viewer / observer-speaker-preview | прегледувач | набљудувач / говорник / преглед |
| reflect / mirrors / reflection | отсликува | следи / рефлексија |
| structure / construction | структура | конструкција |
| theme / topic-subject | изглед | тема (reserved, unused in this domain) |
| validation / verification | валидација | проверка |
| net gain / real-part gain | нето добиток | реален дел на добиток |
| power (electrical) / Power (scale family) | моќност | степенска |

- Loanword-vs-native decision: prefer the EE-standard loanword where one dominates (импеданса, реактанса, адмитанса); prefer native where a native word already dominates common technical usage (отпор, спроводливост, напон).
- Canonical spelling is locked per topic 10; minority spelling variants (eg диаметар) unify to the canonical form (дијаметар).
- Consistency priority can outrank locale-form preference: модул is kept for magnitude even where a locale-preferred plain word (големина) might otherwise apply, to preserve one term across all magnitude contexts.

## 13. Priority ordering

- Precedence chain: correct meaning, then interface convention, then catalog consistency, then disambiguation, then locale numeral form.
- Explicit override: модул for magnitude was chosen over the locale-preferred големина specifically to preserve catalog consistency (priority 3) once ambiguity with amplitude (priority 1, correct meaning) was resolved.

## 14. Grammatical number

- Macedonian translated literals distinguish singular and plural; nouns, adjectives, participles, and finite verbs take the form required by the referenced quantity and head noun.
- After an explicit literal count ending in 1 but not 11, use the singular count form; use the plural count form after all other explicit counts.
- Preserve established collective or counted-noun forms where the domain term requires them; do not invent a plural marker on an acronym, mnemonic, unit symbol, or other retained token.

## 15. Grammatical agreement

- Adjectives and participles agree in gender and number with the head noun, including a standalone label whose head noun is implied (eg feminine избришана where картичка is implied, masculine избришан where фајл is implied).
- Macedonian has no case declension beyond vestigial vocative and no partitive-after-count construction; this sub-concept does not apply.
- Reflexive verbs retain the particle се, eg се вчитува (is loading).
- A standalone label with no explicit head noun defaults to masculine gender unless the implied noun in context (eg feminine картичка) is known.

## 16. Morphological derivation

- Borrowed technical verbs take the -ира suffix (скалира, анимира, интерполира); an ad hoc -увам coinage is forbidden where an -ира form is already established.
- Verbal-noun formation takes -ање/-ување (скалирање, анимирање).
- Prefer loanword+ира verbs for technical operations; prefer native compounding for concrete descriptive terms (жичен приказ, динамички опсег).

## 17. Preposition and sandhi selection

- Macedonian has no context-conditioned preposition-form alternation and no required elision/contraction in formal register; this topic does not apply beyond avoiding colloquial elision in UI text.

## 18. Card/record-label register

- Dialog/editor titles use the fixed designator form Картичка [TAG], картичка capitalized only at title start, tag verbatim.
- Running prose uses a hyphenated/suffixed form, eg GW-картичка, when referencing a card type inline.
- Generic noun картичка stays lowercase in running text; the short form (bare mnemonic, eg GW) is acceptable once context is established.
- Each register stays internally consistent; a title-form string is never cross-converted into the running-prose form or vice versa.

## 19. Multi-paragraph and whitespace fidelity

- Mirror source paragraph breaks at the same positions and preserve whether each break is blank-line or single-line.
- Drop a trailing clause removed from the current source rather than preserving stale translated text.
- Carry the source trailing newline and terminal punctuation mechanically.
- Preserve semantic line breaks; add no line break used only for visual wrapping.
- Preserve complete natural meaning; never truncate or abbreviate a translation to fit an assumed display constraint.

## 20. Current-source fidelity

- Derive every translation from the current source literal and its supplied context.
- Reuse an inherited translation only when its complete meaning agrees with the current source.
- Treat inherited card mnemonics, format-specifier relationships, scale-family names, magnitude/amplitude wording, and theme terminology as unsafe unless they match the decisions in topics 4, 5, 10, and 12.

## 21. Script hygiene

- Forbid non-Macedonian Cyrillic spellings where standard Macedonian requires Ѓ, Ѕ, Ј, Љ, Њ, Ќ, or Џ.
- Forbid Latin-Cyrillic homoglyph mixing within translated prose and within retained tokens; keep Latin `a`, `e`, `o`, `p`, `c`, and `x` only where a topic-4 token's literal spelling requires them.
- Translate plain foreign prose words; retain genuine identifiers, units, extensions, and format tokens exactly as topic 4 specifies.

## 22. Rule-file scope hygiene

- Keep only current decisions that can alter a translated literal's wording, Unicode characters, punctuation, capitalization, embedded tokens, mnemonic marker, or semantic whitespace.
- Omit catalog representation, serialization, headers, flags, validation workflow, rendering, fonts, shaping, layout, widget behavior, runtime formatting, sorting, search, display sizing, audit history, completion state, review metadata, provenance, and source citations.
- State each retained decision directly; include no implementation procedure, bibliography, correction history, or alternative policy source.

## 23. Section-disjointness declaration

- Script mechanics (topics 1, 2, 3, 4, 5, 9, 21) govern character/glyph/token form only.
- Phrasing and structure (topics 6, 7, 10-20) govern word choice, casing, and grammar only.
- Address register (topic 8) governs formality/person only.
- These three axes are non-overlapping; every concept in this file lands in exactly one of them.

## 24. Developer/debug-string policy

- Translate every user-facing family, including commands, labels, dialogs, tooltips, status messages, errors, and user-visible validation results.
- Translate informational diagnostics that explain program state to the user in the terse technical register from topic 7.
- Keep developer-only debug and low-level internal diagnostic families in the source language unless an established sibling family in the same subsystem is already translated; that sibling-family decision governs the whole family.
- Preserve identifiers, function names, retained tokens, and format specifiers verbatim in every family, whether its prose translates or remains in the source language.
- Use priority only to order review; never leave an applicable user-facing or informational literal untranslated because of priority.
