# Norwegian (Bokmål, `no`) translation rules

These rules govern modern Norwegian Bokmål for Norway, written in Latin script under post-1981 orthography, for professional electrical and radio-frequency engineers, antenna designers, and amateur-radio operators.

## 1. Script and orthography

- Use the Latin alphabet with mandatory `æ`, `ø`, and `å`, including `Æ`, `Ø`, and `Å`; never strip diacritics or fold them to `ae`, `oe`, or `aa`.
- Use precomposed letters; combining forms are not applicable to standard Bokmål letters.
- Use straight ASCII apostrophe `'` for quoted retained tokens so their literal spelling remains stable; substitute no curly look-alike.
- Textual joiners and script-specific joining forms are not applicable to the Latin script used here.
- Write left to right; retained technical tokens follow the same direction and require no manual direction override.
- Preserve letter case under topic 6 because Bokmål distinguishes uppercase and lowercase.
- Follow modern Bokmål orthography for Norway; omit archaic and dialectal variants.
- Use one ordinary space between words and between native text and embedded numeric or retained tokens.
- Fuse compounds under Bokmål orthography (eg `Strømretning`); hyphenate before a capitalized proper element or when a loanword boundary would otherwise be ambiguous (eg `patch-strømmer`), and use separate words only where the established grammatical phrase requires them.

## 2. Numerals in literals

- Use Arabic digits for technical values in translated prose; Bokmål has no alternate digit set.
- Use decimal comma (`50,0`) and a space as the thousands separator (`10 000`) for numbers written in translated prose.
- Keep formulas, examples, fixed defaults, and named mathematical or standards constants in source form so their technical identity remains exact.
- Form ordinals and indices with a digit plus period (`1.`, `2.`); the period marks an ordinal or index, while the comma marks a decimal.

## 3. Punctuation and quotation

- Use straight single quotes `'...'` for inline retained tokens (eg `'%s'`) so their literal boundaries remain stable; use no guillemets in interface literals.
- Use Bokmål comma, question mark, and exclamation mark in prose.
- Use no space before a colon or semicolon and one space after it.
- Preserve source ellipsis and dash characters so punctuation-bearing technical labels remain consistent.
- Sentence terminator: full sentences in dialogs/confirmations take a period; short labels, buttons, and menu items omit it.
- Punctuation inside embedded technical runs (format strings, paths, mnemonics) stays in source form, never localized.

## 4. Never-translate tokens

Retain every listed token exactly, with zero translation or transliteration failures.

- Keep NEC2 card mnemonics `GW GA GH EX LD FR RP GE EN SP SM SC NE NH NT TL GN GD GC GX GR GS GM GF CP CM SY XQ EK KH PQ PT` verbatim so file identifiers round-trip.
- Keep unit symbols `Hz kHz MHz GHz dB dBi dBd Ω W K S/m deg %` verbatim so standard scientific notation remains exact.
- Keep figure-of-merit and parameter tokens `VSWR S11 S12 S21 S22 Z Z0 F/B G/T` verbatim so radio-frequency notation remains exact.
- Keep file extensions `.nec .csv .s1p .s2p .png` and every printf-style format specifier verbatim so filenames and substitutions remain exact.
- Keep embedded function names, variable names, identifiers, and configuration keys verbatim because they name code rather than prose.
- Keep physical and mathematical symbols `E H θ φ μ` verbatim because a translated or look-alike letter changes the notation.
- Keep `xnec2c NEC2 GSL OpenGL GTK Smith` verbatim as product, library, toolkit, and chart names.
- Keep `Asinh Reinhard Sigmoid μ-law` verbatim as named transfer functions; translate descriptive `Power` as `Potens` under topic 10.
- Keep geometry loanwords `Segment`, `Patch`, and `Tag`, and process term `fork`, verbatim; translate `wire` as `tråd` because it is not retained.
- Apply topic 1 directionality to every retained token.

## 5. Format-specifier integrity

- Every format specifier from the source is preserved, same set, none added or dropped.
- Default ordering matches source; positional reordering (`%1$s`) is used only where Norwegian word order genuinely requires it.
- The sentence restructures around fixed specifier positions rather than reordering specifiers.
- Numbers inside format specifiers are never localized.

## 6. Capitalization and title-case

