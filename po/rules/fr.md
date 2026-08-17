# fr translation rules

## 1. Script and orthography

- Govern French for France, written in Latin script under standard modern French orthography, for professional electrical and RF engineers, antenna designers, and amateur-radio operators.
- Preserve required letters and diacritics `à â ç é è ê ë î ï ô ù û ü ÿ œ`; strip none and substitute no unaccented look-alike.
- Use precomposed Unicode characters; introduce no combining sequence where a precomposed French character exists.
- Use ASCII apostrophe `'` as the fixed catalog mark; substitute no typographic look-alike.
- Textual joiners and script-specific code-point distinctions are not applicable to French Latin script.
- Write left to right; retain embedded technical tokens in that direction without mirroring or manual direction controls.
- French has letter case; apply topic 6 to all cased text.
- Separate words with one ASCII space, including boundaries between French text and embedded numeric or retained tokens.
- Form compounds as established fused, hyphenated, or spaced French terms; prefer `de`-linked noun phrases for new technical compounds and invent no fused form.

## 2. Numerals in literals

- Use European digits `0`-`9` for technical values written in French prose.
- Use comma as the decimal separator and a plain space as the thousands-grouping separator in literal prose numbers.
- Preserve formulas, worked examples, fixed defaults, named mathematical constants, standards constants, and their punctuation exactly in source form.
- Preserve literal indices as source technical notation; when a French ordinal suffix is required by the source sense, attach it to the digit and keep any index separator distinct from the decimal comma.

## 3. Punctuation and quotation

- Use plain source quotes for quoted interface labels and retained technical tokens; use French guillemets `« »` only for ordinary quoted prose, with one plain space inside each mark.
- Use French comma, question mark, exclamation mark, colon, and semicolon in prose; French has no distinct opening question or exclamation mark.
- Put one plain space before `:`, `;`, `?`, and `!`; put no space before comma or period and one space after prose punctuation when text follows.
- Preserve source `...` as the ellipsis and preserve source dash characters.
- End full sentences with a period, question mark, or exclamation mark as required; omit a terminator from short labels and fragments.
- Keep punctuation within formulas, format specifiers, identifiers, mnemonics, filenames, and other retained technical runs in source form.

## 4. Never-translate tokens

Retain every token in this section character for character; translate, transliterate, case-fold, or respell none.

- Keep NEC2 card mnemonics `GW GA GH EX LD FR RP GE EN SP SM SC NE NH NT TL GN GD GC GX GR GS GM GF CP CM SY XQ EK KH PQ PT` as round-trippable file identifiers.
- Keep SI and electrical-engineering unit symbols in source form, including frequency, gain, impedance, power ratio, temperature, conductivity, angle, and percent symbols; retain `MHz`, `dBi`, `Ω`, `dB`, `deg`, and `%`.
- Keep RF figure-of-merit and parameter tokens in source form, including `VSWR`, S-parameter tokens, `Z`, `Z0`, front/back-ratio tokens, and gain/temperature tokens.
- Keep every source file extension and literal filename suffix unchanged.
- Keep every printf-style format specifier unchanged under topic 5.
- Keep embedded identifiers, function names, variable names, and configuration keys unchanged in developer strings.
- Keep physical and mathematical symbol letters unchanged, including `E`, `H`, `φ`, and `θ`; a look-alike changes the notation.
- Keep product, library, toolkit, and chart proper names unchanged, including `xnec2c`, `NEC2`, `GTK`, Cairo, OpenGL, and Smith.
- Keep named transfer functions and algorithms unchanged, including `Reinhard`, `Asinh`, and `μ-law`; translate descriptive family names as `loi de puissance`, `logarithmique`, `sigmoïde`, and `identité`.
- Keep conditional loanwords `segment`, `patch`, `tag`, `fork`, `shader`, `widget`, and `thread` in the stated forms; translate other ordinary geometry and process words under topic 10.
- Apply topic 1 directionality to every retained token; French requires no bidirectional mirroring rule.

## 5. Format-specifier integrity

- Every specifier from the source is preserved, same set, same count.
- Default ordering matches source; positional reordering (`%1$s`) allowed only where French word order requires it.
- The sentence restructures around fixed specifier positions rather than reordering the specifiers themselves.
- Numbers inside specifiers are never localized.

## 6. Capitalization and title-case

- Sentence case for labels, menu items, and titles; never title case, even where the English source is title case (eg "Color Scale" -> "Échelle de couleur", not "Échelle De Couleur").
- Axis letters (X, Y, Z) retain source uppercase.
- Lowercase math/coordinate variables (eg x, y in formulas) stay lowercase.
- No exception for coordinated option names; each stays sentence case.
- Generic card/record noun ("Carte") capitalized only at sentence start; acronyms (NEC2, VSWR) and proper nouns retain their own case.
- Case distinction present (Latin script); the rules above apply in full, no reduction to mnemonic-only handling.

