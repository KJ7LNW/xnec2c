# Afrikaans (South Africa) translation rules

These rules govern modern standard Afrikaans in Latin script for South African professional electrical and radio-frequency engineers, antenna designers, and amateur-radio operators.

## 1. Script and orthography

- Use Latin script and modern standard South African spelling under the Woordelys en Spelreëls; use no regional or historical variant.
- Preserve required diacritics ë, ï, ê, é, è, ô, û, ó, and á; never strip them or replace them with base-letter look-alikes.
- Use precomposed diacritic characters only; never use combining-diacritic sequences.
- Write the indefinite article `'n` with U+0027 APOSTROPHE; never substitute a curly quote or modifier letter.
- No script-specific joiner or positional-letter distinction applies.
- Write left-to-right; retained technical tokens remain left-to-right.
- Apply topic 6 because the script distinguishes letter case.
- Insert one space between words and between Afrikaans text and embedded technical or numeric tokens.
- Fuse compounds by default; hyphenate a retained token joined to Afrikaans text or where fusion creates an ambiguous vowel sequence; separate words only where standard syntax requires it.

## 2. Numerals in literals

- Use digits `0`-`9` for technical values in translated prose.
- Use a comma as the decimal separator and a space as the thousands separator in numbers physically present in translated prose, eg `50,00` and `10 000`.
- Preserve formulas, examples, fixed defaults, named mathematical constants, and standards constants exactly when they are retained literal tokens, including their source decimal point.
- No digit-plus-Afrikaans-affix ordinal or index form occurs; no index-separator rule applies.

## 3. Punctuation and quotation

- Native low-high quotation marks „…" for prose; retain straight source quotes around embedded technical tokens, for consistency with existing catalog usage.
- Comma, question mark, and exclamation mark follow standard Afrikaans prose form; no distinct opening-mark glyphs beyond the quotation pair above.
- No space before colon, semicolon, or terminal punctuation; one space after.
- Ellipsis is three dots "..."; dashes carry from source unchanged.
- Full stop terminates complete sentences; short labels, menu items, and fragments omit it.
- Punctuation inside embedded technical runs (format specifiers, code, unit expressions) stays in source form.

## 4. Never-translate tokens

Retain every token in this section exactly, with zero translation, transliteration, character substitution, or internal-spacing change.

- Retain NEC2 card mnemonics uppercase: `GW GA GH EX LD FR RP GE EN SP SM SC NE NH NT TL GN GD GC GX GR GS GM GF CP CM SY XQ EK KH PQ PT`.
- Retain standard unit symbols in their source form, including `Hz`, `MHz`, `dB`, `dBi`, `Ω`, `S/m`, `W`, `K`, `deg`, and `%`.
- Retain RF figure-of-merit and parameter tokens in their source form: `VSWR`, `S11` and other `S`-parameter labels, `Z`, `Z0`, front/back-ratio tokens, and gain/temperature tokens.
- Retain every file extension, literal filename, and topic-5 format specifier character-for-character.
- Retain embedded function names, variable names, configuration keys, and other code identifiers character-for-character.
- Retain physical and mathematical symbol letters in source form, including `E`, `H`, `φ`, and `θ`; never replace one with a visual look-alike.
- Retain `xnec2c` and each product, library, toolkit, and chart proper name in its official spelling.
- Retain named transfer functions and algorithms `Reinhard`, `Sigmoid`, `μ-law`, `Log`, `Asinh`, and `Identity`; translate descriptive `Power` as `Mag`.
- Apply one file-wide decision to conditional loanwords: `segment` → `Segment`, `patch` → `Patch`, `tag` → `Merker`, and process `fork` → `fork`.
- Translate geometry and domain terms not listed as retained tokens according to topic 10.
- Keep all retained tokens left-to-right under topic 1.

## 5. Format-specifier integrity

- Every source format specifier is preserved, same set, in the translation.
- Default order matches source order; positional specifiers (`%1$s`) are used only where Afrikaans word order requires reordering.
- Restructure the sentence around fixed specifier positions rather than reorder the specifiers themselves.
- Numbers inside specifiers are never localized (no comma substitution inside `%d`/`%f` output).

