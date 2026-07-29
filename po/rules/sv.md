# sv translation rules

This file governs modern standard Swedish for Sweden, written in Latin script for professional electrical and radio-frequency engineers, antenna designers, and amateur-radio operators using electromagnetic simulation software.

## 1. Script and orthography

- Write left-to-right in the Latin script; bidirectional handling is not applicable.
- Preserve `å`, `ä`, and `ö` as mandatory distinct letters; never strip their diacritics or fold them to `a` or `o`, to preserve Swedish spelling and meaning.
- Use precomposed Unicode for `å`, `ä`, and `ö`, never decomposed base-letter-plus-mark forms, to keep literal spelling consistent.
- Use U+0027 APOSTROPHE only where an apostrophe belongs; Swedish requires no modifier-letter apostrophe, textual joiner, or other ambiguous mark.
- Script-specific joiners and positional forms are not applicable to Swedish Latin script.
- Apply modern post-1906 Swedish orthography for Sweden; no regional variant is admitted.
- Preserve letter case and apply topic 6 to all cased text.
- Put one space between words and between Swedish text and embedded numeric or retained tokens.
- Form established compounds as closed words, eg `strömfördelning`; use a hyphen only to prevent misreading or to join a retained mnemonic to a Swedish noun, eg `GW-kort`.

## 2. Numerals in literals

- Use digits `0-9` for technical values written in translated prose.
- Use decimal comma and space grouping in translated prose, eg `1 000,5`, to follow Swedish technical notation.
- Preserve formulas, examples, fixed defaults, and named mathematical or standards constants exactly where their literal form is part of the source, including their digits and separators.
- Form ordinals with a digit, colon, and suffix, eg `1:a`; keep this index colon distinct from the decimal comma.

## 3. Punctuation and quotation

- Use Swedish double quotation marks `”…”` for quoted prose; retain source-style punctuation inside identifiers, filenames, mnemonics, units, and format tokens to preserve those tokens.
- Use comma, question mark, and exclamation mark in their shared forms; opening punctuation variants are not applicable.
- Put no space before a colon, semicolon, or terminal mark and one space after it.
- Preserve the source ellipsis form as `…` or `...` and preserve an en dash as `–`, so punctuation carries the source distinction.
- End full sentences with their required terminal mark; omit terminal punctuation from short labels, menu items, and field labels.
- Keep punctuation within a retained technical run unchanged.

## 4. Never-translate tokens

Treat every listed token as a zero-failure boundary: retain its exact spelling, case, characters, and left-to-right order; never translate or transliterate it.

- Retain NEC2 card mnemonics `GW GA GH EX LD FR RP GE EN SP SM SC NE NH NT TL GN GD GC GX GR GS GM GF CP CM SY XQ EK KH PQ PT` as file-format identifiers.
- Retain unit symbols `Hz`, `kHz`, `MHz`, `GHz`, `dB`, `dBi`, `Ω`, `V`, `A`, `W`, `K`, `S/m`, `°`, and `%` as standard engineering notation.
- Retain figure-of-merit and parameter tokens `VSWR`, `S11`, `S12`, `S21`, `S22`, `Z`, `Z0`, `F/B`, and `G/T` as radio-frequency notation.
- Retain literal file extensions and filenames, including their punctuation and case, so filenames remain exact.
- Retain every printf-style format specifier, including flags, widths, precisions, lengths, conversion letters, and positional indices; topic 5 governs their placement.
- Retain embedded function names, variable names, configuration keys, and other code identifiers because they name program elements.
- Retain physical and mathematical symbols `E`, `H`, `θ`, `φ`, `γ`, and other source symbol letters because a look-alike changes the notation.
- Retain product, library, toolkit, and proper chart names `xnec2c`, `NEC2`, `GTK`, `Cairo`, `OpenGL`, and `Smith`.
- Retain named curve and algorithm terms `Log`, `Asinh`, `μ-law`, `Reinhard`, `Sigmoid`, and `Identity`; translate descriptive `Power` to `Effekt` to distinguish it from a proper name.
- Translate conditional loanwords according to their locked topic-10 forms: `segment`, `patch`, and `tagg`; retain process term `fork`.
- Translate ordinary foreign prose words; only the enumerated identifiers, symbols, units, proper names, and locked technical terms cross unchanged.

