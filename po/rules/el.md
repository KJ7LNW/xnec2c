# el (Greek) translation rules

## 1. Script and orthography

- Use Standard Modern Greek for Greece, written in the monotonic Greek
  alphabet under the post-1982 orthography, for professional electrical and
  radio-frequency engineers, antenna designers, and amateur-radio operators.
- Use all 24 Greek letters Α-Ω/α-ω. Mark stress with one tonos on the
  stressed vowel of each polysyllabic word (ά έ ή ί ό ύ ώ); use diaeresis
  (ϊ ϋ) where adjacent vowels form separate syllables. Omit polytonic marks.
- Use final lowercase sigma ς word-finally and σ initially or medially.
- Use precomposed accented-vowel code points, eg ά U+03AC; omit decomposed
  vowel-plus-combining-tonos sequences and non-Greek look-alikes.
- Greek has no textual joiner or required joining-form distinction; this
  sub-concept is not applicable.
- Write left-to-right. Retained technical tokens remain left-to-right and
  are neither mirrored nor wrapped in manual direction controls.
- Apply Greek upper/lowercase distinctions under topic 6.
- Separate words with one space; use one space between Greek text and an
  embedded retained or numeric token.
- Form established single-word compounds as fused words and genitive
  technical noun chains as spaced words; use a hyphen only where standard
  Greek orthography requires one.

## 2. Numerals in literals

- Use Western Arabic digits for technical values; omit alphabetic Greek
  numeral forms to preserve engineering notation.
- Use a period as the decimal separator and a comma as the thousands
  separator in numbers physically present in a translated literal, eg
  `1,000.5`, for consistency with fixed technical notation.
- Preserve formulas, worked examples, fixed defaults, and named
  mathematical or standards constants exactly as source tokens; translate
  only their surrounding prose.
- Greek uses no productive ordinal or index affix joined to a literal digit
  in this interface; this sub-concept is not applicable.
- Keep an index separator distinct from the decimal point; preserve the
  source separator in a literal indexed symbol.

## 3. Punctuation and quotation

- Use Greek guillemets «» for quoted prose; retain straight source quotes
  around embedded technical tokens for token consistency.
- Use the source comma, ASCII question mark, exclamation mark, colon, and
  semicolon in Greek interface prose; omit the Greek erotimatiko to keep
  punctuation consistent across technical strings.
- Put no space before a colon, semicolon, or terminal mark and one space
  after a nonterminal punctuation mark.
- Preserve the source ellipsis form, either `…` or `...`, and preserve an
  en dash or em dash where it expresses the source relation.
- End full declarative sentences with a full stop, questions with `?`, and
  exclamations with `!`; omit a terminator from short labels and fragments.
- Preserve source punctuation inside retained technical runs.

## 4. Never-translate tokens

Treat every retained token as a zero-change boundary: preserve its spelling,
case, punctuation, digits, script, and left-to-right order without translation
or transliteration.

- Preserve NEC2 card mnemonics verbatim: GW GA GH EX LD FR RP GE EN SP SM
  SC NE NH NT TL GN GD GC GX GR GS GM GF CP CM SY XQ EK KH PQ PT.
- Preserve standard unit symbols verbatim, including Hz, kHz, MHz, GHz, dB,
  dBi, Ω, W, K, S/m, deg, and %.
- Preserve figure-of-merit and parameter tokens verbatim: VSWR, S11, S12,
  S21, S22, Z, Z0, F/B, and G/T.
- Preserve file extensions verbatim: `.nec`, `.csv`, `.s1p`, `.s2p`, and
  `.png`.
- Preserve every source format specifier verbatim under topic 5.
- Preserve embedded function names, variable names, configuration keys,
  and other code identifiers verbatim inside diagnostic strings.
- Preserve physical and mathematical notation letters verbatim, including
  E, H, Z, γ, φ, θ, Ω, and μ; never replace one with a look-alike code point.
- Preserve product, library, toolkit, program, and chart proper names
  verbatim.
- Preserve named transfer functions and algorithms verbatim: Log, Asinh,
  μ-law, Reinhard, Sigmoid, and Identity. Translate the descriptive family
  name Power as Δύναμη.
