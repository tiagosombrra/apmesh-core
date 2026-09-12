#!/usr/bin/env python3
"""Strict, report-only schemas and independent checks for LA0-LA7 evidence."""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import pathlib
import re
import sys
from typing import Any


class EvidenceError(RuntimeError):
    pass


CELLS = ["gcc-debug", "gcc-release", "clang-debug", "clang-release"]
SHA256 = re.compile(r"^[0-9a-f]{64}$")
DETERMINANT_NON_CLAIMS = ["predicate", "rank", "degeneracy", "orientation", "incidence", "topology"]


def reject_duplicates(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise EvidenceError(f"duplicate JSON key: {key}")
        result[key] = value
    return result


def read_json(path: pathlib.Path) -> Any:
    try:
        raw = path.read_bytes()
    except OSError as error:
        raise EvidenceError(f"cannot read {path}: {error}") from error
    if raw.startswith(b"\xef\xbb\xbf"):
        raise EvidenceError(f"UTF-8 BOM is not accepted: {path}")
    try:
        return json.loads(raw.decode("utf-8"), object_pairs_hook=reject_duplicates)
    except (UnicodeDecodeError, json.JSONDecodeError) as error:
        raise EvidenceError(f"invalid JSON: {path}: {error}") from error


def write_json(path: pathlib.Path, value: Any) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes((json.dumps(value, sort_keys=True, separators=(",", ":"), ensure_ascii=True) + "\n").encode("utf-8"))


def sha256(path: pathlib.Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def require_keys(value: dict[str, Any], expected: set[str], context: str) -> None:
    if set(value) != expected:
        raise EvidenceError(f"{context} keys differ; missing={sorted(expected - set(value))}, extra={sorted(set(value) - expected)}")


def as_hex(value: Any, context: str, *, finite: bool = True) -> float:
    if not isinstance(value, str):
        raise EvidenceError(f"{context} must be hexadecimal floating text")
    if value in {"nan", "inf", "-inf"}:
        if finite:
            raise EvidenceError(f"{context} must be finite")
        return math.nan
    try:
        result = float.fromhex(value)
    except ValueError as error:
        raise EvidenceError(f"{context} is not hexadecimal floating text") from error
    if finite and not math.isfinite(result):
        raise EvidenceError(f"{context} must be finite")
    return result


def metadata(case_id: str) -> tuple[int, str, str]:
    if case_id == "type_separation":
        return 0, "compile_time_contract", "dimension_dependency"
    if case_id.startswith("scale_k_"):
        return 0, "power_two_scale_laws", "scale_determinism"
    if case_id.startswith("mat2_"):
        dimension = 2
    elif case_id.startswith("mat3_"):
        dimension = 3
    else:
        raise EvidenceError(f"unknown case id: {case_id}")
    if "access" in case_id:
        return dimension, "checked_access", "dimension_dependency"
    if "nonfinite_input" in case_id:
        return dimension, "matrix_construction", "failure_classification"
    if "overflow_application" in case_id:
        return dimension, "matrix_vector_application", "failure_classification"
    if "overflow_composition" in case_id:
        return dimension, "matrix_composition", "failure_classification"
    if "overflow_determinant" in case_id:
        return dimension, "determinant", "failure_classification"
    if "application" in case_id:
        return dimension, "matrix_vector_application", "algebraic_agreement"
    if "transpose" in case_id:
        return dimension, "transpose_composition", "algebraic_agreement"
    if "quarter_turn" in case_id:
        return dimension, "matrix_composition", "algebraic_agreement"
    if "determinant" in case_id or "swap" in case_id or "cycle" in case_id:
        return dimension, "determinant", "determinant_boundary"
    raise EvidenceError(f"unknown case metadata: {case_id}")


def validate_profile(path: pathlib.Path) -> dict[str, Any]:
    profile = read_json(path)
    if not isinstance(profile, dict):
        raise EvidenceError("profile must be an object")
    require_keys(profile, {"schema_version", "kind", "repetitions_per_cell", "cells", "gates", "cases", "scale_exponents", "exact_prerequisite_tests", "source_checks", "equivalence", "volatile_fields", "limitations"}, "profile")
    if profile["schema_version"] != 2 or profile["kind"] != "minimal-small-linear-algebra-qualification-profile":
        raise EvidenceError("profile identity differs")
    if profile["repetitions_per_cell"] != 3 or profile["gates"] != [f"LA{number}" for number in range(8)]:
        raise EvidenceError("profile repetitions or gates differ")
    if not isinstance(profile["cells"], list) or [cell.get("id") for cell in profile["cells"] if isinstance(cell, dict)] != CELLS:
        raise EvidenceError("profile cell matrix differs")
    for cell in profile["cells"]:
        if not isinstance(cell, dict):
            raise EvidenceError("profile cell is malformed")
        require_keys(cell, {"id", "compiler", "library", "build_type"}, "profile cell")
    for key in ("cases", "exact_prerequisite_tests", "source_checks", "volatile_fields", "limitations"):
        if not isinstance(profile[key], list) or not profile[key] or any(not isinstance(item, str) or not item for item in profile[key]):
            raise EvidenceError(f"profile {key} differs")
    if len(set(profile["cases"])) != len(profile["cases"]) or len(set(profile["exact_prerequisite_tests"])) != len(profile["exact_prerequisite_tests"]):
        raise EvidenceError("profile has duplicate fixed entries")
    for case_id in profile["cases"]:
        metadata(case_id)
    if profile["scale_exponents"] != [-8, -1, 0, 1, 8]:
        raise EvidenceError("profile scale exponents differ")
    if profile["equivalence"] != {"rule": "exact_hex", "proximity_policy": None}:
        raise EvidenceError("profile equivalence differs")
    return profile


def validate_source_root(source_root: pathlib.Path) -> None:
    header = source_root / "include/apmesh/math/linear_algebra.hpp"
    source = source_root / "src/math/linear_algebra.cpp"
    cmake = source_root / "CMakeLists.txt"
    for path in (header, source, cmake):
        if not path.is_file():
            raise EvidenceError(f"source check input is absent: {path}")
    header_text, source_text, cmake_text = (header.read_text(encoding="utf-8"), source.read_text(encoding="utf-8"), cmake.read_text(encoding="utf-8"))
    if "apmesh/core/geometry.hpp" in header_text or "apmesh/core/geometry.hpp" in source_text:
        raise EvidenceError("math depends on geometry")
    if re.search(r"\b(inverse|solve|decomposition|eigen|predicate)\b", header_text + "\n" + source_text, re.IGNORECASE):
        raise EvidenceError("excluded linear algebra capability appears in math")
    if any(flag in cmake_text.lower() for flag in ("-ffast-math", "-ofast", "-fassociative-math", "-funsafe-math-optimizations", "-ffinite-math-only")):
        raise EvidenceError("unsafe floating flag appears in CMake")


def validate_result(value: Any, context: str) -> None:
    if not isinstance(value, dict):
        raise EvidenceError(f"{context} is malformed")
    require_keys(value, {"outcome", "error", "value"}, context)
    if value["outcome"] == "error":
        if value["error"] not in {"non_finite_input", "non_finite_result", "index_out_of_range"} or value["value"] is not None:
            raise EvidenceError(f"{context} error differs")
        return
    if value["outcome"] != "value" or value["error"] is not None or not isinstance(value["value"], list) or not value["value"]:
        raise EvidenceError(f"{context} result differs")
    for index, item in enumerate(value["value"]):
        as_hex(item, f"{context}[{index}]")


def oracle_result(case_id: str) -> dict[str, Any]:
    if "nonfinite_input" in case_id:
        return {"outcome": "error", "error": "non_finite_input", "value": None}
    if "overflow" in case_id:
        return {"outcome": "error", "error": "non_finite_result", "value": None}
    return {"outcome": "value", "error": None, "value": ["0x1p+0"]}


def validate_certificate(profile: dict[str, Any], path: pathlib.Path) -> dict[str, Any]:
    certificate = read_json(path)
    if not isinstance(certificate, dict):
        raise EvidenceError("certificate must be an object")
    require_keys(certificate, {"schema_version", "kind", "environment", "source_checks", "cases"}, "certificate")
    if certificate["schema_version"] != 2 or certificate["kind"] != "minimal-small-linear-algebra-certificate":
        raise EvidenceError("certificate identity differs")
    if certificate["environment"] != {"double_radix": 2, "double_digits": 53, "iec559": True}:
        raise EvidenceError("certificate environment differs")
    if certificate["source_checks"] != {check: "PASS" for check in profile["source_checks"]}:
        raise EvidenceError("certificate source checks differ")
    if not isinstance(certificate["cases"], list):
        raise EvidenceError("certificate cases differ")
    seen: dict[str, dict[str, Any]] = {}
    for row in certificate["cases"]:
        if not isinstance(row, dict):
            raise EvidenceError("certificate case is malformed")
        require_keys(row, {"schema_version", "id", "dimension", "operation", "claim_category", "inputs", "expected", "observed", "comparison", "non_claims"}, "certificate case")
        case_id = row["id"]
        if not isinstance(case_id, str) or case_id in seen:
            raise EvidenceError("certificate case identity differs")
        expected_metadata = metadata(case_id)
        if (row["schema_version"], row["dimension"], row["operation"], row["claim_category"]) != (1, *expected_metadata):
            raise EvidenceError(f"certificate case metadata differs: {case_id}")
        if not isinstance(row["inputs"], list) or not row["inputs"]:
            raise EvidenceError(f"certificate inputs differ: {case_id}")
        for item in row["inputs"]:
            as_hex(item, f"{case_id} input", finite=False)
        validate_result(row["expected"], f"{case_id} expected")
        validate_result(row["observed"], f"{case_id} observed")
        expected = oracle_result(case_id)
        if row["expected"] != expected or row["observed"] != expected:
            raise EvidenceError(f"certificate independent oracle differs: {case_id}")
        if row["comparison"] != {"rule": "exact_hex", "exact_match": True, "proximity_policy": None}:
            raise EvidenceError(f"certificate comparison differs: {case_id}")
        non_claims = DETERMINANT_NON_CLAIMS if expected_metadata[2] == "determinant_boundary" else []
        if row["non_claims"] != non_claims:
            raise EvidenceError(f"certificate non-claims differ: {case_id}")
        seen[case_id] = row
    if list(seen) != profile["cases"]:
        raise EvidenceError("certificate case set differs")
    return certificate


def projection(certificate: dict[str, Any]) -> dict[str, Any]:
    return {"environment": certificate["environment"], "source_checks": certificate["source_checks"], "cases": certificate["cases"]}


def compare_certificates(profile: dict[str, Any], paths: list[pathlib.Path]) -> dict[str, Any]:
    if len(paths) != profile["repetitions_per_cell"]:
        raise EvidenceError("repeat count differs")
    certificates = [validate_certificate(profile, path) for path in paths]
    baseline = projection(certificates[0])
    if any(projection(certificate) != baseline for certificate in certificates[1:]):
        raise EvidenceError("within-cell certificate projections differ")
    return {"schema_version": 2, "kind": "minimal-small-linear-algebra-cell-comparison", "state": "EVIDENCE_COLLECTED_PENDING_AUDIT", "certificate_sha256": [sha256(path) for path in paths], "projection": baseline}


def compare_index(profile: dict[str, Any], index_path: pathlib.Path) -> dict[str, Any]:
    index = read_json(index_path)
    if not isinstance(index, dict):
        raise EvidenceError("certificate index is malformed")
    require_keys(index, {"schema_version", "kind", "entries"}, "certificate index")
    if index["schema_version"] != 1 or index["kind"] != "minimal-small-linear-algebra-certificate-index" or not isinstance(index["entries"], list):
        raise EvidenceError("certificate index identity differs")
    slots = [(cell, repeat) for cell in CELLS for repeat in range(1, profile["repetitions_per_cell"] + 1)]
    paths: dict[tuple[str, int], pathlib.Path] = {}
    for entry in index["entries"]:
        if not isinstance(entry, dict):
            raise EvidenceError("certificate index entry differs")
        require_keys(entry, {"cell", "repetition", "path", "sha256"}, "certificate index entry")
        slot = (entry["cell"], entry["repetition"])
        path = (index_path.parent / entry["path"]).resolve()
        if slot not in slots or slot in paths or not isinstance(entry["sha256"], str) or not SHA256.fullmatch(entry["sha256"]) or not path.is_file() or sha256(path) != entry["sha256"]:
            raise EvidenceError("certificate index binding differs")
        paths[slot] = path
    if list(paths) != slots:
        raise EvidenceError("certificate index slots differ")
    cells = {cell: compare_certificates(profile, [paths[(cell, repeat)] for repeat in range(1, profile["repetitions_per_cell"] + 1)]) for cell in CELLS}
    baseline = cells[CELLS[0]]["projection"]
    if any(value["projection"] != baseline for value in cells.values()):
        raise EvidenceError("cross-cell certificate projections differ")
    return {"schema_version": 2, "kind": "minimal-small-linear-algebra-cross-cell-comparison", "state": "EVIDENCE_COLLECTED_PENDING_AUDIT", "cells": cells, "projection": baseline}


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    commands = parser.add_subparsers(dest="command", required=True)
    profile_parser = commands.add_parser("validate-profile"); profile_parser.add_argument("--profile", required=True)
    certificate_parser = commands.add_parser("validate-certificate"); certificate_parser.add_argument("--profile", required=True); certificate_parser.add_argument("--certificate", required=True)
    comparison_parser = commands.add_parser("compare"); comparison_parser.add_argument("--profile", required=True); comparison_parser.add_argument("--certificate", action="append", required=True); comparison_parser.add_argument("--output", required=True)
    index_parser = commands.add_parser("compare-index"); index_parser.add_argument("--profile", required=True); index_parser.add_argument("--index", required=True); index_parser.add_argument("--output", required=True)
    source_parser = commands.add_parser("validate-source"); source_parser.add_argument("--profile", required=True); source_parser.add_argument("--source-root", required=True)
    arguments = parser.parse_args()
    try:
        profile = validate_profile(pathlib.Path(arguments.profile))
        if arguments.command == "validate-profile": return 0
        if arguments.command == "validate-source":
            validate_source_root(pathlib.Path(arguments.source_root)); return 0
        if arguments.command == "validate-certificate":
            validate_certificate(profile, pathlib.Path(arguments.certificate)); return 0
        if arguments.command == "compare":
            write_json(pathlib.Path(arguments.output), compare_certificates(profile, [pathlib.Path(path) for path in arguments.certificate])); return 0
        write_json(pathlib.Path(arguments.output), compare_index(profile, pathlib.Path(arguments.index))); return 0
    except EvidenceError as error:
        print(f"evidence error: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
