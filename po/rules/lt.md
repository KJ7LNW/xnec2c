# lt translation rules

## 1. Script and orthography

- Scope: use contemporary standard Lithuanian for Lithuania, in Latin script, for professional electrical and radio-frequency engineers, antenna designers, and amateur-radio operators.
- Required letters beyond the base Latin set are ą, č, ę, ė, į, š, ų, ū, ž and their uppercase forms; preserve every required diacritic and never fold one to a look-alike base letter.
- Use precomposed Unicode Normalization Form C for Lithuanian letters; combining forms are forbidden where a precomposed letter exists, to keep literal spelling consistent.
- Lithuanian uses no orthographic apostrophe, modifier letter, joiner, or script-specific code-point distinction in these literals; not applicable.
- Write all text left-to-right; retained technical tokens keep the same direction.
- Lithuanian distinguishes letter case; apply topic 6 to labels, menu items, and titles.
- Separate words with one space; use one space between Lithuanian text and an embedded foreign, numeric, or unit token unless punctuation attaches directly.
- Form established compounds as one word; use a hyphen only in conventionally hyphenated forms and spaces only between separate words, never to imitate source-language compounds.

## 2. Numerals in literals

- Use Arabic digits for literal technical values in translated prose.
- Use a comma as the decimal separator and a space as the thousands separator in Lithuanian prose.
- Keep formulas, examples, fixed defaults, named mathematical constants, and standards constants in source form, including their digits and separators.
- Join a literal ordinal digit to its Lithuanian ending with a hyphen; treat an index delimiter as notation, not as a decimal comma.

## 3. Punctuation and quotation

- Use Lithuanian quotation marks „...“ for quoted prose; retain source quotation characters inside identifiers and technical tokens to preserve their literal form.
- Lithuanian comma, question mark, and exclamation mark use the same characters as the source; preserve the punctuation required by the translated sentence.
- Put no space before a colon, semicolon, or terminal punctuation mark and one space after it when text follows.
- Use three dots `...` for an ellipsis; preserve a source dash where it expresses the same relation.
- End full sentences with their required terminator; omit a terminator from short labels and fragments.
- Keep punctuation inside formulas, mnemonics, identifiers, and format specifiers in source form.

## 4. Never-translate tokens

Treat retained tokens as immutable: preserve every listed spelling, character, case, and internal punctuation; translate or transliterate none.

- Keep NEC2 card mnemonics verbatim: GW GA GH EX LD FR RP GE EN SP SM SC NE NH NT TL GN GD GC GX GR GS GM GF CP CM SY XQ EK KH PQ PT.
- Keep unit symbols verbatim, including Hz, kHz, MHz, GHz, dB, dBi, Ω, W, K, S/m, °, and %; translate a prose unit name such as `degrees` under topic 10.
- Keep figure-of-merit and parameter tokens verbatim: VSWR, S-parameter forms, Z, Z0, F/B, and G/T.
- Keep file extensions verbatim: .nec, .csv, .s1p, .s2p, .png.
- Keep every printf-style format specifier verbatim; topic 5 governs its position.
- Keep embedded function names, variable names, and configuration keys verbatim in developer strings.
- Keep physical and mathematical symbol letters verbatim, including E, H, φ, θ, and exponent letters.
- Keep product, library, toolkit, and chart names verbatim: xnec2c, GTK, Cairo, OpenGL, and Smith.
- Keep named transfer-function and algorithm terms verbatim: Log, Asinh, μ-law, Reinhard, Sigmoid, Identity; translate the descriptive `Power` family as `laipsninė`.
- Translate geometry concepts as `laidas`, `segmentas`, `lopinėlis`, and `žyma`; keep process `fork` verbatim. Apply each decision file-wide.
- Keep all retained tokens left-to-right under topic 1.

## 5. Format-specifier integrity

- Preserve exactly the source set of format specifiers; zero additions, omissions, substitutions, or malformed specifiers are allowed.
- Keep source order by default; use positional specifiers only when natural Lithuanian syntax requires reordered substitutions.
- Restructure Lithuanian wording around fixed specifier positions before changing their order.
- Never localize digits inside a format specifier.