- Use sentence case, not source-style title case: capitalize only the first word and proper nouns in menu items, dialog titles, and labels (eg `Jordparametere (GD-kort)`).
- Axis letters (X, Y, Z) keep source capitalization; no lowercase override.
- A lowercase math/coordinate variable (eg `x`, `θ`) stays lowercase regardless of surrounding sentence case.
- No coordinated-option-name casing exception applies; every element follows the sentence-case default.
- A generic card/record noun (`kort`) is lowercase mid-sentence, capitalized only as a title's first word; acronyms (VSWR, NEC2) and proper nouns retain their fixed casing.

## 7. Interface register by string type

Treat interface text as concise professional Bokmål; preserve all source meaning without copying source length or word order.

- Commands, buttons, and menu actions use an imperative verb, bare infinitive, or established noun phrase; omit the subject, place the verb before its object, and use the shortest natural complete form (eg `Velg...`).
- Field labels use a noun phrase in normal modifier-before-head order and retain a source colon (eg `Frekvens:`); omit subjects, verbs, and articles unless meaning requires them.
- Dialogs and confirmations use full declarative or interrogative sentences in subject-verb-object order; present the condition or affected object before the requested decision when clarity requires it.
- Tooltips use complete declarative clauses; name the control's effect first and state why an unavailable control is disabled when the source gives that reason.
- Status and error messages use terse impersonal declarative sentences; state the affected entity before its state or failure and retain the source's technical detail.
- User-visible domain entities use the locked topic-10 term as the head; place distinguishing modifiers before the head or form a fused compound under topic 1.
- Prefer natural compounds and complete short phrases; omit no meaning, invent no abbreviation, and impose no source-length limit.
- Developer and debug strings follow topic 24.

## 8. Formality and address

- Use neutral professional Bokmål; omit the archaic formal `De` form and avoid casual, slang, commercial, and archaic registers.
- Carry register through construction and word choice, not verb inflection; Bokmål verbs have no formal-address ending.
- Omit the subject in imperatives and infinitives; use full sentences in dialogs and confirmations under topic 7.
- Address the user only when the source does; then use second-person singular `du` and its forms without honorifics.
- Use declarative or interrogative confirmation sentences without a politeness formula.
- Avoid gender-marking the user; choose natural gender-neutral person terms and preserve grammatical gender and number required by the referenced noun.
- Personal-name order is given name followed by family name; retain a supplied name's own order and spelling.
- Honorifics are not applicable to this interface; add none absent from the source.

## 9. Accelerator/hotkey mnemonics

- GTK hotkey underscore (`_X`) marks the mnemonic letter directly before it.
- Per-container uniqueness: the marked letter must not collide with another mnemonic in the same menu/dialog; shift to a different letter of the Norwegian word on collision.
- Draw the mnemonic letter from the translated Bokmål term, never by transliterating the source term.
- Not applicable: Latin-script Bokmål needs no separate parenthetical mnemonic letter.
- Add a mnemonic only where the source literal carries one.
- Preserve mnemonic markers only in actionable button and menu literals; labels, tooltips, and status literals carry none unless present in the source.
- Avoid æ, ø, å and other hard-to-type letters as mnemonics where a plain Latin letter is available in the same word.

## 10. Domain lexicon

### Electrical primitives

| Concept | Norwegian | Intended sense and purpose / hazard |
|---|---|---|
| current | Strøm(mer) | never "nåværende" (temporal) |
| charge | Ladning(er) | never "kostnad"/"gebyr" (billing); distinct from `load` below |
| voltage | Spenning | electric potential |
| power (electrical) | Effekt | radiated/dissipated watts, power gain, power-flow; distinct from the `Power` scale-family name below |
| impedance | Impedans | complex Z; distinct from resistance and reactance |
| resistance | Resistans | real part of Z; distinct from impedance and `load` |
| reactance | Reaktans | imaginary part of Z |
| inductance | Induktans | inductive property measured in henries; keeps it distinct from reactance |
| capacitance | Kapasitans | capacitive property measured in farads; keeps it distinct from admittance |
| conductivity | Ledningsevne | native term throughout (`Jordledningsevne`, `Trådledningsevne`); never the loanword "konduktivitet" |
| admittance | Admittans | admittance-matrix sense; distinct from impedance |
| load | Belastning | LD-card impedance load, never physical weight; distinct from `Ladning` (charge) |
| gain | Forsterkning | antenna directivity ratio (dB); never "gevinst"/"profitt"; see topic 12 for the amplification collision |
| excitation | Eksitasjon / eksitasjonstype | RF energy input/source, never emotional excitement |
| feedpoint | Matepunkt | antenna feed point; established in freqplots port-selector strings |
| port | Port | excitation / S-parameter port |
| radials | Radialer | ground-plane radial wires (noun); distinct from the adjective `radial` |

