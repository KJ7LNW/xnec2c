# ru translation rules

## 1. Script and orthography
- Scope: Russian for the Russian Federation, written in modern post-1918 Cyrillic orthography for professional electrical and RF engineers, antenna designers, and amateur-radio operators.
- Use all 33 Russian letters; retain «ё» where required by the chosen word and never fold it to «е», so spelling and meaning remain exact.
- Use precomposed Cyrillic characters; combining marks are not applicable to ordinary Russian spelling.
- Apostrophes and textual joiners are not part of Russian orthography; introduce no look-alike marks.
- Write left-to-right; embedded retained tokens remain left-to-right without manual direction marks.
- Preserve upper/lower case distinction and apply topic 6 casing rules.
- Use one inter-word space and one space between Russian text and embedded Latin or numeric tokens.
- Form compounds according to established Russian technical spelling: fuse lexicalized compounds, hyphenate prescribed mixed or appositional forms, and space noun phrases.

## 2. Numerals in literals
- Use digits `0-9` for technical values in translated prose.
- Use comma as the decimal separator and a space as the thousands separator in literal Russian prose, eg `10 000,5`.
- Retain source form in formulas, examples, fixed defaults, and named mathematical or standards constants, so literal technical tokens remain exact.
- Form ordinals and adjectival indices with a hyphenated suffix, eg `1-й`; keep index separators distinct from decimal commas.

## 3. Punctuation and quotation
- Use «ёлочки» for quoted prose and interface text; retain source quotes around embedded filenames, code, and technical tokens for token fidelity.
- Use Russian comma, question mark, and exclamation mark conventions; omit an enumerative comma where Russian syntax does not require one.
- Put no space before a colon, semicolon, or terminal mark and one space after a colon or semicolon.
- Use `…` for an ellipsis; preserve a source dash as `—` when it marks a prose aside.
- End full sentences with `.`; omit a terminator from labels and fragments.
- Keep punctuation inside embedded technical runs in source form.

## 4. Never-translate tokens
- Preserve every retained token exactly; translation, transliteration, character substitution, and case change are zero-failure violations.
- Keep NEC2 card mnemonics verbatim: `GW GA GH EX LD FR RP GE EN SP SM SC NE NH NT TL GN GD GC GX GR GS GM GF CP CM SY XQ EK KH PQ PT`.
- Keep unit symbols and figure-of-merit tokens verbatim: `Hz kHz MHz GHz dB dBi Ω V A W S/m K ° % VSWR S-parameters Z Z0 Zo F/B G/T`.
- Keep file extensions and format specifiers verbatim.
- Keep embedded identifiers, function names, variable names, and configuration keys verbatim in developer strings.
- Keep physical and mathematical symbol letters verbatim, including `E H Z jX φ γ μ`.
- Keep proper technical names verbatim: `xnec2c NEC2 GTK OpenGL Smith Reinhard Sigmoid Asinh μ-law`; translate the descriptive transfer-family name `Power` as «Степенная» to distinguish it from electrical power.
- Use the locked Russian loanwords «патч», «тег», «сегмент», and «форк» for conditional geometry and process terms; these are translated terms, not retained tokens.
- Apply topic 1 directionality to every retained token.

## 5. Format-specifier integrity
- Preserve exactly the source set and count of format specifiers; omission, addition, mutation, and duplication are zero-failure violations.
- Keep source order by default; use positional forms only where natural Russian grammar requires reordering.
- Restructure Russian wording around fixed specifier positions before using positional reordering.
- Keep every number inside a specifier unchanged.

## 6. Capitalization and title-case
- Use sentence case for labels, menu items, and titles; capitalize only the first word plus proper nouns and acronyms.
- Keep axis letters `X Y Z` uppercase and lowercase mathematical or coordinate variables `x y z` lowercase.
- Coordinated option names take no special element-by-element capitalization.
- Write the generic card noun «карта» lowercase except at sentence start; preserve acronym and proper-name casing.

