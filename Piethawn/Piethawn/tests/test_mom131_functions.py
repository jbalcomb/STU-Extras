"""Tests for the source-based function lists (mom131_functions, remom_functions) and the name map. Powered by Claude."""

from __future__ import annotations

import tempfile
import unittest
from pathlib import Path

from mom131_name_sync import mom131_functions as mf
from mom131_name_sync import name_map
from remom_name_sync import remom_functions as rf

C_TEXT = """/* HEADER.C - { a brace in a comment } */
#include "GAME.H"
#define MAX(a, b) ((a) > (b) ? (a) : (b))
struct Point { int x, y; };
int table[3] = { 1, 2, 3 };
int Prototype(int a);
char *msg = "f() {";

/* the first */
void First(void)
{
    if (x) { y(); }
}

static int Second(int a, int b) { return a + b; }

struct Point far *Third(int (*cb)(void))
{
    return 0;
}
"""

ASM_TEXT = """; HEADER.ASM - Fake PROC in a comment
        PUBLIC  Alpha
Alpha   PROC
        ret
Alpha   ENDP
_beta   proc far ; x
        ret
_beta   endp
"""


def payload():
    """IDA layout: seg001 at 13EC:0007 (resident), stub050 at 33DC, ovr050 at 4558. Powered by Claude."""
    return {
        "segments": [
            {"name": "seg001", "start": "13EC:0007", "end": "1484:0006"},
            {"name": "stub050", "start": "33DC:0000", "end": "33DC:0030"},
            {"name": "ovr050", "start": "4558:0000", "end": "4600:0000"},
        ],
        "functions": [
            {"name": "_main", "segment": "seg001", "start_ea": "13EC:0007"},
            {"name": "sub_2", "segment": "seg001", "start_ea": "13EC:0100"},
            {"name": "sub_3", "segment": "seg001", "start_ea": "13EC:0200"},
            {"name": "Save_SAVE_GAM", "segment": "ovr050", "start_ea": "4558:0000"},
            {"name": "Load_SAVE_GAM", "segment": "ovr050", "start_ea": "4558:03F8"},
        ],
    }


class DefinitionTests(unittest.TestCase):
    def test_c_definitions_skip_comments_strings_structs_initializers_prototypes(self):
        """Only real function definitions, in file order, with lines and linkage. Powered by Claude."""
        self.assertEqual(mf.c_definitions(C_TEXT), [("First", 10, "public"), ("Second", 15, "static"), ("Third", 17, "public")])

    def test_asm_definitions(self):
        """PROC lines in either case; a PROC in a comment is not one. Powered by Claude."""
        self.assertEqual(mf.asm_definitions(ASM_TEXT), [("Alpha", 3, "proc"), ("_beta", 6, "proc")])


class LayoutTests(unittest.TestCase):
    def test_addresses(self):
        """IDA and .MAP addresses, resident and overlay. Powered by Claude."""
        layout = mf.Layout(payload())
        self.assertEqual(layout.segment_for("para:13ec"), "seg001")
        self.assertEqual(layout.segment_for("para:4558"), "ovr050")
        linear = 0x13EC0 + 0x100
        self.assertEqual(layout.address("seg001", linear), "13EC:0100")
        self.assertEqual(layout.map_address("seg001", linear), "03EC:0100")
        self.assertEqual(layout.map_address("ovr050", 0x45580 + 0x3F8), "23DC:03F8")

    def test_fill_between_anchors(self):
        """Gaps with as many definitions as procedures are filled in order. Powered by Claude."""
        defs = [("a", 1, "public"), ("b", 2, "proc"), ("c", 3, "public"), ("d", 4, "proc"), ("e", 5, "proc")]
        procs = [{"start_ea": "1000:%04X" % (i * 16)} for i in range(6)]
        by_start = {0x10000 + i * 16: (i + 1, procs[i]) for i in range(6)}
        publics = {"a": 0x10000, "c": 0x10020}
        # a=1, b fills 2, c=3; d, e vs procs 4..6: 2 vs 3, not filled
        self.assertEqual(mf.fill_between_anchors(defs, procs, publics, by_start), {1: 2})


