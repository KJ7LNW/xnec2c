# ja translation rules

## 1. Script and orthography

- Scope: modern Japanese for Japan, written in the standard Japanese mixed
  script and intended for professional electrical and RF engineers, antenna
  designers, and amateur-radio operators.
- Use kanji for established content terms, hiragana for particles and
  inflection, and katakana for the topic-10 loanwords; use Latin and Greek
  only in retained technical tokens under topic 4.
- No letter-case distinction: casing rules reduce to mnemonic-letter handling
  only (topic 9).
- Precomposed form only: dakuten/handakuten (eg ガ, パ) and the katakana
  long-vowel mark ー are precomposed code points, never a base kana plus a
  combining mark.
- Ambiguous-mark policy: the long-vowel mark is U+30FC (KATAKANA-HIRAGANA
  PROLONGED SOUND MARK), never a hyphen-minus or ideographic dash look-alike
  (eg ビューア "viewer" uses ー, not -).
- No word-part joiners, positional letter forms, or ligatures; not
  applicable to this script.
- Directionality: left-to-right, horizontal only; vertical writing (縦書き)
  is not used in this interface.
- Orthographic standard: modern orthography (現代仮名遣い); no historical
  kana spellings.
- No inter-word spacing; particles attach directly to the word they mark.
  Half-width space separates native text from an embedded Latin/numeral
  token only where the source string already does (eg "10 MHz").
- Compound formation: fused, no spacing and no hyphen, for kanji and
  katakana compounds alike (eg 構造ジオメトリ, 遠方界寄与); a hyphen appears
  only inside a retained technical token (eg μ-law).

## 2. Numerals and locale data

- Half-width (ASCII) digits for all technical values; full-width digits
  (eg １２３) are forbidden.
- Decimal separator: period "." — never the Japanese-locale comma variant.
  Exceptions keep the source form unchanged: runtime format-specifier
  output, literal formula/example tokens, fixed default values, and named
  mathematical/standards constants.
- Thousands/grouping separator: comma ",", matching source.
- No digit-plus-native-affix ordinal/index notation in this catalog; where
  an index needs a separator (eg item numbering), use ASCII "." or ")" per
  existing entries, never confused with the decimal separator.

## 3. Punctuation and quotation

- Native quotation marks 「」 are used only inside full natural-language
  prose (dialog/tooltip sentences) that itself requires quoting; embedded
  technical tokens keep their source plain/straight quotes or parentheses
  unchanged, for round-trip consistency with topic 4 tokens.
- 、 and 。 replace the source comma and period inside full sentences
  (dialogs, tooltips, confirmations). Short UI labels, menu items, and
  log/error strings keep source-style punctuation (colons, parentheses,
  ASCII quotes) and do not insert 、 or 。.
- No space before or after 、 。 or an ASCII colon/semicolon used in a
  label (eg "周波数:").
- Ellipsis: 「…」 (single Unicode ellipsis character), not three ASCII
  periods. Source dashes (en/em dash) in prose are retained as "-" per
  existing catalog usage; no separate native dash form is introduced.
- Sentence-terminator policy: full grammatical sentences (dialogs,
  confirmations, tooltips) end in 。; short labels, menu items, and
  fragments omit it, matching topic 7's per-string-type register.
- Punctuation inside an embedded technical run (a mnemonic, unit, path, or
  format specifier) stays in its source form regardless of surrounding
  Japanese punctuation.

## 4. Never-translate tokens

Require zero translation, transliteration, case change, character change, or
internal-spacing change in every retained token below.

- Keep NEC2 card mnemonics verbatim: GW GA GH EX LD FR RP GE EN SP SM SC NE
  NH NT TL GN GD GC GX GR GS GM GF CP CM SY XQ EK KH PQ PT.
- Keep unit symbols verbatim: Hz, kHz, MHz, GHz, dB, dBi, Ω, W, K, S/m, °,
  deg, and %.
- Keep figure-of-merit and parameter tokens verbatim: VSWR, S-parameters,
  S11, S12, S21, S22, Z, Z0, F/B, and G/T.
- Keep file extensions verbatim: .nec, .csv, .s1p, .s2p, and .png; retain a
  complete literal filename character-for-character.
- Keep every source format specifier verbatim under topic 5.
- Keep embedded function names, variable names, configuration keys, and code
  identifiers verbatim in developer strings.
- Keep physical and mathematical symbol letters verbatim: E, H, Z, γ, φ,
  and μ.
