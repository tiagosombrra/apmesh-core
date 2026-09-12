#!/usr/bin/env python3
"""Prepare or explicitly execute the revision-bound LA0-LA7 protocol."""

from __future__ import annotations

import argparse
import os
import pathlib
import platform
import shutil
import subprocess
import sys
import tempfile
from typing import Any

TOOL_ROOT = pathlib.Path(__file__).resolve().parent
if str(TOOL_ROOT) not in sys.path:
    sys.path.insert(0, str(TOOL_ROOT))

from experiment_runtime import (  # noqa: E402
    RuntimeErrorEvidence, clean_candidate, input_identity, read_json, relative_path,
    run_command, sha256_file, tool_version, utc_now, verify_input_identity, write_json, write_state,
)
from minimal_small_linear_algebra_evidence import EvidenceError, compare_index, validate_profile  # noqa: E402


BUILD_TIMEOUT_SECONDS = 300
PROCESS_TIMEOUT_SECONDS = 45
OVERALL_TIMEOUT_SECONDS = 1800
CONFIGURATIONS = {
    "gcc-debug": ("g++-13", False), "gcc-release": ("g++-13", False),
    "clang-debug": ("clang++-18", True), "clang-release": ("clang++-18", True),
}


def fail(message: str) -> RuntimeErrorEvidence:
    return RuntimeErrorEvidence(message)


def canonical(source: pathlib.Path, relative: str, supplied: str, label: str) -> pathlib.Path:
    expected, actual = (source / relative).resolve(), pathlib.Path(supplied).resolve()
    if expected != actual:
        raise fail(f"{label} must resolve to the candidate path")
    return expected


def published_candidate(source: pathlib.Path) -> dict[str, Any]:
    candidate = clean_candidate(source)
    upstream = subprocess.run(["git", "rev-parse", "@{u}"], cwd=source, capture_output=True, text=True, check=False)
    if upstream.returncode != 0 or upstream.stdout.strip() != candidate["commit"]:
        raise fail("candidate upstream is absent or differs from HEAD")
    candidate["upstream_commit"] = upstream.stdout.strip()
    return candidate


def inputs(arguments: argparse.Namespace, source: pathlib.Path) -> dict[str, pathlib.Path]:
    launcher = (source / "tools/run_minimal_small_linear_algebra_qualification.py").resolve()
    if pathlib.Path(__file__).resolve() != launcher:
        raise fail("launcher must execute from the candidate path")
    paths = {
        "profile": canonical(source, "experiments/profiles/minimal_small_linear_algebra.json", arguments.profile, "profile"),
        "protocol": canonical(source, "docs/decisions/GEOMETRY_MINIMAL_SMALL_LINEAR_ALGEBRA_QUALIFICATION_PROTOCOL.md", arguments.protocol, "protocol"),
        "authority": source / "docs/contracts/APMESH_CORE_MINIMAL_SMALL_LINEAR_ALGEBRA_CONTRACT.md",
        "architecture_authority": source / "docs/contracts/APMESH_CORE_ARCHITECTURE_CONTRACT.md",
        "numeric_authority": source / "docs/contracts/APMESH_CORE_NUMERIC_CONTRACT.md",
        "reproducibility_authority": source / "docs/contracts/APMESH_CORE_REPRODUCIBLE_EXPERIMENT_CONTRACT.md",
        "foundation_authority": source / "docs/decisions/FOUNDATION_END_TO_END_REGRESSION.md",
        "point_vector_authority": source / "docs/decisions/GEOMETRY_POINT_VECTOR_QUALIFICATION_PROTOCOL.md",
        "exporter": source / "experiments/minimal_small_linear_algebra_export.cpp",
        "validator": source / "tools/minimal_small_linear_algebra_evidence.py",
        "runner": launcher, "runtime": source / "tools/experiment_runtime.py", "cmake": source / "CMakeLists.txt",
        "math_header": source / "include/apmesh/math/linear_algebra.hpp", "math_source": source / "src/math/linear_algebra.cpp",
        "geometry_header": source / "include/apmesh/core/geometry.hpp", "geometry_source": source / "src/core/geometry.cpp",
        "focused_contract": source / "tests/minimal_small_linear_algebra.cpp", "header_contract": source / "tests/math_header_isolation.cpp",
        "evidence_contract": source / "tests/minimal_small_linear_algebra_evidence_test.py",
        "runner_contract": source / "tests/minimal_small_linear_algebra_runner_test.py",
        "retention_contract": source / "tests/minimal_small_linear_algebra_retention_test.py",
        "state": source / "docs/APMESH_CORE_STATE.md", "roadmap": source / "docs/APMESH_CORE_ROADMAP.md",
    }
    for path in paths.values():
        if not path.is_file():
            raise fail(f"required input is absent: {path}")
    return paths


