from __future__ import annotations

import unittest
from pathlib import Path
from tempfile import TemporaryDirectory

from ida_proc_name_sync_prereq.build_magic_name_sync_handoff import build_handoff, collect_minimal_renames
from ida_proc_name_sync.generate_magic_name_sync import escape_idc_string, load_renames, render_idc


def comparison() -> dict[str, object]:
    return {
        "left_database": "MGC",
        "right_database": "WZD",
        "procedure_pairs": [
            {
                "segment_pair": {"pair_id": "MGC:seg001__WZD:seg001"},
                "procedure_ordinal": 2,
                "left": {
                    "name": "OldName",
                    "segment": "seg001",
                    "start_ea": "seg001:0010",
                },
                "right": {
                    "name": "NewName",
                    "segment": "seg001",
                    "start_ea": "seg001:0014",
                },
                "observations": {
                    "same_idb_name": False,
                    "same_size": True,
                    "same_normalized_body_hash": True,
                    "same_mnemonic_sequence_hash": True,
                },
            },
            {
                "segment_pair": {"pair_id": "MGC:seg001__WZD:seg001"},
                "procedure_ordinal": 1,
                "left": {
                    "name": "SameName",
                    "segment": "seg001",
                    "start_ea": "seg001:0000",
                },
                "right": {
                    "name": "SameName",
                    "segment": "seg001",
                    "start_ea": "seg001:0004",
                },
                "observations": {
                    "same_idb_name": True,
                    "same_size": True,
                    "same_normalized_body_hash": True,
                    "same_mnemonic_sequence_hash": True,
                },
            },
        ],
    }


class IdaProcNameSyncTests(unittest.TestCase):
    def test_collect_minimal_renames_skips_already_synced_names(self) -> None:
        renames = collect_minimal_renames(comparison())

        self.assertEqual(renames, [{"old_name": "OldName", "new_name": "NewName"}])

    def test_handoff_only_contains_minimal_rename_fields(self) -> None:
        handoff = build_handoff(comparison())

        self.assertEqual(handoff["schema_version"], 1)
        self.assertEqual(handoff["artifact"], "magic_proc_name_sync_handoff")
        self.assertEqual(handoff["renames"], [{"old_name": "OldName", "new_name": "NewName"}])
        self.assertEqual(set(handoff["renames"][0]), {"old_name", "new_name"})

    def test_load_renames_rejects_entries_without_required_names(self) -> None:
        with TemporaryDirectory() as temp_dir:
            path = Path(temp_dir) / "renames.json"
            path.write_text('{"renames": [{"old_name": "OldName"}]}', encoding="utf-8")

            with self.assertRaisesRegex(ValueError, "old_name and new_name"):
                load_renames(path)

    def test_escape_idc_string_escapes_quotes_and_backslashes(self) -> None:
        self.assertEqual(escape_idc_string('a"b\\c'), 'a\\"b\\\\c')

    def test_render_idc_contains_safety_checks_and_rename_call(self) -> None:
        idc = render_idc(collect_minimal_renames(comparison()))

        self.assertIn("LocByName(old_name)", idc)
        self.assertIn("GetFunctionName(ea)", idc)
        self.assertIn("MakeNameEx(ea, new_name, SN_CHECK|SN_NOWARN)", idc)
        self.assertIn('sync_magic_proc_name("OldName", "NewName")', idc)
        self.assertIn("magic-name-sync-summary", idc)


if __name__ == "__main__":
    unittest.main()
