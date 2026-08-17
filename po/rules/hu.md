# hu translation rules

## 1. Script and orthography

Use modern standard Hungarian for Hungary, written in the Latin script, for professional electrical and radio-frequency engineers, antenna designers, and amateur-radio operators; the script distinguishes case and runs left-to-right.

- Required diacritics beyond base Latin: á é í ó ú (acute), ö ü (umlaut), ő ű (double acute, Hungarian-only); never substitute ő/ű with ö/ü or drop the marks.
- Precomposed form only; no combining-diacritic sequences.
- Apostrophe/quote code points: use the dedicated quotation marks in topic 3, never the ASCII apostrophe as a stylistic quote.
- No joining/shaping mechanics; not applicable (Latin script, no positional letter forms).
- Directionality: left-to-right throughout; not applicable otherwise.
- Case distinction applies; casing rules follow in topic 6.
- Orthographic standard: modern standard Hungarian orthography (no reform-era variant in use).
- Inter-word spacing: single space; embedded technical tokens (units, mnemonics) take the same single space as native words, never a non-breaking or zero-width join.
- Compound formation: fused compounding is the default (huzalszegmens, huzalsugár, sugárzási diagram as two words only when the second element is a genuine separate noun); hyphenate only at a vowel-collision or clarity boundary; never space-join a native compound.

## 2. Numerals in literals

- Use Arabic digits for every technical value physically present in Hungarian prose; no separate native digit set applies.
- Use a comma as the decimal separator and a space as the thousands separator in literal prose values, eg `1 000,5`.
- Keep formulas, examples, fixed defaults, named mathematical constants, standards constants, and S-parameter indices in their exact source form so their technical identity remains stable.
- Form ordinals and indices with a literal digit plus period, eg `2. szegmens`; treat the period as an ordinal marker, never as a decimal separator.

## 3. Punctuation and quotation

- Native quotation marks: „lower-upper” for in-prose quoting; embedded technical tokens (mnemonics, file paths) keep straight/source quotes when quoted at all, for consistency with format-specifier integrity.
- Native punctuation replaces source counterparts in prose: comma, question mark, exclamation mark, opening quotation mark all take Hungarian form; no ASCII smart-quote substitutes.
- Colon/semicolon spacing: no space before, one space after, matching standard Hungarian typesetting.
- Ellipsis: single "…" character, not three periods; dash preservation: en dash "–" for ranges, hyphen "-" for compounds, never conflated.
- Sentence terminator: period for full sentences and dialog prose; short labels, menu items, and field names omit the terminator.
- Punctuation inside embedded technical runs (format specifiers, file paths, mnemonics) stays in source form, never localized.

## 4. Never-translate tokens

- NEC2 card mnemonics (GW GA GH EX LD FR RP GE EN and the extended set SP SM SC NE NH NT TL GN GD GC GX GR GS GM GF CP CM SY XQ EK KH PQ PT): kept verbatim, uppercase, unchanged.
- Unit symbols: `Hz`, `kHz`, `MHz`, `GHz`, `dB`, `dBi`, `Ω`, `W`, `K`, `S/m`, `deg`, and `%` stay verbatim as standard engineering notation.
- Figure-of-merit and parameter tokens: `VSWR`, every `S`-parameter token, `Z`, `Z0`, `F/B`, and `G/T` stay verbatim as radio-frequency notation.
- File extensions `.nec`, `.inp`, `.out`, `.csv`, `.png`, and `.gplot` stay verbatim as filename components.
- Every printf-style format specifier stays verbatim; topic 5 governs its integrity.
- Embedded identifier/function/variable/config-key names inside developer strings kept verbatim; they name code, not prose.
- Physical/mathematical symbol letters (E, H field letters, angle/exponent letters) kept in source form; no native look-alike substitute.
- Product, library, and named-algorithm/curve terms (xnec2c, the numerics/graphics libraries, Reinhard, Sigmoid, μ-law, Smith chart) kept verbatim; the descriptive "Power" curve family name is the sole exception and translates as hatvány (topic 10).
- Conditional loanwords, decided file-wide: segment → szegmens (translated), patch (noun/label) → kept as "Patch", tag → címke (translated), fork (process) → kept verbatim "fork".
- All of the above follow topic 1's left-to-right rule; no directional override applies.

