# Architecture And Design Philosophy Notes

## Purpose

This note tracks the project-level design posture for Piethawn tooling.

The goal is to avoid building disconnected script islands. New automation should grow as small, reviewable chunks on top of shared library code, so a human can inspect, trust, and rerun each step.

## Current Design Pressure

The project has accumulated useful scripts that solve immediate problems, but several scripts repeat similar concepts:

- procedure identity
- segment identity
- path normalization
- JSON loading and writing
- function matching
- rename planning
- report rendering
- IDC generation

Repeated concepts increase risk because two scripts can appear to do the same thing while quietly disagreeing about data shape or edge cases.

## Design Principle

Prefer a small shared platform plus thin command wrappers.

Library modules should contain:

- data models
- parsing
- validation
- comparison logic
- report helpers
- IDC rendering helpers

Command scripts should:

- parse command-line arguments
- load inputs
- call one narrow operation
- write one main output
- print a short summary

## Chunking Principle

Automation should be broken into human-sized chunks.

Each chunk should:

- have one clear purpose
- have explicit inputs and outputs
- produce a reviewable artifact
- be deterministic
- be testable without IDA when possible
- avoid mutating authoritative state by default

Mutation should come late, after dry-run/report/review steps.

## Trust Principle

A script is easier to trust when:

- its core logic lives in tested shared modules
- its output format is documented
- its failure modes are visible
- it has counterexample tests
- it does not combine discovery, policy, rendering, and mutation in one step

## Anti-Patterns

Avoid:

- one large script that does the whole workflow
- each script inventing its own record format
- policy decisions hidden inside report rendering
- mutation scripts that do their own discovery
- transient matcher outputs becoming durable truth
- creating final-looking artifacts before the process has produced them
- compatibility data being mistaken for approved data

## Preferred Workflow Shape

Use staged artifacts:

1. Inventory
2. Draft
3. Compare
4. Review
5. Approve
6. Plan
7. Apply
8. Verify
9. Cleanup

Each stage should be independently runnable and inspectable.

## Naming Guidance

Use names that reveal authority level:

- `*.legacy-evaluation.json`: old data kept only for comparison
- `*.draft.json`: generated candidate data
- `*.review.md`: human review workbook
- `*.decisions.json`: explicit human/policy decisions
- `*.json`: approved or stable process input, only after validation
- `*.report.json` / `*.report.md`: diagnostic output

Avoid names that make seed, draft, or legacy data look authoritative.

## WZD-To-MGC Sync Application

For WZD-to-MGC procedure-name sync:

- the ongoing sync should consume an approved procedure map
- the one-off process should create that map in chunks
- legacy matcher data can be used as an evaluation asset after the first fresh run
- old matcher scripts should stay only until replacement tooling exists and active dependencies are migrated

The first useful implementation step should be small:

- create shared models and JSON helpers
- build a procedure inventory command on top of them
- test normal and counterexample inputs

Do not start by building the full mapper.

## Working Rule For New Code

Before adding a new script, identify:

- what shared model it uses
- what shared logic it should call
- what single artifact it produces
- what human decision it supports
- what deterministic tests prove it

If those answers are unclear, stop and design the chunk before coding.

## Expected-Match Audit Rule

When the user provides a list of segment pairs that should match exactly, treat those pairs as ground-truth expected matches.

For that phase, do not rediscover candidate matches, do not run global all-vs-all matching, and do not use fuzzy matching as the primary task.

The job is to audit expected matches:

1. Pair procedures by approved segment pair and procedure ordinal.
2. Report which exactness checks pass or fail.
3. Use size, byte hash, normalized text, mnemonic sequence, and edit distance only as mismatch diagnostics.
4. Produce simple review artifacts, preferably TSV, before adding richer formats.

Short rule:

```text
We are not discovering matches.
We are auditing expected matches.
```
