# bn translation rules

## 1. Script and orthography
- Use standard Bengali for Bangladesh, written in the Bengali abugida, for professional electrical and radio-frequency engineers, antenna designers, and amateur-radio operators.
- Mandatory diacritics: vowel matras and hasant (্) for conjuncts; nukta letters (ড় ঢ় য়) keep their nukta form, never fold to the base letter (ড ঢ য).
- Use precomposed Unicode sequences where the standard defines one (eg খণ্ড-ত ৎ as its own code point); otherwise compose via combining vowel signs and hasant, never a visual look-alike substitute.
- Use anusvara (ং) for the nasal mark, never the visually similar ঙ (velar nasal letter) as a substitute.
- Conjuncts (যুক্তাক্ষর) form via hasant-joined consonant clusters; keep standard conjunct forms, no manual spacing inside a conjunct.
- Directionality: left-to-right; embedded LTR technical tokens (units, mnemonics, numerals) need no direction override.
- No letter-case distinction: casing rules (topic 6) reduce to mnemonic-letter selection (topic 9) only.
- Follow modern standard Bengali orthography (Bangla Academy convention); no dialectal or historical spelling variants.
- Insert a space between Bengali text and an embedded Latin/numeric/unit token; no space is required Bengali-word-to-Bengali-word beyond normal word spacing.
- Compound technical terms formed from a transliterated head noun and a Bengali suffix fuse without a space (eg ordinal 1ম, genitive রঙের); multi-word technical phrases stay spaced (eg বিকিরণ প্যাটার্ন).

## 2. Numerals in literals
- Use Western digits 0-9 for every technical value and unit figure in Bengali prose; never substitute Bengali digits ০-৯.
- Use period `.` as the decimal separator and comma `,` for thousands grouping; never use South Asian lakh/crore grouping in a technical literal.
- Preserve the source numeral form in formulas, examples, fixed defaults, and named mathematical or standards constants.
- Fuse a Western digit with a Bengali ordinal suffix (eg 1ম); use the source hyphen or space for an index boundary, never a decimal point.

## 3. Punctuation and quotation
- Bengali technical prose uses source straight quotation marks for quoted text and embedded technical tokens; keep one style catalog-wide.
- Use দাঁড়ি `।` after a complete Bengali declarative sentence and the source question or exclamation mark after the corresponding sentence mood; omit a terminator from short labels and fragments unless the source supplies a colon.
- Use source comma, colon, semicolon, question mark, and exclamation mark; place no space before them or before a terminal mark.
- Preserve source `...` ellipses and hyphen, en-dash, or em-dash characters exactly.
- Preserve punctuation inside format specifiers, filenames, formulas, and other embedded technical runs in source form.

## 4. Never-translate tokens
Every retained token has a zero-failure invariant: preserve its exact source spelling, case, punctuation, digits, and direction; never translate or transliterate it.
- Retain NEC2 card mnemonics `GW GA GH EX LD FR RP GE EN SP SM SC NE NH NT TL GN GD GC GX GR GS GM GF CP CM SY XQ EK KH PQ PT` as file-format identifiers.
- Retain all source unit symbols and parenthetical unit tags, including `(deg)`, as standard electrical-engineering notation.
- Retain `VSWR`, S-parameter notation, `Z`, `Z0`, front/back-ratio tokens, and gain/temperature tokens as radio-frequency notation.
- Retain every file extension, literal filename, and printf-style format specifier, including `%s`, `%d`, `%f`, `%c`, `%%`, and positional forms such as `%1$s`.
- Retain embedded function names, variable names, configuration keys, and other code identifiers in developer strings.
- Retain physical and mathematical symbol letters, including `E` and `H`, in source form; a Bengali look-alike changes the notation.
- Retain product, library, toolkit, chart, and tool proper names in their supplied source form.
- Retain surnamed and standardized transfer-function or algorithm names; translate descriptive `Power` as পাওয়ার only in the scale-family sense fixed by topic 10.
- Translate the conditional loanwords `segment`, `patch`, `tag`, and process `fork` by the locked forms সেগমেন্ট, প্যাচ, ট্যাগ, and ফর্ক; never retain their Latin forms.
- Keep retained tokens left-to-right under topic 1 and add no manual direction-control character.

## 5. Format-specifier integrity
- Preserve exactly the same format-specifier set as the source; omission, addition, translation, or character change is forbidden.
- Keep source order by default; reorder only through source-compatible positional specifiers such as `%1$s` when Bengali grammar requires it.
- Without positional forms, restructure Bengali wording around each fixed specifier position rather than moving the specifier.
- Preserve every digit inside a format specifier in source form.

