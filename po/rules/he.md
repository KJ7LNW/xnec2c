# he translation rules

## 1. Script and orthography

- Govern Modern Hebrew for Israel, written in the Hebrew alphabet under standard unvocalized כתיב מלא, for professional electrical and radio-frequency engineers, antenna designers, and amateur-radio operators.
- Write right-to-left; keep retained Latin, Greek, and numeric technical tokens left-to-right, unmirrored, and without manual direction controls.
- Use letters א-ת with final forms ך ם ן ף ץ at word end; omit niqqud and cantillation marks from translated prose.
- Store Hebrew text in Unicode normalization form NFC; because prose is unvocalized, do not introduce combining marks or compatibility look-alikes.
- Use `׳` U+05F3 GERESH and `״` U+05F4 GERSHAYIM for Hebrew abbreviations; substitute neither ASCII apostrophe nor quotation mark, and add neither mark to a retained token.
- Hebrew has no textual joiner or productive ligature requirement; this feature is not applicable.
- Hebrew has no letter-case distinction; retain source case only in tokens governed by topics 4 and 6.
- Separate words with one space; use one space between Hebrew prose and an embedded retained or numeric token, except where native punctuation or a prefix binds them.
- Join a conventional compact technical compound with maqaf `־` U+05BE (eg מצב־אצווה); use a construct-state or spaced phrase where Hebrew syntax requires it (eg מישור הארקה).

## 2. Numerals in literals

- Use digits `0-9` for literal technical values; do not use Hebrew letter numerals.
- Use period `.` as the decimal separator and comma `,` only where the source literal groups thousands; add no grouping absent from the source.
- Preserve literal formulas, examples, fixed defaults, and named mathematical or standards constants exactly, including their digits and separators.
- Place a literal index after its Hebrew head noun (eg מקטע 3); keep hyphen `-` in a literal index range (eg 3-4) distinct from decimal period `.`.
- Express an ordinal with the natural Hebrew ordinal word when the source denotes order, not an index.

## 3. Punctuation and quotation

- Use `״...״` for quoted Hebrew prose; retain source ASCII quotes around an embedded technical token so its literal form remains recognizable.
- Use comma `,`, question mark `?`, and exclamation mark `!` unchanged; Hebrew has no separate native replacements.
- Put no space before colon, semicolon, comma, or terminal punctuation; put one space after them when text follows.
- Use `...` for ellipsis and preserve the source dash code point and spacing.
- End a full sentence with a full stop; omit a terminator from short labels, menu items, and fragments.
- Keep punctuation inside identifiers, format specifiers, filenames, mnemonics, formulas, and other retained technical runs verbatim.

## 4. Never-translate tokens

- Apply a zero-failure retained-token invariant: preserve every listed token byte-for-byte in source script and direction; never translate or transliterate it.
- Retain NEC2 card codes `GW GA GH EX LD FR RP GE EN SP SM SC NE NH NT TL GN GD GC GX GR GS GM GF CP CM SY XQ EK KH PQ PT` as round-trip file identifiers.
- Retain unit and figure-of-merit notation `MHz dBi Ω % K S/m deg VSWR Z Z0 F/B G/T`; retain every S-parameter as `S` followed by its source port indices.
- Retain file suffixes `.nec .csv .s1p .s2p .png` and every literal filename containing them.
- Retain every printf-style format specifier, including its flags, width, precision, length, conversion letter, and positional index; topic 5 governs integrity.
- Retain embedded function names, variable names, configuration keys, and other code identifiers in diagnostic text.
- Retain physical and mathematical symbols `E H Z R X φ θ` and any source subscript or exponent attached to them; a Hebrew look-alike changes the notation.
- Retain product, library, toolkit, chart, and tool proper names in their source spelling, including `xnec2c`, `NEC2`, `GTK`, and `Cairo`.
- Retain named curve and algorithm terms `Log Asinh μ-law Reinhard Sigmoid Identity`; translate descriptive transfer-family `Power` as חזקה to separate it from electrical הספק.
- Translate conditional geometry terms `segment`, `patch`, `tag`, and `card` as מקטע, טלאי, תג, and כרטיס; retain process term `fork` in source form.
- Apply topic 1 directionality to every retained token inside Hebrew prose.

