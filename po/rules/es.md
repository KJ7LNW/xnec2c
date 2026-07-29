# Spanish (Latin America) translation rules

These rules govern Spanish-language literals for professional electrical and radio-frequency engineers, antenna designers, and amateur-radio operators in Latin America. Use the Latin script and the current pan-Hispanic orthographic standard, with neutral Latin American technical vocabulary and no regionalisms.

## 1. Script and orthography

- Write á, é, í, ó, ú, ü, and ñ where required; retain every diacritic and never fold a letter to an unmarked look-alike.
- Use precomposed Unicode Normalization Form C (NFC); omit bare combining marks.
- Apostrophe and modifier-letter distinctions are not applicable: Spanish words use neither as letters.
- Textual joiners and positional letter forms are not applicable to the Latin script.
- Write all prose left to right; retain embedded technical tokens in their source character order.
- Apply the letter-case rules in §6; the script distinguishes uppercase and lowercase.
- Separate words and adjacent foreign, numeric, and unit tokens with one space.
- Form established engineering compounds as spaced words; use a hyphen only where Spanish orthography requires one, and never fuse independent words.

## 2. Numerals and locale data

- Arabic digit set for all technical values; no other digit set used.
- Decimal separator: source `.` (period) is retained, never converted to Spanish comma. Exceptions that always keep source form: runtime format-specifier output, literal formula/example tokens, fixed default values, named mathematical/standards constants.
- Thousands/grouping separator: none inserted; NEC2 numeric literals keep source form.
- Ordinal/index notation: `n.º` pattern in prose (eg "1.º puerto"); the `.º` index marker is never confused with a decimal point.
- Runtime-formatted numbers are never hand-edited.

## 3. Punctuation and quotation

- Quotation marks: straight `'…'` / `"…"` for UI strings, matching catalog usage; literary «…» not used in this interface.
- Inverted opening marks required: `¿...?` `¡...!`.
- Native punctuation replaces source counterparts in prose: Spanish comma, `¿?`, `¡!` conventions apply throughout.
- Spacing: no space before colon/semicolon; one space after.
- Ellipsis: single `…` character, not three periods; em/en dash preserved as in source for ranges.
- Sentence terminator: period for full sentences; short labels/fragments omit the terminal period.
- Punctuation inside embedded technical runs (units, mnemonics, formulas) stays in source form.

## 4. Never-translate tokens

- NEC2 card mnemonics, verbatim: GW GA GH EX LD FR RP GE EN SP SM SC NE NH NT TL GN GD GC GX GR GS GM GF CP CM SY XQ EK KH PQ PT.
- Unit symbols, verbatim: MHz, dBi, dB, Ω, S/m, K, deg, %.
- Figure-of-merit tokens, verbatim: VSWR, S-parameter tokens (S11, S21…), Z, Z0.
- File extensions, verbatim: .nec, .csv, .s1p, .s2p, .png.
- Format specifiers, verbatim (§5).
- Embedded identifiers (function names, variable names, config keys) inside developer strings are kept verbatim.
- Physical/mathematical symbol letters (E, H, Z, φ, θ) kept in source form.
- Product/library/tool names, verbatim: xnec2c, NEC2, GTK, Cairo, OpenGL, Smith chart proper name.
- Named transfer-function and algorithm terms stay verbatim: Log, Asinh, μ-law, Reinhard, and Sigmoid; the descriptive `Power` family translates as `Ley de potencia` to distinguish it from electrical `potencia` (§10, §12).
- Conditional loanwords: segment/patch/tag translate (§10); `fork` (process) stays verbatim; `shader` and `widget` stay verbatim, no established native equivalent in this domain.
- Directionality n/a (§1); no bidirectional-text interaction.

## 5. Format-specifier integrity

