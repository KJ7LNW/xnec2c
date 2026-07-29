# lv translation rules

## 1. Script and orthography

- Latin script with Latvian diacritics: ā, č, ē, ģ, ī, ķ, ļ, ņ, š, ū, ž; mandatory, never stripped or folded to a bare-Latin look-alike.
- Precomposed diacritic forms only, never combining-mark sequences.
- No ambiguous apostrophe/modifier-letter mark: modern Latvian orthography carries palatalization through ķ/ģ/ļ/ņ, not a separate apostrophe glyph; forbid a plain apostrophe as a diacritic substitute.
- No word-part joining/shaping mechanics: Latin non-cursive script, if-applicable, skipped.
- Directionality: left-to-right only; no bidirectional handling required.
- Has letter-case distinction: uppercase/lowercase; consequence carried into topic 6.
- Locale scope: modern standard Latvian for Latvia, written in Latin script, for professional electrical and RF engineers, antenna designers, and amateur-radio operators.
- Inter-word spacing: single space, including between native text and an embedded foreign/numeric token.
- Compound formation: technical compounds form as spaced genitive chains ("krāsu projekcijas saime"), not fused single words; hyphenation is reserved for coordinate/paired terms only.

## 2. Numerals in literals

- Use Western Arabic digits for technical values in translated prose.
- Use a decimal comma and a non-breaking space for thousands grouping in translated prose.
- Keep formulas, fixed examples, fixed defaults, and named mathematical or standards constants in source form.
- Form ordinals and indices as a digit plus period; keep the index period distinct from the decimal comma.

## 3. Punctuation and quotation

- Use Latvian quotation marks „…" for prose; retain plain source quotes around embedded technical tokens for token consistency.
- Use the source glyphs for comma, question mark, and exclamation mark because Latvian has no replacements.
- Place no space before a colon, semicolon, or terminal mark and one space after it.
- Use one ellipsis character; preserve source en and em dashes.
- End full sentences with a period; omit a terminator from short labels and fragments.
- Keep punctuation inside technical runs in source form.

## 4. Never-translate tokens

Treat retained tokens as immutable: admit zero translation, transliteration, character substitution, or internal punctuation change.

- Keep NEC2 card mnemonics verbatim: GW GA GH EX LD FR RP GE EN SP SM SC NE NH NT TL GN GD GC GX GR GS GM GF CP CM SY XQ EK KH PQ PT.
- Keep unit symbols verbatim: MHz, dBi, Ω, dB, W, V, A, %, and (deg).
- Keep figure-of-merit tokens verbatim: VSWR, S11, S21, Z, Z0, F/B, and G/T.
- Keep file extensions verbatim: .nec, .csv, .s1p, .s2p, and .png.
- Keep format specifiers verbatim per topic 5.
- Keep embedded function names, variable names, and configuration keys verbatim.
- Keep physical and mathematical symbol letters verbatim, including E, H, angle letters, and exponent letters.
- Keep product, library, tool, and chart names verbatim: xnec2c, GTK, Cairo, OpenGL, GSL, and Smith.
- Keep named transfer terms verbatim: Log, Asinh, μ-law, Reinhard, Sigmoid, and Identity; translate descriptive Power as Pakāpes likums and electrical power as jauda.
- Apply the topic-10 decision to conditional loanwords: translate patch as ielāps; retain segments, tags, and fork in their fixed forms.
- Apply topic 1 left-to-right direction to every retained token.

## 5. Format-specifier integrity

Treat source specifiers as an exact invariant: admit zero missing, added, altered, or localized specifiers.

- Preserve the same specifier set and count as the source.
- Keep source order by default; use positional forms only when Latvian grammar requires reordered substitutions.
- Restructure the sentence around fixed specifier positions before using positional reordering.
- Keep every number inside a specifier in source form.

## 6. Capitalization and title-case

- Use sentence case for labels, menu items, and titles; capitalize only the first word and proper nouns.
- Axis letters (X, Y, Z) keep source uppercase as physics convention.
- Lowercase math/coordinate variables (x, y, z) stay lowercase; sentence-case does not override them.
- No coordinated-option capitalize-each-element exception exists in this catalog.
- Generic card/record nouns are lowercase mid-sentence, capitalized only at sentence start; acronyms and proper nouns (NEC2, GTK) retain their original casing.
- Script carries full case distinction (topic 1); this topic is not reduced to mnemonic-only handling.

