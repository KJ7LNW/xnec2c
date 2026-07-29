# nl translation rules

These rules govern standard Dutch for the Netherlands, written in Latin script under the current official spelling, for professional electrical and radio-frequency engineers, antenna designers, and amateur-radio operators using electromagnetic simulation software.

## 1. Script and orthography

- Use the bicameral Latin script so translated literals follow the locale's required writing system.
- Diacritics: trema (ë, ï, ü) marks a separately pronounced vowel in a digraph (eg "geïnstalleerd"); mandatory, never stripped or folded to a plain vowel.
- Precomposed Unicode form for trema letters (U+00EB ë etc), never base-letter + combining diaeresis.
- Apostrophe: plain straight U+0027 throughout (eg elision "'s-Gravenhage" pattern); no typographic ’ substitute.
- No joining/shaping mechanics; Latin non-cursive script, not applicable.
- Directionality: left-to-right only; not applicable.
- Case distinction applies to all casing rules downstream (topic 6).
- Orthographic standard: current official spelling per Woordenlijst Nederlandse Taal (post-2005 reform); no pre-reform spellings.
- Inter-word spacing: single space; single space also between native text and an embedded foreign/numeric token, no extra padding.
- Compound-formation: fuse by default (eg "Stroomvisualisatie", "Draadframe"); hyphenate only at vowel/consonant clash, before a proper noun/abbreviation, or in a modifier compound with a hyphenated head (eg "Verre-veldbijdrage", "Oppervlakteonderverdeling" fused, "V/A-verhouding" hyphenated before the abbreviation); never a spaced compound.

## 2. Numerals in literals

- Use Western Arabic digits 0-9 in translated prose so technical values follow Dutch engineering convention.
- Use a comma as the decimal separator and a period as the thousands separator in numbers physically present in translated prose (eg "1.000,0").
- Keep formulas, examples, fixed defaults, and named mathematical or standards constants in source form so their technical identity remains unchanged.
- Join a literal ordinal or index digit to the suffix "e" or "de" (eg "3e"); treat the suffix as an index marker, never as a decimal separator.

## 3. Punctuation and quotation

- Quotation marks: straight double quotes " throughout, including embedded technical tokens; avoids ambiguity with native „…” typography in short labels/tooltips.
- Comma, period, question mark, exclamation mark: same glyphs as source, no native replacement needed.
- Put no space before a colon or semicolon and one space after it so punctuation follows Dutch prose spacing.
- Ellipsis: single "…" character; en dash "–" for ranges, hyphen "-" for compounds (topic 1).
- Sentence terminator: full stop on complete sentences; menu items, labels, and other fragments omit the terminal period.
- Punctuation inside an embedded technical run (format specifier, code, unit expression) stays in source form.

## 4. Never-translate tokens

- NEC2 card mnemonics kept verbatim: GW GA GH EX LD FR RP GE EN SP SM SC NE NH NT TL GN GD GC GX GR GS GM GF CP CM SY XQ EK KH PQ PT.
- Keep every source unit symbol verbatim: frequency, gain, impedance-magnitude, power-ratio, temperature, conductivity, angle, and percent symbols retain their source spelling (MHz, dBi, Ω, dB, K, S/m, °, %).
- Keep figure-of-merit and parameter tokens verbatim: VSWR, every S-parameter token, Z, Z0, Zref, G/T, and source F/B; translate the user-visible front/back label consistently as "V/A" to preserve its established Dutch abbreviation.
- File extensions and literal filenames kept verbatim: .nec, .csv, .s1p, .s2p.
- Format specifiers kept verbatim (topic 5).
- Embedded identifier/function/variable/config-key names inside developer strings kept verbatim (topic 24).
- Physical/mathematical symbol letters kept verbatim: E, H (field letters), θ, φ (angle), Z (impedance symbol); lowercase coordinate "z" stays lowercase (topic 6).
- Keep product, library, toolkit, and chart names verbatim: xnec2c, GSL, GTK, Cairo, OpenGL, GL, and Smith.
- Keep named transfer-function and algorithm terms verbatim: Log, Asinh, μ-law, Reinhard, Sigmoid, and Identity; translate the descriptive Power family name as "Machtsfunctie" to separate it from electrical power.
- Conditional loanwords, decision locked file-wide: segment, patch, tag kept as NEC2 geometry terms (never generic synonyms); "fork" (process) kept verbatim.
- Not applicable: bidirectional-text mirroring rule (topic 1 states left-to-right only).

