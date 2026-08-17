# ur translation rules

## 1. Script and orthography

Apply standard Pakistani Urdu orthography for professional electrical and RF engineers, antenna designers, and amateur-radio operators.

- Use the Urdu Perso-Arabic abjad in right-to-left prose.
- Use all required Urdu letters, including پ، چ، ژ، گ، ٹ، ڈ، ڑ، ں، ھ، ہ، ی، and ے; retain every required diacritic.
- Normalize Urdu prose to Unicode NFC; use precomposed letters where available and combining marks only where no precomposed character exists.
- Use U+06CC for ی, U+06D2 for ے, and U+06A9 for ک; never fold them to look-alike code points.
- Use U+0621 and the contextually correct hamza carrier for hamza; use U+0650 for izafat where written and never substitute an apostrophe.
- Insert U+200C only where an orthographically required non-joining boundary changes the literal; never insert U+200D or manual direction controls.
- Keep embedded numerals, units, mnemonics, identifiers, and format specifiers left-to-right in their source order without mirroring.
- Treat Urdu as caseless; topic 6 governs retained source-script letter case only.
- Separate Urdu words with one space; use one space at each boundary between Urdu prose and an embedded foreign or numeric token.
- Write established technical compounds as spaced words; use a hyphen only when the retained source token contains it, and never fuse separate lexical words.

## 2. Numerals in literals

- Use digits 0-9 for technical values in Urdu prose; do not use ۰-۹.
- Use "." as the decimal separator and "," as the thousands separator only where the source literal groups digits.
- Preserve formulas, examples, fixed defaults, and named mathematical or standards constants exactly in source form.
- Join a literal index digit to its Urdu affix according to Urdu morphology; keep any index separator distinct from the decimal period.

## 3. Punctuation and quotation

- Use Urdu quotation marks ” “ for quoted Urdu prose; retain plain source quotes around embedded technical tokens for consistent token boundaries.
- Use "،" and "؟" in Urdu prose; retain source exclamation marks and any opening punctuation.
- Attach commas, semicolons, question marks, exclamation marks, and "۔" to the preceding word; place one space after them when text follows.
- Retain ASCII colon and parentheses around technical runs; attach a colon to the preceding label and place one space after it when prose follows.
- Use "..." for an ellipsis and preserve each source hyphen or en dash.
- End full dialogs, confirmations, errors, and other sentences with "۔"; omit a terminator from short labels, menu items, and fragments.
- Preserve source punctuation inside identifiers, units, mnemonics, formulas, and format specifiers.

## 4. Never-translate tokens

Treat every retained token as an exact, zero-failure invariant: preserve spelling, characters, case, punctuation, and left-to-right order; never translate or transliterate it.

- Retain NEC2 card mnemonics: GW GA GH EX LD FR RP GE EN SP SM SC NE NH NT TL GN GD GC GX GR GS GM GF CP CM SY XQ EK KH PQ PT.
- Retain unit symbols used by the catalog, including Hz, kHz, MHz, GHz, dB, dBi, Ω, V, A, W, K, S/m, deg, °, and %.
- Retain RF parameter tokens, including VSWR, S11, S12, S21, S22, Z, Z0, F/B, and G/T.
- Retain literal file extensions, including .nec, .csv, .s1p, .s2p, .png, and every extension present in the source token.
- Retain every printf-style format specifier, including its flags, width, precision, length, conversion, and positional index.
- Retain embedded function names, variable names, configuration keys, and other code identifiers.
- Retain physical and mathematical symbols, including E, H, θ, φ, μ, and exponent letters.
- Retain proper product, library, toolkit, and chart names exactly as supplied, including xnec2c and GTK.
- Retain Log, Asinh, μ-law, Reinhard, Sigmoid, and Identity; translate the descriptive Power transfer family as قوت.
- Transliterate conditional loanwords consistently: segment سیگمنٹ, patch پیچ, tag ٹیگ, and process fork فورک.

## 5. Format-specifier integrity

