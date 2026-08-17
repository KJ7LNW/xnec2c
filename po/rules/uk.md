# uk translation rules

## 1. Script and orthography

- Locale scope: Ukrainian for Ukraine, written in the Ukrainian Cyrillic alphabet under the current normative orthography, for professional electrical and radio-frequency engineers, antenna designers, and amateur-radio operators.
- Required letters `і`, `ї`, `є`, and `ґ` are mandatory; do not strip their marks or replace them with look-alikes.
- Use precomposed Ukrainian letters; combining forms are not part of normal Ukrainian orthography.
- Use apostrophe `’` (`U+2019`) where Ukrainian orthography requires it; forbid ASCII apostrophe and modifier-letter look-alikes in prose.
- Textual joiners and positional letter forms are not applicable to Ukrainian Cyrillic.
- Ukrainian text runs left-to-right; retained technical tokens keep their source character order without manual direction controls.
- Letter-case distinction: present; casing rules governed by topic 6.
- Orthographic standard: current normative Ukrainian orthography, not Soviet-era Russified spelling.
- Inter-word spacing: single space, including between native text and embedded Latin/numeric tokens.
- Compound-formation: follow standard Ukrainian compounding orthography, hyphen for coordinate/appositive compounds, fused form for established compound nouns; no ad-hoc spaced source-language-style compounds.

## 2. Numerals in literals

- Use digits `0-9` for technical values in translated prose.
- Use a period as the decimal separator to keep literal technical values consistent across the catalog; use a space for thousands grouping.
- Keep formulas, examples, fixed defaults, and named mathematical or standards constants in source form, including their literal separators.
- Join a literal ordinal or index digit to its Ukrainian ending with a hyphen; treat the hyphen as an index marker, not a decimal separator.

## 3. Punctuation and quotation

- Native quotation marks: «...» for prose-level quoted text; embedded technical tokens (filenames, format specifiers) keep plain source quotes `"..."`, for consistency with topic 4's verbatim-token rule.
- Ukrainian reuses source comma, question mark, and exclamation mark unchanged; only opening/closing quote glyphs differ from source.
- Spacing: no space before colon or semicolon, one space after; no French-style spacing.
- Ellipsis: single `…` character, used consistently; dash forms (en/em) carried as used in source for ranges.
- Sentence-terminator policy: full sentences (dialogs, tooltips, confirmations) take a terminal period; short labels/fragments (buttons, field labels, menu items) omit terminal punctuation.
- Punctuation inside embedded technical runs (format specifiers, code snippets) stays in source form, unconverted.

## 4. Never-translate tokens

Treat every retained token as an exact, zero-failure invariant: preserve its characters, case, order, and internal punctuation; never translate or transliterate it.

- Keep NEC2 card mnemonics verbatim: `GW GA GH EX LD FR RP GE EN SP SM SC NE NH NT TL GN GD GC GX GR GS GM GF CP CM SY XQ EK KH PQ PT`.
- Keep unit symbols verbatim, including frequency, gain, impedance, power-ratio, temperature, conductivity, angle, and percent units: `Hz`, `kHz`, `MHz`, `dB`, `dBi`, `Ω`, `W`, `K`, `S/m`, `deg`, `%`.
- Keep figure-of-merit and parameter tokens verbatim: `VSWR`, S-parameter tokens such as `S11`, `Z`, `Z0`, `F/B`, and `G/Ta`.
- Keep file extensions and literal filenames verbatim.
- Keep every printf-style format specifier verbatim under topic 5.
- Keep embedded function names, variable names, configuration keys, and other code identifiers verbatim.
- Keep physical and mathematical symbol letters verbatim, including `E`, `H`, `Z`, `φ`, `γ`, and `k`; a Cyrillic look-alike changes the notation.
- Keep product, library, toolkit, chart, and tool names verbatim, including `NEC2`, `xnec2c`, `GSL`, `GTK`, `Cairo`, `OpenGL`, and `Smith`.
- Keep named transfer-function and algorithm terms verbatim: `Log`, `Asinh`, `μ-law`, `Reinhard`, `Sigmoid`, and `Identity`; translate the descriptive family name `Power` as `Степенева`.
- Translate conditional geometry loanwords consistently as `segment`→`сегмент`, `patch`→`патч`, and `tag`→`тег`; keep process term `fork` verbatim.
- Directionality needs no token-specific handling because Ukrainian text is left-to-right.

