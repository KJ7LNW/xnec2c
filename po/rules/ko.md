# ko translation rules

## 1. Script and orthography

- Scope: use contemporary South Korean standard Korean in composed Hangul for professional electrical and radio-frequency engineers, antenna designers, and amateur-radio operators.
- Write native prose with the 19 modern initial consonants, 21 modern vowels, and 27 modern final consonants of Hangul syllable blocks; retain no obsolete letters or non-Hangul logographs in prose.
- Preserve every required vowel and final consonant; never strip, fold, or replace one with a look-alike.
- Encode Hangul as precomposed NFC syllables; emit no decomposed or standalone jamo.
- Native Korean requires no apostrophe, modifier letter, or textual joiner; preserve source code points only inside retained technical tokens.
- Write left-to-right; retained Latin, Greek, and numeric tokens remain left-to-right and are never mirrored or given manual direction controls.
- Hangul has no letter case; topic 6 governs only retained cased tokens and mnemonic letters.
- Follow contemporary South Korean standard spelling and spacing; use no obsolete spelling or regional alternative.
- Separate independent words and attach particles to their nouns; place one space between Korean prose and an embedded Latin or numeric token unless punctuation intervenes.
- Fuse established technical compounds, space ordinary noun phrases, and use a hyphen only when a retained source token requires it.

## 2. Numerals in literals

- Write literal technical values with decimal digits rather than Korean numeral words for consistent engineering notation.
- Use `.` as the decimal separator and `,` as the thousands separator in numbers physically present in translated prose.
- Preserve formulas, examples, fixed defaults, named mathematical constants, standards constants, and numbers inside format specifiers exactly; their fixed notation is part of the retained token.
- Form an ordinal or index with a decimal digit plus the natural Korean counter or index noun, eg `1번`; distinguish a following index full stop from a decimal point by its grammatical role.

## 3. Punctuation and quotation

- Use Korean double quotation marks `“…”` for ordinary quoted prose and single quotation marks `‘…’` for a quote within a quote; keep source straight quotes only when they delimit a retained technical token.
- Use `,`, `?`, and `!` as Korean prose punctuation; use `.` to close a full sentence and omit terminal punctuation from a short label or fragment.
- Place no space before a colon, semicolon, comma, or terminal mark; place one space after it when more prose follows.
- Use the single ellipsis character `…`; preserve a source hyphen or dash when it belongs to a retained token, and otherwise use punctuation natural to the Korean sentence.
- Keep punctuation inside identifiers, units, mnemonics, formulas, filenames, and format specifiers exactly in source form for token integrity.

## 4. Never-translate tokens

Treat every retained token as an exact literal boundary; translation or transliteration failures are zero-tolerance.

- Keep NEC2 card mnemonics verbatim: `GW GA GH EX LD FR RP GE EN SP SM SC NE NH NT TL GN GD GC GX GR GS GM GF CP CM SY XQ EK KH PQ PT`.
- Keep unit symbols verbatim: `Hz MHz GHz dB dBi Ω % K S/m deg °`.
- Keep figure-of-merit and parameter tokens verbatim: `VSWR`, all `S`-parameter forms, `Z`, `Z0`, `Zin`, `F/B`, and `G/T`.
- Keep source file extensions and literal filenames verbatim, including `.nec`, `.csv`, `.s1p`, `.s2p`, and `.png`.
- Keep every format specifier verbatim under topic 5.
- Keep embedded function names, variable names, configuration keys, and other code identifiers verbatim inside topic 24 strings.
- Keep physical and mathematical symbol letters verbatim, including `E`, `H`, `Z`, `θ`, `φ`, and axis letters; a Hangul look-alike changes the notation.
- Keep product, library, toolkit, and tool names verbatim: `xnec2c`, `GTK`, `Cairo`, and `OpenGL`; write the chart proper name as `스미스 차트` for established engineering consistency.
- Keep named transfer functions and algorithms verbatim: `Reinhard`, `Sigmoid`, `Asinh`, `Log`, `Identity`, and `μ-law`; translate descriptive `Power` as `파워(거듭제곱) 계열` to preserve its power-law sense.
- Transliterate the conditional geometry loanwords as `세그먼트`, `패치`, and `태그`, and process `fork` as `포크`; translate other geometry words according to topic 10.
- Apply topic 1 directionality and spacing around every retained token without changing any character inside it.

## 5. Format-specifier integrity

