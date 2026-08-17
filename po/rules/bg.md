# bg translation rules

## 1. Script and orthography

- Scope: standard Bulgarian for Bulgaria, written in the modern 30-letter
  Bulgarian Cyrillic alphabet, for professional electrical and radio-frequency
  engineers, antenna designers, and amateur-radio operators.
- Preserve every Bulgarian letter exactly, including ъ, ь, щ, ю, and я; never
  strip a letter feature or fold a letter to a look-alike.
- Bulgarian Cyrillic letters use their single alphabetic code points; a
  precomposed-versus-combining policy is not applicable.
- Apostrophe and textual-joiner distinctions are not applicable to standard
  Bulgarian technical prose.
- Positional letter joining is not applicable to Bulgarian Cyrillic.
- Directionality: Bulgarian prose is left-to-right; embedded retained technical
  tokens remain left-to-right and are neither mirrored nor given manual
  direction overrides.
- Orthographic standard: modern standard Bulgarian orthography.
- Inter-word spacing: single space between words and between native text and
  embedded foreign/numeric tokens.
- Compound formation: technical compounds are spaced multiword noun phrases
  (eg "диаграма на излъчване"), not fused or hyphenated, except established
  adjective forms (eg "конусен проводник").

## 2. Numerals and locale data

- Digit set: Arabic numerals (0-9) for all technical values; no other digit
  set used.
- Decimal separator: comma in Bulgarian prose numbers, eg `γ = 0,5`.
  Formulas, code examples, fixed code defaults, and named mathematical or
  standards constants retain their literal source separators.
- Thousands/grouping separator: space, where grouping is shown at all.
- Ordinal/index notation: digit plus native suffix (eg "1-ви", "2-ро"); an
  index separator (eg segment index) is distinct from the decimal comma.
- Formulas, examples, fixed defaults, and named mathematical or standards
  constants retain every source digit and separator when they are fixed
  literal tokens.

## 3. Punctuation and quotation

- Native quotation marks: `„“` for Bulgarian prose;
  plain/straight quotes retained only around embedded technical tokens (file
  paths, code) to avoid confusion with prose quotes.
- Comma, question mark, and full stop keep their standard shared Latin/Cyrillic
  shapes; no separate native glyph substitution needed beyond quotation marks.
- Spacing: no space before colon/semicolon, single space after, matching
  standard Bulgarian typography.
- Ellipsis: "…"; dashes in source (en dash for ranges) are preserved as given.
- Sentence terminator: full stop required on complete sentences (tooltips,
  dialogs, status/error messages); short labels, menu items, and buttons omit
  the trailing period.
- Punctuation inside embedded technical runs (code, format specifiers) stays
  in source form.

## 4. Never-translate tokens

- Preserve every retained token with zero character, case, punctuation, or
  internal-space changes; never translate or transliterate a token listed as
  verbatim below.
- Keep all NEC2 card mnemonics verbatim: `GW GA GH EX LD FR RP GE EN SP SM SC
  NE NH NT TL GN GD GC GX GR GS GM GF CP CM SY XQ EK KH PQ PT`.
- Keep unit symbols verbatim: `Hz kHz MHz GHz dB dBi dBd Ω V A W K S/m ° %`;
  keep the parenthetical angle tag `(deg)` verbatim.
- Keep figure-of-merit and parameter tokens verbatim: `VSWR`, S-parameter
  tokens, `Z`, `Z0`, `F/B`, and `G/T`.
- Keep file extensions verbatim: `.nec .csv .s1p .s2p .png`.
- Keep every source format specifier verbatim, including `%s %d %f %c %%`;
  topic 5 governs their set and order.
- Keep embedded function names, variable names, configuration keys, and other
  code identifiers verbatim inside developer strings.
- Keep physical and mathematical symbol letters verbatim, including `E`, `H`,
  `θ`, `φ`, and exponent symbols; a Cyrillic look-alike changes the notation.
