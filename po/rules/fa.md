# fa translation rules

These rules govern standard Iranian Persian in the Perso-Arabic script for professional electrical and radio-frequency engineers, antenna designers, and amateur-radio operators using electromagnetic simulation software.

## 1. Script and orthography
- Use the standard Iranian Persian Perso-Arabic script from the current orthographic standard, written right-to-left.
- Use Persian letters ک ی گ چ پ ژ; retain all required diacritics and never fold them to look-alikes ك ي.
- Use ZWNJ (نیم‌فاصله, U+200C) for bound morphology, eg می‌شود, ‌های, پیش‌فرض; never replace or omit it.
- Use precomposed Persian letters; use combining marks only for required Persian diacritics that lack a precomposed form.
- Keep embedded left-to-right technical tokens in their own direction, unmirrored, with no manual direction overrides.
- Persian has no letter case; apply no case transformation to Persian text.
- Separate Persian words with one regular space, join bound affixes with ZWNJ, and place one regular space between Persian text and an embedded Latin or numeric token.
- Join standard morphological compounds with ZWNJ, eg پیش‌فرض; write lexical multi-word technical phrases with spaces, eg صفحه زمین; use a hyphen only when the established Persian technical form requires one.

## 2. Numerals in literals
- Use Western Arabic digits `0-9` for literal technical values; use no Eastern Arabic-Indic digits.
- Use `.` as the decimal separator and no thousands separator in literal technical values.
- Retain source form for formulas, examples, fixed defaults, and named mathematical or standards constants.
- Attach a native ordinal suffix according to Persian grammar; keep an index marker distinct from the decimal separator.

## 3. Punctuation and quotation
- Use «» for prose quotations; retain straight source quotes around embedded technical tokens for literal-token consistency.
- Use Persian comma ، and question mark ؟ in prose; retain an exclamation mark only when the source contains one.
- Place no space before ، ؛ ؟ or terminal punctuation and one space after it; retain a colon after a field label with no preceding space.
- Use the single ellipsis glyph …; preserve source en and em dashes.
- End full sentences with `.`; omit a terminator from short labels, fragments, and menu items.
- Keep punctuation inside identifiers, paths, format strings, and other embedded technical runs in source form.

## 4. Never-translate tokens
Treat every listed token as a zero-failure boundary: retain its exact source spelling and script; never translate or transliterate it.
- NEC2 card mnemonics: GW GA GH EX LD FR RP GE EN SP SM SC NE NH NT TL GN GD GC GX GR GS GM GF CP CM SY XQ EK KH PQ PT.
- Unit symbols: Hz, kHz, MHz, GHz, dB, dBi, Ω, W, K, S/m, deg, %.
- Figure-of-merit and parameter tokens: VSWR, S11, S21, Z, Z0, F/B, G/T.
- File extensions: .nec, .csv, .s1p, .s2p, .png.
- Format specifiers: `%s`, `%d`, `%i`, `%u`, `%f`, `%g`, `%e`, `%c`, `%p`, `%%`, including their source flags, widths, precisions, lengths, and positional indices.
- Embedded function names, variable names, configuration keys, and other code identifiers.
- Physical and mathematical symbol letters: E, H, Z, θ, φ, λ.
- Product, library, tool, and chart names: xnec2c, NEC2, GTK, Cairo, OpenGL, Smith chart.
- Named transfer functions and algorithms: Log, Asinh, μ-law, Reinhard, Sigmoid; translate the descriptive families Power as تابع توانی and Identity as همانی.
- Conditional loanwords: retain fork; transliterate patch as پچ and tag as تگ; translate segment as قطعه.
- Keep every retained token left-to-right and unmirrored inside right-to-left prose under topic 1.

## 5. Format-specifier integrity
Treat format specifiers as an exact invariant: preserve the source set, count, spelling, and numeric components with zero failures.
- Keep source order by default; use positional forms only when Persian grammar requires reordering and the source format supports them.
- Restructure Persian wording around fixed non-positional specifiers; never move such a specifier to a different substitution slot.
- Never localize digits within a format specifier.

