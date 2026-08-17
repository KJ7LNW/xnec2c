# sk translation rules

## 1. Script and orthography
- Scope: Slovak (`sk`) for Slovakia, written in the Latin script under the current codified Pravidlá slovenského pravopisu, for electromagnetic-simulation software used by professional electrical and radio-frequency engineers, antenna designers, and amateur-radio operators.
- Required diacritics: á ä č ď é í ĺ ľ ň ó ô ŕ š ť ú ý ž; retain them without stripping or folding to look-alikes.
- Precomposed Unicode forms only; never decomposed base+combining accent; never substitute a plain apostrophe for the ľ/ť/ď/ň soft-mark.
- No joining/shaping mechanics: not applicable, Latin script.
- Directionality: left-to-right; no bidirectional handling needed.
- Case distinction: yes, upper/lower case exists; casing rules in topic 6 apply.
- Orthographic standard: current codified Pravidlá slovenského pravopisu; no regional variant.
- Single space between words and between native text and embedded technical/numeric tokens; no thin space.
- Compounding: use spaced modifier phrases by default; use a hyphen only for established coordinated compounds, and do not fuse unrelated technical nouns.

## 2. Numerals in literals
- Use digits `0-9` for technical values in Slovak prose; no alternate digit set applies.
- Use a comma as the decimal separator (`50,0`) and a non-breaking space as the grouping separator (`1 000`) in numbers physically present in translated prose, preserving Slovak numeric meaning.
- Retain formulas, worked examples, fixed defaults, and named mathematical or standards constants exactly in source form so their technical value remains unchanged.
- Form ordinals with a digit plus period (`1.`); use the source-defined separator for an index, never reinterpret it as a decimal separator.

## 3. Punctuation and quotation
- Use Slovak quotation marks `„…“` for quoted prose; retain source quotation marks inside embedded technical tokens so literal token spelling stays unchanged.
- Native punctuation replaces source counterparts in prose: comma before subordinate clauses (že, ktorý, ...), question mark, exclamation mark; no opening inverted marks.
- No space before colon/semicolon/terminal punctuation; one space after.
- Ellipsis: single `…` character, never three periods; dash: en dash `–` for ranges, hyphen `-` for compounds.
- Sentence terminator: period; short labels/fragments omit it, full sentences take it.
- Punctuation inside embedded technical runs (inside `%s` substitutions) stays in source form.

## 4. Never-translate tokens
- NEC2 card mnemonics kept verbatim: GW GA GH EX LD FR RP GE EN, and extended SP SM SC NE NH NT TL GN GD GC GX GR GS GM GF CP CM SY XQ EK KH PQ PT.
- Keep unit symbols verbatim: `Hz`, `kHz`, `MHz`, `GHz`, `dB`, `dBi`, `Ω`, `W`, `K`, `S/m`, `deg`, and `%`; these standard symbols are never translated or transliterated.
- Keep figure-of-merit and parameter tokens verbatim: `VSWR`, `S11`, `S12`, `S21`, `S22`, `Z`, `Z0`, `F/B`, and `G/T`; these established RF tokens are never translated or transliterated.
- Keep file extensions verbatim: `.nec`, `.csv`, `.s1p`, `.s2p`, and `.png`; preserve literal filenames.
- Keep every format specifier verbatim under topic 5; preserve runtime substitution tokens.
- Keep embedded identifiers, function names, variable names, and configuration keys verbatim in developer strings; preserve code identity.
- Keep physical and mathematical symbol letters verbatim, including `E`, `H`, `φ`, and `γ`; prevent native look-alikes from changing notation.
- Keep product, library, toolkit, and chart names verbatim: `xnec2c`, `NEC2`, `GTK`, `Cairo`, `OpenGL`, `GLib`, `GSL`, and `Smith`; preserve proper names.
- Keep named transfer functions and algorithms verbatim: `Log`, `Asinh`, `μ-law`, `Reinhard`, `Sigmoid`, and `Identity`; translate the descriptive `Power` family only as `Mocninová`.
- Conditional loanwords, decided once, file-wide: segment kept as "segment"; patch translates as "plôška"; tag kept as "tag"; fork (process) kept as "fork".