## 7. Interface register by string type

Treat each string family as interface French, not conversational French; preserve all source meaning in the shortest complete natural form.

- Commands, buttons, and menu actions use a subjectless infinitive with the verb first and its object or complement after it (eg "Réinitialiser la vue"); omit direct address, finite indicatives, and noun nominalizations.
- Field labels use a concise noun phrase in head-first French order followed by the retained colon; omit subject and verb.
- Dialogs and confirmations use full declarative or interrogative sentences in subject-verb-object order; place known context before the requested decision and use the formal confirmation pattern from topic 8.
- Tooltips use full declarative sentences in subject-verb-object order; explain function before conditions and retain the source-stated reason that an unavailable control is disabled.
- Status and error messages use impersonal or passive declarative sentences; state the affected entity before its state or failure, then the cause or remedy supplied by the source.
- User-visible domain entities use the locked topic-10 noun as the head, followed by French complements introduced by `de`, `à`, or the technically required preposition; retain proper names and personal-name order from topic 8.
- Prefer natural spaced multiword forms and French complement order; omit no meaning, invent no abbreviation, and imitate no source-length constraint.
- Developer and debug strings follow topic 24.

## 8. Formality and address

- Use a neutral professional register for electrical engineers, RF engineers, antenna designers, and amateur-radio operators in France using standard modern French.
- French distinguishes formal and informal address; use formal constructions and omit `tu`, `ton`, casual speech, slang, commercial language, archaic language, and ceremonial over-formality.
- Commands use the subjectless infinitive; dialogs use complete formal sentences; confirmations use a full question followed by explicit affirmative and negative choices.
- Use impersonal or subjectless constructions instead of an explicit addressee; where direct address is required by the source, use second-person plural morphology without an honorific.
- Avoid gendering the user; inflect adjectives and participles for the grammatical gender and number of the named or implied interface entity under topic 15.
- Use inclusive collective nouns for people; retain required grammatical agreement without introducing paired gender forms absent from the source.
- Honorifics are not applicable to this interface; add none absent from the source.
- Personal names retain given-name then family-name order and their source spelling; translate no personal name.

## 9. Accelerator/hotkey mnemonics

- Preserve `_` if and only if the source literal carries a mnemonic; place it immediately before a typable letter in the translated term and invent none elsewhere.
- Choose the mnemonic from the translated term, not from the source spelling; prefer an unaccented letter where the term offers one.
- Keep mnemonics unique within their interface container by moving `_` to another eligible letter of the same translated term.
- Separate parenthetical mnemonic letters are not applicable because French uses Latin script.

## 10. Domain lexicon

Locked term table; every Appendix A concept resolves to one French term, held consistently catalog-wide.

### Electrical primitives

| Concept | French term | Sense / hazard guarded |
|---|---|---|
| current | courant | electrical current, Amperes; never "actuel" (temporal) |
| charge | charge | electrical charge, Coulombs; not billing/fee/cargo; accepted homonym with load |
| voltage | tension | electric potential |
| power (electrical) | puissance | watts, radiated or dissipated power, gain, and power flow; distinct from the transfer family `loi de puissance` |
| impedance | impédance | complex Z; distinct from résistance and réactance |
| resistance | résistance | real part of Z |
| reactance | réactance | imaginary part of Z |
| inductance | inductance | locked technical sense and catalog consistency |
| capacitance | capacité | locked technical sense and catalog consistency |
| conductivity | conductivité | material S/m |
| admittance | admittance | distinct from impédance |
| load | impédance de charge | LD-card impedance load; distinct from electrical `charge` and physical weight |
| gain | gain | antenna directivity ratio (dB); never profit or amplifier gain |
| excitation | excitation | EM energy input; never emotional excitement |
| feedpoint | point d'alimentation | locked technical sense and catalog consistency |
| port | port | excitation/S-parameter port |
| radials | radiales | noun, ground-plane radial wires; distinct from adjective "radial" |

### Ground and earth

| Concept | French term | Sense / hazard guarded |
|---|---|---|
| ground / ground plane | masse / plan de masse | RF electrical reference; not soil; one term across all ground sub-senses |
| earth (physical medium) | sol | terrain/noise-model earth; distinct from electrical masse |
| ground wave | onde de sol | propagation term, distinct from the ground reference |

### Geometry primitives

