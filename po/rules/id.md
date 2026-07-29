# id translation rules

## 1. Script and orthography

- Scope: Indonesian for Indonesia, written in Latin script under Ejaan Bahasa Indonesia yang Disempurnakan Edisi V, for professional electrical and RF engineers, antenna designers, and amateur-radio operators.
- Alphabet: use the standard 26 Latin letters; Indonesian requires no additional letters or diacritics.
- Precomposed-vs-combining form: not applicable because Indonesian words require no diacritics.
- Apostrophes and textual joiners: use none in native word formation; preserve either only where a retained token requires its exact source code point.
- Script-specific joining and code-point distinctions: not applicable to Indonesian Latin orthography.
- Directionality: write Indonesian and embedded technical tokens left-to-right; do not mirror retained tokens.
- Case distinction: use uppercase and lowercase; capitalize sentence initials and proper nouns, and apply topic 6 to interface labels.
- Spacing: use one space between words and between Indonesian text and embedded foreign or numeric tokens; attach punctuation under topic 3.
- Compounds: use established Indonesian spelling; write ordinary domain compounds as spaced head-plus-qualifier phrases, eg "distribusi arus"; use a fused or hyphenated form only when Ejaan Bahasa Indonesia yang Disempurnakan Edisi V prescribes it.

## 2. Numerals in literals

- Digit set: use `0-9` for technical values physically present in Indonesian prose.
- Separators: use comma as the decimal separator and period as the thousands separator in localized prose numbers, eg `1.234,5`.
- Retained numeric tokens: preserve formulas, fixed source examples, fixed defaults, and named mathematical or standards constants exactly as written in the source.
- Ordinals: use `ke-` plus the literal digit, eg `ke-2`; keep the hyphen distinct from a decimal separator.
- Indices: preserve the source index notation and separator because changing either can alter technical meaning.

## 3. Punctuation and quotation

- Quotation marks: no native quotation-mark substitution convention established; retain source quotes, since UI strings carry minimal literary quotation.
- Native punctuation: standard Latin comma, question mark, exclamation, and period; Indonesian prose punctuation matches source, no substitution.
- Spacing around colon/semicolon/terminal punctuation: no space before, single space after, matching source convention.
- Ellipsis: "..." (three periods); dashes carry through unchanged from source.
- Sentence-terminator policy: dialog/confirmation full sentences take a period; short labels, buttons, and field names omit it.
- Punctuation inside embedded technical runs (format specifiers, mnemonics, unit tokens): stays in source form.

## 4. Never-translate tokens

- Zero-failure invariant: retain every token in this section byte-for-byte in source spelling, script, case, punctuation, and direction; translate, transliterate, inflect, split, or normalize none.
- NEC2 card mnemonics: retain `GW GA GH EX LD FR RP GE EN SP SM SC NE NH NT TL GN GD GC GX GR GS GM GF CP CM SY XQ EK KH PQ PT` verbatim so file-format identifiers round-trip.
- Unit symbols: retain frequency `Hz kHz MHz GHz`, gain `dB dBi`, impedance `Ω`, power and ratio `W dBm`, temperature `K`, conductivity `S/m`, angle `° deg`, and percent `%` verbatim as standard engineering notation.
- Figure-of-merit and parameter tokens: retain `VSWR`, S-parameters (`S11 S12 S21 S22` and indexed forms), `Z`, `Z0`, `|Z|`, `F/B`, and `G/T` verbatim as RF notation.
- File extensions and literal filenames: `.nec`, `.csv`, `.s1p`, `.s2p`, `.png` - kept verbatim.
- Format specifiers: printf-style placeholders kept verbatim, per topic 5.
- Embedded identifiers: function/variable/config-key names inside developer strings kept verbatim.
- Physical/mathematical symbol letters: field letters (`E`, `H`), angle/exponent letters kept in source form.
- Product/library/tool names: program name, numerics/graphics library and toolkit names, chart proper name - kept verbatim.
- Named transfer-function/algorithm terms: surnamed/standardized curve names kept verbatim; the descriptive "Power" family name is the exception that translates, rendered "Pangkat" (distinct from `daya`, electrical power, per topic 10/12).
- Conditional loanwords: `patch` and `tag` kept untranslated; `segment` transliterates to `segmen`; `fork` (process) kept verbatim; `ground plane` kept verbatim as established RF jargon; this decision holds file-wide.
- Directionality: all above tokens render left-to-right inside Indonesian text; topic 1 states no bidirectional case exists, so no mirroring applies.