- Preserve every source format specifier exactly with zero missing, added, or altered conversions; keep numbers inside a specifier unchanged.
- Keep source order by default and restructure Korean wording around fixed specifier positions.
- Use positional forms, eg `%1$s`, only when natural Korean information order requires referents in a different sequence; preserve the same referent set and conversion types.

## 6. Capitalization and title-case

- Hangul has no case distinction; labels, menu items, and titles therefore use the same Korean letter forms rather than imitating source title case.
- Preserve the source case of retained acronyms, proper nouns, unit symbols, and axis letters because case can carry technical meaning.
- Keep a lowercase mathematical or coordinate variable lowercase, even at the start of a Korean label.
- Preserve each source-capitalized retained element in a coordinated option name; Korean words in the same name acquire no capitalization.
- Write the generic card noun `카드` identically in every sentence position; only its retained mnemonic or proper noun keeps source case.
- Apply topic 9 independently to the mnemonic letter inside an accelerator-marked literal.

## 7. Interface register by string type

Treat each string family as its own Korean interface grammar; preserve all source meaning without imitating source length or word order.

- Commands, buttons, and menu actions use the shortest complete action noun or noun phrase; omit subject and object when the controlled entity supplies them, place a required object before the action noun, and use no finite verb ending.
- Field labels use a concise noun phrase in modifier-before-head order followed by the source colon; omit subject, verb, and particles unless needed to distinguish the field.
- Dialog statements use full impersonal 합니다/습니다 sentences in subject-object-verb order; lead with the affected entity or condition and end with the result or action.
- Confirmation questions use a full 합니다/습니까 sentence; state the proposed action before its consequence and place the finite verb last.
- Tooltips use full declarative 합니다/습니다 sentences; name the control's effect first and state the source-provided reason for unavailability after the condition.
- Status and error messages use impersonal declarative 합니다/습니다 sentences; present the affected entity or condition before the result, cause, or recovery information and place the verb last.
- User-visible domain entities use topic 10's locked term as the head noun, with class, state, or quantity modifiers before it; topic 8 governs personal names and address.
- Prefer established compounds in short labels and natural multiword phrases in sentences; omit no semantic distinction and invent no abbreviation for width.
- Topic 24 alone governs developer and debug strings.

## 8. Formality and address

- Use formal neutral 합니다체/습니다체 for every full interface sentence; use no casual, slang, commercial, archaic, intimate, or excessively deferential register.
- Commands and menu labels use topic 7's endingless action-noun form; they carry no separate address level.
- Omit first- and second-person pronouns when an impersonal construction is natural; introduce `사용자` only when the source explicitly distinguishes the user from another actor.
- Korean has no grammatical-gender agreement; introduce no gendered pronoun, occupational title, or default gender absent from the source.
- Express inclusiveness through neutral role nouns and impersonal constructions while preserving any source-required number or animate participant distinction.
- Use no honorific title or honorific subject marker unless the source addresses a named person; when a person is named, retain the supplied name order and use third person only when grammar requires it.
- Realize formality through the sentence-final 합니다/습니다 ending rather than a separate honorific vocabulary layer.
- Form confirmations as full formal questions: state the action, then any consequence, and end with `습니까?`.

## 9. Accelerator/hotkey mnemonics

- Preserve a source mnemonic with one `_` immediately before a typable character in the translated label; the marker remains part of the literal.
- Choose a Hangul character from the translated term for a pure-Korean label; never append or invent a Latin transliteration.
- When the translated label contains a retained Latin term, the mnemonic can mark a source-cased letter in that term when this gives the shortest natural label.
- Add no mnemonic marker when the source carries none, and preserve no source mnemonic letter merely because it occupied the corresponding source position.
- Place the marker before a character available on a standard Korean keyboard layout; avoid punctuation, combining jamo, and characters outside the translated term.

## 10. Domain lexicon

Concept keys are source-domain vocabulary; the Korean column contains the locked target terms.

### Electrical primitives