- Preserve exactly the source set of format specifiers with zero additions, omissions, or character changes.
- Keep source order by default; use positional specifiers only when Urdu grammar requires argument reordering.
- Restructure Urdu wording around fixed non-positional specifiers instead of moving them.
- Preserve every digit inside a specifier in source form.

## 6. Capitalization and title-case

- Use natural Urdu sentence style for labels, menu items, and titles; never imitate source title case through added source-script capitals.
- Preserve uppercase axis letters X, Y, and Z.
- Preserve lowercase mathematical and coordinate variables when the source distinguishes them.
- Preserve the case of each element in a coordinated retained option name.
- Write the generic Urdu card noun identically in every position; preserve card mnemonics and proper names exactly.
- Urdu has no native case distinction; all remaining case decisions apply only to retained tokens.

## 7. Interface register by string type

Treat interface text as concise professional Urdu with subject-object-verb order; preserve complete meaning and use the shortest natural form rather than imitating source length.

- Commands, buttons, and menu actions use a polite imperative with the subject omitted, object before verb, and verb last; use a noun phrase only for a non-action category.
- Field labels use a concise head-final noun phrase followed by the retained colon; omit subject and verb.
- Dialogs and confirmations use full formal sentences with stated objects, an omitted recoverable subject, the main verb last, and the decision or requested action at the end.
- Tooltips use complete declarative wording; when the source identifies an unavailable control, state the disabling reason before the unavailable action.
- Status and error messages use impersonal declarative or passive wording, cause before result, and no user subject.
- Name user-visible domain entities as modifier-before-head Urdu noun phrases; preserve proper names and topic 4 tokens in place.
- Prefer spaced compounds and complete short phrases; never omit meaning or use unnatural abbreviations.

## 8. Formality and address

- Use formal آپ when direct address is required; forbid تم and تو.
- Realize formality through polite plural verb endings, including کریں، دیکھیں، and منتخب کریں; never use casual imperative endings.
- Omit an explicit subject in commands, status text, and errors when Urdu grammar recovers it naturally.
- Avoid gendering the user; use gender-neutral constructions, or masculine agreement only where Urdu requires a default and no neutral form exists.
- Use inclusive collective wording without disrupting required gender, number, or animacy agreement.
- Use second-person formal grammar only for direct address; use no honorific with interface users or personal names.
- Preserve a supplied personal name in its source order; Urdu has no interface-specific name-order conversion.
- Use terse imperatives for commands and full polite sentences for dialogs and confirmations.
- Form confirmations as a full question or statement matching the source intent, with formal verb agreement and terminal punctuation.
- Exclude casual, slang, commercial, archaic, and ornamental over-formal registers.

## 9. Accelerator/hotkey mnemonics

- Preserve the source underscore marker only when the source literal contains one; never invent a mnemonic.
- Place the marker before a typable Latin mnemonic letter shown separately after the translated Urdu term in parentheses.
- Derive that letter from the translated Urdu term's established Latin transliteration, not from the source term.
- Keep mnemonic letters unique within each menu or dialog; choose the next typable transliteration letter when a collision occurs.
- Avoid letters confusable with digits or punctuation.

## 10. Domain lexicon