## 5. Format-specifier integrity

Treat format specifiers as immutable tokens whose exact source set must survive translation.

- Preserve every source specifier with the same conversion, flags, width, precision, length, and count.
- Keep source order by default; use positional forms only when the source already supplies them or Swedish grammar requires reordering.
- Restructure Swedish wording around fixed non-positional specifiers rather than moving them.
- Never localize digits or punctuation inside a specifier.

## 6. Capitalization and title-case

- Use sentence case for labels, menu items, and titles; capitalize only the first word plus proper nouns and acronyms, to follow Swedish interface convention.
- Preserve uppercase axis letters `X`, `Y`, and `Z` as coordinate identifiers.
- Preserve lowercase mathematical and coordinate variables, eg `x` and `φ`, even at the start of a label.
- Apply sentence case across coordinated option names; separate title capitalization of each element is not applicable.
- Write generic `kort` lowercase except sentence-initially; preserve proper names and acronyms such as `Smith` and `VSWR`.

## 7. Interface register by string type

Use Swedish main-clause order with the finite verb in second position where a subject is present; omit recoverable subjects in interface actions, and put known context before new detail.

- Commands, buttons, and menu actions use the shortest natural imperative, with the verb first and the object after it, eg `Spara ändringar`; use an infinitive only where the surrounding literal grammatically selects one.
- Field labels use a concise noun phrase in head-before-modifier order where idiomatic Swedish requires it and retain a source colon, eg `Frekvens:`; they contain no subject or verb.
- Dialog statements use full declarative sentences with subject-verb-object order; confirmations use a direct question with the action before optional consequences.
- Tooltips use full declarative sentences, name the control or action before its effect, and preserve a source-stated reason for unavailability.
- Status and error messages use impersonal declarative clauses; state the affected entity before the condition or failure and omit first- and second-person subjects.
- Name user-visible domain entities with the locked topic-10 noun as the head of an idiomatic closed compound or noun phrase; topic 8 governs personal names.
- Prefer closed compounds and the shortest complete natural wording; preserve every source meaning and avoid abbreviations invented to imitate source length.
- Topic 24 alone governs developer and debug strings.

## 8. Formality and address

- Use neutral modern professional Swedish; omit direct address where natural and use `du` only when the source directly addresses the user.
- Never use distancing `ni`, slang, commercial language, archaic forms, or ceremonial formality.
- Carry register through neutral word choice and impersonal construction; Swedish has no formal-versus-informal verb inflection.
- Use second person singular only for direct address; use no honorific.
- Avoid gender-marking the addressee; use natural gender and number agreement only for the referenced Swedish noun.
- Use inclusive role and person terms without invented gender marking.
- Preserve a person's written name order; Swedish interface text adds no honorific or patronymic pattern.
- Commands and dialogs share the same neutral register; commands omit the subject, while dialog statements retain a subject when grammar requires it.
- Phrase confirmations as direct action questions, eg `Spara ändringarna?`, without a formal circumlocution.

## 9. Accelerator/hotkey mnemonics

- Place `_` immediately before the mnemonic letter within the translated term, eg `_Arkiv`.
- Select the mnemonic from the translated Swedish term, not from the source term, so the visible letter and action agree.
- Separate appended mnemonic letters are not applicable because Swedish uses Latin script.
- Preserve source presence exactly: add a mnemonic only where the source literal carries one.
- Select a typable letter and prefer `a-z` over `å`, `ä`, or `ö` when both occur in the term.

## 10. Domain lexicon

Lock each source concept to the listed Swedish term; the sense and purpose column prevents false-friend substitution and preserves engineering meaning.

### Electrical primitives

