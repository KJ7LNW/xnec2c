# Vietnamese (Vietnam) translation rules

These rules govern modern Vietnamese in quốc ngữ for Vietnam; write for professional
electrical and radio-frequency engineers, antenna designers, and amateur-radio operators.

## 1. Script and orthography

Treat quốc ngữ spelling as semantic content throughout every translated literal.

- Use the Latin quốc ngữ script in precomposed Unicode Normalization Form C (NFC): retain `ă`,
  `â`, `ê`, `ô`, `ơ`, `ư`, `đ`, and every sắc, huyền, hỏi, ngã, and nặng tone mark; never
  strip a diacritic or fold a letter to a look-alike.
- Ambiguous apostrophe, modifier-letter, and textual-joiner code points do not occur in
  standard quốc ngữ; each is not applicable.
- Script-specific joining and positional letter forms are absent; not applicable.
- Write Vietnamese left to right; embedded retained technical tokens remain left to right and
  are never mirrored or wrapped in manual direction controls.
- Apply topic 6 casing because quốc ngữ distinguishes uppercase and lowercase letters.
- Use modern post-1954 Vietnamese orthography for Vietnam; introduce no regional spelling
  alternative.
- Separate native syllables with one ASCII space; place one ASCII space between Vietnamese
  text and an adjacent foreign or numeric token when ordinary word separation applies.
- Form technical compounds as open, space-separated words, eg "trở kháng"; use a hyphen only
  when a locked retained token or established orthography requires one, and never fuse words.

## 2. Numerals in literals

- Use source-form Arabic digits for every technical value physically present in a translated
  literal.
- Retain the source period as the decimal separator and retain source grouping unchanged;
  insert no thousands separator.
- Keep formulas, examples, fixed defaults, named mathematical constants, and standards
  constants in their exact source numeral form.
- Place an ordinal or index digit after its Vietnamese noun with one space and no affix, eg
  "đoạn 3"; this space is an index separator, not a decimal mark.
- Preserve numbers inside format specifiers under topic 5; they are token syntax, not
  localized numerals.

## 3. Punctuation and quotation

- Quotation marks: plain double quotes `"..."` are kept for embedded technical tokens
  (filenames, card mnemonics, literal values) rather than switching to native guillemets, for
  consistency with the source and with topic 4's never-translate roster.
- Native punctuation: comma, question mark, exclamation mark, and opening marks are the same
  ASCII glyphs as the source; Vietnamese prose uses no distinct native forms for these.
- No space before colon or semicolon, and no space before terminal punctuation; spacing
  follows the source convention throughout.
- Ellipsis: single glyph `…`; dashes in the source (en/em dash) are preserved as-is, not
  converted to a native dash form.
- Sentence terminator: full stop (`.`) closes full grammatical sentences (dialogs,
  confirmations, error prose); short labels, menu items, and field-label fragments take no
  terminal punctuation.
- Punctuation embedded inside a technical run (inside a mnemonic, unit, or format specifier)
  stays in source form, never localized.

## 4. Never-translate tokens

Retain every listed token exactly: translate and transliterate none of them.

- Keep NEC2 card mnemonics `GW GA GH EX LD FR RP GE EN SP SM SC NE NH NT TL GN GD GC GX GR
  GS GM GF CP CM SY XQ EK KH PQ PT` verbatim so file-format identifiers preserve identity.
- Keep unit symbols `Hz MHz dB dBi Ω W V A K ° %` and the parenthetical angle tag `(deg)`
  verbatim so standard electrical and radio-frequency notation remains unambiguous; translate
  freestanding prose `degrees` under topic 10.
- Keep figure and parameter tokens `VSWR`, `S11`, `S21`, `Z`, `Z0`, `F/B`, and `G/T` verbatim
  so established radio-frequency notation remains intact; apply the same rule to every
  source S-parameter token.
- Keep file extensions `.nec`, `.csv`, `.s1p`, `.s2p`, and `.png` verbatim so literal filenames
  preserve their format suffix.
- Keep every source format specifier verbatim under topic 5 so placeholder syntax remains
  valid.