- Translate `segment` as Τμήμα; retain `patch` as Patch, `tag` as Tag, and
  process `fork` as fork. Apply each geometry or process choice file-wide.

## 5. Format-specifier integrity

- Preserve every printf-style specifier from the source literal, with the
  same set and no addition or omission.
- Keep specifiers in source order by default.
- Where Greek word order would otherwise require reordering a specifier,
  restructure the surrounding sentence instead of moving the specifier.
- Never localize a number that appears inside a specifier's own
  substitution.

## 6. Capitalization and title-case

- Capitalize only the first word of a menu item, button label, or title;
  omit title case because Greek interface labels use sentence case.
- Keep axis letters (x, y, z) in source case; do not capitalize them.
- Keep a lowercase math/coordinate variable lowercase regardless of
  surrounding sentence-initial capitalization.
- No coordinated option name in this catalog capitalizes each element
  separately; skip that exception.
- Card/record-noun casing by position is topic 18's concern, not this
  topic's.
- Keep acronyms and proper nouns (NEC2, GTK, program name) in their
  source case.

## 7. Interface register by string type

Treat each string family as a fixed interface grammar; preserve all source
meaning without imitating source length.

- Render commands, buttons, and menu actions as concise deverbal nouns or
  noun phrases, with no expressed subject or object unless needed for the
  action's meaning.
- Render field labels as compact noun phrases in normal Greek head-modifier
  or genitive order, followed by the source colon.
- Render dialogs as full declarative sentences and confirmations as full
  formal-plural questions: state the action before its consequence or
  requested decision.
- Render tooltips as full declarative sentences in natural subject-verb-
  object order; include the stated reason when a control is unavailable.
- Render status and error messages as terse impersonal declarative or
  passive sentences: present the affected entity before the result or cause.
- Name user-visible domain entities with the topic-10 head term followed by
  a Greek modifier or genitive complement; retain proper names unchanged
  under topic 4 and personal names under topic 8.
- Prefer the shortest complete natural form and established compounds;
  omit neither meaning nor words solely to match source length.
- Apply topic 24 to developer and diagnostic strings.

## 8. Formality and address

- Use formal second-person plural verb endings for direct address; omit
  informal singular forms.
- Use subject-drop and impersonal constructions for status, error, and
  informational strings; use direct formal address only in dialogs that
  request a user decision.
- Use noun phrases without grammatical person for commands and labels; use
  a full formal-plural question for confirmations.
- Avoid gendered references to people where a natural impersonal form
  exists; preserve required grammatical gender, number, and animacy through
  agreement without introducing exclusionary wording.
- Use neuter gender for a generic technical label or implied status subject
  that has no lexical gender.
- Greek address requires no honorific in this interface; this sub-concept is
  not applicable.
- Use second person only for direct user address and third-person or
  impersonal grammar elsewhere.
- Preserve each person's written given-name and family-name order; do not
  reorder or inflect retained personal names.
- Omit casual, slang, commercial, archaic, and ceremonial registers.

## 9. Accelerator/hotkey mnemonics

- Mark a mnemonic with `_` immediately before its letter inside the
  translated literal.
- Select the mnemonic from the translated Greek term rather than from a
  transliteration of the source term.
- Where activation requires a separate Latin-script key, append that key
  as `(_X)` after the Greek label.
- Add a mnemonic only when the source literal contains one.
- Select a unique, directly typable key within the containing menu or dialog;
  on collision, use another letter from the same translated term.
- A separate key is not applicable to literals whose Greek letter is
  directly usable as the mnemonic.

## 10. Domain lexicon

Concept - target term - sense - purpose/hazard guarded.

### Electrical primitives