class ProgramRowsTests(unittest.TestCase):
    def test_rows_from_modules_and_order(self):
        """MODULES.TSV paragraph -> IDA segment; n-th definition = n-th procedure; ids from the segment. Powered by Claude."""
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            (root / "src" / "wizards").mkdir(parents=True)
            (root / "src" / "wizards" / "MODULES.TSV").write_text("# segment\tsource\n13ec\tWIZARDS.C\n4558\tSAVELOAD.C\n")
            (root / "src" / "wizards" / "WIZARDS.C").write_text("int main(void)\n{\n}\nvoid B(void) { }\nvoid C(void) { }\n")
            (root / "src" / "wizards" / "SAVELOAD.C").write_text("void Save_Game(int s)\n{\n}\nvoid Load_Game(int s)\n{\n}\n")
            rows, counts = mf.program_rows("WZD", root, payload(), {}, None)
        by_name = {r["function"]: r for r in rows}
        self.assertEqual(by_name["Load_Game"]["id"], "WZD o050p02")
        self.assertEqual(by_name["Load_Game"]["ida_address"], "4558:03F8")
        self.assertEqual(by_name["Load_Game"]["map_address"], "23DC:03F8")
        self.assertEqual(by_name["Load_Game"]["line"], 4)
        self.assertEqual(by_name["C"]["id"], "WZD s001p03")
        self.assertEqual(counts["by_order"], 5)
        self.assertEqual(counts["modules_count_differs"], [])


class RemomAndMapTests(unittest.TestCase):
    def test_remom_rows_are_placed_by_id(self):
        """ReMoM id -> IDA segment + ordinal; an alias names _AIL_. Powered by Claude."""
        index = [{"exe": "WZD", "id": "WZD o050p02", "letter": "o", "segment": 50, "proc": 2, "function": "Load_SAVE_GAM",
                  "file": "MoX/src/LOADSAVE.c", "line": 9, "evidence": "definition"},
                 {"exe": "WZD", "id": "WZD o050p09", "letter": "o", "segment": 50, "proc": 9, "function": "X",
                  "file": "f.c", "line": 1, "evidence": "definition"}]
        rows = rf.place_rows(index, {"WZD": mf.Layout(payload())}, {})
        self.assertEqual(rows[0]["ida_address"], "4558:03F8")
        self.assertEqual(rows[1]["located"], "ovr050 has 2 procedures")
        aliases = {"WZD": {"_AIL_": "s037"}, "MGC": {"_AIL_": "s035"}}
        self.assertEqual(rf.ida_segment_name("WZD", "s", 37, aliases), "_AIL_")
        self.assertEqual(rf.ida_segment_name("MGC", "s", 35, aliases), "_AIL_")
        self.assertEqual(rf.ida_segment_name("MGC", "s", 37, aliases), "seg037")

    def test_join_classes(self):
        """same / different / remom-no-code / mom131-only / remom-only. Powered by Claude."""
        def m(rid, fn):
            return {"exe": "WZD", "id": rid, "segment": "ovr050", "proc": "1", "ida_address": "4558:0000",
                    "map_address": "", "ida_name": "", "function": fn, "file": "A.C", "line": "1"}

        def r(rid, fn, evidence="definition"):
            return {"exe": "WZD", "id": rid, "segment": "ovr050", "proc": "1", "ida_address": "4558:0000",
                    "map_address": "", "ida_name": "", "function": fn, "file": "a.c", "line": "1", "evidence": evidence}

        rows = name_map.join([m("WZD o050p01", "A"), m("WZD o050p02", "B"), m("WZD o050p03", "C"), m("", "D")],
                             [r("WZD o050p01", "A"), r("WZD o050p02", "Bee"), r("WZD o050p03", "", "comment"), r("WZD o050p04", "E")])
        got = sorted((x["mom131_function"], x["remom_function"], x["names"]) for x in rows)
        self.assertEqual(got, [("", "E", "remom-only"), ("A", "A", "same"), ("B", "Bee", "different"),
                               ("C", "", "remom-no-code"), ("D", "", "mom131-only")])


