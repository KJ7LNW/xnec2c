# pt translation rules

## 1. Script and orthography

- Locale scope: Brazilian Portuguese (`pt-BR`) in Latin script under the post-1990 Orthographic Agreement, for professional electrical and radio-frequency engineers, antenna designers, and amateur-radio operators in Brazil.
- Diacritics: á â ã à ç é ê í ó ô õ ú; nasal vowels (ã, õ) and cedilla (ç) are mandatory, never dropped or folded to look-alikes.
- Precomposed form: use precomposed Unicode code points (eg U+00E3 ã), not combining-diacritic sequences.
- Ambiguous marks: use straight ASCII apostrophe/quote for structural argument-name delimiters, never a typographic look-alike.
- Joining/shaping mechanics: not applicable, Latin script has no positional letterforms or ligature rules here.
- Directionality: left-to-right; not applicable, no bidirectional text occurs in this catalog.
- Case distinction: Latin script has upper/lower case; casing rules apply in full (topic 6).
- Inter-word spacing: single space between words; embedded foreign/numeric tokens take the same single space.
- Compound formation: spaced or hyphenated per Brazilian norms, never fused (eg "ganho líquido", not "ganholíquido").

## 2. Numerals in literals

- Use digits `0`-`9` for technical values in translated prose.
- Use comma as the decimal separator and period as the thousands separator for numbers physically present in translated prose.
- Preserve formulas, examples, fixed defaults, named mathematical constants, standards constants, and numbers inside format specifiers exactly in source form.
- Attach ordinal suffix "º" or "ª" directly to its digit (eg "1º"); keep an ordinal or index separator distinct from the decimal comma.

## 3. Punctuation and quotation

- Native quotation marks: Portuguese prose normally uses «...»; this catalog keeps straight `'...'` for structural argument-name delimiters, not prose quotation, for consistency with source parsing.
- Native punctuation replaces source counterparts in prose (comma, question mark, exclamation mark); Brazilian Portuguese has no inverted opening marks (¿ ¡).
- Spacing: no space before `:` or `;`; one space after, matching source.
- Ellipsis: three dots "..."; dashes preserved as the source em/en dash.
- Sentence terminator: period on full sentences and tooltips; short labels, menu items, and fragments omit the terminal period.
- Punctuation inside embedded technical runs (format specifiers, file paths, code tokens) stays in source form, never localized.

## 4. Never-translate tokens

- Preserve every token in this section exactly in its stated form; translate or transliterate none of them.
- NEC2 card mnemonics: GW GA GH EX LD FR RP GE EN SP SM SC NE NH NT TL GN GD GC GX GR GS GM GF CP CM SY XQ EK KH PQ PT — kept verbatim, uppercase, unchanged; round-trip file-format identifiers.
- Unit symbols: MHz, dBi, Ω, %, deg — kept verbatim.
- Figure-of-merit tokens: VSWR, S-parameter tokens, Z, Zo, front/back-ratio and gain/temperature tokens — kept verbatim.
- File extensions and literal filenames: .nec, .csv, .s1p, .s2p, .png — kept verbatim.
- Format specifiers (`%f` `%d` `%s`): kept verbatim, never localized (topic 5).
- Embedded identifiers: function names, variable names, config keys inside developer strings — kept verbatim.
- Physical/mathematical symbol letters (E, H, θ, φ, exponent letters): kept in source form.
- Product/library/tool names: xnec2c, NEC2 — kept verbatim, proper nouns.
- Named transfer-function/algorithm terms: Log, Asinh, μ-law, Reinhard, Sigmoid, Identity — kept verbatim as proper/technical names; `Power`, the descriptive transfer-family name, translates to "Lei de potência" so it remains distinct from electrical "potência".
- Conditional loanwords: `patch` and `tag` kept verbatim (NEC2 geometry terms); `segment` translates to "segmento"; `fork` (process) kept verbatim.
- Preserve every retained token in left-to-right source order under topic 1; do not reverse or mirror its characters.

## 5. Format-specifier integrity

- Every specifier from the source is preserved, same set, never dropped or added.
- Default ordering matches source order; positional reordering (`%1$s`) is allowed only when Brazilian Portuguese word order genuinely requires it.
- Prefer restructuring the sentence around fixed specifier positions over reordering specifiers.
- Numbers inside specifiers are never localized (no comma-for-decimal substitution).

