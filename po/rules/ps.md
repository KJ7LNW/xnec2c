# ps translation rules

## 1. Script and orthography
- Scope: standard modern Pashto for Afghanistan, written in the Pashto Perso-Arabic script for professional electrical and radio-frequency engineers, antenna designers, and amateur-radio operators.
- Write right-to-left Pashto prose; keep embedded left-to-right numbers, format specifiers, unit symbols, identifiers, and NEC2 mnemonics in source order, unmirrored, without manual direction marks.
- Preserve the required Pashto letters and orthographic forms: ټ ځ څ ډ ړ ږ ښ ګ ڼ ی ې ۍ ئ; never strip required marks or substitute look-alike code points.
- Use precomposed characters where Unicode provides them; do not construct a precomposed Pashto letter from a base letter and combining mark.
- Use `U+200C` ZERO WIDTH NON-JOINER only where Pashto orthography requires a visible joining break within a word; never substitute an ASCII apostrophe, spacing character, or manual direction mark.
- Pashto has no letter-case distinction; topic 6 therefore governs retained cased tokens and mnemonics only.
- Follow one-space inter-word separation; place one space between Pashto text and an embedded number or retained technical token unless source-token syntax requires adjacency.
- Fuse established single-concept compounds, space grammatical multi-word technical phrases, and use a hyphen only when the established written form requires one.

## 2. Numerals in literals
- Use Western Arabic digits `0-9` for technical values in Pashto prose.
- Use `.` as the decimal separator; introduce no thousands separator or digit grouping.
- Keep formulas, worked examples, fixed defaults, and named mathematical or standards constants in source form, including their digits and separators.
- Index or ordinal digit-plus-affix forms do not occur in this catalog; not applicable.
- Keep an index separator distinct from the decimal point wherever both occur in one literal.

## 3. Punctuation and quotation
- Use `«…»` for quoted Pashto prose; retain source quotation marks inside identifiers, formulas, filenames, and other technical tokens to preserve token spelling.
- Replace prose commas, semicolons, and question marks with `،`, `؛`, and `؟`; Pashto has no separate opening punctuation mark, so none is added.
- Attach colons, semicolons, and terminal punctuation to the preceding text without a space; retain the ASCII `:` after field labels.
- Use `…` for a prose ellipsis; preserve source hyphens and dashes in ranges, signs, identifiers, and technical notation.
- End full Pashto sentences with `۔`; omit a terminator from short labels and fragments.
- Keep punctuation within numbers, units, mnemonics, format specifiers, identifiers, and other embedded technical runs in source form.

## 4. Never-translate tokens
- Preserve every retained token exactly, with zero translation, transliteration, character substitution, mirroring, addition, or omission.
- Keep NEC2 card mnemonics verbatim: `GW GA GH EX LD FR RP GE EN SP SM SC NE NH NT TL GN GD GC GX GR GS GM GF CP CM SY XQ EK KH PQ PT`.
- Keep unit symbols verbatim: `Hz kHz MHz GHz dB dBi Ω W K S/m deg %`.
- Keep figure-of-merit and parameter tokens verbatim: `VSWR S11 S12 S21 S22 Z Z0 F/B G/T`.
- Keep file extensions, literal filenames, and format specifiers verbatim.
- Keep embedded identifiers, function names, variable names, and configuration keys verbatim in developer strings.
- Keep physical and mathematical symbol letters verbatim, including `E H Z θ φ μ`.
- Keep the source spelling of `xnec2c NEC2 GSL GTK OpenGL Cairo Smith`.
- Keep named transfer functions and algorithms verbatim: `Log Asinh μ-law Reinhard Sigmoid Identity`; translate the descriptive `Power` family as `د توان قانون`.
- Use the locked Pashto loan forms `سیګمنټ`, `پیچ`, `ټاګ`, and `فورک` for the conditional terms segment, patch, tag, and process fork; these forms are translated literals, not retained source tokens.
- Apply topic 1 directionality to every retained token.

## 5. Format-specifier integrity
- Preserve exactly the source set of format specifiers: add none, omit none, and alter no conversion, flag, width, precision, length, position, or escaped percent.
- Keep source order by default; use positional specifiers only when natural Pashto grammar cannot preserve that order.
- Restructure Pashto wording around fixed specifier positions before introducing positional reordering.
- Keep every character and digit inside a specifier in source form.

