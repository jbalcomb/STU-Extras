#!/usr/bin/env python3
"""mom_bc.py -- behavioral-cloning training on recorded MoM sessions.

Pipeline: load every ~/mom_session*.txt -> filter to gameplay screens -> label
actions (keys + grid clicks) -> balance the noop-heavy classes -> train a small
MLP policy. With torch+numpy it trains and reports validation accuracy; without
them it prints the dataset summary + the majority-class baseline so you can see
the setup and the target to beat.

    python3 mom_bc.py                       # all ~/mom_session*.txt
    python3 mom_bc.py --glob '/path/*.txt' --noop-ratio 1.0 --epochs 30
"""

import argparse
import collections
import glob
import random

from session_dataset import build_dataset, GAMEPLAY_SCREENS
from mom_gym import mom_action_map, MOM_ACTIONS, MOM_OBS_KEYS, _GRID_BASE


def load_sessions(pattern, keep_screens=GAMEPLAY_SCREENS):
    """Concatenate all matching sessions into (X list of obs, Y list of action ids)."""
    X, Y = [], []
    files = sorted(glob.glob(pattern))
    for f in files:
        xs, ys = build_dataset(f, obs_keys=MOM_OBS_KEYS, action_map=mom_action_map,
                               keep_screens=keep_screens)
        X += (xs.tolist() if hasattr(xs, "tolist") else list(xs))
        Y += [int(a) for a in (list(ys) if hasattr(ys, "__iter__") else ys)]
    return X, Y, files


def balance(X, Y, noop_ratio=1.0, seed=0):
    """Keep every action frame; downsample noop to noop_ratio x (#action frames)."""
    rnd = random.Random(seed)
    noop = [i for i, y in enumerate(Y) if y == 0]
    act = [i for i, y in enumerate(Y) if y != 0]
    keep_noop = rnd.sample(noop, min(len(noop), int(noop_ratio * len(act)))) if act else []
    keep = act + keep_noop
    rnd.shuffle(keep)
    return [X[i] for i in keep], [Y[i] for i in keep]


def summarize(X, Y, tag):
    c = collections.Counter(Y)
    n = len(Y)
    keys = sum(v for k, v in c.items() if 1 <= k < _GRID_BASE)
    clicks = sum(v for k, v in c.items() if k >= _GRID_BASE)
    maj_id, maj_n = c.most_common(1)[0]
    print("[%s] %d frames | noop %d (%.1f%%)  keys %d  clicks %d  | distinct actions %d"
          % (tag, n, c[0], 100 * c[0] / n, keys, clicks, len(c)))
    print("       majority-class baseline (always predict %d): %.1f%% accuracy -- the bar to beat"
          % (maj_id, 100 * maj_n / n))


def _have(mod):
    import importlib.util
    return importlib.util.find_spec(mod) is not None