## 5. Format-specifier integrity

- Every specifier present in the source is preserved, same set, in every translated string.
- Default ordering matches source order; positional reordering (`%1$s`-style) allowed only where Ukrainian word order genuinely requires it.
- Prefer restructuring the sentence around fixed specifier positions over reordering specifiers, given Ukrainian's flexible word order.
- Numbers inside specifiers are never localized (no comma-decimal substitution inside `%f` output); topic 2's decimal rule does not reach into specifier output.

## 6. Capitalization and title-case

- Default casing: sentence case for labels, menu items, and titles; forbid source-language-style capitalize-every-word title case.
- Axis-letter casing: X/Y/Z axis letters keep source uppercase Latin form, never lowercased or Cyrillicized.
- Lowercase math/coordinate variables (`x`, `y`, `k`, `γ`) stay lowercase where the source has them lowercase.
- Keep coordinated option names in sentence case; capitalize an individual element only when it is a retained acronym or proper name.
- Generic card/record noun (`картка`) is lowercase mid-sentence, capitalized only at the title-case header position (topic 18); acronyms and proper nouns (`NEC2`, `VSWR`) retain source case throughout.

## 7. Interface register by string type

Use the shortest complete natural Ukrainian form; preserve all meaning and follow Ukrainian information order rather than source length or spoken-language ellipsis.

- Commands, buttons, and menu actions use an initial perfective infinitive followed by its object or qualifier; omit the subject and avoid gerunds and deverbal nouns.
- Field labels use concise nominative noun phrases with modifier-before-head order and retain a source colon; omit verbs and subjects.
- Dialogs and confirmations use full sentences with subject before predicate when expressed, natural verb-object order, and formal second-person forms only when direct address is necessary.
- Tooltips use complete declarative sentences, place the control or action before its consequence, and state the supplied reason a disabled control is unavailable.
- Status and error messages use impersonal or passive declarative clauses, present the event before its cause or remedy, and omit first- and second-person subjects.
- Prefer established compounds or short noun phrases over mechanical source-order calques; do not abbreviate a word unnaturally or omit a semantic component.
- Name user-visible domain entities as a generic Ukrainian noun followed by a retained designator or proper name; topic 8 governs personal names.
- Topic 24 governs developer and diagnostic strings.

## 8. Formality and address

- Ukrainian distinguishes formal `Ви` from informal `ти`; use capitalized `Ви` only for necessary direct address and never use `ти`.
- Commands use subjectless infinitives; dialogs use professional-neutral full sentences and prefer impersonal constructions over direct address.
- Realize formality through capitalized `Ви` and its plural verb agreement; commands carry no person marking.
- Avoid user-gender marking through impersonal or present-tense constructions; where grammatical gender, number, or animacy is required, agree with the named noun rather than an assumed user identity.
- Use inclusive occupational and group terms that remain natural in technical Ukrainian; omit unnecessary gendered forms.
- Honorifics are not applicable to interface text; use no title unless the source contains one.
- Use second person only in direct confirmations; otherwise use third-person, passive, or subjectless clauses.
- Preserve a personal name in its supplied order and spelling unless the source provides a localized form; Ukrainian interface policy adds no patronymic or honorific.
- Commands use infinitives; dialogs and confirmations use formal full sentences.
- Form confirmations as complete questions, eg `Ви впевнені, що хочете…?`.
- Forbid casual, slang, archaic, ceremonial, commercial, and promotional registers.