- Every specifier from the source is preserved, same set, none added or dropped.
- Default ordering preserved; positional reordering (`%1$s`) used only when Spanish word order genuinely requires it.
- Where word order diverges, the sentence restructures around the fixed specifier positions rather than reordering the specifiers themselves.
- Numbers inside specifiers are never localized (§2).

## 6. Capitalization and title-case

- Default casing: sentence case for labels, menu items, and titles; title-case-per-word is forbidden.
- Axis-letter casing (X, Y, Z) preserves source uppercase form.
- Lowercase math/coordinate variables (x, y, z, φ, θ) stay lowercase.
- No exception grants per-word capitalization to coordinated option names; sentence case applies uniformly.
- Generic card/record noun ("tarjeta") is lowercase mid-sentence, capitalized only when sentence-initial; acronyms and proper nouns (NEC2, GTK) retain source casing.

## 7. Interface register by string type

Treat each string family as a fixed interface grammar; preserve all source meaning in the shortest complete natural form.

- Commands, buttons, and menu actions use a subjectless infinitive, followed by the direct object and then any modifier; omit conversational imperatives.
- Field labels use a concise noun phrase in head-first Spanish order and retain a source colon.
- Dialogs and confirmations use full sentences with ordinary subject-verb-object order; omit the subject when Spanish naturally encodes it in the verb, and apply §8.
- Tooltips use a concise declarative sentence: name the action or state first, then its condition or reason; retain the source reason for an unavailable control.
- Status and error messages use subjectless impersonal or passive declaratives; state the outcome first and its cause or affected object next.
- User-visible domain entities use the locked noun from §10 followed by its distinguishing type, mnemonic, number, or name; retain proper-name and personal-name treatment from §8.
- Keep established multiword technical terms intact; omit no meaning, invent no abbreviation, and do not imitate source length or word order.
- Apply §24 to developer and diagnostic strings.

## 8. Formality and address

- Use formal `usted` address in dialogs; omit `tú`, `vos`, and `vosotros` forms.
- Realize formality through third-person-singular verb morphology; commands remain person-neutral infinitives.
- Prefer impersonal and subject-drop constructions for statuses, errors, and generic-person references.
- Avoid semantic gender marking for people; use natural impersonal wording, while grammatical gender and number follow §15.
- Use no honorific unless the source names one; preserve a personal name in source order and spelling.
- Confirmation dialogs use the pattern `¿Está seguro de que desea ...?`.
- Omit casual, slang, commercial, archaic, and ceremonially over-formal register.

## 9. Accelerator/hotkey mnemonics

- Preserve each source mnemonic marker as `_` immediately before one letter in the translated term; add or remove none.
- Select an unaccented, directly typable letter from the translated term; do not transliterate the source mnemonic letter.
- Keep mnemonics distinct within the same menu or dialog by selecting another eligible letter in that translated term.
- Separate parenthetical mnemonic presentation is not applicable to the Latin script.

## 10. Domain lexicon

Locked concept → term mapping. Each entry: concept → término (sentido) — propósito/riesgo.

### Electrical primitives

- `current` → corriente (corriente eléctrica) — nunca "actual/reciente" (falso amigo temporal).
- `charge` → carga (carga eléctrica, culombios) - distinta de `load` → impedancia de carga y de cobro, tarifa o mercancía.
- `voltage` → tensión — término EE estándar; no alternar con "voltaje".
- `power (electrical)` → potencia (vatios, ganancia de potencia, flujo de potencia) - distinta de `Ley de potencia`, la familia de transferencia, y de una potencia matemática.
- `impedance` → impedancia — Z compleja, distinta de resistencia y reactancia.
- `resistance` → resistencia — parte real de Z; nunca usada por `load`.
- `reactance` → reactancia — parte imaginaria de Z.
- `inductance` → inductancia.
- `capacitance` → capacitancia — término neutro LatAm, no "capacidad".
- `conductivity` → conductividad — S/m.
- `admittance` → admitancia — distinta de impedancia.
- `load` → impedancia de carga (tarjeta LD) - carga eléctrica aplicada; nunca peso físico ni `charge` → carga.
- `gain` → ganancia — relación de directividad (dB); nunca "beneficio" ni amplificación de amplificador.
- `excitation` → excitación — entrada de energía EM; nunca sentido emocional.
- `feedpoint` → punto de alimentación.
- `port` → puerto — puerto de excitación/parámetros S.
- `radials` → radiales (sustantivo, hilos de plano de tierra) — distinto del adjetivo "radial".