| Concept | Term | Sense | Purpose |
|---|---|---|---|
| current | Ρεύμα | electrical current, A | not "τρέχον" (temporal "current") |
| charge | Φορτίο | electrical charge, C | not billing/fee/cargo |
| voltage | Τάση | electric potential | distinct from Ρεύμα, Φορτίο |
| power (electrical) | Ισχύς | watts, power gain, power-flow | distinct from Δύναμη (power-law family name) |
| impedance | Εμπέδηση | complex Z | distinct from Αντίσταση, Αντίδραση |
| resistance | Αντίσταση | real part of Z | distinct from Εμπέδηση, Φορτίο (load) |
| reactance | Αντίδραση | imaginary part of Z | distinct from Αντίσταση |
| inductance | Επαγωγή | named technical sense | locks the canonical term |
| capacitance | Χωρητικότητα | named technical sense | locks the canonical term |
| conductivity | Ειδική Αγωγιμότητα | material S/m | distinct from admittance below |
| admittance | Αγωγιμότητα | admittance-matrix Y | distinct from Εμπέδηση and from material conductivity |
| load | Φορτίο | LD-card impedance load | same noun as charge; NEC2/LD-card context disambiguates |
| gain | Κέρδος | antenna directivity, dB | not profit, not amplifier amplification |
| excitation | Διέγερση | EM energy input/source | not emotional excitement |
| feedpoint | Σημείο Τροφοδοσίας | antenna feed point | locks the canonical term |
| port | Θύρα | excitation/S-parameter port | locks the canonical term |
| radials | Ακτίνες (γείωσης) | ground-plane radial wires, noun | distinct from adjective "ακτινικός" |

### Ground and earth

| Concept | Term | Sense | Purpose |
|---|---|---|---|
| ground / ground plane | Γείωση | RF electrical reference plane, GN/GD, conductivity/type/effects | never "έδαφος"; one term across all ground sub-senses |
| earth (physical medium) | Έδαφος | terrain/noise-model earth, "below ground" geometry | distinct from Γείωση |
| ground wave | κύμα εδάφους | propagation term | distinct radio-propagation sense; keep εδάφους, not γείωση |

### Geometry primitives

| Concept | Term | Sense | Purpose |
|---|---|---|---|
| wire | Αγωγός | thin conductor / GW element | not "σύρμα" (generic cord) |
| segment | Τμήμα | NEC2 geometry subdivision | locks the canonical term |
| patch | Patch | NEC2 surface patch, SP/SM | kept Latin, conditional loanword (topic 4) |
| tag | Tag | NEC2 geometry identifier | kept Latin; not a UI label or card |
| card | κάρτα | NEC2 input record | register in topic 18 |
| kernel | πυρήνας | integral-equation/thin-wire kernel | not an OS kernel |
| cliff | Γκρεμός | two-medium ground-boundary type | not a fracture/break |
| structure | Δομή | antenna model geometry | not "κατασκευή" |
| model | Μοντέλο | NEC model or noise-temperature model | locks the canonical term |
| geometry | Γεωμετρία | the model geometry | locks the canonical term |
| crossed | διασταυρωμένος | transmission-line conductors crossed/reversed | not cut/severed |

### Field, pattern, viewer

