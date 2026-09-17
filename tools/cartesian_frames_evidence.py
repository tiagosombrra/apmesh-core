#!/usr/bin/env python3
"""Report-only validation and comparison for the pre-registered CF0-CF7 protocol."""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
import pathlib
import re
import struct
from itertools import permutations, product
from typing import Any


class EvidenceError(RuntimeError):
    pass


ZERO_2 = ["0x0p+0", "0x0p+0"]
ZERO_3 = ["0x0p+0", "0x0p+0", "0x0p+0"]
IDENTITY_2 = ["0x1p+0", "0x0p+0", "0x0p+0", "0x1p+0"]
IDENTITY_3 = ["0x1p+0", "0x0p+0", "0x0p+0", "0x0p+0", "0x1p+0", "0x0p+0", "0x0p+0", "0x0p+0", "0x1p+0"]


def fixed_inputs(origin: list[str], basis: list[str], exponent: int, value: list[str]) -> dict[str, Any]:
    return {"origin": origin, "basis": basis, "scale_exponent": exponent, "value": value}


FIXED_CASES: dict[str, dict[str, Any]] = {
    "frame2_scale_m1023": {"dimension": 2, "operation": "frame_construction", "outcome": "value", "error": None, "inputs": fixed_inputs(ZERO_2, IDENTITY_2, -1023, [])},
    "frame3_scale_p1023": {"dimension": 3, "operation": "frame_construction", "outcome": "value", "error": None, "inputs": fixed_inputs(ZERO_3, IDENTITY_3, 1023, [])},
    "frame2_duplicate_basis": {"dimension": 2, "operation": "frame_construction", "outcome": "error", "error": "invalid_frame", "inputs": fixed_inputs(ZERO_2, ["0x1p+0", "0x0p+0", "0x1p+0", "0x0p+0"], 0, [])},
    "frame3_duplicate_basis": {"dimension": 3, "operation": "frame_construction", "outcome": "error", "error": "invalid_frame", "inputs": fixed_inputs(ZERO_3, ["0x1p+0", "0x0p+0", "0x0p+0", "0x1p+0", "0x0p+0", "0x0p+0", "0x0p+0", "0x0p+0", "0x1p+0"], 0, [])},
    "frame2_non_unit_basis": {"dimension": 2, "operation": "frame_construction", "outcome": "error", "error": "invalid_frame", "inputs": fixed_inputs(ZERO_2, ["0x1p+1", "0x0p+0", "0x0p+0", "0x1p+0"], 0, [])},
    "frame3_missing_basis": {"dimension": 3, "operation": "frame_construction", "outcome": "error", "error": "invalid_frame", "inputs": fixed_inputs(ZERO_3, ["0x0p+0", "0x0p+0", "0x0p+0", "0x0p+0", "0x0p+0", "0x0p+0", "0x0p+0", "0x0p+0", "0x1p+0"], 0, [])},
    "point2_nan": {"dimension": 2, "operation": "point_construction", "outcome": "error", "error": "non_finite_input", "inputs": fixed_inputs(ZERO_2, IDENTITY_2, 0, ["nan", "0x0p+0"])},
    "point3_nan": {"dimension": 3, "operation": "point_construction", "outcome": "error", "error": "non_finite_input", "inputs": fixed_inputs(ZERO_3, IDENTITY_3, 0, ["nan", "0x0p+0", "0x0p+0"])},
    "mat2_nan": {"dimension": 2, "operation": "basis_construction", "outcome": "error", "error": "non_finite_input", "inputs": fixed_inputs(ZERO_2, ["nan", "0x0p+0", "0x0p+0", "0x1p+0"], 0, [])},
    "mat3_nan": {"dimension": 3, "operation": "basis_construction", "outcome": "error", "error": "non_finite_input", "inputs": fixed_inputs(ZERO_3, ["nan", "0x0p+0", "0x0p+0", "0x0p+0", "0x1p+0", "0x0p+0", "0x0p+0", "0x0p+0", "0x1p+0"], 0, [])},
    "frame2_scale_m1075": {"dimension": 2, "operation": "frame_construction", "outcome": "error", "error": "scale_out_of_range", "inputs": fixed_inputs(ZERO_2, IDENTITY_2, -1075, [])},
    "frame2_scale_m1024": {"dimension": 2, "operation": "frame_construction", "outcome": "error", "error": "scale_out_of_range", "inputs": fixed_inputs(ZERO_2, IDENTITY_2, -1024, [])},
    "frame2_scale_p1024": {"dimension": 2, "operation": "frame_construction", "outcome": "error", "error": "scale_out_of_range", "inputs": fixed_inputs(ZERO_2, IDENTITY_2, 1024, [])},
    "frame3_scale_int_min": {"dimension": 3, "operation": "frame_construction", "outcome": "error", "error": "scale_out_of_range", "inputs": fixed_inputs(ZERO_3, IDENTITY_3, -2147483648, [])},
    "frame2_overflow_point": {"dimension": 2, "operation": "point_to_world", "outcome": "error", "error": "non_finite_result", "inputs": fixed_inputs(ZERO_2, IDENTITY_2, 1, ["0x1.fffffffffffffp+1023", "0x0p+0"])},
    "frame3_overflow_vector": {"dimension": 3, "operation": "vector_to_world", "outcome": "error", "error": "non_finite_result", "inputs": fixed_inputs(ZERO_3, IDENTITY_3, 1, ["0x1.fffffffffffffp+1023", "0x0p+0", "0x0p+0"])},
}