## 5. Format-specifier integrity
- Preserve exactly the source set and count of format specifiers; zero missing, added, or altered specifiers are permitted.
- Keep source order by default; use positional forms such as `%2$s` only when Slovak grammar requires reordering and the format family supports it.
- Restructure Slovak wording around fixed specifier positions when positional reordering is unavailable.
- Keep all numbers inside specifiers unchanged; they are token syntax, not localized numerals.

## 6. Capitalization and title-case
- Use sentence case for labels, menu items, and titles; capitalize no content word solely because it appears in a title.
- Axis letters (X, Y, Z) keep source uppercase; no lowercase override.
- Lowercase math/coordinate variables (x, y, z as variables) stay lowercase.
- Coordinated option names have no special capitalization: apply sentence case to the whole label so only its first element is capitalized unless a proper noun or retained token follows.
- Generic card/record noun (karta, segment) lowercase mid-sentence, capitalized sentence-initial; acronyms (NEC, VSWR) and proper nouns keep their casing.

## 7. Interface register by string type
- Commands, buttons, and menu actions: use a concise infinitive with the verb first and omit the subject; use one action form consistently rather than mixing imperatives and nouns.
- Field labels: use a noun phrase in head-before-modifier order where natural, omit subject and verb, and retain the source colon.
- Dialog statements: use complete formal declarative sentences in neutral subject-verb-object order; omit an recoverable subject, but retain every condition and consequence.
- Confirmations: use a complete formal question with the finite verb before its object and the implied formal subject omitted.
- Tooltips: use a concise descriptive sentence or infinitive action phrase; when the source explains a disabled control, state the blocking condition before the unavailable action.
- Status and error messages: use terse impersonal declaratives, place the failed action or state before its cause, and avoid first-person forms.
- User-visible domain entities: name the generic entity before its identifying mnemonic or number, following topic 18; proper and personal names follow topic 8.
- Prefer the shortest complete natural form: use spaced or hyphenated phrases under topic 1, preserve all meaning, and avoid unnatural abbreviations or source-length imitation.
- Developer and debug strings follow topic 24.

## 8. Formality and address
- Use a neutral professional register throughout; omit casual, slang, commercial, archaic, and ceremonially formal wording.
- Slovak distinguishes formal and informal address: use formal plural `vy` morphology when direct address is unavoidable and never use informal singular `ty` forms.
- Omit the subject pronoun where the verb ending identifies formal address; capitalize `Vy` only in direct personal correspondence, not ordinary interface text.
- Commands use the impersonal infinitive under topic 7; dialogs and confirmations use complete formal sentences, with confirmations formed as direct questions.
- Avoid gender marking through impersonal, passive, abstract-noun, or neutral constructions; where gender, number, or animacy is grammatically required, agree naturally with the named referent without treating one personal gender as universal.
- Honorifics are not applicable to interface literals; add none absent from the source.
- Use third person for named people and preserve the source personal-name order; direct second-person address remains formal.
- Personal names remain unchanged unless the source supplies an established localized form.

## 9. Accelerator/hotkey mnemonics
- GTK `_` mnemonic precedes the access-key letter.
- Place the marker immediately before a keyboard-typable letter in the translated term; resolve a same-container collision by moving it to another letter in that term.
- Mnemonic letter comes from the translated term, never a transliteration of the source.
- Non-Latin presentation (appended parenthetical Latin letter): not applicable, Slovak is Latin script.
- Never invent a mnemonic where the source string carries none.
- Widget-type distinction: buttons, menu items, and labelled entries carry mnemonics; tooltips and status messages never do.
- Diacritic mnemonics (Š, Č, Ž, Ľ, ...) are acceptable when they do not collide with a sibling; shift only on collision, avoiding hard-to-type letters when an ASCII alternative exists in the same word.
- Radiation-pattern menu: Škálovanie _zisku (z), Šumová _teplota (t), Štýl k_reslenia (r), Spoločná pro_jekcia (j) to avoid `p` clash with _Prekryť štruktúru.
- Visualization menu: Spoločná pro_jekcia (j) avoids `p` clash with Os _polarizácie; Špičková v_eľkosť (e).

## 10. Domain lexicon

