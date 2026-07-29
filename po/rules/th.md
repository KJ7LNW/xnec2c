# th translation rules

## 1. Script and orthography
- Scope: standard Thai for Thailand, written in modern Thai script under Royal Institute orthography, for professional electrical and radio-frequency engineers, antenna designers, and amateur-radio operators.
- Writing system: use Thai consonants ก-ฮ, vowel signs, tone marks, numerals under topic 2, and standard Thai signs; Thai uses base characters with combining vowel and tone marks, with no alternate precomposed form.
- Tone marks and vowel signs are mandatory diacritics: never strip or fold to look-alikes.
- No ambiguous-mark code-point issue applies (no apostrophe/joiner ambiguity in Thai orthography).
- Not applicable: Thai orthography has no textual joiner, ambiguous apostrophe, or required code-point distinction beyond its consonants, vowels, tone marks, and signs.
- Directionality: left-to-right; keep embedded technical tokens left-to-right and unchanged.
- No uppercase/lowercase distinction: casing rules (topic 6) reduce to mnemonic-letter handling only (topic 9).
- Orthographic standard: modern standard Thai (Royal Institute spelling); no regional/reform variant in use.
- Use no spaces between Thai words in running prose; insert one space at a Thai-text boundary with an embedded Latin or numeric token.
- Fuse established Thai compounds without spaces or hyphens; use a hyphen or space only when the established technical term requires it.

## 2. Numerals in literals
- Use digits `0-9` for technical values in Thai prose; do not use Thai digits `๐-๙`.
- Use period `.` as the decimal separator and comma `,` as the thousands separator for numbers physically present in translated prose.
- Keep formulas, examples, fixed defaults, and named mathematical or standards constants in source form.
- Form an ordinal or index as `ที่` plus the digit; treat this construction as an index marker, never as a decimal separator.

## 3. Punctuation and quotation
- Native quotation marks: none in standard technical Thai; retain source plain quotes `"…"` for embedded technical tokens, for catalog-wide consistency.
- Formal Thai prose traditionally omits comma/period, but this catalog's technical register borrows Western punctuation directly (`, . ? !`) as already established; keep this borrowed set rather than reverting to punctuation-free prose.
- Spacing around colon/semicolon/terminal punctuation follows source ASCII spacing (no space before, one space after).
- Ellipsis form: `...` (three dots), matching source; dashes carry over from source unchanged.
- Sentence terminator: none required; do not append a Thai full stop; keep the source string's own trailing `\n`/punctuation mechanically, whether the string is a short label or a full sentence.
- Punctuation inside embedded technical runs (units, mnemonics, format specifiers) stays in source form.

## 4. Never-translate tokens
- Keep NEC2 card mnemonics verbatim and untransliterated: GW GA GH EX LD FR RP GE EN SP SM SC NE NH NT TL GN GD GC GX GR GS GM GF CP CM SY XQ EK KH PQ PT.
- Keep unit symbols verbatim and untransliterated: Hz, kHz, MHz, GHz, dB, dBi, dBd, Ω, W, K, S/m, °, %, and `(deg)`.
- Keep figure-of-merit and parameter tokens verbatim and untransliterated: VSWR, S11-style S-parameters, Z, Z0, F/B, and G/T.
- Keep file extensions verbatim and untransliterated: `.nec`, `.csv`, `.s1p`, `.s2p`, and `.png`.
- Keep every printf-style format specifier verbatim; topic 5 governs its set and order.
- Keep embedded function names, variable names, identifiers, and configuration keys verbatim in developer strings.
- Keep physical and mathematical symbol letters verbatim: E, H, Z, φ, and θ.
- Keep product, library, toolkit, and chart names verbatim: xnec2c, NEC2, GTK, GSL, OpenGL, and Smith.
- Keep named transfer functions and algorithms verbatim: Log, Asinh, μ-law, Reinhard, Sigmoid, and Identity; translate the descriptive `Power` family as กำลัง.
- Transliterate conditional loanwords file-wide: `segment` เซกเมนต์, `patch` แพตช์, `tag` แท็ก, and process `fork` ฟอร์ก; these are outside the never-translate set.
- Translate geometry and domain words not enumerated as retained tokens according to topic 10.
- Preserve every retained token's spelling, characters, case, punctuation, and left-to-right order with zero substitutions.

