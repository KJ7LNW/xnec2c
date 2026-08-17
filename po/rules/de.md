# de translation rules

## 1. Script and orthography
- Govern German for Germany in Latin script under current post-1996 standard orthography for professional electrical and RF engineers, antenna designers, and amateur-radio operators.
- Use Ä, Ö, Ü, ä, ö, ü, and ß where orthography requires them; preserve their diacritics and never fold them to look-alikes or replace ß with "ss".
- Use precomposed code points for umlauts and ß; this keeps canonically variable spellings uniform.
- German prose has no required textual joiner or ambiguous apostrophe code point; use straight ASCII quotes only under §3.
- Write left-to-right; no bidirectional handling applies.
- Preserve upper/lowercase distinctions and apply §6 to translated literals.
- Separate words with one space and use the same spacing next to embedded numeric or retained tokens (eg "50 MHz").
- Form closed compounds by default; use a hyphen where a retained token or proper noun joins a German noun (eg "NEC2-Modell"); use separate words only where German grammar requires them.

## 2. Numerals in literals
- Use digits 0-9 for literal technical values; this preserves established German engineering notation.
- Use a comma as the decimal separator (eg "50,0") and a period as the thousands separator (eg "10.000") in translated prose.
- Keep formulas, examples declared verbatim by the source, fixed defaults, and named mathematical or standards constants in source form; this preserves their defined identity.
- Form prose ordinals with a digit plus period (eg "3."); keep card and segment index separators distinct from the decimal comma.

## 3. Punctuation and quotation
- Quotation marks: keep straight ASCII quotes `'...'` for widget-name quoting throughout the catalog; do not introduce „…" typographic quotes, for catalog consistency.
- Native punctuation replacing source counterparts in prose: none required beyond quotes; comma, question mark, exclamation mark, opening marks match source usage.
- Spacing around colon/semicolon/terminal punctuation: no space before, one space after, as in English.
- Use the source ellipsis form and preserve source dash characters; this maintains punctuation identity without introducing a competing form.
- Sentence-terminator policy: full sentences (dialogs, confirmations, status/error messages) take a period; short labels, buttons, and field names omit it.
- Punctuation inside embedded technical runs (file paths, format specifiers, card mnemonics) stays in source form.

## 4. Never-translate tokens
- NEC2 card mnemonics kept verbatim: core `GW GA GH EX LD FR RP GE EN`; extended `SP SM SC NE NH NT TL GN GD GC GX GR GS GM GF CP CM SY XQ EK KH PQ PT`.
- Unit symbols kept verbatim: MHz (frequency), dBi (gain), Ω (impedance magnitude), K (temperature), S/m (conductivity), % (power ratio/percent), deg (angle).
- Figure-of-merit/parameter tokens kept verbatim: VSWR, S-Parameter, Z, Z0 (reference impedance), F/B symbol form (prose form is V/R, §10).
- File extensions kept verbatim: .nec, .csv, .s1p, .s2p, .png.
- Format specifiers: covered in §5.
- Embedded identifiers (function/variable/config-key names in developer strings) kept verbatim.
- Physical/mathematical symbol letters kept verbatim: E, H, I, V, Z, φ, θ.
- Product/library/tool names kept verbatim: NEC2, xnec2c, Smith-Diagramm proper name, any numerics/graphics library name.
- Named transfer-function/algorithm terms kept verbatim (Log, Asinh, μ-law, Reinhard, Sigmoid, Identity); exception: the descriptive family name "Power" translates to "Potenz" (§10).
- Conditional loanwords: Segment, Patch, Tag kept as-is (identical spelling, established German technical usage); "fork" (process) kept as "Fork".
- Directionality: not applicable, German text is left-to-right only (§1).

## 5. Format-specifier integrity
- Preserve every specifier from the source string; same set, same conversion types.
- Default ordering follows source order; reorder only with explicit positional specifiers (`%1$s`) when German word order requires it.
- Prefer restructuring the sentence around fixed specifier positions over reordering specifiers.
- Numbers produced by specifiers are never localized (no comma-for-decimal substitution inside runtime output, §2).

