#!/usr/bin/env python3
"""session_dataset.py -- load recorded DOSBox human sessions into (obs, action)
pairs for behavioral cloning / imitation learning.

Reads the plain-text session format written by record.lua:
    # dosbox-session v1
    F=<frame> t=<ticks> ax=<ax> mdx=<f> mdy=<f> ev=k:space:1,mb:0:1,...

You supply:
  obs_keys    -- which numeric fields form the observation vector (match your env)
  action_map  -- function(frame_record) -> discrete action index
The default action_map mirrors the gym demo: a "space" key-down maps to
"tap:space" (action 1), otherwise "noop" (action 0). Swap it for your task's
key/mouse -> action-index mapping.

Returns numpy arrays if numpy is installed, else plain Python lists -- so it
works as a zero-dependency smoke test and as a real dataset builder.
"""

import argparse


def _num(x):
    try:
        return int(x, 0)
    except (TypeError, ValueError):
        pass
    try:
        return float(x)
    except (TypeError, ValueError):
        return 0


def parse_session(path):
    """Yield per-frame records: {frame, fields{k:num}, events[...], mdx, mdy}."""
    with open(path) as fh:
        for line in fh:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            rec = {"frame": 0, "fields": {}, "events": [], "mdx": 0.0, "mdy": 0.0}
            evtext = ""
            for tok in line.split():
                if "=" not in tok:
                    continue
                k, v = tok.split("=", 1)
                if k == "ev":
                    evtext = v
                elif k == "F":
                    rec["frame"] = int(v)
                elif k in ("mdx", "mdy"):
                    rec[k] = float(v)
                elif k in ("screen", "exe"):
                    rec["fields"][k] = v          # string labels, not numbers
                else:
                    rec["fields"][k] = _num(v)
            if evtext and evtext != "-":
                for e in evtext.split(","):
                    parts = e.split(":")
                    if len(parts) == 3 and parts[0] == "k":
                        rec["events"].append(("key", parts[1], parts[2] == "1"))
                    elif len(parts) == 3 and parts[0] == "mb":
                        rec["events"].append(("mouse_button", int(parts[1]), parts[2] == "1"))
            yield rec


def default_action_map(rec):
    """space key-down -> action 1 (tap:space), else 0 (noop)."""
    for etype, a, pressed in rec["events"]:
        if etype == "key" and a == "space" and pressed:
            return 1
    return 0


# Screens where the WIZARDS.EXE game-state globals are valid. Non-gameplay
# screens (HallOfFame end screen, the main menu, "?") leave turn/units/cities as
# stale/garbage memory, so exclude them from training data.
GAMEPLAY_SCREENS = ("Overland", "City", "EnemyCity", "Combat", "Production",
                    "Cities", "Armies", "Magic", "Items", "Spellbook")


def build_dataset(path, obs_keys=("t", "ax"), action_map=default_action_map,
                  keep_screens=GAMEPLAY_SCREENS):
    """Build (obs, action) arrays. If keep_screens is set, only frames whose
    `screen` field is in it are kept (drops end/menu screens with invalid state).
    Pass keep_screens=None to keep every frame."""
    keep = set(keep_screens) if keep_screens else None
    X, Y, kept, dropped = [], [], 0, 0
    for rec in parse_session(path):
        if keep is not None and rec["fields"].get("screen") not in keep:
            dropped += 1
            continue
        X.append([float(rec["fields"].get(k, 0)) for k in obs_keys])
        Y.append(action_map(rec))
        kept += 1
    if keep is not None and dropped:
        print("[dataset] kept %d gameplay frames, dropped %d non-gameplay frames"
              % (kept, dropped))
    try:
        import numpy as np
        return np.asarray(X, dtype="float32"), np.asarray(Y, dtype="int64")
    except ImportError:
        return X, Y


if __name__ == "__main__":
    ap = argparse.ArgumentParser(description="Load a recorded DOSBox session -> (obs, action)")
    ap.add_argument("path")
    ap.add_argument("--obs-keys", default="t,ax",
                    help="comma-separated obs field names (default: t,ax)")
    ap.add_argument("--screens", default=None,
                    help="comma-separated screens to keep (default: gameplay screens; "
                         "pass 'all' to keep every frame)")
    ap.add_argument("--head", type=int, default=10, help="print first N rows")
    args = ap.parse_args()

    obs_keys = tuple(k for k in args.obs_keys.split(",") if k)
    if args.screens is None:
        keep = GAMEPLAY_SCREENS
    elif args.screens.lower() == "all":
        keep = None
    else:
        keep = tuple(s for s in args.screens.split(",") if s)
    X, Y = build_dataset(args.path, obs_keys=obs_keys, keep_screens=keep)

    n = len(Y)
    try:                       # numpy path
        shape = X.shape
    except AttributeError:     # list path
        shape = "(%d, %d)" % (n, len(obs_keys))
    print("[dataset] %d transitions  obs_keys=%s  X.shape=%s" % (n, obs_keys, shape))
    counts = {}
    for a in (list(Y) if hasattr(Y, "__iter__") else Y):
        counts[int(a)] = counts.get(int(a), 0) + 1
    print("[dataset] action counts: %s" % counts)
    for i in range(min(args.head, n)):
        xi = X[i]
        xi = list(xi) if hasattr(xi, "__iter__") else xi
        print("  frame %2d  obs=%s  action=%d" % (i + 1, [float(v) for v in xi], int(Y[i])))
