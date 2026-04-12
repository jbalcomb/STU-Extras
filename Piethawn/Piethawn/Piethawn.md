
Here is a list of segments I am sure are exactly the same. Add these to a json file and add an override to the process to assume these should be exact matches.
MAGIC,WIZARDS
seg002,seg002
seg003,seg003
seg004,seg004
seg005,seg005
seg006,seg006
seg007,seg007
seg008,seg008
seg009,seg009
seg010,seg010
seg011,seg011
seg012,seg012
seg013,seg013
seg014,seg014
seg015,seg015
seg016,seg016
seg017,seg017
seg018,seg018
seg019,seg019
seg020,seg020
seg021,seg021
seg022,seg022
seg023,seg023
seg024,seg024
seg025,seg025
seg026,seg028
seg027,seg029
seg028,seg030
seg029,seg031
seg030,seg032
seg031,seg033
seg032,seg034
seg033,seg035
seg034,seg036
_AIL_,_AIL_
SOUND,SOUND
stub048,stub050
ovr048,ovr050

One important detail: your list includes stub048 -> stub050 and seg048 -> seg050, so the j_ SAVE_GAM stubs are now forced exact matches. But Load_SAVE_GAM and Save_SAVE_GAM live in ovr048 and ovr050, not seg048 and seg050, so they are still not overridden. If that was meant to cover the overlay bodies too, the next fix is to add ovr048 -> ovr050 to the override JSON.



                     |               |       |
    MAGIC.EXE        WIZARDS.EXE             
[ ] RP_CLK_ExtWait   UU_CLK_ExtWait  seg002  seg002
[x] Set_Page_On      Set_Page_On     seg026  seg028
[ ] UU_Enable_Cancel Enable_Cancel   seg034  seg036







1. Produce .ASM in IDA Pro v5.5
  - change settings
  - save as <EXE NAME>.ASM in project directory
2. Build PROC asm tree
  - extract each PROC
  - save as <PROC NAME>.asm

IDA 5.5 sync workflow now also includes:
- function-name synchronization from WIZARDS to MAGIC using machine-code and positional segment pairing
- function completion synchronization using IDA function colors
- freshness tracking using `MAGIC.idb/id0/id1` and `WIZARDS.idb/id0/id1` timestamps

Relevant scripts:
- `run_post_ida55_dump_pipeline.py`
- `ida55_generate_magic_rename_idc.py`
- `ida55_dump_function_colors.idc`
- `ida55_function_color_dump.py`
- `ida55_merge_completion_status.py`
- `ida55_db_timestamp_config.py`
- `configure_daily_timestamp_task.py`

Later, get C89 code from Gemini and compare to code in ReMoM

I had Claude create a skill for Gemini: .gemini\skills\rasm\SKILL.md
Claude also create a script to use it: Rasm-Gemini.cmd
I had also previously asked it for a prompt for Gemini: RasmAiAgentPrompt.md
Now, we need a python script that makes use of all of this, so that we can work through the todo list of the function that just stubbed ("...__STUB") or even still works in progress ("...__WIP").
This was my test sample: `./Rasm-Gemini.cmd out/MAGIC/seg022/PageFlip_GrowOut__WIP.asm`

Default Gemini translation model for the Piethawn wrappers is `gemini-3.1-pro-preview`.
Override it only when needed with `RASM_GEMINI_MODEL` or `--model`.

C:\STU\devel\ReMoM\doc\#TODO\stub_wip_todo.md
C:\STU\devel\ReMoM\tools\todolist\extract_stub_wip_functions.py
C:\STU\devel\ReMoM\tools\todolist\generate_stub_wip_todo.py

FWIW, the final output todo list is stub_wip_todo.md in C:\STU\devel\ReMoM\doc\#TODO\, create by generate_stub_wip_todo.py and extract_stub_wip_functions.py in C:\STU\devel\ReMoM\tools\todolist\.

Piethawn-side Gemini queue helper:

`c:/python314/python.exe .\rasm_todo_gemini.py list --limit 20`
`c:/python314/python.exe .\rasm_todo_gemini.py next --dry-run`
`c:/python314/python.exe .\rasm_todo_gemini.py run PageFlip_GrowOut__WIP --dry-run`
`c:/python314/python.exe .\rasm_random_todo_gemini.py --dry-run --seed 1`