### Electrical primitives
| Concept | Slovak | Note |
|---|---|---|
| current | prúd | never temporal "aktuálny" |
| charge | náboj | never cargo/billing "poplatok" |
| voltage | napätie | established electromagnetic-engineering sense; locks one catalog term |
| power (electrical) | výkon | distinct from "Power" scale-family name "Mocninová" |
| impedance | impedancia | distinct from odpor/reaktancia |
| resistance | odpor | real part of Z |
| reactance | reaktancia | imaginary part of Z |
| inductance | indukčnosť | established electromagnetic-engineering sense; locks one catalog term |
| capacitance | kapacita | established electromagnetic-engineering sense; locks one catalog term |
| conductivity | vodivosť | established electromagnetic-engineering sense; locks one catalog term |
| admittance | admitancia | distinct from impedancia |
| load | záťaž | LD-card impedance; distinct from náboj |
| gain | zisk | antenna directivity, established RF term; distinct from zosilnenie (amplification) |
| excitation | budenie | EM energy input or source; prevents the emotional sense vzrušenie and the competing synonym excitácia |
| feedpoint | napájací bod | established electromagnetic-engineering sense; locks one catalog term |
| port | port | established electromagnetic-engineering sense; locks one catalog term |
| radials | radiály | ground-plane radial wires, noun |

### Ground and earth
| Concept | Slovak | Note |
|---|---|---|
| ground / ground plane | zem | RF electrical reference for every GN/GD ground sub-sense; prevents soil and competing-term readings |
| earth (physical medium) | terén | physical terrain or noise-model medium; keeps it distinct from electrical zem |
| ground wave | prízemná vlna | distinct from zem reference |

### Geometry primitives
| Concept | Slovak | Note |
|---|---|---|
| wire | drôt | never kábel (cable/cord); distinct from vlákno (thread) |
| segment | segment | NEC2 geometry subdivision |
| patch | plôška | NEC2 surface patch |
| tag | tag | geometry identifier, not a UI label or card |
| card | karta | register in topic 18 |
| kernel | jadro | thin-wire integral kernel, not OS kernel |
| cliff | rozhranie prostredí | two-medium ground boundary, never zlom/prasklina |
| structure | štruktúra | antenna model geometry, never stavba/konštrukcia |
| model | model | NEC model or noise-temperature model |
| geometry | geometria | established electromagnetic-engineering sense; locks one catalog term |
| crossed | skrížené | reversed transmission-line conductors, never prerušené (cut) |

### Field, pattern, viewer
| Concept | Slovak | Note |
|---|---|---|
| field (EM) | pole | distinct from dátové pole (data field) |
| near field | blízke pole | established electromagnetic-engineering sense; locks one catalog term |
| far field | vzdialené pole | established electromagnetic-engineering sense; locks one catalog term |
| far-field contribution | príspevok vzdialeného poľa | per-direction, not near-field animation |
| radiation | žiarenie | established electromagnetic-engineering sense; locks one catalog term |
| radiation pattern | vyžarovací diagram | never šablóna/návrh (template/design) |
| gain pattern | ziskový diagram | established electromagnetic-engineering sense; locks one catalog term |
| polarization | polarizácia | distinct from polarita |
| polarity | polarita | sign of a quantity, never polarizácia |
| phase | fáza | established electromagnetic-engineering sense; locks one catalog term |
| reference phase | referenčná fáza | established electromagnetic-engineering sense; locks one catalog term |
| frequency | frekvencia | established electromagnetic-engineering sense; locks one catalog term |
| wave / wavelength | vlna / vlnová dĺžka | established electromagnetic-engineering sense; locks one catalog term |
| standing wave / traveling wave | stojatá vlna / postupná vlna | opposed pair |
| node | uzol | standing-wave zero, never literal "null" |
| antinode | kmitňa | standing-wave maximum, paired with uzol |
| crest | hrebeň (vlny) | instantaneous apex, comet overlay; distinct from vrchol |
| magnitude | veľkosť | modulus, never amplitúda |
| amplitude | amplitúda | oscillating peak, reserved for "amplitude" only |
| peak value | špičková hodnota | distinct from špičková veľkosť |
| peak magnitude | špičková veľkosť | distinct from špičková hodnota |
| instantaneous | okamžitý | add "(φ=0)" only where source carries it |
| Poynting vector | Poyntingov vektor | established electromagnetic-engineering sense; locks one catalog term |
| solid angle | priestorový uhol | established electromagnetic-engineering sense; locks one catalog term |
| net gain | celkový zisk | never "reálny (časť) zisk" |
| viewer | zobrazovač | observation direction/3D view widget, never pozorovateľ/reproduktor/náhľad |
| flow / flow direction | tok (prúdu) | established electromagnetic-engineering sense; locks one catalog term |
| total field | celkové pole | established electromagnetic-engineering sense; locks one catalog term |

