# WZD To MGC Procedure Name Sync PRD

## Summary

Synchronize procedure names from WZD to MGC using a stable, already-reviewed procedure map.

The disassemblies are not expected to change. Once the segment and procedure relationships are known, the ongoing workflow should not rediscover them. It should only compare the current names in the two IDBs against the approved map, generate a small rename plan, apply the approved MGC renames, and verify the result.

The bottom of this document describes the one-time work required to create and maintain the approved map.

## Ongoing Process

### Intent

Use WZD as the naming source and MGC as the rename target.

The regular operation should answer only these questions:

- for each approved WZD-to-MGC procedure pair, do the names currently differ?
- is the WZD name allowed to replace the MGC name?
- can the MGC procedure still be found at the expected identity?
- did the rename succeed?

It should not spend time proving again that the procedures match.

### Inputs

The ongoing process needs:

- approved WZD-to-MGC procedure map
- current MGC procedure-name snapshot
- current WZD procedure-name snapshot
- optional denylist of names that should not be propagated

The approved map is the authority for procedure relationships. Current snapshots are only used to see what the names are now and to verify that the expected procedures still exist.

### Approved Procedure Map

The approved map should be stable and checked in.

Each record should identify a known relationship:

```json
{
  "pair_id": "MGC:ovr001:17__WZD:ovr003:17",
  "mgc": {
    "segment": "ovr001",
    "procedure_ordinal": 17,
    "start_ea": "1234:5678",
    "baseline_name": "Old_MGC_Name"
  },
  "wzd": {
    "segment": "ovr003",
    "procedure_ordinal": 17,
    "start_ea": "2345:6789",
    "baseline_name": "Better_WZD_Name"
  },
  "relationship": "same_procedure",
  "rename_policy": "wzd_name_wins",
  "notes": "Same logical procedure in paired segments with different ids."
}
```

The map should support these relationship values:

- `same_procedure`
- `same_procedure_different_segment_id`
- `equivalent_one_off`
- `equivalent_but_drifted`
- `mgc_only`
- `wzd_only`
- `do_not_sync`

Only records with an explicit rename policy should produce rename actions.

### Current Name Snapshots

Each ongoing run captures a lightweight procedure-name snapshot from each IDB.

The snapshot should include:

- database label
- IDB path
- snapshot timestamp
- segment id/name
- procedure ordinal within segment
- start address
- end address
- current procedure name

The snapshot does not need to re-hash every function during normal operation. Hashes belong in the one-off discovery process unless we are validating that an IDB has unexpectedly changed.

### Rename Policy

Supported policies:

- `wzd_name_wins`: rename MGC to the current WZD name.
- `keep_mgc_name`: never rename this MGC procedure from WZD.
- `manual_review`: include in report, but do not emit an IDC rename.
- `do_not_sync`: ignore during ongoing sync.

Default behavior should be conservative:

- no policy means no rename
- one-off records do not rename unless explicitly marked `wzd_name_wins`
- drifted-equivalent records do not rename unless explicitly marked `wzd_name_wins`

### Ongoing Workflow

1. Capture current MGC and WZD procedure-name snapshots.
2. Load the approved procedure map.
3. For each mapped pair, resolve the current MGC procedure and WZD procedure.
4. Compare current MGC name to current WZD name.
5. Apply rename policy.
6. Generate a rename plan for MGC only.
7. Review the rename plan.
8. Generate IDC containing only approved MGC renames.
9. Run the IDC against MGC.
10. Capture an after snapshot from MGC.
11. Verify every approved rename reached the expected final name.

### Rename Plan

The rename plan is generated every run and is not the same thing as the approved procedure map.

Each rename-plan row should include:

- pair id
- MGC current name
- WZD current name
- proposed MGC new name
- MGC segment
- WZD segment
- MGC procedure ordinal
- WZD procedure ordinal
- MGC start address
- WZD start address
- relationship
- rename policy
- action
- skip reason, when skipped

Actions:

- `rename`
- `already_synced`
- `skip_policy`
- `skip_unresolved_mgc`
- `skip_unresolved_wzd`
- `skip_name_collision`
- `skip_invalid_wzd_name`
- `manual_review`

### Applying Names

The IDC should be generated from the reviewed rename plan.