- Keep product, library, toolkit, and chart proper names verbatim: xnec2c,
  NEC2, GTK, GSL, OpenGL, and Smith.
- Keep named transfer functions verbatim: Log, Asinh, μ-law, Reinhard,
  Sigmoid, and Identity; translate descriptive Power as べき乗.
- Translate geometry loanwords consistently as ワイヤ, セグメント, パッチ,
  and タグ; render process fork as フォーク. These conditional terms are
  fixed topic-10 choices, not retained source tokens.
- Keep retained tokens left-to-right within Japanese text and apply no manual
  direction override.

## 5. Format-specifier integrity

- Every specifier present in the source string is preserved, same count and
  same conversion set (eg %s, %d, %.2f).
- Default ordering matches the source; positional reordering (eg %1$s) is
  used only where Japanese verb-final word order genuinely requires a
  different specifier sequence than the source.
- Prefer restructuring the sentence around the specifiers' fixed positions
  over reordering the specifiers themselves; reorder only when
  restructuring cannot produce natural Japanese.
- Numbers substituted through a specifier at runtime are never localized in
  the translated string; only the surrounding literal text is translated.

## 6. Capitalization and title-case

- Japanese has no letter-case distinction: write labels, menu items, and
  titles in their canonical topic-10 form without source-style title case.
- Keep axis letters, acronyms, proper nouns, retained tokens, and lowercase
  mathematical or coordinate variables in the exact source case; this
  preserves notation rather than applying a Japanese casing rule.
- Coordinated option names use the same canonical form for every element;
  no element receives case-based emphasis.
- カード has no positional case change; retain each prefixed mnemonic's
  exact case under topic 18.

## 7. Interface register by string type

Treat interface literals as compact technical Japanese; preserve all source
meaning while applying the syntax fixed below.

- Commands, buttons, and menu actions use a noun phrase or verbal noun; omit
  the subject and imperative ending, place modifiers before the head, and
  place the action concept last.
- Field labels use a modifier-before-head noun phrase followed immediately
  by an ASCII colon; omit subject, verb, and particle before the colon.
- Dialogs and confirmations use complete です/ます sentences; omit an
  inferable subject, place conditions and objects before the final predicate,
  and present context before requested action or result.
- Tooltips use complete です/ます sentences in reason-then-effect order;
  when the source explains an unavailable control, retain that reason.
- User-facing status and error messages use impersonal declarative sentences;
  state the affected entity before its state or failure and omit an
  inferable agent.
- Developer and debug strings follow topic 24 rather than the user-facing
  sentence register.
- User-visible domain entities use modifier-before-head order and the locked
  topic-10 head term, eg 放射パターン; personal names follow topic 8.
- Prefer the shortest complete established compound; omit no meaning, invent
  no abbreviation, and do not imitate source length or word order.

## 8. Formality and address

- Use polite です/ます morphology for user-facing dialogs, confirmations,
  tooltips, status messages, and error messages; prohibit casual だ/である,
  slang, commercial, archaic, and over-formal honorific prose.
- Omit first- and second-person pronouns when the subject is inferable; use
  impersonal constructions rather than addressing the operator directly.
- Japanese grammatical gender and gender agreement are not applicable;
  choose role-neutral nouns and omit unnecessary gender marking.
- Inclusive wording uses role or action terms without assumed gender,
  animacy, or social status; required number remains unmarked under topic 14.
- Honorific titles are not applicable to interface actions or diagnostics;
  when a source literal names a person, preserve the supplied personal-name
  order and honorific content without inventing either.
- Commands remain nominal under topic 7; dialogs and confirmations end in
  polite finite predicates; developer strings use topic 24's plain technical
  register.
- Confirmation dialogs state the action and requested confirmation in one
  complete polite sentence rather than a bare response fragment.

## 9. Accelerator/hotkey mnemonics

- Preserve each source mnemonic as an appended parenthetical ASCII marker
  `(_X)` immediately after the translated label; this provides a typable
  mnemonic for a script without case-bearing letters.
- Select `X` from a stable mnemonic association with the translated Japanese
  concept, using its established romanized initial or an unambiguous ASCII
  association; do not copy an unrelated source-word initial.
- Keep the marker separate from kana and kanji; never underline or replace a
  native character.
- Preserve source presence exactly: add no marker where the source has none
  and drop none where the source has one.
- Use only unambiguous ASCII letters or digits and keep markers distinct
  within each menu or dialog; resolve a collision with another association
  to the translated concept.

## 10. Domain lexicon

Established lexicon — reuse these exactly, do not introduce synonyms.