### Color, tone, animation
| Concept | Slovak | Note |
|---|---|---|
| color | farba | established electromagnetic-engineering sense; locks one catalog term |
| color projection | farebná projekcia | which quantity drives hue |
| hue | odtieň | color-wheel angle |
| brightness | jas | luminance channel |
| hue encoding | kódovanie odtieňa | distinct enum from farebná projekcia |
| brightness encoding | kódovanie jasu | distinct enum from farebná projekcia |
| color scale | farebná stupnica | magnitude-to-color mapping, distinct from farebný prechod |
| scale family / color tone | rodina mierky | Power/Log/Asinh/μ-law/Reinhard/Sigmoid selector, distinct from farebná stupnica |
| palette / palette kind | paleta / druh palety | distinct from rodina mierky and farebná projekcia |
| ramp / gradient | farebný prechod | linear color strip, distinct from farebná stupnica |
| gamma | gama | keep γ symbol where source uses it |
| knee | koleno | soft-knee bend point, Asinh/Reinhard parameter k |
| softening | zjemnenie | distinct from kompresia |
| compression | kompresia | dynamic-range |
| contrast | kontrast | established electromagnetic-engineering sense; locks one catalog term |
| dynamic range | dynamický rozsah | established electromagnetic-engineering sense; locks one catalog term |
| floor | spodná hranica | brightness/dB floor, never "podlaha" |
| envelope | obálka | magnitude/amplitude envelope |
| comet | kométa | moving-crest overlay, never reused for geometria |
| overlay (noun) | prekrytie | added visual layer, distinct from verb prekryť |
| animate / animation | animovať / animácia | established electromagnetic-engineering sense; locks one catalog term |
| animated / static | animovaný / statický | category-header adjectives |
| projection | projekcia | color or geometry projection |
| scale (verb/noun) | mierka | to scale / a scale |
| wireframe | drôtený model | never "drôtový model", locked spelling |
| identity | identita | no-op transfer, distinct from jednotková (Smith-chart unity) |
| sentinel | strážna hodnota | unreachable-case guard value |
| bins | koše | discretization buckets |
| companding | kompandovanie | μ-law companding |
| tone mapping | tónové mapovanie | photographic, distinct from farebná projekcia |

### Render and compute
| Concept | Slovak | Note |
|---|---|---|
| renderer | vykresľovač | drawing backend, never "render engine" |
| shader | shader | loanword, no native equivalent established |
| allocation (memory) | alokácia | generic; distinct from spravovaný alokátor |
| managed allocator | spravovaný alokátor | allocator/report |
| thread | vlákno | distinct from drôt (wire) |
| widget | widget | loanword, established in catalog |
| validation | validácia | validation-tree feature; distinct from overenie for verification checks |
| batch mode | dávkový režim | established electromagnetic-engineering sense; locks one catalog term |
| fork (process) | fork | kept verbatim |
| deadlock | uviaznutie | established electromagnetic-engineering sense; locks one catalog term |
| notifier | upozorňovač | established electromagnetic-engineering sense; locks one catalog term |
| token | token | expression-parser lexical item; prevents a general voucher or marker sense |
| operand | operand | expression-parser value consumed by an operator; keeps it distinct from operator |
| operator | operátor | expression-parser operation; keeps it distinct from operand |
| arity | aritnosť | number of operands accepted by an operator; preserves parser terminology |
| override | nahradiť | supersede a value, SY symbol; distinct from prepísať (overwrite) |
| swap | vymeniť | established electromagnetic-engineering sense; locks one catalog term |
| theme | vzhľad | UI/color theme, never "téma" (topic/subject) |

