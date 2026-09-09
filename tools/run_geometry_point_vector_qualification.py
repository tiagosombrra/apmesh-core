#!/usr/bin/env python3
"""Prepare or explicitly execute the pre-registered Point/Vector protocol."""

from __future__ import annotations

import argparse
import os
import pathlib
import platform
import shutil
import subprocess
import sys
import tempfile
import time
from typing import Any

from experiment_runtime import (
    RuntimeErrorEvidence, clean_candidate, input_identity, read_json, relative_path,
    run_command, sha256_file, tool_version, utc_now, verify_input_identity, write_json, write_state,
)
from geometry_point_vector_evidence import EvidenceError, validate_profile


BUILD_TIMEOUT_SECONDS = 300
PROCESS_TIMEOUT_SECONDS = 45
OVERALL_TIMEOUT_SECONDS = 1_800
CONFIGURATIONS = {
    "gcc-debug": ("g++-13", False),
    "gcc-release": ("g++-13", False),
    "clang-debug": ("clang++-18", True),
    "clang-release": ("clang++-18", True),
}
EXECUTABLES = (
    "apmesh_core_geometry_point_vector_export",
    "apmesh_core.geometry_primitives",
    "apmesh_core_bootstrap_smoke",
    "apmesh_core_numeric_contract",
)


def fail(message: str) -> RuntimeErrorEvidence:
    return RuntimeErrorEvidence(message)


def canonical_path(source_root: pathlib.Path, relative: str, supplied: str, name: str) -> pathlib.Path:
    expected = (source_root / relative).resolve()
    actual = pathlib.Path(supplied).resolve()
    if actual != expected:
        raise fail(f"{name} must resolve to the candidate path: {expected}")
    return expected


def published_candidate(source_root: pathlib.Path) -> dict[str, Any]:
    candidate = clean_candidate(source_root)
    upstream = subprocess.run(["git", "rev-parse", "@{u}"], cwd=source_root, capture_output=True, text=True, check=False)
    if upstream.returncode != 0 or upstream.stdout.strip() != candidate["commit"]:
        raise fail("candidate upstream is absent or differs from HEAD")
    candidate["upstream_commit"] = upstream.stdout.strip()
    return candidate


def input_paths(arguments: argparse.Namespace, source_root: pathlib.Path) -> dict[str, pathlib.Path]:
    launcher = (source_root / "tools/run_geometry_point_vector_qualification.py").resolve()
    if pathlib.Path(__file__).resolve() != launcher:
        raise fail(f"launcher must execute from the candidate path: {launcher}")
    paths = {
        "profile": canonical_path(source_root, "experiments/profiles/geometry_point_vector.json", arguments.profile, "profile"),
        "protocol": canonical_path(source_root, "docs/decisions/GEOMETRY_POINT_VECTOR_QUALIFICATION_PROTOCOL.md", arguments.protocol, "protocol"),
        "exporter": canonical_path(source_root, "experiments/geometry_point_vector_export.cpp", arguments.exporter, "exporter"),
        "validator": canonical_path(source_root, "tools/geometry_point_vector_evidence.py", arguments.validator, "validator"),
        "launcher": launcher,
        "runtime": source_root / "tools/experiment_runtime.py",
        "cmake": source_root / "CMakeLists.txt",
        "geometry_header": source_root / "include/apmesh/core/geometry.hpp",
        "geometry_source": source_root / "src/core/geometry.cpp",
        "geometry_contract": source_root / "tests/geometry_primitives.cpp",
        "architecture_authority": source_root / "docs/contracts/APMESH_CORE_ARCHITECTURE_CONTRACT.md",
        "numeric_authority": source_root / "docs/contracts/APMESH_CORE_NUMERIC_CONTRACT.md",
        "reproducibility_authority": source_root / "docs/contracts/APMESH_CORE_REPRODUCIBLE_EXPERIMENT_CONTRACT.md",
        "foundation_authority": source_root / "docs/decisions/FOUNDATION_END_TO_END_REGRESSION.md",
    }
    for path in paths.values():
        if not path.is_file():
            raise fail(f"required input is absent: {path}")
    return paths


