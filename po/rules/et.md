# Estonian (et) translation rules

These rules govern modern standard Estonian for Estonia, written in Latin script, for professional electrical and radio-frequency engineers, antenna designers, and amateur-radio operators.

## 1. Script and orthography

- Use Latin script with mandatory `õ`, `ä`, `ö`, `ü`, `š`, and `ž`, including uppercase forms; preserve each diacritic and prohibit ASCII-folded look-alikes.
- Precomposed forms (NFC) only; forbid combining-diacritic sequences for the same letters.
- No ambiguous joiner/apostrophe code points occur in Estonian technical prose; none to fix.
- Not applicable: joining/shaping mechanics (Estonian has no positional letter forms or ligature rules).
- Directionality: left-to-right only; bidirectional handling not applicable.
- Letter-case distinction exists; casing rules are governed by topic 6.
- Orthographic standard: modern standard Estonian orthography; no reform-era variant in use.
- Inter-word spacing: single space; same single-space convention between native text and embedded foreign/numeric/unit tokens.
- Compound formation: default to a fused compound or genitive-noun chain, eg `voolujaotus`; prohibit source-structure calques, and use hyphenation only where Estonian orthography requires it.

## 2. Numerals in literals

- Use digits `0-9` for technical values in Estonian prose; no other digit set applies.
- Use a comma as the decimal separator and a non-breaking space as the grouping separator in numbers physically present in translated prose.
- Retain source form for formulas, examples, fixed defaults, named mathematical or standards constants, and numbers inside retained tokens.
- Form ordinals and indexes with a digit plus a period, eg `3.`; distinguish this index mark from the decimal comma.

## 3. Punctuation and quotation

- Quotation marks: plain ASCII `'...'`/`"..."`, matching existing catalog usage; do not switch to native „…" typographic quotes mid-catalog, for catalog-wide consistency.
- Comma, question mark, exclamation mark, and opening punctuation mirror the source form; Estonian shares these marks with the source and does not swap them.
- Colon/semicolon/terminal punctuation: no space before, standard single space after, as in the source.
- Ellipsis: three dots (`...` or `…`), matching source usage; preserve source dash characters (en/em) rather than substituting a hyphen.
- Sentence terminator: full grammatical sentences take a period; short labels and menu fragments omit it.
- Punctuation inside embedded technical runs (format specifiers, code, identifiers) stays in source form.

## 4. Never-translate tokens

Retain every listed token exactly, without translation or transliteration; any altered retained token is a failure.

- NEC2 card mnemonics: `GW GA GH EX LD FR RP GE EN SP SM SC NE NH NT TL GN GD GC GX GR GS GM GF CP CM SY XQ EK KH PQ PT`.
- Unit symbols: `Hz`, `kHz`, `MHz`, `GHz`, `dB`, `dBi`, `Ω`, `W`, `K`, `S/m`, `deg`, `°`, and `%`.
- Figure-of-merit and parameter tokens: `VSWR`, S-parameter forms, `Z`, `Z0`, `F/B`, and `G/T`.
- File extensions: `.nec`, `.csv`, `.s1p`, `.s2p`, and `.png`.
- Format specifiers: retain the exact forms governed by topic 5.
- Embedded identifiers: retain function names, variable names, and configuration keys in developer strings.
- Physical and mathematical symbols: retain `E`, `H`, `X`, `Y`, `Z`, `φ`, `θ`, and exponent letters in their source form.
- Product, library, tool, and chart names: retain `xnec2c`, `NEC2`, `GTK`, `OpenGL`, `GSL`, `Cairo`, and `Smith`.
- Named transfer functions and algorithms: retain `Log`, `Asinh`, `μ-law`, `Reinhard`, `Sigmoid`, and `Identity`; translate descriptive `Power` as `astmefunktsioon`.
- Conditional loanwords: retain `segment`, `port`, `gradient`, `gamma`, `shader`, and process term `fork`; translate `patch` and `tag` according to topic 10.
- Not applicable: bidirectional-token handling, because Estonian literals are left-to-right.

## 5. Format-specifier integrity