## 6. Capitalization and title-case

- Use sentence case for sentences, tooltips, labels, menu items, and titles; capitalize only the first word and terms that independently require capitals, never every principal word.
- Keep axis designators `X`, `Y`, and `Z` uppercase, including when the source uses lowercase.
- Keep lowercase mathematical and coordinate variables lowercase unless they are the axis designators above.
- In a coordinated option name, capitalize only the first element unless another element is a proper noun, acronym, or axis designator.
- Write the generic card noun "cartão" lowercase in running prose and capitalize it at the start of dialog or editor titles under topic 18.
- Preserve the canonical casing of acronyms, proper nouns, retained tokens, and product names.

## 7. Interface register by string type

- Commands, buttons, and menu actions: use a subjectless infinitive for action names and a subjectless formal-neutral imperative for direct instructions; place the verb before its object and avoid nominalization when the action is primary.
- Field labels: use a concise noun phrase in head-first order and retain the source colon; omit subjects, verbs, and articles unless Portuguese grammar requires them.
- Dialogs and confirmations: use full formal-neutral sentences in subject-verb-object order; omit the user pronoun when the verb form identifies the addressee, and place the decision or requested action before supporting detail.
- Tooltips: use full declarative sentences in subject-verb-object order; explain the control's function first and, when the source states it, the reason a disabled control is unavailable second.
- Status and error messages: use subjectless impersonal or declarative sentences; place the affected entity before its state or outcome and use passive constructions only when the actor is irrelevant.
- User-visible domain entities: use the canonical noun from topic 10 followed by its distinguishing modifier, except fixed proper names and NEC2 mnemonics retain their source order.
- Use the shortest complete natural form for each string type; preserve all meaning, avoid unnatural abbreviations, and do not imitate source length.
- Form technical compounds as natural spaced or hyphenated Portuguese phrases under topic 1; do not force source-language modifier order.
- Developer and debug strings follow topic 24.

## 8. Formality and address

- Use the formal-neutral second-person register associated with "você" and omit the pronoun when the verb form identifies the addressee; forbid informal "tu" forms.
- Realize formality through third-person imperative endings and neutral word choice, not honorifics.
- Commands and buttons use the subjectless forms in topic 7; dialogs use full formal-neutral sentences.
- Confirmation dialogs state the proposed action and request confirmation without naming the user.
- Avoid gender-marking for the user through impersonal or subjectless phrasing; retain grammatical gender and number agreement for domain nouns.
- Use inclusive role or group nouns without invented gender-neutral spellings; choose natural collective or impersonal constructions when people are referenced.
- Grammatical person is second-person formal-neutral when addressing the user and third-person declarative otherwise.
- Honorifics are not applicable to this interface; personal names retain their supplied order and spelling.
- Forbid casual, slang, commercial, archaic, and bureaucratic registers.

## 9. Accelerator/hotkey mnemonics

- Marker: leading underscore before the accelerator letter (eg "_Potência").
- Per-container uniqueness: on collision within the same menu/dialog, shift the underscore to a different, still-mnemonic, typable letter.
- The mnemonic letter comes from the translated pt term, never a transliteration of the source letter.
- Non-Latin presentation (parenthetical Latin letter): not applicable, Portuguese already uses Latin script.
- Source-presence-only: never invent a mnemonic where the source string carries none.
- Typability: use common Brazilian-keyboard letters; no dead-key-only letters as sole accelerator.

## 10. Domain lexicon

### Electrical primitives