def environment_identity() -> dict[str, Any]:
    return {
        "python": sys.version.splitlines()[0], "platform": platform.platform(),
        "cmake": tool_version("cmake"), "ctest": tool_version("ctest"), "ninja": tool_version("ninja"),
        "gcc": tool_version("g++-13"), "clang": tool_version("clang++-18"), "ldd": tool_version("ldd"),
        "observations": {key: os.environ.get(key, "") for key in ("LANG", "LC_ALL", "TZ")},
    }


def ctest_regex(names: list[str]) -> str:
    return "^(" + "|".join(repr(name)[1:-1] for name in names) + ")$"


def command_plan(profile: dict[str, Any], source: pathlib.Path) -> list[dict[str, Any]]:
    plan: list[dict[str, Any]] = []
    for cell in profile["cells"]:
        compiler, libcxx = CONFIGURATIONS[cell["id"]]
        build = f"@OUTPUT_ROOT@/cells/{cell['id']}/build"
        plan.append({
            "cell": cell["id"], "source_check": [sys.executable, str(source / "tools/minimal_small_linear_algebra_evidence.py"), "validate-source", "--profile", str(source / "experiments/profiles/minimal_small_linear_algebra.json"), "--source-root", str(source)], "configure": ["cmake", "-S", str(source), "-B", build, "-G", "Ninja", f"-DCMAKE_CXX_COMPILER={compiler}", f"-DCMAKE_BUILD_TYPE={cell['build_type']}", f"-DAPMESH_USE_LIBCXX={'ON' if libcxx else 'OFF'}", "-DBUILD_TESTING=ON", "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"],
            "build": ["cmake", "--build", build, "--target", "apmesh_core", "apmesh_core_minimal_small_linear_algebra_export", "apmesh_core.minimal_small_linear_algebra", "apmesh_core.math_header_isolation"],
            "focused_ctest": ["ctest", "--test-dir", build, "--output-on-failure", "-R", "^apmesh_core\\.(minimal_small_linear_algebra|math_header_isolation|minimal_small_linear_algebra_evidence|minimal_small_linear_algebra_runner|minimal_small_linear_algebra_retention)$"],
            "prerequisite_ctest": ["ctest", "--test-dir", build, "--output-on-failure", "-R", ctest_regex(profile["exact_prerequisite_tests"])],
            "exporter": f"{build}/apmesh_core_minimal_small_linear_algebra_export", "repetitions": profile["repetitions_per_cell"],
            "compile_commands": f"cells/{cell['id']}/build/compile_commands.json", "runtime_executables": ["apmesh_core_minimal_small_linear_algebra_export", "apmesh_core.minimal_small_linear_algebra", "apmesh_core.math_header_isolation"],
        })
    return plan


def planned_inventories(plan: list[dict[str, Any]]) -> dict[str, Any]:
    return {"schema_version": 1, "kind": "minimal-small-linear-algebra-planned-inventories", "cells": [{"cell": cell["cell"], "compile_commands": cell["compile_commands"], "runtime_executables": cell["runtime_executables"]} for cell in plan]}