## 5. Format-specifier integrity
- Preserve every source format specifier with the same conversion type and numeric components; add, remove, translate, or localize none.
- Keep source order by default; use explicit positional specifiers only when Thai grammar requires argument reordering and the source format supports them.
- Without positional specifiers, restructure Thai wording around the fixed argument order.

## 6. Capitalization and title-case
- Not applicable: Thai script has no letter-case distinction; preserve case only in retained Latin tokens and mnemonic characters.
- Keep coordinate and mathematical variables in their source case.
- Not applicable: Thai labels, titles, option names, and the generic card noun have no title-case or sentence-case contrast.

## 7. Interface register by string type
- Treat each string type as a fixed Thai interface construction; preserve complete meaning without imitating source length.
- Commands, buttons, and menu actions: use the shortest natural imperative, omit the subject, place the verb before its object, and avoid nominalization when an action is intended.
- Field labels: use a concise noun phrase in head-before-modifier order and retain a source colon.
- Dialogs and confirmations: use full neutral-formal sentences, omit an inferable subject, place known context before the requested action or consequence, and use declarative or interrogative mood as the source requires.
- Tooltips: use a complete explanatory phrase or sentence, lead with the control's action, then state the source-provided reason when a disabled control is unavailable.
- Status and error messages: use impersonal declarative wording, present the affected entity before its state or failure, and omit first- and second-person pronouns.
- User-visible domain entities: use the topic 10 term followed by its distinguishing modifier; retain proper names and personal-name policy from topic 8.
- Prefer established compounds and the shortest complete natural form; omit no meaning and invent no abbreviation.
- Keep developer and debug strings under topic 24 only.

## 8. Formality and address
- Use neutral formal written Thai throughout; prohibit colloquial, royal, commercial, archaic, slang, and particle-bearing spoken registers.
- Carry formality through complete wording and subject omission; Thai has no formal-versus-informal verb inflection for these literals.
- Omit subject pronouns in commands and when context identifies the actor; do not insert `คุณ` solely to represent a source second-person subject.
- Avoid gender marking, gendered self-reference, and gendered politeness particles; use natural unmarked nouns and pronouns when a person must be named.
- Use inclusive role terms without inventing gender, number, or animacy distinctions absent from the source.
- Not applicable: interface address uses no honorific, grammatical-person agreement, or morphology carrying formality.
- Preserve a person's source name order and spelling unless the source supplies a localized form.
- Use terse imperatives for commands and full neutral-formal sentences for dialogs and confirmations.
- Form confirmations as complete neutral questions or statements matching the source mood, without spoken politeness particles.

## 9. Accelerator/hotkey mnemonics
- Preserve `_` only when the source literal contains a mnemonic marker; never invent one.
- Place `_` immediately before a typable base consonant from the translated Thai term, not before a vowel sign or tone mark.
- For a retained Latin token, place `_` before a retained Latin letter from that token; add no separate mnemonic suffix.
- Derive the mnemonic character from the translated literal rather than from a transliteration of the source term.

## 10. Domain lexicon

