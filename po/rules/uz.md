# uz translation rules

## 1. Script and orthography
- Scope: Uzbek for Uzbekistan, written in the post-1995 Uzbek Latin alphabet for professional electrical and radio-frequency engineers, antenna designers, and amateur-radio operators.
- Required alphabet forms include `sh`, `ch`, `ng`, `oʻ`, and `gʻ`; retain every modifier letter and never fold it to a look-alike.
- Modifier letter `ʻ` is U+02BB (MODIFIER LETTER TURNED COMMA); use exclusively, never `'` U+0027 or `’` U+2019, eg `koʻrish`, `oʻqi`, `qutblanish`.
- No combining-vs-precomposed choice: `ʻ` is always a standalone letter, never a combining diacritic.
- Script has letter-case distinction; casing rules in topic 6 apply.
- Directionality: left-to-right; no bidirectional handling needed.
- Inter-word spacing: single space, standard Latin convention; one space between native text and embedded foreign/numeric/unit tokens, eg `10 MHz`.
- Compound formation: spaced by default for technical compounds, eg `yer tekisligi`; fused only for established single-word loans, eg `boʻlaklar`; hyphenation not used for NEC2 domain compounds.
- Orthographic standard: post-1995 Latin reform; pre-1995 Cyrillic-era spellings are never reintroduced (topic 20).

## 2. Numerals in literals
- Use Western Arabic digits `0-9` for literal technical values; do not use Arabic-Indic digits.
- Use decimal comma `,` and space grouping in translated prose, eg `12 500,5`.
- Retain source form for numbers inside formulas, examples, fixed defaults, and named mathematical or standards constants.
- Form literal ordinals and indices with `-chi` or `-inchi`, eg `3-chi`; the hyphen is an index separator, not a decimal separator.

## 3. Punctuation and quotation
- Native quotation marks: guillemets `«»` in prose; embedded technical/code tokens keep source straight quotes for consistency with format specifiers.
- Uzbek Latin punctuation (comma, question mark, exclamation, opening marks) matches source Latin punctuation; no distinct native replacement marks exist.
- No spacing inserted before colon/semicolon/terminal punctuation; standard Latin no-space-before convention.
- Ellipsis: `...` (three dots); dashes in source are preserved as-is.
- Sentence terminator: period `.` on full grammatical sentences (dialogs, confirmations, status/error messages); short labels, buttons, and field names omit it.
- Punctuation inside embedded technical runs (format specifiers, unit tags, file paths) stays in source form.

## 4. Never-translate tokens
- Require zero retained-token failures: keep every token listed here verbatim, without translation or transliteration.
- Keep NEC2 card mnemonics `GW GA GH EX LD FR RP GE EN SP SM SC NE NH NT TL GN GD GC GX GR GS GM GF CP CM SY XQ EK KH PQ PT`.
- Keep unit symbols `%`, `Hz`, `kHz`, `MHz`, `GHz`, `dB`, `dBi`, `Ω`, `W`, `K`, `S/m`, and `deg`.
- Keep figure-of-merit and parameter tokens `VSWR`, `S11`, `S12`, `S21`, `S22`, `Z`, `Z0`, `F/B`, and `G/T`.
- Keep literal file extensions and filenames, including `.nec`, `.out`, `.csv`, `.png`, and `.gplot`.
- Keep every printf-style format specifier under topic 5.
- Keep embedded function names, variable names, configuration keys, and other code identifiers.
- Keep physical and mathematical symbol letters, including `E`, `H`, `φ`, and `μ`.
- Keep product, library, toolkit, and chart proper names in their source spelling.
- Keep named transfer functions `Log`, `Asinh`, `μ-law`, `Reinhard`, `Sigmoid`, and `Identity`; translate descriptive `Power` as `Darajaviy`.
- Keep geometry loanwords `segment`, `patch`, and `tag`, and process term `fork`; translate other geometry concepts through topic 10.
- Apply topic 1 spacing and left-to-right direction to retained tokens inside Uzbek text.