class TwinAndMentionTests(unittest.TestCase):
    def test_mark_twins(self):
        """The MGC id of a shared definition takes the WZD id's outcome. Powered by Claude."""
        from mom131_name_sync.compare_mom131_remom import mark_twins

        def rec(exe, rid, cls, line, new=""):
            return {"exe": exe, "id": rid, "class": cls, "module": "src/wizards/HEAP.C", "line": line,
                    "new_name": new, "remom_name": new, "remom_where": "x.c:1"}

        records = [rec("WZD", "WZD s008p01", "rename", 10, "Heap_Init"), rec("MGC", "MGC s008p01", "no-remom", 10),
                   rec("WZD", "WZD s008p02", "no-code", 20), rec("MGC", "MGC s008p02", "no-remom", 20),
                   rec("WZD", "WZD s008p03", "no-remom", 30), rec("MGC", "MGC s008p03", "no-remom", 30)]
        mark_twins(records)
        self.assertEqual([r["class"] for r in records],
                         ["rename", "twin", "no-code", "twin-no-code", "no-remom", "twin-no-remom"])
        self.assertEqual(records[1]["new_name"], "Heap_Init")

    def test_mention_program_rules(self):
        """Path, nearest definition address, reconcile mention, file name. Powered by Claude."""
        from mom131_name_sync.apply_mom131_renames import mention_program
        addresses = {"a0ec:0000": "WZD", "4358:95d5": "MGC"}
        text = '["a0ec:0000","Unit_Create",1],["4358:95d5","Unit_Create",2] plain Unit_Create here'
        first, second, third = [i for i in range(len(text)) if text.startswith("Unit_Create", i)]
        n = len("Unit_Create")
        self.assertEqual(mention_program("re/magic_names.tsv", text, first, first + n, addresses), "MGC")
        self.assertEqual(mention_program("index.html", text, first, first + n, addresses), "WZD")
        self.assertEqual(mention_program("index.html", text, second, second + n, addresses), "MGC")
        far = "x" * 200 + " Unit_Create makes nothing"
        at = far.index("Unit_Create")
        mentions = [{"file": "B.txt", "name": "Unit_Create", "near": "makes nothing", "program": "MGC"}]
        self.assertEqual(mention_program("B.txt", far, at, at + n, addresses, mentions), "MGC")
        self.assertEqual(mention_program("docs/LINK_TEST_MAGIC.txt", far, at, at + n, addresses), "MGC")
        self.assertEqual(mention_program("C.txt", far, at, at + n, addresses), "")

    def test_rename_doc_one_pass(self):
        """One-meaning names by mapping, two-meaning ones per mention, no chained renames. Powered by Claude."""
        from mom131_name_sync.apply_mom131_renames import rename_doc
        text = "A then (a0ec:0000) Unit_Create and (4358:95d5) Unit_Create and" + " " * 100 + "Unit_Create\n"
        mappings = {"WZD": {"Unit_Create": "Create_Unit", "A": "B"}, "MGC": {"Unit_Create": "Create_Unit_NewGame", "A": "B"}}
        out, count, undecided = rename_doc("x.txt", text, {"A": "B"}, ["Unit_Create"], mappings,
                                           {"Unit_Create": {"a0ec:0000": "WZD", "4358:95d5": "MGC"}})
        self.assertEqual(out, "B then (a0ec:0000) Create_Unit and (4358:95d5) Create_Unit_NewGame and" + " " * 100 + "Unit_Create\n")
        self.assertEqual((count, undecided), (3, [("Unit_Create", "x.txt:1")]))

    def test_map_fills_from_twin(self):
        """An MGC row of a shared definition shows the WZD id's ReMoM function, remom_via = the WZD id. Powered by Claude."""
        def row(exe, rid, fn, remom=""):
            return {"exe": exe, "id": rid, "mom131_function": fn, "mom131_file": "src/wizards/HEAP.C", "mom131_line": "10",
                    "remom_function": remom, "remom_file": "x.c" if remom else "", "remom_line": "1" if remom else "",
                    "remom_evidence": "definition" if remom else "", "remom_via": "", "names": "different" if remom else "mom131-only"}

        rows = [row("WZD", "WZD s008p01", "Heap_Init", "Init_Heap"), row("MGC", "MGC s008p01", "Heap_Init")]
        name_map.fill_from_twins(rows)
        self.assertEqual((rows[1]["remom_function"], rows[1]["remom_via"], rows[1]["names"]), ("Init_Heap", "WZD s008p01", "different"))


class RemomTodoTests(unittest.TestCase):
    def test_gaps_one_per_function_skip_and_our_name(self):
        """Twins listed once, skipped ids left out, IDA name first. Powered by Claude."""
        from mom131_name_sync import remom_todo

        def row(exe, rid, ida, fn, line, evidence="", remom=""):
            return {"exe": exe, "id": rid, "ida_name": ida, "ida_address": "1000:0000", "mom131_function": fn,
                    "mom131_file": "src/wizards/A.C", "mom131_line": line, "remom_function": remom,
                    "remom_file": "x.c" if evidence else "", "remom_line": "5" if evidence else "", "remom_evidence": evidence}

        rows = [row("WZD", "WZD s001p01", "_main", "main", "1", "comment"),
                row("WZD", "WZD s002p01", "Our_Name", "Their_Name", "10", "comment"), row("MGC", "MGC s002p01", "Our_Name", "Their_Name", "10"),
                row("WZD", "WZD s002p02", "sub_123", "Other", "20"),
                row("WZD", "WZD s002p03", "Done", "Done", "30", "definition", "Done")]
        cats = remom_todo.gaps(rows, {"WZD s001p01"})
        self.assertEqual([(w, k) for w, k, _ in cats["no-code"]], [("shared", ("src/wizards/A.C", "10"))])
        self.assertEqual([(w, k) for w, k, _ in cats["no-remom"]], [("WZD-only", ("src/wizards/A.C", "20"))])
        line = remom_todo.item(*cats["no-code"][0][1:], True)
        self.assertTrue(line.startswith("- [ ] **Our_Name** — `WZD s002p01`"))
        self.assertIn("mom131 `Their_Name`", line)


if __name__ == "__main__":
    unittest.main()
