# sr translation rules

## 1. Script and orthography

- Scope: Serbian for Serbia, modern ekavian orthography in the 30-letter Vuk Karadžić Cyrillic alphabet, for professional electrical and radio-frequency engineers, antenna designers, and amateur-radio operators.
- Use Cyrillic exclusively; never switch prose to Latin script.
- Serbian Cyrillic has no combining diacritics; Ђ Ј Љ Њ Ћ Џ are mandatory distinct letters and never fold to look-alikes.
- No precomposed-versus-combining, apostrophe, textual-joiner, or required code-point distinction applies beyond selecting the correct Cyrillic letter.
- Directionality: left-to-right only; no bidirectional handling needed.
- Case distinction exists (uppercase/lowercase); casing rules for labels/titles are topic 6.
- Inter-word spacing: single space; same single space, no special joiner, between native Cyrillic text and an embedded Latin/numeric token.
- Compound formation: default spaced multiword technical terms (e.g. "тачка напајања"); hyphenate only an established fixed compound (e.g. "радио-таласи"); never fuse two roots into one unhyphenated word absent existing catalog precedent.

## 2. Numerals in literals

- Use Arabic digits in translated prose; Serbian has no alternate digit set.
- Use a comma as the decimal separator in literal prose values; retain source form in formulas, examples, fixed defaults, and named mathematical or standards constants.
- Thousands/grouping separator: period, per Serbian convention (e.g. "1.000").
- Ordinal notation: digit followed by period, no native affix suffix (e.g. "1." not "1st"/"1-и"); an index separator (e.g. segment index "3/10") is never read as a decimal point.

## 3. Punctuation and quotation

- Quotation marks: native „…” (low-open, high-close); retain plain "…" only around an embedded technical token per topic 4, for consistency with that token's source form.
- Comma, question mark, and exclamation mark are shared with source punctuation; no separate native glyphs replace them.
- No fixed space before colon/semicolon/terminal punctuation; attach directly to the preceding word, matching source convention.
- Use the single-character ellipsis "…"; preserve a source en dash "–" for ranges and never replace it with a hyphen.
- Sentence terminator: full stop on complete sentences (dialogs, tooltips, error text); short labels and fragments (buttons, field labels, menu items) omit the terminal period.
- Punctuation embedded inside a technical run (file path, format specifier, code fragment) stays in source form, never Cyrillic-substituted.

## 4. Never-translate tokens

- Preserve every token in this section byte-for-byte in its stated form; translate or transliterate none, with zero omissions or substitutions.
- NEC2 card mnemonics kept verbatim: GW GA GH EX LD FR RP GE EN and the extended SP SM SC NE NH NT TL GN GD GC GX GR GS GM GF CP CM SY XQ EK KH PQ PT.
- Unit symbols kept verbatim: MHz, dBi, dBd, Ω, V, A, W, S/m, K, °, %.
- Figure-of-merit tokens kept verbatim: VSWR, S-parameter labels (S11, …), Z0/Zref, front/back-ratio and gain/temperature tokens.
- File extensions and literal filenames kept verbatim: .nec, .csv, .s1p, .s2p, .out.
- Format specifiers kept verbatim and in source order; see topic 5.
- Embedded identifiers (function names, variable names, config keys) inside developer strings are kept verbatim: mem_free, config_widget_*, eventfd, inotify.
- Physical/mathematical symbol letters (E, H field letters; θ, φ angle/exponent letters) stay in source Latin/Greek form, never transliterated to a Cyrillic look-alike.
- Product, library, and tool names kept verbatim: xnec2c, GTK, Cairo, OpenGL, FFTW.
- Named transfer-function/algorithm terms kept verbatim: Log, Asinh, μ-law, Reinhard, Sigmoid; the descriptive family name "Power" is the one exception that translates, as "Степенска" (topic 10).
- Conditional loanwords, decided file-wide: segment stays translated as native-formed "сегмент"; patch retains the adapted technical term "патч" in every sense; tag translates as native "ознака"; the process term fork stays verbatim "fork".
- All Appendix-B tokens above stay left-to-right inside Cyrillic prose per topic 1; no bidirectional mirroring applies since Serbian is LTR-only.

## 5. Format-specifier integrity

