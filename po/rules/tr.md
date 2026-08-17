# Turkish (tr-TR) Translation Rules for xnec2c

These rules govern modern standard Turkish in Turkey, written in the current
TDK Latin orthography, for professional electrical and radio-frequency
engineers, antenna designers, and amateur-radio operators.

## 1. Script and orthography

- Latin-based Turkish alphabet (29 letters): base Latin plus ç, ğ, ı (dotless),
  i (dotted), ö, ş, ü; diacritics mandatory, never stripped or folded to
  look-alikes.
- Use Unicode Normalization Form C for Turkish letters; preserve precomposed
  characters and replace decomposed base-plus-combining sequences so each
  literal has one canonical spelling.
- Never confuse dotted İ/i with dotless I/ı, including when casing a
  mnemonic letter (topic 9) or capitalizing a sentence-initial word.
- No script-specific joining/shaping mechanics; not applicable (Latin script).
- LTR script only; no bidirectional handling needed.
- Turkish alphabet has full upper/lower case distinction; casing rules
  (topic 6) apply normally, with the dotted/dotless split above as the sole
  hazard.
- Modern Turkish orthography (post-1928 Latin alphabet, current TDK
  standard); no regional variant in use.
- Standard word spacing; one space between native text and an embedded
  foreign/numeric/unit token, same as prose spacing elsewhere.
