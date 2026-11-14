#!/usr/bin/env python3
"""dosbox_gym.py -- a Gymnasium environment wrapping the DOSBox Lua bridge.

If `gymnasium` (+ `numpy`) are installed, DosBoxEnv IS a real gymnasium.Env with
gymnasium spaces and numpy observations -- drop it straight into any RL library
(SB3, CleanRL, ...). If they are NOT installed, it transparently falls back to a
tiny built-in Env/spaces shim so you can still import, roll out, and smoke-test
it with zero third-party dependencies. Same file either way.

The env is game-agnostic. Your Lua env script (see gym-env.lua) decides what
goes in each newline-delimited observation line; you tell DosBoxEnv:
  * obs_keys   -- which numeric fields become the Box observation vector
  * actions    -- the discrete action set; Gym action i sends actions[i]
  * reward_key -- obs field carrying the step reward   (optional)
  * done_key   -- obs field carrying episode-done (0/1) (optional)

DOSBox is the TCP server; this connects as the client via DosBoxBridge, one
observation<->action exchange per emulated frame (synchronous lockstep).

Reset note: with no savestate support yet, reset() unblocks the Lua env by
sending `reset_action` (default "reset"); the Lua side is responsible for
restoring initial state on that action (for now our demo resets a step
counter; a real game would reload a savestate -- a future milestone).

Smoke test (start it any time; it retries until DOSBox is listening):
    python3 dosbox_gym.py --episodes 2
"""

import random

from dosbox_bridge import DosBoxBridge, parse_obs

try:
    import numpy as np
    _HAVE_NP = True
except ImportError:
    np = None
    _HAVE_NP = False

try:
    import gymnasium as gym
    from gymnasium import spaces
    _HAVE_GYM = True
except ImportError:
    _HAVE_GYM = False

    # ---- minimal dependency-free fallback for gym.Env + spaces ----
    class _Space(object):
        def sample(self):
            raise NotImplementedError

        def contains(self, x):
            return True

    class _Discrete(_Space):
        def __init__(self, n):
            self.n = int(n)

        def sample(self):
            return random.randrange(self.n)

        def contains(self, x):
            try:
                return 0 <= int(x) < self.n
            except (TypeError, ValueError):
                return False

        def __repr__(self):
            return "Discrete(%d)" % self.n

    class _Box(_Space):
        def __init__(self, low, high, shape, dtype=float):
            self.low, self.high = low, high
            self.shape, self.dtype = tuple(shape), dtype

        def sample(self):
            return tuple(random.uniform(-1.0, 1.0) for _ in range(self.shape[0]))

        def contains(self, x):
            try:
                return len(x) == self.shape[0]
            except TypeError:
                return False

        def __repr__(self):
            return "Box(shape=%r)" % (self.shape,)

    class spaces(object):          # namespace shim
        Discrete = _Discrete
        Box = _Box

    class gym(object):             # namespace shim
        class Env(object):
            metadata = {"render_modes": []}

            def reset(self, *, seed=None, options=None):
                raise NotImplementedError

            def step(self, action):
                raise NotImplementedError

            def render(self):
                return None

            def close(self):
                pass


def _num(x):
    """Parse an obs field to a number: decimal, 0x-hex, or float; else 0."""
    if isinstance(x, (int, float)):
        return x
    try:
        return int(x, 0)
    except (TypeError, ValueError):
        pass
    try:
        return float(x)
    except (TypeError, ValueError):
        return 0


# Screen ids latched by dosbox.current_screen() -- mirrors OG_SCREEN_FUNCS in
# src/misc/og_sort_trace.h. -1 == unknown ("?"). Screen is CATEGORICAL, so it is
# one-hot encoded into the observation (a raw id would imply a false ordering:
# screen 200 is not "40x" screen 5). DEFAULT_SCREENS fixes the one-hot order; any
# id not listed (incl. -1 by default position, and the >=200 extras) falls into a
# trailing "other" bucket.
SCREEN_IDS = {
    "Unknown": -1, "Overland": 5, "City": 0, "Magic": 6, "Production": 8,
    "Items": 9, "Cities": 3, "Armies": 2, "RoadBuild": 7, "Advisor": 14,
    "Diplomacy": 15, "Spellbook": 12,
}
DEFAULT_SCREENS = (-1, 5, 0, 6, 8, 9, 3, 2, 7, 14, 15, 12)
_SCREEN_LABEL = {v: k for k, v in SCREEN_IDS.items()}

