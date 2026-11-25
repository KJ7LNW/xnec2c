# User manual

Apply when instructed to change `doc/xnec2c.html` or `doc/xnec2c.1.in`.

## User-given

Treat prose as teaching what running the program withholds: consequence, limit, correctness, NEC semantics.

- Omit mechanism: how a feature measures, computes, caches, or shares state
- Omit what the user meets unbidden: confirm prompts, auto refresh, tooltips, hover previews
- State what a feature does; drop the clause naming what it excludes, except where the exclusion bears on correctness, eg a display control leaving NEC2 results intact
- Hold the original register; omit figurative wording
- State each fact once; reach its owner by anchor, eg `<a href="#FitView">`
- Reserve an aside for a caveat absent from the body
- Bound a sweeping request to text this change set added unless directed to cover the whole document; prove origin with `git grep -c '<phrase>' HEAD -- <path>`

### file references

Treat the citation class as the link decision: `urlMap` builds a repository URL from the element value for `filepath`, `version-tag`, `commit-hash`, and `issue-ref`.

- Cite the value alone; the base URL stays in `urlMap`
- Give a linking class only to a path the tree tracks; prove it with `wc -l <path>` before citing
- Name the tracked artifact an illustration stands for, eg `examples/137MHz_turnstile.nec` over an invented `~/nec2/turnstile.nec`
- Give the non-linking class, eg `fileext`, to a home path, absolute path, placeholder, extension, generated artifact, and path the tree no longer holds
- Enumerate every citation of a class before editing one; a search at its head limit proves no absence

### change log

Treat an entry as one user-visible outcome, named at the surface a user reaches: menu item, button, dialog, option, key, NEC card, file format, CLI flag.

- Fold every commit behind an outcome into one entry
- Write a fix as the symptom it retires
- Omit imperceptible change nobody contributed: refactor, rename, internal structure, test, build plumbing; a broken build is a fix
- Enter every contributed change at any size; credit its reporter, tester, and author with the issue reference
- Append; shipped entries stand
- Write an issue or pull reference `<span class="issue-ref">#N</span>`