## 5. Format-specifier integrity

- Apply a zero-failure format invariant: preserve exactly the source set, spelling, and multiplicity of format specifiers in every translation.
- Keep source order by default; use positional order only where the source specifiers themselves are positional.
- Restructure Hebrew wording around fixed specifier positions when natural word order differs; never move a non-positional specifier across another specifier.
- Preserve every digit inside a format specifier without localization.

## 6. Capitalization and title-case

- Hebrew has no case distinction: write labels, menu items, and titles in normal Hebrew orthography without imitating source title case.
- Retain exact source case in acronyms, proper names, and other topic-4 tokens (eg `VSWR`, `xnec2c`, `GTK`).
- Retain uppercase axis letters `X Y Z` and lowercase mathematical or coordinate variables `x y` exactly as supplied.
- In a coordinated Hebrew option name, apply ordinary orthography to every element; capitalization adds no distinction.
- Write generic כרטיס identically in every position; retain acronym and proper-name case beside it.

## 7. Interface register by string type

- Commands, buttons, and menu actions use the shortest natural action noun or infinitive, omit the subject, place complements after the action, and avoid a second-person imperative (eg שמירה).
- Field labels use a concise noun or construct-state phrase followed by the source colon; place the governing noun before its complement (eg קוטר תיל:).
- Dialogs and confirmations use complete impersonal sentences with predicate before new detail; confirmations use `האם` plus an infinitive question.
- Tooltips use complete declarative sentences in known-to-new information order; preserve the stated reason that an unavailable control is disabled.
- Status and error messages use terse impersonal declarative clauses, omit an agent unless technically relevant, and state event before diagnostic detail.
- Name user-visible domain entities as a Hebrew head noun or construct-state phrase followed by retained identifiers (eg כרטיס GW); topic 8 governs personal and proper names.
- Prefer compact compounds or construct-state phrases over mechanically matching source length; preserve every semantic component and use no unnatural abbreviation.
- Topic 24 governs developer and diagnostic string families.

## 8. Formality and address

- Modern Hebrew distinguishes gender and number in second-person forms but has no separate formal pronoun; realize professional register through impersonal construction and technical word choice.
- Omit direct address where natural; use nominal or infinitive commands and impersonal dialogs to avoid unnecessary gender marking.
- When direct address is semantically required, recast to a gender-neutral plural or impersonal form; do not default to masculine singular.
- Use no honorific in interface text; this feature is not applicable unless the source literal contains one.
- Preserve a person's supplied name order and spelling; do not add an honorific or alter the name.
- Use no first-person interface voice unless the source identifies an actual speaker; use third-person or subjectless declarative form for system state.
- Keep commands nominal or infinitive and confirmations as `האם` plus infinitive; do not exchange their registers.
- Avoid slang, casual particles, commercial language, archaic forms, and ceremonial over-formality.

## 9. Accelerator/hotkey mnemonics

- Place GTK marker `_` immediately before a typable non-final Hebrew letter within the translated label (eg `_קיטוב`).
- Select the mnemonic letter from the translated term, never from a transliteration of the source letter.
- Hebrew requires no separate parenthetical mnemonic letter; this presentation is not applicable.
- Add a marker only where the source literal contains one; never invent a mnemonic.
- Avoid final letters ך ם ן ף ץ and letters unavailable on a standard Hebrew keyboard layout.
- Keep the marker inside the literal without changing the translated word's spelling or punctuation.

## 10. Domain lexicon

Use each source concept's single Hebrew term; the sense and guard state why the mapping is fixed.

### Electrical primitives