- Compounds: fused when lexicalized in TDK usage (eg "büyükkart" if ever
  lexicalized), otherwise spaced multi-word technical terms (eg "radyal
  teller"); do not hyphenate compounds absent an established precedent.

## 2. Numerals in literals

- Use Arabic digits (`0-9`) for technical values written in translated
  prose; no alternate digit set.
- Use a decimal comma (`50,0`) and a period for thousands grouping (`1.000`)
  in numbers physically present in Turkish prose.
- Keep formulas, examples, fixed defaults, and named mathematical or
  standards constants in their exact source form so their technical value
  remains unchanged.
- Literal digit-plus-suffix ordinals do not occur in this catalog; not
  applicable.
- Use a comma only as a decimal separator and punctuation fixed by the
  applicable expression as an index separator; never conflate the two.

## 3. Punctuation and quotation

- No distinct native quotation-mark convention is used in this catalog;
  retain plain/source quotes around embedded technical tokens for catalog
  consistency.
- Standard Western punctuation replaces no source counterpart; Turkish
  prose uses the same comma, question mark, and exclamation mark shapes as
  the source.
- Standard spacing around colon/semicolon/terminal punctuation: no space
  before, one space after, same as source convention.
- Ellipsis: use `...` (or U+2026) consistently; preserve source dash usage
  unchanged.
- Full-sentence tooltips, instructions, and dialogs take a terminal period
  or question mark; short button/menu/field labels omit the terminator.
- Punctuation inside embedded technical runs (mnemonics, format specifiers,
  file extensions) stays in source form.
- Graph/plot titles keep the compact `&` symbol as-is (eg "Maksimum Kazanç
  & Net Kazanç", "VSWR & S11", "G/Ta & TA"); do not spell it "ve" in a title.
- Natural-language dropdown/menu enumerations spell `&` as "ve" (eg "Büyük
  ve Küçük Eksen ve Toplam Kazanç", "Dikey, Yatay ve Toplam Kazanç").
- `vs` in chart titles renders as " - " (eg "Empedans - Frekans"); hold this
  across all frequency-plot titles.
- Dialog frame-label card parentheticals join mnemonics with "ve", not "&"
  (eg "(GN ve GD Kartları)", "(SP ve SC Kartları)", "(GW ve GC kartları)");
  reserve the compact `&` for graph/plot titles only.

## 4. Never-translate tokens

- NEC2 card mnemonics kept verbatim: core `GW GA GH EX LD FR RP GE EN` plus
  extended `SP SM SC NE NH NT TL GN GD GC GX GR GS GM GF CP CM SY XQ EK KH
  PQ PT`.
- Unit symbols kept verbatim: `MHz`, `dBi`, `dB`, `Ω`, `V`, `A`, `W`,
  `S/m`, `°` / `deg`, `%`, `K` (temperature).
- Figure-of-merit / parameter tokens kept verbatim: `VSWR`, S-parameter
  tokens (`S11`, `S21`, ...), `Z`, `Z0`, front/back-ratio and gain/
  temperature tokens (eg `F/B`, `G/Ta`, `TA`).
- File extensions kept verbatim: `.nec`, `.csv`, `.s1p`, `.s2p`, `.png`,
  `.gplot`.
- Format specifiers kept verbatim per topic 5.
- Embedded identifiers (function names, variable names, config keys) inside
  developer-facing strings are kept, not translated.
- Physical/mathematical symbol letters (field letters `E`/`H`, angle/
  exponent letters) stay in source form; no native look-alike substitute.
- Product/library/tool names kept verbatim: `xnec2c`, GTK, Cairo, OpenGL,
  and any chart-library proper name.
- Named transfer-function/algorithm terms kept verbatim: `Log`, `Asinh`,
  `μ-law`, `Reinhard`, `Sigmoid`, `Identity`; exception: the descriptive
  `Power` family name translates to "Güç" like any other scale-family label.
- Conditional loanwords, locked: `segment` kept as loanword "segment";
  `patch` translates to "yama"; `tag` translates to "etiket"; `fork`
  (process) kept verbatim as "fork".
- Preserve every token in this section exactly, without translation,
  transliteration, character substitution, omission, or addition; zero
  failures are permitted because each token carries technical identity.
- Keep every retained token left-to-right inside Turkish prose, following
  topic 1 without manual direction marks.

## 5. Format-specifier integrity

- Preserve every specifier from the source (`%s`, `%d`, `%f`, `%c`, `%%`,
  `%g`, `%lu`, `%llu`, etc.) in identical order and count; no addition or
  removal.
- Default order matches source order; positional reordering (`%1$s` style)
  only where the catalog format already permits it.
- Turkish is agglutinative/SOV: restructure the sentence around the fixed
  specifier positions rather than reordering specifiers.
- Preserve every digit written inside a format-specifier token exactly;
  topic 2 punctuation never alters the token.
- When a specifier is directly followed by a Turkish case/possessive
  suffix, separate with an apostrophe per Turkish orthography (eg `%s'yi`,
  `%d'e`) only where existing catalog convention already requires it.

## 6. Capitalization and title-case

- Menu items and labels: sentence case, capitalize only the first word and
  proper nouns/acronyms (`NEC2`, `VSWR`); Turkish does not capitalize common
  nouns mid-phrase.
- No axis-letter casing override beyond source convention; carry source
  casing for axis letters as-is.
- A lowercase math/coordinate variable (eg `x`, `y`, `z`) stays lowercase.
- No coordinated-option all-capitalized exception in this catalog.
- Generic card/record noun ("kart") stays lowercase mid-sentence, capitalized
  only sentence-initial; acronyms and proper nouns keep source casing.

## 7. Interface register by string type

- Commands/buttons/menu actions: bare imperative stem, matching the
  established catalog (eg "Sıfırla" = Reset, "Kapat" = Close, "Uygula" =
  Apply, "Ters Çevir" = Invert, "Çalıştır" = Execute); do not "correct" these
  to `-in` forms.
- Field labels: noun phrase plus retained colon.
- Dialogs and confirmations: full grammatical sentences in formal `siz`
  imperatives (`-in/-ın/-un/-ün`, eg "Seçin", "gösterin", "Bakın").
- Tooltips: full sentences; state the reason a disabled control is
  unavailable when the source does.
- Status/error messages: impersonal/declarative, matching source tone; no
  first-person phrasing.
- Commands use a dropped subject, verb-final imperative, and source-order
  information before the verb; field labels use a dropped subject and a
  concise noun phrase with the head after its modifiers.
- Dialogs and confirmations use formal finite clauses with dropped subjects,
  objects before verbs, and the decision or requested action last; tooltips
  present the control or condition before its effect or unavailable reason.
- Status and error messages use dropped subjects where natural, place known
  context before the result, and end with the declarative verb or predicate.
- Use the shortest complete natural form; prefer established compounds or
  concise multiword terms without omitting meaning, inventing abbreviations,
  or imitating source length.
- Name user-visible domain entities as modifier-plus-head noun phrases; keep
  proper-name and personal-name policy in topic 8.
- Developer/debug-string register is topic 24.

## 8. Formality and address

- Turkish distinguishes formal (`siz`) and informal (`sen`) address; the
  interface uses formal `siz`-register only; `sen`-register is forbidden.
- Subject pronouns are dropped by default (Turkish is pro-drop); formality
  is carried by the verb suffix, not an explicit pronoun.
- Turkish lacks grammatical gender and animacy agreement; use gender-neutral
  nouns and dropped subjects so references remain natural and inclusive.
- Use no honorific in interface text; address users in second-person plural
  formal morphology, and retain a person's own given-name-plus-family-name
  order when a personal name occurs.
- Formality realized morphologically via the `-in/-ın/-un/-ün` imperative
  suffix family for `siz`-register commands.
- Register splits by string type per topic 7: bare-stem for commands, full
  `siz`-suffixed sentences for dialogs/tooltips/confirmations.
- Confirmation-dialog pattern: formal 2nd-person-plural verb ending
  (`-manız`/`-eceğinizden emin misiniz`), eg "xnec2c'den çıkmak
  istediğinizden emin misiniz?".
- Prohibited registers: casual/slang, over-formal, commercial, archaic.

## 9. Accelerator/hotkey mnemonics

- One underscore-prefixed mnemonic letter per label, chosen from the
  Turkish translation itself, never copied or transliterated from the source.
- Per-container uniqueness: shift the underscore to a different letter in
  the Turkish word on collision; never invent a letter absent from the
  translated text.
- Mnemonic letter always comes from the translated term.
- No parenthetical-Latin-letter convention needed; Turkish is already Latin
  script, not applicable.
- Never place a mnemonic on a string the source carries none for.
- Mnemonics apply to buttons, menu items, and tab labels; never to
  read-only labels, status text, or tooltips.
- Avoid dotless-ı/dotted-i confusion (topic 1) and avoid ğ/ş/ç as mnemonic
  letters where an ASCII alternative in the same word exists, for
  typability on a standard Turkish keyboard.

## 10. Domain lexicon

| Concept (sense) | Turkish | Purpose / hazard |
|---|---|---|
| current (electrical) | akım | never "şu anki" (temporal) |
| charge (electrical) | yük | catalog precedent ("Charges"→"Yükler"); accepted homonym with load, see topic 11 |
| voltage | gerilim | native term, not loanword "voltaj" |
| power (electrical, watts) | güç | distinct from `Power` scale-family name, same spelling, context-disambiguated |
| impedance | empedans | distinct from resistance/reactance |
| resistance | direnç | real part of Z; distinct from empedans and yük (load) |
| reactance | reaktans | imaginary part of Z |
| inductance | endüktans | canonical catalog term for this source concept |
| capacitance | kapasitans | canonical catalog term for this source concept |
| conductivity | iletkenlik | material S/m |
| admittance | admitans | distinct from empedans |
| load (LD card) | yük / yükleme | "Yük Türü" = Loading Type, "Yük Komutu" = Loading Command; homonym with charge, see topic 11 |
| gain (antenna) | kazanç | never "kâr" (profit), never "amplifikasyon" (amplifier) |
| excitation | uyarım | EM input, not "heyecan" (emotional) |
| feedpoint | besleme noktası | canonical catalog term for this source concept |
| port | port | established loanword |
| radials (noun) | radyal teller | distinct from adjective "radyal" |
| ground / ground plane | toprak / toprak düzlemi | RF reference; not soil; "toprak düzlemi" not "toprak" alone |
| earth (physical medium) | zemin | terrain/noise-model earth; distinct from electrical "toprak" |
| ground wave | yer dalgası | canonical catalog term for this source concept |
| wire | tel | not "kablo" (cable/cord) |
| segment | segment | established loanword, kept |
| patch | yama | canonical catalog term for this source concept |
| tag | etiket | canonical catalog term for this source concept |
| kernel | çekirdek | not an OS kernel |
| cliff | uçurum | two-medium ground-boundary type |
| structure | yapı | not "inşaat" (construction) |
| model | model | canonical catalog term for this source concept |
| geometry | geometri | canonical catalog term for this source concept |
| crossed | çaprazlanmış | transmission-line conductors reversed; not "kesilmiş" (cut) |
| field (EM) | alan | distinct from a data/config "field" |
| near field / far field | yakın alan / uzak alan | opposed pair |
| far-field contribution | uzak alan katkısı | canonical catalog term for this source concept |
| radiation | ışıma | canonical catalog term for this source concept |
| radiation pattern | örüntü | plotted directional response; not "şablon" (template) |
| gain pattern | kazanç örüntüsü | canonical catalog term for this source concept |
| polarization | polarizasyon | distinct from kutupluluk (polarity) |
| polarity | kutupluluk | sign +/-; never conflated with polarizasyon |
| phase / reference phase | faz / referans faz | canonical catalog term for this source concept |
| frequency | frekans | canonical catalog term for this source concept |
| wave / wavelength | dalga / dalga boyu | canonical catalog term for this source concept |
| standing wave / traveling wave | duran dalga / ilerleyen dalga | opposed pair |
| node / antinode | düğüm / karşı düğüm | standing-wave zero/max; distinct from a generic numeric zero |
| crest | dalga tepesi | comet-head instantaneous apex; distinct from tepe değeri/tepe noktası |
| magnitude | büyüklük | modulus/scalar; distinct from genlik |
| impedance magnitude (\|Z\|) | büyüklük | never "genlik" here |
| amplitude | genlik | oscillating peak; distinct from büyüklük |
| peak value | tepe değeri | distinct UI option from tepe büyüklüğü |
| peak magnitude | tepe büyüklüğü | distinct UI option from tepe değeri |
| instantaneous | anlık | add "(φ=0)" only where source carries it |
| Poynting vector | Poynting vektörü | proper name kept |
| solid angle | katı açı | canonical catalog term for this source concept |
| net gain | net kazanç | total-minus-mismatch; distinct from gerçel kazanç (real-part gain) |
| viewer | görüntüleyici | observation direction / 3D view widget; not gözlemci/konuşmacı/önizleme |
| flow / flow direction | akış / akış yönü | canonical catalog term for this source concept |
| total field | toplam alan | canonical catalog term for this source concept |
| color | renk | canonical catalog term for this source concept |
| color projection | renk projeksiyonu | which quantity drives hue |
| hue | ton | color-wheel angle; not "renk" alone |
| brightness | parlaklık | luminance channel |
| hue encoding / brightness encoding | ton kodlaması / parlaklık kodlaması | distinct enums, neither collapses to renk projeksiyonu |
| color scale | renk ölçeği | magnitude-to-color |
| scale family / color tone | renk tonu | transfer-curve family (Power/Log/Asinh/μ-law/Reinhard/Sigmoid/Identity); one term, two source spellings |
| palette / palette kind | palet / palet türü | distinct from renk tonu and renk projeksiyonu |
| ramp / gradient | rampa / gradyan | distinct pair, kept separate despite visual similarity |
| gamma | gama | power-law exponent |
| knee | diz noktası | soft-knee bend point |
| softening | yumuşatma | canonical catalog term for this source concept |
| compression | sıkıştırma | dynamic-range |
| contrast | kontrast | canonical catalog term for this source concept |
| dynamic range | dinamik aralık | canonical catalog term for this source concept |
| floor | taban | min/lower clamp; not a room floor |
| envelope | zarf | magnitude/amplitude envelope |
| comet | kuyrukluyıldız | moving-crest overlay; not geometri |
| overlay (noun) | kaplama | eg "_Overlay Structure"→"_Yapıyı Kapla", "overlay shader"→"kaplama gölgelendiricisi" |
| animate / animation | canlandır / canlandırma | canonical catalog term for this source concept |
| animated / static | canlı / durağan | category-header adjectives |
| projection | projeksiyon | color or geometry, context-disambiguated |
| scale (noun/verb) | ölçek / ölçekle | "ölçekle" only for the imperative verb sense |
| wireframe | tel kafes | canonical catalog term for this source concept |
| identity (no-op transfer) | özdeşlik | distinct from "birim" (unity, Smith-chart) |
| sentinel | nöbetçi değer | unreachable-case guard |
| bins | kutular | discretization buckets |
| companding | sıkıştırma-genişletme | bounded log curve (μ-law), distinct from plain sıkıştırma |
| tone mapping | ton eşleme | photographic tone-map |
| renderer | çizici | drawing backend; distinct from "render motoru" |
| shader | gölgelendirici | canonical catalog term for this source concept |
| allocation / managed allocator | bellek ayırma / yönetilen ayırıcı | canonical catalog term for this source concept |
| thread | iş parçacığı | compute thread |
| widget | bileşen | UI element |
| validation | doğrulama | validation-tree feature |
| batch mode | toplu kip | canonical catalog term for this source concept |
| fork (process) | fork | kept verbatim, see topic 4 |
| deadlock | kilitlenme | canonical catalog term for this source concept |
| notifier | bildirici | canonical catalog term for this source concept |
| token / operand / operator / arity | simge / işlenen / işleç / aritet | expression-parser terms |
| override | geçersiz kıl | supersede a value; distinct from "üzerine yaz" (overwrite) |
| swap | değiştir / takas et | canonical catalog term for this source concept |
| theme | tema | UI/color theme; distinct from "konu" (topic/subject) |
| noise / noise temperature | gürültü / gürültü sıcaklığı | electronic/thermal; not acoustic |
| efficiency | verim | canonical catalog term for this source concept |
| interpolation | enterpolasyon | canonical catalog term for this source concept |
| mnemonic (card code) | anımsatıcı | card's code descriptor; not a memo/note |
| degrees / deg | derece / "(derece)" | freestanding prose vs parenthetical unit tag, treated like other unit tags |
| diameter | çap | native term, locked over loanword "diametre" |
| reflect (geometry mirror) | yansıt | geometric mirror operation |
| reflect (behavioral, "mirrors") | izler | tracks another control's value |
| reflect (physics) | yansıma | noun, physical reflection |
| default(s) | varsayılan | fallback value |
| normalize / normalization | normalleştir / normalleştirme | native term, not transliterated "normalize et" |
| real (part, complex) | gerçel | TDK native term; never "reel" nor "gerçek" (genuine) |
| imaginary (part, complex) | sanal | TDK native term; never "imajiner" |
| major axis / minor axis | büyük eksen / küçük eksen | polarization-ellipse axes; never "ana/yan eksen" |
| visualization | görselleştirme | canonical catalog term for this source concept |
| range (dynamic range, dB) | aralık | not "açı" (angle) |
| file / window / settings / error | dosya / pencere / ayarlar / hata | canonical catalog term for this source concept |

## 11. Disambiguation policy

- Use the correct technical sense per the topic-10 table without appending
  extra qualifying words absent from the source; the EM-simulator
  context disambiguates for the RF-engineer audience.
- Add a qualifier only where the target term would otherwise be genuinely
  ambiguous; none currently required beyond the table's stated forms.
- Accepted intra-domain homonym: "yük" covers both charge (electrical) and
  load/loading (LD card); keep both uses as-is, disambiguated by program
  context (button/card name), rather than inventing a separate qualified
  term.
- No locative/other homonym collision recorded.
- No gerund-vs-noun overload requiring a distinct pair in this catalog.

## 12. Cross-catalog consistency

- One term per concept, reused from the topic-10 table; no synonym
  introduced for an already-mapped concept.
- False-friend pairs, each side distinct and never sharing a translation:
  polarity/kutupluluk vs polarization/polarizasyon; magnitude/büyüklük vs
  amplitude/genlik; peak value/tepe değeri vs peak magnitude/tepe
  büyüklüğü; ground/toprak vs earth/zemin; load-charge homonym/yük (topic
  11, accepted exception); gain/kazanç vs amplification/amplifikasyon vs
  profit/kâr; current/akım vs temporal/güncel; charge/yük vs billing/ücret;
  wire/tel vs cable/kablo and vs thread/iş parçacığı; radiation
  pattern/örüntü vs template/şablon and vs far-field/uzak alan;
  excitation/uyarım vs emotional/heyecan; node-antinode/düğüm-karşı düğüm
  vs generic zero/sıfır; scale family/renk tonu vs hue/ton vs palette
  kind/palet türü vs color projection/renk projeksiyonu; comet/kuyrukluyıldız
  vs geometry/geometri; identity/özdeşlik vs unity/birim; renderer/çizici vs
  render engine/render motoru; override/geçersiz kıl vs overwrite/üzerine
  yaz; viewer/görüntüleyici vs observer/gözlemci, speaker/konuşmacı,
  preview/önizleme; reflect senses (topic 10); structure/yapı vs
  construction/inşaat; theme/tema vs topic/konu; validation/doğrulama vs
  verification checks/sağlama; net gain/net kazanç vs real-part gain/gerçel
  kazanç; power/güç vs Power-family/Güç.
- Loanword-vs-native decisions locked per topic 10 (eg empedans, reaktans,
  admitans as loanwords; gerilim, direnç, çap, normalleştir as native
  terms); minority-outlier spellings unify to the locked form.
- Consistency priority outranks locale-form preference for: segment (kept
  as loanword despite a native alternative existing), and gerçel/sanal
  (native forms locked over the more common loanword "reel"/"imajiner").

## 13. Priority ordering

- Precedence chain: correct meaning, then interface convention (topic 7-8),
  then catalog consistency (topic 12), then disambiguation (topic 11), then
  locale numeral form (topic 2).
- Explicit override: gerçel/sanal (correct-meaning/native-term priority)
  outrank the more common loanword spelling despite lower locale
  familiarity.

## 14. Grammatical number

- Turkish distinguishes singular and plural; form noun plurals with
  `-lar`/`-ler` under vowel harmony, while adjectives and participles remain
  uninflected and finite verbs normally remain singular with inanimate
  plural subjects.
- After an explicit literal count, keep the noun singular; do not add a
  plural marker.
- UI labels use singular unless plurality is semantically required; use a
  plural for a coordinated set of distinct cards, eg `(GN ve GD Kartları)`.

## 15. Grammatical agreement

- Turkish has no grammatical gender, declension-class concord, or
  partitive-after-count rule; those features are not applicable.
- Adjectives and attributive participles do not agree with the head noun in
  gender or number; keep them uninflected in phrases and standalone labels.
- Resolve an omitted head noun from the source context without adding gender
  marking; finite predicates follow topic 14's number rule.
- Retain a reflexive particle when the Turkish construction requires one;
  omission changes the action's participant structure.

## 16. Morphological derivation

- Form a borrowed technical verb with the established Turkish derivational
  suffix when one exists, eg `normalleştir`; otherwise use the loanword plus
  `etmek`. Do not mix both patterns for one concept.
- Native verbal-noun formation via the "-me/-ma" suffix (eg "ölçekleme" =
  scaling, "canlandırma" = animation).
- Prefer native-affix derivation over a loanword equivalent where a native
  form is already locked in topic 10 (eg "normalleştirme" over "normalize
  etme"); compounding follows topic 1's spaced/fused rule.

## 17. Preposition and sandhi selection

- Vowel harmony governs suffix vowel selection (front/back, rounded/
  unrounded) on every attached suffix; apply the harmonic form matching the
  stem's last vowel.
- Consonant assimilation (devoicing) applies at morpheme boundaries per
  standard Turkish orthography (eg "kitap"→"kitabı").
- Use an apostrophe before an affix attached to a proper name, abbreviation,
  literal digit, or format specifier, eg `%s'yi`; attach affixes directly to
  common loanwords.
- Turkish has no article elision or preposition contraction; not applicable.

## 18. Card/record-label register

- Dialog/editor title designator: `<MNEMONIC> Kartı` (eg "GW Kartı").
- Running-prose form: lowercase suffixed mnemonic (eg "GW kartı", "GW ve GC
  kartları" for multiple).
- Generic noun "kart" stays lowercase mid-sentence; no short-vs-long form
  distinction beyond singular/plural (topic 14).
- Each register (title-case designator vs running-prose form) stays
  internally consistent; never cross-converted within one string type.

## 19. Multi-paragraph and whitespace fidelity

- Mirror source paragraph breaks (blank-line vs single-line) at the same
  positions in the translation.
- Drop a clause removed from the current source rather than preserving it
  from a stale prior translation.
- Carry trailing newline and punctuation through mechanically, unchanged.
- Preserve semantic line breaks at source positions; introduce no line break
  for visual wrapping.
- Preserve complete meaning and natural wording without truncation or an
  abbreviation based on an assumed display constraint.

## 20. Current-source fidelity

- Derive every translation from the current source literal and its supplied
  context.
- Reuse an inherited translation only when its complete meaning matches the
  current source.
- Do not inherit prior card names, command registers, or singular/plural
  forms when the current source changed those concepts.

## 21. Script hygiene

- Permit zero incorrect `İ/i` and `I/ı` casing pairs in Turkish prose;
  retained tokens alone keep their literal source characters.
- No additional confusable-script class applies to Turkish Latin prose.
- Translate plain foreign prose; preserve only genuine identifiers, units,
  and other topic-4 tokens.

## 22. Rule-file scope hygiene

- Keep only current decisions that can alter wording, Unicode characters,
  punctuation, capitalization, embedded tokens, mnemonic markers, or
  semantic whitespace in a translated literal.
- Omit catalog representation, serialization, headers, flags, workflow,
  rendering, fonts, layout, widget behavior, runtime formatting, sorting,
  search, display sizing, process metadata, history, completion state,
  provenance, and source citations.
- State each retained translation decision directly; include no procedure,
  bibliography, correction history, or alternative policy source.

## 23. Section-disjointness declaration

- Script mechanics (topics 1, 2, 3, 9, and 21) govern characters,
  orthography, punctuation, and mnemonic marks only.
- Phrasing and structure (topics 5, 6, 7, 13, 14, 16, 17, 18, and 19)
  govern token order, casing, morphology, and semantic whitespace only.
- Address register (topic 8) governs person, formality, honorifics, and
  inclusive reference only.
- Apply each decision on one axis only; a cross-reference imports the result
  without restating its rule.

## 24. Developer/debug-string policy

- Developer-facing/low-priority diagnostic strings translate to Turkish,
  following the same catalog conventions as user-facing strings.
- Identifier, function, and format tokens inside such strings are kept
  verbatim regardless (topic 4).
- Register: terse and technical, matching source tone; no subsystem family
  overrides this default.
- Classify commands, dialogs, tooltips, labels, statuses, and errors as
  user-facing and translate them; classify explanatory notices as
  informational and translate them; classify function-level diagnostics and
  internal state reports as developer-facing and translate them tersely.
- Review priority never permits an applicable user-facing, informational, or
  developer-facing literal to remain untranslated.
- Every translated family preserves embedded format specifiers and topic-4
  tokens regardless of its review priority.
- Render the `BUG:` diagnostic prefix as "KUSUR:", held distinct from the
  "HATA:" prefix that carries ordinary failures; the two severities never
  share a word.
- Keep an identifier-plus-value diagnostic pair contiguous (eg
  `sweep_state=%d`); a qualifier precedes the whole pair and never stands
  between the identifier and its `=`.
