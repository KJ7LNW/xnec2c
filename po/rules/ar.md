# ar translation rules

## 1. Script and orthography

- Scope: use contemporary Modern Standard Arabic for Arabic-script readers across the Arab region, addressing professional electrical and radio-frequency engineers, antenna designers, and amateur-radio operators.
- Write Arabic prose in the Arabic abjad from right to left; retain opposite-direction technical tokens left to right, unmirrored, without manual direction-control characters.
- Use the standard Arabic letters and the required hamza forms `أ إ ؤ ئ ء`; never replace them with bare alef or look-alike characters.
- Omit optional tashkil in ordinary interface prose; retain a required diacritic where omission changes the intended technical word. Use canonical Unicode composition and never strip a required mark.
- Use U+0027 inside a retained token that requires an apostrophe or prime; never substitute a similar Arabic-script mark.
- Arabic has no letter-case distinction; topic 6 governs retained case-sensitive tokens.
- Separate words with one space; use one space between Arabic prose and an embedded technical or numeric token.
- Form technical compounds as spaced idafa or prepositional phrases; omit fused and hyphenated coinages unless the retained source token itself contains a hyphen.

## 2. Numerals in literals

- Use Western digits `0-9` for literal technical values in translated prose; retain their left-to-right order.
- Use `.` as the decimal separator and add no thousands separator to numbers physically present in a translated literal.
- Retain formulas, examples, fixed defaults, and named mathematical or standards constants exactly in source form.
- Express an ordinal or index as a native word plus its literal digit, eg `البطاقة 3`; never fuse the digit to an affix or treat an index separator as a decimal separator.

## 3. Punctuation and quotation

- Use `«»` for quoted Arabic prose; retain source straight quotes around embedded technical tokens so their literal boundaries remain exact.
- Replace prose commas and question marks with `،` and `؟`; retain `!` when the source requires an exclamation mark.
- Place no space before `،` `؟` `:` `؛` or a terminal mark; place one space after a nonterminal mark.
- Use `...` for an ellipsis; retain source dash characters and their range meaning.
- End full dialogs, tooltips, status messages, and error sentences with `.`; omit a terminator from short labels, buttons, and fragments.
- Retain source punctuation inside every embedded technical run.

## 4. Never-translate tokens

Treat every listed token as an exact left-to-right source token: never translate, transliterate, respell, mirror, or substitute any character.

- Retain NEC2 card mnemonics: `GW GA GH EX LD FR RP GE EN SP SM SC NE NH NT TL GN GD GC GX GR GS GM GF CP CM SY XQ EK KH PQ PT`.
- Retain unit symbols used for frequency, gain, impedance magnitude, power ratio, temperature, conductivity, angle, and percent, including `Hz kHz MHz GHz dB dBi Ω W K S/m deg %`.
- Retain figure-of-merit and parameter tokens, including `VSWR`, S-parameter forms, `Z`, `Z0`, front/back-ratio tokens, and gain/temperature tokens.
- Retain file extensions and literal filename suffixes, including `.nec` and `.csv`.
- Retain every printf-style format specifier; topic 5 governs its placement.
- Retain embedded function names, variable names, and configuration keys in developer strings.
- Retain physical and mathematical symbol letters, including `E H X Y Z φ θ`.
- Retain product, library, toolkit, chart, and tool names, including `NEC2` and `xnec2c`.
- Retain named transfer and algorithm terms `Log Asinh μ-law Reinhard Sigmoid Identity`; translate descriptive `Power` as `قدرة` when it names the power-law family or electrical power, with context distinguishing the senses.
- Translate the geometry terms `segment`, `patch`, and `tag` consistently as `قطعة`, `رقعة سطحية`, and `وسم`; retain process term `fork` verbatim.

## 5. Format-specifier integrity

- Preserve exactly the source set of printf-style specifiers; add, remove, split, and merge none.
- Keep specifiers in source order and positions; restructure Arabic wording around those positions rather than reordering them.
- Retain every letter, flag, width, precision, positional index, and digit inside a specifier exactly.

## 6. Capitalization and title-case