def replace_root(argv: list[str], output: pathlib.Path) -> list[str]:
    return [part.replace("@OUTPUT_ROOT@", str(output)) for part in argv]


def expected_slots(profile: dict[str, Any]) -> list[dict[str, Any]]:
    return [{"cell": cell["id"], "repetition": repetition} for cell in profile["cells"] for repetition in range(1, profile["repetitions_per_cell"] + 1)]


def prepare(arguments: argparse.Namespace) -> None:
    source, output = pathlib.Path(arguments.source_root).resolve(), pathlib.Path(arguments.output_root).resolve()
    if output.exists():
        raise fail("output root already exists")
    paths = inputs(arguments, source); profile = validate_profile(paths["profile"]); candidate = published_candidate(source); environment = environment_identity(); plan = command_plan(profile, source)
    output.mkdir(parents=True)
    manifest = {"schema_version": 2, "kind": "minimal-small-linear-algebra-prepared-manifest", "state": "PREPARED", "execution_requested": False, "candidate": candidate, "inputs": input_identity(paths), "environment": environment, "working_directory": str(source), "output_root": str(output), "limits_seconds": {"build": BUILD_TIMEOUT_SECONDS, "process": PROCESS_TIMEOUT_SECONDS, "overall": OVERALL_TIMEOUT_SECONDS}, "plan": plan, "planned_inventories": planned_inventories(plan), "expected_certificate_slots": expected_slots(profile), "gates": {gate: "NOT_EXECUTED" for gate in profile["gates"]}, "retained_limitations": profile["limitations"], "prepared_utc": utc_now()}
    write_json(output / "prepared-manifest.json", manifest); write_json(output / "plan.json", {"schema_version": 1, "kind": "minimal-small-linear-algebra-launch-plan", "cells": plan}); write_state(output, "PREPARED", {"candidate_commit": candidate["commit"], "execution_requested": False, "prepared_manifest_sha256": sha256_file(output / "prepared-manifest.json")})


def load_prepared(arguments: argparse.Namespace) -> tuple[pathlib.Path, dict[str, Any], dict[str, Any]]:
    source, output = pathlib.Path(arguments.source_root).resolve(), pathlib.Path(arguments.output_root).resolve()
    manifest = read_json(output / "prepared-manifest.json"); profile = validate_profile(canonical(source, "experiments/profiles/minimal_small_linear_algebra.json", arguments.profile, "profile"))
    required = {"schema_version", "kind", "state", "execution_requested", "candidate", "inputs", "environment", "working_directory", "output_root", "limits_seconds", "plan", "planned_inventories", "expected_certificate_slots", "gates", "retained_limitations", "prepared_utc"}
    if set(manifest) != required or manifest["schema_version"] != 2 or manifest["kind"] != "minimal-small-linear-algebra-prepared-manifest" or manifest["state"] != "PREPARED" or manifest["execution_requested"] is not False:
        raise fail("prepared manifest schema differs")
    if manifest["candidate"] != published_candidate(source) or manifest["environment"] != environment_identity() or manifest["working_directory"] != str(source) or manifest["output_root"] != str(output):
        raise fail("prepared candidate or environment differs")
    verify_input_identity(manifest["inputs"], inputs(arguments, source))
    plan = command_plan(profile, source)
    if manifest["plan"] != plan or manifest["planned_inventories"] != planned_inventories(plan) or manifest["expected_certificate_slots"] != expected_slots(profile) or manifest["gates"] != {gate: "NOT_EXECUTED" for gate in profile["gates"]} or manifest["retained_limitations"] != profile["limitations"]:
        raise fail("prepared manifest plan differs")
    return output, manifest, profile


def require_success(record: dict[str, Any], context: str) -> None:
    if record["exit_code"] != 0 or record["timed_out"] or record["launch_error"] is not None:
        raise fail(f"{context} failed")