| Concept | Hebrew | Sense and guard |
|---|---|---|
| current | זרם | electrical current; excludes temporal נוכחי and עדכני |
| charge | מטען | electrical charge; excludes billing חיוב and cargo |
| voltage | מתח | electric potential; fixes the engineering sense |
| power (electrical) | הספק | watts and power flow; excludes transfer-family חזקה |
| impedance | עכבה | complex Z; distinct from resistance and reactance |
| resistance | התנגדות | real part of Z; distinct from impedance and load |
| reactance | ריאקטנס | imaginary part of Z; distinct from resistance |
| inductance | השראות | magnetic energy-storage quantity; fixes the circuit sense |
| capacitance | קיבול | electric energy-storage quantity; fixes the circuit sense |
| conductivity | מוליכות | material S/m; uses the established native term |
| admittance | אדמיטנס | admittance-matrix quantity; distinct from impedance |
| load | עומס | LD-card impedance load; excludes weight and charge |
| gain | הגבר | antenna directivity ratio; excludes amplification and profit |
| excitation | עירור | electromagnetic input; excludes emotional excitement |
| feedpoint | נקודת הזנה | antenna feed location; fixes the electrical sense |
| port | פורט | excitation or S-parameter port; excludes a generic opening |
| radials | רדיאלים | ground-plane radial wires as a noun; distinct from the adjective רדיאלי |

### Ground and earth

| Concept | Hebrew | Sense and guard |
|---|---|---|
| ground | הארקה | radio-frequency reference; excludes physical קרקע |
| ground plane | מישור הארקה | plane form of the same electrical-reference concept |
| earth (physical medium) | קרקע | terrain or noise medium; distinct from electrical ground |
| ground wave | גל קרקעי | propagation mode; distinct from the reference plane |

### Geometry primitives

| Concept | Hebrew | Sense and guard |
|---|---|---|
| wire | תיל | thin GW conductor; excludes cable and compute thread |
| segment | מקטע | NEC2 geometry subdivision; fixed translated form |
| patch | טלאי | SP/SM surface element; fixed translated form |
| tag | תג | geometry identifier; excludes UI label and card |
| card | כרטיס | NEC2 input record; topic 18 fixes its registers |
| kernel | ליבה | thin-wire integral kernel; excludes an operating-system kernel |
| cliff | מצוק | two-medium boundary type; excludes fracture |
| structure | מבנה | antenna-model geometry; excludes construction |
| model | מודל | NEC or noise-temperature model; fixes catalog usage |
| geometry | גאומטריה | modeled spatial structure; excludes comet overlay |
| crossed | מוצלב | reversed transmission-line conductors; excludes cut |

### Field, pattern, and viewer

| Concept | Hebrew | Sense and guard |
|---|---|---|
| field (electromagnetic) | שדה | E/H or near/total field; context separates a data field |
| near field | שדה קרוב | near spatial region; paired with far field |
| far field | שדה רחוק | far spatial region; distinct from radiation pattern |
| far-field contribution | תרומת שדה רחוק | directional contribution; excludes near-field animation |
| radiation | קרינה | electromagnetic emission; fixes the physical sense |
| radiation pattern | תבנית קרינה | plotted directional response; excludes generic template |
| gain pattern | תבנית הגבר | radiation pattern of gain; preserves the gain sense |
| polarization | קיטוב | wave or antenna orientation; distinct from polarity |
| polarity | קוטביות | positive or negative sign; distinct from polarization |
| phase | פאזה | oscillatory phase; fixes the engineering sense |
| reference phase | פאזת ייחוס | phase reference; preserves the qualifier |
| frequency | תדר | oscillation rate; fixes technical usage |
| wave | גל | propagating or standing wave; fixes physical usage |
| wavelength | אורך גל | spatial period; distinct from wave itself |
| standing wave | גל עומד | stationary-wave member of the opposed pair |
| traveling wave | גל נע | propagating-wave member of the opposed pair |
| node | צומת | standing-wave zero; distinct from numeric zero |
| antinode | אנטי־צומת | standing-wave maximum; distinct from generic peak |
| crest | פסגה | instantaneous wave apex; distinct from antinode and peak value |
| magnitude | גודל | modulus or scalar size; distinct from amplitude |
| amplitude | משרעת | oscillating peak; distinct from magnitude |
| peak value | ערך שיא | value option; distinct from peak magnitude |
| peak magnitude | גודל שיא | magnitude option; distinct from peak value |
| instantaneous | רגעי | projection mode; add `(φ=0)` only when source carries it |
| Poynting vector | וקטור פוינטינג | electromagnetic power-flow vector; fixes the named quantity |
| solid angle | זווית מרחבית | steradian geometry; fixes the mathematical sense |
| net gain | הגבר נטו | total minus mismatch; excludes real-part gain |
| viewer | צופה | observation direction or 3D view; excludes speaker and preview |
| flow | זרימה | current or patch flow; fixes the directional process |
| flow direction | כיוון זרימה | direction of flow; preserves the qualifier |
| total field | שדה כולל | combined field quantity; preserves totality |

