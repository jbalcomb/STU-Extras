from __future__ import annotations

import tempfile
from contextlib import redirect_stdout
from io import StringIO
import unittest
from datetime import datetime, timezone
from pathlib import Path

from ida_automation import idaw


class IdawTests(unittest.TestCase):
    def test_timestamp_uses_compact_iso_order(self) -> None:
        value = idaw.timestamp(datetime(2026, 4, 30, 21, 34, tzinfo=timezone.utc))
        self.assertEqual(value, "202604302134")

    def test_find_idb_argument_accepts_single_idb(self) -> None:
        result = idaw.find_idb_argument(["-A", "-Sscript.idc", "MAGIC.idb"])
        self.assertEqual(result, Path("MAGIC.idb"))

    def test_find_idb_argument_rejects_missing_idb_counterexample(self) -> None:
        with self.assertRaisesRegex(idaw.IdawError, "Expected exactly one"):
            idaw.find_idb_argument(["-A", "-Sscript.idc"])

    def test_find_idb_argument_rejects_two_idbs_counterexample(self) -> None:
        with self.assertRaisesRegex(idaw.IdawError, "Expected exactly one"):
            idaw.find_idb_argument(["MAGIC.idb", "WIZARDS.idb"])

    def test_backup_path_for_idb_uses_timestamped_stem(self) -> None:
        result = idaw.backup_path_for_idb(Path("/ida/MAGIC.idb"), "202604302134")
        expected = idaw.normalize_path(idaw.DEFAULT_BACKUP_DIR) / "MAGIC-202604302134.idb"
        self.assertEqual(result, expected)

    def test_backup_idb_backs_up_non_empty_idb(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            source = Path(temp_dir) / "MAGIC.idb"
            backup = Path(temp_dir) / "MAGIC-202604302134.idb"
            source.write_bytes(b"idb-data")

            idaw.backup_idb(source, backup)

            self.assertEqual(backup.read_bytes(), b"idb-data")

    def test_backup_idb_rejects_zero_byte_idb_counterexample(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            source = Path(temp_dir) / "MAGIC.idb"
            backup = Path(temp_dir) / "MAGIC-202604302134.idb"
            source.write_bytes(b"")

            with self.assertRaisesRegex(idaw.IdawError, "0-byte IDB"):
                idaw.backup_idb(source, backup)


    def test_main_dry_run_uses_backup_wrapper_without_launching_ida(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            idb = Path(temp_dir) / "MAGIC.idb"
            idb.write_bytes(b"idb-data")
            output = StringIO()

            with redirect_stdout(output):
                result = idaw.main(["--dry-run", "--", "-A", "-Sscript.idc", str(idb)])

            self.assertEqual(result, 0)
            text = output.getvalue()
            self.assertIn("Backup:", text)
            self.assertIn("MAGIC-", text)
            self.assertIn("idaw.exe", text)
            self.assertIn(str(idb), text)


    def test_log_contains_unpacked_database_prompt_ignores_old_content(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            log = Path(temp_dir) / "ida.log"
            log.write_text("IDA has found unpacked version of database C:\\X\\MAGIC.idb\n", encoding="utf-8")
            offsets = idaw.log_offsets([log])
            with log.open("a", encoding="utf-8") as handle:
                handle.write("current run completed normally\n")

            self.assertFalse(idaw.log_contains_unpacked_database_prompt([log], offsets))

    def test_log_contains_unpacked_database_prompt_detects_new_content(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            log = Path(temp_dir) / "ida.log"
            log.write_text("old run completed normally\n", encoding="utf-8")
            offsets = idaw.log_offsets([log])
            with log.open("a", encoding="utf-8") as handle:
                handle.write("IDA has found unpacked version of database C:\\X\\MAGIC.idb\n")

            self.assertTrue(idaw.log_contains_unpacked_database_prompt([log], offsets))

    def test_log_contains_unpacked_database_prompt(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            log = Path(temp_dir) / "ida.log"
            log.write_text("IDA has found unpacked version of database C:\\X\\MAGIC.idb\n", encoding="utf-8")

            self.assertTrue(idaw.log_contains_unpacked_database_prompt([log]))


if __name__ == "__main__":
    unittest.main()