| Concept | Swedish term | Sense and purpose |
|---|---|---|
| current | ström | electrical current in amperes; excludes temporal currency |
| charge | laddning | electrical charge in coulombs; excludes fee and cargo senses |
| voltage | spänning | electric potential; fixes established engineering usage |
| power (electrical) | effekt | radiated or dissipated watts and power flow; distinct from transfer-family `Potens` |
| impedance | impedans | complex `Z`; distinct from resistance and reactance |
| resistance | resistans | real part of `Z`; distinct from impedance and load |
| reactance | reaktans | imaginary part of `Z`; distinct from resistance |
| inductance | induktans | magnetic energy-storage property; fixes the electrical sense |
| capacitance | kapacitans | electric energy-storage property; fixes the electrical sense |
| conductivity | konduktivitet | material property in `S/m`; fixes established technical usage |
| admittance | admittans | matrix or reciprocal-impedance sense; distinct from impedance |
| load | belastning | `LD`-card impedance load; distinct from electrical charge and physical burden |
| gain | antennvinst | antenna directivity ratio in `dB`; distinct from amplifier `förstärkning` and profit `vinst` |
| excitation | matning | electromagnetic source input; excludes emotional excitement |
| feedpoint | matningspunkt | antenna connection point; fixes the feed sense |
| port | port | excitation or S-parameter port; fixes established engineering usage |
| radials | radialtrådar | horizontal ground-plane conductors; distinguishes the noun from adjective `radiell` |

### Ground and earth

| Concept | Swedish term | Sense and purpose |
|---|---|---|
| ground / ground plane | jord / jordplan | radio-frequency reference and its plane; excludes physical terrain |
| earth (physical medium) | mark | terrain or noise-model medium; distinct from electrical `jord` |
| ground wave | markvåg | propagation mode over earth; distinct from the electrical reference |

### Geometry primitives

| Concept | Swedish term | Sense and purpose |
|---|---|---|
| wire | trådelement | thin `GW` conductor; distinct from cable, cord, and compute thread |
| segment | segment | NEC2 geometry subdivision; locks the established loanword |
| patch | patch | NEC2 surface element; locks the untranslated geometry term |
| tag | tagg | NEC2 geometry identifier; distinct from interface label and card |
| card | kort | NEC2 input record; topic 18 fixes its register |
| kernel | kärna | integral-equation or thin-wire kernel; excludes operating-system sense |
| cliff | brant | two-medium ground boundary; excludes fracture and break senses |
| structure | struktur | antenna-model geometry; distinct from construction |
| model | modell | NEC or noise-temperature model; fixes the simulation sense |
| geometry | geometri | model geometry; distinct from overlay effects |
| crossed | korsad | reversed transmission-line conductors; excludes cut or severed |

### Field, pattern, viewer

| Concept | Swedish term | Sense and purpose |
|---|---|---|
| field (EM) | elektromagnetiskt fält | near, total, electric, or magnetic field; distinct from a data field |
| near field / far field | närfält / fjärrfält | opposed spatial regions; preserves symmetric terminology |
| far-field contribution | fjärrfältsbidrag | directional contribution; distinct from near-field animation |
| radiation | strålning | emitted electromagnetic energy; fixes the physical sense |
| radiation pattern | strålningsdiagram | plotted directional response; distinct from template and far-field region |
| gain pattern | antennvinstdiagram | radiation diagram of antenna gain; follows the locked gain term |
| polarization | polarisation | field orientation; distinct from polarity |
| polarity | polaritet | positive or negative sign; distinct from polarization |
| phase | fas | angular wave state; fixes the electromagnetic sense |
| reference phase | referensfas | phase origin; preserves the phase qualifier |
| frequency | frekvens | cycles per unit time; fixes established engineering usage |
| wave / wavelength | våg / våglängd | propagating phenomenon and its spatial period; preserves the relation |
| standing wave / traveling wave | stående våg / fortskridande våg | opposed wave types; preserves symmetric terminology |
| node / antinode | nod / buk | standing-wave minimum and maximum; distinct from a generic numeric zero |
| crest | vågtopp | instantaneous wave apex; distinct from a curve peak |
| magnitude | belopp | scalar modulus; distinct from amplitude |
| amplitude | amplitud | oscillating peak displacement; distinct from magnitude |
| peak value / peak magnitude | toppvärde / toppbelopp | distinct interface options; prevents label collapse |
| instantaneous | momentan | projection at one phase; add `(φ=0)` only when the source contains it |
| Poynting vector | Poyntingvektor | electromagnetic energy-flow vector; preserves the proper name |
| solid angle | rymdvinkel | three-dimensional angular measure; fixes the geometric sense |
| net gain | nettovinst | gain after mismatch loss; distinct from real-part gain |
| viewer | betraktarriktning | observation direction or view orientation; excludes observer, speaker, and preview |
| flow / flow direction | flöde / flödesriktning | patch or current flow; preserves the directional distinction |
| total field | totalfält | combined electromagnetic field; preserves the total qualifier |