CERTIFICATE_NEGATIVE_CASES = ["duplicate_case", "unexpected_case", "forged_expected_value", "wrong_error_classification", "noncanonical_comparison_policy"]
RETENTION_NEGATIVE_CASES = [
    "missing_retained_hash",
    "rehashed_failure_records",
    "forged_detached_verification",
    "rehashed_command_provenance",
    "sealed_failure_verification",
]


def reject_duplicates(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise EvidenceError(f"duplicate JSON key: {key}")
        result[key] = value
    return result


def read_json(path: pathlib.Path) -> Any:
    try:
        return json.loads(path.read_text(encoding="utf-8"), object_pairs_hook=reject_duplicates)
    except (OSError, json.JSONDecodeError) as error:
        raise EvidenceError(f"invalid JSON: {path}") from error


def write_json(path: pathlib.Path, value: Any) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, sort_keys=True, indent=2) + "\n", encoding="utf-8")


def sha256(path: pathlib.Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def require_keys(value: dict[str, Any], expected: set[str], context: str) -> None:
    if set(value) != expected:
        raise EvidenceError(f"{context} keys differ")


def bits(value: float) -> bytes:
    return struct.pack(">d", value)


def floats(value: Any, context: str) -> list[float]:
    if not isinstance(value, list):
        raise EvidenceError(f"{context} is not a list")
    result: list[float] = []
    for entry in value:
        if not isinstance(entry, str):
            raise EvidenceError(f"{context} contains a non-hex value")
        try:
            parsed = float.fromhex(entry)
        except ValueError as error:
            raise EvidenceError(f"{context} contains invalid hex") from error
        if not math.isfinite(parsed):
            raise EvidenceError(f"{context} contains non-finite value")
        result.append(parsed)
    return result


def same_values(left: list[float], right: list[float]) -> bool:
    if len(left) != len(right):
        return False
    return all((a == 0.0 and b == 0.0) or bits(a) == bits(b) for a, b in zip(left, right))


def basis_id(permutation_value: tuple[int, ...], signs: tuple[int, ...]) -> str:
    return "p" + "".join(str(value) for value in permutation_value) + "_s" + "".join("p" if value > 0 else "m" for value in signs)


def expected_bases(dimension: int) -> dict[str, tuple[tuple[int, ...], tuple[int, ...]]]:
    return {
        basis_id(permutation_value, signs): (permutation_value, signs)
        for permutation_value in permutations(range(dimension))
        for signs in product((1, -1), repeat=dimension)
    }


def validate_profile(path: pathlib.Path) -> dict[str, Any]:
    value = read_json(path)
    if not isinstance(value, dict):
        raise EvidenceError("profile is not an object")
    require_keys(value, {
        "schema_version", "kind", "repetitions_per_cell", "cells", "gates",
        "scale_exponents", "boundary_scale_exponents", "basis_enumeration",
        "operations", "fixtures", "accepted_boundary_cases", "adversarial_cases",
        "certificate_negative_cases", "retention_negative_cases",
        "exact_prerequisite_tests", "equivalence", "volatile_fields", "limitations",
    }, "profile")
    expected_cells = [
        {"id": "gcc-debug", "compiler": "GCC 13", "library": "libstdc++", "build_type": "Debug"},
        {"id": "gcc-release", "compiler": "GCC 13", "library": "libstdc++", "build_type": "Release"},
        {"id": "clang-debug", "compiler": "Clang 18", "library": "libc++", "build_type": "Debug"},
        {"id": "clang-release", "compiler": "Clang 18", "library": "libc++", "build_type": "Release"},
    ]
    if value["schema_version"] != 1 or value["kind"] != "cartesian-frames-qualification-profile":
        raise EvidenceError("profile identity differs")
    if value["repetitions_per_cell"] != 3 or value["cells"] != expected_cells:
        raise EvidenceError("profile matrix differs")
    if value["gates"] != [f"CF{index}" for index in range(8)]:
        raise EvidenceError("profile gates differ")
    if value["scale_exponents"] != [-8, -1, 0, 1, 8]:
        raise EvidenceError("profile ordinary scale cases differ")
    if value["boundary_scale_exponents"] != [-1075, -1024, -1023, 1023, 1024, -2147483648]:
        raise EvidenceError("profile scale boundary cases differ")
    if value["operations"] != [
        "point_to_world", "vector_to_world", "point_local_world_round_trip",
        "vector_local_world_round_trip", "point_world_local_round_trip",
        "vector_world_local_round_trip",
    ]:
        raise EvidenceError("profile operations differ")
    if value["equivalence"] != {"rule": "exact_hex", "signed_zero_policy": "normalize_to_positive", "proximity_policy": None}:
        raise EvidenceError("profile equivalence differs")
    if not isinstance(value["basis_enumeration"], dict) or set(value["basis_enumeration"]) != {"2", "3"}:
        raise EvidenceError("profile basis enumeration differs")
    for dimension in (2, 3):
        entry = value["basis_enumeration"][str(dimension)]
        if not isinstance(entry, dict) or set(entry) != {"permutations", "sign_patterns"}:
            raise EvidenceError("profile basis enumeration schema differs")
        declared = {
            basis_id(tuple(permutation_value), tuple(signs))
            for permutation_value in entry["permutations"]
            for signs in entry["sign_patterns"]
        }
        if declared != set(expected_bases(dimension)):
            raise EvidenceError(f"profile basis enumeration differs for {dimension}D")
        fixture = value["fixtures"].get(str(dimension)) if isinstance(value["fixtures"], dict) else None
        if not isinstance(fixture, dict) or set(fixture) != {"origin", "point", "vector"}:
            raise EvidenceError(f"profile fixture differs for {dimension}D")
        for field in ("origin", "point", "vector"):
            if len(floats(fixture[field], f"profile {dimension}D {field}")) != dimension:
                raise EvidenceError(f"profile fixture dimension differs for {dimension}D")
    if value["accepted_boundary_cases"] != ["frame2_scale_m1023", "frame3_scale_p1023"]:
        raise EvidenceError("profile accepted boundary identities differ")
    if value["adversarial_cases"] != [
        "frame2_duplicate_basis", "frame3_duplicate_basis", "frame2_non_unit_basis", "frame3_missing_basis",
        "point2_nan", "point3_nan", "mat2_nan", "mat3_nan", "frame2_scale_m1075", "frame2_scale_m1024",
        "frame2_scale_p1024", "frame3_scale_int_min", "frame2_overflow_point", "frame3_overflow_vector",
    ]:
        raise EvidenceError("profile adversarial identities differ")
    if value["certificate_negative_cases"] != CERTIFICATE_NEGATIVE_CASES or value["retention_negative_cases"] != RETENTION_NEGATIVE_CASES:
        raise EvidenceError("profile negative identities differ")
    expected_prerequisites = [
        "apmesh_core.bootstrap_smoke", "apmesh_core.bootstrap_export", "apmesh_core.bootstrap_tool",
        "apmesh_core.architecture_bootstrap_runner", "apmesh_core.numeric_contract", "apmesh_core.numeric_contract_evidence",
        "apmesh_core.numeric_contract_runner", "apmesh_core.reproducible_experiment_evidence",
        "apmesh_core.reproducible_experiment_runner", "apmesh_core.reproducible_experiment_retention",
        "apmesh_core.reproducible_experiment_negatives", "apmesh_core.geometry_primitives",
        "apmesh_core.geometry_point_vector_evidence", "apmesh_core.geometry_point_vector_runner",
        "apmesh_core.geometry_point_vector_retention", "apmesh_core.geometry_point_vector_foundation_preservation",
        "apmesh_core.minimal_small_linear_algebra", "apmesh_core.math_header_isolation",
        "apmesh_core.minimal_small_linear_algebra_evidence", "apmesh_core.minimal_small_linear_algebra_runner",
        "apmesh_core.minimal_small_linear_algebra_retention",
    ]
    if value["exact_prerequisite_tests"] != expected_prerequisites:
        raise EvidenceError("profile prerequisite allowlist differs")
    if not all(isinstance(value[field], list) for field in ("accepted_boundary_cases", "adversarial_cases", "certificate_negative_cases", "retention_negative_cases", "exact_prerequisite_tests", "volatile_fields", "limitations")):
        raise EvidenceError("profile list field differs")
    return value


def semantic_ids(profile: dict[str, Any]) -> dict[str, tuple[int, tuple[int, ...], tuple[int, ...], int, str]]:
    result: dict[str, tuple[int, tuple[int, ...], tuple[int, ...], int, str]] = {}
    for dimension in (2, 3):
        for name, (permutation_value, signs) in expected_bases(dimension).items():
            for exponent in profile["scale_exponents"]:
                for operation in profile["operations"]:
                    identifier = f"d{dimension}_{name}_k{exponent}_{operation}"
                    result[identifier] = (dimension, permutation_value, signs, exponent, operation)
    return result


def direct_world(origin: list[float], permutation_value: tuple[int, ...], signs: tuple[int, ...], exponent: int, local: list[float], point: bool) -> list[float]:
    scale = math.ldexp(1.0, exponent)
    return [(origin[row] if point else 0.0) + scale * signs[row] * local[permutation_value[row]] for row in range(len(origin))]


def expected_case(profile: dict[str, Any], identifier: str) -> tuple[str, str | None, list[float] | None, dict[str, Any]]:
    semantic = semantic_ids(profile).get(identifier)
    if semantic is not None:
        dimension, permutation_value, signs, exponent, operation = semantic
        fixture = profile["fixtures"][str(dimension)]
        origin = floats(fixture["origin"], "origin")
        point = floats(fixture["point"], "point")
        vector = floats(fixture["vector"], "vector")
        point_world = direct_world(origin, permutation_value, signs, exponent, point, True)
        vector_world = direct_world(origin, permutation_value, signs, exponent, vector, False)
        expected = {
            "point_to_world": point_world,
            "vector_to_world": vector_world,
            "point_local_world_round_trip": point,
            "vector_local_world_round_trip": vector,
            "point_world_local_round_trip": point_world,
            "vector_world_local_round_trip": vector_world,
        }[operation]
        input_value = point if operation.startswith("point_") else vector
        if operation.startswith("point_world"):
            input_value = point_world
        if operation.startswith("vector_world"):
            input_value = vector_world
        basis = [0.0] * (dimension * dimension)
        for row in range(dimension):
            basis[row * dimension + permutation_value[row]] = float(signs[row])
        return "value", None, expected, {"origin": origin, "basis": basis, "scale_exponent": exponent, "value": input_value}
    if identifier in FIXED_CASES:
        fixed = FIXED_CASES[identifier]
        return fixed["outcome"], fixed["error"], [] if fixed["outcome"] == "value" else None, fixed["inputs"]
    raise EvidenceError(f"unexpected certificate case: {identifier}")


def parse_result(value: Any, context: str) -> tuple[str, str | None, list[float] | None]:
    if not isinstance(value, dict):
        raise EvidenceError(f"{context} is malformed")
    require_keys(value, {"outcome", "error", "value"}, context)
    if value["outcome"] == "value" and value["error"] is None:
        return "value", None, floats(value["value"], f"{context}.value")
    if value["outcome"] == "error" and isinstance(value["error"], str) and value["value"] is None:
        return "error", value["error"], None
    raise EvidenceError(f"{context} result schema differs")


def validate_case(profile: dict[str, Any], case: Any) -> dict[str, Any]:
    if not isinstance(case, dict):
        raise EvidenceError("certificate case is not an object")
    require_keys(case, {
        "schema_version", "id", "dimension", "operation", "claim_category", "inputs",
        "expected", "observed", "comparison", "non_claims",
    }, "certificate case")
    if case["schema_version"] != 1 or not isinstance(case["id"], str):
        raise EvidenceError("certificate case identity differs")
    expected_outcome, expected_error, expected_values, expected_inputs = expected_case(profile, case["id"])
    if case["claim_category"] != "cartesian_similarity":
        raise EvidenceError("certificate category differs")
    if not isinstance(case["inputs"], dict) or set(case["inputs"]) != {"origin", "basis", "scale_exponent", "value"}:
        raise EvidenceError("certificate inputs differ")
    semantic = semantic_ids(profile).get(case["id"])
    if semantic is not None:
        if case["dimension"] != semantic[0] or case["operation"] != semantic[-1]:
            raise EvidenceError("semantic certificate metadata differs")
        actual_inputs = {
            "origin": floats(case["inputs"]["origin"], "case origin"),
            "basis": floats(case["inputs"]["basis"], "case basis"),
            "scale_exponent": case["inputs"]["scale_exponent"],
            "value": floats(case["inputs"]["value"], "case value"),
        }
        if actual_inputs["scale_exponent"] != expected_inputs["scale_exponent"] or any(not same_values(actual_inputs[key], expected_inputs[key]) for key in ("origin", "basis", "value")):
            raise EvidenceError("semantic certificate inputs differ")
    elif case["id"] in FIXED_CASES:
        fixed = FIXED_CASES[case["id"]]
        if case["dimension"] != fixed["dimension"] or case["operation"] != fixed["operation"] or case["inputs"] != expected_inputs:
            raise EvidenceError("adversarial certificate metadata differs")
    else:
        raise EvidenceError("certificate case is not preregistered")
    for field in ("expected", "observed"):
        outcome, error, values = parse_result(case[field], f"{case['id']} {field}")
        if outcome != expected_outcome or error != expected_error or (expected_values is not None and not same_values(values or [], expected_values)):
            raise EvidenceError(f"{case['id']} {field} differs from independent oracle")
    comparison = case["comparison"]
    if comparison != {"rule": "exact_hex", "signed_zero_policy": "normalize_to_positive", "exact_match": True, "proximity_policy": None}:
        raise EvidenceError(f"{case['id']} comparison differs")
    if case["non_claims"] != ["orientation", "handedness", "rank", "incidence", "topology", "general_invertibility"]:
        raise EvidenceError(f"{case['id']} non-claims differ")
    return {"id": case["id"], "expected": case["expected"], "observed": case["observed"], "comparison": comparison}


def validate_certificate(profile: dict[str, Any], path: pathlib.Path) -> dict[str, Any]:
    value = read_json(path)
    if not isinstance(value, dict):
        raise EvidenceError("certificate is not an object")
    require_keys(value, {"schema_version", "kind", "environment", "cases"}, "certificate")
    if value["schema_version"] != 1 or value["kind"] != "cartesian-frames-certificate":
        raise EvidenceError("certificate identity differs")
    if value["environment"] != {"double_radix": 2, "double_digits": 53, "iec559": True}:
        raise EvidenceError("certificate floating environment differs")
    if not isinstance(value["cases"], list):
        raise EvidenceError("certificate cases differ")
    expected_ids = set(semantic_ids(profile)) | set(profile["accepted_boundary_cases"]) | set(profile["adversarial_cases"])
    seen: set[str] = set()
    projection: list[dict[str, Any]] = []
    for case in value["cases"]:
        if not isinstance(case, dict) or case.get("id") in seen:
            raise EvidenceError("duplicate or malformed certificate case")
        seen.add(case["id"])
        projection.append(validate_case(profile, case))
    if seen != expected_ids:
        raise EvidenceError("certificate case set differs")
    return {"schema_version": 1, "kind": "cartesian-frames-semantic-projection", "cases": projection}


def validate_source(source_root: pathlib.Path) -> dict[str, str]:
    header = (source_root / "include/apmesh/core/geometry.hpp").read_text(encoding="utf-8")
    source = (source_root / "src/core/geometry.cpp").read_text(encoding="utf-8")
    for forbidden in ("Transform2", "Transform3", "quaternion", "projective", "std::sin", "std::cos"):
        if forbidden in header or forbidden in source:
            raise EvidenceError(f"excluded Cartesian Frames capability present: {forbidden}")
    if "class CartesianFrame2" not in header or "class CartesianFrame3" not in header:
        raise EvidenceError("Cartesian Frame public types are absent")
    if "is_signed_permutation_basis" not in source or "scale_is_reciprocal_safe" not in source:
        raise EvidenceError("Cartesian Frame bounded validation is absent")
    return {
        "bounded_cartesian_frames_only": "PASS",
        "no_general_transforms": "PASS",
        "no_hidden_tolerance": "PASS",
    }


def certificate_projections(profile: dict[str, Any], index_path: pathlib.Path) -> dict[tuple[str, int], dict[str, Any]]:
    index = read_json(index_path)
    if not isinstance(index, dict):
        raise EvidenceError("certificate index is not an object")
    require_keys(index, {"schema_version", "kind", "entries"}, "certificate index")
    if index["schema_version"] != 1 or index["kind"] != "cartesian-frames-certificate-index" or not isinstance(index["entries"], list):
        raise EvidenceError("certificate index identity differs")
    expected_slots = [{"cell": cell["id"], "repetition": repetition} for cell in profile["cells"] for repetition in range(1, profile["repetitions_per_cell"] + 1)]
    projections: dict[tuple[str, int], dict[str, Any]] = {}
    for entry in index["entries"]:
        if not isinstance(entry, dict) or set(entry) != {"cell", "repetition", "path", "sha256"}:
            raise EvidenceError("certificate index entry differs")
        if not isinstance(entry["cell"], str) or not isinstance(entry["repetition"], int):
            raise EvidenceError("certificate index slot differs")
        certificate = (index_path.parent / entry["path"]).resolve()
        if not certificate.is_file() or sha256(certificate) != entry["sha256"]:
            raise EvidenceError("certificate index hash differs")
        slot = (entry["cell"], entry["repetition"])
        if slot in projections:
            raise EvidenceError("certificate index contains a duplicate slot")
        projections[slot] = validate_certificate(profile, certificate)
    if set(projections) != {(item["cell"], item["repetition"]) for item in expected_slots}:
        raise EvidenceError("certificate index slots differ")
    return projections


def compare_per_cell_certificates(profile: dict[str, Any], index_path: pathlib.Path) -> dict[str, Any]:
    projections = certificate_projections(profile, index_path)
    cells: list[dict[str, Any]] = []
    for cell in profile["cells"]:
        name = cell["id"]
        baseline = projections[(name, 1)]
        repetitions = list(range(1, profile["repetitions_per_cell"] + 1))
        if any(projections[(name, repetition)] != baseline for repetition in repetitions[1:]):
            raise EvidenceError("per-cell semantic projection differs")
        cells.append({
            "cell": name,
            "baseline_repetition": 1,
            "compared_repetitions": repetitions[1:],
            "status": "EVIDENCE_COLLECTED_PENDING_AUDIT",
        })
    return {
        "schema_version": 1,
        "kind": "cartesian-frames-per-cell-comparisons",
        "status": "EVIDENCE_COLLECTED_PENDING_AUDIT",
        "cells": cells,
    }


def compare_certificates(profile: dict[str, Any], index_path: pathlib.Path) -> dict[str, Any]:
    projections = certificate_projections(profile, index_path)
    expected_slots = [{"cell": cell["id"], "repetition": repetition} for cell in profile["cells"] for repetition in range(1, profile["repetitions_per_cell"] + 1)]
    baseline = projections[(expected_slots[0]["cell"], expected_slots[0]["repetition"])]
    if any(projections[(slot["cell"], slot["repetition"])] != baseline for slot in expected_slots[1:]):
        raise EvidenceError("cross-cell semantic projection differs")
    return {"schema_version": 1, "kind": "cartesian-frames-cross-cell-comparison", "status": "EVIDENCE_COLLECTED_PENDING_AUDIT", "slots": expected_slots}


def gate_summary(profile: dict[str, Any], limitations: list[str]) -> dict[str, Any]:
    return {
        "schema_version": 1,
        "kind": "cartesian-frames-gate-summary",
        "state": "EVIDENCE_COLLECTED_PENDING_AUDIT",
        "gates": {gate: "EVIDENCE_COLLECTED_PENDING_AUDIT" for gate in profile["gates"]},
        "limitations": limitations,
    }


def render_gate_summary_markdown(summary: dict[str, Any]) -> str:
    if not isinstance(summary, dict) or set(summary) != {"schema_version", "kind", "state", "gates", "limitations"} or summary.get("schema_version") != 1 or summary.get("kind") != "cartesian-frames-gate-summary" or summary.get("state") != "EVIDENCE_COLLECTED_PENDING_AUDIT" or not isinstance(summary.get("gates"), dict) or not isinstance(summary.get("limitations"), list):
        raise EvidenceError("gate summary differs")
    rows = "\n".join(f"| `{gate}` | `{state}` |" for gate, state in summary["gates"].items())
    limitations = "\n".join(f"- {item}" for item in summary["limitations"])
    return (
        "# Cartesian Frames gate summary\n\n"
        "Status: `EVIDENCE_COLLECTED_PENDING_AUDIT`\n\n"
        "| Gate | Evidence state |\n"
        "| --- | --- |\n"
        f"{rows}\n\n"
        "## Retained limitations\n\n"
        f"{limitations}\n"
    )


def validate_per_cell_comparisons(profile: dict[str, Any], index_path: pathlib.Path, comparison_path: pathlib.Path) -> dict[str, Any]:
    expected = compare_per_cell_certificates(profile, index_path)
    if read_json(comparison_path) != expected:
        raise EvidenceError("per-cell comparison recomputation differs")
    return expected


def validate_gate_summary_markdown(summary: dict[str, Any], path: pathlib.Path) -> None:
    if path.read_text(encoding="utf-8") != render_gate_summary_markdown(summary):
        raise EvidenceError("gate summary Markdown differs")


def negative_outcomes(profile: dict[str, Any], certificate: pathlib.Path, output: pathlib.Path) -> None:
    baseline = read_json(certificate)
    if not isinstance(baseline, dict):
        raise EvidenceError("baseline certificate differs")
    mutations: dict[str, Any] = {}
    duplicate = copy.deepcopy(baseline); duplicate["cases"].append(copy.deepcopy(duplicate["cases"][0])); mutations["duplicate_case"] = duplicate
    unexpected = copy.deepcopy(baseline); unexpected["cases"][0]["id"] = "unexpected_case"; mutations["unexpected_case"] = unexpected
    expected_value = copy.deepcopy(baseline); expected_value["cases"][0]["expected"]["value"][0] = "0x1p+99"; mutations["forged_expected_value"] = expected_value
    wrong_error = copy.deepcopy(baseline); target = next(item for item in wrong_error["cases"] if item["id"] == "frame2_duplicate_basis"); target["observed"]["error"] = "scale_out_of_range"; mutations["wrong_error_classification"] = wrong_error
    policy = copy.deepcopy(baseline); policy["cases"][0]["comparison"]["signed_zero_policy"] = "raw"; mutations["noncanonical_comparison_policy"] = policy
    outcomes: list[dict[str, str]] = []
    for name in profile["certificate_negative_cases"]:
        try:
            temporary = output.parent / f".{name}.json"
            write_json(temporary, mutations[name])
            validate_certificate(profile, temporary)
        except EvidenceError:
            outcomes.append({"id": name, "result": "REJECTED"})
        else:
            raise EvidenceError(f"negative mutation was accepted: {name}")
    write_json(output, {"schema_version": 1, "kind": "cartesian-frames-negative-outcomes", "outcomes": outcomes})


def validate_negative_outcomes(profile: dict[str, Any], path: pathlib.Path) -> dict[str, Any]:
    value = read_json(path)
    if not isinstance(value, dict):
        raise EvidenceError("negative outcomes are not an object")
    require_keys(value, {"schema_version", "kind", "outcomes"}, "negative outcomes")
    expected = [{"id": item, "result": "REJECTED"} for item in profile["certificate_negative_cases"]]
    if value["schema_version"] != 1 or value["kind"] != "cartesian-frames-negative-outcomes" or value["outcomes"] != expected:
        raise EvidenceError("negative outcomes differ")
    return value


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--profile", required=True)
    commands = parser.add_subparsers(dest="command", required=True)
    source = commands.add_parser("validate-source"); source.add_argument("--source-root", required=True); source.add_argument("--output", required=True)
    certificate = commands.add_parser("validate-certificate"); certificate.add_argument("--certificate", required=True)
    negative_validation = commands.add_parser("validate-negative-outcomes"); negative_validation.add_argument("--outcomes", required=True)
    compare = commands.add_parser("compare"); compare.add_argument("--index", required=True); compare.add_argument("--report", required=True)
    negatives = commands.add_parser("negative-outcomes"); negatives.add_argument("--certificate", required=True); negatives.add_argument("--output", required=True)
    collect = commands.add_parser("collect"); collect.add_argument("--index", required=True); collect.add_argument("--output", required=True)
    arguments = parser.parse_args()
    try:
        profile = validate_profile(pathlib.Path(arguments.profile))
        if arguments.command == "validate-source":
            write_json(pathlib.Path(arguments.output), validate_source(pathlib.Path(arguments.source_root)))
        elif arguments.command == "validate-certificate":
            validate_certificate(profile, pathlib.Path(arguments.certificate))
        elif arguments.command == "validate-negative-outcomes":
            validate_negative_outcomes(profile, pathlib.Path(arguments.outcomes))
        elif arguments.command == "compare":
            comparison = compare_certificates(profile, pathlib.Path(arguments.index))
            pathlib.Path(arguments.report).write_text("# Cartesian Frames report-only comparison\n\nQualification: EVIDENCE_COLLECTED_PENDING_AUDIT\n", encoding="utf-8")
            print(json.dumps(comparison, sort_keys=True))
        elif arguments.command == "negative-outcomes":
            negative_outcomes(profile, pathlib.Path(arguments.certificate), pathlib.Path(arguments.output))
        else:
            comparison = compare_certificates(profile, pathlib.Path(arguments.index))
            write_json(pathlib.Path(arguments.output), comparison)
    except EvidenceError as error:
        print(f"evidence error: {error}", file=__import__("sys").stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