## 6. Capitalization and title-case

- Sentence case for labels, menu items, and dialog titles; no capitalize-every-noun convention.
- Proper nouns and NEC2 card mnemonics stay uppercase as in source, unaffected by sentence case.
- Axis letters (X, Y, Z) keep source casing; no forced case override.
- Lowercase math/coordinate variables in prose (eg phi, theta) stay lowercase.
- No coordinated-option-name capitalization exception occurs in this catalog.
- Generic card noun "kaart" is lowercase in running prose, sentence-case only at sentence start; acronyms and proper nouns retain source case.

## 7. Interface register by string type

Use the shortest complete natural Afrikaans form; preserve all meaning, prefer standard compounds, and never abbreviate or imitate source length.

- Commands, buttons, and menu actions use a terse imperative with the implied user omitted, the verb first, and the object after it; avoid infinitives and deverbal nouns, eg `Stoor`.
- Field labels use a noun phrase in modifier-before-head order and retain a source colon; use no subject or verb, eg `Frekwensie:`.
- Dialog statements use full declarative sentences in subject-verb-object order; confirmations use the topic-8 question pattern and present the decision before supporting detail.
- Tooltips use complete declarative sentences, lead with the control's effect, and state the source-supplied reason when a control is unavailable.
- Status and error messages use impersonal declarative wording, place the result or failure first, and follow it with the affected entity or cause; avoid direct address.
- Name user-visible domain entities with the topic-10 canonical noun as head and place descriptive modifiers before it; topic 8 governs personal and proper names.
- Topic 24 alone governs developer and debug strings.

## 8. Formality and address

- Use neutral professional `jy` address where direct second-person address is required; never use formal `u` or casual slang.
- Omit direct address in commands, labels, tooltips, status messages, and errors through imperative or impersonal constructions; use `jy` in confirmations and dialogs that explicitly address the user.
- Use the confirmation pattern `Is jy seker jy wil ...?`.
- Afrikaans verbs do not mark formality; no formality-specific verb ending applies.
- Grammatical gender agreement is absent; avoid unnecessary gendered personal nouns and use natural inclusive role terms.
- Use second person only for the addressed user; use third person for named people.
- Use no honorific unless the source contains one; preserve a personal name in its source order and spelling.
- Prohibit over-formal, commercial, archaic, and slang registers.

## 9. Accelerator/hotkey mnemonics

- Preserve a source mnemonic with one underscore immediately before its letter inside the translated literal, eg `_Stoor`.
- Choose a mnemonic letter from the translated term, never from a transliteration of the source term.
- Keep mnemonic letters unique within their menu or dialog; on collision, use another distinctive letter from the same translated term.
- Because Afrikaans uses Latin script, add no separate parenthetical mnemonic letter.
- Add no mnemonic when the source literal has none.
- Use a directly typable unaccented letter; never select a diacritic letter.

## 10. Domain lexicon

Lock every concept below to its target term; reuse it everywhere, introduce no synonym.

### Electrical primitives

| Concept | Term | Sense | Purpose |
|---|---|---|---|
| current | Stroom | electrical current (A) | not temporal "current/recent" |
| charge | Lading | electrical charge (C) | not billing/fee/cargo |
| voltage | Spanning | electric potential | correct-sense and one-term consistency |
| power (electrical) | Krag | watts, power gain, power-flow | distinct from the Power transfer-family name "Mag" and from mathematical exponentiation |
| impedance | Impedansie | complex Z | distinct from resistance and reactance |
| resistance | Weerstand | real part of Z | distinct from impedance and load |
| reactance | Reaktansie | imaginary part of Z | correct-sense and one-term consistency |
| inductance | Induktansie | Appendix A technical sense | correct-sense and one-term consistency |
| capacitance | Kapasitansie | Appendix A technical sense | correct-sense and one-term consistency |
| conductivity | Geleidingsvermoë | material S/m | correct-sense and one-term consistency |
| admittance | Admittansie | admittance-matrix sense | distinct from impedance |
| load | Belasting | LD-card impedance load | not physical weight; false friend of charge, see topic 12 |
| gain | Aanwins | antenna directivity ratio (dB) | not profit, not amplifier amplification |
| excitation | Opwekking | EM energy input/source | not emotional excitement |
| feedpoint | Voedingspunt | antenna feed point | correct-sense and one-term consistency |
| port | Poort | excitation / S-parameter port | correct-sense and one-term consistency |
| radials | Radiale | ground-plane radial wires (noun) | distinct from the adjective "radiale" (radial) |