## 6. Capitalization and title-case
Persian has no letter-case distinction: apply no title case, sentence case, or noun capitalization to Persian labels, menu items, titles, axis names, coordinated options, or the generic card noun. Preserve the exact source case of retained mnemonics, acronyms, proper nouns, axis letters, and lowercase mathematical or coordinate variables.

## 7. Interface register by string type
Use standard written Persian interface grammar; preserve the shortest complete natural wording without abbreviating meaning or imitating source length.
- Commands, buttons, and menu actions: use a formal second-person plural imperative, omit the overt subject شما, place the object before the final verb, and keep the action as the information focus.
- Field labels: use a concise noun phrase in modifier-head order natural to Persian, followed by the retained colon; use no subject or verb.
- Dialogs and confirmations: use full formal sentences in subject-object-verb order, omitting an inferable subject; express confirmations as a complete question with an imperative button pair.
- Tooltips: use a complete declarative sentence in natural subject-object-verb order; when the source gives a disabled-state reason, state the unavailable action before its reason.
- Status and error messages: use terse impersonal declarative or passive sentences, place the affected entity before the result, and use no first- or second-person subject.
- User-visible domain entities: place the generic class noun before a retained identifier, eg کارت GW; use Persian ezafe order for translated modifiers.
- Use established ZWNJ compounds and natural multi-word phrases under topic 1; never omit source meaning or use an unnatural abbreviation.
- Apply topic 24, not this section, to developer and debug strings.

## 8. Formality and address
- Use formal, impersonal written Persian throughout; forbid informal address, slang, spoken contractions, commercial phrasing, archaic diction, and ceremonial over-formality.
- Use second-person plural formal imperatives for direct commands; omit the pronoun شما unless contrast requires it.
- Persian has no grammatical gender in pronouns or verbs: introduce no gender marking and use natural gender-neutral wording.
- Express inclusion through neutral role nouns and impersonal constructions; preserve required number and animacy without adding gender.
- Use no honorific when addressing the user; use second person only for direct commands and third-person or impersonal forms elsewhere.
- Write personal names in the culturally standard given-name then family-name order when the source separates those parts; preserve an unsplit source name verbatim.
- Persian has no dedicated formal verb ending; realize formality through plural imperative morphology, construction, and word choice.
- Keep commands imperative and dialogs formal full sentences; use a full-sentence question plus an imperative button pair for confirmation dialogs.

## 9. Accelerator/hotkey mnemonics
- Preserve the source `_` marker only when the source literal contains one; never invent a mnemonic.
- Place `_` immediately before a typable Persian base letter in the translated term; select the letter from that term, not from a source transliteration.
- Use no parenthetical Latin mnemonic because Persian letters can carry the marker directly.
- Avoid a ZWNJ-adjacent or diacritic-bearing target; choose another base letter in the same translated term when a collision occurs.

## 10. Domain lexicon

### Electrical primitives
| concept | term | sense / purpose |
|---|---|---|
| current | جریان | electrical current (A); not temporal "recent" |
| charge | بار الکتریکی | electrical charge (C); not billing |
| voltage | ولتاژ | electric potential |
| power (electrical) | توان | radiated or dissipated watts, power gain, and power flow; distinct from the transfer-family تابع توانی and a mathematical exponent |
| impedance | امپدانس | complex Z; distinct from resistance/reactance |
| resistance | مقاومت | real part of Z |
| reactance | راکتانس | imaginary part of Z |
| inductance | اندوکتانس | canonical technical term; prevents synonym drift |
| capacitance | ظرفیت خازنی | canonical technical term; prevents synonym drift |
| conductivity | رسانایی | material S/m |
| admittance | ادمیتانس | admittance-matrix sense; distinct from impedance |
| load | بار (امپدانس) | LD-card impedance load; shares base noun بار with charge, kept distinct by this qualifier |
| gain | بهره | antenna directivity ratio (dB); not amplification, not profit |
| excitation | تحریک | EM energy source; not emotional excitement |
| feedpoint | نقطه تغذیه | antenna feed point |
| port | پورت | excitation / S-parameter port |
| radials | سیم‌های شعاعی | ground-plane radial wires (noun); adjective "radial" is شعاعی |