## 5. Format-specifier integrity
- Require zero format-token failures: preserve exactly the source set of specifiers in every translation.
- Keep source order by default; use positional forms such as `%1$s` only when Uzbek grammar requires reordered values.
- Restructure the sentence around fixed specifier positions rather than moving non-positional specifiers.
- Preserve every number and other character inside each specifier verbatim.

## 6. Capitalization and title-case
- Default casing: sentence case for labels, menu items, dialog titles; title-case (capitalizing every word) is forbidden except NEC2 card mnemonics, which retain their fixed uppercase two-letter form.
- Axis letters `X`, `Y`, `Z` stay uppercase in all positions, including mid-sentence.
- A lowercase math/coordinate variable (eg `x` in a formula) stays lowercase, distinct from the axis label `X`.
- No coordinated-option capitalization exception exists in this catalog.
- Generic card/record noun (`karta`/`kartasi`) is lowercase mid-sentence, capitalized only when sentence-initial or in a title (topic 18); acronyms and proper nouns retain source casing.

## 7. Interface register by string type
- Commands, buttons, and menu actions: omit the subject; use a formal second-person imperative with the object before the final verb, eg `Tanlang`.
- Field labels: use a head-final noun phrase without a verb and retain the source colon.
- Dialogs and confirmations: use complete formal sentences with stated objects, omitted recoverable subjects, and the verb in final position.
- Tooltips: use concise declarative sentences; place the unavailable control or condition before its reason, and include the reason when the source does.
- Status and error messages: use impersonal declaratives; present the affected entity before its state or failure and omit first-person subjects.
- User-visible domain entities: place modifiers before the topic-10 head term; retain proper names and personal names under topic 8.
- Prefer the shortest complete natural form; use spaced head-final compounds where topic 1 requires them, preserve all meaning, and introduce no unnatural abbreviation.
- Apply topic 24 to developer and debug strings.

## 8. Formality and address
- Use formal `siz` register throughout, realized by `-ing` or `-ingiz` verb endings; omit the pronoun when recoverable and never use informal `sen` stems.
- Uzbek has no grammatical gender or gender agreement; use neutral role nouns and natural number forms without adding gender.
- Inclusive wording requires no special morphology beyond neutral role terms; no animacy-based alternative applies.
- Use no honorific in interface text; use second person only for commands and direct confirmations, otherwise use impersonal constructions.
- Preserve a person's supplied name order; Uzbek interface grammar imposes no alternate personal-name order.
- Use the same formal register for commands, tooltips, dialogs, and status messages.
- Form confirmations as full formal imperative or declarative sentences ending in a period.
- Exclude casual, slang, commercial, archaic, and ceremonially over-formal registers.

## 9. Accelerator/hotkey mnemonics
- Mnemonic marker: `_` before the accelerator letter, matching GTK convention.
- Draw the mnemonic letter from the translated Uzbek term, never from the source term.
- Script is Latin with full case distinction; no appended parenthetical Latin-letter form is needed (that convention applies only to non-Latin scripts).
- A mnemonic is never invented where the source string carries none.
- Avoid anchoring a mnemonic on a word beginning with the modifier letter `ʻ` (`oʻ`/`gʻ` digraphs), as it is not directly typable as an accelerator.

## 10. Domain lexicon
- Lock each concept to its sole term throughout the catalog; in these tables, `—` in `sense` means the concept key's established engineering sense, and `—` in `hazard guarded` means canonical-term consistency.

### Electrical primitives
| concept | term | sense | hazard guarded |
|---|---|---|---|
| current | tok | electrical current | not `joriy`/`hozirgi` (temporal "present") |
| charge | zaryad | electrical charge | not billing/fee (`toʻlov`) |
| voltage | kuchlanish | electric potential | — |
| power (electrical) | quvvat | radiated/dissipated watts | distinct from `Power` family name (`darajaviy`) |
| impedance | impedans | complex Z | distinct from `qarshilik` (resistance), `reaktans` (reactance) |
| resistance | qarshilik | real part of Z | distinct from impedance and `yuk` (load) |
| reactance | reaktans | imaginary part of Z | distinct from resistance |
| inductance | induktivlik | — | — |
| capacitance | sigʻim | — | — |
| conductivity | oʻtkazuvchanlik | material S/m | native term, not a loanword |
| admittance | admittans | admittance-matrix Y | distinct from impedance |
| load | yuk | LD-card impedance load | not physical weight/burden; distinct from `zaryad` (charge), no homonym collision |
| gain | kuchaytirish | antenna directivity ratio (dB) | distinct from `amplifikatsiya` (amplifier gain), `foyda` (profit) |
| excitation | qoʻzgʻatish | EM energy input/source | not `hayajon` (emotional excitement) |
| feedpoint | boshlangʻich nuqta | antenna feed point | — |
| port | port | excitation/S-parameter port | — |
| radials | radiallar | horizontal ground-plane radial wires (noun) | distinct from the adjective `radial` |

