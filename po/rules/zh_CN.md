# zh_CN translation rules

These rules govern Simplified Chinese for mainland China, written in the GB 18030 Simplified orthographic standard, for professional electrical and radio-frequency engineers, antenna designers, and amateur-radio operators.

## 1. Script and orthography

- Use Simplified Chinese Han characters only in translated prose; use no Traditional variants.
- Use GB 18030 / Unicode CJK Unified Ideographs and the mainland Simplified orthographic standard.
- Han text has no required diacritics; combining-mark policy is not applicable.
- Preserve any combining sequence only when it belongs to a retained technical token; do not substitute a look-alike code point.
- Script-specific joiners and positional letter forms are not applicable to Han text.
- Write left-to-right; opposite-direction runs and manual direction overrides are not applicable.
- Han script has no letter-case distinction; topic 6 governs only retained Latin tokens and mnemonic letters.
- Join Chinese characters without inter-word spaces; join adjacent Latin or numeric technical tokens without a space unless semantic separation requires one.
- Form Chinese compounds as fused words; use spaces or hyphens only when they belong to a retained token.
- REQUIRED: translated prose contains zero Traditional variants, stripped required marks, or look-alike substitutions.

## 2. Numerals in literals

- Use Western Arabic digits `0-9` for literal technical values; do not replace them with Chinese numeral characters.
- Use `.` as the decimal separator in literal numbers.
- Preserve source grouping in literal numbers; do not add a thousands separator absent from the source.
- Preserve formulas, examples, fixed defaults, and named mathematical or standards constants verbatim.
- Form native ordinals and indices as `第N`; keep an index separator distinct from the decimal separator.

## 3. Punctuation and quotation

- Use full-width Chinese punctuation `：，。；？！（）` around Chinese prose.
- Use `「」` for quoted Chinese prose; use source ASCII quotes around retained technical tokens.
- Place no ASCII space before or after Chinese colons, semicolons, commas, or terminal punctuation.
- Use `……` for an ellipsis; preserve source dash characters inside retained technical runs.
- Omit `。` from short labels, menu items, and buttons; end full-sentence dialogs, confirmations, and tooltips with native terminal punctuation.
- Keep punctuation inside identifiers, file paths, card mnemonics, format specifiers, and other retained technical runs in source form.

## 4. Never-translate tokens

Treat this roster as a zero-failure boundary: retain every listed token exactly, without translation or transliteration.

- NEC2 card mnemonics: `GW GA GH EX LD FR RP GE EN SP SM SC NE NH NT TL GN GD GC GX GR GS GM GF CP CM SY XQ EK KH PQ PT`.
- Unit symbols: `MHz`, `GHz`, `Hz`, `dBi`, `dB`, `Ω`, `K`, `S/m`, `deg`, `%`.
- Figure-of-merit and parameter tokens: `VSWR`, S-parameter tokens, `Z`, `Z0`, front/back-ratio tokens, and `G/T`.
- File extensions: `.nec`, `.csv`, `.s1p`, `.s2p`, `.png`, `.nec2`, `.out`.
- Format specifiers: printf-style placeholders including `%s`, `%d`, `%f`, `%1$s`; topic 5 applies.
- Embedded identifiers: function names, variable names, and configuration keys inside developer strings.
- Physical and mathematical symbol letters: `E`, `H`, `θ`, `φ`, and exponent letters.
- Product, library, and tool names: `xnec2c`, `GTK`, `Cairo`, `OpenGL`.
- Named transfer functions and algorithms: `Log`, `Asinh`, `μ-law`, `Reinhard`, `Sigmoid`; translate descriptive `Power` as 幂函数.
- Translate geometry loanwords as `segment` = 分段, `patch` = 面片, and `tag` = 标签号; retain process `fork` as `fork`.
- Apply topic 1 directionality to every retained token; no bidirectional exception applies.

## 5. Format-specifier integrity

- REQUIRED: preserve the exact source set and count of format specifiers with zero omissions, additions, type changes, or localized digits.
- Keep source order by default; use positional specifiers only when Chinese information order requires reordering.
- Restructure Chinese wording around fixed non-positional specifiers rather than moving them.

## 6. Capitalization and title-case

- Chinese labels, menu items, and titles have no sentence-case or title-case distinction.
- Preserve source casing in Latin acronyms, proper nouns, axis letters, mathematical variables, card mnemonics, and retained tokens.
- Keep lowercase mathematical and coordinate variables lowercase.
- Coordinated Chinese option names have no element-capitalization distinction.
- Chinese generic card and record nouns have no case distinction; topic 18 fixes their positional forms.

