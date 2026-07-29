# ro (Romanian, Romania) translation rules

These rules govern modern standard Romanian for professional electrical and
radio-frequency engineers, antenna designers, and amateur-radio operators in
Romania.

## 1. Script and orthography

- writing system: Latin, left-to-right; letter-case distinction present
  (upper/lowercase), so casing rules (topic 6) apply.
- diacritics ă â î ș ț (U+0103, U+00E2, U+00EE, U+0219, U+021B) are mandatory:
  never strip them, fold them to plain letters, or replace comma-below ș ț
  with cedilla look-alikes ş ţ (U+015F, U+0163).
- precomposed form only (NFC); no combining-diacritic sequences.
- word-internal joiner/apostrophe code points: not applicable, Romanian uses
  none.
- script-specific joining/shaping mechanics: not applicable, Latin script has
  no positional letterforms or ligature rules here.
- orthographic standard: modern post-1993 DOOM2 reform; `â` mid-word, `î`
  word-initial/word-final (eg "în", "hotărî", "pârâu").
- inter-word spacing: single space; embedded foreign/numeric tokens and unit
  symbols keep source spacing (eg "10 MHz", "5%").
- compound-formation: spaced or hyphenated per standard orthography, never an
  ad hoc fused form (eg "plan de masă", not "planmasă").

## 2. Numerals in literals

- use digits 0-9 for technical values in Romanian prose.
- use comma as the decimal separator and period as the grouping separator in
  numbers physically written as Romanian prose.
- keep formulas, examples, fixed defaults, and named mathematical or standards
  constants exactly in source form.
- form ordinals as a digit plus the native affix (eg "al 2-lea"); treat its
  hyphen as an index marker, distinct from the decimal comma.

## 3. Punctuation and quotation

- native quotation marks „…” (U+201E opening, U+201D closing); embedded
  technical tokens and literal file-format strings keep source plain quotes
  `"` for round-trip consistency.
- native punctuation replaces source in prose: comma, question mark,
  exclamation mark; Romanian shares the source's base glyphs so no further
  substitution applies.
- spacing: no space before colon/semicolon/terminal punctuation; one space
  after.
- ellipsis: single glyph `…`; source dashes (en/em) are preserved as given.
- sentence terminator: full stop for complete sentences; short
  labels/fragments/menu items omit it.
- punctuation inside embedded technical runs (format strings, filenames, card
  mnemonics) stays in source form, unconverted.

## 4. Never-translate tokens

Apply a zero-failure invariant: preserve every retained token character for
character, with source case, spelling, punctuation, and direction; translate or
transliterate none.

- keep NEC2 card mnemonics verbatim: GW GA GH EX LD FR RP GE EN SP SM SC NE NH
  NT TL GN GD GC GX GR GS GM GF CP CM SY XQ EK KH PQ PT.
- keep every source unit symbol verbatim, including Ω, dBi, dB, MHz, %, and °;
  these symbols preserve standard electrical and radio-frequency notation.
- keep VSWR, S-parameter forms, Z, Z0, front/back-ratio tokens, and
  gain/temperature tokens verbatim; these preserve established RF notation.
- keep every source file extension and printf-style format specifier verbatim;
  these preserve literal file and substitution syntax.
- keep embedded function names, variable names, and configuration keys
  verbatim; they identify program entities rather than prose.
- keep physical and mathematical symbol letters verbatim, including E, H, φ,
  θ, and γ; native look-alikes can change the notation.
- keep product, library, toolkit, tool, and chart proper names in their source
  spelling; they remain proper names.
- keep Log, Asinh, μ-law, Reinhard, and Sigmoid verbatim; translate the
  descriptive family `Power` as "Lege de putere" to separate it from
  electrical power.
- keep `Patch`, `tag`, and process `fork`; translate `segment` as "segment";
  these choices apply file-wide and distinguish conditional loanwords from
  the retained roster.

## 5. Format-specifier integrity

Apply a zero-failure invariant to format tokens.

- preserve exactly the source specifier set in the translation; add, drop, or
  alter none.
- default order matches source order; positional reordering (`%1$s`,
  `%2$d`...) is allowed only where Romanian grammar forces a different word
  order.
- for divergent word order, restructure the surrounding sentence around the
  specifier's fixed position rather than moving the specifier itself.
- numbers produced inside a specifier are never localized (topic 2).

## 6. Capitalization and title-case