### Color, tone, and animation

| Concept | Hebrew | Sense and guard |
|---|---|---|
| color | צבע | visible color property; parent chroma concept |
| color projection | הטלת צבע | quantity driving hue; distinct from palette and tone family |
| hue | גוון | color-wheel angle; distinct from color projection |
| brightness | בהירות | luminance channel; distinct from hue |
| hue encoding | קידוד גוון | hue enum; distinct from color projection |
| brightness encoding | קידוד בהירות | brightness enum; distinct from color projection |
| color scale | קנה מידת צבע | magnitude-to-color mapping; distinct from scale family |
| scale family / color tone | משפחת קנה מידה | transfer-curve family; collapses both source spellings to one term |
| palette | פלטה | color collection; distinct from palette kind |
| palette kind | סוג פלטה | palette-layout enum; distinct from scale family |
| ramp | מדרג | palette-kind strip; distinct from gradient operation |
| gradient | גרדיאנט | linear color transition; distinct from ramp kind |
| gamma | גמא | power-law exponent; fixes the mathematical sense |
| knee | נקודת כיפוף | soft-knee bend point; excludes anatomical sense |
| softening | ריכוך | dynamic-range smoothing; fixes tone-control usage |
| compression | דחיסה | dynamic-range reduction; excludes file compression |
| contrast | ניגודיות | tonal separation; fixes image-control usage |
| dynamic range | טווח דינמי | span of represented levels; fixes signal usage |
| floor | סף תחתון | minimum clamp; excludes a room floor |
| envelope | מעטפת | amplitude or magnitude envelope; fixes signal usage |
| comet | שביט | moving-crest overlay; excludes geometry |
| overlay (noun) | שכבת־על | added visual layer; distinct from verb להניח מעל |
| animate | להנפיש | animation action; distinct from category adjective |
| animation | הנפשה | animation noun; distinct from action form |
| animated | מונפש | changing category adjective; paired with static |
| static | סטטי | phase-invariant category adjective; paired with animated |
| projection | הטלה | color or geometry projection; fixes technical usage |
| scale (verb) | לשנות קנה מידה | scaling action; distinct from noun |
| scale (noun) | קנה מידה | scale object; distinct from action |
| wireframe | רשת תילים | wire-mesh display mode; excludes antenna wire geometry |
| identity | זהות | pass-through transfer; distinct from unity |
| sentinel | ערך שומר | unreachable-case guard value; fixes software sense |
| bins | תאים | discretization buckets; excludes storage containers |
| companding | קומפנדינג | bounded logarithmic curve; fixes signal-processing sense |
| tone mapping | מיפוי גוונים | photographic transfer process; distinct from scale family |

### Render and compute

| Concept | Hebrew | Sense and guard |
|---|---|---|
| renderer | מציג | drawing backend; excludes render engine wording |
| shader | שיידר | graphics shader; fixes the compute sense |
| allocation (memory) | הקצאה | memory allocation; excludes general assignment |
| managed allocator | מקצה מנוהל | managed memory allocator or report; preserves management sense |
| thread | תהליכון | compute thread; distinct from wire תיל |
| widget | רכיב | interface element; fixes software usage |
| validation | אימות | validation-tree feature; distinct from verification בדיקה |
| batch mode | מצב־אצווה | noninteractive batch mode; fixes software usage |
| fork (process) | fork | retained process operation; excludes utensil sense |
| deadlock | מבוי סתום | thread deadlock; fixes concurrency usage |
| notifier | מתריע | notification component; fixes software role |
| token | אסימון | parser token; excludes authentication token |
| operand | אופרנד | parser operand; distinct from operator |
| operator | אופרטור | parser operator; distinct from operand |
| arity | מספר אופרנדים | operand count; excludes unrelated דרגה senses |
| override | עקיפה | superseding a value; distinct from overwrite כתיבה מחדש |
| swap | החלפה | exchange of two values; fixes operation sense |
| theme | ערכת נושא | interface or color theme; distinct from topic נושא |