### Electrical primitives

| Concept | Term | Sense / purpose |
|---|---|---|
| current | 電流 | electrical current; never the temporal "current/recent" sense |
| charge | 電荷 | electrical charge; never billing/fee |
| voltage | 電圧 | electric potential |
| power (electrical) | 電力 | radiated/dissipated watts, power gain, power flow; distinct from Power curve family below and from math べき乗 |
| Power (curve family) | べき乗 | transfer-function family name; the stated exception that translates (topic 4), distinct from 電力 |
| impedance | インピーダンス | complex Z; distinct from 抵抗 and リアクタンス |
| resistance | 抵抗 | real part of Z |
| reactance | リアクタンス | imaginary part of Z |
| inductance | インダクタンス | canonical engineering term; preserves one catalog term per concept |
| capacitance | キャパシタンス | parallel loanword form with インダクタンス |
| conductivity | 導電率 | native term, material S/m |
| admittance | アドミタンス | distinct from インピーダンス |
| load | 負荷 | LD-card impedance load; not physical weight; not a homonym with 電荷 in Japanese |
| gain | 利得 | antenna directivity ratio (dB); never profit, never amplifier 増幅 |
| excitation | 励振 | EM energy input/source; never emotional 興奮 |
| feedpoint | 給電点 | antenna feed point |
| port | ポート | excitation/S-parameter port |
| radials | ラジアル | ground-plane radial wires (noun); distinct from the adjective 放射状の |

### Ground and earth

| Concept | Term | Sense / purpose |
|---|---|---|
| ground / ground plane | グラウンド | RF electrical reference plane, GN/GD cards; never soil; always full form グラウンド, never contracted グランド, catalog-wide |
| earth (physical medium) | 大地 | terrain/noise-model earth, "below ground" geometry; distinct from グラウンド |
| ground wave | 地表波 | propagation term; distinct from グラウンド |

### Geometry primitives

| Concept | Term | Sense / purpose |
|---|---|---|
| wire | ワイヤ | thin conductor / GW element; never cable/cord ケーブル; not a homonym with thread スレッド |
| segment | セグメント | NEC2 geometry subdivision |
| patch | パッチ | NEC2 surface patch (SP/SM); loanword decision held file-wide |
| tag | タグ | NEC2 geometry identifier; never a UI label or a card |
| card | カード | NEC2 input record; register handled in topic 18 |
| kernel | カーネル | thin-wire integral-equation kernel; never an OS kernel |
| cliff | クリフ | two-medium ground-boundary type; kept as technical loanword to avoid the "fracture/break" reading of 崖 |
| structure | 構造 | the antenna model geometry; never construction 構築 |
| model | モデル | NEC model or noise-temperature model |
| geometry | ジオメトリ | the model geometry |
| crossed | 交差した | transmission-line conductors crossed/reversed; never cut/severed 切断された |

### Field, pattern, viewer

| Concept | Term | Sense / purpose |
|---|---|---|
| field (EM) | 界 | near/total/E/H field; distinct from a UI/config フィールド |
| near field | 近傍界 | opposed to 遠方界 |
| far field | 遠方界 | opposed to 近傍界 |
| far-field contribution | 遠方界寄与 | per-direction contribution; not near-field animation |
| radiation | 放射 | radiated emission |
| radiation pattern | 放射パターン | plotted directional response; never a template/design テンプレート |
| gain pattern | 利得パターン | the gain radiation pattern |
| polarization | 偏波 | antenna/wave field orientation; distinct from 極性 |
| polarity | 極性 | sign (+/-) of a quantity; false friend of 偏波, never conflated |
| phase | 位相 | canonical engineering term; preserves one catalog term per concept |
| reference phase | 基準位相 | animation reference phase |
| frequency | 周波数 | FR card frequency |
| wave / wavelength | 波 / 波長 | canonical engineering term; preserves one catalog term per concept |
| standing wave / traveling wave | 定在波 / 進行波 | opposed pair |
| node / antinode | 節 / 腹 | standing-wave zero/maximum; standard physics terms, not a generic numeric null/zero |
| crest | 波頭 | instantaneous wave apex (comet-head); distinct from a curve/step peak ピーク |
| magnitude | 大きさ | modulus of a quantity (\|Z\|, current, charge); never 振幅 |
| amplitude | 振幅 | oscillating-quantity peak; distinct from 大きさ, used only where source says "amplitude" |
| peak value | ピーク値 | one of two distinct UI options; never collapsed with peak magnitude |
| peak magnitude | ピークの大きさ | the other UI option; built on the 大きさ root to stay distinct from ピーク値 |
| instantaneous | 瞬時値 | projection mode; "(φ=0)" qualifier added only where source carries it |
| Poynting vector | ポインティングベクトル | canonical engineering term; preserves one catalog term per concept |
| solid angle | 立体角 | canonical engineering term; preserves one catalog term per concept |
| net gain | 正味利得 | total-minus-mismatch gain; never ネットゲイン, never 実数利得 |
| viewer | ビューア | observation direction / 3D view widget; never 観測者, 話者, or a preview プレビュー |
| flow / flow direction | フロー方向 | patch/current flow |
| total field | 全界 | radiation total field |

