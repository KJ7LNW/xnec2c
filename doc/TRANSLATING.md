# Translation Guide for xnec2c

## Domain Context

xnec2c is electromagnetic simulation software for antenna and radio frequency (RF) design.

**Domain:** Electrical engineering - antenna design, RF circuits, electromagnetics

**Target Audience:** Electrical engineers, RF engineers, amateur radio operators, antenna designers

---

## Critical Terminology

### Never Translate These Terms

**NEC2 Identifiers:**
- NEC2 (software name)
- Card mnemonics: GW, GA, GH, EX, LD, FR, RP, GE, EN
- segment, patch, tag (NEC2 geometry terms)

**Technical Abbreviations:**
- VSWR (Voltage Standing Wave Ratio)
- dBi (decibels relative to isotropic)
- S-parameters (scattering parameters)
- Z (impedance)

**File Extensions:**
- .nec, .csv, .s1p, .s2p, .png

**Unit Symbols:**
- MHz, dBi, Ω, degrees

---

## Common Translation Errors

### Electrical vs Common Meanings

| English | WRONG Translation Context | CORRECT Context |
|---------|---------------------------|-----------------|
| charge | billing/cost | electrical charge (Coulombs) |
| current | present time/recent | electrical current (Amperes) |
| ground | soil/dirt | electrical ground plane/Earth reference |
| wire | cable/cord | thin conductor in antenna model |
| gain | profit/increase | antenna directivity ratio (dB) |
| pattern | design/template | 3D directional radiation response |
| excitation | emotional state | electromagnetic energy input |
| load | weight/burden | electrical impedance |
| radials | pertaining to radius | horizontal ground plane wires |

---

## Translatable Strings (from POT file)

### Graph Titles
Translate these but preserve unit symbols:

```
"Raw Gain dBi" → keep "dBi"
"Net Gain dBi" → keep "dBi"
"VSWR vs Frequency" → keep "VSWR"
"Z-real" → electrical impedance (real part)
"Z-imag" → electrical impedance (imaginary part)
```

### Dialog Messages
Full translation:

```
"Are you sure you want to quit xnec2c?"
"A new NEC2 input file may not be created"
"xnec2c geometry optimizer"
```

### Menu Items
Full translation, context-aware:

```
"File", "Edit", "View" → standard UI terms
"Frequency Plots" → electromagnetic frequency
"Radiation Pattern" → antenna radiation
```

---

## Translation Priorities

**HIGH (user-facing UI):**
- Menu items
- Dialog messages
- Button labels
- Window titles
- User error messages

**MEDIUM (informational):**
- Status messages
- Confirmation dialogs
- File type descriptions

**LOW (keep English):**
- Debug output (pr_debug)
- NEC2 geometry errors
- Developer messages

---

## Quality Checklist

Before submitting translation:

- [ ] Technical terms verified in electrical engineering dictionary
- [ ] NEC2 card mnemonics (GW, EX, LD, etc.) unchanged
- [ ] Unit symbols (dBi, MHz, Ω, VSWR) preserved
- [ ] Mathematical notation unchanged
- [ ] File extensions not translated
- [ ] "charge/current/ground" translated as electrical terms
- [ ] Consistent terminology throughout file
- [ ] Proper plural forms in header
- [ ] No fuzzy entries remaining

---

## Plural Forms

Add correct plural form header for your language:

```
Arabic:    nplurals=6; plural=(n==0 ? 0 : n==1 ? 1 : n==2 ? 2 : n%100>=3 && n%100<=10 ? 3 : n%100>=11 ? 4 : 5);
Bengali:   nplurals=2; plural=(n != 1);
Chinese:   nplurals=1; plural=0;
German:    nplurals=2; plural=(n != 1);
Greek:     nplurals=2; plural=(n != 1);
Spanish:   nplurals=2; plural=(n != 1);
French:    nplurals=2; plural=(n > 1);
Hebrew:    nplurals=2; plural=(n != 1);
Hindi:     nplurals=2; plural=(n != 1);
Italian:   nplurals=2; plural=(n != 1);
Japanese:  nplurals=1; plural=0;
Korean:    nplurals=1; plural=0;
Portuguese: nplurals=2; plural=(n != 1);
Russian:   nplurals=3; plural=(n%10==1 && n%100!=11 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2);
Turkish:   nplurals=2; plural=(n != 1);
Urdu:      nplurals=2; plural=(n != 1);
```

---

## Formality Guidelines by Language

Professional engineering software requires appropriate formality levels for each language.

### European Languages