- Every specifier from the source (%s, %d, %f, %c, %%) is preserved, same set, same left-to-right order as the msgid.
- Keep source order by default; positional reordering is allowed only when the source supplies positional specifiers and natural Serbian grammar requires it.
- With non-positional specifiers, restructure the sentence around their fixed positions rather than moving them.
- Numbers inside specifiers are never localized (comma-for-decimal, grouping separators) since they are runtime-substituted.

## 6. Capitalization and title-case

- Default casing for labels, menu items, and titles: sentence case; never English-style title-case or German-style noun capitalization.
- Axis letters (X, Y, Z) keep source uppercase.
- A lowercase math/coordinate variable (e.g. "z=" in a coordinate readout) stays lowercase; never uppercased to match an axis label.
- Acronyms and proper nouns retain their original casing (VSWR, xnec2c, GTK).
- Capitalize every element of a coordinated option name only when each element is an independent proper name; otherwise use sentence case.
- Use lowercase "картица" in running prose and capitalize it only at sentence start or in a dialog/editor title.

## 7. Interface register by string type

- Commands, buttons, and menu actions use a second-person-plural imperative with the subject omitted, verb first when natural, and the object or qualifier after it.
- Field labels use a concise noun phrase in natural head-before-modifier order and retain the source colon; use no verb or subject.
- Dialogs and confirmations use complete sentences with the subject omitted when recoverable, finite verbs in natural Serbian order, and known information before the requested decision.
- Tooltips use a complete descriptive indicative or impersonal sentence; when the source gives a disabled control's reason, state that reason after the unavailable function.
- Status and error messages use impersonal, declarative, or passive sentences; state the affected entity before the condition and omit first- and second-person subjects.
- Use case inflection and natural multiword phrases rather than source-order compounds; choose the shortest complete form without lost meaning, unnatural abbreviation, or mechanical source-length imitation.
- Name user-visible domain entities as a generic Serbian noun followed by a retained proper identifier or numeral, eg "картица GW" or "сегмент 3"; topic 8 governs personal names.
- Topic 24 governs developer and debug strings.

## 8. Formality and address

- Use polite second-person-plural verb morphology for direct commands; omit the explicit pronoun "Ви".
- Serbian permits subject omission: carry person and number in the verb ending and omit recoverable subjects.
- Avoid gender-marked past-tense forms for the user; use imperative, present, passive, or impersonal constructions that remain natural and inclusive.
- Preserve grammatically required gender, number, and animacy agreement for named entities without assigning them to the user.
- Use no honorifics; address the user only through polite verb morphology. Personal names retain their supplied order and spelling.
- Commands use polite imperatives; dialogs use neutral complete sentences; status, error, and tooltip text use indicative, passive, or impersonal forms.
- Confirm with a full yes/no question in the pattern "Да ли ...?".
- Omit casual, slang, archaic, commercial, and ceremonially over-formal register.

## 9. Accelerator/hotkey mnemonics

- Marker convention: GTK underscore prefix on the mnemonic letter (e.g. "_Отвори").
- Per-container uniqueness: no two accelerators in the same menu/dialog share a letter; shift to the next distinctive letter in the translated term on collision.
- The mnemonic letter is drawn from the Serbian translated term, never transliterated from the English source letter.
- Cyrillic mnemonics are not directly typable on a Latin keyboard layout; where the source string relies on a Latin-keyboard accelerator, append the parenthetical Latin letter after the Cyrillic label (e.g. "Отвори (_O)").
- A mnemonic is added only where the source string carries one; never invent a mnemonic absent from source.
- Mnemonics apply to buttons, menu items, and tab labels; never to field labels, tooltips, or status text.
- Avoid letters that are visually ambiguous or awkward to reach when choosing among candidate letters in the translated term.

## 10. Domain lexicon

- Lock every row to one Serbian term to preserve the intended electromagnetic, interface, or compute sense and prevent catalog synonym drift; row notes add concept-specific hazards.

