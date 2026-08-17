# Danish translation rules

These rules govern Danish (`da_DK`) in Latin script under modern Danish orthography for professional electrical and radio-frequency engineers, antenna designers, and amateur-radio operators.

## 1. Script and orthography

- Use the Danish Latin alphabet with `æ`, `ø`, and `å` (`Æ`, `Ø`, `Å`) after `z`; preserve every required letter and diacritic without folding to a look-alike.
- Encode `æ`, `ø`, and `å` as precomposed Unicode characters in NFC; omit combining substitutes.
- Use U+0027 for an apostrophe where required; omit curly and modifier-letter look-alikes.
- Script-specific joining, positional forms, and textual joiners are not applicable.
- Write left to right; retained technical tokens remain left to right without mirroring or manual direction controls.
- Preserve letter case according to topic 6.
- Use modern post-1948 Danish orthography for Denmark.
- Separate words and adjacent numeric or retained tokens with one space unless Danish punctuation or compound rules require none.
- Form established compounds as one word; use a hyphen only where Danish orthography requires one around a retained token, and use separate words only for syntactic phrases.

## 2. Numerals in literals

- Use digits `0`-`9` for technical values written in translated prose.
- Use a comma as the decimal separator and a period as the grouping separator; use a thin space only where the source literal already requires grouped technical notation.
- Retain source spelling for formulas, examples, fixed defaults, and named mathematical or standards constants.
- Write an ordinal as a digit plus period; keep this index marker distinct from the decimal comma.

## 3. Punctuation and quotation

- Use straight double quotes `"..."` consistently for quoted interface text and embedded technical tokens.
- Use Danish comma, question-mark, and exclamation-mark forms, which share the source code points; add no opening punctuation.
- Insert no space before `%`, `:`, `;`, `?`, `!`, or terminal punctuation; retain one following space where a sentence continues.
- Use three periods for an ellipsis; preserve the source dash type where the dash carries range or clause meaning.
- End full sentences with a period; omit a terminator from short labels, buttons, and fragments.
- Preserve source punctuation inside format specifiers, identifiers, filenames, formulas, and other retained technical runs.

## 4. Never-translate tokens

Treat every token in this section as exact: retain its spelling, case, characters, and direction; neither translate nor transliterate it.

- Retain NEC2 card mnemonics: `GW GA GH EX LD FR RP GE EN SP SM SC NE NH NT TL GN GD GC GX GR GS GM GF CP CM SY XQ EK KH PQ PT`.
- Retain unit and figure-of-merit tokens: `Hz`, `kHz`, `MHz`, `GHz`, `dB`, `dBi`, `Ω`, `W`, `K`, `S/m`, `%`, `°`, `VSWR`, S-parameter tokens, impedance and reference-impedance symbols, front/back-ratio tokens, and gain/temperature tokens.
- Retain file extensions: `.nec`, `.csv`, `.s1p`, `.s2p`, `.png`.
- Retain every printf-style format specifier, including `%s`, `%d`, and positional forms such as `%1$s`.
- Retain function names, variable names, configuration keys, and other embedded identifiers in developer strings.
- Retain physical and mathematical symbol letters, including `E`, `H`, `θ`, `φ`, and coordinate-axis letters.
- Retain product, library, toolkit, and chart proper names, including `xnec2c`, `NEC2`, `GTK`, `Cairo`, `OpenGL`, and `Smith`.
- Retain named transfer functions and algorithms: `Log`, `Asinh`, `μ-law`, `Reinhard`, `Sigmoid`, and `Identity`; translate the descriptive `Power` family as `potens`.
- Retain the conditional loanwords `segment`, `patch`, `tag`, and process term `fork`; translate other ordinary geometry words according to topic 10.

## 5. Format-specifier integrity

- Preserve the exact source set of format specifiers with zero omissions, additions, type changes, or malformed tokens.
- Keep source order by default; use positional specifiers only where Danish syntax requires operand reordering and the source supports positional forms.
- Restructure Danish wording around fixed specifier positions where reordering is unavailable.
- Preserve every digit inside a format specifier exactly.

## 6. Capitalization and title-case