## 5. Format-specifier integrity

- Every source format specifier (printf-style %s/%d/%.2f etc.) is preserved, same set, none added or dropped.
- Default ordering matches source order; positional reordering (%1$s style) is permitted only where Hungarian word order otherwise breaks the sentence.
- Prefer restructuring the sentence around fixed specifier positions over reordering specifiers.
- Numbers inside format specifiers are never localized; they render in the runtime's own form, not with comma decimal or space grouping.

## 6. Capitalization and title-case

- Default casing: sentence case for labels, menu items, dialog titles, and running prose; never title-case every word.
- Axis letters (X, Y, Z) keep source capital form; no lowercase override.
- Exemption: a lowercase math/coordinate variable (eg "n", "phi") stays lowercase even at string start if the source keeps it lowercase.
- No coordinated-option-name exception applies; Hungarian does not capitalize each word of a multi-word option.
- Generic card/record noun ("card") is lowercase mid-sentence, sentence-case at string start; acronyms (NEC2, VSWR) and proper nouns retain their fixed capitalization regardless of position.

## 7. Interface register by string type

Treat each interface family as fixed Hungarian grammar; preserve all source meaning in the shortest complete natural form.

- Commands, buttons, and menu actions use a sentence-case deverbal noun with no subject or object unless the object distinguishes the action, eg `Mentés`; place modifiers before the action noun.
- Field labels use a concise noun phrase in modifier-head order with the source colon retained, eg `Frekvencia:`; use no verb or subject.
- Dialog instructions use full sentences with an omitted formal subject, an explicit object before the polite imperative verb, and the requested information before supporting detail.
- Confirmations use the topic-8 question pattern with the affected entity before the action; retain full sentence punctuation.
- Tooltips use a complete declarative explanation in cause-then-consequence order; when the source explains an unavailable control, state that reason rather than only its state.
- Status and error messages use impersonal declarative or passive constructions; state the affected entity before its state or failure and omit first-person narration.
- Name user-visible domain entities in modifier-head order; keep proper names and personal names under topic 8.
- Prefer established fused compounds to longer phrases; never omit meaning, invent an abbreviation, or imitate source length.
- Topic 24 alone governs developer and debug strings.

## 8. Formality and address

- Hungarian distinguishes formal/informal address; the interface uses formal register throughout (3rd-person-singular/plural verb conjugation implying omitted "Ön/Önök"); informal "te" forms are forbidden.
- Register carried by pro-drop and verb-ending morphology, not by inserted pronouns; never write "Ön" or "Önök" explicitly in interface strings.
- Impersonal/subject-drop convention: default for status and instruction text; the conjugated verb ending alone signals formality and person.
- Gender marking is not applicable because Hungarian verbs and third-person pronouns carry no grammatical gender; use role and function nouns that include every person naturally.
- Required number and person remain explicit through verb conjugation where context demands them; do not introduce exclusion through gendered borrowed role names.
- Honorifics are not used in interface text; grammatical address uses third person with the formal subject omitted.
- Personal names retain the Hungarian family-name-first order when the interface presents a Hungarian name; retain an externally fixed name's supplied order.
- Morphological realization: formality lives in the verb's conjugational suffix (eg -ja/-je, -zon/-zen imperative-polite forms), not in a separate word.
- Register split by string type: commands/instructions use polite imperative; dialogs/confirmations use full formal-register sentences; status/error messages use impersonal declarative, still formal by default conjugation.
- Confirmation-dialog pattern: "Biztosan … szeretne …?"
- Prohibited registers: casual, slang, over-formal/archaic, commercial, and the informal "te" conjugation.

## 9. Accelerator/hotkey mnemonics

- Mnemonic marker: underscore before the accelerator letter (eg "_Mentés"), placed at the first letter unless that letter collides within its container.
- The mnemonic letter is drawn from the translated Hungarian term itself, never transliterated from the English source letter.
- Non-Latin-script presentation (appended parenthetical Latin letter): not applicable; Hungarian is already Latin-script.
- Source-presence-only rule: never add a mnemonic to a string whose source carries none.
- Typability: avoid diacritic letters (á, ő, ű, etc.) as the mnemonic letter where an unaccented alternative exists in the same word, since accelerator input expects the base key.

