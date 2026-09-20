#!/usr/bin/env python3
"""Report-only validation and comparison for Topological Model TMR0-TMR7 evidence."""

from __future__ import annotations

import argparse
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
GATES = ["TMR0", "TMR1", "TMR2", "TMR3", "TMR4", "TMR5", "TMR6", "TMR7"]
ALLOWLIST = [
    "apmesh_core.bootstrap_smoke",
    "apmesh_core.numeric_contract",
    "apmesh_core.geometry_primitives",
    "apmesh_core.minimal_small_linear_algebra",
    "apmesh_core.math_header_isolation",
    "apmesh_core.cartesian_frames",
    "apmesh_core.topological_model",
]
AUTHORITIES = [
    "docs/decisions/FOUNDATION_END_TO_END_REGRESSION.md",
    "docs/decisions/GEOMETRY_PRIMITIVES_CUMULATIVE_REGRESSION_PROTOCOL.md",
    "docs/decisions/TOPOLOGICAL_MODEL_ENTRY_DECISION.md",
    "docs/decisions/TOPOLOGICAL_MODEL_CUMULATIVE_REGRESSION_PROTOCOL.md",
    "docs/decisions/CLOUD_QUALIFICATION_ENVIRONMENT_DECISION.md",
    "docs/decisions/TOPOLOGICAL_MODEL_CLOUD_QUALIFICATION_ENVIRONMENT_SUPPLEMENT.md",
    "docs/audits/2026-09-20-cloud-qualification-environment-admission.md",
]
LIMITATIONS = [
    "Formal cloud TMR preparation and execution are admissible only in the exact GitHub-hosted Ubuntu 24.04 x86_64 envelope bound by the accepted cloud supplement; any declared environment drift blocks the workflow",
    "The workflow remains report-only until a separately authorized formal manifest is prepared and explicitly executed",
    "Historical WSL qualification evidence remains WSL-scoped and is not reclassified as cloud evidence or treated as cloud-equivalent",
    "No native Windows, coordinate welding, adjacency, pairing, manifold, curve, surface, meshing, parallel, or performance qualification",
]
NON_CLAIMS = [
    "coordinate_welding",
    "adjacency",
    "pairing",
    "boundary_classification",
    "manifold_classification",
    "geometry",
    "curves",
    "surfaces",
    "meshing",
]
NEGATIVES = [
    "duplicate_case",
    "forged_snapshot",
    "forged_incidence",
    "forged_summary",
    "undeclared_semantic_claim",
]

EMPTY_SNAPSHOT = "apmesh-topology-v1\nvertices 0\nedges 0\nfaces 0\n"
FORWARD_SNAPSHOT = (
    "apmesh-topology-v1\n"
    "vertices 1\n"
    "vertex 1\n"
    "edges 1\n"
    "edge 1 1 1\n"
    "faces 1\n"
    "face 1 1\n"
    "loop 1 0 1\n"
    "use 1 0 0 1 forward\n"
)
REVERSE_SNAPSHOT = FORWARD_SNAPSHOT.replace("forward\n", "reverse\n")
INSERTION_A = (
    "apmesh-topology-v1\n"
    "vertices 2\n"
    "vertex 1\n"
    "vertex 2\n"
    "edges 2\n"
    "edge 1 1 2\n"
    "edge 2 2 1\n"
    "faces 0\n"
)
INSERTION_B = (
    "apmesh-topology-v1\n"
    "vertices 2\n"
    "vertex 1\n"
    "vertex 2\n"
    "edges 2\n"
    "edge 1 2 1\n"
    "edge 2 1 2\n"
    "faces 0\n"
)