## 7. Interface register by string type

Treat interface strings as concise technical Latvian; preserve all source meaning without imitating source length.

- Commands, buttons, and menu actions use a subjectless infinitive or deverbal noun, with the action before its object; use the shortest complete natural form.
- Field labels use a noun phrase in modifier-before-head order and retain the source colon; omit a subject and finite verb.
- Dialogs and confirmations use full formal sentences with natural subject omission, finite-verb placement, and known information before the requested decision.
- Tooltips use short declarative clauses that name the control effect before conditions; when the source gives a disabled-control reason, state that reason after the unavailable action.
- Status and error messages use impersonal declarative clauses, placing the affected entity before the state or failure; retain terse technical wording.
- User-visible domain entities use a specific modifier before the generic head noun, with mnemonic or proper-name tokens retained in source form.
- Form spaced genitive chains per topic 1; avoid unnatural abbreviations and preserve all semantic distinctions.
- Apply topic 24 to developer and debug strings.

## 8. Formality and address

- Use formal jūs address; omit informal tu address.
- Mark formal address through second-person plural verb endings; omit the subject pronoun unless contrast requires it.
- Use subjectless infinitives for commands and formal questions for dialogs.
- Form confirmations as a full question beginning with vai and a formal finite verb.
- Prefer impersonal or passive wording to avoid unnecessary gender marking; retain natural grammatical gender, number, and animacy when the named noun requires agreement.
- Use inclusive role and person terms; omit wording that assumes a person's gender when the source does not state it.
- Use no honorific unless the source contains one; address users in second-person plural and preserve a person's given-name then family-name order.
- Use neutral professional register; omit casual, slang, commercial, archaic, and ceremonially formal wording.

## 9. Accelerator/hotkey mnemonics

- Mnemonic marker: single `_` immediately before the mnemonic letter.
- Uniqueness is per container (menu/dialog); a collision shifts to a distinctive interior consonant, not the first letter.
- The mnemonic letter is drawn from the translated Latvian term, never a transliteration of the source letter.
- No non-Latin-script parenthetical presentation applies; Latvian is Latin-script throughout.
- A mnemonic is added only where the source string carries one; never invented.
- Widget-type distinction: menu items and buttons carry mnemonics; labels, tooltips, and status text never do.
- Typability: standard Latvian/US keyboard layout supports all diacritic letters directly; prefer a base (non-diacritic) letter as the mnemonic when an equally natural choice exists, to minimize extra keystrokes.

## 10. Domain lexicon

