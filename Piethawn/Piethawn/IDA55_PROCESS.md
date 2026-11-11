
BEGIN:  2026-11-11-14:48

Still need a better way to represent this spider-web of overlapping projects...

Processes/Pipelines, End-To-End:
   1. sync MAGIC names to WIZARDS
   2. sync MAGIC and WIZARDS names to ReMoM

entry:
C:\STU_DASM\IDA55_MOM131\MAGIC.idb
C:\STU_DASM\IDA55_MOM131\WIZARDS.idb
manual segment map  (MGC-vs-WZD.segment-map.json)
C:\STU\devel\STU-Extras\Piethawn\Piethawn\ida55_exact_match_overrides.json

intermediate:
   phase 1
      hash per proc, from .asm file

exit:
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\MAGIC\
C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\



Terminating Nodes / Exit Nodes / Outputs:
   1. for reference elsewhere, including by AI Agents; .asm files per proc, organized by segment and executable
      a. C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\MAGIC\
      b. C:\STU\devel\STU-Extras\Piethawn\Piethawn\out\WIZARDS\



ida55_registry_dump.py  >>>   ida55_registry_settings.json

ida55_clean_export.py   >>>   ida55-reg-before.reg
ida55_clean_export.py   >>>   ida55-reg-after.reg



END:  2026-11-11-14:48



IDA Automation:
ida_automation/idaw.py
Everything that runs an IDC script goes through it.
It refuses to start if another IDA is running or unpacked .id0/.id1/.nam/.til files sit beside the IDB.
It backs up the .idb to C:\STU_DASM\IDA55_MOM131\
runs C:\STU_DASM\IDA55\idaw.exe with the arguments you pass
afterwards checks for a 0-byte IDB, unpacked-database warnings and leftover unpacked files



Input:
.asm files
func info
manual segment map




As of 202611091531, ...
script export of data from MGC and WZD .idb's
split whole EXE asm into seg/proc.asm
upsert names



IDA Export script:
check .idb's aren't current open
backup .idbs
Python script to run command line IDA (idaw.exe) with IDC script
cleanup: good .idb's, idaw isn't running, etc.

Path to IDA:
C:\STU_DASM\IDA55\
Path to MoMv131 .idb's:
C:\STU_DASM\IDA55_MOM131\

One-Time Snap-Shot of drake178's names
C:\STU_DASM\drake178\

@binaryalgorithm (BinAlgo)
C:\STU\devel\mom131-source-map\
...need to map names...


¿¿¿
The STU_DASM & ReMoM name sync is its own process, that is a preprocess for the name mapping.
(the finished proc color color sync is its own process as well, but it is not a preprocess for the name mapping)
Because, ...
at the point of name mapping,
we need the onetime snapshot of drake178s IDA names
we need the current snapshot of my IDA names
we need the onetime snapshot of binaryalgorithm's C code names
we need the current snapshot of my C code names
we need the onetime snapshot of the MoO2 debug-symbol names
???

The IDA export feeds the name sync and the ASM split

ida55_clean_export.py


ida_dasm_export/ida55_clean_export.py
→ in/MAGIC.asm, in/WIZARDS.asm

ida_proc_data_export/ida55_export_function_data.py
→ in/*.idb.ida55-functions.txt


export .ASM    |-> split .ASM



```mermaid
flowchart TD
    IDB["MAGIC.idb / WIZARDS.idb"]
    EXP1["ida_dasm_export/ida55_clean_export.py<br/>→ in/MAGIC.asm, in/WIZARDS.asm"]
    EXP2["ida_proc_data_export/ida55_export_function_data.py<br/>→ in/*.idb.ida55-functions.txt"]
    SPLIT["ida55_split_asm.py → out/MAGIC, out/WIZARDS per-proc .asm"]
    INV["ida55_functions_to_inventory.py<br/>→ out/ida55-procedure-snapshots/{MGC,WZD}.procedure-inventory.json"]
    BODY["ida55_asm_body_evidence.py<br/>→ {MGC,WZD}.asm-body-evidence.json"]
    JOIN["ida55_join_procedure_evidence.py<br/>→ {MGC,WZD}.procedure-evidence.json"]
    SEGMAP["MGC-vs-WZD.segment-map.json<br/>(hand-maintained)"]
    CMP["compare_procedure_evidence.py<br/>→ MGC-vs-WZD.procedure-evidence-comparison.json"]
    HAND["build_magic_name_sync_handoff.py<br/>→ magic_sync_names_from_wzd.json"]
    GEN["generate_magic_name_sync.py<br/>→ ida_proc_name_sync/magic_sync_names_from_wzd.idc"]
    APPLY["Apply in IDA 5.5 on MAGIC.idb<br/>(renames, then SaveBase)"]

    IDB --> EXP1 --> SPLIT
    IDB --> EXP2 --> INV
    EXP1 --> BODY
    INV --> JOIN
    BODY --> JOIN
    JOIN --> CMP
    SEGMAP --> CMP
    CMP --> HAND --> GEN --> APPLY
```
