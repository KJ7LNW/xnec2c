# Finnish (Finland) translation rules

These rules govern current standard Finnish in the Latin script for professional electrical and radio-frequency engineers, antenna designers, and amateur-radio operators in Finland.

## 1. Script and orthography

- Use the Finnish Latin alphabet; retain `ä` and `ö` as mandatory letters, never fold them to `a` or `o`, and use `å` only where a retained proper name requires it.
- Use precomposed `ä` and `ö`, never combining-mark sequences, to keep literal spelling consistent.
- Apostrophe, modifier-letter, textual-joiner, and positional-form distinctions are not applicable to standard Finnish orthography.
- Write left-to-right; opposite-direction handling is not applicable to Finnish prose.
- Preserve letter case; apply topic 6 to interface casing.
- Use current standard Finnish orthography for Finland, with no regional or historical variant.
- Put one space between words and between Finnish text and embedded foreign or numeric tokens.
- Form a fused compound where standard Finnish has one; use a hyphen where a retained technical token joins a Finnish element, eg `μ-laki-kompandointi`.

## 2. Numerals in literals

- Use Arabic digits for literal technical values in translated prose.
- Use a decimal comma and a space as the thousands separator in literal prose, eg `50,0`.
- Keep formulas, examples, fixed defaults, and named mathematical or standards constants in source form so their technical identity remains unchanged.
- Mark an ordinal with a period after its literal digit, eg `1.`; use the decimal comma only inside a decimal number so index and decimal notation remain distinct.

## 3. Punctuation and quotation

- Native quotation marks: `”...”`, identical glyph both sides, not `„...“` or `«...»`; embedded technical tokens keep source straight quotes for consistency with topic 4.
- Native punctuation replaces source counterparts in prose: comma, question mark, exclamation point as standard Finnish usage; no special opening marks.
- Spacing: no space before colon/semicolon, single space after.
- Ellipsis: single `…` character; dashes preserved as in source (en dash for ranges).
- Sentence terminator: period on full sentences; short labels/fragments omit it.
- Punctuation inside embedded technical runs (format specifiers, mnemonics) stays in source form.

## 4. Never-translate tokens

Keep every token in this section byte-for-byte in its stated source form; never translate or transliterate it.

- Keep NEC2 card mnemonics `GW GA GH EX LD FR RP GE EN SP SM SC NE NH NT TL GN GD GC GX GR GS GM GF CP CM SY XQ EK KH PQ PT` verbatim so file-format identifiers round-trip.
- Keep source unit symbols for frequency, gain, impedance magnitude, power ratio, temperature, conductivity, angle, and percent verbatim, including `MHz`, `dBi`, `Ω`, `deg`, and `%`, to preserve standard notation.
- Keep `VSWR`, S-parameter tokens, `Z`, `Z0`, front/back-ratio tokens, and gain/temperature tokens verbatim to preserve radio-frequency notation.
- Keep every source file extension and literal filename verbatim so it continues to name the same format or file.
- Keep every source format specifier verbatim, including `%s`, `%d`, `%f`, `%c`, and `%%`; topic 5 governs their placement.
- Keep embedded function names, variable names, and configuration keys verbatim because they identify code.
- Keep physical and mathematical symbol letters verbatim, including `E`, `H`, `φ`, `x`, `y`, and `z`, because a look-alike changes the notation.
- Keep product, library, toolkit, and chart proper names verbatim, including `xnec2c`, `GTK`, `Cairo`, and `OpenGL`.
- Keep named transfer functions and algorithms `Log`, `Asinh`, `μ-law`, `Reinhard`, `Sigmoid`, and `Identity` verbatim; translate the descriptive `Power` family name as `potenssi`.
- Apply one file-wide decision to conditional loanwords: retain `patch` with Finnish plural `patchit`; naturalize `segment` as `segmentti`; translate `tag` as `tunniste`; retain process `fork` verbatim.
- Keep retained tokens left-to-right under topic 1; bidirectional mirroring is not applicable.