| Concept | Latvian term | Sense / purpose-hazard |
|---|---|---|
| current (electrical) | strāva / strāvas | electrical current, A; never "pašreizējais" (temporal) |
| charge (electrical) | lādiņš / lādiņi | electrical charge, C; never "maksa" (billing) |
| voltage | spriegums | electric potential |
| power (electrical) | jauda | radiated/dissipated W; distinct from "Pakāpes likums" (Power scale-family name) and from "pastiprinājums" (gain) |
| impedance | impedance | complex Z; loanword; distinct from resistance and reactance |
| resistance | pretestība | real part of Z; distinct from impedance and from "slodze" (load) |
| reactance | reaktance | imaginary part of Z |
| inductance | induktivitāte | Locked technical term for correct sense and catalog consistency. |
| capacitance | kapacitāte | Locked technical term for correct sense and catalog consistency. |
| conductivity | vadītspēja | material S/m, native term |
| admittance | admitance | admittance-matrix sense; distinct from impedance |
| load (LD-card) | slodze | impedance load; never "svars" (physical weight) |
| gain | pastiprinājums | antenna directivity ratio, dB; never profit, never amplifier amplification |
| excitation | ierosme | EM energy input/source; never emotional excitement |
| feedpoint | barošanas punkts | antenna feed point |
| port | ports | excitation/S-parameter port; direct loan |
| radials (noun) | radiāli | horizontal ground-plane radial wires; distinct from the adjective "radiāls" |
| ground / ground plane | zeme / zemes plakne | RF reference plane, GN/GD cards; never "augsne" (soil); never "zemējums" (earthing); one term across all ground sub-senses |
| earth (physical medium) | zemes virsma | terrain/noise-model earth, "below ground" geometry; distinct from electrical "zeme" |
| ground wave | zemes vilnis | propagation term; distinct from the "zeme" ground reference |
| wire | vads | thin conductor / GW element; never "kabelis" (cable/cord) |
| segment | segments | NEC2 geometry subdivision; direct loan |
| patch | ielāps | NEC2 surface patch (SP/SM); translate, not keep |
| tag | tags | NEC2 geometry identifier; direct loan; never a UI label or card |
| card | karte | NEC2 input record; register handled in topic 18 |
| kernel | kodols | integral-equation/thin-wire kernel; not an OS kernel |
| cliff | krauja | two-medium ground-boundary type; not a fracture/break |
| structure | struktūra | the antenna model geometry; never "celtniecība" (construction) |
| model | modelis | NEC model or noise-temperature model |
| geometry | ģeometrija | the model geometry |
| crossed | sakrustots | transmission-line conductors crossed/reversed; never "pārgriezts" (cut/severed) |
| field (EM) | lauks | near/total/E/H field; accepted homonym with the UI/config "lauks" (data field), context disambiguates |
| near field | tuvais lauks | opposed to far field |
| far field | tālais lauks | opposed to near field |
| far-field contribution | tālā lauka ieguldījums | per-direction contribution; not near-field animation |
| radiation | starojums | radiated emission |
| radiation pattern | starojuma diagramma | plotted directional response; never "veidne/paraugs" (template/design) |
| gain pattern | pastiprinājuma diagramma | the gain radiation pattern |
| polarization | polarizācija | antenna/wave field orientation; distinct from "polaritāte" |
| polarity | polaritāte | sign (+/-) of a quantity; false friend of polarizācija |
| phase | fāze | Locked technical term for correct sense and catalog consistency. |
| reference phase | atskaites fāze | Locked technical term for correct sense and catalog consistency. |
| frequency | frekvence | Locked technical term for correct sense and catalog consistency. |
| wave | vilnis | Locked technical term for correct sense and catalog consistency. |
| wavelength | viļņa garums | Locked technical term for correct sense and catalog consistency. |
| standing wave | stāvvilnis | opposed to traveling wave |
| traveling wave | skrejvilnis | opposed to standing wave |
| node (standing-wave) | mezgls | standing-wave zero; distinct wording from "nulle" used in the tick-mark tooltip |
| antinode | pretmezgls | standing-wave maximum; distinct wording from "maksimums" used in the tick-mark tooltip |
| crest | viļņa kore | instantaneous wave apex (comet-head); distinct from a curve/step "maksimums" (peak) |
| magnitude | modulis | modulus of a quantity (\|Z\|, scalar); distinct from "amplitūda" |
| amplitude | amplitūda | oscillating-quantity peak; distinct from "modulis" |
| peak value | maksimālā vērtība | distinct UI option from peak magnitude, must not collapse |
| peak magnitude | maksimālais modulis | distinct UI option from peak value, must not collapse |
| instantaneous | momentānais | projection mode; add "(φ=0)" qualifier only where the source carries it |
| Poynting vector | Poringa vektors | Locked technical term for correct sense and catalog consistency. |
| solid angle | telpiskais leņķis | Locked technical term for correct sense and catalog consistency. |
| net gain | neto pastiprinājums | total-minus-mismatch gain; not "reālās daļas pastiprinājums" (real-part gain) |
| viewer | skatītājs | observation direction and/or the 3D view widget; never observer/speaker/preview |
| flow / flow direction | plūsma / plūsmas virziens | patch/current flow |
| total field | kopējais lauks | Locked technical term for correct sense and catalog consistency. |
| color | krāsa | Locked technical term for correct sense and catalog consistency. |
| color projection | krāsu projekcija | which quantity drives hue |
| hue | nokrāsa | color-wheel angle; distinct from "mēroga saime" (scale family/color tone) |
| brightness | spilgtums | luminance channel |
| hue encoding | nokrāsas kodējums | distinct internal enum, never collapses to "krāsu projekcija" |
| brightness encoding | spilgtuma kodējums | distinct internal enum, never collapses to "krāsu projekcija" |
| color scale | krāsu skala | magnitude-to-color scale; distinct from "mērogs" (general scale) and "mēroga saime" (scale family) |
| scale family / color tone | mēroga saime | the transfer-curve family (Power/Log/Asinh/μ-law/Reinhard/Sigmoid/Identity); one concept, two source spellings, one Latvian term; distinct from "nokrāsa" (hue) |
| palette / palette kind | palete / paletes veids | palette-layout enum; distinct from scale family and color projection |
| ramp / gradient | gradients | a palette kind / linear color strip |
| gamma | gamma | power-law exponent; loanword |
| knee | lūzuma punkts | soft-knee bend point |
| softening | mīkstināšana | dynamic-range softening |
| compression | kompresija | dynamic-range compression |
| contrast | kontrasts | Locked technical term for correct sense and catalog consistency. |
| dynamic range | dinamiskais diapazons | Locked technical term for correct sense and catalog consistency. |
| floor | apakšējā robeža | minimum/lower clamp (brightness/dB floor); never a room floor |
| envelope | aploksne | magnitude/amplitude envelope; accepted homonym with postal envelope, context disambiguates |
| comet | komēta / komētas galva | moving-crest overlay effect; "galva" (head), never diminutive "galviņa"; not geometry |
| overlay (noun) | pārklājums | an added visual layer; distinct from the verb "uzklāt" (to overlay) |
| animate | animēt | Locked technical term for correct sense and catalog consistency. |
| animation | animācija | Locked technical term for correct sense and catalog consistency. |
| animated | animēts | category-header adjective, dynamic |
| static | statisks | category-header adjective, phase-invariant |
| projection | projekcija | color or geometry projection |
| scale (verb) | mērogot | Locked technical term for correct sense and catalog consistency. |
| scale (noun) | mērogs | Locked technical term for correct sense and catalog consistency. |
| wireframe | stiepļu karkass | wire-mesh render mode |
| identity | identitāte | no-op/passthrough transfer; distinct from "vienība" (unity, Smith-chart) |
| sentinel | sargvērtība | unreachable-case guard value |
| bins | intervāli | discretization buckets |
| companding | kompandēšana | bounded log curve (μ-law) |
| tone mapping | toņu attēlošana | photographic tone-map; domain context (photographic) separates it from "mēroga saime" |
| renderer | renderētājs | drawing backend; never "render engine" |
| shader | šeiders | loanword |
| allocation (memory) | piešķīrums | Locked technical term for correct sense and catalog consistency. |
| managed allocator | pārvaldītais atmiņas piešķīrējs | allocation and the allocator/report |
| thread | pavediens | compute thread; distinct Latvian word from "vads" (wire), no homonym risk |
| widget | logrīks | UI element |
| validation | validācija | the validation-tree feature; distinct from "verifikācija" (verification checks) |
| batch mode | pakešrežīms | Locked technical term for correct sense and catalog consistency. |
| fork (process) | fork | kept verbatim per topic 4 conditional-loanword decision |
| deadlock | strupceļš | Locked technical term for correct sense and catalog consistency. |
| notifier | paziņotājs | Locked technical term for correct sense and catalog consistency. |
| token (parser) | tokens | expression-parser term, direct loan |
| operand | operands | expression-parser term, direct loan |
| operator | operators | expression-parser term, direct loan |
| arity | aritāte | expression-parser term, direct loan |
| override | aizstāt | supersede a value (SY symbol); distinct from "pārrakstīt" (overwrite) |
| swap | apmainīt | exchange |
| theme (UI) | tēma | UI/color theme; distinct from "temats" (topic/subject) |
| noise / noise temperature | troksnis / trokšņa temperatūra | electronic/thermal noise; accepted homonym, no separate acoustic word in Latvian, context disambiguates |
| efficiency | efektivitāte | Locked technical term for correct sense and catalog consistency. |
| interpolation | interpolācija | Locked technical term for correct sense and catalog consistency. |
| mnemonic | apzīmējums | a card's code descriptor; never "piezīme" (memo/note) |
| degrees / deg | grādi | freestanding axis/prose form; the parenthetical "(deg)" unit tag stays verbatim per topic 4 |
| diameter | diametrs | canonical loanword, one choice, file-wide |
| reflect (geometry) | atspoguļot | geometry mirror operation |
| reflect (behavioral) | atbilst | a control tracking another; distinct from geometry mirroring and physical reflection |
| reflect (physics) | atstarošana | physics reflection, noun; three distinct senses across this row group |
| default(s) | noklusējuma vērtība(s) | fallback value |
| normalize / normalization | normalizēt / normalizācija | translated, not transliterated; native-derived term already exists |