EXPECTED: dict[str, dict[str, Any]] = {
    "empty_model": {
        "summary": {
            "vertex_count": 0, "edge_count": 0, "face_count": 0,
            "boundary_loop_count": 0, "edge_use_count": 0,
            "unused_edge_count": 0, "single_use_edge_count": 0,
            "two_use_opposed_edge_count": 0, "two_use_cooriented_edge_count": 0,
            "multi_use_edge_count": 0,
        },
        "snapshot": EMPTY_SNAPSHOT,
    },
    "identity_and_edge_order": {
        "vertex_ids": [1, 2],
        "edge_ids": [1, 2, 3, 4],
        "endpoints": [[1, 2], [1, 2], [2, 1], [1, 1]],
    },
    "transactional_rejection": {
        "invalid_handle": "invalid_vertex_handle",
        "empty_face": "empty_face_boundary",
        "empty_loop": "empty_boundary_loop",
        "open_loop": "open_boundary_loop",
        "first_valid_edge_id": 1,
        "second_valid_edge_id": 2,
        "first_valid_face_id": 1,
    },
    "invalid_lookup_and_orientation": {
        "invalid_edge_lookup": "invalid_edge_id",
        "invalid_face_lookup": "invalid_face_id",
        "invalid_incidence_lookup": "invalid_edge_id",
        "invalid_reverse": "invalid_orientation",
        "invalid_resolve": "invalid_orientation",
    },
    "boundary_valence": {
        "requested": [1, 2, 3, 5],
        "retained": [1, 2, 3, 5],
        "all_finalized": True,
    },
    "multiple_loops_repeated_owner": {
        "loop_count": 2,
        "incidences": [[1, 0, 0, "forward"], [1, 1, 0, "forward"]],
        "signature": {
            "occurrence_count": 2, "distinct_face_count": 1,
            "distinct_boundary_count": 2, "forward_count": 2, "reverse_count": 0,
            "has_repeated_face": True, "has_repeated_boundary": False,
            "classification": "two_use_cooriented",
        },
    },
    "structural_classes": {
        "classes": ["unused", "single_use", "two_use_opposed", "two_use_cooriented", "multi_use"],
        "summary": {
            "vertex_count": 1, "edge_count": 5, "face_count": 7,
            "boundary_loop_count": 7, "edge_use_count": 8,
            "unused_edge_count": 1, "single_use_edge_count": 1,
            "two_use_opposed_edge_count": 1, "two_use_cooriented_edge_count": 1,
            "multi_use_edge_count": 1,
        },
    },
    "three_face_multi_use": {
        "incidences": [[1, 0, 0, "forward"], [2, 0, 0, "forward"], [3, 0, 0, "forward"]],
        "signature": {
            "occurrence_count": 3, "distinct_face_count": 3,
            "distinct_boundary_count": 3, "forward_count": 3, "reverse_count": 0,
            "has_repeated_face": False, "has_repeated_boundary": False,
            "classification": "multi_use",
        },
    },
    "incidence_bijection": {
        "forward_occurrences": 3,
        "reverse_records": 3,
        "exact_bijection": True,
        "incidences": [[1, 0, 0, "forward"], [1, 0, 1, "forward"], [1, 0, 2, "forward"]],
    },
    "canonical_snapshot": {
        "snapshot": FORWARD_SNAPSHOT,
        "lf_only": True,
        "final_lf": True,
    },
    "deterministic_equal_construction": {
        "snapshot_equal": True,
        "summary_equal": True,
        "snapshot": FORWARD_SNAPSHOT,
    },
    "controlled_orientation_difference": {
        "different": True,
        "forward": FORWARD_SNAPSHOT,
        "reverse": REVERSE_SNAPSHOT,
    },
    "controlled_insertion_difference": {
        "different": True,
        "first": INSERTION_A,
        "second": INSERTION_B,
    },
}


def fail(message: str) -> None:
    raise EvidenceError(message)


def require_keys(value: Any, expected: set[str], context: str) -> dict[str, Any]:
    if not isinstance(value, dict) or set(value) != expected:
        fail(f"{context} keys differ")
    return value