## 6. Capitalization and title-case

- Use sentence case for labels, menu items, and titles: capitalize the first word and proper nouns only; never impose title case.
- Keep axis letters X, Y, and Z uppercase.
- Keep lowercase mathematical and coordinate variables lowercase, including φ and θ.
- Lithuanian coordinated option names take sentence case as a whole; capitalizing every element is not applicable.
- Apply topic 18 to the generic card or record noun by position.
- Preserve acronym and proper-name casing, including VSWR and NEC2.

## 7. Interface register by string type

Use concise standard interface Lithuanian; preserve all meaning and choose the shortest complete natural form rather than copying source length or word order.

- Commands, buttons, and menu actions use a subjectless infinitive with the action first and its object or qualifier after it; avoid imperatives and needless nominalization.
- Field labels use a sentence-case noun phrase in modifier-head order, omit a subject and verb, and retain a final colon when the source label has one.
- Dialog statements use full declarative sentences in neutral subject-verb-object order; confirmations use the question pattern in topic 8 and place the decision before supporting detail.
- Tooltips use full declarative sentences with the function first; when the source explains a disabled control, follow with the reason for its unavailability.
- Status and error messages use terse impersonal declarative clauses, place the result or failure first, and add the affected entity or cause after it.
- User-visible domain entities use the locked modifier-head names in topic 10; topic 8 governs personal names and address.
- Prefer established single-word derivations where natural; use multiword phrases where Lithuanian grammar or technical precision requires them, without unnatural abbreviations.
- Apply topic 24 to developer and debug strings.

## 8. Formality and address

- Lithuanian distinguishes informal singular and polite plural address; use polite plural verb morphology in dialogs and confirmations, with the pronoun omitted.
- Commands use the infinitive and status messages use impersonal clauses, so neither addresses the user directly.
- Carry neutral professional formality through syntax and word choice; prohibit informal singular imperatives and explicit `tu` address.
- Avoid gendered references to users; where grammatical gender is required for an implied technical head noun, agree with that noun and use masculine only as the unmarked fallback under topic 15.
- Use inclusive neutral role nouns and plural constructions where natural; preserve required gender, number, and animacy agreement.
- Honorifics are not used in interface literals; not applicable.
- Use second-person polite plural only in dialogs that address the user; use no first-person interface voice.
- Preserve a person's displayed name order from the source; Lithuanian interface policy adds no honorific or reordered personal name.
- Commands use infinitives, dialogs use neutral formal sentences, and status messages use impersonal declarations.
- Form confirmations as `Ar tikrai norite [infinitive]...?`.
- Prohibit casual, slang, over-formal, commercial, and archaic registers.

## 9. Accelerator/hotkey mnemonics

- Preserve an underscore mnemonic marker only when the source literal contains one; place it immediately before a typable letter in the translated term.
- Choose the mnemonic letter from the Lithuanian translation, never from a transliteration of the source term.
- A separate-script mnemonic presentation is not applicable because Lithuanian uses Latin script.
- Never add a mnemonic to a source literal without one and never remove one from a source literal that has one.
- Prefer a base Latin letter available in the translated term over a diacritic letter; use a diacritic letter only when needed to preserve a source mnemonic naturally.

## 10. Domain lexicon

Locked term table: concept - target term - sense - purpose/hazard guarded.