- Every Appendix A concept above is mapped; none collapsed silently and none marked not-applicable, since all concepts occur in the source domain.
- Reuse the terms above verbatim across the catalog; introduce no synonym for a concept already mapped.

## 11. Disambiguation policy

- The correct technical sense is chosen for each ambiguous term from topic 10.
- No qualifier absent from the source is added (eg never "elektriskā strāva" when the source states only "Currents"); the antenna-simulator context already disambiguates.
- A qualifier is added only where the Latvian term would otherwise be genuinely ambiguous.
- Accepted intra-domain homonyms: "lauks" (EM field vs UI/config field); "troksnis" (electronic noise, no separate acoustic-domain word).
- Accepted locative/other homonym: "aploksne" (magnitude/amplitude envelope vs postal envelope).
- Gerund vs noun senses are distinguished naturally by Latvian verb/noun morphology (eg "mērogot" verb vs "mērogs" noun; "animēt" verb vs "animācija" noun); no overload risk.

## 12. Cross-catalog consistency

Treat each source concept as one fixed Latvian term; reuse topic 10 throughout the catalog.

| Source distinction | Fixed Latvian terms | Purpose |
|---|---|---|
| polarity / polarization | polaritāte / polarizācija | Separate quantity sign from field orientation. |
| magnitude / amplitude | modulis / amplitūda | Separate scalar modulus from oscillating peak. |
| peak value / peak magnitude | maksimālā vērtība / maksimālais modulis | Keep the two UI options distinct. |
| ground / earth | zeme / zemes virsma | Separate RF reference from physical medium. |
| load / charge | slodze / lādiņš | Separate impedance load from electrical charge. |
| gain / amplification / profit | pastiprinājums / signāla pastiprināšana / peļņa | Reserve gain for antenna directivity. |
| current / present | strāva / pašreizējais | Separate electrical and temporal senses. |
| charge / fee | lādiņš / maksa | Separate electrical charge from billing. |
| wire / cable / thread | vads / kabelis / pavediens | Separate conductor, cord, and compute thread. |
| radiation pattern / template / far field | starojuma diagramma / veidne / tālais lauks | Separate plotted response, design, and spatial region. |
| excitation / excitement | ierosme / sajūsma | Reserve ierosme for EM input. |
| node / antinode / numeric zero | mezgls / pretmezgls / nulle | Separate standing-wave extrema from a number. |
| scale family / hue / palette kind / color projection | mēroga saime / nokrāsa / paletes veids / krāsu projekcija | Keep four color-system concepts distinct. |
| comet / geometry | komēta / ģeometrija | Separate overlay effect from model geometry. |
| identity / unity | identitāte / vienība | Separate passthrough transfer from Smith-chart unity. |
| renderer / render engine | renderētājs / renderēšanas dzinis | Reserve renderētājs for the drawing backend. |
| override / overwrite | aizstāt / pārrakstīt | Separate value supersession from destructive writing. |
| viewer / observer / speaker / preview | skatītājs / novērotājs / skaļrunis / priekšskatījums | Reserve skatītājs for observation direction and 3D view. |
| reflect geometry / track behavior / physical reflection | atspoguļot / atbilst / atstarošana | Separate mirror operation, control tracking, and physics. |
| structure / construction | struktūra / būvniecība | Reserve struktūra for model geometry. |
| theme / topic | tēma / temats | Separate interface styling from subject matter. |
| validation / verification | validācija / verifikācija | Separate validation-tree use from checks. |
| net gain / real-part gain | neto pastiprinājums / reālās daļas pastiprinājums | Keep total-minus-mismatch distinct from a real component. |
| electrical power / Power curve | jauda / Pakāpes likums | Separate watts from the transfer family. |