**German (de)**
Use formal Sie pronouns and imperative forms in all contexts.

**French (fr)**
Use formal vous address in all interface elements.

**Spanish (es)**
Use formal usted address; prefer neutral Latin American Spanish for broader reach.

**Portuguese (pt)**
Use Brazilian Portuguese with formal você address.

**Italian (it)**
Use formal Lei pronoun with third-person verb conjugations.

**Greek (el)**
Use plural formal constructions in professional contexts.

**Turkish (tr)**
Use formal siz pronoun; avoid informal sen which appears disrespectful.

**Russian (ru)**
Use formal Вы pronoun (capitalized in direct address).

### Asian Languages

**Japanese (ja)**
Use polite です/ます forms exclusively; avoid casual だ/である forms.

**Korean (ko)**
Use formal 합니다 verb endings in buttons and commands; use 해요 for less formal elements.

**Chinese (zh_CN)**
Use standard 您 (formal you) where applicable; maintain professional tone without excessive formality.

### South Asian Languages

**Hindi (hi)**
Use आप (formal you) in all contexts; maintain respectful tone.

**Bengali (bn)**
Use আপনি (formal you) consistently; professional register required.

**Urdu (ur)**
Use آپ (formal you) in all interface elements; maintain courteous tone.

### RTL Languages

**Arabic (ar)**
Use standard Modern Arabic forms; avoid dialectal variations in technical interface.

**Hebrew (he)**
Use contemporary pronouns matching gender and number; maintain professional register.

---

## Right-to-Left (RTL) Language Requirements

Arabic, Hebrew, and Urdu are RTL languages requiring special layout considerations.

### Layout Mirroring

Mirror these UI elements:
- Navigation arrows and progression indicators
- Tree view expand/collapse controls
- Slider controls and scroll bars
- Breadcrumb navigation
- Tab order in dialogs

Preserve original orientation:
- Icons and symbols (mathematical, electrical)
- Antenna geometry visualizations
- Graphs and plots
- Technical diagrams
- Logo and brand elements

### Typography

**Font Size Adjustments**
- Arabic: increase by 2pt for buttons, labels, titles
- Hebrew: increase by 2pt for buttons, labels, titles
- Urdu: increase by 2pt for buttons, labels, titles

**Text Expansion**
- Arabic: 30% expansion vs English
- Hebrew: 20% expansion vs English
- Urdu: 25% expansion vs English

**Script Characteristics**
- No uppercase in Arabic, Hebrew, Urdu
- Arabic uses Kashida for justification
- No word breaking across lines in Arabic

### Bidirectional Text

Numbers and technical notation remain left-to-right:
- "123 MHz" displays as "123 MHz" not "zHM 321"
- File paths: `/home/user/file.nec` preserve LTR
- NEC2 mnemonics: "GW", "EX" remain LTR
- Units: "dBi", "Ω" remain LTR

Mixed LTR/RTL text follows Unicode Bidirectional Algorithm:
- Technical terms embedded in RTL text
- File names in RTL sentences
- English abbreviations in RTL context

### Verification

Test RTL layouts:
- Dialog symmetry preserved
- Menu alignment correct
- Button order reversed appropriately
- Tooltips positioned correctly
- Graph labels readable

---

## Language Reference Table

| Code | Language | Script | Formality | Expansion | Notes |
|------|----------|--------|-----------|-----------|-------|
| ar | Arabic | RTL | Formal Modern Arabic | +30% | No uppercase, +2pt font |
| bn | Bengali | LTR | আপনি (formal) | +15% | Indic script |
| de | German | LTR | Sie | +30% | Compound words |
| el | Greek | LTR | Plural formal | +10% | Polytonic avoided |
| es | Spanish | LTR | usted | +15% | Latin American neutral |
| fr | French | LTR | vous | +15% | Canadian vs European |
| he | Hebrew | RTL | Professional | +20% | No uppercase, +2pt font |
| hi | Hindi | LTR | आप (formal) | +20% | Devanagari script |
| it | Italian | LTR | Lei | +10% | Technical terminology |
| ja | Japanese | LTR | です/ます | +10% | No spaces, vertical OK |
| ko | Korean | LTR | 합니다 | +5% | Hangul syllables |
| pt | Portuguese | LTR | você | +15% | Brazilian standard |
| ru | Russian | LTR | Вы | +15% | Cyrillic, cases |
| tr | Turkish | LTR | siz | +10% | Agglutinative |
| ur | Urdu | RTL | آپ (formal) | +25% | Nastaliq script, +2pt font |
| zh_CN | Chinese | LTR | 您 where needed | -30% | Simplified characters |