- default casing: sentence case for labels, menu items, titles; capitalize
  only the first word and proper nouns/acronyms (NEC2, VSWR, GW, EX, LD, FR,
  RP); never title-case every word.
- axis-letter casing: keep source axis letters (X, Y, Z) uppercase as given;
  no override.
- lowercase math/coordinate variables (eg φ, θ) stay lowercase regardless of
  sentence-case position.
- exception: coordinated option names pairing two distinct quantities/modes
  with `/` or `+` capitalize each element, mirroring the source label (eg
  "Staționară/Călătoare", "Curent + Sarcină").
- generic card/record noun `card`/`carduri` is lowercase mid-phrase,
  capitalized only as the first word of a standalone label (eg "Card GH" vs
  "card EX").
- acronyms and proper nouns retain source casing.

## 7. Interface register by string type

Treat each string family as distinct Romanian interface grammar; preserve all
source meaning in the shortest complete natural form.

- commands, buttons, and menu actions: use a terse imperative or established
  action noun, omit the subject, place the verb before its object, and use no
  conversational filler (eg "Salvează").
- field labels: use a compact noun phrase in head-before-dependent order and
  retain a source colon (eg "Frecvență:").
- dialogs and confirmations: use full sentences in formal indicative or
  interrogative mood; state known information before the requested decision.
- tooltips: use a complete descriptive clause in normal subject-verb-object
  order; preserve the source reason when a control is unavailable.
- status and error messages: use an impersonal reflexive, passive-like, or
  declarative sentence; present the affected entity before its state where
  natural and omit direct address.
- user-visible domain entities: name the entity type before its identifier or
  qualifier (eg "Card GH"); apply personal-name policy only under topic 8.
- prefer standard Romanian compounds and concise multiword terms; omit no
  meaning, invent no abbreviation, and do not imitate source length.
- apply topic 24 only to developer and diagnostic strings.

## 8. Formality and address

- use a formal, neutral professional register carried by construction and word
  choice; omit an explicit address pronoun in short commands.
- use subject omission and impersonal forms for commands and status messages;
  use formal second-person plural endings only where a dialog addresses the
  user directly.
- avoid grammatical gender for the user through impersonal constructions;
  where gender, number, or animacy is grammatically required, agree with the
  named entity rather than inferring a person category.
- honorifics are not applicable: add none to interface text.
- use second person only in confirmations; use the pattern "Sigur doriți
  să..." and no informal singular form.
- personal names, when present in source text, retain source order and receive
  no added honorific.
- prohibit casual, slang, commercial, archaic, and ceremonially formal
  registers.

## 9. Accelerator/hotkey mnemonics

- place `_` immediately before the mnemonic letter inside the translated
  literal.
- choose the letter from the Romanian term; never copy or transliterate the
  source mnemonic independently of that term.
- separate parenthetical mnemonic letters are not applicable because Romanian
  uses the same script as its interface terms.
- preserve source presence exactly: add no mnemonic where the source has none
  and retain one where the source has one.
- choose a directly typable letter; prefer an unaccented letter from the same
  term when available.
- where translated siblings collide, move `_` to another suitable letter in
  the same term or an adjacent word without changing the wording.

## 10. Domain lexicon

Lock every row to one Romanian term for correct engineering sense and catalog
consistency; a stated hazard adds a narrower false-friend boundary.

