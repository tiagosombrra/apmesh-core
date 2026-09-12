#!/usr/bin/env python3
"""Prepare and verify report-only LA0-LA7 qualification evidence; never executes cells."""

from __future__ import annotations

import argparse
import pathlib
import subprocess
import sys
from typing import Any

TOOL_ROOT = pathlib.Path(__file__).resolve().parent
if str(TOOL_ROOT) not in sys.path:
    sys.path.insert(0, str(TOOL_ROOT))

from experiment_runtime import RuntimeErrorEvidence, input_identity, read_json, sha256_file, utc_now, write_json, write_state
from minimal_small_linear_algebra_evidence import EvidenceError, compare_certificates, validate_certificate, validate_profile


def fail(message: str) -> RuntimeErrorEvidence:
    return RuntimeErrorEvidence(message)


def canonical(source: pathlib.Path, relative: str, supplied: str, label: str) -> pathlib.Path:
    expected = (source / relative).resolve()
    actual = pathlib.Path(supplied).resolve()
    if expected != actual:
        raise fail(f"{label} must resolve to the candidate path")
    return expected


def inputs(arguments: argparse.Namespace, source: pathlib.Path) -> dict[str, pathlib.Path]:
    paths = {
        "profile": canonical(source, "experiments/profiles/minimal_small_linear_algebra.json", arguments.profile, "profile"),
        "protocol": canonical(source, "docs/decisions/GEOMETRY_MINIMAL_SMALL_LINEAR_ALGEBRA_QUALIFICATION_PROTOCOL.md", arguments.protocol, "protocol"),
        "authority": source / "docs/contracts/APMESH_CORE_MINIMAL_SMALL_LINEAR_ALGEBRA_CONTRACT.md",
        "exporter": source / "experiments/minimal_small_linear_algebra_export.cpp",
        "validator": source / "tools/minimal_small_linear_algebra_evidence.py",
        "runner": source / "tools/run_minimal_small_linear_algebra_qualification.py",
        "runtime": source / "tools/experiment_runtime.py",
        "cmake": source / "CMakeLists.txt",
        "math_header": source / "include/apmesh/math/linear_algebra.hpp",
        "math_source": source / "src/math/linear_algebra.cpp",
        "geometry_header": source / "include/apmesh/core/geometry.hpp",
        "geometry_source": source / "src/core/geometry.cpp",
        "focused_contract": source / "tests/minimal_small_linear_algebra.cpp",
        "header_contract": source / "tests/math_header_isolation.cpp",
        "state": source / "docs/APMESH_CORE_STATE.md",
        "roadmap": source / "docs/APMESH_CORE_ROADMAP.md",
    }
    for path in paths.values():
        if not path.is_file():
            raise fail(f"required input is absent: {path}")
    return paths


def clean_published_candidate(source: pathlib.Path) -> dict[str, str]:
    status = subprocess.run(["git", "status", "--porcelain"], cwd=source, capture_output=True, text=True, check=False)
    head = subprocess.run(["git", "rev-parse", "HEAD"], cwd=source, capture_output=True, text=True, check=False)
    upstream = subprocess.run(["git", "rev-parse", "@{u}"], cwd=source, capture_output=True, text=True, check=False)
    if status.returncode != 0 or status.stdout.strip() or head.returncode != 0 or upstream.returncode != 0:
        raise fail("candidate must be clean with an upstream")
    if head.stdout.strip() != upstream.stdout.strip():
        raise fail("candidate upstream differs from HEAD")
    return {"commit": head.stdout.strip(), "upstream_commit": upstream.stdout.strip()}


def plan(profile: dict[str, Any]) -> list[dict[str, Any]]:
    return [{"cell": cell["id"], "compiler": cell["compiler"], "library": cell["library"], "build_type": cell["build_type"],
             "repetitions": profile["repetitions_per_cell"], "execution": "NOT_REQUESTED"} for cell in profile["cells"]]