## 5. Format-specifier integrity

- Zero-failure invariant: preserve every source format specifier byte-for-byte with the same conversion, flags, width, precision, length, positional index, and occurrence count; add, remove, translate, or corrupt none.
- Ordering: retain source order by default; use explicit positional specifiers only where the source already supplies them and Indonesian grammar requires argument presentation in another order.
- Fixed positions: restructure Indonesian wording around non-positional specifiers rather than moving or renumbering them.
- Specifier numbers: preserve all digits inside a specifier exactly; never localize them.

## 6. Capitalization and title-case

- Default casing: sentence case for labels, menu items, and dialog titles; no title-case-per-word carried from the source-language source.
- Axis-letter casing: X/Y/Z axis letters keep source uppercase form.
- Lowercase math/coordinate variable exemption: a lowercase source variable (eg `x`, `y` in a formula) stays lowercase.
- Coordinated option names: capitalize only the first element and any proper noun, eg `Pilih semua/Tidak ada`; do not apply title case to each element.
- Generic card/record noun casing: use lowercase in running prose and sentence-initial capitalization in titles; use `Tidak ada` for the standalone option and lowercase its parenthetical qualifier, eg `Tidak ada (geometri)`. Retain acronym and proper-name casing.
- Script carries case distinction; this topic is not reduced to mnemonic-only handling.

## 7. Interface register by string type

- Commands, buttons, and menu actions: use the shortest complete imperative with the verb first, omit the subject, retain every object and qualifier, and avoid nominalization, eg "Simpan".
- Field labels: use a concise head-plus-qualifier noun phrase, omit subject and verb, and retain a source colon.
- Dialogs and confirmations: use full subject-predicate sentences in natural Indonesian order; use interrogative mood for questions and place the requested decision after its context.
- Tooltips: use a complete declarative clause or concise verb phrase; state function before conditions and retain the source reason that an unavailable control is disabled.
- Status and error messages: use impersonal declarative clauses, place the affected entity before its state when natural, and use passive `di-` forms where the result matters more than the actor.
- User-visible domain entities: use the topic 10 term as the head followed by its qualifier; retain proper names under topic 4 and personal-name order under topic 8.
- Terseness: use the shortest natural complete form; preserve all meaning, avoid ad hoc abbreviations, and do not imitate source length.
- Developer and debug strings follow topic 24.

## 8. Formality and address

- Register: use formal standard Indonesian; use capitalized `Anda` only where the source addresses the user, and omit informal `kamu` and `lu`.
- Commands: omit `Anda` and politeness particles; use `silakan` only where the source expresses a polite request.
- Status and error messages: prefer impersonal subject omission under topic 7; retain grammatical person only where source meaning requires it.
- Confirmations: use `Apakah Anda yakin...` followed by the action or consequence.
- Gender and agreement: not applicable because Indonesian nouns, pronouns, verbs, and adjectives carry no grammatical gender.
- Inclusive language: use role or function nouns rather than gendered descriptions; preserve natural number and animacy without adding gender.
- Honorifics: add none unless the source names one; preserve a supplied honorific without inventing a local substitute.
- Personal names: retain the supplied name order and spelling; Indonesian literals impose no family-name inversion.
- Morphological formality: not applicable because formality is lexical rather than marked by verb endings.
- Prohibited registers: omit casual, slang, regional, archaic, over-formal, and commercial wording.

## 9. Accelerator/hotkey mnemonics

- Marker: place one underscore immediately before the mnemonic letter inside the translated literal.
- Letter choice: select a typable Latin letter present in the translated term; do not preserve or transliterate the source mnemonic letter when that letter is absent from the translation.
- Source presence: preserve one mnemonic only where the source carries one; add none where the source has none.
- Separate-letter presentation: not applicable because Indonesian uses Latin script and needs no parenthetical mnemonic.
- Typability: use an unaccented `A-Z` letter from the translated term.