def environment_identity() -> dict[str, Any]:
    return {
        "python": sys.version.splitlines()[0],
        "platform": platform.platform(),
        "cmake": tool_version("cmake"),
        "ctest": tool_version("ctest"),
        "ninja": tool_version("ninja"),
        "gcc": tool_version("g++-13"),
        "clang": tool_version("clang++-18"),
        "ldd": tool_version("ldd"),
        "observations": {key: os.environ.get(key, "") for key in ("LANG", "LC_ALL", "TZ")},
    }


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
            "compile_command_validation": [
                sys.executable, str(source_root / "tools/geometry_point_vector_evidence.py"),
                "validate-compile-commands", "--compile-commands", f"{build_root}/compile_commands.json",
            ],
            "build": ["cmake", "--build", build_root],
            "focused_ctest": ["ctest", "--test-dir", build_root, "-L", profile["focused_ctest_label"], "--output-on-failure"],
            "foundation_ctest": ["ctest", "--test-dir", build_root, "-L", profile["foundation_ctest_label"], "--output-on-failure"],
            "repetitions": profile["repetitions_per_configuration"],
            "dependency_executables": list(EXECUTABLES),
        })
    return plan


def replace_output_root(argv: list[str], output_root: pathlib.Path) -> list[str]:
    return [part.replace("@OUTPUT_ROOT@", str(output_root)) for part in argv]


def require_success(record: dict[str, Any], context: str) -> None:
    if record["exit_code"] != 0 or record["timed_out"] or record["launch_error"] is not None:
        raise fail(f"{context} failed")


def expected_slots(profile: dict[str, Any]) -> list[dict[str, Any]]:
    return [{"cell": entry["name"], "repetition": repetition} for entry in profile["configurations"]
            for repetition in range(1, profile["repetitions_per_configuration"] + 1)]


def prepare(arguments: argparse.Namespace) -> tuple[pathlib.Path, dict[str, Any]]:
    source_root = pathlib.Path(arguments.source_root).resolve()
    output_root = pathlib.Path(arguments.output_root).resolve()
    if output_root.exists():
        raise fail(f"output root already exists: {output_root}")
    profile = validate_profile(canonical_path(source_root, "experiments/profiles/geometry_point_vector.json", arguments.profile, "profile"))
    candidate = published_candidate(source_root)
    inputs = input_paths(arguments, source_root)
    environment = environment_identity()
    output_root.mkdir(parents=True)
    manifest = {
        "schema_version": 2,
        "kind": "geometry-point-vector-prepared-manifest",
        "state": "PREPARED",
        "execution_requested": False,
        "candidate": candidate,
        "inputs": input_identity(inputs),
        "environment": environment,
        "working_directory": str(source_root),
        "output_root": str(output_root),
        "limits_seconds": {"build": BUILD_TIMEOUT_SECONDS, "process": PROCESS_TIMEOUT_SECONDS, "overall": OVERALL_TIMEOUT_SECONDS},
        "plan": command_plan(profile, source_root),
        "expected_certificate_slots": expected_slots(profile),
        "gates": {gate: "NOT_EXECUTED" for gate in profile["gates"]},
        "retained_limitations": profile["limitations"],
        "prepared_utc": utc_now(),
    }
    write_json(output_root / "prepared-manifest.json", manifest)
    write_json(output_root / "plan.json", {"schema_version": 2, "kind": "geometry-point-vector-launch-plan", "cells": manifest["plan"]})
    write_state(output_root, "PREPARED", {"candidate_commit": candidate["commit"], "execution_requested": False,
                                          "prepared_manifest_sha256": sha256_file(output_root / "prepared-manifest.json")})
    return output_root, manifest


def load_prepared(arguments: argparse.Namespace) -> tuple[pathlib.Path, dict[str, Any], dict[str, Any]]:
    source_root = pathlib.Path(arguments.source_root).resolve()
    output_root = pathlib.Path(arguments.output_root).resolve()
    manifest = read_json(output_root / "prepared-manifest.json")
    profile_path = canonical_path(source_root, "experiments/profiles/geometry_point_vector.json", arguments.profile, "profile")
    profile = validate_profile(profile_path)
    expected_keys = {
        "schema_version", "kind", "state", "execution_requested", "candidate", "inputs", "environment",
        "working_directory", "output_root", "limits_seconds", "plan", "expected_certificate_slots", "gates",
        "retained_limitations", "prepared_utc",
    }
    if set(manifest) != expected_keys or manifest["schema_version"] != 2 or manifest["kind"] != "geometry-point-vector-prepared-manifest":
        raise fail("prepared manifest schema differs")
    if manifest["state"] != "PREPARED" or manifest["execution_requested"] is not False:
        raise fail("manifest is not an executable PREPARED manifest")
    if manifest["candidate"] != published_candidate(source_root):
        raise fail("prepared manifest candidate differs")
    if manifest["environment"] != environment_identity() or manifest["working_directory"] != str(source_root) or manifest["output_root"] != str(output_root):
        raise fail("prepared execution environment differs")
    verify_input_identity(manifest["inputs"], input_paths(arguments, source_root))
    if manifest["plan"] != command_plan(profile, source_root) or manifest["expected_certificate_slots"] != expected_slots(profile):
        raise fail("prepared manifest plan differs")
    if manifest["gates"] != {gate: "NOT_EXECUTED" for gate in profile["gates"]} or manifest["retained_limitations"] != profile["limitations"]:
        raise fail("prepared manifest claims differ")
    return output_root, manifest, profile