## 7. Interface register by string type
- Commands, buttons, and menu actions: use a terse perfective infinitive with the action first and the object after it; omit the subject, eg «Сохранить модель».
- Field labels: use a concise noun phrase in normal modifier-before-head order and retain a source colon.
- Dialog statements: use complete declarative sentences with subject before predicate unless natural Russian information structure requires an impersonal predicate.
- Confirmations: use a complete formal question; place the decision context before the final requested action when that order improves clarity.
- Tooltips: use complete concise sentences in action-then-condition order; when the source describes an unavailable control, include its reason.
- Status and error messages: use impersonal or passive declarative clauses; present the affected entity before its state and omit first-person narration.
- User-visible domain entities: put agreed modifiers before the topic-10 head term unless established Russian engineering terminology fixes another order.
- Across string types, use the shortest complete natural engineering wording; preserve all meaning, avoid unnatural abbreviations, and do not imitate source length.
- Developer and debug strings follow topic 24.

## 8. Formality and address
- Use formal capitalized «Вы» only when direct address is required; prohibit «ты».
- Prefer impersonal subject omission; commands use infinitives, while dialogs use full formal sentences.
- Avoid gendered forms for the user; use natural plural or impersonal constructions where grammatical gender or number is required.
- Use inclusive professional role terms that do not presume gender; retain grammatical agreement with the selected noun.
- Honorifics are not applicable to interface text; do not add them.
- Use second person only in necessary direct address; personal names retain their supplied order and form.
- Russian has no formal verb-ending opposition in this interface register; formality is carried by pronoun and construction choice.
- Form confirmations as full formal questions, eg «Вы уверены, что хотите продолжить?».
- Exclude casual, slang, commercial, archaic, and ceremonially over-formal registers.

## 9. Accelerator/hotkey mnemonics
- Preserve a source mnemonic marker as `_` immediately before a typable Cyrillic letter within the translated term.
- Choose the mnemonic from the translated term, never from source-letter transliteration.
- A separate parenthetical mnemonic is not applicable because Cyrillic letters serve directly.
- Add no mnemonic when the source has none.
- Resolve collisions with another typable letter from the translated term; avoid `Ъ Ь Ы Й` where a more accessible letter exists.

## 10. Domain lexicon
- Read every mapping as four fields: source concept key; locked Russian term; the stated technical sense, or the source-domain sense where no qualifier follows; and the stated hazard, or catalog consistency where no hazard follows.
- Use each locked term only for its mapped sense; the row purpose is to preserve correct engineering meaning, prevent the stated collision, and keep catalog wording consistent.

Electrical primitives:
- `current` — ток; electrical current (A), not temporal "recent".
- `charge` — заряд; electrical, not billing/fee.
- `voltage` — напряжение.
- `power (electrical)` — мощность (Вт); distinct from `power` transfer-family name → «Степенная».
- `impedance` — импеданс; distinct from resistance/reactance.
- `resistance` — активное сопротивление; real part of Z.
- `reactance` — реактивное сопротивление; imaginary part of Z.
- `inductance` — индуктивность.
- `capacitance` — ёмкость.
- `conductivity` — проводимость; material S/m.
- `admittance` — адмиттанс; distinct from импеданс and from проводимость.
- `load` — нагрузка; LD-card impedance, not weight; distinct from заряд.
- `gain` — усиление; antenna directivity ratio, accepted homonym with amplifier-gain sense, context disambiguates; not profit.
- `excitation` — возбуждение; EM energy input, accepted homonym with emotional-excitement sense, technical context disambiguates.
- `feedpoint` — точка питания.
- `port` — порт; excitation/S-parameter port.
- `radials` — радиалы (noun); distinct from adjective «радиальный».

Ground and earth:
- `ground` / `ground plane` — земля / плоскость земли; electrical reference plane, never soil.
- `earth (physical medium)` — грунт; terrain/noise-model earth, distinct from электрическая земля.
- `ground wave` — земная волна; propagation term, distinct from the ground reference.

Geometry primitives:
- `wire` — провод; not cable/cord.
- `segment` — сегмент.
- `patch` — патч; transliterated, locked.
- `tag` — тег; not a UI label or a card.
- `card` — карта; register in topic 18.
- `kernel` — ядро; thin-wire kernel, not an OS kernel.
- `cliff` — уступ; two-medium ground-boundary type, not a fracture.
- `structure` — структура; the antenna model, never «конструкция».
- `model` — модель.
- `geometry` — геометрия.
- `crossed` — перекрёстный; conductors crossed/reversed, not cut.