### Ground and earth

| Concept | Term | Sense | Purpose |
|---|---|---|---|
| ground / ground plane | Grond / Grondvlak | RF electrical reference plane | one term across all ground sub-uses; not soil |
| earth (physical medium) | Aarde | terrain/noise-model earth, "below ground" geometry | distinct from electrical ground |
| ground wave | Grondgolf | propagation term | distinct from the ground reference |

### Geometry primitives

| Concept | Term | Sense | Purpose |
|---|---|---|---|
| wire | Draad | thin conductor / GW element | not cable/cord; homonym with thread, see topic 11 |
| segment | Segment | NEC2 geometry subdivision | established loanword, held over any native alternative |
| patch | Patch | NEC2 surface patch (SP/SM) | kept untranslated, matches catalog precedent |
| tag | Merker | NEC2 geometry identifier | not a UI label or a card |
| card | Kaart | NEC2 input record | register at topic 18 |
| kernel | Kernel | integral-equation / thin-wire kernel | established loanword, not "Kern"; not an OS kernel |
| cliff | Krans | two-medium ground-boundary type | not a fracture/break |
| structure | Struktuur | the antenna model geometry | not "konstruksie" |
| model | Model | NEC model or noise-temperature model | correct-sense and one-term consistency |
| geometry | Geometrie | the model geometry | correct-sense and one-term consistency |
| crossed | Gekruis | transmission-line conductors crossed/reversed | not cut/severed |

### Field, pattern, viewer

| Concept | Term | Sense | Purpose |
|---|---|---|---|
| field (EM) | Veld | near/total/E/H field | distinct from a data/config field; context disambiguates |
| near field | Nabyeveld | correct-sense and one-term consistency | opposed pair with far field, kept symmetric |
| far field | Verreveld | correct-sense and one-term consistency | opposed pair with near field, kept symmetric |
| far-field contribution | Verreveld-bydrae | per-direction contribution | not near-field animation |
| radiation | Straling | radiated emission | correct-sense and one-term consistency |
| radiation pattern | Stralingspatroon | plotted directional response | not a template/design |
| gain pattern | Aanwinspatroon | the gain radiation pattern | correct-sense and one-term consistency |
| polarization | Polarisasie | antenna/wave field orientation | false friend of polarity, see topic 12 |
| polarity | Polariteit | sign (+/-) of a quantity | false friend of polarization |
| phase | Fase | Appendix A technical sense | correct-sense and one-term consistency |
| reference phase | Verwysingsfase | Appendix A technical sense | correct-sense and one-term consistency |
| frequency | Frekwensie | Appendix A technical sense | correct-sense and one-term consistency |
| wave | Golf | Appendix A technical sense | correct-sense and one-term consistency |
| wavelength | Golflengte | Appendix A technical sense | correct-sense and one-term consistency |
| standing wave | Staande golf | short form "Staande" in paired UI labels | opposed pair with traveling wave |
| traveling wave | Reisende golf | short form "Reisend" in paired UI labels | opposed pair with standing wave |
| node | Nodus | standing-wave zero | opposed pair with antinode |
| antinode | Antinodus | standing-wave maximum | opposed pair with node |
| crest | Kruin | instantaneous wave apex (comet-head) | distinct from a curve/step peak (Piek) |
| magnitude | Grootte | modulus of a quantity (\|Z\|, scalar) | distinct from amplitude |
| amplitude | Amplitude | oscillating-quantity peak | distinct from magnitude |
| peak value | Piekwaarde | distinct UI option from peak magnitude | never collapsed with peak magnitude |
| peak magnitude | Piekgrootte | distinct UI option from peak value | never collapsed with peak value |
| instantaneous | Oombliklik | projection mode | "(φ=0)" qualifier added only where the source carries it |
| Poynting vector | Poynting-vektor | correct-sense and one-term consistency | named quantity, surname kept verbatim |
| solid angle | Ruimtehoek | Appendix A technical sense | correct-sense and one-term consistency |
| net gain | Netto-aanwins | total-minus-mismatch gain | not "real (part) gain" |
| viewer | Kyker | observation direction and the 3D view widget | one term for both source uses; not observer, speaker, preview, or the generic view noun "Aansig" |
| flow / flow direction | Vloei / Vloeirigting | patch/current flow | correct-sense and one-term consistency |
| total field | Totale veld | Appendix A technical sense | correct-sense and one-term consistency |