## 5. Format-specifier integrity

- Every specifier from the source is preserved, same set, unchanged.
- Default order matches source; positional reordering (`%1$s`) allowed only where Finnish word order forces it.
- Divergent word order restructures the surrounding sentence around fixed specifier positions rather than reordering the specifiers themselves.
- Numbers inside specifiers are never localized to comma/space form.

## 6. Capitalization and title-case

- Use sentence case for labels, menu items, and titles; capitalize no common noun solely because it appears in a title.
- Axis-letter casing: source casing retained unchanged (X, Y, Z).
- Lowercase math/coordinate variables (x, y, z, φ) stay lowercase regardless of position.
- Coordinated option names: no per-element capitalization; only sentence-initial capitalization applies.
- Generic card/record noun (`kortti`): lowercase in running prose, capitalized only when sentence-initial or inside a dialog/editor title (topic 18).
- Acronyms and proper nouns (NEC2, VSWR, GTK) retain source casing.

## 7. Interface register by string type

Use Finnish interface grammar for each string family; preserve all source meaning in the shortest complete natural form.

- Write commands, buttons, and menu actions as subjectless second-person singular imperatives with the verb before its object, eg `Avaa`.
- Write field labels as concise noun phrases in natural modifier-before-head order and retain a source colon.
- Write dialogs and confirmations as full sentences with context before the requested decision; use indicative statements or a direct confirmation question as appropriate.
- Write tooltips as impersonal declarative sentences in natural subject-verb-object order; state the supplied reason a disabled control is unavailable.
- Write status and error messages as impersonal declarative or passive sentences; present the condition before its consequence or remedy.
- Prefer a standard fused compound where natural Finnish has one; otherwise use the shortest complete multiword phrase without abbreviation, omitted meaning, or mechanical source-length matching.
- Name user-visible domain entities with the topic 10 term as the head and place Finnish modifiers before it; topic 8 governs proper and personal names.
- Apply topic 24 to developer and diagnostic strings.

## 8. Formality and address

- Finnish has no strong T-V distinction; formal `Te` reads archaic/stilted in engineering software and is forbidden.
- Register is carried by construction instead: impersonal passive (`voidaan`) for tooltips/status, 2nd-person singular imperative for commands, full sentences for dialogs.
- Subject-drop: imperative and passive constructions naturally omit the subject pronoun; retained.
- Gender marking and gender agreement are not applicable; use gender-neutral personal reference and natural number and case forms for inclusive language.
- Omit honorifics unless a source proper name contains one; use second person only in commands and otherwise prefer an impersonal construction.
- Preserve a person's established full-name order and spelling because the interface does not reorder personal names.
- Realize register through the imperative, passive, or indicative verb form, never through pronoun choice.
- Split register by string type: commands use the imperative; dialogs use full sentences; tooltips and status messages use impersonal declaratives or the passive.
- Write a confirmation as a full-sentence impersonal question that names the action before requesting confirmation.
- Exclude casual, slang, over-formal `Te` forms, commercial tone, and archaic forms.

## 9. Accelerator/hotkey mnemonics

- Marker convention: underscore before the mnemonic letter (`_x`), GTK convention.
- Draw the mnemonic letter from the translated Finnish term, never from a transliteration of the source term.
- Separate parenthetical mnemonic presentation is not applicable to the Latin script.
- Preserve a mnemonic marker only where the source literal contains one; never invent one.
- Prefer an easily typed unaccented letter present in the translation; use `ä` or `ö` only when no suitable unaccented letter is available.

## 10. Domain lexicon

Electrical primitives:

