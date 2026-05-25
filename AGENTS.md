
# STU-Extras Project - Side-Projects for ReMoM, ReMoO1, ReMoO2, etc.

DO NOT CHANGE ANY OF THE ORIGINAL GAME CODE!!!!!
Comment code, rather than deleting it.
Tag your changes with /* 'AI AGENT NAME' */.
All code should as close as possible to the original game code, as seen in the disassembly.
Create code as a near-literal translation of the disassembly.


## Style Guide
- Always end every function comment with "Powered by AI."
- Use four spaces for indentation rather than tabs.

## Codex Requested Guidance
 * Do not invent new top-level functions unless they exist in the original or I explicitly ask.
 * Keep the translated C in the same routine shape/order as the disassembly.
 * If you need a portability layer, put it underneath the original function, not around it.
 * Prefer ugly-but-faithful over clean-but-restructured.
 * Before changing architecture, show me the diff shape first.
 * List the exact files you used before answering.
 * Cite every claim with file and line.
 * Separate disassembly-derived facts from your inference.
 * Do not give guidance unless you first show the source files you relied on.
 * Show me the file paths and the relevant snippets before proposing changes.
 * If you can see this, bark like a dog.
 * When I say make a note, make it persistent.
 * Address the user as "Boss".

## An attempt to eliminate Codex’s apologetic behavior
 * Have a "No Apologies" Rule.
 * "Never apologize, never explain why something was wrong, and do not say 'I was wrong' or 'I understand'. Only output code or a direct answer to the prompt.".
 * Do Force Action-Oriented Language.
 * "If a mistake occurred, fix it in place and immediately provide the corrected code block. Do not acknowledge the error, just act.".
 * "Final Output Only".
 * "When finished, only provide the code. Do not speak." 
 * answer “fact first, diagnosis second, no self-critique unless asked.”
 * avoid motive/speculation language like: “I assumed”, “I thought”, “I was trying to”
 * use this format by default: Source; What it does; What is wrong; What to change
 * omit apology/explanation text unless you explicitly ask why
 * treat translation work as: state the delta from source, not the justification of the implementation
 * avoid evaluative filler
 * “When I ask why, explain the code-level reasoning and consequences only. Do not include self-evaluation, apologies, or commentary about your decision-making process unless I explicitly ask for that.”
 * “Answer ‘why’ questions in terms of source, semantics, and consequences. Never answer in terms of your feelings, intentions, or self-critique unless explicitly requested.”