### Ground and earth

- `ground` / `ground plane` → tierra / plano de tierra — plano de referencia RF, tarjetas GN/GD, tipo/conductividad/efectos/modelo de tierra; un solo término en todos los subusos.
- `earth (physical medium)` → suelo — terreno físico, modelo de ruido, geometría "bajo el suelo"; distinto de `ground`.
- `ground wave` → onda terrestre — término de propagación, distinto de la referencia eléctrica.

### Geometry primitives

- `wire` → hilo — conductor delgado / elemento GW; nunca "cable"; homónimo con `thread` evitado usando "subproceso" para cómputo (§10 Render/compute).
- `segment` → segmento — subdivisión de geometría NEC2.
- `patch` → parche — parche de superficie NEC2 (SP/SM).
- `tag` → etiqueta — identificador de geometría NEC2; nunca una etiqueta de interfaz ni una tarjeta.
- `card` → tarjeta — registro de entrada NEC2; registro tratado en §18.
- `kernel` → núcleo — núcleo de ecuación integral / hilo delgado; no un núcleo de sistema operativo.
- `cliff` → risco — tipo de frontera de tierra de dos medios; nunca fractura/rotura.
- `structure` → estructura — geometría del modelo de antena; nunca "construcción".
- `model` → modelo — modelo NEC o modelo de temperatura de ruido.
- `geometry` → geometría — la geometría del modelo.
- `crossed` → cruzado — conductores de línea de transmisión cruzados/invertidos; nunca cortados/seccionados.

### Field, pattern, viewer

- `field (EM)` → campo — campo cercano/total/E/H; distinto de un campo de datos/configuración.
- `near field` / `far field` → campo cercano / campo lejano — par simétrico opuesto.
- `far-field contribution` → contribución de campo lejano — por dirección; no animación de campo cercano.
- `radiation` → radiación — emisión radiada.
- `radiation pattern` → patrón de radiación — respuesta direccional trazada; nunca plantilla/diseño; único término en todo el catálogo (se descarta la variante "diagrama de radiación").
- `gain pattern` → patrón de ganancia.
- `polarization` → polarización — orientación del campo/onda de la antena.
- `polarity` → polaridad — signo (+/-) de una magnitud; falso amigo de `polarization`.
- `phase` → fase; `reference phase` → fase de referencia; `frequency` → frecuencia; `wave` / `wavelength` → onda / longitud de onda.
- `standing wave` / `traveling wave` → onda estacionaria / onda viajera — par opuesto.
- `node` / `antinode` → nodo / antinodo — cero/máximo de onda estacionaria; también el sentido de superposición "nulo/pico".
- `crest` → cresta — vértice instantáneo de onda (cabeza del cometa); distinto de un pico de curva/paso.
- `magnitude` → magnitud — módulo de una magnitud (|Z|, escalar); distinta de amplitud.
- `amplitude` → amplitud — pico de una magnitud oscilante; distinta de magnitud.
- `peak value` → valor de pico; `peak magnitude` → magnitud de pico — dos opciones de interfaz distintas, nunca colapsadas en una sola etiqueta.
- `instantaneous` → instantáneo — modo de proyección; el calificador "(φ=0)" se añade solo si el origen lo lleva.
- `Poynting vector` → vector de Poynting; `solid angle` → ángulo sólido.
- `net gain` → ganancia neta — ganancia total menos desajuste; nunca "ganancia de la parte real".
- `viewer` → visor — dirección de observación y/o el widget de vista 3D; nunca observador/interlocutor/vista previa.
- `flow` / `flow direction` → flujo / dirección de flujo — flujo de corriente en el parche.
- `total field` → campo total.