### Color, tone, animation subsystem

| Concept | Term | Sense / purpose |
|---|---|---|
| color | 色 | canonical engineering term; preserves one catalog term per concept |
| color projection | 色投影 | which quantity drives hue; distinct from 階調 and パレット種別 |
| hue | 色相 | color-wheel angle |
| brightness | 明度 | luminance channel |
| hue encoding | 色相エンコーディング | distinct enum from 明度エンコーディング and from 色投影 |
| brightness encoding | 明度エンコーディング | distinct enum from 色相エンコーディング |
| color scale | カラースケール | magnitude-to-color scale |
| scale family / color tone | 階調 | transfer-curve family (Power/Log/Asinh/μ-law/Reinhard/Sigmoid/Identity); one concept, one term for both source spellings |
| palette / palette kind | パレット / パレット種別 | palette-layout enum; distinct from 階調 and 色投影 |
| ramp / gradient | グラデーション | a palette kind / linear color strip |
| gamma | ガンマ | power-law exponent |
| knee | ニーポイント | soft-knee bend point |
| softening | ソフトニング | dynamic-range softening |
| compression | 圧縮 | dynamic-range compression |
| contrast | コントラスト | canonical engineering term; preserves one catalog term per concept |
| dynamic range | ダイナミックレンジ | canonical engineering term; preserves one catalog term per concept |
| floor | フロア | minimum/lower clamp (brightness/dB floor); kept katakana to avoid the physical-floor reading of 床 |
| envelope | エンベロープ | magnitude/amplitude envelope |
| comet | コメット | moving-crest overlay effect; never confused with geometry ジオメトリ |
| overlay (noun) | オーバーレイ | an added visual layer; distinct from the verb 重ねる |
| animate / animation | アニメーション化 / アニメーション | canonical engineering term; preserves one catalog term per concept |
| animated / static | アニメーション / 静的 | category-header adjectives (dynamic vs phase-invariant) |
| projection | 投影 | color or geometry projection |
| scale (verb) | 拡大縮小する | canonical engineering term; preserves one catalog term per concept |
| scale (noun) | スケール | canonical engineering term; preserves one catalog term per concept |
| wireframe | ワイヤフレーム | wire-mesh render mode |
| identity | Identity | retained no-op/passthrough transfer-family name; distinct from 単位 (unity, Smith chart) |
| sentinel | センチネル | unreachable-case guard value |
| bins | ビン | discretization buckets |
| companding | コンパンディング | bounded log curve (μ-law) |
| tone mapping | トーンマッピング | photographic tone-map; distinct from 階調 (scale family) |

### Render and compute

| Concept | Term | Sense / purpose |
|---|---|---|
| renderer | レンダラー | drawing backend; never render engine レンダーエンジン |
| shader | シェーダー | canonical engineering term; preserves one catalog term per concept |
| allocation (memory) / managed allocator | メモリ確保 / アロケータ | allocation action / the allocator |
| thread | スレッド | compute thread; not a homonym with wire ワイヤ in Japanese |
| widget | ウィジェット | UI element |
| validation | 検証 | the validation-tree feature; Japanese does not lexically separate validation from verification in this domain, so one term covers both by decision |
| batch mode | バッチモード | canonical engineering term; preserves one catalog term per concept |
| fork (process) | フォーク | kept verbatim per topic 4 |
| deadlock | デッドロック | canonical engineering term; preserves one catalog term per concept |
| notifier | 通知 | canonical engineering term; preserves one catalog term per concept |
| token / operand / operator / arity | トークン / オペランド / 演算子 / 項数 | expression-parser terms |
| override | オーバーライド | supersede a value (SY symbol); never 上書き |
| swap | 入れ替え | exchange |
| theme | テーマ | UI/color theme; never 話題 (topic/subject) |