- `current` → virta; electrical sense only, guards against a temporal "recent" misread.
- `charge` → varaus; electrical sense only, guards against billing/fee misread.
- `voltage` → jännite.
- `power (electrical)` → teho; distinct from the `Power` scale-family member (→ `potenssi`) and from `gain` (→ vahvistus).
- `impedance` → impedanssi; distinct from resistanssi/reaktanssi.
- `resistance` → resistanssi; real part of Z, distinct from impedanssi and kuorma.
- `reactance` → reaktanssi; imaginary part of Z.
- `inductance` → induktanssi.
- `capacitance` → kapasitanssi.
- `conductivity` → johtavuus; native term, no loanword used.
- `admittance` → admittanssi; distinct from impedanssi.
- `load` → kuorma; LD-card impedance load, distinct from varaus (charge) and from physical weight.
- `gain` → vahvistus.
- `excitation` / `excitation type` → herätys / Herätteen tyyppi; distinct from emotional excitement.
- `feedpoint` → herätepiste.
- `port` → portti; excitation/S-parameter port.
- `radials` → vastapainot (noun); distinct from the adjective "radiaalinen".

Ground and earth:

- `ground` / `ground plane` → maataso; one term across GN/GD cards, ground type, conductivity, and effects.
- `earth (physical medium)` → maaperä; terrain/noise-model sense, distinct from maataso.
- `ground wave` → maa-aalto; propagation term, distinct from maataso.

Geometry primitives:

- `wire` → lanka; GW conductor element, distinct from säie (compute thread) and from kaapeli (cable/cord, not used).
- `segment` → segmentti/segmentit; conditional loanword, never re-derived as "osa".
- `patch` → patch/patchit; kept verbatim per topic 4.
- `tag` → tunniste; NEC2 geometry identifier, distinct from a UI label (nimike) or a card (kortti).
- `card` → kortti; register handled in topic 18.
- `kernel` → ydin; integral-equation/thin-wire kernel, distinct from an OS kernel (not used in this catalog).
- `cliff` → jyrkänne; two-medium ground-boundary type, distinct from a fracture/break.
- `structure` → rakenne; antenna model geometry, distinct from "rakentaminen" (construction).
- `model` → malli.
- `geometry` → geometria.
- `crossed` → ristikytketty; transmission-line conductors crossed/reversed, distinct from cut/severed.

Field, pattern, viewer:

- `field (EM)` → kenttä; accepted homonym with a UI/config field, context disambiguates.
- `near field` / `far field` → lähikenttä / kaukokenttä; opposed pair.
- `far-field contribution` → kaukokentän osuus; per-direction contribution, distinct from near-field animation.
- `radiation` → säteily.
- `radiation pattern` → säteilykuvio; plotted directional response, distinct from a template/design.
- `gain pattern` → vahvistuskuvio; the gain radiation pattern.
- `polarization` → polarisaatio (`polarization axis` → polarisaatioakseli).
- `polarity` → napaisuus; sign of a quantity, distinct from polarisaatio.
- `phase` → vaihe.
- `reference phase` → referenssivaihe.
- `frequency` → taajuus.
- `wave` / `wavelength` → aalto / aallonpituus.
- `standing wave` / `traveling wave` → seisova aalto / etenevä aalto; opposed pair.
- `node` / `antinode` → solmu / vastasolmu; standing-wave zero/maximum, also the null/peak overlay sense.
- `crest` → harja; instantaneous wave apex, distinct from a curve/step peak (huippu).
- `magnitude` → suuruus; never itseisarvo, one term catalog-wide.
- `amplitude` → amplitudi; oscillating-quantity peak, distinct from suuruus.
- `peak value` vs `peak magnitude` → huippuarvo vs huippusuuruus; two distinct options, never collapsed to one label.
- `instantaneous` → hetkellinen; add "(φ=0)" only where the source carries it.
- `Poynting vector` → Poyntingin vektori.
- `solid angle` → avaruuskulma.
- `net gain` → nettovahvistus; total-minus-mismatch gain, distinct from reaaliosan vahvistus (real-part gain).
- `viewer` → katselusuunta (observation direction) / katseluikkuna (3D view widget); distinct from a speaker or a preview.
- `flow` / `flow direction` → virtaus / virtaussuunta.
- `total field` → kokonaiskenttä.