- Keep embedded function names, variable names, config keys, and other code identifiers
  verbatim so developer strings continue to name the same program entity.
- Keep physical and mathematical symbol letters `E`, `H`, `θ`, `φ`, and `j` verbatim so a
  native look-alike cannot change the notation.
- Keep product, library, toolkit, and chart names `xnec2c`, `NEC2`, `GTK`, `Cairo`, `OpenGL`,
  and `Smith` verbatim as proper names.
- Keep named transfer and algorithm terms `Log`, `Asinh`, `μ-law`, `Reinhard`, `Sigmoid`, and
  `Identity` verbatim; translate descriptive family name `Power` as `Lũy thừa` under topic 10.
- Keep conditional loanwords `tag` and process `fork` verbatim; translate `segment` as `đoạn`
  and `patch` as `mảnh` under topic 10, with the same decision at every occurrence.
- Keep every retained token left to right under topic 1, without mirroring or manual direction
  controls.

## 5. Format-specifier integrity

- Preserve every source specifier exactly once with the same conversion, flags, width,
  precision, length modifier, and positional index.
- Keep source specifier order by restructuring Vietnamese wording around fixed placeholder
  positions.
- Reorder only explicitly positional placeholders when natural Vietnamese syntax cannot
  preserve source order; retain each original positional index unchanged.
- Keep every digit inside a specifier in source form; localize only surrounding prose.

## 6. Capitalization and title-case

- Default casing: sentence case for labels, menu items, dialog titles, and prose — only the
  first word of a sentence/label and proper nouns are capitalized; English-style title case
  on multi-word menu labels is forbidden (eg "Hiển thị bề mặt" not "Hiển Thị Bề Mặt").
- Axis-letter casing: an axis letter (X, Y, Z) keeps its source uppercase form (eg "Trục X").
- A lowercase math/coordinate variable (eg `x`, `y` in a formula) keeps its source lowercase
  form; sentence-case does not raise it.
- No coordinated-option-name capitalization pattern is evidenced in this catalog; not
  applicable.
- Acronyms and proper nouns (`VSWR`, `NEC2`, `xnec2c`) retain source casing regardless of
  sentence position.
- Generic card/record-noun casing is register-dependent; resolved in topic 18.

## 7. Interface register by string type

Treat each interface string family as a fixed grammar; preserve all source meaning in the
shortest complete natural form.

- Commands, buttons, and menu actions use a subject-dropped imperative: verb first, then
  object and qualifiers; use neither an infinitive nor a deverbal noun.
- Field labels use a concise noun phrase in head-before-modifier order and retain a source
  colon, eg "Tần số:".
- Dialog statements use full declarative sentences in subject-verb-object order; confirmation
  questions use topic 8's explicit-subject pattern.
- Tooltips use full declarative sentences in cause-before-result order; when the source gives
  a reason that a control is unavailable, retain that reason.
- Status and error messages use subject-dropped declarative clauses: state the affected entity,
  condition, then cause or remedy; do not recast an error as a command.
- User-visible domain entities use the topic 10 canonical noun followed by its identifier or
  distinguishing modifier; proper and personal names follow topic 8.
- Prefer open multiword compounds and natural Vietnamese information order; omit no source
  meaning, invent no abbreviation, and do not imitate source length or token order.
- Keep format-specifier order under topic 5 while restructuring the surrounding sentence.
- Developer and debug strings follow topic 24.

## 8. Formality and address

- Use neutral professional address: omit the subject except in direct confirmation questions,
  where "bạn" is the second-person singular pronoun, eg "Bạn có chắc chắn muốn thoát xnec2c
  không?".
- Commands, status text, errors, labels, and tooltips use the impersonal subject-dropped forms
  in topic 7; confirmations use a full question with "bạn", verb phrase, object, then "không?".
- Vietnamese marks formality through pronoun choice and subject omission, not verb endings;
  morphological formality is not applicable.
- Grammatical gender and gender agreement are absent; use gender-neutral role and person terms
  without adding a gender marker.