## 7. Interface register by string type

Treat interface text as technical written Chinese: place known information before the action or result, omit recoverable subjects, and preserve every source meaning in the shortest natural complete form.

- Commands, buttons, and menu actions use a concise verb-object command or established noun phrase; omit the subject and sentence-final particles.
- Field labels use a head-final noun phrase followed by `：`; place qualifiers before the head noun.
- Dialogs and confirmations use full subject-predicate sentences; include 您 only for explicit direct address and place the decision or consequence before the requested confirmation.
- Tooltips use complete declarative sentences in purpose-then-condition order; state the reason whenever the source says a disabled control is unavailable.
- Status and error messages use impersonal declarative result-then-cause order; avoid direct address and imperative mood.
- User-visible domain entities use modifier-before-head order and the locked topic 10 term as the head; topic 8 governs personal names and proper address.
- Prefer fused technical compounds and concise multiword phrases; omit no meaning, use no unnatural abbreviation, and do not imitate source length.
- Topic 24 governs developer and debug strings.

## 8. Formality and address

- Use 您 for explicit direct address in confirmations and warnings; omit pronouns from commands, labels, status messages, logs, and indirect statements.
- Do not use informal 你, casual speech, slang, commercial language, archaic language, or honorific language beyond 您.
- Chinese verb morphology has no formal or informal ending; express formality only through pronoun choice and impersonal construction.
- Chinese grammatical gender and gender agreement are not applicable; use gender-neutral role and person terms.
- Grammatical animacy marking is not applicable; preserve natural number meaning without exclusionary person labels.
- Honorific titles are not applicable unless present in a proper personal name; preserve personal names in family-name-then-given-name order when localization supplies a Chinese name.
- Use full sentences with 您 in direct confirmation dialogs; end yes/no questions with `？` and statements with `。`.

## 9. Accelerator/hotkey mnemonics

- Preserve the GTK `_` marker only when the source literal contains one; never add a mnemonic to an unmarked source.
- Append the mnemonic as parenthesized `_` plus a typable Latin letter after the Chinese term.
- Choose the Latin letter from the standard romanization of the translated Chinese term, not from the source term.
- Keep mnemonic letters unique within each menu or dialog; resolve a collision with another unused letter from the same translated term.
- Avoid visually ambiguous mnemonic letters `O`, `I`, and `l` where another letter from the translated term is available.
- Use mnemonics only in source-marked menu items, buttons, and labels; tooltips and status or error messages carry none unless their source literal is marked.

## 10. Domain lexicon

Electrical primitives:

| concept | zh_CN | sense | note |
|---|---|---|---|
| current | 电流 | electrical current (A) | not temporal "current/recent" |
| charge | 电荷 | electrical charge (C) | not billing/fee/cargo |
| voltage | 电压 | electric potential | canonical technical sense; prevents synonym drift |
| power (electrical) | 功率 | radiated/dissipated W, gain, power-flow | distinct from `Power` 幂函数 and math 幂 |
| impedance | 阻抗 | complex Z | distinct from 电阻/电抗 |
| resistance | 电阻 | real part of Z | distinct from 阻抗/负载 |
| reactance | 电抗 | imaginary part of Z | canonical technical sense; prevents synonym drift |
| inductance | 电感 | canonical technical sense; prevents synonym drift | preserves the locked domain meaning |
| capacitance | 电容 | canonical technical sense; prevents synonym drift | preserves the locked domain meaning |
| conductivity | 电导率 | material S/m | canonical technical sense; prevents synonym drift |
| admittance | 导纳 | admittance matrix | distinct from 阻抗 |
| load | 负载 | LD-card impedance load | not weight/burden; not 电荷 |
| gain | 增益 | antenna directivity ratio (dB) | not profit 利润, not amplifier 放大 |
| excitation | 激励 | EM energy input/source | not emotional 兴奋 |
| feedpoint | 馈电点 | antenna feed point | canonical technical sense; prevents synonym drift |
| port | 端口 | excitation/S-parameter port | canonical technical sense; prevents synonym drift |
| radials | 辐射线 | ground-plane radial wires (noun) | distinct from adjective 径向的 |

Ground and earth:

| concept | zh_CN | sense | note |
|---|---|---|---|
| ground / ground plane | 地 | RF reference plane; GN/GD cards, type/conductivity/effects/model | one term across all electrical-reference sub-senses; not soil or physical 大地 |
| earth (physical medium) | 大地 | terrain/noise-model earth, below-ground geometry | distinct from electrical 地 |
| ground wave | 地波 | propagation term | distinct from 地 reference |