| Concept | Term | Sense | Purpose |
|---|---|---|---|
| field (EM) | πεδίο | near/total/E/H field | shares the word with a data/config field; program context disambiguates |
| near field | εγγύς πεδίο | named technical sense | opposed pair with far field |
| far field | μακρινό πεδίο | named technical sense | opposed pair with near field |
| far-field contribution | συνεισφορά μακρινού πεδίου | per-direction contribution | not near-field animation |
| radiation | ακτινοβολία | radiated emission | locks the canonical term |
| radiation pattern | Διάγραμμα ακτινοβολίας | plotted directional response | not a template/design; one term catalog-wide |
| gain pattern | Διάγραμμα κέρδους | the gain radiation pattern | locks the canonical term |
| polarization | Πόλωση | antenna/wave field orientation | never used for polarity below |
| polarity | Πολικότητα | sign (+/-) of a quantity | false friend of Πόλωση; kept distinct |
| phase | Φάση | named technical sense | locks the canonical term |
| reference phase | φάση αναφοράς | named technical sense | locks the canonical term |
| frequency | Συχνότητα | named technical sense | locks the canonical term |
| wave / wavelength | κύμα / μήκος κύματος | named technical sense | locks the canonical term |
| standing wave | στάσιμο κύμα | named technical sense | opposed pair with traveling wave |
| traveling wave | οδεύον κύμα | named technical sense | opposed pair with standing wave |
| node | δεσμός | standing-wave zero | also the null/overlay sense |
| antinode | κοιλία | standing-wave maximum | also the peak/overlay sense |
| crest | κορυφή κύματος | instantaneous wave apex, comet-head | distinct from μέγιστο (curve/step peak) |
| magnitude | Μέτρο | modulus of a quantity, |Z| | distinct from amplitude |
| amplitude | Πλάτος | oscillating-quantity peak | distinct from magnitude |
| peak value | μέγιστη τιμή | named technical sense | distinct UI option from peak magnitude; never collapsed |
| peak magnitude | μέγιστο μέτρο | named technical sense | distinct UI option from peak value; never collapsed |
| instantaneous | στιγμιαίο | projection mode | append "(φ=0)" only where the source carries it |
| Poynting vector | διάνυσμα Poynting | named technical sense | named term kept per topic 4 |
| solid angle | στερεά γωνία | named technical sense | locks the canonical term |
| net gain | καθαρό κέρδος | total-minus-mismatch gain | not "πραγματικό κέρδος" (real-part gain) |
| viewer | Σημείο θέασης | observation direction / 3D view widget | not παρατηρητής (observer), not προεπισκόπηση (preview) |
| flow / flow direction | Ροή / κατεύθυνση ροής | patch/current flow | keeps flow distinct from geometry direction |
| total field | ολικό πεδίο | named technical sense | locks the canonical term |

### Color, tone, animation subsystem

| Concept | Term | Sense | Purpose |
|---|---|---|---|
| color | χρώμα | named technical sense | locks the canonical term |
| color projection | προβολή χρώματος | which quantity drives hue | locks the canonical term |
| hue | απόχρωση | color-wheel angle | locks the canonical term |
| brightness | φωτεινότητα | luminance channel | locks the canonical term |
| hue encoding | κωδικοποίηση απόχρωσης | internal enum | distinct from brightness encoding and from color projection |
| brightness encoding | κωδικοποίηση φωτεινότητας | internal enum | distinct from hue encoding and from color projection |
| color scale | κλίμακα χρώματος | magnitude-to-color scale | locks the canonical term |
| scale family / color tone | τόνος χρώματος | transfer-curve family (Power/Log/Asinh/μ-law/Reinhard/Sigmoid/Identity) | maps both source spellings to one term |
| palette / palette kind | τύπος παλέτας | palette-layout enum | distinct from scale family and color projection |
| ramp / gradient | διαβάθμιση | a palette kind / linear color strip | locks the canonical term |
| gamma | γάμα (γ) | power-law exponent | locks the canonical term |
| knee | Γόνατο | soft-knee bend point | UI slider label "Γόνατο:"; never "σημείο καμπής" |
| softening | εξομάλυνση | dynamic-range softening | locks the canonical term |
| compression | συμπίεση | dynamic-range compression | distinct from companding below |
| contrast | αντίθεση | named technical sense | locks the canonical term |
| dynamic range | δυναμικό εύρος | named technical sense | locks the canonical term |
| floor | κατώφλι | minimum/lower clamp, brightness/dB floor | not a room floor |
| envelope | περιβάλλουσα | magnitude/amplitude envelope | locks the canonical term |
| comet | Κομήτης | moving-crest overlay effect | distinct from geometry |
| overlay (noun) | επικάλυψη | an added visual layer | distinct from the verb "να επικαλύψει" |
| animate / animation | Κίνηση | motion effect or action | preserves the action/category sense |
| animated / static | Κίνηση / Στατικό | dynamic vs phase-invariant category | keeps the opposed categories distinct |
| projection | προβολή | color or geometry projection | locks the canonical term |
| scale (verb) | κλιμάκωση | to scale | locks the canonical term |
| scale (noun) | κλίμακα | a scale | locks the canonical term |
| wireframe | Συρματόπλεγμα | wire-mesh render mode | distinct from "Πλέγμα" used for the ground-radial grid |
| identity | ταυτοτική (μετατροπή) | no-op/passthrough transfer | distinct from μοναδιαία (unity, Smith-chart) |
| sentinel | τιμή-φρουρός | unreachable-case guard value | locks the canonical term |
| bins | κάδοι | discretization buckets | locks the canonical term |
| companding | Συμπίεση μ-νόμου | bounded log curve (μ-law) | distinct from generic "συμπίεση" above |
| tone mapping | τονική απεικόνιση | photographic tone-map | locks the canonical term |