### Color, tone, animation subsystem

| Concept | Swedish term | Sense and purpose |
|---|---|---|
| color | färg | visible color; fixes the interface sense |
| color projection | färgprojektion | quantity mapped to hue; distinct from tone family, palette kind, and hue |
| hue | färgton | color-wheel angle; distinct from nuance and transfer-family tone |
| brightness | ljusstyrka | luminance channel; distinct from hue |
| hue encoding / brightness encoding | färgtonskodning / ljusstyrkekodning | distinct encoding enums; neither means color projection |
| color scale | färgskala | magnitude-to-color mapping; distinct from generic scaling |
| scale family / color tone | skalfamilj | transfer-curve family; one term for both source spellings |
| palette / palette kind | palett / palettyp | color set and its layout enum; distinct from scale family |
| ramp / gradient | rampskala / gradient | palette kind and linear color strip; preserves the source distinction |
| gamma | gamma | power-law exponent; fixes established technical usage |
| knee | knäpunkt | soft-knee bend point; excludes body-part sense |
| softening | uppmjukning | curve smoothing parameter; fixes the dynamic-range sense |
| compression | komprimering | dynamic-range reduction; excludes file compression where context differs |
| contrast | kontrast | tonal separation; fixes the color-control sense |
| dynamic range | dynamiskt omfång | ratio between extremes; fixes the signal sense |
| floor | nedre gräns | minimum clamp; excludes room-floor sense |
| envelope | envelopp | magnitude or amplitude boundary; fixes the signal sense |
| comet | komet | moving-crest overlay; distinct from geometry |
| overlay (noun) | överlagring | added visual layer; distinct from the verb |
| animate / animation | animera / animering | action and resulting motion; preserves grammatical role |
| animated / static | animerad / statisk | dynamic and phase-invariant categories; preserves opposition |
| projection | projektion | color or geometry mapping; fixes the technical sense |
| scale | skala | verb and noun selected by grammar; fixes the scaling sense |
| wireframe | trådmodell | mesh-only view mode; distinct from antenna wire geometry |
| identity | identitet | no-op transfer; distinct from Smith-chart unity `enhet` |
| sentinel | vaktvärde | unreachable-case guard value; excludes ordinary token sense |
| bins | intervall | discretization buckets; uses one Swedish statistical term |
| companding | kompandering | bounded logarithmic compression; fixes the signal-processing sense |
| tone mapping | tonmappning | photographic range mapping; distinct from hue and scale family |
| Power | Potens | power-law transfer family; distinct from electrical `effekt` |

### Render and compute

| Concept | Swedish term | Sense and purpose |
|---|---|---|
| renderer | renderare | drawing backend; distinct from render engine `rendermotor` |
| shader | shader | graphics-program stage; locks established technical usage |
| allocation (memory) / managed allocator | minnesallokering / hanterad allokerare | memory reservation and manager; excludes general allocation |
| thread | beräkningstråd | compute thread; distinct from geometry `trådelement` |
| widget | gränssnittselement | user-interface element; fixes the interface sense |
| validation | validering | validation-tree feature; distinct from verification `verifiering` |
| batch mode | satsläge | noninteractive grouped operation; fixes the compute sense |
| fork (process) | fork | process duplication; locks the retained process term |
| deadlock | dödläge | mutual waiting state; fixes the concurrency sense |
| notifier | aviserare | event notification component; fixes the software sense |
| token / operand / operator / arity | token / operand / operator / aritet | expression-parser concepts; preserves four distinct roles |
| override | åsidosätta | supersede a value; distinct from overwrite `skriva över` |
| swap | byta plats | exchange two values; distinct from replacement |
| theme | tema | interface color theme; distinct from subject `ämne` |