def verify_detached_candidate(source: pathlib.Path, candidate: dict[str, Any]) -> dict[str, Any]:
    with tempfile.TemporaryDirectory(prefix="apmesh-core-la-detached-") as temporary:
        detached = pathlib.Path(temporary) / "candidate"
        added = subprocess.run(["git", "worktree", "add", "--detach", str(detached), candidate["commit"]], cwd=source, capture_output=True, text=True, check=False)
        if added.returncode != 0:
            raise fail("detached candidate worktree could not be created")
        try:
            observed = clean_candidate(detached)
            expected_inventory = [{"path": row["path"], "sha256": row["sha256"]} for row in candidate["source_inventory"]]
            actual_inventory = [{"path": row["path"], "sha256": row["sha256"]} for row in observed["source_inventory"]]
            if observed["commit"] != candidate["commit"] or actual_inventory != expected_inventory:
                raise fail("detached candidate identity differs")
            return {"result": "PASS", "candidate_commit": candidate["commit"], "source_inventory_count": len(actual_inventory)}
        finally:
            removed = subprocess.run(["git", "worktree", "remove", "--force", str(detached)], cwd=source, capture_output=True, text=True, check=False)
            if removed.returncode != 0:
                raise fail("detached candidate worktree could not be removed")


def seal_output(output: pathlib.Path, source: pathlib.Path, manifest: dict[str, Any]) -> None:
    detached = verify_detached_candidate(source, manifest["candidate"])
    write_json(output / "detached-verification.json", {"schema_version": 1, "kind": "minimal-small-linear-algebra-detached-verification", **detached})
    files = sorted(path for path in output.rglob("*") if path.is_file() and path.name != "retention-manifest.json" and "/build/" not in path.relative_to(output).as_posix())
    write_json(output / "retention-manifest.json", {"schema_version": 2, "kind": "minimal-small-linear-algebra-retention", "candidate_commit": manifest["candidate"]["commit"], "prepared_manifest_sha256": sha256_file(output / "prepared-manifest.json"), "files": [{"path": relative_path(output, path), "sha256": sha256_file(path), "size": path.stat().st_size} for path in files]})
    verify_retention(argparse.Namespace(output_root=str(output)))


def execute(arguments: argparse.Namespace) -> None:
    output, manifest, profile = load_prepared(arguments)
    source = pathlib.Path(arguments.source_root).resolve(); write_state(output, "RUNNING", {"candidate_commit": manifest["candidate"]["commit"], "prepared_manifest_sha256": sha256_file(output / "prepared-manifest.json")})
    records: list[dict[str, Any]] = []; entries: list[dict[str, Any]] = []
    try:
        for cell in manifest["plan"]:
            for stage in ("source_check", "configure", "build", "focused_ctest", "prerequisite_ctest"):
                record = run_command(replace_root(cell[stage], output), source, output / "logs", f"{cell['cell']}-{stage}", BUILD_TIMEOUT_SECONDS)
                records.append(record); require_success(record, f"{cell['cell']} {stage}")
            exporter = pathlib.Path(cell["exporter"].replace("@OUTPUT_ROOT@", str(output)))
            for repetition in range(1, profile["repetitions_per_cell"] + 1):
                certificate = output / "certificates" / f"{cell['cell']}-{repetition}.json"; certificate.parent.mkdir(parents=True, exist_ok=True)
                record = run_command([str(exporter), str(certificate)], source, output / "logs", f"{cell['cell']}-certificate-{repetition}", PROCESS_TIMEOUT_SECONDS)
                records.append(record); require_success(record, f"{cell['cell']} certificate {repetition}")
                entries.append({"cell": cell["cell"], "repetition": repetition, "path": relative_path(output, certificate), "sha256": sha256_file(certificate)})
        write_json(output / "command-records.json", {"schema_version": 1, "kind": "minimal-small-linear-algebra-command-records", "records": records})
        write_json(output / "certificate-index.json", {"schema_version": 1, "kind": "minimal-small-linear-algebra-certificate-index", "entries": entries})
        comparison = compare_index(profile, output / "certificate-index.json"); write_json(output / "cross-cell-comparison.json", comparison)
        write_json(output / "terminal-manifest.json", {"schema_version": 2, "kind": "minimal-small-linear-algebra-terminal-manifest", "state": "EXECUTED_PENDING_AUDIT", "candidate": manifest["candidate"], "prepared_manifest_sha256": sha256_file(output / "prepared-manifest.json"), "records": "command-records.json", "comparison": "cross-cell-comparison.json", "retention_manifest": "retention-manifest.json"})
        write_state(output, "EXECUTED_PENDING_AUDIT", {"candidate_commit": manifest["candidate"]["commit"], "comparison": "cross-cell-comparison.json"})
        seal_output(output, source, manifest)
    except (RuntimeErrorEvidence, EvidenceError) as error:
        write_json(output / "failure.json", {"schema_version": 1, "kind": "minimal-small-linear-algebra-failure", "message": str(error), "records": records})
        write_state(output, "BLOCKED", {"candidate_commit": manifest["candidate"]["commit"], "closure_failure": True, "failure": "failure.json"})
        raise


