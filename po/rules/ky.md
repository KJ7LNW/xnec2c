# Kyrgyz translation rules

These rules govern modern Kyrgyz Cyrillic for Kyrgyzstan and professional electrical and radio-frequency engineers, antenna designers, and amateur-radio operators.

## 1. Script and orthography

- Use Kyrgyz Cyrillic only for translated prose; write left to right.
- Preserve the full alphabet, including mandatory Ң/ң, Ө/ө, and Ү/ү; never strip diacritics or replace these letters with look-alikes.
- Use precomposed letters; combining alternatives do not apply to this orthography.
- Use U+0027 only where a retained token requires an apostrophe; do not substitute modifier-letter or typographic look-alikes.
- Textual joiners do not apply; insert no joiner characters.
- Apply normal upper/lowercase distinctions under topic 6.
- Follow modern Kyrgyz Cyrillic orthography used in Kyrgyzstan.
- Separate words and native text from embedded numeric or retained tokens with one space, except where the token's fixed spelling requires adjacency.
- Form technical compounds by standard genitive/possessive attachment or spaced noun phrases; use a hyphen only where Kyrgyz suffix attachment to a digit or retained token requires it.
- Keep E, H, and F/B in their source forms within Kyrgyz compounds to preserve electromagnetic notation.

## 2. Numerals in literals

- Use digits 0-9 for literal technical values.
- Use a comma as the decimal separator and a thin space as the grouping separator in translated prose.
- Keep formulas, examples, fixed defaults, and named mathematical or standards constants exactly as supplied when they are retained technical tokens.
- Attach an ordinal suffix directly with a hyphen, eg 3-чү; treat the hyphen as an affix boundary, never as a decimal separator.

## 3. Punctuation and quotation

- Use « » for quoted prose; retain straight quotes only when they belong to a retained technical token.
- Use comma, period, question mark, and exclamation mark in their shared forms; no distinct opening punctuation applies.
- Insert no space before a colon, semicolon, or terminal punctuation; insert one space after it when text follows.
- Use … for an ellipsis and preserve a source dash as —.
- End full sentences with a period; omit a terminator from short labels, buttons, and fragments.
- Keep punctuation inside formulas, identifiers, units, format specifiers, and other retained technical runs unchanged.

## 4. Never-translate tokens

Treat every item in this section as a zero-failure boundary: retain its exact characters, order, case, and left-to-right spelling; never translate or transliterate it.

- NEC2 card mnemonics: GW, GA, GH, EX, LD, FR, RP, GE, EN, SP, SM, SC, NE, NH, NT, TL, GN, GD, GC, GX, GR, GS, GM, GF, CP, CM, SY, XQ, EK, KH, PQ, PT.
- Unit symbols: Hz, MHz, dB, dBi, Ω, W, K, S/m, °, and %.
- Figure-of-merit and parameter tokens: VSWR, S11, every S-parameter token, Z, Z0, F/B, and G/T.
- File extensions: .nec, .csv, .s1p, .s2p, and .png.
- Format specifiers: %s, %d, %f, %g, %c, %%, and positional forms such as %N$s.
- Embedded identifiers: function names, variable names, configuration keys, and other code identifiers.
- Physical and mathematical symbols: E, H, φ, μ, and other source symbol letters used as notation.
- Product, library, toolkit, and chart names: xnec2c, NEC2, GTK, OpenGL, and Smith.
- Named transfer functions and algorithms: Log, Asinh, μ-law, Reinhard, Sigmoid, and Identity; translate the descriptive Power family as Кубат.
- Translate conditional geometry loanwords to the locked Cyrillic forms сегмент, патч, and тег; retain process operation fork in Latin form.

## 5. Format-specifier integrity

Treat format tokens as immutable placeholders.

- Preserve exactly the same complete set of source specifiers; add, remove, or alter none.
- Preserve source order by default.
- Restructure Kyrgyz wording around fixed positions; use positional specifiers only when natural grammar otherwise fails.
- Keep every number inside a specifier unchanged.

## 6. Capitalization and title case

- Use title case for menu and button labels; use sentence case for titles, field labels, dialogs, tooltips, and status or error messages.
- Keep axis letters X, Y, and Z uppercase.
- Keep lowercase mathematical and coordinate variables lowercase.
- Capitalize each element of a coordinated option name when each element is a proper or retained technical name.
- Capitalize Карта in a dialog or editor title and use карта in running prose; preserve the fixed case of NEC2, xnec2c, and card mnemonics.