- Keep product, library, toolkit, and chart names verbatim: `xnec2c`, `GSL`,
  `OpenGL`, `GTK`, and `Smith`.
- Keep named transfer and algorithm terms verbatim: `Log`, `Asinh`, `μ-law`,
  `Reinhard`, `Sigmoid`, and `Identity`; translate descriptive `Power` as
  `степенна функция`.
- Translate the conditional geometry and process loanwords once and reuse the
  locked Cyrillic forms: `segment` → `сегмент`, `patch` → `патч`, `tag` →
  `таг`, and process `fork` → `форк`.
- Translate ordinary geometry and domain words unless this section lists them
  as retained tokens.

## 5. Format-specifier integrity

- Preserve the source specifier set with zero failures: add, remove, split,
  merge, or alter no specifier.
- Keep source order by default; use positional forms such as `%1$s` only when
  natural Bulgarian syntax requires argument reordering and the format
  supports it.
- When positions are fixed, restructure the Bulgarian sentence around them
  rather than moving the specifiers.
- Preserve every digit, dollar sign, flag, width, precision, length modifier,
  and conversion character inside each specifier; localize none of them.

## 6. Capitalization and title-case

- Default casing: sentence case for labels, menu items, and section headers,
  not title case; only the first word and proper nouns capitalize.
- Axis letters `X`, `Y`, and `Z` stay uppercase in Bulgarian literals even
  when the source uses lowercase; this overrides sentence case under topic 13.
- Lowercase math/coordinate variables (eg γ, φ) stay lowercase; this
  exemption is distinct from the axis-letter rule above.
- Coordinated-option each-word-capitalization: not applicable, not used in
  this catalog.
- Generic card/record noun casing by position: see topic 18.
- Case distinction exists in Bulgarian, so this topic applies in full.

## 7. Interface register by string type

- Commands and buttons use a short second-person-plural imperative with the
  subject omitted; place the verb before its object or complement.
- Menu actions and section headers use a verbal noun or a plain noun phrase;
  omit a subject and finite verb, and place governing nouns before `на`
  complements.
- Field labels use a concise noun phrase followed by the retained colon;
  place the head noun before its qualifier unless established Bulgarian
  technical usage requires an adjective first.
- Dialogs and confirmations use full grammatical sentences in natural
  subject-verb-object order, with Bulgarian topic-focus variation only where
  it improves the given-new information flow.
- Tooltips use full declarative sentences; state the unavailable condition
  before its reason when the source supplies both, and preserve the source
  reason for every disabled control.
- Status and error messages use impersonal or subjectless declarative
  sentences; present the affected entity before the condition or failure and
  place diagnostic detail after the main clause.
- User-visible domain entities use the topic-10 head term followed by its
  identifying mnemonic, number, axis, or qualifier in natural Bulgarian
  order; topic 8 governs personal names and address.
- Use the shortest complete natural Bulgarian form for every string type;
  preserve all meaning, avoid unnatural abbreviations, and never imitate the
  source length or word order mechanically.
- Topic 24 alone governs developer and debug strings.

## 8. Formality and address

- Use formal direct address throughout the interface: capitalize `Вие` and
  `Ви`, use second-person-plural verb endings and agreement for one or more
  addressees, and never use informal `ти` forms.
- Commands omit the subject pronoun and carry formal address through the
  second-person-plural imperative; dialogs state the subject only where a
  natural Bulgarian sentence requires emphasis or contrast.
- Confirmations use a complete formal question ending in `?`; present the
  proposed action before the request for confirmation.
- Avoid unnecessary gender marking for people; where Bulgarian grammar
  requires gender or number, agree naturally with the named person or use an
  inclusive plural construction that preserves the source meaning.
- Use no honorific unless the source names one; preserve the source
  grammatical person and write personal names in given-name then family-name
  order.
- Commands use neutral professional imperatives; dialogs, tooltips, statuses,
  and errors use formal impersonal or declarative constructions.
