# cs translation rules

## 1. Script and orthography
- Scope: Czech for the Czech Republic, written in the Latin script under current codified Czech orthography, for professional electrical and RF engineers, antenna designers, and amateur-radio operators.
- Required letters with diacritics: `á č ď é ě í ň ó ř š ť ú ů ý ž`; preserve every diacritic and never fold it to a base letter or look-alike.
- Use precomposed Unicode forms; omit combining-mark sequences.
- Use `U+2019` for an apostrophe in Czech prose; omit ASCII apostrophe and modifier-letter look-alikes. No other ambiguous native mark requires a fixed code point.
- Textual joiners are not applicable to Czech orthography.
- Write all prose left-to-right; retain embedded technical tokens in their source order without mirroring or manual direction controls.
- Czech distinguishes letter case; keep common nouns lowercase except at sentence start or where topics 6 and 18 specify a title form.
- Use one normal space between words and between Czech text and embedded technical or numeric tokens; use a non-breaking space between a literal number and unit symbol, eg `10 MHz`.
- Form compounds according to codified usage: prefer an established multiword technical phrase, eg `vyzařovací diagram`; fuse or hyphenate only an established lexical compound.

## 2. Numerals in literals
- Use European digits `0`-`9` for literal technical values in Czech prose.
- Write a decimal comma and space grouping in localized prose, eg `1 250,5`.
- Retain formulas, source examples, fixed defaults, and named mathematical or standards constants exactly when they are fixed technical tokens rather than prose numbers.
- Form an ordinal with a period after the digit, eg `1.`; use a hyphen before a Czech suffix when a literal index requires one, and never treat that separator as a decimal mark.
- Preserve digits inside format specifiers under topic 5.

## 3. Punctuation and quotation
- Native quotation marks are „ “; embedded technical tokens keep plain `"..."` for catalog-wide consistency.
- Native punctuation replaces source counterparts in prose: comma, question mark, exclamation mark, opening marks follow Czech typographic convention.
- No space before colon or semicolon; one space after.
- Ellipsis renders as `…`; dashes in source ranges are retained as `–`.
- Sentence terminator: full sentences take a period; short labels and fragments (menu items, field labels) omit it.
- Punctuation inside embedded technical runs (paths, specifiers, identifiers) stays in source form.

## 4. Never-translate tokens
- NEC2 card mnemonics kept verbatim: `GW GA GH EX LD FR RP GE EN SP SM SC NE NH NT TL GN GD GC GX GR GS GM GF CP CM SY XQ EK KH PQ PT`.
- Unit symbols kept verbatim: `MHz`, `dBi`, `Ω`, `dB`, `deg`/`°`, `%`, `K`, `S/m`.
- Figure-of-merit tokens kept verbatim: `VSWR`, S-parameter tokens, `Z`/`Z0`, front/back-ratio and gain/temperature tokens.
- File extensions kept verbatim: `.nec`, `.csv`, `.s1p`, `.s2p`, `.png`.
- Format specifiers kept verbatim; see topic 5.
- Embedded identifiers (function/variable/config-key names) inside developer strings kept verbatim; see topic 24.
- Physical/mathematical symbol letters (`E`, `H`, `Z`, `φ`, `θ`) kept in source form.
- Product/library/tool names kept verbatim: `NEC2`, `Smith` (chart name), and any graphics/toolkit library names appearing in strings.
- Named transfer-function and algorithm terms kept verbatim: `Asinh`, `μ-law`, `Reinhard`, `Sigmoid`, `Identity`; translate the descriptive transfer-family name `Power` as `Mocninná` to distinguish it from electrical `výkon`.
- Conditional loanwords, decided file-wide: retain `segment` and process term `fork`; translate `patch` as `ploška` and `tag` as `značka` in every literal.
- Geometry/domain loanwords outside this roster translate when Czech has an established engineering term; roster tokens never translate or transliterate.
- Retain every roster token exactly, including case, punctuation, script, and left-to-right order under topic 1.