Electrical primitives
- `current` - srovė - electrical current - not "dabartinis" (temporal present).
- `charge` - krūvis - electrical charge - not "kaina/mokestis" (billing).
- `voltage` - įtampa - electric potential - standard native term.
- `power (electrical)` - galia - radiated/dissipated watts, power gain - distinct from `Power` scale-family (laipsninė) and math power-law.
- `impedance` - varža - complex Z - established catalog usage; distinct from resistance/reactance below.
- `resistance` - aktyvioji varža - real part of Z - shares root with impedance by design, stays distinct in context.
- `reactance` - reaktyvioji varža - imaginary part of Z - as above.
- `inductance` - induktyvumas - standard native term.
- `capacitance` - talpa - standard native term.
- `conductivity` - laidumas - material S/m - native term.
- `admittance` - laidis - admittance-matrix Y - distinct from varža/impedance.
- `load` - apkrova - LD-card impedance load - never "krūvis", reserved for charge.
- `gain` - stiprinimas - antenna directivity ratio - never "pelnas" (profit) or amplifier amplification.
- `excitation` - sužadinimas - EM energy input/source - never emotional excitement.
- `feedpoint` - maitinimo taškas - antenna feed point.
- `port` - prievadas - excitation/S-parameter port.
- `radials` (noun) - spinduliniai laidai - ground-plane radial wires - distinct from adjective "radialinis/spindulinis".

Ground and earth
- `ground` / `ground plane` - įžeminimas - RF electrical reference plane, GN/GD ground cards, all ground sub-senses - not soil; one term catalog-wide.
- `earth (physical medium)` - žemė - terrain/noise-model earth, "below ground" geometry - distinct from įžeminimas.
- `ground wave` - paviršinė banga - propagation term - distinct from the ground reference.

Geometry primitives
- `wire` - laidas - thin conductor/GW element - dominant established term across ~30 sites; "viela" was a minority outlier, unified to "laidas".
- `segment` - segmentas - NEC2 geometry subdivision - conditional loanword, kept standard.
- `patch` - lopinėlis - NEC2 surface patch (SP/SM).
- `tag` - žyma - NEC2 geometry identifier - not a UI label (etiketė) or a card (kortelė).
- `card` - kortelė - NEC2 input record - running-prose register in topic 18.
- `kernel` - branduolys - integral-equation/thin-wire kernel - not an OS kernel.
- `cliff` - skardis - two-medium ground-boundary type - not a fracture/break; newly logged, topic 22.
- `structure` - struktūra - the antenna model geometry - never "konstrukcija" (construction).
- `model` - modelis - NEC model or noise-temperature model.
- `geometry` - geometrija - the model geometry.
- `crossed` - sukryžiuoti - transmission-line conductors crossed/reversed - not "perkirsti" (cut/severed).

Field, pattern, viewer
- `field (EM)` - laukas - near/total/E/H field - distinct from a data/config field (laukelis).
- `near field` - artimasis laukas.
- `far field` / `far-field contribution` - tolimasis laukas / tolimojo lauko indėlis - symmetric with near field.
- `radiation` - spinduliuotė - radiated emission.
- `radiation pattern` - spinduliuotės diagrama - plotted directional response - not "šablonas" (template).
- `gain pattern` - stiprinimo diagrama - the gain radiation pattern.
- `polarization` - poliarizacija - antenna/wave field orientation - never "poliarumas" (reserved for polarity).
- `polarity` - poliarumas - sign of a quantity - never "poliarizacija".
- `phase` - fazė.
- `reference phase` - atskaitos fazė.
- `frequency` - dažnis.
- `wave` / `wavelength` - banga / bangos ilgis.
- `standing wave` / `traveling wave` - stovinti banga / bėgančioji banga - opposed pair.
- `node` / `antinode` - mazgas / pūkšnis - standing-wave zero/maximum; also the null/peak overlay sense; never "kraštinis taškas".
- `crest` - ketera - instantaneous wave apex (comet-head) - distinct from a curve/step peak (viršūnė).
- `magnitude` - dydis - modulus of a quantity - distinct from amplitude.
- `amplitude` - amplitudė - oscillating-quantity peak - distinct from magnitude.
- `peak value` - didžiausia reikšmė - distinct UI option from peak magnitude, never collapsed.
- `peak magnitude` - didžiausias dydis - as above.
- `instantaneous` - momentinis - projection mode; append "(φ=0)" only where source carries it.
- `Poynting vector` - Pointingo vektorius - proper-name term, declined natively.
- `solid angle` - erdvinis kampas.
- `net gain` - grynasis stiprinimas - total-minus-mismatch gain - not "realiosios dalies stiprinimas" (real-part gain).
- `viewer` - peržiūra - observation direction and the 3D view control - never `stebėtojas`, `kalbėtojas`, or `peržiūros vaizdas`.
- `flow` / `flow direction` - srautas / srauto kryptis - patch/current flow.
- `total field` - bendras laukas.