### Ground and earth

| Concept | Norwegian | Intended sense and purpose / hazard |
|---|---|---|
| ground / ground plane | Jord / Jordplan | RF electrical reference plane, GN/GD ground cards, ground type/conductivity/effects/model; one term across every electrical-ground sub-sense |
| earth (physical medium) | Grunn | terrain and noise-model medium, including `under grunnivå`; keeps it distinct from electrical `Jord` |
| ground wave | Jordbølge | propagation mode along the ground; keeps it distinct from the `Jord` electrical reference despite the shared stem |
| sky (antenna-temp model) | Himmel | sky region in the antenna-temperature model; avoids adding an absent zone sense |

### Geometry primitives

| Concept | Norwegian | Intended sense and purpose / hazard |
|---|---|---|
| wire | Tråd | NEC2 GW element; never "kabel"/"snor"; homonym with `thread` (compute) - context disambiguates |
| segment | Segment | NEC2 geometry subdivision; kept as loanword |
| patch | Patch | NEC2 surface element; locks the retained geometry loanword and keeps it distinct from a generic area |
| tag | Tag | NEC2 geometry identifier; not a UI label or a card |
| card | Kort | NEC2 input record; register is topic 18 |
| kernel | Kjerne | integral-equation / thin-wire kernel; never an OS kernel sense |
| cliff | Klippe | two-medium ground-boundary type name; not a fracture/break sense |
| structure | Struktur | antenna model geometry; never "konstruksjon" |
| model | Modell | NEC model or noise-temperature model |
| geometry | Geometri | the model geometry |
| crossed | Krysset | transmission-line conductors crossed/reversed; not cut/severed |

### Field, pattern, viewer

| Concept | Norwegian | Intended sense and purpose / hazard |
|---|---|---|
| field (EM) | Felt | near/total/E/H field; homonym with a data/config field in Norwegian - program context disambiguates |
| near field / far field | Nærfelt / Fjernfelt | opposed spatial regions, kept symmetric |
| far-field contribution | Bidrag til fjernfelt | per-direction contribution; never confused with `Nærfelt` |
| radiation | Stråling | radiated emission |
| radiation pattern | Strålingsdiagram | plotted directional response; keeps it distinct from a template and from the far-field region |
| gain pattern | Forsterkningsdiagram | the gain radiation pattern |
| polarization | Polarisering / Polariseringsakse | antenna/wave field orientation; kept separate from `Polaritet` |
| polarity | Polaritet | sign (+/-) of a quantity; false friend of `Polarisering` |
| phase | Fase | angular phase of an oscillating quantity; keeps it distinct from polarity |
| reference phase | Referansefase | phase used as the comparison origin; keeps the reference sense explicit |
| frequency | Frekvens | oscillation rate; preserves the radio-frequency sense |
| wave / wavelength | Bølge / Bølgelengde | propagating oscillation / its spatial period; keeps the two concepts distinct |
| standing wave / traveling wave | Stående bølge / Vandrende bølge | opposed pair |
| node / antinode | Knutepunkt / Buk | standing-wave zero / maximum; also carries the null/peak overlay sense; distinct from a generic numeric null |
| crest | Bølgetopp | instantaneous wave apex (comet-head); distinct from a curve/step `Toppverdi` |
| magnitude | Størrelse | modulus of a quantity (\|Z\|, scalar); never "magnitud" (seismic/astronomical); homonymous with generic "quantity" - context disambiguates |
| amplitude | Amplitude | oscillating-quantity peak; distinct from `Størrelse` |
| peak value | Toppverdi | distinct UI option from `Toppstørrelse`; the two never collapse to one label |
| peak magnitude | Toppstørrelse | distinct UI option from `Toppverdi` |
| instantaneous | Øyeblikkelig | projection mode; add "(φ=0)" only where the source carries it |
| Poynting vector | Poynting-vektor | electromagnetic power-flow vector; preserves the named physical quantity |
| solid angle | Romvinkel | three-dimensional angular measure; keeps it distinct from a plane angle |
| net gain | Nettoforsterkning | total-minus-mismatch gain; never "reell forsterkning" (real-part gain) |
| viewer | Visning | observation direction or three-dimensional view; keeps it distinct from observer, speaker, and preview senses |
| flow / flow direction | Strømning / Strømningsretning | patch/current flow; distinct compound from `Strømretning` (current direction) |
| total field | Totalfelt | combined electromagnetic field; keeps it distinct from an individual field component |