### Metrics and miscellaneous

| Concept | Swedish term | Sense and purpose |
|---|---|---|
| noise / noise temperature | brus / brustemperatur | electronic noise and equivalent temperature; excludes acoustic racket |
| efficiency | verkningsgrad | useful-to-input ratio; fixes the engineering sense |
| interpolation | interpolering | values between samples; fixes the numerical sense |
| mnemonic | mnemonik | card-code descriptor; distinct from memo or note |
| degrees / deg | grader / `(deg)` | prose angle name versus retained unit tag; preserves their roles |
| diameter | diameter | circle width; locks the canonical technical term |
| reflect | spegla / följa / reflektera | geometry operation, behavioral tracking, and physical reflection; keeps all three senses distinct |
| default(s) | standardvärde(n) | fallback value or values; excludes moral or financial senses |
| normalize / normalization | normalisera / normalisering | rescale to a reference; fixes native technical derivation |

## 11. Disambiguation policy

- Select the topic-10 engineering sense for every ambiguous source concept; simulator context supplies the technical domain.
- Add no qualifier absent from the source when the locked term is unambiguous; add the shortest Swedish qualifier only when the unqualified target term would retain two applicable senses.
- Accept no intra-domain homonym among Appendix C pairs; use their distinct topic-10 terms.
- Use `jord` only for the electrical reference and `mark` only for physical terrain, preventing the locative collision.
- Resolve an overloaded action noun by grammar: use finite or infinitive `skala` for the verb and noun phrase `en skala` for the noun.

## 12. Cross-catalog consistency

- Use one locked topic-10 term per concept throughout translated literals; introduce no competing synonym.
- Keep every required distinction: `polaritet`/`polarisation`; `belopp`/`amplitud`; `toppvärde`/`toppbelopp`; `jord`/`mark`; `belastning`/`laddning`; `antennvinst`/`förstärkning`/`vinst`; `ström`/`aktuell`; `laddning`/`avgift`; `trådelement`/`kabel`/`sladd`/`beräkningstråd`; `strålningsdiagram`/`mönster`/`fjärrfält`; `matning`/`upphetsning`; `nod`/`buk`/`nollställe`; `skalfamilj`/`färgton`/`palettyp`/`färgprojektion`; `komet`/`geometri`; `identitet`/`enhet`; `renderare`/`rendermotor`; `åsidosätta`/`skriva över`; `betraktarriktning`/`observatör`/`högtalare`/`förhandsvisning`; `spegla`/`följa`/`reflektera`; `struktur`/`konstruktion`; `tema`/`ämne`; `validering`/`verifiering`; `nettovinst`/`realdelsvinst`; `effekt`/`Potens`.
- Apply topic 4 and topic 10 as the single loanword-versus-native decision and canonical spelling authority.
- Replace minority spellings with the locked form, preserving one term per concept.
- Let catalog consistency outrank a plausible local synonym only for locked cases such as untranslated `patch`; correct meaning remains the higher priority.

## 13. Priority ordering

- Apply this precedence: correct meaning, Swedish interface convention, cross-catalog consistency, disambiguation, then literal numeral form.
- Preserve untranslated `patch` by catalog consistency over a native synonym; use Swedish interface word order over source order where meaning remains intact.

## 14. Grammatical number

- Use singular and plural; select singular only for an explicit count of `1` and plural for every other explicit count.
- Inflect countable nouns for number and make attributive adjectives and participles agree with them; Swedish finite verbs do not vary by number.
- Use the indefinite singular after `1` and the appropriate indefinite plural after other explicit counts unless definiteness is required by the sentence.

## 15. Grammatical agreement

