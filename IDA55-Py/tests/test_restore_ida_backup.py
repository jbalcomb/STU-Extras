from __future__ import annotations

import tempfile
import unittest
from pathlib import Path

from ida55py import restore_ida_backup as restore_backup


class RestoreIdaBackupTests(unittest.TestCase):
    def test_latest_backup_ignores_zero_byte_backup(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            (root / "MAGIC-202605051200.idb").write_bytes(b"older")
            (root / "MAGIC-202605051300.idb").write_bytes(b"")

            self.assertEqual(restore_backup.latest_backup(root, "MAGIC").name, "MAGIC-202605051200.idb")

    def test_dry_run_refuses_nonzero_current_without_flag(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            (root / "MAGIC.idb").write_bytes(b"current")
            (root / "MAGIC-202605051200.idb").write_bytes(b"backup")

            with self.assertRaises(restore_backup.RestoreBackupError):
                restore_backup.restore_game(
                    root,
                    "MAGIC",
                    apply=False,
                    allow_overwrite_nonzero=False,
                    stamp="202605051400",
                )

    def test_apply_restores_backup_and_preserves_damaged_current(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            current = root / "MAGIC.idb"
            damaged = root / "MAGIC-damaged-202605051400.idb"
            current.write_bytes(b"")
            (root / "MAGIC-202605051200.idb").write_bytes(b"backup")

            lines = restore_backup.restore_game(
                root,
                "MAGIC",
                apply=True,
                allow_overwrite_nonzero=False,
                stamp="202605051400",
            )

            self.assertEqual(current.read_bytes(), b"backup")
            self.assertEqual(damaged.read_bytes(), b"")
            self.assertIn("status=restored", lines)

    def test_apply_reads_backup_from_separate_backup_dir(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            idb_dir = Path(temp_dir) / "idbs"
            backup_dir = Path(temp_dir) / "backups"
            idb_dir.mkdir()
            backup_dir.mkdir()
            current = idb_dir / "WIZARDS.idb"
            current.write_bytes(b"")
            (backup_dir / "WIZARDS-202605051200.idb").write_bytes(b"backup")

            lines = restore_backup.restore_game(
                idb_dir,
                "WIZARDS",
                apply=True,
                allow_overwrite_nonzero=False,
                stamp="202605051400",
                backup_dir=backup_dir,
            )

            self.assertEqual(current.read_bytes(), b"backup")
            self.assertIn("status=restored", lines)

    def test_idb_names_lists_idbs_but_not_backups_or_damaged_copies(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            for name in ("MAGIC.idb", "WIZARDS.idb", "MAGIC-202605051200.idb", "MAGIC-damaged-202605051400.idb", "notes.txt"):
                (root / name).write_bytes(b"x")

            self.assertEqual(restore_backup.idb_names(root, None), ["MAGIC", "WIZARDS"])
            self.assertEqual(restore_backup.idb_names(root, ["ORION2.idb", "MAGIC"]), ["ORION2", "MAGIC"])


if __name__ == "__main__":
    unittest.main()
