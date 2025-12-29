# Test Fixtures for SY Card Support

This directory contains NEC2 test files for validating SY (symbolic variable) card support.

## Files

- **sy_comma_format.nec** - Tests comma-separated symbol definitions on single SY card
- **sy_separate_cards.nec** - Tests symbols defined on separate SY cards
- **sy_math_geom.nec** - Tests mathematical expressions in geometry section
- **sy_math_cmnd.nec** - Tests mathematical expressions in command section

## Expected Behavior

All files should load and parse successfully with:
- Symbols evaluated correctly
- Expressions computed according to operator precedence
- No errors during file parsing