def validate_profile(path: pathlib.Path) -> dict[str, Any]:
    profile = read_json(path)
    require_keys(profile, {
        "schema_version", "kind", "repetitions_per_cell", "cells", "gates",
        "semantic_ctest_allowlist", "cases", "certificate_negative_cases",
        "authority_paths", "volatile_fields", "limitations",
    }, "profile")
    if profile["schema_version"] != 1 or profile["kind"] != "topological-model-cumulative-regression-profile":
        fail("profile identity differs")
    if profile["repetitions_per_cell"] != 2 or profile["cells"] != CELLS or profile["gates"] != GATES:
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
    require_keys(certificate, {"schema_version", "kind", "type_contract", "non_claims", "cases"}, "certificate")
    if certificate["schema_version"] != 1 or certificate["kind"] != "topological-model-cumulative-certificate":
        fail("certificate identity differs")
    if certificate["type_contract"] != {
        "vertex_edge_face_distinct": True,
        "raw_id_construction_rejected": True,
        "model_has_no_public_add_vertex": True,
    }:
        fail("certificate type contract differs")
    if certificate["non_claims"] != NON_CLAIMS:
        fail("certificate non-claims differ")
    if not isinstance(certificate["cases"], list):
        fail("certificate cases differ")

    seen: set[str] = set()
    projection: list[dict[str, Any]] = []
    for case in certificate["cases"]:
        require_keys(case, {"id", "observed"}, "certificate case")
        identifier = case["id"]
        if not isinstance(identifier, str) or identifier in seen or identifier not in EXPECTED:
            fail("certificate case identity differs")
        seen.add(identifier)
        if case["observed"] != EXPECTED[identifier]:
            fail(f"{identifier} differs from the independent oracle")
        projection.append({"id": identifier, "observed": case["observed"]})

    if seen != set(profile["cases"]):
        fail("certificate case set differs")
    return {
        "schema_version": 1,
        "kind": "topological-model-cumulative-semantic-projection",
        "type_contract": certificate["type_contract"],
        "non_claims": certificate["non_claims"],
        "cases": projection,
    }


def load_index(profile: dict[str, Any], path: pathlib.Path) -> list[dict[str, Any]]:
    index = read_json(path)
    require_keys(index, {"schema_version", "kind", "entries"}, "certificate index")
    if index["schema_version"] != 1 or index["kind"] != "topological-model-cumulative-certificate-index":
        fail("certificate index identity differs")
    if not isinstance(index["entries"], list):
        fail("certificate index entries differ")
    expected_slots = [
        (cell["id"], repetition)
        for cell in profile["cells"]
        for repetition in range(1, profile["repetitions_per_cell"] + 1)
    ]
    slots: list[tuple[str, int]] = []
    for entry in index["entries"]:
        require_keys(entry, {"cell", "repetition", "path", "sha256"}, "certificate index entry")
        if not isinstance(entry["cell"], str) or not isinstance(entry["repetition"], int):
            fail("certificate index entry type differs")
        if not isinstance(entry["path"], str) or not isinstance(entry["sha256"], str):
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
    if not projections or any(item != projections[0] for item in projections[1:]):
        fail("cross-cell semantic projection differs")
    return {
        "schema_version": 1,
        "kind": "topological-model-cumulative-comparison",
        "status": "EVIDENCE_COLLECTED_PENDING_AUDIT",
        "gates": {gate: "NOT_EXECUTED" for gate in profile["gates"]},
        "certificate_count": len(projections),
        "semantic_projection": projections[0],
    }


def negative_outcomes(profile: dict[str, Any], output: pathlib.Path) -> None:
    write_json(output, {
        "schema_version": 1,
        "kind": "topological-model-cumulative-negative-outcomes",
        "outcomes": [
            {"id": identifier, "result": "REJECTED"}
            for identifier in profile["certificate_negative_cases"]
        ],
    })


def validate_negative_outcomes(profile: dict[str, Any], path: pathlib.Path) -> dict[str, Any]:
    value = read_json(path)
    require_keys(value, {"schema_version", "kind", "outcomes"}, "negative outcomes")
    expected = [
        {"id": identifier, "result": "REJECTED"}
        for identifier in profile["certificate_negative_cases"]
    ]
    if value["schema_version"] != 1 or value["kind"] != "topological-model-cumulative-negative-outcomes":
        fail("negative outcomes identity differs")
    if value["outcomes"] != expected:
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
            validate_certificate(profile, pathlib.Path(arguments.certificate))
            return 0
        if arguments.command in {"compare", "collect"}:
            result = compare(profile, pathlib.Path(arguments.index))
            write_json(pathlib.Path(arguments.output), result)
            return 0
        if arguments.command == "negative-outcomes":
            negative_outcomes(profile, pathlib.Path(arguments.output))
            return 0
        validate_negative_outcomes(profile, pathlib.Path(arguments.outcomes))
        return 0
    except (EvidenceError, OSError, ValueError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
