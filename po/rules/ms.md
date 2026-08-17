# Malay (ms-MY) Translation Rules for xnec2c

These rules govern standard Malaysian Malay in Rumi script for professional electrical and RF engineers, antenna designers, and amateur-radio operators.

## 1. Script and Orthography

- Use the Rumi alphabet only; omit Jawi from translated literals.
- Use the base 26-letter set; Malay requires no additional letters or diacritics.
- Precomposed-vs-combining: not applicable, ASCII Latin only.
- Apostrophe: straight ASCII `'` for elision in loanwords; no curly-quote look-alike.
- Joining/shaping mechanics: not applicable, non-cursive script, no positional letter forms.
- Directionality: left-to-right; embedded technical tokens stay LTR, unmirrored.
- Case distinction: yes; sentence case is default (topic 6).
- Orthographic standard: Ejaan Rumi Baharu, DBP-standard spelling.
- Spacing: single space between words and between native text and embedded numerals/technical tokens.
- Compound formation: open (spaced) compounds default for technical noun phrases, eg "satah bumi"; fused form only for a single lexicalized loanword.

## 2. Numerals in Literals

- Use digits `0`-`9` for technical values written in translated prose.
- Use period `.` as the decimal separator and comma `,` as the thousands separator.
- Keep formulas, examples, fixed defaults, and named mathematical or standards constants in source form.
- Form ordinals and indexes with `ke-` plus the digit, eg `ke-2`; treat the hyphen as an affix separator, never a decimal separator.

## 3. Punctuation and Quotation

- Native quotation marks: none distinct from source; retain straight `"..."` for embedded technical tokens and filenames.
- Prose punctuation: comma, question mark, exclamation mark, opening marks match source, no substitution.
- Spacing: no space before colon, semicolon, or terminal punctuation.
- Ellipsis: `...` (three periods), not `…`; dashes carry verbatim from source.
- Sentence terminator: full sentences (dialogs, confirmations, status/error messages) take a terminal period; short labels/fragments (menu items, buttons, field labels) omit it.
- Punctuation inside embedded technical runs (mnemonics, format specifiers, paths) stays in source form.

## 4. Never-translate Tokens

Keep every listed token byte-for-byte in source form; never translate, transliterate, localize, or alter its case, punctuation, or internal spacing.

- Keep NEC2 card mnemonics verbatim: `GW GA GH EX LD FR RP GE EN SP SM SC NE NH NT TL GN GD GC GX GR GS GM GF CP CM SY XQ EK KH PQ PT`.
- Keep unit symbols verbatim: `Hz kHz MHz GHz dB dBi dBm Ω W K S/m deg %`.
- Keep figure-of-merit and parameter tokens verbatim: `VSWR S11 S12 S21 S22 Z Zo F/B G/T`.
- Keep file extensions verbatim: `.nec .csv .s1p .s2p .png .gplot`.
- Keep every source format specifier verbatim, including `%s %d %f %c %%` and positional forms.
- Keep embedded function names, variable names, and configuration keys verbatim in developer strings.
- Keep physical and mathematical symbols verbatim: `E H θ φ μ`.
- Keep product, library, toolkit, and chart names verbatim: `xnec2c NEC2 gnuplot OpenGL Cairo MKL OpenBLAS OpenMP Smith`.
- Keep named transfer functions and algorithms verbatim: `Log Asinh μ-law Reinhard Sigmoid Identity`; translate descriptive `Power` as `Kuasa`.
- Translate conditional geometry loanwords consistently as `segment` → `segmen` and `patch` → `tampalan`; retain `tag` and process `fork` verbatim.
- Keep retained tokens left-to-right and unmirrored within Malay prose, following topic `1`.

## 5. Format-specifier Integrity

- Preserve every source specifier with the same spelling, type, and occurrence count.
- Keep source order by default; use positional forms, eg `%1$s`, only when natural Malay information order requires reordering.
- Prefer restructuring Malay wording around fixed specifier positions rather than moving the specifiers.
- Keep all flags, widths, precisions, argument indexes, and conversion letters inside specifiers unchanged.