### Metrics and miscellaneous
| Concept | Slovak | Note |
|---|---|---|
| noise / noise temperature | šum / teplota šumu | electronic/thermal, never acoustic |
| efficiency | účinnosť | established electromagnetic-engineering sense; locks one catalog term |
| interpolation | interpolácia | established electromagnetic-engineering sense; locks one catalog term |
| mnemonic | mnemonický kód | card code descriptor, never poznámka (memo) |
| degrees / deg | stupne / "(deg)" | freestanding prose vs parenthetical unit tag |
| diameter | priemer | canonical spelling, never archaic "diametr" |
| reflect (geometry) | zrkadliť | geometry mirror operation |
| reflect (behavioral) | sleduje | control tracking another; distinct from geometry zrkadliť and physical odraz |
| reflect (physics) | odraz | physical reflection |
| default(s) | predvolený | fallback value |
| normalize / normalization | normalizovať / normalizácia | translate, never transliterate |

## 11. Disambiguation policy
- Choose the correct technical sense for each ambiguous term from context.
- Never add a qualifier absent from the source; program context already disambiguates.
- Add a qualifier only where the Slovak term would otherwise be genuinely ambiguous.
- Accepted intra-domain homonym: "zisk" serves both antenna gain and financial profit; established RF term of art, context disambiguates.
- Locative homonym: "zem" also denotes country/land in general Slovak; electrical/terrain sense disambiguated by context.
- Gerund vs noun: "tok" (flow, noun) stays distinct from "prúdenie" (flowing, gerund); use "tok" for the domain concept.

## 12. Cross-catalog consistency
- One term per concept, reused from the established lexicon; never introduce a synonym for an already-mapped concept.
- Keep every Appendix C concept distinct under the locked topic-10 terms: `polarita` / `polarizácia`; `veľkosť` / `amplitúda`; `špičková hodnota` / `špičková veľkosť`; `zem` / `terén`; `záťaž` / `náboj`; `zisk` / `zosilnenie` / `profit`; `prúd` / `aktuálny`; `náboj` / `poplatok`; `drôt` / `kábel` / `vlákno`; `vyžarovací diagram` / `šablóna` / `vzdialené pole`; `budenie` / `vzrušenie`; `uzol` and `kmitňa` / `nula`; `rodina mierky` / `odtieň` / `druh palety` / `farebná projekcia`; `kométa` / `geometria`; `identita` / `jednotková`; `vykresľovač` / `vykresľovací mechanizmus`; `nahradiť` / `prepísať`; `zobrazovač` / `pozorovateľ` / `reproduktor` / `náhľad`; `zrkadliť` / `sleduje` / `odraz`; `štruktúra` / `konštrukcia`; `vzhľad` / `téma`; `validácia` / `overenie`; `celkový zisk` / `zisk reálnej časti`; `výkon` / `Mocninová`. Never use one member of a group for another concept.
- Loanword-vs-native: widget, shader, fork, tag, segment kept as loanwords; locked spelling for each.
- Minority-outlier spellings unify to the canonical form, eg "drôtený model" never "drôtový model".
- Catalog consistency outranks locale-form preference where the catalog already establishes a loanword (widget, segment, tag) over an available native alternative.

## 13. Priority ordering
- Precedence chain: correct meaning, then interface convention, then catalog consistency, then disambiguation, then locale numeral form.
- Override ruling: "widget" and "segment" stay loanwords despite native alternatives, because catalog consistency (topic 12) outranks locale-form preference.

## 14. Grammatical number
- Slovak translated literals distinguish singular, nominative plural for counts ending in `2`-`4` outside the teens, and genitive plural for zero, teens, and other counts.
- Inflect nouns, adjectives, participles, and finite verbs for the grammatical number required by the literal subject or explicit count.
- After an explicit literal count, use singular with `1`, nominative plural with `2`-`4`, and genitive plural with `0`, `5+`, and teen values; retain the exact count while selecting its natural Slovak form.

## 15. Grammatical agreement
- Adjectives and participles agree in gender, number, and case with the head noun, including the implied head noun of a standalone label.
- Apply the count-conditioned nominative or genitive plural from topic 14 to the head noun and every agreeing modifier.
- Reflexive particle "sa" retained where the verb requires it.
- Standalone label default gender: neuter singular for abstract UI settings (nouns in -ie, -enie).

