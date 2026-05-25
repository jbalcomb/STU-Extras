#!/usr/bin/env python3
"""Join IDB procedure metadata with ASM body hash evidence."""

from __future__ import annotations

import argparse
import json
from pathlib import Path
from typing import Any, Dict, List, Tuple


def load_json(path: Path) -> Dict[str, Any]:
    return json.loads(path.read_text(encoding="utf-8"))


def write_json(path: Path, payload: Dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(payload, indent=2) + "\n", encoding="utf-8")


def procedure_key(record: Dict[str, Any]) -> Tuple[str, int]:
    return (str(record["segment"]), int(record["procedure_ordinal"]))


def join_procedure_evidence(
    inventory: Dict[str, Any],
    body_evidence: Dict[str, Any],
) -> Dict[str, Any]:
    evidence_by_key = {procedure_key(record): record for record in body_evidence.get("procedures", [])}
    used_keys: set[Tuple[str, int]] = set()
    joined: List[Dict[str, Any]] = []
    missing: List[Dict[str, Any]] = []

    for procedure in inventory.get("procedures", []):
        key = procedure_key(procedure)
        evidence = evidence_by_key.get(key)
        record = dict(procedure)
        if evidence is None:
            missing.append({"reason": "missing_asm_body_evidence", "procedure": dict(procedure)})
        else:
            used_keys.add(key)
            record["asm_body"] = {
                "name": evidence.get("name"),
                "raw_body_sha256": evidence.get("raw_body_sha256"),
                "normalized_body_sha256": evidence.get("normalized_body_sha256"),
                "mnemonic_sequence_sha256": evidence.get("mnemonic_sequence_sha256"),
                "raw_line_count": evidence.get("raw_line_count"),
                "normalized_line_count": evidence.get("normalized_line_count"),
                "instruction_count": evidence.get("instruction_count"),
            }
        joined.append(record)

    unused = [
        {"reason": "missing_idb_inventory_record", "procedure": dict(record)}
        for key, record in sorted(evidence_by_key.items())
        if key not in used_keys
    ]

    if missing or unused:
        raise ValueError(f"Cannot join evidence cleanly: missing={len(missing)} unused={len(unused)}")

    return {
        "schema_version": 1,
        "artifact": "procedure_inventory_with_asm_body_evidence",
        "database": inventory.get("database"),
        "source_inventory": inventory.get("source_path"),
        "source_asm_body_evidence": body_evidence.get("source_path"),
        "function_count": len(joined),
        "summary": {
            "inventory_procedure_count": len(inventory.get("procedures", [])),
            "asm_body_procedure_count": len(body_evidence.get("procedures", [])),
            "joined_procedure_count": len(joined),
        },
        "procedures": joined,
    }


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Join IDB procedure inventory with ASM body hash evidence.")
    parser.add_argument("--inventory", type=Path, required=True, help="Procedure inventory JSON.")
    parser.add_argument("--asm-body-evidence", type=Path, required=True, help="ASM body evidence JSON.")
    parser.add_argument("--output", type=Path, required=True, help="Output joined procedure evidence JSON.")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    joined = join_procedure_evidence(
        load_json(args.inventory),
        load_json(args.asm_body_evidence),
    )
    write_json(args.output, joined)
    summary = joined["summary"]
    print(f"Wrote {args.output}")
    print(f"Database: {joined['database']}")
    print(f"Joined procedures: {summary['joined_procedure_count']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