Field, pattern, viewer:
- `field (EM)` — поле; accepted homonym with a UI data field, context disambiguates.
- `near field` / `far field` — ближнее поле / дальнее поле.
- `far-field contribution` — вклад в дальнее поле.
- `radiation` — излучение.
- `radiation pattern` — диаграмма направленности; not a template/design.
- `gain pattern` — диаграмма усиления.
- `polarization` — поляризация; wave/antenna field orientation.
- `polarity` — полярность; sign of a quantity, never «поляризация».
- `phase` — фаза.
- `reference phase` — опорная фаза.
- `frequency` — частота.
- `wave` / `wavelength` — волна / длина волны.
- `standing wave` / `traveling wave` — стоячая волна / бегущая волна.
- `node` / `antinode` — узел / пучность; standing-wave sense, also serves the null/peak overlay sense.
- `crest` — гребень; instantaneous wave apex, distinct from a curve/step peak.
- `magnitude` — модуль; distinct from амплитуда.
- `amplitude` — амплитуда; oscillating-quantity peak.
- `peak value` — пиковое значение; distinct UI option from `peak magnitude` — пиковый модуль.
- `instantaneous` — мгновенное; add «(φ=0)» qualifier only where the source carries it.
- `Poynting vector` — вектор Пойнтинга.
- `solid angle` — телесный угол.
- `net gain` — суммарное усиление; distinct from real-part gain.
- `viewer` — точка наблюдения (observation direction) / окно просмотра (3D view widget); never observer/speaker/preview.
- `flow` / `flow direction` — поток / направление потока.
- `total field` — суммарное поле.

Color, tone, animation subsystem:
- `color` — цвет.
- `color projection` — цветовая проекция; hue-source selection, distinct from `scale family`.
- `hue` — оттенок.
- `brightness` — яркость.
- `hue encoding` / `brightness encoding` — кодирование оттенка / кодирование яркости; distinct enums.
- `color scale` — цветовая шкала.
- `scale family` / `color tone` — семейство шкал; one concept, one term (Power/Log/Asinh/μ-law/Reinhard/Sigmoid/Identity).
- `palette` / `palette kind` — палитра / тип палитры.
- `ramp` / `gradient` — градиент.
- `gamma` — гамма.
- `knee` — излом; soft-knee bend point.
- `softening` — смягчение.
- `compression` — сжатие; dynamic-range.
- `contrast` — контраст.
- `dynamic range` — динамический диапазон.
- `floor` — нижний предел; brightness/dB floor, not a room floor.
- `envelope` — огибающая.
- `comet` — комета; moving-crest overlay effect, not geometry — known fuzzy-inheritance hazard, see topic 20.
- `overlay (noun)` — оверлей; distinct from the verb «накладывать».
- `animate` / `animation` — анимировать / анимация.
- `animated` / `static` — анимировано / статично.
- `projection` — проекция; color or geometry projection.
- `scale` — масштабировать (verb) / масштаб (noun).
- `wireframe` — каркасный.
- `identity` — тождественное; no-op transfer, distinct from `unity` — единичное (Smith-chart).
- `sentinel` — сигнальное значение; unreachable-case guard.
- `bins` — корзины; discretization buckets.
- `companding` — компандирование; bounded log curve (μ-law).
- `tone mapping` — тональное отображение; photographic tone-map.

Render and compute:
- `renderer` — рендерер; never «движок рендеринга».
- `shader` — шейдер.
- `allocation (memory)` / `managed allocator` — выделение памяти / менеджер распределения памяти.
- `thread` — поток; accepted homonym with `flow`, context disambiguates.
- `widget` — виджет.
- `validation` — валидация; distinct from verification — проверка.
- `batch mode` — пакетный режим.
- `fork (process)` — форк; transliterated per topic 4.
- `deadlock` — взаимоблокировка.
- `notifier` — уведомитель.
- `token` / `operand` / `operator` / `arity` — токен / операнд / оператор / арность.
- `override` — переопределение; supersede a value, never «перезапись».
- `swap` — обмен.
- `theme` — оформление; UI/color theme, never «тема» (topic/subject).

