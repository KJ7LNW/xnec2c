# Tamil (ta) Translation Rules for xnec2c

## 1. Script and orthography

- Scope: modern standard Tamil for Tamil-literate professional electrical and RF engineers, antenna designers, and amateur-radio operators.
- Writing system: Tamil script (U+0B80-U+0BFF) only; no dual-script use.
- Combining vowel signs (Unicode standard Tamil block) apply normally; no precomposed/combining ambiguity in this script.
- ஃ (ayudha letter, U+0B83) is a distinct codepoint; never substitute an ASCII apostrophe or look-alike for it.
- No joining/shaping mechanics apply (non-cursive script); topic is not-applicable beyond the ஃ codepoint rule above.
- Directionality: strict LTR; bidi handling is not-applicable, never apply direction overrides.
- No letter-case distinction; casing rules (topic 6) reduce to mnemonic/loanword-spelling consistency only.
- Orthographic standard: modern standard Tamil (செந்தமிழ்); no competing reform-era variant in use.
- Inter-word spacing: standard space between words; one space between Tamil text and an embedded Latin/numeric token.
- Compound formation: native compounds fuse without hyphen by default (eg தரைத்தளம், not தரை-தளம்); hyphen reserved for joining a kept loanword/mnemonic to a native suffix (eg NEC-அட்டை).

## 2. Numerals in literals

- Digit set: Arabic numerals (0-9) for all technical/measurement values; Tamil numeral glyphs forbidden.
- Decimal separator: point (.) in numbers physically present in translated prose.
- Thousands/grouping separator: comma (,) when a grouped literal occurs; do not add grouping absent from the source.
- Formulas, examples, fixed defaults, and named mathematical or standards constants retain their source digits and separators to preserve exact technical notation.
- Ordinal/index notation: join a literal digit to the native suffix (eg 1வது); keep an index separator distinct from the decimal point.

## 3. Punctuation and quotation

- Quotation marks: retain source plain/straight quotes ("") for embedded technical tokens; no native Tamil quotation glyph introduced, for catalog consistency with technical content.
- Native punctuation replaces source counterparts in prose: comma (,), question mark (?), exclamation (!) render as in source (Western forms are standard in Tamil software UI); no separate Tamil-script punctuation substituted.
- Spacing: no space before colon/semicolon/terminal punctuation, one space after, matching source convention.
- Ellipsis: three dots (...); dash: preserved as in source (en dash for ranges, hyphen for compounds).
- Sentence terminator: period (.) for full sentences; short labels/fragments and menu items omit it.
- Punctuation inside an embedded technical run (a format specifier, unit, mnemonic) stays in source form, never localized.

## 4. Never-translate tokens

Resolve every Appendix B roster member; keep each verbatim, in source script/direction.

- NEC2 card mnemonics: GW GA GH EX LD FR RP GE EN SP SM SC NE NH NT TL GN GD GC GX GR GS GM GF CP CM SY XQ EK KH PQ PT.
- Unit symbols: MHz, dBi, Ω, dB, K (temperature), S/m (conductivity), deg/° (angle), % (percent).
- Figure-of-merit/parameter tokens: VSWR, S-parameters, Z, Z0, F/B ratio, gain/temperature tokens.
- File extensions: .nec, .csv, .s1p, .s2p, .png, .gplot.
- Format specifiers: %s %d %f %c %% (topic 5).
- Embedded identifiers: function names, variable names, config keys inside developer strings (topic 24).
- Physical/mathematical symbol letters: E, H (field), θ, φ (angle/exponent) - never substitute a Tamil letter for these.
- Product/library/tool names: xnec2c, GTK, Cairo, OpenGL, gnuplot.
- Named transfer-function/algorithm terms: retain Log, Asinh, μ-law, Reinhard, and Sigmoid verbatim; translate the descriptive Power family as அடுக்குவிதி to separate it from electrical வலு.
- Conditional loanwords, decided file-wide: translate segment/patch/tag as பிரிவு/தொகுதி/குறிச்சொல்; retain process token fork exactly as fork when it names the operation.
- Every retained token above remains byte-for-byte unchanged; translate plain prose words that are not identifiers, symbols, units, file suffixes, or proper technical names.