## 10. Domain lexicon

| Concept | Term | Sense | Purpose/hazard |
|---|---|---|---|
| current | áram | electrical current | not "jelenlegi" (temporal) |
| charge | töltés | electrical charge | not billing/fee |
| voltage | feszültség | electric potential | |
| power (electrical) | teljesítmény | radiated/dissipated watts, power gain | distinct from Power curve family and math power-law |
| Power (curve family) | hatvány | scale-family transfer curve | the translating exception; distinct from teljesítmény |
| impedance | impedancia | complex Z | distinct from resistance/reactance |
| resistance | ellenállás | real part of Z | distinct from impedancia, terhelés |
| reactance | reaktancia | imaginary part of Z | |
| inductance | induktivitás | | |
| capacitance | kapacitás | | |
| conductivity | vezetőképesség | material S/m | |
| admittance | admittancia | admittance-matrix sense | distinct from impedancia |
| load | terhelés | LD-card impedance load | not physical weight; distinct from töltés |
| gain | nyereség | antenna directivity ratio (dB) | not "erősítés" (amplifier gain), not profit |
| excitation | gerjesztés | EM energy input | not emotional excitement |
| feedpoint | betáplálási pont | antenna feed point | one fixed engineering term |
| port | port | excitation/S-parameter port | kept, established loanword |
| radials | radiálisok | ground-plane radial wires (noun) | distinct from adjective radiális |
| ground / ground plane | föld / földsík | RF reference plane, GN/GD cards | not talaj (soil); one term across sub-senses |
| earth (physical medium) | talaj | terrain/noise-model earth | distinct from föld |
| ground wave | földhullám | propagation term | distinct from föld |
| wire | huzal | thin conductor / GW element | not "vezeték"; one term across compounds |
| segment | szegmens | NEC2 geometry subdivision | |
| patch (noun/label) | Patch | NEC2 surface patch (SP/SM) | kept per existing catalog usage |
| tag | címke | NEC2 geometry identifier | not a UI label/card |
| card | kártya | NEC2 input record | register in topic 18 |
| kernel | magfüggvény | thin-wire integral-equation kernel | not OS kernel |
| cliff | szikla | two-medium ground-boundary type | not fracture/break |
| structure | szerkezet | antenna model geometry | not "construction" |
| model | modell | NEC model or noise-temperature model | |
| geometry | geometria | model geometry | |
| crossed | keresztezett | transmission-line conductors crossed | not cut/severed |
| field (EM) | mező | near/total/E/H field | accepted homonym with data field "mező"; context disambiguates |
| near field / far field | közeltér / távoltér | opposed spatial regions | keep symmetric |
| far-field contribution | távoltér-hozzájárulás | per-direction contribution | not near-field animation |
| radiation | sugárzás | radiated emission | |
| radiation pattern | sugárzási diagram | plotted directional response | not template/design |
| gain pattern | nyereségdiagram | gain radiation pattern | |
| polarization | polarizáció | antenna/wave field orientation | distinct from polaritás |
| polarity | polaritás | sign (+/-) of a quantity | false friend of polarizáció |
| phase | fázis | | |
| reference phase | referenciafázis | | |
| frequency | frekvencia | | |
| wave / wavelength | hullám / hullámhossz | | |
| standing wave / traveling wave | állóhullám / haladóhullám | opposed pair | |
| node / antinode | csomópont / anticsomópont | standing-wave zero/maximum, also null/peak overlay | |
| crest | hullámcsúcs | instantaneous wave apex | distinct from üstökösfej (comet) and csúcsérték (curve peak) |
| magnitude | magnitúdó | modulus of a quantity | distinct from amplitúdó |
| amplitude | amplitúdó | oscillating-quantity peak | distinct from magnitúdó |
| peak value / peak magnitude | csúcsérték / csúcsmagnitúdó | two distinct UI options | never collapse either into amplitúdó or into one shared label |
| instantaneous | azonnali | projection mode | add "(φ=0)" only where source carries it |
| Poynting vector | Poynting-vektor | | kept proper name |
| solid angle | térszög | | |
| net gain | eredő nyereség | total-minus-mismatch gain | not "valós rész nyereség" |
| viewer | néző | observation direction / 3D view widget | not megfigyelő/előadó/előnézet |
| flow / flow direction | áramlás / áramlás iránya | patch/current flow | |
| total field | teljes mező | | |
| color | szín | | |
| color projection | színvetület | quantity driving hue | |
| hue | árnyalat | color-wheel angle | |
| brightness | fényerő | luminance channel | |
| hue encoding / brightness encoding | árnyalatkódolás / fényerőkódolás | internal enums | distinct from színvetület |
| color scale | színskála | magnitude-to-color scale | |
| scale family / color tone | skálacsalád | transfer-curve family (Power/Log/Asinh/μ-law/Reinhard/Sigmoid/Identity) | one concept, two source spellings, one term; not "színcsalád" |
| palette / palette kind | paletta / palettatípus | palette-layout enum | distinct from skálacsalád, színvetület |
| ramp / gradient | színátmenet | palette kind / linear color strip | |
| gamma | gamma | power-law exponent | kept, math symbol |
| knee | térdpont | soft-knee bend point | |
| softening | lágyítás | dynamic-range compression term | |
| compression | kompresszió | | |
| contrast | kontraszt | | |
| dynamic range | dinamikatartomány | | |
| floor | alsó határ | minimum/lower clamp (brightness/dB floor) | not a room floor |
| envelope | burkológörbe | magnitude/amplitude envelope | |
| comet | üstökös | moving-crest overlay effect | not geometry; see topic 20 |
| overlay (noun) | rárajzolás | added visual layer | distinct from the verb "to overlay" |
| animate / animation | animál / animáció | | |
| animated / static | animált / statikus | category-header adjectives | |
| projection | vetület | color or geometry projection | |
| scale | skála (noun) / méretez (verb) | | |
| wireframe | drótváz | wire-mesh render mode | |
| identity (transfer curve) | azonosság | no-op/passthrough transfer | distinct from unitás (Smith-chart unity) |
| sentinel | őrérték | unreachable-case guard value | |
| bins | rekeszek | discretization buckets | |
| companding | kompandálás | bounded log curve (μ-law) | kept loanword |
| tone mapping | tónusleképezés | photographic tone-map | |
| renderer | megjelenítő | drawing backend | not "render engine" |
| shader | shader | | kept, graphics loanword |
| allocation (memory) / managed allocator | foglalás / felügyelt foglaló | allocation and allocator/report | |
| thread | szál | compute thread | no collision with huzal (wire) |
| widget | widget | UI element | kept, established GTK term |
| validation | validáció | validation-tree feature | distinct from ellenőrzés (verification checks) |
| batch mode | kötegelt mód | | |
| fork (process) | fork | process fork | kept verbatim |
| deadlock | patthelyzet | | |
| notifier | értesítő | | |
| token / operand / operator / arity | token / operandus / operátor / aritás | expression-parser terms | |
| override | felülbírál | supersede a value (SY symbol) | not felülír (overwrite) |
| swap | felcserél | exchange | one fixed verb, distinct from overwrite |
| theme | színtéma | UI/color theme | always compounded with szín-; bare "téma" reserved for topic/subject |
| noise / noise temperature | zaj / zajhőmérséklet | electronic/thermal noise | not acoustic racket |
| efficiency | hatásfok | | |
| interpolation | interpoláció | | |
| mnemonic | mnemonik | card's code descriptor | not memo/note |
| degrees / deg | fok / (deg) | freestanding prose vs unit tag | tag kept like other unit tags |
| diameter | átmérő | canonical native term | not the loanword "diaméter" |
| reflect | tükröz / követ / visszaver | geometry mirror / behavioral tracking / physics reflection | three distinct senses, never merged |
| default(s) | alapértelmezett érték | fallback value | one fixed noun phrase |
| normalize / normalization | normalizál / normalizálás | | translated, not transliterated |
| ellenőrzés (verification) | ellenőrzés | non-tree correctness checks | distinct from validáció |