- Preserve the exact source set of format specifiers, including `%s`, `%d`, `%f`, `%c`, `%%`, and positional forms; missing, added, or altered specifiers have zero tolerance.
- Default: keep source order; use explicit positional `%N$s` only where Estonian word order genuinely requires reordering.
- Prefer restructuring the sentence around fixed specifier positions over reordering specifiers; Estonian is SVO-default and rarely needs reordering.
- Numbers inside specifiers are never localized (decimal/thousands rules of topic 2 do not apply inside a specifier).

## 6. Capitalization and title-case

- Default: sentence case, capitalizing only the first word and proper nouns; forbid title-casing every word (eg "Color Scale" → "Värviskaala", not "Värvi Skaala").
- Axis letters (X, Y, Z) retain source capitalization as mathematical symbols (topic 1, topic 4).
- A lowercase math/coordinate variable (eg `x`, `y`) stays lowercase regardless of sentence position.
- No coordinated-option-name capitalization exception is attested in this catalog; default sentence case governs.
- Generic card/record common nouns (eg `kaart`) stay lowercase by default; acronyms and proper nouns (NEC2, VSWR, xnec2c) retain their fixed form.
- Applicable in full: the script carries letter-case distinction.

## 7. Interface register by string type

- Commands, buttons, and menu actions use the second-person singular imperative without an explicit subject; place the object before or after the verb as natural Estonian information structure requires.
- Field labels use a concise noun phrase in modifier-before-head order and retain a source colon.
- Dialogs and confirmations use complete sentences, subject-verb-object order where a subject is explicit, and the formal `teie` register from topic 8.
- Tooltips use complete declarative sentences in reason-before-consequence or condition-before-action order; preserve any source reason that a control is unavailable.
- Status and error messages use an impersonal declarative, passive, or verbal-noun construction without a subject pronoun; present the affected entity before the result when that order is clearest.
- Name user-visible domain entities with a modifier before the head noun or a fused compound; retain proper names unchanged under topics 4 and 8.
- Use the shortest complete natural form; prefer fused compounds and genitive chains, preserve all meaning, and omit unnatural abbreviations and source-length imitation.
- Apply topic 24 to developer and diagnostic strings.

## 8. Formality and address

- Address the user with formal second-person plural `teie` forms in dialogs and questions; prohibit informal second-person singular address.
- Express formality through verb endings and word choice; use no honorific.
- Use subjectless imperatives for commands and explicit or omitted `teie` according to natural sentence structure in dialogs; use impersonal subject-drop in status and informational messages.
- Use the confirmation pattern `Kas olete kindel, et…` or `Kas soovite…` according to meaning.
- Not applicable: grammatical gender and gendered pronouns; use natural gender-neutral wording without inventing gender distinctions.
- Preserve required grammatical number and case while referring to people inclusively.
- Use second person only when the interface addresses the user; use Estonian given-name-before-family-name order when a full personal name is presented.
- Prohibit casual, slang, over-formal, commercial, and archaic register.

## 9. Accelerator/hotkey mnemonics

- Preserve a source mnemonic with `_` immediately before a mnemonic letter in the translated literal; use a letter from the Estonian term rather than the source term.
- Add no mnemonic when the source literal has none.
- Prefer an easily typed base Latin letter where the translated term offers one; otherwise use the required Estonian letter.
- Not applicable: separate parenthetical mnemonic letters, because Estonian uses Latin script.

## 10. Domain lexicon

Established/preferred terms; one target term per concept, reused consistently across the catalog.

### Electrical primitives

| Concept | Estonian | Sense / hazard guarded |
|---|---|---|
| current | vool | electrical current; never "praegune" (temporal); accepted homonym with `flow` (topic 11) |
| charge | laeng | electrical charge; never "tasu" (billing) |
| voltage | pinge | electric potential |
| power (electrical) | võimsus | watts/power-flow; distinct from the `Power` scale-family name |
| impedance | impedants | complex Z; distinct from resistance and load |
| resistance | takistus | real part of Z; distinct from impedance and load |
| reactance | reaktants | imaginary part of Z |
| inductance | induktiivsus | electromagnetic inductance; canonical electrical term |
| capacitance | mahtuvus | electromagnetic capacitance; canonical electrical term |
| conductivity | erijuhtivus | material S/m; native term |
| admittance | admitants | distinct from impedance |
| load | koormus | LD-card impedance load; not physical weight; distinct from charge and resistance |
| gain | võimendus | antenna directivity ratio; never "kasum" (profit) or "võimendamine" (amplifier act) |
| excitation | ergastus | EM energy input; never "elevus" (emotional excitement) |
| feedpoint | toitepunkt | antenna feed point; canonical fused compound |
| port | port | loanword, kept as-is |
| radials | radiaalid | ground-plane radial wires (noun); distinct from adjective "radiaalne" |

