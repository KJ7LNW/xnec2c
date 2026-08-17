# Croatian (Croatia) translation rules

These rules govern Croatian for Croatia, written in the modern Croatian Latin script and current standard orthography, for professional electrical and radio-frequency engineers, antenna designers, and amateur-radio operators.

## 1. Script and orthography

- Use the Croatian Latin script with mandatory letters and diacritics `č ć đ š ž` and `Č Ć Đ Š Ž`; never strip diacritics or replace them with look-alikes.
- Use precomposed Unicode characters; combining sequences are forbidden to keep literals canonically consistent.
- Script-specific textual joiners are not applicable because Croatian Latin writing uses none.
- Write left-to-right; embedded technical and numeric tokens remain left-to-right without manual direction controls.
- Apply topic 6 because the script distinguishes uppercase and lowercase letters.
- Follow the modern Croatian orthographic standard used in Croatia.
- Separate words and embedded technical or numeric tokens with one ordinary space.
- Form established compounds as one word; otherwise use the standard Croatian hyphenated or spaced form required by the compound's grammatical relation.

## 2. Numerals in literals

- Use European Arabic digits `0–9` for technical values in translated prose.
- Use a decimal comma and a grouping point in Croatian prose numbers.
- Keep formulas, source examples, fixed defaults, named mathematical constants, standards constants, and numeric format masks exactly in source form because their spelling is technical data.
- Keep a literal digit with its Croatian ordinal or index ending according to Croatian orthography; distinguish an index comma from a decimal comma by the source concept and retain the index notation.

## 3. Punctuation and quotation

- Use Croatian quotation marks `„…”` for quoted prose; retain straight source quotes around embedded technical tokens for literal fidelity.
- Use the source-shaped comma, question mark, and exclamation mark because Croatian requires no distinct replacements or opening marks.
- Place no space before a colon, semicolon, comma, or terminal mark and one space after it where text follows.
- Use the ellipsis character `…` in prose; retain a source dash when it marks a fixed technical range or token.
- End full dialog and message sentences with the required terminal mark; omit it from short labels and fragments.
- Keep punctuation inside identifiers, format specifiers, filenames, formulas, and other technical runs in source form.

## 4. Never-translate tokens

Treat every retained token as a zero-failure invariant: preserve its characters, case, punctuation, and order; never translate or transliterate it.

- Keep NEC2 card mnemonics verbatim: `GW GA GH EX LD FR RP GE EN SP SM SC NE NH NT TL GN GD GC GX GR GS GM GF CP CM SY XQ EK KH PQ PT`.
- Keep standard unit symbols verbatim, including `Hz`, `kHz`, `MHz`, `GHz`, `dB`, `dBi`, `Ω`, `W`, `K`, `S/m`, `deg`, and `%`.
- Keep figure-of-merit and parameter tokens verbatim, including `VSWR`, S-parameter designators, `Z`, `Z0`, front/back-ratio tokens, and gain/temperature tokens.
- Keep literal file extensions and `printf`-style format specifiers verbatim.
- Keep identifier, function, variable, and configuration-key names embedded in developer strings verbatim because they name program objects.
- Keep physical and mathematical symbol letters verbatim, including electric-field, magnetic-field, angle, and exponent symbols.
- Keep proper product, library, toolkit, program, chart, and named-algorithm names verbatim; translate the descriptive `Power` transfer-family name as specified in topic 10.
- Translate conditional loanwords file-wide as follows: `segment` → `segment`, `patch` → `zakrpa`, and process `fork` → `račvanje`; retain `tag` verbatim.
- Translate other geometry and domain words unless this section explicitly retains them.

## 5. Format-specifier integrity

Treat format-specifier preservation as a zero-failure invariant.

- Preserve exactly the source set and count of format specifiers.
- Keep source order by default; reorder only with explicit positional specifiers already present in the source.
- Restructure Croatian wording around fixed specifier positions when natural word order differs.
- Never localize digits or punctuation inside a specifier.

## 6. Capitalization and title-case

- Use sentence case for labels, menu items, and titles; do not capitalize every content word.
- Keep axis designators uppercase when they name axes; retain a lowercase mathematical or coordinate variable when its source token is lowercase.
- Capitalize each element of a coordinated option name only when each element is independently a proper name or retained acronym.
- Write the generic card noun lowercase except at sentence start; retain acronym and proper-name casing.

## 7. Interface register by string type

Use Croatian interface grammar rather than spoken-language or source-language order.

