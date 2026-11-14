from __future__ import annotations

import unittest
from contextlib import redirect_stdout
from io import StringIO
from pathlib import Path

import run_rename_sync as rename_sync


class RunRenameSyncTests(unittest.TestCase):
    def test_chain_runs_six_step_kinds_in_order(self) -> None:
        scripts = [Path(command[2]).name for command in rename_sync.rename_chain_commands("py")]

        self.assertEqual(
            scripts,
            [
                "ida55_functions_to_inventory.py",
                "ida55_asm_body_evidence.py",
                "ida55_join_procedure_evidence.py",
                "ida55_functions_to_inventory.py",
                "ida55_asm_body_evidence.py",
                "ida55_join_procedure_evidence.py",
                "compare_procedure_evidence.py",
                "build_magic_name_sync_handoff.py",
                "generate_magic_name_sync.py",
            ],
        )

    def test_each_step_reads_what_an_earlier_step_wrote(self) -> None:
        commands = rename_sync.rename_chain_commands("py")
        written = set()
        for command in commands:
            for flag in ("--inventory", "--asm-body-evidence", "--left", "--right", "--comparison", "--renames"):
                if flag in command:
                    self.assertIn(command[command.index(flag) + 1], written, f"{flag} in {Path(command[2]).name}")
            for flag in ("--output", "--output-idc"):
                if flag in command:
                    written.add(command[command.index(flag) + 1])

        self.assertIn(str(rename_sync.RENAME_IDC), written)

    def test_handoff_is_written_once_next_to_the_idc(self) -> None:
        self.assertEqual(rename_sync.HANDOFF.parent, rename_sync.RENAME_IDC.parent)


class RenameApplyTests(unittest.TestCase):
    """Applying the renames automatically: batch IDC, work copy with write-back, log check. Powered by Claude."""

    def test_batch_idc_command_reads_the_handoff_and_passes_batch(self) -> None:
        command = rename_sync.batch_idc_command("py")
        self.assertEqual(command[command.index("--renames") + 1], str(rename_sync.HANDOFF))
        self.assertEqual(command[command.index("--output-idc") + 1], str(rename_sync.BATCH_RENAME_IDC))
        self.assertIn("--batch", command)

    def test_apply_command_uses_work_copy_with_write_back_before_separator(self) -> None:
        command = rename_sync.apply_command("py", "idaw.exe", Path("C:/ida/MAGIC.idb"), Path("w"), Path("C:/ida"), Path("l.log"))
        separator = command.index("--")
        for flag in ("--work-dir", "--write-back", "--backup-dir", "--ida-exe"):
            self.assertLess(command.index(flag), separator, flag)
        self.assertEqual(command[-1], str(Path("C:/ida/MAGIC.idb")))
        self.assertTrue(command[-2].startswith("-S") and command[-2].endswith("magic_sync_names_from_wzd.batch.idc"))
        self.assertIn("-Ll.log", command)

    def test_verify_re_exports_magic_only(self) -> None:
        for command in rename_sync.verify_export_commands("py", 900):
            self.assertEqual(command[command.index("--game") + 1], "MAGIC")

    def test_parse_rename_log_counts_results_and_flags_problems(self) -> None:
        log = "\n".join([
            "Executing function 'main'...",
            "renamed,LBX_IMG_VShiftRect,Shear_Bitmap_Y,1234",
            "already-synced,Old,New,5678",
            "collision,A,B",
            "magic-name-sync-summary,attempted=3,ok=2,failed=1",
            "magic-name-sync-saved",
        ])
        report = rename_sync.parse_rename_log(log)
        self.assertEqual(report["counts"]["renamed"], 1)
        self.assertEqual(report["counts"]["already-synced"], 1)
        self.assertEqual(report["problems"], ["collision,A,B"])
        self.assertTrue(report["saved"])
        self.assertTrue(report["summary"].startswith("magic-name-sync-summary"))

    def test_parse_rename_log_without_summary_counterexample(self) -> None:
        report = rename_sync.parse_rename_log("renamed,A,B,1\n")
        self.assertIsNone(report["summary"])
        self.assertFalse(report["saved"])

    def test_zero_renames_never_starts_ida(self) -> None:
        calls = []
        originals = (rename_sync.run_chain, rename_sync.rename_count, rename_sync.apply_renames, rename_sync.required_inputs)
        rename_sync.run_chain = lambda python, dry_run: calls.append("chain")
        rename_sync.rename_count = lambda path: 0
        rename_sync.apply_renames = lambda args, stamp: calls.append("apply")
        rename_sync.required_inputs = lambda: []
        try:
            with redirect_stdout(StringIO()):
                self.assertEqual(rename_sync.main([]), 0)
        finally:
            rename_sync.run_chain, rename_sync.rename_count, rename_sync.apply_renames, rename_sync.required_inputs = originals
        self.assertEqual(calls, ["chain"])


class BatchIdcTests(unittest.TestCase):
    """The batch rename IDC must end IDA with an exit code; the GUI one must not. Powered by Claude."""

    def test_batch_idc_exits_with_status(self) -> None:
        from ida_proc_name_sync.generate_magic_name_sync import render_idc

        text = render_idc([{"old_name": "A", "new_name": "B"}], batch=True)
        self.assertIn("Exit(0);", text)
        self.assertIn("Exit(1);", text)
        self.assertIn("Exit(2);", text)
        self.assertLess(text.index("SaveBase"), text.index("Exit(0);"))

    def test_gui_idc_has_no_exit(self) -> None:
        from ida_proc_name_sync.generate_magic_name_sync import render_idc

        self.assertNotIn("Exit(", render_idc([{"old_name": "A", "new_name": "B"}]))


if __name__ == "__main__":
    unittest.main()
