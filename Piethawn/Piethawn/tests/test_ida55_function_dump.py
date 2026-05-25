from __future__ import annotations

import hashlib
import tempfile
import unittest
from pathlib import Path

from ida_proc_data_export.ida55_function_dump import parse_dump


class Ida55FunctionDumpTests(unittest.TestCase):
    def test_parse_dump_combines_bytes_hashes_and_color_fields(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            dump_path = Path(temp_dir) / "MAGIC.idb.ida55-functions.txt"
            dump_path.write_text(
                "\n".join(
                    [
                        "idb_path=C:\\STU_DASM\\IDA55_MOM131\\MAGIC.idb",
                        "default_color=4294967295",
                        "default_color_hex=FFFFFFFF",
                        "function[0].name=Example",
                        "function[0].segment=seg001",
                        "function[0].entry_ea=seg001:0000",
                        "function[0].start_ea=seg001:0000",
                        "function[0].end_ea=seg001:0002",
                        "function[0].size=2",
                        "function[0].funcattr_color=16777088",
                        "function[0].funcattr_color_hex=00FFFF80",
                        "function[0].item_color=16777088",
                        "function[0].item_color_hex=00FFFF80",
                        "function[0].has_user_color=1",
                        "function[0].chunk[0].start=seg001:0000",
                        "function[0].chunk[0].end=seg001:0002",
                        "function[0].chunk[0].size=2",
                        "function[0].chunk[0].bytes=B800",
                        "function_count=1",
                        "",
                    ]
                ),
                encoding="utf-8",
            )

            payload = parse_dump(dump_path)

        function = payload["functions"][0]
        self.assertEqual(function["bytes_hex"], "B800")
        self.assertEqual(function["bytes_sha256"], hashlib.sha256(bytes.fromhex("B800")).hexdigest())
        self.assertEqual(function["chunk_bytes_sha256"], [hashlib.sha256(bytes.fromhex("B800")).hexdigest()])
        self.assertEqual(function["funcattr_color_hex"], "00FFFF80")
        self.assertEqual(function["item_color_hex"], "00FFFF80")
        self.assertIs(function["has_user_color"], True)
        self.assertIs(function["is_default_color"], False)

    def test_parse_dump_marks_default_color(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            dump_path = Path(temp_dir) / "WIZARDS.idb.ida55-functions.txt"
            dump_path.write_text(
                "\n".join(
                    [
                        "function[0].name=DefaultColor",
                        "function[0].item_color_hex=FFFFFFFF",
                        "function[0].has_user_color=0",
                        "function_count=1",
                        "",
                    ]
                ),
                encoding="utf-8",
            )

            payload = parse_dump(dump_path)

        function = payload["functions"][0]
        self.assertIs(function["has_user_color"], False)
        self.assertIs(function["is_default_color"], True)


if __name__ == "__main__":
    unittest.main()