- Keep inclusive wording natural by naming the role, action, or person without assumptions
  about gender, age, status, number, or animacy beyond the source.
- Honorifics are not applicable to this interface; use no title unless it is part of a source
  personal name.
- Personal names retain their supplied order and spelling; do not infer or reorder family and
  given names.
- Prohibit commercial address "quý khách", status-marked address "ông/bà", slang,
  over-formal, and archaic wording; these conflict with the neutral professional register.

## 9. Accelerator/hotkey mnemonics

- Preserve the source mnemonic marker `_`; place it immediately before a typable
  syllable-initial letter in the translated term, retaining that letter's diacritics, eg
  `_Tần số`.
- Choose the mnemonic from the translated term, never from a source transliteration; keep it
  unique among controls in the same container by selecting another syllable-initial letter.
- A separate-script mnemonic presentation is not applicable because Vietnamese uses Latin
  letters.
- Add a mnemonic only when the source literal contains one; never add one to a label, tooltip,
  or other source string without a marker.
- Prefer an unaccented letter when equally distinctive; otherwise retain the required
  Vietnamese letter and diacritics.

## 10. Domain lexicon

Concept key, chosen Vietnamese term, and the sense/hazard the mapping guards.

### Electrical primitives

| Concept | Vietnamese | Purpose |
|---|---|---|
| current | dòng điện | never "hiện tại" (present time) |
| charge | điện tích | never "phí" (billing); no accepted homonym with load |
| voltage | điện áp | canonical domain sense; prevents synonym drift |
| power (electrical) | công suất (điện) | distinct from "Power" scale-family name → "Lũy thừa" |
| impedance | trở kháng | distinct from resistance, reactance |
| resistance | điện trở | real part of Z |
| reactance | điện kháng | imaginary part of Z |
| inductance | độ tự cảm | canonical domain sense; prevents synonym drift |
| capacitance | điện dung | canonical domain sense; prevents synonym drift |
| conductivity | độ dẫn điện | material S/m |
| admittance | dẫn nạp | distinct from impedance |
| load | tải | not "gánh nặng" (burden); no accepted homonym with charge |
| gain | độ lợi | antenna directivity (dB); never "lợi nhuận" (profit) |
| excitation | kích thích | EM energy input; no emotional sense in Vietnamese |
| feedpoint | điểm cấp điện | canonical domain sense; prevents synonym drift |
| port | cổng (kích thích/tham số S) | not a hardware connector |
| radials | dây tiếp đất tỏa tia | noun; distinct from adjective "(hình) tia" |

### Ground and earth

| Concept | Vietnamese | Purpose |
|---|---|---|
| ground / ground plane | mặt đất | RF electrical reference; one term across all ground senses |
| earth (physical medium) | nền đất | terrain/noise-model earth; distinct from mặt đất |
| ground wave | sóng mặt đất | propagation term, distinct from the reference sense |

### Geometry primitives

| Concept | Vietnamese | Purpose |
|---|---|---|
| wire | dây (dẫn) | thin conductor; not "cáp" (cable/cord) |
| segment | đoạn (dây) | NEC2 geometry subdivision |
| patch | mảnh | NEC2 surface patch |
| tag | tag (giữ nguyên) | avoids collision with "thẻ" = card (topic 4) |
| card | thẻ (lệnh/dữ liệu) | register in topic 18 |
| kernel | hạt nhân (tích phân) | not an OS kernel |
| cliff | vách | two-medium ground boundary; not "đứt gãy" (fracture) |
| structure | cấu trúc | the model geometry; not "xây dựng" (construction) |
| model | mô hình | canonical domain sense; prevents synonym drift |
| geometry | hình học | canonical domain sense; prevents synonym drift |
| crossed | bắt chéo | conductors crossed/reversed; not "cắt đứt" (severed) |

### Field, pattern, viewer

