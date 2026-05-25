from __future__ import annotations

import tempfile
import unittest
from pathlib import Path
import os

import rasm_sync_translations as sync


class RasmSyncTranslationsTests(unittest.TestCase):
    def test_discover_matched_translation(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir) / "out"
            path = root / "WIZARDS" / "ovr001"
            path.mkdir(parents=True)
            (path / "Foo__WIP.asm").write_text("asm", encoding="utf-8")
            (path / "Foo__WIP.c").write_text("c", encoding="utf-8")

            records = sync.discover_translations(root)

            self.assertEqual(len(records), 1)
            self.assertEqual(records[0].status, "matched")

    def test_discover_move_candidate(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir) / "out"
            old_path = root / "MAGIC" / "seg001"
            new_path = root / "WIZARDS" / "ovr114"
            old_path.mkdir(parents=True)
            new_path.mkdir(parents=True)
            (old_path / "Foo__WIP.c").write_text("c", encoding="utf-8")
            (new_path / "Foo__WIP.asm").write_text("asm", encoding="utf-8")

            record = sync.discover_translations(root)[0]

        self.assertEqual(record.status, "move-candidate")
        self.assertEqual(record.target_c.name, "Foo__WIP.c")

    def test_discover_move_conflict_counterexample(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir) / "out"
            old_path = root / "MAGIC" / "seg001"
            new_path = root / "WIZARDS" / "ovr114"
            old_path.mkdir(parents=True)
            new_path.mkdir(parents=True)
            (old_path / "Foo__WIP.c").write_text("old", encoding="utf-8")
            (new_path / "Foo__WIP.asm").write_text("asm", encoding="utf-8")
            (new_path / "Foo__WIP.c").write_text("new", encoding="utf-8")

            record = sync.discover_translations(root)[0]

            self.assertEqual(record.status, "move-conflict")
            self.assertIsNone(record.target_c)

    def test_apply_renames_moves_only_candidates(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir) / "out"
            old_path = root / "MAGIC" / "seg001"
            new_path = root / "WIZARDS" / "ovr114"
            old_path.mkdir(parents=True)
            new_path.mkdir(parents=True)
            source = old_path / "Foo__WIP.c"
            target = new_path / "Foo__WIP.c"
            source.write_text("c", encoding="utf-8")
            (new_path / "Foo__WIP.asm").write_text("asm", encoding="utf-8")

            records = sync.discover_translations(root)
            applied = sync.apply_renames(records)

            self.assertEqual(applied, [{"from": sync.path_text(source), "to": sync.path_text(target)}])
            self.assertFalse(source.exists())
            self.assertTrue(target.exists())

    def test_parse_checklist_finds_rasm_named_entries(self) -> None:
        text = """[x] c:/python314/python.exe .\\rasm_named_gemini.py Done_Function
[] c:/python314/python.exe .\\rasm_named_gemini.py Todo_Function --force
not a checklist line
"""
        entries = sync.parse_checklist(text)

        self.assertEqual([entry.function_name for entry in entries], ["Done_Function", "Todo_Function"])
        self.assertTrue(entries[0].checked)
        self.assertFalse(entries[1].checked)

    def test_plan_checklist_updates_marks_unchecked_existing_translation(self) -> None:
        text = "[] c:/python314/python.exe .\\rasm_named_gemini.py Todo_Function --force\n"
        entries = sync.parse_checklist(text)
        updates = sync.plan_checklist_updates(entries, {"Todo_Function"})

        self.assertEqual(len(updates), 1)
        self.assertEqual(updates[0].new_line, "[x] c:/python314/python.exe .\\rasm_named_gemini.py Todo_Function --force")

    def test_plan_checklist_updates_leaves_checked_entries_alone(self) -> None:
        text = "[x] c:/python314/python.exe .\\rasm_named_gemini.py Done_Function\n"
        entries = sync.parse_checklist(text)
        updates = sync.plan_checklist_updates(entries, {"Done_Function"})

        self.assertEqual(updates, [])

    def test_normalize_windows_path_under_posix(self) -> None:
        repo_root = Path("/repo")
        result = sync.normalize_path(Path(r"C:\STU\devel\ReMoM\doc\#TODO\stub_wip_todo.md"), repo_root)
        if os.name == "nt":
            self.assertEqual(result, Path(r"C:\STU\devel\ReMoM\doc\#TODO\stub_wip_todo.md"))
        else:
            self.assertEqual(result, Path("/mnt/c/STU/devel/ReMoM/doc/#TODO/stub_wip_todo.md"))


if __name__ == "__main__":
    unittest.main()