| Source concept | Romanian | Sense / hazard guarded |
|---|---|---|
| current (electrical) | curent | Amperes; not temporal "recent" |
| charge (electrical) | sarcină electrică | Coulombs; not billing/fee or impedance load |
| voltage | tensiune | electric potential |
| power (electrical) | putere (electrică) | watts/gain; distinct from "Power" scale-family name |
| impedance | impedanță | complex Z |
| resistance | rezistență | real part of Z |
| reactance | reactanță | imaginary part of Z |
| inductance | inductanță | |
| capacitance | capacitate | |
| conductivity | conductivitate | material S/m |
| admittance | admitanță | admittance-matrix sense |
| load (impedance) | impedanță de sarcină | LD-card impedance; distinct from electrical charge |
| gain | câștig | antenna directivity dB; not profit/amplification |
| excitation | excitație | EM input; not emotional excitement |
| feedpoint | punct de alimentare | antenna feed point |
| port | port | excitation/S-parameter port |
| radials (noun) | radiale | ground-plane radial wires; distinct from adjective "radial" |
| ground / ground plane | masă / plan de masă | RF reference; not soil |
| earth (physical medium) | pământ / sol | terrain/noise-model earth; distinct from electrical ground |
| ground wave | undă de sol | propagation term |
| wire | fir / fire | conductor; not cable/cord |
| segment | segment | NEC2 geometry subdivision |
| patch | Patch | kept loanword (topic 4) |
| tag | tag | kept loanword (topic 4) |
| card | card | register in topic 18 |
| kernel | nucleu (integral) | thin-wire kernel; not OS kernel |
| cliff | prag (tip stâncă) | ground-boundary type; not a fracture |
| structure | structură | antenna model geometry; not "construcție" |
| model | model | NEC or noise-temperature model |
| geometry | geometrie | model geometry |
| crossed | încrucișat(e) | reversed conductors; not cut/severed |
| field (EM) | câmp | near/far/E/H field; disambiguated from data/config field by context |
| near field | câmp apropiat | |
| far field | câmp îndepărtat | |
| far-field contribution | contribuție de câmp îndepărtat | per-direction; not near-field animation |
| radiation | radiație | radiated emission |
| radiation pattern | diagramă de radiație | plotted response; not a template |
| gain pattern | diagramă de câștig | |
| polarization | polarizare | field orientation |
| polarity | polaritate | sign of a value; not polarizare |
| phase | fază | |
| reference phase | fază de referință | |
| frequency | frecvență | |
| wave / wavelength | undă / lungime de undă | |
| standing wave | undă staționară | |
| traveling wave | undă călătoare | |
| node / antinode | nod / antinod | standing-wave zero/max |
| crest | creastă | instantaneous wave apex; not curve peak ("vârf") |
| magnitude | magnitudine | modulus/scalar; distinct from amplitude |
| amplitude | amplitudine | oscillating peak; distinct from magnitude |
| peak value | valoare de vârf | distinct UI option from peak magnitude |
| peak magnitude | magnitudine de vârf | distinct UI option from peak value |
| instantaneous | instantaneu(ă) | "(φ=0)" qualifier added only if source carries it |
| Poynting vector | vector Poynting | named vector, kept structure |
| solid angle | unghi solid | |
| net gain | câștig net | total minus mismatch; not "câștig real" |
| viewer | vizualizator | observation direction / 3D view widget; not observator/difuzor/previzualizare |
| flow / flow direction | flux / direcție flux | patch/current flow |
| total field | câmp total | |
| color | culoare | |
| color projection | proiecție de culoare | which quantity drives hue |
| hue | nuanță | color-wheel angle |
| brightness | luminozitate | luminance channel |
| hue encoding | codare nuanță | distinct enum, not "proiecție de culoare" |
| brightness encoding | codare luminozitate | distinct enum |
| color scale | scală de culoare | magnitude-to-color scale |
| scale family / color tone | familie de scalare | transfer-curve family; one term for both source spellings |
| palette / palette kind | paletă / tip paletă | distinct from scale family and color projection |
| ramp / gradient | rampă / degrade | palette kind / linear strip |
| gamma | gama | power-law exponent |
| knee | cot (moale) | soft-knee bend point |
| softening | atenuare | dynamic-range easing |
| compression | compresie | dynamic-range compression |
| contrast | contrast | |
| dynamic range | interval dinamic | |
| floor | prag minim | brightness/dB floor; not a room floor |
| envelope | anvelopă | magnitude/amplitude envelope |
| comet | cometă | moving-crest overlay effect; not geometry |
| overlay (noun) | suprapunere | added visual layer; verb is "a suprapune" |
| animate / animation | a anima / animație | |
| animated / static | animat / static | category-header adjectives |
| projection | proiecție | color or geometry projection |
| scale (verb/noun) | a scala / scală | |
| wireframe | cadru de sârmă | established idiom; "sârmă" used only here |
| identity | identitate | no-op transfer; distinct from unitate (Smith chart) |
| sentinel | valoare santinelă | unreachable-case guard |
| bins | intervale | discretization buckets |
| companding | compandare | bounded log curve (μ-law) |
| tone mapping | mapare tonală | photographic tone-map |
| renderer | renderer | drawing backend; kept loanword, never "motor de randare" |
| shader | shader | kept loanword |
| allocation (memory) | alocare (memorie) | |
| managed allocator | alocator gestionat | |
| thread (compute) | fir de execuție | homonym with "fir" (wire); context disambiguates |
| widget | widget | UI element, kept loanword |
| validation | validare | validation-tree feature; distinct from verificare (checks) |
| batch mode | mod loturi | |
| fork (process) | fork | kept loanword (topic 4) |
| deadlock | blocaj | |
| notifier | notificator | |
| token / operand / operator / arity | token / operand / operator / aritate | expression-parser terms |
| override | prevalare | supersede a value (SY); not "suprascriere" (overwrite) |
| swap | interschimbare | exchange |
| theme | temă | UI/color theme; not "subiect" |
| noise / noise temperature | zgomot / temperatură de zgomot | electronic/thermal; not acoustic |
| efficiency | eficiență | |
| interpolation | interpolare | |
| mnemonic | mnemonică | card code descriptor; not a memo |
| degrees / deg | grade / (deg) | freestanding prose vs unit tag; tag kept as "(deg)" |
| diameter | diametru | canonical term, no competing synonym retained |
| reflect (geometry) | a reflecta (geometrie) | mirror operation |
| reflect (behavioral) | oglindește | tracking another control |
| reflect (physics) | reflexie | physical reflection; three distinct senses |
| default(s) | valoare implicită / implicite | fallback value |
| normalize / normalization | a normaliza / normalizare | translated, not transliterated |