## 5. Format-specifier integrity
- Every source specifier is preserved, same set, none added or removed.
- Default order is preserved; reorder only via positional syntax (`%1$s`), never by moving specifier tokens.
- For word-order divergence, restructure the sentence around fixed specifier positions rather than reordering specifiers.
- Numbers inside specifiers are never localized to comma decimal or space grouping.

## 6. Capitalization and title-case
- Default casing for labels, menu items, and titles is sentence case; English-style Title Case is forbidden.
- Axis labels (X/Y/Z) stay uppercase in UI headers.
- A lowercase math/coordinate variable in the msgid stays lowercase, eg `z = 0`; it names a variable, not a UI axis label.
- No coordinated-option all-caps exception exists in this catalog.
- A generic card/record noun stays lowercase in running prose; capitalizes only as a dialog/editor title (topic 18). Acronyms (`NEC2`, `VSWR`) and proper nouns (`Smith`) retain source casing.

## 7. Interface register by string type
- Commands, buttons, and menu actions use a subjectless infinitive with the object or complement in natural Czech order, eg `Uložit model`; omit imperatives, gerunds, and needless nominalization.
- Field labels use a concise noun phrase in natural modifier-head order plus the source colon, eg `Referenční frekvence:`; omit a subject and finite verb.
- Dialogs and confirmations use full sentences in natural subject-verb-object order; omit the subject where Czech naturally recovers it, and use the mood required by the message.
- Tooltips use the shortest complete declarative sentence; when the source describes an unavailable control, state the disabling condition before or with the unavailable action.
- Status and error messages use terse impersonal, passive, or third-person declarative clauses; present the affected entity before the result when that order aids diagnosis.
- Name user-visible domain entities with the locked topic-10 term and natural Czech modifier-head order; retain proper names under topics 4 and 8.
- Prefer complete native multiword phrases over source-order calques, fused inventions, mechanical length matching, or abbreviations that omit meaning.
- Developer and debug strings follow topic 24.

## 8. Formality and address
- Czech distinguishes informal `ty` from formal `vy` in 2nd-person verb conjugation.
- Default interface register avoids the choice via the impersonal infinitive (topic 7); `ty` is forbidden throughout the catalog.
- Where direct 2nd-person address is unavoidable (confirmation dialogs), use formal `vy` conjugation.
- Impersonal/subject-drop phrasing carries register elsewhere: infinitive commands, passive/impersonal status messages.
- Gender-marking avoidance: a standalone adjective label agrees with its implied noun's gender, never a default masculine (topic 15).
- Formality is realized morphologically through the verb ending (`vy`-form conjugation) wherever 2nd person appears.
- Register splits by string type: commands take the infinitive, dialogs take a full sentence with formal `vy`, messages stay impersonal.
- Confirmation-dialog pattern: use a full interrogative sentence with formal second-person plural only when direct address is unavoidable; otherwise use an impersonal construction.
- Avoid gendered references to the user; where grammatical gender or number is required for another person, agree naturally with the named or contextually known referent without using masculine as a universal default.
- Use inclusive professional role terms that preserve required Czech agreement and animacy.
- Honorifics are not applicable to interface address; grammatical person is impersonal by default and formal second-person plural only under the preceding rule.
- Write personal names in the person's supplied order and form; do not add or translate honorifics.
- Prohibited registers: informal second-person singular, casual, slang, commercial, archaic, and ceremonially over-formal.

## 9. Accelerator/hotkey mnemonics
- Mnemonic marker is a leading underscore `_` directly before the mnemonic letter, matching the msgid convention.
- Mnemonic letters stay unique per container (menu/dialog); a collision shifts to the next distinguishing letter in the same translated term.
- The mnemonic letter is drawn from the translated Czech term, never transliterated from the source English letter.
- No non-Latin presentation applies; Latin script only.
- Never invent a mnemonic where the source msgid carries none.
- Menu items and buttons carry mnemonics; labels, tooltips, and status messages never do.
- Avoid a diacritic letter as the mnemonic where a plain-letter alternative exists in the same word; diacritics are harder to type as accelerators.

