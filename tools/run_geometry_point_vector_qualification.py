#!/usr/bin/env python3
"""Prepare or explicitly execute the pre-registered Point/Vector protocol."""

from __future__ import annotations

import argparse
import pathlib
import shutil
import subprocess
import sys
from typing import Any

from experiment_runtime import (
    RuntimeErrorEvidence,
    clean_candidate,
    input_identity,
    read_json,
    run_command,
    tool_version,
    utc_now,
    verify_input_identity,
    write_json,
    write_state,
)
from geometry_point_vector_evidence import EvidenceError, compare, validate_certificate, validate_profile


BUILD_TIMEOUT_SECONDS = 300
PROCESS_TIMEOUT_SECONDS = 45
OVERALL_TIMEOUT_SECONDS = 1_800
CONFIGURATIONS = {
    "gcc-debug": ("g++-13", False),
    "gcc-release": ("g++-13", False),
    "clang-debug": ("clang++-18", True),
    "clang-release": ("clang++-18", True),
}


def fail(message: str) -> RuntimeErrorEvidence:
    return RuntimeErrorEvidence(message)


def published_candidate(source_root: pathlib.Path) -> dict[str, Any]:
    candidate = clean_candidate(source_root)
    upstream = subprocess.run(["git", "rev-parse", "@{u}"], cwd=source_root, capture_output=True, text=True, check=False)
    if upstream.returncode != 0 or upstream.stdout.strip() != candidate["commit"]:
        raise fail("candidate upstream is absent or differs from HEAD")
    candidate["upstream_commit"] = upstream.stdout.strip()
    return candidate


def input_paths(arguments: argparse.Namespace, source_root: pathlib.Path) -> dict[str, pathlib.Path]:
    paths = {
        "profile": pathlib.Path(arguments.profile).resolve(),
        "protocol": pathlib.Path(arguments.protocol).resolve(),
        "exporter": pathlib.Path(arguments.exporter).resolve(),
        "validator": pathlib.Path(arguments.validator).resolve(),
        "launcher": pathlib.Path(__file__).resolve(),
        "cmake": source_root / "CMakeLists.txt",
        "geometry_header": source_root / "include/apmesh/core/geometry.hpp",
        "geometry_source": source_root / "src/core/geometry.cpp",
        "geometry_contract": source_root / "tests/geometry_primitives.cpp",
    }
    for path in paths.values():
        if not path.is_file():
            raise fail(f"required input is absent: {path}")
    return paths


def command_plan(profile: dict[str, Any], source_root: pathlib.Path) -> list[dict[str, Any]]:
    plan: list[dict[str, Any]] = []
    for entry in profile["configurations"]:
        name = entry["name"]
        compiler, libcxx = CONFIGURATIONS[name]
        build_root = f"@OUTPUT_ROOT@/cells/{name}/build"
        plan.append({
            "name": name,
            "configure": [
                "cmake", "-S", str(source_root), "-B", build_root, "-G", "Ninja",
                f"-DCMAKE_CXX_COMPILER={compiler}", f"-DCMAKE_BUILD_TYPE={entry['build_type']}",
                f"-DAPMESH_USE_LIBCXX={'ON' if libcxx else 'OFF'}", "-DBUILD_TESTING=ON",
                "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
            ],
            "build": ["cmake", "--build", build_root, "--target", "apmesh_core_geometry_point_vector_export", "apmesh_core.geometry_primitives"],
            "focused_ctest": ["ctest", "--test-dir", build_root, "-L", profile["focused_ctest_label"], "--output-on-failure"],
            "foundation_ctest": ["ctest", "--test-dir", build_root, "-L", profile["foundation_ctest_label"], "--output-on-failure"],
            "repetitions": profile["repetitions_per_configuration"],
        })
    return plan


def replace_output_root(argv: list[str], output_root: pathlib.Path) -> list[str]:
    return [item.replace("@OUTPUT_ROOT@", str(output_root)) for item in argv]


def require_success(record: dict[str, Any], context: str) -> None:
    if record["exit_code"] != 0 or record["timed_out"] or record["launch_error"] is not None:
        raise fail(f"{context} failed")


def prepare(arguments: argparse.Namespace) -> tuple[pathlib.Path, dict[str, Any]]:
    source_root = pathlib.Path(arguments.source_root).resolve()
    output_root = pathlib.Path(arguments.output_root).resolve()
    if output_root.exists():
        raise fail(f"output root already exists: {output_root}")
    profile = validate_profile(pathlib.Path(arguments.profile))
    candidate = published_candidate(source_root)
    inputs = input_paths(arguments, source_root)
    output_root.mkdir(parents=True)
    manifest = {
        "schema_version": 1,
        "kind": "geometry-point-vector-qualification-manifest",
        "state": "PREPARED",
        "execution_requested": False,
        "candidate": candidate,
        "inputs": input_identity(inputs),
        "environment": {
            "python": sys.version.splitlines()[0], "cmake": tool_version("cmake"), "ninja": tool_version("ninja"),
            "gcc": tool_version("g++-13"), "clang": tool_version("clang++-18"),
        },
        "limits_seconds": {"build": BUILD_TIMEOUT_SECONDS, "process": PROCESS_TIMEOUT_SECONDS, "overall": OVERALL_TIMEOUT_SECONDS},
        "plan": command_plan(profile, source_root),
        "gates": {gate: "NOT_EXECUTED" for gate in profile["gates"]},
        "prepared_utc": utc_now(),
    }
    write_json(output_root / "manifest.json", manifest)
    write_json(output_root / "plan.json", {"schema_version": 1, "kind": "geometry-point-vector-launch-plan", "cells": manifest["plan"]})
    write_state(output_root, "PREPARED", {"candidate_commit": candidate["commit"], "execution_requested": False})
    return output_root, manifest


