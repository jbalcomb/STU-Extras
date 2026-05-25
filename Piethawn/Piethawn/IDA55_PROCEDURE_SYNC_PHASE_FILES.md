# IDA55 Procedure Sync Phase Files

This document lists the scripts and generated files currently involved in the MGC/WZD procedure evidence and name-sync phase.

| File | Purpose / Usage |
|---|---|
| [IDA_IDB_Sync-Info.md](IDA_IDB_Sync-Info.md) | Human source notes for known MGC/WZD segment relationships and unresolved procedure questions. |
| [ida_dasm_export\ida55_clean_export.idc](ida_dasm_export\ida55_clean_export.idc) | IDA 5.5 export script. Applies clean export settings, unhides collapsed areas, then exports `MAP`, `ASM`, `INC`, and `LST`. |
| [ida55_procedure_snapshot.idc](ida55_procedure_snapshot.idc) | IDA 5.5 metadata snapshot script. Captures procedure names, segments, ordinals, start/end EAs, and sizes from the IDB. |
| [ida55_procedure_inventory.py](ida55_procedure_inventory.py) | Converts the IDC procedure snapshot text into structured procedure inventory JSON. |
| [ida55_asm_body_evidence.py](ida55_asm_body_evidence.py) | Parses exported ASM and produces per-procedure body evidence: raw hash, normalized hash, mnemonic hash, line counts, instruction count. |
| [ida55_join_procedure_evidence.py](ida55_join_procedure_evidence.py) | Joins IDB metadata inventory with ASM body evidence by `segment + procedure_ordinal`. Used to prove every procedure has body evidence. |
| [ida_proc_name_sync_prereq/write_expected_match_audit_tsv.py](ida_proc_name_sync_prereq/write_expected_match_audit_tsv.py) | One-off/prereq script. Audits expected exact matches by approved segment pair and procedure ordinal. Produces a simple TSV with pass/fail checks. |
| [ida_proc_name_sync_prereq/compare_procedure_evidence.py](ida_proc_name_sync_prereq/compare_procedure_evidence.py) | One-off/prereq script. Compares joined MGC/WZD procedure evidence using the segment map and procedure ordinal. Produces pair-level evidence. |
| [ida55_bucket_procedure_comparison.py](ida55_bucket_procedure_comparison.py) | Buckets comparison results into review categories such as rename candidates, mnemonic-only matches, and differing bodies. |
| [ida_proc_name_sync_prereq/build_magic_name_sync_handoff.py](ida_proc_name_sync_prereq/build_magic_name_sync_handoff.py) | One-off/prereq script. Builds the minimal handoff JSON from the procedure comparison artifact, then copies the final handoff into `ida_proc_name_sync`. The handoff contains only `old_name` and `new_name` rename pairs. |
| [ida_proc_name_sync/generate_magic_name_sync.py](ida_proc_name_sync/generate_magic_name_sync.py) | Generates the MAGIC IDB IDC name-sync script from the minimal rename handoff JSON. |
| [tests/test_ida_proc_name_sync.py](tests/test_ida_proc_name_sync.py) | Tests the handoff builder, handoff validation, IDC escaping, and IDC safety checks. |
| [ida_proc_name_sync_prereq/MGC-vs-WZD.segment-map.json](ida_proc_name_sync_prereq/MGC-vs-WZD.segment-map.json) | One-off/prereq segment-pair input used by the comparison. |
| [out/ida55-procedure-snapshots/MGC.procedure-inventory.json](out/ida55-procedure-snapshots/MGC.procedure-inventory.json) | MGC IDB procedure metadata inventory. |
| [out/ida55-procedure-snapshots/WZD.procedure-inventory.json](out/ida55-procedure-snapshots/WZD.procedure-inventory.json) | WZD IDB procedure metadata inventory. |
| [out/ida55-procedure-snapshots/MGC.asm-body-evidence.json](out/ida55-procedure-snapshots/MGC.asm-body-evidence.json) | MGC ASM body evidence generated from the standard export. |
| [out/ida55-procedure-snapshots/WZD.asm-body-evidence.json](out/ida55-procedure-snapshots/WZD.asm-body-evidence.json) | WZD ASM body evidence generated from the standard export. |
| [out/ida55-procedure-snapshots/MGC.procedure-evidence.json](out/ida55-procedure-snapshots/MGC.procedure-evidence.json) | Strict joined MGC metadata + ASM body evidence. |
| [out/ida55-procedure-snapshots/WZD.procedure-evidence.json](out/ida55-procedure-snapshots/WZD.procedure-evidence.json) | Strict joined WZD metadata + ASM body evidence. |
| [ida_proc_name_sync_prereq/expected-match-audit.tsv](ida_proc_name_sync_prereq/expected-match-audit.tsv) | One-off/prereq TSV audit of expected exact matches. Rows are approved segment-pair + ordinal relationships, with failed checks sorted first. |
| [ida_proc_name_sync_prereq/MGC-vs-WZD.procedure-evidence-comparison.json](ida_proc_name_sync_prereq/MGC-vs-WZD.procedure-evidence-comparison.json) | One-off/prereq procedure-pair evidence generated from joined files plus the segment map. |
| [ida_proc_name_sync_prereq/MGC-vs-WZD.procedure-review-buckets.json](ida_proc_name_sync_prereq/MGC-vs-WZD.procedure-review-buckets.json) | One-off/prereq machine-readable bucketed review report. |
| [ida_proc_name_sync_prereq/MGC-vs-WZD.procedure-review-buckets.md](ida_proc_name_sync_prereq/MGC-vs-WZD.procedure-review-buckets.md) | One-off/prereq human-readable bucketed review report. |
| [ida_proc_name_sync/magic_sync_names_from_wzd.idc](ida_proc_name_sync/magic_sync_names_from_wzd.idc) | Generated IDC script for renaming MAGIC procedures to the corresponding WZD procedure names. |
| [ida_proc_name_sync/magic_sync_names_from_wzd.json](ida_proc_name_sync/magic_sync_names_from_wzd.json) | Minimal handoff JSON for generated MAGIC rename actions. Contains only the fields needed by the IDC generator. |
| [ida_proc_name_sync_prereq/magic_sync_names_from_wzd.json](ida_proc_name_sync_prereq/magic_sync_names_from_wzd.json) | Prereq-owned copy of the minimal handoff generated from comparison evidence. |