- Use sentence case for labels, buttons, menu items, and titles; omit title-case capitalization.
- Keep coordinate-axis letters `X`, `Y`, and `Z` uppercase.
- Keep lowercase mathematical and coordinate variables lowercase, including at sentence start.
- Coordinated option names requiring independent capitalization are not applicable.
- Write generic `kort` lowercase except at the start of a sentence or title; preserve acronym and proper-name casing.

## 7. Interface register by string type

Treat interface strings as concise technical Danish; preserve all source meaning without mechanical length matching or unnatural abbreviation.

- Write commands, buttons, and menu actions as subjectless imperatives with the verb first and the object after it.
- Write field labels as noun phrases in Danish modifier-before-head order and retain a source colon.
- Write dialogs and confirmations as complete impersonal sentences; present context before the requested decision and place the finite verb according to Danish main-clause or question order.
- Write tooltips as complete declarative clauses in cause-before-consequence order; retain the source reason for an unavailable control.
- Write status and error messages as impersonal declarative clauses; state the affected operation before its result where natural.
- Name domain entities with the locked topic-10 head term and ordinary Danish modifier-before-head compound order.
- Prefer an established fused compound to a calque-like multiword phrase; use the shortest complete natural form without dropping meaning.
- Apply topic 24 to developer and debug strings.

## 8. Formality and address

- Prefer subjectless commands and impersonal statements; use informal singular `du` only where direct address cannot be omitted, and never use formal `De`.
- Use neutral professional wording rather than gendered person nouns; where grammatical gender or number is required, agree with the referent without implying a person's gender.
- Honorifics are not applicable; omit titles of address unless they are part of a retained personal name.
- Use second person only under the direct-address rule above; use ordinary Danish given-name then family-name order when a personal name is written in prose.
- Danish verbs do not mark formality or person, so morphological formality is not applicable.
- Use the same neutral register for commands, dialogs, confirmations, tooltips, and messages.
- Phrase confirmations as neutral statements or impersonal questions, with `du` only when omission would make the sentence incomplete.
- Omit casual, slang, commercial, archaic, ceremonial, and over-formal register.

## 9. Accelerator/hotkey mnemonics

- Preserve a source mnemonic with underscore `_` immediately before a typable letter inside the translated term.
- Choose the mnemonic letter from the Danish term, not from a transliteration or the source term.
- Separate parenthetical mnemonic letters are not applicable to Danish Latin script.
- Add no mnemonic when the source literal has none.
- Prefer `a`-`z`; avoid `æ`, `ø`, and `å` where a readily typable letter from the same translated term exists.

## 10. Domain lexicon