| concept | 한국어 | sense / hazard guarded |
|---|---|---|
| current | 전류 | electrical current (A); never 현재/최근 (temporal) |
| charge | 전하 | electrical charge (C); never billing/fee/cargo |
| voltage | 전압 | electric potential |
| power (electrical) | 전력 | radiated/dissipated watts, power gain/flow; distinct from `Power` transfer-family name (파워 계열, topic 4) and from a math power-law |
| impedance | 임피던스 | complex Z; distinct from 저항/리액턴스 |
| resistance | 저항 | real part of Z; distinct from impedance and load |
| reactance | 리액턴스 | imaginary part of Z |
| inductance | 인덕턴스 | established engineering sense; prevents synonym drift |
| capacitance | 커패시턴스 | established engineering sense; prevents synonym drift |
| conductivity | 전도율 | material S/m |
| admittance | 어드미턴스 | admittance-matrix sense; distinct from impedance |
| load | 부하 | LD-card impedance load; never physical weight; not merged with 전하 (charge) |
| gain | 이득 | antenna directivity ratio (dB); never profit or amplifier amplification |
| excitation | 여기 | EM energy input/source; never emotional excitement; see topic 11 for the locative-여기 collision |
| feedpoint | 급전점 | antenna feed point |
| port | 포트 | excitation/S-parameter port |
| radials | 방사형 도선 | ground-plane radial wires (noun); distinct from the adjective 방사형 alone |

### Ground and earth

| concept | 한국어 | sense / hazard guarded |
|---|---|---|
| ground / ground plane | 접지 / 접지면 | RF electrical reference plane, GN/GD ground cards; never soil; 접지 for bare ground/grounding, 접지면 for "ground plane" |
| earth (physical medium) | 대지 | terrain/noise-model earth, "below ground" geometry; distinct from electrical 접지 |
| ground wave | 지표파 | propagation term; distinct from 접지 |

### Geometry primitives

| concept | 한국어 | sense / hazard guarded |
|---|---|---|
| wire | 도선 | thin conductor/GW element; never 케이블/코드 |
| segment | 세그먼트 | NEC2 geometry subdivision |
| patch | 패치 | NEC2 surface patch (SP/SM); transliterated, not translated (topic 4) |
| tag | 태그 | NEC2 geometry identifier; never a UI label or a card |
| card | 카드 | NEC2 input record; register per topic 18 |
| kernel | 커널 | integral-equation/thin-wire kernel; never an OS kernel |
| cliff | 클리프 | two-medium ground-boundary type name; never a fracture/break |
| structure | 구조체 | the antenna model geometry; never "건설" (construction) |
| model | 모델 | NEC model or noise-temperature model |
| geometry | 형상 | the model geometry |
| crossed | 교차된 | transmission-line conductors crossed/reversed; never cut/severed |

### Field, pattern, viewer

| concept | 한국어 | sense / hazard guarded |
|---|---|---|
| field (EM) | 장 | electromagnetic field; distinct from a data or configuration field |
| near field | 근접장 | spatial region near the antenna; paired consistently with 원거리장 |
| far field | 원거리장 | spatial region far from the antenna; paired consistently with 근접장 |
| far-field contribution | 원거리장 기여 | per-direction contribution; distinct from near-field animation |
| radiation | 방사 | electromagnetic emission; prevents the nontechnical exposure sense |
| radiation pattern | 방사 패턴 | plotted directional response; distinct from a template, design, and far-field region |
| gain pattern | 이득 패턴 | gain-valued radiation pattern; preserves its relation to 방사 패턴 |
| polarization | 편파 | antenna or wave-field orientation; distinct from 극성 |
| polarity | 극성 | sign of a quantity; distinct from 편파 |
| phase | 위상 | angular phase of a field or signal; preserves the engineering sense |
| reference phase | 기준 위상 | phase used as the comparison origin; preserves its relation to 위상 |
| frequency | 주파수 | oscillation rate; preserves the radio-frequency sense |
| wave / wavelength | 파 / 파장 | propagating oscillation and its spatial period; keeps the two related concepts distinct |
| standing wave | 정재파 | stationary interference pattern; paired consistently with 진행파 |
| traveling wave | 진행파 | propagating wave; paired consistently with 정재파 |
| node / antinode | 마디 / 배 | standing-wave zero and maximum, including the overlay sense; distinct from a generic numeric zero |
| crest | 마루 | instantaneous wave apex; distinct from a curve or step peak |
| magnitude | 크기 | modulus or scalar size; distinct from 진폭 |
| amplitude | 진폭 | oscillating-quantity peak; distinct from 크기 |
| peak value | 피크값 | value at a peak; distinct from 피크 크기 |
| peak magnitude | 피크 크기 | peak modulus option; distinct from 피크값 |
| instantaneous | 순시 | projection at one phase; add `(φ=0)` only when the source carries it |
| Poynting vector | 포인팅 벡터 | electromagnetic power-flow vector; preserves the named technical quantity |
| solid angle | 입체각 | three-dimensional angular measure; distinct from a planar angle |
| net gain | 순이득 | total gain after mismatch effects; distinct from real-part gain |
| viewer | 뷰어 | observation or 3D-view entity; distinct from observer, speaker, and preview |
| flow / flow direction | 흐름 / 흐름 방향 | current or patch flow and its direction; preserves the directional distinction |
| total field | 전체장 | combined electromagnetic field; distinct from an individual component |