- Commands, buttons, and menu actions use the shortest complete second-person singular imperative, omit the subject, place the verb first, and follow it with the object or complement.
- Field labels use a concise noun phrase in normal modifier-head order and retain a source colon.
- Dialogs and confirmations use full formal second-person plural sentences with natural subject omission and Croatian subject-verb-object information order; confirmations use interrogative mood.
- Tooltips use complete concise explanatory clauses; when the source explains a disabled control, preserve the reason after the unavailable action or state.
- Status and error messages use impersonal declarative sentences, placing the affected entity before new diagnostic information when natural.
- Keep commands terse and entities as natural noun phrases; never omit meaning, abbreviate unnaturally, or imitate source length.
- Name user-visible domain entities as generic noun plus retained proper designator where applicable; personal and proper names follow topic 8.
- Apply topic 24 to developer and diagnostic strings.

## 8. Formality and address

- Use formal second-person plural for dialogs and informational prose; prohibit informal singular address there.
- Use established short singular imperatives for commands because Croatian interface convention carries neutrality through the command construction.
- Omit the subject pronoun because verb morphology identifies the addressee; include `Vi` only when the source emphasizes the addressee, capitalized as formal address.
- Avoid gendered reference to the user through impersonal or second-person constructions; when agreement is unavoidable, use the natural masculine generic form without excluding any user.
- Honorifics are not applicable because the interface does not address users by title.
- Use second person only for direct instructions; use Croatian personal-name order, given name then family name, when a source literal names a person.
- Express formality with second-person-plural verb morphology, commonly `-te` or `-ite`.
- Keep commands terse-imperative and dialogs full-formal-sentence; form confirmations as `Jeste li sigurni…?`.
- Prohibit casual, slang, archaic, commercial, and ceremonially over-formal registers.

## 9. Accelerator/hotkey mnemonics

- Place `_` immediately before the mnemonic letter inside the translated literal.
- Choose the mnemonic from the translated Croatian term, never from a source transliteration.
- A separate parenthetical mnemonic letter is not applicable because Croatian uses Latin script.
- Preserve mnemonic presence exactly: never add one when the source has none and never drop one when present.
- Prefer an easily typed letter without a diacritic when the same translated term provides one.

## 10. Domain lexicon

Locked term per concept; reuse below rather than coining a synonym.

### Electrical primitives

- `current` → `struja` - electrical Amperes; not the temporal "sadašnji/nedavni" sense.
- `charge` → `naboj` - electrical Coulombs; not `naplata`/billing.
- `voltage` → `napon`.
- `power (electrical)` → `snaga` - watts, power gain, and power flow; distinct from the `Power` transfer-family label `potencijska`, preventing confusion between an electrical quantity and a mathematical power law.
- `impedance` → `impedancija` - complex Z, distinct from resistance/reactance.
- `resistance` → `otpor` - real part of Z; distinct from `impedancija` and from `opterećenje` (load).
- `reactance` → `reaktancija` - imaginary part of Z.
- `inductance` → `induktivitet`; `capacitance` → `kapacitet`.
- `conductivity` → `vodljivost`.
- `admittance` → `admitancija` - distinct from impedance.
- `load` → `opterećenje` - LD-card impedance load; not physical weight; kept distinct from `naboj` (charge).
- `gain` → `dobitak` - antenna directivity ratio, dB; not `dobit` (profit), not `pojačanje` (amplifier amplification).
- `excitation` → `pobuda` - EM energy input/source (eg `Vrsta pobude`); not `uzbuđenje` (emotional excitement).
- `feedpoint` → `napojna točka`.
- `port` → `priključak` - excitation/S-parameter port.
- `radials` → `radijali` (noun) - ground-plane radial wires; distinct from the adjective `radijalni`.

### Ground and earth

- `ground` / `ground plane` → `uzemljenje` - single term across GN/GD ground cards, ground type, conductivity, effects, and model; not soil.
- `earth (physical medium)` → `tlo` - terrain/noise-model earth, "ispod tla" geometry; distinct from `uzemljenje`.
- `ground wave` → `prizemni val` - distinct from the `uzemljenje` reference sense.

### Geometry primitives

- `wire` → `žica` - thin conductor/GW element; not `kabel`; unify every wire sub-use.
- `segment` → `segment`.
- `patch` → `zakrpa` - locked translation, per topic 4.
- `tag` → `tag` - locked untranslated loanword, per topic 4; not a UI label or a card.
- `card` → `kartica` - NEC2 input record; register in topic 18.
- `kernel` → `jezgra` - integral-equation/thin-wire kernel; not an OS kernel.
- `cliff` → `litica` - two-medium ground-boundary type; not a fracture/break.
- `structure` → `struktura` - the antenna model geometry; not `izgradnja` (construction).
- `model` → `model`.
- `geometry` → `geometrija`.
- `crossed` → `unakrsno` - transmission-line conductors crossed/reversed; not `prerezano` (cut/severed).