## 6. Capitalization and title-case
- Bengali has no letter-case distinction; this topic carries no casing rule.
- The only related decision is mnemonic-letter selection, covered under topic 9.

## 7. Interface register by string type
Treat each string family as written Bengali interface grammar; preserve all source meaning in the shortest complete natural form.
- Commands, buttons, and menu actions use a subjectless formal imperative ending in -উন; place the object before the final verb (eg ফাইল নির্বাচন করুন), never use an infinitive or deverbal noun.
- Field labels use a compact noun phrase in head-final order plus the source colon; add no subject or verb (eg ফ্রিকোয়েন্সি:).
- Dialogs and confirmations use full formal sentences in subject-object-verb order; omit the subject when recoverable, place the finite verb last, and retain the source question or statement mood.
- Tooltips use complete declarative or instructional clauses in normal head-final order; when the source explains a disabled control, preserve the reason before the resulting unavailability.
- Status and error messages use terse impersonal declaratives with the affected entity before the final predicate; add no first- or second-person subject.
- User-visible domain entities use the topic-10 term followed by its qualifier or grammatical suffix as Bengali requires; retain proper names under topics 4 and 8.
- Fuse suffixal compounds and space multiword phrases under topic 1; never truncate meaning, invent an abbreviation, or imitate source length.
- Developer and debug strings follow topic 24.

## 8. Formality and address
- Use formal আপনি address and formal verb endings in user-directed text; omit তুমি, তুই, casual, slang, commercial, archaic সাধু ভাষা, and ceremonially over-formal wording.
- Realize formality primarily through the verb ending -উন; omit an explicit আপনি subject when the ending and context identify the addressee.
- Commands use terse subjectless formal imperatives; dialogs and confirmations use full formal sentences under topic 7.
- Confirmation dialogs use a complete formal question or statement matching the source mood and ending with the applicable topic-3 terminator.
- Bengali has no grammatical gender or gendered address; gender-default and gender-avoidance rules are not applicable.
- Use inclusive role or person nouns without added sex, gender, number, or animacy marking absent from the source.
- Bengali interface address requires no honorific title beyond formal আপনি and its verb form; added honorifics are not applicable.
- Preserve a person's supplied name order and spelling; Bengali interface grammar imposes no reordered personal-name form.

## 9. Accelerator/hotkey mnemonics
- Preserve the source mnemonic marker `_` exactly once and place it immediately before the selected mnemonic character in the translation.
- Select a character associated with the translated Bengali term, not a transliteration of the source mnemonic.
- When a separate typable key is required, append one parenthetical Latin mnemonic (eg সংরক্ষণ (_S)); keep that key easy to type without a dead-key sequence.
- Add no mnemonic marker when the source literal has none.

## 10. Domain lexicon
Reuse each mapped term catalog-wide; never coin an alternate synonym for a concept already mapped here.

