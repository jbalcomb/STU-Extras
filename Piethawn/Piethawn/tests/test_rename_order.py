"""Tests for piethawn_sync.rename_order and its use in the WZD -> MGC handoff. Powered by Claude."""

from __future__ import annotations

import unittest

from ida_proc_name_sync_prereq.build_magic_name_sync_handoff import build_handoff
from piethawn_sync.rename_order import order_rename_pairs


def pairs(steps):
    """(old, new) tuples of rename steps. Powered by Claude."""
    return [(s["old_name"], s["new_name"]) for s in steps]


class RenameOrderTests(unittest.TestCase):
    def test_independent_renames_keep_input_order(self) -> None:
        steps = order_rename_pairs([{"old_name": "B", "new_name": "B2"}, {"old_name": "A", "new_name": "A2"}])
        self.assertEqual(pairs(steps), [("B", "B2"), ("A", "A2")])

    def test_chain_frees_the_name_first(self) -> None:
        steps = order_rename_pairs([{"old_name": "A", "new_name": "B"}, {"old_name": "B", "new_name": "C"}])
        self.assertEqual(pairs(steps), [("B", "C"), ("A", "B")])

    def test_swap_goes_through_a_temporary_name(self) -> None:
        steps = order_rename_pairs([
            {"old_name": "Disable_Cancel", "new_name": "Enable_Cancel"},
            {"old_name": "Enable_Cancel", "new_name": "Disable_Cancel"},
        ])
        self.assertEqual(pairs(steps), [
            ("Disable_Cancel", "Disable_Cancel__rnsync_tmp"),
            ("Enable_Cancel", "Disable_Cancel"),
            ("Disable_Cancel__rnsync_tmp", "Enable_Cancel"),
        ])

    def test_three_way_cycle(self) -> None:
        steps = order_rename_pairs([
            {"old_name": "A", "new_name": "B"}, {"old_name": "B", "new_name": "C"}, {"old_name": "C", "new_name": "A"},
        ])
        names = {"A": 1, "B": 2, "C": 3}  # simulate: apply each step and check no name is ever taken twice
        for old, new in pairs(steps):
            self.assertNotIn(new, names)
            names[new] = names.pop(old)
        self.assertEqual(names, {"B": 1, "C": 2, "A": 3})

    def test_taken_temporary_name_counterexample(self) -> None:
        with self.assertRaisesRegex(ValueError, "temporary name"):
            order_rename_pairs(
                [{"old_name": "A", "new_name": "B"}, {"old_name": "B", "new_name": "A"}], existing_names={"A__rnsync_tmp"}
            )

    def test_duplicate_target_counterexample(self) -> None:
        with self.assertRaisesRegex(ValueError, "both be renamed"):
            order_rename_pairs([{"old_name": "A", "new_name": "X"}, {"old_name": "B", "new_name": "X"}])

    def test_handoff_orders_a_swap(self) -> None:
        def pair(ordinal, left, right):
            return {"procedure_ordinal": ordinal, "left": {"name": left, "segment": "seg036"}, "right": {"name": right}}
        handoff = build_handoff({"procedure_pairs": [
            pair(1, "Disable_Cancel", "Enable_Cancel"), pair(2, "Enable_Cancel", "Disable_Cancel"),
        ]})
        self.assertEqual(pairs(handoff["renames"])[0], ("Disable_Cancel", "Disable_Cancel__rnsync_tmp"))
        self.assertEqual(len(handoff["renames"]), 3)


if __name__ == "__main__":
    unittest.main()