Color, tone, animation subsystem:

- `color` → väri.
- `color projection` → väriprojektio; which quantity drives hue.
- `hue` → sävy; distinct axis from kirkkaus (brightness), eg "Phase Hue" → Vaihesävy.
- `brightness` → kirkkaus.
- `hue encoding` / `brightness encoding` → sävykoodaus / kirkkauskoodaus; distinct internal enums, neither collapses to väriprojektio.
- `color scale` → väriskaala; magnitude-to-color scale.
- `scale family` / `color tone` → skaalaperhe; one concept, two source spellings, one term, covering Power/Log/Asinh/μ-law/Reinhard/Sigmoid/Identity.
- `palette` / `palette kind` → paletti / palettityyppi; distinct from skaalaperhe and väriprojektio.
- `ramp` / `gradient` → liukuma; a palette kind and a linear color strip, one term.
- `gamma` → gamma.
- `knee` → pehmennyspolvi (softening knee).
- `softening` → collapses onto pehmennyspolvi; Finnish always pairs the two, no standalone term.
- `compression` → kompressio.
- `contrast` → kontrasti; difference between light and dark values.
- `dynamic range` → dynamiikka-alue; represented value span, distinct from kontrasti and kompressio.
- `floor` → alaraja; minimum/lower clamp, distinct from a physical room floor.
- `envelope` → verhokäyrä (`peak envelope` → huippuverhokäyrä).
- `comet` → komeetta; moving-crest overlay effect, distinct from rakenne (geometry) to prevent cross-concept substitution.
- `overlay (noun)` → peitekerros; distinct from the verb "peittää" (to overlay).
- `animate` / `animation` → animoi / animaatio.
- `animated` / `static` → animoitu / staattinen; category-header adjectives.
- `projection` → projektio; base term shared by color and geometry projection.
- `scale` → skaalaa (verb) / skaala (noun).
- `wireframe` → rautalankamalli.
- `identity` → identiteetti; no-op passthrough transfer, distinct from unity (Smith-chart, ykkönen).
- `sentinel` → vartioarvo; unreachable-case guard value.
- `bins` → lokerot; discretization buckets.
- `companding` → kompandointi (μ-law companding → μ-laki-kompandointi).
- `tone mapping` → sävykartoitus (Reinhard tone-map → Reinhardin sävykartoitus).

Render and compute:

- `renderer` → piirtäjä; drawing backend, never "renderöintimoottori" (render engine).
- `shader` → varjostin.
- `allocation (memory)` / `managed allocator` → muistivaraus / hallittu varaaja; compound disambiguates memory allocation from varaus (electrical charge).
- `thread` → säie; compute thread, no collision with lanka (wire), context disambiguates regardless.
- `widget` → säädin; UI element.
- `validation` → validointi; the validation-tree feature.
- `batch mode` → eräajo.
- `fork (process)` → fork; kept verbatim per topic 4.
- `deadlock` → lukkiutuma.
- `notifier` → ilmoitin.
- `token` / `operand` / `operator` / `arity` → tunnus / operandi / operaattori / ariteetti; expression-parser terms.
- `override` → korvaus; supersede a value (SY symbol), distinct from ylikirjoitus (overwrite).
- `swap` → vaihto.
- `theme` → teema; UI/color theme, distinct from aihe (topic/subject).

Metrics and miscellaneous:

- `noise` / `noise temperature` → kohina / kohinalämpötila; electronic/thermal, distinct from acoustic racket.
- `efficiency` → hyötysuhde.
- `interpolation` → interpolointi.
- `mnemonic` → tunnuskoodi; a card's code descriptor, distinct from muistio (memo/note).
- `degrees` / `deg` → aste (freestanding axis/prose) vs `(deg)` (parenthetical unit tag, kept verbatim per topic 4).
- `diameter` → halkaisija; native term chosen over the loanword "diametri".
- `reflect` → peilaa (geometry mirror operation) / seuraa (behavioral "mirrors …" tracking) / heijastuu (physics reflection); three distinct senses, never merged.
- `default(s)` → oletus / oletusarvo.
- `normalize` / `normalization` → normalisoi / normalisointi; translated, not transliterated.