## 5. Format-specifier integrity

- Every specifier from the source is preserved, same set, none added or dropped.
- Default ordering matches source order; positional reordering (eg %1$s) allowed only where Dutch word order requires it.
- Prefer restructuring the sentence around fixed specifier positions over reordering specifiers.
- Numbers inside specifiers are never localized; stay period-decimal, source digit grouping.

## 6. Capitalization and title-case

- Default casing: sentence case for labels, menu items, and titles; capitalize only the first word, proper nouns, and acronyms; do not capitalize every noun.
- Axis-letter casing: X/Y/Z uppercase on axis labels.
- Exemption: a lowercase math/coordinate variable (eg "z" in "step size limited at z=") stays lowercase; the axis-letter rule does not override a source variable.
- No coordinated-option-name capitalization exception found; not applicable.
- Generic card/record noun casing by position: topic 18.
- Acronyms and proper nouns (NEC2, VSWR, xnec2c) retain source casing regardless of sentence position.

## 7. Interface register by string type

Treat each string family as a distinct interface grammar; preserve all source meaning in the shortest complete natural form.

- Commands, buttons, and menu actions use a subjectless infinitive or noun phrase (eg "Opslaan"); place the verb first when an object follows and omit conversational imperatives.
- Field labels use a compact noun phrase in head-final compound order plus the retained colon; omit subjects, verbs, and articles unless required for natural meaning.
- Dialogs and confirmations use full declarative or interrogative sentences in formal register; place the subject before the finite verb in declarations and invert them in direct questions.
- Tooltips use full sentences ordered as control or action, effect, then condition or reason; preserve the source's reason when a disabled control is unavailable.
- Status and error messages use impersonal declarative or passive clauses; present the affected entity before the state or failure and omit first-person narration.
- Name user-visible domain entities with the locked topic-10 term as the compound head and put narrowing modifiers before it; keep proper names and personal names under topic 8.
- Prefer fused compounds where natural; use multiword phrases when compounding obscures meaning, and never abbreviate or imitate source length at the cost of meaning.
- Apply topic 24 to developer and debug strings.

## 8. Formality and address

- Use formal "u" only when direct second-person address is unavoidable; omit informal "je", "jij", and "jouw" to keep professional register consistent.
- Prefer impersonal constructions: commands use a subjectless infinitive, while dialogs and confirmations use "u" only when grammar requires an explicit subject.
- Use third-person singular verb agreement with formal "u"; Dutch has no separate formal verb-ending morphology.
- Avoid unnecessary gender marking and use natural inclusive occupational and participant nouns; retain grammatical gender and number agreement required by topic 15.
- Omit honorifics unless the source names one; use second person only under the direct-address rule and preserve a person's source name order.
- Form confirmations as full formal sentences, with the condition before the requested or warned action when that is the natural information order (eg "voordat u benchmarks uitvoert").
- Omit casual, slang, commercial, archaic, and ceremonially over-formal registers so engineering text remains neutral and professional.

## 9. Accelerator/hotkey mnemonics

- Mnemonic marker: underscore "_" placed immediately before the mnemonic letter (GTK convention).
- Choose a distinct mnemonic letter within each related command group so the translated literal remains unambiguous.
- The mnemonic letter is drawn from the translated Dutch term, never a transliteration of the source letter.
- Non-Latin appended-Latin-letter presentation: not applicable, Dutch is Latin script.
- Preserve a mnemonic marker only when the source literal carries one; never invent one from the string's interface role.
- Typability: avoid trema letters (ë, ï, ü) as the mnemonic key; pick an adjacent plain letter instead.

## 10. Domain lexicon

Locked concept → term (sense); purpose/hazard.

### Electrical primitives