Notes:
- The script reads the ReMoM todo markdown by default from `C:\STU\devel\ReMoM\doc\#TODO\stub_wip_todo.md`.
- It resolves TODO function names to Piethawn ASM under `out\MAGIC` and `out\WIZARDS`.
- It invokes `Rasm-Gemini.cmd` and sets `GEMINI_SYSTEM_MD=RasmAiAgentPrompt.md` for the Gemini run.
- `Rasm-Gemini.cmd` defaults to `gemini-3.1-pro-preview` when `RASM_GEMINI_MODEL` is unset.
- Use `--force` to regenerate an existing `.c` output.
- `rasm_random_todo_gemini.py` picks one pending TODO entry at random and submits that function to Gemini.

Do WZD s22p29 UU_DBG_SetSelectSetting() and WZD s22p30 UU_DBG_SelectDialog()
Do UU_VGA_DrawDblRect

Graphics.c - messy

// WZD s14p08
// 1oom  uidraw.c  ui_draw_line_limit()
void Clipped_Line(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t color)
    Clipped_Line_Base(x1, y1, x2, y2, ST_FALSE, color, ST_NULL, ST_NULL, ST_NULL);
// WZD s14p09
// 1oom  uidraw.c  ui_draw_line_limit_ctbl()
void Clipped_Multi_Colored_Line(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t colortbl[], int colornum, int colorpos)
    Clipped_Line_Base(x1, y1, x2, y2, 1, 0, &colortbl[0], colornum, colorpos);

// WZD s14p10
void Clipped_Line_Base(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t Patterned, uint8_t color, uint8_t colortbl[], int16_t colornum, int16_t colorpos)
static void ui_draw_line_limit_do(int x1, int y1, int x2, int y2, uint8_t color, const uint8_t *colortbl, int colornum, int colorpos)

// WZD s16p04
// MoO2  Multi_Colored_Line()
void Multi_Colored_Line(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint8_t colortbl[], int16_t colornum, int16_t colorpos)
void ui_draw_line_ctbl(int x1, int y1, int x2, int y2, const uint8_t *colortbl, int colornum, int pos)

// WZD s14p11
// drake178: UU_VGA_DrawBiColorRect()
// MoO2  Module: graphics  Interlaced_Fill()
// UU_Interlaced_Fill()
// WZD s14p12
// UU_VGA_DrawRect()
// WZD s14p13
// UU_VGA_WndDrawRect()
// WZD s14p14
// UU_VGA_DrawDblRect()

// WZD s16p05
// UU_VGA_CreateColorWave
// WZD s16p06
// UU_VGA_DiagColumns

// WZD s16p07
// 1oom  rnd.c  rnd_bitfiddle()
uint16_t rnd_bitfiddle__1oom(uint16_t ax)

// WZD s16p08
// MoO2  Module: graphics  Tint_Fill()
void Tint_Fill(int16_t x1, int16_t y1, int16_t x2, int16_t y2, int16_t remap_block)

// WZD s16p09
// drake178: VGA_Grayscale_Rect()
// MoO2  Module: graphics  Gray_Scale_Fill()
// VGA_Grayscale_Rect
// Gray_Scale_Fill

// WZD s16p10
// drake178: UU_VGA_Columns()
// MoO2  
// UU_VGA_Columns

// WZD s16p11
// drake178: UU_VGA_ScrambleRect()
// MoO2  
// UU_VGA_ScrambleRect





MAGIC ovr059 is the same as WIZARDS ovr159 - Draw_Help_Entry(), etc.



What might this look like convert from MS-DOS to SDL2 on Windows?



C:\STU\devel\ReMoM\doc\#MoO2\ORION2.asm
Module: shear
Does the function Capture_Screen_Block in WIZARDS.EXE look like it is the same function as Capture_Screen_Block_ (Address: 01:0014791B) in ORION2.EXE?

I have enough to answer: the ORION2 routine is a much smaller linear-framebuffer helper, while the WIZARDS routine does the same job on planar VGA memory. I’m summarizing the exact equivalence and the differences next.Yes, it looks like the same logical function, but not the same low-level implementation.

The strongest evidence is that the two routines do the same job with the same outer contract: copy a rectangular block from the current screen buffer into a preallocated picture/bitmap buffer. ReMoM’s own annotation in capture.c already points in that direction by tying WIZARDS Capture_Screen_Block() to MoO2 Capture_Screen_Block_() in the shear module.

What differs is the framebuffer model:

WIZARDS version in Capture_Screen_Block.asm is 16-bit planar VGA code.
It computes src_width, src_height, and src_ofst.
It derives the starting VGA plane from x1 & 3.
It programs port 0x3CE/0x3CF to select the read map.
It copies one vertical strip per plane byte and advances by 80 bytes per scanline.
ORION2 version in ORION2.asm is a compact near helper for a linear framebuffer.
Inputs are already reduced by the caller to eax = src_ofst, ecx = dst_ofst, edx = row_width, ebx = row_count.
The body is just rep movsb, then add esi, screen_pixel_width2, sub esi, edx, dec ebx, loop.
That is row-by-row rectangular copy on a chunky buffer.
So the right conclusion is:

Same semantic role: yes.
Same code shape: no.
WIZARDS is the older planar-VGA ancestor.
ORION2 is the later linear/chunky-buffer version after the hard VGA plane handling has been pushed out of this helper.
A concise mapping is:

WIZARDS Capture_Screen_Block(frame_data, x1, y1, x2, y2)
ORION2 Capture_Screen_Block_(src_ofst, dst_ofst, width, height)
That is exactly the kind of refactoring you would expect when moving from direct VGA access to a software framebuffer: the rectangle math and planar deinterleaving disappear from the callee, and the callee becomes a simple row copier.

If you want, I can next extract the ORION2 caller around 01:0014791B and map its registers back to the WIZARDS parameters so the equivalence is explicit.





c:/python314/python.exe .\rasm_named_gemini.py UU_VGA_DrawRect --game WIZARDS --force



.\.venv\Scripts\python.exe .\ida55_split_asm.py .\MAGIC.asm

.\.venv\Scripts\python.exe .\ida55_split_asm.py .\WIZARDS.asm

cat out/MAGIC/seg001/Menu_Screen_Control.asm | gemini -p "This is disassembly output from IDA Pro v5.5 for MAGIC.EXE from the game 'Master of Magic' (1994). It was compiled with Borland C++ v3.0. What might this have looked like in C89?" > out/MAGIC/seg001/Menu_Screen_Control.md


out/MAGIC/seg022/PageFlip_GrowOut__WIP.asm

./.gemini

{{args}}
PageFlip_GrowOut__WIP
{{args}}.asm

Narp.
Have to get rid of seg022 and MAGIC.



How to Use Prompt Templates in Gemini CLI:
Custom Commands (TOML): Create TOML files in ~/.gemini/commands/ (global) or .gemini/commands/ (local project) to save prompt structures.
System Prompts: Override default behavior by setting a custom system prompt using GEMINI_SYSTEM_MD.
MCP Prompts: Use the Model Context Protocol (starting v0.1.15) to access predefined prompt templates from servers, which appear as actionable slash commands.
JSON Templates: Define structured prompt templates in JSON format within a prompts/ directory to handle complex tasks like code refactoring or diff explanations. 
Benefits:
Reusability: Write complex, multi-line prompts once (e.g., for CRUD operations or code analysis) and reuse them.
Consistency: Standardize responses across projects.
Dynamic Input: Use arguments to fill in placeholders within the template. 
You can also use tools like LM Studio's command template generator to help build these templates. 
LM Studio



https://geminicli.com/docs/cli/custom-commands/

File locations and precedence
Gemini CLI discovers commands from two locations, loaded in a specific order:

User commands (global): Located in ~/.gemini/commands/. These commands are available in any project you are working on.
Project commands (local): Located in <your-project-root>/.gemini/commands/. These commands are specific to the current project and can be checked into version control to be shared with your team.
If a command in the project directory has the same name as a command in the user directory, the project command will always be used. This allows projects to override global commands with project-specific versions.

Naming and namespacing
The name of a command is determined by its file path relative to its commands directory. Subdirectories are used to create namespaced commands, with the path separator (/ or \) being converted to a colon (:).

A file at ~/.gemini/commands/test.toml becomes the command /test.
A file at <project>/.gemini/commands/git/commit.toml becomes the namespaced command /git:commit.
Tip

After creating or modifying .toml command files, run /commands reload to pick up your changes without restarting the CLI.


                      


Error when talking to Gemini API Full report available at: C:\Users\jbalcomb\AppData\Local\Temp\gemini-client-error-Turn.run-sendMessageStream-2026-03-11T18-17-53-682Z.json ModelNotFoundError: Requested entity was not found.




https://geminicli.com/docs/cli/tutorials/automation/#build-your-own-custom-ai-tools

https://geminicli.com/docs/cli/tutorials/shell-commands/



gemini -p "Write a poem about TypeScript"

Run Gemini CLI in headless mode by providing a prompt with the -p (or --prompt) flag. This bypasses the interactive chat interface and prints the response to standard output (stdout).