### Field, pattern, viewer

- `field (EM)` → `polje` - near/total/E/H field; accepted homonym with the UI/config `polje` (data field), disambiguated by context per topic 11.
- `near field` / `far field` → `bliže polje` / `daleko polje` - opposed pair, kept symmetric.
- `far-field contribution` → `prilog dalekog polja` - per-direction contribution; not the near-field animation.
- `radiation` → `zračenje`.
- `radiation pattern` → `uzorak zračenja` - plotted directional response; not a template/design.
- `gain pattern` → `uzorak dobitka`.
- `polarization` → `polarizacija` - antenna/wave field orientation.
- `polarity` → `polaritet` - sign of a quantity; false friend of `polarizacija`.
- `phase` → `faza`; `reference phase` → `referentna faza`; `frequency` → `frekvencija`; `wave` / `wavelength` → `val` / `valna duljina`.
- `standing wave` / `traveling wave` → `stojni val` / `putujući val` - opposed pair.
- `node` / `antinode` → `čvor` / `trbuh` - standing-wave zero/maximum; also the null/peak overlay sense.
- `crest` → `greben` - instantaneous wave apex, comet-head; distinct from `vrh` (curve/step peak).
- `magnitude` → `iznos` - modulus of a quantity (|Z|, scalar); distinct from `amplituda`.
- `amplitude` → `amplituda` - oscillating-quantity peak; distinct from `iznos`.
- `peak value` vs `peak magnitude` → `vršna vrijednost` vs `vršni iznos` - two distinct UI options, never collapsed to one label.
- `instantaneous` → `trenutačni` - projection mode; add the "(φ=0)" qualifier only where the source carries it.
- `Poynting vector` → `Poyntingov vektor`; `solid angle` → `prostorni kut`.
- `net gain` → `neto dobitak` - total-minus-mismatch gain; not "stvarni dio dobitka".
- `viewer` → `preglednik` - observation direction and the 3D view widget; not `promatrač`/`govornik` (observer/speaker) or a preview.
- `flow` / `flow direction` → `tok` / `smjer toka` - patch/current flow.
- `total field` → `ukupno polje`.

### Color, tone, animation subsystem

- `color` → `boja`.
- `color projection` → `projekcija boje` - which quantity drives hue.
- `hue` → `nijansa` - color-wheel angle.
- `brightness` → `svjetlina` - luminance channel.
- `hue encoding` / `brightness encoding` → `kodiranje nijanse` / `kodiranje svjetline` - distinct internal enums; neither collapses to `projekcija boje`.
- `color scale` → `skala boje` - magnitude-to-color scale.
- `scale family` / `color tone` → `obitelj skale` - one concept under both source spellings; family labels are `Potencijska`, `Log`, `Asinh`, `μ-law`, `Reinhard`, `Sigmoid` and `Identity`, keeping the mathematical `Power` family distinct from electrical `snaga`.
- `palette` / `palette kind` → `paleta` / `vrsta palete` - distinct from scale family and color projection.
- `ramp` / `gradient` → `gradijent` - a palette kind/linear color strip.
- `gamma` → `gama` - power-law exponent.
- `knee` → `koljeno` - soft-knee bend point.
- `softening` → `ublažavanje`; `compression` → `kompresija` (dynamic-range); `contrast` → `kontrast`; `dynamic range` → `dinamički raspon`.
- `floor` → `donja granica` - minimum/lower clamp (brightness/dB floor); not a room floor.
- `envelope` → `omotnica` - magnitude/amplitude envelope.
- `comet` → `komet` - moving-crest overlay effect; not `geometrija`, a known fuzzy-inheritance hazard.
- `overlay (noun)` → `preklop` - an added visual layer; distinct from the verb `preklopiti`.
- `animate` / `animation` → `animirati` / `animacija`.
- `animated` / `static` → `animirano` / `statično` - category-header adjectives.
- `projection` → `projekcija` - color or geometry projection.
- `scale` → `skala` (noun) / `skalirati` (verb).
- `wireframe` → `žičani prikaz`.
- `identity` → `identitet` - no-op/passthrough transfer; distinct from `jedinica` (Smith-chart unity).
- `sentinel` → `stražarska vrijednost` - unreachable-case guard value.
- `bins` → `koševi` - discretization buckets.
- `companding` → `kompandiranje` - bounded log curve, μ-law.
- `tone mapping` → `mapiranje tonova` - photographic tone-map.

### Render and compute