## 7. Interface register by string type

Use subject-object-verb order where all elements are stated; omit recoverable subjects and place the governing verb or verbal noun last.

- Commands, buttons, and menu actions use a bare infinitive or verbal noun, omit the subject and object when context supplies them, and place the action last.
- Field labels use a concise noun phrase in modifier-head order followed by the retained colon; use no finite verb.
- Dialogs and confirmations use complete declarative or interrogative sentences, state the affected object before the final verb, and use the formal register from topic 8.
- Tooltips use complete declarative sentences in cause-before-result order; when the source gives a disabled control's reason, retain that reason.
- Status and error messages use impersonal declarative clauses, present the affected entity before its state or action, and use no first-person subject.
- User-visible domain entities use modifier-head naming, eg жыштык графиги; preserve proper names under topics 4 and 8.
- Prefer the shortest complete natural form and standard Kyrgyz compounds; preserve all meaning, use no unnatural abbreviation, and do not imitate source length or word order.

## 8. Formality and address

- Use neutral-formal сиз morphology in sentences that address the user; prohibit сен forms, slang, commercial language, archaic diction, and ceremonial over-formality.
- Commands and labels use subjectless infinitives or verbal nouns; dialogs and confirmations use complete formal sentences.
- Use an explicit сиз only where omission makes the addressee ambiguous; otherwise carry formality in the verb ending or construction.
- Kyrgyz has no grammatical gender: use gender-neutral role nouns and pronoun-free impersonal constructions; grammatical gender agreement is not applicable.
- Keep inclusive wording natural by naming roles or actions rather than assuming identity, sex, or social status.
- Honorifics are not applicable to ordinary interface text; add none absent from the source.
- Use second person only for direct confirmations or instructions and third-person or impersonal forms for status and error text.
- Preserve a person's supplied name order and spelling; do not add patronymics, titles, or honorifics absent from the source.
- Confirmation dialogs state the proposed action, affected object, and consequence, then place the formal question or confirmation verb last.

## 9. Accelerator and hotkey mnemonics

- Place the GTK `_` marker immediately before a typable Kyrgyz Cyrillic letter within the translated term.
- Select the mnemonic from the translated term, never from a source-word transliteration.
- Keep each mnemonic distinct within its interface container by moving `_` to another letter in the same translated term.
- A separate appended mnemonic letter is not applicable; use the in-word Kyrgyz Cyrillic letter only.
- Preserve source presence: add a mnemonic only when the source literal carries one.
- Use mnemonics in buttons, menu items, and tab labels only where present in the source; do not add them to field labels, tooltips, or messages.
- Avoid Ъ, Ь, and Ё because they are unsuitable mnemonic positions in established Kyrgyz terms.

## 10. Domain lexicon

Established lexicon (reuse exactly; never introduce a synonym for a
concept already mapped here). Column 4 states the purpose/hazard the
mapping guards, carried from the authoritative inventory.

### Electrical primitives

| Source concept | Kyrgyz | Sense | Purpose/hazard |
|---|---|---|---|
| current | ток | electrical current, A | never "учур/азыркы" (temporal "current") |
| charge | заряд | electrical charge, C | false friend of billing/fee (topic 12) |
| voltage | чыңалуу | electric potential | native term, locked |
| power (electrical) | кубаттуулук | radiated or dissipated watts, power gain, power flow | separates electrical power from Кубат, the transfer-family name, and mathematical exponentiation |
| impedance | импеданс | complex Z | separates impedance from resistance and reactance |
| resistance | каршылык | real part of Z | separates resistance from impedance and load |
| reactance | реактивдик каршылык | imaginary part of Z | separates reactance from resistance |
| inductance | индуктивдүүлүк | magnetic energy storage per current change | locks the electrical sense |
| capacitance | сыйымдуулук | electric charge storage per voltage | locks the electrical sense |
| conductivity | өткөргүчтүк | material conductivity in S/m | locks the native engineering term |
| admittance | адмиттанс | complex Y and admittance matrices | separates admittance from impedance |
| load | жүктөм | LD-card impedance load | separates impedance load from physical burden and electrical charge |
| gain | күч | antenna directivity ratio in dB | separates antenna gain from profit and amplifier amplification күчөтүү |
| excitation | козгоо | electromagnetic energy input or source | separates excitation from emotional feeling |
| feedpoint | коректөө чекити | antenna feed point | locks the antenna connection sense |
| port | порт | excitation/S-parameter port | loanword |
| radials | радиалдар | horizontal ground-plane radial wires (noun) | distinct from adjective "radial" |