| Concept | French term | Sense / hazard guarded |
|---|---|---|
| wire | fil | thin conductor/GW element; not câble/cordon |
| segment | segment | NEC2 subdivision |
| patch | patch | NEC2 surface patch (SP/SM); kept, not translated |
| tag | tag | NEC2 identifier; not a UI label or a carte |
| card | carte | NEC2 input record; register in topic 18 |
| kernel | noyau | integral-equation kernel; not an OS kernel |
| cliff | falaise | two-medium ground-boundary type; not a fracture |
| structure | structure | antenna model geometry; not "construction" |
| model | modèle | NEC model or noise-temperature model |
| geometry | géométrie | model geometry |
| crossed | croisé | conductors crossed/reversed; not cut/severed |

### Field, pattern, viewer

| Concept | French term | Sense / hazard guarded |
|---|---|---|
| field (EM) | champ | near/total/E/H field; distinct from a data/config field |
| near field / far field | champ proche / champ lointain | opposed pair, kept symmetric |
| far-field contribution | contribution en champ lointain | per-direction; not near-field animation |
| radiation | rayonnement | radiated emission |
| radiation pattern | diagramme de rayonnement | plotted directional response; not template/design |
| gain pattern | diagramme de gain | locked technical sense and catalog consistency |
| polarization | polarisation | antenna/wave field orientation |
| polarity | polarité | sign (+/-); false friend of polarisation |
| phase | phase | locked technical sense and catalog consistency |
| reference phase | phase de référence | locked technical sense and catalog consistency |
| frequency | fréquence | locked technical sense and catalog consistency |
| wave / wavelength | onde / longueur d'onde | locked technical sense and catalog consistency |
| standing wave / traveling wave | onde stationnaire / onde progressive | opposed pair |
| node / antinode | nœud / ventre | standing-wave zero/maximum; also the null/peak overlay sense |
| crest | crête | instantaneous wave apex, comet-head; distinct from a curve/step peak |
| magnitude | module | modulus of a quantity; distinct from amplitude |
| amplitude | amplitude | oscillating peak; distinct from module |
| peak value vs peak magnitude | valeur de crête vs module de crête | two distinct options, never collapsed to one label |
| instantaneous | instantané | projection mode; "(φ=0)" qualifier only where source carries it |
| Poynting vector | vecteur de Poynting | locked technical sense and catalog consistency |
| solid angle | angle solide | locked technical sense and catalog consistency |
| net gain | gain net | total-minus-mismatch; not "gain réel" |
| viewer | visualiseur | observation direction and 3D view entity; distinct from observateur, orateur, and aperçu |
| flow / flow direction | flux / sens du flux | patch/current flow |
| total field | champ total | locked technical sense and catalog consistency |

### Color, tone, animation subsystem

| Concept | French term | Sense / hazard guarded |
|---|---|---|
| color | couleur | locked technical sense and catalog consistency |
| color projection | projection de couleur | which quantity drives hue |
| hue | teinte | color-wheel angle |
| brightness | luminosité | luminance channel |
| hue encoding / brightness encoding | codage de teinte / codage de luminosité | distinct enums, neither collapses to projection de couleur |
| color scale | échelle de couleur | magnitude-to-color scale |
| scale family / color tone | famille d'échelle | transfer-curve family (`loi de puissance`, `logarithmique`, `Asinh`, `μ-law`, `Reinhard`, `sigmoïde`, `identité`); one concept, one term |
| palette / palette kind | palette / type de palette | distinct from famille d'échelle and projection de couleur |
| ramp / gradient | rampe / dégradé | palette kind / linear strip |
| gamma | gamma | power-law exponent |
| knee | genou | soft-knee bend point |
| softening | adoucissement | locked technical sense and catalog consistency |
| compression | compression | dynamic-range |
| contrast | contraste | locked technical sense and catalog consistency |
| dynamic range | plage dynamique | locked technical sense and catalog consistency |
| floor | plancher | minimum/lower clamp; not a room floor |
| envelope | enveloppe | magnitude/amplitude envelope |
| comet | comète | moving-crest overlay; not geometry |
| overlay (noun) | incrustation | added visual layer; distinct from the verb superposer |
| animate / animation | animer / animation | locked technical sense and catalog consistency |
| animated / static | animé / statique | category-header adjectives |
| projection | projection | color or geometry projection |
| scale | échelle (nom) / mettre à l'échelle (verbe) | noun vs verb, kept distinct |
| wireframe | fil de fer | wire-mesh render mode |
| identity | identité | no-op/passthrough transfer; distinct from unité (Smith chart) |
| sentinel | sentinelle | unreachable-case guard value |
| bins | compartiments | discretization buckets |
| companding | compression logarithmique | bounded log curve (μ-law) |
| tone mapping | mappage tonal | photographic tone-map |