### Metrics and miscellaneous

| Concept | Term | Sense / purpose |
|---|---|---|
| noise / noise temperature | 雑音 / 雑音温度 | electronic/thermal noise; never 騒音 (acoustic racket) |
| efficiency | 効率 | canonical engineering term; preserves one catalog term per concept |
| interpolation | 補間 | canonical engineering term; preserves one catalog term per concept |
| mnemonic | ニーモニック | a card's code descriptor; never メモ |
| degrees / deg | 度 | freestanding axis/prose form; the parenthetical unit tag "(deg)" stays Latin per topic 1/4 like other unit tags |
| diameter | 直径 | native term chosen over the loanword ダイアメーター |
| reflect | 反転 / 連動する / 反射 | three distinct senses: geometry mirror operation, behavioral tracking ("mirrors …"), and physics reflection |
| default(s) | デフォルト | fallback value |
| normalize / normalization | 正規化 | translated, not transliterated |

Unit symbols, NEC2 mnemonics, file extensions, and Ω/dBi/MHz/VSWR/γ/φ/μ
stay in Latin/Greek form per topics 1 and 4.

## 11. Disambiguation policy

- The correct electrical/RF sense is chosen for every ambiguous term (eg 電流
  alone for "Currents", never 電気電流 or 電流量).
- No qualifier absent from the source is added; program context already
  disambiguates charge, current, ground, wire, gain, pattern, excitation,
  and load.
- A qualifier is added only where the Japanese term would otherwise be
  genuinely ambiguous (eg the "(φ=0)" tag on 瞬時値 only when source carries
  it, per topic 10).
- No accepted intra-domain homonym exists in this catalog: 負荷 (load) and
  電荷 (charge) are kept lexically distinct rather than merged.
- No locative/other homonym collision exists: スレッド (compute thread) and
  ワイヤ (wire) do not collide in Japanese, unlike in some other languages.
- Gerund vs noun senses of "scale" are distinguished: 拡大縮小する (verb,
  to scale) vs スケール (noun, a scale), per topic 10.

## 12. Cross-catalog consistency

- One term per concept, reused from the topic 10 table; no new synonym is
  introduced for an already-mapped concept.
- Keep these Appendix C concepts lexically distinct:
  極性/偏波, 大きさ/振幅, ピーク値/ピークの大きさ, グラウンド/大地,
  負荷/電荷, 利得/増幅/利益, 電流/現在, 電荷/請求, ワイヤ/ケーブル,
  ワイヤ/スレッド, 放射パターン/テンプレート, 放射パターン/遠方界,
  励振/興奮, 節・腹/ゼロ, 階調/色相/パレット種別/色投影,
  コメット/ジオメトリ, Identity/単位, レンダラー/レンダーエンジン,
  オーバーライド/上書き, ビューア/観測者・話者・プレビュー,
  反転/連動する/反射, 構造/構築, テーマ/話題, 正味利得/実数利得,
  and 電力/べき乗; this prevents false-sense inheritance.
- Validation and verification collapse to 検証 because established Japanese
  engineering usage does not provide a stable lexical distinction here;
  program context carries the feature-versus-check sense.
- Loanword-vs-native decisions are locked: グラウンド never グランド;
  直径 never ダイアメーター; カーネル, インピーダンス, リアクタンス,
  インダクタンス, キャパシタンス, アドミタンス as loanwords (no native
  synonym competes for these in this catalog).
- Any minority-outlier spelling found elsewhere in the catalog unifies to
  the canonical form stated in topic 10.
- Catalog consistency outranks a more "natural" locale-form alternative in
  the two concrete cases above (グラウンド over グランド, native 直径 over
  loanword ダイアメーター).

## 13. Priority ordering

Precedence chain when decisions conflict: correct meaning, then interface
convention (topic 7), then catalog consistency (topic 12), then
disambiguation (topic 11), then locale numeral form (topic 2).

- グラウンド over グランド: catalog consistency overrides the shorter,
  also-attested locale form.
- 負荷/電荷 kept distinct rather than merged into an available homonym:
  correct meaning overrides brevity.

## 14. Grammatical number

- Japanese nouns, adjectives, participles, and verbs do not inflect for
  grammatical number in these literals; invent no plural marker.
- After an explicit literal count, keep the canonical unmarked noun form;
  add a counter only when the source concept and established Japanese term
  require one.

## 15. Grammatical agreement

- Not applicable: Japanese has no grammatical gender or number concord for
  adjectives, participles, implied heads, or standalone labels.