## 6. Capitalization and Title-case

- Use sentence case for labels, menu items, and dialog titles; do not capitalize every major word.
- Axis letters X/Y/Z stay uppercase, unchanged from source.
- Exemption: a lowercase math/coordinate variable (eg "x", "y" in a formula) stays lowercase.
- Coordinated option names (eg "Minor/Major Axis") follow sentence case, no per-element capitalization: "Paksi Kecil/Utama".
- Generic card/record noun "kad" is lowercase mid-sentence, capitalized only at title/dialog-heading position (topic 18); acronyms and proper nouns (NEC2, VSWR) retain source casing always.

## 7. Interface Register by String Type

Use the shortest complete natural Malay form; preserve all meaning and follow Malay modifier order rather than source length or word order.

- Commands, buttons, and menu actions: use a subjectless active imperative with the verb first and its object after it, eg `Simpan fail`.
- Field labels: use a concise noun phrase in head-modifier order followed by the retained colon, eg `Frekuensi:`.
- Dialog statements: use complete declarative sentences in subject-verb-object order; place known context before new or consequential information.
- Confirmations: use the interrogative pattern `Adakah anda pasti ...?`; retain the explicit subject only for direct address.
- Tooltips: use a complete phrase or sentence ordered as action, condition, then consequence; include the source-stated reason that an unavailable control is disabled.
- Status and error messages: use subjectless impersonal declarations, with the affected entity before its state, eg `Fail tidak dijumpai`.
- User-visible domain entities: place the generic head before its identifier or modifier, eg `Kad GW` and `corak sinaran`.
- Developer and diagnostic strings follow topic `24`.

## 8. Formality and Address

- Malay has no formal-informal pronoun split; carry a neutral professional register through word choice and construction.
- Drop the subject in commands, labels, status messages, and errors; use `anda` only when a dialog must address the user directly.
- Use no honorifics; use second person only as `anda`, and omit first person unless the source meaning requires it.
- Preserve personal names in their source order and form; Malay interface text adds no honorific or gendered title.
- Malay has no grammatical gender, animacy agreement, or gendered occupational forms; choose neutral role nouns and natural number-neutral constructions.
- Malay has no formality-bearing verb endings; commands use the bare imperative and dialogs use neutral full sentences.
- Use confirmations in the form `Adakah anda pasti ...?`.
- Exclude `kamu`, `awak`, slang, colloquial register, court register, marketing language, and archaic forms.

## 9. Accelerator/hotkey Mnemonics

- Place underscore `_` immediately before a typable letter within the translated term.
- Choose the mnemonic letter from that translated term, never from a source-word transliteration.
- Malay uses Rumi letters directly; no separate mnemonic presentation applies.
- Preserve source presence: add a mnemonic only when the source literal contains one.
- Prefer an easily typed letter already present in the term; use another letter in that term when required to keep sibling mnemonics distinct.

## 10. Domain Lexicon

Locked term table, one row per Appendix A concept: concept, chosen Malay term, intended sense, purpose/hazard guarded.

### Electrical primitives

| Concept | Term | Sense | Purpose/hazard |
|---|---|---|---|
| current | arus | electrical current (A) | not "semasa" (temporal current/recent) |
| charge | cas | electrical charge (C) | not "caj" (billing/fee) |
| voltage | voltan | electric potential | - |
| power (electrical) | kuasa | radiated or dissipated watts, power gain, and power flow | lowercase distinguishes it from transfer-family label `Kuasa` |
| impedance | galangan | complex Z | distinct from resistance and reactance |
| resistance | rintangan | real part of Z | distinct from impedance and load |
| reactance | reaktans | imaginary part of Z | - |
| inductance | induktans | - | - |
| capacitance | kapasitans | - | - |
| conductivity | kekonduksian | material S/m | native term |
| admittance | admitans | admittance-matrix sense | distinct from impedance |
| load | beban | LD-card impedance load | not physical weight; never merged with cas |
| gain | gandaan | antenna directivity ratio (dB) | not "keuntungan" (profit), not "penguatan" (amplifier amplification) |
| excitation | pengujaan | EM energy input/source | not emotional excitement |
| feedpoint | titik suapan | antenna feed point | - |
| port | port | excitation/S-parameter port | kept as established loanword, distinct from "titik suapan" |
| radials | jejari | ground-plane radial wires (noun) | distinct from adjective "radial", sense fixed by grammatical position |