| Concept | Vietnamese | Purpose |
|---|---|---|
| field (EM) | trường (điện từ) | distinct from a data field "trường (dữ liệu)" |
| near field / far field | trường gần / trường xa | opposed pair, kept symmetric |
| far-field contribution | đóng góp trường xa | per-direction; not near-field animation |
| radiation | bức xạ | canonical domain sense; prevents synonym drift |
| radiation pattern | giản đồ bức xạ | not "mẫu/khuôn"; not "đồ thị bức xạ" ("đồ thị" = XY plot) |
| gain pattern | giản đồ độ lợi | same "giản đồ" rule as radiation pattern |
| polarization | phân cực | canonical domain sense; prevents synonym drift |
| polarity | cực tính | sign (+/-); false friend of polarization |
| phase | pha | canonical domain sense; prevents synonym drift |
| reference phase | pha tham chiếu | canonical domain sense; prevents synonym drift |
| frequency | tần số | canonical domain sense; prevents synonym drift |
| wave / wavelength | sóng / bước sóng | canonical domain sense; prevents synonym drift |
| standing wave / traveling wave | sóng đứng / sóng chạy | opposed pair |
| node / antinode | nút sóng / bụng sóng | standing-wave zero/max; also null/peak overlay sense |
| crest | đỉnh sóng (tức thời) | comet-head apex; distinct from a curve "đỉnh (đường cong)" |
| magnitude | độ lớn | modulus/scalar; distinct from amplitude |
| amplitude | biên độ | oscillating peak; distinct from magnitude |
| peak value | giá trị đỉnh | distinct UI option from peak magnitude, never collapsed |
| peak magnitude | độ lớn đỉnh | distinct UI option from peak value, never collapsed |
| instantaneous | tức thời | "(φ=0)" qualifier only when the source carries it |
| Poynting vector | vector Poynting | proper name kept; "vector" translates |
| solid angle | góc khối | canonical domain sense; prevents synonym drift |
| net gain | độ lợi ròng | never "độ lợi thực" ("thực" = real part) |
| viewer | hướng quan sát / khung xem | observation direction / 3D view widget; not "người quan sát" |
| flow / flow direction | dòng chảy / hướng dòng chảy | patch/current flow |
| total field | trường tổng | canonical domain sense; prevents synonym drift |

### Color, tone, animation

| Concept | Vietnamese | Purpose |
|---|---|---|
| color | màu (sắc) | canonical domain sense; prevents synonym drift |
| color projection | phép chiếu màu | which quantity drives hue |
| hue | sắc màu | color-wheel angle |
| brightness | độ sáng | luminance channel |
| hue encoding | mã hoá sắc màu | distinct enum, not "phép chiếu màu" |
| brightness encoding | mã hoá độ sáng | distinct enum, not "phép chiếu màu" |
| color scale | thang màu | magnitude-to-color scale |
| scale family / color tone | họ thang chuyển đổi | one concept, two source spellings, one term |
| palette / palette kind | bảng màu / kiểu bảng màu | distinct from scale family and color projection |
| ramp / gradient | dải chuyển màu | a palette kind / linear strip |
| gamma | gamma (hệ số) | power-law exponent; loanword kept |
| knee | điểm gãy mềm | soft-knee bend point |
| softening | làm mềm | dynamic-range |
| compression | nén | dynamic-range |
| contrast | độ tương phản | canonical domain sense; prevents synonym drift |
| dynamic range | dải động | canonical domain sense; prevents synonym drift |
| floor | mức sàn | minimum/lower clamp; not a room floor |
| envelope | đường bao | magnitude/amplitude envelope |
| comet | hiệu ứng sao chổi | moving-crest overlay; not geometry (fuzzy-inheritance hazard) |
| overlay (noun) | lớp phủ | added visual layer; distinct from verb "phủ lên" |
| animate / animation | hoạt ảnh | never "hoạt hình" (cartoon-flavored); catalog-wide |
| animated / static | động / tĩnh | category-header adjectives |
| projection | phép chiếu | color or geometry projection |
| scale | tỷ lệ | noun; verb "tỷ lệ hoá" (topic 11) |
| wireframe | khung dây | canonical domain sense; prevents synonym drift |
| identity | đồng nhất | no-op transfer; distinct from unity "đơn vị" (Smith-chart) |
| sentinel | giá trị canh gác | unreachable-case guard value |
| bins | ngăn (rời rạc hoá) | discretization buckets |
| companding | nén-giãn (logarit) | μ-law bounded log curve |
| tone mapping | ánh xạ tông màu | photographic tone-map |