### Color, tone, animation subsystem

| Concept | Norwegian | Intended sense and purpose / hazard |
|---|---|---|
| color | Farge | visible color concept; supplies the common head for color compounds |
| color projection | Fargeprojeksjon | which quantity drives hue |
| hue | Fargevinkel | color-wheel angle; keeps it distinct from `Fargetone` as the scale family |
| brightness | Lysstyrke | luminance channel |
| hue encoding | Fargevinkelkoding | distinct internal enum; renamed from prior `fargetonekoding` to free `Fargetone` for the scale-family concept |
| brightness encoding | Lysstyrkekoding | distinct internal enum; never collapses to "fargeprojeksjon" |
| color scale | Fargeskala | magnitude-to-color scale |
| scale family / color tone | Fargetone | the transfer-curve family (Potens/Log/Asinh/μ-law/Reinhard/Sigmoid/Identitet); one concept, two source spellings, one term |
| palette / palette kind | Palett / Palettype | color-set and its kind; keeps both distinct from `Fargetone` and `Fargeprojeksjon` |
| ramp / gradient | Rampe / Fargeovergang | a palette kind / linear color strip |
| gamma | Gamma | power-law exponent |
| knee | Kne | soft-knee bend point |
| softening | Oppmykning | dynamic-range softening |
| compression | Kompresjon | dynamic-range compression |
| contrast | Kontrast | separation between light and dark values; keeps it distinct from dynamic range |
| dynamic range | Dynamisk område | span between minimum and maximum represented levels; keeps it distinct from contrast |
| floor | Gulv | minimum/lower clamp (brightness/dB floor); context distinguishes from a literal room floor |
| envelope | Omhyllingskurve | magnitude/amplitude envelope |
| comet | Komet | moving-crest overlay effect; keeps it distinct from geometry |
| overlay (noun) | Overlegg | an added visual layer; distinct from the verb "å legge over" |
| animate / animation | Animere / Animasjon | action and process of time-varying presentation; keeps verb and noun forms consistent |
| animated / static | Animert / Statisk | time-varying versus phase-invariant category adjectives; keeps the opposed states distinct |
| projection | Projeksjon | color or geometry projection |
| scale | Skalere (verb) / Skala (noun) | resizing action / measurement mapping; keeps grammatical senses distinct |
| wireframe | Trådmodell | wire-mesh render mode |
| identity | Identitet | no-op/passthrough transfer; distinct from `Enhet` (unity, Smith-chart) |
| sentinel | Vaktverdi | unreachable-case guard value |
| bins | Intervaller | discretization buckets |
| companding | Kompandering | bounded log curve (μ-law) |
| tone mapping | Tonekartlegging | photographic tone-map |

### Render and compute

| Concept | Norwegian | Intended sense and purpose / hazard |
|---|---|---|
| renderer | Renderer | drawing backend; never "rendermotor" (render engine) |
| shader | Shader | kept as loanword |
| allocation (memory) / managed allocator | Allokering / (administrert) allokator | allocation and the allocator/report |
| thread | Prosess-tråd | compute thread; keeps it distinct from geometry `Tråd` (wire) |
| widget | Kontrollelement | user-interface element named in a literal; avoids retaining an unnecessary implementation loanword |
| validation | Validering | the validation-tree feature; distinct from `Verifisering` (checks) |
| batch mode | Batch-modus | noninteractive grouped operation; preserves the compute-mode sense |
| fork (process) | Fork | process creation operation; retains the locked technical loanword |
| deadlock | Fastlåsning | mutual waiting state; preserves the concurrency failure sense |
| notifier | Varsler | component that signals an event; keeps it distinct from a warning message |
| token / operand / operator / arity | Token / Operand / Operator / Aritet | expression-parser terms; `Token` kept as loanword, the rest translate natively |
| override | Overstyre | supersede a value (SY symbol); never "overskrive" (overwrite) |
| swap | Bytte | exchange |
| theme | Tema | UI/color theme; never "emne" (topic/subject) |

### Metrics and miscellaneous