### Ground and earth
| concept | term | sense | hazard guarded |
|---|---|---|---|
| ground / ground plane | yer tekisligi | RF electrical reference plane, GN/GD cards | one term across all electrical-ground sub-senses; distinct from physical `yer muhiti` |
| earth (physical medium) | yer muhiti | terrain/noise-model earth, sub-ground geometry | distinct from electrical `yer tekisligi` |
| ground wave | yer toʻlqini | propagation term | distinct from the ground reference |

### Geometry primitives
| concept | term | sense | hazard guarded |
|---|---|---|---|
| wire | sim | thin conductor / GW element | not `kabel` (cable/cord); distinct from `tola` (thread) |
| segment | segment | NEC2 geometry subdivision | kept untranslated (topic 4) |
| patch | patch | NEC2 surface patch (SP/SM) | kept untranslated (topic 4) |
| tag | tag | NEC2 geometry identifier | kept untranslated; not a UI label or card |
| card | karta / kartasi | NEC2 input record | register in topic 18 |
| kernel | yadro | integral-equation / thin-wire kernel | not an OS kernel; context disambiguates |
| cliff | jarlik | two-medium ground-boundary type | not `yorilish` (fracture/break) |
| structure | tuzilma | antenna model geometry | not `qurilish` (construction) |
| model | model | NEC model or noise-temperature model | — |
| geometry | geometriya | model geometry | — |
| crossed | kesishgan | transmission-line conductors crossed/reversed | not `kesilgan` (cut/severed) |

### Field, pattern, viewer
| concept | term | sense | hazard guarded |
|---|---|---|---|
| field (EM) | maydon | near/total/E/H field | distinct from a data/config field |
| near field | yaqin maydon | — | symmetric with far field |
| far field | uzoq maydon | — | symmetric with near field |
| far-field contribution | uzoq maydon hissasi | per-direction contribution | not near-field animation |
| radiation | nurlanish | radiated emission | — |
| radiation pattern | nurlanish diagrammasi | plotted directional response | not `andoza`/dizayn (template/design); distinct from `uzoq maydon` |
| gain pattern | kuchaytirish diagrammasi | gain radiation pattern | — |
| polarization | qutblanish | antenna/wave field orientation | never reused for polarity |
| polarity | qutblilik | sign (+/-) of a quantity | false friend of polarization |
| phase | faza | — | — |
| reference phase | tayanch faza | — | — |
| frequency | chastota | — | — |
| wave / wavelength | toʻlqin / toʻlqin uzunligi | — | — |
| standing wave | turgʻun toʻlqin | — | opposed to traveling wave |
| traveling wave | yuruvchi toʻlqin | — | opposed to standing wave |
| node | tugun | standing-wave zero | carries the null overlay sense |
| antinode | qorin | standing-wave maximum | carries the peak overlay sense |
| crest | toʻlqin uchi | instantaneous wave apex (comet-head) | distinct from `eng yuqori qiymat`/`eng yuqori kattalik` (curve/step peak) |
| magnitude | kattalik | modulus of a quantity (|Z|, scalar) | distinct from amplitude |
| amplitude | amplituda | oscillating-quantity peak | distinct from magnitude |
| peak value | eng yuqori qiymat | UI option | must not collapse with peak magnitude |
| peak magnitude | eng yuqori kattalik | UI option | must not collapse with peak value |
| instantaneous | lahzalik | projection mode | add `(φ=0)` qualifier only where source carries it |
| Poynting vector | Poynting vektori | — | proper name kept |
| solid angle | fazoviy burchak | — | — |
| net gain | sof kuchaytirish | total-minus-mismatch gain | not `haqiqiy qism kuchaytirishi` (real-part gain) |
| viewer | koʻruvchi | observation direction / 3D view widget | not `kuzatuvchi` (observer), `nutq soʻzlovchi` (speaker), `oldindan koʻrish` (preview) |
| flow / flow direction | oqim / oqim yoʻnalishi | patch/current flow | distinct from `tola` (thread) |
| total field | umumiy maydon | — | — |

