"""Build structural procedure pair drafts from generic IDA inventories."""

from __future__ import annotations

from collections import defaultdict
from typing import Any, Dict, Iterable, List, Sequence, Tuple


def procedures_by_segment(procedures: Iterable[Dict[str, Any]]) -> Dict[str, List[Dict[str, Any]]]:
    grouped: Dict[str, List[Dict[str, Any]]] = defaultdict(list)
    for procedure in procedures:
        grouped[str(procedure["segment"])].append(procedure)
    for segment, items in grouped.items():
        grouped[segment] = sorted(
            items,
            key=lambda item: (
                int(item["procedure_ordinal"]),
                str(item["start_ea"]),
                str(item["name"]),
            ),
        )
    return dict(grouped)


def default_same_segment_pairs(left_segments: Iterable[str], right_segments: Iterable[str]) -> List[Dict[str, str]]:
    return [
        {
            "left_segment": segment,
            "right_segment": segment,
            "relationship": "same_segment_id",
            "source": "default_same_segment_id",
        }
        for segment in sorted(set(left_segments) & set(right_segments))
    ]


def normalize_segment_pair(
    pair: Dict[str, Any],
    left_database: str,
    right_database: str,
) -> Dict[str, Any]:
    left_field = f"{left_database.lower()}_segment"
    right_field = f"{right_database.lower()}_segment"
    left_segment = pair.get("left_segment", pair.get(left_field))
    right_segment = pair.get("right_segment", pair.get(right_field))
    if left_segment is None or right_segment is None:
        raise ValueError(
            "Segment map pair is missing left/right segment fields "
            f"(looked for left_segment/right_segment and {left_field}/{right_field})."
        )
    normalized = dict(pair)
    normalized["left_segment"] = str(left_segment)
    normalized["right_segment"] = str(right_segment)
    return normalized


def load_segment_pairs_from_map(
    segment_map: Dict[str, Any],
    left_database: str,
    right_database: str,
) -> List[Dict[str, Any]]:
    pairs = segment_map.get("segment_pairs")
    if not isinstance(pairs, list):
        raise ValueError("Segment map JSON must contain a segment_pairs list.")
    return [normalize_segment_pair(pair, left_database, right_database) for pair in pairs]


def ea_value(value: object) -> int | None:
    text = str(value or "")
    if ":" in text:
        text = text.split(":", 1)[1]
    if text.lower().startswith("0x"):
        text = text[2:]
    try:
        return int(text, 16)
    except ValueError:
        return None


def ea_segment(value: object) -> str | None:
    text = str(value or "")
    if ":" not in text:
        return None
    return text.split(":", 1)[0]


def procedure_evidence(procedure: Dict[str, Any]) -> Dict[str, Any]:
    start = ea_value(procedure.get("start_ea"))
    end = ea_value(procedure.get("end_ea"))
    ea_count = None
    start_segment = ea_segment(procedure.get("start_ea"))
    end_segment = ea_segment(procedure.get("end_ea"))
    if start_segment == end_segment and start is not None and end is not None and end >= start:
        ea_count = end - start
    return {
        "size": procedure.get("size"),
        "line_count": procedure.get("line_count"),
        "ea_count": procedure.get("ea_count", ea_count),
        "bytes_sha256": procedure.get("bytes_sha256"),
        "chunk_bytes_sha256": list(procedure.get("chunk_bytes_sha256") or []),
    }


def procedure_identity(procedure: Dict[str, Any]) -> Dict[str, Any]:
    return {
        "database": procedure.get("database"),
        "name": procedure.get("name"),
        "segment": procedure.get("segment"),
        "procedure_ordinal": procedure.get("procedure_ordinal"),
        "start_ea": procedure.get("start_ea"),
        "end_ea": procedure.get("end_ea"),
        "size": procedure.get("size"),
    }


def evidence_observations(left: Dict[str, Any], right: Dict[str, Any]) -> Dict[str, Any]:
    left_evidence = procedure_evidence(left)
    right_evidence = procedure_evidence(right)
    comparable_fields = [
        field
        for field in ("size", "line_count", "ea_count", "bytes_sha256", "chunk_bytes_sha256")
        if left_evidence.get(field) is not None and right_evidence.get(field) is not None
    ]
    matching_fields = [
        field
        for field in comparable_fields
        if left_evidence.get(field) == right_evidence.get(field)
    ]
    differing_fields = [
        field
        for field in comparable_fields
        if left_evidence.get(field) != right_evidence.get(field)
    ]
    return {
        "same_name": left.get("name") == right.get("name"),
        "left_evidence": left_evidence,
        "right_evidence": right_evidence,
        "comparable_fields": comparable_fields,
        "matching_fields": matching_fields,
        "differing_fields": differing_fields,
    }