### Ground and earth

| Concept | Estonian | Sense / hazard guarded |
|---|---|---|
| ground / ground plane | maandus / maandustasand | RF electrical reference plane; never "muld" (soil) |
| earth (physical medium) | pinnas | terrain/noise-model earth; distinct from electrical `maandus` |
| ground wave | pinnalaine | propagation term; distinct from the ground reference |

### Geometry primitives

| Concept | Estonian | Sense / hazard guarded |
|---|---|---|
| wire | traat | thin conductor/GW element; never "kaabel" (cable) |
| segment | segment | NEC2 subdivision; loanword, kept |
| patch | (pinna)plaat | NEC2 surface patch; translated, not kept |
| tag | silt | NEC2 geometry identifier; not a UI label or card |
| card | kaart | NEC2 input record; register in topic 18 |
| kernel | tuum | integral-equation/thin-wire kernel; not an OS kernel |
| cliff | järsak | two-medium ground-boundary type; not a fracture |
| structure | struktuur | antenna model geometry; not "ehitus" (construction) |
| model | mudel | NEC model or noise-temperature model |
| geometry | geomeetria | the model geometry |
| crossed | ristatud | transmission-line conductors crossed/reversed; not cut/severed |

### Field, pattern, viewer

| Concept | Estonian | Sense / hazard guarded |
|---|---|---|
| field (EM) | väli | near/total/E/H field; accepted homonym with UI data "väli" (topic 11) |
| near field / far field | lähiväli / kaugväli | opposed pair, kept symmetric |
| far-field contribution | kaugvälja panus | per-direction contribution |
| radiation | kiirgus | radiated emission |
| radiation pattern | kiirgusdiagramm | plotted directional response; not a template ("mall") |
| gain pattern | võimendusdiagramm | the gain radiation pattern |
| polarization | polarisatsioon | antenna/wave field orientation |
| polarity | polaarsus | sign of a quantity; false friend of polarization, kept distinct |
| phase | faas | electromagnetic phase; canonical technical term |
| reference phase | võrdlusfaas | phase reference; distinct fused compound |
| frequency | sagedus | oscillation frequency; canonical technical term |
| wave / wavelength | laine / lainepikkus | wave and its spatial period; keep the related terms consistent |
| standing wave / traveling wave | seisulaine / jooksev laine | opposed pair |
| node / antinode | sõlm / kõhk | standing-wave zero/maximum; never "antisõlm" (calque) |
| crest | hari | instantaneous wave apex (comet-head); distinct from a curve/step "tipp" |
| magnitude | tugevus | modulus of a quantity, incl. current/charge along wires and \|Z\|; umbrella term, distinct from amplitude |
| amplitude | amplituud | oscillating-quantity peak value context only |
| peak value | tippväärtus | distinct UI option from peak magnitude |
| peak magnitude | tipu amplituud | distinct UI option from peak value |
| instantaneous | hetkeline | eg "Instantaneous Magnitude" → "Hetkeline tugevus"; add "(φ=0)" only where source carries it |
| Poynting vector | Poyntingi vektor | proper-name loanword construction |
| solid angle | ruuminurk | three-dimensional angular measure; canonical technical term |
| net gain | netovõimendus | total-minus-mismatch gain; never "reaalosa võimendus" |
| viewer | vaataja | observation direction / 3D view widget; never "vaatleja/kõneleja/eelvaade" |
| flow / flow direction | vool / voolusuund | patch/current flow; "voolusuund" established compound |
| total field | summaarne väli | combined electromagnetic field; distinct from a component field |

