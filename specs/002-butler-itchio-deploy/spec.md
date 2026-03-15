# Feature Specification: Butler itch.io Deployment

**Feature Branch**: `002-butler-itchio-deploy`
**Created**: 2026-03-08
**Status**: Draft
**Input**: User description: "Add a feature for using the Butler tool for itch.io"

## User Scenarios & Testing

### User Story 1 - One-Command Deployment to itch.io (Priority: P1)

A developer building the MoM Scenario Editor wants to deploy the latest WASM build to itch.io with a single command. After running the WASM build, the developer executes a deploy command that packages the build output and pushes it to the correct itch.io project page using Butler. The developer sees progress feedback and a confirmation that the upload succeeded, including a link to the published page.

**Why this priority**: This is the core value of the feature. Without one-command deployment, the developer must manually zip files and upload through the itch.io web dashboard, which is error-prone and slow.

**Independent Test**: Run the deploy command after a successful WASM build. Verify that the itch.io project page is updated with the new build and is playable in the browser.

**Acceptance Scenarios**:

1. **Given** a successful WASM build exists in the build output directory, **When** the developer runs the deploy command, **Then** Butler pushes the build to the configured itch.io project and channel, and the developer sees a success message with the project URL.
2. **Given** Butler is not installed or not on PATH, **When** the developer runs the deploy command, **Then** the system displays a clear error message explaining how to install Butler.
3. **Given** no WASM build output exists, **When** the developer runs the deploy command, **Then** the system displays an error message indicating the build must be run first.

---

### User Story 2 - Build and Deploy in One Step (Priority: P2)

A developer wants to build the WASM target and immediately deploy it to itch.io without running two separate commands. A combined build-and-deploy command compiles the project, verifies the build succeeded, and then pushes the result to itch.io.

**Why this priority**: Reduces the deployment workflow from two steps to one, preventing the common mistake of deploying a stale build.

**Independent Test**: Run the combined build-and-deploy command from a clean state. Verify that the WASM build is produced and the itch.io project page is updated.

**Acceptance Scenarios**:

1. **Given** the project source is ready to build, **When** the developer runs the build-and-deploy command, **Then** the system builds the WASM target, and on success, pushes the output to itch.io.
2. **Given** the WASM build fails (compilation errors), **When** the developer runs the build-and-deploy command, **Then** the system reports the build errors and does NOT attempt to push to itch.io.

---

### User Story 3 - Configure itch.io Project Target (Priority: P3)

A developer wants to configure which itch.io project and channel the builds are pushed to, so that different developers or forks can deploy to their own itch.io pages. The configuration is stored in a project file that can be version-controlled or kept local.

**Why this priority**: Enables multiple team members or forks to use the deployment feature without hardcoding a single itch.io target.

**Independent Test**: Change the project target configuration to a different itch.io project. Run the deploy command. Verify the build is pushed to the newly configured project, not the previous one.

**Acceptance Scenarios**:

1. **Given** no deployment configuration exists, **When** the developer runs the deploy command for the first time, **Then** the system prompts for or documents the required configuration values (itch.io username, project name, channel name).
2. **Given** a valid deployment configuration exists, **When** the developer runs the deploy command, **Then** the system uses the configured values without prompting.
3. **Given** the deployment configuration has an invalid project target, **When** the developer runs the deploy command, **Then** Butler reports the error and the system displays it clearly.

---

### Edge Cases

- What happens when Butler authentication has expired or the API key is not set? The system should display a clear message directing the developer to run `butler login`.
- What happens when the itch.io project does not exist yet? Butler will report an error; the system should surface this with guidance to create the project on itch.io first.
- What happens when the network is unavailable during upload? Butler handles retries internally; the system should pass through Butler's error output.
- What happens when the WASM build output directory contains extra files (e.g., test binaries)? The deploy should only package the necessary files (.html, .js, .wasm, and any asset files).

## Requirements

### Functional Requirements

- **FR-001**: System MUST provide a deploy command that pushes the WASM build output to itch.io using Butler.
- **FR-002**: System MUST verify that Butler is installed and accessible before attempting deployment.
- **FR-003**: System MUST verify that the WASM build output exists and contains the required files (.html, .js, .wasm) before attempting deployment.
- **FR-004**: System MUST display Butler's progress output (upload percentage, patching status) to the developer during deployment.
- **FR-005**: System MUST display a success message with the itch.io project URL upon successful deployment.
- **FR-006**: System MUST display clear, actionable error messages when deployment fails (missing Butler, missing build, authentication failure, network error).
- **FR-007**: System MUST provide a combined build-and-deploy command that builds the WASM target first and only deploys on successful build.
- **FR-008**: System MUST support configurable itch.io project target (username/project-name) and channel name.
- **FR-009**: System MUST only package deployment-relevant files (the .html, .js, .wasm output and any bundled assets), excluding test binaries, build intermediates, and source files.
- **FR-010**: System MUST use Butler's `push` command with a channel name appropriate for the platform (e.g., "html5" for web builds).

### Key Entities

- **Deployment Configuration**: The target itch.io project identifier (username/project-name), channel name, and build output directory path.
- **Build Output**: The set of files produced by the WASM build that constitute the deployable application (.html, .js, .wasm, asset files).
- **Deployment Result**: The outcome of a Butler push operation including success/failure status, version number assigned by itch.io, and the project URL.

## Success Criteria

### Measurable Outcomes

- **SC-001**: A developer can deploy a WASM build to itch.io in under 2 minutes (excluding build time), compared to the current manual process of zipping and uploading via the web dashboard.
- **SC-002**: The deploy command succeeds on first attempt in 95% of cases when prerequisites are met (Butler installed, authenticated, build exists).
- **SC-003**: When prerequisites are not met, 100% of failure cases produce an error message that identifies the specific missing prerequisite and how to resolve it.
- **SC-004**: The combined build-and-deploy command reduces the full publish workflow to a single command invocation.

## Assumptions

- Butler is installed separately by the developer (this feature does not auto-install Butler).
- The developer has already created an itch.io account and project page.
- The developer has already authenticated Butler via `butler login` (API key is cached locally by Butler).
- The WASM build is performed using the existing Emscripten build configuration in the project.
- The default channel name for web/HTML5 builds is "html5", following itch.io conventions.
- Deployment configuration can be stored in a simple project-level file (e.g., a config file in the repository root or a section in an existing config file).

## Scope Boundaries

### In Scope

- Deploy script/command that wraps Butler's `push` command
- Combined build-and-deploy workflow
- Prerequisite validation (Butler installed, build exists, auth status)
- Configurable itch.io project target
- Progress and error feedback

### Out of Scope

- Automatic installation of Butler
- itch.io project creation (must be done manually on itch.io)
- Butler authentication flow (developer runs `butler login` separately)
- Non-WASM deployment targets (desktop builds are not deployed to itch.io)
- CI/CD pipeline integration (this is a local developer tool; CI integration is a future feature)
- Version tagging or release notes on itch.io (Butler assigns versions automatically)