- `current` → corrente — electrical current, Amperes; never "atual" (temporal sense).
- `charge` → carga elétrica — electrical charge, Coulombs; never "cobrança" (billing) and never the impedance-load term "carga"; the qualifier preserves the required distinction from `load`.
- `voltage` → tensão — electric potential.
- `power (electrical)` → potência — radiated/dissipated watts, power gain, and power flow; distinct from the `Power` transfer-family term "Lei de potência" and from a mathematical exponentiation operation.
- `impedance` → impedância — complex Z; distinct from resistance and reactance.
- `resistance` → resistência — real part of Z; distinct from impedância and from `load`.
- `reactance` → reatância — imaginary part of Z.
- `inductance` → indutância.
- `capacitance` → capacitância.
- `conductivity` → condutividade — material S/m.
- `admittance` → admitância — admittance-matrix sense; distinct from impedância.
- `load` → carga — LD-card impedance load; never physical weight/burden ("peso"/"carregamento") and never electrical `charge` ("carga elétrica").
- `gain` → ganho — antenna directivity ratio (dB); never "lucro" (profit), never amplifier amplification.
- `excitation` → excitação — EM energy input/source; never the emotional sense.
- `feedpoint` → ponto de alimentação.
- `port` → porta — excitation/S-parameter port; no collision with other domain senses in this catalog.
- `radials` → radiais (noun) — horizontal ground-plane radial wires; distinct from the adjective "radial".

### Ground and earth

- `ground` / `ground plane` → terra / plano de terra — RF electrical reference plane, GN/GD ground cards, ground type/conductivity/effects/model; never "solo"; one term across every ground sub-sense (eg "condutividade da terra", "efeitos da terra", "terra de Sommerfeld", "modelo/tipo de terra").
- `earth (physical medium)` → solo — terrain/noise-model earth, "below ground" geometry; distinct from electrical `ground`.
- `ground wave` → onda de terra — propagation term; distinct from the ground reference plane.

### Geometry primitives

- `wire` → fio — thin conductor/GW element; never "cabo" (cable/cord).
- `segment` → segmento — NEC2 geometry subdivision; translates (not kept as loanword, topic 4).
- `patch` → patch — NEC2 surface patch (SP/SM); kept as loanword file-wide, including the GTK "patches" tab, which names this same geometry term.
- `tag` → tag — NEC2 geometry identifier; kept as loanword file-wide; never a UI label or card.
- `card` → cartão — NEC2 input record; register handled in topic 18.
- `kernel` → kernel — integral-equation/thin-wire kernel, kept as the standard Brazilian CS loanword; not an OS kernel, disambiguated by context.
- `cliff` → descontinuidade de terra — two-medium ground-boundary type; never a fracture/break.
- `structure` → estrutura — the antenna model geometry; never "construção".
- `model` → modelo — NEC model or noise-temperature model.
- `geometry` → geometria — the model geometry.
- `crossed` → cruzados — transmission-line conductors crossed/reversed; never cut/severed.

### Field, pattern, viewer

- `field (EM)` → campo — near/total/E/H field; same pt word as a data/config field, disambiguated by context as in the source.
- `near field` / `far field` → campo próximo / campo distante — opposed spatial regions, kept symmetric.
- `far-field contribution` → contribuição de campo distante — per-direction contribution; not near-field animation.
- `radiation` → radiação — radiated emission.
- `radiation pattern` → diagrama de radiação — plotted directional response; never "modelo" (template/design); "diagrama" used consistently for pattern file-wide, "padrão" reserved for `default(s)` (below) so the two never collide.
- `gain pattern` → diagrama de ganho.
- `polarization` → polarização — antenna/wave field orientation.
- `polarity` → polaridade — sign (+/-) of a quantity; false friend of `polarization`, never conflated.
- `phase` → fase.
- `reference phase` → fase de referência.
- `frequency` → frequência.
- `wave` / `wavelength` → onda / comprimento de onda.
- `standing wave` / `traveling wave` → onda estacionária / onda progressiva — opposed pair.
- `node` / `antinode` → nó / antinó — standing-wave zero/maximum; also carries the null/peak overlay sense.
- `crest` → crista — instantaneous wave apex (comet-head); distinct from a curve/step "pico" (peak).
- `magnitude` → magnitude — modulus of a quantity (|Z|, scalar); distinct from amplitude.
- `amplitude` → amplitude — oscillating-quantity peak; distinct from magnitude.
- `peak value` vs `peak magnitude` → valor de pico vs magnitude de pico — two distinct UI options, never collapsed to one label.
- `instantaneous` → instantâneo — projection mode; add the "(φ=0)" qualifier only where the source carries it.
- `Poynting vector` → vetor de Poynting — proper name, kept.
- `solid angle` → ângulo sólido.
- `net gain` → ganho líquido — total-minus-mismatch gain; never "ganho da parte real".
- `viewer` → visualizador — observation direction and/or the 3D view widget; never "observador".
- `flow` / `flow direction` → fluxo / direção do fluxo — patch/current flow.
- `total field` → campo total.