- Make adjectives and participles agree with the head noun in common or neuter gender and in singular or plural, including labels with an implied head.
- Resolve a standalone label from its known referent; use common gender only when no referent fixes a neuter noun, eg `jordplan`.
- Partitive case after a count is not applicable because Swedish has no productive partitive case.
- Retain `sig` only where the selected Swedish verb construction is reflexive; never copy reflexivity from source syntax alone.

## 16. Morphological derivation

- Derive established borrowed technical verbs with `-era`, eg `normalisera`; reject ad hoc competing verb suffixes.
- Form verbal nouns with established `-ing` or `-ering` morphology, eg `normalisering`, and use the locked agent noun where topic 10 supplies one.
- Prefer established Swedish derivation and closed compounding over an unestablished multiword loan phrase; retain only topic-4 tokens unchanged.

## 17. Preposition and sandhi selection

Swedish has no productive sound-conditioned preposition alternation, mandatory sandhi, or elision in these literals; no form selection applies.

## 18. Card/record-label register

- Use `GW-kort` as the fixed mnemonic-plus-designator form in editor and dialog titles.
- Use the same hyphenated mnemonic compound in running prose because Swedish has no distinct suffixed record register.
- Write generic `kort` lowercase except sentence-initially; use no separate long or short designator.
- Keep this form within both registers and substitute no label or record synonym.

## 19. Multi-paragraph and whitespace fidelity

- Mirror each source paragraph break at the same semantic position: preserve blank-line `\n\n` breaks and single semantic line breaks distinctly.
- Remove any translated trailing clause absent from the current source.
- Preserve a source trailing newline and terminal punctuation in the translated literal.
- Preserve semantic line breaks; introduce no visual wrapping absent from the source.
- Preserve complete natural meaning; never truncate or abbreviate for an assumed display limit.

## 20. Current-source fidelity

- Derive every translation from the current source literal and its supplied context.
- Reuse inherited wording only when its complete meaning matches the current source and context.
- Do not inherit wording that confuses polarity with polarization, ground with earth, comet with geometry, or a removed trailing clause with current content.

## 21. Script hygiene

- Permit Latin letters outside Swedish prose only inside a retained topic-4 token or established topic-10 loanword; translate ordinary foreign words.
- Treat an unintended non-Swedish letter, decomposed `å`/`ä`/`ö`, or foreign homoglyph inside Swedish prose as a zero-failure violation.
- Preserve a foreign character only where the exact spelling of an identifier, symbol, unit, proper name, or locked retained token requires it.

## 22. Rule-file scope hygiene

- Keep only current decisions that can alter wording, Unicode characters, punctuation, capitalization, embedded tokens, mnemonic markers, or semantic whitespace in a translated literal.
- Omit catalog representation, serialization, headers, flags, workflow, rendering, fonts, shaping, layout, control behavior, runtime formatting, sorting, search, display sizing, audit history, completion state, review metadata, provenance, and citations.
- State the governing translation decision directly; include no procedure, bibliography, correction history, or alternative policy source.

## 23. Section-disjointness declaration

- Topics 1-3 govern script, numerals, punctuation, and literal mechanics only.
- Topics 6-9 and 18 govern interface phrasing, syntax, address, and mnemonic presentation only.
- Topics 10-13 govern lexical meaning, distinctions, consistency, and precedence only.
- Topics 14-17 govern inflection, agreement, derivation, and context-conditioned forms only.
- Topic 24 governs developer and debug string families only; each decision belongs to one axis and is not duplicated across sections.

## 24. Developer/debug-string policy

- Translate user-facing actions, labels, dialogs, tooltips, status messages, and errors in full; their review priority never permits an untranslated applicable string.
- Translate informational diagnostics that describe program state in terse technical Swedish.
- Translate developer-facing `BUG()` and assertion-style diagnostics in terse technical Swedish; no subsystem family overrides this policy.
- Preserve every identifier, function name, configuration key, physical symbol, retained token, and format specifier inside all three families exactly as topics 4 and 5 require.
- Keep linguistic priority separate from token preservation: every translated family follows the same immutable-token boundaries.