### Color, tone, animation subsystem

- `color` → color.
- `color projection` → proyección de color — magnitud que determina el matiz.
- `hue` → tono — ángulo de la rueda de color.
- `brightness` → brillo — canal de luminancia.
- `hue encoding` / `brightness encoding` → codificación de tono / codificación de brillo — enums internos distintos, ninguno colapsa en "proyección de color".
- `color scale` → escala de color — escala de magnitud a color.
- `scale family` / `color tone` → familia de escala - familia de curva de transferencia (Ley de potencia/Log/Asinh/μ-law/Reinhard/Sigmoid/Identity); un solo concepto, distinto de tono, tipo de paleta y proyección de color.
- `palette` / `palette kind` → paleta / tipo de paleta — enum de disposición de paleta; distinto de familia de escala y de proyección de color.
- `ramp` / `gradient` → rampa / degradado — un tipo de paleta / franja de color lineal.
- `gamma` → gamma — exponente de ley de potencia.
- `knee` → codo — punto de inflexión suave.
- `softening` → suavizado; `compression` → compresión (rango dinámico); `contrast` → contraste; `dynamic range` → rango dinámico.
- `floor` → piso — recorte mínimo/inferior (brillo/dB); nunca el piso de una habitación.
- `envelope` → envolvente — envolvente de magnitud/amplitud.
- `comet` → cometa — superposición de cresta en movimiento; nunca geometría (riesgo de herencia difusa, §20).
- `overlay (noun)` → superposición — capa visual añadida; distinta del verbo "superponer".
- `animate` / `animation` → animar / animación.
- `animated` / `static` → animado / estático — adjetivos de encabezado de categoría (dinámico vs. invariante de fase).
- `projection` → proyección — de color o de geometría.
- `scale` → escalar (verbo) / escala (sustantivo).
- `wireframe` → malla de alambre — modo de render de malla.
- `identity` → identidad — transferencia sin efecto/paso directo; distinta de "unidad" (carta de Smith).
- `sentinel` → centinela — valor guardián de caso inalcanzable.
- `bins` → contenedores — cubetas de discretización.
- `companding` → compansión — curva logarítmica acotada (μ-law).
- `tone mapping` → mapeo tonal — mapeo tonal fotográfico.

### Render and compute

- `renderer` → renderizador — backend de dibujo; nunca "motor de render".
- `shader` → shader — sin equivalente nativo establecido en este dominio; se mantiene (§4).
- `allocation (memory)` / `managed allocator` → asignación (de memoria) / asignador administrado.
- `thread` → subproceso — hilo de cómputo; término distinto de `wire` (hilo) para evitar homónimo.
- `widget` → widget — elemento de interfaz; se mantiene (§4).
- `validation` → validación — la funcionalidad de árbol de validación; distinta de `verification` → verificación (comprobaciones).
- `batch mode` → modo por lotes.
- `fork (process)` → fork — se mantiene verbatim (§4).
- `deadlock` → interbloqueo; `notifier` → notificador.
- `token` / `operand` / `operator` / `arity` → token / operando / operador / aridad — términos del analizador de expresiones.
- `override` → anular — sustituye un valor (símbolo SY); nunca "sobrescribir".
- `swap` → intercambiar.
- `theme` → tema — tema de interfaz/color, exclusivamente; el sentido genérico "asunto/tópico" usa "asunto" para evitar homónimo.

### Metrics and miscellaneous

