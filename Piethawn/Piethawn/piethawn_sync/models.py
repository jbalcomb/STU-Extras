"""Data models for reusable Piethawn sync artifacts."""

from __future__ import annotations

from dataclasses import dataclass
from typing import Any, Dict, List, Optional


@dataclass(frozen=True)
class ProcedureRecord:
    database: str
    name: str
    segment: str
    procedure_ordinal: int
    start_ea: str
    end_ea: str
    size: int

    def to_dict(self) -> Dict[str, Any]:
        return {
            "database": self.database,
            "name": self.name,
            "segment": self.segment,
            "procedure_ordinal": self.procedure_ordinal,
            "start_ea": self.start_ea,
            "end_ea": self.end_ea,
            "size": self.size,
        }


@dataclass(frozen=True)
class ProcedureInventory:
    database: str
    source_path: str
    idb_path: Optional[str]
    input_file: Optional[str]
    function_count: int
    procedures: List[ProcedureRecord]

    def to_dict(self) -> Dict[str, Any]:
        return {
            "schema_version": 1,
            "artifact": "procedure_inventory",
            "database": self.database,
            "source_path": self.source_path,
            "idb_path": self.idb_path,
            "input_file": self.input_file,
            "function_count": self.function_count,
            "procedures": [procedure.to_dict() for procedure in self.procedures],
        }
