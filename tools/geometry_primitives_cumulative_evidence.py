#!/usr/bin/env python3

"""Report-only validation and comparison for the cumulative Geometry stage."""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import pathlib
import sys
from typing import Any

from experiment_runtime import read_json, sha256_file, write_json


class EvidenceError(RuntimeError):
    pass


CELLS = [
    {"id": "gcc-debug", "compiler": "GCC 13", "library": "libstdc++", "build_type": "Debug"},
    {"id": "gcc-release", "compiler": "GCC 13", "library": "libstdc++", "build_type": "Release"},
    {"id": "clang-debug", "compiler": "Clang 18", "library": "libc++", "build_type": "Debug"},
    {"id": "clang-release", "compiler": "Clang 18", "library": "libc++", "build_type": "Release"},
]
GATES = ["GPR0", "GPR1", "GPR2", "GPR3", "GPR4", "GPR5", "GPR6", "GPR7"]
ALLOWLIST = [
    "apmesh_core.bootstrap_smoke",
    "apmesh_core.numeric_contract",
    "apmesh_core.geometry_primitives",
    "apmesh_core.minimal_small_linear_algebra",
    "apmesh_core.math_header_isolation",
    "apmesh_core.cartesian_frames",
]
AUTHORITIES = [
    "docs/decisions/FOUNDATION_END_TO_END_REGRESSION.md",
    "docs/decisions/GEOMETRY_POINT_VECTOR_QUALIFICATION.md",
    "docs/decisions/GEOMETRY_MINIMAL_SMALL_LINEAR_ALGEBRA_QUALIFICATION.md",
    "docs/decisions/GEOMETRY_CARTESIAN_FRAMES_QUALIFICATION_PROTOCOL.md",
    "docs/decisions/GEOMETRY_PRIMITIVES_CUMULATIVE_REGRESSION_PROTOCOL.md",
]
LIMITATIONS = [
    "WSL Ubuntu 24.04 GCC 13/libstdc++ and Clang 18/libc++ only",
    "Evidence collection is report-only; PREPARED and execution do not decide a scientific result",
    "No native Windows, parallel, predicate, topology, curve, surface, or meshing qualification",
]
NON_CLAIMS = ["orientation", "predicate", "coincidence", "incidence", "topology"]
NEGATIVES = [
    "duplicate_case", "forged_integrated_result", "wrong_error_classification",
    "noncanonical_signed_zero", "undeclared_topology_claim",
]

# This table is the independent, fixed oracle for the experimental C++ exporter.
EXPECTED: dict[str, tuple[int, str, str, str | None, list[float] | None]] = {
    "frame2_affine_difference": (2, "frame_affine_difference", "value", None, [-8.0, -4.0]),
    "frame3_affine_difference": (3, "frame_affine_difference", "value", None, [-8.0, 16.0, 4.0]),
    "frame2_point_round_trip": (2, "point_round_trip", "value", None, [1.0, -2.0]),
    "frame3_point_round_trip": (3, "point_round_trip", "value", None, [1.0, -2.0, 4.0]),
    "mat2_compose_apply": (2, "compose_apply", "value", None, [-4.0, 4.0]),
    "mat3_compose_apply": (3, "compose_apply", "value", None, [4.0, 8.0, 4.0]),
    "mat2_transpose_composition": (2, "transpose_composition", "value", None, [0.0, 2.0, 1.0, 0.0]),
    "mat3_transpose_composition": (3, "transpose_composition", "value", None, [0.0, 0.0, 2.0, -1.0, 0.0, 0.0, 0.0, 1.0, 0.0]),
    "frame2_identity_point": (2, "identity_point", "value", None, [1.0, -2.0]),
    "frame2_signed_zero_vector": (2, "signed_zero_vector", "value", None, [0.0, 0.0]),
    "frame2_invalid_basis": (2, "frame_construction", "error", "invalid_frame", None),
    "vector2_zero_normalize": (2, "normalize", "error", "zero_length", None),
    "frame2_scale_out_of_range": (2, "frame_construction", "error", "scale_out_of_range", None),
    "vector2_nonfinite_construction": (2, "vector_construction", "error", "non_finite_input", None),
    "mat2_vector_overflow": (2, "matrix_vector_application", "error", "non_finite_result", None),
}


def fail(message: str) -> None:
    raise EvidenceError(message)


def require_keys(value: Any, expected: set[str], context: str) -> dict[str, Any]:
    if not isinstance(value, dict) or set(value) != expected:
        fail(f"{context} keys differ")
    return value