## 5. Format-specifier integrity

- Every specifier from the source carries through unchanged, same set: %s %d %f %c %%.
- Default order matches source order; positional reordering (%1$s) allowed only where the source itself uses positional specifiers.
- Tamil (SOV) sentences restructure around fixed specifier positions rather than reordering the specifiers themselves.
- Numbers inside specifiers are never localized (topic 2 exceptions apply here).

## 6. Capitalization and title-case

- Tamil has no letter-case distinction; labels, menu items, and titles use normal Tamil spelling rather than source title-case patterns.
- Kept Latin tokens retain source casing exactly (eg MHz, not Mhz) to preserve their standardized identity.
- Axis letters and physical or coordinate variables retain source casing; a lowercase variable stays lowercase.
- Coordinated option names receive no capitalization treatment; distinguish their elements through normal Tamil word boundaries.
- The generic card noun has no case distinction; retained acronyms and proper names preserve source casing.

## 7. Interface register by string type

Treat interface literals as Tamil engineering prose; use subject-object-verb order and place established domain modifiers before their head nouns.

- Commands, buttons, and menu actions use a polite subjectless imperative ending in -கவும்; use a deverbal noun only for an established action label, and omit bare stems.
- Field labels use a compact noun phrase in modifier-head order followed by the retained colon; they contain no subject or finite verb.
- Dialogs use full formal sentences in subject-object-verb order; omit the subject when context identifies it, and place the requested action or result before the final verb.
- Confirmations use a formal question with the affected object and action before the final interrogative verb; topic 8 fixes address and person.
- Tooltips use a complete declarative explanation in cause-then-effect order; when the source explains a disabled control, retain that reason.
- Status and error messages use subjectless impersonal declaratives; state the affected entity before its condition or result and omit first- and second-person pronouns.
- User-visible domain entities use the topic-10 term as the head noun with qualifying type, state, or subsystem before it; retained proper names remain in source order.
- Use the shortest complete natural form: prefer native compounds for stable concepts and multiword phrases where fusion would obscure meaning; omit no source meaning and invent no abbreviation.
- Developer and debug strings follow topic 24, not this topic.

## 8. Formality and address

- Tamil distinguishes formal and informal address; use formal or impersonal constructions and prohibit informal நீ forms to maintain professional register.
- Commands express formality through the -கவும் verb ending; dialogs use formal finite verbs, while labels and status text remain impersonal.
- Omit direct second-person pronouns in labels, tooltips, and status text; use நீங்கள் only when a confirmation must explicitly address the user.
- Confirmation dialogs use the formal pattern நீங்கள் ... விரும்புகிறீர்களா? with the action before the final interrogative verb.
- Avoid gender marking for users; use natural inanimate or neuter agreement for technical entities and plural-respect forms when directly addressing a person.
- Honorifics are not applicable to interface actions; do not add them absent a named person in the source.
- Personal names retain source order because the interface defines no Tamil-specific name-reordering context.
- Inclusive language uses role or function nouns rather than gendered person nouns; required grammatical class follows topic 15.
- Prohibit casual speech, slang, commercial tone, and archaic literary register; use modern formal technical Tamil.

## 9. Accelerator/hotkey mnemonics

- Preserve the source mnemonic marker only when present; append a parenthetical typable Latin letter representing a sound from the translated Tamil term, eg திற (_T).
- Derive the letter from the translated term's conventional Latin transliteration, never from the source term or an unrelated synonym, so the mnemonic identifies the visible label.
- Resolve a collision with another typable letter from the same translated term; do not alter the translation solely to obtain a mnemonic.
- Use the appended parenthetical form for this script; do not insert the marker into a Tamil grapheme.
- Add no mnemonic to an unmarked source string.
- Choose a readily typed, visually distinct Latin letter and preserve its marker punctuation exactly.

## 10. Domain lexicon

Locked target term per Appendix A concept; columns are concept, term, sense, purpose/hazard guarded.