### Ground and earth
| concept | term | sense / purpose |
|---|---|---|
| ground / ground plane | زمین / صفحه زمین | RF electrical reference; one term across all ground sub-senses |
| earth (physical medium) | خاک | terrain/soil; distinct from electrical زمین |
| ground wave | موج زمینی | propagation term |

### Geometry primitives
| concept | term | sense / purpose |
|---|---|---|
| wire | سیم | thin conductor / GW element; not cable/cord |
| segment | قطعه | NEC2 subdivision |
| patch | پچ | NEC2 surface patch (SP/SM); transliteration locked, not translated |
| tag | تگ | NEC2 geometry identifier; not a UI label (برچسب) or a card |
| card | کارت | NEC2 input record; register per topic 18 |
| kernel | هسته | integral-equation kernel; not an OS kernel |
| cliff | پرتگاه | two-medium ground-boundary type; not a fracture |
| structure | ساختار | antenna model geometry; not construction (سازه) |
| model | مدل | NEC model or noise-temperature model |
| geometry | هندسه | the model geometry |
| crossed | متقاطع | transmission-line conductors crossed/reversed; not cut |

### Field, pattern, viewer
| concept | term | sense / purpose |
|---|---|---|
| field (EM) | میدان | distinct from a UI/config field (فیلد) |
| near field | میدان نزدیک | opposed pair with far field |
| far field | میدان دور | opposed pair with near field |
| far-field contribution | سهم میدان دور | per-direction contribution |
| radiation | تشعشع | radiated emission |
| radiation pattern | الگوی تشعشع | plotted directional response; not a template/design (قالب) |
| gain pattern | الگوی بهره | the gain radiation pattern |
| polarization | قطبش | field/wave orientation |
| polarity | قطبیت | sign of a quantity; false friend of polarization |
| phase | فاز | canonical technical term; prevents synonym drift |
| reference phase | فاز مرجع | canonical technical term; prevents synonym drift |
| frequency | فرکانس | canonical technical term; prevents synonym drift |
| wave | موج | canonical technical term; prevents synonym drift |
| wavelength | طول موج | canonical technical term; prevents synonym drift |
| standing wave | موج ایستا | opposed pair with traveling wave |
| traveling wave | موج سیار | opposed pair with standing wave |
| node | گره | standing-wave zero |
| antinode | پادگره | standing-wave maximum |
| crest | تاج موج | instantaneous wave apex (comet-head); distinct from اوج (curve/step peak) |
| magnitude | دامنه | modulus of a quantity (|Z|, scalar); distinct from amplitude |
| amplitude | دامنه نوسان | oscillating-quantity peak; distinct from magnitude |
| peak value | مقدار اوج | distinct UI option from peak magnitude |
| peak magnitude | دامنه اوج | distinct UI option from peak value |
| instantaneous | لحظه‌ای | projection mode; add the `(φ=0)` qualifier only where the source carries it |
| Poynting vector | بردار پوینتینگ | canonical technical term; prevents synonym drift |
| solid angle | زاویه فضایی | canonical technical term; prevents synonym drift |
| net gain | بهره خالص | total-minus-mismatch gain; not بهره بخش حقیقی (real-part gain) |
| viewer | نمایشگر | observation direction / 3D view widget; not ناظر/گوینده/پیش‌نمایش |
| flow / flow direction | جهت جریان | patch/current flow direction; reuses established جریان, same underlying physical quantity |
| total field | میدان کل | canonical technical term; prevents synonym drift |