---

## Translation Quality Management

### Translation Memory and Consistency

Use translation memory (TM) tools to maintain consistency:
- Leverage previously translated segments
- Build project-specific TM database
- Share TM across related technical documentation
- Review fuzzy matches carefully for technical accuracy

### Terminology Management

Maintain consistent terminology glossaries:
- Create electrical engineering term database
- Document NEC2-specific terminology decisions
- Share glossary with all translators
- Update glossary when domain terms evolve
- Lock critical technical terms (VSWR, impedance, etc.)

### Translator Assignment

Assign single translator per language:
- Ensures consistent voice and terminology
- Reduces conflicting translation choices
- Maintains uniform formality level
- Builds translator domain expertise over time

### Linguistic Conventions

Preserve source language characteristics:
- Maintain grammatical mood (imperative, indicative)
- Respect target language capitalization rules
- Preserve sentence structure where culturally appropriate
- Adapt punctuation to target language norms

### UI Space Management

Accommodate text expansion in layouts:
- Reserve 30-40% extra space for Germanic/Romance languages
- Reserve 50% extra space for Arabic
- Use tooltips for detailed information when space limited
- Test truncation behavior with longest translations
- Avoid fixed-width UI elements where possible

### Capitalization by Language

Follow target language conventions:
- German: Capitalize all nouns
- English: Sentence case for labels, title case for menus
- French: Minimal capitalization except proper nouns
- Spanish/Portuguese: Minimal capitalization
- Turkish: Proper case handling for dotted/dotless i
- Russian: Capitalize formal Вы in direct address
- Arabic/Hebrew/Urdu: No uppercase concept
- Chinese/Japanese/Korean: No case distinction

---

## AI Translation Template

For AI translation services, use this prompt:

```
You are translating electromagnetic simulation software for electrical engineers.

DOMAIN: Antenna design, RF circuits, electromagnetics
TARGET: Professional engineers, amateur radio operators

CRITICAL CONTEXT:
- "charge" = electrical charge (Coulombs), NOT billing
- "current" = electrical current (Amperes), NOT present time
- "ground" = electrical ground plane, NOT soil
- "wire" = thin conductor, NOT cable
- "gain" = antenna directivity (dB), NOT profit
- "load" = electrical impedance, NOT weight

PRESERVE UNCHANGED:
- NEC2 mnemonics: GW, GA, EX, LD, FR, RP, GE, EN
- File extensions: .nec, .csv, .s1p, .s2p
- Unit symbols: MHz, dBi, Ω, degrees
- Technical terms: VSWR, S-parameters, impedance
- Mathematical expressions

OUTPUT FORMAT: Standard PO file with proper header

Translate to [TARGET_LANGUAGE]:
[paste POT file content here]
```

---

## Verification Commands

After translation, verify:

```bash
# Check syntax
msgfmt -c -v your_language.po

# Verify units preserved
grep -E "dBi|MHz|VSWR|Ω" your_language.po

# Check NEC2 mnemonics untranslated
grep "msgid.*GW\|EX\|LD\|FR\|RP" your_language.po

# Count translated vs untranslated
msgfmt --statistics your_language.po
```

---

## Reference Resources

Consult these for technical terminology:

1. IEEE Standard Dictionary of Electrical and Electronics Terms
2. ITU Radio Regulations (RF terminology)
3. ARRL Handbook (amateur radio)
4. Local electrical engineering standards for your language

---

## Example Translations

### English → German

| Source | Translation | Notes |
|--------|-------------|-------|
| "segment connection error" | "Segmentverbindungsfehler" | NEC2 geometry |
| "charge distribution" | "Ladungsverteilung" | electrical charge |
| "ground plane" | "Groundplane" or "Masseebene" | RF term |
| "radiation pattern" | "Strahlungsdiagramm" | antenna pattern |
| "Raw Gain dBi" | "Rohgewinn dBi" | preserve "dBi" |

### English → Spanish

| Source | Translation | Notes |
|--------|-------------|-------|
| "Are you sure you want to quit xnec2c?" | "¿Está seguro de salir de xnec2c?" | UI dialog |
| "excitation source" | "fuente de excitación" | RF source |
| "current distribution" | "distribución de corriente" | electrical |
| "impedance matching" | "adaptación de impedancia" | RF engineering |
| "VSWR vs Frequency" | "VSWR vs Frecuencia" | preserve "VSWR" |

---

## Contact

Translation questions: xnec2c-bugs@z.ewheeler.org

Report terminology errors or suggest improvements to this guide.