| Concept | Tamil term | Sense | Purpose/hazard |
|---|---|---|---|
| current | மின்னோட்டம் | electrical current, A | not நடப்பு/தற்போதைய (temporal) |
| charge | மின்னூட்டம் | electrical charge, C | not கட்டணம் (billing) |
| voltage | மின்னழுத்தம் | electric potential | - |
| power (electrical) | வலு | radiated/dissipated W, gain, power-flow | distinct from திறன் (efficiency) and Power transfer-family name (see scale family) |
| impedance | இம்பீடன்ஸ் | complex Z | locked sole term; மின்தடை no longer used as an alt (that is resistance) |
| resistance | மின்தடை | real part of Z | distinct from impedance and load |
| reactance | ரியாக்டன்ஸ் | imaginary part of Z | distinct from resistance/impedance |
| inductance | இண்டக்டன்ஸ் | - | transliteration, no established native term |
| capacitance | கெப்பாசிட்டன்ஸ் | - | transliteration, consistent with inductance |
| conductivity | கடத்துத்திறன் | material S/m | native compound term |
| admittance | அட்மிட்டன்ஸ் | admittance matrix | distinct from impedance |
| load | சுமை | LD-card impedance load | accepted homonym with physical weight; engineering context fixes the impedance sense, and மின்னூட்டம் keeps charge distinct |
| gain | ஆதாயம் | antenna directivity ratio, dB | never இலாபம் (profit), never amplifier amplification |
| excitation | தூண்டல் | EM energy input/source | never உற்சாகம் (emotional excitement) |
| feedpoint | ஊட்டுப்புள்ளி | antenna feed point | - |
| port | போர்ட் | excitation/S-parameter port | transliteration |
| radials | ஆரக்கம்பிகள் | ground-plane radial wires, noun | distinct from adjective ஆரை-சார்ந்த (radial) |
| ground / ground plane | தரை / தரைத்தளம் | RF reference plane, GN/GD, ground type/conductivity/effects/model | one root term family across all sub-senses; not மண் (soil) |
| earth (physical medium) | பூமி | terrain/noise-model earth | distinct from electrical ground |
| ground wave | தரை அலை | propagation term | distinct from the ground reference |
| wire | கம்பி | thin conductor/GW element | not கேபிள் (cable/cord); not த்ரெட் (thread, compute) |
| segment | பிரிவு | NEC2 geometry subdivision | - |
| patch | தொகுதி | NEC2 surface patch, SP/SM | translated, not kept verbatim (topic 4) |
| tag | குறிச்சொல் | NEC2 geometry identifier | not a UI label or a card |
| card | அட்டை | NEC2 input record | register in topic 18 |
| kernel | கர்னல் | integral-equation/thin-wire kernel | not an OS kernel; context disambiguates |
| cliff | கிளிஃப் | two-medium ground-boundary type | not a fracture/break |
| structure | கட்டமைப்பு | antenna model geometry | not கட்டுமானம் (construction) |
| model | மாதிரி | NEC model / noise-temperature model | - |
| geometry | வடிவியல் | the model geometry | - |
| crossed | குறுக்கிடப்பட்ட | transmission-line conductors crossed/reversed | not cut/severed |
| field (EM) | புலம் | near/total/E/H field | UI/config field uses உள்ளீட்டுப் புலம் when a collision is genuinely possible |
| near field | அண்மைப் புலம் | opposed spatial region | symmetric with far field |
| far field | தூரப் புலம் | opposed spatial region | symmetric with near field |
| far-field contribution | தூரப்புலப் பங்களிப்பு | per-direction contribution | not near-field animation |
| radiation | கதிர்வீச்சு | radiated emission | - |
| radiation pattern | கதிர்வீச்சு வடிவம் | plotted directional response | locked sole term; not a template/design |
| gain pattern | ஆதாயக் கதிர்வீச்சு வடிவம் | the gain radiation pattern | - |
| polarization | முனைவு | antenna/wave field orientation | false friend of polarity |
| polarity | குறி | sign (+/-) of a quantity | false friend of polarization |
| phase | கட்டம் | - | - |
| reference phase | குறிப்புக் கட்டம் | - | - |
| frequency | அதிர்வெண் | - | - |
| wave / wavelength | அலை / அலைநீளம் | - | - |
| standing wave / traveling wave | நிலை அலை / பயண அலை | opposed pair | - |
| node / antinode | கணு / எதிர்க்கணு | standing-wave zero/maximum | also carries the null/peak overlay sense |
| crest | அலை உச்சி | instantaneous wave apex, comet-head | distinct from curve/step peak (உச்ச மதிப்பு) |
| magnitude | பருமன் | modulus of a quantity, scalar | distinct from amplitude |
| amplitude | வீச்சு | oscillating-quantity peak | distinct from magnitude |
| peak value | உச்ச மதிப்பு | UI option | distinct label from peak magnitude |
| peak magnitude | உச்ச பருமன் | UI option | distinct label from peak value |
| instantaneous | உடனடி | projection mode | append (φ=0) only where source carries it |
| Poynting vector | பாய்ண்டிங் வெக்டர் | - | transliteration, proper noun |
| solid angle | திண்ம கோணம் | - | - |
| net gain | நிகர ஆதாயம் | total-minus-mismatch gain | not "real-part gain" |
| viewer | பார்வையாளர் | observation direction / 3D view widget | not observer/speaker/preview |
| flow / flow direction | ஓட்டம் / ஓட்ட திசை | patch/current flow | - |
| total field | மொத்தப் புலம் | - | - |
| color | நிறம் | - | - |
| color projection | நிற ப்ரொஜெக்ஷன் | which quantity drives hue | transliteration, no established native term; distinct from hue/palette/scale family |
| hue | நிறத்தொனி | color-wheel angle | - |
| brightness | ஒளிர்வு | luminance channel | - |
| hue encoding | நிறத்தொனி குறியீடு | internal enum | distinct from color projection |
| brightness encoding | ஒளிர்வு குறியீடு | internal enum | distinct from color projection |
| color scale | நிற அளவீடு | magnitude-to-color scale | - |
| scale family / color tone | அளவீட்டுக் குடும்பம் | transfer-curve family | one concept for the two source spellings; Power member is அடுக்குவிதி, while Log/Asinh/μ-law/Reinhard/Sigmoid/Identity remain retained names |
| palette / palette kind | பாலெட் / பாலெட் வகை | palette-layout enum | distinct from scale family and color projection |
| ramp | சரிவு | a palette kind | - |
| gradient | சாய்வு | linear color strip | distinct from ramp |
| gamma | காமா | power-law exponent | transliteration |
| knee | மூட்டு புள்ளி | soft-knee bend point | - |
| softening | மென்மையாக்கல் | - | - |
| compression | சுருக்கம் | dynamic-range compression | - |
| contrast | மாறுபாடு | - | - |
| dynamic range | இயங்குவீச்சு | - | - |
| floor | அடிமட்டம் | minimum/lower clamp, brightness/dB floor | not a room floor |
| envelope | சூழ்வளைவு | magnitude/amplitude envelope | - |
| comet | வால்நட்சத்திரம் | moving-crest overlay effect | not geometry (fuzzy-inheritance hazard, topic 20) |
| overlay (noun) | மேலடுக்கு | an added visual layer | distinct from verb மேலிடுதல் (to overlay) |
| animate / animation | அசைவூட்டு / அசைவூட்டம் | - | - |
| animated | அசைவூட்டப்பட்ட | category-header adjective | dynamic vs static |
| static | நிலையான | category-header adjective, phase-invariant | opposed to animated |
| projection | ப்ரொஜெக்ஷன் | color or geometry projection | reuses color-projection transliteration |
| scale (verb) | அளவிடு | to scale | - |
| scale (noun) | அளவீடு | a scale | distinct from color scale (qualified form) |
| wireframe | கம்பி வலைப்பின்னல் | wire-mesh render mode | - |
| identity | அடையாளம் | no-op/passthrough transfer | distinct from unity (ஒருமை, Smith-chart) |
| sentinel | காவலர் மதிப்பு | unreachable-case guard value | - |
| bins | தொட்டிகள் | discretization buckets | - |
| companding | காம்பாண்டிங் | bounded log curve, μ-law | transliteration |
| tone mapping | தொனி மேப்பிங் | photographic tone-map | - |
| renderer | ரெண்டரர் | drawing backend | not "render engine" (Appendix C) |
| shader | ஷேடர் | - | transliteration |
| allocation (memory) | ஒதுக்கீடு | - | - |
| managed allocator | நிர்வகிக்கப்பட்ட ஒதுக்கீட்டாளர் | allocator/report | - |
| thread | த்ரெட் | compute thread | not கம்பி (wire); no collision in Tamil, stated explicitly |
| widget | விட்ஜெட் | UI element | transliteration |
| validation | செல்லுபடித்தன்மை சரிபார்ப்பு | the validation-tree feature | distinct from verification (உறுதிப்படுத்தல்) where the file separates them |
| batch mode | தொகுதி பயன்முறை | - | - |
| fork (process) | fork | process fork | retained verbatim as an operation token |
| deadlock | டெட்லாக் | - | transliteration |
| notifier | அறிவிப்பாளர் | - | - |
| token | டோக்கன் | expression-parser term | transliteration |
| operand | ஆபரண்ட் | expression-parser term | transliteration |
| operator | ஆபரேட்டர் | expression-parser term | transliteration |
| arity | ஆரிட்டி | expression-parser term | transliteration, niche |
| override | மேலெழுதல் | supersede a value, SY symbol | distinct from overwrite (மாற்றியெழுது, file/data replace) |
| swap | இடமாற்று | exchange | - |
| theme | தீம் | UI/color theme | not பொருள்/தலைப்பு (topic/subject) |
| noise | இரைச்சல் | electronic/thermal noise | not acoustic racket; context disambiguates |
| noise temperature | இரைச்சல் வெப்பநிலை | - | - |
| efficiency | திறன் | - | distinct from power (வலு) |
| interpolation | இடைச்செருகல் | - | - |
| mnemonic (card) | நினைவூட்டி | a card's code descriptor | distinct from accelerator letter (topic 9, முடுக்கு எழுத்து), not a memo/note |
| degrees (freestanding) | டிகிரி | axis/prose degrees | - |
| deg (unit tag) | (deg) | parenthetical unit tag | retained as a standard unit token |
| diameter | விட்டம் | - | native term, locked, no loanword alt |
| reflect (geometry) | கண்ணாடி பிரதிபலிப்பு | mirror operation | distinct from mirrors/reflection below |
| mirrors (behavioral) | பின்பற்றுகிறது | a control tracking another | distinct from geometry reflect and physics reflection |
| reflection (physics) | எதிரொளிப்பு | physics reflection | distinct from geometry reflect and mirrors |
| default(s) | இயல்புநிலை | fallback value | - |
| normalize / normalization | இயல்பாக்கு / இயல்பாக்கம் | - | translated, not transliterated; native term exists |