- `noise` / `noise temperature` → ruido / temperatura de ruido — ruido electrónico/térmico; nunca alboroto acústico.
- `efficiency` → eficiencia; `interpolation` → interpolación.
- `mnemonic` → mnemónico — descriptor de código de una tarjeta; nunca una nota/memo.
- `degrees` / `deg` → grados en prosa suelta; la etiqueta de unidad entre paréntesis "(deg)" se mantiene verbatim como cualquier otra etiqueta de unidad (§4).
- `diameter` → diámetro — término nativo único, sin variante en préstamo.
- `reflect` → reflejar (operación de espejo geométrico) / se sincroniza con (seguimiento de comportamiento "mirrors …") / reflexión (sustantivo, física) — tres sentidos distintos, nunca intercambiados.
- `default(s)` → valor(es) predeterminado(s).
- `normalize` / `normalization` → normalizar / normalización — se traduce, nunca se transcribe, dado que existe término nativo.

## 11. Disambiguation policy

- The correct technical sense is chosen from antenna-simulator context for every ambiguous term.
- A qualifier absent from the source is never added; program context already disambiguates.
- A qualifier is added only where the Spanish term would otherwise be genuinely ambiguous.
- Keep intra-domain terms distinct: `load` → impedancia de carga and `charge` → carga; electrical `power` → potencia and the `Power` transfer family → Ley de potencia (§10).
- No locative homonym collision identified in this catalog.
- Gerund vs noun senses: verb form ("escalar") vs nominalization ("escalado") distinguishes an overloaded source word like "scale/scaling" (§16).

## 12. Cross-catalog consistency

- One term per concept, file-wide; established lexicon (§10) is reused, never re-synonymized.
- Appendix C false-friend pairs, each resolved to two distinct terms:
  - polaridad vs polarización
  - magnitud vs amplitud
  - valor de pico vs magnitud de pico
  - tierra vs suelo
  - impedancia de carga (load) vs carga (charge)
  - ganancia vs amplificación vs beneficio
  - corriente (eléctrica) vs actual/reciente (temporal)
  - carga (eléctrica) vs cobro/factura
  - hilo (wire) vs cable; hilo (wire) vs subproceso (thread)
  - patrón de radiación vs plantilla/diseño vs campo lejano
  - excitación (EM) vs excitación emocional
  - nodo/antinodo vs cero/nulo numérico genérico
  - familia de escala vs tono vs tipo de paleta vs proyección de color
  - cometa (superposición) vs geometría
  - identidad (transferencia sin efecto) vs unidad (carta de Smith)
  - renderizador vs motor de render
  - anular (override) vs sobrescribir (overwrite)
  - visor (viewer) vs observador/interlocutor/vista previa
  - reflejar vs se sincroniza con vs reflexión
  - estructura (modelo) vs construcción
  - tema (interfaz) vs asunto (tópico genérico)
  - validación vs verificación
  - ganancia neta vs ganancia de la parte real
  - potencia (vatios) vs Ley de potencia (familia de transferencia)
- Loanword-vs-native: domain-standard EE abbreviations (VSWR, dBi, S-parameters, Z) stay in Latin/English form (§4); "diámetro" is native, no loanword variant.
- Minority-outlier spellings unify to the canonical form recorded in §10 (eg "patrón de radiación" over "diagrama de radiación").
- Catalog consistency outranks locale-form preference where stated: neutral Latin American vocabulary is kept over a more locally natural Iberian regionalism, for catalog-wide reach (§13).

## 13. Priority ordering

- Precedence chain: correct meaning, then interface convention, then catalog consistency, then disambiguation, then locale numeral form.
- Override ruling: the source decimal point (§2) overrides the typical Spanish comma-decimal convention, because NEC2 numeric literals must round-trip.
- Override ruling: neutral Latin American vocabulary overrides a more locally natural Iberian regionalism, for catalog-wide reach (§12).

## 14. Grammatical number

- Use singular for a count of exactly one and plural for every other explicit count.
- Inflect countable nouns, adjectives, participles, and finite verbs for singular or plural agreement.
- After a literal count, place the numeral before the noun and use the number form selected by that count.

