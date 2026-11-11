from __future__ import annotations

import unittest
from pathlib import Path

from ida_proc_data_export.ida55_functions_to_inventory import build_inventory_from_functions_text


def sample_function_dump() -> str:
    return "\n".join(
        [
            r"idb_path=C:\STU_DASM\IDA55_MOM131\MAGIC.idb",
            "input_file=MAGIC.EXE",
            "function[0].name=Later",
            "function[0].segment=ovr056",
            "function[0].entry_ea=3035",
            "function[0].start_ea=3035",
            "function[0].end_ea=0:000553AA",
            "function[0].size=149",
            "function[0].flags=17426",
            "function[0].chunk_count=1",
            "function[0].bytes=55CB8BEC",
            "function[1].name=Earlier",
            "function[1].segment=ovr056",
            "function[1].entry_ea=2FD3",
            "function[1].start_ea=2FD3",
            "function[1].end_ea=3035",
            "function[1].size=98",
            "function[1].flags=17426",
            "function[1].item_color_hex=FFFFFFFF",
            "function[2].name=OtherSegment",
            "function[2].segment=ovr057",
            "function[2].entry_ea=0000",
            "function[2].start_ea=0000",
            "function[2].end_ea=0010",
            "function[2].size=16",
            "function[2].flags=6",
            "function_count=3",
            "segment[0].name=ovr056",
            "segment[0].start=2FD3",
            "segment[0].end=3035",
            "segment_count=1",
            "",
        ]
    )


class Ida55FunctionsToInventoryTests(unittest.TestCase):
    def test_inventory_matches_snapshot_derived_shape(self) -> None:
        inventory = build_inventory_from_functions_text(sample_function_dump(), "MGC", Path("MAGIC.idb.ida55-functions.txt"))
        record = inventory.procedures[0].to_dict()

        self.assertEqual(
            sorted(record.keys()),
            ["database", "end_ea", "name", "procedure_ordinal", "segment", "size", "start_ea"],
        )

    def test_ordinals_assigned_by_segment_and_hex_ea(self) -> None:
        inventory = build_inventory_from_functions_text(sample_function_dump(), "MGC", Path("f.txt"))

        self.assertEqual(inventory.function_count, 3)
        self.assertEqual([record.name for record in inventory.procedures], ["Earlier", "Later", "OtherSegment"])
        self.assertEqual([record.procedure_ordinal for record in inventory.procedures], [1, 2, 1])

    def test_extra_function_dump_fields_are_ignored(self) -> None:
        # bytes/chunk/color columns present in the function dump must not break inventory building.
        inventory = build_inventory_from_functions_text(sample_function_dump(), "MGC", Path("f.txt"))

        self.assertEqual(inventory.idb_path, r"C:\STU_DASM\IDA55_MOM131\MAGIC.idb")
        self.assertEqual(inventory.procedures[1].name, "Later")

    def test_missing_size_is_a_counterexample(self) -> None:
        text = sample_function_dump().replace("function[0].size=149", "function[0].size=not-a-number")

        with self.assertRaisesRegex(ValueError, "non-integer field: size"):
            build_inventory_from_functions_text(text, "MGC", Path("bad.txt"))


if __name__ == "__main__":
    unittest.main()