### Ground and earth

| Source concept | Kyrgyz | Sense | Purpose/hazard |
|---|---|---|---|
| ground / ground plane | жерге туташуу тегиздиги | RF electrical reference and GN/GD ground | separates electrical ground from physical earth |
| earth (physical medium) | жер | terrain, noise medium, and below-ground geometry | separates physical earth from electrical ground |
| ground wave | жер толкуну | propagation along the earth | separates propagation from the ground reference |

### Geometry primitives

| Source concept | Kyrgyz | Sense | Purpose/hazard |
|---|---|---|---|
| wire | зым | thin conductor or GW element | separates wire from кабель and compute жип |
| segment | сегмент | NEC2 geometry subdivision | locks the conditional loanword |
| patch | патч | NEC2 SP/SM surface element | locks the conditional loanword |
| tag | тег | NEC2 geometry identifier | separates identifier from label and card; locks е spelling |
| card | карта | NEC2 input record | separates records from identifiers; topic 18 fixes register |
| kernel | өзөк | integral-equation thin-wire kernel | separates this sense from a system kernel |
| cliff | жар | two-medium ground boundary | separates boundary type from a fracture |
| structure | структура | antenna model geometry | separates model structure from курулуш |
| model | модель | NEC or noise-temperature model | locks the engineering sense |
| geometry | геометрия | model geometry | separates geometry from the комета overlay |
| crossed | кайчылаш | reversed transmission-line conductors | separates crossing from cutting |

### Field, pattern, viewer

| Source concept | Kyrgyz | Sense | Purpose/hazard |
|---|---|---|---|
| field (EM) | талаа | near, total, E, or H field | separates electromagnetic field from data fields |
| near field / far field | жакын талаа / алыс талаа | opposed spatial regions | preserves the symmetric pair |
| far-field contribution | алыс талаа салымы | directional contribution | separates contribution from near-field animation |
| radiation | нурлануу | radiated emission | separates process from plotted pattern |
| radiation pattern | нурланма диаграммасы | plotted directional response | separates response from template and far field |
| gain pattern | күч диаграммасы | gain radiation pattern | locks the plotted-gain sense |
| polarization | поляризация | wave or antenna orientation | separates orientation from sign |
| polarity | полярдуулук | quantity sign | separates sign from orientation |
| phase | фаза | wave phase | locks the electromagnetic sense |
| reference phase | эталондук фаза | phase datum | separates reference phase from phase generally |
| frequency | жыштык | cycles per unit time | locks the engineering sense |
| wave / wavelength | толкун / толкун узундугу | wave and its spatial period | preserves related but distinct concepts |
| standing wave / traveling wave | туруктуу толкун / жүрүүчү толкун | opposed wave states | preserves the opposed pair |
| node / antinode | түйүн / антитүйүн | standing-wave zero and maximum | separates them from generic нөл |
| crest | төбө | instantaneous wave apex | separates crest from magnitude peak чоку |
| magnitude | чоңдук | scalar modulus, eg Z magnitude | separates magnitude from amplitude |
| amplitude | амплитуда | oscillating quantity's peak | separates amplitude from magnitude |
| peak value | чоку мааниси | peak-value option | separates it from peak magnitude |
| peak magnitude | чоку чоңдугу | peak-magnitude option | separates it from peak value |
| instantaneous | учурдагы | projection mode | permits (φ=0) only when present in the source |
| Poynting vector | Пойнтинг вектору | electromagnetic energy-flux vector | preserves the proper name and vector sense |
| solid angle | катуу бурч | three-dimensional angular measure | locks the geometric sense |
| net gain | таза күч | total gain minus mismatch loss | separates net gain from real-part gain |
| viewer | көрүүчү | observation direction or 3D view | separates viewer from байкоочу and preview |
| flow / flow direction | агым / агым багыты | patch or current flow | separates flow from compute жип |
| total field | жалпы талаа | combined electromagnetic field | separates total from component fields |

### Color, tone, and animation subsystem

