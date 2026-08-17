# hi (Hindi) translation rules

## 1. Script and orthography

- Locale scope: India-region standard modern Hindi in Devanagari
  (देवनागरी), Unicode block U+0900–U+097F, for professional electrical and
  radio-frequency engineers, antenna designers, and amateur-radio operators.
- Orthography follows the current Government of India standard. Encode text
  in Unicode NFC with canonically ordered combining marks; do not substitute
  visually similar code points.
- Nukta (़) is mandatory on borrowed-sound consonants where the established
  spelling carries it: फ़ाइल, ग़, ज़, फ़ीडपॉइंट; never drop the nukta or
  substitute the plain consonant.
- Virama (्) forms required conjunct spellings. Joiners, apostrophe-like
  modifier letters, and ambiguous textual marks are not applicable to this
  vocabulary; do not introduce them.
- Directionality: left-to-right; no bidirectional runs occur (embedded
  Latin/numeric tokens read left-to-right inline, same direction as
  surrounding Hindi text).
- No letter-case distinction exists in Devanagari; casing rules apply only
  to embedded Latin-script tokens (topic 6).
- Inter-word spacing: space-separated words; a space always separates
  native Hindi text from an embedded Latin/numeric technical token (eg
  `आवृत्ति MHz`, `NEC2 सिम्युलेटर`), never fused.
- Compound formation: use the established Hindi spelling for lexicalized
  compounds; write ordinary multi-word technical phrases with spaces, and
  use a hyphen only for an established paired technical relation.

## 2. Numerals and locale data

- Digit set: Western/Arabic digits (0-9) for every technical value, count,
  frequency, and unit; never Devanagari numerals (०-९).
- Decimal separator: period `.`, same as source. Exceptions kept in source
  form unconditionally: runtime format-specifier output, literal
  formula/example tokens, fixed default values, and named
  mathematical/standards constants.
- Thousands/grouping separator: international 3-digit comma grouping, same
  as source; never the lakh/crore grouping.
- Ordinal/index notation: an index like `Element 1,1` keeps the digit pair
  and comma as in source; the comma there is an index separator, distinct
  from the decimal-point separator above, never conflated.
- Formulas, examples, fixed defaults, and named mathematical or standards
  constants retain every literal digit and separator from the source.

## 3. Punctuation and quotation

- Quotation marks: no distinct native glyph; retain source straight quotes
  `"…"` for both prose and embedded technical tokens, for catalog
  consistency.
- Native punctuation replacing source counterparts in prose: the Devanagari
  danda (।) replaces the Latin full stop `.` at sentence end. Comma `,`,
  question mark `?`, and exclamation `!` keep their source Western forms;
  Hindi uses no distinct opening punctuation marks.
- Spacing: no space before colon/semicolon/danda; one space after.
- Ellipsis form: `…` retained as in source. Dashes (`-`, `–`) retained as
  in source, never converted to a native mark.
- Sentence-terminator policy: full grammatical sentences (dialogs,
  confirmations, status/error messages) take the trailing danda; short
  labels, buttons, and field names take no trailing punctuation. Never mix
  a Latin period and a danda as sentence-final marks across sibling
  strings.
- Punctuation inside an embedded technical run (a file path, format
  specifier, code token) stays in source form, unconverted.

## 4. Never-translate tokens

- NEC2 card mnemonics kept verbatim: `GW GA GH EX LD FR RP GE EN` and the
  extended set `SP SM SC NE NH NT TL GN GD GC GX GR GS GM GF CP CM SY XQ EK
  KH PQ PT`.
- Unit symbols kept verbatim: `MHz`, `dBi`, `dB`, `Ω`, `%`, `K`, `S/m`,
  and the parenthetical unit tag `(deg)` (the freestanding prose word
  "degrees" translates, per topic 10).
- Figure-of-merit tokens kept verbatim: `VSWR`, S-parameter tokens, `Z0`
  and other impedance/reference-impedance symbols, `F/B`, `G/T`.
- File extensions kept verbatim: `.nec`, `.csv`, `.s1p`, `.s2p`, `.png`;
  a complete literal filename keeps its source spelling and suffix.
- Format specifiers kept verbatim (topic 5).
- Embedded identifiers (function names, variable names, config keys) inside
  developer strings kept verbatim (topic 24).