| Concept | Urdu term | Sense | Hazard guarded |
|---|---|---|---|
| current(s) | کرنٹ / کرنٹس | electrical, Amperes | not "حالیہ" recent/present |
| charge(s) | چارج / چارجز | electrical, Coulombs | not "معاوضہ" billing |
| voltage | وولٹیج | electric potential | established technical sense; catalog consistency |
| power (electrical) | پاور | radiated or dissipated watts, power gain, and power flow | distinct from the Power transfer-family term قوت |
| impedance | امپیڈینس | complex Z | distinct from resistance/reactance |
| resistance | مزاحمت | real part of Z | distinct from impedance and load |
| reactance | ری ایکٹنس | imaginary part of Z | distinct from resistance |
| inductance | انڈکٹینس | established technical sense; catalog consistency | catalog consistency |
| capacitance | کیپیسیٹینس | established technical sense; catalog consistency | catalog consistency |
| conductivity | چالکائیت | material S/m | native term |
| admittance | ایڈمیٹینس | admittance-matrix sense | distinct from impedance |
| load | لوڈ | LD-card impedance load | not physical weight/burden |
| gain | گین | antenna directivity ratio, dB | not "نفع" profit, not amplifier amplification |
| excitation | ایکسائٹیشن | EM energy input/source | not emotional excitement |
| feedpoint | فیڈ پوائنٹ | antenna feed point | established technical sense; catalog consistency |
| port | پورٹ | excitation/S-parameter port | established technical sense; catalog consistency |
| radials | ریڈیل تاریں | horizontal ground-plane radial wires, noun | distinct from adjective "radial" ریڈیل |
| ground / ground plane | گراؤنڈ / گراؤنڈ پلین | RF electrical reference plane, GN/GD, ground type/conductivity | not soil; one term across all ground sub-senses |
| earth (physical medium) | زمین | terrain/noise-model earth, below-ground geometry | distinct from electrical ground |
| ground wave | گراؤنڈ ویو | propagation term | distinct from ground reference |
| wire | تار | thin conductor / GW element | not cable/cord; one term across wire sub-uses |
| segment | سیگمنٹ | NEC2 geometry subdivision | established technical sense; catalog consistency |
| patch | پیچ | NEC2 surface patch, SP/SM | kept as loanword, not translated |
| tag | ٹیگ | NEC2 geometry identifier | not a UI label or a card |
| card | ڈیٹا کارڈ | NEC2 input record | register in topic 18 |
| kernel | کرنل | integral-equation/thin-wire kernel | not an OS kernel; context disambiguates |
| cliff | کلف | two-medium ground-boundary type | not a fracture/break |
| structure | ساخت | antenna model geometry | not "تعمیر" construction |
| model | ماڈل | NEC model or noise-temperature model | established technical sense; catalog consistency |
| geometry | جیومیٹری | the model geometry | established technical sense; catalog consistency |
| crossed | کراس شدہ | transmission-line conductors crossed/reversed | not cut/severed |
| field (EM) | فیلڈ | near/total/E/H field | distinct from a data/config field |
| near field / far field | نزدیکی فیلڈ / فار فیلڈ | opposed spatial regions | kept symmetric |
| far-field contribution | فار فیلڈ کا حصہ | per-direction contribution | not near-field animation |
| radiation | ریڈی ایشن | radiated emission | established technical sense; catalog consistency |
| radiation pattern | ریڈی ایشن پیٹرن | plotted directional response | not template/design; not far-field alone |
| gain pattern | گین پیٹرن | the gain radiation pattern | established technical sense; catalog consistency |
| polarization | پولرائزیشن | antenna/wave field orientation | false friend of polarity |
| polarity | قطبیت | sign, +/-, of a quantity | false friend of polarization |
| phase | فیز | established technical sense; catalog consistency | catalog consistency |
| reference phase | حوالہ فیز | established technical sense; catalog consistency | catalog consistency |
| frequency | فریکوئنسی | established technical sense; catalog consistency | catalog consistency |
| wave / wavelength | ویو / ویو لینتھ | established technical sense; catalog consistency | catalog consistency |
| standing wave / traveling wave | اسٹینڈنگ ویو / ٹریولنگ ویو | opposed pair | kept symmetric |
| node / antinode | نوڈ / اینٹی نوڈ | standing-wave zero/maximum, also null/peak overlay sense | not a generic numeric زero |
| crest | کرسٹ | instantaneous wave apex, comet-head | distinct from a curve/step peak چوٹی |
| magnitude | شدت | modulus of a quantity, \|Z\|, scalar | distinct from amplitude |
| amplitude | ایمپلیٹیوڈ | oscillating-quantity peak | distinct from magnitude |
| peak value | چوٹی کی قدر | established technical sense; catalog consistency | distinct UI option from peak magnitude |
| peak magnitude | چوٹی کی شدت | established technical sense; catalog consistency | distinct UI option from peak value |
| instantaneous | فوری | projection mode | "(φ=0)" qualifier added only where source carries it |
| Poynting vector | پوائنٹنگ ویکٹر | established technical sense; catalog consistency | catalog consistency |
| solid angle | سالڈ اینگل | established technical sense; catalog consistency | catalog consistency |
| net gain | نیٹ گین | total-minus-mismatch gain | not "حقیقی گین" real-part gain |
| viewer | ویور | observation direction and/or 3D view widget | not observer/speaker/preview |
| flow / flow direction | بہاؤ / بہاؤ کی سمت | patch/current flow | established technical sense; catalog consistency |
| total field | کل فیلڈ | established technical sense; catalog consistency | catalog consistency |
| color | کلر | established technical sense; catalog consistency | catalog consistency |
| color projection | کلر پروجیکشن | which quantity drives hue | established technical sense; catalog consistency |
| hue | ہیو | color-wheel angle | established technical sense; catalog consistency |
| brightness | برائٹنس | luminance channel | established technical sense; catalog consistency |
| hue encoding / brightness encoding | ہیو انکوڈنگ / برائٹنس انکوڈنگ | distinct internal enums | neither collapses to color projection |
| color scale | کلر اسکیل | magnitude-to-color scale | established technical sense; catalog consistency |
| scale family / color tone | کلر ٹون | transfer-curve family, Power/Log/Asinh/μ-law/Reinhard/Sigmoid/Identity | one concept, two source spellings, one term |
| palette / palette kind | پیلیٹ / پیلیٹ کی قسم | palette-layout enum | distinct from scale family and color projection |
| ramp / gradient | ریمپ / گریڈیئنٹ | palette kind / linear color strip | established technical sense; catalog consistency |
| gamma | گاما | power-law exponent | established technical sense; catalog consistency |
| knee | نی | soft-knee bend point | established technical sense; catalog consistency |
| softening | سافٹننگ | dynamic-range | established technical sense; catalog consistency |
| compression | کمپریشن | dynamic-range | established technical sense; catalog consistency |
| contrast | کنٹراسٹ | established technical sense; catalog consistency | catalog consistency |
| dynamic range | ڈائنامک رینج | established technical sense; catalog consistency | catalog consistency |
| floor | فلور | minimum/lower clamp, brightness/dB floor | not a room floor |
| envelope | اینویلوپ | magnitude/amplitude envelope | established technical sense; catalog consistency |
| comet | کومٹ | moving-crest overlay effect | not geometry |
| overlay (noun) | اوورلے / اوورلیز | added visual layer | distinct from verb "to overlay" |
| animate / animation | اینیمیٹ / اینیمیشن | established technical sense; catalog consistency | catalog consistency |
| animated / static | اینیمیٹڈ / اسٹیٹک | category-header adjectives | dynamic vs phase-invariant |
| projection | پروجیکشن | color or geometry projection | established technical sense; catalog consistency |
| scale | پیمانہ | verb and noun | established technical sense; catalog consistency |
| wireframe | وائرفریم | wire-mesh render mode | established technical sense; catalog consistency |
| identity | آئیڈینٹیٹی | no-op/passthrough transfer | distinct from unity وحدت, Smith-chart |
| sentinel | سینٹینل | unreachable-case guard value | established technical sense; catalog consistency |
| bins | بنز | discretization buckets | established technical sense; catalog consistency |
| companding | کمپینڈنگ | bounded log curve, μ-law | established technical sense; catalog consistency |
| tone mapping | ٹون میپنگ | photographic tone-map | established technical sense; catalog consistency |
| renderer | رینڈرر | drawing backend | not "render engine" |
| shader | شیڈر | established technical sense; catalog consistency | catalog consistency |
| allocation / managed allocator | میموری ایلوکیشن / میموری ایلوکیٹر | allocation and allocator/report | established technical sense; catalog consistency |
| thread | تھریڈ | compute thread | homonym with wire in some languages; not here, context disambiguates |
| widget | ویجٹ | UI element | established technical sense; catalog consistency |
| validation | ویلیڈیشن | the validation-tree feature | distinct from تصدیق verification |
| batch mode | بیچ موڈ | established technical sense; catalog consistency | catalog consistency |
| fork (process) | فورک | process fork | kept verbatim as loanword |
| deadlock | ڈیڈلاک | established technical sense; catalog consistency | catalog consistency |
| notifier | نوٹیفائر | established technical sense; catalog consistency | catalog consistency |
| token / operand / operator / arity | ٹوکن / آپرینڈ / آپریٹر / آرٹی | expression-parser terms | established technical sense; catalog consistency |
| override | اوور رائیڈ | supersede a value, SY symbol | not "اوور رائٹ" overwrite |
| swap | سویپ | exchange | established technical sense; catalog consistency |
| theme | تھیم | UI/color theme | not "موضوع" topic/subject |
| noise / noise temperature | نائز / نائز ٹمپریچر | electronic/thermal noise | not acoustic racket |
| efficiency | افادیت | established technical sense; catalog consistency | catalog consistency |
| interpolation | انٹرپولیشن | established technical sense; catalog consistency | catalog consistency |
| mnemonic | نیمونک | a card's code descriptor | not a memo/note |
| degrees / deg | ڈگری / (deg) | freestanding axis/prose vs parenthetical unit tag | tag treated like other unit tags, kept verbatim |
| diameter | ڈایامیٹر | canonical loanword | one choice, not native "قطر" |
| reflect | ری فلیکٹ / کی نقل کرنا / انعکاس | geometry mirror / behavioral tracking / physics reflection | three distinct senses |
| default(s) | ڈیفالٹ | fallback value | established technical sense; catalog consistency |
| normalize / normalization | نارملائز کرنا / نارملائزیشن | established technical sense; catalog consistency | loanword form, one choice |
| level | سطح | established technical sense; catalog consistency | catalog consistency |
| draw style | ڈرا اسٹائل | established technical sense; catalog consistency | catalog consistency |
| linear | لینیئر | established technical sense; catalog consistency | catalog consistency |
| loop | لوپ | established technical sense; catalog consistency | catalog consistency |
| save | محفوظ | established technical sense; catalog consistency | catalog consistency |
| surface patches | سطحی پیچ | established technical sense; catalog consistency | catalog consistency |
| memory allocation | میموری ایلوکیشن | see allocation | established technical sense; catalog consistency |
| optimizer | آپٹیمائزر | established technical sense; catalog consistency | catalog consistency |
| output thread | آؤٹ پٹ تھریڈ | see thread | established technical sense; catalog consistency |
| config / configuration | کنفیگ / کنفیگریشن | established technical sense; catalog consistency | catalog consistency |
| electric field | برقی فیلڈ | see field (EM) | established technical sense; catalog consistency |
| magnetic field | مقناطیسی فیلڈ | see field (EM) | established technical sense; catalog consistency |
| direction | سمت | established technical sense; catalog consistency | catalog consistency |
| axis | محور | established technical sense; catalog consistency | catalog consistency |
| value | قدر | established technical sense; catalog consistency | catalog consistency |
| peak | چوٹی | curve/step peak | distinct from crest کرسٹ |
| invalid | غلط | established technical sense; catalog consistency | catalog consistency |
| missing widget | غائب ویجٹ | see widget | established technical sense; catalog consistency |
| unknown widget type | نامعلوم ویجٹ قسم | see widget | established technical sense; catalog consistency |
| data card | ڈیٹا کارڈ | see card | established technical sense; catalog consistency |
| required | درکار | established technical sense; catalog consistency | catalog consistency |
| failed | ناکام | established technical sense; catalog consistency | catalog consistency |

