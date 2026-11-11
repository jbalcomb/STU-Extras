from __future__ import annotations

import unittest

from ida_proc_name_sync_prereq import write_match_hash_tsvs as match_hash


class MatchHashTsvTests(unittest.TestCase):
    def test_split_pairs_by_hash_separates_yay_and_nay(self) -> None:
        pairs = [
            ({"name": "s1p1", "bytes_sha256": "aaa"}, {"name": "s1p1", "bytes_sha256": "aaa"}),
            ({"name": "s1p2", "bytes_sha256": "bbb"}, {"name": "s1p2", "bytes_sha256": "ccc"}),
        ]

        yay, nay = match_hash.split_pairs_by_hash(pairs)

        self.assertEqual(match_hash.render_tsv(yay), "MAGIC\tWIZARDS\ns1p1\ts1p1\n")
        self.assertEqual(match_hash.render_tsv(nay), "MAGIC\tWIZARDS\ns1p2\ts1p2\n")

    def test_pair_functions_by_known_segments_uses_position_counterexample(self) -> None:
        magic = {"seg001": [{"name": "m1", "bytes_sha256": "1"}, {"name": "m2", "bytes_sha256": "2"}]}
        wizards = {"seg101": [{"name": "w1", "bytes_sha256": "1"}]}
        segment_pairs = [{"mgc_segment": "seg001", "wzd_segment": "seg101"}]

        pairs = match_hash.pair_functions_by_known_segments(magic, wizards, segment_pairs)

        self.assertEqual(match_hash.render_tsv(pairs), "MAGIC\tWIZARDS\nm1\tw1\nm2\t\n")


if __name__ == "__main__":
    unittest.main()