- Physical/mathematical symbol letters kept verbatim in source case: `E`,
  `H`, `Z`, axis letters `X`/`Y`/`Z`, exponent/angle letters.
- Product/library/tool names kept verbatim: `xnec2c`, `NEC2`, `GTK`,
  `Cairo`, `OpenGL`, `Smith` (chart name).
- Named transfer-function/algorithm terms kept verbatim: `Log`, `Asinh`,
  `μ-law`, `Reinhard`, `Sigmoid`, `Identity`. Exception: the descriptive
  `Power` scale-family name translates, as घात (topic 10; distinct from
  electrical power शक्ति, topic 10/12).
- Conditional loanwords, declared once and held file-wide: `segment`,
  `patch`, `tag` stay untranslated in Latin script, matching source case,
  at every site (bare NEC2 geometry terms and geometry-card field labels
  alike, eg `Segment संख्या`, "Patch"); `fork` (process) stays untranslated,
  no native equivalent coined.
- Retained-token integrity is a zero-failure invariant: every listed
  mnemonic, unit, figure-of-merit token, suffix, specifier, identifier,
  symbol letter, proper name, algorithm name, and declared loanword keeps
  its exact source spelling, script, punctuation, and case; none is translated
  or transliterated.
- These embedded Latin tokens follow topic 1's left-to-right rule; no
  direction override.

## 5. Format-specifier integrity

- Every specifier present in the source is preserved, same set, in the
  translation.
- Preserve the source specifier order by default. When natural Hindi word
  order requires argument reordering and the format supports positional
  specifiers, retain the same specifier set and use explicit positions.
- Where positions cannot change, restructure the Hindi sentence around the
  fixed specifiers without changing their spelling or internal numbers.
- Format-specifier preservation is a zero-failure invariant: no specifier
  may be added, removed, translated, transliterated, or internally localized.

## 6. Capitalization and title-case

- Devanagari has no letter-case distinction. Hindi labels, menu items, and
  titles use ordinary sentence wording rather than source-style title case;
  coordinated option names apply the same rule to every element.
- Embedded acronyms and proper nouns (`NEC2`, `GTK`, `VSWR`) retain source
  case verbatim.
- Axis letters `X`/`Y`/`Z` retain source uppercase.
- A lowercase math/coordinate variable embedded from the C source stays
  lowercase: `z` in "step size limited at z=" / "Hankel not valid for z =
  0" is never capitalized to `Z`; the axis-letter uppercase convention
  above governs axis names, not an echoed source variable.
- Mnemonic-paired dialog-title parentheticals (`(GA card)`, `(RP Card)`)
  keep the source English word's case exactly.

## 7. Interface register by string type

- Commands, buttons, and menu actions use a subjectless formal imperative:
  object or complement first, verb last, eg `फ़ाइल चुनें`.
- Field labels use the shortest complete noun phrase in modifier-head order,
  followed by the source colon; they contain no supplied subject or verb.
- Dialogs and confirmations use full sentences in subject-object-verb order;
  yes/no confirmations place `क्या` first and the finite verb last.
- Tooltips use complete declarative sentences in known-to-new information
  order; when the source gives a disabled control's reason, state the control,
  then the reason.
- Status and error messages use subject-drop, impersonal or passive
  declaratives with the affected domain entity before the result or cause.
- User-visible domain entities use the topic 10 canonical head term after
  its modifiers; proper names and personal names follow topic 8.
- Terseness: labels and buttons stay as terse noun/verb phrases; dialogs
  and messages use full clauses. Multi-word technical phrases stay spaced
  (topic 1), never fused; restructure word order around SOV grammar
  (topic 5) rather than forcing source word order.
- Developer/debug-string register is topic 24.

## 8. Formality and address

- आप (formal "you") and its verb agreements used throughout; तू/तुम never
  used.
- Formal imperative endings: `-करें` (चुनें, देखें, सक्षम करें), never
  casual `-करो`/`-देखो` forms.
- Where a string's subject is unspecified (most status/label strings),
  Hindi uses impersonal/subject-drop or passive construction rather than
  supplying a pronoun; where a verb must still agree in gender with no
  stated subject, the default is masculine agreement, the established
  convention in this catalog.