- Lock direct loans to impedance, segments, ports, tags, gamma, šeiders, diametrs, tokens, operands, operators, aritāte, and fork; use the topic-10 native terms elsewhere.
- Replace catalog outlier spellings with the topic-10 canonical form for consistency.
- Retain impedance over a native paraphrase: catalog consistency outranks locale-form preference for this concept.

## 13. Priority ordering

- Precedence chain: correct meaning, then interface convention, then catalog consistency, then disambiguation, then locale numeral form.
- Explicit override: "impedance" retained as a loanword despite an available native paraphrase, because catalog consistency (topic 12) outranks locale-form preference here.

## 14. Grammatical number

- Use singular and plural noun, adjective, participle, and verb forms required by Latvian grammar.
- After a literal count ending in 1 but not 11, use the singular form; after counts ending in 0 or 11-19, use the genitive plural; use the plural form after other counts.
- Make every adjective and participle agree with the count-selected noun form.

## 15. Grammatical agreement

- Adjectives and participles agree in gender and number with the head noun, including a standalone label whose head noun is implied.
- Standard Latvian declension classes apply; a partitive genitive follows quantifiers such as "daudz"/"vairāki", not after every count.
- Reflexive verb particles ("-ties") are retained where the base verb is reflexive (eg "pārliecināties").
- A standalone label with no stated head noun defaults to the masculine grammatical form, unless it names a UI element whose Latvian noun is grammatically feminine (eg "vērtība"), in which case the feminine form is used.