## 9. Accelerator/hotkey mnemonics

- Place underscore `_` immediately before the mnemonic letter inside the translated literal.
- Choose the mnemonic from the translated term itself; do not transliterate the source mnemonic letter.
- Use a native Cyrillic mnemonic directly; a separate parenthetical letter is not applicable.
- Preserve source presence exactly: add a mnemonic only where the source literal carries one.
- Keep sibling mnemonics distinct by moving `_` to another typable letter within the translated term.
- Prefer an unshifted common consonant and avoid letters that require a dead key or awkward input.

## 10. Domain lexicon

Locked term table, one row per Appendix A concept; four fields per row: concept, Ukrainian term, sense, purpose/hazard guarded.

### Electrical primitives

| Concept | Term | Sense | Purpose |
|---|---|---|---|
| current | струм | electrical current, A | not `поточний` (temporal) |
| charge | заряд | electrical charge, C | not billing/cargo |
| voltage | напруга | electric potential | correct technical sense |
| power (electrical) | потужність | radiated/dissipated W, power gain, power-flow | distinct from `Power` transfer-family name (`Степенева`) and math power-law (`степінь`) |
| impedance | імпеданс | complex Z | distinct from resistance/reactance |
| resistance | опір | real part of Z | distinct from impedance and load |
| reactance | реактивний опір | imaginary part of Z | distinct from resistance |
| inductance | індуктивність | established engineering sense | canonical catalog term |
| capacitance | ємність | established engineering sense | canonical catalog term |
| conductivity | провідність | material S/m | native term |
| admittance | адмітанс | admittance-matrix sense | distinct from impedance |
| load | навантаження | LD-card impedance load | not physical weight, not merged with charge |
| gain | підсилення | antenna directivity ratio, dB | not profit, not amplifier amplification |
| excitation | збудження | EM energy input/source | not emotional excitement |
| feedpoint | точка живлення | antenna feed point | correct technical sense |
| port | порт | excitation/S-parameter port | correct technical sense |
| radials | радіали | ground-plane radial wires, noun | distinct from adjective `радіальний` |

### Ground and earth

| Concept | Term | Sense | Purpose |
|---|---|---|---|
| ground / ground plane | заземлення | RF electrical reference plane and its plane, GN/GD cards, type, conductivity, effects, and model | one canonical electrical-reference term; not physical terrain |
| earth (physical medium) | ґрунт | terrain/noise-model earth, below-ground geometry | distinct from electrical ground |
| ground wave | земна хвиля | propagation term | distinct from the ground reference |

### Geometry primitives

| Concept | Term | Sense | Purpose |
|---|---|---|---|
| wire | дріт | thin conductor, GW element | not `кабель`; not `провід` (electrical lead/line) |
| segment | сегмент | NEC2 geometry subdivision | correct technical sense |
| patch | патч | NEC2 surface patch, SP/SM | kept as loanword, not translated |
| tag | тег | NEC2 geometry identifier | not a UI label or a card |
| card | картка | NEC2 input record | register in topic 18 |
| kernel | ядро | integral-equation/thin-wire kernel | not an OS kernel |
| cliff | уступ | two-medium ground-boundary type | not a fracture/break |
| structure | структура | antenna model geometry | not `конструкція` |
| model | модель | NEC model or noise-temperature model | correct technical sense |
| geometry | геометрія | the model geometry | correct technical sense |
| crossed | схрещені | transmission-line conductors crossed/reversed | not cut/severed |

### Field, pattern, viewer