- Arabic has no case distinction: write labels, menu items, and titles as natural nominal phrases without title-case imitation.
- Retain uppercase axis letters `X Y Z`; retain a lowercase mathematical or coordinate variable exactly as sourced.
- Apply the same natural phrase casing to every element of a coordinated option name.
- Write generic `بطاقة` identically in titles and running prose; retain the exact case of acronyms and proper names.

## 7. Interface register by string type

Use concise professional Modern Standard Arabic; preserve every source meaning without source-order imitation or unnatural abbreviation.

- Commands, buttons, and menu actions: use a subjectless verbal noun or concise imperative; place the action before an expressed object and omit a pronoun, eg `حفظ`.
- Field labels: use a head-first nominal phrase followed by the retained colon; omit subject and verb, eg `التردد:`.
- Dialog statements: use a full declarative sentence in natural verb-subject-object or subject-verb-object order; state known information before its consequence.
- Confirmations: use a full interrogative or declarative sentence; state the action and consequence before the requested confirmation.
- Tooltips: use a full declarative sentence ordered as control purpose then condition or reason; when the source gives a disabled-state reason, retain that reason.
- Status and error messages: use a subjectless impersonal declarative or passive sentence; state the affected entity before diagnostic detail when clarity requires it.
- User-visible domain entities: name the specific entity first and attach qualifiers in idafa or adjective order, eg `نمط الإشعاع`; retain proper-name and personal-name rules from topic 8.
- Prefer the shortest complete natural form: use a compound or multiword phrase where Arabic grammar or technical precision requires it.
- Apply topic 24 separately to developer and debug strings.

## 8. Formality and address

- Arabic interface prose uses formal professional Modern Standard Arabic; it has no formal/informal pronoun opposition applicable to this interface.
- Omit direct second-person pronouns and use subjectless or impersonal constructions; use grammatical person only when the source explicitly addresses a person.
- Avoid added gender marking for the reader; apply the inherent gender and number of referenced nouns without treating masculine grammar as personal address.
- Use inclusive role nouns and neutral constructions where natural; retain required grammatical gender, number, and animacy agreement.
- Honorifics are not applicable: add none absent from the source.
- Preserve a person's source name order and spelling; Arabic interface grammar does not reorder personal names.
- Realize professional formality through lexical choice, verbal nouns, concise imperatives, and complete agreement, not a special register suffix.
- Keep commands and labels terse; write dialogs and confirmations as complete sentences.
- State a confirmation's action and consequence; never reduce it to an isolated yes/no fragment.
- Exclude colloquial, slang, commercial, archaic, and ceremonially over-formal register.

## 9. Accelerator/hotkey mnemonics

- Preserve the source mnemonic marker only when present; never introduce one into an unmarked source literal.
- Place `_` immediately before a typable Arabic letter within the translated term.
- Choose the mnemonic from the translated term itself; never append or substitute a transliterated source letter.
- When two translated literals in the same source-defined mnemonic group collide, choose another common-keyboard letter from the affected translated term.

## 10. Domain lexicon

Locked terms, applied everywhere in po/ar.po; correct technical sense per topic 11. Concept key — Arabic term — sense/hazard guarded.

### Electrical primitives

| Concept | Arabic | Guards |
|---|---|---|
| current | تيار | electrical current (A); never حالي (present-time) |
| charge | شحنة | electrical charge (C); never فاتورة (billing) |
| voltage | جهد | electric potential |
| power (electrical) | قدرة | dissipated/radiated watts, power gain; distinct from Power tone-map family name (kept English) and from a math power-law (n/a in this domain) |
| impedance | ممانعة | complex Z; distinct from مقاومة and مفاعلة |
| resistance | مقاومة | real part of Z; distinct from ممانعة and حمل |
| reactance | مفاعلة | imaginary part of Z |
| inductance | مِحاثّة | self-inductance |
| capacitance | سعة (كهربائية) | qualify سعة with كهربائية where amplitude سعة could otherwise collide (topic 11) |
| conductivity | موصلية | material S/m |
| admittance | مقبولية | admittance-matrix sense; distinct from ممانعة |
| load | حمل | LD-card impedance load; never ثقل (physical weight) |
| gain | كسب | antenna directivity ratio (dB); never ربح (profit) or تضخيم (amplifier gain) |
| excitation | إثارة | EM energy input/source; never إثارة انفعالية (emotional excitement) |
| feedpoint | نقطة التغذية | antenna feed point |
| port | منفذ | excitation/S-parameter port |
| radials | أسلاك شعاعية | horizontal ground-plane radial wires (noun); distinct from شعاعي (the adjective radial) |

