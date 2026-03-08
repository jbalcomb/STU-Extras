<!--
Sync Impact Report
===================
Version change: (none) → 1.0.0
Modified principles: N/A (initial constitution)
Added sections:
  - Core Principles (5 principles)
  - Technology Constraints
  - Development Workflow
  - Governance
Removed sections: N/A
Templates requiring updates:
  - .specify/templates/plan-template.md — ✅ no update needed
    (Constitution Check section is generic; gates derived at plan time)
  - .specify/templates/spec-template.md — ✅ no update needed
    (Requirements section is generic; FR items derived at spec time)
  - .specify/templates/tasks-template.md — ✅ no update needed
    (Phase structure is generic; task categories derived at task time)
  - .specify/templates/commands/*.md — ✅ N/A (no command files exist)
Follow-up TODOs: none
-->

# RemomMapgenItchio Constitution

## Core Principles

### I. Binary Fidelity (NON-NEGOTIABLE)

The exported .GAM file MUST load and play correctly in Master of Magic
via ReMoM. This is the product's reason for existing. Every struct MUST
use `#pragma pack(push, 2)` for 8086-compatible alignment. Every struct
size MUST be verified with `static_assert` at compile time. Field
offsets, array sizes, and total file size (123,300 bytes) MUST match
the ReMoM reference exactly. A single byte of misalignment silently
corrupts saves.

**Rationale**: The .GAM format has no headers, versioning, or length
prefixes — just raw bytes at hard-coded offsets. There is no recovery
from layout errors.

### II. Layer Separation

The codebase MUST maintain three distinct layers:
- **Data Layer** (`MomData` library): game structs, world grid,
  serialization. Zero SDL2 dependency. Independently testable.
- **Editor Layer**: tool state, undo/redo, cursor tracking.
  No SDL2 dependency.
- **Presentation Layer**: SDL2 window, rendering, input handling.
  Depends on Editor and Data layers only.

Each layer MUST depend only downward. Presentation MUST NOT leak into
Data or Editor. New features MUST be placed in the correct layer.

**Rationale**: The Data layer must compile to WASM for itch.io web
distribution. SDL2 cannot run in Emscripten without the Presentation
layer being the sole consumer. Testability requires headless execution.

### III. Test-Verified Compatibility

All data format changes MUST include corresponding test updates:
- Struct size assertions for every packed struct
- .GAM binary round-trip tests (write then read, compare)
- .mom_scenario JSON round-trip tests (serialize then deserialize,
  compare)

Tests MUST pass before any data layer change is considered complete.
Round-trip tests are the primary defense against silent alignment bugs
that code review alone cannot catch.

**Rationale**: Binary compatibility bugs are silent, catastrophic, and
undetectable without automated verification.

### IV. WASM-Ready Data Layer

The `MomData` static library MUST compile without platform-specific
dependencies. No Win32 APIs, no SDL2, no filesystem assumptions beyond
standard C++17. All I/O MUST go through `std::istream`/`std::ostream`
abstractions so Emscripten's virtual filesystem works transparently.

**Rationale**: The itch.io web build requires compiling the data layer
to WebAssembly via Emscripten. Platform-specific code in the data layer
blocks this critical distribution channel.

### V. Simplicity First

Start with the simplest working implementation. Use colored blocks
before text rendering, hardcoded paths before file dialogs, base
terrain indices before auto-tiling. Add complexity only when a specific
user need is validated. Avoid abstractions for hypothetical future
requirements.

**Rationale**: The project serves a niche community. Shipping a working
tool quickly is more valuable than a polished tool that ships late.
Premature complexity adds maintenance burden without proven benefit.

## Technology Constraints

- **Language**: C++17 (`CMAKE_CXX_STANDARD 17`)
- **Build system**: CMake 4.2+, Ninja generator preferred
- **Struct packing**: `#pragma pack(push, 2)` for all game data structs
- **Dependencies**: SDL2 2.32.2 (presentation only), nlohmann/json
  3.11.3 (JSON serialization), GoogleTest 1.17.0 (testing)
- **Target platforms**: Windows (primary), Emscripten/WASM (future)
- **Binary format**: .GAM files MUST be exactly 123,300 bytes
- **Reference source**: ReMoM project (`c:\STU\devel\STU-Extras\ReMoM`)
  is the authoritative source for all struct definitions and file layout

## Development Workflow

- **Build verification**: `cmake -B build -G Ninja` then
  `cmake --build build` MUST succeed with zero warnings in data layer
  code before any commit
- **Test verification**: `ctest --output-on-failure` in the build
  directory MUST pass all tests before any commit
- **Manual verification**: For any .GAM export changes, load the
  exported file in ReMoM and verify terrain, cities, units, and nodes
  appear as placed in the editor
- **Undo coverage**: Every user-facing mutation MUST be wrapped in an
  undo command. No direct state modification from input handlers.
- **Code style**: Four spaces for indentation. Function comments end
  with "Powered by Claude."

## Governance

This constitution is the authoritative guide for architectural and
quality decisions in RemomMapgenItchio. When a proposed change conflicts
with a principle above, the principle wins unless the constitution is
formally amended first.

**Amendment procedure**:
1. Describe the proposed change and rationale
2. Identify which principles are affected
3. Update this document with the new version number
4. Update any dependent templates if principle-driven sections change

**Versioning policy**: MAJOR.MINOR.PATCH semantic versioning.
- MAJOR: Principle removal or incompatible redefinition
- MINOR: New principle added or existing principle materially expanded
- PATCH: Clarifications, wording, or non-semantic refinements

**Compliance review**: All data layer changes MUST demonstrate
compliance with Principles I (Binary Fidelity) and III (Test-Verified
Compatibility) via passing tests. Architecture changes MUST demonstrate
compliance with Principle II (Layer Separation).

**Version**: 1.0.0 | **Ratified**: 2026-03-08 | **Last Amended**: 2026-03-08