### Ground and earth

| Concept | Term | Sense | Purpose/hazard |
|---|---|---|---|
| ground / ground plane | bumi | RF reference plane, ground plane, and GN/GD cards | one electrical-reference term across all sub-senses; distinct from physical `tanah` |
| earth (physical medium) | tanah | terrain/noise-model earth, "below ground" geometry | distinct from electrical "bumi" |
| ground wave | gelombang bumi | propagation term | distinct from the ground reference itself |

### Geometry primitives

| Concept | Term | Sense | Purpose/hazard |
|---|---|---|---|
| wire | wayar | thin conductor / GW element | not "kabel" (cable/cord); distinct from compute "benang" (thread) |
| segment | segmen | NEC2 geometry subdivision | kept as loanword (topic 4) |
| patch | tampalan | NEC2 surface patch (SP/SM) | translated, not kept verbatim (topic 4) |
| tag | tag | NEC2 geometry identifier | kept verbatim; not a UI label or a card |
| card | kad | NEC2 input record | register per topic 18 |
| kernel | kernel | integral-equation/thin-wire kernel | kept verbatim loanword, not an OS kernel |
| cliff | tebing | two-medium ground-boundary type | not a fracture/break |
| structure | struktur | antenna model geometry | not "pembinaan" (construction) |
| model | model | NEC model or noise-temperature model | kept verbatim loanword, sense fixed by context |
| geometry | geometri | the model geometry | - |
| crossed | disilang | transmission-line conductors crossed/reversed | not "dipotong" (cut/severed) |

### Field, pattern, viewer

| Concept | Term | Sense | Purpose/hazard |
|---|---|---|---|
| field (EM) | medan | near/total/E/H field | catalog carries no data/config-field sense; no separate term needed |
| near field | medan dekat | opposed spatial region | pairs symmetrically with medan jauh |
| far field | medan jauh | opposed spatial region | pairs symmetrically with medan dekat |
| far-field contribution | sumbangan medan jauh | per-direction contribution | not near-field animation |
| radiation | sinaran | radiated emission | - |
| radiation pattern | corak sinaran | plotted directional response | not template/design; distinct from the far-field region itself |
| gain pattern | corak gandaan | the gain radiation pattern | - |
| polarization | polarisasi | antenna/wave field orientation | distinct from polarity (Appendix C) |
| polarity | kekutuban | sign (+/-) of a quantity | false friend of polarisasi |
| phase | fasa | - | - |
| reference phase | fasa rujukan | - | - |
| frequency | frekuensi | - | - |
| wave / wavelength | gelombang / panjang gelombang | - | - |
| standing wave | gelombang pegun | opposed to traveling wave | - |
| traveling wave | gelombang berjalan | opposed to standing wave | - |
| node / antinode | nod / antinod | standing-wave zero / maximum | distinct from a generic numeric null/zero |
| crest | puncak gelombang | instantaneous wave apex (comet-head) | distinct from curve/step "puncak"; qualifier used only in comet-overlay context |
| magnitude | magnitud | modulus of a quantity | distinct from amplitude |
| amplitude | amplitud | oscillating-quantity peak | distinct from magnitude |
| peak value | nilai puncak | distinct UI option | never collapsed with magnitud puncak |
| peak magnitude | magnitud puncak | distinct UI option | never collapsed with nilai puncak |
| instantaneous | seketika | projection mode | "(φ=0)" qualifier kept verbatim only where source carries it |
| Poynting vector | vektor Poynting | - | proper-noun component kept verbatim |
| solid angle | sudut pepejal | - | - |
| net gain | gandaan bersih | total-minus-mismatch gain | not "gandaan nyata" (real-part gain) |
| viewer | pemapar | observation direction / 3D view widget | not observer/speaker/preview |
| flow / flow direction | aliran / arah aliran | patch/current flow | - |
| total field | jumlah medan | quantifier precedes noun | - |