### Ground and earth

| Concept | Arabic | Guards |
|---|---|---|
| ground / ground plane | أرضي / مستوى أرضي | RF reference plane, GN/GD cards, ground type/conductivity/effects/model; one term across every ground sub-sense; never تربة |
| earth (physical medium) | تربة | terrain/noise-model earth, "below ground" geometry; distinct from أرضي |
| ground wave | موجة أرضية | propagation term; distinct from أرضي |

### Geometry primitives

| Concept | Arabic | Guards |
|---|---|---|
| wire | سلك | thin conductor/GW element; never كابل (cable); distinct from خيط (compute thread) |
| segment | قطعة | NEC2 geometry subdivision; never a generic portion |
| patch | رقعة سطحية | NEC2 surface primitive; one term in geometry and visualization |
| tag | وسم | NEC2 geometry identifier; never a UI label or card |
| card | بطاقة | NEC2 input record; register in topic 18 |
| kernel | نواة | integral-equation/thin-wire kernel; never an OS kernel sense (n/a elsewhere in this file) |
| cliff | جرف | two-medium ground-boundary type; never a fracture/break sense |
| structure | بنية | antenna model geometry; never إنشاء (construction) |
| model | نموذج | NEC model or noise-temperature model |
| geometry | هندسة | the model geometry |
| crossed | متقاطعة | transmission-line conductors crossed/reversed; never مقطوعة (cut/severed) |

### Field, pattern, viewer

| Concept | Arabic | Guards |
|---|---|---|
| field (EM) | مجال | near/total/E/H field: مجال قريب / كلي / كهربائي / مغناطيسي; distinct from حقل |
| field (data/config) | حقل | UI/config data field only; never for the EM field |
| near field / far field | مجال قريب / مجال بعيد | opposed pair, kept symmetric |
| far-field contribution | مساهمة الحقل البعيد | per-direction contribution; not near-field animation |
| radiation | إشعاع | radiated emission |
| radiation pattern | نمط الإشعاع | plotted directional response; never a template/design sense; distinct from وضع (render mode) |
| gain pattern | نمط الكسب | the gain radiation pattern |
| polarization | استقطاب | antenna/wave field orientation |
| polarity | قطبية | sign (+/-) of a quantity; false friend of استقطاب |
| phase / reference phase | طور / الطور المرجعي | wave phase and its reference; retain the distinction |
| frequency | تردد | oscillation rate; never a general repetition count |
| wave / wavelength | موجة / طول موجي | phenomenon and spatial period; keep distinct |
| standing wave / traveling wave | موجة واقفة / موجة متنقلة | opposed stationary and propagating forms |
| node / antinode | عُقدة / بطن | standing-wave zero and maximum; distinct from a generic numeric zero |
| crest | قمة | instantaneous wave apex; distinct from a curve peak |
| magnitude | مقدار | scalar modulus; distinct from oscillating peak `مطال` |
| amplitude | مطال | oscillating peak; distinct from modulus and capacitance |
| peak value / peak magnitude | قيمة قصوى / القيمة القصوى للمقدار | two distinct options; never collapse them |
| instantaneous | لحظي | projection mode; add `(φ=0)` only when sourced |
| Poynting vector | متجه بوينتنغ | electromagnetic energy-flow vector |
| solid angle | زاوية مجسمة | three-dimensional angular measure |
| net gain | الكسب الصافي | total minus mismatch; never real-part gain |
| viewer | عارض | observation direction or 3D view; never speaker or preview |
| flow / flow direction | تدفق / اتجاه التدفق | current or patch flow and its direction |
| total field | المجال الكلي | combined electromagnetic field |