## 11. Disambiguation policy

- Choose the correct technical sense for every ambiguous term per the topic-10 table; program context already disambiguates in most placements.
- Never add a qualifier absent from the source string; add one only where the Hungarian term would otherwise be genuinely ambiguous (eg magnitúdó needing "Z-" prefix for impedance context).
- Accepted intra-domain homonym: "mező" serves both EM field and generic data field; no separate qualifier is added since UI context disambiguates.
- No locative homonym collision is recorded for this file.
- Gerund vs noun senses: "méretezés" (the scaling/sizing act, noun) is distinct from "méretez" (to scale, verb); never interchange the two forms.

## 12. Cross-catalog consistency

- One term per concept, reused from the topic-10 table; no synonym introduced for an already-mapped concept.
- False-friend pairs (Appendix C) resolved: polaritás vs polarizáció; magnitúdó vs amplitúdó; csúcsérték vs csúcsmagnitúdó; föld vs talaj; terhelés vs töltés; nyereség vs erősítés/haszon; áram vs `jelenlegi`; töltés vs számlázás; huzal vs kábel and szál; sugárzási diagram vs sablon/terv and távoltér; gerjesztés vs izgalom; csomópont/anticsomópont vs generikus nulla; skálacsalád vs árnyalat vs palettatípus vs színvetület; üstökös vs geometria; azonosság vs unitás; megjelenítő vs renderelőmotor; felülbírál vs felülír; néző vs megfigyelő/előadó/előnézet; tükröz vs követ vs visszaver; szerkezet vs építés; színtéma vs téma; validáció vs ellenőrzés; eredő nyereség vs valósrész-nyereség; teljesítmény vs hatvány. Each side retains a distinct translation.
- Loanword-vs-native decisions locked: port, widget, shader, fork, kompandálás, gamma kept as loanwords; átmérő, normalizálás kept native over the loanword alternative.
- Minority-outlier spellings (eg "diaméter", "vezeték" for wire) unify to the canonical form on sight.
- Consistency priority can outrank locale-form preference: "Patch" stays capitalized and untranslated across every string even where a lowercase native gloss would otherwise be idiomatic.