## 11. Disambiguation policy

- The correct technical sense is chosen per term per topic 10's table.
- No qualifier absent from the source is added (eg "کرنٹس دیکھیں", not "برقی کرنٹس دیکھیں"); program context, an EM simulator, already disambiguates.
- A qualifier is added only where the Urdu term would otherwise be genuinely ambiguous outside domain context.
- Accepted intra-domain homonym: none currently reused across concepts in this catalog.
- No locative/other homonym collision identified.
- Gerund vs noun: progressive verb-phrase status strings (eg نظرانداز کیا جا رہا ہے, "ignoring ...") stay grammatically distinct from static noun-phrase terms (eg جیومیٹری, "geometry").

## 12. Cross-catalog consistency

- Use one locked topic 10 term per concept throughout the catalog; introduce no synonym for a mapped concept.
- Keep every Appendix C side distinct:
  - polarity قطبیت vs polarization پولرائزیشن.
  - magnitude شدت vs amplitude ایمپلیٹیوڈ.
  - peak value چوٹی کی قدر vs peak magnitude چوٹی کی شدت.
  - ground گراؤنڈ vs earth زمین.
  - load لوڈ vs charge چارج.
  - gain گین vs amplification ایمپلیفیکیشن vs profit نفع.
  - current کرنٹ vs present or recent حالیہ.
  - charge چارج vs billing or fee معاوضہ.
  - wire تار vs cable or cord کیبل vs compute thread تھریڈ.
  - radiation pattern ریڈی ایشن پیٹرن vs template or design سانچہ vs far field فار فیلڈ.
  - excitation ایکسائٹیشن vs emotional excitement جذباتی جوش.
  - node نوڈ and antinode اینٹی نوڈ vs generic numeric zero صفر.
  - scale family or color tone کلر ٹون vs hue ہیو vs palette kind پیلیٹ کی قسم vs color projection کلر پروجیکشن.
  - comet کومٹ vs geometry جیومیٹری.
  - identity آئیڈینٹیٹی vs Smith-chart unity وحدت.
  - renderer رینڈرر vs render engine رینڈر انجن.
  - override اوور رائیڈ vs overwrite اوور رائٹ.
  - viewer ویور vs observer مبصر, speaker مقرر, and preview پیش منظر.
  - geometry reflect ری فلیکٹ vs behavioral mirrors پیروی کرنا vs physical reflection انعکاس.
  - structure ساخت vs construction تعمیر.
  - theme تھیم vs topic or subject موضوع.
  - validation ویلیڈیشن vs verification تصدیق.
  - net gain نیٹ گین vs real-part gain حقیقی جزو گین.
  - electrical power پاور vs Power transfer family قوت.