## 6. Capitalization and title-case
- Pashto has no letter case; sentence case, title case, coordinated-name capitalization, and generic card-noun capitalization are not applicable.
- Preserve the source case of retained acronyms, proper names, NEC2 mnemonics, axis letters, and physical or coordinate variables; keep lowercase source variables lowercase.

## 7. Interface register by string type
- Commands, buttons, and menu actions: omit the subject; use a polite plural imperative with the object before the final verb.
- Field labels: use a short noun phrase in natural modifier-head order followed by the retained ASCII colon; use no verb or subject.
- Dialogs and confirmations: use full subject-object-verb sentences when an explicit subject is natural; otherwise use an impersonal clause, placing the action and consequence before the final verb.
- Tooltips: use the shortest complete declarative sentence; state function first and the source-supplied reason for unavailability second.
- Status and error messages: use terse passive or impersonal declaratives, present the affected entity before the condition, and place the verb last.
- User-visible domain entities: use the locked topic-10 term as the head and place Pashto qualifiers before or around it according to natural genitive construction.
- Prefer natural compounds or short multi-word phrases; omit no meaning, coin no opaque abbreviation, and do not imitate source length.
- Classify developer and debug strings under topic 24.

## 8. Formality and address
- Pashto has no formal/informal pronoun opposition used by this interface; realize respectful neutrality through polite plural imperatives and impersonal declaratives.
- Omit an explicit addressee in commands, status text, and errors; use second-person plural verb morphology only where a command requires it.
- Avoid gender marking when the source is neutral; where grammar requires agreement, follow the grammatical gender and number of the topic-10 head noun without implying a person's gender.
- Use inclusive role nouns without adding sex, social rank, or animacy absent from the source.
- Honorifics and personal address do not occur in this catalog; not applicable.
- Preserve a person's supplied name order and add no title; grammatical person beyond command morphology is not applicable.
- Commands use polite plural imperatives; dialogs use neutral full sentences; status and errors use passive or impersonal forms.
- Confirmation dialogs state the proposed action and consequence in one complete neutral sentence.
- Exclude casual, slang, commercial, archaic, and ceremonially over-formal wording.

## 9. Accelerator/hotkey mnemonics
- Preserve the GTK `_` marker only when the source literal contains one; add no mnemonic to an unmarked source literal.
- For Pashto-script labels, append one readily typed Latin mnemonic in the form ` (_X)`; select `X` from the catalog's locked Latin spelling of the translated term, never from the source mnemonic.
- Keep the selected Latin letter free of dead-key and diacritic input.
- For a retained Latin label, place `_` immediately before the selected mnemonic character within the label.

## 10. Domain lexicon