### Color, tone, animation subsystem

| Concept | Term | Sense | Purpose |
|---|---|---|---|
| color | Kleur | Appendix A technical sense | correct-sense and one-term consistency |
| color projection | Kleurprojeksie | which quantity drives hue | distinct from hue, palette kind, scale family, see topic 12 |
| hue | Skakering | color-wheel angle | compounds as "Faseskakering" (Phase Hue), "kleurskakeringswiel" (hue wheel) |
| brightness | Helderheid | luminance channel | correct-sense and one-term consistency |
| hue encoding | Skakeringkodering | internal enum | distinct internal enum, never collapses to color projection |
| brightness encoding | Helderheidkodering | internal enum | distinct internal enum, never collapses to color projection |
| color scale | Kleurskaal | magnitude-to-color scale | correct-sense and one-term consistency |
| scale family / color tone | Kleurtoon | transfer-curve family (Mag/Log/Asinh/μ-law/Reinhard/Sigmoid/Identiteit) | one concept, two source spellings, one term; the Power family is "Mag", distinct from electrical "Krag"; "invalid color tone" → "ongeldige kleurtoon" |
| palette | Palet | Appendix A technical sense | correct-sense and one-term consistency |
| palette kind | Paletsoort | palette-layout enum | distinct from scale family and color projection |
| ramp / gradient | Kleurstrook | a palette kind / linear color strip | correct-sense and one-term consistency |
| gamma | Gamma | power-law exponent | established scientific term, kept |
| knee | Knie | soft-knee bend point | signal-processing sense |
| softening | Versagting | dynamic-range | correct-sense and one-term consistency |
| compression | Kompressie | dynamic-range | correct-sense and one-term consistency |
| contrast | Kontras | Appendix A technical sense | correct-sense and one-term consistency |
| dynamic range | Dinamiese bestek | Appendix A technical sense | correct-sense and one-term consistency |
| floor | Vloerwaarde | minimum/lower clamp (brightness/dB floor) | not a room floor |
| envelope | Omhulsel | magnitude/amplitude envelope | correct-sense and one-term consistency |
| comet | Komeet | moving-crest overlay effect | not geometry, known fuzzy-inheritance hazard, see topic 12/20 |
| overlay (noun) | Oorlegsel | an added visual layer | distinct from the verb "oorlê" |
| animate / animation | Animeer / Animasie | correct-sense and one-term consistency | irregular noun form "Animasie" kept as established exception |
| animated / static | Geanimeer / Staties | category-header adjectives | dynamic vs phase-invariant; no hotkey underscore, listbox/combo entries |
| projection | Projeksie | color or geometry projection | correct-sense and one-term consistency |
| scale | Skaal / Skaleer | noun "Skaal", verb "Skaleer" | context-dependent |
| wireframe | Draadraam | wire-mesh render mode | correct-sense and one-term consistency |
| identity | Identiteit | no-op/passthrough transfer | distinct from unity (Smith-chart) |
| sentinel | Wagwaarde | unreachable-case guard value | correct-sense and one-term consistency |
| bins | Vakke | discretization buckets | correct-sense and one-term consistency |
| companding | Kompandering | bounded log curve (μ-law) | established engineering loanword |
| tone mapping | Toonafbeelding | photographic tone-map | correct-sense and one-term consistency |

