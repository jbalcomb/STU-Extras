# Tasks: Butler itch.io Deployment

**Input**: Design documents from `/specs/002-butler-itchio-deploy/`
**Prerequisites**: plan.md (required), spec.md (required for user stories), research.md, data-model.md, quickstart.md

**Tests**: No tests requested — this feature is shell scripts validated by manual dry-run and actual deployment.

**Organization**: Tasks are grouped by user story to enable independent implementation and testing of each story.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3)
- Include exact file paths in descriptions

---

## Phase 1: Setup

**Purpose**: Create configuration template and example file

- [X] T001 Create example deployment configuration file at deploy.conf.example

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: No blocking prerequisites needed — deploy scripts are self-contained shell scripts with no shared infrastructure.

**Checkpoint**: Setup complete — user story implementation can begin.

---

## Phase 3: User Story 1 - One-Command Deployment (Priority: P1) MVP

**Goal**: Deploy a WASM build to itch.io with a single command (`bash deploy_itchio.sh`)

**Independent Test**: Run `bash deploy_itchio.sh` after a successful WASM build. Verify Butler pushes the build and itch.io project page is updated.

### Implementation for User Story 1

- [X] T002 [US1] Create deploy_itchio.sh with Butler detection (`command -v butler`) and clear install guidance at deploy_itchio.sh
- [X] T003 [US1] Add WASM build output validation (check RemomMapgenItchio.html, .js, .wasm exist; warn if unexpected files found) to deploy_itchio.sh
- [X] T004 [US1] Add `butler push` command with hardcoded default target and progress passthrough to deploy_itchio.sh
- [X] T005 [US1] Add success message with project URL and error handling for push failures (including auth-expired detection with `butler login` guidance) to deploy_itchio.sh

**Checkpoint**: User Story 1 complete — `bash deploy_itchio.sh` deploys a WASM build to itch.io with prerequisite checks and feedback.

---

## Phase 4: User Story 2 - Build and Deploy in One Step (Priority: P2)

**Goal**: Add `--build` flag that runs the WASM build before deploying

**Independent Test**: Run `bash deploy_itchio.sh --build` from a clean state. Verify WASM build is produced and then pushed to itch.io.

### Implementation for User Story 2

- [X] T006 [US2] Add `--build` flag parsing and `build_wasm.sh` invocation with exit-code check to deploy_itchio.sh
- [X] T007 [US2] Add build failure handling — display build errors and skip deployment on failure in deploy_itchio.sh

**Checkpoint**: User Story 2 complete — `bash deploy_itchio.sh --build` builds and deploys in one step.

---

## Phase 5: User Story 3 - Configurable Project Target (Priority: P3)

**Goal**: Load itch.io target settings from `deploy.conf` instead of hardcoded values

**Independent Test**: Create `deploy.conf` with a different itch.io project. Run deploy. Verify Butler pushes to the configured project.

### Implementation for User Story 3

- [X] T008 [US3] Add deploy.conf loading (source key=value file) with fallback defaults to deploy_itchio.sh
- [X] T009 [US3] Add validation for required config values (ITCHIO_USER, ITCHIO_GAME must be non-empty) to deploy_itchio.sh
- [X] T010 [US3] Replace hardcoded Butler target with `${ITCHIO_USER}/${ITCHIO_GAME}:${ITCHIO_CHANNEL}` in deploy_itchio.sh
- [X] T011 [US3] Add missing-config guidance — prompt developer to copy deploy.conf.example if deploy.conf not found in deploy_itchio.sh

**Checkpoint**: User Story 3 complete — deployment target is fully configurable via deploy.conf.

---

## Phase 6: Polish & Cross-Cutting Concerns

**Purpose**: Windows support and final validation

- [X] T012 [P] Create Windows batch wrapper at deploy_itchio.bat
- [X] T013 Validate quickstart.md instructions match actual script behavior

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies — can start immediately
- **Foundational (Phase 2)**: N/A — no blocking prerequisites
- **User Story 1 (Phase 3)**: Depends on Phase 1 (deploy.conf.example exists for reference)
- **User Story 2 (Phase 4)**: Depends on Phase 3 (deploy script must exist before adding --build flag)
- **User Story 3 (Phase 5)**: Depends on Phase 3 (deploy script must exist before adding config loading)
- **Polish (Phase 6)**: Depends on Phase 5 (all features in deploy_itchio.sh finalized before creating .bat wrapper)

### User Story Dependencies

- **User Story 1 (P1)**: Can start after Setup — no dependencies on other stories
- **User Story 2 (P2)**: Depends on US1 — adds `--build` flag to the existing deploy script
- **User Story 3 (P3)**: Depends on US1 — replaces hardcoded values with config loading in the existing deploy script

### Within Each User Story

- Tasks within each story are sequential (all modify the same file: deploy_itchio.sh)
- No parallel opportunities within stories due to single-file constraint

### Parallel Opportunities

- T012 (deploy_itchio.bat) can run in parallel with Phase 5 tasks since it's a separate file
- US2 and US3 could theoretically run in parallel after US1, but since both modify deploy_itchio.sh, sequential execution is safer

---

## Parallel Example: Phase 6

```bash
# These can run in parallel (different files):
Task: "Create Windows batch wrapper at deploy_itchio.bat"         # T012
Task: "Validate quickstart.md instructions match actual behavior"  # T013
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 1: Create deploy.conf.example
2. Complete Phase 3: Implement deploy_itchio.sh with Butler detection, build validation, push, and feedback
3. **STOP and VALIDATE**: Test deployment with a real WASM build and itch.io project
4. Deploy is functional with hardcoded target

### Incremental Delivery

1. Phase 1 → Configuration template ready
2. Phase 3 (US1) → Basic deploy works → Test with real deployment (MVP!)
3. Phase 4 (US2) → Combined build-and-deploy works → Test with --build flag
4. Phase 5 (US3) → Configurable target works → Test with custom deploy.conf
5. Phase 6 → Windows support and documentation validation
6. Each story adds value without breaking previous stories

---

## Notes

- All user story tasks modify `deploy_itchio.sh` — must be executed sequentially within and across US1→US2→US3
- [P] tasks = different files, no dependencies
- [Story] label maps task to specific user story for traceability
- Butler must be installed at C:\butler or on PATH for testing
- Commit after each phase completion
- Stop at any checkpoint to validate story independently