Metrics and miscellaneous:
- `noise` / `noise temperature` — шум / шумовая температура; electronic/thermal, context excludes acoustic sense.
- `efficiency` — эффективность / КПД (ratio sense).
- `interpolation` — интерполяция.
- `mnemonic` — мнемоника; a card's code descriptor, not a memo.
- `degrees` / `deg` — градусы (freestanding axis/prose) vs «(град.)» (parenthetical unit tag, same as other unit tags).
- `diameter` — диаметр; canonical loanword, sole choice.
- `reflect` — отразить (geometry mirror) / повторяет (behavioral "mirrors …") / отражение (physics); three distinct senses.
- `default(s)` — по умолчанию.
- `normalize` / `normalization` — нормализация; native-formed term, not transliterated.

## 11. Disambiguation policy
- Correct technical sense is chosen per ambiguous term from context (electromagnetic simulator domain).
- A qualifier absent from the source is never added; program context already disambiguates.
- A qualifier is added only where the target term would otherwise be genuinely ambiguous, e.g. drop «электрический» from `ground`/`current` unless the sentence would read ambiguously without it.
- Accepted intra-domain homonyms: `поток` (flow/thread), `поле` (EM field/UI field), `усиление` (antenna gain/amplifier gain), `возбуждение` (excitation/emotional-excitement, technical context only).
- No locative homonym collision identified in this catalog.
- `возбуждение` and `излучение` cover both the act and the resulting state; no separate gerund/noun split required.

## 12. Cross-catalog consistency
- Reuse one topic-10 term per concept; introduce no competing synonym.
- Keep `polarity` «полярность» distinct from `polarization` «поляризация».
- Keep `magnitude` «модуль» distinct from `amplitude` «амплитуда»; keep `peak value` «пиковое значение» distinct from `peak magnitude` «пиковый модуль».
- Keep electrical `ground` «земля» distinct from physical `earth` «грунт».
- Keep impedance `load` «нагрузка» distinct from electrical `charge` «заряд» and billing «плата».
- Keep antenna `gain` «усиление» distinct from amplifier action «усиление сигнала» where a qualifier is required and from profit «прибыль».
- Keep electrical `current` «ток» distinct from temporal «текущий»; keep electrical `charge` «заряд» distinct from billing «плата».
- Keep conductor `wire` «провод» distinct from cable «кабель», cord «шнур», and compute `thread` «поток».
- Keep `radiation pattern` «диаграмма направленности» distinct from template «шаблон» and `far field` «дальнее поле».
- Keep electromagnetic `excitation` «возбуждение» distinct from emotional «волнение».
- Keep standing-wave `node` «узел» and `antinode` «пучность» distinct from generic zero «нуль».
- Keep `scale family` and `color tone` «семейство шкал», `hue` «оттенок», `palette kind` «тип палитры», and `color projection` «цветовая проекция» distinct.
- Keep overlay `comet` «комета» distinct from `geometry` «геометрия».
- Keep transfer `identity` «тождественное» distinct from Smith-chart unity «единичное».
- Keep backend `renderer` «рендерер» distinct from render engine «движок рендеринга».
- Keep `override` «переопределение» distinct from overwrite «перезапись».
- Keep observation `viewer` «точка наблюдения» and view-widget `viewer` «окно просмотра» distinct from observer «наблюдатель», speaker «докладчик», and preview «предпросмотр».
- Keep geometry `reflect` «отразить», behavioral `mirrors` «повторяет», and physical `reflection` «отражение» distinct.
- Keep model `structure` «структура» distinct from construction «конструкция»; keep UI `theme` «оформление» distinct from subject «тема».
- Keep tree-feature `validation` «валидация» distinct from verification checks «проверка».
- Keep `net gain` «суммарное усиление» distinct from real-part gain «усиление по действительной части».
- Keep electrical `power` «мощность» distinct from transfer-family `Power` «Степенная».
- Lock loanword spellings «патч», «тег», «сегмент», «адмиттанс», «порт», and «форк»; unify spelling outliers to these forms.
- Let correct meaning and established catalog consistency override locale numeral preference only for the literal exceptions in topic 2 and the locked term «структура».

## 13. Priority ordering
- Precedence: correct meaning, then interface convention, then catalog consistency, then disambiguation, then locale numeral form.
- Explicit overrides: decimal-literal consistency (topic 2) outranks locale-comma preference; `структура`/«конструкция» precedent outranks the general noun choice.