### Render and compute

| Concept | Vietnamese | Purpose |
|---|---|---|
| renderer | bộ dựng hình | backend; not "công cụ dựng hình" (render engine) |
| shader | shader (giữ nguyên) | canonical domain sense; prevents synonym drift |
| allocation (memory) | cấp phát (bộ nhớ) | canonical domain sense; prevents synonym drift |
| managed allocator | bộ cấp phát có quản lý | canonical domain sense; prevents synonym drift |
| thread | luồng (tính toán) | distinct from "dây" (wire); no homonym collision |
| widget | tiện ích (giao diện) | native term, not loanword |
| validation | kiểm định | distinct from verification "kiểm tra" |
| batch mode | chế độ hàng loạt | canonical domain sense; prevents synonym drift |
| fork (process) | fork (giữ nguyên) | conditional loanword (topic 4) |
| deadlock | bế tắc (luồng) | canonical domain sense; prevents synonym drift |
| notifier | bộ thông báo | canonical domain sense; prevents synonym drift |
| token | token (giữ nguyên) | expression-parser term |
| operand | toán hạng | canonical domain sense; prevents synonym drift |
| operator | toán tử | canonical domain sense; prevents synonym drift |
| arity | ngôi | eg "một ngôi"/"hai ngôi" |
| override | thay thế (giá trị) | supersede; distinct from overwrite "ghi đè" |
| swap | hoán đổi | canonical domain sense; prevents synonym drift |
| theme | giao diện (màu) | UI/color theme; not "chủ đề" (topic/subject) |

### Metrics and miscellaneous

| Concept | Vietnamese | Purpose |
|---|---|---|
| noise / noise temperature | nhiễu / nhiệt độ nhiễu | electronic/thermal; not "tiếng ồn" (acoustic) |
| efficiency | hiệu suất | canonical domain sense; prevents synonym drift |
| interpolation | nội suy | canonical domain sense; prevents synonym drift |
| mnemonic | mã gợi nhớ | a card's code descriptor; not a memo/note |
| degrees / deg | độ | prose form; the tag "(deg)" is kept verbatim (topic 4) |
| diameter | đường kính | canonical, no competing native synonym |
| reflect (geometry mirror) | lấy đối xứng (gương) | mirror operation |
| reflect (mirrors …, tracking) | đồng bộ theo | behavioral tracking, distinct from mirror op |
| reflect (physics) | phản xạ | distinct from mirror op and behavioral tracking |
| default(s) | mặc định | canonical domain sense; prevents synonym drift |
| normalize / normalization | chuẩn hoá | translated, not transliterated |

## 11. Disambiguation policy

- The correct technical sense is chosen for every ambiguous term (topic 10 table).
- No qualifier absent from the source is added; program context already disambiguates.
- A qualifier is added only where the Vietnamese term alone would be genuinely ambiguous
  outside this domain (eg "điện" appended to "dòng điện").
- No accepted intra-domain homonym exists for load/charge, or for ground/earth ("mặt đất" vs
  "nền đất" stay distinct — topic 10).
- No locative homonym collision requires resolution beyond the ground/earth split above.
- Gerund vs noun senses: "scale" as verb is "tỷ lệ hoá", as noun "tỷ lệ"; "animate" as verb is
  "làm hoạt ảnh", as noun "hoạt ảnh" — the noun form is the catalog-wide default term.

## 12. Cross-catalog consistency

Use one topic 10 term per source concept throughout the catalog; introduce no local synonym.

- Keep `cực tính` for polarity distinct from `phân cực` for polarization.
- Keep `độ lớn` for magnitude distinct from `biên độ` for amplitude; keep `giá trị đỉnh` for
  peak value distinct from `độ lớn đỉnh` for peak magnitude.
