from __future__ import annotations

import tempfile
import unittest
from pathlib import Path

from ida_automation import expand_duplicate_segments_to_procedures as expand


def proc(name: str, segment: str, ordinal: int):
    return {"name": name, "segment": segment, "procedure_ordinal": ordinal}


class ExpandDuplicateSegmentsToProceduresTests(unittest.TestCase):
    def test_read_segment_pairs_uses_two_column_fwv_format(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            path = Path(temp_dir) / "segments.fwv"
            path.write_text("MAGIC   WIZARDS\nseg001  seg101\nSOUND   SOUND\n", encoding="utf-8")

            self.assertEqual(expand.read_segment_pairs(path), [("seg001", "seg101"), ("SOUND", "SOUND")])

    def test_render_expanded_pairs_uses_procedure_ordinal(self) -> None:
        magic = {"seg001": [proc("m1", "seg001", 1), proc("m2", "seg001", 2)]}
        wizards = {"seg101": [proc("w1", "seg101", 1)]}

        pairs = expand.expected_procedure_pairs(magic, wizards, [("seg001", "seg101")])

        self.assertEqual(
            expand.render_fwv(pairs),
            "MAGIC        WIZARDS\ns001p001:m1  s101p001:w1\ns001p002:m2  s101p002:missing\n",
        )

    def test_non_numbered_segment_names_stay_readable(self) -> None:
        self.assertEqual(expand.procedure_id(proc("music", "SOUND", 1), "SOUND", 1), "SOUNDp001:music")


if __name__ == "__main__":
    unittest.main()
