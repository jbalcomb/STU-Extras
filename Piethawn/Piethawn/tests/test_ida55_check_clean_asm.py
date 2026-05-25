from __future__ import annotations

import tempfile
import unittest
from contextlib import redirect_stdout
from io import StringIO
from pathlib import Path

from ida_dasm_export import ida55_check_clean_asm as clean_asm


VALID_ASM = """Ideal
p386n
model large
segment seg000 byte public 'CODE' use16
assume cs:seg000
proc start far
mov     ax, bx
endp start
seg000 ends
end start
"""


class CleanAsmTests(unittest.TestCase):
    def test_valid_clean_asm_passes(self) -> None:
        result = clean_asm.check_asm_text(Path("valid.asm"), VALID_ASM)
        self.assertTrue(result.ok)
        self.assertEqual(result.proc_count, 1)
        self.assertEqual(result.endp_count, 1)

    def test_tab_counterexample_fails(self) -> None:
        result = clean_asm.check_asm_text(Path("tab.asm"), VALID_ASM.replace("mov     ax", "mov\tax"))
        self.assertFalse(result.ok)
        self.assertIn("INF_ENTAB", [finding.rule for finding in result.findings])

    def test_line_prefix_counterexample_fails(self) -> None:
        text = VALID_ASM.replace("mov     ax, bx", "seg000:0000 mov     ax, bx")
        result = clean_asm.check_asm_text(Path("prefix.asm"), text)
        self.assertFalse(result.ok)
        self.assertIn("INF_SHOWPREF", [finding.rule for finding in result.findings])

    def test_opcode_prefix_counterexample_fails(self) -> None:
        text = VALID_ASM.replace("mov     ax, bx", "B8 00 00 mov     ax, bx")
        result = clean_asm.check_asm_text(Path("opcode.asm"), text)
        self.assertFalse(result.ok)
        self.assertIn("INF_BINPREF", [finding.rule for finding in result.findings])

    def test_xref_counterexample_fails(self) -> None:
        text = VALID_ASM.replace("mov     ax, bx", "mov     ax, bx ; CODE XREF: start+1")
        result = clean_asm.check_asm_text(Path("xref.asm"), text)
        self.assertFalse(result.ok)
        self.assertIn("INF_XREFNUM", [finding.rule for finding in result.findings])


    def test_border_counterexample_fails(self) -> None:
        text = VALID_ASM.replace("mov     ax, bx", "; ---------------------------------------------------------------------------\nmov     ax, bx")
        result = clean_asm.check_asm_text(Path("border.asm"), text)
        self.assertFalse(result.ok)
        self.assertIn("INF_BORDER", [finding.rule for finding in result.findings])

    def test_proc_endp_mismatch_counterexample_fails(self) -> None:
        result = clean_asm.check_asm_text(Path("mismatch.asm"), VALID_ASM.replace("endp start\n", ""))
        self.assertFalse(result.ok)
        messages = "\n".join(finding.message for finding in result.findings)
        self.assertIn("proc/endp count mismatch", messages)

    def test_missing_proc_counterexample_fails(self) -> None:
        text = """Ideal
segment seg000 byte public 'CODE' use16
seg000 ends
"""
        result = clean_asm.check_asm_text(Path("missing.asm"), text)
        self.assertFalse(result.ok)
        messages = "\n".join(finding.message for finding in result.findings)
        self.assertIn("expected at least", messages)

    def test_missing_file_counterexample_fails(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            result = clean_asm.check_asm_file(Path(temp_dir) / "missing.asm")
        self.assertFalse(result.ok)
        self.assertEqual(result.findings[0].rule, "file")

    def test_main_returns_success_for_clean_file(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            path = Path(temp_dir) / "valid.asm"
            path.write_text(VALID_ASM, encoding="utf-8")
            output = StringIO()
            with redirect_stdout(output):
                result = clean_asm.main([str(path)])
        self.assertEqual(result, 0)
        self.assertIn("Clean ASM check passed", output.getvalue())

    def test_main_returns_failure_for_dirty_file(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            path = Path(temp_dir) / "dirty.asm"
            path.write_text(VALID_ASM.replace("mov     ax", "mov\tax"), encoding="utf-8")
            output = StringIO()
            with redirect_stdout(output):
                result = clean_asm.main([str(path)])
        self.assertEqual(result, 1)
        self.assertIn("INF_ENTAB", output.getvalue())


if __name__ == "__main__":
    unittest.main()