## 10. Domain lexicon

| Concept | Term | Sense / hazard |
|---|---|---|
| current | arus | electrical current; not temporal "terkini" |
| charge | muatan | electrical charge; not billing "tagihan"/cargo |
| voltage | tegangan | electric potential |
| power (electrical) | daya | watts/gain/flow; distinct from "Power" family name (`Pangkat`) and math power-law (`pangkat`) |
| impedance | impedansi | complex Z; distinct from resistansi/reaktansi |
| resistance | resistansi | real part of Z |
| reactance | reaktansi | imaginary part of Z |
| inductance | induktansi | established technical sense; locks one catalog term |
| capacitance | kapasitansi | established technical sense; locks one catalog term |
| conductivity | konduktivitas | material S/m |
| admittance | admitansi | admittance-matrix sense, distinct from impedansi |
| load | beban | LD-card impedance load; not physical weight; distinct from muatan |
| gain | gain | antenna directivity ratio in dB; keeps it distinct from amplifier `penguatan` and profit `keuntungan` |
| excitation | eksitasi | EM energy input; not emotional excitement |
| feedpoint | titik catu | antenna feed point |
| port | port | kept, excitation/S-parameter port |
| radials | radial | ground-plane radial wires (noun); "radial" also serves the adjective, context disambiguates (topic 11) |
| ground / ground plane | ground plane | kept verbatim, RF reference plane, GN/GD cards, one term across all sub-senses; not `tanah` |
| earth (physical medium) | tanah | terrain/noise-model earth; distinct from ground plane |
| ground wave | gelombang tanah | propagation term, distinct from ground plane |
| wire | kawat | thin conductor/GW element; not cable/cord `kabel` |
| segment | segmen | NEC2 geometry subdivision |
| patch | patch | kept, NEC2 surface patch (SP/SM) |
| tag | tag | kept, NEC2 geometry identifier; not a UI label or a card |
| card | kartu | NEC2 input record; register in topic 18 |
| kernel | kernel | thin-wire kernel; not an OS kernel, context disambiguates |
| cliff | tebing | two-medium ground-boundary type; not a fracture |
| structure | struktur | antenna model geometry; not "konstruksi" |
| model | model | NEC model or noise-temperature model |
| geometry | geometri | the model geometry |
| crossed | disilangkan | transmission-line conductors reversed; not cut/severed |
| field (EM) | medan | near/total/E/H field; distinct from a data/config field |
| near field / far field | medan dekat / medan jauh | opposed spatial regions, kept symmetric |
| far-field contribution | kontribusi medan jauh | per-direction contribution |
| radiation | radiasi | radiated emission |
| radiation pattern | pola radiasi | plotted directional response; not template/design |
| gain pattern | pola penguatan | the gain radiation pattern |
| polarization | polarisasi | antenna/wave field orientation |
| polarity | polaritas | sign (+/-); false friend of polarisasi |
| phase | fase | established technical sense; locks one catalog term |
| reference phase | fase referensi | established technical sense; locks one catalog term |
| frequency | frekuensi | established technical sense; locks one catalog term |
| wave / wavelength | gelombang / panjang gelombang | established technical sense; locks one catalog term |
| standing wave / traveling wave | gelombang berdiri / gelombang berjalan | opposed pair |
| node / antinode | simpul / perut | standing-wave zero/max; also carries null/peak overlay sense |
| crest | puncak sesaat | instantaneous wave apex (comet-head); distinct from a curve/step peak |
| magnitude | magnitudo | modulus of a quantity; distinct from amplitudo |
| amplitude | amplitudo | oscillating-quantity peak; distinct from magnitudo |
| peak value / peak magnitude | Nilai Puncak / Magnitudo Puncak | two distinct UI options, never collapsed |
| instantaneous | sesaat | add "(φ=0)" only where source carries it |
| Poynting vector | vektor Poynting | kept name |
| solid angle | sudut ruang | established technical sense; locks one catalog term |
| net gain | penguatan bersih | total-minus-mismatch gain; not "penguatan bagian riil" |
| viewer | Viewer | kept untranslated, established catalog term; never "Pengamat" |
| flow / flow direction | aliran / arah aliran | patch/current flow |
| total field | medan total | established technical sense; locks one catalog term |
| color | warna | established technical sense; locks one catalog term |
| color projection | proyeksi warna | which quantity drives hue |
| hue | corak warna | color-wheel angle |
| brightness | kecerahan | luminance channel |
| hue encoding / brightness encoding | pengkodean corak / pengkodean kecerahan | distinct internal enums, neither collapses to "proyeksi warna" |
| color scale | skala warna | magnitude-to-color scale |
| scale family / color tone | keluarga skala | one concept, two source spellings, one term; members Power/Log/Asinh/μ-law/Reinhard/Sigmoid/Identity kept as named curves except "Power" → "Pangkat" |
| palette / palette kind | palet / jenis palet | distinct from scale family and color projection |
| ramp / gradient | gradasi | a palette kind / linear color strip |
| gamma | gamma | kept, power-law exponent |
| knee | lutut | soft-knee bend point |
| softening | pelunakan | established technical sense; locks one catalog term |
| compression | kompresi | dynamic-range |
| contrast | kontras | established technical sense; locks one catalog term |
| dynamic range | rentang dinamis | established technical sense; locks one catalog term |
| floor | batas bawah | minimum/lower clamp (brightness/dB floor); not a room floor |
| envelope | selubung | magnitude/amplitude envelope |
| comet | komet | moving-crest overlay effect; not geometry |
| overlay (noun) | overlay | kept, added visual layer; distinct from verb "melapisi" |
| animate / animation | animasikan / animasi | established technical sense; locks one catalog term |
| animated / static | Animasi / Statis | category-header adjectives (dynamic vs phase-invariant) |
| projection | proyeksi | color or geometry projection |
| scale | skala / skalakan | noun `skala`, verb `skalakan` |
| wireframe | rangka kawat | wire-mesh render mode |
| identity | identitas | no-op/passthrough transfer; distinct from unity (`kesatuan`, Smith-chart) |
| sentinel | sentinel | kept, unreachable-case guard value |
| bins | bin | kept, discretization buckets |
| companding | companding | kept, bounded log curve (μ-law), no established native term |
| tone mapping | pemetaan tona | photographic tone-map |
| renderer | renderer | kept, drawing backend; not "mesin render" |
| shader | shader | kept |
| allocation (memory) / managed allocator | alokasi (memori) / pengalokasi terkelola | established technical sense; locks one catalog term |
| thread | thread | kept, compute thread; distinct from `kawat` (wire), context disambiguates |
| widget | widget | kept, UI element |
| validation | validasi | validation-tree feature; distinct from verification checks `verifikasi` |
| batch mode | mode batch | established technical sense; locks one catalog term |
| fork (process) | fork | kept verbatim |
| deadlock | deadlock | kept |
| notifier | notifier | kept |
| token / operand / operator / arity | token / operand / operator / aritas | expression-parser terms |
| override | menimpa | supersede a value (SY symbol); not overwrite (`menulis ulang`) |
| swap | tukar | exchange |
| theme | tema | UI/color theme; not "topik/subjek" |
| noise / noise temperature | derau / suhu derau | electronic/thermal noise; not acoustic "kebisingan" |
| efficiency | efisiensi | established technical sense; locks one catalog term |
| interpolation | interpolasi | established technical sense; locks one catalog term |
| mnemonic | mnemonik | a card's code descriptor; not a memo/note |
| degrees / deg | derajat / "(deg)" | freestanding axis/prose "derajat" vs the parenthetical unit tag "(deg)", kept like other unit tags |
| diameter | diameter | canonical loanword over native synonym "garis tengah" |
| reflect | pantulkan / mencerminkan / refleksi | geometry mirror op / behavioral tracking / physics reflection - three distinct senses |
| default(s) | baku | fallback value |
| normalize / normalization | normalisasi | standard EYD-assimilated form, treated as translated, not raw transliteration |

