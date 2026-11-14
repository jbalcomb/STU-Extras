#!/usr/bin/env python3
"""dosbox_bridge.py -- Python side of the DOSBox Lua <-> Python bridge.

DOSBox (the Lua env script) is the TCP *server*; this module connects to it as
the client. Protocol: newline-delimited UTF-8 text. One step is:

    obs  = bridge.recv_obs()      # <- one observation line from DOSBox
    bridge.send_action(action)    # -> one action line to DOSBox

The exact line formats are defined by your Lua env script -- swap the plain
"k=v k=v" text below for JSON or a packed binary encoding whenever you like;
the transport does not care.

Run directly for a self-contained demo/handshake test:
    python3 dosbox_bridge.py --port 8642 --steps 8
(Start it any time; it retries until DOSBox is listening.)
"""

import argparse
import socket
import time


class DosBoxBridge:
    """Blocking, lockstep client for the DOSBox Lua bridge."""

    def __init__(self, host="127.0.0.1", port=8642, connect_timeout=30.0):
        self.sock = None
        self._buf = b""
        deadline = time.time() + connect_timeout
        last_err = None
        while time.time() < deadline:
            try:
                s = socket.create_connection((host, port), timeout=5.0)
                s.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
                self.sock = s
                return
            except OSError as e:            # DOSBox may not be listening yet
                last_err = e
                time.sleep(0.2)
        raise ConnectionError(
            "could not connect to DOSBox bridge at %s:%d (%s)"
            % (host, port, last_err))

    def recv_obs(self):
        """Return the next observation line (str), or None if DOSBox closed."""
        while b"\n" not in self._buf:
            chunk = self.sock.recv(4096)
            if not chunk:
                return None
            self._buf += chunk
        line, _, self._buf = self._buf.partition(b"\n")
        return line.decode("utf-8", "replace").rstrip("\r")

    def send_action(self, action):
        self.sock.sendall((action + "\n").encode("utf-8"))

    def close(self):
        if self.sock is not None:
            try:
                self.sock.close()
            finally:
                self.sock = None

    # context-manager sugar
    def __enter__(self):
        return self

    def __exit__(self, *exc):
        self.close()


def parse_obs(line):
    """Parse 'k=v k=v ...' into a dict of strings (env-specific; adjust freely)."""
    out = {}
    for tok in line.split():
        if "=" in tok:
            k, v = tok.split("=", 1)
            out[k] = v
    return out


def demo(host, port, steps, connect_timeout):
    print("[py] connecting to %s:%d ..." % (host, port), flush=True)
    with DosBoxBridge(host, port, connect_timeout) as br:
        print("[py] connected", flush=True)
        for i in range(steps):
            obs = br.recv_obs()
            if obs is None:
                print("[py] DOSBox closed the connection", flush=True)
                break
            fields = parse_obs(obs)
            # trivial scripted policy: tap space every 3rd step, quit on the last
            if i == steps - 1:
                action = "quit"
            elif i % 3 == 2:
                action = "tap:space"
            else:
                action = "noop"
            print("[py] step %d  obs=%s  ->  action=%s" % (i, fields, action),
                  flush=True)
            br.send_action(action)
            if action == "quit":
                break
    print("[py] done", flush=True)


if __name__ == "__main__":
    ap = argparse.ArgumentParser(description="DOSBox Lua bridge client / demo")
    ap.add_argument("--host", default="127.0.0.1")
    ap.add_argument("--port", type=int, default=8642)
    ap.add_argument("--steps", type=int, default=8)
    ap.add_argument("--connect-timeout", type=float, default=30.0)
    args = ap.parse_args()
    demo(args.host, args.port, args.steps, args.connect_timeout)