| Concept | Term | Sense / purpose |
|---|---|---|
| current | جریان | electrical current (A); not temporal "recent" |
| charge | چارج | EM charge (C); kept distinct from load (بار) |
| voltage | ولتاژ | electric potential |
| power (electrical) | توان | radiated/dissipated watts and power flow; distinct from a mathematical exponent and `د توان قانون` (Power transfer family) |
| impedance | امپیدنس | complex Z; distinct from resistance and reactance |
| resistance | مقاومت | real part of Z; distinct from impedance and load |
| reactance | ری‌اکتانس | imaginary part of Z |
| inductance | انډکتانس | magnetic energy-storage property; locks the engineering sense |
| capacitance | کپیسیتانس | electric charge-storage property; locks the engineering sense |
| conductivity | هدایت | material S/m |
| admittance | ادمیتانس | distinct from impedance |
| load | بار | LD-card impedance load; not physical weight; kept distinct from چارج (charge) |
| gain | ګټه | antenna directivity ratio (dB); feminine noun, agreement per topic 15; never transliterated as ګین |
| excitation | تحریک | EM energy input/source; not emotional excitement |
| feedpoint | فیډ پواینټ | antenna feed point, transliterated |
| port | پورټ | excitation/S-parameter port |
| radials | رادیالونه | ground-plane radial wires (noun); distinct from adjective "radial" |
| ground / ground plane | ګراونډ | RF electrical reference plane, GN/GD cards, one term across all ground sub-senses; not soil |
| earth (physical medium) | ځمکه | terrain/noise-model earth, "below ground" geometry; distinct from ګراونډ |
| ground wave | د ګراونډ څپه | propagation term; distinct from the ground reference itself |
| wire | تار | thin conductor / GW element; not cable/cord |
| segment | سیګمنټ | NEC2 geometry subdivision, transliterated (topic 4) |
| patch | پیچ | NEC2 surface patch (SP/SM), transliterated (topic 4) |
| tag | ټاګ | NEC2 geometry identifier, transliterated (topic 4); not a UI label or card |
| card | کارډ | NEC2 input record; register in topic 18 |
| kernel | کرنل | integral-equation/thin-wire kernel; not an OS kernel |
| cliff | کلیف | two-medium ground-boundary type; not a fracture |
| structure | جوړښت | antenna model geometry |
| model | ماډل | NEC model or noise-temperature model |
| geometry | جیومیتري | the model geometry |
| crossed | کراس شوی | transmission-line conductors crossed/reversed; not پرې شوی (cut/severed) |
| field (EM) | ساحه | near/total/E/H field; distinct from a data/config field |
| near field | نږدې ساحه | opposed spatial region, symmetric with far field |
| far field | لرې ساحه | opposed spatial region, symmetric with near field |
| far-field contribution | د لرې ساحې ونډه | per-direction contribution; not near-field animation |
| radiation | تشعشع | radiated emission |
| radiation pattern | د تشعشع نمونه | plotted directional response; not a template/design |
| gain pattern | د ګټې نمونه | the gain radiation pattern |
| polarization | پولاریزیشن | antenna/wave field orientation |
| polarity | قطبیت | sign (+/-); false friend of پولاریزیشن |
| phase | فاز | angular position of a periodic quantity; locks the electromagnetic sense |
| reference phase | مرجع فاز | phase used as the comparison origin; distinguishes it from an unconstrained phase |
| frequency | فریکونسي | oscillation rate; locks the radio-frequency sense |
| wave / wavelength | څپه / د څپې اوږدوالی | propagating oscillation / its spatial period; keeps the related concepts distinct |
| standing wave | ولاړه څپه | opposed pair with traveling wave |
| traveling wave | تلونکې څپه | opposed pair with standing wave |
| node / antinode | نوډ / انټي‌نوډ | standing-wave zero/maximum; also the "null/peak" overlay sense |
| crest | د څپې سر | instantaneous wave apex (comet-head); distinct from چوکۍ (curve/step peak) |
| magnitude | قدر | modulus of a quantity (|Z|, scalar); distinct from امپلیتیوډ |
| amplitude | امپلیتیوډ | oscillating-quantity peak; distinct from قدر |
| peak value | چوکۍ ارزښت | distinct UI option from peak magnitude, never collapsed |
| peak magnitude | چوکۍ قدر | distinct UI option from peak value, never collapsed |
| instantaneous | سمدستي | projection mode; "(φ=0)" qualifier added only where source carries it |
| Poynting vector | د پوینټنګ ویکتور | proper name, transliterated |
| solid angle | کلک زاویه | three-dimensional angular measure; distinguishes it from a planar angle |
| net gain | خالص ګټه | total-minus-mismatch gain; not "real (part) gain" |
| viewer | لیدونکي | observation direction / 3D view widget; oblique لیدونکي, nominative لیدونکی; never ویونکي (speaker) |
| flow / flow direction | بهیر / د بهیر لوری | patch/current flow |
| total field | ټوله ساحه | combined electromagnetic field; distinguishes it from an individual contribution |
| color | رنګ | general color concept; distinct from رنګیزه زاویه (hue) |
| color projection | د رنګ پروجیکشن | which quantity drives hue |
| hue | رنګیزه زاویه | color-wheel angle; separates hue from رنګ (color), د پالیټ ډول, and د رنګ پروجیکشن |
| brightness | روڼتیا | luminance channel; established, not "transparency" |
| hue encoding | د رنګ کوډ کول | distinct internal enum from color projection |
| brightness encoding | د روڼتیا کوډ کول | distinct internal enum from color projection |
| color scale | د رنګ پیمانه | magnitude-to-color scale |
| scale family / color tone | د پیمانې کورنۍ | transfer-curve family; one term for both source spellings, distinct from hue, palette kind, and color projection; member labels are `د توان قانون Log Asinh μ-law Reinhard Sigmoid Identity` |
| palette / palette kind | پالیټ / د پالیټ ډول | distinct from scale family and color projection |
| ramp / gradient | رمپ / تدریجي بدلون | a palette kind / linear color strip |
| gamma | ګاما | power-law exponent |
| knee | زنګون | soft-knee bend point |
| softening | نرمول | dynamic-range curve smoothing; distinct from geometric smoothing |
| compression | فشردنی | dynamic-range compression |
| contrast | کنټراست | separation between displayed tonal levels; locks the color-transfer sense |
| dynamic range | متحرک ساحه | keep "dB" unit |
| floor | لاندنۍ حد | minimum/lower clamp (brightness/dB floor); not a room floor |
| envelope | پوښ | magnitude or amplitude boundary; distinct from لایه (overlay) |
| comet | کامېټ | moving-crest overlay effect, transliterated parallel to Latin scale-family names; not geometry |
| overlay (noun) | لایه | added visual layer; distinct from پوښ (envelope) and پوښنه (verification, unrelated sense) |
| animate / animation | انیمیشن کول / انیمیشن | action / resulting motion display; preserves verb and noun roles |
| animated / static | متحرک / ثابت | category-header adjectives; متحرک only for this pairing, not general "animation" |
| projection | پروجیکشن | color or geometry projection |
| scale (verb) | پیمانه کول | change a displayed or numeric scale; preserves the action sense |
| scale (noun) | پیمانه | magnitude or display scale; preserves the noun sense |
| wireframe | تار جالۍ | wire-mesh render mode |
| identity | آیډنټیټي | no-op/passthrough transfer; distinct from یووالی (unity, Smith-chart) |
| sentinel | سینټینل | unreachable-case guard value |
| bins | بینونه | discretization buckets |
| companding | کمپاندینګ | bounded log curve (μ-law) |
| tone mapping | د ټون نقشه کول | photographic tone-map |
| renderer | رینډرر | drawing backend; not "render engine" |
| shader | شیډر | graphics shading program; locks the rendering-domain sense |
| allocation (memory) | تخصیص | memory reservation act or result; distinct from assignment of a value |
| managed allocator | مدیریت شوی تخصیص کوونکی | allocator/report |
| thread | تريډ | compute thread; distinct from تار (wire conductor) |
| widget | ویجیټ | interface element; locks the user-interface sense |
| validation | اعتبارسنجونه | validation-tree feature; distinct from تایید (verification checks) |
| batch mode | بیچ حالت | non-interactive grouped processing mode; locks the program-mode sense |
| fork (process) | فورک | kept verbatim (topic 4) |
| deadlock | ډیډلاک | threads waiting cyclically with no progress; locks the concurrency sense |
| notifier | خبرورکوونکی | mechanism that signals a state change; distinct from a user notice |
| token | ټوکن | expression-parser term |
| operand | عملوند | expression-parser term |
| operator | عملګر | expression-parser term |
| arity | اریتي | expression-parser term |
| override | اوورراید | supersede a value (SY symbol); not بیا لیکل (overwrite) |
| swap | بدلول | exchange |
| theme | بڼه | interface and color theme; distinct from موضوع (topic or subject) |
| noise / noise temperature | شور / د شور تودوخه | electronic/thermal noise; not acoustic racket |
| efficiency | موثریت | useful output ratio; locks the antenna-performance sense |
| interpolation | انټرپولیشن | estimation between computed samples; locks the numerical-method sense |
| mnemonic | مینیمونیک | a card's code descriptor; not a memo/note |
| degrees / deg | درجې / (deg) | freestanding prose "degrees" vs the parenthetical unit tag, which is kept like other unit tags (topic 4) |
| diameter | قطر | canonical spelling; no synonym introduced |
| reflect (geometry mirror) | عکس اچونه | geometry mirror operation |
| reflect (behavioral mirrors) | تعقیبول | a control tracking another |
| reflect (physics) | انعکاس | physical reflection |
| defaults | ډیفالټ | fallback value |
| normalize / normalization | نورمالول / نورمالیزه | scale relative to a reference / that operation; locked canonical forms prevent synonym drift |


