"""Build procedure body evidence from IDA ASM exports."""

from __future__ import annotations

from typing import Any, Dict, List, Tuple

from asm_proc_tools import normalize_asm, sha256_text
from ida55_split_asm import parse_asm


def procedure_body_evidence(text: str) -> Dict[str, Any]:
    normalized_text, mnemonic_text = normalize_asm(text)
    return {
        "raw_asm_sha256": sha256_text(text),
        "normalized_asm_sha256": sha256_text(normalized_text),
        "mnemonic_sha256": sha256_text(mnemonic_text),
        "raw_line_count": len(text.splitlines()),
        "normalized_line_count": len(normalized_text.splitlines()) if normalized_text else 0,
        "instruction_count": len([line for line in mnemonic_text.splitlines() if line]),
    }


def build_asm_evidence_from_text(
    text: str,
    database: str,
    source_path: str,
    default_segment: str = "_no_segment_",
) -> Dict[str, Any]:
    buckets = parse_asm(text, default_segment=default_segment)
    procedures: List[Dict[str, Any]] = []
    for segment_name in sorted(buckets):
        bucket = buckets[segment_name]
        for ordinal, proc in enumerate(bucket.procedures, start=1):
            body_text = "".join(proc.lines)
            procedures.append(
                {
                    "database": database,
                    "name": proc.name,
                    "segment": segment_name,
                    "procedure_ordinal": ordinal,
                    "evidence": procedure_body_evidence(body_text),
                }
            )

    return {
        "schema_version": 1,
        "artifact": "asm_procedure_evidence",
        "database": database,
        "source_path": source_path,
        "procedure_count": len(procedures),
        "procedures": procedures,
    }


def evidence_key(record: Dict[str, Any]) -> Tuple[str, int]:
    return (str(record["segment"]), int(record["procedure_ordinal"]))


def combine_inventory_with_asm_evidence(
    inventory: Dict[str, Any],
    asm_evidence: Dict[str, Any],
) -> Dict[str, Any]:
    evidence_by_key = {evidence_key(record): record for record in asm_evidence.get("procedures", [])}
    used_keys: set[Tuple[str, int]] = set()
    enriched_procedures: List[Dict[str, Any]] = []
    missing_evidence: List[Dict[str, Any]] = []

    for procedure in inventory.get("procedures", []):
        key = (str(procedure["segment"]), int(procedure["procedure_ordinal"]))
        enriched = dict(procedure)
        evidence_record = evidence_by_key.get(key)
        if evidence_record is None:
            missing_evidence.append(
                {
                    "reason": "no_asm_body_evidence",
                    "procedure": {
                        "database": procedure.get("database"),
                        "name": procedure.get("name"),
                        "segment": procedure.get("segment"),
                        "procedure_ordinal": procedure.get("procedure_ordinal"),
                    },
                }
            )
        else:
            used_keys.add(key)
            enriched["asm_body"] = {
                "name": evidence_record.get("name"),
                **dict(evidence_record.get("evidence", {})),
            }
        enriched_procedures.append(enriched)

    unused_evidence = [
        {
            "reason": "no_inventory_procedure",
            "procedure": {
                "database": record.get("database"),
                "name": record.get("name"),
                "segment": record.get("segment"),
                "procedure_ordinal": record.get("procedure_ordinal"),
            },
        }
        for key, record in sorted(evidence_by_key.items())
        if key not in used_keys
    ]

    return {
        "schema_version": 1,
        "artifact": "procedure_inventory_with_asm_evidence",
        "database": inventory.get("database"),
        "source_inventory": inventory.get("source_path"),
        "source_asm_evidence": asm_evidence.get("source_path"),
        "function_count": len(enriched_procedures),
        "summary": {
            "inventory_procedure_count": len(inventory.get("procedures", [])),
            "asm_evidence_procedure_count": len(asm_evidence.get("procedures", [])),
            "enriched_procedure_count": len(enriched_procedures) - len(missing_evidence),
            "missing_evidence_count": len(missing_evidence),
            "unused_evidence_count": len(unused_evidence),
        },
        "procedures": enriched_procedures,
        "missing_evidence": missing_evidence,
        "unused_evidence": unused_evidence,
    }
