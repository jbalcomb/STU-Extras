from __future__ import annotations

import tempfile
import unittest
from pathlib import Path

from ida_proc_status_sync import generate_status_idc, status_plan


class StatusPlanTests(unittest.TestCase):
    def test_status_plan_propagates_completed_color_across_same_current_name(self) -> None:
        magic_key = ("MAGIC", "SharedDone")
        wizards_key = ("WIZARDS", "SharedDone")
        magic_nodes = {
            magic_key: {
                "game": "MAGIC",
                "name": "SharedDone",
                "segment": "seg001",
                "entry_ea": "1000",
                "start_ea": "1000",
                "end_ea": "1005",
                "size": 5,
                "item_color": 16777088,
                "item_color_hex": "00FFFF80",
                "funcattr_color": 16777088,
                "funcattr_color_hex": "00FFFF80",
                "has_user_color": True,
            }
        }
        wizards_nodes = {
            wizards_key: {
                "game": "WIZARDS",
                "name": "SharedDone",
                "segment": "seg001",
                "entry_ea": "1000",
                "start_ea": "1000",
                "end_ea": "1005",
                "size": 5,
                "item_color": -1,
                "item_color_hex": "FFFFFFFF",
                "funcattr_color": -1,
                "funcattr_color_hex": "FFFFFFFF",
                "has_user_color": False,
            }
        }

        plan = status_plan.build_status_plan(magic_nodes, wizards_nodes, "00ffff80")

        self.assertEqual(plan["summary"]["join_key"], "name")
        self.assertEqual(plan["summary"]["sync_update_count_by_game"], {"MAGIC": 0, "WIZARDS": 1})
        self.assertEqual(plan["updates_by_game"]["WIZARDS"][0]["name"], "SharedDone")
        self.assertEqual(plan["updates_by_game"]["WIZARDS"][0]["desired_color_hex"], "00FFFF80")

    def test_status_plan_does_not_update_different_current_name_counterexample(self) -> None:
        magic_nodes = {
            ("MAGIC", "MagicDone"): {
                "game": "MAGIC",
                "name": "MagicDone",
                "segment": "seg001",
                "entry_ea": "1000",
                "start_ea": "1000",
                "end_ea": "1005",
                "size": 5,
                "item_color_hex": "00FFFF80",
            }
        }
        wizards_nodes = {
            ("WIZARDS", "WizardsTodo"): {
                "game": "WIZARDS",
                "name": "WizardsTodo",
                "segment": "seg001",
                "entry_ea": "2000",
                "start_ea": "2000",
                "end_ea": "2005",
                "size": 5,
                "item_color_hex": "FFFFFFFF",
            }
        }

        plan = status_plan.build_status_plan(magic_nodes, wizards_nodes, "00FFFF80")

        self.assertEqual(plan["summary"]["sync_update_count_by_game"], {"MAGIC": 0, "WIZARDS": 0})


class StatusIdcGenerationTests(unittest.TestCase):
    def test_generate_status_idc_uses_plan_updates(self) -> None:
        plan = {
            "summary": {"completed_color_hex": "00FFFF80"},
            "updates_by_game": {
                "MAGIC": [{"name": "MagicTodo", "desired_color_hex": "00FFFF80"}],
                "WIZARDS": [],
            },
        }
        with tempfile.TemporaryDirectory() as temp_dir:
            magic_idc = Path(temp_dir) / "mgc.idc"
            wizards_idc = Path(temp_dir) / "wzd.idc"

            generate_status_idc.write_status_idc(plan, magic_idc, wizards_idc)

            self.assertIn('sync_completion(log_handle, "MagicTodo", 0x00FFFF80);', magic_idc.read_text(encoding="utf-8"))
            self.assertIn("update_count = 0;", wizards_idc.read_text(encoding="utf-8"))


if __name__ == "__main__":
    unittest.main()