## 11. Disambiguation policy
- Domain context (EM simulator) selects the correct technical sense for each ambiguous term; no qualifier is added beyond what the English source carries.
- Add a qualifier only when the locked Pashto term would otherwise remain genuinely ambiguous; topic 10 already distinguishes `رنګ` (color) from `رنګیزه زاویه` (hue).
- Accepted intra-domain homonym: بار (load) is not merged with چارج (charge), per topic 12.
- Locative homonym: لوری (direction) is kept distinct from لار (path/way), never used interchangeably.
- Gerund vs noun: تخصیص covers both the allocation act and the allocation noun; context disambiguates, no separate coinage needed.

## 12. Cross-catalog consistency
- One term per concept, reused from the topic 10 table; no synonym introduced for an already-mapped concept.
- False-friend pairs (Appendix C), each side kept distinct:
  - قطبیت (polarity, sign) vs پولاریزیشن (polarization, wave/antenna orientation).
  - قدر (magnitude, modulus) vs امپلیتیوډ (amplitude, oscillating peak).
  - چوکۍ ارزښت (peak value) vs چوکۍ قدر (peak magnitude).
  - ګراونډ (ground, electrical reference) vs ځمکه (earth, physical medium).
  - بار (load) vs چارج (charge); never merged despite both glossing loosely as "burden" in general Pashto.
  - ګټه (antenna gain) vs تقویه (amplifier amplification) vs سود (profit).
  - جریان (electrical current) vs اوسنی (present/current in time).
  - چارج (electrical charge) vs فیس (billing fee).
  - تار (wire conductor) vs کېبل (cable or cord) vs تريډ (compute thread).
  - د تشعشع نمونه (radiation pattern) vs قالب (template or design) vs لرې ساحه (far-field region).
  - تحریک (electromagnetic excitation) vs هیجان (emotional excitement).
  - نوډ / انټي‌نوډ (standing-wave node/antinode) vs a generic numeric zero (صفر).
  - د پیمانې کورنۍ (scale family/color tone) vs رنګیزه زاویه (hue) vs د پالیټ ډول (palette kind) vs د رنګ پروجیکشن (color projection): four distinct chroma concepts.
  - کامېټ (comet, overlay) vs جوړښت (geometry): a known fuzzy-inheritance hazard (topic 20).
  - آیډنټیټي (identity, no-op transfer) vs یووالی (unity, Smith-chart).
  - رینډرر (drawing backend) vs د رینډر انجن (render engine).
  - اوورراید (override, supersede) vs بیا لیکل (overwrite).
  - لیدونکي (viewer, observation/3D view) vs ویونکي (speaker) or a preview sense.
  - عکس اچونه (reflect, mirror op) vs تعقیبول (mirrors, behavioral tracking) vs انعکاس (reflection, physics).
  - جوړښت (model structure) vs ودانول (construction).
  - بڼه (interface theme) vs موضوع (topic or subject).
  - اعتبارسنجونه (validation-tree feature) vs تایید (verification checks).
  - خالص ګټه (net gain) vs real-part gain.
  - توان (electrical power) vs د توان قانون (Power transfer family).