- `renderer` → `iscrtavač` - drawing backend; not "render engine".
- `shader` → `shader` - kept, established computing loanword.
- `allocation (memory)` / `managed allocator` → `alokacija` / `upravljeni alokator`.
- `thread` → `dretva` - compute thread; homonym risk with `žica` (wire) avoided by using `dretva`, never `nit`/`niti` (`niti` also serves as the conjunction "neither/nor"; leave that use alone).
- `widget` → `element sučelja` - UI element.
- `validation` → `validacija` - the validation-tree feature; distinct from `provjera` (verification checks).
- `batch mode` → `skupni način` - matches UI `Skupno`, never `batch način`; the literal `--batch` flag name stays untranslated when named as a flag.
- `fork (process)` → `račvanje`/`račvati` - never `forkanje`.
- `deadlock` → `zastoj`; `notifier` → `obavjestitelj`.
- `token` / `operand` / `operator` / `arity` → `token` / `operand` / `operator` / `aritet` - expression-parser terms.
- `override` → `nadjačati` - supersede a value (SY symbol); not `prebrisati` (overwrite).
- `swap` → `zamijeniti`.
- `theme` → `tema` - UI/color theme; accepted homonym with "topic/subject" resolved by context, per topic 11.

### Metrics and miscellaneous

- `noise` / `noise temperature` → `šum` / `temperatura šuma` - electronic/thermal noise; not `buka` (acoustic).
- `efficiency` → `učinkovitost`; `interpolation` → `interpolacija`.
- `mnemonic` → `mnemonik` - a card's code descriptor; not `bilješka` (memo/note).
- `degrees` / `deg` → `stupnjevi` freestanding vs `(deg)` parenthetical unit tag kept untranslated like other unit tags.
- `diameter` → `promjer` - locked native term, no loanword variant.
- `reflect` → three distinct senses: `zrcaliti` (geometry mirror operation), `prati` (behavioral "mirrors ..." tracking), `reflektirati`/`odraz` (physics reflection).
- `default(s)` → `zadano`/`zadane vrijednosti` - established, eg `Poništi na _zadane vrijednosti`.
- `normalize` / `normalization` → `normalizirati` / `normalizacija` - translated native-integrated form, not transliterated.

## 11. Disambiguation policy

- Select the technical sense established in topic 10 for every ambiguous source term.
- Do not add a qualifier when source context already distinguishes the concept; add the shortest Croatian qualifier only when the unqualified target would remain ambiguous.
- Accept `polje` for both electromagnetic and data fields and `tema` for both interface theme and subject because context separates each homonym.
- Use `skaliranje` for the act and `skala` for the noun, keeping gerund and object senses distinct.

## 12. Cross-catalog consistency

- Use one canonical term per concept throughout the catalog; replace minority spellings and synonyms with topic 10's term.
- Keep these pairs distinct: `polaritet` / `polarizacija`; `iznos` / `amplituda`; `vršna vrijednost` / `vršni iznos`; `uzemljenje` / `tlo`; `opterećenje` / `naboj`; `dobitak` / `pojačanje` / `dobit`; `struja` / `sadašnji`; `naboj` / `naplata`; `žica` / `kabel` / `dretva`; `uzorak zračenja` / `predložak` / `daleko polje`; `pobuda` / `uzbuđenje`; `čvor` and `trbuh` / `nula`; `obitelj skale` / `nijansa` / `vrsta palete` / `projekcija boje`; `komet` / `geometrija`; `identitet` / `jedinica`; `iscrtavač` / `pogonski sustav iscrtavanja`; `nadjačati` / `prebrisati`; `preglednik` / `promatrač` / `govornik` / `pretpregled`; `zrcaliti` / `pratiti` / `reflektirati`; `struktura` / `izgradnja`; `tema` / `predmet`; `validacija` / `provjera`; `neto dobitak` / `dobitak stvarnog dijela`; `snaga` / `potencijska`.
- Lock native or integrated forms `dretva`, `knjižnica`, `račvanje`, and `skupni način`; do not alternate with competing loan forms.
- Retain canonical `tag` despite a native alternative because file-wide consistency outranks locale preference for this identifier concept.

## 13. Priority ordering

- Resolve conflicts in this order: correct technical meaning, Croatian interface convention, catalog consistency, disambiguation, then literal numeral form.
- Use short singular imperatives for command literals despite the formal-plural dialog register because string-type convention has higher priority than address uniformity.
- Use `tag` consistently despite native-word preference because catalog consistency outranks that lower preference.

## 14. Grammatical number