### Color, tone, animation subsystem

| Concept | Estonian | Sense / hazard guarded |
|---|---|---|
| color | värv | visible color; parent term for the color subsystem |
| color projection | värviprojektsioon | which quantity drives hue |
| hue | toon | color-wheel angle |
| brightness | heledus | luminance channel |
| hue encoding / brightness encoding | toonikodeering / heleduskodeering | internal enums; distinct from user-facing "värviprojektsioon" |
| color scale | värviskaala | magnitude-to-color scale |
| scale family / color tone | skaala pere | transfer-curve family; one concept and one term; translate `Power` as `astmefunktsioon`, retain the named families under topic 4 |
| palette / palette kind | palett / paletiliik | palette-layout enum; distinct from scale family and color projection |
| ramp / gradient | gradient | loanword; one term for both senses, eg "värvigradient", "Vikerkaar-gradient" |
| gamma | gamma | power-law exponent; loanword |
| knee | põlv | soft-knee bend point |
| softening | pehmendus | transfer-curve softening; distinct from blur |
| compression | kompressioon | dynamic-range compression |
| contrast | kontrast | tonal contrast; canonical imaging term |
| dynamic range | dünaamikaulatus | represented magnitude span; canonical fused compound |
| floor | alammäär | minimum/lower clamp; never "põrand" (room floor); eg "Brightness floor" → "Heleduse alammäär" |
| envelope | ümbris | magnitude/amplitude envelope |
| comet | komeet | moving-crest overlay effect; not geometry (known fuzzy-inheritance hazard, topic 20) |
| overlay (noun) | kiht | added visual layer, eg "Overlays:" → "Kihid:" |
| overlay (verb) | kihina kuvama | to overlay; distinct from the noun `kiht` |
| animate / animation | animeeri / animatsioon | action and process forms; keep verb and noun distinct |
| animated / static | animeeritud / staatiline | category-header adjectives |
| projection | projektsioon | color or geometry projection |
| scale (noun) / scale (verb) | skaala / skaleerima | distinguish the measure from the scaling action |
| wireframe | traatraam | wire-mesh render mode |
| identity | identsus | no-op/passthrough transfer; distinct from `unity` |
| unity | ühik | Smith-chart unity; distinct from `identity` |
| sentinel | valvurväärtus | unreachable-case guard value |
| bins | vahemikud | discretization buckets; never "täkked" (notches); eg "Depth Bins" → "Sügavuse vahemikud" |
| companding | kompandeerimine | bounded log curve (μ-law) |
| tone mapping | toonikaardistus | photographic tone-map; shares the "tooni-" stem with `hue`, disambiguated by context (topic 11) |

### Render and compute

| Concept | Estonian | Sense / hazard guarded |
|---|---|---|
| renderer | renderdaja | drawing backend; never "renderdusmootor" (render engine) |
| shader | shader | loanword, kept |
| allocation (memory) / managed allocator | mälueraldus / mäluhaldur | memory allocation and its manager; keep operation and component distinct |
| thread | lõim | compute thread; no homonym collision with `wire` (traat) in Estonian |
| widget | vidin | user-interface element; canonical native term |
| validation | valideerimine | the validation-tree feature; distinct from `verification` (kontroll) |
| batch mode | pakkrežiim | noninteractive grouped operation; canonical fused compound |
| fork (process) | fork | kept verbatim, loanword decision (topic 4) |
| deadlock | ummik | mutual-wait failure; distinct from a generic delay |
| notifier | teavitaja | notification component; canonical agent noun |
| token / operand / operator / arity | märgend / operand / operaator / arguutide arv | expression-parser terms |
| override | alistus | supersede a value (SY symbol); never "ülekirjutus" (overwrite); after a count use partitive: "%d sümboli alistust" |
| swap | vahetama | exchange two values; distinct from replacement |
| theme | teema | UI/color theme; Estonian has no distinct native alternative from "topic/subject," so context alone disambiguates (topic 11) |

### Metrics and miscellaneous