- Confirmation dialogs use a full-sentence yes/no question form (eg "क्या
  आप निश्चित हैं?"), danda- or question-mark-terminated per topic 3.
- Commands use formal imperatives; dialogs use formal address or impersonal
  sentences; informational text uses neutral professional prose.
- Avoid unnecessary gender marking and gendered occupational labels. Where
  agreement is required without an expressed head, use the established
  masculine default without implying the user's gender.
- Honorifics are not applicable in interface strings. Use second-person plural
  formal agreement with `आप`; preserve a person's written name order and add
  no title absent from the source.
- This axis is independent of script mechanics (topic 1) and lexical choice
  (topic 10): formality governs only pronouns, honorifics, and verb endings.
- Prohibited registers: casual/slang, commercial marketing tone, archaic
  Sanskritized diction, and over-formal bureaucratic phrasing.

## 9. Accelerator/hotkey mnemonics

- Marker convention: a leading underscore (`_`) before the mnemonic letter
  in the msgstr, exactly mirroring GTK's underscore-mnemonic mechanism.
- Choose an unambiguous, easily typed ASCII mnemonic letter from the
  translated term's displayed parenthetical form; use another letter from
  that term when the first would duplicate a sibling mnemonic.
- The mnemonic letter is drawn from the translated Hindi term's own
  presentation, never transliterated from the English source letter.
- Presentation for Devanagari labels: append a parenthetical Latin letter
  after the Hindi label, eg "जोड़ें (_A)", since Devanagari text is not
  directly keyed as a single-character accelerator.
- Source-presence-only rule: a `_` mnemonic is added only to a msgstr
  whose msgid carries one; a dialog title or message with no `_` in the
  source never gains one ("Batch Math Library", "Mathlib Help", "Mathlib
  Benchmark", "Mathlib Benchmark Help" take no `_`, even though the
  mirroring menu items "Mathlib _Help", "Mathlib _Benchmarks" do).
- Preserve a mnemonic marker only where the source literal carries one;
  its string type does not independently add or remove a marker.

## 10. Domain lexicon

Locked term table; concept key → chosen term (sense; purpose/hazard guarded).
For a row without a narrower hazard note, its stated engineering sense guards
correct meaning and its single chosen term guards catalog consistency.

### Electrical primitives

- current → धारा (electrical current, A; never the temporal sense).
- charge → आवेश (electrical charge, C; never billing/fee).
- voltage → वोल्टता (electric potential).
- power (electrical) → शक्ति (radiated/dissipated watts, power gain,
  power-flow; distinct from घात, the math power-law scale-family name,
  topic 4/12).
- impedance → प्रतिबाधा (complex Z; distinct from प्रतिरोध and प्रतिघात).
- resistance → प्रतिरोध (real part of Z).
- reactance → प्रतिघात (imaginary part of Z).
- inductance → प्रेरकत्व.
- capacitance → धारिता.
- conductivity → चालकता (material S/m).
- admittance → एडमिटेंस (transliterated loanword, beyond intro-curricula
  vocabulary; distinct from प्रतिबाधा and from चालकता).
- load → लोड (LD-card impedance load, transliterated to avoid collision
  with आवेश/charge; never physical weight/burden).
- gain → लाभ (antenna directivity ratio, dB; an accepted homonym with the
  general Hindi word for "profit" - context disambiguates, topic 11/12;
  never used for amplifier amplification, प्रवर्धन).
- excitation → उत्तेजन (EM energy input/source, used as noun and adjective:
  "उत्तेजन प्रकार", "उत्तेजन कमांड", "उत्तेजन-पोर्ट"; never the emotional
  sense).
- feedpoint → फ़ीडपॉइंट.
- port → पोर्ट.
- radials (noun) → रेडियल (ground-plane radial wires; invariant plural,
  number carried by quantifier/context); radial (adjective) → रेडियल
  also - an accepted intra-domain homonym, context disambiguates.

### Ground and earth

- ground / ground plane → ग्राउंड, compounded as ग्राउंड प्लेन (RF
  electrical reference plane, GN/GD ground cards, type/conductivity/model
  senses; one term across every ground sub-use; never soil).
- earth (physical medium) → भूमि (terrain/noise-model earth, "below
  ground" geometry; distinct from ग्राउंड).
- ground wave → भूमि तरंग (propagation term; distinct from ग्राउंड).

### Geometry primitives

- wire → तार (thin conductor / GW element; never cable/cord, केबल).
- segment → kept verbatim, "Segment"/"segment" (conditional loanword,
  topic 4).
- patch → kept verbatim, "Patch"/"patch" (topic 4).
- tag → kept verbatim, "Tag"/"tag" (topic 4; never a UI label or a card).
- card → कार्ड in running prose ("NE कार्ड जोड़ें"); source English "Card"
  kept in a mnemonic-paired dialog-title parenthetical ("(GA card)"),
  matching source case (topic 6/18); register never cross-converted.
- kernel → कर्नेल (integral-equation/thin-wire kernel; never an OS kernel).
- cliff → चट्टान (two-medium ground-boundary type, named for a coastal
  cliff geometry; never a fracture/break).
- structure → संरचना (the antenna model geometry; never निर्माण/
  "construction").
- model → मॉडल (NEC model or noise-temperature model; one term, context
  disambiguates).
- geometry → ज्यामिति.
- crossed → क्रॉस किए गए (transmission-line conductors crossed/reversed;
  never cut/severed).

### Field, pattern, viewer

- field (EM) → क्षेत्र (near/total/E/H field); field (UI data/config) →
  फ़ील्ड - two distinct terms, never conflated.
- near field / far field → निकट क्षेत्र / दूर क्षेत्र (opposed pair).
- far-field contribution → दूर-क्षेत्र योगदान (per-direction; never the
  near-field animation sense).
- radiation → विकिरण.
- radiation pattern → विकिरण प्रतिरूप (plotted directional response; never
  a template/design; one term catalog-wide).
- gain pattern → लाभ प्रतिरूप (the gain radiation pattern; "प्रतिरूप" is
  the one word for "pattern" across both concepts, topic 22).
- polarization → ध्रुवण (antenna/wave field orientation).
- polarity → ध्रुवता (sign of a quantity; false friend of ध्रुवण, never
  shared).
- phase → फ़ेज; reference phase → संदर्भ फ़ेज.
- frequency → आवृत्ति.
- wave / wavelength → तरंग / तरंगदैर्घ्य (from दीर्घ → दैर्घ्य; always घ्य,
  never the misspelling तरंगदैर्ध्य).
- standing wave / traveling wave → स्थिर तरंग / प्रगामी तरंग (opposed pair).
- node / antinode → निस्पंद / प्रस्पंद (standing-wave zero/maximum; also
  carries the null/peak overlay sense).
- crest → तरंग-शीर्ष (instantaneous wave apex, comet-head; distinct from
  शिखर, a curve/step peak).
- magnitude → परिमाण (modulus of a quantity, |Z|, scalar; distinct from
  आयाम).
- amplitude → आयाम (oscillating-quantity peak; distinct from परिमाण).
- peak value → शिखर मान; peak magnitude → शिखर परिमाण - two distinct
  labels, never collapsed to one.
- instantaneous → तात्क्षणिक (projection mode; append "(φ=0)" only where
  the source carries it).
- Poynting vector → पॉयन्टिंग सदिश; solid angle → घन कोण.
- net gain → निवल लाभ (total-minus-mismatch gain; never "real-part gain").
- viewer → दर्शक (observation direction and/or the 3D view widget; never
  observer/speaker/preview).
- flow / flow direction → प्रवाह / प्रवाह दिशा (patch/current flow).
- total field → कुल क्षेत्र.

### Color, tone, animation subsystem

- color → रंग.
- color projection → रंग प्रक्षेपण (which quantity drives hue).
- hue → ह्यू (color-wheel angle; distinct from रंग and from रंग प्रक्षेपण).
- brightness → चमक (luminance channel).
- hue encoding / brightness encoding → ह्यू एन्कोडिंग / चमक एन्कोडिंग
  (distinct internal enums; neither collapses to रंग प्रक्षेपण).
- color scale → रंग स्केल (magnitude-to-color scale).
- scale family / color tone → स्केल फैमिली (the transfer-curve family
  Power/Log/Asinh/μ-law/Reinhard/Sigmoid/Identity; one concept, two source
  spellings, one term).
- palette / palette kind → पैलेट / पैलेट प्रकार (palette-layout enum;
  distinct from स्केल फैमिली and रंग प्रक्षेपण).
- ramp / gradient → ग्रेडिएंट (one term; "ramp" collapses onto this parent
  as a palette kind - a linear color strip).
- gamma → गामा (power-law exponent).
- knee → नी बिंदु (soft-knee bend point).
- softening → सॉफ़्टनिंग; compression → संपीडन (dynamic-range); contrast →
  कंट्रास्ट; dynamic range → डायनामिक रेंज (transliterated, no concise
  native equivalent in this domain).
- floor → न्यूनतम सीमा (minimum/lower clamp, brightness/dB floor; never a
  room floor).
- envelope → आवरण (magnitude/amplitude envelope).
- comet → कॉमेट (moving-crest overlay effect; never geometry/ज्यामिति -
  a known fuzzy-inheritance hazard, topic 20).
- overlay (noun) → ओवरले; overlay (verb) → ओवरले करना - distinct forms.
- animate / animation → एनिमेट करना / एनिमेशन.
- animated / static → एनिमेटेड / स्थिर (category-header adjectives, dynamic
  vs phase-invariant).
- projection → प्रक्षेपण (color or geometry projection; one term, context
  disambiguates).
- scale → verb स्केल करना, noun स्केल.
- wireframe → वायरफ़्रेम (fixed technical compound, not तार+फ्रेम).
- identity → आइडेंटिटी (no-op/passthrough transfer); unity (Smith-chart) →
  इकाई - distinct terms, never shared.
- sentinel → सेंटिनल (unreachable-case guard value).
- bins → बिन (discretization buckets).
- companding → कंपैंडिंग (bounded log curve, μ-law).
- tone mapping → टोन मैपिंग (photographic tone-map).

### Render and compute

- renderer → रेंडरर (drawing backend; never "render engine").
- shader → शेडर.
- allocation (memory) → आवंटन; managed allocator → प्रबंधित आवंटक.
- thread → थ्रेड (compute thread; no collision with तार/wire in Hindi,
  both remain distinct words).
- widget → विजेट.
- validation → सत्यापन (the validation-tree feature); verification
  (checks) → जाँच - two distinct terms, never conflated.
- batch mode → बैच मोड.
- fork (process) → kept verbatim, "fork" (topic 4).
- deadlock → डेडलॉक; notifier → नोटिफ़ायर.
- token → टोकन; operand → ऑपरेंड; operator → ऑपरेटर; arity → एरिटी
  (expression-parser terms).
- override → ओवरराइड करना (supersede a value, SY symbol); overwrite (if it
  occurs) → मिटाकर लिखना - distinct terms, never shared.
- swap → स्वैप (exchange).
- theme → थीम (UI/color theme; never विषय/"topic-subject").

### Metrics and miscellaneous

- noise / noise temperature → शोर / शोर तापमान (electronic/thermal noise;
  never acoustic racket).
- efficiency → दक्षता; interpolation → प्रक्षेप.
- mnemonic → मेमोनिक (a card's code descriptor; never ज्ञापन/"memo-note").
- degrees / deg → freestanding prose "डिग्री" translates; the parenthetical
  unit tag "(deg)" stays verbatim like other unit tags (topic 4).
- diameter → व्यास (canonical native term; one choice, no loanword
  variant used).
- reflect → three distinct senses, never merged: geometry mirror op →
  प्रतिबिंबित करना; behavioral "mirrors …" (a control tracking another) →
  अनुसरण करता है; physics reflection → परावर्तन.
- default(s) → डिफ़ॉल्ट (fallback value).
- normalize / normalized / normalization → सामान्यीकरण / सामान्यीकृत
  (translated, never transliterated as नॉर्मलाइज़्ड/नॉर्मलाइज़ेशन).
- minor (axis) → लघु; major (axis) → मुख्य (never माइनर).
- element (geometry-card dialog heading, Arc/Helical/Patch/Wire Element) →
  तत्व, consistently, never एलिमेंट/एलीमेंट/अवयव; a matrix entry is a
  distinct sense ("Element 1,1", "Admittance Matrix Elements") → अवयव.
- surface (fixed compound "surface patch") → सरफेस patch; "surface" alone
  translates fully: "Surface" → सतह, "Surface Subdivision" → सतह
  उप-विभाजन.

Established terms are reused catalog-wide; no synonym is coined for a
concept already mapped above. Add any newly encountered translated-literal
domain concept directly to this table with its term, sense, and guarded
hazard.

## 11. Disambiguation policy

- The correct technical sense is chosen for every ambiguous term; program
  context already disambiguates, so no qualifier absent from the source
  (eg "विद्युत") is appended.
- A qualifier is added only where the Hindi term would otherwise be
  genuinely ambiguous within the same string.
- Accepted intra-domain homonyms this catalog legitimately reuses: लाभ
  (gain, technical, vs the general "profit" sense - context disambiguates);
  रेडियल (radials, noun, vs radial, adjective).
- No locative or other homonym collision has been found; none is recorded.
- Gerund-vs-noun overload is resolved per pair, never collapsed: स्केल
  (noun) vs स्केल करना (verb); ओवरले (noun) vs ओवरले करना (verb); एनिमेशन
  (noun) vs एनिमेट करना (verb).

## 12. Cross-catalog consistency

- One term per concept, reused from the topic 10 table everywhere it
  recurs; no competing synonym is introduced at a new site.
- Loanword-vs-native decision is locked per concept in topic 10 (eg
  चालकता native, एडमिटेंस loanword); the canonical spelling given there is
  the only spelling used.
- Minority-outlier spellings (eg तरंगदैर्ध्य for wavelength) unify to the
  canonical form (तरंगदैर्घ्य).
- Consistency priority can outrank locale-form preference: "Segment",
  "Patch", "Tag" stay in Latin script even where a native gloss exists,
  because the established NEC2-catalog convention (topic 4) outranks the
  general translate-domain-lexicon preference (topic 13).
- Appendix false-friend pairs, each resolved to two distinct terms that
  never share a translation:
  - ध्रुवता (polarity) vs ध्रुवण (polarization).
  - परिमाण (magnitude) vs आयाम (amplitude).
  - शिखर मान (peak value) vs शिखर परिमाण (peak magnitude).
  - ग्राउंड (ground) vs भूमि (earth).
  - लोड (load) vs आवेश (charge).
  - लाभ (gain) vs प्रवर्धन (amplifier amplification) vs the unmarked
    "profit" sense of लाभ (accepted homonym, topic 11).
  - धारा (electrical current) vs वर्तमान (temporal "current/present").
  - आवेश (electrical charge) vs शुल्क (billing/fee, reserved, unused so
    far).
  - तार (wire) vs केबल (cable/cord) vs थ्रेड (compute thread).
  - विकिरण प्रतिरूप (radiation pattern) vs टेम्पलेट (template/design) vs
    दूर क्षेत्र (far-field region).
  - उत्तेजन (EM excitation) vs उत्साह (emotional excitement, reserved,
    unused so far).
  - निस्पंद/प्रस्पंद (node/antinode) vs शून्य (generic numeric null/zero).
  - स्केल फैमिली (scale family/color tone) vs ह्यू (hue) vs पैलेट प्रकार
    (palette kind) vs रंग प्रक्षेपण (color projection) - four distinct
    chroma concepts.
  - कॉमेट (comet overlay) vs ज्यामिति (geometry).
  - आइडेंटिटी (identity, no-op transfer) vs इकाई (unity, Smith-chart).
  - रेंडरर (drawing backend) vs रेंडर इंजन (render engine).
  - ओवरराइड करना (override) vs मिटाकर लिखना (overwrite).
  - दर्शक (viewer) vs प्रेक्षक (observer) vs वक्ता (speaker) vs
    पूर्वावलोकन (preview).
  - प्रतिबिंबित करना (reflect, mirror op) vs अनुसरण करता है (mirrors,
    behavioral tracking) vs परावर्तन (reflection, physics).
  - संरचना (structure) vs निर्माण (construction).
  - थीम (theme) vs विषय (topic/subject).
  - सत्यापन (validation) vs जाँच (verification).
  - निवल लाभ (net gain) vs वास्तविक-अंश लाभ (real-part gain).
  - शक्ति (electrical power) vs घात (Power scale-family name).

## 13. Priority ordering

- Precedence chain: correct meaning, then interface convention, then
  catalog consistency, then disambiguation, then locale numeral form.
- Explicit override recorded: catalog consistency with the established
  NEC2 mnemonic-paired convention outranks the general native-translation
  preference for "Segment"/"Patch"/"Tag" (topic 12).

## 14. Grammatical number

- Hindi translated literals distinguish singular and plural nouns, adjectives,
  participles, and finite verbs; each agreeing form follows its explicit or
  implied head.
- After an explicit literal count, use the established unmarked counted-noun
  form, eg `2 segment`; do not add an imported plural suffix.
- Invariant technical loanwords retain one form; number is carried by the
  count, determiner, or agreeing Hindi words.

## 15. Grammatical agreement

- Adjectives and participles agree in gender and number with the head
  noun, including a standalone label whose head noun is implied (eg
  "चयनित टैग" - चयनित agrees with the masculine टैग).
- A standalone label with no stated or inferable noun defaults to
  masculine agreement, matching the topic 8 default-gender convention.
- Reflexive particle अपना/स्वयं is retained wherever the source expresses a
  reflexive relation (eg "अपना पथ चुनें").
- Native masculine `-आ` nouns take the oblique `-े` form before a
  postposition; other native and invariant loanword declension classes keep
  their established Hindi forms.
- No partitive-after-count declension applies beyond the counted-noun rule
  in topic 14.

## 16. Morphological derivation

- Borrowed technical verbs form via [English stem] + करना: "रीसेट करें",
  "डिलीट करें", "स्केल करना", "एनिमेट करना"; never a direct Hindi
  conjugation of the English root.
- Verbal-noun formation for borrowed process nouns uses the transliterated
  English -ing form as an invariant noun (एनिमेशन, रेंडरिंग), not a
  native gerund coinage.
- Native-affix preference: use the established NCERT-standard native term
  where topic 10 lists one (eg धारा, not "करंट"); transliterate with the
  करना-support-verb pattern above only where no concise native term exists.

## 17. Preposition and sandhi selection

- Hindi uses postpositions (के लिए, से, में, पर), not prepositions;
  selection follows the governed noun's oblique case form (eg masculine
  -आ nouns take oblique -े before a postposition).
- Sanskrit-style sandhi (vowel/consonant fusion at compound junctures) is
  not applied in coined technical phrases; words stay in plain juncture
  for readability (eg "ग्राउंड प्लेन", not a fused sandhi form).
- No elision/contraction rule beyond standard oblique-case marking applies
  in this catalog's vocabulary.

## 18. Card/record-label register

- Fixed designator form in dialog/editor titles: source English "Card"
  kept verbatim inside a mnemonic-paired parenthetical, matching source
  case (eg "(GA card)", "(RP Card)").
- Running-prose form in messages: कार्ड, translated (eg "NE कार्ड जोड़ें",
  "कार्ड हटाएं").
- Generic-noun casing: लोअरकेस कार्ड in prose; no short-vs-long form split
  beyond the fixed-vs-prose register already stated.
- Each register stays internally consistent; a title-register instance is
  never rewritten into prose form or vice versa.

## 19. Multi-paragraph and whitespace fidelity

- Paragraph breaks in the source (blank-line vs single-line) mirror at the
  same positions in the Hindi translation.
- A trailing clause removed from the current source is dropped from the
  translation rather than carried forward from a stale prior version.
- Trailing newline and terminal punctuation carry mechanically from the
  source's structural markers, independent of the danda/period choice in
  topic 3.
- अर्थपूर्ण पंक्ति-विराम स्रोत के उन्हीं स्थानों पर रखें; स्रोत में अनुपस्थित
  दृश्य पंक्ति-लपेट अनुवाद में न जोड़ें।
- अनुमानित प्रदर्शन-सीमा के लिए अर्थ न काटें और अस्वाभाविक संक्षेप न बनाएं;
  सबसे छोटा पूर्ण और स्वाभाविक हिंदी रूप रखें।

## 20. Current-source fidelity

- Derive every translation from the current source literal and its supplied
  context; reuse an inherited translation only when its complete meaning
  matches both.
- Do not inherit prior wording for a changed grammatical head, sentence
  boundary, `gain pattern`, `power`, `validation`, or `verification`; resolve
  each from the current source sense and topics 3, 10, 12, and 15.
- Remove any trailing clause absent from the current source rather than
  preserving stale translated meaning.

## 21. Script hygiene

- Latin-Devanagari homoglyph substitution is not applicable because the
  scripts have no shared confusable letter class used by this vocabulary.
- Script hygiene is a zero-failure invariant: Hindi prose uses Devanagari;
  Latin characters occur only inside a topic 4 retained token or the topic 9
  mnemonic presentation whose literal spelling requires them.
- Translate every plain source-language prose word; preserve genuine
  identifiers, units, mnemonics, and other retained tokens exactly.

## 22. Rule-file scope hygiene

- केवल वे निर्णय रखें जो अनूदित अक्षरशः पाठ के शब्दों, Unicode वर्णों,
  विराम-चिह्नों, बड़े-छोटे अक्षरों, अंतःस्थापित टोकनों, स्मरणाक्षर चिह्नों
  या अर्थपूर्ण रिक्त-स्थान को बदल सकते हैं।
- कैटलॉग निरूपण, क्रमांकन, शीर्षलेख, ध्वज, सत्यापन कार्यप्रवाह, रेंडरिंग,
  फ़ॉन्ट, स्वचालित आकार-निर्माण, लेआउट, विजेट व्यवहार, रनटाइम स्वरूपण,
  छँटाई, खोज, प्रदर्शन आकार, लेखा इतिहास, पूर्णता स्थिति, समीक्षा मेटाडेटा,
  उद्गम और स्रोत-उद्धरण के नियम न रखें।
- प्रत्येक निर्णय को वर्तमान निर्देश के रूप में सीधे लिखें; कार्यान्वयन
  प्रक्रिया, ग्रंथ-सूची, सुधार इतिहास और वैकल्पिक नीति-स्रोत न जोड़ें।

## 23. Section-disjointness declaration

- Script mechanics (topic 1, 6) governs glyph/case/spacing only.
- Phrasing and structure (topics 5, 7, 9, 11, 13, 16, 17, 18, 19) governs
  sentence and label construction only.
- Address register (topic 8) governs pronoun and verb-conjugation
  formality only.
- These three axes are non-overlapping; a concept placed in one is never
  restated as a rule in another.

## 24. Developer/debug-string policy

- सभी उपयोगकर्ता-मुखी आदेश, लेबल, संवाद, सहायता-पाठ और त्रुटि संदेश हिंदी
  में अनूदित हों; कोई समीक्षा-प्राथमिकता उन्हें अनूदित छोड़ने की अनुमति नहीं
  देती।
- सूचनात्मक स्थिति और प्रगति संदेश हिंदी में संक्षिप्त, निर्वैयक्तिक और
  घोषणात्मक रूप में अनूदित हों।
- विकासकर्ता-मुखी निदान और निम्न-स्तरीय त्रुटि संदेश हिंदी में संक्षिप्त,
  तकनीकी रूप में अनूदित हों; किसी उपतंत्र में इससे भिन्न स्थापित समान-परिवार
  रूप मिले तो उसी परिवार का रूप लागू हो।
- प्रत्येक अनूदित परिवार में अंतःस्थापित पहचानकर्ता, फलन-नाम, विन्यास-कुंजी,
  प्रारूप-विनिर्देशक और topic 4 के संरक्षित टोकन अक्षरशः रहें। परिवार की
  समीक्षा-प्राथमिकता इस शून्य-विफलता टोकन नियम को नहीं बदलती।
- `BUG:` निदान-उपसर्ग "बग:" रूप में लिखा जाए और सामान्य विफलताओं के "त्रुटि:"
  उपसर्ग से अलग बना रहे; दोनों गंभीरताएँ एक ही शब्द कभी साझा न करें।
- पहचानकर्ता और मान का युग्म सटा हुआ रहे (जैसे `sweep_state=%d`); विशेषण पूरे
  युग्म से पहले आए और पहचानकर्ता तथा उसके `=` के बीच कभी न आए।