### Render and compute

| Concept | Term | Sense | Purpose |
|---|---|---|---|
| renderer | Μηχανισμός απόδοσης | drawing backend | not "μηχανή rendering" (render engine, forbidden phrasing) |
| shader | shader | named technical sense | kept loanword, conditional (topic 4) |
| allocation (memory) | εκχώρηση (μνήμης) | named technical sense | locks the canonical term |
| managed allocator | διαχειριζόμενος εκχωρητής | allocator/report | locks the canonical term |
| thread | Νήμα | compute thread | no collision with wire (Αγωγός) in Greek |
| widget | γραφικό στοιχείο | UI element | locks the canonical term |
| validation | επικύρωση | the validation-tree feature | distinct from επαλήθευση (verification/checks) |
| batch mode | λειτουργία δέσμης | named technical sense | locks the canonical term |
| fork (process) | fork | process fork | kept verbatim, conditional loanword (topic 4) |
| deadlock | αδιέξοδο | named technical sense | locks the canonical term |
| notifier | ειδοποιητής | named technical sense | locks the canonical term |
| token | διακριτικό | expression-parser term | locks the canonical term |
| operand | τελεσταίος | expression-parser term | locks the canonical term |
| operator | τελεστής | expression-parser term | locks the canonical term |
| arity | αριθμός ορισμάτων | expression-parser term | locks the canonical term |
| override | παράκαμψη | supersede a value, SY symbol | not αντικατάσταση (overwrite) |
| swap | εναλλαγή | exchange | locks the canonical term |
| theme | θέμα | UI/color theme | Greek "θέμα" also means topic/subject; catalog uses only the UI-theme sense, so no ambiguity arises in practice (topic 22) |

### Metrics and miscellaneous

| Concept | Term | Sense | Purpose |
|---|---|---|---|
| noise | θόρυβος | electronic/thermal noise | not acoustic racket |
| noise temperature | θερμοκρασία θορύβου | named technical sense | locks the canonical term |
| efficiency | απόδοση | named technical sense | locks the canonical term |
| interpolation | παρεμβολή | named technical sense | locks the canonical term |
| mnemonic | μνημονικό | a card's code descriptor | not a memo/note |
| degrees / deg | μοίρες / (deg) | freestanding axis-prose vs parenthetical unit tag | tag treated like other unit tags (topic 4) |
| diameter | διάμετρος | canonical loanword | locked spelling, no competing native synonym |
| reflect (geometry) | κατοπτρισμός | geometry mirror operation | distinct from the two senses below |
| reflect (behavioral) | αντικατοπτρίζει | a control tracking another ("mirrors …") | distinct from the geometry and physics senses |
| reflect (physics) | ανάκλαση | physical reflection | distinct from the geometry and behavioral senses |
| default(s) | προεπιλογή / προεπιλογές | fallback value | locks the canonical term |
| normalize / normalization | κανονικοποίηση | named technical sense | translated, not transliterated |

No qualifier such as "ηλεκτρικό/ηλεκτρικά" is added to Ρεύμα/Φορτίο where
the electromagnetic-simulator program context already disambiguates the
sense.

## 11. Disambiguation policy

- Choose the correct technical sense for every ambiguous term; never leave
  the generic sense where a domain sense applies.
- Never add a qualifier absent from the source; program context already
  disambiguates (eg bare Ρεύμα/Φορτίο, topic 10).
- Add a qualifier only where the target term would otherwise be genuinely
  ambiguous within the same string.
- Accepted intra-domain homonym: Φορτίο serves both charge and load;
  declared and held file-wide (topic 10).