| Concept | Estonian | Sense / hazard guarded |
|---|---|---|
| noise / noise temperature | müra / müratemperatuur | electronic/thermal noise, not acoustic racket |
| efficiency | kasutegur | output-to-input ratio; canonical engineering term |
| interpolation | interpolatsioon | estimation between samples; canonical numerical term |
| mnemonic | mnemoonik | a card's code descriptor; not "märkus" (memo/note) |
| degrees / deg | kraadid (prose) / "(deg)" (unit tag) | freestanding prose translates; the parenthetical unit tag is kept verbatim, same policy as other unit tags |
| diameter | läbimõõt | native term preferred over loanword "diameeter" (topic 16) |
| reflect | peegeldama | geometry mirror operation; distinct from `mirrors` and `reflection` |
| mirrors | jälgib | behavioral tracking (a control tracking another); distinct from `reflect` |
| reflection | peegeldus | physics reflection; distinct from `reflect` and `mirrors` |
| default(s) | vaikeväärtus(ed) | fallback value |
| normalize / normalization | normeerima / normeerimine | translated, not transliterated |

## 11. Disambiguation policy

- Choose the correct technical sense for each ambiguous term (topic 10); program context already disambiguates, so no qualifier is added beyond the source.
- Add a qualifier only where the Estonian term would otherwise be genuinely ambiguous.
- Accepted intra-domain homonyms, reused deliberately: `vool` (current / flow), `väli` (EM field / UI data field), `teema` (theme / topic), the `tooni-` stem (hue / tone mapping).
- No locative homonym collision identified beyond the above.
- Gerund vs noun senses: `skaala` (noun) vs `skaleerima` (verb, gerund sense), `kiht` (noun) vs `kihina kuvama` (verb) — kept distinct (topic 10).

## 12. Cross-catalog consistency

- One term per concept, reused from the established lexicon (topic 10); no synonym introduced for an already-mapped concept.
- Appendix C false-friend pairs, each resolved to two distinct terms:
  - polaarsus vs polarisatsioon
  - tugevus (magnitude, incl. \|Z\|) vs amplituud (amplitude)
  - tippväärtus vs tipu amplituud
  - maandus vs pinnas
  - koormus vs laeng
  - võimendus (gain) vs võimendamine (amplification) vs kasum (profit)
  - vool (current) vs praegune (temporal)
  - laeng (charge) vs tasu (billing)
  - traat (wire) vs kaabel (cable); traat vs lõim — no collision in Estonian
  - kiirgusdiagramm vs mall (template) vs kaugväli (far field)
  - ergastus vs elevus
  - sõlm/kõhk vs null (generic numeric zero)
  - skaala pere vs toon vs paletiliik vs värviprojektsioon
  - komeet vs geomeetria
  - identsus vs ühik
  - renderdaja vs renderdusmootor (forbidden)
  - alistus vs ülekirjutus (forbidden)
  - vaataja vs vaatleja/kõneleja/eelvaade
  - peegeldama vs jälgib vs peegeldus
  - struktuur vs ehitus
  - teema vs topic/subject — exception: no distinct native alternative exists, context disambiguates (topic 11)
  - valideerimine vs kontroll (verification)
  - netovõimendus vs reaalosa võimendus (forbidden)
  - võimsus (electrical power) vs astmefunktsioon (Power scale-family name)
- Loanwords with a locked canonical spelling, matching source Latin spelling with no phonetic respelling: `port`, `segment`, `gradient`, `gamma`, `shader`, `fork`.
- Minority-outlier spellings of any of the above unify to the canonical spelling.
- Consistency outranks locale-form preference where already decided: `port` is kept as loanword despite the native alternative "pesa," for catalog-wide consistency with established usage.

## 13. Priority ordering

- Apply this precedence: correct technical meaning, interface convention, catalog consistency, disambiguation, then literal numeral form.
- Map `load` to `koormus` rather than `takistus`: correct separation from resistance overrides prior catalog usage.

## 14. Grammatical number

- Use singular and plural noun, adjective, participle, and verb forms according to the literal's grammatical subject and referent.
- After an explicit count of `1`, use the singular nominative; after literal counts of `2` or more, use the singular partitive where Estonian count syntax requires it.
- Not applicable: uninflected-number handling, because Estonian marks grammatical number.

