from __future__ import annotations

import unittest

from piethawn_sync.ida_snapshot import convert_int_fields, ea_sort_key, int_field, parse_snapshot_text, required_field


def sample_dump() -> str:
    return "\n".join(
        [
            r"idb_path=C:\STU_DASM\IDA55_MOM131\MAGIC.idb",
            "input_file=MAGIC.EXE",
            "function[1].name=Second",
            "function[1].size=98",
            "function[0].name=First",
            "function[0].size=149",
            "segment[0].name=ovr056",
            "function_count=2",
            "",
        ]
    )


class IdaSnapshotParserTests(unittest.TestCase):
    def test_parse_snapshot_text_splits_meta_and_indexed_sections(self) -> None:
        meta, sections = parse_snapshot_text(sample_dump())

        self.assertEqual(meta["input_file"], "MAGIC.EXE")
        self.assertEqual(meta["function_count"], "2")
        self.assertEqual(sorted(sections), ["function", "segment"])
        self.assertEqual([record["name"] for record in sections["function"]], ["First", "Second"])

    def test_parse_snapshot_text_keeps_equals_signs_in_values(self) -> None:
        meta, _ = parse_snapshot_text("note=a=b\n")

        self.assertEqual(meta["note"], "a=b")

    def test_parse_snapshot_text_rejects_non_key_value_line_counterexample(self) -> None:
        with self.assertRaisesRegex(ValueError, "Line 2 is not key=value"):
            parse_snapshot_text("input_file=MAGIC.EXE\nnot a record\n")

    def test_parse_snapshot_text_nests_chunk_records_under_functions(self) -> None:
        meta, sections = parse_snapshot_text(
            "\n".join(
                [
                    "function_count=1",
                    "function[0].name=Example",
                    "function[0].chunk[1].bytes=CB",
                    "function[0].chunk[0].bytes=B800",
                    "function[0].chunk_count=2",
                ]
            )
        )

        self.assertEqual(meta, {"function_count": "1"})
        function = sections["function"][0]
        self.assertEqual(function["chunk_count"], "2")
        self.assertEqual([chunk["bytes"] for chunk in function["chunk"]], ["B800", "CB"])

    def test_parse_snapshot_text_keeps_chunk_keys_out_of_meta_counterexample(self) -> None:
        # Regression: nested keys used to fall through into meta (4832 stray entries for MAGIC).
        meta, _ = parse_snapshot_text("function[0].chunk[0].start=seg001:0000\n")

        self.assertEqual(meta, {})

    def test_parse_snapshot_text_keeps_values_as_strings(self) -> None:
        _, sections = parse_snapshot_text("function[0].size=149\nfunction[0].item_color_hex=00808080\n")

        self.assertEqual(sections["function"][0], {"size": "149", "item_color_hex": "00808080"})

    def test_convert_int_fields_converts_only_listed_fields(self) -> None:
        meta, sections = parse_snapshot_text(
            "function_count=1\ninput_file=MAGIC.EXE\n"
            "function[0].size=149\nfunction[0].item_color_hex=00808080\nfunction[0].chunk[0].size=2\n"
        )

        convert_int_fields(meta, sections, {"": {"function_count"}, "function": {"size"}, "function.chunk": {"size"}})

        self.assertEqual(meta, {"function_count": 1, "input_file": "MAGIC.EXE"})
        function = sections["function"][0]
        self.assertEqual(function["size"], 149)
        self.assertEqual(function["item_color_hex"], "00808080")
        self.assertEqual(function["chunk"][0]["size"], 2)

    def test_convert_int_fields_rejects_non_integer_counterexample(self) -> None:
        meta, sections = parse_snapshot_text("function[0].size=lots\n")

        with self.assertRaisesRegex(ValueError, r"function\[0\]\.size is not an integer: 'lots'"):
            convert_int_fields(meta, sections, {"function": {"size"}})

    def test_ea_sort_key_treats_ida_offsets_as_hex(self) -> None:
        self.assertLess(ea_sort_key("2FD3"), ea_sort_key("3035"))
        self.assertEqual(ea_sort_key("0:000553AA"), 0x553AA)
        self.assertEqual(ea_sort_key("1000:01A7"), 0x01A7)
        self.assertEqual(ea_sort_key("0x10"), 0x10)

    def test_ea_sort_key_unparseable_counterexample(self) -> None:
        self.assertEqual(ea_sort_key(None), 0)
        self.assertEqual(ea_sort_key("not-hex"), 0)

    def test_required_field_missing_counterexample(self) -> None:
        with self.assertRaisesRegex(ValueError, "Procedure #3 is missing required field: name"):
            required_field({"name": ""}, "name", 3)

    def test_int_field_non_integer_counterexample(self) -> None:
        self.assertEqual(int_field({"size": "149"}, "size", 1), 149)
        with self.assertRaisesRegex(ValueError, "non-integer field: size"):
            int_field({"size": "not-a-number"}, "size", 1)


if __name__ == "__main__":
    unittest.main()