- Use each topic-10 loanword or native form with its exact locked spelling; use `اپتیمایزر` and `امپیدنس`, not variant spellings.
- Replace every minority spelling with its topic-10 canonical form.
- Catalog consistency outranks a competing locale spelling for `نورمالول`, `نورمالیزه`, `اپتیمایزر`, and `امپیدنس`.

## 13. Priority ordering
- Precedence chain when decisions conflict: correct meaning first, then interface-register convention (topic 7), then catalog consistency (topic 12), then disambiguation (topic 11), then locale numeral form (topic 2).
- Explicit override: the locked `نورمالیزه` form overrides native-root preference because catalog consistency has higher precedence.

## 14. Grammatical number
- Pashto translated literals use singular and plural noun, adjective, participle, and verb forms.
- Use singular agreement for an explicit count of one and the natural plural construction for other explicit counts.
- Match coordinated nouns with plural agreement unless the phrase has one grammatical head.

## 15. Grammatical agreement
- Make adjectives and participles agree in gender and number with the explicit or implied head noun.
- Resolve a standalone label through its topic-10 domain noun; use that noun's gender and number, eg feminine `ګټه` takes `ټوله` and `نورمالیزه شوې`.
- Use the established declension class of each locked term; introduce no partitive form after a count because Pashto technical count phrases in this catalog do not require one.
- Retain a reflexive particle whenever the selected Pashto verb requires it.