# Real MoM (WIZARDS.EXE) observation keys -- match mom-record.lua's OBS_WZD field
# names. Use as obs_keys for a MoM policy, together with screens=DEFAULT_SCREENS:
#   DosBoxEnv(obs_keys=MOM_OBS_KEYS, screens=DEFAULT_SCREENS, ...)
# The dynamic, decision-relevant scalars + resources + cursor. The per-game
# constants (difficulty/magic/landsize/num_players) are also in the trace if a
# policy wants them, but are omitted here since they don't change within a game.
MOM_OBS_KEYS = ("turn", "units", "cities", "gold", "mana", "sel_unit",
                "mouse_x", "mouse_y")


class DosBoxEnv(gym.Env):
    """Gym(nasium) env over the DOSBox Lua bridge."""

    metadata = {"render_modes": []}

    def __init__(self, host="127.0.0.1", port=8642,
                 obs_keys=("t", "ax"),
                 actions=("noop", "tap:space"),
                 reward_key="reward", done_key="done",
                 obs_low=-1e9, obs_high=1e9,
                 connect_timeout=30.0,
                 max_episode_steps=None,
                 reset_action="reset",
                 quit_action="quit",
                 screen_key="screen", screens=None,
                 reward_fn=None, done_fn=None):
        self.host, self.port = host, port
        self.obs_keys = list(obs_keys)
        self.actions = list(actions)
        self.reward_key = reward_key
        self.done_key = done_key
        # Computed reward/termination (preferred over reward_key/done_key when set).
        # reward_fn(prev_fields, fields)->float; done_fn(fields)->bool. Used for MoM,
        # where reward is derived from observation deltas (no in-game reward field).
        self.reward_fn = reward_fn
        self.done_fn = done_fn
        self._prev_fields = {}
        self.connect_timeout = connect_timeout
        self.max_episode_steps = max_episode_steps
        self.reset_action = reset_action
        self.quit_action = quit_action
        # Categorical screen -> one-hot. `screens` = ordered list of ids to encode
        # (None disables it). A trailing "other" slot catches any unlisted id.
        self.screen_key = screen_key
        self.screens = [int(s) for s in screens] if screens else []
        self._screen_index = {sid: i for i, sid in enumerate(self.screens)}
        self._n_screen = (len(self.screens) + 1) if self.screens else 0
        self.bridge = None
        self._steps = 0
        self.last_info = {}

        self.action_space = spaces.Discrete(len(self.actions))
        obs_dim = len(self.obs_keys) + self._n_screen
        if _HAVE_NP:
            self.observation_space = spaces.Box(
                low=obs_low, high=obs_high, shape=(obs_dim,), dtype=np.float32)
        else:
            self.observation_space = spaces.Box(
                low=obs_low, high=obs_high, shape=(obs_dim,))

    # ---- helpers ----
    def _make_obs(self, fields):
        vals = [float(_num(fields.get(k, 0))) for k in self.obs_keys]
        if self.screens:
            onehot = [0.0] * self._n_screen
            sid = int(_num(fields.get(self.screen_key, -1)))
            onehot[self._screen_index.get(sid, len(self.screens))] = 1.0  # else "other"
            vals.extend(onehot)
        if _HAVE_NP:
            return np.asarray(vals, dtype=np.float32)
        return tuple(vals)

    def _decode(self, line):
        fields = parse_obs(line)
        obs = self._make_obs(fields)
        if self.reward_fn is not None:
            reward = float(self.reward_fn(self._prev_fields, fields))
        elif self.reward_key:
            reward = float(_num(fields.get(self.reward_key, 0)))
        else:
            reward = 0.0
        if self.done_fn is not None:
            done = bool(self.done_fn(fields))
        elif self.done_key:
            done = bool(int(_num(fields.get(self.done_key, 0))))
        else:
            done = False
        self._prev_fields = fields
        return obs, reward, done, fields

    def _connect(self):
        if self.bridge is None:
            self.bridge = DosBoxBridge(self.host, self.port, self.connect_timeout)

    # ---- gym API ----
    def reset(self, *, seed=None, options=None):
        if _HAVE_GYM:
            super().reset(seed=seed)
        if seed is not None:
            random.seed(seed)
        fresh = (self.bridge is None)
        self._connect()
        self._steps = 0
        self._prev_fields = {}          # first reward after reset is neutral
        if (not fresh) and self.reset_action:
            # a mid-run reset: unblock the Lua env (waiting for an action) and
            # let it restore initial state before it emits the next observation.
            self.bridge.send_action(self.reset_action)
        line = self.bridge.recv_obs()
        if line is None:
            raise ConnectionError("DOSBox closed before the first observation")
        obs, _reward, _done, fields = self._decode(line)
        _sid = int(_num(fields.get(self.screen_key, -1)))
        self.last_info = {"raw": fields, "screen_id": _sid,
                          "screen": _SCREEN_LABEL.get(_sid, "other")}
        return obs, dict(self.last_info)

    def step(self, action):
        if self.bridge is None:
            raise RuntimeError("step() called before reset()")
        self.bridge.send_action(self.actions[int(action)])
        line = self.bridge.recv_obs()
        self._steps += 1
        if line is None:                       # peer closed -> episode ended
            return self._make_obs({}), 0.0, True, False, {"disconnected": True}
        obs, reward, done, fields = self._decode(line)
        terminated = done
        truncated = (self.max_episode_steps is not None
                     and self._steps >= self.max_episode_steps)
        _sid = int(_num(fields.get(self.screen_key, -1)))
        self.last_info = {"raw": fields, "screen_id": _sid,
                          "screen": _SCREEN_LABEL.get(_sid, "other")}
        return obs, reward, terminated, truncated, dict(self.last_info)

    def close(self):
        if self.bridge is not None:
            try:
                if self.quit_action:
                    self.bridge.send_action(self.quit_action)
            except OSError:
                pass
            self.bridge.close()
            self.bridge = None