- Accepted cross-sense homonym: πεδίο serves both the EM field and a
  data/config field; program context disambiguates.
- Distinguish a gerund/process sense from a plain-noun sense of an
  overloaded source word via the standard -ση (process) vs -μα (result)
  nominal suffix pair where both readings occur.

## 12. Cross-catalog consistency

- One target term per concept; reuse the term already established in the
  domain-lexicon table (topic 10) everywhere that concept recurs.
- Resolve every Appendix C false-friend pair distinctly (also recorded in
  topic 10): πολικότητα ≠ πόλωση; μέτρο ≠ πλάτος; μέγιστη τιμή ≠ μέγιστο
  μέτρο; γείωση ≠ έδαφος; φορτίο (load) vs φορτίο (charge), held as a
  declared accepted homonym rather than split (topic 11); κέρδος ≠
  ενίσχυση (amplifier amplification) ≠ κέρδος/κερδοφορία (profit); ρεύμα ≠
  τρέχον (temporal); φορτίο ≠ χρέωση (billing); αγωγός (wire) ≠ καλώδιο
  (cable/cord); αγωγός (wire) ≠ νήμα (thread); διάγραμμα ακτινοβολίας ≠
  πρότυπο (template/design) and ≠ μακρινό πεδίο (far field region);
  διέγερση ≠ ενθουσιασμός (emotional excitement); δεσμός/κοιλία
  (node/antinode) ≠ a generic numeric μηδενικό (null/zero); τόνος
  χρώματος ≠ απόχρωση ≠ τύπος παλέτας ≠ προβολή χρώματος, four distinct
  chroma concepts; κομήτης ≠ γεωμετρία; ταυτοτική (identity) ≠ μοναδιαία
  (unity); μηχανισμός απόδοσης (renderer) ≠ "μηχανή rendering" (render
  engine); παράκαμψη (override) ≠ αντικατάσταση (overwrite); σημείο
  θέασης (viewer) ≠ παρατηρητής/προεπισκόπηση; κατοπτρισμός/αντικατοπτρίζει/
  ανάκλαση, three distinct reflect senses; δομή (structure) ≠ κατασκευή
  (construction); θέμα (theme) carries no distinct alternate spelling from
  topic/subject in Greek, held as a declared non-clashing homonym (topic
  10); επικύρωση (validation) ≠ επαλήθευση (verification); καθαρό κέρδος
  ≠ πραγματικό κέρδος; ισχύς (electrical power) ≠ δύναμη (power-law
  family name).
- Retain Patch, Tag, shader, and fork as Latin-script loanwords; use the
  locked Greek term in topic 10 for every other domain concept.
- Unify variant spellings to each canonical topic-10 form; use Patch rather
  than an inflected or transliterated variant.
- Consistency outranks general locale preference for period-decimal literals
  under topic 2 and retained Latin-script loanwords under topic 4.

## 13. Priority ordering

- Precedence chain, highest first: correct meaning, then interface
  convention (topic 7), then catalog consistency (topic 12), then
  disambiguation (topic 11), then locale numeral form (topic 2).
- Explicit override: catalog consistency overrides locale numeral
  preference for the period-decimal literal case (topic 2, topic 12).
- Explicit override: the never-translate roster (topic 4) overrides
  ordinary domain-lexicon translation for every listed token.

## 14. Grammatical number

- Use singular and plural grammatical number in nouns, adjectives,
  participles, and finite verbs according to the literal meaning.
- After an explicit literal count of one, use singular agreement; after
  every other explicit count, use plural agreement unless a fixed technical
  expression requires the genitive construction in topic 15.
- An uninflected no-plural-marker rule is not applicable because Greek
  marks grammatical number.

## 15. Grammatical agreement

- Adjectives and participles agree in gender, number, and case with their
  head noun, including a standalone label whose head noun is implied
  rather than stated.
- After a counted quantity, use the standard Greek partitive/genitive-
  plural construction where the source implies a count-of-items phrase.
- Keep a reflexive/middle-voice verb ending (eg an -εται form) where the
  Greek verb is inherently reflexive in a status/error sentence.