| Concept | Serbian (Cyrillic) | Sense / hazard |
|---|---|---|
| current (electrical) | струја | electrical current — never "тренутни" (temporal "current/recent") |
| charge (electrical) | наелектрисање | Coulombs — never billing/fee ("наплата") |
| voltage | напон | electric potential |
| power (electrical) | снага | radiated/dissipated watts — distinct from the "Power" curve-family name (Степенска) and from math power-law |
| impedance | импеданса | complex Z — distinct from resistance/reactance |
| resistance | отпор | real part of Z |
| reactance | реактанса | imaginary part of Z |
| inductance | индуктивност | |
| capacitance | капацитивност | |
| conductivity | проводљивост | material S/m |
| admittance | адмитанса | distinct from impedance |
| load (LD-card) | оптерећење | LD-card impedance load — not physical weight; distinct from charge |
| gain (antenna) | добитак | directivity ratio dB — never "појачање" (amplifier amplification) nor "профит" (business profit) |
| excitation | побуда | EM energy input — never "узбуђење" (emotional excitement) |
| feedpoint | тачка напајања | antenna feed point |
| port | порт | excitation/S-parameter port |
| radials (noun) | радијали | ground-plane radial wires — distinct from adjective "радијални" |
| ground / ground plane | уземљење | RF electrical reference plane, GN/GD cards — never soil; one term across all ground sub-senses |
| earth (physical medium) | тло | terrain/noise-model earth, "below ground" geometry — distinct from electrical уземљење |
| ground wave | земљани талас | propagation term, distinct from уземљење |
| wire | жица | thin conductor / GW element — never cable/cord; distinct from thread (нит) |
| segment | сегмент | NEC2 geometry subdivision |
| patch (surface) | патч | NEC2 surface element; one adapted technical term prevents collision with a software repair patch |
| tag | ознака | NEC2 geometry identifier — not a UI label, not a card |
| card (NEC record) | картица | register handled in topic 18 |
| kernel | језгро | integral-equation/thin-wire kernel — never OS kernel; never transliterated "Кернел" |
| cliff | литица | two-medium ground-boundary type — never "прелом" (fracture) |
| structure | структура | antenna model geometry — never "конструкција" (construction) |
| model | модел | NEC model or noise-temperature model |
| geometry | геометрија | the model geometry |
| crossed | укрштени | transmission-line conductors crossed/reversed — never "исечени" (cut/severed) |
| field (EM) | поље | near/total/E/H field — distinct from a data/config field |
| near field | блиско поље | opposed pair, keep symmetric with far field |
| far field | далеко поље | opposed pair, keep symmetric with near field |
| far-field contribution | допринос далеког поља | per-direction contribution — not near-field animation |
| radiation | зрачење | radiated emission |
| radiation pattern | дијаграм зрачења | plotted directional response — not a template/design |
| gain pattern | дијаграм добитка | the gain radiation pattern |
| polarization | поларизација | antenna/wave field orientation — distinct from polarity |
| polarity | поларитет | sign (+/-) of a quantity — false friend of polarization |
| phase | фаза | |
| reference phase | референтна фаза | |
| frequency | фреквенција | |
| wave / wavelength | талас / таласна дужина | |
| standing wave | стојећи талас | opposed pair with traveling wave |
| traveling wave | путујући талас | opposed pair with standing wave |
| node / antinode | чвор / трбух | standing-wave zero/maximum — distinct from a generic numeric null/zero |
| crest | гребен | instantaneous wave apex (comet-head) — distinct from curve peak (врх) |
| magnitude | модул | modulus of a quantity (|Z|, scalar) — distinct from amplitude |
| amplitude | амплитуда | oscillating-quantity peak — distinct from magnitude |
| peak value | вршна вредност | distinct UI option from peak magnitude, never collapsed |
| peak magnitude | вршни модул | distinct UI option from peak value, never collapsed |
| instantaneous | тренутни | projection mode; append "(φ=0)" only where the source string carries it |
| Poynting vector | Појнтингов вектор | |
| solid angle | просторни угао | |
| net gain | нето добитак | total-minus-mismatch gain — never "реални добитак" (real-part gain) |
| viewer | гледиште | observation direction or 3D view; a non-person term prevents collision with observer, speaker, and preview |
| flow / flow direction | ток / смер тока | patch/current flow |
| total field | укупно поље | |
| color | боја | |
| color projection | пројекција боје | which quantity drives hue |
| hue | нијанса | color-wheel angle |
| brightness | осветљеност | luminance channel |
| hue encoding | кодирање нијансе | distinct enum — never collapsed into "color projection" |
| brightness encoding | кодирање осветљености | distinct enum — never collapsed into "color projection" |
| color scale | скала боје | magnitude-to-color scale |
| scale family / color tone | породица скале | transfer-curve family (Power/Log/Asinh/μ-law/Reinhard/Sigmoid); one term for both source spellings |
| palette / palette kind | палета / врста палете | distinct from scale family and color projection |
| ramp / gradient | рампа / градијент | a palette kind / linear color strip |
| gamma | гама (експонент) | power-law exponent |
| knee | колено | soft-knee bend point |
| softening | омекшавање | dynamic-range softening |
| compression | компресија | dynamic-range compression |
| contrast | контраст | contrast steepness: нагиб контраста |
| dynamic range | динамички опсег | |
| floor | доња граница | minimum/lower clamp (brightness/dB floor) — never a room floor |
| envelope | обвојница | magnitude/amplitude envelope |
| comet | комета | moving-crest overlay effect — never geometry (known fuzzy-inheritance hazard) |
| overlay (noun) | преклоп | added visual layer, masc. noun, plural "преклопи" — distinct from the verb "to overlay" |
| animate / animation | анимирати / анимација | |
| animated / static | анимирано / статично | category-header adjectives |
| projection | пројекција | color or geometry projection |
| scale (verb/noun) | скалирати / скала | |
| wireframe | жичани приказ | wire-mesh render mode |
| identity | идентитет | no-op/passthrough transfer — distinct from unity (јединица, Smith-chart) |
| sentinel | стражарска вредност | unreachable-case guard value |
| bins | бинови | discretization buckets |
| companding | компандовање | bounded log curve (μ-law) |
| tone mapping | мапирање тонова | photographic tone-map |
| renderer | рендерер | drawing backend — never "мотор за рендеровање" (render engine) |
| shader | шејдер | |
| allocation (memory) | алокација | |
| managed allocator | управљани алокатор | allocation and the allocator/report |
| thread | нит | compute thread — distinct word from wire (жица), no homonym collision in Serbian |
| widget | виџет | UI element |
| validation | верификација | the validation-tree feature — distinct from verification/checks (провера) |
| batch mode | групни режим | |
| fork (process) | fork | kept verbatim, see topic 4 |
| deadlock | мртва брава | |
| notifier | обавештавач | |
| token (expression parser) | токен | not left as Latin "token" |
| operand | операнд | |
| operator | оператор | |
| arity | арност | |
| override | надјачавање | supersede a value (SY symbol) — distinct from overwrite (преписивање) |
| swap | размена | exchange |
| theme | тема | UI/color theme; if a literal "topic/subject" sense ever occurs, use "предмет" to avoid the тема/тема collision |
| noise / noise temperature | шум / температура шума | electronic/thermal noise — never acoustic racket |
| efficiency | ефикасност | |
| interpolation | интерполација | |
| mnemonic | мнемоник | a card's code descriptor — not a memo/note |
| degrees / deg | степени | freestanding axis/prose "степени"; the parenthetical unit tag "(deg)" stays a kept unit tag per topic 4 |
| diameter | пречник | native technical term — never the loanword "дијаметар" |
| radius | полупречник | native technical term — never the loanword "радијус" |
| reflect (geometry mirror) | пресликати | geometry mirror operation |
| reflect (behavioral mirroring) | пратити | a control tracking another ("mirrors …") |
| reflect (physics) | рефлектовати | physics reflection |
| default(s) | подразумевано | fallback value |
| normalize / normalization | нормализовати / нормализација | translated, not transliterated |
| unity (Smith-chart) | јединица | distinct from identity (идентитет) |
| "Power" (transfer-family curve name) | Степенска | the descriptive exception that translates; distinct from снага (electrical power) |