def prepare(arguments: argparse.Namespace) -> None:
    source = pathlib.Path(arguments.source_root).resolve()
    output = pathlib.Path(arguments.output_root).resolve()
    if output.exists():
        raise fail("output root must not exist")
    paths = inputs(arguments, source)
    profile = validate_profile(paths["profile"])
    candidate = clean_published_candidate(source)
    output.mkdir(parents=True)
    manifest = {
        "schema_version": 1, "kind": "minimal-small-linear-algebra-prepared-manifest", "state": "PREPARED",
        "execution_requested": False, "candidate": candidate, "inputs": input_identity(paths),
        "plan": plan(profile), "gates": {gate: "NOT_EXECUTED" for gate in profile["gates"]},
        "prerequisite_tests": profile["exact_prerequisite_tests"], "limitations": profile["limitations"], "prepared_utc": utc_now(),
    }
    write_json(output / "prepared-manifest.json", manifest)
    write_json(output / "plan.json", {"schema_version": 1, "kind": "minimal-small-linear-algebra-launch-plan", "cells": manifest["plan"]})
    write_state(output, "PREPARED", {"candidate_commit": candidate["commit"], "execution_requested": False,
                                      "prepared_manifest_sha256": sha256_file(output / "prepared-manifest.json")})


def collect(arguments: argparse.Namespace) -> None:
    profile = validate_profile(pathlib.Path(arguments.profile))
    paths = [pathlib.Path(path) for path in arguments.certificate]
    result = compare_certificates(profile, paths)
    result["gates"] = {gate: "EVIDENCE_COLLECTED_PENDING_AUDIT" for gate in profile["gates"]}
    write_json(pathlib.Path(arguments.output), result)


def verify_retention(arguments: argparse.Namespace) -> None:
    root = pathlib.Path(arguments.output_root).resolve()
    retention = read_json(root / "retention-manifest.json")
    if not isinstance(retention, dict) or set(retention) != {"schema_version", "kind", "candidate_commit", "prepared_manifest_sha256", "files"}:
        raise fail("retention manifest schema differs")
    if retention["schema_version"] != 1 or retention["kind"] != "minimal-small-linear-algebra-retention" or not isinstance(retention["files"], list):
        raise fail("retention manifest identity differs")
    prepared = root / "prepared-manifest.json"
    if not prepared.is_file() or retention["prepared_manifest_sha256"] != sha256_file(prepared):
        raise fail("retention prepared binding differs")
    expected = {"prepared-manifest.json", "retention-manifest.json"}
    for entry in retention["files"]:
        if not isinstance(entry, dict) or set(entry) != {"path", "sha256", "size"}:
            raise fail("retention entry schema differs")
        path = (root / entry["path"]).resolve()
        if root not in path.parents or not path.is_file() or path.stat().st_size != entry["size"] or sha256_file(path) != entry["sha256"]:
            raise fail("retention entry differs")
        expected.add(entry["path"])
    actual = {path.relative_to(root).as_posix() for path in root.rglob("*") if path.is_file()}
    if actual != expected:
        raise fail("retention file set differs")


def self_check(arguments: argparse.Namespace) -> None:
    source = pathlib.Path(arguments.source_root).resolve()
    paths = inputs(arguments, source)
    validate_profile(paths["profile"])


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source-root", required=True)
    parser.add_argument("--profile", required=True)
    parser.add_argument("--protocol", required=True)
    subparsers = parser.add_subparsers(dest="command", required=True)
    prepare_parser = subparsers.add_parser("prepare")
    prepare_parser.add_argument("--output-root", required=True)
    collect_parser = subparsers.add_parser("collect")
    collect_parser.add_argument("--certificate", action="append", required=True)
    collect_parser.add_argument("--output", required=True)
    retention_parser = subparsers.add_parser("verify-retention")
    retention_parser.add_argument("--output-root", required=True)
    subparsers.add_parser("self-check")
    arguments = parser.parse_args()
    try:
        if arguments.command == "prepare":
            prepare(arguments)
        elif arguments.command == "collect":
            collect(arguments)
        elif arguments.command == "verify-retention":
            verify_retention(arguments)
        else:
            self_check(arguments)
        return 0
    except (EvidenceError, RuntimeErrorEvidence) as error:
        print(f"LA qualification error: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