| Concept | Danish term | Sense | Purpose/hazard |
|---|---|---|---|
| current (electrical) | strøm | electrical current, Amperes | not the temporal "present/recent" sense |
| charge (electrical) | ladning | electrical charge, Coulombs | not billing/fee/cargo |
| voltage | spænding | electric potential | canonical term for catalog consistency |
| power (electrical) | effekt | radiated/dissipated watts, power gain, power-flow | distinct from `Power` transfer-family name, below |
| power (transfer-family name) | potens | power-law scale/tone family | translates, unlike other named transfer functions; false-friend of `effekt` |
| impedance | impedans | complex Z | distinct from resistance/reactance |
| resistance | modstand | real part of Z | keep distinct from impedans and belastning |
| reactance | reaktans | imaginary part of Z | canonical term for catalog consistency |
| inductance | induktans | magnetic energy storage per current | canonical term for catalog consistency |
| capacitance | kapacitans | electric charge storage per voltage | canonical term for catalog consistency |
| conductivity | ledningsevne | material S/m | native term used in place of loanword |
| admittance | admittans | admittance-matrix sense | distinct from impedans |
| load | belastning | LD-card impedance load | not physical weight/burden; kept distinct from ladning |
| gain | forstærkning | antenna directivity ratio (dB) | not profit, not amplifier amplification |
| excitation | excitation | EM energy input/source | kept as loanword, catalog precedent; not emotional excitement |
| feedpoint | fødepunkt | antenna feed point | canonical term for catalog consistency |
| port | port | excitation/S-parameter port | accepted homonym with common Danish "port" (gate/harbor), disambiguated by context |
| radials | radialer | horizontal ground-plane radial wires (noun) | distinct from the adjective `radial` |
| ground / ground plane | jord / jordplan | RF electrical reference plane, GN/GD ground cards | not soil; one term across all ground sub-senses; kept distinct from earth |
| earth (physical medium) | jordbund | terrain/noise-model earth, "below ground" geometry | distinct from electrical `jord` |
| ground wave | jordbølge | propagation term | distinct from the ground reference |
| wire | tråd | thin conductor / GW element | not cable/cord; homonym with compute `tråd` (thread), context disambiguates |
| segment | segment | NEC2 geometry subdivision | kept, conditional loanword |
| patch | patch | NEC2 surface patch (SP/SM) | kept, conditional loanword |
| tag | tag | NEC2 geometry identifier | kept, conditional loanword; accepted homonym with common Danish "tag" (roof), card-context disambiguates |
| card | kort | NEC2 input record | register handled in topic 18 |
| kernel | kerne | integral-equation/thin-wire kernel | not an OS kernel; context disambiguates |
| cliff | klippe | two-medium ground-boundary type | not a fracture/break |
| structure | struktur | the antenna model geometry | not "konstruktion" |
| model | model | NEC model or noise-temperature model | canonical term for catalog consistency |
| geometry | geometri | the model geometry | canonical term for catalog consistency |
| crossed | krydset | transmission-line conductors crossed/reversed | not cut/severed |
| field (EM) | felt | near/total/E/H field | shares the word with a data/config field; context disambiguates |
| near field / far field | nærfelt / fjernfelt | opposed spatial regions | kept symmetric |
| far-field contribution | fjernfeltbidrag | per-direction contribution | not near-field animation |
| radiation | stråling | radiated emission | canonical term for catalog consistency |
| radiation pattern | strålingsdiagram | plotted directional response | not a template/design; do not use "strålingsmønster" |
| gain pattern | forstærkningsdiagram | the gain radiation pattern | canonical term for catalog consistency |
| polarization | polarisering | antenna/wave field orientation | kept distinct from polaritet |
| polarity | polaritet | sign (+/-) of a quantity | false friend of polarisering |
| phase | fase | angular phase of a periodic quantity | canonical term for catalog consistency |
| reference phase | referencefase | phase used as the angular reference | distinct from general phase |
| frequency | frekvens | oscillation cycles per unit time | canonical radio-frequency term |
| wave / wavelength | bølge / bølgelængde | propagating oscillation / its spatial period | keep quantity and length distinct |
| standing wave / traveling wave | stående bølge / vandrende bølge | opposed pair | canonical term for catalog consistency |
| node / antinode | knude / antiknude | standing-wave zero/maximum | also carries the null/peak overlay sense |
| crest | kam | instantaneous wave apex (comet-head) | distinct from a curve/step peak |
| magnitude | størrelse | modulus of a quantity (\|Z\|, scalar) | distinct from amplitude |
| amplitude | amplitude | oscillating-quantity peak | reserved for this sense only, e.g. color projection |
| peak value | spidsværdi | distinct UI option | must not collapse with spidsstørrelse |
| peak magnitude | spidsstørrelse | distinct UI option | must not collapse with spidsværdi |
| instantaneous | øjeblikkelig | projection mode | bare label; no "(φ=0)" suffix once msgid drops it, qualifier lives in tooltip strings |
| Poynting vector | Poynting-vektor | canonical term for catalog consistency | proper name `Poynting` kept, generic noun translated |
| solid angle | rumvinkel | three-dimensional angular measure | canonical electromagnetic geometry term |
| net gain | nettoforstærkning | total-minus-mismatch gain | not "real (part) gain" |
| viewer | visning / visnings- | observation direction and/or the 3D view widget | not observer/speaker/preview |
| flow / flow direction | strømning / strømningsretning | patch/current flow | one word, established |
| total field | totalfelt | combined electromagnetic field | distinct from a component field |
| color | farve | visible color as a display quantity | parent term for the color subsystem |
| color projection | farveprojektion | which quantity drives hue | canonical term for catalog consistency |
| hue | farvetone | color-wheel angle | canonical term for catalog consistency |
| brightness | lysstyrke | luminance channel | canonical term for catalog consistency |
| hue encoding / brightness encoding | farvetonekodning / lysstyrkekodning | distinct internal enums | neither collapses into farveprojektion |
| color scale | farveskala | magnitude-to-color scale | canonical term for catalog consistency |
| scale family / color tone | farvefamilie | transfer-curve family (Power/Log/Asinh/μ-law/Reinhard/Sigmoid/Identity) | one concept, two source spellings, one term |
| palette / palette kind | palet / palettetype | palette-layout enum | distinct from farvefamilie and farveprojektion |
| ramp / gradient | rampe / gradient | a palette kind / linear color strip | canonical term for catalog consistency |
| gamma | gamma | power-law exponent | kept, standard math term |
| knee | knæ | soft-knee bend point | canonical term for catalog consistency |
| softening | udblødning | dynamic-range softening | canonical term for catalog consistency |
| compression | kompression | dynamic-range compression | canonical term for catalog consistency |
| contrast | kontrast | separation between light and dark values | distinct from dynamic range |
| dynamic range | dynamikområde | span between minimum and maximum represented values | distinct from contrast |
| floor | bund | minimum/lower clamp (brightness/dB floor) | not a room floor |
| envelope | indhyldningskurve | magnitude/amplitude envelope | canonical term for catalog consistency |
| comet | komet | moving-crest overlay effect | not geometry |
| overlay (noun) | overlay | an added visual layer | distinct from the verb `lægge oven på` |
| animate / animation | animere / animation | vary a view over phase or time / that process | keep verb and noun forms distinct |
| animated / static | animeret / statisk | category-header adjectives | dynamic vs phase-invariant |
| projection | projektion | color or geometry projection | canonical term for catalog consistency |
| scale | skala (noun) / skalere (verb) | a value mapping / to resize or map | keep noun and verb senses distinct |
| wireframe | trådgitter | wire-mesh render mode | canonical term for catalog consistency |
| identity | identitet | no-op/passthrough transfer | distinct from unity (Smith-chart) `enhed` |
| sentinel | sentinelværdi | unreachable-case guard value | canonical term for catalog consistency |
| bins | bins | discretization buckets | kept, established technical loanword |
| companding | kompandering | bounded log curve (μ-law) | established Danish audio-engineering loanword |
| tone mapping | tonemapping | photographic tone-map | kept, established loanword |
| renderer | renderer | drawing backend | not "rendermotor" |
| shader | shader | canonical term for catalog consistency | kept |
| allocation (memory) / managed allocator | allokering / hukommelsesallokator | allocation and the allocator/report | canonical term for catalog consistency |
| thread | beregningstråd | compute thread | distinct from geometry `tråd` (wire) |
| widget | widget | UI element | kept, internal dev term |
| validation | validering | the validation-tree feature | kept distinct from verifikation (checks) |
| batch mode | batch-tilstand | noninteractive grouped processing mode | distinct from an ordinary collection |
| fork (process) | fork | process fork | kept verbatim |
| deadlock | deadlock | canonical term for catalog consistency | kept, established CS loanword |
| notifier | notifier | canonical term for catalog consistency | kept |
| token / operand / operator / arity | token / operand / operator / aritet | expression-parser terms | canonical term for catalog consistency |
| override | tilsidesætte | supersede a value (SY symbol) | not overwrite |
| swap | ombytte | exchange | canonical term for catalog consistency |
| theme | tema | UI/color theme | not "emne" (topic/subject) |
| noise / noise temperature | støj / støjtemperatur | electronic/thermal noise | not acoustic racket |
| efficiency | effektivitet | useful output divided by input | canonical engineering ratio term |
| interpolation | interpolation | estimation between known samples | canonical numerical-method term |
| mnemonic | mnemonic | a card's code descriptor | not a memo/note |
| degrees / deg | grader / "(grad)" | freestanding axis/prose vs parenthetical unit tag | tag treated like other unit tags |
| diameter | diameter | canonical loanword | one choice, no native synonym used |
| reflect | spejle (geometry) / følger (behavioral "mirrors …") / reflektere (physics) | three distinct senses | never collapsed |
| default(s) | standardværdi(er) | fallback value | canonical term for catalog consistency |
| normalize / normalization | normalisere / normalisering | canonical term for catalog consistency | translated, not transliterated |