Color, tone, animation subsystem
- `color` - spalva.
- `color projection` - spalvų projekcija - which quantity drives hue.
- `hue` - atspalvis - color-wheel angle.
- `brightness` - ryškumas - luminance channel.
- `hue encoding` - atspalvio kodavimas - distinct enum, never collapsed to `spalvų projekcija`.
- `brightness encoding` - ryškumo kodavimas - distinct enum, never collapsed to `spalvų projekcija`.
- `color scale` - spalvų skalė - magnitude-to-color scale.
- `scale family` / `color tone` - tonų šeima - transfer-curve family, one concept and one term.
- `palette` / `palette kind` - paletė / paletės tipas - palette layout, distinct from scale family and color projection.
- `ramp` / `gradient` - gradientas - palette kind or linear color strip.
- `gamma` - gama - power-law exponent.
- `knee` - linkis - soft-knee bend point.
- `softening` - minkštinimas - dynamic-range softening.
- `compression` - suspaudimas - general dynamic-range compression.
- `companding` - glaudinimas - bounded logarithmic μ-law curve.
- `contrast` - kontrastas.
- `dynamic range` - dinaminis diapazonas.
- `floor` - riba - minimum or lower clamp, not a room floor.
- `envelope` - gaubtas - magnitude or amplitude envelope.
- `comet` - kometa - moving-crest overlay effect, never geometry.
- `overlay` (noun) - perdanga - added visual layer.
- `overlay` (verb) - uždėti - action distinct from the noun.
- `animate` / `animation` - animuoti / animacija.
- `animated` / `static` - animuotas / statinis - category adjectives that agree with their head noun.
- `projection` - projekcija - color or geometry projection.
- `scale` (noun, color) - skalė.
- `scale` (verb, resize) - keisti mastelį.
- `wireframe` - vielinis karkasas - idiomatic wire-mesh mode, distinct from geometry `laidas`.
- `identity` - tapatybė - no-op transfer, distinct from Smith-chart `vienetas`.
- `sentinel` - sarginė reikšmė - unreachable-case guard value.
- `bins` - intervalai - discretization buckets.
- `tone mapping` - tonų atvaizdavimas - photographic tone map, distinct from `tonų šeima`.

Render and compute
- `renderer` - atvaizdavimo posistemis - drawing backend, never `atvaizdavimo variklis`.
- `shader` - šeideris - naturalized technical loanword.
- `allocation (memory)` / `managed allocator` - paskirstymas / valdomas paskirstytuvas - memory allocation and its manager.
- `thread` - gija - compute thread, distinct from geometry `laidas`.
- `widget` - valdiklis - interface element.
- `validation` - validavimas - validation-tree feature, distinct from verification `patikrinimas`.
- `batch mode` - paketinis režimas.
- `fork (process)` - fork - retained process term.
- `deadlock` - aklavietė.
- `notifier` - pranešėjas.
- `token` / `operand` / `operator` / `arity` - leksema / operandas / operatorius / vietų skaičius - expression-parser terms.
- `override` - nustelbti - supersede a value, never overwrite `perrašyti`.
- `swap` - sukeisti - exchange.
- `theme` - tema - interface or color theme; context distinguishes the accepted homonym for topic or subject.