def parse_hex(value: Any, context: str) -> float:
    if not isinstance(value, str):
        fail(f"{context} is not hexadecimal text")
    try:
        result = float.fromhex(value)
    except ValueError as error:
        raise EvidenceError(f"{context} is not hexadecimal text") from error
    if not math.isfinite(result):
        fail(f"{context} is non-finite")
    return result


def equivalent(left: float, right: float) -> bool:
    return (left == 0.0 and right == 0.0) or left == right


def canonical_hex(value: float) -> str:
    return "0x0.0p+0" if value == 0.0 else value.hex()


def validate_profile(path: pathlib.Path) -> dict[str, Any]:
    profile = read_json(path)
    require_keys(profile, {
        "schema_version", "kind", "repetitions_per_cell", "cells", "gates",
        "semantic_ctest_allowlist", "cases", "certificate_negative_cases",
        "authority_paths", "volatile_fields", "limitations",
    }, "profile")
    if profile["schema_version"] != 1 or profile["kind"] != "geometry-primitives-cumulative-regression-profile":
        fail("profile identity differs")
    if profile["repetitions_per_cell"] != 3 or profile["cells"] != CELLS or profile["gates"] != GATES:
        fail("profile matrix or gates differ")
    if profile["semantic_ctest_allowlist"] != ALLOWLIST or profile["cases"] != list(EXPECTED):
        fail("profile semantic coverage differs")
    if profile["certificate_negative_cases"] != NEGATIVES or profile["authority_paths"] != AUTHORITIES:
        fail("profile authority or negative coverage differs")
    if not isinstance(profile["volatile_fields"], list) or profile["limitations"] != LIMITATIONS:
        fail("profile list field differs")
    return profile


def validate_certificate(profile: dict[str, Any], path: pathlib.Path) -> dict[str, Any]:
    certificate = read_json(path)
    require_keys(certificate, {"schema_version", "kind", "environment", "type_separation", "non_claims", "cases"}, "certificate")
    if certificate["schema_version"] != 1 or certificate["kind"] != "geometry-primitives-cumulative-certificate":
        fail("certificate identity differs")
    if certificate["environment"] != {"double_radix": 2, "double_digits": 53, "iec559": True}:
        fail("certificate floating environment differs")
    expected_separation = {
        "frame2_accepts_point2": True, "frame2_rejects_point3": True,
        "frame3_accepts_point3": True, "frame3_rejects_point2": True,
        "frame2_accepts_vector2": True, "frame2_rejects_vector3": True,
        "frame3_accepts_vector3": True, "frame3_rejects_vector2": True,
    }
    if certificate["type_separation"] != expected_separation or certificate["non_claims"] != NON_CLAIMS:
        fail("certificate type boundary or non-claim differs")
    if not isinstance(certificate["cases"], list):
        fail("certificate cases differ")
    projection: list[dict[str, Any]] = []
    seen: set[str] = set()
    for case in certificate["cases"]:
        require_keys(case, {"id", "dimension", "operation", "observed"}, "certificate case")
        identifier = case["id"]
        if not isinstance(identifier, str) or identifier in seen or identifier not in EXPECTED:
            fail("certificate case identity differs")
        seen.add(identifier)
        dimension, operation, outcome, error, expected_values = EXPECTED[identifier]
        if case["dimension"] != dimension or case["operation"] != operation:
            fail(f"{identifier} metadata differs")
        observed = require_keys(case["observed"], {"outcome", "error", "values"}, f"{identifier} observed")
        if observed["outcome"] != outcome or observed["error"] != error:
            fail(f"{identifier} outcome differs from independent oracle")
        if outcome == "error":
            if observed["values"] is not None:
                fail(f"{identifier} error carries values")
            projection.append({"id": identifier, "outcome": outcome, "error": error, "values": None})
            continue
        if not isinstance(observed["values"], list) or expected_values is None or len(observed["values"]) != len(expected_values):
            fail(f"{identifier} value shape differs")
        parsed = [parse_hex(value, f"{identifier} value") for value in observed["values"]]
        if not all(equivalent(actual, expected) for actual, expected in zip(parsed, expected_values)):
            fail(f"{identifier} values differ from independent oracle")
        projection.append({"id": identifier, "outcome": outcome, "error": None, "values": [canonical_hex(value) for value in parsed]})
    if seen != set(profile["cases"]):
        fail("certificate case set differs")
    return {"schema_version": 1, "kind": "geometry-primitives-cumulative-semantic-projection", "cases": projection}