## 13. Priority ordering

- Precedence chain: correct meaning first, then interface-register convention (topic 7), then catalog consistency (topic 12), then disambiguation (topic 11), then locale numeral form (topic 2).
- Explicit override: "Patch" retains English capitalization even where sentence-case (topic 6) would otherwise lowercase it, since catalog consistency here outranks the general casing rule.

## 14. Grammatical number

- Hungarian translated literals distinguish singular and plural nouns, adjectives, participles, and finite verbs where grammar requires agreement.
- After an explicit literal count, keep the counted noun singular, eg `3 szegmens`; agreement elsewhere follows the grammatical subject rather than the numeric plural category.
- For count-dependent alternatives, use singular at `n=1` and plural otherwise; this choice governs wording, not catalog metadata.

## 15. Grammatical agreement

- Hungarian has no grammatical gender; gender/number concord of adjectives with nouns is not applicable.
- Adjectives are invariant when attributive (no number agreement); a standalone predicate adjective takes plural marking only when its noun is explicitly plural.
- Declension: technical nouns take standard case suffixes (accusative -t, superessive -on/-en/-ön, etc.) selected by the noun's own vowel harmony class, never a fixed suffix copied across nouns.
- Partitive-after-count: not applicable; Hungarian nouns stay singular after a numeral (eg "3 szegmens", not "szegmensek").
- No reflexive-particle retention applies to this domain's technical vocabulary.
- A standalone label with an implied head noun (eg "Radiális" for "radiális huzal") carries no separate gender marking, since Hungarian has none.

## 16. Morphological derivation

- Borrowed technical verbs/nouns take the standard -ál/-áz verb suffix (eg normalizál, animál) and the -ás/-és deverbal-noun suffix (eg mentés, animáció uses the -ió loanword-noun form); ad hoc alternate suffixes are forbidden.
- Verbal-noun formation for UI actions defaults to -ás/-és (Mentés, Törlés, Visszaállítás), per topic 7.
- Native-affix formation is preferred for established domain terms (vezetőképesség, hatásfok); loanword nouns (widget, port, shader) keep their source form with only Hungarian case suffixes attached, never a derived Hungarian root.
- Compounding (topic 1) is the default strategy for multi-concept technical nouns (huzalszegmens, sugárirányú) over a multiword phrase.

## 17. Preposition and sandhi selection