### Color, tone, animation subsystem
| concept | term | sense | hazard guarded |
|---|---|---|---|
| color | rang | — | — |
| color projection | rang proyeksiyasi | which quantity drives hue | — |
| hue | tus | color-wheel angle | never `rang toni`; matches `Faza tusi` |
| brightness | yorqinlik | luminance channel | — |
| hue encoding | tus kodlash | internal enum | distinct from brightness encoding, never collapses to color projection |
| brightness encoding | yorqinlik kodlash | internal enum | distinct from hue encoding |
| color scale | rang masshtabi | magnitude-to-color scale | — |
| scale family / color tone | rang ohangi | transfer-curve family (Power/Log/Asinh/μ-law/Reinhard/Sigmoid/Identity) | one concept, one term; distinct from hue, palette kind, color projection |
| palette / palette kind | palitra / palitra turi | palette-layout enum | distinct from scale family and color projection |
| ramp / gradient | gradient | palette kind / linear color strip | reuse established loanword, not `masshtab`/`shkala` |
| gamma | gamma | power-law exponent | — |
| knee | tizza nuqtasi | soft-knee bend point | — |
| softening | yumshatish | dynamic-range softening | — |
| compression | siqish | dynamic-range compression | — |
| contrast | kontrast | — | — |
| dynamic range | dinamik diapazon | — | — |
| floor | pastki chegara | minimum/lower clamp (brightness/dB floor) | not a room floor |
| envelope | qobiq | magnitude/amplitude envelope | — |
| comet | kometa | moving-crest overlay effect | not geometry; known fuzzy-inheritance hazard |
| overlay (noun) | qatlam | added visual layer | distinct from the verb "to overlay" |
| animate / animation | animatsiya qilish / animatsiya | — | — |
| animated / static | animatsiyali / statik | category-header adjectives | dynamic vs phase-invariant |
| projection | proyeksiya | color or geometry projection | canonical spelling with `y` |
| scale | masshtab / masshtablash | verb (to scale) and noun (a scale) | — |
| wireframe | sim karkas | wire-mesh render mode | — |
| identity | identifikatsiya | no-op/passthrough transfer | distinct from `birlik` (unity, Smith-chart) |
| unity | birlik | Smith-chart normalized value 1 | distinct from identity |
| sentinel | qoʻriqchi qiymat | unreachable-case guard value | — |
| bins | boʻlaklar | discretization buckets | — |
| companding | kompanding | bounded log curve (μ-law) | — |
| tone mapping | tonal xaritalash | photographic tone-map | distinct from `rang ohangi` (scale family) |
| Power (family name) | Darajaviy | power-law transfer-curve family name | the sole translated named-algorithm term (topic 4); distinct from `quvvat` (electrical power) |

### Render and compute
| concept | term | sense | hazard guarded |
|---|---|---|---|
| renderer | renderer | drawing backend | not `render dvigateli` (render engine) |
| shader | shader | — | — |
| allocation (memory) | ajratish | memory allocation | — |
| managed allocator | boshqariladigan ajratuvchi | allocator/report | — |
| thread | tola | compute thread | distinct from `sim` (wire) and `oqim` (flow) |
| widget | vidjet | UI element | — |
| validation | tasdiqlash | the validation-tree feature | distinct from `tekshiruv` (verification checks) |
| verification | tekshiruv | correctness checks | distinct from `tasdiqlash` (validation-tree feature) |
| batch mode | paket rejimi | — | — |
| fork (process) | fork | process fork | kept verbatim (topic 4) |
| deadlock | oʻzaro bloklanish | mutual thread blocking | — |
| notifier | xabarnoma | — | — |
| token / operand / operator / arity | token / operand / operator / arity | expression-parser terms | kept as established loanwords |
| override | ustunlik berish | supersede a value (SY symbol) | not `ustiga yozish` (overwrite) |
| swap | almashtirish | exchange | — |
| theme | tema | UI/color theme | not `mavzu` (topic/subject) |