Geometry primitives:

| concept | zh_CN | sense | note |
|---|---|---|---|
| wire | 导线 | thin conductor / GW element | not 电缆; unify every use |
| segment | 分段 | NEC2 geometry subdivision | translated, not kept |
| patch | 面片 | NEC2 surface patch (SP/SM) | translated, not kept, file-wide |
| tag | 标签号 | NEC2 geometry identifier | not UI 标签 or a 卡 |
| card | 卡 | NEC2 input record | register per topic 18 |
| kernel | 核函数 | thin-wire integral kernel | not OS 内核 |
| cliff | 断崖 | two-medium ground-boundary type | not fracture/break |
| structure | 结构 | antenna model geometry | not 施工 (construction) |
| model | 模型 | NEC model or noise-temperature model | canonical technical sense; prevents synonym drift |
| geometry | 几何 | model geometry | canonical technical sense; prevents synonym drift |
| crossed | 交叉 | transmission-line conductors crossed/reversed | not cut/severed |

Field, pattern, viewer:

| concept | zh_CN | sense | note |
|---|---|---|---|
| field (EM) | 场 | near/total/E/H field | distinct from data/config 字段 |
| near field / far field | 近场 / 远场 | opposed spatial regions | keep symmetric |
| far-field contribution | 远场贡献 | per-direction contribution | not near-field animation |
| radiation | 辐射 | radiated emission | canonical technical sense; prevents synonym drift |
| radiation pattern | 辐射方向图 | plotted directional response | not template/design; one term catalog-wide |
| gain pattern | 增益方向图 | the gain radiation pattern | canonical technical sense; prevents synonym drift |
| polarization | 极化 | antenna/wave field orientation | canonical technical sense; prevents synonym drift |
| polarity | 极性 | sign (+/-) of a quantity | false friend of 极化; never shares its translation |
| phase | 相位 | canonical technical sense; prevents synonym drift | preserves the locked domain meaning |
| reference phase | 参考相位 | canonical technical sense; prevents synonym drift | preserves the locked domain meaning |
| frequency | 频率 | canonical technical sense; prevents synonym drift | preserves the locked domain meaning |
| wave / wavelength | 波 / 波长 | canonical technical sense; prevents synonym drift | preserves the locked domain meaning |
| standing wave | 驻波 | opposed pair with 行波 | canonical technical sense; prevents synonym drift |
| traveling wave | 行波 | opposed pair with 驻波 | canonical technical sense; prevents synonym drift |
| node / antinode | 波节 / 波腹 | standing-wave zero/maximum | also the null/peak overlay sense |
| crest | 波峰 | instantaneous wave apex (comet-head) | distinct from curve/step peak 峰值 |
| magnitude | 幅度 | modulus of a quantity (|Z|, scalar) | distinct from 振幅 |
| amplitude | 振幅 | oscillating-quantity peak | distinct from 幅度 |
| peak value | 峰值 | UI option | distinct from 峰值幅度; never collapse |
| peak magnitude | 峰值幅度 | UI option | distinct from 峰值; never collapse |
| instantaneous | 瞬时 | projection mode | add "(φ=0)" only where source carries it |
| Poynting vector | 坡印廷矢量 | canonical technical sense; prevents synonym drift | preserves the locked domain meaning |
| solid angle | 立体角 | canonical technical sense; prevents synonym drift | preserves the locked domain meaning |
| net gain | 净增益 | total-minus-mismatch gain | not 实部增益 |
| viewer | 视角 | observation direction / 3D view widget | not 观察者/讲者/预览 |
| flow / flow direction | 流向 | patch/current flow | canonical technical sense; prevents synonym drift |
| total field | 总场 | canonical technical sense; prevents synonym drift | preserves the locked domain meaning |

Color, tone, animation:

| concept | zh_CN | sense | note |
|---|---|---|---|
| color | 颜色 | generic color | in "color projection" compound use 色彩 (established) |
| color projection | 色彩投影 | which quantity drives hue | never 颜色投影 |
| hue | 色调 | color-wheel angle | canonical technical sense; prevents synonym drift |
| brightness | 亮度 | luminance channel | canonical technical sense; prevents synonym drift |
| hue encoding | 色调编码 | internal enum | distinct from 色彩投影, does not collapse |
| brightness encoding | 亮度编码 | internal enum | distinct from 色彩投影, does not collapse |
| color scale | 色阶 | magnitude-to-color scale | canonical technical sense; prevents synonym drift |
| scale family / color tone | 色阶族 | transfer-curve family (Power/Log/Asinh/μ-law/Reinhard/Sigmoid/Identity) | never 颜色族; two spellings, one term |
| palette / palette kind | 调色板 | palette-layout enum | distinct from 色阶族 and 色彩投影 |
| ramp / gradient | 渐变 | palette kind / linear color strip | canonical technical sense; prevents synonym drift |
| gamma | 伽马值 | power-law exponent | canonical technical sense; prevents synonym drift |
| knee | 拐点 | soft-knee bend point | canonical technical sense; prevents synonym drift |
| softening | 软化 | dynamic-range softening | canonical technical sense; prevents synonym drift |
| compression | 压缩 | dynamic-range compression | canonical technical sense; prevents synonym drift |
| contrast | 对比度 | canonical technical sense; prevents synonym drift | preserves the locked domain meaning |
| dynamic range | 动态范围 | canonical technical sense; prevents synonym drift | preserves the locked domain meaning |
| floor | 下限 | minimum/lower clamp (brightness/dB floor) | not 地板 |
| envelope | 包络 | magnitude/amplitude envelope | canonical technical sense; prevents synonym drift |
| comet | 彗星 | moving-crest overlay effect | not 几何; known fuzzy-inheritance hazard |
| overlay (noun) | 叠加层 | added visual layer | distinct from verb 叠加 |
| animate / animation | 动画 | canonical technical sense; prevents synonym drift | preserves the locked domain meaning |
| animated | 动态的 | category-header adjective | opposed to 静态的 |
| static | 静态的 | category-header adjective | opposed to 动态的 |
| projection | 投影 | color or geometry projection | generic parent of 色彩投影 |
| scale (verb) | 缩放 | to scale | canonical technical sense; prevents synonym drift |
| scale (noun) | 色阶 / 比例尺 | a color scale / a measurement scale | context selects; never interchange |
| wireframe | 线框 | wire-mesh render mode | canonical technical sense; prevents synonym drift |
| identity | 恒等 | no-op/passthrough transfer | distinct from 归一 (unity, Smith-chart) |
| sentinel | 哨兵值 | unreachable-case guard value | canonical technical sense; prevents synonym drift |
| bins | 分箱 | discretization buckets | canonical technical sense; prevents synonym drift |
| companding | 压扩 | bounded log curve (μ-law) | canonical technical sense; prevents synonym drift |
| tone mapping | 色调映射 | photographic tone-map | distinct from 色阶族 |

Render and compute:

| concept | zh_CN | sense | note |
|---|---|---|---|
| renderer | 渲染器 | drawing backend | not 渲染引擎 |
| shader | 着色器 | canonical technical sense; prevents synonym drift | preserves the locked domain meaning |
| allocation (memory) | 内存分配 | allocation | canonical technical sense; prevents synonym drift |
| managed allocator | 托管分配器 | allocator/report | canonical technical sense; prevents synonym drift |
| thread | 线程 | compute thread | no collision with 导线 (wire) in Chinese |
| widget | 控件 | UI element | canonical technical sense; prevents synonym drift |
| validation | 校验 | validation-tree feature | distinct from 验证 (verification checks) |
| batch mode | 批处理模式 | canonical technical sense; prevents synonym drift | preserves the locked domain meaning |
| fork (process) | fork | process fork | kept verbatim, topic 4 |
| deadlock | 死锁 | canonical technical sense; prevents synonym drift | preserves the locked domain meaning |
| notifier | 通知器 | canonical technical sense; prevents synonym drift | preserves the locked domain meaning |
| token | 词法单元 | expression-parser term | canonical technical sense; prevents synonym drift |
| operand | 操作数 | expression-parser term | canonical technical sense; prevents synonym drift |
| operator | 运算符 | expression-parser term | canonical technical sense; prevents synonym drift |
| arity | 元数 | expression-parser term | canonical technical sense; prevents synonym drift |
| override | 覆盖 | supersede a value (SY symbol) | not 覆写 (overwrite) |
| swap | 交换 | canonical technical sense; prevents synonym drift | preserves the locked domain meaning |
| theme | 主题 | UI/color theme | not 话题 (topic/subject) |

Metrics and miscellaneous:

| concept | zh_CN | sense | note |
|---|---|---|---|
| noise / noise temperature | 噪声 / 噪声温度 | electronic/thermal noise | not acoustic racket |
| efficiency | 效率 | canonical technical sense; prevents synonym drift | preserves the locked domain meaning |
| interpolation | 插值 | canonical technical sense; prevents synonym drift | preserves the locked domain meaning |
| mnemonic | 助记符 | card's code descriptor | not 备忘 (memo/note) |
| degrees / deg | 度 / (deg) | freestanding axis/prose vs parenthetical unit tag | tag stays "(deg)" like other unit tags |
| diameter | 直径 | canonical term | no competing native synonym |
| reflect (geometry) | 镜像 | geometry mirror operation | distinct sense 1 of 3 |
| reflect (behavioral) | 联动 | "mirrors …" a control tracking another | distinct sense 2 of 3 |
| reflect (physics) | 反射 | physics reflection | distinct sense 3 of 3 |
| default(s) | 默认值 | fallback value | canonical technical sense; prevents synonym drift |
| normalize / normalization | 归一化 | canonical technical sense; prevents synonym drift | translate, not transliterate |

| optimizer | 优化器 | feature-search optimizer subsystem | keeps the subsystem distinct from a generic improvement action |

## 11. Disambiguation policy

- Select the topic 10 technical sense for every ambiguous source concept.
- Add no qualifier when program context already selects one sense.
- Add a qualifier only when the canonical Chinese term remains genuinely ambiguous in the translated literal.
- Reuse 场 across near, total, electric, and magnetic field senses as an accepted intra-domain homonym.
- Avoid locative collisions through the locked 地 for electrical ground and 大地 for the physical medium.
- Render source gerund and noun senses of `scale` as verb 缩放 and noun 色阶 or 比例尺 according to the named scale type.

## 12. Cross-catalog consistency

Use one locked term per concept; unify outlier spellings to topic 10 and keep every pair below distinct.

- `polarity` = 极性; `polarization` = 极化, separating sign from field orientation.
- `magnitude` = 幅度; `amplitude` = 振幅, separating modulus from oscillating peak.
- `peak value` = 峰值; `peak magnitude` = 峰值幅度, preserving two interface options.
- `ground` = 地; `earth` = 大地, separating electrical reference from physical medium.
- `load` = 负载; `charge` = 电荷, separating impedance from electrical charge.
- `gain` = 增益; amplification = 放大; profit = 利润, preserving antenna directivity sense.
- `current` = 电流; temporal present or recent = 当前, preserving electrical sense.
- `charge` = 电荷; billing or fee = 费用, preserving electrical sense.
- `wire` = 导线; cable or cord = 电缆; compute `thread` = 线程, separating conductor and compute senses.
- `radiation pattern` = 辐射方向图; template or design = 模板; `far field` = 远场, separating plotted response and spatial region.
- `excitation` = 激励; emotional excitement = 兴奋, preserving electromagnetic input sense.
- `node` = 波节 and `antinode` = 波腹; generic numeric null or zero = 零值, preserving standing-wave senses.
- `scale family / color tone` = 色阶族; `hue` = 色调; `palette kind` = 调色板; `color projection` = 色彩投影, preserving four color-system concepts.
- `comet` = 彗星; `geometry` = 几何, preventing overlay and model inheritance collisions.
- `identity` = 恒等; Smith-chart unity = 归一, separating pass-through and unity senses.
- `renderer` = 渲染器; render engine = 渲染引擎, preserving backend scope.
- `override` = 覆盖; overwrite = 覆写, separating supersession from replacement.
- `viewer` = 视角; observer, speaker, or preview use their ordinary distinct terms, preserving observation and view-widget sense.
- Geometry `reflect` = 镜像; behavioral tracking = 联动; physical reflection = 反射.
- `structure` = 结构; construction = 施工, preserving model-geometry sense.
- `theme` = 主题; topic or subject = 话题, preserving interface-theme sense.
- `validation` = 校验; verification checks = 验证, preserving validation-tree sense.
- `net gain` = 净增益; real-part gain = 实部增益, preserving mismatch-adjusted gain.
- Electrical `power` = 功率; transfer-family `Power` = 幂函数, separating watts from the power-law curve.
- Use canonical 直径 for `diameter`; use 色彩投影 and 色阶族 instead of lexical variants even when another locale form seems natural.

## 13. Priority ordering

- Apply this precedence: correct meaning, interface convention, catalog consistency, disambiguation, then literal numeral form.
- Keep 极性 and 极化 distinct because correct meaning outranks lexical convenience.
- Use 色彩投影 and 色阶族 because catalog consistency outranks free lexical variation.
- Retain `fork` because the never-translate boundary outranks native-term preference.