The IDC should:

- run only against MGC
- find the expected MGC function by address when possible
- verify that the current function name matches the reviewed plan or an allowed current-name variant
- check whether the target WZD name already exists elsewhere in MGC
- rename the function
- log `renamed`, `already-synced`, `not-found`, `collision`, or `failed`

The IDC should not attempt to infer unresolved matches. If a mapped procedure cannot be resolved, it should log and skip.

### Verification

After the IDC runs, capture MGC names again and compare against the reviewed plan.

The verification report should show:

- expected renames
- successful renames
- skipped renames
- failed renames
- unexpected name changes
- unresolved mapped procedures

The ongoing milestone is complete when every approved `rename` action is either applied or has a recorded skip/failure reason.

### Ongoing Outputs

- `mgc-procedure-names-before.json`
- `wzd-procedure-names-current.json`
- `wzd-to-mgc-rename-plan.json`
- `wzd-to-mgc-rename-plan.md`
- `wzd_to_mgc_rename.idc`
- `MGC.idb.wzd-to-mgc-rename.log`
- `mgc-procedure-names-after.json`
- `wzd-to-mgc-rename-verification.json`

### Ongoing Non-Goals

- Do not rediscover segment pairs.
- Do not rediscover procedure equivalence.
- Do not rewrite translated `.c` files.
- Do not update translation checklists.
- Do not update TODO completion state.
- Do not sync IDA completion colors.
- Do not rename WZD from MGC.

## Handling Known Special Cases During Ongoing Sync

### Same Segment, Different Segment ID

The approved map records the segment relationship. Ongoing sync uses that map directly.

The tool should not reject a pair merely because MGC and WZD segment ids differ.

### One-Off Procedures

One-off procedures are expected map records, not runtime mysteries.

Default ongoing behavior:

- report them
- do not rename them

Only rename one-offs when the approved map explicitly says `wzd_name_wins`.

### Equivalent One-Off Procedures

Equivalent one-offs may share purpose without being a strict same-procedure match.

Default ongoing behavior:

- report as `manual_review` or `skip_policy`
- do not rename automatically

### Equivalent But Drifted Procedures

Drifted equivalents are procedures that are related but no longer identical.

Default ongoing behavior:

- report as `manual_review` or `skip_policy`
- do not rename automatically

If a drifted equivalent has an explicit `wzd_name_wins` policy, the rename can be emitted, but the plan should keep the relationship visible.

## One-Off Map Creation Process

This section describes setup work. It should not be part of every sync run.

### One-Off Intent

Create the approved WZD-to-MGC procedure map once, then maintain it manually when new knowledge is discovered.

This is where we prove which segments and procedures correspond.

### One-Off Inputs

The one-off process can use richer data:

- full ASM exports
- function dumps
- segment dumps
- byte hashes
- normalized instruction hashes
- procedure order within segments
- known overlay correspondence
- manual notes
- pair-diff reports

The first authoritative capture boundary for procedure names is a generic IDA IDC snapshot:

- `ida55_procedure_snapshot.idc`

This IDC captures the current procedure names and addresses directly from the IDB. Python tooling may parse and validate the snapshot, but should not use old function-dump conversion artifacts as the source of truth for this process.

### Existing Asset Evaluation

The old post-IDA flow already contains useful matching pieces:

- `ida55_exact_match_overrides.json`
- `ida55_match_functions.py`
- `ida55_generate_magic_rename_idc.py`

For this new process, only part of that behavior belongs in the one-off map creation stage.

Keep:

- segment-pair data from `ida55_exact_match_overrides.json`
- by-position pairing as evidence for known segment pairs
- exact byte hash matching as one evidence source
- name-mismatch reporting as review input

Do not keep as the long-term shape:

- generating MGC renames directly from a freshly recomputed matcher result
- treating segment-pair overrides as runtime sync configuration
- emitting `magic_rename_to_wizards.idc` directly from transient match data

New ownership:

- segment-pair data moves to `ida_proc_name_sync_prereq/MGC-vs-WZD.segment-map.json` only after the one-off process generates and validates it
- procedure relationships move to `wzd-to-mgc-procedure-map.json`
- rename generation moves to the ongoing sync plan/IDC flow

Current prereq segment map:

- the segment-pair data now lives at `ida_proc_name_sync_prereq/MGC-vs-WZD.segment-map.json`
- the comparison step uses this prereq-owned file directly
- future review can replace or refine it without depending on the old `wip` evaluation path

Legacy status:

- `ida55_exact_match_overrides.json` is legacy transitional data.
- `ida55_match_functions.py` is legacy transitional matching code.
- `ida55_generate_magic_rename_idc.py` is legacy transitional rename-generation code.
- Keep them available until name sync no longer depends on legacy matcher output.

### Segment Mapping

Some same logical segments have different segment ids between MGC and WZD.

The one-off segment mapping process should record:

- MGC segment id/name
- WZD segment id/name
- relationship type
- evidence
- confidence
- reviewer/date

Useful evidence:

- matching procedure order
- matching procedure hashes
- matching neighboring segments
- known overlay layout
- manual reverse-engineering notes

Output:

- `ida_proc_name_sync_prereq/MGC-vs-WZD.segment-map.json`

### Procedure Mapping

The one-off procedure mapping process should classify each known pair.

Relationship classes:

- `same_procedure`: same code and same role
- `same_procedure_different_segment_id`: same code and same role, segment ids differ
- `equivalent_one_off`: similar role, one-off use in one database
- `equivalent_but_drifted`: historically related, but code diverged
- `mgc_only`: no WZD counterpart
- `wzd_only`: no MGC counterpart
- `do_not_sync`: known relationship, but names should not be synced

Useful evidence:

- exact byte hash match
- normalized instruction hash match
- same ordinal in an approved segment pair
- same call context
- same nearby procedure context
- manual review

Output:

- `wzd-to-mgc-procedure-map.json`

### One-Off Safety Decisions

During map creation, decide:

- whether WZD names always win for exact matches
- whether WZD names win for segment-id-shifted matches
- whether equivalent one-offs should share names or preserve local names
- whether drifted equivalents should share names or preserve local names
- which temporary suffixes or generated names should never be propagated
- whether old MGC names should be retained in notes

### One-Off Reports

Generate human-readable reports for review:

- exact matches with different names
- segment-id-shifted matches
- one-off procedures
- equivalent one-offs
- drifted equivalents
- procedure-name collisions
- proposed `do_not_sync` entries

These reports help create the approved map, but they are not part of the normal sync loop.

### Map Maintenance

The approved map should change only when:

- a missed relationship is discovered
- an existing relationship was classified incorrectly
- a rename policy decision changes
- a procedure should be added to `do_not_sync`
- better evidence is added for a special case

Every map edit should be reviewable in source control.

### Cleanup Phase

After the first fresh one-off map has been generated, reviewed, and used by the ongoing sync process, run a cleanup pass.

Cleanup should evaluate and either remove, archive, or clearly deprecate:

- `ida55_exact_match_overrides.json`
- `ida55_match_functions.py`
- `ida55_generate_magic_rename_idc.py`
- `out/ida55-sync/magic_rename_to_wizards.*` as the direct rename output path

Cleanup should not happen until replacement scripts produce equivalent or better reports and no active pipeline depends on the legacy output shape.

## Expected-Match Audit Boundary

When a segment pair is already identified as an exact-match segment pair, the one-off process must audit the expected ordinal matches instead of rediscovering candidates.

For these segment pairs:

- pair MGC procedure ordinal N with WZD procedure ordinal N
- treat the ordinal pair as the expected relationship
- report mismatch reasons with simple review artifacts
- use byte hash, size, normalized text, mnemonic sequence, and edit distance as diagnostics only
- do not run all-vs-all matching or candidate search for this phase

Short rule:

```text
We are not discovering matches.
We are auditing expected matches.
```

## Success Criteria

Ongoing sync is successful when:

- it uses the approved map without rediscovering matches
- it generates a small, reviewable MGC rename plan
- it applies only reviewed WZD-to-MGC renames
- it logs every action
- it verifies MGC names after the IDC run

The one-off discovery work is successful when:

- segment pairs are recorded
- procedure pairs are recorded
- one-offs are explicitly classified
- drifted equivalents are explicitly classified
- rename policies are explicit
- the approved map can drive the ongoing process without recomputing equivalence