### Render and compute

| Concept | French term | Sense / hazard guarded |
|---|---|---|
| renderer | moteur de rendu | drawing backend; distinct from a generic `système de rendu` |
| shader | shader | kept verbatim, dev-facing |
| allocation (memory) / managed allocator | allocation (mémoire) / allocateur géré | allocation and the allocator/report |
| thread | thread | compute thread; kept verbatim to avoid collision with fil (wire) |
| widget | widget | UI element, kept verbatim |
| validation | validation | the validation-tree feature; distinct from vérification (checks) |
| batch mode | mode batch | locked technical sense and catalog consistency |
| fork (process) | fork | kept verbatim |
| deadlock | interblocage | locked technical sense and catalog consistency |
| notifier | notificateur | locked technical sense and catalog consistency |
| token / operand / operator / arity | jeton / opérande / opérateur / arité | expression-parser terms |
| override | supplanter | supersede a value (SY symbol); not écraser (overwrite) |
| swap | échanger | locked technical sense and catalog consistency |
| theme | thème | UI/color theme; not "sujet" |

### Metrics and miscellaneous

| Concept | French term | Sense / hazard guarded |
|---|---|---|
| noise / noise temperature | bruit / température de bruit | electronic/thermal noise; not acoustic racket |
| efficiency | efficacité | locked technical sense and catalog consistency |
| interpolation | interpolation | locked technical sense and catalog consistency |
| mnemonic | mnémonique | a card's code descriptor; not a memo/note |
| degrees / deg | degrés / (deg) | freestanding prose vs parenthetical unit tag; tag kept like other unit tags |
| diameter | diamètre | cognate term, no competing native synonym in French |
| reflect | réfléchir (géométrie) / miroir de (comportemental) / réflexion (physique) | three distinct senses, never merged |
| default(s) | valeur(s) par défaut | fallback value |
| normalize / normalization | normaliser / normalisation | translated, never transliterated |

## 11. Disambiguation policy

- Choose the correct technical sense per entry; NEC2/RF context already disambiguates without an added qualifier.
- Never add a qualifier absent from the source (eg do not turn "View Currents" into "Voir les courants électriques").
- Add a qualifier only where the French term would otherwise be genuinely ambiguous.
- Keep the intra-domain terms `charge` and `impédance de charge` distinct even where context alone would disambiguate them.
- Locative homonym: `sol` (ground/earth) also carries a generic "floor/soil" sense in French; RF context resolves it without an added qualifier.
- Gerund vs noun: "animer" (verb) kept distinct from "animation" (noun) by grammatical role.

## 12. Cross-catalog consistency

- One term per concept; reuse the topic-10 table term at every recurrence.
- Keep every false-friend side distinct: `polarité` vs `polarisation`; `module` vs `amplitude`; `valeur de crête` vs `module de crête`; `masse` vs `sol`; `impédance de charge` vs `charge`; `gain` vs `amplification` vs `profit`; `courant` vs `actuel` or `récent`; `charge` vs `facturation`; `fil` vs `câble` or `cordon`, and `fil` vs `thread`; `diagramme de rayonnement` vs `gabarit` or `modèle`, and vs `champ lointain`; `excitation` vs `enthousiasme`; `nœud` or `ventre` vs `zéro numérique`; `famille d'échelle` vs `teinte` vs `type de palette` vs `projection de couleur`; `comète` vs `géométrie`; `identité` vs `unité`; `moteur de rendu` vs `système de rendu`; `supplanter` vs `écraser`; `visualiseur` vs `observateur`, `orateur`, or `aperçu`; `réfléchir` vs `miroir de` vs `réflexion`; `structure` vs `construction`; `thème` vs `sujet`; `validation` vs `vérification`; `gain net` vs `gain réel`; `puissance` vs `loi de puissance`.
- Loanword-vs-native: segment, patch, tag, fork, shader, widget, thread kept as loanwords or verbatim per topic 4; every other concept uses a native French term.
- Canonical spelling: `μ-law` kept in its symbol form, never spelled "mu-law" or transliterated.
- No minority-outlier spelling recorded; one spelling per term throughout the catalog.
- Consistency outranks locale-form preference for: plain quotes over guillemets (topic 3), plain space over non-breaking space before `:` (topic 3).

## 13. Priority ordering

- Precedence chain: correct meaning, then interface convention, then catalog consistency, then disambiguation, then locale numeral form.
- Explicit override: catalog consistency overrides standard print-French punctuation convention for quotes and colon-spacing (topics 3, 12).