def load_prepared(arguments: argparse.Namespace) -> tuple[pathlib.Path, dict[str, Any], dict[str, Any]]:
    source_root = pathlib.Path(arguments.source_root).resolve()
    output_root = pathlib.Path(arguments.output_root).resolve()
    manifest = read_json(output_root / "manifest.json")
    profile = validate_profile(pathlib.Path(arguments.profile))
    expected_keys = {"schema_version", "kind", "state", "execution_requested", "candidate", "inputs", "environment", "limits_seconds", "plan", "gates", "prepared_utc"}
    if set(manifest) != expected_keys or manifest["schema_version"] != 1 or manifest["kind"] != "geometry-point-vector-qualification-manifest":
        raise fail("prepared manifest schema differs")
    if manifest["state"] != "PREPARED" or manifest["execution_requested"] is not False:
        raise fail("manifest is not an executable PREPARED manifest")
    if manifest["candidate"] != published_candidate(source_root):
        raise fail("prepared manifest candidate differs")
    verify_input_identity(manifest["inputs"], input_paths(arguments, source_root))
    if manifest["plan"] != command_plan(profile, source_root) or manifest["gates"] != {gate: "NOT_EXECUTED" for gate in profile["gates"]}:
        raise fail("prepared manifest plan differs")
    return output_root, manifest, profile


def execute(arguments: argparse.Namespace, output_root: pathlib.Path, manifest: dict[str, Any], profile: dict[str, Any]) -> int:
    records: list[dict[str, Any]] = []
    write_state(output_root, "RUNNING", {"candidate_commit": manifest["candidate"]["commit"]})
    try:
        for cell in manifest["plan"]:
            cell_root = output_root / "cells" / cell["name"]
            cell_root.mkdir(parents=True)
            cell_records: list[dict[str, Any]] = []
            for stage in ("configure", "build"):
                record = run_command(replace_output_root(cell[stage], output_root), output_root, cell_root / "logs", stage, BUILD_TIMEOUT_SECONDS)
                cell_records.append(record)
                require_success(record, f"{cell['name']} {stage}")
            for repetition in range(cell["repetitions"]):
                record = run_command(replace_output_root(cell["focused_ctest"], output_root), output_root, cell_root / "logs", f"focused-ctest-{repetition + 1}", PROCESS_TIMEOUT_SECONDS)
                cell_records.append(record)
                require_success(record, f"{cell['name']} focused CTest {repetition + 1}")
            exporter = cell_root / "build" / "apmesh_core_geometry_point_vector_export"
            certificates: list[str] = []
            for repetition in range(cell["repetitions"]):
                certificate = cell_root / f"certificate-{repetition + 1}.json"
                record = run_command([str(exporter), "certificate", str(certificate)], cell_root, cell_root / "logs", f"certificate-{repetition + 1}", PROCESS_TIMEOUT_SECONDS)
                cell_records.append(record)
                require_success(record, f"{cell['name']} certificate {repetition + 1}")
                validate_certificate(certificate)
                certificates.append(str(certificate.relative_to(output_root)))
            record = run_command(replace_output_root(cell["foundation_ctest"], output_root), output_root, cell_root / "logs", "foundation-ctest", BUILD_TIMEOUT_SECONDS)
            cell_records.append(record)
            require_success(record, f"{cell['name']} Foundation preservation CTest")
            records.append({"cell": cell["name"], "state": "PASS", "records": cell_records, "certificates": certificates})
        certificate_paths = [output_root / path for record in records for path in record["certificates"]]
        report = output_root / "report.md"
        compare(pathlib.Path(arguments.profile), certificate_paths, report)
        manifest["state"] = "EXECUTED_PENDING_AUDIT"
        manifest["execution"] = {"started_utc": utc_now(), "ended_utc": utc_now(), "records": records, "report": str(report.relative_to(output_root))}
        manifest["gates"] = {gate: "EVIDENCE_COLLECTED_PENDING_AUDIT" for gate in profile["gates"]}
        write_json(output_root / "manifest.json", manifest)
        write_state(output_root, "EXECUTED_PENDING_AUDIT", {"candidate_commit": manifest["candidate"]["commit"], "gate_count": len(profile["gates"])})
        return 0
    except (EvidenceError, RuntimeErrorEvidence) as error:
        manifest["state"] = "BLOCKED"
        manifest["execution"] = {"ended_utc": utc_now(), "records": records, "reason": str(error)}
        write_json(output_root / "manifest.json", manifest)
        write_state(output_root, "BLOCKED", {"reason": str(error)})
        return 1


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    for name in ("source-root", "profile", "protocol", "exporter", "validator", "output-root"):
        parser.add_argument(f"--{name}", required=True)
    parser.add_argument("--execute", action="store_true")
    return parser.parse_args()


def main() -> int:
    arguments = parse_arguments()
    try:
        if arguments.execute:
            output_root, manifest, profile = load_prepared(arguments)
            return execute(arguments, output_root, manifest, profile)
        prepare(arguments)
        return 0
    except (EvidenceError, RuntimeErrorEvidence) as error:
        print(error, file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
