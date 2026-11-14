"""Tests for the ReMoM -> WIZARDS.idb name sync: snapshot, comparison, validation and reconciliation. Powered by Claude."""

from __future__ import annotations

import json
import tempfile
import unittest
from contextlib import redirect_stdout, redirect_stderr
from io import StringIO
from pathlib import Path

from remom_name_sync import compare_remom_wzd as cmp
from remom_name_sync import remom_index


def write(path: Path, text: str) -> None:
    """Write a test file, creating folders. Powered by Claude."""
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8")


def ida_payload(functions):
    """A minimal function-dump JSON payload: functions = [(segment, start_ea, name)]. Powered by Claude."""
    return {
        "meta": {"idb_path": "C:/ida/WIZARDS.idb"},
        "functions": [
            {"name": name, "segment": seg, "start_ea": ea, "end_ea": ea, "size": 1} for seg, ea, name in functions
        ],
    }


def index_row(remom_id, function, evidence="definition", file="MoM/src/X.c", line=1):
    """One remom_index row for a WZD id like 'o076p01'. Powered by Claude."""
    letter, rest = remom_id[0], remom_id[1:]
    segment, proc = rest.split("p")
    return {"exe": "WZD", "id": f"WZD {letter}{int(segment):03d}p{int(proc):02d}", "letter": letter,
            "segment": int(segment), "proc": int(proc), "function": function, "file": file, "line": line,
            "evidence": evidence}


def run_classify(functions, rows, reconcile=None):
    """classify + order_renames on in-memory data; returns (records by id, plan). Powered by Claude."""
    reconcile = reconcile or {"segment_aliases": {}, "entries": {}}
    ida = cmp.ida_procedures(ida_payload(functions), reconcile["segment_aliases"])
    records = cmp.classify(rows, ida, reconcile)
    plan = cmp.order_renames(records, ida)
    return {r["id"]: r for r in records}, plan