### Color, tone, animation
| concept | term | sense / purpose |
|---|---|---|
| color | رنگ | canonical technical term; prevents synonym drift |
| color projection | فرافکنی رنگ | which quantity drives hue; not تصویر رنگ |
| hue | رنگ‌مایه | color-wheel angle |
| brightness | روشنایی | luminance channel |
| hue encoding | رمزگذاری رنگ‌مایه | distinct enum from brightness encoding |
| brightness encoding | رمزگذاری روشنایی | distinct enum from hue encoding |
| color scale | مقیاس رنگ | magnitude-to-color scale; distinct from scale family |
| scale family / color tone | تن | transfer-curve family; one concept for two source spellings, with Power rendered as تابع توانی and kept distinct from electrical توان |
| palette | پالت | canonical technical term; prevents synonym drift |
| palette kind | نوع پالت | palette-layout enum; distinct from scale family and color projection |
| ramp / gradient | شیب | a palette kind / linear color strip; one term |
| gamma | گاما | power-law exponent |
| knee | زانو | soft-knee bend point |
| softening | نرم‌سازی | dynamic-range softening |
| compression | فشرده‌سازی | dynamic-range compression |
| contrast | کنتراست | canonical technical term; prevents synonym drift |
| dynamic range | دامنه دینامیکی | fixed compound; distinct sense from دامنه (magnitude) |
| floor | کف | minimum/lower clamp; not a room floor |
| envelope | پوش | magnitude/amplitude envelope |
| comet | دنباله‌دار | moving-crest overlay effect; not هندسه (geometry) |
| overlay (noun) | لایه‌ی روی‌هم | added visual layer; distinct from the verb روی‌هم‌گذاری کردن |
| animate / animation | انیمیشن | canonical technical term; prevents synonym drift |
| animated / static | متحرک / ایستا | category-header adjectives |
| projection | فرافکنی | color or geometry projection |
| scale (noun / verb) | مقیاس / مقیاس‌گذاری کردن | a scale / to scale |
| wireframe | مش سیمی | wire-mesh render mode |
| identity | همانی | no-op/passthrough transfer; distinct from unity (یکه, Smith-chart) |
| sentinel | نگهبان | unreachable-case guard value |
| bins | سطل‌ها | discretization buckets; not بین (preposition "between") |
| companding | کمپندینگ | bounded log curve (μ-law); distinct from compression |
| tone mapping | نگاشت تن | photographic tone-map |

### Render and compute
| concept | term | sense / purpose |
|---|---|---|
| renderer | رندرر | drawing backend; never موتور رندر ("render engine") |
| shader | شیدر | canonical technical term; prevents synonym drift |
| allocation (memory) / managed allocator | تخصیص / تخصیص‌دهنده مدیریت‌شده | allocation and the allocator/report |
| thread | رشته | compute thread; not homonymous with سیم (wire) in Persian |
| widget | ابزارک | UI element |
| validation | اعتبارسنجی | validation-tree feature; distinct from verification (راستی‌آزمایی) |
| batch mode | حالت دسته‌ای | canonical technical term; prevents synonym drift |
| fork (process) | fork | kept verbatim per topic 4; light-verb form per topic 16 |
| deadlock | بن‌بست | canonical technical term; prevents synonym drift |
| notifier | اعلان‌گر | canonical technical term; prevents synonym drift |
| token | نشانه | expression-parser term |
| operand | عملوند | expression-parser term |
| operator | عملگر | expression-parser term |
| arity | تعداد آرگومان | expression-parser term |
| override | لغو مقدار | supersede a value (SY symbol); not overwrite (رونویسی) |
| swap | تعویض | exchange |
| theme | پوسته | UI/color theme; not موضوع (topic/subject) |

### Metrics and miscellaneous
| concept | term | sense / purpose |
|---|---|---|
| noise / noise temperature | نویز / دمای نویز | electronic/thermal; not acoustic سروصدا |
| efficiency | بازده | canonical technical term; prevents synonym drift |
| interpolation | درون‌یابی | canonical technical term; prevents synonym drift |
| mnemonic | یادیار | a card's code descriptor; not یادداشت (memo/note) |
| degrees / deg | درجه | freestanding prose "degrees"; the parenthetical unit tag "(deg)" stays verbatim per topic 4 |
| diameter | قطر | canonical native term, locked |
| reflect (geometry mirror) | قرینه‌سازی | geometry mirror operation |
| reflect (behavioral tracking) | هم‌گام‌سازی با | "X mirrors Y" behavioral tracking |
| reflect (physics) | بازتاب | physical reflection |
| default(s) | پیش‌فرض(ها) | fallback value |
| normalize / normalization | بهنجارسازی | translated, not transliterated |