Metrics and miscellaneous
- `noise` / `noise temperature` - triukšmas / triukšmo temperatūra - electronic or thermal noise, not acoustic racket.
- `efficiency` - našumas.
- `interpolation` - interpoliacija.
- `mnemonic` - mnemonika - card code descriptor, not memo `pastaba`.
- `degrees` / `deg` - laipsniai / `(laipsn.)` - prose degree name versus parenthetical unit tag.
- `diameter` - skersmuo - canonical native term, never `diametras`.
- `reflect` - veidrodiniu būdu atspindėti / atkartoti / atspindėti - geometry mirror operation, behavioral tracking, and physical reflection remain distinct.
- `default(s)` - numatytoji reikšmė - fallback value.
- `normalize` / `normalization` - normalizuoti / normalizavimas - established Lithuanian technical derivation.

## 11. Disambiguation policy

- Choose the technical sense fixed in topic 10 for every ambiguous source concept.
- Add no qualifier absent from the source when program context already fixes the sense.
- Add a qualifier only when the Lithuanian target would otherwise remain genuinely ambiguous.
- Accept `tema` as the context-resolved homonym for both interface theme and topic or subject.
- Keep locative `žemė` distinct from electrical-reference `įžeminimas`.
- Translate noun `scale` as `skalė` and verb `scale` as `keisti mastelį`.

## 12. Cross-catalog consistency

- Use one locked term per concept from topic 10; introduce no competing synonym.
- Keep every false-friend set distinct: `poliarumas` / `poliarizacija`; `dydis` / `amplitudė`; `didžiausia reikšmė` / `didžiausias dydis`; `įžeminimas` / `žemė`; `apkrova` / `krūvis`; antenna `stiprinimas` / amplifier `stiprinimo koeficientas` / profit `pelnas`; `srovė` / `dabartinis`; `krūvis` / `mokestis`; `laidas` / `kabelis` / `gija`; `spinduliuotės diagrama` / `šablonas` / `tolimasis laukas`; `sužadinimas` / `susijaudinimas`; `mazgas` and `pūkšnis` / generic `nulis`; `tonų šeima` / `atspalvis` / `paletės tipas` / `spalvų projekcija`; `kometa` / `geometrija`; `tapatybė` / `vienetas`; `atvaizdavimo posistemis` / `atvaizdavimo variklis`; `nustelbti` / `perrašyti`; `peržiūra` / `stebėtojas` / `kalbėtojas` / `peržiūros vaizdas`; `veidrodiniu būdu atspindėti` / `atkartoti` / `atspindėti`; `struktūra` / `konstrukcija`; `tema` / subject sense `dalykas` where distinction is required; `validavimas` / `patikrinimas`; `grynasis stiprinimas` / `realiosios dalies stiprinimas`; electrical `galia` / transfer-family `laipsninė`.
- Prefer an established native derivation, including `skersmuo`; use a standardized loanword only for a locked technical term, including `segmentas`, `modelis`, `geometrija`, and `šeideris`.
- Use canonical `laidas` for the geometry conductor and eliminate outlier `viela` in that sense.
- Let catalog consistency outrank a locale-form preference for geometry `laidas` and the locked technical loanwords above.

## 13. Priority ordering

- Apply this precedence: correct technical meaning, interface convention, catalog consistency, disambiguation, then literal numeral form.
- Correct meaning overrides brevity for all false-friend sets in topic 12; catalog consistency overrides a regional preference for geometry `viela` in favor of `laidas`.

## 14. Grammatical number

- Lithuanian uses singular and plural noun, adjective, participle, and verb forms; make every agreeing word match the grammatical number required by the literal.
- After an explicit count ending in 1 but not 11, use nominative singular; after 2-9 outside 11-19, use nominative plural; after 0, 10-19, or a count ending in those forms, use genitive plural.
- Use plural verb agreement with a plural subject and singular agreement with a singular subject; impersonal status clauses remain impersonal.

## 15. Grammatical agreement

- Make adjectives and participles agree in gender, number, and case with the head noun, including an implied head in a standalone label.
- Category headers `Animated` and `Static` modify feminine plural `projekcijos`: use `Animuotos` and `Statinės`.
- Mode label `Instantaneous` implies masculine singular `vektorius` or `vaizdas`: use `Momentinis`, including `Momentinis (φ=0)` only where the source includes the qualifier.
- Apply standard count government from topic 14 to the noun and every agreeing modifier.
- Retain the reflexive particle in reflexive verbs, including the `-tis` ending.
- Use the gender of an explicit or established implied head noun; use masculine only when no head noun determines gender.