### Render and compute

| Concept | Term | Sense | Purpose |
|---|---|---|---|
| renderer | Tekenaar | drawing backend | not a "render engine"; eg "as-tekenaar" |
| shader | Skakeerder | Appendix A technical sense | correct-sense and one-term consistency |
| allocation (memory) | Toewysing | Appendix A technical sense | correct-sense and one-term consistency |
| managed allocator | Bestuurde toewyser | the allocator/report | correct-sense and one-term consistency |
| thread | Draad | compute thread | homonym with wire; program context disambiguates |
| widget | Legstuk | UI element | consistent across config_widget/callbacks/sy_overrides |
| validation | Validering | the validation-tree feature | distinct from verification (Verifikasie) |
| batch mode | Bondelmodus | Appendix A technical sense | correct-sense and one-term consistency |
| fork (process) | fork | correct-sense and one-term consistency | kept verbatim, conditional loanword per topic 4 |
| deadlock | Verstrikking | Appendix A technical sense | correct-sense and one-term consistency |
| notifier | Kennisgewer | Appendix A technical sense | correct-sense and one-term consistency |
| token | Simbool | expression-parser term | correct-sense and one-term consistency |
| operand | Operand | expression-parser term | correct-sense and one-term consistency |
| operator | Operator | expression-parser term | correct-sense and one-term consistency |
| arity | Ariteit | expression-parser term | correct-sense and one-term consistency |
| override | Oorskryf | supersede a value (SY symbol) | not overwrite (Vervang), see topic 12 |
| swap | Verwissel | exchange | correct-sense and one-term consistency |
| theme | Tema | UI/color theme | not "topic/subject" |

### Metrics and miscellaneous

| Concept | Term | Sense | Purpose |
|---|---|---|---|
| noise / noise temperature | Ruis / Ruistemperatuur | electronic/thermal noise | not "geraas" (acoustic racket); eg "Antennaruistemperatuur" |
| efficiency | Doeltreffendheid | Appendix A technical sense | correct-sense and one-term consistency |
| interpolation | Interpolasie | Appendix A technical sense | correct-sense and one-term consistency |
| mnemonic | Mnemoniek | a card's code descriptor | not a memo/note |
| degrees (freestanding, prose/axis title) | grade | eg "Rad Angle - deg" → "Stralingshoek - grade" | correct-sense and one-term consistency |
| deg (parenthetical unit tag) | deg | kept verbatim like other unit tags (m), (Hz) | eg "Initial Phi (deg)" → "Aanvanklike Phi (deg)" |
| diameter | Diameter | canonical loanword | never mixed with "Deursnee"; eg "Diameter Seg", "Diam. Taps", "Draaddiameter" |
| reflect | Spieël (geometry mirror) / Volg (mirrors, behavioral tracking) / Weerkaats (physics reflection) | three distinct senses | never collapsed into one term |
| default(s) | Verstek | fallback value | correct-sense and one-term consistency |
| normalize / normalization | Normaliseer / Normalisering | correct-sense and one-term consistency | translated, not transliterated |

### Extra concepts found in this catalog, beyond Appendix A

| Concept | Term | Sense | Purpose |
|---|---|---|---|
| pattern cut | Patroonsnit / Patroonsnitte | correct-sense and one-term consistency | single linking -s only, never double-s "Patroonssnitte" |
| dielectric | Diëlektries(e) | material property | correct-sense and one-term consistency |
| optimization | Optimalisering | process/setting | never the orphan "Optimering"; menu path label "Optimalisering Instellings" |
| optimizer | Optimeerder | agent noun (thread, tool) | distinct from optimization |

## 11. Disambiguation policy

- Each ambiguous source term resolves to its correct technical sense per the topic-10 table.
- No qualifier beyond what the source carries is added; program context already disambiguates (eg bare "Belasting" for load, no added "(electrical)").
- A qualifier is added only where the Afrikaans term would otherwise be genuinely ambiguous.
- Accepted intra-domain homonym: "Draad" serves both wire (conductor) and thread (compute); program context disambiguates.
- No locative homonym collision occurs in this catalog.
- Gerund vs noun senses stay distinct: "Animasie" (the animation, noun) vs "Animeer" (to animate, verb) are never interchanged.