Feed data into Gemini using the standard Unix pipe |. Gemini reads the standard input (stdin) as context and answers your question using standard output.

cat error.log | gemini -p "Explain why this failed"
Get-Content error.log | gemini -p "Explain why this failed"
git diff | gemini -p "Write a commit message for these changes"




Scenario: Bulk documentation generator
You have a folder of Python scripts and want to generate a README.md for each one.

Save the following code as generate_docs.sh (or generate_docs.ps1 for Windows):

macOS/Linux (generate_docs.sh)

#!/bin/bash

# Loop through all Python files
for file in *.py; do
  echo "Generating docs for $file..."

  # Ask Gemini CLI to generate the documentation and print it to stdout
  gemini -p "Generate a Markdown documentation summary for @$file. Print the
  result to standard output." > "${file%.py}.md"
done



Extract structured JSON data
When writing a script, you often need structured data (JSON) to pass to tools like jq. To get pure JSON data from the model, combine the --output-format json flag with jq to parse the response field.

Scenario: Extract and return structured data
Save the following script as generate_json.sh (or generate_json.ps1 for Windows):

macOS/Linux (generate_json.sh)

#!/bin/bash

# Ensure we are in a project root
if [ ! -f "package.json" ]; then
  echo "Error: package.json not found."
  exit 1
fi

# Extract data
gemini --output-format json "Return a raw JSON object with keys 'version' and 'deps' from @package.json" | jq -r '.response' > data.json

3. Check data.json. The file should look like this:
{
  "version": "1.0.0",
  "deps": {
    "react": "^18.2.0"
  }
}














search "IDA"
...search "Hex-Rays"

HKEY_CURRENT_USER\Software\Hex-Rays\IDA\
¿ ~== HKEY_USERS\S-1-5-21-77677389-1171127288-2707885236-9669\Software\Hex-Rays\IDA\ ?


PS C:\Users\jbalcomb\OneDrive - ID Networks, Inc\devel\Piethawn>
& "C:/Users/jbalcomb/OneDrive - ID Networks, Inc/devel/Piethawn/.venv/Scripts/Activate.ps1"

(.venv) PS C:\Users\jbalcomb\OneDrive - ID Networks, Inc\devel\Piethawn>
& "C:/Users/jbalcomb/OneDrive - ID Networks, Inc/devel/Piethawn/.venv/Scripts/python.exe" "c:/Users/jbalcomb/OneDrive - ID Networks, Inc/devel/Piethawn/ida55_registry_dump.py"

Wrote ida55_registry_settings.json

(.venv) PS C:\Users\jbalcomb\OneDrive - ID Networks, Inc\devel\Piethawn>
& "c:\Users\jbalcomb\OneDrive - ID Networks, Inc\devel\Piethawn\.venv\Scripts\Activate.ps1"

(.venv) PS C:\Users\jbalcomb\OneDrive - ID Networks, Inc\devel\Piethawn> 





Hand-Crafted Examples

Extract Each Individual PROC from the IDA ASM dump

UU_VGA_Bleep.asm
Randomize.asm
Random.asm

At this point, the next required input is the real IDA-generated .asm file or a representative excerpt. Once you add that, I can write the segment/procedure splitter against the actual 5.5 syntax instead of guessing.

...

I handcrafted examples for you. I extracted the procedures UU_VGA_Bleep, Randomize, Random from MAGIC.asm and saved them in out/MAGIC as UU_VGA_Bleep.asm, Randomize.asm, Random.asm. 





.\.venv\Scripts\python.exe .\ida55_split_asm.py .\MAGIC.asm

.\.venv\Scripts\python.exe .\ida55_split_asm.py .\WIZARDS.asm

cat out/MAGIC/seg001/Menu_Screen_Control.asm | gemini -p "This is disassembly output from IDA Pro v5.5 for MAGIC.EXE from the game 'Master of Magic' (1994). It was compiled with Borland C++ v3.0. What might this have looked like in C89?" > out/MAGIC/seg001/Menu_Screen_Control.md

cat out/WIZARDS/ovr114/AI_BU_AssignAction__WIP.asm | gemini -p "This is disassembly output from IDA Pro v5.5 for MAGIC.EXE from the game 'Master of Magic' (1994). It was compiled with Borland C++ v3.0. What might this have looked like in C89?" > out/WIZARDS/ovr114/AI_BU_AssignAction__WIP.md





undone
unsure
funzie

unsure
    bug?
        AI_BU_AssignAction__WIP()