## 16. Morphological derivation

- Borrowed technical verbs use the "-ēt" affix family (eg "mērogot"); non-standard colloquial affixes are forbidden.
- Verbal nouns use the "-šana"/"-ēšana" gerund form (eg "mērogošana").
- Native roots are preferred where an established native term exists (eg "pastiprinājums" over a loan); loanwords are used for internationally standardized RF/CS terms (impedance, port, segment, tag).
- Compounding follows the spaced genitive-chain strategy from topic 1.

## 17. Preposition and sandhi selection

- Not applicable: Latvian prepositions carry no phonologically-conditioned form alternation and require no elision/contraction rules for technical strings.

## 18. Card/record-label register

- Dialog/editor titles use the fixed designator form: mnemonic kept verbatim, followed by the Latvian noun in the nominative (eg "GW - Vads").
- Running prose uses the genitive/adjectival form instead (eg "vada segments"), never the title designator.
- The generic card/record noun is lowercase mid-sentence; no separate short/long form distinction exists beyond the title-vs-prose split.
- The title-designator register and the running-prose register each stay internally consistent and are never cross-converted.

## 19. Multi-paragraph and whitespace fidelity

- Mirror every source paragraph break at the same position, preserving blank-line and single-line distinctions.
- Drop clauses absent from the current source instead of retaining stale translated text.
- Preserve source trailing newlines and punctuation.
- Preserve semantic line breaks; add no visual wrapping absent from the source literal.
- Preserve complete meaning; use no truncation or unnatural abbreviation for an assumed display limit.

## 20. Current-source fidelity

- Derive each translation from the current source literal and supplied context.
- Reuse an inherited translation only when its complete meaning matches the current source.
- Do not inherit prior mappings that merge magnitude with amplitude or hue with scale family.

## 21. Script hygiene

- Use required Latvian letters and diacritics in every prose word; permit bare or foreign-script characters only inside retained tokens whose literal spelling requires them.
- Translate plain foreign prose words; retain only identifiers, units, mnemonics, proper names, and other topic-4 tokens.
- Admit zero diacritic-stripped substitutions or foreign homoglyphs in Latvian prose.

## 22. Rule-file scope hygiene

- Include only current decisions that can alter translated literal wording, Unicode characters, punctuation, capitalization, embedded tokens, mnemonic markers, or semantic whitespace.
- Omit catalog representation, serialization, headers, flags, workflow, rendering, fonts, shaping, layout, widget behavior, runtime formatting, sorting, search, display sizing, audit history, completion state, review metadata, provenance, and citations.
- State each linguistic decision directly; include no implementation procedure, correction history, or alternative policy source.

## 23. Section-disjointness declaration

- Topics 1-3 and 21 govern script, digits, punctuation, and script hygiene only.
- Topics 7 and 18 govern interface phrasing, information order, and entity-label structure only.
- Topics 8 and 15 govern address register and grammatical agreement only.
- Topics 4, 10-12 govern retained tokens and concept-term identity only.
- Keep these axes non-overlapping; place each decision in one governing topic.

## 24. Developer/debug-string policy

- Translate user-facing commands, labels, dialogs, tooltips, status messages, and errors; review priority never permits an applicable user-facing literal to remain untranslated.
- Translate informational notices and diagnostics in terse technical Latvian.
- Translate developer-facing and low-priority debug strings in terse technical Latvian; no subsystem family overrides this rule.
- Preserve every embedded identifier, function name, format specifier, mnemonic, unit, and topic-4 retained token in all three families.
- Keep linguistic priority separate from token preservation: every translated family follows topics 4 and 5.