### Color, tone, animation subsystem

| concept | 한국어 | sense / hazard guarded |
|---|---|---|
| color | 색상 | established engineering sense; prevents synonym drift |
| color projection | 색상 투영 | which quantity drives hue |
| hue | 색조 | color-wheel angle |
| brightness | 명도 | luminance channel |
| hue encoding | 색조 인코딩 | distinct enum from 명도 인코딩; neither collapses to 색상 투영 |
| brightness encoding | 명도 인코딩 | distinct enum from 색조 인코딩; neither collapses to 색상 투영 |
| color scale | 색상 스케일 | magnitude-to-color scale |
| scale family / color tone | 색조 계열 | the transfer-curve family (Power/Log/Asinh/μ-law/Reinhard/Sigmoid/Identity); one concept, one term, regardless of source spelling |
| palette | 팔레트 | established engineering sense; prevents synonym drift |
| palette kind | 팔레트 종류 | palette-layout enum; distinct from 색조 계열 and 색상 투영 |
| ramp / gradient | 그라디언트 | a palette kind/linear color strip |
| gamma | 감마 | power-law exponent |
| knee | 니 포인트 | soft-knee bend point |
| softening | 완화 | dynamic-range softening |
| compression | 압축 | dynamic-range compression |
| contrast | 대비 | established engineering sense; prevents synonym drift |
| dynamic range | 다이내믹 레인지 | established engineering sense; prevents synonym drift |
| floor | 플로어 | minimum/lower clamp (brightness/dB floor); never a room floor |
| envelope | 엔벨로프 | magnitude/amplitude envelope |
| comet | 코멧 | moving-crest overlay effect; distinct from geometry and unsafe to inherit across those senses |
| overlay (noun) | 오버레이 | an added visual layer; distinct from the verb 오버레이하다 |
| animate / animation | 애니메이션 | established engineering sense; prevents synonym drift |
| animated / static | 동적 / 정적 | category-header adjectives (dynamic vs phase-invariant) |
| projection | 투영 | color or geometry projection |
| scale | 스케일 | verb (스케일하다) and noun (스케일) |
| wireframe | 와이어프레임 | wire-mesh render mode |
| identity | 항등 | no-op/passthrough transfer; distinct from 유니티 (Smith-chart unity) |
| sentinel | 센티널 | unreachable-case guard value |
| bins | 구간 | discretization buckets |
| companding | 컴팬딩 | bounded log curve (μ-law) |
| tone mapping | 톤 매핑 | photographic tone-map |

### Render and compute

| concept | 한국어 | sense / hazard guarded |
|---|---|---|
| renderer | 렌더러 | drawing backend; never "렌더 엔진" (render engine) |
| shader | 셰이더 | established engineering sense; prevents synonym drift |
| allocation (memory) / managed allocator | 할당 / 관리형 할당자 | allocation and the allocator/report |
| thread | 스레드 | compute thread; context disambiguates from 도선 (wire) |
| widget | 위젯 | UI element |
| validation | 검증 | the validation-tree feature |
| batch mode | 배치 모드 | established engineering sense; prevents synonym drift |
| fork (process) | 포크 | process fork; kept as loanword |
| deadlock | 데드락 | established engineering sense; prevents synonym drift |
| notifier | 노티파이어 | established engineering sense; prevents synonym drift |
| token / operand / operator / arity | 토큰 / 피연산자 / 연산자 / 항수 | expression-parser terms |
| override | 오버라이드 | supersede a value (SY symbol); never 덮어쓰기 (overwrite) |
| swap | 스왑 | exchange |
| theme | 테마 | UI/color theme; never "주제" (topic/subject) |

### Metrics and miscellaneous

| concept | 한국어 | sense / hazard guarded |
|---|---|---|
| noise / noise temperature | 잡음 / 잡음 온도 | electronic/thermal noise; never acoustic racket |
| efficiency | 효율 | established engineering sense; prevents synonym drift |
| interpolation | 보간 | established engineering sense; prevents synonym drift |
| mnemonic | 니모닉 | a card's code descriptor; never a memo/note |
| degrees / deg | 도 / (deg) | freestanding axis/prose "도" vs the parenthetical unit tag "(deg)"; tag kept per topic 4 |
| diameter | 직경 | canonical Sino-Korean engineering term; 지름 (native synonym) is rejected for catalog consistency |
| reflect | 미러 (지오메트리) / 따라 움직이다 (동작 추종) / 반사 (물리) | three distinct senses: geometry mirror op, behavioral tracking ("mirrors …"), physics reflection; never merged |
| default(s) | 기본값 | fallback value |
| normalize / normalization | 정규화 | translated, not transliterated |