| Concept | Term | Sense | Purpose/hazard guarded |
|---|---|---|---|
| current | কারেন্ট | electrical current, A | distinct from temporal "recent" |
| charge | চার্জ | electrical charge, C | distinct from billing/fee |
| voltage | ভোল্টেজ | electric potential | preserves this concept's locked technical sense |
| power (electrical) | বৈদ্যুতিক ক্ষমতা | radiated/dissipated watts and power flow | distinct from scale-family পাওয়ার and a mathematical power law |
| impedance | ইম্পিড্যান্স | complex Z | distinct from resistance/reactance |
| resistance | রেজিস্ট্যান্স | real part of Z | distinct from impedance/load |
| reactance | রিঅ্যাক্ট্যান্স | imaginary part of Z | preserves this concept's locked technical sense |
| inductance | ইন্ডাক্ট্যান্স | technical sense named by the source concept | prevents substitution by a neighboring catalog concept |
| capacitance | ক্যাপাসিট্যান্স | technical sense named by the source concept | prevents substitution by a neighboring catalog concept |
| conductivity | পরিবাহিতা | material S/m, native term | preserves this concept's locked technical sense |
| admittance | অ্যাডমিট্যান্স | admittance matrix | distinct from impedance |
| load | লোড | LD-card impedance load | distinct from charge; not physical weight |
| gain | গেইন | antenna directivity ratio, dB | not profit, not amplifier gain |
| excitation | এক্সাইটেশন | EM energy input/source | not emotional excitement |
| feedpoint | ফিড পয়েন্ট | antenna feed point | preserves this concept's locked technical sense |
| port | পোর্ট | excitation/S-param port | preserves this concept's locked technical sense |
| radials | রেডিয়াল | ground-plane radial wires, noun | context distinguishes noun from adjective "radial" |
| ground / ground plane | গ্রাউন্ড | RF reference plane, GN/GD cards | one term across all ground sub-senses; not soil |
| earth (physical medium) | মাটি | terrain/noise-model earth | distinct from গ্রাউন্ড |
| ground wave | গ্রাউন্ড ওয়েভ | propagation term | distinct from গ্রাউন্ড reference |
| wire | তার | thin conductor / GW element | not cable/cord; one term across all wire uses |
| segment | সেগমেন্ট | NEC2 geometry subdivision | preserves this concept's locked technical sense |
| patch | প্যাচ | NEC2 surface patch (SP/SM) | transliterated, one decision file-wide |
| tag | ট্যাগ | NEC2 geometry identifier | not a UI label or card |
| card | কার্ড | NEC2 input record | register in topic 18 |
| kernel | কার্নেল | integral-equation/thin-wire kernel | not an OS kernel |
| cliff | ক্লিফ | two-medium ground-boundary type | not a fracture/break |
| structure | স্ট্রাকচার | antenna model geometry | distinct from নির্মাণ "construction" |
| model | মডেল | NEC or noise-temperature model | preserves this concept's locked technical sense |
| geometry | জ্যামিতি | model geometry, native term | preserves this concept's locked technical sense |
| crossed | ক্রসড | transmission-line conductors crossed | not cut/severed |
| field (EM) | ক্ষেত্র | near/total/E/H field | distinct from a data/config field |
| near field / far field | নিকট ক্ষেত্র / দূর-ক্ষেত্র | opposed spatial regions | keep symmetric pairing |
| far-field contribution | দূর-ক্ষেত্র অবদান | per-direction contribution | not near-field animation |
| radiation | বিকিরণ | radiated emission, native term | never রেডিয়েশন |
| radiation pattern | বিকিরণ প্যাটার্ন | plotted directional response | not template/design |
| gain pattern | গেইন প্যাটার্ন | the gain radiation pattern | preserves this concept's locked technical sense |
| polarization | পোলারাইজেশন | antenna/wave field orientation | distinct from polarity |
| polarity | পোলারিটি | sign of a quantity | false friend of polarization |
| phase | দশা | technical sense named by the source concept | prevents substitution by a neighboring catalog concept |
| reference phase | রেফারেন্স দশা | technical sense named by the source concept | prevents substitution by a neighboring catalog concept |
| frequency | ফ্রিকোয়েন্সি | technical sense named by the source concept | prevents substitution by a neighboring catalog concept |
| wave / wavelength | তরঙ্গ / তরঙ্গদৈর্ঘ্য | native terms | preserves this concept's locked technical sense |
| standing wave / traveling wave | স্থির তরঙ্গ / চলমান তরঙ্গ | opposed pair | preserves this concept's locked technical sense |
| node / antinode | নোড / অ্যান্টিনোড | standing-wave zero/max | also carries null/peak overlay sense |
| crest | শীর্ষ | instantaneous wave apex, comet-head | distinct from a curve/step peak |
| magnitude | ম্যাগনিচিউড | modulus of a quantity | distinct from amplitude |
| amplitude | অ্যামপ্লিচিউড | oscillating-quantity peak | distinct from magnitude |
| peak value / peak magnitude | পিক ভ্যালু / পিক ম্যাগনিচিউড | two distinct UI options | never collapse to one label |
| instantaneous | তাৎক্ষণিক | projection mode, native term | add (φ=0) qualifier only when source carries it |
| Poynting vector | পয়েন্টিং ভেক্টর | technical sense named by the source concept | prevents substitution by a neighboring catalog concept |
| solid angle | ঘন কোণ | native term | preserves this concept's locked technical sense |
| net gain | নেট গেইন | total-minus-mismatch gain | not "real (part) gain" |
| viewer | ভিউয়ার | observation direction / 3D view widget | not observer/speaker/preview |
| flow / flow direction | প্রবাহ / প্রবাহের দিক | patch/current flow | flow ≠ current itself, which stays কারেন্ট |
| total field | মোট ক্ষেত্র | technical sense named by the source concept | prevents substitution by a neighboring catalog concept |
| color | রং | bare noun with anusvara; genitive রঙের | preserves this concept's locked technical sense |
| color projection | রঙের প্রক্ষেপণ | which quantity drives hue | preserves this concept's locked technical sense |
| hue | হিউ | color-wheel angle | preserves this concept's locked technical sense |
| brightness | ব্রাইটনেস | luminance channel | preserves this concept's locked technical sense |
| hue encoding / brightness encoding | হিউ এনকোডিং / ব্রাইটনেস এনকোডিং | distinct internal enums | neither collapses to color projection |
| color scale | কালার স্কেল | magnitude-to-color scale | preserves this concept's locked technical sense |
| scale family / color tone | স্কেল ফ্যামিলি | transfer-curve family (Power/Log/Asinh/μ-law/Reinhard/Sigmoid/Identity) | one concept, one term for both source spellings |
| palette / palette kind | প্যালেট / প্যালেট কাইন্ড | palette-layout enum | distinct from scale family and color projection |
| ramp / gradient | র‍্যাম্প / গ্র্যাডিয়েন্ট | palette kind / linear strip | preserves this concept's locked technical sense |
| gamma | গামা | power-law exponent | preserves this concept's locked technical sense |
| knee | নী পয়েন্ট | soft-knee bend point | preserves this concept's locked technical sense |
| softening | সফটেনিং | technical sense named by the source concept | prevents substitution by a neighboring catalog concept |
| compression | কম্প্রেশন | dynamic-range | preserves this concept's locked technical sense |
| contrast | কনট্রাস্ট | technical sense named by the source concept | prevents substitution by a neighboring catalog concept |
| dynamic range | ডায়নামিক রেঞ্জ | technical sense named by the source concept | prevents substitution by a neighboring catalog concept |
| floor | ফ্লোর | minimum/lower clamp | not a room floor |
| envelope | এনভেলপ | magnitude/amplitude envelope | preserves this concept's locked technical sense |
| comet | কমেট | moving-crest overlay effect | not geometry (fuzzy-inheritance hazard, topic 20) |
| overlay (noun) | ওভারলে | an added visual layer | distinct from verb "overlay" |
| animate / animation | অ্যানিমেট / অ্যানিমেশন | action verb / resulting motion | keeps the imperative verb distinct from the animation noun |
| animated / static | অ্যানিমেটেড / স্ট্যাটিক | category-header adjectives | preserves this concept's locked technical sense |
| projection | প্রক্ষেপণ | color or geometry projection | preserves this concept's locked technical sense |
| scale | স্কেল | verb and noun, same term | preserves this concept's locked technical sense |
| wireframe | ওয়্যারফ্রেম | wire-mesh render mode | preserves this concept's locked technical sense |
| identity | আইডেন্টিটি | no-op/passthrough transfer | distinct from unity (Smith-chart) |
| sentinel | সেন্টিনেল | unreachable-case guard value | preserves this concept's locked technical sense |
| bins | বিন | discretization buckets | preserves this concept's locked technical sense |
| companding | কম্প্যান্ডিং | bounded log curve (μ-law) | preserves this concept's locked technical sense |
| tone mapping | টোন ম্যাপিং | photographic tone-map | preserves this concept's locked technical sense |
| renderer | রেন্ডারার | drawing backend | not "render engine" |
| shader | শেডার | technical sense named by the source concept | prevents substitution by a neighboring catalog concept |
| allocation (memory) / managed allocator | অ্যালোকেশন / ম্যানেজড অ্যালোকেটর | technical sense named by the source concept | prevents substitution by a neighboring catalog concept |
| thread | থ্রেড | compute thread | distinct from তার (wire); no homonym collision in Bengali |
| widget | উইজেট | UI element | preserves this concept's locked technical sense |
| validation | ভ্যালিডেশন | validation-tree feature | distinct from verification checks |
| batch mode | ব্যাচ মোড | technical sense named by the source concept | prevents substitution by a neighboring catalog concept |
| fork (process) | ফর্ক | process fork | transliterated per topic 4 |
| deadlock | ডেডলক | technical sense named by the source concept | prevents substitution by a neighboring catalog concept |
| notifier | নোটিফায়ার | technical sense named by the source concept | prevents substitution by a neighboring catalog concept |
| token / operand / operator / arity | টোকেন / অপারেন্ড / অপারেটর / অ্যারিটি | expression-parser terms | preserves this concept's locked technical sense |
| override | ওভাররাইড | supersede a value (SY symbol) | not overwrite |
| swap | সোয়াপ | exchange | preserves this concept's locked technical sense |
| theme | থিম | UI/color theme | not topic/subject |
| noise / noise temperature | নয়েজ / নয়েজ তাপমাত্রা | electronic/thermal noise | not acoustic racket |
| efficiency | দক্ষতা | native term | preserves this concept's locked technical sense |
| interpolation | ইন্টারপোলেশন | technical sense named by the source concept | prevents substitution by a neighboring catalog concept |
| mnemonic | নেমোনিক | a card's code descriptor | not a memo/note |
| degrees / deg | ডিগ্রি (freestanding) / (deg) tag verbatim | axis/prose degrees vs unit tag | tag follows topic 4 verbatim-unit rule |
| diameter | ব্যাস | native term, canonical choice | not ডায়ামিটার |
| reflect | রিফ্লেক্ট (geometry mirror) / অনুসরণ করে (behavioral "mirrors") / প্রতিফলন (physics) | three distinct senses | never collapse the three |
| default(s) | ডিফল্ট | fallback value | preserves this concept's locked technical sense |
| normalize / normalization | স্বাভাবিকীকরণ | native term | translate, not transliterate |