## 11. Disambiguation policy

- Correct technical sense chosen per the topic-10 table for every ambiguous term.
- No qualifier is added beyond the source; program context (EM simulator, RF-engineer audience) already disambiguates.
- A qualifier is added only where the target term would otherwise be genuinely ambiguous (eg field: புலம் vs உள்ளீட்டுப் புலம்).
- Accepted intra-domain reuse: none beyond field/UI-field above.
- Accepted non-domain homonym: சுமை can mean physical weight; use it for impedance load because NEC2 context fixes the sense, while மின்னூட்டம் keeps electrical charge distinct.
- Gerund vs noun senses (eg "scale", "animate") kept distinct per their topic-10 rows.

## 12. Cross-catalog consistency

- One term per concept; reuse the topic-10 table, never introduce a synonym for an already-mapped concept.
- Appendix C false-friend pairs, resolved to two distinct terms each:
  - polarity குறி vs polarization முனைவு.
  - magnitude பருமன் vs amplitude வீச்சு.
  - peak value உச்ச மதிப்பு vs peak magnitude உச்ச பருமன்.
  - ground தரை vs earth பூமி.
  - load சுமை vs charge மின்னூட்டம்.
  - gain ஆதாயம் vs amplification (பெருக்கம், unmapped in this catalog) vs profit இலாபம்.
  - current மின்னோட்டம் vs temporal "current/recent" நடப்பு.
  - charge மின்னூட்டம் vs billing/fee கட்டணம்.
  - wire கம்பி vs cable/cord கேபிள்; wire கம்பி vs thread த்ரெட்.
  - radiation pattern கதிர்வீச்சு வடிவம் vs template/design (unmapped, not used) vs far field தூரப் புலம்.
  - excitation தூண்டல் vs emotional excitement உற்சாகம்.
  - node/antinode கணு/எதிர்க்கணு vs a generic numeric null/zero (பூஜ்ஜியம், kept distinct by context).
  - scale family அளவீட்டுக் குடும்பம் vs hue நிறத்தொனி vs palette kind பாலெட் வகை vs color projection நிற ப்ரொஜெக்ஷன்.
  - comet வால்நட்சத்திரம் vs geometry வடிவியல்.
  - identity அடையாளம் vs unity ஒருமை.
  - renderer ரெண்டரர் vs render engine (not used).
  - override மேலெழுதல் vs overwrite மாற்றியெழுது.
  - viewer பார்வையாளர் vs observer/speaker/preview (not used).
  - reflect கண்ணாடி பிரதிபலிப்பு vs mirrors பின்பற்றுகிறது vs reflection எதிரொளிப்பு.
  - structure கட்டமைப்பு vs construction கட்டுமானம்.
  - theme தீம் vs topic/subject பொருள்/தலைப்பு.
  - validation செல்லுபடித்தன்மை சரிபார்ப்பு vs verification உறுதிப்படுத்தல்.
  - net gain நிகர ஆதாயம் vs real-part gain (unmapped, not used).
  - power (electrical) வலு vs Power transfer family அடுக்குவிதிக் குடும்பம்; never share a translation.