## 11. Disambiguation policy

- Correct technical sense is chosen per context for every ambiguous term.
- No qualifier is added beyond what the source expresses; program context already disambiguates (eg kenttä serving both EM field and UI field).
- A qualifier is added only where the Finnish term would otherwise be genuinely ambiguous (eg muistivaraus vs varaus, separating memory allocation from electrical charge).
- Accepted intra-domain homonym: kenttä (EM field / UI field), resolved by context.
- No locative homonym collision identified in this catalog.
- Distinguish a verbal noun with the `-minen` suffix from a bare noun where one source word carries both senses, eg `skaalaa` as a verb and `skaala` as a noun.

## 12. Cross-catalog consistency

- One term per concept; reuse the topic 10 table file-wide, never introduce a synonym for an already-mapped concept.
- Keep every Appendix C side distinct: `napaisuus`≠`polarisaatio`; `suuruus`≠`amplitudi`; `huippuarvo`≠`huippusuuruus`; `maataso`≠`maaperä`; `kuorma`≠`varaus`; antenna `vahvistus`≠amplifier `vahvistaminen`≠profit `voitto`; electrical `virta`≠temporal `nykyinen`; electrical `varaus`≠billing `veloitus`; `lanka`≠`kaapeli`≠`säie`; `säteilykuvio`≠design `malline`≠`kaukokenttä`; electromagnetic `herätys`≠emotional `innostus`; `solmu`/`vastasolmu`≠numeric `nolla`; `skaalaperhe`≠`sävy`≠`palettityyppi`≠`väriprojektio`; `komeetta`≠`geometria`; `identiteetti`≠`ykkönen`; `piirtäjä`≠`renderöintimoottori`; `korvaus`≠`ylikirjoitus`; `katselusuunta`/`katseluikkuna`≠`tarkkailija`≠`puhuja`≠`esikatselu`; `peilaa`≠`seuraa`≠`heijastuu`; `rakenne`≠`rakentaminen`; `teema`≠`aihe`; tree-feature `validointi`≠checks `tarkistus`; `nettovahvistus`≠`reaaliosan vahvistus`; electrical `teho`≠transfer-family `potenssi`.
- Lock loanword and native choices: retain source `patch` with plural `patchit`; naturalize `segment` as `segmentti`; use native `halkaisija` rather than loanword `diametri`.
- Minority-outlier spellings (diametri, itseisarvo) unify to the canonical form (halkaisija, suuruus).
- Consistency priority outranks locale-form preference: suuruus is used for magnitude everywhere, even where itseisarvo would read more idiomatic in an isolated math context.

## 13. Priority ordering

- Precedence chain: correct meaning, then interface convention, then catalog consistency, then disambiguation, then locale numeral form.
- Explicit override: catalog consistency (suuruus for magnitude) overrides the more idiomatic math-register alternative (itseisarvo).

## 14. Grammatical number

- Use singular and plural noun, adjective, participle, and verb forms according to the referent's grammatical number.
- After the literal count `1`, use singular nominative where syntax permits; after other cardinal counts, use a singular partitive noun and matching case on its modifiers, eg `3 segmenttiä`.
- Use plural agreement when the phrase refers to multiple entities without an explicit cardinal count.

## 15. Grammatical agreement

- Adjectives/participles agree with the head noun in case and number, including standalone labels whose head noun is implied (eg "aktiivinen" agrees with the implied "segmentti").
- Declension-class choice follows standard Finnish noun-type rules; a counted quantity after a numeral greater than one takes the partitive (eg "3 segmenttiä").
- Reflexive-particle retention: not applicable to UI strings.
- Standalone-label default gender: not applicable, Finnish has no grammatical gender.

## 16. Morphological derivation