| Source concept | Kyrgyz | Sense | Purpose/hazard |
|---|---|---|---|
| color | түс | visible color | locks the general color term |
| color projection | түс проекциясы | quantity driving hue | separates projection from encoding and palette |
| hue | өң | color-wheel angle | separates hue from scale family and palette |
| brightness | жарыктык | luminance channel | separates luminance from hue |
| hue encoding | өң кодировкасы | hue-mapping enum | separates encoding from color projection |
| brightness encoding | жарыктык кодировкасы | brightness-mapping enum | separates encoding from color projection |
| color scale | түс масштабы | magnitude-to-color scale | separates scale from transfer family |
| scale family / color tone | масштаб тиби | transfer-curve family | collapses two source spellings to one concept |
| palette / palette kind | палитра / палитра түрү | palette and its layout enum | separates palette from scale family and projection |
| ramp / gradient | тилке | linear color strip | collapses equivalent source concepts consistently |
| gamma | гамма | power-law exponent | locks the mathematical sense |
| knee | тизе | soft-knee bend point | separates bend point from anatomy by context |
| softening | жумшартуу | curve softening | locks the dynamic-range sense |
| compression | кысуу | dynamic-range compression | separates compression from file compression |
| contrast | контраст | tonal contrast | locks the visual sense |
| dynamic range | динамикалык диапазон | represented magnitude span | locks the signal and display sense |
| floor | түбү | lower brightness or dB clamp | separates lower clamp from room floor |
| envelope | конверт | magnitude or amplitude envelope | separates signal envelope from mail |
| comet | комета | moving-crest overlay | separates overlay from geometry |
| overlay (noun) | каптама | added visual layer | separates noun from verb кабаттоо |
| animate / animation | анимациялоо / анимация | action and resulting motion mode | preserves verb and noun forms |
| animated / static | динамикалык / статикалык | moving and phase-invariant categories | preserves the opposed pair |
| projection | проекция | color or geometry projection | locks the technical sense |
| scale | масштабдоо / масштаб | scaling action and scale noun | preserves verb and noun forms |
| wireframe | зым тор | wire-mesh render mode | separates render mesh from antenna wire |
| identity | тождик | no-op transfer | separates passthrough from unity бирдик |
| sentinel | сакчы | unreachable-case guard value | locks the software sense |
| bins | интервалдар | discretization buckets | separates buckets from physical containers |
| companding | компандинг | bounded logarithmic curve | locks the μ-law sense |
| tone mapping | тон картасы | photographic tone mapping | separates tone mapping from palette selection |

### Render and compute

| Source concept | Kyrgyz | Sense | Purpose/hazard |
|---|---|---|---|
| renderer | рендерер | drawing backend | separates backend from render engine |
| shader | шейдер | graphics shader | locks the graphics sense |
| allocation (memory) | бөлүштүрүү | memory allocation | separates allocation from ordinary assignment |
| managed allocator | башкарылуучу бөлүштүргүч | managed memory allocator | separates allocator from an allocation event |
| thread | жип | compute thread | separates thread from flow агым and wire зым |
| widget | виджет | interface element | locks the interface sense |
| validation | валидация | validation-tree feature | separates validation from verification текшерүү |
| batch mode | топтомдук режим | noninteractive batch operation | locks the canonical native construction |
| fork (process) | fork | process creation operation | preserves the conditional retained loanword |
| deadlock | дедлок | mutually blocked computation | locks the concurrency sense |
| notifier | билдирүүчү | notification mechanism | separates mechanism from message |
| token | токен | expression-parser token | locks the parser sense |
| operand | операнд | operator input | separates operand from operator |
| operator | оператор | expression operation | separates operator from operand |
| arity | арность | operand count | locks the parser sense |
| override | басуу | supersede a value | separates override from overwrite and swap |
| swap | алмаштыруу | exchange two values | separates swap from override |
| theme | тема | interface or color theme | separates theme from subject matter by context |

### Metrics and miscellaneous