def verify_detached_candidate(source_root: pathlib.Path, candidate: dict[str, Any]) -> dict[str, Any]:
    with tempfile.TemporaryDirectory(prefix="apmesh-core-pv-retention-") as temporary:
        detached = pathlib.Path(temporary) / "candidate"
        added = subprocess.run(["git", "worktree", "add", "--detach", str(detached), candidate["commit"]],
                               cwd=source_root, capture_output=True, text=True, check=False)
        if added.returncode != 0:
            raise fail("detached candidate worktree could not be created")
        try:
            observed = clean_candidate(detached)
            if observed["commit"] != candidate["commit"]:
                raise fail("detached candidate revision differs")
            expected_inventory = [{"path": row["path"], "sha256": row["sha256"]} for row in candidate["source_inventory"]]
            observed_inventory = [{"path": row["path"], "sha256": row["sha256"]} for row in observed["source_inventory"]]
            if observed_inventory != expected_inventory:
                raise fail("detached candidate source inventory differs")
            return {"result": "PASS", "candidate_commit": candidate["commit"], "source_inventory_count": len(observed_inventory)}
        finally:
            removed = subprocess.run(["git", "worktree", "remove", "--force", str(detached)],
                                    cwd=source_root, capture_output=True, text=True, check=False)
            if removed.returncode != 0:
                raise fail("detached candidate worktree could not be removed")


def seal_output(output_root: pathlib.Path, source_root: pathlib.Path, manifest: dict[str, Any]) -> dict[str, Any]:
    detached = verify_detached_candidate(source_root, manifest["candidate"])
    write_json(output_root / "detached-verification.json",
               {"schema_version": 1, "kind": "geometry-point-vector-detached-verification", **detached})
    files = sorted(path for path in output_root.rglob("*") if path.is_file() and path.name != "retention-manifest.json")
    retention = {
        "schema_version": 1,
        "kind": "geometry-point-vector-retention",
        "candidate_commit": manifest["candidate"]["commit"],
        "prepared_manifest_sha256": sha256_file(output_root / "prepared-manifest.json"),
        "files": [{"path": relative_path(output_root, path), "sha256": sha256_file(path), "size": path.stat().st_size} for path in files],
    }
    write_json(output_root / "retention-manifest.json", retention)
    verified = read_json(output_root / "retention-manifest.json")
    for row in verified["files"]:
        path = output_root / row["path"]
        if not path.is_file() or path.stat().st_size != row["size"] or sha256_file(path) != row["sha256"]:
            raise fail("retention seal differs")
    verify_retention(output_root, source_root)
    return {"retention_manifest": "retention-manifest.json", "retention_manifest_sha256": sha256_file(output_root / "retention-manifest.json"),
            "detached_verification": "detached-verification.json"}


def verify_retention(output_root: pathlib.Path, source_root: pathlib.Path) -> None:
    prepared = read_json(output_root / "prepared-manifest.json")
    terminal = read_json(output_root / "terminal-manifest.json")
    retention = read_json(output_root / "retention-manifest.json")
    detached = read_json(output_root / "detached-verification.json")
    if (prepared.get("kind") != "geometry-point-vector-prepared-manifest" or
            terminal.get("kind") != "geometry-point-vector-terminal-manifest" or
            terminal.get("candidate") != prepared.get("candidate") or
            terminal.get("prepared_manifest_sha256") != sha256_file(output_root / "prepared-manifest.json") or
            terminal.get("retention_manifest") != "retention-manifest.json"):
        raise fail("terminal evidence identity differs")
    if (retention.get("kind") != "geometry-point-vector-retention" or
            retention.get("candidate_commit") != prepared["candidate"]["commit"] or
            retention.get("prepared_manifest_sha256") != sha256_file(output_root / "prepared-manifest.json") or
            not isinstance(retention.get("files"), list)):
        raise fail("retention manifest identity differs")
    expected_paths = {"retention-manifest.json"}
    for row in retention["files"]:
        if not isinstance(row, dict) or set(row) != {"path", "sha256", "size"} or not isinstance(row["path"], str):
            raise fail("retention manifest entry differs")
        path = output_root / row["path"]
        if not path.is_file() or path.stat().st_size != row["size"] or sha256_file(path) != row["sha256"]:
            raise fail("retention manifest hash differs")
        expected_paths.add(row["path"])
    actual_paths = {relative_path(output_root, path) for path in output_root.rglob("*") if path.is_file()}
    if actual_paths != expected_paths:
        raise fail("retention manifest file set differs")
    observed_detached = verify_detached_candidate(source_root, prepared["candidate"])
    if detached != {"schema_version": 1, "kind": "geometry-point-vector-detached-verification", **observed_detached}:
        raise fail("detached retention verification differs")


