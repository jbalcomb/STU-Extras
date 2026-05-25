from __future__ import annotations

import json
import tempfile
import unittest
from pathlib import Path

from piethawn_sync.ida_functions import build_inventory, ea_sort_key, load_inventory
from wzd_mgc_inventory import default_output_path


def sample_dump() -> dict[str, object]:
    return {
        "meta": {
            "idb_path": r"C:\STU_DASM\IDA55_MOM131\MAGIC.idb",
            "input_file": "MAGIC.EXE",
        },
        "functions": [
            {
                "name": "Second",
                "segment": "seg001",
                "start_ea": "0:00000020",
                "end_ea": "0:00000030",
                "size": 16,
                "bytes_sha256": "bbb",
                "chunk_bytes_sha256": ["bbb"],
            },
            {
                "name": "First",
                "segment": "seg001",
                "start_ea": "0:00000010",
                "end_ea": "0:00000020",
                "size": 16,
                "bytes_sha256": "aaa",
                "chunk_bytes_sha256": ["aaa"],
            },
            {
                "name": "OtherSegment",
                "segment": "seg002",
                "start_ea": "4",
                "end_ea": 8,
                "size": 4,
            },
        ],
    }


class WzdMgcInventoryTests(unittest.TestCase):
    def test_ea_sort_key_handles_ida_text_and_integer_values(self) -> None:
        self.assertLess(ea_sort_key("0:00000010"), ea_sort_key("0:00000020"))
        self.assertEqual(ea_sort_key(12), 0x12)
        self.assertEqual(ea_sort_key("3035"), 0x3035)

    def test_build_inventory_sorts_and_assigns_segment_ordinals(self) -> None:
        inventory = build_inventory(sample_dump(), "MGC", Path("MAGIC.idb.ida55-functions.json"))

        self.assertEqual(inventory.database, "MGC")
        self.assertEqual(inventory.function_count, 3)
        self.assertEqual([item.name for item in inventory.procedures], ["First", "Second", "OtherSegment"])
        self.assertEqual([item.procedure_ordinal for item in inventory.procedures], [1, 2, 1])
        self.assertEqual(inventory.procedures[2].bytes_sha256, None)

    def test_inventory_to_dict_is_stable_json_shape(self) -> None:
        document = build_inventory(sample_dump(), "WZD", Path("WIZARDS.json")).to_dict()

        self.assertEqual(document["schema_version"], 1)
        self.assertEqual(document["artifact"], "procedure_inventory")
        self.assertEqual(document["database"], "WZD")
        self.assertEqual(document["procedures"][0]["name"], "First")

    def test_load_inventory_reads_json_file(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            path = Path(temp_dir) / "functions.json"
            path.write_text(json.dumps(sample_dump()), encoding="utf-8")

            inventory = load_inventory(path, "MGC")

            self.assertEqual(inventory.function_count, 3)

    def test_missing_required_name_counterexample(self) -> None:
        data = sample_dump()
        data["functions"][0].pop("name")

        with self.assertRaisesRegex(ValueError, "missing required field: name"):
            build_inventory(data, "MGC", Path("bad.json"))

    def test_non_list_chunk_hash_counterexample(self) -> None:
        data = sample_dump()
        data["functions"][0]["chunk_bytes_sha256"] = "not-a-list"

        with self.assertRaisesRegex(ValueError, "non-list field: chunk_bytes_sha256"):
            build_inventory(data, "MGC", Path("bad.json"))

    def test_default_output_path_uses_lowercase_database_label(self) -> None:
        self.assertEqual(default_output_path("MGC"), Path("out") / "wzd-mgc-sync" / "mgc-procedure-inventory.json")


if __name__ == "__main__":
    unittest.main()