## 11. Disambiguation policy

- The correct technical sense is chosen directly for each ambiguous term (струja=current, наелектрисање=charge, добитак=gain, импеданса=impedance) without adding a qualifying adjective ("електрична...") absent from the English source; program context already disambiguates.
- A qualifier is added only where the Serbian term would otherwise be genuinely ambiguous outside that context.
- No accepted intra-domain homonym applies; use the distinct terms fixed in topics 10 and 12.
- No locative homonym requires a separate rule in Serbian.
- Gerund vs noun senses of an overloaded source word (e.g. "scaling" vs "a scale") are distinguished per the scale entry in topic 10 (скалирати for the verb, скала for the noun).

## 12. Cross-catalog consistency

- One term per concept, reused from topic 10's locked table everywhere it recurs; no synonym variation across the catalog.
- Appendix-C false-friend pairs, each resolved to two distinct Serbian terms that never share a translation:
  - polarity (поларитет) vs polarization (поларизација)
  - magnitude (модул) vs amplitude (амплитуда)
  - peak value (вршна вредност) vs peak magnitude (вршни модул)
  - ground (уземљење) vs earth (тло)
  - load (оптерећење) vs charge (наелектрисање)
  - gain (добитак) vs amplification (појачање) vs profit (профит)
  - current (струја) vs present/recent (тренутни)
  - charge (наелектрисање) vs billing/fee (наплата)
  - wire (жица) vs cable/cord (кабл); wire (жица) vs thread (нит)
  - radiation pattern (дијаграм зрачења) vs template/design (шаблон/пројекат) vs far field (далеко поље)
  - excitation (побуда) vs emotional excitement (узбуђење)
  - node/antinode (чвор/трбух) vs generic numeric null/zero (нула)
  - scale family/color tone (породица скале) vs hue (нијанса) vs palette kind (врста палете) vs color projection (пројекција боје)
  - comet (комета) vs geometry (геометрија)
  - identity (идентитет) vs unity (јединица)
  - renderer (рендерер) vs render engine (мотор за рендеровање, forbidden)
  - override (надјачавање) vs overwrite (преписивање)
  - viewer (гледиште) vs observer (посматрач) vs speaker (говорник) vs preview (преглед)
  - reflect: geometry mirror (пресликати) vs behavioral mirrors (пратити) vs physics reflection (рефлектовати)
  - structure (структура) vs construction (конструкција)
  - theme (тема) vs topic/subject (предмет)
  - validation (верификација) vs verification (провера)
  - net gain (нето добитак) vs real-part gain (реални добитак)
  - power/electrical (снага) vs "Power" transfer-family name (Степенска)