## 14. Grammatical number

- French uses singular and plural nouns, adjectives, participles, and finite verbs; make every agreeing form match the grammatical number of its head or subject.
- After an explicit literal count, use singular for `0` and `1` and plural for values greater than `1`, while preserving any fixed technical token under topics 2 and 4.
- Preserve invariant acronyms, symbols, and retained terms without inventing a plural marker.

## 15. Grammatical agreement

- Make adjectives and participles agree in gender and number with the explicit or implied head noun, including standalone labels.
- Resolve an implied standalone head from the named interface entity (eg masculine `visualiseur`, feminine `carte`).
- French has no noun-case declension and no partitive after a numeral; these features are not applicable.
- Retain a reflexive particle where the chosen French verb requires it; omit none required by the verb's sense.

## 16. Morphological derivation

- Derive borrowed technical verbs with the established French `-er` pattern; use no improvised conjugation family.
- Form verbal nouns with the established French derivative for the concept, including `-tion`, `-age`, or a lexical noun; use an infinitive as a noun only where idiomatic.
- Prefer an established native derivative to an unassimilated loan; form new technical compounds under topic 1.

## 17. Preposition and sandhi selection

- Apply elision before a vowel or mute `h`, including `l'`, `d'`, and `qu'` (eg `d'échelle`).
- Apply required contractions `à le` → `au`, `à les` → `aux`, `de le` → `du`, and `de les` → `des`.
- Further sound-conditioned preposition alternation is not applicable to French.

## 18. Card/record-label register

- Use `Carte` as the fixed generic designator when it begins a dialog or editor title; retain the following NEC2 mnemonic unchanged.
- Use lowercase `carte` in running prose and join its designation as a spaced phrase (eg `carte GW`).
- French has no separate short and long generic designator; this distinction is not applicable.
- Keep title and running-prose forms internally consistent and never cross-convert them.

## 19. Multi-paragraph and whitespace fidelity

- Mirror every source paragraph break at the same position and preserve whether it is a blank-line break or a single newline.
- Preserve semantic line breaks; introduce no line break for visual wrapping.
- Drop every clause removed from the current source instead of inheriting it from an older translation.
- Carry source trailing newlines and punctuation mechanically.
- Preserve complete meaning; truncate nothing and invent no abbreviation for an assumed display limit.

## 20. Current-source fidelity

- Derive every translation from the current source literal and its supplied context.
- Reuse an inherited or copied translation only when its complete meaning agrees with the current source and context.
- Treat prior wording with an obsolete qualifier, removed clause, changed technical sense, or changed string-family role as unsafe to inherit.

## 21. Script hygiene

- Forbid non-Latin homoglyphs and foreign-script characters in French prose; permit them only where a retained token in topic 4 requires its exact spelling.
- Translate ordinary foreign prose words; retain only genuine identifiers, symbols, units, filenames, proper names, and locked loanwords.
- Mixed-script adjacency needs no additional French rule beyond the exact-token boundary above.

## 22. Rule-file scope hygiene

- Keep only current decisions that can alter wording, Unicode characters, punctuation, capitalization, embedded tokens, mnemonic markers, or semantic whitespace in a translated literal.
- Omit catalog representation, serialization, headers, flags, workflow, rendering, fonts, shaping, layout, widget behavior, runtime formatting, sorting, search, display sizing, audit history, completion state, review metadata, provenance, and citations.
- State each retained decision directly; include no implementation procedure, bibliography, correction history, or alternative policy.

## 23. Section-disjointness declaration

- Topics 1 and 21 govern script characters and orthography; topics 2, 3, 5-7, 9, and 14-20 govern phrasing and literal structure; topic 8 governs formality and address.
- Apply each decision on only its stated axis so script mechanics, phrasing, and address register never compete.

## 24. Developer/debug-string policy

- Translate user-facing commands, labels, dialogs, tooltips, statuses, and errors into complete French; review priority never permits an applicable user-facing string to remain untranslated.
- Translate informational diagnostics intended for operators into concise technical French.
- Translate developer-facing diagnostics, including `mem_track`, `config_widget`, `themes`, `validation_dump`, and `prerender` families, into terse neutral technical French; no subsystem family overrides this rule.
- Preserve every embedded identifier, function name, configuration key, format specifier, and topic-4 token in all three families regardless of review priority.
- Render the `BUG:` diagnostic prefix as "BOGUE :", held distinct from the "ERREUR :" prefix that carries ordinary failures; the two severities never share a word.
- Keep an identifier-plus-value diagnostic pair contiguous (eg `sweep_state=%d`); a qualifier precedes the whole pair and never stands between the identifier and its `=`.