No concept from Appendix A is absent from this table; none require not-applicable marking for Bengali.

## 11. Disambiguation policy
- Choose the correct technical sense for each ambiguous term per topic 10's table; never leave a term ambiguous where the table resolves it.
- Never add a qualifier absent from the source (eg বৈদ্যুতিক before কারেন্ট/চার্জ); program context (an EM simulator) already disambiguates.
- Add a qualifier only where the Bengali term would otherwise be genuinely ambiguous outside that context.
- Accepted intra-domain homonym: রেডিয়াল serves both the noun (radials) and the adjective (radial); context disambiguates, no separate term is coined.
- No locative/other homonym collision is recorded for this catalog beyond radials.
- গেইন (gerund/verb "gaining") vs গেইন (the noun, antenna directivity) is not a live ambiguity in this catalog; the noun sense is fixed by topic 10.

## 12. Cross-catalog consistency
- Use one locked topic-10 term per concept; replace every minority or outlier spelling with that form.
- Keep each false-friend set distinct: পোলারিটি / পোলারাইজেশন; ম্যাগনিচিউড / অ্যামপ্লিচিউড; পিক ভ্যালু / পিক ম্যাগনিচিউড; গ্রাউন্ড / মাটি; লোড / চার্জ; গেইন / বিবর্ধন / লাভ; কারেন্ট / সাম্প্রতিক; চার্জ / ফি; তার / কেবল / থ্রেড; বিকিরণ প্যাটার্ন / নকশা / দূর-ক্ষেত্র; এক্সাইটেশন / উত্তেজনা; নোড / অ্যান্টিনোড / শূন্য; স্কেল ফ্যামিলি / হিউ / প্যালেট কাইন্ড / রঙের প্রক্ষেপণ; কমেট / জ্যামিতি; আইডেন্টিটি / একক; রেন্ডারার / রেন্ডার ইঞ্জিন; ওভাররাইড / ওভাররাইট; ভিউয়ার / পর্যবেক্ষক / বক্তা / পূর্বরূপ; রিফ্লেক্ট / অনুসরণ করে / প্রতিফলন; স্ট্রাকচার / নির্মাণ; থিম / বিষয়; ভ্যালিডেশন / যাচাই; নেট গেইন / বাস্তব-অংশ গেইন; বৈদ্যুতিক ক্ষমতা / পাওয়ার. Never assign both sides of a slash-delimited distinction the same translation.
- Use transliterated electrical and computing terms only where topic 10 locks them; use the locked native forms তার, জ্যামিতি, ব্যাস, ঘন কোণ, পরিবাহিতা, দক্ষতা, স্বাভাবিকীকরণ, মাটি, and মোট ক্ষেত্র elsewhere.
- Use রং as the bare noun and রঙের before a dependent noun; use বিকিরণ for every radiation compound. These catalog-consistency rulings outrank an alternate locale spelling or loanword preference.

