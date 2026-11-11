from __future__ import annotations

import unittest
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


if __name__ == "__main__":
    unittest.main()
