"""Tests for the ReMoM -> mom131-source-map name sync: .MAP conversion, comparison rules, renaming. Powered by Claude."""

from __future__ import annotations

import json
import tempfile
import unittest
from collections import Counter
from pathlib import Path

from mom131_name_sync import compare_mom131_remom as cmp
from mom131_name_sync import mom131_names as names
from mom131_name_sync.apply_mom131_renames import merged_mapping, unscoped_mapping
from mom131_name_sync.source_words import code_words, rename_text

MAP_TEXT = """
 Start  Stop   Length Name               Class

 00000H 0000FH 00010H UTIL_TEXT          CODE
 01000H 0100FH 00010H SAVELOAD_TEXT      STUBSEG

  Address         Publics by Value

 0000:0000       _Random
 0000:0008 idle  _Clear_Memory_Far_Long_Name_Cut_S
 0100:0000       _Save_Game
 0100:0040       _Load_Game
 0200:0000 idle  _some_data

Program entry point at 0000:0000
"""


def ida_payload():
    """IDA layout matching MAP_TEXT: seg022 at 1000:0000, stub050 at 1100:0000, ovr050 at 4558:0000. Powered by Claude."""
    return {
        "segments": [
            {"name": "seg022", "start": "1000:0000", "end": "1001:0000"},
            {"name": "stub050", "start": "1100:0000", "end": "1100:0010"},
            {"name": "ovr050", "start": "4558:0000", "end": "4560:0000"},
        ],
        "functions": [
            {"name": "Random", "segment": "seg022", "start_ea": "1000:0000"},
            {"name": "Mem_Clear", "segment": "seg022", "start_ea": "1000:0008"},
            {"name": "thunk", "segment": "stub050", "start_ea": "1100:0000"},
            {"name": "Save_SAVE_GAM", "segment": "ovr050", "start_ea": "4558:0000"},
            {"name": "Load_SAVE_GAM", "segment": "ovr050", "start_ea": "4558:0040"},
        ],
    }


def row(exe, rid, function, module="src/wizards/A.C"):
    """A mom131 snapshot row. Powered by Claude."""
    return {"exe": exe, "id": rid, "function": function, "module": module, "ida_name": "x"}


def remom(rid, function, evidence="definition"):
    """A ReMoM index row. Powered by Claude."""
    return {"exe": rid.split()[0], "id": rid, "function": function, "file": "MoM/src/X.c", "line": 1, "evidence": evidence}


def scope(wzd=(), mgc=(), shared=(), modules=None):
    """A SourceScope: words are given per program's own files and the shared files. Powered by Claude."""
    own = {"WZD": Counter(wzd), "MGC": Counter(mgc)}
    shared_words = Counter(shared)
    words = {exe: own[exe] + shared_words for exe in own}
    modules = modules or {"src/wizards/A.C": {"WZD"}, "src/magic/M.C": {"MGC"}, "src/wizards/S.C": {"WZD", "MGC"}}
    return cmp.SourceScope(words, own, shared_words, modules)


def run(rows, remom_rows, sc, entries=None):
    """classify -> {(exe, id): record}. Powered by Claude."""
    records = cmp.classify(rows, remom_rows, {"entries": entries or {}}, sc)
    return {r["id"]: r for r in records}, cmp.plan(records, sc)