- add any further source-domain concept to this table with one locked Romanian
  term, intended sense, and consistency or false-friend purpose.

## 11. Disambiguation policy

- choose the correct technical sense for every ambiguous source term.
- add no qualifier absent from the source when program context already fixes
  the sense (eg "Curenți").
- add a qualifier only where the Romanian term would otherwise remain
  ambiguous.
- separate electrical charge "sarcină electrică" from impedance load
  "impedanță de sarcină"; neither term is shortened where the two senses can
  collide.
- no locative homonym collision applies to the locked lexicon.
- resolve verbal and nominal senses by their topic 10 entries (eg "a scala"
  versus "scală").

## 12. Cross-catalog consistency

- one term per concept across the catalog; reuse the topic 10 lexicon, no ad
  hoc synonym introduced.
- false-friend pairs, resolved distinctly: polaritate≠polarizare;
  magnitudine≠amplitudine; valoare de vârf≠magnitudine de vârf;
  masă≠pământ/sol; impedanță de sarcină≠sarcină electrică;
  câștig≠amplificare≠profit; curent(electric)≠recent(temporal);
  sarcină electrică≠facturare;
  fir(conductor)≠cablu, and fir≠fir de execuție(thread); diagramă de
  radiație≠șablon/proiect, and ≠câmp îndepărtat; excitație≠entuziasm;
  nod/antinod≠zero numeric generic; familie de scalare≠nuanță≠tip
  paletă≠proiecție de culoare (four distinct chroma concepts);
  cometă≠geometrie; identitate≠unitate; renderer≠"motor de randare";
  prevalare≠suprascriere; vizualizator≠observator/difuzor/previzualizare;
  a reflecta≠oglindește≠reflexie; structură≠construcție; temă≠subiect;
  validare≠verificare; câștig net≠câștig real; putere electrică≠Lege de
  putere (familie de scalare).
- loanword-vs-native decisions locked: Patch, tag, fork, renderer, shader,
  widget kept as loanwords; segment, geometrie, structură translate
  natively; the canonical spelling for each is the one used in topic 10.
- minority-outlier spellings (eg cedilla ş/ţ forms, alternate
  transliterations) unify to the canonical comma-diacritic form (topic 1).
- catalog consistency outranks a local synonym only after correct meaning and
  interface convention; `Patch`, `tag`, `fork`, `renderer`, `shader`, and
  `widget` therefore keep their locked spellings.

## 13. Priority ordering

- apply this precedence: correct meaning, interface convention, catalog
  consistency, disambiguation, then literal numeral form.
- correct meaning overrides prior catalog reuse for `load`: use "impedanță de
  sarcină", not the colliding short form "sarcină".

## 14. Grammatical number

- use Romanian singular and plural noun forms; inflect dependent adjectives,
  participles, and verbs for the same grammatical number.
- after an explicit literal count, use singular with `1`, the first plural
  form with `0` and with counts whose final two digits are `01-19` when the
  whole count is not `1`, and the general plural form with all other counts.
- invent no number marker outside standard Romanian inflection.

## 15. Grammatical agreement

- adjectives and participles agree in gender/number with the head noun,
  including standalone labels whose head noun is implied.