## 13. Priority ordering
- Precedence: correct meaning, then interface convention (topic 7/8), then catalog consistency (topic 12), then disambiguation (topic 11), then locale numeral form (topic 2).
- Override ruling: রং keeps the anusvara spelling even where a locale-preferred variant might differ, because catalog consistency (topic 12) outranks locale-form preference here.

## 14. Grammatical number
- Bengali distinguishes singular and plural nouns but leaves a technical noun unmarked after an explicit literal count (eg 3 তার); the count supplies number.
- Without an explicit count, use singular for one entity and the natural plural classifier -গুলো or formal -গুলি for multiple in full prose; use animate -এরা only for people.
- Adjectives and participles remain invariant for number; finite verbs take the number and honorific form required by their subject, with formal address governed by topic 8.

## 15. Grammatical agreement
- Bengali has no grammatical gender or declension-class concord; adjectives and participles remain invariant with explicit or implied head nouns.
- Resolve a standalone label through its implied technical noun without adding gender, number, or partitive marking.
- Retain a required reflexive element when the source meaning is reflexive; otherwise add none.

## 16. Morphological derivation
- Form borrowed technical verbs with a transliterated noun plus করা (eg অ্যানিমেট করা); never attach a coined native verbal suffix directly to the loanword.
- Form established borrowed verbal nouns with their locked noun form (eg রেন্ডারিং or অ্যানিমেশন), not a coined -ন derivative.
- Follow topic 10 for each native-term or loanword choice; fuse a Bengali suffix to its head and space multiword technical compounds under topic 1.