### Color, tone, animation subsystem

| Concept | Arabic | Guards |
|---|---|---|
| color | لون | chromatic property |
| color projection | إسقاط لوني | quantity driving hue; distinct from geometry projection |
| hue | صبغة | color-wheel angle; distinct from palette and scale family |
| brightness | سطوع | luminance channel; distinct from hue |
| hue encoding / brightness encoding | ترميز الصبغة / ترميز السطوع | distinct encoding modes |
| color scale | مقياس الألوان | magnitude-to-color relation |
| scale family / color tone | عائلة اللون | transfer-curve family; one term for both source spellings |
| palette / palette kind | لوحة الألوان / نوع اللوحة | color set and layout enum; distinct from scale family |
| ramp / gradient | تدرّج | linear color strip or palette kind |
| gamma | غاما | power-law exponent |
| knee | نقطة الانحناء | soft-knee bend point |
| softening / compression | تنعيم / ضغط | distinct dynamic-range operations |
| contrast / dynamic range | تباين / المدى الديناميكي | visual separation and represented span |
| floor | حد أدنى | lower clamp; never room floor or electrical ground |
| envelope | مغلف | amplitude or magnitude boundary |
| comet | مذنّب | moving-crest overlay; never geometry |
| overlay (noun) | تراكب | added visual layer; distinct from verb `يُراكب` |
| animate / animation | تحريك / رسوم متحركة | action and resulting motion mode |
| animated / static | متحرك / ثابت | dynamic and phase-invariant categories |
| projection | إسقاط | generic color or geometry projection |
| scale (verb / noun) | يُحجّم / تحجيم | action and scale operation |
| wireframe | إطار سلكي | wire-mesh drawing mode |
| identity | مطابق | no-op transfer; distinct from Smith-chart unity `وحدة` |
| sentinel | قيمة حارسة | unreachable-case guard value |
| bins | فئات | discretization buckets; never physical boxes |
| companding | ضغط لوغاريتمي | bounded logarithmic curve such as retained `μ-law` |
| tone mapping | تخطيط الدرجات اللونية | photographic dynamic-range mapping |

### Render and compute

| Concept | Arabic | Guards |
|---|---|---|
| renderer | الراسم | drawing backend; never render engine |
| shader | مظلل | graphics shading program |
| allocation / managed allocator | تخصيص الذاكرة / المخصص المُدار | memory operation and its manager |
| thread | خيط | compute thread; distinct from conductor `سلك` |
| widget | عنصر واجهة | interface element |
| validation | التحقق | validation-tree feature; verification shares this term in Arabic |
| batch mode | الوضع الدفعي | noninteractive grouped operation |
| fork (process) | fork | retained process operation, per topic 4 |
| deadlock | جمود | mutual waiting state |
| notifier | مُخطِر | notification mechanism |
| token / operand / operator / arity | رمز / مُعامَل / عامل / عدد المعاملات | distinct parser concepts |
| override | تجاوز | supersede a value; never overwrite `كتابة فوق` |
| swap | تبديل | exchange two values |
| theme | سمة | interface or color theme; never topic `موضوع` |

### Metrics and miscellaneous

| Concept | Arabic | Guards |
|---|---|---|
| noise / noise temperature | ضوضاء / درجة حرارة الضوضاء | electronic noise and its equivalent temperature |
| efficiency | كفاءة | output-to-input ratio |
| interpolation | استيفاء | estimate between known samples |
| mnemonic | مُعرِّف رمزي | card code descriptor; distinct from hotkey letter |
| degrees / deg | درجة / `deg` | prose angle name versus retained unit tag |
| diameter | قطر | single canonical geometric term |
| reflect | عكس / يطابق / انعكاس | mirror operation, behavioral tracking, and physical reflection |
| default(s) | افتراضي / القيم الافتراضية | fallback value or set |
| normalize / normalization | تسوية / تطبيع | native action and process terms, not transliterations |

## 11. Disambiguation policy