## 10. Domain lexicon

- Lock each concept to the listed Czech term. The sense column fixes its engineering meaning; where no separate hazard is named, the mapping prevents synonym drift and preserves catalog consistency.

### Electrical primitives
| Concept | Czech | Sense / hazard |
|---|---|---|
| current | proud | electrical (A); never "aktuální/nynější" (temporal) |
| charge | náboj | electrical (C); never billing/fee |
| voltage | napětí | electric potential |
| power (electrical) | výkon | watts, radiated or dissipated power, power gain, and power flow; distinct from transfer-family `Mocninná` and mathematical exponentiation |
| impedance | impedance | complex Z; kept loanword, distinct from resistance/reactance |
| resistance | odpor | real part of Z |
| reactance | reaktance | imaginary part of Z |
| inductance | indukčnost | |
| capacitance | kapacita | |
| conductivity | vodivost | material S/m |
| admittance | admitance | distinct from impedance |
| load | zátěž | LD-card impedance; never "váha/náklad" |
| gain | zisk | antenna directivity (dB); never profit or amplifier amplification |
| excitation | buzení | EM energy input/source; never emotional excitement |
| feedpoint | napájecí bod | |
| port | port | excitation/S-parameter port; kept loanword |
| radials | radiály | ground-plane radial wires (noun); distinct from adjective "radiální" |

### Ground and earth
| Concept | Czech | Sense / hazard |
|---|---|---|
| ground / ground plane | zem | RF reference plane across GN/GD cards, type, conductivity, effects, and model; never physical soil or protective earthing |
| earth (physical medium) | zemský povrch / půda | terrain/noise-model earth; distinct from electrical zem |
| ground wave | zemská vlna | propagation term, distinct from ground reference |

### Geometry primitives
| Concept | Czech | Sense / hazard |
|---|---|---|
| wire | vodič | thin conductor/GW element; never "kabel/šňůra" |
| segment | segment | NEC2 subdivision, kept loanword |
| patch | ploška | NEC2 surface element in every literal; never an untranslated label or software correction |
| tag | značka | geometry identifier; never a UI label or card |
| card | karta | NEC2 input record; register in topic 18 |
| kernel | jádro | integral-equation kernel; never OS kernel |
| cliff | sráz | two-medium ground-boundary type; never fracture/break |
| structure | struktura | antenna model geometry; never "konstrukce" |
| model | model | NEC model or noise-temperature model |
| geometry | geometrie | model geometry |
| crossed | zkřížený | transmission-line conductors reversed; never cut/severed |

### Field, pattern, viewer
| Concept | Czech | Sense / hazard |
|---|---|---|
| field (EM) | pole | near/total/E/H field; distinct from a data/config field |
| near field / far field | blízké pole / vzdálené pole | opposed regions, kept symmetric |
| far-field contribution | příspěvek vzdáleného pole | per-direction, not near-field animation |
| radiation | vyzařování | |
| radiation pattern | vyzařovací diagram | plotted directional response; never a template/design |
| gain pattern | diagram zisku | |
| polarization | polarizace | antenna/wave field orientation |
| polarity | polarita | sign of a quantity; false friend of polarizace |
| phase | fáze | |
| reference phase | referenční fáze | |
| frequency | frekvence | |
| wave / wavelength | vlna / vlnová délka | |
| standing wave / traveling wave | stojaté vlnění / postupné vlnění | opposed pair |
| node / antinode | uzel / kmitna | standing-wave zero/maximum; also the null/peak overlay sense |
| crest | hřeben (vlny) | instantaneous wave apex; distinct from a curve/step peak |
| magnitude | velikost | modulus of a quantity (|Z|); distinct from amplitude |
| amplitude | amplituda | oscillating-quantity peak; distinct from magnitude |
| peak value | špičková hodnota | distinct UI option from peak magnitude |
| peak magnitude | špičková velikost | distinct UI option from peak value |
| instantaneous | okamžitý/á/é | projection mode; add `(φ=0)` only where source carries it; agrees with implied noun's gender (topic 15) |
| Poynting vector | Poyntingův vektor | |
| solid angle | prostorový úhel | |
| net gain | čistý zisk | total-minus-mismatch gain; never "reálná část zisku" |
| viewer | pohled | observation direction or 3D view; never a person, speaker, or preview |
| flow / flow direction | tok / směr toku | patch/current flow |
| total field | celkové pole | |