## 6. Capitalization and title-case
- All German nouns capitalized, including inside compound technical nouns (Strahlungsdiagramm, Ladungsverteilung, Speisepunkt).
- Menu items, buttons, and labels use noun/infinitive phrases with normal German capitalization (first word and embedded nouns capitalized); never English-style capitalize-every-word.
- Axis-letter casing: keep source casing for axis letters (X, Y, Z) and lowercase math/coordinate variables (x, y, z) unchanged; German capitalization rules do not override these.
- Coordinated option names: no per-element capitalization beyond normal noun capitalization.
- Generic card/record noun ("Karte"): lowercase in running prose, capitalized only at sentence start; acronyms and proper nouns (NEC2, VSWR) keep source casing.

## 7. Interface register by string type
Treat each string family as its own German interface grammar; preserve all source meaning in the shortest complete natural form.
- Commands, buttons, and menu actions use an infinitive or deverbal noun without an explicit subject; place complements before the final infinitive where German syntax requires it.
- Field labels use a concise noun phrase in normal German modifier-head order and retain a source colon.
- Dialogs and confirmations use full formal sentences with finite verb placement, explicit information order, and address under §8.
- Tooltips use complete declarative sentences; preserve the source reason that an unavailable control is disabled.
- Status and error messages use impersonal declarative or passive clauses, put the affected entity before the result where natural, and omit first-person narration.
- Name user-visible domain entities with the canonical §10 term as the compound head; attach retained identifiers under §1.
- Prefer closed compounds for labels and multiword clauses for explanations; omit no meaning and invent no abbreviation to imitate source length.
- Apply §24 to developer and debug strings.

## 8. Formality and address
- Use formal "Sie" in dialogs whenever address is required; inflect the finite verb for third-person plural formal agreement and never use informal "du".
- Use subjectless infinitives or noun phrases for commands and labels; use impersonal or passive status and error messages.
- Preserve each referenced noun's grammatical gender and number while choosing natural inclusive role nouns or neutral constructions where people are mentioned.
- Use no honorific unless the source names one; use second-person formal address only in dialogs and confirmations.
- Preserve a person's supplied name order; German interface grammar imposes no alternate personal-name order.
- Form confirmations as complete formal questions, placing "wirklich" next to the action when confirmation emphasis is present.
- Exclude casual, slang, commercial, archaic, and ceremonially over-formal register; this maintains a neutral professional voice.

## 9. Accelerator/hotkey mnemonics
- Mnemonic marker: underscore before the mnemonic letter in the source string (eg `_Speichern`), same convention in the translation.
- Choose a mnemonic letter from the translated term and move it within that term to avoid a collision in the same menu or dialog; this preserves local uniqueness.
- Separate parenthetical mnemonic presentation is not applicable because German uses Latin script.
- Preserve source presence: never add a mnemonic to a literal whose source has none.
- Choose a directly typable letter and avoid ß or an umlaut as the mnemonic; this keeps keyboard access reliable.

## 10. Domain lexicon

Concept keys are English source terms; the German term is the only language-specific content per row.

### Electrical primitives
- `current` - Strom; electrical current, Ampere; not temporal "aktuell/gegenwärtig".
- `charge` - Ladung; electrical charge, Coulomb; not billing (Rechnung/Gebühr).
- `voltage` - Spannung; electric potential.
- `power (electrical)` - Leistung; radiated/dissipated watts, power gain, power flow; distinct from "Potenz" (math power-law family) and from "Gewinn" (antenna gain).
- `impedance` - Impedanz; complex Z; distinct from Widerstand and Reaktanz.
- `resistance` - Widerstand; real part of Z; distinct from Impedanz and Last.
- `reactance` - Reaktanz; imaginary part of Z.
- `inductance` - Induktivität.
- `capacitance` - Kapazität.
- `conductivity` - Leitfähigkeit; material S/m.
- `admittance` - Admittanz; admittance-matrix sense; distinct from Impedanz.
- `load` - Last; LD-card impedance load; not physical weight; not homonymous with Ladung in German, no collision.
- `gain` - Gewinn; antenna directivity ratio (dB); not Profit/Gewinnspanne, not Verstärkung (amplifier).
- `excitation` - Anregung; EM energy input/source; not emotional Erregung/Aufregung.
- `feedpoint` - Speisepunkt; antenna feed point.
- `port` - Port; excitation/S-parameter port; loanword kept.
- `radials` - Radial (pl. Radials); horizontal ground-plane radial wire, noun; distinct from the adjective "radial".