- Exclude casual, slang, commercial, archaic, and ceremonially over-formal
  registers.

## 9. Accelerator/hotkey mnemonics

- Marker: underscore before a Cyrillic letter of the Bulgarian word (`_X`),
  per GTK convention (eg "_Мигновена", "_Пикова стойност").
- Per-container uniqueness required; a collision is resolved by choosing a
  different letter within the same translated word.
- Choose the mnemonic letter from the translated Bulgarian term, never by
  transliterating the source mnemonic letter.
- Non-Latin appended-Latin-letter presentation: not applicable, Bulgarian
  keyboards type Cyrillic directly.
- Never invent a mnemonic where the source string carries none.
- Preserve a mnemonic marker only when the source literal carries one; never
  add or remove one based on the string type.
- Avoid rare or hard-to-reach letters (eg Щ, Ъ) as mnemonics; prefer common
  consonants.

## 10. Domain lexicon

| Concept | Bulgarian term | Sense | Purpose/hazard guarded |
|---|---|---|---|
| current | ток | electrical current in amperes | distinct from temporal `текущ` |
| charge | заряд | electrical charge | not billing/fee |
| voltage | напрежение | electric potential | canonical electrical quantity |
| power (electrical) | мощност | watts, power gain/flow | distinct from "Power" transfer-family name (степенна) and math power-law |
| impedance | импеданс | complex Z | distinct from resistance/reactance |
| resistance | съпротивление | real part of Z | distinct from impedance and load |
| reactance | реактанс | imaginary part of Z | distinct from resistance and impedance |
| inductance | индуктивност | magnetic energy storage per current change | canonical electrical quantity |
| capacitance | капацитет | electric charge stored per potential difference | electrical sense, not generic capacity |
| conductivity | проводимост | material S/m | native term |
| admittance | адмитанс | admittance matrix | distinct from impedance |
| load | товар | LD-card impedance load | not physical weight; distinct from charge |
| gain | антенно усилване | antenna directivity ratio in dB | distinct from amplifier amplification and profit |
| excitation | възбуждане | EM energy input/source | not emotional excitement |
| feedpoint | точка на захранване | antenna feed point | canonical antenna connection term |
| port | порт | excitation or S-parameter port | technical connection sense |
| radials | радиали | ground-plane radial wires (noun) | distinct from adjective "радиален" |
| ground / ground plane | земя / земна равнина | RF electrical reference plane, GN/GD cards | one concept, compound variant for "plane"; never "заземяваща равнина" |
| earth (physical medium) | земна среда | terrain or noise-model physical medium | distinct from electrical `земя` |
| ground wave | земна вълна | propagation term | distinct from the electrical ground reference |
| wire | проводник | thin conductor / GW element | not cable/cord; not "нишка" (thread) |
| segment | сегмент | NEC2 geometry subdivision | locked conditional-loanword decision |
| patch | патч | NEC2 surface element for SP or SM | locked loanword spelling, never `пач` |
| tag | таг | NEC2 geometry identifier | not a UI label or a card |
| card | карта | NEC2 input record | register at topic 18 |
| kernel | ядро | integral-equation/thin-wire kernel | not an OS kernel |
| cliff | рязка граница | two-medium ground-boundary type | not a fracture/break |
| structure | структура | antenna model geometry | not "строителство" |
| model | модел | NEC or noise-temperature model | canonical simulation sense |
| geometry | геометрия | spatial definition of the antenna model | distinct from the comet overlay |
| crossed | кръстосани | crossed/reversed conductors | not cut/severed |
| field (EM) | поле | near/total/E/H field | accepted homonym with UI data field, context disambiguates |
| near field / far field | близко поле / далечно поле | opposed spatial regions | kept symmetric |
| far-field contribution | принос на далечното поле | per-direction contribution | not near-field animation |
| radiation | излъчване | radiated electromagnetic emission | distinct from a radiation pattern |
| radiation pattern | диаграма на излъчване | plotted directional response | not a template/design |
| gain pattern | диаграма на усилване | gain radiation pattern | consistent with radiation-pattern terminology |
| polarization | поляризация | antenna/wave field orientation | distinct from polarity |
| polarity | полярност | positive or negative sign of a quantity | distinct from `поляризация` |
| phase | фаза | angular position of an oscillating quantity | canonical electromagnetic sense |
| reference phase | референтна фаза | phase used as the angular reference | distinct from an unqualified phase value |
| frequency | честота | oscillation rate | canonical radio-frequency quantity |
| wave / wavelength | вълна / дължина на вълната | propagating oscillation / its spatial period | keep the related concepts distinct |
| standing wave / traveling wave | стояща вълна / бягаща вълна | stationary interference pattern / propagating wave | preserve the opposed pair |
| node / antinode | възел / пучност | standing-wave zero/maximum | also the null/peak overlay sense |
| crest | гребен на вълната | instantaneous comet-head apex | distinct from "връх" (curve/step peak) |
| magnitude | модул | modulus of a quantity | distinct from amplitude |
| amplitude | амплитуда | oscillating-quantity peak | distinct from magnitude |
| peak value | пикова стойност | UI option | must not collapse with peak magnitude |
| peak magnitude | пиков модул | UI option | must not collapse with peak value |
| instantaneous | мигновен(а) | projection mode | "(φ=0)" qualifier added only where source carries it |
| Poynting vector | вектор на Пойнтинг | electromagnetic energy-flux vector | preserve the named physical quantity |
| solid angle | пространствен ъгъл | three-dimensional angular measure | distinct from a planar angle |
| net gain | нетно усилване | total-minus-mismatch gain | not "real-part gain" |
| viewer | изглед | observation direction or 3D view | not a person, speaker, or preview |
| flow / flow direction | поток / посока на потока | patch or current flow and its direction | preserve noun and directional form |
| total field | общо поле | combined electromagnetic field | distinct from an individual contribution |
| color | цвят | visible color | parent term for the color subsystem |
| color projection | цветова проекция | quantity that drives hue | distinct from scale family and palette kind |
| hue | тон | color-wheel angle | not reused for scale family |
| brightness | яркост | luminance channel | distinct from hue and magnitude scale |
| hue encoding / brightness encoding | кодиране на тон / кодиране на яркост | distinct internal enums | neither collapses to color projection |
| color scale | цветова скала | magnitude-to-color mapping | distinct from geometric scale |
| scale family / color tone | цветово семейство | transfer-curve family (Power/Log/Asinh/μ-law/Reinhard/Sigmoid/Identity) | one concept, one term; not "тон" |
| palette / palette kind | палитра / вид палитра | palette-layout enum | distinct from scale family and color projection |
| ramp / gradient | градация | palette kind or linear color strip | one term for both source spellings |
| gamma | гама | power-law exponent | technical transfer-curve parameter |
| knee | коляно | soft-knee bend point | dynamic-range control, not anatomy |
| softening | омекотяване | reduction of transfer-curve harshness | dynamic-range control sense |
| compression | компресия | dynamic-range reduction | distinct from data compression |
| contrast | контраст | separation between displayed levels | tone-control sense |
| dynamic range | динамичен диапазон | span between represented minimum and maximum | canonical signal and tone sense |
| floor | долен праг | minimum/lower clamp | not a room floor |
| envelope | обвивка | magnitude or amplitude envelope | signal boundary, not a physical wrapper |
| comet | комета | moving-crest overlay effect | not geometry (fuzzy-inheritance hazard, topic 20) |
| overlay (noun) | наслагване | an added visual layer | distinct from the verb "наслагвам" |
| animate / animation | анимиране / анимация | apply or name temporal visual motion | preserve verb and noun forms |
| animated / static | анимиран / статичен | dynamic versus phase-invariant categories | preserve the opposed adjectives |
| projection | проекция | color or geometry mapping | shared parent term for both qualified senses |
| scale | мащаб / мащабирам | display scale / action of scaling | preserve distinct noun and verb forms |
| wireframe | каркасен изглед | wire-mesh drawing mode | distinct from model wire geometry |
| identity | идентичност | no-op/passthrough transfer | distinct from unity (Smith-chart, "единица") |
| sentinel | сентинел | unreachable-case guard value | distinct from an ordinary data value |
| bins | интервали | discretization buckets | not "кошчета" (trash bins) |
| companding | компандиране | bounded logarithmic μ-law curve | canonical signal-processing sense |
| tone mapping | тонално пресъздаване | photographic tone-map transform | distinct from hue and scale family |
| renderer | рендерер | drawing backend | not "render engine" |
| shader | шейдър | graphics-program stage | established graphics term |
| allocation (memory) / managed allocator | заделяне на памет / управляван разпределител | memory allocation / allocator component | distinguish the operation from its manager |
| thread | нишка | compute thread | no collision with "проводник" (wire) in Bulgarian |
| widget | уиджет | interface element | established interface term |
| validation | валидация | the validation-tree feature | distinct from verification ("проверка") |
| batch mode | пакетен режим | non-interactive grouped processing mode | distinct from an interface session |
| fork (process) | форк | process creation by duplication | locked Cyrillic conditional-loanword form |
| deadlock | взаимно блокиране | threads waiting cyclically | concurrency failure sense |
| notifier | известител | component that reports a state change | technical component sense |
| token / operand / operator / arity | токен / операнд / оператор / арност | expression-parser lexical and operation terms | keep all four parser concepts distinct |
| override | заместване | supersede a value (SY symbol) | not overwrite ("презаписване") |
| swap | размяна | exchange two values or roles | distinct from replacement or overwrite |
| theme | оформление | interface or color theme | distinct from `тема` for topic or subject |
| noise / noise temperature | шум / шумова температура | electronic/thermal noise | not acoustic racket |
| efficiency | ефективност | useful-output to input-power ratio | electrical performance metric |
| interpolation | интерполация | estimation between sampled values | numerical-method sense |
| mnemonic | мнемоника | a card's code descriptor | not a memo/note; distinct from the hotkey mnemonic of topic 9 |
| degrees / deg | градуси / (deg) | freestanding prose term / retained parenthetical unit tag | keep the unit tag verbatim under topic 4 |
| diameter | диаметър | canonical loanword | native synonym not used |
| reflect / mirrors / reflection | отразяване / следване / отражение | geometry mirror operation / behavioral tracking / physical reflection | keep all three senses distinct |
| default(s) | стойност по подразбиране | fallback value or values | canonical interface fallback term |
| normalize / normalization | нормализирам / нормализация | scale to a defined reference / that operation | use natural Bulgarian derived forms |