## 11. Disambiguation policy

- Use the correct technical sense from topic 10 without adding a qualifier absent from the source (eg "전류" alone for "View Currents", not "전기 전류").
- Add a qualifying word only where Korean usage would otherwise be genuinely ambiguous within the specific UI context; program context already disambiguates in the general case.
- Accepted intra-domain homonym: 여기 (excitation) also spells the locative "here"; never let both senses collide in one string. Reword the locative (eg "이 목록에", "여기서" only when no excitation term shares the string) so the sole remaining 여기 reads as excitation.
- Accepted homonym: 스레드 (compute thread) vs 도선 (wire); context disambiguates, no qualifier added.
- Gerund vs noun: 스케일 covers both the verb (스케일하다) and the noun (스케일) sense of an overloaded source word "scale"; the 하다 suffix marks the verb form.

## 12. Cross-catalog consistency

- One term per concept: every occurrence of a topic 10 concept reuses its locked term; no synonym is introduced for a concept already mapped.
- Keep every Appendix C pair distinct:
  - `polarity` 극성 vs `polarization` 편파
  - `magnitude` 크기 vs `amplitude` 진폭
  - `peak value` 피크값 vs `peak magnitude` 피크 크기
  - `ground` 접지 vs `earth` 대지
  - `load` 부하 vs `charge` 전하
  - `gain` 이득 vs `amplification` 증폭 vs `profit` 이익
  - electrical `current` 전류 vs temporal `current` 현재
  - electrical `charge` 전하 vs billing `charge` 요금
  - `wire` 도선 vs `cable` 케이블 vs `cord` 코드 vs compute `thread` 스레드
  - `radiation pattern` 방사 패턴 vs `template` 서식 vs `design` 설계 vs `far field` 원거리장
  - electromagnetic `excitation` 여기 vs emotional `excitement` 흥분
  - standing-wave `node` 마디 and `antinode` 배 vs numeric `null` 널 and `zero` 영
  - `scale family` and `color tone` 색조 계열 vs `hue` 색조 vs `palette kind` 팔레트 종류 vs `color projection` 색상 투영
  - overlay `comet` 코멧 vs `geometry` 형상
  - transfer `identity` 항등 vs Smith-chart `unity` 유니티
  - `renderer` 렌더러 vs `render engine` 렌더 엔진
  - `override` 오버라이드 vs `overwrite` 덮어쓰기
  - `viewer` 뷰어 vs `observer` 관측자 vs `speaker` 발표자 vs `preview` 미리 보기
  - geometry `reflect` 미러 vs behavioral `mirrors` 따라 움직이다 vs physical `reflection` 반사
  - model `structure` 구조체 vs `construction` 건설
  - interface `theme` 테마 vs `topic` 주제
  - validation-tree `validation` 검증 vs procedural `verification` 확인
  - `net gain` 순이득 vs `real-part gain` 실수부 이득
  - electrical `power` 전력 vs transfer-family `Power` 파워 계열
- Loanword vs native: prefer the loanword already established in Korean engineering usage (topic 10) over coining a native paraphrase; lock the spelling given in the table as canonical.
- Minority-outlier spellings (eg alternate transliterations of a loanword) unify to the table's canonical spelling.
- Consistency priority outranks locale-form preference for 직경 over 지름 (topic 10) and for every locked loanword spelling in topic 10.

## 13. Priority ordering

- Precedence chain: correct meaning (topic 10/11) first, then interface convention (topic 7/8), then catalog consistency (topic 12), then disambiguation (topic 11), then locale numeral form (topic 2).
- Explicit override: 직경 over 지름 is a topic-12 consistency ruling that outranks the native-synonym preference topic 10 would otherwise allow.

## 14. Grammatical number

- Korean nouns, adjectives, participles, and verbs do not inflect for singular, dual, or plural in translated literals; preserve number through context or an explicit count rather than invented agreement.
- After an explicit literal count, use the uninflected noun with the natural Korean counter when the concept requires one; attach no plural suffix.
- Omit `들` by default; use it only when the source explicitly contrasts a group with one member and omission would erase that contrast.