## 14. Grammatical number
- Use singular, paucal genitive singular, and plural genitive forms according to the literal count.
- Use nominative singular after counts ending in `1` except `11`; genitive singular after `2-4` except `12-14`; use genitive plural elsewhere.
- Make adjectives, participles, and finite verbs agree with the selected noun number and sentence construction.

## 15. Grammatical agreement
- Make adjectives and participles agree in gender, number, and case with the head noun, including an implied head noun in a standalone label.
- Apply the count-governed case forms from topic 14.
- Retain «-ся» where the selected verb is reflexive by meaning, eg «вычисляется».
- Derive a standalone label's default gender from its chosen Russian head noun.

## 16. Morphological derivation
- Form established borrowed technical verbs with `-ировать`, eg «интерполировать»; do not invent competing verb families.
- Form verbal nouns with the established `-ция` or `-ание` pattern selected by the verb, eg «масштабирование».
- Prefer an established native technical term; use a loanword only where topics 4 and 10 lock it.
- Form compounds under topic 1 rather than applying one universal hyphen rule.

## 17. Preposition and sandhi selection
- Phonologically conditioned preposition forms, elision, contraction, and sandhi are not applicable; select prepositions lexically with their governed case.

## 18. Card/record-label register
- Use the fixed capitalized designator «Карта GW» in dialog and editor titles.
- Use the lowercase case-inflected form in running prose, eg «в карте GW».
- Use the mnemonic alone as the short form only after the card context is established.
- Keep title and running-prose registers internally consistent and do not cross-convert them within one literal.

## 19. Multi-paragraph and whitespace fidelity
- Mirror source paragraph breaks at the same positions, preserving the distinction between blank lines and single line breaks.
- Remove a trailing clause removed from the current source rather than inheriting stale wording.
- Carry source trailing newlines and punctuation exactly.
- Preserve semantic line breaks; introduce no line break for visual wrapping.
- Preserve complete meaning; never truncate or abbreviate wording for an assumed display constraint.

## 20. Current-source fidelity
- Derive every translation from the current source literal and supplied context.
- Reuse an inherited translation only when its complete meaning agrees with the current source.
- Treat prior mappings between «комета» and «геометрия», or between «полярность» and «поляризация», as unsafe because each pair names distinct concepts.

## 21. Script hygiene
- Forbid Cyrillic homoglyphs inside retained Latin tokens and stray Latin homoglyphs inside Russian prose; retained tokens whose spelling requires Latin characters are the only exception.
- Require zero character substitutions in NEC2 mnemonics, units, identifiers, and proper technical names.
- Translate plain foreign prose; preserve genuine identifiers, units, and topic-4 tokens verbatim.

## 22. Rule-file scope hygiene
- Keep only current decisions that can alter wording, Unicode characters, punctuation, capitalization, embedded tokens, mnemonic markers, or semantic whitespace in a translated literal.
- Omit catalog representation, serialization, headers, flags, workflow, rendering, fonts, shaping, layout, widget behavior, runtime formatting, sorting, search, display sizing, process metadata, audit history, completion state, provenance, and citations.
- State each decision directly; include no implementation procedure, bibliography, correction history, or alternative policy source.

## 23. Section-disjointness declaration
- Topics 1-6, 9, and 21 govern script and token mechanics; topics 7, 10-20, and 22 govern wording and structure; topic 8 governs address register; topic 24 governs source-string families.
- Keep these axes non-overlapping so each decision has one authority.

## 24. Developer/debug-string policy
- Translate user-facing controls, dialogs, tooltips, status messages, and errors into Russian; priority never permits an applicable user-facing literal to remain untranslated.
- Translate informational diagnostics intended for operators into concise technical Russian.
- Keep developer-facing debug and low-level internal diagnostics in source form unless one subsystem has an established translated sibling family; follow that family consistently.
- Preserve identifiers, function names, format specifiers, and all topic-4 tokens verbatim in every string family.
- Keep translated diagnostics terse, impersonal, and technical; review priority changes ordering only, never token-preservation rules.
- Render the `BUG:` diagnostic prefix as "ДЕФЕКТ:", held distinct from the "ОШИБКА:" prefix that carries ordinary failures; the two severities never share a word.
- Keep an identifier-plus-value diagnostic pair contiguous (eg `sweep_state=%d`); a qualifier precedes the whole pair and never stands between the identifier and its `=`.