### Metrics and miscellaneous

| Concept | Hebrew | Sense and guard |
|---|---|---|
| noise | רעש | electronic noise; excludes acoustic disturbance |
| noise temperature | טמפרטורת רעש | thermal noise metric; preserves the compound sense |
| efficiency | יעילות | engineering ratio; fixes metric usage |
| interpolation | אינטרפולציה | numerical interpolation; fixes computational usage |
| mnemonic | קוד כרטיס | card-code descriptor; excludes memo or note |
| degrees | מעלות | freestanding angle word; distinct from retained `(deg)` tag |
| deg | `deg` | parenthetical unit tag retained by topic 4 |
| diameter | קוטר | canonical geometry term; excludes competing synonyms |
| reflect (geometry mirror) | שיקוף | geometry operation; distinct from tracking and reflection |
| reflect (behavioral tracking) | עוקב אחר | one control follows another; distinct from mirror operation |
| reflect (physics) | החזרה | physical reflection; distinct from the other senses |
| default(s) | ברירת מחדל | fallback value; fixes interface usage |
| normalize | לנרמל | normalization action; uses the established technical verb |
| normalization | נירמול | normalization noun; paired with the established verb |

## 11. Disambiguation policy

- Select topic 10's technical sense for every ambiguous source concept; program context supplies the sense without an added qualifier.
- Add a qualifier absent from the source only when the chosen Hebrew term remains ambiguous in its actual literal; otherwise preserve source specificity.
- Accept שדה for both electromagnetic and data fields because domain context separates them; introduce no synonym solely to remove this homonym.
- Avoid locative collisions through the fixed compounds נקודת הזנה, מישור הארקה, and כיוון זרימה.
- Distinguish action from noun: לשנות קנה מידה versus קנה מידה, להניח מעל versus שכבת־על, and לנרמל versus נירמול.

## 12. Cross-catalog consistency

- Use one topic-10 term per concept throughout the catalog; unify minority spellings to standard unvocalized כתיב מלא.
- Keep every false-friend family distinct: קוטביות/kיטוב; גודל/משרעת; ערך שיא/גודל שיא; הארקה/קרקע; עומס/מטען; הגבר/הגברה/רווח; זרם/נוכחי/עדכני; מטען/חיוב; תיל/כבל/תהליכון; תבנית קרינה/תבנית/שדה רחוק; עירור/התרגשות; צומת/אנטי־צומת/אפס; משפחת קנה מידה/גוון/סוג פלטה/הטלת צבע; שביט/גאומטריה; זהות/יחידה; מציג/מנוע רינדור; עקיפה/כתיבה מחדש; צופה/דובר/תצוגה מקדימה; שיקוף/עוקב אחר/החזרה; מבנה/בנייה; ערכת נושא/נושא; אימות/בדיקה; הגבר נטו/הגבר של חלק ממשי; הספק/חזקה.
- Keep the topic-4 loanword or native-term ruling file-wide; introduce no alternate spelling or synonym.
- Let catalog consistency outrank a local synonym preference where topic 10 fixes a term, including הגבר in every gain compound.

## 13. Priority ordering

- Apply this precedence: correct technical meaning, interface convention, catalog consistency, disambiguation, then literal numeral form.
- Correct technical meaning overrides local brevity: retain תבנית קרינה rather than shortening it to ambiguous תבנית.
- Interface convention overrides source syntax: use Hebrew construct-state order while preserving every source concept.
- Catalog consistency overrides a locally smoother synonym: retain the fixed topic-10 term in compounds.

## 14. Grammatical number

- Hebrew uses singular and plural noun, adjective, participle, and verb forms; select each form by the referent's grammatical number.
- After an explicit literal count, use singular after one and plural after every other cardinal (eg מקטע אחד, 2 מקטעים); preserve natural construct-state number forms.

## 15. Grammatical agreement

- Make adjectives and participles agree in gender and number with their head noun, including standalone labels with an implied head (eg לינארי for an implied masculine גל).
- After a count, keep noun and modifier agreement in the number selected by topic 14; Hebrew has no separate partitive case.
- Retain a reflexive verb form where the source sense is reflexive (eg מתעדכן for self-updating).
- Resolve a standalone label with no stated referent to masculine singular; use the explicit referent's gender when context supplies one.

