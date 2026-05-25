from __future__ import annotations

import unittest

from piethawn_sync.procedure_compare import (
    compare_inventories,
    default_same_segment_pairs,
    load_segment_pairs_from_map,
)


def inventory(database: str, procedures: list[dict[str, object]]) -> dict[str, object]:
    return {
        "schema_version": 1,
        "artifact": "procedure_inventory",
        "database": database,
        "procedures": procedures,
    }


def procedure(name: str, segment: str, ordinal: int, size: int = 4) -> dict[str, object]:
    return {
        "database": "DB",
        "name": name,
        "segment": segment,
        "procedure_ordinal": ordinal,
        "start_ea": f"{segment}:{ordinal:04X}",
        "end_ea": f"{segment}:{ordinal + 1:04X}",
        "size": size,
    }


class Ida55ProcedureCompareTests(unittest.TestCase):
    def test_default_same_segment_pairs_uses_intersection_only(self) -> None:
        pairs = default_same_segment_pairs(["seg001", "left_only"], ["seg001", "right_only"])

        self.assertEqual(len(pairs), 1)
        self.assertEqual(pairs[0]["left_segment"], "seg001")
        self.assertEqual(pairs[0]["right_segment"], "seg001")

    def test_compare_pairs_by_mapped_segment_and_ordinal(self) -> None:
        left = inventory(
            "MGC",
            [
                procedure("Same", "seg001", 1),
                procedure("OldName", "seg001", 2),
                procedure("LeftExtra", "seg001", 3),
                procedure("LeftOnlySegment", "seg999", 1),
            ],
        )
        right = inventory(
            "WZD",
            [
                procedure("Same", "seg001", 1),
                procedure("NewName", "seg001", 2),
            ],
        )

        report = compare_inventories(left, right)

        self.assertEqual(report["summary"]["procedure_pair_count"], 2)
        self.assertEqual(report["summary"]["different_name_observation_count"], 1)
        self.assertEqual(report["procedure_pairs"][1]["left"]["name"], "OldName")
        self.assertEqual(report["procedure_pairs"][1]["right"]["name"], "NewName")
        self.assertEqual(report["procedure_pairs"][1]["pairing_rule"], "mapped_segment_and_procedure_ordinal")
        self.assertEqual(report["summary"]["left_unpaired_count"], 2)
        self.assertEqual({item["reason"] for item in report["left_unpaired"]}, {"no_right_ordinal_counterpart", "no_segment_pair"})

    def test_segment_map_maps_different_segment_ids(self) -> None:
        left = inventory("MGC", [procedure("LeftName", "seg026", 1)])
        right = inventory("WZD", [procedure("RightName", "seg028", 1)])
        segment_map = {
            "segment_pairs": [
                {
                    "mgc_segment": "seg026",
                    "wzd_segment": "seg028",
                    "relationship": "manual_test",
                }
            ]
        }

        report = compare_inventories(
            left,
            right,
            load_segment_pairs_from_map(segment_map, "MGC", "WZD"),
        )

        self.assertEqual(report["summary"]["procedure_pair_count"], 1)
        self.assertEqual(report["procedure_pairs"][0]["segment_pair"]["relationship"], "manual_test")
        self.assertEqual(report["procedure_pairs"][0]["segment_pair"]["left_segment"], "seg026")
        self.assertEqual(report["procedure_pairs"][0]["segment_pair"]["right_segment"], "seg028")

    def test_evidence_difference_counterexample(self) -> None:
        left = inventory("MGC", [procedure("SameName", "seg001", 1, size=4)])
        right = inventory("WZD", [procedure("SameName", "seg001", 1, size=8)])

        report = compare_inventories(left, right)

        self.assertEqual(report["summary"]["evidence_difference_count"], 1)
        self.assertEqual(report["evidence_differences"][0]["observations"]["differing_fields"], ["size"])
        self.assertEqual(report["evidence_differences"][0]["observations"]["left_evidence"]["size"], 4)

    def test_line_count_and_hashes_are_recorded_when_present(self) -> None:
        left_proc = procedure("SameName", "seg001", 1)
        right_proc = procedure("SameName", "seg001", 1)
        left_proc["line_count"] = 9
        right_proc["line_count"] = 9
        left_proc["bytes_sha256"] = "abc"
        right_proc["bytes_sha256"] = "def"
        left = inventory("MGC", [left_proc])
        right = inventory("WZD", [right_proc])

        report = compare_inventories(left, right)
        observations = report["procedure_pairs"][0]["observations"]

        self.assertIn("line_count", observations["matching_fields"])
        self.assertIn("bytes_sha256", observations["differing_fields"])


if __name__ == "__main__":
    unittest.main()