## 14. Grammatical number

- Chinese nouns, adjectives, participles, and verbs do not inflect for grammatical number.
- After an explicit literal count, use the unchanged noun form and do not add a plural marker.

## 15. Grammatical agreement

- Grammatical gender, number concord, declension class, partitive forms, reflexive agreement, and standalone-label gender are not applicable to Chinese.

## 16. Morphological derivation

- Form borrowed technical nouns and verbs by established meaning-based calque; avoid phonetic borrowing unless topic 4 retains the source token.
- Use the unchanged lexical form as noun or verb when established Chinese usage permits it; syntax selects the function.
- Form verbal nouns without an added derivational suffix when the established term already serves that role.
- Prefer native compounds and fused modifier-head forms; topic 1 governs separators.

## 17. Preposition and sandhi selection

- Context-conditioned preposition forms, sound-triggered sandhi selection, elision, and contraction are not applicable to Chinese literals.

## 18. Card/record-label register

- Use `<mnemonic> 卡` as the fixed short designator in dialog and editor titles.
- Use `<mnemonic> 记录` in running prose; use a full descriptive record name only when the mnemonic lacks sufficient context.
- Keep the generic noun uncapitalized because Chinese has no case; reserve 卡 for titles and 记录 for body prose.
- Keep each register internally consistent and do not cross-convert the short and long forms.

## 19. Multi-paragraph and whitespace fidelity

- Preserve source paragraph breaks at the same positions, including blank lines and single line breaks that carry meaning.
- Drop clauses absent from the current source; do not retain text inherited from an older source.
- Preserve source trailing newlines and punctuation.
- Preserve semantic line breaks; omit visual wrapping absent from the source literal.
- Preserve complete meaning; do not truncate or abbreviate text to fit an assumed display size.

## 20. Current-source fidelity

- Derive each translation from the current source literal and its supplied context.
- Reuse an inherited translation only when its complete meaning matches the current source.
- Treat inherited forms of `color projection` and `scale family / color tone` as unsafe unless they retain the canonical terms 色彩投影 and 色阶族.

## 21. Script hygiene

- Use Simplified Chinese characters in translated prose with zero wrong-script or Latin/CJK homoglyph substitutions.
- Permit non-Chinese characters only inside retained tokens whose literal spelling requires them.
- Translate ordinary foreign prose; retain only identifiers, units, symbols, proper names, and other tokens fixed by topic 4.

## 22. Rule-file scope hygiene

- Keep only current decisions that can alter wording, Unicode characters, punctuation, capitalization, embedded tokens, mnemonic markers, or semantic whitespace in a translated literal.
- Omit catalog representation, serialization, headers, flags, validation workflow, rendering, fonts, shaping, layout, widget behavior, runtime formatting, sorting, search, display sizing, audit history, completion state, review metadata, provenance, and citations.
- State each decision directly; omit implementation procedures, correction history, bibliographies, and alternative policy sources.

## 23. Section-disjointness declaration

- Keep script and orthography in topic 1, literal-token and punctuation mechanics in topics 2-6 and 9, interface phrasing and structure in topics 7 and 18-20, and address register in topic 8.
- Assign each decision to one axis only; do not duplicate a concept across these sections.

## 24. Developer/debug-string policy

- Translate developer-facing diagnostics into terse technical Chinese; retain source component prefixes with their literal ASCII colon.
- Preserve identifiers, function names, configuration keys, format specifiers, and retained tokens verbatim in every diagnostic.
- Use the same policy across sibling diagnostics within a subsystem unless an established sibling family fixes a different Chinese phrasing pattern.
- Classify commands, labels, dialogs, confirmations, tooltips, status messages, and errors as user-facing; translate all of them completely.
- Classify progress and state reports as informational; translate them completely in concise declarative form.
- Classify assertions, allocator reports, parser traces, and internal subsystem diagnostics as developer-facing; translate their prose in terse technical form.
- Use priority only to order review; never leave applicable user-facing or informational text untranslated.
- Preserve embedded format and retained tokens in every translated family regardless of review priority.
- Render the `BUG:` diagnostic prefix as "缺陷：", held distinct from the "错误：" prefix that carries ordinary failures; the two severities never share a word.
- Keep an identifier-plus-value diagnostic pair contiguous (eg `sweep_state=%d`); a qualifier precedes the whole pair and never stands between the identifier and its `=`.