- Loanword-vs-native decision, locked spelling: segment=сегмент (adapted loan), patch=патч (adapted technical term), radius=полупречник, diameter=пречник, kernel=језгро — all native or adapted-loan forms as listed in topic 10, never the raw transliterated loanword (радијус, дијаметар, Кернел).
- Minority-outlier spellings in the catalog unify to the topic-10 canonical form; a stray "радијус" or "Кернел" corrects to полупречник/језгро.
- Consistency priority outranks locale-form preference where topic 10 states a native term over an equally valid loanword (radius, kernel): catalog-wide uniformity wins over a locally plausible loan spelling.

## 13. Priority ordering

- Precedence chain: correct meaning, then interface convention (topics 6-9), then catalog consistency (topic 12), then disambiguation (topic 11), then locale numeral form (topic 2).
- Explicit override: catalog consistency overrode locale-plausible loanword spelling for radius/kernel (topic 12); interface convention (imperative register) overrode literal source mood for command/button strings (topic 7).

## 14. Grammatical number

- Use singular, paucal, and plural forms in translated literals; inflect nouns, adjectives, participles, and finite verbs for the selected number.
- After a literal count ending in `1` but not `11`, use singular; after `2`-`4` but not `12`-`14`, use paucal; use plural otherwise.
- Realize the paucal noun as genitive singular and the plural noun after `0` or `5+` as genitive plural, with matching agreement.

## 15. Grammatical agreement