class MapConversionTests(unittest.TestCase):
    def test_resident_overlay_and_long_names(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            path = Path(temp_dir) / "W.MAP"
            path.write_text(MAP_TEXT, encoding="latin-1")
            words = {"Clear_Memory_Far_Long_Name_Cut_Short"}
            rows, counts = names.build_rows("WZD", path, ida_payload(), Path(temp_dir), {}, words, ["src/wizards/UTIL.C", "src/wizards/SAVELOAD.C"])
            by_id = {r["id"]: r for r in rows}
            self.assertEqual(by_id["WZD s022p01"]["function"], "Random")
            self.assertEqual(by_id["WZD s022p01"]["module"], "src/wizards/UTIL.C")
            self.assertEqual(by_id["WZD s022p02"]["function"], "Clear_Memory_Far_Long_Name_Cut_Short")
            self.assertEqual(by_id["WZD o050p01"]["function"], "Save_Game")
            self.assertEqual(by_id["WZD o050p01"]["ida_name"], "Save_SAVE_GAM")
            self.assertEqual(by_id["WZD o050p02"]["module"], "src/wizards/SAVELOAD.C")
            self.assertEqual(counts["not_a_function_start"], 1)  # _some_data

    def test_map_without_publics_counterexample(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            path = Path(temp_dir) / "W.MAP"
            path.write_text(" Start  Stop   Length Name   Class\n", encoding="latin-1")
            with self.assertRaisesRegex(ValueError, "Publics by Value"):
                names.parse_map(path)


class SourceWordsTests(unittest.TestCase):
    def test_c_renames_code_and_comments_never_literals(self) -> None:
        text = '/* calls Old_Name */ int Old_Name(void) { puts("Old_Name"); return Other; } // Old_Name\n'
        out, count = rename_text(Path("A.C"), text, {"Old_Name": "New_Name", "Other": "Old_Name"})
        self.assertEqual(out, '/* calls New_Name */ int New_Name(void) { puts("Old_Name"); return Old_Name; } // New_Name\n')
        self.assertEqual(count, 4)

    def test_asm_underscore_follows_and_literal_kept(self) -> None:
        out, _ = rename_text(Path("A.ASM"), "PUBLIC _Old_Name ; Old_Name\nmsg db 'Old_Name',0\n", {"Old_Name": "New"})
        self.assertEqual(out, "PUBLIC _New ; New\nmsg db 'Old_Name',0\n")

    def test_text_files_rename_everywhere_and_not_parts_of_words(self) -> None:
        out, _ = rename_text(Path("README.md"), "see `Old_Name` and Old_Name_Ex", {"Old_Name": "New"})
        self.assertEqual(out, "see `New` and Old_Name_Ex")

    def test_code_words_skip_comments_and_literals(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            (root / "src").mkdir()
            (root / "src" / "A.C").write_text('int Real(void); /* Ghost */ char *s = "Ghost2";\n', encoding="latin-1")
            words = code_words(root, [root / "src" / "A.C"])
            self.assertIn("Real", words)
            self.assertNotIn("Ghost", words)
            self.assertNotIn("Ghost2", words)


class CompareTests(unittest.TestCase):
    def test_same_rename_no_code_no_remom(self) -> None:
        sc = scope(wzd=["A", "Keep"])
        records, steps = run(
            [row("WZD", "WZD s001p01", "A"), row("WZD", "WZD s001p02", "Keep"), row("WZD", "WZD s001p03", "Gone"), row("WZD", "WZD s001p04", "Lone")],
            [remom("WZD s001p01", "B"), remom("WZD s001p02", "Keep"), remom("WZD s001p03", "", "comment")], sc)
        self.assertEqual(records["WZD s001p01"]["class"], "rename")
        self.assertEqual(records["WZD s001p02"]["class"], "same")
        self.assertEqual(records["WZD s001p03"]["class"], "no-code")
        self.assertEqual(records["WZD s001p04"]["class"], "no-remom")
        self.assertEqual([(s["old_name"], s["new_name"], s["programs"]) for s in steps], [("A", "B", ["WZD"])])

    def test_not_in_source_counterexample(self) -> None:
        records, _ = run([row("WZD", "WZD s000p01", "_abort")], [remom("WZD s000p01", "Abort")], scope())
        self.assertEqual(records["WZD s000p01"]["class"], "not-in-source")

    def test_shared_module_rename_applies_to_both_programs(self) -> None:
        sc = scope(shared=["Near_Alloc"])
        records, steps = run([row("WZD", "WZD s008p05", "Near_Alloc", "src/wizards/S.C"), row("MGC", "MGC s008p05", "Near_Alloc", "src/wizards/S.C")],
                             [remom("WZD s008p05", "Near_Allocate_Next")], sc)
        self.assertEqual(records["WZD s008p05"]["class"], "rename")
        self.assertEqual(steps[0]["programs"], ["MGC", "WZD"])

    def test_shared_module_ids_disagree_counterexample(self) -> None:
        sc = scope(shared=["Near_Alloc"])
        records, _ = run([row("WZD", "WZD s008p05", "Near_Alloc", "src/wizards/S.C"), row("MGC", "MGC s008p05", "Near_Alloc", "src/wizards/S.C")],
                         [remom("WZD s008p05", "Near_Allocate_Next"), remom("MGC s008p05", "Other_Name")], sc)
        self.assertEqual(records["WZD s008p05"]["class"], "conflicting-targets")

    def test_program_functions_may_differ(self) -> None:
        sc = scope(wzd=["Unit_Create"], mgc=["Unit_Create"])
        records, steps = run([row("WZD", "WZD o121p01", "Unit_Create"), row("MGC", "MGC o051p37", "Unit_Create", "src/magic/M.C")],
                             [remom("WZD o121p01", "Create_Unit"), remom("MGC o051p37", "Create_Unit_NewGame")], sc)
        self.assertEqual(records["WZD o121p01"]["class"], "rename")
        self.assertEqual(records["MGC o051p37"]["class"], "rename")
        self.assertEqual(len(steps), 2)

    def test_program_function_used_in_shared_file_counterexample(self) -> None:
        sc = scope(wzd=["Screen_Present"], mgc=["Screen_Present"], shared=["Screen_Present"])
        records, _ = run([row("WZD", "WZD s001p03", "Screen_Present")], [remom("WZD s001p03", "PageFlip_FX")], sc)
        self.assertEqual(records["WZD s001p03"]["class"], "conflicting-targets")

    def test_collision_duplicate_and_swap(self) -> None:
        sc = scope(wzd=["A", "B", "C", "Taken"])
        records, steps = run(
            [row("WZD", "WZD s001p01", "A"), row("WZD", "WZD s001p02", "B"), row("WZD", "WZD s001p03", "C")],
            [remom("WZD s001p01", "B"), remom("WZD s001p02", "A"), remom("WZD s001p03", "Taken")], sc)
        self.assertEqual(records["WZD s001p01"]["class"], "rename")  # swap: fine, renames apply at once
        self.assertEqual(records["WZD s001p03"]["class"], "collision")
        records, _ = run([row("WZD", "WZD s001p01", "A"), row("WZD", "WZD s001p02", "B")],
                         [remom("WZD s001p01", "Same"), remom("WZD s001p02", "Same")], sc)
        self.assertEqual(records["WZD s001p01"]["class"], "duplicate-target")

    def test_significance_clash_counterexample(self) -> None:
        sc = scope(wzd=["A", "B"])
        long1, long2 = "Spell_Casting_Screen_Reset_Map_Draw__1", "Spell_Casting_Screen_Reset_Map_Draw__2"
        records, _ = run([row("WZD", "WZD s001p01", "A"), row("WZD", "WZD s001p02", "B")],
                         [remom("WZD s001p01", long1), remom("WZD s001p02", long2)], sc)
        self.assertEqual(records["WZD s001p01"]["class"], "significance-clash")

    def test_library_name_and_keyword_counterexample(self) -> None:
        sc = scope(wzd=["far_strcpy", "Draw_It"])
        sc.reserved |= {"_fstrcpy"}
        records, steps = run([row("WZD", "WZD s009p07", "far_strcpy"), row("WZD", "WZD s009p08", "Draw_It")],
                             [remom("WZD s009p07", "_fstrcpy"), remom("WZD s009p08", "far")], sc)
        self.assertEqual(records["WZD s009p07"]["class"], "library-name")
        self.assertEqual(records["WZD s009p08"]["class"], "library-name")
        self.assertEqual(steps, [])

    def test_reconcile_skip_and_name(self) -> None:
        sc = scope(wzd=["A", "B"])
        records, steps = run([row("WZD", "WZD s001p01", "A"), row("WZD", "WZD s001p02", "B")],
                             [remom("WZD s001p01", "X", "definition?"), remom("WZD s001p02", "Y")], sc,
                             {"WZD s001p01": {"action": "name", "name": "Short"}, "WZD s001p02": {"action": "skip"}})
        self.assertEqual(records["WZD s001p01"]["new_name"], "Short")
        self.assertEqual(records["WZD s001p02"]["class"], "skipped")
        self.assertEqual([s["new_name"] for s in steps], ["Short"])

    def test_invalid_reconcile_counterexample(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            path = Path(temp_dir) / "reconcile.json"
            path.write_text(json.dumps({"entries": {"WZD s001p01": {"action": "accept-letter"}}}), encoding="utf-8")
            with self.assertRaisesRegex(ValueError, "action must be one of"):
                cmp.load_reconcile(path)


class ApplyTests(unittest.TestCase):
    def test_merged_mapping_leaves_disagreeing_names(self) -> None:
        mapping, disagree = merged_mapping({"WZD", "MGC"}, {"WZD": {"Unit_Create": "Create_Unit", "A": "B"},
                                                            "MGC": {"Unit_Create": "Create_Unit_NewGame", "A": "B"}})
        self.assertEqual(mapping, {"A": "B"})
        self.assertEqual(disagree, {"Unit_Create"})

    def test_unscoped_mapping_only_one_meaning(self) -> None:
        mapping, left = unscoped_mapping({"WZD": {"X": "Y", "K": "K2"}, "MGC": {}}, {"WZD": Counter(), "MGC": Counter({"K": 1})})
        self.assertEqual(mapping, {"X": "Y"})
        self.assertEqual(left, ["K"])


class BorlandNamesTests(unittest.TestCase):
    def test_only_code_of_included_headers_counts(self) -> None:
        """DIR (in a header no module includes) and Fill (only in a comment) are free; _fstrcpy (STRING.H, included
        through a nested include) is not. Powered by Claude."""
        with tempfile.TemporaryDirectory() as tmp:
            root = Path(tmp)
            inc = root / "INCLUDE"
            (inc / "SYS").mkdir(parents=True)
            (inc / "STRING.H").write_text("#include <_defs.h>\n/* Fill patterns */\nchar far *_fstrcpy(char far *d, const char far *s);\n")
            (inc / "_DEFS.H").write_text("#define _Cdecl cdecl\n")
            (inc / "DIRENT.H").write_text("typedef struct { int fd; } DIR;\n")
            (root / "src").mkdir()
            (root / "src" / "A.C").write_text('#include <string.h>\n#include "GAME.H"\n')
            names = cmp.borland_names(inc, root)
        self.assertIn("_fstrcpy", names)
        self.assertIn("_Cdecl", names)
        self.assertNotIn("DIR", names)
        self.assertNotIn("Fill", names)


if __name__ == "__main__":
    unittest.main()