- Loanword-vs-native decision: transliterate a technical EE/CS term with no established native Tamil equivalent (impedance, thread, shader, gamma); use the native compound where one is standard (resistance, conductivity, diameter, geometry).
- Minority-outlier spellings unify to the topic-10 canonical spelling; no variant spelling admitted elsewhere in the catalog.
- Consistency priority outranks locale-form preference for impedance/resistance/reactance (topic 13 override).

## 13. Priority ordering

- Precedence: correct meaning, then interface convention, then catalog consistency, then disambiguation, then locale numeral form.
- Override ruling: catalog consistency overrides locale-form preference for impedance/resistance/reactance, locking three distinct terms (இம்பீடன்ஸ்/மின்தடை/ரியாக்டன்ஸ்) instead of the prior overlapping மின்தடை alt for impedance.

## 14. Grammatical number

- Tamil literals distinguish singular and plural nouns; use -கள் for countable technical nouns where natural and leave mass or established compound nouns unmarked.
- After an explicit literal count, use the natural counted-noun form and do not duplicate plurality already expressed by the numeral.
- Adjectives and participles do not take a separate number ending for inanimate technical heads; finite verbs use the number form required by an explicit rational subject.

## 15. Grammatical agreement

- Tamil marks gender and number concord for rational nouns; inanimate technical nouns take neuter agreement and no adjective number inflection.
- A standalone label with an implied technical head resolves to the neuter/inanimate class and adds no agreement marker.
- After an explicit count, keep modifiers invariant and select verb agreement from the grammatical subject rather than the numeral alone.
- Rational person or role nouns take natural rational-class agreement without inventing gender absent from the source.
- Partitive-after-count and obligatory reflexive-particle agreement are not applicable to these translated literals.