- Select the technical sense locked in topic 10 for every ambiguous source concept.
- Add no qualifier absent from the source when context already identifies the technical sense.
- Add a qualifier only when the Arabic term remains genuinely ambiguous, eg `سعة كهربائية` for capacitance near amplitude.
- Accept no intra-domain homonym requiring one shared Arabic term; topic 10 separates every listed collision.
- Avoid locative collision by using `أرضي` for electrical ground, `تربة` for physical earth, and `حد أدنى` for floor.
- Render an action as a verb and its process or result as a verbal noun, eg `يُحجّم` and `تحجيم`.

## 12. Cross-catalog consistency

- Reuse one locked topic-10 term for each concept; unify every spelling outlier to that form.
- Keep `قطبية` polarity distinct from `استقطاب` polarization.
- Keep `مقدار` magnitude distinct from `مطال` amplitude, and `قيمة قصوى` distinct from `القيمة القصوى للمقدار`.
- Keep `أرضي` ground distinct from `تربة` earth; keep `حمل` load distinct from `شحنة` charge.
- Keep `كسب` antenna gain distinct from `تضخيم` amplification and `ربح` profit.
- Keep `تيار` current distinct from temporal `حالي`; keep electrical `شحنة` distinct from billing `فاتورة`.
- Keep `سلك` wire distinct from `كابل` cable and compute `خيط`.
- Keep `نمط الإشعاع` distinct from template `قالب` and far-field region `المجال البعيد`.
- Keep electromagnetic `إثارة` distinct from emotional `انفعال`.
- Keep `عُقدة` and `بطن` distinct from generic numeric `صفر`.
- Keep `عائلة اللون`, `صبغة`, `نوع اللوحة`, and `إسقاط لوني` as four distinct chroma concepts.
- Keep overlay `مذنّب` distinct from geometry `هندسة`; keep `مطابق` identity distinct from unity `وحدة`.
- Keep `الراسم` renderer distinct from render engine `محرك الرسم`.
- Keep `تجاوز` override distinct from overwrite `كتابة فوق`.
- Keep `عارض` viewer distinct from observer `مراقب`, speaker `متحدث`, and preview `معاينة`.
- Keep mirror operation `عكس`, behavioral tracking `يطابق`, and physical reflection `انعكاس` distinct.
- Keep model `بنية` distinct from construction `إنشاء`; keep interface theme `سمة` distinct from topic `موضوع`.
- Arabic uses `التحقق` for both validation and verification; collapse this unavailable distinction explicitly.
- Keep `الكسب الصافي` distinct from real-part gain `كسب الجزء الحقيقي`.
- Keep electrical and power-law `قدرة` distinguished by context from retained named transfer terms.
- Prefer native or established Arabic terms except retained `fork`; lock `صبغة` and `مطال` over competing colloquial forms to preserve catalog distinctions.

## 13. Priority ordering

- Apply this precedence: correct technical meaning, interface convention, catalog consistency, disambiguation, then literal numeral form.
- Use `مطال` for amplitude and `سعة` for capacitance: technical distinction overrides the broader colloquial overlap.

## 14. Grammatical number

- Use Arabic singular, dual, and plural noun forms; make adjectives, participles, and verbs agree with the selected number.
- After zero, use the plural noun; after one, use singular; after two, use dual; after `3-10`, use plural; after `11-99`, use singular accusative; follow the same cycle for larger literal counts.
- Choose a sound or broken plural according to the locked noun's established Modern Standard Arabic form; never invent a plural marker.

## 15. Grammatical agreement

- Make adjectives and participles agree in gender and number with the head noun, including an implied head in a standalone label.
- Apply Arabic numeral-noun agreement and the required noun case after an explicit count.
- Reflexive-particle retention is not applicable: the governed technical constructions require no reflexive particle.
- Resolve a standalone label's gender from its implied noun; use masculine only when no feminine head is present.

## 16. Morphological derivation