- Suffix vowel selection follows Hungarian vowel harmony: back-vowel nouns take back-vowel suffix allomorphs, front-vowel (unrounded/rounded) nouns take the matching front allomorph (eg "-ban/-ben", "-nak/-nek" chosen by the noun's own vowel class); the trigger is the noun's last vowel, not the preceding word.
- Elision/contraction: no obligatory elision rule beyond standard vowel-harmony suffix selection; none is invented for this domain.

## 18. Card/record-label register

- Dialog and editor titles use the fixed designator `MN kártya`, eg `GW kártya szerkesztése`; this form keeps the mnemonic visually distinct.
- Running prose uses the suffixed form `MN-kártya`, eg `a GW-kártya`; the hyphen binds the retained mnemonic to the inflected Hungarian noun.
- Keep `kártya` lowercase except at sentence start; no separate short generic noun applies.
- Keep title and running-prose forms internally consistent; never cross-convert their spacing and hyphenation.

## 19. Multi-paragraph and whitespace fidelity

- Mirror source paragraph breaks at the same positions; preserve the distinction between blank-line and single-line breaks.
- Preserve semantic line breaks; introduce no visual wrapping absent from the source literal.
- Drop every trailing clause removed from the current source rather than retaining stale translated text.
- Carry the source's trailing newline and terminal punctuation mechanically.
- Preserve all meaning; never truncate text or use an unnatural abbreviation to fit an assumed display limit.

## 20. Current-source fidelity

- Derive every translation from the complete current source text and supplied context.
- Reuse an inherited, copied, or fuzzy prior translation only when its complete meaning agrees with the current source and context.
- Treat `üstökös` as unsafe inheritance in a `szerkezet` or geometry context; select the term from the current referent.

## 21. Script hygiene

- Require zero Hungarian prose occurrences where `ő` or `ű` is stripped or replaced by `ö`, `ü`, `ó`, `ú`, `o`, or `u`; retained tokens are exempt only when their literal spelling requires those characters.
- Translate plain foreign words; keep only genuine identifiers, units, and topic-4 retained tokens in source form.
- Confusable-script rules are not applicable because Hungarian prose shares no second script; retained tokens remain governed by topic 4.

## 22. Rule-file scope hygiene

- Retain only current decisions that can alter a Hungarian translated literal's wording, Unicode characters, punctuation, capitalization, embedded tokens, mnemonic marker, or semantic whitespace.
- Omit catalog representation, serialization, headers, flags, validation workflow, rendering, fonts, shaping, layout, widget behavior, runtime formatting, sorting, search, display sizing, audit history, completion state, review metadata, provenance, and source citations because they do not govern translated literals.
- State each decision directly; include no implementation procedure, bibliography, correction history, or competing policy source.

## 23. Section-disjointness declaration

- Script mechanics govern characters, orthography, punctuation, casing, mnemonic marks, and script hygiene in topics 1-6, 9, and 21.
- Interface phrasing and structure govern wording, grammar, terminology, agreement, and semantic whitespace in topics 7 and 10-20.
- Address register governs formality, person, honorifics, and inclusive reference in topic 8.
- Keep these axes non-overlapping: place each decision on exactly one axis; topics 22 and 24 govern scope and string-family policy rather than any of the three linguistic axes.

## 24. Developer/debug-string policy

- Classify user-facing commands, labels, dialogs, tooltips, status messages, and errors as user-facing; translate every applicable string into Hungarian in the topic-7 register.
- Classify explanatory notices and noninteractive progress text as informational; translate them into concise, impersonal Hungarian.
- Classify internal diagnostics, sentinel messages, parser details, and subsystem traces as developer-facing; translate them into terse technical Hungarian.
- Review priority changes review order only; it never permits an applicable user-facing, informational, or developer-facing string to remain untranslated.
- Keep identifiers, function names, config keys, format specifiers, and topic-4 retained tokens verbatim in every family; token preservation is independent of linguistic priority.
- Apply no subsystem-specific sibling-family override; use the family registers above throughout this file.
- Render the `BUG:` diagnostic prefix as "PROGRAMHIBA:", held distinct from the "HIBA:" prefix that carries ordinary failures; the two severities never share a word.
- Keep an identifier-plus-value diagnostic pair contiguous (eg `sweep_state=%d`); a qualifier precedes the whole pair and never stands between the identifier and its `=`.