## 11. Disambiguation policy
- Correct technical sense chosen per term; no qualifier invented that is absent from the source.
- Program context (NEC2 simulator) already disambiguates جریان/بار as electrical; no "الکتریکی" qualifier added unless the source itself qualifies it.
- A qualifier is added only where the Persian term would otherwise be genuinely ambiguous outside that context.
- Accepted intra-domain homonym: بار serves both charge (بار الکتریکی) and load (بار امپدانس); the compound qualifier keeps them distinct.
- No locative homonym collision identified in the current catalog.
- Gerund vs noun: "…سازی" nominalizes a process (eg رندرسازی), distinct from the bare noun/imperative pair used elsewhere for the same root.

## 12. Cross-catalog consistency
- One Persian term per concept, reused from topic 10 everywhere it recurs; no synonym introduced for an already-mapped concept.
- False-friend pairs, each side locked to a distinct term:
  - polarity قطبیت vs polarization قطبش
  - magnitude دامنه vs amplitude دامنه نوسان
  - peak value مقدار اوج vs peak magnitude دامنه اوج
  - ground زمین vs earth خاک
  - load بار (امپدانس) vs charge بار الکتریکی
  - gain بهره vs amplification تقویت vs profit سود
  - current (electrical) جریان vs current (temporal) اخیر/کنونی
  - charge (electrical) بار الکتریکی vs billing/fee هزینه
  - wire سیم vs cable/cord کابل; wire سیم vs thread رشته (not homonymous in Persian)
  - radiation pattern الگوی تشعشع vs template/design قالب/طرح vs far field میدان دور
  - excitation تحریک vs emotional excitement هیجان
  - node/antinode گره/پادگره vs generic numeric null/zero صفر
  - scale family/color tone تن vs hue رنگ‌مایه vs palette kind نوع پالت vs color projection فرافکنی رنگ
  - comet دنباله‌دار vs geometry هندسه
  - identity همانی vs unity یکه
  - renderer رندرر vs render engine (forbidden: موتور رندر)
  - override لغو مقدار vs overwrite رونویسی
  - viewer نمایشگر vs observer/speaker/preview ناظر/گوینده/پیش‌نمایش
  - reflect (geometry) قرینه‌سازی vs mirrors (behavioral) هم‌گام‌سازی با vs reflection (physics) بازتاب
  - structure ساختار vs construction سازه
  - theme پوسته vs topic/subject موضوع
  - validation اعتبارسنجی vs verification راستی‌آزمایی
  - net gain بهره خالص vs real-part gain بهره بخش حقیقی
  - power (electrical) توان vs Power (scale-family name) تابع توانی
- Loanword vs native, canonical spelling locked: پچ, رندرر, تگ, یادیار.
- Minority-outlier spellings unify to the locked canonical form.
- Catalog consistency outranks locale-form preference per topic 13's precedence, eg پچ/رندرر/تگ stay transliterated over a more "natural" native paraphrase.

## 13. Priority ordering
- Precedence: correct meaning > interface-register convention (topic 7) > catalog consistency (topic 12) > disambiguation (topic 11) > numeral/locale form (topic 2).
- Override ruling: پچ, رندرر, تگ stay transliterated against a more natural native paraphrase, because catalog consistency with the existing NEC2/UI corpus outranks locale-form preference.

## 14. Grammatical number
- Persian distinguishes singular and plural nouns but does not require plural marking after an explicit literal count; use the singular noun after a numeral.
- Use ها with the required ZWNJ for an uncounted grammatical plural when the sentence requires plurality; keep adjectives and participles uninflected for number.
- Conjugate a finite verb for the grammatical number of an explicit animate subject; use the standard singular agreement permitted for inanimate plural subjects.