### Ground and earth
- `ground` / `ground plane` - Masse / Masseebene; RF electrical reference and reference plane across GN/GD cards, conductivity, effects, and models; excludes physical terrain and locks one term for each sub-sense.
- `earth (physical medium)` - Erdreich; terrain and noise-model medium and below-ground geometry; distinct from electrical Masse.
- `ground wave` - Bodenwelle; propagation term; distinct from the Masse reference.

### Geometry primitives
- `wire` - Draht; thin conductor/GW element; not Kabel.
- `segment` - Segment; NEC2 geometry subdivision.
- `patch` - Patch; NEC2 surface patch (SP/SM); kept as loanword, one decision file-wide.
- `tag` - Tag; NEC2 geometry identifier; not a UI label or a card.
- `card` - Karte; NEC2 input record; register in §18.
- `kernel` - Kernel; integral-equation/thin-wire kernel; not an OS kernel.
- `cliff` - Geländesprung; two-medium ground-boundary type; not a fracture/break.
- `structure` - Struktur; the antenna model geometry; not "Konstruktion".
- `model` - Modell; NEC model or noise-temperature model.
- `geometry` - Geometrie; the model geometry.
- `crossed` - gekreuzt; transmission-line conductors crossed/reversed; not cut/severed.

### Field, pattern, viewer
- `field (EM)` - Feld; near/total/E/H field; distinct from a UI data field (Eingabefeld).
- `near field` / `far field` - Nahfeld / Fernfeld; opposed spatial regions, kept symmetric.
- `far-field contribution` - Fernfeldbeitrag; per-direction contribution; not near-field animation.
- `radiation` - Abstrahlung; radiated emission.
- `radiation pattern` - Strahlungsdiagramm; plotted directional response; not a template/design (Vorlage/Design); one term catalog-wide.
- `gain pattern` - Gewinndiagramm; the gain radiation pattern.
- `polarization` - Polarisation; antenna/wave field orientation.
- `polarity` - Polarität; sign (+/-) of a quantity; false friend of Polarisation.
- `phase` - Phase.
- `reference phase` - Referenzphase.
- `frequency` - Frequenz.
- `wave` / `wavelength` - Welle / Wellenlänge.
- `standing wave` / `traveling wave` - Stehwelle / Wanderwelle; opposed pair; "Stehwellenverhältnis" for spelled-out prose, VSWR acronym stays "VSWR".
- `node` / `antinode` - Knoten / Bäuche; standing-wave zero/maximum; also the null/peak overlay sense.
- `crest` - Wellenkamm; instantaneous wave apex (comet head); distinct from a curve/step peak.
- `magnitude` - Betrag; modulus of a quantity (|Z|, scalar); distinct from Amplitude.
- `amplitude` - Amplitude; oscillating-quantity peak; distinct from Betrag.
- `peak value` vs `peak magnitude` - Spitzenwert vs Spitzenbetrag; two distinct UI options, never collapsed to one label.
- `instantaneous` - Momentanwert; projection mode; add "(φ=0)" only where the source carries it.
- `Poynting vector` - Poynting-Vektor.
- `solid angle` - Raumwinkel.
- `net gain` - Nettogewinn; total-minus-mismatch gain; not "Realteil-Gewinn"; keep "Nettogewinn" intact in longer compounds ("Nettogewinn-Kurven", "Nettogewinn-Diagramm"), never split as "Netto-Gewinn", matching the "_Nettogewinn" plot-toggle widget label.
- `viewer` - Betrachter; observation direction and/or the 3D view widget; not Beobachter/Sprecher/Vorschau.
- `flow` / `flow direction` - Fluss / Flussrichtung; patch/current flow.
- `total field` - Gesamtfeld.