### Color, tone, animation subsystem

- `color` → cor.
- `color projection` → projeção de cor — which quantity drives hue.
- `hue` → matiz — color-wheel angle.
- `brightness` → brilho — luminance channel.
- `hue encoding` / `brightness encoding` → codificação de matiz / codificação de brilho — distinct internal enums, neither collapses to "projeção de cor".
- `color scale` → escala de cor — magnitude-to-color scale.
- `scale family` / `color tone` → família de escala — one concept, two source spellings (Power/Log/Asinh/μ-law/Reinhard/Sigmoid/Identity), one pt term.
- `palette` / `palette kind` → paleta / tipo de paleta — distinct from scale family and color projection.
- `ramp` / `gradient` → rampa / gradiente — a palette kind / linear color strip.
- `gamma` → gama — power-law exponent.
- `knee` → ponto de joelho — soft-knee bend point.
- `softening` → suavização; `compression` → compressão (dynamic-range); `contrast` → contraste; `dynamic range` → faixa dinâmica.
- `floor` → piso — minimum/lower clamp (brightness/dB floor); never a room floor, disambiguated by context.
- `envelope` → envoltória — magnitude/amplitude envelope.
- `comet` → cometa — moving-crest overlay effect; never `geometry` (estrutura), a known fuzzy-inheritance hazard.
- `overlay (noun)` → sobreposição — an added visual layer; distinct from the verb "sobrepor".
- `animate` / `animation` → animar / animação.
- `animated` / `static` → animado / estático — category-header adjectives (dynamic vs phase-invariant).
- `projection` → projeção — color or geometry projection.
- `scale` → escala (noun) / escalar (verb).
- `wireframe` → wireframe — kept as the standard Brazilian 3D-graphics loanword.
- `identity` → identidade — no-op/passthrough transfer; distinct from unity (Smith-chart).
- `sentinel` → sentinela — unreachable-case guard value.
- `bins` → compartimentos — discretization buckets.
- `companding` → compansão — bounded log curve (μ-law), established Brazilian telecom term.
- `tone mapping` → mapeamento de tons — photographic tone-map.

### Render and compute

- `renderer` → renderizador — drawing backend; never "motor de renderização".
- `shader` → shader — kept as the standard Brazilian graphics-programming loanword.
- `allocation (memory)` / `managed allocator` → alocação (de memória) / alocador gerenciado.
- `thread` → thread — kept as loanword; no collision with `wire` (fio) in Portuguese.
- `widget` → widget — kept as the standard Brazilian GTK/UI loanword.
- `validation` → validação — the validation-tree feature; distinct from "verificação" (checks) where this catalog separates them.
- `batch mode` → modo em lote.
- `fork (process)` → fork — kept verbatim (topic 4).
- `deadlock` → deadlock — kept as standard CS loanword.
- `notifier` → notificador.
- `token` / `operand` / `operator` / `arity` → token (kept) / operando / operador / aridade — expression-parser terms.
- `override` → substituir — supersede a value (SY symbol); distinct from `swap` (overwrite sense stays "sobrescrever" where that concept occurs).
- `swap` → trocar — exchange.
- `theme` → tema — UI/color theme; never "assunto"/"tópico".

### Metrics and miscellaneous

- `noise` / `noise temperature` → ruído / temperatura de ruído — electronic/thermal noise; standard EE usage, no collision with the acoustic sense in this catalog.
- `efficiency` → eficiência.
- `interpolation` → interpolação.
- `mnemonic` → mnemônico — a card's code descriptor; never "memorando"/"nota".
- `degrees` / `deg` → graus (freestanding axis/prose) vs "(deg)" (parenthetical unit tag) — the tag is kept verbatim like other unit tags (topic 4), never translated to "(graus)".
- `diameter` → diâmetro — the sole term, no loanword-vs-native choice needed.
- `reflect` → refletir (geometry mirror operation) vs espelhar (behavioral "mirrors ..." tracking) vs reflexão (physics reflection, noun) — three distinct senses, never conflated.
- `default(s)` → padrão — fallback value; reserved exclusively for this sense so it never collides with `radiation pattern` → "diagrama" (above).
- `normalize` / `normalization` → normalizar / normalização — native affix, never transliterated.