class RemomIndexTests(unittest.TestCase):
    def test_scan_finds_definitions_comments_and_shared_claims(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            write(root / "MoM" / "src" / "A.c", "\n".join([
                "// WZD o076p01",
                "void Do_Thing(void)",
                "{",
                "}",
                "// WZD o076p02",
                "int Shared(int x)",
                "{",
                "}",
                "// WZD o076p04",
                "",
            ]))
            write(root / "MoX" / "src" / "B.c", "\n".join([
                "// WZD o076p03",
                "int Shared(int x)",
                "{",
                "}",
                "",
            ]))
            rows = remom_index.index_rows(remom_index.scan_remom(str(root)))
            by_id = {r["id"]: r for r in rows}
            self.assertEqual(by_id["WZD o076p01"]["function"], "Do_Thing")
            self.assertEqual(by_id["WZD o076p01"]["evidence"], "definition")
            self.assertEqual(by_id["WZD o076p02"]["evidence"], "definition?")
            self.assertEqual(by_id["WZD o076p03"]["evidence"], "definition?")
            self.assertEqual(by_id["WZD o076p04"]["evidence"], "comment")

    def test_tag_is_for_the_signature_directly_below(self) -> None:
        """Comments, doc blocks, '/* HACK */' and multi-line signatures are fine between tag and code; code or data
        first makes the tag bind nothing (Check_Cost is not o147p01); '// === // WZD ...'; a mistyped id. Powered by Claude."""
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            doc = ["/*", "; notes"] + ["; more"] * 80 + ["*/"]
            write(root / "MoM" / "src" / "L.c", "\n".join(
                ["// MGC  o50p01"] + doc + ["// void Newgame_Control(void)", "/* HACK */  int16_t Newgame_Control(void)", "{", "}",
                 "// WZD o147p01", "int16_t wx;", "static void Check_Cost(void)", "{", "}",
                 "void Move_Path_Find(int16_t a)", "{", "}",
                 "// WZD s01p07", "int16_t Get_Weighted_Choice(int16_t * w,", "                            int16_t n)", "{", "}",
                 "// MGC o56p17", "// === // WZD s01p07  int16_t Get_Weighted_Choice(int16_t * w, int16_t n);", "",
                 "// WZD o138p01", "static void Spell_Load(void)", "{", "}",
                 "// WZD  s14o03", "void Load_Font_File(char * f)", "{", "}", ""]))
            malformed, misplaced = [], []
            rows = remom_index.index_rows(remom_index.scan_remom(str(root), malformed, None, misplaced))
            got = {r["id"]: (r["function"], r["evidence"]) for r in rows}
        self.assertEqual(got["MGC o050p01"], ("Newgame_Control", "definition"))
        self.assertEqual(got["WZD o147p01"], ("", "comment"))
        self.assertEqual(got["WZD s001p07"], ("Get_Weighted_Choice", "definition"))
        self.assertEqual(got["MGC o056p17"], ("Get_Weighted_Choice", "same-as"))
        self.assertEqual(got["WZD o138p01"], ("Spell_Load", "definition"))
        self.assertEqual([(f, t) for f, _, t in malformed], [("MoM/src/L.c", "// WZD  s14o03")])
        self.assertEqual(misplaced, [])

    def test_tags_in_c_and_h_must_agree(self) -> None:
        """The Fields.c/Fields.h case: s36p08/s36p09 on swapped prototypes vs. definitions is a conflict; an area tag
        whose id binds through another tag is reported as misplaced. Powered by Claude."""
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            write(root / "MoX" / "src" / "Fields.c", "\n".join([
                "// WZD s36p08", "void Enable_Cancel(void);", "// WZD s36p09", "void Disable_Cancel(void);", "",
                "// WZD s36p08", "void Disable_Cancel(void)", "{", "}", "", "// WZD s36p09", "void Enable_Cancel(void)", "{", "}",
                "// WZD s36p10", "int16_t mouse_auto_exit;", "void Disable_Mouse_Wait(void)", "{", "}", ""]))
            write(root / "MoX" / "src" / "Fields.h", "\n".join(["// WZD s36p10", "void Disable_Mouse_Wait(void);", ""]))
            misplaced = []
            rows = remom_index.index_rows(remom_index.scan_remom(str(root), None, None, misplaced))
        got = {}
        for r in rows:
            got.setdefault(r["id"], set()).add((r["function"], r["evidence"]))
        self.assertEqual(got["WZD s036p08"], {("Enable_Cancel", "conflict?"), ("Disable_Cancel", "conflict?")})
        self.assertEqual(got["WZD s036p09"], {("Enable_Cancel", "conflict?"), ("Disable_Cancel", "conflict?")})
        self.assertEqual(got["WZD s036p10"], {("Disable_Mouse_Wait", "prototype")})
        self.assertEqual(misplaced, [("MoX/src/Fields.c", 15, "WZD s036p10", "Disable_Mouse_Wait")])
        lines = {(r["file"], int(r["line"])) for r in rows if r["id"] == "WZD s036p08"}
        self.assertEqual(lines, {("MoX/src/Fields.c", 1), ("MoX/src/Fields.c", 6)})   # the tags, not the signatures

    def test_consecutive_tags_are_a_list(self) -> None:
        """The Mouse.h case: an index table of '// WZD s35p30 ...' lines over a note and a prototype tags nothing;
        a 2nd tag of one EXE after a note still replaces the 1st. Powered by Claude."""
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            write(root / "MoX" / "src" / "Mouse.h", "\n".join([
                "// WZD s35p30  Save_Mouse_On_Page   CRL_Save_RSP", "// WZD s35p31  Save_Mouse_Off_Page  CRL_Save_DSP",
                "// WZD s35p32  IN_CRL_Save_RSP", "", "/*", "    note", "*/", "void Save_Mouse_On_Page(int x);", "",
                "// WZD s35p33", "// note", "// WZD s35p34", "void Restore(void);", ""]))
            write(root / "MoX" / "src" / "Mouse.c", chr(10).join([
                "void Save_Mouse_On_Page(int x)", "{", "}", "void Restore(void)", "{", "}", ""]))
            rows = remom_index.index_rows(remom_index.scan_remom(str(root)))
        got = {r["id"]: (r["function"], r["evidence"]) for r in rows}
        for p in ("30", "31", "32", "33"):
            self.assertEqual(got["WZD s035p" + p], ("", "comment"))
        self.assertEqual(got["WZD s035p34"], ("Restore", "prototype"))

    def test_twice_claimed_function_points_at_the_tags(self) -> None:
        """The EMS.h case: '// s13p16' lacks WZD, so '// WZD s13p15' falls through to the prototype. The row points at
        the tag (EMS.h), and the bare id is reported as mistyped. Powered by Claude."""
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            write(root / "MoX" / "src" / "EMS.c", "\n".join([
                "// WZD s13p15", "// EMM_CheckHandleOpen()", "", "// WZD s13p16", "void EMM_ReleaseAll(void)", "{", "}", ""]))
            write(root / "MoX" / "src" / "EMS.h", "\n".join([
                "// WZD s13p15", "// EMM_CheckHandleOpen()", "", "// s13p16", "void EMM_ReleaseAll(void);", "",
                "void F(void)", "{", "    // o100p04", "}", ""]))
            malformed = []
            rows = remom_index.index_rows(remom_index.scan_remom(str(root), malformed))
        got = {r["id"]: (r["function"], r["file"], int(r["line"]), r["evidence"]) for r in rows}
        self.assertEqual(got["WZD s013p15"], ("EMM_ReleaseAll", "MoX/src/EMS.h", 1, "prototype?"))
        self.assertEqual(got["WZD s013p16"], ("EMM_ReleaseAll", "MoX/src/EMS.c", 4, "definition?"))
        self.assertEqual(malformed, [("MoX/src/EMS.h", 4, "// s13p16")])   # the indented call-site note is not flagged

    def test_stacked_wzd_and_mgc_ids_share_the_function(self) -> None:
        """The LOADER.c case: '// MGC s001p07' over '// WZD o52p15' over a note and the function; a 2nd id of one EXE
        replaces the 1st; two of one EXE on consecutive lines are a list and bind nothing. Powered by Claude."""
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            write(root / "MoM" / "src" / "L.c", "\n".join([
                "// MGC s001p07",
                "// WZD o52p15",
                "// AKA Empty_Load_Fn8",
                "/*",
                "proc Load_TERRSTAT far",
                "*/",
                "void Load_TERRSTAT(void)",
                "{",
                "}",
                "// WZD o52p20",
                "// WZD o52p21",
                "void Later(void)",
                "{",
                "}",
                "",
            ]))
            rows = remom_index.index_rows(remom_index.scan_remom(str(root)))
            got = {r["id"]: (r["function"], r["evidence"]) for r in rows}
        self.assertEqual(got["MGC s001p07"], ("Load_TERRSTAT", "definition"))
        self.assertEqual(got["WZD o052p15"], ("Load_TERRSTAT", "definition"))
        self.assertEqual(got["WZD o052p20"], ("", "comment"))
        self.assertEqual(got["WZD o052p21"], ("", "comment"))

    def test_ids_and_code_inside_block_comments_are_ignored(self) -> None:
        """The FLIC_Draw.c case: a note pastes '// WZD s33p07' and a header inside /* */. Powered by Claude."""
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            write(root / "MoX" / "src" / "F.c", "\n".join([
                "// WZD s33p07",
                "void Mask(int a)",
                "{",
                "}",
                "/*",
                "// WZD s33p07",
                "void Mask(int a)",
                "*/",
                "void Mask_Inverse(int a)",
                "{",
                "}",
                "/* one-line note */ // WZD s33p09",
                "char *s = \"/*\"; // WZD s33p08",
                "void After(void)",
                "{",
                "}",
                "",
            ]))
            rows = remom_index.index_rows(remom_index.scan_remom(str(root)))
            by_id = {r["id"]: r for r in rows}
            self.assertEqual([r["function"] for r in rows if r["id"] == "WZD s033p07"], ["Mask"])
            self.assertEqual(by_id["WZD s033p07"]["evidence"], "definition")
            self.assertNotIn("WZD s033p09", by_id)
            self.assertNotIn("WZD s033p08", by_id)  # an id after code on the same line is not an id comment

    def test_snapshot_round_trips_and_writes_manifest(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            write(root / "MoX" / "src" / "B.c", "// WZD s012p03\nvoid Fn(void)\n{\n}\n")
            out = root / "snap" / "remom_index.tsv"
            rows = remom_index.take_snapshot(root, out)
            self.assertEqual(remom_index.read_index(out)[0]["function"], "Fn")
            self.assertEqual(remom_index.read_index(out)[0]["segment"], 12)
            manifest = json.loads(Path(str(out) + ".manifest.json").read_text(encoding="utf-8"))
            self.assertEqual(manifest["rows"], len(rows))


class CompareTests(unittest.TestCase):
    def test_same_rename_and_no_code(self) -> None:
        records, plan = run_classify(
            [("ovr076", "10", "Old_Name"), ("ovr076", "20", "Same_Name"), ("ovr076", "30", "sub_30")],
            [index_row("o76p1", "New_Name"), index_row("o76p2", "Same_Name"), index_row("o76p3", "", "comment")],
        )
        self.assertEqual(records["WZD o076p01"]["class"], "rename")
        self.assertEqual(records["WZD o076p02"]["class"], "same")
        self.assertEqual(records["WZD o076p03"]["class"], "no-code")
        self.assertEqual(plan, [{"id": "WZD o076p01", "old_name": "Old_Name", "new_name": "New_Name"}])

    def test_proc_ordinal_follows_address_order(self) -> None:
        records, _ = run_classify([("seg010", "20", "Second"), ("seg010", "10", "First")], [index_row("s10p1", "First")])
        self.assertEqual(records["WZD s010p01"]["class"], "same")

    def test_segment_alias_maps_named_segment(self) -> None:
        reconcile = {"segment_aliases": {"SOUND": "s038"}, "entries": {}}
        records, _ = run_classify([("SOUND", "10", "Play_Sound")], [index_row("s38p1", "Play_Sound")], reconcile)
        self.assertEqual(records["WZD s038p01"]["class"], "same")

    def test_no_ida_proc_and_wrong_letter_counterexamples(self) -> None:
        records, _ = run_classify([("ovr076", "10", "A")], [index_row("s76p1", "A"), index_row("o99p1", "B")])
        self.assertEqual(records["WZD s076p01"]["class"], "wrong-letter")
        self.assertEqual(records["WZD o099p01"]["class"], "no-ida-proc")

    def test_ambiguous_and_multiple_functions_counterexamples(self) -> None:
        records, _ = run_classify(
            [("ovr076", "10", "A"), ("ovr076", "20", "B")],
            [index_row("o76p1", "Helper", "definition?"), index_row("o76p2", "X"), index_row("o76p2", "Y")],
        )
        self.assertEqual(records["WZD o076p01"]["class"], "ambiguous")
        self.assertEqual(records["WZD o076p02"]["class"], "multiple-functions")

    def test_invalid_name_counterexample(self) -> None:
        records, _ = run_classify([("ovr076", "10", "A")], [index_row("o76p1", "Bad-Name")])
        self.assertEqual(records["WZD o076p01"]["class"], "invalid-name")

    def test_duplicate_target_counterexample(self) -> None:
        records, plan = run_classify(
            [("ovr076", "10", "A"), ("ovr077", "10", "B")],
            [index_row("o76p1", "Same"), index_row("o77p1", "Same")],
        )
        self.assertEqual(records["WZD o076p01"]["class"], "duplicate-target")
        self.assertEqual(records["WZD o077p01"]["class"], "duplicate-target")
        self.assertEqual(plan, [])

    def test_collision_with_a_name_that_stays_counterexample(self) -> None:
        records, _ = run_classify(
            [("ovr076", "10", "A"), ("ovr077", "10", "Taken")],
            [index_row("o76p1", "Taken")],
        )
        self.assertEqual(records["WZD o076p01"]["class"], "collision")

    def test_chain_renames_free_the_name_first(self) -> None:
        _, plan = run_classify(
            [("ovr076", "10", "A"), ("ovr077", "10", "B")],
            [index_row("o76p1", "B"), index_row("o77p1", "C")],
        )
        self.assertEqual([(s["old_name"], s["new_name"]) for s in plan], [("B", "C"), ("A", "B")])

    def test_swap_uses_a_temporary_name(self) -> None:
        _, plan = run_classify(
            [("seg036", "10", "Disable_Cancel"), ("seg036", "20", "Enable_Cancel")],
            [index_row("s36p1", "Enable_Cancel"), index_row("s36p2", "Disable_Cancel")],
        )
        self.assertEqual(
            [(s["old_name"], s["new_name"]) for s in plan],
            [("Disable_Cancel", "Disable_Cancel__rnsync_tmp"), ("Enable_Cancel", "Disable_Cancel"),
             ("Disable_Cancel__rnsync_tmp", "Enable_Cancel")],
        )

    def test_swap_with_taken_temporary_name_counterexample(self) -> None:
        records, plan = run_classify(
            [("seg036", "10", "A"), ("seg036", "20", "B"), ("seg037", "10", "A__rnsync_tmp")],
            [index_row("s36p1", "B"), index_row("s36p2", "A")],
        )
        self.assertIn("temp-name-taken", {records["WZD s036p01"]["class"], records["WZD s036p02"]["class"]})
        self.assertTrue(all("__rnsync_tmp" not in s["new_name"] for s in plan))

    def test_reconcile_skip_name_and_accept_letter(self) -> None:
        reconcile = {"segment_aliases": {}, "entries": {
            "WZD o076p01": {"action": "skip"},
            "WZD o076p02": {"action": "name", "name": "Right_Name"},
            "WZD s077p01": {"action": "accept-letter"},
        }}
        records, plan = run_classify(
            [("ovr076", "10", "A"), ("ovr076", "20", "B"), ("ovr077", "10", "C")],
            [index_row("o76p1", "Helper", "definition?"), index_row("o76p2", "Helper", "definition?"), index_row("s77p1", "C_New")],
            reconcile,
        )
        self.assertEqual(records["WZD o076p01"]["class"], "skipped")
        self.assertEqual(records["WZD o076p02"]["new_name"], "Right_Name")
        self.assertEqual(records["WZD s077p01"]["class"], "rename")
        self.assertEqual({s["new_name"] for s in plan}, {"Right_Name", "C_New"})

    def test_invalid_reconcile_file_counterexample(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            path = Path(temp_dir) / "reconcile.json"
            path.write_text(json.dumps({"segment_aliases": {"SOUND": "38"}, "entries": {"WZD o076p01": {"action": "drop"}}}), encoding="utf-8")
            with self.assertRaisesRegex(ValueError, "sNNN or oNNN"):
                cmp.load_reconcile(path)

    def test_compare_writes_report_plan_and_template_and_halts(self) -> None:
        with tempfile.TemporaryDirectory() as temp_dir:
            root = Path(temp_dir)
            index = root / "remom_index.tsv"
            remom_index.write_index([index_row("o76p1", "New_Name"), index_row("o99p1", "Lost")], index)
            functions = root / "WIZARDS.idb.ida55-functions.json"
            functions.write_text(json.dumps(ida_payload([("ovr076", "10", "Old_Name")])), encoding="utf-8")
            out = root / "out"
            with redirect_stdout(StringIO()), redirect_stderr(StringIO()):
                code = cmp.main(["--index", str(index), "--functions", str(functions), "--reconcile", str(root / "none.json"), "--out-dir", str(out)])
            self.assertEqual(code, 3)
            template = json.loads((out / "problems.reconcile-template.json").read_text(encoding="utf-8"))
            self.assertEqual(template["entries"]["WZD o099p01"]["problem"], "no-ida-proc")
            plan = json.loads((out / cmp.PLAN_NAME).read_text(encoding="utf-8"))
            self.assertEqual(plan["renames"][0]["new_name"], "New_Name")
            self.assertTrue((out / "remom-vs-wzd.report.md").exists())

    def test_stale_reconcile_entries_are_reported(self) -> None:
        records, _ = run_classify([("ovr076", "10", "A")], [index_row("o76p1", "A")])
        stale = cmp.stale_reconcile_entries(list(records.values()), {"entries": {"WZD o001p01": {"action": "skip"}}})
        self.assertEqual(stale, ["WZD o001p01"])


if __name__ == "__main__":
    unittest.main()