## 17. Postposition and sandhi selection
- Select the standard locative allomorph from the stem ending: use -এ after the applicable vowel-ending form and -তে after the applicable consonant-ending form.
- Bengali technical literals require no elision or contraction beyond standard suffix and postposition attachment.

## 18. Card/record-label register
- Use the fixed title designator কার্ড + Western numeral or তার + Western numeral (eg কার্ড 1).
- In running prose, attach the required genitive or locative suffix to the complete designator (eg তার 1-এর); do not hyphenate the noun and numeral.
- Bengali has no generic-noun case distinction; use the bare designator in titles and its grammatically suffixed form in prose.
- Keep title and running-prose forms internally consistent; never substitute one register for the other within a literal.

## 19. Multi-paragraph and whitespace fidelity
- Mirror source paragraph breaks, whether blank-line or single-line, at the same positions in the translation.
- Drop every clause removed from the current source; never preserve stale trailing text.
- Preserve source trailing newlines and punctuation exactly.
- Preserve semantic line breaks; add no visual wrapping absent from the source literal.
- Preserve complete meaning; never truncate or abbreviate wording to meet an assumed display limit.

## 20. Current-source fidelity
- Derive every translation from the current source literal and its supplied context.
- Reuse an inherited or copied translation only when its complete meaning agrees with the current source.
- Treat inherited কমেট for জ্যামিতি, a collapsed নিকট ক্ষেত্র/দূর-ক্ষেত্র pair, and an informal ending replacing formal -উন as unsafe source mismatches; apply topics 8 and 10 instead.

## 21. Script hygiene
- Bengali prose has a zero-failure invariant for wrong-script letters ৰ and ৱ and foreign-script homoglyphs; permit foreign glyphs only inside a topic-4 retained token whose literal spelling requires them.
- Translate plain foreign prose words; preserve genuine identifiers, units, symbols, and other retained tokens verbatim under topic 4.

## 22. Rule-file scope hygiene
- Keep only current decisions that can alter wording, Unicode characters, punctuation, capitalization, embedded tokens, mnemonic markers, or semantic whitespace in a translated literal.
- Omit catalog representation, serialization, headers, flags, validation workflow, rendering, fonts, shaping, layout, widget behavior, runtime formatting, sorting, search, display sizing, audit history, completion state, review metadata, provenance, and source citations.
- State each retained decision directly; include no implementation procedure, bibliography, correction history, or alternative policy source.

## 23. Section-disjointness declaration
- Script mechanics (topics 1-3 and 21) govern only character form; phrasing and structure (topics 5-7 and 14-20) govern only wording, grammar, order, and semantic whitespace; address register (topic 8) governs only formality, pronouns, and verb endings. Place each decision on one axis; topic 10 supplies terms without duplicating these rules.

## 24. Developer/debug-string policy
- Translate user-facing commands, labels, dialogs, confirmations, tooltips, statuses, and errors in the register assigned by topics 7-8.
- Translate informational diagnostics as terse, impersonal technical statements.
- Translate developer-facing and low-priority debug strings as terse technical statements; sibling strings within one subsystem retain one established phrasing pattern.
- Preserve every embedded identifier, function name, format specifier, and topic-4 retained token verbatim in every string family, independent of translation priority.
- Use priority only to order review; never leave an applicable user-facing, informational, or developer-facing literal untranslated because of priority.