## 11. Disambiguation policy

- The correct technical sense is chosen for each ambiguous term per the table in topic 10.
- No qualifier absent from the source is added; program context already disambiguates.
- A qualifier is added only where the Danish term would otherwise be genuinely ambiguous within that string.
- Accepted intra-domain homonym: `port` (excitation/S-parameter port) reuses the everyday Danish word for gate/harbor; context disambiguates.
- Accepted locative homonym: `tag` (NEC2 identifier) collides with the everyday Danish word for "roof"; card context disambiguates.
- Gerund vs noun senses of `scale` are resolved distinctly in topic 10 (`skalere` verb, `skala` noun).

## 12. Cross-catalog consistency

- One term per concept, reused from the topic-10 table across the whole catalog; no synonym introduced for an already-mapped concept.
- False-friend pairs (Appendix C), each resolved to two distinct terms that never share a translation:
  - `polaritet` (sign) vs `polarisering` (wave/antenna orientation).
  - `størrelse` (modulus/scalar) vs `amplitude` (oscillating peak).
  - `spidsværdi` (peak value) vs `spidsstørrelse` (peak magnitude).
  - `jord` (electrical reference) vs `jordbund` (physical terrain/medium).
  - `belastning` (impedance load) vs `ladning` (electrical charge).
  - `forstærkning` (directivity gain) vs amplifier amplification vs `fortjeneste` (profit).
  - `strøm` (electrical current) vs `nylig`/`aktuel` (temporal "current/recent").
  - `ladning` (electrical charge) vs `gebyr`/`fakturering` (billing/fee).
  - `tråd` (conductor) vs `kabel` (cable/cord) vs `beregningstråd` (compute thread).
  - `strålingsdiagram` (plotted response) vs `skabelon`/`design` (template/design); vs `fjernfelt` (far-field region).
  - `excitation` (EM input) vs `begejstring` (emotional excitement).
  - `knude`/`antiknude` (standing-wave) vs a generic numeric `nul` (null/zero).
  - `farvefamilie` vs `farvetone` vs `palettetype` vs `farveprojektion` — four distinct chroma concepts.
  - `komet` (overlay) vs `geometri` — known fuzzy-inheritance hazard.
  - `identitet` (no-op transfer) vs `enhed` (unity, Smith-chart).
  - `renderer` (backend) vs `rendermotor` (render engine, forbidden).
  - `tilsidesætte` (override) vs `overskrive` (overwrite).
  - `visning` (viewer/3D view) vs `observatør`/`taler`/`forhåndsvisning` (observer/speaker/preview).
  - `spejle` (mirror op) vs `følger` (behavioral tracking) vs `reflektere` (physics reflection).
  - `struktur` (model geometry) vs `konstruktion` (construction).
  - `tema` (UI theme) vs `emne` (topic/subject).
  - `validering` (tree feature) vs `verifikation` (checks).
  - `nettoforstærkning` vs real-part gain.
  - `effekt` (electrical watts) vs `potens` (`Power` transfer-family name).