## 11. Disambiguation policy

- Correct technical sense chosen per entry from program context, eg "arus" for current always resolves to electrical, never temporal.
- No qualifier added beyond what the source carries; program context already disambiguates, eg "Lihat Arus", not "Lihat Arus Listrik".
- A qualifier is added only where the Indonesian term would otherwise be genuinely ambiguous, eg "(φ=0)" appended to "sesaat" only when the source carries it.
- Accepted intra-domain homonyms: "radial" (noun radials / adjective radial); "port" (loanword, unambiguous); "kernel" (thin-wire kernel vs OS kernel, context disambiguates).
- Locative/other homonym collision: none recorded; "tanah" (earth/terrain) and "ground plane" (kept source-language) already separate the pair.
- Gerund vs noun: "penguatan" (process) vs "gain" (kept noun, paired with dBi) splits the overloaded source word "gain", per topic 10.
- Validation vs verification: translate the validation-tree feature as `validasi` and a verification check as `verifikasi`; never collapse them.

## 12. Cross-catalog consistency

- One-term-per-concept: reuse the topic 10 table catalog-wide; introduce no synonym for a concept already mapped.
- False-friend pairs (Appendix C) resolved, each side distinct:

| Side A | Term A | Side B | Term B |
|---|---|---|---|
| polarity | polaritas | polarization | polarisasi |
| magnitude | magnitudo | amplitude | amplitudo |
| peak value | Nilai Puncak | peak magnitude | Magnitudo Puncak |
| ground | ground plane | earth | tanah |
| load | beban | charge | muatan |
| gain (directivity) | gain/penguatan | amplification / profit | penguatan sinyal / keuntungan |
| current | arus | present/recent | terkini |
| charge | muatan | billing/fee | tagihan |
| wire | kawat | cable/cord / thread | kabel / thread |
| radiation pattern | pola radiasi | template/design / far-field | pola desain / medan jauh |
| excitation | eksitasi | emotional excitement | kegembiraan |
| node/antinode | simpul/perut | generic numeric null/zero | nol |
| scale family/color tone | keluarga skala | hue / palette kind / color projection | corak warna / jenis palet / proyeksi warna |
| comet | komet | geometry | geometri |
| identity | identitas | unity (Smith-chart) | kesatuan |
| renderer | renderer | render engine | mesin render (avoided) |
| override | menimpa | overwrite | menulis ulang |
| viewer | Viewer | observer/speaker/preview | pengamat/pembicara/pratinjau (avoided) |
| reflect | pantulkan | mirrors (behavioral) / reflection (physics) | mencerminkan / refleksi |
| structure | struktur | construction | konstruksi |
| theme | tema | topic/subject | topik/subjek |
| validation | validasi | verification checks | verifikasi |
| net gain | penguatan bersih | real-part gain | penguatan bagian riil |
| power (electrical) | daya | Power (transfer-family name) | Pangkat |
- Loanword-vs-native decision: established computing loanwords kept for consistency - `file` (not `berkas`), `klik`, `thread`, `byte`, `debug`, `verbose`, `parameter`, `frekuensi`; canonical spelling locked to these forms.
- Minority-outlier spellings (eg "berkas" for file) unify to the canonical catalog form ("file").
- Consistency priority outranks locale-form preference where a loanword is already established over a purist native alternative, eg "file" over "berkas", "ground plane"/"patch"/"tag" over native coinages.