def execute(arguments: argparse.Namespace, output_root: pathlib.Path, manifest: dict[str, Any], profile: dict[str, Any]) -> int:
    records: list[dict[str, Any]] = []
    started_monotonic = time.monotonic()
    started_utc = utc_now()

    def write_progress() -> None:
        write_json(output_root / "execution-progress.json", {"schema_version": 1, "kind": "geometry-point-vector-execution-progress",
                                                               "started_utc": started_utc, "records": records})

    def remaining_budget() -> None:
        if time.monotonic() - started_monotonic >= OVERALL_TIMEOUT_SECONDS:
            raise fail("overall qualification time limit exceeded")

    def record_command(cell: dict[str, Any], argv: list[str], stage: str, timeout: int) -> dict[str, Any]:
        remaining_budget()
        record = run_command(argv, output_root, output_root / "cells" / cell["cell"] / "logs", stage, timeout)
        cell["records"].append(record)
        write_progress()
        require_success(record, f"{cell['cell']} {stage}")
        return record

    terminal: dict[str, Any]
    write_state(output_root, "RUNNING", {"candidate_commit": manifest["candidate"]["commit"], "started_utc": started_utc})
    try:
        for plan_cell in manifest["plan"]:
            cell = {"cell": plan_cell["name"], "state": "RUNNING", "records": [], "certificates": [], "negative_fixtures": None, "dependencies": []}
            records.append(cell)
            write_progress()
            for stage in ("configure", "compile_command_validation", "build"):
                record_command(cell, replace_output_root(plan_cell[stage], output_root), stage, BUILD_TIMEOUT_SECONDS)
            for repetition in range(1, plan_cell["repetitions"] + 1):
                record_command(cell, replace_output_root(plan_cell["focused_ctest"], output_root), f"focused-ctest-{repetition}", PROCESS_TIMEOUT_SECONDS)
                certificate = output_root / "cells" / cell["cell"] / f"certificate-{repetition}.json"
                exporter = output_root / "cells" / cell["cell"] / "build" / "apmesh_core_geometry_point_vector_export"
                record_command(cell, [str(exporter), "certificate", str(certificate)], f"certificate-{repetition}", PROCESS_TIMEOUT_SECONDS)
                record_command(cell, [sys.executable, str(pathlib.Path(arguments.validator).resolve()), "validate-certificate",
                                      "--profile", str(pathlib.Path(arguments.profile).resolve()), "--certificate", str(certificate)],
                               f"certificate-validation-{repetition}", PROCESS_TIMEOUT_SECONDS)
                cell["certificates"].append({"cell": cell["cell"], "repetition": repetition,
                                             "path": relative_path(output_root, certificate), "sha256": sha256_file(certificate)})
                write_progress()
            negative = output_root / "cells" / cell["cell"] / "negative-fixtures.json"
            record_command(cell, [sys.executable, str(pathlib.Path(arguments.validator).resolve()), "negative-self-check",
                                  "--profile", str(pathlib.Path(arguments.profile).resolve()),
                                  "--certificate", str(output_root / "cells" / cell["cell"] / "certificate-1.json"),
                                  "--compile-commands", str(output_root / "cells" / cell["cell"] / "build" / "compile_commands.json"),
                                  "--output", str(negative)], "negative-fixtures", PROCESS_TIMEOUT_SECONDS)
            cell["negative_fixtures"] = {"path": relative_path(output_root, negative), "sha256": sha256_file(negative)}
            record_command(cell, replace_output_root(plan_cell["foundation_ctest"], output_root), "foundation-ctest", BUILD_TIMEOUT_SECONDS)
            for executable_name in plan_cell["dependency_executables"]:
                executable = output_root / "cells" / cell["cell"] / "build" / executable_name
                if not executable.is_file():
                    raise fail(f"{cell['cell']} dependency executable is absent: {executable_name}")
                record = record_command(cell, ["ldd", str(executable)], f"ldd-{executable_name}", PROCESS_TIMEOUT_SECONDS)
                cell["dependencies"].append({"executable": executable_name, "path": relative_path(output_root, executable),
                                             "sha256": sha256_file(executable), "ldd_record": record})
                write_progress()
            cell["state"] = "PASS"
            write_progress()
        certificate_entries = [certificate for cell in records for certificate in cell["certificates"]]
        index = output_root / "certificate-index.json"
        write_json(index, {"schema_version": 1, "kind": "geometry-point-vector-certificate-index", "entries": certificate_entries})
        comparison_cell = {"cell": "cross-cell-comparison", "state": "RUNNING", "records": [], "certificates": [], "negative_fixtures": None, "dependencies": []}
        records.append(comparison_cell)
        report = output_root / "report.md"
        record_command(comparison_cell, [sys.executable, str(pathlib.Path(arguments.validator).resolve()), "compare",
                                         "--profile", str(pathlib.Path(arguments.profile).resolve()), "--index", str(index), "--report", str(report)],
                       "compare", PROCESS_TIMEOUT_SECONDS)
        comparison_cell["state"] = "PASS"
        dependency_inventory = output_root / "runtime-dependencies.json"
        write_json(dependency_inventory, {"schema_version": 1, "kind": "geometry-point-vector-runtime-dependencies",
                                          "candidate_commit": manifest["candidate"]["commit"],
                                          "cells": [{"cell": cell["cell"], "dependencies": cell["dependencies"]} for cell in records if cell["cell"] != "cross-cell-comparison"]})
        terminal = {
            "schema_version": 2, "kind": "geometry-point-vector-terminal-manifest",
            "state": "EXECUTED_PENDING_AUDIT", "candidate": manifest["candidate"],
            "prepared_manifest_sha256": sha256_file(output_root / "prepared-manifest.json"),
            "execution": {"started_utc": started_utc, "ended_utc": utc_now(), "records": records,
                          "certificate_index": "certificate-index.json", "report": "report.md",
                          "runtime_dependencies": "runtime-dependencies.json"},
            "gates": {gate: "EVIDENCE_COLLECTED_PENDING_AUDIT" for gate in profile["gates"]},
            "retained_limitations": profile["limitations"],
            "retention_manifest": "retention-manifest.json",
        }
        write_json(output_root / "terminal-manifest.json", terminal)
        write_state(output_root, "EXECUTED_PENDING_AUDIT", {"candidate_commit": manifest["candidate"]["commit"],
                                                             "terminal_manifest": "terminal-manifest.json"})
        seal_output(output_root, pathlib.Path(arguments.source_root).resolve(), manifest)
        return 0
    except (EvidenceError, RuntimeErrorEvidence) as error:
        terminal = {
            "schema_version": 2, "kind": "geometry-point-vector-terminal-manifest", "state": "BLOCKED",
            "candidate": manifest["candidate"], "prepared_manifest_sha256": sha256_file(output_root / "prepared-manifest.json"),
            "execution": {"started_utc": started_utc, "ended_utc": utc_now(), "records": records, "reason": str(error)},
            "gates": {gate: "BLOCKED" for gate in profile["gates"]}, "retained_limitations": profile["limitations"],
            "retention_manifest": "retention-manifest.json",
        }
        write_json(output_root / "terminal-manifest.json", terminal)
        write_state(output_root, "BLOCKED", {"reason": str(error), "terminal_manifest": "terminal-manifest.json"})
        try:
            seal_output(output_root, pathlib.Path(arguments.source_root).resolve(), manifest)
        except RuntimeErrorEvidence as retention_error:
            terminal["retention_error"] = str(retention_error)
            write_json(output_root / "terminal-manifest.json", terminal)
        return 1


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    for name in ("source-root", "profile", "protocol", "exporter", "validator", "output-root"):
        parser.add_argument(f"--{name}", required=True)
    parser.add_argument("--execute", action="store_true")
    parser.add_argument("--verify-retention", action="store_true")
    return parser.parse_args()


def main() -> int:
    arguments = parse_arguments()
    try:
        if arguments.execute and arguments.verify_retention:
            raise fail("--execute and --verify-retention are mutually exclusive")
        if arguments.verify_retention:
            verify_retention(pathlib.Path(arguments.output_root).resolve(), pathlib.Path(arguments.source_root).resolve())
            return 0
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