### Color, tone, animation subsystem
- `color` - Farbe.
- `color projection` - Farbprojektion; which quantity drives hue.
- `hue` - Farbton; color-wheel angle.
- `brightness` - Helligkeit; luminance channel.
- `hue encoding` / `brightness encoding` - Farbton-Kodierung / Helligkeits-Kodierung; distinct internal enums, neither collapses to "Farbprojektion".
- `color scale` - Farbskala; magnitude-to-color scale.
- `scale family` / `color tone` - Tonwertfamilie; the transfer-curve family (Power/Log/Asinh/μ-law/Reinhard/Sigmoid/Identity); one concept, two source spellings, one term; avoid "Farbton" here, reserved for hue.
- `palette` / `palette kind` - Palette / Palettenart; palette-layout enum; distinct from Tonwertfamilie and Farbprojektion.
- `ramp` / `gradient` - Rampe / Verlauf; a palette kind / linear color strip.
- `gamma` - Gamma; power-law exponent.
- `knee` - Knie; soft-knee bend point.
- `softening` - Weichheit; tone-mapping softening knee.
- `compression` - Kompression; dynamic-range compression.
- `contrast` - Kontrast.
- `dynamic range` - Dynamikbereich.
- `floor` - Untergrenze; minimum/lower clamp (brightness/dB floor); not a room floor.
- `envelope` - Einhüllende; magnitude/amplitude envelope; envelope null (magnitude minimum) is "Nullstelle", distinct from "Knoten" (node/antinode overlay).
- `comet` - Komet / Kometenkopf; moving-crest overlay effect; not geometry.
- `overlay (noun)` - Overlay; an added visual layer; distinct from the verb "überlagern".
- `animate` / `animation` - animieren / Animation.
- `animated` / `static` - animiert / statisch; category-header adjectives (dynamic vs phase-invariant).
- `projection` - Projektion; color or geometry projection.
- `scale` - skalieren (verb) / Skala (noun).
- `wireframe` - Drahtgitter; wire-mesh render mode.
- `identity` - Identität; no-op/passthrough transfer; distinct from Eins/Unity (Smith-chart normalization, §12).
- `sentinel` - Sentinel; unreachable-case guard value.
- `bins` - Bins; discretization buckets; loanword kept.
- `companding` - Companding; bounded log curve (μ-law); loanword kept.
- `tone mapping` - Tone-Mapping; photographic tone-map; loanword kept.

### Render and compute
- `renderer` - Renderer; drawing backend; not "Render-Engine".
- `shader` - Shader.
- `allocation (memory)` / `managed allocator` - Allokation / verwalteter Allokator; allocation and the allocator/report.
- `thread` - Thread; compute thread; no homonym collision with "Draht" (wire) in German.
- `widget` - Widget; UI element; loanword kept.
- `validation` - Validierung; the validation-tree feature; distinct from Überprüfung (verification checks).
- `batch mode` - Batch-Modus.
- `fork (process)` - Fork; process fork; kept verbatim (§4).
- `deadlock` - Deadlock.
- `notifier` - Notifier.
- `token` / `operand` / `operator` / `arity` - Token / Operand / Operator / Stelligkeit; expression-parser terms.
- `override` - Übersteuern; supersede a value (SY symbol); not "Überschreiben" (file overwrite, §12).
- `swap` - Tausch / Vertauschen; exchange.
- `theme` - Theme; UI/color theme; not "Thema" (topic/subject).

### Metrics and miscellaneous
- `noise` / `noise temperature` - Rauschen / Rauschtemperatur; electronic/thermal noise; not acoustic Lärm.
- `efficiency` - Wirkungsgrad.
- `interpolation` - Interpolation.
- `mnemonic` - Mnemonic; a card's code descriptor; not a memo/note (Notiz).
- `degrees` / `deg` - Grad / "(deg)"; freestanding axis/prose "Grad" vs the parenthetical unit tag "(deg)"; treat the tag like other unit tags (§4).
- `diameter` - Durchmesser; canonical native term chosen over the loanword "Diameter"; one choice file-wide.
- `reflect` - spiegeln (geometry mirror operation) / folgt (behavioral "mirrors …", a control tracking another) / reflektieren (physics reflection); three distinct senses.
- `default(s)` - Standard / Vorgabe; fallback value.
- `normalize` / `normalization` - normalisieren / Normalisierung; translated, not transliterated, since a native form exists.