### Color, tone, animation subsystem
| Concept | Czech | Sense / hazard |
|---|---|---|
| color | barva | |
| color projection | barevná projekce | which quantity drives hue |
| hue | odstín | color-wheel angle |
| brightness | jas | luminance channel |
| hue encoding / brightness encoding | kódování odstínu / kódování jasu | distinct enums, neither collapses to "color projection" |
| color scale | barevná škála | magnitude-to-color scale |
| scale family / color tone | rodina škál | Power/Log/Asinh/μ-law/Reinhard/Sigmoid/Identity transfer-curve family; one concept, one term |
| palette / palette kind | paleta / druh palety | distinct from scale family and color projection |
| ramp | lineární přechod | a palette kind |
| gradient | barevný přechod | linear color strip |
| gamma | gama | power-law exponent |
| knee | koleno | soft-knee bend point |
| softening | změkčení | |
| compression | komprese | dynamic-range |
| contrast | kontrast | |
| dynamic range | dynamický rozsah | |
| floor | spodní mez | brightness/dB minimum clamp; never a room floor |
| envelope | obálka | magnitude/amplitude envelope |
| comet | kometa | moving-crest overlay effect; never geometry (fuzzy-inheritance hazard, topic 20) |
| overlay (noun) | překryvná vrstva | added visual layer; distinct from the verb "překrýt" |
| animate / animation | animovat / animace | |
| animated / static | animované / statické | category-header adjectives, neuter agreement |
| projection | projekce | color or geometry projection |
| scale (noun) | měřítko | a scale |
| scale (verb) | škálovat | to scale |
| wireframe | drátěný model | wire-mesh render mode |
| identity | identita | no-op/passthrough transfer; distinct from unity (Smith-chart) |
| sentinel | hlídací hodnota | unreachable-case guard value |
| bins | koše | discretization buckets |
| companding | companding | bounded log curve (μ-law); kept loanword, no established native term |
| tone mapping | mapování tónů | photographic tone-map |

### Render and compute
| Concept | Czech | Sense / hazard |
|---|---|---|
| renderer | vykreslovač | drawing backend; never "render engine" |
| shader | shader | kept loanword |
| allocation (memory) / managed allocator | alokace / správce alokací | |
| thread | vlákno | compute thread; no collision with wire (vodič) in Czech |
| widget | ovládací prvek | UI element |
| validation | validace | the validation-tree feature; distinct from verification/kontrola |
| batch mode | dávkový režim | |
| fork (process) | fork | kept verbatim |
| deadlock | uváznutí | established native CS term |
| notifier | oznamovač | |
| token / operand / operator / arity | token / operand / operátor / arita | expression-parser terms |
| override | potlačit | supersede a value (SY symbol); distinct from overwrite |
| swap | prohodit | exchange |
| theme | motiv | UI/color theme; never "téma" (subject/topic) |

### Metrics and miscellaneous
| Concept | Czech | Sense / hazard |
|---|---|---|
| noise / noise temperature | šum / šumová teplota | electronic/thermal; never "hluk" (acoustic racket) |
| efficiency | účinnost | |
| interpolation | interpolace | |
| mnemonic | mnemonický kód | a card's code descriptor; never a memo/note |
| degrees / deg | stupně / (deg) | freestanding prose form vs the parenthetical unit tag, tag treated like other unit tags |
| diameter | průměr | canonical native term, chosen over loanword "diametr" |
| reflect (geometry) | zrcadlit | mirror operation |
| reflect (behavioral) | sledovat/kopíruje | one control tracking another |
| reflect (physics) | odrážet | reflection |
| default(s) | výchozí hodnota(y) | fallback value |
| normalize / normalization | normalizovat / normalizace | native verb pattern, not a transliteration |