| Concept | Norwegian | Intended sense and purpose / hazard |
|---|---|---|
| noise / noise temperature | Støy / Støytemperatur | electronic/thermal noise, never acoustic racket |
| efficiency | Virkningsgrad | output-to-input performance ratio; preserves the engineering sense |
| interpolation | Interpolasjon | estimation between sampled values; preserves the numerical-method sense |
| mnemonic | Mnemonikk | a card's code descriptor; not a memo/note |
| degrees / deg | Grader / (deg) | freestanding axis/prose "grader" vs the parenthetical unit tag "(deg)"; the tag is treated like other unit tags |
| diameter | Diameter | canonical loanword, locked over the native alternative "tverrmål" |
| reflect (geometry mirror) | Speil (imperativ) | geometry transform verb, parallels `Flytt`/`Skaler`; noun form `Speiling` only in `Speilingsalternativer` |
| reflect (behavioral "mirrors …") | Speiler | present-tense verb for a control tracking another (eg "Mirrors the Currents button" -> `Speiler Strømmer-knappen`) |
| reflect (physics) | Refleksjon | physics reflection coefficient/angle; distinct noun from `Speil`/`Speiler` |
| default(s) | Standardverdi(er) | fallback value |
| normalize / normalization | Normalisere / Normalisering | translated natively, not transliterated |

## 11. Disambiguation policy

- The correct technical sense is chosen for each ambiguous term (eg `Størrelse` for magnitude, never `magnitud`).
- No qualifier absent from the source is added; program context (NEC2 electromagnetic simulator) already disambiguates (eg "Strømmer" alone for "Currents", not "Elektriske strømmer").
- A qualifier is added only where the Norwegian term would otherwise be genuinely ambiguous.
- Accepted intra-domain homonym: `Størrelse` serves both "magnitude" and generic "quantity"; program context disambiguates.
- Accepted homonym: `Felt` serves both "electromagnetic field" and a generic data/config field; program context disambiguates.
- No additional locative homonym collision applies beyond the listed `Jord` and `Grunn` distinction.
- Gerund vs noun senses of "reflect" resolve to the three distinct `Speil` / `Speiler` / `Refleksjon` entries in topic 10.

## 12. Cross-catalog consistency

- Use one topic-10 term per concept and unify every outlier spelling to that canonical form.
- Keep these pairs distinct: `Polaritet`/`Polarisering`, `Størrelse`/`Amplitude`, `Toppverdi`/`Toppstørrelse`, `Jord`/`Grunn`, `Belastning`/`Ladning`, `Forsterkning`/`Signalforsterkning`/`Gevinst`, `Strøm`/`Nåværende`, `Ladning`/`Gebyr`, `Tråd`/`Kabel`, `Tråd`/`Prosess-tråd`, `Strålingsdiagram`/`Mal`/`Fjernfelt`, `Eksitasjon`/`Begeistring`, `Knutepunkt`/`Buk`/`Nullpunkt`, `Fargetone`/`Fargevinkel`/`Palettype`/`Fargeprojeksjon`, `Komet`/`Geometri`, `Identitet`/`Enhet`, `Renderer`/`Rendermotor`, `Overstyre`/`Overskrive`, `Visning`/`Observatør`/`Taler`/`Forhåndsvisning`, `Speil`/`Speiler`/`Refleksjon`, `Struktur`/`Konstruksjon`, `Tema`/`Emne`, `Validering`/`Verifisering`, `Nettoforsterkning`/`Realdelforsterkning`, and `Effekt`/`Potens`; never assign both sides of a contrast one translation.
- Keep `Patch`, `Segment`, `Tag`, `Fork`, `Renderer`, `Shader`, `Token`, and `Diameter` as canonical loanwords; keep `Ledningsevne`, `Grunn`, `Tråd`, `Overstyre`, and `Vaktverdi` as canonical native forms.
- Let catalog consistency outrank a locale-form preference only for the locked topic-10 terms, including `Patch` over a native alternative.

## 13. Priority ordering

- Precedence: correct meaning, then interface convention (topic 7), then catalog consistency (topic 12), then disambiguation (topic 11), then locale numeral form (topic 2).
- Explicit override: `Patch` stays the loanword over the native `flate*` alternative because catalog consistency outranks locale-form preference.
- Explicit override: `Effekt` is reserved for electrical power and `Potens` for the scale-family name, overriding the more common single-term instinct, because Appendix A's power/Power distinction outranks lexical economy.

## 14. Grammatical number

- Bokmål distinguishes singular and plural in nouns, adjectives, and participles; finite verbs do not inflect for number.
- Use singular after the literal count `1` and plural after other explicit literal counts.
- Choose definite or indefinite noun form from the sentence meaning rather than from source morphology.

