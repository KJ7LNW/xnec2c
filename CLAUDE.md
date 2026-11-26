# xnec2c

@.memory/INDEX.md

- `pr_*` functions should not display more than one line (count('\n')==1)
- use `pr_*` (eg, pr_warn, pr_err, pr_info, ...) functions instead of printf or fprintf. 
- use BUG() not abort() for any critical failure cases.  BUG() Does not abort so handle any problems after the bug. 
- when adding options update the man page doc/xnec2c.1.in and doc/xnec2c.html
- The following files are TOO BIG TO READ. Instead, use Grep and Read(offset)
	- doc/xnec2c.html
	- resources/xnec2c.glade
- Hint hot-path branches with `likely()` / `unlikely()`.
- never run `make check` unless you changed a test
- Do not build the program, unless I tell you to do that.
  - If I do authorize you to build, do not pass CPU parallel flags, those are already defined in my profile via MAKEFLAGS. 
- Do not invoke tests or check commands unless instructed. 
- Bulk mechanical changes must always use Coccinelle spatch via `./scripts/spatch-quiet --sp-file cocci/<patchname>.cocci <args>`. Each cocci file must be a complete idempotent transaction. For all changes related to the change set with no dangling edge cases. Always start by performing changes that will trigger compiler errors. For example, do not leave old structure names in place. Always rename them first so the compiler can guide us if there's a problem. 
	- Use spatch when the cocci file costs fewer output tokens than the hand edits. Weigh both first; pick the smaller. File count grants no exemption.
	- spatch .cocci files need minimum comments just for structure, not for explanation. 
	- A plan calling for hand edits does not change this; weigh it before editing, plan aside.
	- You are responsible for running the patch, do not leave it to the user. 
	- One pattern over many sites favors spatch: one rule, fixed cost. A value that differs per site favors hand edits: each site bloats the rule.
	- Before a directory-wide run, enumerate every matched site and confirm each is a valid target; one invalid match corrupts the transaction.
	- Sequence interdependent patches prerequisite-first so each sweep matches already-converted code.
	- Keep a rule idempotent by matching the original construct, never its replacement.
	- A rule taking an address-of or needing an lvalue cannot rewrite a cast or non-lvalue operand; hand-convert those sites or constrain the metavariable.
	- spatch supports: typed metavariables (`expression`, `identifier`, `type`, `constant`, `statement`, `field`, `function`, `parameter list`); `...` spanning intervening code; disjunction `( A | B )`; named rules inheriting metavariables; `when` and position constraints; isomorphisms matching variant spellings (eg `x` and `x != NULL`); declaration and struct-field edits; `+`/`-` inserting before or after a match; control-flow-graph-aware matching.
- Existing original functions are `Named_Like_This()` So if those types of functions get moved, the name should be retained. However, new functions are created to be `named_like_this()`
- Notice() already calls pr_notice() so do not add duplicates in the same line. 
- No documentation (html/man) changes unless instructed. 

- Target C11 features
- Maintain header file dependency references in the appropriate make file locations. 

# repo

- never remove cocci/ or debug/ scripts.

# testing

- t/ - only use this directory for tests that are ongoing and must validate across future changes. Focus only on correctness validation here for regression checks, but not implementation considerations. 
- debug/ - Use this directory for any tests that are ephemeral in the sense that they're only necessary during the development cycle itself and serve no development testing purpose in the future. Error on this side of the fence for testing. 