- Loanword-vs-native decision, locked spellings: `excitation`, `port`, `komet`, `gamma`, `shader`, `renderer`, `widget`, `fork`, `deadlock`, `kompandering`, `tonemapping`, `bins` are kept as loanwords; no native replacement is substituted for these.
- Minority-outlier spellings unify to the canonical form: `strålingsmønster` is forbidden, unify to `strålingsdiagram`.
- Consistency priority outranks locale-form preference for `excitation`, kept as loanword over the available native `excitering` because catalog precedent already established the loanword.

## 13. Priority ordering

- Precedence chain: correct meaning, then Danish interface convention (sentence case, imperative), then catalog-wide lexicon consistency (reuse topic 10), then disambiguation only if context is insufficient, then locale numeral form.
- Explicit override rulings:
  - `excitation` kept as loanword: catalog consistency overrides the native-term preference.
  - `tag` kept short despite the locative homonym: meaning-correctness overrides the default disambiguation-qualifier addition.

## 14. Grammatical number

- Danish distinguishes singular and plural in nouns, adjectives, participles, and finite constructions where the predicate carries number.
- Use singular after the literal count `1`; use the established plural form after `0` and counts greater than `1`.
- Inflect the noun naturally after an explicit count; do not copy an uninflected source noun into Danish prose.

## 15. Grammatical agreement