- Add every source-domain concept not already inventoried to this table with
  one Bulgarian term, its intended technical sense, and the collision or
  consistency purpose the mapping serves.

## 11. Disambiguation policy

- The correct technical sense is chosen for each term from topic 10; no
  qualifier absent from the source is added, since program context already
  disambiguates.
- A qualifier is added only where the target term would otherwise be
  genuinely ambiguous.
- Accepted intra-domain homonyms are limited to `поле` for an electromagnetic
  field and a data field, and `мнемоника` for a card code and an accelerator
  letter; context selects the intended sense.
- No unresolved locative homonym remains: ground vs earth is resolved at
  topic 10 as two distinct terms.
- Gerund vs noun senses: "мащабиране" (scaling, verbal noun) is distinct from
  "мащаб" (a scale, noun).

## 12. Cross-catalog consistency

- Use one topic-10 term per concept throughout the catalog; introduce no
  synonym for a mapped concept.

| Concepts that remain distinct | Locked Bulgarian terms |
|---|---|
| polarity / polarization | полярност / поляризация |
| magnitude / amplitude | модул / амплитуда |
| peak value / peak magnitude | пикова стойност / пиков модул |
| ground / earth | земя / земна среда |
| load / charge | товар / заряд |
| gain / amplifier amplification / profit | антенно усилване / усилване на усилвател / печалба |
| electrical current / temporal current | ток / текущ |
| electrical charge / fee | заряд / такса |
| wire / cable or cord / compute thread | проводник / кабел / нишка |
| radiation pattern / template / far-field region | диаграма на излъчване / шаблон / далечно поле |
| excitation / emotional excitement | възбуждане / вълнение |
| node or antinode / numeric zero | възел или пучност / нула |
| scale family / hue / palette kind / color projection | цветово семейство / тон / вид палитра / цветова проекция |
| comet overlay / geometry | комета / геометрия |
| identity transfer / Smith-chart unity | идентичност / единица |
| renderer / render engine | рендерер / машина за рендериране |
| override / overwrite | заместване / презаписване |
| viewer / observer / speaker / preview | изглед / наблюдател / говорител / предварителен преглед |
| geometry reflect / behavioral tracking / physical reflection | отразяване / следване / отражение |
| model structure / construction | структура / строителство |
| interface theme / topic or subject | оформление / тема |
| validation tree / verification checks | валидация / проверка |
| net gain / real-part gain | нетно усилване / усилване на реалната част |
| electrical power / Power transfer family | мощност / степенна функция |