| concept | target term | sense / purpose |
|---|---|---|
| current | กระแส | electrical current; never ปัจจุบัน (temporal) |
| charge | ประจุ | electrical charge; never billing/fee |
| voltage | แรงดันไฟฟ้า | electric potential |
| power (electrical) | กำลังไฟฟ้า | watts/dissipated/radiated power; distinct from Power scale family |
| power (Power scale family) | กำลัง | math power-law transfer curve; distinct from electrical power |
| impedance | อิมพีแดนซ์ | complex Z; distinct from resistance/reactance |
| resistance | ความต้านทาน | real part of Z |
| reactance | รีแอกแตนซ์ | imaginary part of Z |
| inductance | ความเหนี่ยวนำ | established technical sense; catalog consistency |
| capacitance | ความจุ | established technical sense; catalog consistency |
| conductivity | สภาพนำไฟฟ้า | material S/m |
| admittance | แอดมิตแตนซ์ | admittance-matrix sense; distinct from impedance |
| load | โหลด | LD-card impedance load; never physical weight |
| gain | อัตราขยาย | antenna directivity ratio (dB); never profit/amplification |
| excitation | การกระตุ้น | EM energy input/source; never emotional excitement |
| feedpoint | จุดป้อนสัญญาณ | antenna feed point |
| port | พอร์ต | excitation/S-parameter port |
| radials | เส้นเรเดียล | radial wires (noun); distinct from adjective เรเดียล |
| ground / ground plane | กราวด์ / ระนาบกราวด์ | RF electrical reference; never ดิน (soil) |
| earth (physical medium) | พื้นดิน | terrain/noise-model earth; distinct from ground |
| ground wave | คลื่นดิน | propagation term |
| wire | ลวด | thin conductor / GW element; never สาย except fixed compounds สายส่ง (transmission line), สายอากาศ (antenna) |
| segment | เซกเมนต์ | NEC2 geometry subdivision |
| patch | แพตช์ | NEC2 surface patch (SP/SM) |
| tag | แท็ก | NEC2 geometry identifier; never a UI label |
| card | การ์ด | NEC2 input record; register per topic 18 |
| kernel | เคอร์เนล | thin-wire kernel; never OS kernel |
| cliff | หน้าผา | two-medium ground-boundary type; not a physical fracture |
| structure | โครงสร้าง | antenna model geometry; never งานก่อสร้าง (construction) |
| model | แบบจำลอง | NEC model or noise-temperature model |
| geometry | เรขาคณิต | the model geometry |
| crossed | ไขว้ | transmission-line conductors crossed; never cut/severed |
| field (EM) | สนาม | near/total/E/H field; distinct from field (data/config) |
| field (data/config) | ฟิลด์ | UI data field; distinct from สนาม |
| near field | สนามใกล้ | opposed to far field |
| far field | สนามไกล | opposed to near field |
| far-field contribution | ส่วนร่วมสนามไกล | per-direction contribution |
| radiation | การแผ่รังสี | radiated emission |
| radiation pattern | แพตเทิร์นการแผ่รังสี | plotted directional response; never แม่แบบ (template/design) |
| gain pattern | แพตเทิร์นอัตราขยาย | the gain radiation pattern |
| polarization | โพลาไรเซชัน | antenna/wave field orientation; distinct from polarity |
| polarity | ขั้ว | sign (+/-); false friend of polarization |
| phase | เฟส | established technical sense; catalog consistency |
| reference phase | เฟสอ้างอิง | established technical sense; catalog consistency |
| frequency | ความถี่ | established technical sense; catalog consistency |
| wave | คลื่น | established technical sense; catalog consistency |
| wavelength | ความยาวคลื่น | established technical sense; catalog consistency |
| standing wave | คลื่นนิ่ง | opposed to traveling wave |
| traveling wave | คลื่นเคลื่อนที่ | opposed to standing wave |
| node | โนด | standing-wave zero; also null overlay sense |
| antinode | แอนติโนด | standing-wave maximum; also peak overlay sense |
| crest | ยอดคลื่น | instantaneous wave apex (comet-head); distinct from peak |
| magnitude | ขนาด | modulus/scalar; distinct from amplitude |
| amplitude | แอมพลิจูด | oscillating-quantity peak; distinct from magnitude |
| peak value | ค่าพีค | distinct UI option from peak magnitude |
| peak magnitude | ขนาดพีค | distinct UI option from peak value |
| instantaneous | ค่าฉับพลัน | projection mode; add "(φ=0)" only where source carries it |
| Poynting vector | เวกเตอร์พอยน์ติง | proper noun |
| solid angle | มุมตัน | established technical sense; catalog consistency |
| net gain | อัตราขยายสุทธิ | total-minus-mismatch gain; not real-part gain |
| viewer (direction) | ทิศทางการมอง | observation direction |
| viewer (3D widget) | หน้าต่างมุมมอง 3 มิติ | the 3D view widget; never ผู้สังเกตการณ์/ผู้พูด/ตัวอย่าง |
| flow / flow direction | การไหล / ทิศทางการไหล | patch/current flow |
| total field | สนามรวม | established technical sense; catalog consistency |
| color | สี | established technical sense; catalog consistency |
| color projection | การฉายสี | which quantity drives hue |
| hue | ฮิว | color-wheel angle |
| brightness | ความสว่าง | luminance channel |
| hue encoding | การเข้ารหัสฮิว | distinct enum, never collapses to color projection |
| brightness encoding | การเข้ารหัสความสว่าง | distinct enum, never collapses to color projection |
| color scale | สเกลสี | magnitude-to-color scale |
| scale family / color tone | ตระกูลสเกล | transfer-curve family (Power/Log/Asinh/μ-law/Reinhard/Sigmoid/Identity); one term, two source spellings |
| palette / palette kind | พาเลต / ชนิดพาเลต | distinct from scale family and color projection |
| ramp / gradient | เกรเดียนต์ | one palette-kind concept, two source spellings |
| gamma | แกมมา | power-law exponent |
| knee | จุดหักเข่า | soft-knee bend point |
| softening | การปรับให้นุ่มนวล | dynamic-range softening |
| compression | การบีบอัด | dynamic-range compression |
| contrast | คอนทราสต์ | established technical sense; catalog consistency |
| dynamic range | ช่วงพลวัต | established technical sense; catalog consistency |
| floor | ค่าพื้น | minimum/lower clamp; never a physical room floor |
| envelope | เอนเวโลป | magnitude/amplitude envelope |
| comet | ดาวหาง | moving-crest overlay effect; never geometry |
| overlay (noun) | เลเยอร์ซ้อนทับ | added visual layer; distinct from verb ซ้อนทับ |
| animate / animation | แอนิเมชัน | established technical sense; catalog consistency |
| animated (header) | มีแอนิเมชัน | category-header adjective |
| static (header) | คงที่ | category-header adjective |
| projection | การฉาย | color or geometry projection |
| scale (verb) | ปรับสเกล | to scale |
| scale (noun) | สเกล | a scale |
| wireframe | โครงลวด | wire-mesh render mode |
| identity | เอกลักษณ์ | no-op/passthrough transfer; distinct from unity |
| unity (Smith-chart) | ค่าหนึ่งหน่วย | distinct from identity |
| sentinel | เซนติเนล | unreachable-case guard value |
| bins | ถังข้อมูล | discretization buckets |
| companding | คอมแพนดิง | bounded log curve (μ-law) |
| tone mapping | โทนแมปปิง | photographic tone-map |
| renderer | ตัวเรนเดอร์ | drawing backend; never เอนจินเรนเดอร์ (render engine) |
| shader | เชดเดอร์ | established technical sense; catalog consistency |
| allocation (memory) | การจัดสรรหน่วยความจำ | established technical sense; catalog consistency |
| managed allocator | ตัวจัดสรรหน่วยความจำที่มีการจัดการ | allocator/report |
| thread | เธรด | compute thread; no collision with wire (ลวด) in Thai |
| widget | วิดเจ็ต | UI element |
| validation | การตรวจสอบความถูกต้อง | the validation-tree feature; distinct from verification |
| verification | การตรวจทาน | verification checks; distinct from validation |
| batch mode | โหมดแบตช์ | established technical sense; catalog consistency |
| fork (process) | ฟอร์ก | process fork; transliterated, decision locked (topic 4) |
| deadlock | เดดล็อก | established technical sense; catalog consistency |
| notifier | ตัวแจ้งเตือน | established technical sense; catalog consistency |
| token | โทเคน | expression-parser term |
| operand | โอเปอแรนด์ | expression-parser term |
| operator | โอเปอเรเตอร์ | expression-parser term |
| arity | อาริตี | expression-parser term |
| override | แทนที่ | supersede a value (SY symbol); distinct from overwrite |
| overwrite | เขียนทับ | distinct from override |
| swap | สลับ | exchange |
| theme | ธีม | UI/color theme; never หัวข้อ (topic/subject) |
| noise / noise temperature | สัญญาณรบกวน / อุณหภูมิสัญญาณรบกวน | electronic/thermal noise; never acoustic เสียงดัง |
| efficiency | ประสิทธิภาพ | established technical sense; catalog consistency |
| interpolation | การประมาณค่าในช่วง | established technical sense; catalog consistency |
| mnemonic | นีโมนิก | a card's code descriptor; never บันทึก (memo/note) |
| degrees (freestanding) | องศา | prose/axis usage |
| deg (unit tag) | (deg) | parenthetical unit tag, kept verbatim like other unit tags |
| diameter | เส้นผ่านศูนย์กลาง | canonical native term; locked over any transliteration |
| reflect (geometry mirror op) | พลิกสะท้อน | distinct sense 1 of 3 |
| reflect (behavioral tracking) | ติดตามค่า | distinct sense 2 of 3 |
| reflect (physics reflection) | การสะท้อน | distinct sense 3 of 3 |
| default(s) | ค่าเริ่มต้น | fallback value |
| normalize / normalization | การทำให้เป็นมาตรฐาน | native term, not transliterated |