- Derive a borrowed technical verb with an established Arabic pattern, preferring Form II where causative technical usage requires it, eg `يُحجّم`; never attach an ad hoc transliterated ending.
- Form the verbal noun from the selected verb pattern, eg `تحجيم` from `حجّم`.
- Prefer native derivation over a loanword except the exact retained tokens in topic 4; form compounds as spaced idafa phrases.

## 17. Preposition and sandhi selection

- Context-conditioned written preposition forms are not applicable: the following sound does not change the written preposition.
- Elision and contraction are not applicable to these literals: pronunciation-only hamzat al-wasl does not alter their spelling.

## 18. Card/record-label register

- In dialog and editor titles, use fixed designator `بطاقة GW`: generic noun followed by the retained mnemonic.
- In running prose, use an idafa or prepositional phrase, eg `في بطاقة GW`; never fuse or hyphenate the mnemonic.
- Use full `بطاقة` in both registers; Arabic has no case-based capitalization or short designator variant.
- Keep title and running-prose forms internally consistent and never cross-convert them.

## 19. Multi-paragraph and whitespace fidelity

- Mirror every source paragraph break at the same position, preserving blank-line and single-line distinctions.
- Remove a trailing clause when the current source omits it; never inherit stale trailing meaning.
- Preserve the source count and placement of trailing newlines and terminal punctuation.
- Preserve semantic line breaks; add no line break for visual wrapping.
- Preserve complete meaning and natural wording; never truncate or abbreviate for an assumed display limit.

## 20. Current-source fidelity

- Derive every translation from the current source literal and supplied context.
- Reuse an inherited translation only when its complete meaning matches the current source and its topic-10 sense.
- Treat prior translations of `comet`, `geometry`, `amplitude`, `capacitance`, `ground`, `earth`, `patch`, `segment`, and `tag` as unsafe until reconciled to this file's locked terms.

## 21. Script hygiene

- Permit only standard Arabic letters in translated prose; forbid Arabic-Indic digits `٠-٩`, extension letters `پ چ ژ گ`, alternate forms `ی ک`, and cross-script homoglyphs.
- Permit a forbidden-script character only inside an exact retained token whose source spelling requires it.
- Translate plain source-language words; retain only identifiers, symbols, units, proper names, and tokens listed in topic 4.
- Require zero wrong-script, homoglyph, and malformed mixed-direction token occurrences.

## 22. Rule-file scope hygiene

- Include only current decisions that alter wording, Unicode characters, punctuation, capitalization, embedded tokens, mnemonic markers, or semantic whitespace in translated literals.
- Omit catalog representation, serialization, headers, flags, workflow, rendering, fonts, shaping, layout, widget behavior, runtime formatting, sorting, search, display sizing, audit history, completion state, review metadata, provenance, and citations.
- State each decision directly; include no implementation procedure, bibliography, correction history, or competing policy.

## 23. Section-disjointness declaration

- Topic 1 governs script mechanics: characters, direction, orthography, and spacing.
- Topics 5-7, 9-12, and 18 govern phrasing and structure: token placement, grammar, mnemonics, terminology, and record naming.
- Topic 8 governs address register: person, formality, inclusion, and prohibited tone.
- Keep these axes non-overlapping; resolve each concept in its governing section only.

## 24. Developer/debug-string policy

- Translate user-facing strings fully into Modern Standard Arabic; review priority never permits leaving one untranslated.
- Translate informational status and diagnostic strings into concise technical Modern Standard Arabic.
- Translate developer-facing and debug strings into terse technical Modern Standard Arabic unless an established sibling message family in the same subsystem retains source prose; apply that family decision consistently.
- Retain embedded identifiers, function names, format specifiers, and topic-4 tokens in every string family regardless of translation priority.
- Classify commands, labels, dialogs, confirmations, tooltips, user errors, and results as user-facing; classify status and progress messages as informational; classify assertions, parser internals, and low-level diagnostics as developer-facing.
- Render the `BUG:` diagnostic prefix as "خلل:", held distinct from the "خطأ:" prefix that carries ordinary failures; the two severities never share a word.
- Keep an identifier-plus-value diagnostic pair contiguous (eg `sweep_state=%d`); a qualifier precedes the whole pair and never stands between the identifier and its `=`.
