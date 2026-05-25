from __future__ import annotations

import unittest

from ida_proc_name_sync_prereq import write_expected_match_audit_tsv as audit


def proc(name: str, segment: str, ordinal: int, size: int, raw: str, normalized: str, mnemonic: str, instr: int):
    return {
        "name": name,
        "segment": segment,
        "procedure_ordinal": ordinal,
        "size": size,
        "asm_body": {
            "raw_body_sha256": raw,
            "normalized_body_sha256": normalized,
            "mnemonic_sequence_sha256": mnemonic,
            "instruction_count": instr,
        },
    }


class ExpectedMatchAuditTsvTests(unittest.TestCase):
    def test_render_tsv_puts_mismatches_first(self) -> None:
        pairs = [
            (
                proc("m1", "seg001", 1, 10, "aaa", "bbb", "ccc", 4),
                proc("w1", "seg101", 1, 10, "aaa", "bbb", "ccc", 4),
                "seg001",
                "seg101",
                1,
            ),
            (
                proc("m2", "seg001", 2, 10, "aaa", "bbb", "ccc", 4),
                proc("w2", "seg101", 2, 11, "xxx", "bbb", "yyy", 5),
                "seg001",
                "seg101",
                2,
            ),
        ]

        result = audit.render_tsv(pairs)

        lines = result.splitlines()
        self.assertEqual(lines[0], "status\tMAGIC\tWIZARDS\tsize_match\traw_match\tnormalized_match\tmnemonic_match\tmagic_bytes\twizards_bytes\tmagic_instr\twizards_instr")
        self.assertEqual(lines[1], "mismatch\tseg001p2:m2\tseg101p2:w2\tno\tno\tyes\tno\t10\t11\t4\t5")
        self.assertEqual(lines[2], "match\tseg001p1:m1\tseg101p1:w1\tyes\tyes\tyes\tyes\t10\t10\t4\t4")

    def test_expected_pairs_includes_missing_counterpart(self) -> None:
        magic = {("seg001", 1): proc("m1", "seg001", 1, 10, "a", "b", "c", 4)}
        wizards = {}

        pairs = audit.expected_pairs(magic, wizards, [("seg001", "seg101")])

        self.assertEqual(audit.render_tsv(pairs).splitlines()[1], "missing_wizards\tseg001p1:m1\tseg101p1:\tmissing\tmissing\tmissing\tmissing\t10\t\t4\t")


if __name__ == "__main__":
    unittest.main()