- current → "Stroom" (Amperes); never "huidig"/"actueel" (temporal sense).
- charge → "Lading" (Coulombs); never "kosten"/billing sense.
- voltage → "Spanning".
- power (electrical) → "Vermogen" (radiated/dissipated watts, power gain, power-flow); distinct from the transfer-family name, now "Machtsfunctie" (topic 22).
- impedance → "Impedantie" (complex Z); distinct from resistance, reactance, and load.
- resistance → "Weerstand" (real part of Z).
- reactance → "Reactantie" (imaginary part of Z).
- inductance → "Inductantie".
- capacitance → "Capaciteit".
- conductivity → "Geleidbaarheid" (native term, S/m).
- admittance → "Admittantie"; distinct from impedance.
- load → "Last" (LD-card impedance load); not physical weight; distinct from "Impedantie" and from "Lading" (charge).
- gain → "Versterking" (antenna directivity ratio, dB); use "Versterkingsfactor" for amplifier amplification and "Winst" for profit so the three senses never collapse.
- excitation → "Excitatie" (EM energy input); never emotional excitement.
- feedpoint → "Voedingspunt".
- port → "Poort" (excitation/S-parameter port).
- radials → "Radialen" (noun, ground-plane wires); distinct from adjective "radiaal".

### Ground and earth

- ground/ground plane → "Grond" (RF electrical reference plane; GN/GD ground cards, ground type/conductivity/effects/model); one term across every ground sub-sense; not soil.
- earth (physical medium) → "Aardbodem" (terrain/noise-model earth, "below ground" geometry); distinct from "Grond".
- ground wave → "Grondgolf" (propagation term); distinct from the "Grond" reference.

### Geometry primitives

- wire → "Draad"; never cable/cord.
- segment → "Segment" (NEC2 subdivision; kept, topic 4).
- patch → "Patch" (NEC2 surface patch; kept, topic 4).
- tag → "Tag" (NEC2 geometry identifier; kept, topic 4); not a UI label or card.
- card → "Kaart" (NEC2 input record; register at topic 18).
- kernel → "Kern" (integral-equation/thin-wire kernel); never OS kernel.
- cliff → "Klif" (two-medium ground-boundary type); never fracture/break.
- structure → "Structuur" (antenna model geometry); never "constructie".
- model → "Model" (NEC model or noise-temperature model).
- geometry → "Geometrie" (the model geometry).
- crossed → "Gekruist" (transmission-line conductors crossed/reversed); never cut/severed.

### Field, pattern, viewer

- field (EM) → "Veld"; homonym with UI/config field, disambiguated by context (topic 11).
- near field → "Nabije veld"; far field → "Verre veld"; symmetric pair.
- far-field contribution → "Verre-veldbijdrage" (per-direction); not near-field animation.
- radiation → "Straling".
- radiation pattern → "Stralingspatroon"; never template/design.
- gain pattern → "Versterkingspatroon".
- polarization → "Polarisatie"; distinct from "Polariteit" (topic 12).
- polarity → "Polariteit" (sign of a quantity); false friend of polarization.
- phase → "Fase"; reference phase → "Referentiefase".
- frequency → "Frequentie".
- wave → "Golf"; wavelength → "Golflengte".
- standing wave → "Staande golf"; traveling wave → "Lopende golf"; opposed pair.
- node/antinode → "Knopen/buiken" (standing-wave zero/maximum; also the null/peak overlay sense).
- crest → "Top" (instantaneous wave apex, including a comet head); distinct from generic curve/step "Piek" so the wave feature has one locked term.
- magnitude → "Grootte" (modulus, eg "Z-grootte"); distinct from amplitude.
- amplitude → "Amplitude" (oscillating-quantity peak); reserved for the color-projection/field-amplitude family.
- peak value → "Piekwaarde"; peak magnitude → "Piekgrootte"; two distinct options, never collapsed.
- instantaneous → "Momentaan"; "(φ=0)" qualifier added only where the source carries it.
- Poynting vector → "Poynting-vector" (hyphenated, canonical spelling); solid angle → "Ruimtehoek".
- net gain → "Netto versterking"; distinct from "Reële versterking" (real-part gain).
- viewer → "Viewer" (loanword; observation direction / 3D view widget); distinct from "Weergave" (Display heading) and from observer/speaker/preview.
- flow/flow direction → "Stroomrichting" (patch/current flow).
- total field → "Totaal veld".

### Color, tone, animation subsystem