## 16. Morphological derivation

- Borrowed technical verbs form with transliteration plus the auxiliary செய் ("do"), eg ரெண்டர் செய் ("to render"); a Tamil verbal suffix directly on the Latin transliteration is forbidden.
- Verbal-noun formation for native terms uses the standard -அல்/-தல் suffix (eg இயல்பாக்கம், "normalization").
- Native-affix compounds preferred where a standard Tamil term exists (topic 10); loanword-plus-செய் preferred for borrowed action verbs with no native equivalent.

## 17. Preposition and sandhi selection

- Standard Tamil sandhi (புணர்ச்சி) applies at native morpheme boundaries when a native affix attaches (eg தரை + தளம் → தரைத்தளம்).
- Sandhi is never applied across a kept transliterated/Latin-script token boundary; such a token stays separated by a plain space or hyphen (topic 1).
- No further elision/contraction rule needed beyond standard sandhi.

## 18. Card/record-label register

- Dialog and editor titles use the fixed designator mnemonic + அட்டை, eg GW அட்டை, to identify the NEC2 record compactly.
- Running prose inflects அட்டை with the case suffix required by the sentence, eg அட்டையின், while retaining the mnemonic as a separate uninflected token.
- Tamil has no generic-noun casing distinction and no separate long designator; title and prose forms differ only through required Tamil inflection.
- Keep title and running-prose forms within their own register so record names remain recognizable.