### Color, tone, animation subsystem

| Concept | Term | Sense | Purpose/hazard |
|---|---|---|---|
| color | warna | - | - |
| color projection | unjuran warna | which quantity drives hue | - |
| hue | rona | color-wheel angle | - |
| brightness | kecerahan | luminance channel | - |
| hue encoding | pengekodan rona | distinct internal enum | not collapsed to unjuran warna |
| brightness encoding | pengekodan kecerahan | distinct internal enum | not collapsed to unjuran warna |
| color scale | skala warna | magnitude-to-color scale | - |
| scale family / color tone | keluarga skala | transfer-curve family (Power/Log/Asinh/μ-law/Reinhard/Sigmoid/Identity) | one concept, one term for both source spellings |
| palette / palette kind | palet / jenis palet | palette-layout enum | distinct from scale family and color projection |
| ramp | tanjakan | a palette kind | distinct from gradient |
| gradient | kecerunan | linear color strip | distinct from ramp |
| gamma | gama | power-law exponent | - |
| knee | lutut | soft-knee bend point | - |
| softening | pelembutan | - | - |
| compression | mampatan | dynamic-range compression | - |
| contrast | kontras | - | - |
| dynamic range | julat dinamik | - | - |
| floor | aras minimum | minimum or lower clamp for brightness or dB | avoids the physical-room sense of `lantai` |
| envelope | sampul | magnitude/amplitude envelope | - |
| comet | komet | moving-crest overlay effect | not geometry (Appendix C hazard) |
| overlay (noun) | lapisan tindanan | an added visual layer | distinct from the verb "menindankan" |
| animate / animation | animasikan / animasi | action verb / process noun | preserves grammatical function across commands and labels |
| animated | beranimasi | category-header adjective | opposed to statik |
| static | statik | phase-invariant | opposed to beranimasi |
| projection | unjuran | color or geometry projection | sense fixed by context |
| scale (verb) | skalakan | to scale | - |
| scale (noun) | skala | a scale | one term covers color-scale and generic scale, sense fixed by context |
| wireframe | rangka dawai | wire-mesh render mode | - |
| identity | identiti | no-op/passthrough transfer | distinct from unity/Smith-chart (Appendix C) |
| sentinel | sentinel | unreachable-case guard value | kept verbatim loanword |
| bins | bekas | discretization buckets | - |
| companding | kompanding | bounded log curve (μ-law) | kept as loanword |
| tone mapping | pemetaan ton | photographic tone-map | - |

### Render and compute

| Concept | Term | Sense | Purpose/hazard |
|---|---|---|---|
| renderer | penerap | drawing backend | not "enjin penerap" (render engine, Appendix C) |
| shader | shader | - | kept verbatim loanword |
| allocation (memory) | peruntukan | allocation and the allocator/report | - |
| managed allocator | peruntuk terurus | - | - |
| thread | benang | compute thread | no collision with wire/wayar; parity noted for source homonym risk |
| widget | widget | UI element | kept verbatim loanword |
| validation | pengesahan | the validation-tree feature | catalog does not lexically separate validation from verification; one term covers both |
| batch mode | mod kelompok | - | - |
| fork (process) | fork | process fork | kept verbatim (topic 4) |
| deadlock | kebuntuan | - | - |
| notifier | pemberitahu | - | - |
| token | token | expression-parser term | kept verbatim loanword |
| operand | operan | - | - |
| operator | operator | - | - |
| arity | ariti | - | - |
| override | mengatasi | supersede a value (SY symbol) | not "menulis ganti" (overwrite, Appendix C) |
| swap | tukar ganti | exchange | - |
| theme | tema | UI/color theme | not "topik/subjek" (Appendix C) |

### Metrics and miscellaneous