| Concept | Term | Sense | Purpose |
|---|---|---|---|
| field (EM) | поле | near/total/E/H field | distinct from a UI data/config field, disambiguated by context (topic 11) |
| near field | ближнє поле | opposed spatial region | keep symmetric with far field |
| far field | дальнє поле | opposed spatial region | keep symmetric with near field |
| far-field contribution | внесок дальнього поля | per-direction contribution | not near-field animation |
| radiation | випромінювання | radiated emission | correct technical sense |
| radiation pattern | діаграма випромінювання | plotted directional response, catalog-wide | not a template/design, not `діаграма спрямованості` |
| gain pattern | діаграма підсилення | the gain radiation pattern | correct technical sense |
| polarization | поляризація | antenna/wave field orientation | false friend of polarity |
| polarity | полярність | sign (+/-) of a quantity | false friend of polarization |
| phase | фаза | established engineering sense | canonical catalog term |
| reference phase | опорна фаза | established engineering sense | canonical catalog term |
| frequency | частота | established engineering sense | canonical catalog term |
| wave / wavelength | хвиля / довжина хвилі | established engineering sense | canonical catalog term |
| standing wave | стояча хвиля | opposed pair with traveling wave | correct technical sense |
| traveling wave | біжуча хвиля | opposed pair with standing wave | correct technical sense |
| node | вузол | standing-wave zero | also carries the null-overlay sense |
| antinode | пучність | standing-wave maximum | also carries the peak-overlay sense |
| crest | гребінь | instantaneous wave apex, comet-head | distinct from a curve/step peak |
| magnitude | модуль | modulus of a quantity, `|Z|`, scalar | distinct from amplitude |
| amplitude | амплітуда | oscillating-quantity peak | distinct from magnitude |
| peak value | пікове значення | UI option, distinct from peak magnitude | must not collapse with peak magnitude |
| peak magnitude | піковий модуль | UI option, distinct from peak value | must not collapse with peak value |
| instantaneous | миттєве (значення) | projection mode | `(φ=0)` qualifier added only where source carries it |
| Poynting vector | вектор Пойнтінга | established engineering sense | canonical catalog term |
| solid angle | тілесний кут | established engineering sense | canonical catalog term |
| net gain | чисте підсилення | total-minus-mismatch gain | not `дійсне підсилення` (real-part gain) |
| viewer | огляд | observation direction or 3D viewing area, resolved by context | not observer, speaker, or preview |
| flow / flow direction | потік / напрям потоку | patch/current flow | correct technical sense |
| total field | сумарне поле | established engineering sense | canonical catalog term |

### Color, tone, animation subsystem

| Concept | Term | Sense | Purpose |
|---|---|---|---|
| color | колір | established engineering sense | canonical catalog term |
| color projection | проекція кольору | which quantity drives hue | one canonical term, distinct from hue, palette kind, and scale family |
| hue | відтінок | color-wheel angle | correct technical sense |
| brightness | яскравість | luminance channel | correct technical sense |
| hue encoding | кодування відтінку | internal enum | distinct from color projection |
| brightness encoding | кодування яскравості | internal enum | distinct from color projection |
| color scale | шкала кольору | magnitude-to-color scale | correct technical sense |
| scale family / color tone | сімейство шкали | transfer-curve family, Power/Log/Asinh/μ-law/Reinhard/Sigmoid/Identity | one concept, two source spellings, one term |
| palette | палітра | correct technical sense | distinct from scale family and color projection |
| palette kind | тип палітри | palette-layout enum | distinct from scale family and color projection |
| ramp / gradient | градієнт | a palette kind, linear color strip | correct technical sense |
| gamma | гамма | power-law exponent | correct technical sense |
| knee | коліно | soft-knee bend point | correct technical sense |
| softening | згладжування | dynamic-range | correct technical sense |
| compression | стиснення | dynamic-range | correct technical sense |
| contrast | контраст | established engineering sense | canonical catalog term |
| dynamic range | динамічний діапазон | established engineering sense | canonical catalog term |
| floor | нижня межа | minimum/lower clamp, brightness/dB floor | not a room floor |
| envelope | обвідна | magnitude/amplitude envelope | correct technical sense |
| comet | комета | moving-crest overlay effect | distinct from geometry |
| overlay (noun) | накладання (шар) | an added visual layer | distinct from the verb `накладати` |
| animate / animation | анімувати / анімація | established engineering sense | canonical catalog term |
| animated / static | анімований / статичний | category-header adjectives, dynamic vs phase-invariant | preserve the opposed category meanings |
| projection | проекція | color or geometry projection | correct technical sense |
| scale | шкала (noun) / масштабувати (verb) | correct technical sense | verb and noun kept distinct |
| wireframe | каркас | wire-mesh render mode | correct technical sense |
| identity | тотожність | no-op/passthrough transfer | distinct from unity, `одиниця` (Smith-chart) |
| sentinel | вартове значення | unreachable-case guard value | correct technical sense |
| bins | кошики | discretization buckets | correct technical sense |
| companding | компандування | bounded log curve, μ-law | correct technical sense |
| tone mapping | тональне відображення | photographic tone-map | correct technical sense |