| Source concept | Kyrgyz | Sense | Purpose/hazard |
|---|---|---|---|
| noise / noise temperature | шум / шум температурасы | electronic noise and equivalent temperature | separates electronic noise from acoustic racket |
| efficiency | эффективдүүлүк | output-to-input ratio | locks the engineering sense |
| interpolation | интерполяция | values estimated between samples | locks the numerical sense |
| mnemonic | мнемоника | NEC2 card code descriptor | separates mnemonic from memo or note |
| degrees (freestanding) | градус | prose or axis angle unit | separates prose form from the retained tag |
| deg (unit tag) | (deg) | parenthetical angle tag | preserves the exact unit tag |
| diameter | диаметр | width through a circle | locks the canonical term |
| reflect (geometry) | чагылдыруу | geometry mirror operation | separates operation from behavior and physics |
| mirrors (behavioral) | кайталайт | one control tracks another | separates tracking from geometry and reflection |
| reflection (physics) | чагылуу | physical wave reflection | separates physics from operation and tracking |
| default(s) | демейки | fallback value | locks the configuration sense |
| normalize / normalization | калыпташтыруу | scaling to a reference | locks the native term and avoids transliteration |

Add any further domain concept directly to this table with one target term, sense, and purpose.

## 11. Disambiguation policy

- Select the technical sense fixed by topic 10 for every ambiguous source concept.
- Add no qualifier absent from the source when subsystem context resolves the sense.
- Add a qualifier only when the locked Kyrgyz term remains ambiguous within that literal.
- Accept Кубат for the transfer family and кубаттуулук for electrical power; no intra-domain homonym remains.
- No locative homonym requires a special rule.
- Render an overloaded action as a verbal noun and its object or measure as a noun, eg масштабдоо versus масштаб.

## 12. Cross-catalog consistency

- Reuse one locked Kyrgyz term per concept from topic 10; unify every spelling outlier to that form.
- Prefer the locked native or established loan form in topic 10; introduce no synonym based on source variation.
- Lock тег, топтомдук режим, алмаштыруу, and диаметр even where another locale form exists.
- Keep every pair below distinct:

| Source concepts | Kyrgyz terms | Purpose |
|---|---|---|
| polarity / polarization | полярдуулук / поляризация | separate sign from wave orientation |
| magnitude / amplitude | чоңдук / амплитуда | separate modulus from oscillating peak |
| peak value / peak magnitude | чоку мааниси / чоку чоңдугу | preserve two interface options |
| ground / earth | жерге туташуу тегиздиги / жер | separate electrical reference from physical medium |
| load / charge | жүктөм / заряд | separate impedance from electric charge |
| gain / amplification / profit | күч / күчөтүү / пайда | separate directivity, amplifier action, and commerce |
| current / present | ток / учурдагы | separate electrical and temporal senses |
| charge / fee | заряд / төлөм | separate electric charge from billing |
| wire / cable / thread | зым / кабель / жип | separate conductor, cord, and computation |
| radiation pattern / template / far field | нурланма диаграммасы / шаблон / алыс талаа | separate response, design, and region |
| excitation / emotional excitement | козгоо / толкундануу | separate electromagnetic input from emotion |
| node / antinode / numeric zero | түйүн / антитүйүн / нөл | separate wave extrema from a number |
| scale family / hue / palette kind / color projection | масштаб тиби / өң / палитра түрү / түс проекциясы | preserve four color concepts |
| comet / geometry | комета / геометрия | separate overlay from model geometry |
| identity / unity | тождик / бирдик | separate passthrough from chart unity |
| renderer / render engine | рендерер / рендер кыймылдаткычы | separate backend from engine |
| override / overwrite | басуу / үстүнөн жазуу | separate superseding from writing replacement data |
| viewer / observer / preview | көрүүчү / байкоочу / алдын ала көрүү | separate view, person, and preview action |
| reflect / mirrors / reflection | чагылдыруу / кайталайт / чагылуу | separate geometry, tracking, and physics |
| structure / construction | структура / курулуш | separate model geometry from building |
| theme / subject | тема / предмет | separate interface theme from subject matter |
| validation / verification | валидация / текшерүү | separate tree validation from correctness checks |
| net gain / real-part gain | таза күч / чыныгы бөлүк күчү | separate mismatch-adjusted and real-part quantities |
| electrical power / Power family | кубаттуулук / Кубат | separate watts from transfer curve |

## 13. Priority ordering

- Apply this precedence: correct technical meaning, interface convention, catalog consistency, disambiguation, then literal numeral form.
- Use тег over тэг because canonical consistency outranks spelling variation.
- Use separate кубаттуулук and Кубат terms because correct meaning outranks source-form similarity.

## 14. Grammatical number