- Keep `mặt đất` for electrical ground distinct from `nền đất` for the physical earth medium.
- Keep `tải` for impedance load distinct from `điện tích` for electrical charge.
- Keep `độ lợi` for antenna gain distinct from `khuếch đại` for amplifier amplification and
  `lợi nhuận` for profit.
- Keep `dòng điện` for electrical current distinct from `hiện tại` for temporal present; keep
  `điện tích` for electrical charge distinct from `phí` for billing.
- Keep `dây` for a thin conductor distinct from `cáp` for cable, `dây điện` for cord, and
  `luồng` for a compute thread.
- Keep `giản đồ bức xạ` for radiation pattern distinct from `mẫu thiết kế` for a template and
  `trường xa` for the far-field region.
- Keep `kích thích` for electromagnetic excitation distinct from `phấn khích` for emotion.
- Keep `nút sóng` and `bụng sóng` for standing-wave node and antinode distinct from `điểm
  không` for a generic numeric null.
- Keep `họ thang chuyển đổi`, `sắc màu`, `kiểu bảng màu`, and `phép chiếu màu` distinct for
  scale family, hue, palette kind, and color projection.
- Keep `hiệu ứng sao chổi` for the comet overlay distinct from `hình học` for geometry; keep
  `đồng nhất` for identity transfer distinct from `đơn vị` for Smith-chart unity.
- Keep `bộ dựng hình` for renderer distinct from `công cụ dựng hình` for render engine; keep
  `thay thế` for override distinct from `ghi đè` for overwrite.
- Keep `hướng quan sát` or `khung xem` for viewer distinct from `người quan sát`, `người nói`,
  and `xem trước` for observer, speaker, and preview.
- Keep geometry `lấy đối xứng`, behavioral `đồng bộ theo`, and physical `phản xạ` distinct for
  the three reflect senses.
- Keep `cấu trúc` for model geometry distinct from `xây dựng` for construction; keep `giao
  diện` for a user-interface theme distinct from `chủ đề` for topic or subject.
- Keep `kiểm định` for the validation-tree feature distinct from `kiểm tra` for verification.
- Keep `độ lợi ròng` for net gain distinct from `độ lợi phần thực` for real-part gain; keep
  electrical `công suất` distinct from transfer-family `Lũy thừa`.
- Keep declared loanwords `shader`, `token`, `fork`, `tag`, and `gamma`; use topic 10's native
  terms elsewhere and unify every outlier spelling to the canonical form.
- Use `hoạt ảnh`, never `hoạt hình`, for animation; this prevents the cartoon sense.
- Let catalog consistency override locale-form preference only for `tag`, whose retained form
  prevents collision with card term `thẻ`.

## 13. Priority ordering

- Precedence chain: correct meaning, then interface convention, then catalog consistency,
  then disambiguation, then locale numeral form.
- Override ruling: `tag` stays English, overriding the default translate-preference, because
  catalog consistency (avoiding collision with "thẻ" = card) outranks the loanword-avoidance
  interface convention (topic 4, topic 12).

## 14. Grammatical number

- Vietnamese uses one uninflected noun, adjective, participle, and verb form for singular and
  plural references; add no agreement suffix or plural inflection.
- After every explicit literal count, use the bare noun after the numeral, eg "5 tệp"; add a
  classifier only when natural Vietnamese grammar requires it and the source meaning permits
  it.
- Add a plural word only when the source explicitly expresses plurality and omission would
  lose that contrast; never infer plurality from interface context.

## 15. Grammatical agreement

- Grammatical gender, number concord, declension classes, partitive count forms, and reflexive
  particles are absent in Vietnamese; each is not applicable.
- A standalone label uses the same invariant adjective, participle, and noun forms as an
  explicit phrase; no implied head noun introduces agreement.

## 16. Morphological derivation

- Form technical nouns and verbs with established Vietnamese or Hán Việt roots and open
  compounding; prohibit ad-hoc affixes on source-language roots.
- Form a verbal noun with the established `-hóa` construction only when that derivation is
  idiomatic, eg "chuẩn hóa"; otherwise use the locked noun or verb in topic 10.