def verify_retention(arguments: argparse.Namespace) -> None:
    root = pathlib.Path(arguments.output_root).resolve(); retention = read_json(root / "retention-manifest.json")
    if not isinstance(retention, dict) or set(retention) != {"schema_version", "kind", "candidate_commit", "prepared_manifest_sha256", "files"} or retention["schema_version"] != 2 or retention["kind"] != "minimal-small-linear-algebra-retention" or not isinstance(retention["files"], list):
        raise fail("retention manifest schema differs")
    prepared = root / "prepared-manifest.json"
    if not prepared.is_file() or retention["prepared_manifest_sha256"] != sha256_file(prepared): raise fail("retention prepared binding differs")
    expected = {"prepared-manifest.json", "retention-manifest.json"}
    for entry in retention["files"]:
        if not isinstance(entry, dict) or set(entry) != {"path", "sha256", "size"}: raise fail("retention entry schema differs")
        path = (root / entry["path"]).resolve()
        if root not in path.parents or not path.is_file() or path.stat().st_size != entry["size"] or sha256_file(path) != entry["sha256"]: raise fail("retention entry differs")
        expected.add(entry["path"])
    actual = {path.relative_to(root).as_posix() for path in root.rglob("*") if path.is_file() and "/build/" not in path.relative_to(root).as_posix()}
    if actual != expected: raise fail("retention file set differs")


def self_check(arguments: argparse.Namespace) -> None:
    source = pathlib.Path(arguments.source_root).resolve(); profile = validate_profile(canonical(source, "experiments/profiles/minimal_small_linear_algebra.json", arguments.profile, "profile")); inputs(arguments, source)
    if len(command_plan(profile, source)) != 4 or len(expected_slots(profile)) != 12: raise fail("fixed qualification matrix differs")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__); parser.add_argument("--source-root", required=True); parser.add_argument("--profile", required=True); parser.add_argument("--protocol", required=True); parser.add_argument("--output-root")
    commands = parser.add_subparsers(dest="command", required=True); commands.add_parser("self-check"); commands.add_parser("prepare"); commands.add_parser("execute"); commands.add_parser("verify-retention")
    arguments = parser.parse_args()
    try:
        if arguments.command == "self-check": self_check(arguments)
        elif arguments.command == "prepare":
            if not arguments.output_root: raise fail("output root is required")
            prepare(arguments)
        elif arguments.command == "execute":
            if not arguments.output_root: raise fail("output root is required")
            execute(arguments)
        else:
            if not arguments.output_root: raise fail("output root is required")
            verify_retention(arguments)
        return 0
    except (EvidenceError, RuntimeErrorEvidence) as error:
        print(f"LA qualification error: {error}", file=sys.stderr); return 2


if __name__ == "__main__":
    raise SystemExit(main())