### Render and compute

| Concept | Term | Sense | Purpose |
|---|---|---|---|
| renderer | рендерер | drawing backend | not `рушій рендерингу` (render engine) |
| shader | шейдер | established engineering sense | canonical catalog term |
| allocation (memory) | виділення пам’яті | allocation event | correct technical sense |
| managed allocator | керований розподільник | allocator/report | correct technical sense |
| thread | потік (виконання) | compute thread | homonym-adjacent to `дріт` (wire), disambiguated by distinct term choice |
| widget | віджет | UI element | correct technical sense |
| validation | валідація | validation-tree feature | distinct from `перевірка` (verification checks) |
| batch mode | пакетний режим | established engineering sense | canonical catalog term |
| fork (process) | fork | process fork | kept verbatim (topic 4) |
| deadlock | взаємне блокування | established engineering sense | canonical catalog term |
| notifier | сповіщувач | established engineering sense | canonical catalog term |
| token | токен | expression-parser term | correct technical sense |
| operand | операнд | expression-parser term | correct technical sense |
| operator | оператор | expression-parser term | correct technical sense |
| arity | арність | expression-parser term | correct technical sense |
| override | перевизначення | supersede a value, SY symbol | not `перезапис` (overwrite) |
| swap | обмін | exchange | correct technical sense |
| theme | тема | UI/color theme | not `предмет`/`тематика` (topic/subject) |

### Metrics and miscellaneous

| Concept | Term | Sense | Purpose |
|---|---|---|---|
| noise / noise temperature | шум / шумова температура | electronic/thermal noise | not acoustic racket |
| efficiency | ефективність | established engineering sense | canonical catalog term |
| interpolation | інтерполяція | established engineering sense | canonical catalog term |
| mnemonic | мнемоніка | a card's code descriptor | not a memo/note |
| degrees / deg | градуси / `deg` | freestanding prose term vs retained parenthetical unit tag | keep the unit tag verbatim under topic 4 |
| diameter | діаметр | canonical loanword | one choice, no native synonym coined |
| reflect | віддзеркалити (geometry) / віддзеркалює (behavioral tracking) / відбиття (physics) | three distinct senses | never collapsed to one term |
| default(s) | типове значення | fallback value | one canonical term |
| normalize / normalization | нормалізувати / нормалізація | correct technical sense | translated, not transliterated, native term exists |

Add no qualifier absent from the current source literal; use program context for disambiguation under topic 11.

## 11. Disambiguation policy

- The correct technical sense is chosen for each ambiguous term per the topic 10 table.
- No qualifier absent from the source is added; program context already disambiguates.
- A qualifier is added only where the Ukrainian term would otherwise be genuinely ambiguous.
- Accepted intra-domain homonym: `поле` covers both the EM-field sense and the UI data/config-field sense, disambiguated by surrounding context, no separate coinage.
- Ukrainian has no unresolved locative homonym collision in this lexicon; keep `потік` for compute thread distinct from `дріт` for wire.
- Gerund vs noun overload: `scale` resolves to `масштабувати` (verb) vs `шкала` (noun), never one term for both.