- Every row above is the file-wide locked term; use no synonym for a mapped concept.
- Add any translated domain concept not yet represented here directly to this table with its sense and consistency purpose.

## 11. Disambiguation policy
- Choose topic 10's technical sense for every ambiguous source term.
- Add no qualifier absent from the source when program context already disambiguates the term.
- Add the shortest Thai qualifier only when the locked term would otherwise remain genuinely ambiguous in that literal; keep กระแส unqualified for electrical current in an unambiguous engineering context.
- No accepted intra-domain homonym is currently in use in this catalog.
- No locative/other homonym collision is currently recorded.
- Gerund vs noun senses of an overloaded source word: Thai verbs serve as nouns without inflection; context alone disambiguates, no separate marking is added.

## 12. Cross-catalog consistency
- Use one topic 10 term per concept file-wide; introduce no synonym or minority spelling.
- Keep these Appendix C terms distinct: `polarity` ขั้ว / `polarization` โพลาไรเซชัน; `magnitude` ขนาด / `amplitude` แอมพลิจูด; `peak value` ค่าพีค / `peak magnitude` ขนาดพีค.
- Keep these Appendix C terms distinct: `ground` กราวด์ / `earth` พื้นดิน; `load` โหลด / `charge` ประจุ; `gain` อัตราขยาย / amplifier amplification การขยายสัญญาณ / profit กำไร.
- Keep these Appendix C terms distinct: electrical `current` กระแส / temporal present ปัจจุบัน; electrical `charge` ประจุ / fee ค่าธรรมเนียม; `wire` ลวด / cable สายเคเบิล / compute `thread` เธรด.
- Keep these Appendix C terms distinct: `radiation pattern` แพตเทิร์นการแผ่รังสี / template แม่แบบ / `far field` สนามไกล; `excitation` การกระตุ้น / emotional excitement ความตื่นเต้น.
- Keep these Appendix C terms distinct: `node` โนด / `antinode` แอนติโนด / generic zero ศูนย์; `scale family` ตระกูลสเกล / `hue` ฮิว / `palette kind` ชนิดพาเลต / `color projection` การฉายสี.
- Keep these Appendix C terms distinct: `comet` ดาวหาง / `geometry` เรขาคณิต; `identity` เอกลักษณ์ / unity ค่าหนึ่งหน่วย; `renderer` ตัวเรนเดอร์ / render engine เอนจินเรนเดอร์.
- Keep these Appendix C terms distinct: `override` แทนที่ / `overwrite` เขียนทับ; `viewer` ทิศทางการมอง or หน้าต่างมุมมอง 3 มิติ / observer ผู้สังเกตการณ์ / speaker ผู้พูด / preview ตัวอย่าง.
- Keep these Appendix C terms distinct: geometry `reflect` พลิกสะท้อน / behavioral tracking ติดตามค่า / physics reflection การสะท้อน; `structure` โครงสร้าง / construction งานก่อสร้าง.
- Keep these Appendix C terms distinct: `theme` ธีม / topic หัวข้อ; `validation` การตรวจสอบความถูกต้อง / `verification` การตรวจทาน; `net gain` อัตราขยายสุทธิ / real-part gain อัตราขยายส่วนจริง.
- Keep electrical `power` กำลังไฟฟ้า distinct from the `Power` scale-family term กำลัง.
- Lock native forms กระแส, ประจุ, อัตราขยาย, and ความต้านทาน; lock borrowed forms อิมพีแดนซ์, เฟส, and แอมพลิจูด.
- Unify every outlier spelling to the topic 10 form; catalog consistency overrides locale preference for อัตราขยาย over เกน.