### Metrics and miscellaneous
| concept | term | sense | hazard guarded |
|---|---|---|---|
| noise / noise temperature | shovqin / shovqin harorati | electronic/thermal noise | not acoustic racket |
| efficiency | samaradorlik | — | — |
| interpolation | interpolatsiya | — | — |
| mnemonic | mnemonika | a card's code descriptor | not a memo/note |
| degrees / deg | gradus / (grad) | freestanding prose vs parenthetical unit tag | tag treated like other unit tags (topic 4) |
| diameter | diametr | canonical loanword | locked over any native synonym |
| reflect (geometry) | aks ettirish | geometry mirror operation | distinct sense 1 of 3 |
| reflect (behavioral) | kuzatib boradi | control tracking another ("mirrors …") | distinct sense 2 of 3 |
| reflect (physics) | qaytish | physical reflection | distinct sense 3 of 3 |
| default(s) | standart qiymat | fallback value | — |
| normalize / normalization | meʼyorlashtirish / meʼyorlash | — | native term, not transliterated |

## 11. Disambiguation policy
- The correct technical sense is chosen per context for every ambiguous term (`yer`, `yadro`, `oqim`-family terms).
- No qualifier absent from the source is added; program context already disambiguates.
- A qualifier is added only where the target term would otherwise be genuinely ambiguous, eg `koʻruvchi kuchaytirish` (Viewer Gain) never gains an added `yoʻnalishidagi` (direction) unless the source string is "Gain in Viewer Direction".
- Accepted intra-domain homonym: `yadro` (kernel) serves both the integral-equation kernel and, in developer strings, an OS kernel; context disambiguates.
- No locative homonym collision remains after separating `yer` (ground) from `yer yuzasi` (earth).
- Gerund vs noun senses of an overloaded source word are distinguished by the standard `-ish` suffix, eg `masshtablash` (the act of scaling) vs `masshtab` (a scale).

## 12. Cross-catalog consistency
- One term per concept, reused from the topic-10 table; no synonym coined for an already-mapped concept.
- Keep every Appendix C distinction from topic 10: polarity/polarization, magnitude/amplitude, peak value/peak magnitude, ground/earth, load/charge, gain/amplification/profit, current/temporal, charge/billing, wire/cable/thread, radiation pattern/template/far-field, excitation/excitement, node-antinode/generic zero, scale family/hue/palette kind/color projection, comet/geometry, identity/unity, renderer/render engine, override/overwrite, viewer/observer/speaker/preview, reflect/mirrors/reflection, structure/construction, theme/topic, validation/verification, net gain/real-part gain, and electrical power/`Power`; never reuse one side's topic-10 term for another.
- Loanword vs native: use the locked loanwords `vidjet`, `fayl`, `renderer`, `gradient`, and `karta`; use established native terms `oʻtkazuvchanlik`, `meʼyorlashtirish`, and `qarshilik`.
- Minority-outlier spellings unify to the canonical form: `logarifmik` (never `logaritmik`); `proyeksiya` (never `proeksiya`); `sikl` (never `tsikl`).
- Consistency priority outranks locale-form preference for: `gradient` (kept as loanword over a native calque), `renderer` (kept over `render dvigateli`), `tema` (kept distinct from `mavzu` despite native overlap).

## 13. Priority ordering
- Precedence chain: correct meaning, then interface convention, then catalog consistency, then disambiguation, then locale numeral form.
- Explicit override: `tema` overrides the native-preference default (topic 12) to resolve the theme/topic false-friend pair (correct meaning outranks native-term preference).
- Explicit override: `Darajaviy` translates the `Power` family name against the general never-translate rule for algorithm names (topic 4), because the source itself treats it as descriptive.