## 15. Grammatical agreement

- Adjectives and participles agree in number and case with the head noun, including standalone labels whose head noun is implied (eg "Aktiivne" agreeing with an implied "port").
- Declension: standard case/declension selection; numbers 2 and above take the noun in partitive singular (eg "%d sümboli alistust").
- Reflexive verb forms retain their reflexive marking where the source implies self-action (eg a control that "resets itself").
- Not applicable: gender concord and standalone-label default-gender resolution (Estonian has no grammatical gender).

## 16. Morphological derivation

- Adapt borrowed technical verbs and nouns with the standard `-eeri-` family, eg `valideerima`; prohibit ad hoc suffixing directly onto an unadapted source stem.
- Verbal-noun formation: `-mine` suffix (eg "valideerimine", "renderdamine").
- Prefer a native term where one is established (eg "läbimõõt" over "diameeter"); accept a loanword only for domain terms lacking a native equivalent (topic 4's conditional loanwords).
- Compounding strategy: fused compound preferred (topic 1).

## 17. Preposition and sandhi selection

- Not applicable: Estonian adpositions do not select a form by the following sound, and translated literals require no sandhi, elision, or contraction rule.

## 18. Card/record-label register

- Dialog/editor titles use the fixed designator form: mnemonic plus "kaart" (eg "GW kaart", "Kaardi redaktor").
- Running prose uses the genitive-compound form (eg "GW-kaardi").
- Generic noun "kaart" stays lowercase; short form (bare mnemonic) is used in tables/headers, long form ("GW kaart") in prose.
- Each register stays internally consistent within its context and is never cross-converted.

## 19. Multi-paragraph and whitespace fidelity

- Mirror source paragraph breaks at the same positions and preserve the distinction between blank-line and single-line breaks.
- Drop clauses absent from the current source rather than retaining stale translated text.
- Preserve source trailing newlines and punctuation exactly.
- Preserve semantic line breaks; add no visual wrapping absent from the source literal.
- Preserve complete meaning and natural wording; never truncate or abbreviate to fit an assumed display size.

## 20. Current-source fidelity

- Derive every translation from the current source literal and its supplied context.
- Reuse an inherited translation only when its complete meaning agrees with the current source.
- Treat prior translations of `comet`, `geometry`, `magnitude`, and `amplitude` as unsafe unless they preserve the distinctions in topics 10 and 12.

## 21. Script hygiene

- Use `õ`, `ä`, `ö`, `ü`, `š`, and `ž` exactly where Estonian spelling requires them; ASCII-folded substitutions have zero tolerance outside retained tokens.
- Translate plain foreign words; retain only genuine identifiers, symbols, units, and named tokens under topic 4.
- Not applicable: cross-script homoglyph leakage, because Estonian translated prose uses the same Latin script as retained source tokens.

## 22. Rule-file scope hygiene

- Include only current decisions that can alter wording, Unicode characters, punctuation, capitalization, embedded tokens, mnemonic markers, or semantic whitespace in an Estonian translated literal.
- Omit catalog representation, serialization, headers, flags, validation workflow, rendering, fonts, shaping, layout, widget behavior, runtime formatting, sorting, search, display sizing, audit history, completion state, review metadata, provenance, and source citations.
- State each decision directly; include no implementation procedure, bibliography, correction history, or alternative policy source.

## 23. Section-disjointness declaration

- Script mechanics (topics 1, 21), phrasing/structure (topics 5-7, 9, 14-19), and address register (topic 8) are non-overlapping sections; each concept is located in exactly one of them.

## 24. Developer/debug-string policy

- Translate all user-facing controls, dialogs, tooltips, status messages, errors, and informational diagnostics into Estonian; review priority never permits an applicable user-facing literal to remain untranslated.
- Retain explicitly developer-facing debug and low-level diagnostic strings in source form unless an established sibling family in the same subsystem is translated; follow that family consistently.
- Use terse technical register in translated diagnostics.
- Preserve every embedded identifier, function name, format specifier, and retained token verbatim in every string family.
- Keep linguistic priority separate from token preservation: translated user-facing and informational families obey the same token invariants as developer-facing strings.