- color → "Kleur"; color projection → "Kleurprojectie".
- hue → "Tint"; brightness → "Helderheid".
- hue encoding → "Tintcodering"; brightness encoding → "Helderheidscodering"; distinct internal enums, neither collapses to "Kleurprojectie".
- color scale → "Kleurschaal" (magnitude-to-color scale).
- scale family / color tone → "Schaalfamilie" (the transfer-curve family: Power/Log/Asinh/μ-law/Reinhard/Sigmoid/Identity); one concept, one term.
- palette → "Palet"; palette kind → "Paletsoort"; distinct from scale family and color projection.
- ramp/gradient → "Verloop" (palette kind / linear color strip); distinct from "Bereik" (dB range control).
- gamma → "Gamma" (power-law exponent); knee → "Knik" (soft-knee bend point).
- softening → "Verzachting"; compression → "Compressie"; contrast → "Contrast"; dynamic range → "Dynamisch bereik"; distinct from plain "Bereik".
- floor → "Ondergrens" (minimum/lower clamp, eg "Helderheidsondergrens"); never a room floor.
- envelope → "Omhullende" (magnitude/amplitude envelope).
- comet → "Komeet" (moving-crest overlay); never geometry (topic 12).
- overlay (noun) → "Overlay" (added visual layer, loanword); verb "to overlay" → "overlayen"; distinct usages.
- animate → "Animeren"; animation → "Animatie".
- animated/static (category headers) → "Geanimeerd"/"Statisch".
- projection → "Projectie" (parent of color/geometry projection).
- scale (verb) → "Schalen"; scale (noun) → "Schaal".
- wireframe → "Draadframe" (wire-mesh render mode).
- identity → "Identiteit" (no-op/passthrough transfer); distinct from "Eenheid" (unity, Smith-chart).
- sentinel → "Sentinelwaarde" (unreachable-case guard).
- bins → "Bakken" (discretization buckets).
- companding → "Companding" (μ-law bounded log curve; kept, no established native equivalent).
- tone mapping → "Tone mapping" (photographic tone-map; kept, established graphics-practice loanword).

### Render and compute

- renderer → "Renderer" (drawing backend, kept); never "render-engine".
- shader → "Shader" (kept).
- allocation (memory) → "Allocatie"; managed allocator → "Beheerde allocator".
- thread → "Thread" (compute thread, kept loanword); distinct from "Draad" (wire).
- widget → "Widget" (UI element, kept).
- validation → "Validatie" (the validation-tree feature); distinct from "Verificatie" (checks).
- batch mode → "Batchmodus".
- fork (process) → "Fork" (kept, topic 4).
- deadlock → "Deadlock" (kept); notifier → "Melder".
- token/operand/operator/arity → "Token"/"Operand"/"Operator"/"Ariteit" (expression-parser terms, kept technical loanwords).
- override → "Overrulen" (supersede a value, SY symbol); distinct from "Overschrijven" (overwrite).
- swap → "Wisselen" (exchange).
- theme → "Thema" (UI/color theme); distinct from "onderwerp" (topic/subject).

### Metrics and miscellaneous

