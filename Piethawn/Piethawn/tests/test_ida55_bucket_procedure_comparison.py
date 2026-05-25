from __future__ import annotations

import unittest

from ida55_bucket_procedure_comparison import bucket_comparison


def pair(
    left_name: str,
    right_name: str,
    *,
    same_normalized: bool | None,
    same_mnemonic: bool | None,
) -> dict[str, object]:
    return {
        "segment_pair": {"pair_id": "MGC:seg001__WZD:seg001"},
        "procedure_ordinal": 1,
        "left": {
            "name": left_name,
            "segment": "seg001",
            "start_ea": "seg001:0000",
        },
        "right": {
            "name": right_name,
            "segment": "seg001",
            "start_ea": "seg001:0000",
        },
        "observations": {
            "same_idb_name": left_name == right_name,
            "same_size": True,
            "same_normalized_body_hash": same_normalized,
            "same_mnemonic_sequence_hash": same_mnemonic,
            "different_body_fields": [],
        },
    }


class Ida55BucketProcedureComparisonTests(unittest.TestCase):
    def test_buckets_rename_candidate_when_hash_matches_and_name_differs(self) -> None:
        comparison = {
            "artifact": "procedure_evidence_comparison",
            "left_database": "MGC",
            "right_database": "WZD",
            "pairing_rule": "mapped_segment_and_procedure_ordinal",
            "procedure_pairs": [pair("Old", "New", same_normalized=True, same_mnemonic=True)],
        }

        report = bucket_comparison(comparison)

        self.assertEqual(report["bucket_counts"]["same_normalized_hash_different_name"], 1)
        self.assertEqual(report["buckets"]["same_normalized_hash_different_name"][0]["left_name"], "Old")

    def test_buckets_mnemonic_only_match_before_same_name_drift(self) -> None:
        comparison = {
            "procedure_pairs": [pair("Same", "Same", same_normalized=False, same_mnemonic=True)],
        }

        report = bucket_comparison(comparison)

        self.assertEqual(report["bucket_counts"]["same_mnemonic_hash_different_normalized_hash"], 1)


if __name__ == "__main__":
    unittest.main()