- Lock the topic-10 Cyrillic spellings of established technical loanwords;
  unify every outlier spelling to that form for catalog consistency.
- Catalog consistency overrides a competing locale form for `патч` and
  `земна равнина`; use only these locked forms for their mapped concepts.

## 13. Priority ordering

- Precedence: correct meaning, then interface convention, then catalog
  consistency, then disambiguation, then locale numeral form.
- Explicit override: the axis-letter uppercase rule (topic 6) overrides the
  sentence-case default for X, Y, Z.

## 14. Grammatical number

- Bulgarian translated literals distinguish singular and plural nouns,
  adjectives, participles, and finite verbs; make every agreeing form match
  the grammatical number of its head or subject.
- Use singular after the literal count `1` and plural after other counts,
  except where the masculine count form applies.
- After literal counts ending in `2`, `3`, or `4`, use the Bulgarian masculine
  count form for countable masculine nouns; use the ordinary plural where the
  count form is unavailable or the noun class does not permit it.

## 15. Grammatical agreement

- Adjectives and participles agree in gender and number with the head noun,
  including standalone labels whose head noun is implied.
- A standalone label's default gender follows the noun's own inherent
  grammatical gender (eg neuter for "-ане/-ене" verbal nouns, masculine or
  feminine per the noun used); no gender-neutral phrasing is invented.