## 19. Multi-paragraph and whitespace fidelity

- Mirror every source paragraph break at the same position, preserving the distinction between a blank line and a single line break.
- Preserve semantic line breaks and add no visual wrapping absent from the source literal.
- Drop a trailing clause absent from the current source rather than retaining inherited wording.
- Carry trailing newlines and punctuation at the same boundary as the source.
- Preserve complete natural meaning; do not truncate or abbreviate for an assumed display limit.

## 20. Current-source fidelity

- Derive each Tamil literal from the complete current source text and supplied context.
- Reuse inherited wording only when its full meaning agrees with the current source; catalog precedent cannot override the current literal.
- Treat மின்தடை as unsafe inherited wording for impedance because it denotes resistance; use இம்பீடன்ஸ் for impedance.
- Treat வால்நட்சத்திரம் as unsafe inherited wording outside the comet animation overlay because it does not denote geometry.

## 21. Script hygiene

- Require zero substitutions among distinct Tamil consonants, including ர்/ற் and ல்/ள்/ழ்; each code point carries its own lexical value.
- Require zero Latin letters in Tamil prose except inside a retained topic-4 token or a topic-9 mnemonic.
- Translate a plain foreign prose word; retain it only when topic 4 classifies its exact spelling as an identifier, unit, suffix, symbol, product name, algorithm name, or operation token.
- Cross-script homoglyph handling beyond these Tamil/Latin boundaries is not applicable because the writing systems share no letters.

## 22. Rule-file scope hygiene

- Keep only decisions that can change wording, Unicode characters, punctuation, capitalization, embedded tokens, mnemonic markers, or semantic whitespace in a translated literal.
- Omit catalog representation, serialization, headers, flags, workflow, rendering, fonts, shaping, layout, widget behavior, runtime formatting, sorting, search, display sizing, audit history, completion state, review metadata, provenance, and citations.
- State only the current Tamil decision; include no implementation procedure, correction history, or alternative policy source.

## 23. Section-disjointness declaration

- Script mechanics in topics 1-5, 9, and 21 govern code points, digits, punctuation, retained tokens, specifiers, and mnemonic presentation only.
- Phrasing and structure in topics 6-7 and 10-20 govern casing consequences, interface syntax, terminology, ambiguity, consistency, morphology, and semantic whitespace only.
- Address register in topic 8 governs formality, person, honorifics, and inclusive reference only.
- Rule scope in topic 22 and string-family policy in topic 24 govern their named boundaries without redefining the three linguistic axes.
- Keep these sections non-overlapping so each concept has one authority.

## 24. Developer/debug-string policy

- Translate every user-facing action, dialog, tooltip, status, error, and validation message under topics 7-8; review priority never permits omission.
- Translate informational logs that communicate program state to users in the terse impersonal technical register from topic 7.
- Leave strings explicitly classified as developer-only diagnostics or debug traces in the source language; subsystem siblings follow the same classification.
- Retain identifiers, function names, config keys, format specifiers, units, and other topic-4 tokens verbatim inside every translated or untranslated family.
- Keep linguistic priority separate from token preservation: all translated families obey topics 4-5 regardless of review order.
- Render the `BUG:` diagnostic prefix as "வழு:", held distinct from the "பிழை:" prefix that carries ordinary failures; the two severities never share a word.
- Keep an identifier-plus-value diagnostic pair contiguous (eg `sweep_state=%d`); a qualifier precedes the whole pair and never stands between the identifier and its `=`.