def _fmt(obs):
    try:
        return "[" + ",".join("%.0f" % v for v in obs) + "]"
    except TypeError:
        return str(obs)


def rollout_demo(host, port, episodes, seed):
    env = DosBoxEnv(host=host, port=port,
                    obs_keys=("t", "ax"),
                    actions=("noop", "tap:space"),
                    reward_key="reward", done_key="done",
                    screens=DEFAULT_SCREENS,        # one-hot the MoM screen
                    max_episode_steps=1000)
    print("[gym] backend: %s + %s"
          % ("gymnasium" if _HAVE_GYM else "fallback-shim",
             "numpy" if _HAVE_NP else "pure-python"), flush=True)
    print("[gym] action_space=%r  observation_space=%r"
          % (env.action_space, env.observation_space), flush=True)
    print("[gym] obs layout: [%s | onehot screen(%d)+other]"
          % (",".join(env.obs_keys), len(env.screens)), flush=True)
    try:
        for ep in range(episodes):
            obs, info = env.reset(seed=seed + ep)
            print("[gym] episode %d reset -> obs=%s" % (ep, _fmt(obs)), flush=True)
            ep_return, steps, terminated, truncated = 0.0, 0, False, False
            while not (terminated or truncated):
                action = env.action_space.sample()             # random policy
                obs, reward, terminated, truncated, info = env.step(action)
                ep_return += reward
                steps += 1
                print("[gym]   step %2d  act=%d(%-9s) screen=%s obs=%s  r=%.1f term=%s trunc=%s"
                      % (steps, action, env.actions[action], info.get("screen", "?"),
                         _fmt(obs), reward, terminated, truncated), flush=True)
            print("[gym] episode %d finished: steps=%d return=%.1f"
                  % (ep, steps, ep_return), flush=True)
    finally:
        env.close()
    print("[gym] closed", flush=True)


if __name__ == "__main__":
    import argparse
    ap = argparse.ArgumentParser(description="DOSBox Gym env demo / smoke test")
    ap.add_argument("--host", default="127.0.0.1")
    ap.add_argument("--port", type=int, default=8642)
    ap.add_argument("--episodes", type=int, default=2)
    ap.add_argument("--seed", type=int, default=0)
    args = ap.parse_args()
    rollout_demo(args.host, args.port, args.episodes, args.seed)