- Prefer each established transliterated loanword over an alternate native calque; lock spelling to topic 10.
- Unify every outlier spelling to the topic 10 canonical form.
- Let catalog consistency override locale-form preference for segment سیگمنٹ, patch پیچ, tag ٹیگ, and fork فورک.

## 13. Priority ordering

- Precedence chain: correct meaning, then interface convention, then catalog consistency, then disambiguation, then locale numeral form.
- Override ruling: catalog consistency overrides locale-form preference for segment/patch/tag/fork (topic 12).

## 14. Grammatical number

- Use singular and plural noun forms; select singular only for an explicit count of 1 and plural for every other explicit count.
- Form established technical-loanword plurals with the locked -ز or -س suffix, including کرنٹس and چارجز.
- Inflect native Urdu nouns by their established singular, direct-plural, and oblique-plural patterns.
- Make adjectives, participles, and verbs agree with the selected grammatical number; keep invariant loanword modifiers unchanged.

## 15. Grammatical agreement

- Adjectives and participles agree in gender/number with the head noun, including standalone labels whose head noun is implied (eg "غائب" agrees with implied masculine ویجٹ).
- Loanword technical nouns take an invariant, oblique-case-neutral form; no native declension class applied to them.
- Partitive-after-count: not applicable, Urdu counted nouns require no partitive marker.
- Reflexive-particle retention: not applicable to this catalog's imperative/status strings.
- A standalone label defaults to masculine agreement unless its head noun is inherently feminine (eg فریکوئنسی is feminine, agreement follows).