## 12. Cross-catalog consistency

- One term per concept, catalog-wide; reuse established lexicon rather than coining a synonym.
- Appendix C false-friend pairs, each side given a distinct term:

| Pair | Distinct terms |
|---|---|
| polarity / polarization | полярність / поляризація |
| magnitude / amplitude | модуль / амплітуда |
| peak value / peak magnitude | пікове значення / піковий модуль |
| ground / earth | заземлення / ґрунт |
| load / charge | навантаження / заряд |
| gain / amplification / profit | підсилення / ампліфікація / прибуток |
| current / temporal current | струм / поточний |
| charge / billing | заряд / плата |
| wire / cable / thread | дріт / кабель / потік |
| radiation pattern / template / far field | діаграма випромінювання / шаблон / дальнє поле |
| excitation / excitement | збудження / хвилювання |
| node / antinode / generic null | вузол / пучність / нуль |
| scale family / hue / palette kind / color projection | сімейство шкали / відтінок / тип палітри / проекція кольору |
| comet / geometry | комета / геометрія |
| identity / unity | тотожність / одиниця |
| renderer / render engine | рендерер / рушій рендерингу |
| override / overwrite | перевизначення / перезапис |
| viewer / observer / speaker / preview | огляд / спостерігач / динамік / попередній перегляд |
| reflect / mirrors / reflection | віддзеркалити / віддзеркалює / відбиття |
| structure / construction | структура / конструкція |
| theme / topic-subject | тема / предмет |
| validation / verification | валідація / перевірка |
| net gain / real-part gain | чисте підсилення / дійсне підсилення |
| power / Power (transfer family) | потужність / Степенева |

- Lock loanword and native-term decisions catalog-wide: use `дріт` for wire and `патч`, `сегмент`, and `тег` for their geometry concepts.
- Unify spelling and inflection to each canonical topic 10 term; admit no minority outlier.
- Catalog consistency overrides locale numeral preference only for the period decimal separator fixed in topic 2; correct technical meaning remains the higher priority.

## 13. Priority ordering

- Precedence chain: correct meaning, then interface convention, then catalog consistency, then disambiguation, then locale numeral form.
- Explicit override rulings on record: decimal-point retention (`0.5`, `48.16 dB`) overrides comma-decimal, catalog consistency over locale numeral form; `діаграма випромінювання` overrides `діаграма спрямованості` in staged tooltips, catalog consistency over local phrasing preference; `дріт` overrides `провід` for wire, catalog consistency over an equally valid synonym.

## 14. Grammatical number

- Ukrainian translated literals distinguish singular, paucal, and plural forms.
- Use singular after counts ending in `1` except those ending in `11`; use the paucal nominative plural after counts ending in `2`-`4` except `12`-`14`; use the genitive plural after all other counts.
- Make adjectives, participles, and finite verbs agree with the selected noun number and case.

## 15. Grammatical agreement

