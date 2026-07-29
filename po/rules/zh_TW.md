# zh_TW translation rules

Traditional Chinese for Taiwan, written in Taiwan-standard Traditional Han characters, for professional electrical and radio-frequency engineers, antenna designers, and amateur-radio operators using xnec2c.

## 1. Script and orthography

- Script: Traditional Chinese Han characters; embedded Latin/ASCII tokens coexist unchanged.
- No diacritics; no precomposed-vs-combining ambiguity; no letter-case distinction (casing reduces to topic 6's Latin-token handling).
- Directionality: left-to-right; no bidirectional mirroring concern.
- Orthographic standard: Taiwan MOE/CNS 11643 traditional forms; use no regional or simplified variant forms.
- Spacing: no space between Han characters; one space between Chinese text and an embedded Latin/numeric token, eg `檢視 3D 結構`.
- Compound formation: native compounds fuse with no separator; ASCII compound tokens (unit symbols, mnemonics) keep source form.

## 2. Numerals in literals

- Digit set: Western Arabic digits (0-9) for all technical values; Chinese numeral characters never used for technical values.
- Decimal separator: period `.` in numbers physically present in translated prose.
- Thousands/grouping separator: comma `,` in literal grouped numbers.
- Retain formulas, examples, fixed defaults, and named mathematical or standards constants in source numeral form to preserve exact technical identity.
- Index and ordinal notation uses the 第 prefix or 之 suffix with an Arabic digit, eg `第 1 條`; these affixes are distinct from the decimal separator.

## 3. Punctuation and quotation

- Native quotation marks 「」/『』 for prose quotation; embedded technical tokens (literal strings, filenames) keep plain ASCII quotes.
- Full-width ideographic punctuation replaces source ASCII punctuation in Chinese prose: 。，、；：？！「」.
- No space before/after colon, semicolon, or terminal punctuation in Chinese text; one space separates a Chinese clause from an adjacent ASCII/numeric run.
- Ellipsis: single-character … (U+2026); never three periods.
- Dash: retain ASCII hyphen-minus for ranges/compounds, eg `GW-EX`; no em-dash substitution.
- Sentence terminator: 。 closes full declarative sentences, dialogs, and confirmations; short labels, menu items, and field names omit it.
- Parentheses enclosing English/Latin terms inline with Chinese text stay ASCII `()`.
- Punctuation inside embedded technical runs stays in source form, eg `%d items`, format specifiers, file paths.

## 4. Never-translate tokens

- NEC2 card mnemonics kept verbatim: GW GA GH EX LD FR RP GE EN, and extended SP SM SC NE NH NT TL GN GD GC GX GR GS GM GF CP CM SY XQ EK KH PQ PT.
- Unit symbols kept verbatim: dBi, MHz, Ω, VSWR, %; degrees carries as the parenthetical unit tag `(deg)` like other unit tags.
- Figure-of-merit tokens kept verbatim: VSWR, S-parameter notation, Z/Z0 impedance symbols, front/back-ratio and noise-temperature tokens.
- File extensions kept verbatim: .nec, .csv, .s1p, .s2p, .png.
- Embedded identifiers (function names, variable names, config keys) inside developer strings kept verbatim - see topic 24.
- Physical/mathematical symbol letters (E, H, Z, exponents) stay in source form.
- Product, library, tool, and chart names kept verbatim: xnec2c, NEC2, GSL, OpenGL, GTK, and Smith.
- Named transfer-function and algorithm terms kept verbatim: Log, Asinh, μ-law, Reinhard, Sigmoid, Identity; the descriptive `Power` family name translates as 冪次 per topic 10.
- Conditional loanwords: segment (線段), patch (面片), tag (標籤), fork (fork) - segment/patch/tag translate to native terms; fork stays verbatim as a process term; held consistently file-wide.
- All roster tokens follow topic 1's LTR rule; never reordered or mirrored.

## 5. Format-specifier integrity

- Every source format specifier (%d, %s, %.2f) is preserved, same set, none added or dropped.
- Default order matches source; positional reordering (%1$s) is allowed only where Chinese word order requires it.
- For divergent word order, restructure the sentence around fixed specifier positions rather than reordering the specifiers themselves.
- Numbers inside specifiers are never localized; runtime substitution stays in source numeral form.

## 6. Capitalization and title-case

- Han script has no letter-case distinction; apply casing rules only to retained Latin tokens and mnemonic letters.
- Retain the source casing of Latin acronyms, NEC2 mnemonics, and axis letters `X`/`Y`/`Z` to preserve technical identity.
- Keep lowercase mathematical and coordinate variables lowercase to preserve notation.
- Coordinated option names require no element-capitalization rule because Han script has no case.
- Distinguish the title designator 卡 from the running-prose noun 卡片 by topic 18, not by case.

## 7. Interface register by string type

- Commands, buttons, and menu actions use a subjectless verb-object imperative, with the verb first and the shortest natural complete object, eg `開啟檔案`.
- Field labels use a compact noun phrase in modifier-head order and retain the source colon as `：`, eg `頻率：`.
- Dialog statements use complete topic-comment or subject-predicate sentences; confirmations present the affected object before the requested decision and end in `嗎？`.
- Tooltips use complete declarative clauses in cause-then-availability order; when the source gives a disabled-state reason, retain that reason, eg `僅在近場模式下可用`.
- Status and error messages use impersonal declarative clauses, place the affected entity before its state or failure, and omit first- and second-person pronouns.
- Use fused technical compounds and concise noun phrases; preserve every source meaning and avoid forced abbreviations or source-length imitation.
- Name user-visible domain entities in modifier-head order, with the technical qualifier before the head noun, eg `遠場貢獻`; apply topic 18 to NEC2 card names.
- Topic 8 governs address register; topic 24 governs developer and debug strings.

## 8. Formality and address

- Use neutral professional Taiwan engineering register; omit direct address and express formality through precise construction and word choice because verbs have no formality inflection.
- Commands use subjectless imperative verb-object clauses; dialogs and confirmations use complete neutral sentences.
- Form confirmations as object-first declarative questions ending in `嗎？`, never as commands.
- Omit first- and second-person pronouns and honorifics; direct personal address is not applicable to these interface literals.
- Avoid gendered pronouns and address forms; express required number or animacy with natural neutral nouns.
- Inclusive wording uses role or entity nouns rather than gendered person terms.
- Personal-name order is not applicable because the interface does not address or name people.
- Prohibit casual particles `啦` and `喔`, slang, archaic constructions, ceremonial over-formality, and commercial language.

## 9. Accelerator/hotkey mnemonics

- Preserve a source mnemonic as the literal suffix `(_X)` after the translated label, eg `檢視幾何結構(_G)`, to expose the accelerator in Han-script text.
- Choose `X` from a typable Latin letter associated with the translated term; do not transliterate or copy the source mnemonic merely because it occupied that source position.
- Use an unshifted, non-dead-key letter so the mnemonic remains directly typable.
- Add no mnemonic marker when the source literal has none.

## 10. Domain lexicon

| concept | zh_TW | sense / hazard guarded |
|---|---|---|
| current | 電流 | electrical; not "recent" |
| charge | 電荷 | electrical; not billing |
| voltage | 電壓 | electric potential |
| power (electrical) | 功率 | watts/gain/flow; distinct from 冪次 (math power-law) |
| impedance | 阻抗 | complex Z; distinct from resistance/reactance |
| resistance | 電阻 | real part of Z; distinct from impedance/load |
| reactance | 電抗 | imaginary part of Z |
| inductance | 電感 | canonical technical sense; prevents synonym drift |
| capacitance | 電容 | canonical technical sense; prevents synonym drift |
| conductivity | 導電率 | material S/m |
| admittance | 導納 | distinct from impedance |
| load | 負載 | LD-card impedance; not weight; not conflated with charge |
| gain | 增益 | antenna directivity ratio; not profit/amplification |
| excitation | 激勵 | EM energy input; not emotional excitement |
| feedpoint | 饋點 | antenna feed point |
| port | 埠 | excitation/S-parameter port |
| radials | 輻射地線 | ground-plane radial wires (noun); distinct from adjective 徑向 |
| ground / ground plane | 接地 | RF reference plane, GN/GD cards; not soil; one term across all sub-senses |
| earth (physical medium) | 大地 | terrain/noise-model earth; distinct from electrical 接地 |
| ground wave | 地波 | propagation term; distinct from 接地 |
| wire | 導線 | thin conductor/GW element; not cable/cord |
| segment | 線段 | NEC2 geometry subdivision |
| patch | 面片 | NEC2 surface patch (SP/SM) |
| tag | 標籤 | NEC2 geometry identifier; not a UI label or a card |
| card | 卡片 | NEC2 input record; topic 18 shortens only mnemonic-bound title designators to 卡 |
| kernel | 核心 | thin-wire integral kernel; not an OS kernel |
| cliff | 懸崖型 | two-medium ground-boundary type; not a fracture |
| structure | 結構 | antenna model geometry; not "construction" |
| model | 模型 | NEC model or noise-temperature model |
| geometry | 幾何 | the model geometry |
| crossed | 交叉 | transmission-line conductors reversed; not cut/severed |
| field (EM) | 場 | near/total/E/H field; distinct from a data/config 欄位 |
| near field / far field | 近場 / 遠場 | opposed spatial regions |
| far-field contribution | 遠場貢獻 | per-direction; distinct from 近場動畫 |
| radiation | 輻射 | radiated emission |
| radiation pattern | 輻射場型 | plotted directional response; prevents confusion with a template, design, or far-field region |
| gain pattern | 增益場型 | the gain radiation pattern |
| polarization | 極化 | EM wave/antenna orientation; not polarity |
| polarity | 極性 | sign of a quantity; false friend of 極化 |
| phase | 相位 | canonical technical sense; prevents synonym drift |
| reference phase | 參考相位 | canonical technical sense; prevents synonym drift |
| frequency | 頻率 | canonical technical sense; prevents synonym drift |
| wave / wavelength | 波 / 波長 | canonical technical sense; prevents synonym drift |
| standing wave / traveling wave | 駐波 / 行波 | opposed pair |
| node / antinode | 波節 / 波腹 | standing-wave zero/maximum |
| crest | 波峰瞬時點 | instantaneous wave apex (comet-head); distinct from curve/step peak |
| magnitude | 量值 | modulus of a quantity; distinct from amplitude |
| amplitude | 振幅 | oscillating-quantity peak; distinct from magnitude |
| peak value | 峰值 | distinct UI option from peak magnitude |
| peak magnitude | 峰值量值 | distinct UI option from peak value |
| instantaneous | 瞬時 | projection mode; add (φ=0) qualifier only where source carries it |
| Poynting vector | 坡印廷向量 | canonical technical sense; prevents synonym drift |
| solid angle | 立體角 | canonical technical sense; prevents synonym drift |
| net gain | 淨增益 | total-minus-mismatch gain; not "real (part) gain" |
| viewer | 檢視器 | observation direction / 3D view widget; not observer/speaker/preview |
| flow / flow direction | 流向 | patch/current flow |
| total field | 總場 | canonical technical sense; prevents synonym drift |
| color | 顏色 | canonical technical sense; prevents synonym drift |
| color projection | 顏色投影 | which quantity drives hue |
| hue | 色相 | color-wheel angle |
| brightness | 亮度 | luminance channel |
| hue encoding | 色相編碼 | distinct enum from brightness encoding and color projection |
| brightness encoding | 亮度編碼 | distinct enum from hue encoding and color projection |
| color scale | 顏色刻度 | magnitude-to-color scale |
| scale family / color tone | 刻度家族 | transfer-curve family; one concept, one term; never 顏色家族 |
| palette / palette kind | 調色盤 | palette-layout enum; distinct from scale family and color projection |
| ramp / gradient | 漸層 | a palette kind / linear color strip |
| gamma | 伽瑪值 | power-law exponent |
| knee | 拐點 | soft-knee bend point |
| softening | 柔化 | dynamic-range softening |
| compression | 壓縮 | dynamic-range compression |
| contrast | 對比 | canonical technical sense; prevents synonym drift |
| dynamic range | 動態範圍 | canonical technical sense; prevents synonym drift |
| floor | 底限 | minimum/lower clamp; not a room floor |
| envelope | 包絡 | magnitude/amplitude envelope |
| comet | 彗星 | moving-crest overlay effect; not geometry |
| overlay (noun) | 疊加層 | an added visual layer; distinct from verb 疊加 |
| animate / animation | 動畫 | canonical technical sense; prevents synonym drift |
| animated (category, adj.) | 動態 | paired with static 靜態; distinct from noun/verb 動畫 |
| static (category, adj.) | 靜態 | canonical technical sense; prevents synonym drift |
| projection | 投影 | color or geometry projection |
| scale (verb/noun) | 縮放 / 刻度 | verb "to scale" 縮放; noun "a scale" 刻度 |
| wireframe | 線框 | wire-mesh render mode |
| identity | 恆等 | no-op/passthrough transfer; distinct from unity 單位 (Smith-chart) |
| sentinel | 哨符值 | unreachable-case guard value |
| bins | 分箱 | discretization buckets |
| companding | 壓伸 | bounded log curve (μ-law) |
| tone mapping | 色調映射 | photographic tone-map |
| renderer | 繪製器 | drawing backend; not "render engine" |
| shader | 著色器 | canonical technical sense; prevents synonym drift |
| allocation (memory) | 記憶體配置 | allocation and the allocator/report |
| managed allocator | 受管配置器 | canonical technical sense; prevents synonym drift |
| thread | 執行緒 | compute thread; distinct from 導線 (wire) |
| widget | 元件 | UI element |
| validation | 驗證樹 | the validation-tree feature; distinct from 檢查 (verification checks) |
| batch mode | 批次模式 | canonical technical sense; prevents synonym drift |
| fork (process) | fork | process fork; kept verbatim per topic 4 |
| deadlock | 死結 | canonical technical sense; prevents synonym drift |
| notifier | 通知器 | canonical technical sense; prevents synonym drift |
| token | 詞元 | expression-parser term |
| operand | 運算元 | expression-parser term |
| operator | 運算子 | expression-parser term |
| arity | 元數 | expression-parser term |
| override | 覆寫 | supersede a value (SY symbol); not 覆蓋 (overwrite) |
| swap | 交換 | canonical technical sense; prevents synonym drift |
| theme | 佈景主題 | UI/color theme; not topic/subject |
| noise / noise temperature | 雜訊 / 雜訊溫度 | electronic/thermal; not acoustic racket |
| efficiency | 效率 | canonical technical sense; prevents synonym drift |
| interpolation | 內插 | canonical technical sense; prevents synonym drift |
| mnemonic | 助記碼 | a card's code descriptor; not a memo/note |
| degrees / deg | 度 / (deg) | freestanding prose 度 vs parenthetical unit tag (deg), per topic 4 |
| diameter | 直徑 | canonical term; no competing synonym |
| reflect | 鏡射 / 鏡像追蹤 / 反射 | geometry mirror 鏡射; behavioral tracking 鏡像追蹤; physics 反射 - three distinct senses |
| default(s) | 預設值 | fallback value |
| normalize / normalization | 正規化 | translated, not transliterated |

## 11. Disambiguation policy

- Each ambiguous term resolves to its correct technical sense per topic 10's table; no alternate sense substituted.
- No qualifier is added beyond what the source states; program context already disambiguates.
- A qualifier is added only where the zh_TW term would otherwise be genuinely ambiguous, eg 瞬時值 (φ=0) kept distinct from bare 瞬時.
- Accepted intra-domain homonym: none recorded for zh_TW; 負載 (load) and 電荷 (charge) are held distinct, not merged.
- No locative homonym collision recorded.
- Gerund vs noun senses: 動畫 serves both the noun "animation" and verb "to animate"; no separate coinage needed.

## 12. Cross-catalog consistency

- Reuse the single topic 10 target for each concept across the catalog to prevent synonym drift.
- Keep these Appendix C targets distinct: `polarity` 極性 / `polarization` 極化; `magnitude` 量值 / `amplitude` 振幅; `peak value` 峰值 / `peak magnitude` 峰值量值; `ground` 接地 / `earth` 大地; `load` 負載 / `charge` 電荷; `gain` 增益 / `amplification` 放大 / `profit` 利潤; `current` 電流 / `present or recent` 目前或近期; `charge` 電荷 / `billing or fee` 計費或費用.
- Keep these Appendix C targets distinct: `wire` 導線 / `cable or cord` 纜線 / `thread` 執行緒; `radiation pattern` 輻射場型 / `template or design` 範本或設計 / `far field` 遠場; `excitation` 激勵 / `emotional excitement` 興奮; `node or antinode` 波節或波腹 / `generic null or zero` 通用零值; `scale family or color tone` 刻度家族 / `hue` 色相 / `palette kind` 調色盤 / `color projection` 顏色投影.
- Keep these Appendix C targets distinct: `comet` 彗星 / `geometry` 幾何; `identity` 恆等 / `unity` 單位; `renderer` 繪製器 / `render engine` 繪圖引擎; `override` 覆寫 / `overwrite` 覆蓋; `viewer` 檢視器 / `observer, speaker, or preview` 觀察者、講者或預覽; `reflect` 鏡射 / `mirrors` 鏡像追蹤 / `reflection` 反射.
- Keep these Appendix C targets distinct: `structure` 結構 / `construction` 建造; `theme` 佈景主題 / `topic or subject` 主題; `validation` 驗證樹 / `verification` 檢查; `net gain` 淨增益 / `real-part gain` 實部增益; `power` 功率 / `Power family` 冪次.
- Prefer native technical terms; retain a loanword only when topic 4 places it in the never-translate roster, and lock the topic 10 spelling to preserve consistency.
- Minority-outlier spellings (eg legacy 顏色家族 for scale family) unify to the canonical form 刻度家族.
- Consistency priority outranks locale-form preference where a single card/data-type must render identically across every dialog, eg 卡片 always for the standalone noun, never a per-dialog synonym.

## 13. Priority ordering

- Precedence: correct meaning, then interface convention (topic 7), then catalog consistency (topic 12), then disambiguation (topic 11), then locale numeral form (topic 2).
- Override ruling: 極性 vs 極化 resolved by meaning even where a shorter catalog-consistent term would collide; correctness outranks brevity.

## 14. Grammatical number

- Chinese translated literals have one uninflected noun form for singular and plural reference; adjectives, participles, and verbs also carry no number inflection.
- After an explicit literal count, retain the same noun form and add no plural marker, preserving natural analytic grammar.

## 15. Grammatical agreement

- Not applicable: Chinese has no grammatical gender, number concord, or declension class; adjectives and participles carry no agreement marking.

## 16. Morphological derivation

- Borrowed technical verbs/nouns render as native compounds (eg 正規化, 內插), never a phonetic transliteration.
- Verbal-noun formation: bare verb-stem compound doubles as the noun, eg 動畫, 縮放; no separate nominalizing affix.
- Native-affix compounding is standard; loanword transliteration is used only for roster proper nouns (topic 4).

## 17. Preposition and sandhi selection

- Not applicable: Chinese prepositions (介詞) are lexical items with no phonologically conditioned form selection; no elision/contraction rule applies to UI prose.

## 18. Card/record-label register

- Dialog/editor-title designator: concise suffix 卡, eg `FR 卡`, `EX 卡`, `GC 卡`, `SP/SM 卡`.
- Running-prose form: standalone noun 卡片, eg `刪除卡片`, `管理卡片`, `卡片助記碼`.
- Generic-noun casing: no case distinction; short form 卡 pairs only with a mnemonic, long form 卡片 stands alone.
- Each register stays internally consistent: a mnemonic-suffixed title never substitutes 卡片, and a standalone noun never substitutes bare 卡.

## 19. Multi-paragraph and whitespace fidelity

- Preserve source paragraph breaks at the same positions and distinguish blank-line breaks from single-line breaks to retain semantic structure.
- Remove any trailing clause absent from the current source so stale meaning does not survive.
- Preserve source trailing newlines and terminal punctuation exactly to retain literal boundaries.
- Preserve semantic line breaks; add no line break solely for visual wrapping.
- Preserve complete meaning; never truncate wording or use an unnatural abbreviation for an assumed display limit.

## 20. Current-source fidelity

- Derive every translation from the current source literal and its supplied context to preserve the current meaning.
- Reuse an inherited translation only when its complete meaning agrees with the current source and context.
- Treat `顏色家族` as unsafe inheritance for `scale family`; use `刻度家族`.
- Treat `動畫` as unsafe inheritance for the category adjective `animated`; use `動態`.

## 21. Script hygiene

- Translated prose contains zero simplified character variants and zero mismatched half-width punctuation; allow a non-native character only inside a retained token whose literal spelling requires it.
- Translate plain foreign prose words; retain only genuine identifiers, unit symbols, and topic 4 roster tokens to preserve the never-translate boundary.

## 22. Rule-file scope hygiene

- Retain only current decisions that can alter wording, Unicode characters, punctuation, capitalization, embedded tokens, mnemonic markers, or semantic whitespace in a translated literal.
- Omit catalog representation, serialization, headers, flags, validation procedure, rendering, fonts, shaping, layout, widget behavior, runtime number formatting, sorting, search, display sizing, audit history, completion state, review metadata, provenance, and citations.
- State each decision directly; include no implementation procedure, bibliography, correction history, or alternative policy source.

## 23. Section-disjointness declaration

- Script mechanics in topics 1, 3, 6, and 21 govern characters and punctuation; phrasing and structure in topics 5, 7, 9, 18, and 19 govern literal composition; address register in topic 8 governs interpersonal stance. These axes do not overlap, and each concept belongs to one axis.

## 24. Developer/debug-string policy

- Translate every user-facing command, label, dialog, tooltip, status, and error string; review priority never permits leaving one untranslated.
- Translate informational diagnostics intended for operators in the terse, impersonal technical register of topic 7.
- Keep developer-facing debug and internal diagnostic strings in their source language; no subsystem family overrides this decision.
- Preserve every embedded identifier, function name, format specifier, and topic 4 retained token verbatim in every string family.
- Keep linguistic priority separate from token preservation: every translated family applies topics 4 and 5 regardless of review order.