## 16. Morphological derivation

- Derive borrowed technical verbs through an established Hebrew verb pattern (eg לנרמל); omit ad-hoc mixed-script verbs.
- Form verbal nouns through the corresponding Hebrew action-noun pattern (eg נירמול).
- Prefer an established native root and affix over transliteration; retain only proper and technical names assigned to topic 4.
- Form technical compounds by topic 1's maqaf-or-spaced rule.

## 17. Preposition and sandhi selection

- Apply the standard unvocalized form of conjunction ו before all following sounds; niqqud-dependent variants do not apply under topic 1.
- Contract the definite article after prefixed ב, ל, and כ into the conventional unvocalized written form (eg בהגבר, לתדר, כמודל).

## 18. Card/record-label register

- Use כרטיס followed by the retained code in dialog and editor titles (eg כרטיס GW).
- Use construct or definite prose form in messages (eg כרטיס ה-GW).
- Use generic כרטיס in short labels and כרטיס קלט NEC2 on first mention where the literal requires the full sense.
- Keep title and running-prose forms distinct and internally consistent.

## 19. Multi-paragraph and whitespace fidelity

- Mirror source paragraph breaks at the same positions, preserving the distinction between a blank line and a single line break.
- Drop a trailing clause absent from the current source rather than retaining inherited text.
- Preserve source trailing newlines and punctuation exactly.
- Preserve semantic line breaks; add no line break for visual wrapping.
- Preserve complete meaning and natural wording; never truncate or abbreviate for an assumed display limit.

## 20. Current-source fidelity

- Derive every translation from the current source literal and its supplied context.
- Reuse an inherited translation only when its complete meaning agrees with the current source and context.
- Treat these inherited pairings as unsafe without re-derivation: שביט/גאומטריה, הגבר/רווח, תיל/כבל/תהליכון, הארקה/קרקע, גודל/משרעת, and אפס/שיא.

## 21. Script hygiene

- Apply a zero-failure script invariant: Hebrew prose contains only Hebrew-script text plus retained tokens whose literal spelling requires another script.
- Translate plain source-language words; retain only identifiers, units, symbols, names, and other tokens assigned to topic 4.
- Hebrew and retained technical scripts have no shared homoglyph class requiring an additional language-specific prohibition; this feature is not applicable.

## 22. Rule-file scope hygiene

- Keep only current decisions that can alter a translated literal's wording, Unicode characters, punctuation, capitalization, embedded tokens, mnemonic marker, or semantic whitespace.
- Omit catalog representation, serialization, headers, flags, workflow, rendering, fonts, shaping, layout, widget behavior, runtime formatting, sorting, search, display sizing, audit history, completion state, review metadata, provenance, and citations.
- State each decision directly; include no procedure, bibliography, correction history, or alternative policy.

## 23. Section-disjointness declaration

- Script mechanics (§1, §4, §6, §9, §21) govern character and retained-token form only.
- Phrasing and structure (§2, §3, §5, §7, §10-§13, §15-§20, §24) govern wording, sentence form, whitespace, and lexicon only.
- Address register (§8) governs person, gender avoidance, and formality only.
- Assign each decision to exactly one axis.

## 24. Developer/debug-string policy

- Translate user-facing strings fully; translate informational status and diagnostic strings in terse technical Hebrew; translate developer-facing strings in the same register unless an established subsystem family consistently retains its source-language prose.
- Treat priority as review order only; never leave an applicable user-facing or informational literal untranslated because of priority.
- Preserve every embedded identifier, function name, format specifier, and topic-4 retained token verbatim in every string family.
- Keep token preservation independent of linguistic priority and surrounding-string translation policy.
- Render the `BUG:` diagnostic prefix as "תקלה:", held distinct from the "שגיאה:" prefix that carries ordinary failures; the two severities never share a word.
- Keep an identifier-plus-value diagnostic pair contiguous (eg `sweep_state=%d`); a qualifier precedes the whole pair and never stands between the identifier and its `=`.