def compare_inventories(
    left_inventory: Dict[str, Any],
    right_inventory: Dict[str, Any],
    segment_pairs: Sequence[Dict[str, Any]] | None = None,
) -> Dict[str, Any]:
    left_grouped = procedures_by_segment(left_inventory.get("procedures", []))
    right_grouped = procedures_by_segment(right_inventory.get("procedures", []))
    pairs = list(segment_pairs) if segment_pairs is not None else default_same_segment_pairs(left_grouped, right_grouped)

    procedure_pairs: List[Dict[str, Any]] = []
    left_unpaired: List[Dict[str, Any]] = []
    right_unpaired: List[Dict[str, Any]] = []
    evidence_differences: List[Dict[str, Any]] = []
    paired_left_keys: set[Tuple[str, int]] = set()
    paired_right_keys: set[Tuple[str, int]] = set()

    for pair in pairs:
        left_segment = str(pair["left_segment"])
        right_segment = str(pair["right_segment"])
        left_items = left_grouped.get(left_segment, [])
        right_items = right_grouped.get(right_segment, [])
        pair_count = min(len(left_items), len(right_items))

        for index in range(pair_count):
            left = left_items[index]
            right = right_items[index]
            left_key = (str(left["segment"]), int(left["procedure_ordinal"]))
            right_key = (str(right["segment"]), int(right["procedure_ordinal"]))
            paired_left_keys.add(left_key)
            paired_right_keys.add(right_key)
            observations = evidence_observations(left, right)
            record = {
                "left": procedure_identity(left),
                "right": procedure_identity(right),
                "segment_pair": dict(pair),
                "pairing_rule": "mapped_segment_and_procedure_ordinal",
                "procedure_ordinal": int(left["procedure_ordinal"]),
                "observations": observations,
            }
            procedure_pairs.append(record)
            if observations["differing_fields"]:
                evidence_differences.append(record)

        for item in left_items[pair_count:]:
            paired_left_keys.add((str(item["segment"]), int(item["procedure_ordinal"])))
            left_unpaired.append(
                {
                    "reason": "no_right_ordinal_counterpart",
                    "segment_pair": dict(pair),
                    "procedure": procedure_identity(item),
                }
            )
        for item in right_items[pair_count:]:
            paired_right_keys.add((str(item["segment"]), int(item["procedure_ordinal"])))
            right_unpaired.append(
                {
                    "reason": "no_left_ordinal_counterpart",
                    "segment_pair": dict(pair),
                    "procedure": procedure_identity(item),
                }
            )

    paired_left_segments = {str(pair["left_segment"]) for pair in pairs}
    paired_right_segments = {str(pair["right_segment"]) for pair in pairs}
    for segment in sorted(set(left_grouped) - paired_left_segments):
        for item in left_grouped[segment]:
            left_unpaired.append({"reason": "no_segment_pair", "procedure": procedure_identity(item)})
    for segment in sorted(set(right_grouped) - paired_right_segments):
        for item in right_grouped[segment]:
            right_unpaired.append({"reason": "no_segment_pair", "procedure": procedure_identity(item)})

    return {
        "schema_version": 1,
        "artifact": "procedure_pair_draft",
        "pairing_rule": "mapped_segment_and_procedure_ordinal",
        "left_database": left_inventory.get("database"),
        "right_database": right_inventory.get("database"),
        "summary": {
            "left_procedure_count": len(left_inventory.get("procedures", [])),
            "right_procedure_count": len(right_inventory.get("procedures", [])),
            "segment_pair_count": len(pairs),
            "procedure_pair_count": len(procedure_pairs),
            "same_name_observation_count": sum(1 for item in procedure_pairs if item["observations"]["same_name"]),
            "different_name_observation_count": sum(1 for item in procedure_pairs if not item["observations"]["same_name"]),
            "evidence_difference_count": len(evidence_differences),
            "left_unpaired_count": len(left_unpaired),
            "right_unpaired_count": len(right_unpaired),
        },
        "segment_pairs": pairs,
        "procedure_pairs": procedure_pairs,
        "evidence_differences": evidence_differences,
        "left_unpaired": left_unpaired,
        "right_unpaired": right_unpaired,
    }