## 15. Grammatical agreement

- Not applicable: Korean has no grammatical gender, noun declension class, adjective or participle concord, partitive-after-count form, required reflexive agreement particle, or implied-head default gender; introduce none in standalone labels.

## 16. Morphological derivation

- Form a verb from a locked loanword noun with `하다`, eg `오버라이드하다`; do not replace that stem with a newly coined native verb or attach an ad hoc foreign suffix.
- Use the locked noun itself as its verbal noun, eg `오버레이`; attach `하다` only where the literal requires a finite or attributive verb.
- Preserve topic 10's native-term or loanword choice; form general Korean technical compounds according to topic 1 rather than transliterating a source compound word by word.

## 17. Preposition and sandhi selection

- Particle allomorphy is conditioned by whether the preceding syllable ends in a consonant (받침) or a vowel: 이/가, 을/를, 은/는, 와/과, (으)로; select the form matching the actual preceding syllable of the translated term, not the source word.
- No elision/contraction beyond standard particle allomorphy is required in this catalog.

## 18. Card/record-label register

- Dialog/editor titles use the fixed designator form "<mnemonic> 카드" (eg "GW 카드 편집").
- Running prose in messages uses the same "<mnemonic> 카드" noun phrase; Korean has no separate hyphenated/suffixed running-prose variant, so the designator form is reused verbatim in prose.
- Generic-noun casing: 카드 is never capitalized (topic 6); no short-vs-long form distinction exists for 카드 in this catalog.
- Each register stays internally consistent per string type; a title-form label is never rewritten into a different running-prose form.

## 19. Multi-paragraph and whitespace fidelity

- Mirror every source paragraph break at the same semantic position, preserving the distinction between a blank line and a single line break.
- Preserve a source semantic line break and add no line break solely for visual wrapping.
- Drop a trailing clause absent from the current source rather than inheriting it from an older translation.
- Carry source trailing newlines and punctuation exactly unless Korean sentence grammar requires the corresponding topic 3 punctuation.
- Preserve complete natural meaning; never truncate wording or invent an abbreviation to satisfy an assumed display width.

## 20. Current-source fidelity

- Derive every translated literal from the complete current source text and its supplied context.
- Reuse inherited Korean wording only when its full meaning agrees with the current source; source meaning overrides prior wording.
- Treat these inherited mappings as unsafe without current contextual agreement: 코멧 vs 형상, 근접장 vs 원거리장, 마디/배 vs numeric 영, and 피크값 vs 피크 크기.

## 21. Script hygiene

- Permit zero non-Hangul logographs, kana, decomposed jamo, or foreign-script prose characters in translated Korean text; allow foreign characters only inside a topic 4 retained token whose spelling requires them.
- Hangul syllable blocks share no operative homoglyph class with retained Latin or Greek technical notation; preserve the retained token instead of substituting a visually similar character.
- Translate ordinary foreign prose and apply topic 10 to approved loanwords; keep only genuine identifiers, units, symbols, and named terms under topic 4.

## 22. Rule-file scope hygiene

- Keep only current decisions that can alter a translated literal's wording, Unicode characters, punctuation, capitalization, embedded tokens, mnemonic marker, or semantic whitespace.
- Omit catalog representation, serialization, headers, flags, validation workflow, rendering, fonts, shaping, layout, widget behavior, runtime formatting, sorting, search, display sizing, audit history, completion state, review metadata, provenance, and source citations.
- State each retained decision directly; include no implementation procedure, bibliography, correction history, alternative policy source, or completion claim.

## 23. Section-disjointness declaration

- Topic 1 (script/orthography) governs writing-system mechanics only.
- Topics 7, 9, 16-18 govern phrasing/structure (string form by type, mnemonic placement, derivation, register-by-record) only.
- Topic 8 governs address/formality register only.
- These three axes are non-overlapping: a given concept is classified under exactly one of them, never split or duplicated across them.

## 24. Developer/debug-string policy

- Translate user-facing errors, warnings, notices, dialogs, tooltips, status text, and command results; review priority never permits an applicable user-facing string to remain untranslated.
- Translate informational diagnostics intended to explain program state to an operator in concise technical 합니다/습니다 prose.
- Translate developer-facing debug and trace strings in terse technical Korean; omit conversational address and retain a subsystem's established sibling-string terminology.
- Keep identifiers, function names, variable names, configuration keys, format specifiers, and topic 4 tokens verbatim in every family.
- Preserve embedded format and retained tokens independently of string-family priority.