## 11. Disambiguation policy

- The correct technical sense is chosen for each ambiguous term from context, never guessed.
- Do not add a qualifier when program context already disambiguates the term (eg "View Currents" → "Ver Correntes").
- Add a qualifier only when the Portuguese term would otherwise merge distinct required concepts.
- Keep `load` and `charge` distinct: use "carga" for impedance load and "carga elétrica" for electrical charge; never use "peso" or "carregamento" for either concept.
- No locative/other homonym collision is recorded for pt.
- Gerund vs noun senses of an overloaded source word: resolved by context per instance; none recorded as systematically ambiguous in this catalog.

## 12. Cross-catalog consistency

- One term per concept, file-wide; reuse established lexicon; no synonym introduced for an already-mapped concept.
- False-friend pairs, each side kept distinct, never sharing a translation:
  - `polarity` → polaridade vs `polarization` → polarização.
  - `magnitude` → magnitude vs `amplitude` → amplitude.
  - `peak value` → valor de pico vs `peak magnitude` → magnitude de pico.
  - `ground` → terra vs `earth` → solo.
  - `load` → carga vs `charge` → carga elétrica, never shared.
  - `gain` → ganho vs amplifier amplification vs profit ("lucro"), never shared.
  - `current` → corrente vs temporal "atual".
  - `charge` → carga elétrica vs billing ("cobrança").
  - `wire` → fio vs cable/cord ("cabo"); `wire` → fio vs `thread` → thread, no collision since thread stays a loanword.
  - `radiation pattern` → diagrama de radiação vs template/design ("modelo") vs `far field` → campo distante.
  - `excitation` → excitação vs emotional excitement.
  - `node`/`antinode` → nó/antinó vs a generic numeric null/zero.
  - `scale family`/`color tone` → família de escala vs `hue` → matiz vs `palette kind` → tipo de paleta vs `color projection` → projeção de cor — four distinct chroma concepts.
  - `comet` → cometa vs `geometry` → geometria/estrutura.
  - `identity` → identidade vs unity (Smith-chart).
  - `renderer` → renderizador vs "motor de renderização".
  - `override` → substituir vs "sobrescrever" (overwrite).
  - `viewer` → visualizador vs observer/speaker/preview.
  - `reflect` → refletir vs espelhar vs reflexão.
  - `structure` → estrutura vs "construção".
  - `theme` → tema vs "assunto"/"tópico".
  - `validation` → validação vs "verificação".
  - `net gain` → ganho líquido vs "ganho da parte real".
  - `power (electrical)` → potência vs `Power` (scale-family name) → Lei de potência, never shared.
- Loanword-vs-native decision, locked: `patch`, `tag`, `fork`, `kernel`, `shader`, `widget`, `wireframe`, `thread`, `token`, `deadlock` are kept as loanwords file-wide; `segment` translates to "segmento".
- Minority-outlier spellings unify to the canonical pt form given above.
- Consistency priority outranks locale-form preference for `pattern` → "diagrama" (never "padrão", reserved for `default`), a concrete case where catalog consistency wins.

## 13. Priority ordering

- Precedence chain: correct meaning, then interface convention, then catalog consistency, then disambiguation, then locale numeral form.
- Explicit override: "diagrama" for `pattern` overrides any locale-preferred synonym, to preserve the "padrão"/"diagrama" split (topics 10, 12).

## 14. Grammatical number

- Use singular and plural noun, adjective, participle, and verb forms; make every agreeing element match the referent's number.
- After an explicit literal count, use singular with exactly `1` and plural with zero or any other count.
- Keep invariant technical tokens unchanged while inflecting the surrounding Portuguese words.

## 15. Grammatical agreement