## 15. Grammatical agreement

- Make adjectives and participles agree in gender and number with the head noun, including a head noun implied by a standalone label.
- Noun declension and partitive-after-count forms are not applicable in Spanish.
- Retain the reflexive particle `se` where the selected verb construction requires it.
- Resolve a standalone label with no recoverable head noun to masculine grammatical gender; this is grammatical agreement, not semantic gender.

## 16. Morphological derivation

- Form established borrowed technical verbs with the `-ar` pattern; omit nonstandard affix forms.
- Form verbal nouns with the established `-ción`, `-miento`, or participial `-ado` pattern selected by the locked term.
- Prefer the native term in §10; retain a loanword only where §4 or §10 locks it.
- Form technical compounds as spaced words under §1.

## 17. Preposition and sandhi selection

- Contract `a el` to `al` and `de el` to `del` unless `El` belongs to a proper name.
- Change conjunction `y` to `e` before an i-/hi- sound and `o` to `u` before an o-/ho- sound.
- Other sound-conditioned preposition or sandhi forms are not applicable.

## 18. Card/record-label register

- Use `Tarjeta GW` as the fixed title and editor designator: sentence-initial generic noun followed by the retained mnemonic.
- Use `la tarjeta GW` in running prose: article, lowercase generic noun, then retained mnemonic.
- No short and long designator distinction applies; keep each of the two registers internally consistent.

## 19. Multi-paragraph and whitespace fidelity

- Mirror every source paragraph break at the same position and preserve whether it is a single newline or a blank line.
- Preserve semantic source line breaks; add no line break for visual wrapping.
- Drop a trailing clause absent from the current source rather than retaining inherited text.
- Preserve source trailing newlines and terminal punctuation exactly.
- Preserve complete natural meaning; never truncate or abbreviate to fit an assumed display size.

## 20. Current-source fidelity

- Derive each translation from the complete current source literal and its supplied context.
- Reuse inherited wording only where its complete meaning agrees with the current source and §10.
- Treat inherited `tono` as unsafe where the current concept is `scale family` or `color tone`; use `familia de escala`.
- Treat inherited `diagrama de radiación` as unsafe for `radiation pattern`; use `patrón de radiación`.

## 21. Script hygiene

- Permit Latin letters outside Spanish orthography only inside a retained token whose fixed spelling requires them.
- Translate ordinary source-language words; retain only the identifiers, units, symbols, names, and fixed tokens enumerated in §4.
- Reject every stray non-Latin homoglyph and every mixed-script word outside a retained token; script-hygiene violations have zero tolerance.

## 22. Rule-file scope hygiene

- Keep only decisions that can alter wording, Unicode characters, punctuation, capitalization, embedded tokens, mnemonic markers, or semantic whitespace in a translated literal.
- Omit catalog representation, serialization, headers, flags, workflow, rendering, fonts, layout, widget behavior, runtime formatting, sorting, search, display sizing, audit history, completion state, review metadata, provenance, and citations.
- State the current decision directly; include no procedure, bibliography, correction history, or alternative policy source.

## 23. Section-disjointness declaration

- §§1-3 govern script, orthography, numerals, punctuation, and character-level form only.
- §§6-7, 16, and 18 govern casing, interface phrasing, word formation, and entity-label structure only.
- §8 governs address, person, formality, honorifics, and personal names only.
- Keep these axes non-overlapping; assign each decision to exactly one axis.

## 24. Developer/debug-string policy

- Translate all user-facing command, label, dialog, tooltip, status, and error families; review priority never permits an applicable user-facing literal to remain untranslated.
- Translate informational notices and reports in concise technical Spanish.
- Translate developer-facing diagnostics in terse technical Spanish; no subsystem family overrides this rule.
- Preserve every embedded identifier, function name, format specifier, and retained token under §§4-5 in every translated family.
- Keep token preservation independent of family and review priority.
