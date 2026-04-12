#!/usr/bin/env python3
"""
Match IDA 5.5 structures and enumerations across MAGIC and WIZARDS and
generate IDC rename scripts.
"""

from __future__ import annotations

import argparse
import json
import re
from dataclasses import dataclass
from difflib import SequenceMatcher
from pathlib import Path
from typing import Dict, Iterable, List, Optional, Sequence, Tuple


STRUCT_START_RE = re.compile(r"^struc\s+(?P<name>[A-Za-z_@$?][\w@$?]*)\s*;\s*\(sizeof=(?P<size>0x[0-9A-Fa-f]+|\d+)", re.IGNORECASE)
STRUCT_END_RE = re.compile(r"^ends\s+(?P<name>[A-Za-z_@$?][\w@$?]*)\b", re.IGNORECASE)
ENUM_START_RE = re.compile(r"^;\s*enum\s+(?P<name>[A-Za-z_@$?][\w@$?]*)\b(?P<suffix>.*)$", re.IGNORECASE)
ENUM_VALUE_RE = re.compile(r"^(?P<name>[A-Za-z_@$?][\w@$?]*)\s*=\s*(?P<value>.+?)\s*$")
GENERIC_TOKENS = {
    "enum",
    "type",
    "types",
    "flag",
    "flags",
    "bits",
    "bit",
    "status",
    "data",
    "record",
    "table",
    "list",
    "info",
}


@dataclass
class TypeRecord:
    game: str
    kind: str
    name: str
    normalized_name: str
    preferred_name_style: bool
    meta: Dict[str, object]


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Match and sync IDA 5.5 structs/enums between MAGIC and WIZARDS.")
    parser.add_argument("--magic-inc", default=str(Path("in") / "MAGIC.inc"))
    parser.add_argument("--wizards-inc", default=str(Path("in") / "WIZARDS.inc"))
    parser.add_argument("--output", default=str(Path("out") / "ida55-sync" / "type-sync.json"))
    parser.add_argument("--magic-idc", default=str(Path("out") / "ida55-sync" / "magic_type_sync.idc"))
    parser.add_argument("--wizards-idc", default=str(Path("out") / "ida55-sync" / "wizards_type_sync.idc"))
    parser.add_argument("--fuzzy-threshold", type=float, default=0.78)
    return parser.parse_args()


def preferred_struct_name(name: str) -> bool:
    return bool(re.fullmatch(r"s_[A-Z0-9_]+", name))


def preferred_enum_name(name: str) -> bool:
    return bool(re.fullmatch(r"e_[A-Z0-9_]+", name))


def normalize_name(name: str) -> str:
    lowered = name.lower()
    lowered = re.sub(r"^(enum_|s_|e_)", "", lowered)
    lowered = lowered.replace("__", "_")
    return lowered


def parse_size(value: str) -> int:
    return int(value, 16) if value.lower().startswith("0x") else int(value)


def parse_inc(path: Path, game: str) -> List[TypeRecord]:
    lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
    records: List[TypeRecord] = []
    index = 0
    while index < len(lines):
        line = lines[index].strip()

        struct_match = STRUCT_START_RE.match(line)
        if struct_match:
            name = struct_match.group("name")
            size = parse_size(struct_match.group("size"))
            members: List[Dict[str, object]] = []
            index += 1
            while index < len(lines):
                member_line = lines[index].strip()
                end_match = STRUCT_END_RE.match(member_line)
                if end_match:
                    break
                if member_line and not member_line.startswith(";"):
                    parts = member_line.split(None, 1)
                    member_name = parts[0]
                    member_decl = parts[1] if len(parts) > 1 else ""
                    members.append({"name": member_name, "decl": member_decl})
                index += 1
            records.append(
                TypeRecord(
                    game=game,
                    kind="struct",
                    name=name,
                    normalized_name=normalize_name(name),
                    preferred_name_style=preferred_struct_name(name),
                    meta={"size": size, "member_count": len(members), "members": members},
                )
            )
            index += 1
            continue

        enum_match = ENUM_START_RE.match(line)
        if enum_match:
            name = enum_match.group("name")
            suffix = enum_match.group("suffix").strip()
            values: List[Dict[str, object]] = []
            index += 1
            while index < len(lines):
                enum_line = lines[index].strip()
                if not enum_line:
                    index += 1
                    continue
                if enum_line.startswith("; enum ") or enum_line.startswith("struc "):
                    break
                if enum_line.startswith(";"):
                    index += 1
                    continue
                end_match = STRUCT_END_RE.match(enum_line)
                if end_match:
                    break
                value_match = ENUM_VALUE_RE.match(enum_line)
                if value_match:
                    values.append({"name": value_match.group("name"), "value": value_match.group("value")})
                index += 1
            records.append(
                TypeRecord(
                    game=game,
                    kind="enum",
                    name=name,
                    normalized_name=normalize_name(name),
                    preferred_name_style=preferred_enum_name(name),
                    meta={
                        "flags": suffix,
                        "value_count": len(values),
                        "values": values,
                    },
                )
            )
            continue

        index += 1
    return records