## 15. Grammatical agreement

- Adjectives and participles agree in gender (common/neuter) and number with the head noun, including a standalone label whose head noun is implied (eg neuter "et felt" -> "aktivt felt"; common "en linje" -> "aktiv linje").
- No declension-class choice or partitive-after-count rule applies; Bokmål nouns do not decline for case.
- No reflexive-particle retention is required by any imperative/infinitive string in this catalog.
- Resolve a standalone label's gender from its implied head noun; when no head noun is recoverable, use common gender for a natural neutral label.

## 16. Morphological derivation

- Borrowed technical verbs take the standard `-ere` affix (eg `eksitere`, `interpolere`); ad hoc affixes are forbidden.
- Verbal-noun formation uses `-asjon`/`-ing` per the established form (eg `Eksitasjon`, not `eksitering`).
- Native-affix compounding is preferred over a loanword string where topic 10 already establishes a native term; loanwords are kept where topic 10 or topic 4 locks them.

## 17. Preposition and sandhi selection

- Not applicable: Bokmål has no context-conditioned preposition form, sandhi, elision, or obligatory contraction affecting these translated literals.

## 18. Card/record-label register

- Use the fixed designator `[mnemonic]-kort` in dialog and editor titles (eg `GD-kort`) so the record type remains identifiable.
- Use the same hyphenated `[mnemonic]-kort` form in running prose; Bokmål has no distinct suffixed prose form.
- Keep generic `kort` lowercase except as the first word of a title or sentence; use no separate short and long forms.
- Apply this one register consistently in titles and prose; no cross-register conversion is applicable.

## 19. Multi-paragraph and whitespace fidelity

- Mirror source paragraph breaks at the same positions and preserve whether each break is blank-line or single-line.
- Drop clauses absent from the current source rather than retaining inherited text.
- Preserve source trailing newlines and punctuation.
- Preserve semantic line breaks; add no line break solely for visual wrapping.
- Preserve complete meaning; never truncate text or use an unnatural abbreviation for an assumed display limit.

## 20. Current-source fidelity

- Derive every translation from the current source literal and its supplied context.
- Reuse inherited wording only when its complete meaning matches the current source.
- Do not inherit `magnitud` for `magnitude`, one term for both `polarity` and `polarization`, formal `De` address, `Komet` for `geometry`, or `fargetonekoding` for `hue encoding`; these forms violate topics 8 and 10.

## 21. Script hygiene

- Keep translated prose in the Latin letters defined by topic 1; foreign-script letters and Latin-look-alike homoglyphs have zero permitted occurrences outside retained tokens whose exact spelling requires them.
- Translate plain foreign words; retain only identifiers, units, proper names, and other tokens locked by topic 4.

## 22. Rule-file scope hygiene

- Keep only current decisions that can alter a translated literal's wording, Unicode characters, punctuation, capitalization, embedded tokens, mnemonic marker, or semantic whitespace.
- Omit catalog representation, serialization, headers, flags, workflow, rendering, fonts, shaping, layout, widget behavior, runtime formatting, sorting, search, display sizing, audit history, completion state, review metadata, provenance, and citations.
- State each decision directly; include no implementation procedure, bibliography, correction history, or alternative policy source.

## 23. Section-disjointness declaration

- Script mechanics (topic 1), structural/technical writing conventions (topics 2-7, 9-10), and address register (topic 8) are non-overlapping: topic 1 governs symbols and orthography, topics 2-7 and 9-10 govern phrasing and lexicon structure, topic 8 governs address/formality only; every concept lands in exactly one place.

## 24. Developer/debug-string policy

- Translate user-facing controls, dialogs, errors, and help text into Bokmål; review priority never permits an applicable user-facing literal to remain untranslated.
- Translate informational status, progress, and result strings into terse technical Bokmål.
- Translate developer-facing diagnostics into terse technical Bokmål; no subsystem family retains source-language prose.
- Preserve every embedded identifier, function name, format specifier, and topic-4 retained token verbatim in every family.
- Keep linguistic translation policy separate from token preservation; every translated family obeys topics 4 and 5.
- Render the `BUG:` diagnostic prefix as "PROGRAMFEIL:", held distinct from the "FEIL:" prefix that carries ordinary failures; the two severities never share a word.
- Keep an identifier-plus-value diagnostic pair contiguous (eg `sweep_state=%d`); a qualifier precedes the whole pair and never stands between the identifier and its `=`.