- Use Croatian singular, paucal, and plural forms with matching noun, adjective, participle, and verb morphology.
- After counts ending in `1` except `11`, use nominative singular; after `2–4` except `12–14`, use genitive singular paucal; otherwise use genitive plural.
- The analytic or uninflected rule is not applicable because Croatian marks grammatical number.

## 15. Grammatical agreement

- Make adjectives and participles agree with the head noun in gender, number, and case, including labels with an implied head.
- Apply the declension selected in topic 14 after explicit counts and use the corresponding agreement form.
- Retain required reflexive `se`, eg `elementi se hlade`.
- Resolve a standalone label's gender from its implied noun, eg feminine agreement with `zakrpa`.

## 16. Morphological derivation

- Integrate borrowed technical verbs with established Croatian derivation, commonly `-irati`; prohibit ad hoc competing suffixes for the same stem.
- Form verbal nouns with the established `-acija` or `-anje` pattern according to the verb class.
- Prefer an established native stem and native compounding over a new loanword; retain only loanwords locked in topics 4 and 10.

## 17. Preposition and sandhi selection

- Use `sa` instead of `s` before `s`, `š`, `z`, `ž`, and consonant clusters that make `s` difficult to pronounce.
- Further elision and contraction are not applicable in Croatian interface literals.

## 18. Card/record-label register

- Use the fixed title designator `Kartica` with the retained mnemonic, eg `Kartica GW`.
- In running prose, use inflected lowercase `kartica` with the mnemonic in natural Croatian order.
- No separate short and long card-noun forms apply.
- Keep title and running-prose forms within their own registers; do not cross-convert them.

## 19. Multi-paragraph and whitespace fidelity

Treat source semantic whitespace as a zero-failure invariant.

- Mirror source paragraph breaks at the same positions, preserving blank-line and single-line distinctions.
- Remove clauses absent from the current source rather than retaining stale translated text.
- Carry trailing newlines and punctuation mechanically from the source.
- Preserve semantic line breaks; do not introduce line breaks solely for visual wrapping.
- Preserve complete meaning without truncation or unnatural abbreviation based on an assumed display limit.

## 20. Current-source fidelity

- Derive every translation from the current source literal and supplied context.
- Reuse inherited text only when its complete meaning matches the current source.
- Treat prior translations of `field`, `comet`, `power`, `ground`, `viewer`, and `radiation pattern` as unsafe until their topic 10 sense matches the current source.

## 21. Script hygiene

Treat script hygiene as a zero-failure invariant.

- Forbid bare-letter substitutions for `č ć đ š ž`, including `c c dj s z`, except inside a retained token whose literal spelling requires those characters.
- Translate ordinary source words; retain only genuine identifiers, units, symbols, and names enumerated in topic 4.
- Cross-script adjacency rules are not applicable because translated Croatian prose uses one script.

## 22. Rule-file scope hygiene

- Include only current decisions that can alter wording, Unicode characters, punctuation, capitalization, embedded tokens, mnemonic markers, or semantic whitespace in a translated literal.
- Omit catalog representation, serialization, headers, flags, workflow, rendering, fonts, shaping, layout, widget behavior, runtime formatting, sorting, search, display sizing, audit history, completion state, review metadata, provenance, and citations.
- State each retained decision directly; include no implementation procedure, bibliography, correction history, or alternative policy source.

## 23. Section-disjointness declaration

- Topic 1 governs script, code points, orthography, direction, spacing, and compounds.
- Topics 2–7, 9, and 14–21 govern literal phrasing, grammar, punctuation, tokens, and semantic whitespace.
- Topic 8 alone governs address, formality, person, honorifics, and inclusive reference.
- Keep these axes non-overlapping so each decision has one authority.

## 24. Developer/debug-string policy

- Translate all user-facing commands, labels, dialogs, tooltips, status messages, and errors; translation priority never permits an applicable user-facing literal to remain untranslated.
- Translate informational diagnostic strings in terse technical Croatian.
- Keep developer-facing and low-priority debug strings in the source language unless an established sibling family in the same subsystem is already translated; then follow that family consistently.
- Preserve every identifier, function name, format specifier, unit, and retained token inside every string family regardless of translation policy.
- Classify commands, labels, dialogs, tooltips, status messages, and user errors as user-facing; explanatory diagnostics as informational; internal traces, assertions, and code-oriented diagnostics as developer-facing.
- Render the `BUG:` diagnostic prefix as "DEFEKT:", held distinct from the "GREŠKA:" prefix that carries ordinary failures; use one spelling catalog-wide and never alternate with "POGREŠKA:".
- Keep an identifier-plus-value diagnostic pair contiguous (eg `sweep_state=%d`); a qualifier precedes the whole pair and never stands between the identifier and its `=`.