def similarity(left: TypeRecord, right: TypeRecord) -> float:
    name_score = SequenceMatcher(None, left.normalized_name, right.normalized_name).ratio()
    score = name_score
    if left.kind == "struct":
        left_members = [member["name"].lower() for member in left.meta["members"]]
        right_members = [member["name"].lower() for member in right.meta["members"]]
        if left_members and right_members:
            score = max(score, SequenceMatcher(None, ",".join(left_members), ",".join(right_members)).ratio())
    else:
        left_values = [value["name"].lower() for value in left.meta["values"]]
        right_values = [value["name"].lower() for value in right.meta["values"]]
        if left_values and right_values:
            score = max(score, SequenceMatcher(None, ",".join(left_values), ",".join(right_values)).ratio())

    metrics = shape_metrics(left, right)
    return round(
        0.55 * name_score
        + 0.20 * score
        + 0.15 * metrics["size_score"]
        + 0.10 * metrics["count_score"],
        6,
    )


def ratio_by_difference(left_value: int, right_value: int) -> float:
    largest = max(left_value, right_value)
    if largest <= 0:
        return 1.0
    return max(0.0, 1.0 - (abs(left_value - right_value) / largest))


def shape_metrics(left: TypeRecord, right: TypeRecord) -> Dict[str, float]:
    if left.kind == "struct":
        size_score = ratio_by_difference(int(left.meta["size"]), int(right.meta["size"]))
        count_score = ratio_by_difference(int(left.meta["member_count"]), int(right.meta["member_count"]))
    else:
        size_score = 1.0
        count_score = ratio_by_difference(int(left.meta["value_count"]), int(right.meta["value_count"]))
    return {
        "size_score": round(size_score, 6),
        "count_score": round(count_score, 6),
    }


def significant_tokens(name: str) -> set[str]:
    return {token for token in normalize_name(name).split("_") if token and token not in GENERIC_TOKENS}


def token_overlap(left: TypeRecord, right: TypeRecord) -> float:
    left_tokens = significant_tokens(left.name)
    right_tokens = significant_tokens(right.name)
    if not left_tokens or not right_tokens:
        return 0.0
    return len(left_tokens & right_tokens) / len(left_tokens | right_tokens)


def member_or_value_overlap(left: TypeRecord, right: TypeRecord) -> float:
    if left.kind == "struct":
        left_names = {str(member["name"]).lower() for member in left.meta["members"]}
        right_names = {str(member["name"]).lower() for member in right.meta["members"]}
    else:
        left_names = {str(value["name"]).lower() for value in left.meta["values"]}
        right_names = {str(value["name"]).lower() for value in right.meta["values"]}
    if not left_names or not right_names:
        return 0.0
    return len(left_names & right_names) / len(left_names | right_names)


def fuzzy_metrics(left: TypeRecord, right: TypeRecord, score: float) -> Dict[str, float | bool | str]:
    tokens = token_overlap(left, right)
    overlap = member_or_value_overlap(left, right)
    shape = shape_metrics(left, right)
    if left.kind == "struct":
        high_confidence = (
            tokens >= 0.4
            or (tokens > 0.0 and overlap >= 0.25)
            or (score >= 0.93 and (tokens > 0.0 or overlap >= 0.5 or shape["size_score"] >= 0.95))
            or overlap >= 0.999999
        )
    else:
        high_confidence = (
            tokens >= 0.4
            or overlap >= 0.5
            or shape["count_score"] >= 0.95
            or (score >= 0.95 and (tokens > 0.0 or overlap >= 0.2))
        )
    return {
        "token_overlap": round(tokens, 6),
        "member_value_overlap": round(overlap, 6),
        "size_score": shape["size_score"],
        "count_score": shape["count_score"],
        "high_confidence": high_confidence,
        "confidence": "high" if high_confidence else "review",
    }


def is_reasonable_fuzzy_match(left: TypeRecord, right: TypeRecord, score: float) -> bool:
    metrics = fuzzy_metrics(left, right, score)
    tokens = float(metrics["token_overlap"])
    overlap = float(metrics["member_value_overlap"])
    if score >= 0.92:
        return tokens > 0.0 or overlap >= 0.15
    return tokens >= 0.25 or overlap >= 0.35


def choose_canonical(left: TypeRecord, right: TypeRecord) -> TypeRecord:
    if left.preferred_name_style != right.preferred_name_style:
        return left if left.preferred_name_style else right
    if left.game != right.game:
        return right if right.game == "WIZARDS" else left
    return left