def train_numpy(Xtr, Ytr, Xva, Yva, n_actions, epochs, seed):
    """Class-weighted multinomial logistic regression -- numpy only, trains in
    seconds. A linear baseline policy; swap to train_torch (an MLP) for capacity."""
    import numpy as np
    Xtr = np.asarray(Xtr, "float64"); Xva = np.asarray(Xva, "float64")
    mu, sd = Xtr.mean(0), Xtr.std(0) + 1e-6                # standardize
    Xtr = np.hstack([(Xtr - mu) / sd, np.ones((len(Xtr), 1))])   # + bias column
    Xva = np.hstack([(Xva - mu) / sd, np.ones((len(Xva), 1))])
    Ytr = np.asarray(Ytr); Yva = np.asarray(Yva)
    counts = np.bincount(Ytr, minlength=n_actions).astype(float)
    cw = np.sqrt(counts.sum() / (counts + 1.0)); cw /= cw.mean()   # mild up-weight of rare actions
    W = np.zeros((Xtr.shape[1], n_actions)); lr = 0.5
    Yoh = np.zeros((len(Ytr), n_actions)); Yoh[np.arange(len(Ytr)), Ytr] = 1
    wsamp = cw[Ytr][:, None]
    for ep in range(epochs):
        Z = Xtr @ W; Z -= Z.max(1, keepdims=True)
        P = np.exp(Z); P /= P.sum(1, keepdims=True)
        W -= lr * (Xtr.T @ ((P - Yoh) * wsamp)) / len(Ytr)
        if (ep + 1) % max(1, epochs // 10) == 0:
            pred = (Xva @ W).argmax(1)
            acc = (pred == Yva).mean()
            m = Yva != 0
            aacc = (pred[m] == Yva[m]).mean() if m.any() else 0.0
            print("   epoch %3d  val_acc %.3f  (action-only frames %.3f)" % (ep + 1, acc, aacc))
    return W


def train_torch(Xtr, Ytr, Xva, Yva, n_actions, epochs, seed):
    import numpy as np
    import torch
    import torch.nn as nn
    torch.manual_seed(seed)
    Xtr = np.asarray(Xtr, "float32"); Xva = np.asarray(Xva, "float32")
    mu, sd = Xtr.mean(0), Xtr.std(0) + 1e-6            # standardize features
    Xtr = (Xtr - mu) / sd; Xva = (Xva - mu) / sd
    Xtr = torch.tensor(Xtr); Xva = torch.tensor(Xva)
    Ytr = torch.tensor(np.asarray(Ytr, "int64")); Yva = torch.tensor(np.asarray(Yva, "int64"))
    # class-weighted loss so rare click/key actions count
    counts = torch.bincount(Ytr, minlength=n_actions).float()
    w = (counts.sum() / (counts + 1.0)); w = w / w.mean()
    net = nn.Sequential(nn.Linear(Xtr.shape[1], 128), nn.ReLU(),
                        nn.Linear(128, 128), nn.ReLU(),
                        nn.Linear(128, n_actions))
    opt = torch.optim.Adam(net.parameters(), 1e-3)
    lossf = nn.CrossEntropyLoss(weight=w)
    for ep in range(epochs):
        net.train(); opt.zero_grad()
        loss = lossf(net(Xtr), Ytr); loss.backward(); opt.step()
        if (ep + 1) % max(1, epochs // 10) == 0:
            net.eval()
            with torch.no_grad():
                acc = (net(Xva).argmax(1) == Yva).float().mean().item()
            print("   epoch %3d  loss %.3f  val_acc %.3f" % (ep + 1, loss.item(), acc))
    return net


if __name__ == "__main__":
    ap = argparse.ArgumentParser(description="Behavioral cloning on recorded MoM sessions")
    ap.add_argument("--glob", default="/home/jbalcomb/mom_session*.txt")
    ap.add_argument("--noop-ratio", type=float, default=1.0,
                    help="noop frames kept per action frame after balancing (default 1.0)")
    ap.add_argument("--epochs", type=int, default=40)
    ap.add_argument("--val-frac", type=float, default=0.2)
    ap.add_argument("--seed", type=int, default=0)
    args = ap.parse_args()

    X, Y, files = load_sessions(args.glob)
    print("loaded %d sessions, %d gameplay transitions, action space %d\n"
          % (len(files), len(Y), len(MOM_ACTIONS)))
    summarize(X, Y, "raw")
    Xb, Yb = balance(X, Y, args.noop_ratio, args.seed)
    summarize(Xb, Yb, "balanced")

    # train/val split
    rnd = random.Random(args.seed)
    idx = list(range(len(Yb))); rnd.shuffle(idx)
    cut = int(len(idx) * (1 - args.val_frac))
    tr, va = idx[:cut], idx[cut:]
    Xtr, Ytr = [Xb[i] for i in tr], [Yb[i] for i in tr]
    Xva, Yva = [Xb[i] for i in va], [Yb[i] for i in va]

    if _have("torch"):
        print("\ntraining MLP policy with torch (%d train / %d val)..." % (len(Ytr), len(Yva)))
        train_torch(Xtr, Ytr, Xva, Yva, len(MOM_ACTIONS), args.epochs, args.seed)
    elif _have("numpy"):
        print("\ntraining softmax policy with numpy (%d train / %d val)..." % (len(Ytr), len(Yva)))
        train_numpy(Xtr, Ytr, Xva, Yva, len(MOM_ACTIONS), args.epochs, args.seed)
    else:
        print("\n[no numpy/torch] set up a venv and install numpy (fast) to train:")
        print("      python3 -m venv ~/momrl-venv")
        print("      ~/momrl-venv/bin/pip install numpy")
        print("      ~/momrl-venv/bin/python mom_bc.py")
        print("The dataset above is ready; the majority baseline is the bar to beat.")