- declension-class and partitive-after-count choices follow standard
  Romanian noun-phrase rules; no domain-specific override.
- the reflexive particle "se" is retained in impersonal constructions
  (topic 7) where grammatically required.
- a standalone label with no explicit head noun defaults to masculine
  singular agreement, matching the implied "buton"/"câmp" head.

## 16. Morphological derivation

- borrowed technical verbs form with the standard "-a" infinitive family (eg
  "a randa", "a interpola"); ad hoc "-iza" coinages are forbidden where a
  "-a" form is already established in the catalog.
- verbal-noun formation uses the "-are" nominalization (eg "randare",
  "interpolare", "alocare").
- native-affix derivation is preferred over a new loanword when a native
  form is already established (eg "compandare" over an untranslated
  "companding"); established loanwords (topics 4, 10) are kept as-is, not
  re-derived.

## 17. Preposition and sandhi selection

- not applicable: Romanian prepositions do not vary by a following-sound
  trigger.
- elision/contraction follows standard written Romanian orthography with no
  domain-specific exception.

## 18. Card/record-label register

- fixed designator form in dialog/editor titles: "Card <MNEMONIC>" (eg "Card
  GH"), capitalized as the first word of the label.
- running-prose form in messages: lowercase "card <mnemonic>" inline,
  optionally suffixed in a compound (eg "cardul GH", "cardurile GN și GD").
- generic-noun casing follows topic 6; no short-vs-long form distinction
  beyond singular/plural ("card"/"carduri").
- each register (title-form vs prose-form) stays internally consistent per
  string type; never cross-converted mid-catalog.

## 19. Multi-paragraph and whitespace fidelity

- mirror source paragraph breaks at the same positions, preserving the
  distinction between blank-line and single-line breaks.
- drop every trailing clause absent from the current source rather than
  retaining inherited text.
- preserve source trailing newlines and terminal punctuation exactly.
- preserve semantic line breaks and add no visual wrapping absent from the
  source literal.
- preserve complete meaning; do not truncate or abbreviate text for an assumed
  display limit.

## 20. Current-source fidelity

- derive every translation from the complete current source literal and its
  supplied context.
- reuse inherited wording only when its full meaning agrees with the current
  source and context.
- do not inherit a shortened `load` or `charge` term, a mnemonic detached from
  its translated word, an informal singular address form, or a stale trailing
  clause.

## 21. Script hygiene

Apply a zero-failure invariant to Romanian prose.

- forbid cedilla letters `ş` and `ţ`, combining replacements for precomposed
  Romanian letters, and foreign-script homoglyphs adjacent to Romanian words.
- allow a foreign-script character only inside a retained token whose literal
  source spelling requires it.
- translate plain foreign words; preserve only the identifiers, symbols,
  units, names, and loanwords locked by topics 4 and 10.

## 22. Rule-file scope hygiene

- retain only current decisions that can alter wording, Unicode characters,
  punctuation, capitalization, embedded tokens, mnemonic markers, or semantic
  whitespace in a translated literal.
- omit catalog representation, serialization, headers, flags, workflow,
  rendering, fonts, shaping, layout, widget behavior, runtime formatting,
  sorting, search, display sizing, audit history, completion state, review
  metadata, provenance, and citations.
- state each retained decision directly; include no implementation procedure,
  bibliography, correction history, or alternative policy source.

## 23. Section-disjointness declaration

- topic 1 (script mechanics), topics 6-7 and 9 (phrasing/structure/mnemonic
  register), and topic 8 (address register) are non-overlapping: topic 1
  governs glyph/orthography only, topics 6-7/9 govern casing/wording/
  mnemonic structure, topic 8 governs formality/pronoun choice; each concept
  in this file is addressed in exactly one of these sections.

## 24. Developer/debug-string policy

- translate user-facing commands, labels, dialogs, tooltips, status messages,
  errors, and informational notices into Romanian regardless of review
  priority.
- keep developer-only `pr_debug` strings and internal-only diagnostics in
  source form; use terse technical wording for any diagnostic family already
  established as translated within its subsystem.
- preserve every embedded identifier, function name, retained token, and
  format specifier verbatim in every family, translated or untranslated.
- classify a string by its audience and effect: interaction and actionable
  diagnostics are user-facing, explanatory notices are informational, and
  implementation-state traces are developer-facing.
- use priority only to order review; never use it to leave a user-facing or
  informational string untranslated.