## 12. Cross-catalog consistency

- One term per concept, reused from the topic-10 table everywhere it recurs; no synonym variation for a mapped concept.
- False-friend pairs, each resolved to two distinct terms, never sharing a translation:
  - polarity (Polariteit) vs polarization (Polarisasie).
  - magnitude (Grootte) vs amplitude (Amplitude).
  - peak value (Piekwaarde) vs peak magnitude (Piekgrootte).
  - ground (Grond) vs earth (Aarde).
  - load (Belasting) vs charge (Lading).
  - gain (Aanwins) vs amplifier amplification (Versterking) vs profit (Wins).
  - current, electrical (Stroom) vs current, temporal (Huidige).
  - charge, electrical (Lading) vs billing/fee (Heffing).
  - wire (Draad) vs cable/cord (Kabel); wire vs thread (both Draad, disambiguated by context per topic 11).
  - radiation pattern (Stralingspatroon) vs template/design (Sjabloon/Ontwerp) vs far field (Verreveld).
  - excitation (Opwekking) vs emotional excitement (Opgewondenheid).
  - node/antinode (Nodus/Antinodus) vs a generic numeric null/zero (Nul).
  - scale family/color tone (Kleurtoon) vs hue (Skakering) vs palette kind (Paletsoort) vs color projection (Kleurprojeksie) - four distinct terms, never collapsed.
  - comet (Komeet) vs geometry (Geometrie).
  - identity (Identiteit) vs unity, Smith-chart (Eenheid).
  - renderer (Tekenaar) vs render engine (Tekenenjin).
  - override (Oorskryf) vs overwrite (Vervang).
  - viewer (Kyker) vs observer (Waarnemer), speaker (Spreker), preview (Voorskou), and generic view (Aansig).
  - reflect (Spieël) vs mirrors, behavioral tracking (Volg) vs reflection, physics (Weerkaatsing).
  - structure (Struktuur) vs construction (Konstruksie, not used for this concept).
  - theme (Tema) vs topic/subject (Onderwerp, not used for this concept).
  - validation (Validering) vs verification (Verifikasie).
  - net gain (Netto-aanwins) vs real-part gain (Reëeldeel-aanwins).
  - power, electrical (Krag) vs the Power transfer-family name (Mag).
- Loanword-vs-native locks: Segment (not "Deel"), Patch (untranslated), Kernel (not "Kern"), Diameter (not "Deursnee").
- Minority-outlier spellings unify to the canonical form (eg any stray "Optimering" unifies to "Optimalisering").
- Consistency priority outranks locale-form preference for Segment, Patch, Kernel, and Diameter, held as established loanwords over any native alternative.

## 13. Priority ordering

- Precedence chain: correct meaning, then interface convention (topic 7), then catalog consistency (topic 12), then disambiguation (topic 11), then locale numeral form (topic 2).
- "Optimalisering" overrides the shorter "Optimering" for catalog consistency, even where the shorter form reads more naturally standalone.
- "Segment", "Patch", "Kernel", and "Diameter" override native-synonym locale preference for catalog consistency.

## 14. Grammatical number

- Afrikaans distinguishes singular and plural; use singular only for an explicit count of `1` and plural for every other explicit count.
- Form noun plurals with the standard lexical `-e` or `-s` pattern, eg `segment` → `segmente`; never invent a uniform suffix.
- Make attributive adjectives agree through their standard inflected form where required; participles follow the same attributive rule.
- Finite verbs do not change for grammatical number in modern Afrikaans.

## 15. Grammatical agreement

- Attributive adjectives take standard Afrikaans -e inflection before a noun (eg "die groot antenna"); apply the standard rule, no interface-specific exception.
- No grammatical gender exists in Afrikaans; gender concord is not applicable.
- No declension-class or partitive-after-count rule applies; Afrikaans has no case declension.
- No reflexive-particle retention is required beyond standard Afrikaans reflexive verbs, none of which occur among this catalog's established terms.
- A standalone label needs no default-gender resolution; not applicable.