- After the numerals 2-4 (and compounds ending in them), masculine nouns take
  the special count form (eg "два проводника", not "два проводници").
- Reflexive verbs retain the "се" particle in impersonal/passive status
  constructions (eg "избира се").
- Bulgarian has grammatical gender and number agreement; this topic applies
  in full.

## 16. Morphological derivation

- Borrowed technical verbs take the standard "-ирам" suffix family (eg
  "мащабирам", "нормализирам", "интерполирам"); no other verb-forming suffix
  is used for loanwords.
- Verbal nouns use the "-ане/-ене" suffix (eg "мащабиране", "възбуждане",
  "отразяване") for menu/section-header labels (topic 7).
- Prefer the established international EE loanword where one exists
  (импеданс, реактанс) over native coinage; prefer native multiword
  compounding for descriptive UI terms (диаграма на излъчване) over calque
  borrowing.
- Compounding strategy: multiword noun-phrase compounds ("noun + на + noun")
  are preferred over fused compounds for technical descriptive terms.

## 17. Preposition and sandhi selection

- Phonologically triggered sandhi, preposition alternation, elision, and
  contraction are not applicable; choose fixed Bulgarian prepositions by
  grammatical meaning, not by the following sound.

## 18. Card/record-label register

- Use the long title designator `Карта` followed by the retained mnemonic in
  dialog and editor titles, eg `Карта GW`.