## 11. Disambiguation policy
- Choose the §10 technical sense for every ambiguous source concept; this prevents general-language false friends.
- Add no qualifier when program context identifies the sense; add one only when the translated literal itself would otherwise remain ambiguous.
- German uses no accepted intra-domain homonym for the listed concepts and no locative collision; keep Last/Ladung and Draht/Thread distinct.
- Resolve overloaded action and noun senses by grammar and the §10 mapping (eg "scale" as "skalieren" or "Skala").

## 12. Cross-catalog consistency
- One term per concept: reuse the §10 lexicon; never introduce a synonym for a concept already mapped.
- False-friend pairs, each resolved to two distinct German terms, never sharing a translation:
  - Polarität (sign) vs Polarisation (wave/antenna orientation).
  - Betrag (modulus/scalar) vs Amplitude (oscillating peak).
  - Spitzenwert vs Spitzenbetrag.
  - Masse/Masseebene (electrical reference) vs Erdreich (physical terrain).
  - Last (impedance) vs Ladung (electrical charge).
  - Gewinn (directivity) vs Verstärkung (amplifier) vs Profit/Gewinnspanne.
  - Strom (electrical) vs aktuell/gegenwärtig (temporal).
  - Ladung (electrical) vs Rechnung/Gebühr (billing).
  - Draht (conductor) vs Kabel (cable/cord); Draht vs Thread (compute), no collision.
  - Strahlungsdiagramm (plotted response) vs Vorlage/Design; vs Fernfeld.
  - Anregung (EM input) vs Erregung/Aufregung (emotional).
  - Knoten/Bäuche (standing-wave) vs Nullstelle (generic numeric null/envelope minimum).
  - Tonwertfamilie vs Farbton (hue) vs Palettenart vs Farbprojektion - four distinct chroma concepts.
  - Komet (overlay) vs Geometrie - stale-inheritance hazard (§20).
  - Identität (no-op transfer) vs Eins/Unity (Smith-chart normalization).
  - Renderer (backend) vs "Render-Engine" (forbidden phrasing).
  - Übersteuern (supersede) vs Überschreiben (overwrite).
  - Betrachter (observation/3D view) vs Beobachter/Sprecher/Vorschau.
  - spiegeln (mirror op) vs folgt (behavioral tracking) vs reflektieren (physics).
  - Struktur (model geometry) vs Konstruktion.
  - Theme (UI) vs Thema (topic/subject).
  - Validierung (tree feature) vs Überprüfung (checks).
  - Nettogewinn vs Realteil-Gewinn.
  - Leistung (electrical watts) vs Potenz (power-law transfer-family name).
- Lock each loanword/native decision to §10: keep Patch, Tag, Segment, Port, Fork, Widget, Theme, Bins, Companding, and Tone-Mapping; use Masseebene and Durchmesser as the canonical native forms.
- Unify minority spellings, including split "Netto-Gewinn", to the §10 canonical form.
- Let established catalog consistency outrank a competing locale preference only for the locked spellings enumerated here.

## 13. Priority ordering
- Precedence chain: correct meaning, then interface convention (§7-9), then catalog consistency (§12), then disambiguation (§11), then locale numeral form (§2).
- Explicit override: established catalog forms Radiobutton, Combobox, and Radio-Menüeintrag outrank an unestablished native coinage; this preserves recognizable interface terminology without changing the precedence chain.

## 14. Grammatical number
- Use singular and plural; inflect nouns, adjectives, participles, and finite verbs for the grammatical number required by the literal.
- After an explicit count of one, use singular; after zero or any count other than one, use plural.
- Preserve irregular and invariant technical plurals in their established German forms rather than inventing a uniform suffix.