## 13. Priority ordering
- Precedence chain: correct meaning, then interface convention (topics 7-9), then catalog consistency (topic 12), then disambiguation (topic 11), then locale numeral form (topic 2).
- Explicit override ruling: อัตราขยาย (native compound) overrides the otherwise-default transliteration pattern for "gain", for catalog consistency and meaning clarity.

## 14. Grammatical number
- Thai nouns, adjectives, participles, and verbs do not inflect for grammatical number; add no plural marker absent from the source meaning.
- After an explicit literal count, keep the noun uninflected and place the count with its applicable classifier when natural Thai requires one.
- Use lexical quantity words only when the source expresses that quantity; never infer plurality from interface context.

## 15. Grammatical agreement
- Not applicable: Thai has no grammatical gender, number concord, declension class, participial agreement, or gendered standalone-label default.
- Not applicable: Thai requires no partitive form after a count and no reflexive particle agreement.

## 16. Morphological derivation
- Use the topic 10 locked native or borrowed form for every technical noun and verb; introduce no competing derivation.
- Form a verbal noun with `การ` plus the verb when a process noun is required; use the bare verb for an action command.
- Form established native compounds without spaces or hyphens; transliterate only concepts whose topic 10 term is locked as a loanword.
- Not applicable: Thai has no inflectional affix family selected by noun class or grammatical gender.

