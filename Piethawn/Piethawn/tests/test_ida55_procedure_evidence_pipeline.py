from __future__ import annotations

import unittest

from ida55_asm_body_evidence import build_asm_body_evidence
from ida_proc_name_sync_prereq.compare_procedure_evidence import compare_procedure_evidence
from ida55_join_procedure_evidence import join_procedure_evidence


ASM_SAMPLE = """segment seg001 byte public 'CODE' use16
proc First near
mov     ax, 1
loc_1000:
ret
endp First
proc Second near
mov     bx, 2
ret
endp Second
seg001 ends
segment seg003 byte public 'CODE' use16
proc Third near
mov     cx, 3
ret
endp Third
seg003 ends
"""


def inventory(database: str) -> dict[str, object]:
    return {
        "schema_version": 1,
        "artifact": "procedure_inventory",
        "database": database,
        "source_path": f"{database}.snapshot.txt",
        "procedures": [
            {
                "database": database,
                "name": "First",
                "segment": "seg001",
                "procedure_ordinal": 1,
                "start_ea": "seg001:0000",
                "end_ea": "seg001:0004",
                "size": 4,
            },
            {
                "database": database,
                "name": "Second",
                "segment": "seg001",
                "procedure_ordinal": 2,
                "start_ea": "seg001:0004",
                "end_ea": "seg001:0008",
                "size": 4,
            },
        ],
    }


class Ida55ProcedureEvidencePipelineTests(unittest.TestCase):
    def test_asm_body_evidence_tracks_segment_ordinals_and_hashes(self) -> None:
        evidence = build_asm_body_evidence(ASM_SAMPLE, "MGC", "sample.asm", "_none_")

        self.assertEqual(evidence["procedure_count"], 3)
        self.assertEqual(evidence["procedures"][0]["segment"], "seg001")
        self.assertEqual(evidence["procedures"][0]["procedure_ordinal"], 1)
        self.assertEqual(evidence["procedures"][1]["procedure_ordinal"], 2)
        self.assertEqual(evidence["procedures"][2]["segment"], "seg003")
        self.assertEqual(len(evidence["procedures"][0]["normalized_body_sha256"]), 64)
        self.assertGreater(evidence["procedures"][0]["instruction_count"], 0)

    def test_join_requires_matching_inventory_and_asm_body_records(self) -> None:
        evidence = build_asm_body_evidence(ASM_SAMPLE, "MGC", "sample.asm", "_none_")

        with self.assertRaises(ValueError):
            join_procedure_evidence(inventory("MGC"), evidence)

    def test_join_succeeds_when_inventory_and_asm_body_records_match(self) -> None:
        evidence = build_asm_body_evidence(ASM_SAMPLE, "MGC", "sample.asm", "_none_")
        exact_inventory = inventory("MGC")
        exact_inventory["procedures"].append(
            {
                "database": "MGC",
                "name": "Third",
                "segment": "seg003",
                "procedure_ordinal": 1,
                "start_ea": "seg003:0000",
                "end_ea": "seg003:0004",
                "size": 4,
            }
        )

        joined = join_procedure_evidence(exact_inventory, evidence)

        self.assertEqual(joined["summary"]["joined_procedure_count"], 3)
        self.assertIn("normalized_body_sha256", joined["procedures"][0]["asm_body"])

    def test_compare_uses_segment_map_and_procedure_ordinal(self) -> None:
        left_evidence = build_asm_body_evidence(ASM_SAMPLE, "MGC", "left.asm", "_none_")
        right_asm = ASM_SAMPLE.replace("segment seg003", "segment seg004").replace("seg003 ends", "seg004 ends")
        right_evidence = build_asm_body_evidence(right_asm, "WZD", "right.asm", "_none_")
        left_inventory = inventory("MGC")
        left_inventory["procedures"].append(
            {
                "database": "MGC",
                "name": "Third",
                "segment": "seg003",
                "procedure_ordinal": 1,
                "start_ea": "seg003:0000",
                "end_ea": "seg003:0004",
                "size": 4,
            }
        )
        right_inventory = inventory("WZD")
        right_inventory["procedures"].append(
            {
                "database": "WZD",
                "name": "Third",
                "segment": "seg004",
                "procedure_ordinal": 1,
                "start_ea": "seg004:0000",
                "end_ea": "seg004:0004",
                "size": 4,
            }
        )
        left_joined = join_procedure_evidence(left_inventory, left_evidence)
        right_joined = join_procedure_evidence(right_inventory, right_evidence)
        segment_map = {
            "segment_pairs": [
                {"mgc_segment": "seg001", "wzd_segment": "seg001"},
                {"mgc_segment": "seg003", "wzd_segment": "seg004"},
            ]
        }

        comparison = compare_procedure_evidence(left_joined, right_joined, segment_map)

        self.assertEqual(comparison["summary"]["procedure_pair_count"], 3)
        self.assertEqual(comparison["summary"]["same_normalized_body_hash_count"], 3)


if __name__ == "__main__":
    unittest.main()