- noise → "Ruis"; noise temperature → "Ruistemperatuur"; never acoustic racket.
- efficiency → "Efficiëntie"; interpolation → "Interpolatie".
- mnemonic → "Mnemonic" (a card's code descriptor, kept); never memo/note.
- degrees/deg: freestanding prose "graden"; parenthetical unit tag "(deg)" kept verbatim like other unit tags.
- diameter → "Diameter" (canonical loanword, locked; never "middellijn").
- reflect: geometry mirror operation → "Spiegelen"; behavioral "mirrors …" (tracking) → "Volgt"; physics reflection → "Reflectie"; three distinct senses, never merged.
- default(s) → "Standaardwaarde(n)" (fallback value).
- normalize → "Normaliseren"; normalization → "Normalisatie" (native-formed, not transliterated).

## 11. Disambiguation policy

- The correct technical sense is chosen for every ambiguous term from topic 10; no other sense substituted.
- No qualifier absent from the source is added; program context (an EM simulator) already disambiguates (eg no "elektrische" prefix before Stroom/Lading/Spanning).
- A qualifier is added only where the Dutch term would otherwise be genuinely ambiguous.
- Accepted intra-domain homonym: "Veld" covers both EM field and UI/config field; context disambiguates, no qualifier added.
- Accepted intra-domain homonym: "Grond" is a single term across every ground sub-sense (topic 10); not a collision requiring qualification.
- No locative/other homonym collision found beyond the above; not applicable.
- Gerund vs noun senses: "Schalen" (verb, to scale) vs "Schaal"/"Schalen" (noun, a scale/scales); distinguished by grammatical context, never a fixed suffix rule.

## 12. Cross-catalog consistency

- One term per concept, reused from the established lexicon (topic 10); no synonym coined for an already-mapped concept.
- Appendix C pairs, each resolved to two distinct Dutch terms:
  - polarity "Polariteit" vs polarization "Polarisatie".
  - magnitude "Grootte" vs amplitude "Amplitude".
  - peak value "Piekwaarde" vs peak magnitude "Piekgrootte".
  - ground "Grond" vs earth "Aardbodem".
  - load "Last" vs charge "Lading".
  - gain "Versterking" vs amplification "Versterkingsfactor" vs profit "Winst".
  - current "Stroom" vs temporal "actueel"/"huidig" (forbidden sense).
  - charge "Lading" vs billing "kosten" (forbidden sense).
  - wire "Draad" vs cable/cord (forbidden synonym); wire "Draad" vs thread "Thread".
  - radiation pattern "Stralingspatroon" vs template/design (forbidden sense); vs far field "Verre veld".
  - excitation "Excitatie" vs emotional excitement (forbidden sense).
  - node/antinode "Knopen/buiken" vs a generic numeric null/zero (forbidden collapse).
  - scale family "Schaalfamilie" vs hue "Tint" vs palette kind "Paletsoort" vs color projection "Kleurprojectie"; four distinct chroma concepts.
  - comet "Komeet" vs geometry "Geometrie".
  - identity "Identiteit" vs unity "Eenheid".
  - renderer "Renderer" vs render engine (forbidden phrasing).
  - override "Overrulen" vs overwrite "Overschrijven".
  - viewer "Viewer" vs observer/speaker/preview (forbidden synonyms).
  - reflect: "Spiegelen"/"Volgt"/"Reflectie" (topic 10), never merged.
  - structure "Structuur" vs construction "constructie" (forbidden sense).
  - theme "Thema" vs topic/subject "onderwerp" (forbidden sense).
  - validation "Validatie" vs verification "Verificatie".
  - net gain "Netto versterking" vs real-part gain "Reële versterking".
  - power "Vermogen" vs the "Power" transfer-family name "Machtsfunctie".
- Loanword-vs-native locked spellings: "Diameter" (not "middellijn"), "Renderer", "Shader", "Widget", "Thread", "Fork", "Deadlock", "Overlay", "Viewer", "Companding", "Tone mapping" kept as loanwords; "Poynting-vector" hyphenated canonical spelling.
- Minority-outlier spellings unify to the canonical form (eg always "Draadframe" one word, never "draad-frame"; always "Vermogen" singular for electrical power, never "vermogens" in generic UI prose).
- Consistency priority can outrank locale-form preference: "Overlay" stays the loanword though a native "Overlegging" exists, to match established graphics-terminology precedent; "Companding" and "Tone mapping" stay loanwords for the same reason.

## 13. Priority ordering

- Precedence chain: correct meaning, then interface convention, then catalog consistency, then disambiguation, then locale numeral form.
- Override ruling: "Vermogen" is reserved for electrical power; the transfer-family sense is renamed "Machtsfunctie" to resolve the Appendix C collision (topic 22).
- Override ruling: "Grond" stays the single ground-reference term though "Aarde" is the more common everyday word, for consistency with the established GN-card catalog usage.

## 14. Grammatical number

- Use singular and plural number in translated literals; choose singular after an explicit count of 1 and plural after every other explicit count.
- Inflect nouns for number with their established plural in "-en" or "-s"; apply the lexically correct class rather than inventing a uniform ending.
- Make finite verbs agree with a singular or plural subject and make adjectives and participles agree where topic 15 requires it.

## 15. Grammatical agreement

- Adjective/participle concord with the head noun: common-gender ("de") nouns take the "-e" adjective ending (eg "de nieuwe kaart"); neuter ("het") nouns in the indefinite singular take the bare form (eg "een nieuw model").
- Standalone labels whose head noun is implied default to common gender "-e" agreement unless the implied noun is a known "het"-word (eg "het model").
- No case declension; no partitive-after-count rule; not applicable.
- Reflexive-particle retention where required (eg "zich herstellen" keeps "zich").

## 16. Morphological derivation

- Form borrowed technical verbs with the established "-eren" suffix (eg "normaliseren"); omit a bare source-language infinitive without the Dutch ending.
- Verbal-noun formation: "-atie" for Latinate roots (eg "Interpolatie", "Animatie", "Compressie"); "-ing" for native-root verbs (eg "Verzachting").
- Native affix preferred over a bare loanword suffix; compounding follows topic 1 (fuse by default).

## 17. Preposition and sandhi selection

- Not applicable: Dutch has no context-conditioned preposition-form selection or sandhi rule bearing on technical UI strings.

## 18. Card/record-label register

- Fixed designator form in dialog/editor titles: "Opdrachtkaart" / "Geometriekaart".
- Running-prose form in messages: lowercase "opdrachtkaart"/"geometriekaart", not the capitalized title form.
- Generic-noun casing: "kaart" lowercase in running prose, capitalized only as a title/heading; short form "Kaart" acceptable in width-constrained UI, long form in titles.
- Each register stays internally consistent; never cross-converted between title and running-prose form.

## 19. Multi-paragraph and whitespace fidelity

- Mirror source paragraph breaks at the same positions and preserve whether each boundary is a blank line or a single line break so semantic grouping remains intact.
- Drop a trailing clause removed from the current source so stale meaning does not survive.
- Preserve source trailing newlines and punctuation exactly so literal boundaries retain their semantics.
- Preserve semantic line breaks and add no visual wrapping absent from the source literal.
- Preserve complete natural meaning; never truncate or abbreviate a translation to fit an assumed display constraint.

## 20. Current-source fidelity

- Derive every translation from the complete current source literal and its supplied context so no changed meaning is inherited from prior wording.
- Reuse an inherited translation only when its complete meaning agrees with the current source and context.
- Treat prior "Grond" for physical terrain and prior "Vermogen" for the transfer-family sense as unsafe inheritance; use "Aardbodem" and "Machtsfunctie" respectively under topic 10.

## 21. Script hygiene

- Use only the Latin letters and required diacritics from topic 1 in translated prose; forbid Greek, Cyrillic, and other-script homoglyphs unless a retained identifier, unit, or mathematical token in topic 4 requires its literal character.
- Translate plain foreign prose words into Dutch; keep only genuine identifiers, units, symbols, proper names, and locked loanwords listed by these rules.

## 22. Rule-file scope hygiene

- Keep only current decisions that can alter wording, Unicode characters, punctuation, capitalization, embedded tokens, mnemonic markers, or semantic whitespace in a translated literal.
- Omit catalog representation, serialization, headers, flags, validation procedure, rendering, fonts, shaping, layout, widget behavior, runtime formatting, sorting, search, display sizing, audit history, completion state, review metadata, provenance, and citations.
- State each retained decision directly; include no implementation procedure, bibliography, correction history, or competing policy source.

## 23. Section-disjointness declaration

- Script mechanics (topic 1), interface phrasing/structure (topics 6, 7, 16, 18, 19), and address register (topic 8) are non-overlapping axes; every concept lands in exactly one of these sections.

## 24. Developer/debug-string policy

- Translate user-facing commands, labels, dialogs, tooltips, status text, and errors completely; their review priority never permits an applicable string to remain untranslated.
- Translate informational notices and reports in concise technical Dutch.
- Translate developer-facing diagnostics, including `pr_*` families, in terse impersonal technical Dutch unless an established sibling family within the same subsystem retains source-language prose.
- Keep identifiers, function names, variables, configuration keys, format specifiers, and topic-4 tokens verbatim inside every translated family (eg `config_widget_lookup`, `mem-report`, `rc_config_field_size`).
- Use priority only to order review; preserve embedded format and retained tokens in every family regardless of priority.