- Use the short running-prose form consisting of the retained mnemonic plus
  the lowercase suffixed noun `редът`, eg `GW редът`; omit repeated `карта`
  where the record sense is clear.
- Write generic `карта` lowercase in running prose and capitalize it only at
  sentence start.
- Keep title and running-prose registers internally consistent; never
  cross-convert their long and short forms.

## 19. Multi-paragraph and whitespace fidelity

- Mirror every source paragraph break at the same position, preserving the
  distinction between a blank line and a single line break.
- Drop every trailing clause absent from the current source rather than
  carrying it from an inherited translation.
- Preserve source trailing newlines and punctuation exactly.
- Preserve semantic line breaks; add no line break used only for visual
  wrapping.
- Preserve the complete meaning in natural Bulgarian; never truncate it or
  use an unnatural abbreviation to fit an assumed display width.

## 20. Current-source fidelity

- Derive every Bulgarian literal from the complete current source text and
  its supplied context.
- Reuse an inherited translation only when its complete meaning agrees with
  the current source.
- Never inherit `комета` for `geometry` or `геометрия` for `comet`; these
  source concepts require their distinct topic-10 terms.

## 21. Script hygiene

- Bulgarian prose contains zero Latin homoglyphs in Cyrillic words; letters
  resembling Cyrillic forms are allowed only inside a retained topic-4 token
  whose exact spelling requires them.
- Translate plain foreign words into Bulgarian; retain genuine identifiers,
  symbols, units, and proper technical names exactly as topic 4 defines.
- Cyrillic-Latin adjacency is allowed only at a natural boundary between
  Bulgarian prose and an embedded retained token, with topic-1 spacing.

## 22. Rule-file scope hygiene

- Retain only current decisions that can alter the wording, Unicode
  characters, punctuation, capitalization, embedded tokens, mnemonic marker,
  or semantic whitespace of a Bulgarian translated literal.
- Omit catalog representation, serialization, headers, flags, validation
  workflow, rendering, fonts, shaping, layout, widget behavior, runtime
  formatting, sorting, search, display sizing, audit history, completion
  state, review metadata, provenance, and source citations.
- State each retained decision directly; include no implementation procedure,
  bibliography, correction history, or alternative policy source.

## 23. Section-disjointness declaration

- Script/orthography (topic 1), phrasing/structure (topics 2, 6, 7, 18), and
  address/formality register (topic 8) are non-overlapping: topic 1 governs
  writing-system mechanics only, topics 2/6/7/18 govern sentence and label
  construction only, topic 8 governs pronoun and politeness register only.

## 24. Developer/debug-string policy

- Translate user-facing controls, dialogs, tooltips, status messages, and
  errors into Bulgarian; their applicability is independent of review
  priority.
- Translate informational diagnostics and developer-facing debug strings into
  Bulgarian in a concise, neutral, technical register.
- Preserve every embedded identifier, function name, configuration key,
  symbol, and format specifier verbatim in every translated string family.
- Apply any established sibling-string terminology within the same subsystem;
  no subsystem overrides the Bulgarian translation policy.
- Review priority changes ordering only; it never permits an applicable
  user-facing, informational, or developer-facing literal to remain
  untranslated.
- Render the `BUG:` diagnostic prefix as "ДЕФЕКТ:", held distinct from the
  "ГРЕШКА:" prefix that carries ordinary failures; the two severities never
  share a word.
- Keep an identifier-plus-value diagnostic pair contiguous (eg
  `sweep_state=%d`); a qualifier precedes the whole pair and never stands
  between the identifier and its `=`.