- Use singular and plural noun forms; form plurals with the vowel-harmonized -лар/-лер/-лор/-лөр family.
- Keep a noun bare after an explicit literal count, eg 3 сегмент; add no plural suffix.
- Use the plural noun when plurality is stated without an explicit count.
- Adjectives and participles remain invariant for number; finite verbs agree with an expressed subject where Kyrgyz grammar requires it.

## 15. Grammatical agreement

- Grammatical gender and gender concord are not applicable.
- Keep adjectives and participles invariant with singular, plural, stated, or implied head nouns.
- Declension class distinctions do not apply beyond regular vowel harmony and consonant alternation.
- Partitive case after a count is not applicable; keep the counted noun bare.
- Retain a reflexive suffix or particle only where the chosen Kyrgyz verb requires it.
- Resolve standalone labels without an implied gender because Kyrgyz nouns carry none.

## 16. Morphological derivation

- Form borrowed technical verbs with the vowel-harmonized -ла/-ле/-ло/-лө family and its phonological variants; use no competing loan-verb suffix.
- Form verbal nouns with -уу/-үү or the harmonized form required by the stem, eg көрүү and сактоо.
- Prefer a native verb root where topic 10 locks one; otherwise combine the established loan root with a native suffix.
- Form compounds through genitive/possessive attachment or modifier-head noun phrases under topic 1.

## 17. Preposition and sandhi selection

- Preposition selection is not applicable because Kyrgyz uses postpositions and case suffixes.
- Select suffix allomorphs by vowel harmony and final-consonant class, including -лар/-лер/-лор/-лөр and -дан/-ден/-тан/-тен.
- Elision and contraction are not applicable beyond regular suffix allomorphy.

## 18. Card and record-label register

- Use the fixed title designator Карта after the retained mnemonic, eg GW Картасы.
- Use lowercase карта with the required possessive and case suffixes in running prose.
- No separate short and long card nouns apply.
- Keep title and running-prose forms within their own registers; do not cross-convert them.

## 19. Multi-paragraph and whitespace fidelity

- Mirror each source blank-line paragraph break and semantic single-line break at the same position.
- Remove any translated trailing clause absent from the current source.
- Preserve trailing newlines and terminal punctuation mechanically.
- Preserve semantic line breaks; add no visual wrapping absent from the source literal.
- Preserve complete meaning; never truncate or abbreviate for an assumed display width.

## 20. Current-source fidelity

- Derive every translation from the current source literal and supplied context.
- Reuse inherited wording only when its complete meaning agrees with the current source.
- Treat комета versus геометрия, тег versus тэг, and кубаттуулук versus Кубат as unsafe inheritance boundaries.

## 21. Script hygiene

- Permit Latin characters in translated prose only inside retained tokens from topic 4.
- Forbid Cyrillic homoglyph substitutions for Latin H, E, K, P, C, A, O, T, and X inside retained tokens; require zero failures.
- Render ordinary loanwords fully in the locked Cyrillic forms, including сегмент, патч, тег, and виджет.
- Translate ordinary foreign prose; retain only genuine identifiers, symbols, units, mnemonics, proper names, and conditional tokens from topic 4.

## 22. Rule-file scope hygiene

- Include only decisions that alter wording, Unicode characters, punctuation, capitalization, embedded tokens, mnemonic markers, or semantic whitespace in translated literals.
- Omit catalog representation, serialization, headers, flags, workflow, rendering, fonts, shaping, layout, widget behavior, runtime formatting, sorting, search, display sizing, history, completion state, review metadata, provenance, and citations.
- State each current decision directly; include no implementation procedure, bibliography, correction history, or alternative policy source.

## 23. Section-disjointness declaration

- Topic 1 governs script, character, spacing, compound, and direction mechanics only.
- Topics 6-9 govern casing, interface syntax, address register, and mnemonic placement only.
- Topics 10-12 govern the single Kyrgyz term assigned to each source-domain concept only.
- Keep these axes non-overlapping so every decision has one authoritative section.

## 24. Developer and debug-string policy

- Translate every user-facing control, dialog, tooltip, status, warning, and error string into Kyrgyz under topics 7 and 8.
- Translate informational diagnostics intended for users into terse, technical Kyrgyz; review priority never permits an applicable string to remain untranslated.
- Retain developer-only debug and internal tracing prose in its source form; use this category only when the string is not presented as user information.
- Preserve every embedded identifier, function name, format specifier, and retained token from topics 4 and 5 in all three families.
- No subsystem-specific sibling family overrides these classifications.