- Form borrowed technical verbs and nouns with the `-oida`/`-ointi` family, eg `validoida`/`validointi`; do not retain a raw source `-ing` ending.
- Verbal-noun formation uses the `-minen` suffix on the verb stem where no established `-ointi` form exists.
- Native-affix compounding is preferred over a loanword where a natural native term exists (eg rautalankamalli over a raw "wireframe"); the loanword is retained where no natural native term exists (patch, fork).

## 17. Preposition and sandhi selection

- Not applicable: Finnish uses case suffixes rather than prepositions, and UI strings require no elision/sandhi handling.

## 18. Card/record-label register

- Dialog/editor titles use the fixed designator form: mnemonic plus `kortti` (eg "GW-kortti").
- Running prose uses the suffixed/genitive form (eg "GW-kortin").
- Keep `kortti` lowercase in running prose; use the short mnemonic alone only where the source literal itself uses the short designator.
- Each register stays internally consistent; the title form and the running-prose form are never cross-converted.

## 19. Multi-paragraph and whitespace fidelity

- Mirror source paragraph breaks at the same positions and preserve whether each break is blank-line or single-line.
- Drop a trailing clause removed from the current source rather than retaining inherited text.
- Preserve source trailing newlines and punctuation.
- Preserve semantic source line breaks; add no line break solely for visual wrapping.
- Preserve complete meaning and natural wording; never truncate or abbreviate for an assumed display constraint.

## 20. Current-source fidelity

- Derive every translation from the complete current source literal and its supplied context.
- Reuse an inherited translation only when its full meaning agrees with the current source.
- Do not inherit `komeetta` for a geometry concept or `rakenne` for the comet overlay; prior translations of these concepts are unsafe without current-context agreement.

## 21. Script hygiene

- Exclude Cyrillic and Greek homoglyphs from Finnish prose where a Finnish Latin letter is intended; permit a non-Latin character only inside a retained token whose spelling requires it, eg `φ`.
- Translate ordinary source-language words; retain only the identifiers, units, symbols, proper names, and technical tokens fixed by topic 4.
- Reject mixed-script adjacency inside a Finnish word; a retained technical token adjacent to Finnish text follows topic 1's spacing or compound rule.

## 22. Rule-file scope hygiene

- Keep only current decisions that can alter wording, Unicode characters, punctuation, capitalization, embedded tokens, mnemonic markers, or semantic whitespace in a translated literal.
- Omit catalog representation, serialization, headers, flags, validation workflow, rendering, fonts, shaping, layout, widget behavior, runtime formatting, sorting, search, display sizing, audit history, completion state, review metadata, provenance, and citations.
- State each retained decision directly; include no implementation procedure, correction history, or competing policy.

## 23. Section-disjointness declaration

- Script mechanics (topic 1), phrasing/structure (topics 6, 7, 9, 18, 19), and address register (topic 8) are non-overlapping: topic 1 governs character/glyph choice, topics 6/7/9/18/19 govern word and sentence form, topic 8 governs pronoun and verb-form register; every concept lands in exactly one section.

## 24. Developer/debug-string policy

- Translate user-facing actions, labels, dialogs, tooltips, errors, and status messages into Finnish; priority never permits an applicable user-facing string to remain untranslated.
- Translate informational diagnostics and developer-facing debug strings into Finnish in a terse technical register.
- Apply an established sibling-string term within the same subsystem where it is more specific than the general register rule.
- Keep identifiers, function names, format specifiers, and all topic 4 tokens verbatim in every string family regardless of translation or review priority.
- Use family classification only to set linguistic register and review order; preserve format and retained tokens in every translated family.
- Render the `BUG:` diagnostic prefix as "OHJELMAVIRHE:", held distinct from the "VIRHE:" prefix that carries ordinary failures; the two severities never share a word.
- Keep an identifier-plus-value diagnostic pair contiguous (eg `sweep_state=%d`); a qualifier precedes the whole pair and never stands between the identifier and its `=`.