- Prefer established native or Hán Việt compounds; retain only topic 4's declared loanwords.

## 17. Preposition and sandhi selection

- Context-conditioned preposition forms, sandhi, written elision, and contraction are absent
  in Vietnamese; each is not applicable.

## 18. Card/record-label register

- Use the fixed designator "thẻ <MNEMONIC>" in dialog and editor titles, eg "thẻ FR".
- Use the same open noun phrase in running prose, adding the required Vietnamese preposition,
  eg "trong thẻ FR"; no hyphenated or suffixed variant applies.
- Keep generic "thẻ" lowercase except at sentence start; no distinct short and long forms
  apply.
- Keep title and running-prose forms internally consistent; do not convert either into a
  source-style compound.

## 19. Multi-paragraph and whitespace fidelity

- Mirror each source paragraph break at the same position, preserving blank-line and
  single-line distinctions.
- Preserve semantic line breaks; add no visual wrapping absent from the source literal.
- Drop every trailing clause removed from the current source rather than retaining inherited
  text.
- Preserve source trailing newlines and punctuation exactly as semantic literal content.
- Preserve complete meaning; never truncate wording or invent an unnatural abbreviation for
  an assumed display limit.

## 20. Current-source fidelity

- Derive every translation from the current source literal and supplied context.
- Reuse an inherited translation only when its complete meaning agrees with the current
  source; otherwise translate the current source anew.
- Treat inherited "hoạt hình" for the animation concept as unsafe; use topic 10's canonical
  "hoạt ảnh".

## 21. Script hygiene

- Forbid source-language words in Vietnamese prose unless topic 4 retains their exact spelling;
  translate ordinary foreign words.
- Forbid stripped-diacritic Vietnamese, decomposed look-alikes, and substitution of `d` for
  `đ`; allow source-script characters only inside a retained token that requires them.
- Cross-script confusables are not applicable to ordinary Vietnamese prose; topic 4's retained
  Greek mathematical letters remain exact tokens rather than prose characters.

## 22. Rule-file scope hygiene

- Keep only decisions that alter translated literal wording, Unicode characters, punctuation,
  capitalization, embedded tokens, mnemonic markers, or semantic whitespace.
- Omit catalog representation, serialization, headers, flags, validation workflow, rendering,
  fonts, shaping, layout, widget behavior, runtime formatting, sorting, search, display sizing,
  audit history, completion state, review metadata, provenance, and citations.
- State the current Vietnamese decision directly; include no implementation procedure,
  correction history, bibliography, or alternative policy source.

## 23. Section-disjointness declaration

- Script mechanics govern only character repertoire, normalization, direction, spacing,
  punctuation, case, mnemonic characters, and script-confusable hygiene in topics 1, 3, 6, 9,
  and 21.
- Literal structure governs only numeral spelling, retained tokens, format placeholders,
  interface grammar, lexicon, ambiguity, consistency, grammatical form, derivation, card
  labels, source meaning, and semantic whitespace in topics 2, 4, 5, 7, and 10-20.
- Address register governs only pronouns, person reference, honorifics, inclusivity, and
  formality in topic 8.
- Rule scope and developer-family policy govern only their declared boundaries in topics 22
  and 24; each decision belongs to one axis.

## 24. Developer/debug-string policy

Treat every catalog string family by audience; review priority never permits an applicable
user-facing string to remain untranslated.

- User-facing commands, labels, dialogs, confirmations, tooltips, status messages, and errors
  translate into Vietnamese under topics 1-23.
- Informational notices, reports, and user-visible diagnostics translate into Vietnamese in a
  concise professional register.
- Developer-facing debug traces, internal assertions, function diagnostics, and low-level
  parser diagnostics stay in source form unless their established subsystem family is
  user-visible; a user-visible family follows the informational rule.
- Keep embedded identifiers, function names, config keys, format specifiers, and topic 4
  retained tokens verbatim in every family.
- Any translated diagnostic uses terse technical Vietnamese and preserves the complete source
  meaning; token retention is independent of family priority.