## 16. Morphological derivation
- Form borrowed technical verbs with the Pashto light verbs `کول` or `کېدل`; attach no invented verbal suffix to a loan root.
- Form verbal nouns from the `کول` infinitive construction.
- Prefer an established native root; otherwise combine the locked loan noun with `کول` or `کېدل`.
- Apply topic 1 compound spacing to every derived form.

## 17. Preposition and sandhi selection
- Pashto in this catalog has no context-conditioned preposition alternation, mandatory elision, or contraction; not applicable.

## 18. Card/record-label register
- Use `کارډ` as the fixed NEC2 record designator.
- In dialog and editor titles, place the retained mnemonic before the designator: `GW کارډ`.
- In running prose, inflect or govern `کارډ` naturally within the sentence; do not hyphenate it to the mnemonic.
- Pashto has no generic-noun casing or separate short and long card form; not applicable.
- Keep title and running-prose forms within their stated registers.

## 19. Multi-paragraph and whitespace fidelity
- Mirror each source paragraph break at the same position, preserving the distinction between a blank line and a single line break.
- Preserve semantic line breaks; introduce no line break for visual wrapping.
- Drop any trailing clause absent from the current source rather than retaining inherited wording.
- Preserve source trailing newlines and the punctuation policy selected for the translated literal.
- Preserve complete meaning; use no truncation or unnatural abbreviation based on an assumed display width.

## 20. Current-source fidelity
- Derive every translation from the current source literal and its supplied context.
- Reuse an inherited translation only when its complete meaning matches the current source and context.
- Treat inherited wording for `polarity/polarization`, `current/charge`, `near field/far-field contribution`, chroma diagnostics, and standing-wave `node/null` as unsafe until resolved through topics 10-12.

## 21. Script hygiene
- Require zero wrong-script or homoglyph substitutions in Pashto prose: forbid `ي`, `ى`, and other non-Pashto look-alikes where topic 1 requires `ی`, `ې`, `ۍ`, or `ئ`.
- Permit a foreign-script character only inside a topic-4 retained token whose exact spelling contains it.
- Translate ordinary foreign words into Pashto; retain only genuine identifiers, names, units, mnemonics, symbols, filenames, and format tokens.
- Forbid mixed-script adjacency inside a Pashto word; separate an embedded retained token according to topic 1.

## 22. Rule-file scope hygiene
- Keep only current decisions that can alter wording, Unicode characters, punctuation, capitalization, embedded tokens, mnemonic markers, or semantic whitespace in a translated literal.
- Omit catalog representation, serialization, headers, flags, workflow, rendering, fonts, shaping, layout, widget behavior, runtime formatting, sorting, search, display sizing, audit history, completion state, review metadata, provenance, and citations.
- State the current decision directly; include no implementation procedure, correction history, or alternative policy source.

## 23. Section-disjointness declaration
- Topics 1, 3, and 21 govern script characters and punctuation; topics 5-7, 9, 13, and 18-19 govern token and string construction; topics 8 and 15 govern address and agreement.
- Apply each decision on one axis only; use topic 10 as the sole term-mapping authority.

## 24. Developer/debug-string policy
- Translate user-facing commands, labels, dialogs, confirmations, tooltips, status text, and errors into Pashto; their review priority never permits an applicable string to remain untranslated.
- Translate informational notices and reports into concise technical Pashto.
- Translate developer-facing diagnostics, parser messages, allocator reports, assertions, and debug text into terse impersonal technical Pashto.
- Keep identifiers, functions, variables, configuration keys, format specifiers, and all topic-4 retained tokens verbatim in every family.
- Apply the same token-preservation rules regardless of family or review priority; no subsystem family overrides this policy.
</content>
