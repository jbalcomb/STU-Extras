# Demo Automation PRD

## Summary

Build a hybrid automation system that can:

1. Launch and drive a repeatable game demo or test scenario.
2. Control OBS to record the run and optionally switch scenes.
3. Save the resulting video as a local artifact.
4. Upload the video to YouTube with metadata.
5. Reuse the same automation stack for functional and regression testing.

The system should use Robot Framework as the top-level orchestrator and Python for custom integrations with OBS, the game, and YouTube.

## Goals

- Provide a one-command way to run a scripted demo and capture video.
- Reuse the automation flow for regression and smoke testing.
- Keep the system reliable by preferring APIs over GUI click automation.
- Produce logs and artifacts that are easy to inspect after a run.
- Keep Windows support first-class.

## Non-Goals

- Full cloud CI video capture in phase 1.
- Image-based GUI automation as the primary control path.
- Automated public publishing workflow before private upload is stable.
- Replacing existing build tooling for the game.

## Users

- Project owner creating demo videos for progress updates.
- Developer running repeatable gameplay/regression checks.
- Future contributors who need a readable automation entry point.

## Primary Use Cases

### Use Case 1: Record local demo

Run an automated script that launches the game, drives a known demo sequence, records it in OBS, and stores the rendered file locally.

### Use Case 2: Record and upload nightly build video

Run a scheduled or manual workflow that records the demo and uploads the resulting video to YouTube as `private` or `unlisted`.

### Use Case 3: Reuse automation for tests

Run the same demo-driving logic in Robot Framework without OBS recording, to validate that the game boots, enters a scenario, and behaves as expected.

## Product Requirements

### Functional Requirements

- The system must support a local command that runs the full demo pipeline.
- The system must support a local command that runs only test automation without recording.
- The system must control OBS through `obs-websocket`.
- The system must support configurable OBS scene/profile names.
- The system must support configurable game launch command and working directory.
- The system must support configurable output video directory.
- The system must upload videos to YouTube through the YouTube Data API.
- The system must support title, description, tags, playlist, and privacy settings.
- The system must write machine-readable logs or metadata for each run.
- The system must preserve the recorded file path and, if uploaded, the returned YouTube video ID.

### Reliability Requirements

- Prefer API integration over GUI automation wherever possible.
- Use GUI automation only for gaps that cannot be handled by a stable API or game-side hook.
- Fail with actionable error messages if OBS, credentials, or output paths are misconfigured.
- Save partial artifacts even when upload fails.

### Testability Requirements

- Robot Framework test cases must be able to call Python keyword libraries.
- The demo-driving layer must be usable with or without OBS enabled.
- Smoke tests must be able to run without YouTube credentials.

## Architecture

### Recommended Stack

- Orchestration: Robot Framework
- Custom keyword libraries: Python
- OBS control: `obs-websocket`
- YouTube upload: Google YouTube Data API v3
- Windows desktop fallback: `rpaframework` Windows/Desktop libraries or `pywinauto`

### Why This Stack

- Robot Framework gives us readable end-to-end scenarios and reports.
- Python is the best place to implement custom OBS and YouTube integrations.
- OBS control is much more reliable through WebSocket than through UI clicking.
- YouTube upload is best handled with OAuth and the official API.
- Desktop automation stays available as a fallback instead of becoming the foundation.

## Phases

### Phase 1: Local recording MVP

- Create Robot Framework suite structure.
- Create Python OBS keyword library.
- Create Python game control keyword library.
- Launch game/demo, start OBS recording, run scenario, stop recording.
- Save local artifacts and logs.

### Phase 2: Test automation baseline

- Reuse the game control layer for smoke/regression tests.
- Add pass/fail assertions and Robot reports.
- Add a no-recording mode.

### Phase 3: YouTube upload

- Add OAuth credential loading.
- Upload recorded video.
- Persist returned video ID and URL metadata.
- Start with `private` uploads by default.

### Phase 4: Hardening

- Add retry behavior for transient OBS and upload failures.
- Add scheduled run support.
- Add richer metadata templates and release note generation.

## Proposed Repository Layout

```text
automation/
  robot/
    demo_publish.robot
    smoke.robot
    resources/
      common.resource
  python/
    obs_keywords.py
    youtube_keywords.py
    game_keywords.py
    config.py
  config/
    demo_automation.example.json
  artifacts/
    recordings/
    logs/
```

## Example Flows

### Flow A: Record demo only

1. Validate config and dependencies.
2. Launch or connect to OBS.
3. Launch game.
4. Drive demo/test sequence.
5. Start recording.
6. Wait for sequence completion.
7. Stop recording.
8. Save artifact metadata.

### Flow B: Record and upload

1. Complete Flow A.
2. Authenticate to YouTube.
3. Upload video with metadata.
4. Save returned video ID and status.

### Flow C: Regression smoke test

1. Launch game.
2. Drive scripted scenario.
3. Assert expected checkpoints.
4. Produce Robot report.

## Configuration

Configuration should live in a local file that is easy to override and should not contain committed secrets.

Suggested fields:

- `game.executable`
- `game.args`
- `game.working_dir`
- `obs.host`
- `obs.port`
- `obs.password`
- `obs.scene_name`
- `obs.recording_profile`
- `video.output_dir`
- `youtube.client_secret_file`
- `youtube.token_file`
- `youtube.privacy_status`
- `youtube.default_title_template`
- `youtube.default_description_template`

## Success Criteria

- A developer can run one command and get a recorded local demo video.
- A developer can run one command and get a Robot test report without recording.
- OBS start/stop recording works consistently on the primary dev machine.
- Upload succeeds to YouTube `private` using OAuth credentials.
- Logs clearly show where a run failed and what artifact was produced.

## Risks

- Game control may require some GUI automation if the game lacks a scriptable demo path.
- YouTube OAuth setup is more involved than local recording.
- Upload permissions may be limited by Google project verification status.
- OBS scene and source setup may drift if changed manually outside automation.

## Open Questions

- How will the game demo be driven: built-in demo mode, scripted input playback, or Windows UI automation?
- Should upload happen automatically on every successful run or only by explicit flag?
- Do we want only `private`/`unlisted`, or eventually scheduled/public publishing?
- Do we want OBS to launch automatically or connect only to an already-open instance?

## What You Need To Install Or Set Up

### Required for Phase 1

- Python 3.10+ on Windows.
- OBS Studio 28+.
- OBS `obs-websocket` enabled with a known password.
- A dedicated OBS scene/profile for demo recording.
- A stable way to launch the game from the command line.

### Required Python Packages

- `robotframework`
- `rpaframework`
- `google-api-python-client`
- `google-auth-oauthlib`
- `google-auth-httplib2`
- An OBS WebSocket Python client library to be chosen during implementation

### Optional, Only If We Need Browser Automation

- Node.js LTS
- `robotframework-browser`
- `rfbrowser init`

### Required for YouTube Upload

- Google Cloud project
- YouTube Data API v3 enabled
- OAuth client credentials downloaded as JSON
- YouTube channel/account to authorize

### Recommended Local Setup Tasks

- Decide the canonical demo scenario to automate first.
- Create an OBS scene named specifically for automated recording.
- Choose the local output directory for recordings.
- Create a Google Cloud project for uploads when ready.

## Implementation Recommendation

Start with Phase 1 and Phase 2 together:

- Robot Framework for orchestration
- Python keyword libraries for OBS and game control
- Local video artifact output only

Then add Phase 3 after the local recording path is stable.