| Concept | Term | Sense | Purpose/hazard |
|---|---|---|---|
| noise / noise temperature | hingar / suhu hingar | electronic/thermal noise | not acoustic racket |
| efficiency | kecekapan | - | - |
| interpolation | interpolasi | - | - |
| mnemonic | mnemonik | a card's code descriptor | not a memo/note |
| degrees / deg | darjah / (deg) | freestanding prose vs parenthetical unit tag | "(deg)" kept verbatim like other unit tags |
| diameter | diameter | canonical loanword | locked spelling, no native synonym used |
| reflect | pantul / mencerminkan / pantulan | geometry mirror op / behavioral tracking / physics reflection | three distinct senses, three distinct terms |
| default(s) | lalai | fallback value | - |
| normalize / normalization | menormalkan / penormalan | - | translated, not transliterated |

### Catalog extensions beyond Appendix A

| Concept | Term | Sense | Purpose/hazard |
|---|---|---|---|
| optimizer | pengoptimum | process that searches for an improved solution | distinguishes the process from the resulting optimum |
| threshold | ambang | comparison boundary | avoids a generic limit sense |
| overlap | pertindihan | shared geometric or value range | distinguishes intersection from replacement |
| major axis | paksi utama | longer principal axis | keeps the opposed axis pair symmetric |
| minor axis | paksi kecil | shorter principal axis | keeps the opposed axis pair symmetric |

## 11. Disambiguation Policy

- The correct technical sense is chosen per the topic-10 table for every ambiguous term.
- No qualifier absent from the source is added, eg "View Currents" → "Lihat Arus", not "Lihat Arus Elektrik"; program context already disambiguates.
- A qualifier is added only where the Malay term itself would otherwise be genuinely ambiguous, eg "puncak gelombang" qualifies "puncak" only in comet-overlay context.
- Accepted intra-domain homonym: "kuasa" covers both electrical power and the Power transfer-family name; no other accepted homonym recorded.
- Locative/other homonym collision: "bumi" (ground) and "tanah" (earth) stay distinct, no collision.
- Gerund vs noun: "loading" gerund (reading/parsing a file or card → memuatkan/pemuatkan) is distinct from the electrical LD load (→ beban); "Loading Command (LD Card)" → "Arahan Beban", "loading data card error" → "kad data pemuatan".

## 12. Cross-catalog Consistency

- Reuse one topic-`10` term for each concept; introduce no competing synonym.
- Keep these forms distinct: `kekutuban` / `polarisasi`; `magnitud` / `amplitud`; `nilai puncak` / `magnitud puncak`; `bumi` / `tanah`; `beban` / `cas`; `gandaan` / `penguatan` / `keuntungan`; `arus` / `semasa`; `cas` / `caj`; `wayar` / `kabel` / `benang`; `corak sinaran` / `medan jauh`; `pengujaan` / `keterujaan`; `nod` and `antinod` / `sifar`; `keluarga skala` / `rona` / `jenis palet` / `unjuran warna`; `komet` / `geometri`; `identiti` / `kesatuan`; `penerap` / `enjin penerapan`; `mengatasi` / `menulis ganti`; `pemapar` / `pemerhati` / `penceramah` / `pratonton`; `pantul` / `mencerminkan` / `pantulan`; `struktur` / `pembinaan`; `tema` / `topik`; `gandaan bersih` / `gandaan nyata`; and electrical `kuasa` / transfer-family `Kuasa`.
- Malay uses `pengesahan` for both validation and verification; collapse the pair because the language has no established distinct technical terms in this interface.
- Lock these canonical loanwords: `segmen`, `port`, `model`, `kernel`, `sentinel`, `shader`, `widget`, `token`, `fork`, `tag`, and `kompanding`.
- Unify every outlier spelling to the locked form.
- Prefer catalog consistency over a native alternative for `port`; retain `tag` to prevent collision with `kad`.

## 13. Priority Ordering

- Apply this precedence: correct meaning, interface convention, catalog consistency, disambiguation, then literal numeral form.
- Retain `port` because catalog consistency outranks a new native coinage; distinguish electrical `kuasa` from transfer-family `Kuasa` because correct meaning outranks uniform casing.

## 14. Grammatical Number