## 16. Morphological derivation

- Borrowed technical verbs/nouns derive via loanword plus Urdu light verb (کرنا / ہونا), eg "سیو کرنا", "نارملائز کرنا"; coining a native-root verbalization is forbidden.
- Verbal-noun formation: an established -ing loanword (eg رینڈرنگ) is kept as-is rather than re-derived with a native suffix.
- Prefer the established loanword-plus-light-verb pattern over a native affix on a technical root.
- Compounding stays spaced two-word form (topic 1), not a fused derivational compound.

## 17. Preposition and sandhi selection

- Not applicable: Urdu postpositions (کا/کی/کے, میں, پر) select by gender/number of the governed noun, not by phonological sandhi; no sound-triggered form selection applies to this catalog.
- No elision/contraction rules required for this catalog.

## 18. Card/record-label register

- Fixed designator form in dialog/editor titles: "ڈیٹا کارڈ" (eg "GW ڈیٹا کارڈ").
- Distinct running-prose form in messages: suffixed reference (eg "... کارڈ میں").
- Generic-noun casing is not applicable, no case distinction (topic 6); short form "کارڈ" used in running prose, long form "ڈیٹا کارڈ" reserved for titles/headers.
- Each register stays internally consistent, never cross-converted between title and prose contexts.

## 19. Multi-paragraph and whitespace fidelity