def exact_name_matches(magic_records: Sequence[TypeRecord], wizards_records: Sequence[TypeRecord]) -> Tuple[List[Dict[str, object]], set[Tuple[str, str]]]:
    matches: List[Dict[str, object]] = []
    used: set[Tuple[str, str]] = set()
    wizards_map: Dict[Tuple[str, str], TypeRecord] = {(record.kind, record.normalized_name): record for record in wizards_records}
    for magic in magic_records:
        key = (magic.kind, magic.normalized_name)
        wizards = wizards_map.get(key)
        if not wizards:
            continue
        canonical = choose_canonical(magic, wizards)
        matches.append(
            {
                "kind": magic.kind,
                "match_method": "normalized_name",
                "score": 1.0,
                "magic": magic,
                "wizards": wizards,
                "canonical": canonical,
            }
        )
        used.add((magic.kind, magic.name))
        used.add((wizards.kind, wizards.name))
    return matches, used


def fuzzy_matches(
    magic_records: Sequence[TypeRecord],
    wizards_records: Sequence[TypeRecord],
    used: set[Tuple[str, str]],
    threshold: float,
) -> List[Dict[str, object]]:
    matches: List[Dict[str, object]] = []
    remaining_magic = [record for record in magic_records if (record.kind, record.name) not in used]
    remaining_wizards = [record for record in wizards_records if (record.kind, record.name) not in used]
    for kind in ("struct", "enum"):
        magic_kind = [record for record in remaining_magic if record.kind == kind]
        wizards_kind = [record for record in remaining_wizards if record.kind == kind]
        scored: List[Tuple[float, TypeRecord, TypeRecord]] = []
        for magic in magic_kind:
            for wizards in wizards_kind:
                score = similarity(magic, wizards)
                if score >= threshold and is_reasonable_fuzzy_match(magic, wizards, score):
                    scored.append((score, magic, wizards))
        scored.sort(key=lambda item: (-item[0], item[1].name, item[2].name))
        matched_magic: set[str] = set()
        matched_wizards: set[str] = set()
        for score, magic, wizards in scored:
            if magic.name in matched_magic or wizards.name in matched_wizards:
                continue
            canonical = choose_canonical(magic, wizards)
            matches.append(
                {
                    "kind": kind,
                    "match_method": "string_distance",
                    "score": round(score, 6),
                    "fuzzy_metrics": fuzzy_metrics(magic, wizards, score),
                    "magic": magic,
                    "wizards": wizards,
                    "canonical": canonical,
                }
            )
            matched_magic.add(magic.name)
            matched_wizards.add(wizards.name)
    return matches


def serialize_record(record: TypeRecord) -> Dict[str, object]:
    return {
        "game": record.game,
        "kind": record.kind,
        "name": record.name,
        "normalized_name": record.normalized_name,
        "preferred_name_style": record.preferred_name_style,
        "meta": record.meta,
    }


def escape_idc_string(value: str) -> str:
    return value.replace("\\", "\\\\").replace('"', '\\"')


def render_idc(target_game: str, renames: Sequence[Dict[str, object]]) -> str:
    lines: List[str] = [
        "#include <idc.idc>",
        "",
        "static rename_struct(log_handle, old_name, new_name)",
        "{",
        "  auto id;",
        "  auto existing;",
        "  id = GetStrucIdByName(old_name);",
        "  if (id == -1) {",
        '    fprintf(log_handle, "missing-struct,%s\\n", old_name);',
        "    return;",
        "  }",
        "  existing = GetStrucIdByName(new_name);",
        "  if (existing != -1 && existing != id) {",
        '    fprintf(log_handle, "name-conflict-struct,%s,%s\\n", old_name, new_name);',
        "    return;",
        "  }",
        "  if (!SetStrucName(id, new_name)) {",
        '    fprintf(log_handle, "rename-failed-struct,%s,%s\\n", old_name, new_name);',
        "    return;",
        "  }",
        '  fprintf(log_handle, "renamed-struct,%s,%s\\n", old_name, new_name);',
        "}",
        "",
        "static rename_enum(log_handle, old_name, new_name)",
        "{",
        "  auto id;",
        "  auto existing;",
        "  id = GetEnum(old_name);",
        "  if (id == -1) {",
        '    fprintf(log_handle, "missing-enum,%s\\n", old_name);',
        "    return;",
        "  }",
        "  existing = GetEnum(new_name);",
        "  if (existing != -1 && existing != id) {",
        '    fprintf(log_handle, "name-conflict-enum,%s,%s\\n", old_name, new_name);',
        "    return;",
        "  }",
        "  if (!SetEnumName(id, new_name)) {",
        '    fprintf(log_handle, "rename-failed-enum,%s,%s\\n", old_name, new_name);',
        "    return;",
        "  }",
        '  fprintf(log_handle, "renamed-enum,%s,%s\\n", old_name, new_name);',
        "}",
        "",
        "static main()",
        "{",
        "  auto log_path;",
        "  auto log_handle;",
        '  log_path = GetIdbPath() + ".type-sync.log";',
        '  log_handle = fopen(log_path, "w");',
        "  if (log_handle == 0) {",
        '    Warning("Could not open log file: %s", log_path);',
        "    return;",
        "  }",
        f'  fprintf(log_handle, "target_game,{escape_idc_string(target_game)}\\n");',
    ]
    for rename in renames:
        if rename["kind"] == "struct":
            lines.append(
                '  rename_struct(log_handle, "%s", "%s");'
                % (escape_idc_string(rename["old_name"]), escape_idc_string(rename["new_name"]))
            )
        else:
            lines.append(
                '  rename_enum(log_handle, "%s", "%s");'
                % (escape_idc_string(rename["old_name"]), escape_idc_string(rename["new_name"]))
            )
    lines.extend(
        [
            "  fclose(log_handle);",
            '  Message("Wrote type sync log to %s\\n", log_path);',
            "}",
            "",
        ]
    )
    return "\n".join(lines)