## 16. Morphological derivation

- Naturalize borrowed technical verbs with `-uoti` and form their verbal nouns with the established `-avimas` or `-imas` family; prohibit competing nonstandard suffixes.
- Form native verbal nouns with the established `-imas` or `-umas` suffix selected by the stem.
- Prefer an established native root and affix over a raw loanword; form compounds under topic 1.

## 17. Preposition and sandhi selection

- Lithuanian has no context-conditioned sandhi, elision, or obligatory contraction in these literals; not applicable.

## 18. Card/record-label register

- In dialog and editor titles, use the fixed designator form `[mnemonic] kortelė`, eg `GW kortelė`.
- In running prose, use the declined domain noun required by the sentence, eg `laido duomenys`; do not substitute the title designator.
- Write generic `kortelė` lowercase in running prose and capitalize it only when it begins a title.
- Keep title and running-prose forms internally consistent and never cross-convert them.

## 19. Multi-paragraph and whitespace fidelity

- Mirror every source paragraph break at the same position, preserving the distinction between a blank line and a single line break.
- Drop a trailing clause absent from the current source rather than retaining inherited text.
- Preserve source trailing newlines and terminal punctuation exactly where they carry literal meaning.
- Preserve semantic line breaks; add no line break solely for visual wrapping.
- Preserve complete meaning and natural wording; never truncate or abbreviate for an assumed display limit.

## 20. Current-source fidelity

- Derive every translation from the complete current source literal and its supplied context.
- Reuse an inherited translation only when its full meaning agrees with the current source.
- Treat inherited `viela` for the geometry conductor, stale trailing clauses, and a geometry sense inherited for `comet` as unsafe; replace them with the decisions in topics 10, 12, and 19.

## 21. Script hygiene

- Permit zero missing Lithuanian diacritics and zero foreign-script homoglyphs in translated prose; retained topic-4 tokens are the only exceptions when their literal spelling requires such characters.
- Translate ordinary foreign prose words; retain only genuine identifiers, symbols, units, proper names, and other topic-4 tokens.
- Lithuanian has no second native script requiring mixed-script adjacency rules; not applicable.

## 22. Rule-file scope hygiene

- Keep only current decisions that can alter wording, Unicode characters, punctuation, capitalization, embedded tokens, mnemonic markers, or semantic whitespace in a translated literal.
- Omit catalog representation, serialization, headers, flags, workflow, rendering, fonts, shaping, layout, widget behavior, runtime formatting, sorting, search, display sizing, audit history, completion state, review metadata, provenance, and citations.
- State each decision directly; include no implementation procedure, bibliography, correction history, or alternative policy source.

## 23. Section-disjointness declaration

- Topics 1 and 21 govern script mechanics; topics 2, 3, 5-7, 13, and 19 govern phrasing and literal structure; topic 8 governs address register. These axes do not overlap, and each concept belongs to one axis.

## 24. Developer/debug-string policy

- Translate all user-facing strings, including controls, dialogs, tooltips, status messages, and errors; review priority never permits leaving one untranslated.
- Translate informational diagnostics and developer-facing debug strings into terse, impersonal, technical Lithuanian; no subsystem family overrides this policy.
- Keep identifiers, function names, configuration keys, format specifiers, and all topic-4 retained tokens verbatim inside every translated string family.
- Treat user-facing, informational, and developer-facing families as distinct review priorities only; apply the same format-token and retained-token invariants to all three.
- Render the `BUG:` diagnostic prefix as "DEFEKTAS:", held distinct from the "KLAIDA:" prefix that carries ordinary failures; the two severities never share a word.
- Keep an identifier-plus-value diagnostic pair contiguous (eg `sweep_state=%d`); a qualifier precedes the whole pair and never stands between the identifier and its `=`.