## 15. Grammatical agreement
- Persian has no grammatical gender or adjective declension: add no gender concord or gender-based standalone-label form.
- Adjectives and participles follow the head noun through ezafe and remain unchanged for number; an implied head in a standalone label changes no adjective form.
- Use the singular noun after an explicit count; Persian has no partitive-after-count form.
- Retain the reflexive pronoun خود when the intended Persian construction requires reflexive meaning.

## 16. Morphological derivation
- Form borrowed technical verbs with a loanword plus کردن, eg رندر کردن; never inflect a borrowed noun as a Persian verb stem.
- Form process nouns with the established native root or the productive suffix سازی, eg نرم‌سازی.
- Prefer the topic-10 native term and standard Persian compounding; transliterate only a locked topic-4 loanword.

## 17. Preposition and sandhi selection
Not applicable: Persian has no context-conditioned preposition allomorphy, obligatory elision, or contraction in these literals; write ezafe according to standard Persian orthography.

## 18. Card/record-label register
- Use کارت <mnemonic> as the fixed designator in dialog and editor titles, eg کارت GW.
- Use the same spaced کارت <mnemonic> form in running prose because Persian has no distinct suffixed record designator.
- Keep کارت uncapitalized and unabbreviated; preserve the retained mnemonic under topic 4.
- Keep title and prose instances internally consistent; do not add a hyphen or suffix to either register.

## 19. Multi-paragraph and whitespace fidelity
- Mirror every source paragraph break at the same position, preserving blank lines and single line breaks by kind.
- Drop any trailing clause absent from the current source instead of retaining inherited text.
- Carry source trailing newlines and punctuation mechanically.
- Preserve semantic source line breaks; add no line break for visual wrapping.
- Preserve complete natural meaning; never truncate or abbreviate for an assumed display limit.

## 20. Current-source fidelity
- Derive every translation from the current source literal and its supplied context.
- Reuse an inherited translation only when its complete meaning agrees with the current source.
- Treat inherited بار, تن, مقیاس, and دامنه as unsafe until their topic-10 senses match the current source concept.

## 21. Script hygiene
Treat script hygiene as a zero-failure invariant across translated prose.
- Use no Arabic look-alikes ك ي, Eastern Arabic-Indic digits ٠-٩, or Latin homoglyphs for Persian letters outside a retained topic-4 token whose spelling requires them.
- Translate ordinary foreign prose words; retain only genuine identifiers, units, symbols, and named tokens under topic 4.

## 22. Rule-file scope hygiene
- Keep only current decisions that can alter wording, Unicode characters, punctuation, capitalization, embedded tokens, mnemonic markers, or semantic whitespace in a translated literal.
- Omit catalog representation, serialization, headers, flags, workflow, rendering, fonts, shaping, layout, widget behavior, runtime formatting, sorting, search, display sizing, audit history, completion state, review metadata, provenance, and citations.
- State each decision directly; include no implementation procedure, bibliography, correction history, or alternative policy source.

## 23. Section-disjointness declaration
- Topics 1-3 and 21 govern script, characters, direction, punctuation, and orthographic spacing only.
- Topics 5-7 and 10-20 govern token placement, wording, grammar, lexicon, and semantic whitespace only.
- Topics 8-9 govern address, formality, and mnemonic placement only.
- Assign each decision to one axis; shared spelling never merges distinct topic-10 concepts.

## 24. Developer/debug-string policy
- Translate user-facing interface, warning, error, notice, and confirmation strings in full under topics 7-8.
- Translate informational status and progress strings in terse technical Persian under topic 7.
- Keep developer-only debug and internal diagnostic strings in the source language unless an established sibling family in the same subsystem is user-visible and translated.
- Preserve every identifier, function name, format specifier, unit, and retained token under topics 4-5 regardless of string family.
- Use review priority only to order linguistic attention; never leave an applicable user-facing or informational string untranslated because of its priority.
- Render the `BUG:` diagnostic prefix as "اشکال:", held distinct from the "خطا:" prefix that carries ordinary failures; the two severities never share a word.
- Keep an identifier-plus-value diagnostic pair contiguous (eg `sweep_state=%d`); a qualifier precedes the whole pair and never stands between the identifier and its `=`.