## 11. Disambiguation policy
- Choose the correct technical sense for each ambiguous term; no qualifier absent from the source is added, since program context (an EM simulator) already disambiguates, eg `Proudy` for "Currents", never `Elektrické proudy`.
- Add a qualifier only where the Czech term would otherwise be genuinely ambiguous.
- No accepted intra-domain homonym overrides the locked topic-10 terms; electrical `výkon` and transfer-family `Mocninná` remain distinct.
- Locative collision: `zem` (electrical ground) vs `zemský povrch` (physical earth) avoided by reserving the short form for the electrical sense only.
- `buzení` (excitation) covers both the noun and the action sense; no separate gerund form needed.

## 12. Cross-catalog consistency
- One term per concept, reused from the established lexicon (topic 10); no synonym introduced for an already-mapped concept.
- Keep every false-friend side distinct: `polarita`/`polarizace`; `velikost`/`amplituda`; `špičková hodnota`/`špičková velikost`; `zem`/`zemský povrch`; `zátěž`/`náboj`; antenna `zisk`/amplifier `zesílení`/profit `zisk`; electrical `proud`/temporal `současný`; electrical `náboj`/fee `poplatek`; `vodič`/`kabel`/compute `vlákno`; `vyzařovací diagram`/design `vzor`/`vzdálené pole`; `buzení`/emotion `vzrušení`; standing-wave `uzel` and `kmitna`/numeric `nula`; `rodina škál`/`odstín`/`druh palety`/`barevná projekce`; `kometa`/`geometrie`; transfer `identita`/Smith-chart `jednotka`; `vykreslovač`/renderovací jádro; `potlačit`/`přepsat`; `pohled`/`pozorovatel`/`reproduktor`/`náhled`; `zrcadlit`/`sledovat`/`odrážet`; `struktura`/`konstrukce`; `motiv`/`téma`; `validace`/`ověření`; `čistý zisk`/`zisk reálné části`; electrical `výkon`/transfer-family `Mocninná`. Never reuse one side for the other.
- Loanword-vs-native decisions locked: `impedance`, `segment`, `admitance`, `port`, `shader`, `companding`, `fork` kept as loanwords; `zisk`, `zátěž`, `odpor`, `vodivost` use native roots.
- Minority-outlier spellings unify to the diacritic-correct canonical form; a stripped-diacritic variant never stands.
- Catalog consistency locks `ploška`, `zem`, `pohled`, and `Mocninná` across all applicable literals even where an isolated prior label used another form.

## 13. Priority ordering
- Precedence chain: correct meaning, then interface convention (topic 7), then catalog consistency (topic 12), then disambiguation (topic 11), then locale numeral form (topic 2).
- Recorded overrides: correct electrical sense selects `zem` over terrain terms; interface convention selects infinitive commands; catalog consistency selects `ploška`, `pohled`, and `Mocninná` over prior outliers.

## 14. Grammatical number
- Czech uses singular and plural noun, adjective, participle, and verb forms; choose among the catalog's three count forms by `n==1`, `n==2..4`, and all other values.
- After an explicit literal count, use singular nominative for `1`, plural nominative for `2`-`4`, and genitive plural for `0`, `5` and above, and fractional counts; make agreeing adjectives, participles, and verbs follow that form.

## 15. Grammatical agreement
- Make adjectives, pronouns, participles, and finite verbs agree with the head noun or subject in gender, number, person, and animacy where Czech marks them.
- Resolve a standalone label from its implied or sibling head noun, never from a default masculine; eg implied neuter `zobrazení` gives `Okamžité`, while feminine `hodnota` gives `Okamžitá (φ=0)`.
- Use the established declension class of each topic-10 noun; after counts, apply topic 14's nominative or genitive form and propagate agreement through modifiers.
- Retain required reflexive particles `se` and `si`, eg `načíst se`; omitting one changes the verb's grammar or sense.