## 13. Priority ordering

- Precedence chain: correct meaning, then interface convention, then catalog consistency, then disambiguation, then locale numeral form.
- Explicit overrides: "file" beats native purist "berkas" by catalog-consistency priority (topic 12); "ground plane", "patch", and "tag" kept as loanwords by domain-clarity priority ranking above native-coinage preference.

## 14. Grammatical number

- Indonesian nouns, adjectives, participles, and verbs are uninflected for grammatical number; use one invariant form.
- After an explicit literal count, use the bare noun without reduplication or a plural affix, eg `2 segmen`.
- Express plurality lexically only where source meaning requires it and no explicit count supplies it; do not invent reduplication merely to mirror source morphology.

## 15. Grammatical agreement

- Not applicable: Indonesian has no grammatical gender and no adjective/participle concord with a head noun; adjectives and participles take one invariant form regardless of the noun's number or class.

## 16. Morphological derivation

- Borrowed technical verbs/nouns: standard Indonesian verbal prefixes (`meN-`, `di-`) apply to a loanword root where a verb form is needed, eg "skalakan" (to scale) from "skala"; ad hoc source-language `-ing`/`-ed` suffixing on a loanword root is forbidden.
- Verbal-noun formation: `peN-...-an` suffix on the verb root, eg "penguatan" (gain, the process) from "kuat".
- Native-affix-vs-loanword preference: prefer a native affix on an established loanword root over inventing a new native root; compounding follows topic 1's noun+qualifier order.

