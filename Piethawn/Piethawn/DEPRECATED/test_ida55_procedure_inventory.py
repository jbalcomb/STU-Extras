from __future__ import annotations

import tempfile
import unittest
from pathlib import Path

from piethawn_sync.ida_snapshot import build_inventory_from_snapshot_text, ea_sort_key, parse_snapshot_text


def sample_snapshot() -> str:
    return "\n".join(
        [
            "snapshot_schema=ida55-procedure-snapshot-v1",
            r"idb_path=C:\STU_DASM\IDA55_MOM131\MAGIC.idb",
            "input_file=MAGIC.EXE",
            "procedure[0].name=Later",
            "procedure[0].segment=ovr056",
            "procedure[0].entry_ea=3035",
            "procedure[0].start_ea=3035",
            "procedure[0].end_ea=0:000553AA",
            "procedure[0].size=149",
            "procedure[0].flags=17426",
            "procedure[1].name=Earlier",
            "procedure[1].segment=ovr056",
            "procedure[1].entry_ea=2FD3",
            "procedure[1].start_ea=2FD3",
            "procedure[1].end_ea=3035",
            "procedure[1].size=98",
            "procedure[1].flags=17426",
            "procedure[2].name=OtherSegment",
            "procedure[2].segment=ovr057",
            "procedure[2].entry_ea=0000",
            "procedure[2].start_ea=0000",
            "procedure[2].end_ea=0010",
            "procedure[2].size=16",
            "procedure[2].flags=6",
            "procedure_count=3",
            "",
        ]
    )


class Ida55ProcedureInventoryTests(unittest.TestCase):
    def test_parse_snapshot_text_splits_meta_and_indexed_sections(self) -> None:
        meta, sections = parse_snapshot_text(sample_snapshot())

        self.assertEqual(meta["snapshot_schema"], "ida55-procedure-snapshot-v1")
        self.assertEqual(len(sections["procedure"]), 3)
        self.assertEqual(sections["procedure"][0]["name"], "Later")

    def test_ea_sort_key_treats_ida_offsets_as_hex(self) -> None:
        self.assertLess(ea_sort_key("2FD3"), ea_sort_key("3035"))
        self.assertEqual(ea_sort_key("0:000553AA"), 0x553AA)

    def test_build_inventory_assigns_ordinals_by_segment_and_hex_ea(self) -> None:
        inventory = build_inventory_from_snapshot_text(sample_snapshot(), "MGC", Path("snapshot.txt"))

        self.assertEqual(inventory.function_count, 3)
        self.assertEqual([record.name for record in inventory.procedures], ["Earlier", "Later", "OtherSegment"])
        self.assertEqual([record.procedure_ordinal for record in inventory.procedures], [1, 2, 1])
        self.assertEqual(inventory.procedures[1].start_ea, "3035")

    def test_missing_name_counterexample(self) -> None:
        text = sample_snapshot().replace("procedure[0].name=Later\n", "")

        with self.assertRaisesRegex(ValueError, "missing required field: name"):
            build_inventory_from_snapshot_text(text, "MGC", Path("bad.txt"))

    def test_bad_size_counterexample(self) -> None:
        text = sample_snapshot().replace("procedure[0].size=149", "procedure[0].size=not-a-number")

        with self.assertRaisesRegex(ValueError, "non-integer field: size"):
            build_inventory_from_snapshot_text(text, "MGC", Path("bad.txt"))

    def test_cli_input_can_be_read_from_file(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            path = Path(temp_dir) / "snapshot.txt"
            path.write_text(sample_snapshot(), encoding="utf-8")

            inventory = build_inventory_from_snapshot_text(path.read_text(encoding="utf-8"), "WZD", path)

            self.assertEqual(inventory.database, "WZD")
            self.assertEqual(inventory.idb_path, r"C:\STU_DASM\IDA55_MOM131\MAGIC.idb")


if __name__ == "__main__":
    unittest.main()