## 17. Preposition and sandhi selection
- Not applicable: Thai prepositions do not change form according to the following sound.
- Not applicable: Thai requires no orthographic elision, contraction, or sandhi in these literals.

## 18. Card/record-label register
- Use `การ์ด <MNEMONIC>` as the fixed designator in dialog and editor titles.
- Use the same uninflected `การ์ด <MNEMONIC>` form in running prose; Thai has no distinct suffixed or hyphenated record form.
- Keep `การ์ด` in its single full form; Thai has no case distinction or separate short designator.
- Keep title and running-prose uses internally consistent; do not abbreviate or cross-convert either register.

## 19. Multi-paragraph and whitespace fidelity
- Mirror source paragraph breaks at the same positions, preserving blank lines and single line breaks distinctly.
- Drop clauses absent from the current source rather than retaining stale translated text.
- Preserve source trailing newlines and punctuation mechanically.
- Preserve semantic line breaks; add no line break solely for visual wrapping.
- Preserve complete meaning; never truncate wording or use an unnatural abbreviation for an assumed display limit.

## 20. Current-source fidelity
- Derive every translation from the current source text and its supplied context.
- Reuse an inherited translation only when its complete meaning agrees with the current source.
- Do not inherit เกน for `gain`; use the locked term อัตราขยาย from topics 10, 12, and 13.

## 21. Script hygiene
- Use Thai script for translated prose; permit Latin, Greek, digits, and symbols only inside retained tokens whose literal spelling requires them.
- Forbid isolated foreign-script letters that imitate Thai prose characters or split a Thai word.
- Translate plain foreign words; keep only identifiers, units, symbols, proper names, and other topic 4 tokens verbatim.

## 22. Rule-file scope hygiene
- Keep only current decisions that alter translated literal wording, Unicode characters, punctuation, capitalization, embedded tokens, mnemonic markers, or semantic whitespace.
- Omit catalog representation, serialization, headers, flags, workflow, rendering, fonts, shaping, layout, widget behavior, runtime formatting, sorting, search, display sizing, audit history, completion state, review metadata, provenance, and citations.
- State each retained decision directly; include no implementation procedure, bibliography, correction history, or alternative policy source.

## 23. Section-disjointness declaration
- Script mechanics (topic 1) governs code points, directionality, and spacing only.
- Phrasing and structure (topics 5, 7, 9, 18, and 19) govern sentence and label construction only.
- Address register (topic 8) governs politeness, pronouns, and formality only.
- Keep these axes non-overlapping; place every concept on exactly one axis.

## 24. Developer/debug-string policy
- Translate user-facing commands, labels, dialogs, tooltips, status messages, errors, and informational diagnostics into Thai.
- Translate developer-facing diagnostics and assertion text into terse, technical Thai without politeness marking.
- Keep embedded identifiers, function names, variable names, configuration keys, format specifiers, and topic 4 retained tokens verbatim in every string family.
- Treat priority as review order only; translate every applicable user-facing string.
- Preserve embedded format and retained tokens independently of each family's register or review priority.
