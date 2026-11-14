#!/usr/bin/env python3
"""mom_gym.py -- a Master of Magic Gymnasium environment.

Wraps DosBoxEnv (dosbox_gym.py) with MoM-specific pieces:
  * observation  = the real WIZARDS.EXE game state (turn/units/cities/gold/mana/
                   selected-unit/cursor) + a one-hot of the current screen,
  * action space = the overland unit-play controls a human actually uses
                   (8-way keypad movement + next-unit + turn control), and
  * reward/done  = COMPUTED from observation deltas (MoM has no reward field);
                   defaults are placeholders -- swap in your task's reward.

Runtime pairing: launch DOSBox with mom-gym.conf (mom-gym-env.lua). The Lua env
snapshots the episode-start machine state (RAM+CPU) the first time it reaches a
stable Overland screen, then serves a synchronous lockstep loop; env.reset()
sends "reset" and the Lua side load_state()s back to that snapshot -> every
episode starts from identical conditions (true multi-episode RL).

Smoke test the Python half against the NOWAIT test env (see mom-gym-env.lua):
    python3 mom_gym.py --episodes 2
"""

import argparse

from dosbox_gym import DosBoxEnv, DEFAULT_SCREENS, MOM_OBS_KEYS, _num

# ---- Action space: keyboard + grid-discretized mouse clicks ----
# MoM is mouse-driven, so a click is a first-class action. We discretize the
# 320x200 screen into a GRID_W x GRID_H grid; a "click cell (i,j)" action clicks
# that cell's center. Combined with the keyboard actions and noop this is one
# Discrete space the policy outputs and the Lua env injects (mouse_set + click).
# Action strings: "noop", a DOSBox key name (tapped), or "click:xf,yf" (fractions).
KEY_ACTIONS = ("kp7", "kp8", "kp9", "kp4", "kp6", "kp1", "kp2", "kp3",
               "n", "space", "enter", "esc")
SCREEN_W, SCREEN_H = 320, 200
GRID_W, GRID_H = 16, 10                       # 160 click cells (~20x20 px each)


def _grid_click_action(i, j):
    """Action string clicking the CENTER of grid cell (i, j)."""
    return "click:%.4f,%.4f" % ((i + 0.5) / GRID_W, (j + 0.5) / GRID_H)


MOM_ACTIONS = (("noop",) + KEY_ACTIONS +
               tuple(_grid_click_action(i, j)
                     for j in range(GRID_H) for i in range(GRID_W)))
# indices: 0 = noop ; 1..len(KEY_ACTIONS) = keys ; then GRID_W*GRID_H grid clicks

_KEY_INDEX = {name: 1 + k for k, name in enumerate(KEY_ACTIONS)}
_GRID_BASE = 1 + len(KEY_ACTIONS)


def _cell_of(mx, my):
    i = min(GRID_W - 1, max(0, int(mx * GRID_W / SCREEN_W)))
    j = min(GRID_H - 1, max(0, int(my * GRID_H / SCREEN_H)))
    return i, j


def mom_action_map(rec):
    """Recorded frame -> discrete action index, for behavioral cloning.

    A mouse click (spatial) takes priority and maps to the grid cell under the
    cursor at that frame; else a recognized key-down; else noop (0). Pass to
    session_dataset.build_dataset(action_map=mom_action_map)."""
    f = rec["fields"]
    for etype, a, pressed in rec["events"]:
        if etype == "mouse_button" and pressed:
            i, j = _cell_of(_num(f.get("mouse_x", 0)), _num(f.get("mouse_y", 0)))
            return _GRID_BASE + j * GRID_W + i
    for etype, a, pressed in rec["events"]:
        if etype == "key" and pressed and a in _KEY_INDEX:
            return _KEY_INDEX[a]
    return 0


def mom_reward(prev, cur):
    """Placeholder reward from observation deltas. TUNE FOR YOUR TASK.

    Rewards empire growth (cities) and army size (units); the first step after a
    reset (prev == {}) is neutral. `prev`/`cur` are dicts of string fields.
    """
    if not prev:
        return 0.0
    d_cities = _num(cur.get("cities", 0)) - _num(prev.get("cities", 0))
    d_units = _num(cur.get("units", 0)) - _num(prev.get("units", 0))
    return 1.0 * d_cities + 0.1 * d_units


def mom_done(cur):
    """Placeholder termination: the human is defeated when they hold no cities.

    (At game start cities >= 1. Combine with max_episode_steps for a turn cap.)
    """
    return _num(cur.get("cities", 1)) <= 0


def make_mom_env(host="127.0.0.1", port=8642,
                 obs_keys=MOM_OBS_KEYS,
                 actions=MOM_ACTIONS,
                 screens=DEFAULT_SCREENS,
                 reward_fn=mom_reward,
                 done_fn=mom_done,
                 max_episode_steps=500,
                 connect_timeout=300.0):
    """Build a MoM DosBoxEnv. Observation = numeric obs_keys + one-hot(screen);
    reward/termination are computed from observation deltas."""
    return DosBoxEnv(
        host=host, port=port,
        obs_keys=obs_keys, actions=actions, screens=screens,
        reward_key=None, done_key=None,        # computed, not read from the wire
        reward_fn=reward_fn, done_fn=done_fn,
        max_episode_steps=max_episode_steps,
        connect_timeout=connect_timeout,
        reset_action="reset", quit_action="quit")


def _fmt(obs):
    try:
        return "[" + ",".join("%.0f" % v for v in obs) + "]"
    except TypeError:
        return str(obs)


def demo(host, port, episodes, seed):
    import random
    random.seed(seed)
    env = make_mom_env(host=host, port=port)
    print("[mom] action_space=%r" % (env.action_space,), flush=True)
    print("[mom] observation_space=%r  (%d obs_keys + %d screens + other)"
          % (env.observation_space, len(env.obs_keys), len(env.screens)), flush=True)
    print("[mom] actions=%s" % (list(env.actions),), flush=True)
    try:
        for ep in range(episodes):
            obs, info = env.reset(seed=seed + ep)
            print("[mom] ep %d reset -> screen=%s obs=%s"
                  % (ep, info.get("screen", "?"), _fmt(obs)), flush=True)
            ret, steps, term, trunc = 0.0, 0, False, False
            while not (term or trunc):
                a = env.action_space.sample()
                obs, r, term, trunc, info = env.step(a)
                ret += r
                steps += 1
                if steps <= 6 or term:
                    print("[mom]   step %3d act=%-5s screen=%-9s obs=%s r=%.2f term=%s"
                          % (steps, env.actions[a], info.get("screen", "?"),
                             _fmt(obs), r, term), flush=True)
            print("[mom] ep %d done: steps=%d return=%.2f" % (ep, steps, ret), flush=True)
    finally:
        env.close()
    print("[mom] closed", flush=True)


if __name__ == "__main__":
    ap = argparse.ArgumentParser(description="Master of Magic Gym env demo / smoke test")
    ap.add_argument("--host", default="127.0.0.1")
    ap.add_argument("--port", type=int, default=8642)
    ap.add_argument("--episodes", type=int, default=2)
    ap.add_argument("--seed", type=int, default=0)
    args = ap.parse_args()
    demo(args.host, args.port, args.episodes, args.seed)