- Malay nouns, adjectives, participles, and verbs do not inflect for grammatical number in translated literals.
- After an explicit count, use the unmarked noun form without reduplication or an invented plural marker, eg `2 wayar`.

## 15. Grammatical Agreement

- Not applicable: Malay has no grammatical gender or number concord.

## 16. Morphological Derivation

- Borrowed technical verbs/nouns take the standard meN-/peN-...-an affix family, eg "menormalkan", "penormalan", "pengoptimum"; ad hoc affix-less borrowing forbidden where a standard derivation exists.
- Verbal-noun formation uses the peN-...-an pattern, eg "pemuatan" from "muat".
- Native affixation on loan roots is preferred over importing a foreign derivational form; compounding is open (spaced), per topic 1.

## 17. Preposition and Sandhi Selection

- Not applicable: Malay prepositions are invariant, no context-conditioned form or elision rule applies.

## 18. Card/record-label Register

- Fixed designator form in dialog/editor titles: "Kad [Mnemonic]", eg "Kad GW".
- Running-prose form in messages: lowercase "kad" plus mnemonic, no hyphenation, eg "ralat kad data GW".
- Generic-noun casing: "kad" lowercase mid-sentence, "Kad" capitalized only at title-heading position (topic 6); no separate short-vs-long form distinction.
- Each register stays internally consistent; the title form never appears mid-sentence and vice versa.

## 19. Multi-paragraph and Whitespace Fidelity

- Mirror every source paragraph break at the same position, preserving the distinction between a blank line and a single line break.
- Preserve semantic line breaks; add no visual wrapping absent from the source literal.
- Drop clauses removed from the current source rather than inheriting stale trailing text.
- Carry source trailing newlines and terminal punctuation exactly.
- Preserve complete meaning; never truncate wording or use an unnatural abbreviation for an assumed display limit.

## 20. Current-source Fidelity

- Derive every translation from the current source text and its supplied context.
- Reuse inherited wording only when its complete meaning agrees with the current source.
- Treat these inherited mappings as unsafe without full contextual agreement: gerund `loading` versus electrical `load`, excitation `port` versus `feedpoint`, and NEC model versus noise-temperature model.

## 21. Script Hygiene

- Use only Rumi letters in translated prose; allow non-Rumi characters only inside retained tokens whose exact spelling requires them.
- Reject mixed-script words and foreign homoglyphs adjacent to Rumi letters; preserve `Ω`, `θ`, `φ`, and `μ` only as topic-`4` symbols.
- Translate plain foreign prose; retain only genuine identifiers, units, symbols, and proper technical names listed in topic `4`.

## 22. Rule-file Scope Hygiene

- Retain only current decisions that can alter wording, Unicode characters, punctuation, capitalization, embedded tokens, mnemonic markers, or semantic whitespace in translated literals.
- Omit catalog representation, serialization, headers, flags, workflow, rendering, fonts, shaping, layout, widget behavior, runtime formatting, sorting, search, display sizing, audit history, completion state, review metadata, provenance, and citations.
- State each decision directly; include no implementation procedure, bibliography, correction history, or competing policy.

## 23. Section-disjointness Declaration

- Topics `1` and `3` govern script characters and punctuation; topics `6`, `7`, `18`, and `19` govern phrasing and structure; topic `8` governs address register. Apply each decision on only its named axis.

## 24. Developer/debug-string Policy

- Translate user-facing controls, dialogs, tooltips, status messages, errors, and informational notices into Malay; review priority never permits an applicable user-facing literal to remain untranslated.
- Keep developer-facing diagnostics and low-level debug strings in source form unless an established sibling family in the same subsystem is already translated.
- Use terse technical Malay for any translated diagnostic family.
- Preserve every embedded identifier, function name, format specifier, unit, and retained token in all string families, independent of translation priority.
- Render the `BUG:` diagnostic prefix as "PEPIJAT:", held distinct from the "RALAT:" prefix that carries ordinary failures; the two severities never share a word.
- Keep an identifier-plus-value diagnostic pair contiguous (eg `sweep_state=%d`); a qualifier precedes the whole pair and never stands between the identifier and its `=`.