- Adjectives and participles take gender/number concord with the head noun, including standalone labels whose head noun is implied.
- Numerals above one govern genitive-plural/paucal noun forms per standard Ukrainian numeral-noun agreement (e.g. `2 сегменти` vs `5 сегментів`); apply wherever a counted noun appears.
- Reflexive particle `-ся` is retained wherever the verb's lexical form requires it (e.g. `з’являється`).
- A standalone label resolves its default gender from the referenced Ukrainian noun (e.g. an option label agrees with the implied noun's own gender); default to masculine as the unmarked technical form when no referenced noun is identifiable.

## 16. Morphological derivation

- Borrowed technical verbs take the standard `-увати`/`-ювати` suffix (e.g. `масштабувати`, `компандувати`); non-standard calques forbidden.
- Verbal nouns take the standard `-ння` suffix (e.g. `масштабування`, `накладання`, `кодування`).
- Prefer an established native or already-assimilated loanword over a freshly coined calque; compounding follows topic 1's fused/hyphenated rule.

## 17. Preposition and sandhi selection

- The `у`/`в`, `і`/`й`, and `з`/`із`/`зі` euphonic pairs alternate by the preceding word's final sound, consonant favors `у`/`і`/`зі`, vowel favors `в`/`й`; apply throughout prose strings.
- No elision/contraction rule applies beyond the alternation above.

## 18. Card/record-label register

- Dialog and editor titles use the fixed designator form `Картка <MNEMONIC>` (e.g. `Картка GW`).
- Running prose in messages uses the distinct lowercase inline form `картка <mnemonic>`, never the title-case form.
- Generic noun `картка` stays lowercase mid-sentence, capitalized only at the title-case header position; short form `картка` is default, long form `вхідна картка NEC2` reserved for first introduction or help text.
- Each register stays internally consistent and is never cross-converted between title and running-prose form.

## 19. Multi-paragraph and whitespace fidelity

- Mirror source paragraph breaks at the same positions, preserving the distinction between blank lines and single line breaks.
- Drop clauses absent from the current source rather than retaining inherited text.
- Preserve source trailing newlines and punctuation exactly.
- Preserve semantic line breaks; add no line break solely for visual wrapping.
- Preserve complete meaning and natural words without truncation or display-driven abbreviation.

## 20. Current-source fidelity

- Derive each translation from the current source literal and its supplied context.
- Reuse inherited wording only when its complete meaning agrees with the current source.
- Do not inherit prior wording for memory-operation names, validation and excitation terms, color and palette diagnostics, current and charge labels, far-field contributions, animation overlays, load-failure diagnostics, or polarity and polarization; these concept families have distinct locked meanings.

## 21. Script hygiene

- Require zero Latin/Cyrillic homoglyph substitutions in Ukrainian prose, including mixed-script forms built from `а/a`, `е/e`, `о/o`, `р/p`, `с/c`, `х/x`, or `у/y`; allow foreign-script characters only inside a retained topic 4 token whose exact spelling requires them.
- Translate plain foreign prose into Ukrainian; keep genuine identifiers, units, symbols, and names under topic 4.

## 22. Rule-file scope hygiene

- Keep only current decisions that can alter wording, Unicode characters, punctuation, capitalization, embedded tokens, mnemonic markers, or semantic whitespace in a translated literal.
- Omit catalog representation, serialization, headers, flags, validation workflow, rendering, fonts, shaping, layout, widget behavior, runtime formatting, sorting, search, display sizing, audit history, completion state, review metadata, provenance, and source citations.
- State each retained decision directly; include no implementation procedure, bibliography, correction history, or alternative policy source.

## 23. Section-disjointness declaration

- Script mechanics (topic 1), punctuation (topic 3), phrasing/structure (topics 6, 7, 10-12, 15-18), and address register (topic 8) are non-overlapping axes; each governs a distinct property of a string, and each concept lands in exactly one section, cross-referenced rather than duplicated.

## 24. Developer/debug-string policy

- Translate user-facing controls, dialogs, tooltips, status messages, and errors into Ukrainian; review priority never permits an applicable user-facing literal to remain untranslated.
- Translate informational logs, notices, reports, and progress messages into concise professional Ukrainian.
- Translate developer-facing assertions, diagnostics, parser messages, and unreachable-case reports into terse technical Ukrainian.
- A subsystem-wide established diagnostic family can fix wording for its sibling strings; apply that wording consistently without changing the translation policy.
- Preserve every embedded identifier, function name, format specifier, unit, symbol, and retained token under topics 4 and 5 in every string family.
- Render the `BUG:` diagnostic prefix as "ДЕФЕКТ:", held distinct from the "ПОМИЛКА:" prefix that carries ordinary failures; the two severities never share a word.
- Keep an identifier-plus-value diagnostic pair contiguous (eg `sweep_state=%d`); a qualifier precedes the whole pair and never stands between the identifier and its `=`.
