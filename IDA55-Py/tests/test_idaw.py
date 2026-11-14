from __future__ import annotations

import tempfile
from contextlib import redirect_stdout
from io import StringIO
import unittest
from datetime import datetime, timezone
from pathlib import Path
from unittest.mock import patch

from ida55py import idaw


NO_IDA_RUNNING = '"chrome.exe","999","Console","1","1 K"'
IDA_GUI_RUNNING = '"idag.exe","16028","Console","1","69,744 K"'


class WorkCopyTests(unittest.TestCase):
    """--work-dir: IDA runs on a copy; the real IDB is never modified. Powered by Claude."""

    def make_idb(self, root: Path) -> Path:
        """Create a fake packed IDB under root/ida. Powered by Claude."""
        idb = root / "ida" / "MAGIC.idb"
        idb.parent.mkdir()
        idb.write_bytes(b"real-idb-data")
        return idb

    def run_main(self, argv: list[str], fake_ida=None) -> tuple[object, str]:
        """Run idaw.main with no IDA process running and an optional fake IDA. Powered by Claude."""
        output = StringIO()
        with redirect_stdout(output), patch.object(idaw, "probe_process_list", return_value=NO_IDA_RUNNING):
            if fake_ida is None:
                return idaw.main(argv), output.getvalue()
            with patch.object(idaw.subprocess, "run", side_effect=fake_ida):
                return idaw.main(argv), output.getvalue()

    def test_prepare_work_copy_copies_idb(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            idb = self.make_idb(Path(temp_dir))
            work = Path(temp_dir) / "work"
            copy = idaw.prepare_work_copy(idb, work)
            self.assertEqual(copy, work / "MAGIC.idb")
            self.assertEqual(copy.read_bytes(), b"real-idb-data")

    def test_prepare_work_copy_rejects_non_empty_dir_counterexample(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            idb = self.make_idb(Path(temp_dir))
            work = Path(temp_dir) / "work"
            work.mkdir()
            (work / "leftover.txt").write_text("x", encoding="utf-8")
            with self.assertRaisesRegex(idaw.IdawError, "not empty"):
                idaw.prepare_work_copy(idb, work)

    def test_prepare_work_copy_rejects_zero_byte_idb_counterexample(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            idb = self.make_idb(Path(temp_dir))
            idb.write_bytes(b"")
            with self.assertRaisesRegex(idaw.IdawError, "0-byte"):
                idaw.prepare_work_copy(idb, Path(temp_dir) / "work")

    def test_replace_idb_argument(self) -> None:
        self.assertEqual(
            idaw.replace_idb_argument(["-A", "-Sx.idc", "C:/ida/MAGIC.idb"], Path("W/MAGIC.idb")),
            ["-A", "-Sx.idc", str(Path("W/MAGIC.idb"))],
        )

    def test_dry_run_creates_nothing(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            idb = self.make_idb(Path(temp_dir))
            work = Path(temp_dir) / "work"
            result, text = self.run_main(["--dry-run", "--work-dir", str(work), "--", "-A", "-Sx.idc", str(idb)])
            self.assertEqual(result, 0)
            self.assertIn("Work:", text)
            self.assertIn(str(work / "MAGIC.idb"), text)
            self.assertFalse(work.exists())

    def test_ida_runs_on_copy_and_original_is_untouched(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            idb = self.make_idb(Path(temp_dir))
            work = Path(temp_dir) / "work"
            seen = {}

            def fake_ida(command, *args, **kwargs):
                seen["command"] = command
                Path(command[-1]).with_name("MAGIC.idb.out.txt").write_text("dump", encoding="utf-8")
                return idaw.subprocess.CompletedProcess(command, 0)

            result, _ = self.run_main(["--ida-exe", "X:/idaw.exe", "--work-dir", str(work), "--", "-A", "-Sx.idc", str(idb)], fake_ida)
            self.assertEqual(result, 0)
            self.assertEqual(seen["command"][-1], str(work / "MAGIC.idb"))
            self.assertEqual(idb.read_bytes(), b"real-idb-data")
            self.assertTrue((work / "MAGIC.idb.out.txt").exists())
            self.assertEqual(sorted(p.name for p in idb.parent.iterdir()), ["MAGIC.idb"])

    def test_damaged_copy_raises_and_original_is_untouched(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            idb = self.make_idb(Path(temp_dir))
            work = Path(temp_dir) / "work"

            def fake_ida_that_breaks_the_copy(command, *args, **kwargs):
                work_idb = Path(command[-1])
                work_idb.write_bytes(b"")
                work_idb.with_suffix(".id0").write_bytes(b"part")
                return idaw.subprocess.CompletedProcess(command, 0)

            with redirect_stdout(StringIO()), patch("sys.stderr", new_callable=StringIO):
                with self.assertRaisesRegex(idaw.IdawError, "0-byte work copy"):
                    self.run_main(["--work-dir", str(work), "--", "-A", "-Sx.idc", str(idb)], fake_ida_that_breaks_the_copy)
            self.assertEqual(idb.read_bytes(), b"real-idb-data")
            self.assertFalse(idaw.find_unpacked_parts(idb))

    def fake_ida_renaming_copy(self, exit_code: int = 0):
        """A fake IDA that changes the work copy's bytes, as a saving rename script would. Powered by Claude."""
        def fake(command, *args, **kwargs):
            Path(command[-1]).write_bytes(b"renamed-idb-data")
            return idaw.subprocess.CompletedProcess(command, exit_code)
        return fake

    def test_write_back_replaces_real_idb_and_backs_it_up(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            idb = self.make_idb(Path(temp_dir))
            work, backups = Path(temp_dir) / "work", Path(temp_dir) / "backups"
            with patch("sys.stderr", new_callable=StringIO):
                result, text = self.run_main(
                    ["--work-dir", str(work), "--write-back", "--backup-dir", str(backups), "--", "-A", "-Sx.idc", str(idb)],
                    self.fake_ida_renaming_copy(),
                )
            self.assertEqual(result, 0)
            self.assertEqual(idb.read_bytes(), b"renamed-idb-data")
            backup_files = list(backups.iterdir())
            self.assertEqual(len(backup_files), 1)
            self.assertEqual(backup_files[0].read_bytes(), b"real-idb-data")
            self.assertIn("Written back:", text)
            self.assertEqual(sorted(p.name for p in idb.parent.iterdir()), ["MAGIC.idb"])

    def test_write_back_skipped_when_ida_exits_non_zero_counterexample(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            idb = self.make_idb(Path(temp_dir))
            work = Path(temp_dir) / "work"
            with patch("sys.stderr", new_callable=StringIO) as err:
                result, _ = self.run_main(
                    ["--work-dir", str(work), "--write-back", "--", "-A", "-Sx.idc", str(idb)],
                    self.fake_ida_renaming_copy(exit_code=1),
                )
            self.assertEqual(result, 1)
            self.assertEqual(idb.read_bytes(), b"real-idb-data")
            self.assertTrue((work / "MAGIC.idb").exists())
            self.assertIn("NOT WRITTEN BACK", err.getvalue())

    def test_write_back_refused_when_real_idb_changed_meanwhile_counterexample(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            idb = self.make_idb(Path(temp_dir))
            work = Path(temp_dir) / "work"

            def fake(command, *args, **kwargs):
                Path(command[-1]).write_bytes(b"renamed-idb-data")
                idb.write_bytes(b"someone-saved-the-real-idb")
                return idaw.subprocess.CompletedProcess(command, 0)

            with self.assertRaisesRegex(idaw.IdawError, "changed while IDA worked"):
                self.run_main(["--work-dir", str(work), "--write-back", "--", "-A", "-Sx.idc", str(idb)], fake)
            self.assertEqual(idb.read_bytes(), b"someone-saved-the-real-idb")

    def test_write_back_without_work_dir_is_rejected_counterexample(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            idb = self.make_idb(Path(temp_dir))
            with self.assertRaisesRegex(idaw.IdawError, "needs --work-dir"):
                self.run_main(["--write-back", "--", "-A", str(idb)])

    def test_work_dir_with_backup_dir_is_rejected_counterexample(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            idb = self.make_idb(Path(temp_dir))
            with self.assertRaisesRegex(idaw.IdawError, "cannot be used together"):
                self.run_main(["--work-dir", str(Path(temp_dir) / "w"), "--backup-dir", temp_dir, "--", "-A", str(idb)])


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
        result = idaw.backup_path_for_idb(Path("/ida/MAGIC.idb"), "202604302134", Path("/backups"))
        self.assertEqual(result, Path("/backups") / "MAGIC-202604302134.idb")

    def test_resolve_backup_dir_prefers_cli_then_env_then_idb_folder(self) -> None:
        idb = Path("/ida/MAGIC.idb")
        with patch.dict(idaw.os.environ, {idaw.BACKUP_DIR_ENV: "/from-env"}):
            self.assertEqual(idaw.resolve_backup_dir("/from-cli", idb), idaw.normalize_path("/from-cli"))
            self.assertEqual(idaw.resolve_backup_dir(None, idb), idaw.normalize_path("/from-env"))
        with patch.dict(idaw.os.environ, {}, clear=True):
            self.assertEqual(idaw.resolve_backup_dir(None, idb), Path("/ida"))

    def test_resolve_ida_exe_prefers_cli_then_env_then_default(self) -> None:
        with patch.dict(idaw.os.environ, {idaw.IDA_EXE_ENV: "/env/idaw.exe"}):
            self.assertEqual(idaw.resolve_ida_exe("/cli/idaw.exe"), idaw.normalize_path("/cli/idaw.exe"))
            self.assertEqual(idaw.resolve_ida_exe(None), idaw.normalize_path("/env/idaw.exe"))
        with patch.dict(idaw.os.environ, {}, clear=True):
            self.assertEqual(idaw.resolve_ida_exe(None), idaw.normalize_path(idaw.DEFAULT_IDA_EXE))

    def test_main_dry_run_uses_cli_ida_exe_and_backup_dir(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            idb = Path(temp_dir) / "MAGIC.idb"
            idb.write_bytes(b"idb-data")
            backup_dir = Path(temp_dir) / "backups"
            output = StringIO()

            with redirect_stdout(output), patch.object(idaw, "probe_process_list", return_value=NO_IDA_RUNNING):
                result = idaw.main(
                    ["--dry-run", "--ida-exe", "X:/ida/idag.exe", "--backup-dir", str(backup_dir),
                     "--", "-A", "-Sscript.idc", str(idb)]
                )

            self.assertEqual(result, 0)
            text = output.getvalue()
            self.assertIn(str(backup_dir / "MAGIC-"), text)
            self.assertIn("idag.exe", text)
            self.assertFalse(backup_dir.exists())

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

            with redirect_stdout(output), patch.object(idaw, "probe_process_list", return_value=NO_IDA_RUNNING), \
                    patch.dict(idaw.os.environ, {}, clear=True):
                result = idaw.main(["--dry-run", "--", "-A", "-Sscript.idc", str(idb)])

            self.assertEqual(result, 0)
            text = output.getvalue()
            self.assertIn("Backup:", text)
            self.assertIn(str(Path(temp_dir) / "MAGIC-"), text)
            self.assertIn("idaw.exe", text)
            self.assertIn(str(idb), text)


    def test_find_running_ida_processes_matches_gui_instance(self) -> None:
        with patch.object(idaw, "probe_process_list", return_value=f"{IDA_GUI_RUNNING}\n{NO_IDA_RUNNING}"):
            self.assertEqual(idaw.find_running_ida_processes(), [IDA_GUI_RUNNING])

    def test_find_running_ida_processes_ignores_unrelated_processes(self) -> None:
        with patch.object(idaw, "probe_process_list", return_value=NO_IDA_RUNNING):
            self.assertEqual(idaw.find_running_ida_processes(), [])

    def test_find_unpacked_parts_detects_expanded_database(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            idb = Path(temp_dir) / "MAGIC.idb"
            idb.write_bytes(b"")
            for suffix in (".id0", ".id1"):
                idb.with_suffix(suffix).write_bytes(b"part")

            names = sorted(part.name for part in idaw.find_unpacked_parts(idb))

            self.assertEqual(names, ["MAGIC.id0", "MAGIC.id1"])

    def test_find_unpacked_parts_returns_empty_for_packed_database(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            idb = Path(temp_dir) / "MAGIC.idb"
            idb.write_bytes(b"idb-data")

            self.assertEqual(idaw.find_unpacked_parts(idb), [])

    def test_preflight_accepts_packed_database_with_no_ida_running(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            idb = Path(temp_dir) / "MAGIC.idb"
            idb.write_bytes(b"idb-data")

            with redirect_stdout(StringIO()), patch.object(idaw, "probe_process_list", return_value=NO_IDA_RUNNING):
                idaw.preflight_checks(idb, False, False)

    def test_preflight_rejects_concurrent_ida_session_counterexample(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            idb = Path(temp_dir) / "MAGIC.idb"
            idb.write_bytes(b"idb-data")

            with redirect_stdout(StringIO()), patch.object(idaw, "probe_process_list", return_value=IDA_GUI_RUNNING):
                with self.assertRaisesRegex(idaw.IdawError, "already running"):
                    idaw.preflight_checks(idb, False, False)

    def test_preflight_rejects_unpacked_parts_counterexample(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            idb = Path(temp_dir) / "MAGIC.idb"
            idb.write_bytes(b"idb-data")
            idb.with_suffix(".id0").write_bytes(b"part")

            with redirect_stdout(StringIO()), patch.object(idaw, "probe_process_list", return_value=NO_IDA_RUNNING):
                with self.assertRaisesRegex(idaw.IdawError, "unpacked database parts"):
                    idaw.preflight_checks(idb, False, False)

    def test_preflight_allow_flags_bypass_both_checks(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            idb = Path(temp_dir) / "MAGIC.idb"
            idb.write_bytes(b"idb-data")
            idb.with_suffix(".id0").write_bytes(b"part")

            with redirect_stdout(StringIO()), patch.object(idaw, "probe_process_list", return_value=IDA_GUI_RUNNING):
                idaw.preflight_checks(idb, True, True)

    def test_probe_process_list_fails_closed_when_listing_unavailable_counterexample(self) -> None:
        with patch.object(idaw.subprocess, "run", side_effect=OSError("tasklist missing")):
            with self.assertRaisesRegex(idaw.IdawError, "Could not list running processes"):
                idaw.probe_process_list()

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