def load_index(profile: dict[str, Any], path: pathlib.Path) -> list[dict[str, Any]]:
    index = read_json(path)
    require_keys(index, {"schema_version", "kind", "entries"}, "certificate index")
    if index["schema_version"] != 1 or index["kind"] != "geometry-primitives-cumulative-certificate-index" or not isinstance(index["entries"], list):
        fail("certificate index identity differs")
    expected_slots = [(cell["id"], repetition) for cell in profile["cells"] for repetition in range(1, profile["repetitions_per_cell"] + 1)]
    slots: list[tuple[str, int]] = []
    for entry in index["entries"]:
        require_keys(entry, {"cell", "repetition", "path", "sha256"}, "certificate index entry")
        if not isinstance(entry["cell"], str) or not isinstance(entry["repetition"], int) or not isinstance(entry["path"], str) or not isinstance(entry["sha256"], str):
            fail("certificate index entry type differs")
        slots.append((entry["cell"], entry["repetition"]))
    if slots != expected_slots:
        fail("certificate index coverage differs")
    return index["entries"]


def compare(profile: dict[str, Any], index_path: pathlib.Path) -> dict[str, Any]:
    entries = load_index(profile, index_path)
    projections: list[dict[str, Any]] = []
    for entry in entries:
        path = (index_path.parent / entry["path"]).resolve()
        if not path.is_file() or sha256_file(path) != entry["sha256"]:
            fail("certificate index hash differs")
        projections.append(validate_certificate(profile, path))
    if not projections or any(projection != projections[0] for projection in projections[1:]):
        fail("cross-cell semantic projection differs")
    return {
        "schema_version": 1,
        "kind": "geometry-primitives-cumulative-comparison",
        "status": "EVIDENCE_COLLECTED_PENDING_AUDIT",
        "gates": {gate: "NOT_EXECUTED" for gate in profile["gates"]},
        "certificate_count": len(projections),
        "semantic_projection": projections[0],
    }


def negative_outcomes(profile: dict[str, Any], output: pathlib.Path) -> None:
    write_json(output, {
        "schema_version": 1,
        "kind": "geometry-primitives-cumulative-negative-outcomes",
        "outcomes": [{"id": identifier, "result": "REJECTED"} for identifier in profile["certificate_negative_cases"]],
    })


def validate_negative_outcomes(profile: dict[str, Any], path: pathlib.Path) -> dict[str, Any]:
    value = read_json(path)
    require_keys(value, {"schema_version", "kind", "outcomes"}, "negative outcomes")
    expected = [{"id": identifier, "result": "REJECTED"} for identifier in profile["certificate_negative_cases"]]
    if value["schema_version"] != 1 or value["kind"] != "geometry-primitives-cumulative-negative-outcomes" or value["outcomes"] != expected:
        fail("negative outcomes differ")
    return value


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--profile", required=True)
    commands = parser.add_subparsers(dest="command", required=True)
    commands.add_parser("validate-profile")
    commands.add_parser("validate-certificate").add_argument("--certificate", required=True)
    compare_parser = commands.add_parser("compare")
    compare_parser.add_argument("--index", required=True)
    compare_parser.add_argument("--output", required=True)
    collect_parser = commands.add_parser("collect")
    collect_parser.add_argument("--index", required=True)
    collect_parser.add_argument("--output", required=True)
    negative_parser = commands.add_parser("negative-outcomes")
    negative_parser.add_argument("--output", required=True)
    validate_negative_parser = commands.add_parser("validate-negative-outcomes")
    validate_negative_parser.add_argument("--outcomes", required=True)
    arguments = parser.parse_args()
    try:
        profile = validate_profile(pathlib.Path(arguments.profile))
        if arguments.command == "validate-profile":
            return 0
        if arguments.command == "validate-certificate":
            validate_certificate(profile, pathlib.Path(arguments.certificate)); return 0
        if arguments.command in {"compare", "collect"}:
            result = compare(profile, pathlib.Path(arguments.index))
            write_json(pathlib.Path(arguments.output), result)
            return 0
        if arguments.command == "negative-outcomes":
            negative_outcomes(profile, pathlib.Path(arguments.output)); return 0
        validate_negative_outcomes(profile, pathlib.Path(arguments.outcomes)); return 0
    except (EvidenceError, OSError, ValueError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