- Adjectives and participles agree in gender (common/neuter) and number with the head noun, including a standalone label's implied head noun.
- A standalone technical label with no explicit head noun defaults to neuter agreement (`Nyt` not `Ny`), matching the default gender of abstract/technical Danish nouns in this catalog.
- Not applicable: Danish nouns carry no case declension and no partitive-after-count construction.
- Reflexive-particle `sig` is retained only where the verb is semantically reflexive (`opdaterer sig`), never added purely for register.

## 16. Morphological derivation

- Form borrowed technical verbs with the Danish infinitive suffix `-e` (`eksportere`, `rendere`); omit an untranslated source-language `-ing` gerund.
- Verbal-noun formation uses the native suffix `-ing`/`-else`/`-ning` (`beregning` for computation), except where the loanword itself is already an established catalog term (`rendering`, `kompandering`).
- Native-affix compounding is preferred over a new loanword unless an established loanword already covers the concept (topic 12); compounding strategy follows topic 1.

## 17. Preposition and sandhi selection

- Not applicable: Danish has no phonologically conditioned sandhi or context-conditioned preposition/form selection in this register; no elision/contraction rule is required.

## 18. Card/record-label register

- Dialog/editor titles use the fixed designator form: `kort` name plus mnemonic, e.g. `GW-kort`.
- Running-prose messages use the same hyphenated form as the title; Danish does not distinguish a separate suffixed prose form, one decision file-wide.
- Generic noun `kort` is lowercase mid-sentence, capitalized only at a sentence or title's start; no separate short-vs-long form exists.
- The register stays internally consistent; a title-form instance is never converted to a different running-prose form or vice versa.

## 19. Multi-paragraph and whitespace fidelity

- Mirror source paragraph breaks at the same positions, preserving the distinction between blank and single line breaks.
- Preserve semantic line breaks; add no line break for visual wrapping.
- Drop a trailing clause removed from the current source rather than retaining stale translated text.
- Preserve source trailing newlines and punctuation exactly.
- Preserve the complete meaning; never truncate wording or use an unnatural abbreviation for an assumed display limit.

## 20. Current-source fidelity

- Derive every translation from the complete current source literal and its supplied context.
- Reuse inherited wording only when its complete meaning agrees with the current source and context.
- Do not inherit `strålingsmønster` for `radiation pattern` or `nærfelt-animation` for `far-field contribution`; use the topic-10 terms.

## 21. Script hygiene

- Permit non-Danish letters and homoglyph-prone characters only inside a retained token whose exact spelling requires them; translated prose has zero wrong-script characters.
- Translate plain source-language words; retain only the identifiers, units, proper names, and locked loanwords defined in topics 4, 10, and 12.

## 22. Rule-file scope hygiene

- Keep only current decisions that can alter wording, Unicode characters, punctuation, capitalization, embedded tokens, mnemonic markers, or semantic whitespace in a translated literal.
- Omit catalog representation, serialization, headers, flags, workflow, rendering, fonts, shaping, layout, widget behavior, runtime formatting, sorting, search, display sizing, audit history, completion state, review metadata, provenance, and citations.
- State each decision directly; include no implementation procedure, bibliography, correction history, or alternative policy source.

## 23. Section-disjointness declaration

- Script mechanics (topics 1, 3, 9), phrasing/structure (topics 6, 7, 10, 11, 16, 18), and address register (topic 8) are non-overlapping axes; each concept is placed in exactly one section — formality only in topic 8, casing only in topic 6, term choice only in topic 10.

## 24. Developer/debug-string policy

- Translate user-facing commands, labels, dialogs, tooltips, status messages, and errors completely.
- Translate informational diagnostics and notices completely in concise professional Danish.
- Translate developer-facing debug and low-priority diagnostic strings in terse technical Danish; no subsystem family uses a source-language exception.
- Preserve embedded identifiers, function names, format specifiers, units, and topic-4 retained tokens in every string family.
- Use priority only to order review; it never permits an applicable user-facing, informational, or developer-facing literal to remain untranslated.
- Render the `BUG:` diagnostic prefix as "PROGRAMFEJL:", held distinct from the "FEJL:" prefix that carries ordinary failures; the two severities never share a word.
- Keep an identifier-plus-value diagnostic pair contiguous (eg `sweep_state=%d`); a qualifier precedes the whole pair and never stands between the identifier and its `=`.