## 17. Preposition and sandhi selection

- Not applicable: Indonesian has no phonologically conditioned form selection (no sandhi) and no elision/contraction rule required for this catalog's technical prose.

## 18. Card/record-label register

- Fixed designator form in dialog/editor titles: "Kartu [TAG]" (eg "Kartu GW") for a NEC2 record referenced as a titled entity.
- Running-prose form in messages: "kartu [tag]" lower-case, unhyphenated, eg "pada kartu GW ini".
- Generic-noun casing: "kartu" is lower-case in running prose, title-case only as the leading word of a dialog/editor title; no short-vs-long form distinction exists.
- Each register stays internally consistent: title form and prose form are never cross-converted within one string.

## 19. Multi-paragraph and whitespace fidelity

- Mirror source paragraph breaks at the same positions and preserve whether each break is blank-line or single-line.
- Preserve semantic line breaks; add no line break solely for visual wrapping.
- Drop a trailing clause removed from the current source rather than retaining stale translated text.
- Preserve each source trailing newline and punctuation mark exactly.
- Preserve complete meaning; do not truncate or abbreviate wording to satisfy an assumed display limit.

## 20. Current-source fidelity

- Derive every translation from the complete current source literal and its supplied context.
- Reuse inherited wording only when its complete meaning agrees with the current source.
- Unsafe inherited concepts: replace `berkas` with locked `file`, replace `Pengamat` with locked `Viewer`, and omit a stale `Anda` from a terse command whose current source does not address the user.

## 21. Script hygiene

- Wrong-script and cross-script homoglyph filtering is not applicable to ordinary Indonesian Latin prose.
- Use only Indonesian Latin letters in translated prose; allow other scripts or confusable characters only inside a topic 4 retained token whose exact spelling requires them.
- Translate plain foreign prose; retain only genuine identifiers, units, symbols, and proper technical names listed in topic 4.

## 22. Rule-file scope hygiene

- Keep only decisions that can alter translated-literal wording, Unicode characters, punctuation, capitalization, embedded tokens, mnemonic markers, or semantic whitespace.
- Omit catalog representation, serialization, headers, flags, workflow, rendering, fonts, shaping, layout, widget behavior, runtime formatting, sorting, search, display sizing, process metadata, audit history, completion state, review metadata, provenance, and citations.
- State each current decision directly; include no implementation procedure, correction history, bibliography, or alternative policy source.

## 23. Section-disjointness declaration

- Script mechanics (topic 1), phrasing/structure (topics 6, 7, 16), and address register (topic 8) are non-overlapping: topic 1 governs orthography/writing-system, topics 6-7 and 16 govern casing/word-choice/derivation, topic 8 governs formal-vs-informal address; a given concept lands in exactly one of these three axes.

## 24. Developer/debug-string policy

- User-facing strings: translate all commands, labels, dialogs, tooltips, status text, and errors into standard Indonesian under topics 7-8.
- Informational strings: translate notices, progress text, reports, and explanatory diagnostics into concise technical Indonesian.
- Developer-facing strings: translate debug and low-priority diagnostics into terse technical Indonesian; no subsystem family retains source prose.
- Preserve every embedded identifier, function name, format specifier, unit, symbol, and topic 4 retained token verbatim in every family.
- Review priority changes ordering only; it never permits an applicable user-facing, informational, or developer-facing literal to remain untranslated.