## 15. Grammatical agreement
- Adjectives and participles agree in gender, number, and case with the head noun, including standalone labels whose head noun is implied (eg "Gemessen" agrees with the implied feminine "Last": "gemessene Last").
- Declension-class choices follow standard German weak/strong/mixed adjective declension per article presence; no partitive-after-count rule applies (German has none).
- Reflexive-particle retention: keep "sich" where the German verb is inherently reflexive (eg "sich ändern"), never dropped.
- A standalone label resolves its default gender from the head noun's inherent gender (der Gewinn, die Last, das Muster); use the referenced §10 noun's gender, never a generic default.

## 16. Morphological derivation
- Borrowed technical verbs take the standard weak "-ieren" suffix (skalieren, validieren, normalisieren, animieren); no ad-hoc English-suffix borrowing.
- Verbal-noun formation uses "-ung" (Skalierung, Validierung, Normalisierung).
- Prefer native compounding for nouns; prefer "-ieren" loanword-verb formation over coining a native verb where none is established.

## 17. Preposition and sandhi selection
- Not applicable: German has no context-conditioned sound-based form selection (sandhi).
- Elision/contraction: use standard preposition-article contractions in prose (im, zum, zur, ins, am); use the uncontracted form only where grammar requires it (eg before a proper noun).

## 18. Card/record-label register
- NEC2 card dialog and editor titles use the fixed designator "(XX-Karte)" (eg "Strahlungsdiagramm (RP-Karte)"); this keeps the mnemonic verbatim while translating the record noun.
- Running prose (status/error messages) uses the hyphenated German "XX-Karte" (eg "RP-Karte", "GW-Karte", "Doppelte FR-Karte … übersprungen").
- Generic-noun casing: "Karte" lowercase in running prose per §6; no short-vs-long form distinction beyond the title/prose split above.
- Each register stays internally consistent; never cross-convert one style into the other.

## 19. Multi-paragraph and whitespace fidelity
- Mirror source paragraph breaks at the same positions and preserve whether each break is single-line or blank-line separated.
- Drop clauses absent from the current source instead of carrying stale translated text.
- Preserve trailing newlines, terminal punctuation, and semantic line breaks; add no visual wrapping absent from the literal.
- Preserve complete meaning with natural wording; never truncate or abbreviate for an assumed display limit.

## 20. Current-source fidelity
- Derive each translation from the current source literal and supplied context.
- Reuse inherited text only when its complete meaning agrees with the current source.
- Treat inherited "Komet" outside the overlay concept and "Weichheit" outside the softening parameter as unsafe; remap them through §10.

## 21. Script hygiene
- No distinct-script exclusion applies because ordinary German and retained tokens use the Latin script; exact retained-token spelling remains mandatory.
- Translate plain foreign prose while preserving only the identifiers, units, symbols, and proper technical names enumerated in §4.

## 22. Rule-file scope hygiene
- Keep only decisions that alter wording, Unicode characters, punctuation, capitalization, embedded tokens, mnemonic markers, or semantic whitespace in a translated literal.
- Omit catalog representation, serialization, headers, flags, review workflow, rendering, fonts, layout, widget behavior, runtime formatting, sorting, search, display sizing, process metadata, history, provenance, and citations.
- State the current linguistic decision directly and include no implementation procedure or alternative policy source.

## 23. Section-disjointness declaration
- §§1-3 govern script, symbol, and punctuation mechanics; §§6-7 and §9 govern casing and phrase structure; §8 governs address and social register.
- Keep these axes non-overlapping so each decision has one authority.

## 24. Developer/debug-string policy
- Translate all user-facing controls, dialogs, tooltips, statuses, errors, and informational diagnostics into German; review priority never permits an applicable user-facing literal to remain untranslated.
- Keep developer-only debug diagnostics in source form unless their subsystem already presents that family to users; translated diagnostics use terse technical German.
- Preserve every identifier, function name, format specifier, mnemonic, unit, and retained token inside every string family under §§4-5, independent of translation priority.
- Render the source `BUG:` diagnostic prefix as "PROGRAMMFEHLER:", held distinct from the "FEHLER:" prefix that carries ordinary failures; the two severities never share a word.
- Keep an identifier-plus-value diagnostic pair contiguous (eg "sweep_state=%d"); a qualifier precedes the whole pair and never stands between the identifier and its "=".