## 14. Grammatical number
- Uzbek distinguishes singular from plural with `-lar`; use plural nouns, invariant adjectives and participles, and number-neutral verb forms where the literal meaning is plural.
- After an explicit literal count, use the unmarked singular noun, eg `3 karta`, not `3 kartalar`.
- Do not infer plural marking from interface context when the source does not express plurality.

## 15. Grammatical agreement
- Uzbek has no grammatical gender; this topic is not applicable beyond case marking.
- Adjectives and participles are invariant; no gender/number concord with the head noun.
- Case suffixes (accusative `-ni`, genitive `-ning`, dative `-ga/-ka/-qa`) mark grammatical role per standard Uzbek grammar; no partitive-after-count shift exists.
- No reflexive-particle retention rule applies.
- Standalone labels carry no default gender.

## 16. Morphological derivation
- Form borrowed technical verbs with native `-la` or `-lash`, eg `renderlash`; do not use `-irovka` or `-irovat` derivations.
- Verbal nouns use the native `-ish` gerund suffix, eg `koʻrish`, `masshtablash`.
- Loanword root plus native affix is preferred over a full calque.
- Compounding: technical compounds are spaced by default (topic 1); fused only for established single-word loans.

## 17. Preposition and sandhi selection
- Select dative `-ga`, `-ka`, or `-qa` and ablative `-dan` or `-tan` from the stem-final sound under standard Uzbek consonant harmony.
- Elision and contraction are not applicable to Uzbek translated literals.

## 18. Card/record-label register
- Fixed designator form in dialog/editor titles: capitalized `Karta`, eg `GW Kartasi`.
- Running-prose form in messages: lowercase possessive-suffixed `kartasi`, eg `GW kartasi oʻchirildi`.
- Generic-noun casing: lowercase mid-sentence, capitalized only sentence-initial or in a title; no separate short-vs-long form exists.
- Each register (title vs prose) stays internally consistent and is never cross-converted.

## 19. Multi-paragraph and whitespace fidelity
- Mirror source paragraph breaks at the same positions, preserving blank-line and single-line distinctions.
- Drop any trailing clause absent from the current source rather than retaining inherited text.
- Preserve source trailing newlines and punctuation.
- Preserve semantic line breaks; introduce no line break solely for visual wrapping.
- Preserve complete meaning and natural wording without abbreviation or truncation for an assumed display limit.

## 20. Current-source fidelity
- Derive every translation from the current source literal and its supplied context.
- Reuse inherited wording only when its complete meaning agrees with the current source.
- Do not inherit pre-1995 orthography, `tsikl`, `logaritmik`, or `Momentan`; use the topic-1 standard and the canonical topic-10 terms.

## 21. Script hygiene
- Require zero wrong-script homoglyphs in Uzbek prose; characters outside Uzbek Latin are allowed only inside retained tokens whose literal spelling requires them.
- Require zero U+0027 or U+2019 substitutes for Uzbek U+02BB in translated words.
- Translate plain foreign prose; retain only identifiers, units, proper names, and other topic-4 tokens.

## 22. Rule-file scope hygiene
- Keep only current decisions that can alter wording, Unicode characters, punctuation, capitalization, embedded tokens, mnemonic markers, or semantic whitespace in a translated literal.
- Omit catalog representation, serialization, headers, flags, workflow, rendering, fonts, shaping, layout, widget behavior, runtime formatting, sorting, search, display sizing, history, status, metadata, provenance, and citations.
- State each decision directly without implementation procedure or alternative policy.

## 23. Section-disjointness declaration
- Script mechanics in topics 1, 3, and 9 govern character and punctuation form only.
- Phrasing and structure in topics 6, 7, 16, 18, and 19 govern word and sentence construction only.
- Address register in topic 8 governs social formality only.
- Keep these axes non-overlapping so each concept has one governing section.

## 24. Developer/debug-string policy
- Translate user-facing controls and dialogs, informational status and error messages, and developer-facing diagnostics.
- Review user-facing families before informational and developer-facing families; review priority never permits an applicable literal to remain untranslated.
- Use terse technical declaratives for developer-facing diagnostics; use topic-7 grammar for user-facing and informational families.
- Preserve every embedded identifier, function name, format specifier, unit, and retained token in every string family.
- Apply the same policy throughout each subsystem; no sibling-family exception overrides it.