- Declension classes, partitive-after-count forms, and agreement-driven
  reflexive particles are not applicable.

## 16. Morphological derivation

- Form borrowed technical verbs and verbal nouns with the locked katakana
  term plus する; use the locked kanji term plus する where topic 10 selects
  a native term.
- Introduce no alternate derivational suffix for an established concept;
  this preserves one term per concept.
- Fuse kanji and katakana technical compounds directly; insert a particle
  only when required by sentence grammar rather than compound formation.

## 17. Particle and sandhi selection

- Select は, が, を, に, で, と, から, and まで by the translated predicate's
  grammatical relation; do not copy a source preposition mechanically.
- Order postpositional phrases before the predicate and use に for target or
  state, で for means or locus of action, and と for quoted or paired values
  where those relations apply.
- Sound-conditioned sandhi, article elision, and preposition contraction are
  not applicable to Japanese interface literals.

## 18. Card/record-label register

- Use mnemonic + カード in dialog and editor titles, eg GWカード.
- Use the same mnemonic + カード form in running prose; Japanese has no
  distinct hyphenated record form.
- Use the bare mnemonic only in compact technical contexts where the source
  names the code alone; retain its exact case under topics 4 and 6.
- Keep title, prose, and bare-code uses internally consistent; do not
  cross-convert their fixed forms.

## 19. Multi-paragraph and whitespace fidelity

- Mirror every source paragraph break at the same position, preserving the
  distinction between a blank line and a single line break.
- Preserve semantic line breaks; introduce no visual wrapping absent from
  the source literal.
- Drop a trailing clause removed from the current source rather than carrying
  it from an inherited translation.
- Preserve the presence and position of trailing newlines; apply topic 3 to
  the translated terminal punctuation.
- Preserve complete meaning; do not truncate wording or use an unnatural
  abbreviation for an assumed display limit.

## 20. Current-source fidelity

- Derive every translation from the current source literal and its supplied
  context.
- Reuse an inherited translation only when its complete meaning and string
  type agree with the current source.
- Do not inherit です/ます into a developer diagnostic, 振幅 for source
  "magnitude", or ピーク値 for source "peak magnitude"; these known stale
  mappings violate topics 7, 10, and 24.

## 21. Script hygiene

- Require zero wrong-script or homoglyph failures in translated prose.
- Use half-width ASCII for retained Latin letters and digits; prohibit their
  full-width forms outside a retained token whose spelling requires them.
- Use U+30FC for the katakana long-vowel mark; prohibit hyphen-minus, dash,
  minus, and vertical-bar look-alikes in its place.
- Keep Greek and Latin homoglyphs distinct from kana and kanji; allow each
  only inside a retained symbol, identifier, unit, or proper name whose exact
  spelling requires it.
- Translate plain foreign prose; retain only the tokens enumerated in topic
  4, without translation or transliteration.

## 22. Rule-file scope hygiene

- Include only decisions that can alter wording, Unicode characters,
  punctuation, capitalization, embedded tokens, mnemonic markers, or
  semantic whitespace in a translated literal.
- Omit catalog representation, serialization, headers, flags, workflow,
  rendering, fonts, shaping, layout, widget behavior, runtime formatting,
  sorting, search, display sizing, audit history, completion state, review
  metadata, provenance, and source citations.
- State each current decision directly; include no implementation procedure,
  bibliography, correction history, or alternative policy source.

## 23. Section-disjointness declaration

- Keep four axes non-overlapping: script mechanics (topics 1-6, 9, 21)
  governs literal code points and punctuation; phrasing and structure
  (topics 7, 10-13, 18-20) govern word choice and sentence form; address
  register (topic 8) governs politeness and person; developer-string policy
  (topic 24) governs diagnostic audience and register.
- Place each decision on one axis only; references may connect axes without
  restating their rules.

## 24. Developer/debug-string policy

- Translate every user-facing command, label, dialog, tooltip, status, and
  error string into Japanese under topics 7 and 8.
- Translate informational notices and logs into concise technical Japanese;
  use plain impersonal predicates unless the message directly addresses the
  operator.
- Translate developer-facing diagnostics, assertion text, parser messages,
  and subsystem reports into terse plain-form technical Japanese; review
  priority never permits an applicable literal to remain untranslated.
- Preserve every embedded identifier, function name, configuration key,
  format specifier, unit, and retained token verbatim in all three families.
- A subsystem's established sibling wording may fix terminology, but it does
  not override the family register or token-preservation rules.