def build_renames(matches: Sequence[Dict[str, object]], target_game: str) -> List[Dict[str, object]]:
    renames: List[Dict[str, object]] = []
    for match in matches:
        if match["match_method"] == "string_distance" and not match.get("fuzzy_metrics", {}).get("high_confidence", False):
            continue
        canonical = match["canonical"]
        target = match[target_game.lower()]
        if canonical.name == target.name:
            continue
        renames.append(
            {
                "kind": match["kind"],
                "match_method": match["match_method"],
                "score": match["score"],
                "confidence": match.get("fuzzy_metrics", {}).get("confidence", "exact"),
                "old_name": target.name,
                "new_name": canonical.name,
                "canonical_game": canonical.game,
                "canonical_name": canonical.name,
            }
        )
    renames.sort(key=lambda item: (item["kind"], item["old_name"], item["new_name"]))
    return renames


def main() -> int:
    args = parse_args()
    magic_records = parse_inc(Path(args.magic_inc), "MAGIC")
    wizards_records = parse_inc(Path(args.wizards_inc), "WIZARDS")

    exact_matches, used = exact_name_matches(magic_records, wizards_records)
    fuzzy = fuzzy_matches(magic_records, wizards_records, used, args.fuzzy_threshold)
    all_matches = exact_matches + fuzzy

    magic_renames = build_renames(all_matches, "MAGIC")
    wizards_renames = build_renames(all_matches, "WIZARDS")

    payload = {
        "summary": {
            "magic_record_count": len(magic_records),
            "wizards_record_count": len(wizards_records),
            "exact_match_count": len(exact_matches),
            "fuzzy_match_count": len(fuzzy),
            "magic_rename_count": len(magic_renames),
            "wizards_rename_count": len(wizards_renames),
            "fuzzy_threshold": args.fuzzy_threshold,
        },
        "magic_records": [serialize_record(record) for record in magic_records],
        "wizards_records": [serialize_record(record) for record in wizards_records],
        "matches": [
            {
                "kind": match["kind"],
                "match_method": match["match_method"],
                "score": match["score"],
                "fuzzy_metrics": match.get("fuzzy_metrics"),
                "magic": serialize_record(match["magic"]),
                "wizards": serialize_record(match["wizards"]),
                "canonical": serialize_record(match["canonical"]),
            }
            for match in all_matches
        ],
        "magic_renames": magic_renames,
        "wizards_renames": wizards_renames,
    }

    output_path = Path(args.output)
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(json.dumps(payload, indent=2), encoding="utf-8")

    magic_idc = Path(args.magic_idc)
    magic_idc.parent.mkdir(parents=True, exist_ok=True)
    magic_idc.write_text(render_idc("MAGIC", magic_renames), encoding="utf-8")

    wizards_idc = Path(args.wizards_idc)
    wizards_idc.parent.mkdir(parents=True, exist_ok=True)
    wizards_idc.write_text(render_idc("WIZARDS", wizards_renames), encoding="utf-8")

    print(f"Wrote {output_path}")
    print(f"Wrote {magic_idc}")
    print(f"Wrote {wizards_idc}")
    print(f"Exact matches: {len(exact_matches)}")
    print(f"Fuzzy matches: {len(fuzzy)}")
    print(f"MAGIC renames: {len(magic_renames)}")
    print(f"WIZARDS renames: {len(wizards_renames)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