## 16. Morphological derivation

- Borrowed technical verbs/nouns take the standard -eer suffix (eg "optimaliseer", "animeer", "interpoleer"); no other loanword verb-forming affix is used.
- Verbal-noun formation uses the -ing suffix on the -eer stem (eg "optimalisering"); "animasie" is kept as the established irregular exception.
- Native-affix derivation is preferred where an established native term exists (eg "toewysing" for allocation, not a loan calque); loanwords are retained where no native term is established (eg Segment, Patch, Kernel).
- Compounding fuses as one word per topic 1; this is the standard technical-noun compounding strategy, not a separate rule.

## 17. Preposition and sandhi selection

Not applicable: Afrikaans has no context-conditioned preposition-form selection or sandhi alternation relevant to this catalog. The fixed contraction "'n" is an orthographic form (topic 1), not a sandhi rule.

## 18. Card/record-label register

- Dialog/editor titles use the fixed designator form: lowercase "kaart" with the mnemonic prefixed (eg "GA-kaart").
- Running prose in messages uses the hyphenated form with the mnemonic (eg "GA-kaart lees").
- The generic noun "kaart" stays lowercase in both registers except at sentence start; no short-vs-long form distinction exists.
- Each register stays internally consistent; a dialog title never converts to the prose form or vice versa within one string.

## 19. Multi-paragraph and whitespace fidelity

- Preserve each source paragraph break at the same position and retain its blank-line or single-line form.
- Remove a trailing clause absent from the current source rather than retaining inherited wording.
- Preserve source trailing newlines and terminal punctuation exactly.
- Preserve semantic line breaks; add no line break for visual wrapping.
- Preserve complete meaning; never truncate or abbreviate wording for an assumed display constraint.

## 20. Current-source fidelity

- Derive every translation from the current source text and its supplied context.
- Reuse an inherited translation only when its complete meaning agrees with the current source.
- Treat inherited wording for changed format specifiers, changed word order, `Optimalisering`, `Patroonsnitte`, and `Komeet` as unsafe until re-derived from the current source.

## 21. Script hygiene

- Afrikaans prose uses only the Latin letters and diacritics defined in topic 1; foreign-script characters and Latin homoglyph substitutions have zero tolerance.
- Permit a foreign-script character only inside a retained token whose fixed spelling requires it.
- Translate plain foreign words; retain only genuine identifiers, units, symbols, and proper technical names under topic 4.

## 22. Rule-file scope hygiene

- Keep only current decisions that can alter wording, Unicode characters, punctuation, capitalization, embedded tokens, mnemonic markers, or semantic whitespace in a translated literal.
- Omit catalog representation, serialization, headers, flags, workflow, rendering, fonts, shaping, layout, widget behavior, runtime formatting, sorting, search, display sizing, audit history, completion state, review metadata, provenance, and source citations.
- State each decision directly; include no implementation procedure, bibliography, correction history, or alternative policy source.

## 23. Section-disjointness declaration

Topic 1 governs script mechanics, topic 7 governs phrasing and structure by interface string type, and topic 8 governs address register; these axes do not overlap, and each concept belongs to one axis.

## 24. Developer/debug-string policy

- Translate user-facing commands, labels, dialogs, tooltips, status messages, and errors; translate informational diagnostics that users can encounter; translate developer-facing diagnostics in terse technical Afrikaans.
- Review priority changes order only; it never permits an applicable user-facing or informational string to remain untranslated.
- Retain identifiers, function names, format specifiers, units, and other topic-4 tokens verbatim in every string family.
- Use the terse technical status/error register from topic 7 for developer-facing strings; no subsystem-specific sibling family overrides this rule.
- Render the `BUG:` diagnostic prefix as "PROGRAMFOUT:", held distinct from the "FOUT:" prefix that carries ordinary failures; the two severities never share a word.
- Keep an identifier-plus-value diagnostic pair contiguous (eg `sweep_state=%d`); a qualifier precedes the whole pair and never stands between the identifier and its `=`.