- Adjectives and participles concord in gender, number, and case with their head noun, including a standalone label whose head noun is implied (e.g. an adjective-only label defaults per topic 15's gender rule below).
- Declension-class follows standard Serbian noun paradigms; after a count of 2-4 the noun takes paucal (genitive singular) form, after 5+ or 0 it takes genitive plural.
- Reflexive particle "се" is retained wherever the underlying verb is reflexive (e.g. "учитава се").
- A standalone label with no explicit head noun defaults to masculine gender unless the established head noun for that concept in the catalog is feminine or neuter (e.g. "тачка" is feminine, so labels built on "тачка напајања" agree feminine).

## 16. Morphological derivation

- Adapt borrowed technical verbs with the established `-овати` or `-ирати` family, eg "рендеровати" and "скалирати"; never retain a foreign inflection.
- Form verbal nouns with `-ање` or `-ење`, eg "рендеровање" and "скалирање".
- Native-affix formation is preferred over a bare loanword wherever an established native term exists (topic 10); an unavoidable loanword takes standard Serbian morphological adaptation, not a raw English inflection.
- Compounding strategy: spaced multiword by default; hyphenate only an established fixed compound, per topic 1.

## 17. Preposition and sandhi selection

- The preposition "с/са" alternates by the following sound: "с" before most consonants, "са" before a consonant cluster beginning with s/š/z/ž and before certain pronouns; apply this alternation, not a fixed single form.
- No elision or contraction rule beyond the с/са alternation is required for this catalog.

## 18. Card/record-label register

- Dialog and editor titles use the fixed designator form "Картица GW" or the numbered form "Картица 3".
- Running prose in messages uses the distinct suffixed form: "на картици 3", "картица 3 је...".
- Generic-noun casing: "картица" lowercase in running prose, capitalized only at sentence start or as part of a proper dialog title.
- Each register (title-form vs prose-form) stays internally consistent within its context and is never cross-converted.

## 19. Multi-paragraph and whitespace fidelity

- Mirror source paragraph breaks at the same positions, preserving the distinction between blank lines and single line breaks.
- Drop every trailing clause absent from the current source; never retain stale translated meaning.
- Preserve source trailing newlines and punctuation exactly.
- Preserve semantic line breaks; add no visual wrapping absent from the source literal.
- Preserve complete natural meaning; never truncate or abbreviate for an assumed display limit.

## 20. Current-source fidelity

- Derive every translation from the current source literal and its supplied context.
- Reuse an inherited translation only when its complete meaning agrees with the current source.
- Treat prior translations of `magnitude`, `amplitude`, `ground`, `earth`, `comet`, `geometry`, `validation`, and `verification` as unsafe unless they preserve the distinctions fixed in topics 10 and 12.

## 21. Script hygiene

- Cyrillic prose contains zero Latin-script letters or mixed Latin-Cyrillic adjacency outside retained tokens whose literal spelling requires Latin characters.
- Translate every plain foreign word; retain only identifiers, units, mnemonics, product names, and other tokens fixed by topic 4.
- Treat Latin look-alikes for Cyrillic letters as wrong-script characters in prose; use the Cyrillic code point required by topic 1.

## 22. Rule-file scope hygiene

- Keep only current decisions that can alter wording, Unicode characters, punctuation, capitalization, embedded tokens, mnemonic markers, or semantic whitespace in a translated literal.
- Omit catalog representation, serialization, headers, flags, validation workflow, rendering, fonts, shaping, layout, widget behavior, runtime formatting, sorting, search, display sizing, audit history, completion state, review metadata, provenance, and source citations.
- State each decision directly; include no implementation procedure, bibliography, correction history, or alternative policy source.

## 23. Section-disjointness declaration

- Script mechanics govern only character, punctuation, numeral, and retained-token form in topics 1-6, 9, and 21.
- Phrasing and structure govern only sentence grammar, terminology, agreement, derivation, source fidelity, and semantic whitespace in topics 7, 10-17, 19, and 20.
- Address register governs user formality only in topic 8; card/record register governs record naming only in topic 18.
- Keep these axes disjoint; place each decision in its single governing topic.

## 24. Developer/debug-string policy

- Translate every user-facing action, label, dialog, tooltip, status, warning, and error into Serbian; review priority never permits an applicable user-facing literal to remain untranslated.
- Translate informational diagnostics and developer-facing debug strings into terse technical Serbian; no subsystem family overrides this policy.
- Preserve every embedded identifier, function name, config key, format specifier, unit, mnemonic, and retained technical token verbatim in every translated family.
- Keep linguistic priority separate from token preservation: all translated families obey topics 4 and 5 regardless of review order.
- Render the `BUG:` diagnostic prefix as "ДЕФЕКТ:", held distinct from the "ГРЕШКА:" prefix that carries ordinary failures; the two severities never share a word.
- Keep an identifier-plus-value diagnostic pair contiguous (eg `sweep_state=%d`); a qualifier precedes the whole pair and never stands between the identifier and its `=`.
