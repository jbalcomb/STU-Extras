
## Style Guide
- Always end every function comment with "Powered by AI."
- Use four spaces for indentation rather than tabs.

## Tooling Architecture Guidance
- Before creating new Piethawn automation scripts, read `ARCHITECTURE_DESIGN_PHILOSOPHY.md`.
- Prefer shared library modules plus thin command wrappers.
- Build small, deterministic, reviewable chunks instead of large script islands.
- Do not create final-looking artifacts before the process has generated and validated them.
- Keep legacy/evaluation data clearly separated from approved process inputs.

## AI Agent Operating Notes
- Run IDA IDC scripts through `ida_automation\idaw.py` instead of raw `idaw.exe` and instead of asking the user to run them, unless blocked by a real execution failure or missing input.

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
 