## Current Command Shape

```bash
python3 ida55_procedure_inventory.py /mnt/c/STU_DASM/IDA55_MOM131/MAGIC.idb.ida55-procedure-snapshot.txt --database MGC --output out/ida55-procedure-snapshots/MGC.procedure-inventory.json
python3 ida55_procedure_inventory.py /mnt/c/STU_DASM/IDA55_MOM131/WIZARDS.idb.ida55-procedure-snapshot.txt --database WZD --output out/ida55-procedure-snapshots/WZD.procedure-inventory.json

python3 ida55_asm_body_evidence.py /mnt/c/STU_DASM/IDA55_MOM131/MAGIC.asm --database MGC --output out/ida55-procedure-snapshots/MGC.asm-body-evidence.json
python3 ida55_asm_body_evidence.py /mnt/c/STU_DASM/IDA55_MOM131/WIZARDS.asm --database WZD --output out/ida55-procedure-snapshots/WZD.asm-body-evidence.json

python3 ida55_join_procedure_evidence.py --inventory out/ida55-procedure-snapshots/MGC.procedure-inventory.json --asm-body-evidence out/ida55-procedure-snapshots/MGC.asm-body-evidence.json --output out/ida55-procedure-snapshots/MGC.procedure-evidence.json
python3 ida55_join_procedure_evidence.py --inventory out/ida55-procedure-snapshots/WZD.procedure-inventory.json --asm-body-evidence out/ida55-procedure-snapshots/WZD.asm-body-evidence.json --output out/ida55-procedure-snapshots/WZD.procedure-evidence.json

python3 ida_proc_name_sync_prereq/write_expected_match_audit_tsv.py

python3 ida_proc_name_sync_prereq/compare_procedure_evidence.py --left out/ida55-procedure-snapshots/MGC.procedure-evidence.json --right out/ida55-procedure-snapshots/WZD.procedure-evidence.json --segment-map ida_proc_name_sync_prereq/MGC-vs-WZD.segment-map.json --output ida_proc_name_sync_prereq/MGC-vs-WZD.procedure-evidence-comparison.json

python3 ida55_bucket_procedure_comparison.py ida_proc_name_sync_prereq/MGC-vs-WZD.procedure-evidence-comparison.json --json-output ida_proc_name_sync_prereq/MGC-vs-WZD.procedure-review-buckets.json --markdown-output ida_proc_name_sync_prereq/MGC-vs-WZD.procedure-review-buckets.md --limit-per-bucket 25

python3 ida_proc_name_sync_prereq/build_magic_name_sync_handoff.py --comparison ida_proc_name_sync_prereq/MGC-vs-WZD.procedure-evidence-comparison.json --output ida_proc_name_sync_prereq/magic_sync_names_from_wzd.json --final-copy ida_proc_name_sync/magic_sync_names_from_wzd.json
python3 ida_proc_name_sync/generate_magic_name_sync.py --renames ida_proc_name_sync/magic_sync_names_from_wzd.json --output-idc ida_proc_name_sync/magic_sync_names_from_wzd.idc
```
