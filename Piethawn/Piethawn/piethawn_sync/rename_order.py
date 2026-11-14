"""Order a set of procedure renames so they can be applied one at a time in IDA without colliding."""

from __future__ import annotations

from typing import Dict, Iterable, List

TEMP_SUFFIX = "__rnsync_tmp"


def order_rename_pairs(renames: Iterable[Dict[str, str]], existing_names: Iterable[str] = ()) -> List[Dict[str, str]]:
    """Return rename steps {old_name, new_name} in a safe order: a procedure that frees a name is renamed before the
    one that takes it (chains), and a cycle (names swapped) is broken by first moving one member to
    <old>__rnsync_tmp. Input order is kept wherever nothing forces a change. Raises ValueError when two renames share
    an old or new name, or a temporary name is already in existing_names. Powered by Claude."""
    renames = [{"old_name": str(r["old_name"]), "new_name": str(r["new_name"])} for r in renames]
    by_old: Dict[str, Dict[str, str]] = {}
    targets: Dict[str, str] = {}
    for r in renames:
        if r["old_name"] in by_old:
            raise ValueError(f"Two renames for {r['old_name']}.")
        if r["new_name"] in targets:
            raise ValueError(f"{targets[r['new_name']]} and {r['old_name']} would both be renamed to {r['new_name']}.")
        by_old[r["old_name"]] = r
        targets[r["new_name"]] = r["old_name"]
    existing = set(existing_names) | set(by_old) | set(targets)

    state: Dict[str, int] = {}
    current: Dict[str, str] = {}
    steps: List[Dict[str, str]] = []

    def visit(r: Dict[str, str]) -> None:
        old = r["old_name"]
        if state.get(old) == 2:
            return
        if state.get(old) == 1:  # back at a rename still in progress: a cycle; free its name first
            temp = old + TEMP_SUFFIX
            if temp in existing:
                raise ValueError(f"Cannot break the rename cycle through {old}: temporary name {temp} already exists.")
            steps.append({"old_name": old, "new_name": temp})
            current[old] = temp
            return
        state[old] = 1
        blocker = by_old.get(r["new_name"])
        if blocker is not None and blocker is not r:
            visit(blocker)
        state[old] = 2
        steps.append({"old_name": current.get(old, old), "new_name": r["new_name"]})

    for r in renames:
        visit(r)
    return steps