- Adjectives and participles agree in gender and number with the head noun, including standalone labels whose head noun is implied (eg "Ativado" masculine vs "Ativada" feminine, per the implied noun).
- Declension-class choices and partitive-after-count rules: not applicable, Portuguese has no noun-declension classes.
- Reflexive-particle retention: retain "-se" where the verb is inherently reflexive (eg "atualizar-se").
- A standalone label with no recoverable head noun defaults to masculine agreement, unless the referenced NEC2/UI concept is conventionally feminine (eg "a carga" → feminine agreement).

## 16. Morphological derivation

- Borrowed technical verbs take the standard "-ar" infinitive affix (eg "normalizar"); forbid ad hoc borrowed participial suffixes.
- Verbal-noun formation uses "-ção"/"-mento" nominalization (eg "normalização", "encerramento").
- Native-affix forms are preferred over loanwords wherever a native term exists (eg "normalizar", never a transliterated form); compounding follows the spaced/hyphenated multiword rule (topic 1).

## 17. Preposition and sandhi selection

- Preposition-plus-article contraction is context-conditioned and mandatory wherever the following word takes an article: de+o→do, de+a→da, em+o→no, em+a→na, and their plural/other-article forms.
- Elision beyond the contractions above: not required.
- Sandhi (sound-conditioned form change): not applicable beyond the contractions above.

## 18. Card/record-label register

- Fixed designator form in dialog/editor titles: uppercase card mnemonic plus capitalized generic noun (eg "Cartão GW").
- Running-prose form in messages: lowercase generic noun, mnemonic uppercase inline (eg "o cartão GW define...").
- No short-vs-long form distinction is recorded for pt.
- Each register stays internally consistent and is never cross-converted between title and prose forms.

## 19. Multi-paragraph and whitespace fidelity

- Mirror source paragraph breaks at the same positions, preserving the distinction between blank lines and single line breaks.
- Preserve semantic line breaks and omit visual wrapping absent from the source literal.
- Drop trailing clauses removed from the current source rather than retaining stale translated text.
- Carry source trailing newlines and punctuation mechanically.
- Preserve complete meaning; never truncate text or use an unnatural abbreviation for an assumed display constraint.

## 20. Current-source fidelity

- Derive every translation from the current source text and supplied context.
- Reuse an inherited or copied translation only when its complete meaning agrees with the current source.
- Do not inherit "solo" where electrical `ground` requires "terra", or "saída padrão" where `default exit` requires "encerramento padrão".

## 21. Script hygiene

- No separate wrong-script or homoglyph class applies because Portuguese prose and retained Latin-script tokens share the same script; topic 1 governs ambiguous marks and diacritics.
- Translate every plain source word; allow a non-Portuguese character sequence only inside a retained identifier, unit, symbol, proper name, or named token listed in topic 4.

## 22. Rule-file scope hygiene

- Keep only current decisions that can alter wording, Unicode characters, punctuation, capitalization, embedded tokens, mnemonic markers, or semantic whitespace in a translated literal.
- Omit catalog representation, serialization, headers, flags, workflow, rendering, fonts, shaping, layout, widget behavior, runtime formatting, sorting, search, display sizing, audit history, completion state, review metadata, provenance, and citations.
- State each decision directly; include no implementation procedure, correction history, bibliography, or alternative policy source.

## 23. Section-disjointness declaration

- Topic 1 governs script and orthographic mechanics; topics 2, 6, and 7 govern literal phrasing and structure; topic 8 governs address register. These axes do not overlap, and each decision belongs to one axis.

## 24. Developer/debug-string policy

- User-facing controls, dialogs, tooltips, status messages, and errors translate fully; review priority never permits an applicable user-facing string to remain untranslated.
- Informational diagnostics translate in the terse technical register defined for status messages in topic 7.
- Developer-facing and debug strings translate in the same terse technical register; no subsystem-specific sibling family overrides this rule.
- Preserve identifiers, function names, format specifiers, and retained tokens inside every translated family under topics 4 and 5.
- Treat linguistic translation priority and token preservation as separate decisions: every translated family preserves embedded tokens regardless of review priority.
- Render the `BUG:` diagnostic prefix as "DEFEITO:", held distinct from the "ERRO:" prefix that carries ordinary failures; the two severities never share a word.
- Keep an identifier-plus-value diagnostic pair contiguous (eg `sweep_state=%d`); a qualifier precedes the whole pair and never stands between the identifier and its `=`; the identifier itself is never translated.
