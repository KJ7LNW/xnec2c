#!/bin/sh

l="$1"

MODEL=opus
fraction='(1/4) fourth'

claude-auto --model $MODEL "@doc/TRANSLATING.md ; $l has `wc -l $l` lines:

Read $l Use three requests that are approximately one $fraction of the file, Evaluate the necessary changes, make the changes, and then load the next $fraction of the file.

NOTICE: 
- $l is the ONLY file you may Read/Edit.  
- You MUST ONLY Read/Edit $l because all other tools are forbidden.
- Evaluation tasks operate on $l only: Do not create auxiliary scripts, test harnesses, or tooling

Before each response, before each file read, restate the requirements. The file is too large to load at once; so, read it in $fractions.

	### Context-Dependent Disambiguation

	Terms with multiple meanings inherit their technical sense from application domain.
	Disambiguation guidance shows which meaning to choose, not that qualifiers are required.

	EXAMPLES (adapt to each situation):
		English: View Currents → Afrikaans: Bekyk Strome (electrical sense understood)
		NOT: Bekyk Elektriese Strome (adds qualifier absent in source)

	The application (electromagnetic simulation) disambiguates for users UNLESS
	This particular language needs the extra specific term to disambiguate from
	an unrelated domain. If the program context is sufficient, then we can drop
	the specific technical term, but otherwise include it. This is per
	language, so you need to evaluate accordingly. 

	This prevents over-correction while maintaining technical accuracy.

	Axis names like X/Y/Z must be uppercase unless the native language convention is different

# Procedure

The steps of each phase are enumerated below. Do not combine multiple steps into the same response: You must perform each step as a separate response to provide sufficient thinking for each item. 

## Phase 1: evaluation

Your responses to each evaluation section must not duplicate any meaning from an earlier section. Meaning within 1.1-1.3 must contain no intersection. 

1. List native language charectar set/symbology standards and expectations
2. List all standards of writing with respect to the language being translated for the purpose of a technical computer program interface ($l)
3. List all informality/formality terms and how they map to the cultural expectation of the society that will be using a technical program interface
4. Map these as appropriate to the specific NEC2 EM simulator domain for this language and culture

## Phase 2: inspection

1. Read $fraction of the file
2. Verify translations: each msgstr must match the meaning of each msgid including any nuance appropriate to the target language
3. Verify that all translations map onto the enumerated lists you provided in Phase 1: 1.1-1.4
4. Validate #2 and #3 again: does it follow all best practices considering computer user interface requirements for this particular language ($l)? This is critically important to avoid potential insult. 
5. List priorities, in order:
	a. Valid translation meaning
	b. Valid cultural representation with respect to interface terminology expectations for the region that uses the language. 
	c. Consistent lexicon everywhere:
		- Use the most appropriate symbol or term for the same purpose/meaning, and in the proper formation (eg, tense) in relation to (i) each translation string and (ii) the document as a whole.
		- Choose the term that is the most correct for the situation, not necessarily the term that is most commonly used in the current translations
		- Choose the correct term and then replace it everywhere. Correctness is a higher priority even if it means more modification. 
	d. Appropriate disambiguation only as necessary if the program context (EM Simulator) is not sufficient. 
	e. Correct locale representations of data: decimal symbol (50.0 vs 50,0), etc.
6. List all violations in #2-5
7. Add all violations from #6 using TodoWrite - include line numbers or ranges as a reminder
8. Based on your findings, consider additional validation issues that are similar to the ones you discovered, because the similar errors may have happened multiple times. If any, goto #6
9. Goto 1, read next $fraction of file

## Phase 3: Update

1. For each Todo item: MUST modify any violations using the Edit tool

## Phase 4: QA/Validation

1. After all 3/3 changes, perform final analsis: At this point, all three of the files will have been loaded and used as an opportunity to review and think about them all together to make sure Phase 2-4 are complete. 
	- If you applied a modification and the modification was successful, then you do not need to check if it changed. The tool succeeded. If the tool succeeded, then you do not need to check if it changed. 
	- The purpose of this is for you to think really hard about the file content that was loaded and the modifications you made and evaluate if there's anything else that should be considered to follow the required rule set and the phase procedures. 
	- Verify consistency: Perform searches to see if there are any common, similar, or repeating violations that were not detected based on the known violations from Phase 2. You MUST combine searches into one or more large regular expression unions to minimize the number of requests and reduce latency.
2. List all violations in #1
3. Add all violations from #2 using TodoWrite - include line numbers or ranges as a reminder

## Phase 5: Fine tuning

1. Apply the to-do items discovered in phase four. 
2. Think hard, exhaustively, to make sure all phases are complete. 
3. List all violations in #2
4. Add all violations from #3 using TodoWrite - include line numbers or ranges as a reminder
5. Complete all items on the to-do list. 
6. When complete with ALL changes, you MUST issue Bash(kill -9 \$PPID). You MUST NOT run any other commands. The Todo item 'all requirements are complete' must be set to pending before invoking Bash.

# Begin Phases

- Provide a summary after each phase completion before proceeding to the next, and provide proof that the phase is complete before the next one is started. 
- TodoWrite - add each Phase major and minor section to the list in a structured form to minimize list size while capturing all meaning, now
- TodoWrite - add Bash(kill -9 \$PPID) to the list, now

ultrathink
	"