## 16. Morphological derivation
- Borrowed technical verbs use the standard `-ovat` suffix only where no native verb exists; a native equivalent is preferred otherwise.
- Verbal-noun formation uses `-ování`/`-ení`, eg `vykreslování` for rendering.
- Native-affix roots are preferred; a loanword is reserved for internationally fixed EE/RF terms (`impedance`, `admitance`).

## 17. Preposition and sandhi selection
- Euphonic-vowel insertion before a consonant cluster is the only context-conditioned form selection: `k`/`ke`, `s`/`se`, `v`/`ve`, `z`/`ze`.
- No elision/contraction rule applies in formal technical Czech.

## 18. Card/record-label register
- Use the fixed capitalized designator `Karta` or the specific topic-10 record term in dialog and editor titles, eg `Ploška`.
- Use the declined lowercase noun in running prose, eg `karta`, `ploška`, or `plošky`.
- Keep the generic noun lowercase outside sentence start; shorten only by omitting a recoverable generic noun, never by retaining a source-language label.
- Keep title-designator and running-prose forms internally consistent; do not cross-convert their casing or inflection.

## 19. Multi-paragraph and whitespace fidelity
- Mirror every source paragraph break at the same position, preserving the distinction between a blank line and a single semantic line break.
- Drop a trailing clause removed from the current source; never inherit it from an older translation.
- Carry source trailing newlines and punctuation mechanically.
- Preserve semantic line breaks; introduce no visual wrapping absent from the source literal.
- Preserve complete meaning; never truncate or abbreviate for an assumed display constraint.

## 20. Current-source fidelity
- Derive every translation from the current source literal and supplied context.
- Reuse an inherited translation only when its complete meaning matches the current source.
- Re-derive entries involving `kometa` versus `geometrie`, `ploška` versus unrelated label senses, and electrical `výkon` versus transfer-family `mocninná škála`; prior wording is unsafe evidence for these concepts.

## 21. Script hygiene
- Permit non-Czech letters and Greek symbols only inside a retained token listed in topic 4; translate ordinary foreign words.
- Forbid Cyrillic and Greek homoglyphs in Czech prose where Latin letters are required, including mixed-script adjacency within a Czech word.
- Preserve a retained token's literal script and code points even where a native look-alike exists.

## 22. Rule-file scope hygiene
- Keep only current decisions that can alter wording, Unicode characters, punctuation, capitalization, embedded tokens, mnemonic markers, or semantic whitespace in a translated literal.
- Omit catalog storage, headers, flags, workflow, rendering, fonts, shaping, layout, widget behavior, runtime formatting, sorting, search, display sizing, audit history, completion state, review metadata, provenance, and citations.
- State each retained decision directly; include no implementation procedure, bibliography, correction history, or competing policy.

## 23. Section-disjointness declaration
- Script mechanics (topics 1-3) govern character/punctuation form only; phrasing and structure (topics 6-7, 14-19) govern word choice and sentence shape; address register (topic 8) governs formality alone. Each concept lands in exactly one section.

## 24. Developer/debug-string policy
- Translate user-facing errors, warnings, confirmations, tooltips, labels, and status messages; review priority never permits an applicable user-facing literal to remain untranslated.
- Translate informational diagnostics intended for operators in terse, technical Czech.
- Keep developer-only debug diagnostics in source form unless an established sibling family in the same subsystem translates its prose; then follow that family consistently.
- Preserve every embedded identifier, function name, format specifier, unit, and retained token verbatim in every family, independent of translation priority.
- Classify a literal by its audience and purpose, not merely by an identifier-like prefix: user-facing requests action, informational reports operator-relevant state, and developer-facing diagnoses implementation state.
- Render the `BUG:` diagnostic prefix as "DEFEKT:", held distinct from the "CHYBA:" prefix that carries ordinary failures; the two severities never share a word.
- Keep an identifier-plus-value diagnostic pair contiguous (eg `sweep_state=%d`); a qualifier precedes the whole pair and never stands between the identifier and its `=`.