## 16. Morphological derivation
- Borrowed technical verbs take the `-ovať` suffix (`kompandovať`, `interpolovať`); omit the nonstandard `-irovať` variant.
- Verbal-noun formation: "-ovanie" (kompandovanie, interpolovanie).
- Prefer the native root where an established native term exists (zisk, not a "gain" loanword); otherwise adopt the "-ovať" loanword pattern.
- Compounding: two-word or hyphenated phrase, never a fused compound (topic 1).

## 17. Preposition and sandhi selection
- Prepositions alternate before certain consonant clusters: s/so, k/ku, v/vo, z/zo (eg "so segmentom" before s+consonant).
- Elision/contraction: not applicable, Slovak has no articles to contract.

## 18. Card/record-label register
- Dialog/editor titles: fixed designator form ("Karta GW", "Segment č. X").
- Running prose: genitive/adjectival construction distinct from the title form ("v karte GW").
- Generic-noun casing: lowercase mid-sentence, capitalized sentence-initial (topic 6).
- Short-vs-long form: mnemonic alone ("GW") in tables, spelled form ("karta GW") in prose.
- Each register stays internally consistent, never cross-converted.

## 19. Multi-paragraph and whitespace fidelity
- Mirror source paragraph boundaries at the same positions, preserving the distinction between blank lines and single line breaks.
- Preserve semantic source line breaks; add no line break solely for visual wrapping.
- Drop a trailing clause removed from the current source rather than retaining stale translated meaning.
- Carry source trailing newlines and punctuation exactly unless Slovak sentence punctuation under topic 3 replaces the source counterpart.
- Preserve complete meaning; never truncate or abbreviate wording to fit an assumed display size.

## 20. Current-source fidelity
- Derive each translation from the complete current source literal and its supplied context.
- Reuse inherited wording only when its full meaning agrees with the current source; current meaning overrides prior wording.
- Do not inherit prior translations for changed plural counts, reassigned source text, or concepts whose technical sense differs from the current context.

## 21. Script hygiene
- Slovak prose permits base Latin letters and the diacritics listed in topic 1; zero wrong-script letters, foreign orthographic marks, or Latin homoglyph substitutions are permitted outside a retained token whose spelling requires them.
- Translate plain foreign prose into Slovak; retain genuine identifiers, units, symbols, and proper technical names under topic 4.

## 22. Rule-file scope hygiene
- Keep only current decisions that can alter the wording, Unicode characters, punctuation, capitalization, embedded tokens, mnemonic markers, or semantic whitespace of a Slovak translated literal.
- Omit catalog representation, serialization, headers, flags, validation procedure, rendering, fonts, shaping, layout, widget behavior, runtime formatting, sorting, search, display sizing, audit history, completion state, review metadata, provenance, and source citations.
- State each decision directly; include no implementation procedure, bibliography, correction history, or alternative policy source.

## 23. Section-disjointness declaration
- Treat script mechanics, phrasing and structure, and address register as non-overlapping axes: topics 1 and 21 govern characters and orthography; topics 2, 3, 5-7, 9, and 14-20 govern literal form and grammar; topic 8 alone governs social address. Place each decision on one axis only.

## 24. Developer/debug-string policy
- Translate user-facing controls, dialogs, errors, and actionable diagnostics into Slovak; review priority never permits an applicable user-facing literal to remain untranslated.
- Translate informational status, progress, and result strings into Slovak in a concise neutral register.
- Translate developer-facing and low-priority internal diagnostics into Slovak in a terse, technical, impersonal register; no subsystem family overrides this rule.
- Preserve identifiers, function names, configuration keys, format specifiers, and all topic-4 retained tokens verbatim in every string family, independent of translation priority.
- Render the `BUG:` diagnostic prefix as "DEFEKT:", held distinct from the "CHYBA:" prefix that carries ordinary failures; the two severities never share a word.
- Keep an identifier-plus-value diagnostic pair contiguous (eg `sweep_state=%d`); a qualifier precedes the whole pair and never stands between the identifier and its `=`.