- Mirror source paragraph breaks at the same positions, preserving blank lines and single line breaks by type.
- Drop any trailing clause removed from the current source instead of retaining stale translated text.
- Preserve source trailing newlines and punctuation.
- Preserve semantic line breaks; omit visual wrapping absent from the source literal.
- Preserve complete meaning; never truncate or abbreviate Urdu text to fit an assumed display limit.

## 20. Current-source fidelity

- Derive every translation from the current source text and supplied context.
- Reuse an inherited translation only when its complete meaning agrees with the current source.
- Never inherit prior mappings that merge comet with geometry, magnitude with amplitude, or peak value with peak magnitude.

## 21. Script hygiene

- Require zero forbidden homoglyphs in Urdu prose: use U+06CC ی and U+06D2 ے instead of U+064A ي, and U+06A9 ک instead of U+0643 ك.
- Permit another-script character only inside a retained token whose literal spelling requires it.
- Translate plain source-language words; retain genuine identifiers, units, and topic 4 tokens.

## 22. Rule-file scope hygiene

- Include only current decisions that can alter a translated literal's wording, Unicode characters, punctuation, capitalization, embedded tokens, mnemonic marker, or semantic whitespace.
- Omit catalog representation, serialization, headers, flags, workflow, rendering, fonts, shaping, layout, widget behavior, runtime formatting, sorting, search, display sizing, audit history, completion state, review metadata, provenance, and citations.
- State each retained decision directly; include no implementation procedure, correction history, or alternative policy source.

## 23. Section-disjointness declaration

- Script mechanics in topics 1-3 govern character and symbol form only.
- Phrasing and structure in topics 5-7 and 9-20 govern wording and sentence construction only.
- Address register in topic 8 governs pronouns and politeness only.
- Keep these axes non-overlapping so each decision occurs in one section.

## 24. Developer/debug-string policy

- Translate all user-facing controls, dialogs, tooltips, errors, and status strings into Urdu; review priority never permits an applicable user-facing string to remain untranslated.
- Translate informational notices and progress messages into concise, declarative Urdu.
- Translate developer-facing diagnostics into terse technical Urdu without politeness marking.
- Keep identifiers, function names, format specifiers, and topic 4 retained tokens verbatim in every string family.
- Apply the same policy to every subsystem; no sibling-family override exists.
- Render the `BUG:` diagnostic prefix as "بگ:", held distinct from the "خرابی:" prefix that carries ordinary failures; the two severities never share a word.
- Keep an identifier-plus-value diagnostic pair contiguous (eg `sweep_state=%d`); a qualifier precedes the whole pair and never stands between the identifier and its `=`.
