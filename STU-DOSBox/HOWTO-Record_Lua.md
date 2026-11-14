

What our training looks like — the honest picture
Your instinct to look was the right one, because it surfaced the real gap and we fixed it. Here's where things stand:

The good:

The action space now captures mouse clicks (the fix): Discrete(173) = noop + 12 keys + 160 grid cells. Your 921 clicks are now real actions, and their targets are meaningful — the top one (≈290,190, bottom-right, clicked 52×) is almost certainly the Next-Turn button.
The full training pipeline is built and verified: record → filter to gameplay → label (keys + grid clicks) → balance noop → train. There's a ready-to-run trainer in mom_bc.py, and the grid-click env loop is confirmed working.
The reality check:


3 sessions  →  58,787 gameplay frames
            →  but only ~1,470 real-action frames (97.5% is you watching)
            →  balanced to ~2,940 frames, across 132 distinct actions
That's the crux: ~1,500 action examples is too few to imitation-learn a 173-action policy. A model trained on this now would barely beat the "always noop" baseline. It's not a pipeline problem — it's a data-volume problem.

So "what training looks like" right now is: the machinery is ready, the data is too thin. Two concrete things stand between you and a model that plays:

More sessions. This is the big one. 10-20+ sessions would give you thousands of action examples. It's now painless — just run the recorder (auto-named files, fast). Play varied situations, and click/act deliberately (the watching frames are cheap; the actions are what's scarce).

pip install numpy torch — then python3 mom_bc.py trains the MLP and reports accuracy.
Right now it prints the dataset summary and the baseline to beat.
My recommendation: bank a batch of sessions next — say 10 — then we train and actually see a learning curve. When you've got them, point me at the folder and I'll run the training, tune the balancing, and we'll see whether the policy starts reproducing your clicks. The hard infrastructure is all done and proven; from here it's mostly feeding it data.








Human session recording + replay — complete ✅
You can now capture human play for behavioral cloning, and play traces back deterministically. Verified end-to-end in three parts (capture → replay → dataset).

What was added — input capture, the mirror of the injection you already had:

Engine taps DOSBox's real input choke points — keyboard.cpp:221 and ints/mouse.cpp:449 now call LUA_Capture* hooks (declared in the new include/dosbox_lua.h). Zero cost until armed.
New Lua API: dosbox.record_input(true) and dosbox.poll_input() → the key/mouse events since the last frame.
Deliverables in lua-examples/:

File	Role
record.lua	logs (observation + human input) per frame to a plain-text trace
replay.lua	re-injects a trace on matching frames — deterministic playback
session_dataset.py	parses traces → (obs, action) arrays for BC (configurable obs keys + action mapping)
record.conf / replay.conf	ready-to-run configs
The key insight that makes this robust: capture and injection share the same choke points, so (a) the self-test could simulate a human by injecting, and (b) a recorded trace round-trips perfectly — record → replay reproduces the exact input the guest saw. That replay path doubles as a fidelity test: run a human trace against ReMoM or the trained model and check they reproduce it.

How you actually record a real human session

DOSBOX_RECORD_FILE=~/mom_session1.txt \
  ./src/dosbox -conf lua-examples/record.conf   # (point [lua] script + mount MoM)
Play normally, quit DOSBox — the trace is saved (flushed every frame). Then:


python3 lua-examples/session_dataset.py ~/mom_session1.txt --obs-keys t,ax
gives you the BC dataset. For MoM specifically, you'd point record.lua's observation reads at real MoM globals (which we have offsets for) and map MoM's mouse-clicks/keys to your action set.

Where the project stands
You now have the complete ML toolchain inside DOSBox: read/write state, inject input, a lockstep Python bridge, a Gym env, and human demonstration recording/replay — enough to bootstrap a model with imitation learning and then train it with RL, plus a deterministic replay harness for your RE/fidelity work.

The two most valuable remaining pieces (both noted in memory):

Savestate-based reset() — the one gap for true multi-episode RL (snapshot/restore the machine so each episode starts identically).
Framebuffer/pixel observations — if you want the model to see the screen, not just RAM.
Want me to tackle savestate reset() next?

python3 session_dataset.py /tmp/mom_session1.txt



To make the next session trainable, I need two things from you

  1. Which WIZARDS.EXE globals should the model observe? (gold, mana, current turn, selected unit, etc.) — and their DGROUP offsets. I won't guess these; your ReMoM wiki / RE notes are authoritative.

  2. The in-game cursor position offset (WIZARDS.EXE) so clicks become (x, y) actions.

Give me those and I'll wire them into mom-record.lua, and add OG_FORCE_SEED to the record command so the session is replayable and re-derivable. Then a 10-minute session like this one becomes a genuinely usable dataset.