- A standalone label with no stated head noun defaults to neuter gender
  (topic 8).

## 16. Morphological derivation

- Naturalize a borrowed technical verb with the standard -άρω/-ίζω
  informatics suffix family; do not coin a non-standard suffix.
- Form a verbal noun with the standard -ση (process) or -μα (result)
  nominalizing suffix, matching the sense intended (topic 11).
- Prefer native Greek scientific vocabulary and genitive-chain compounding
  under topic 1 over a source-language calque; use a loanword only where
  topic 4 retains it or no established Greek technical term exists.

## 17. Preposition and sandhi selection

- Retain the movable-ν (κινούμενο ν) on a word like "την"/"στον"/"δεν"
  when the next word begins with a vowel or a plosive consonant; drop it
  otherwise, per standard Demotic orthography.
- No article-elision or vowel-contraction rule applies in this catalog's
  technical prose register; skip that sub-concept.

## 18. Card/record-label register

- Dialog and editor titles use the fixed designator form "Κάρτα <TAG>"
  (eg "Κάρτα GW").
- Running prose in messages uses the genitive running form "της κάρτας
  <TAG>" rather than the fixed-title form.
- Keep the generic noun "κάρτα" lowercase in every position; spell it in
  full, never abbreviated.
- Keep each register (title-form vs running-prose form) internally
  consistent; never cross-convert one into the other within the same
  string type.

## 19. Multi-paragraph and whitespace fidelity

- Preserve every source paragraph boundary at the same position and retain
  whether it is a single newline or a blank-line break.
- Omit clauses absent from the current source literal.
- Preserve trailing newlines, punctuation, and other semantic whitespace.
- Preserve meaningful source line breaks; introduce no line break solely
  for visual wrapping.
- Preserve complete meaning and natural wording without abbreviating or
  truncating for an assumed display width.

## 20. Current-source fidelity

- Derive each translation from the complete current source literal and its
  supplied context.
- Reuse inherited wording only where its full meaning agrees with the
  current source.
- Do not inherit translations among polarity and polarization, comet and
  geometry, or distinct color-projection, encoding, scale-family, and
  palette diagnostics; apply their separate topic-10 terms.

## 21. Script hygiene

- Use Greek code points for Greek prose; forbid Latin-script homoglyphs of
  Greek letters and mixed-script word forms outside a retained topic-4
  token.
- Translate plain foreign prose words; preserve genuine identifiers, units,
  proper names, and notation exactly as topic 4 requires.
- Permit a Greek-letter or Latin-letter adjacency only where the literal
  spelling of a retained technical token requires it.

## 22. Rule-file scope hygiene

- Keep only current decisions that can alter wording, Unicode characters,
  punctuation, capitalization, embedded tokens, mnemonic markers, or
  semantic whitespace in a translated literal.
- Omit catalog representation, serialization, headers, flags, workflow,
  rendering, fonts, shaping, layout, widget behavior, runtime formatting,
  sorting, search, display sizing, audit history, completion state, review
  metadata, provenance, and source citations.
- State each decision directly; include no implementation procedure,
  bibliography, correction history, or competing policy.

## 23. Section-disjointness declaration

Script mechanics in topics 1, 3, and 21 govern code points, direction, and
punctuation; retained-token policy in topics 4, 5, and 9 governs embedded
token spelling and mnemonic markers; phrasing in topics 6, 7, 18, and 19
governs casing, grammar, order, and semantic whitespace; address register in
topic 8 governs formality and person. Apply each concept on only its stated
axis.

## 24. Developer/debug-string policy

- Translate user-facing actions, labels, dialogs, tooltips, statuses, and
  errors in the string-type register defined by topics 7 and 8.
- Translate informational notices as concise impersonal declarative
  sentences.
- Translate developer-facing diagnostics as terse technical declarative
  sentences; diagnostic priority changes review order, not translation
  policy.
- Preserve every embedded identifier, function name, retained token, and
  format specifier in every translated family under topics 4 and 5.
- No subsystem-specific sibling family overrides this diagnostic register;
  this sub-concept is not applicable.
