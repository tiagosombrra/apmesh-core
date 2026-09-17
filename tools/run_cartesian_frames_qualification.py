#!/usr/bin/env python3
"""Prepare and verify CF0-CF7 evidence; formal execution is intentionally absent."""

from __future__ import annotations

import argparse
import json
import pathlib
import re
import subprocess
import sys
from typing import Any

TOOL_ROOT = pathlib.Path(__file__).resolve().parent
if str(TOOL_ROOT) not in sys.path:
    sys.path.insert(0, str(TOOL_ROOT))

from cartesian_frames_evidence import EvidenceError, validate_profile, validate_source  # noqa: E402
from experiment_runtime import (  # noqa: E402
    RuntimeErrorEvidence, clean_candidate, input_identity, read_json, relative_path,
    sha256_file, tool_version, utc_now, write_json, write_state,
)


class RunnerError(RuntimeError):
    pass


FOCUSED_TESTS = [
    "apmesh_core.cartesian_frames",
    "apmesh_core.cartesian_frames_evidence",
    "apmesh_core.cartesian_frames_runner",
    "apmesh_core.cartesian_frames_retention",
]
# Lifecycle records intentionally evolve from PREPARED to terminal state.  Only
# these immutable preparation bytes belong to the preparation seal.
PREPARATION_FILES = ("prepared-manifest.json", "plan.json", "planned-inventories.json", "profile.json")


def fail(message: str) -> RunnerError:
    return RunnerError(message)


def canonical(source: pathlib.Path, relative: str, supplied: str, label: str) -> pathlib.Path:
    expected = (source / relative).resolve()
    actual = pathlib.Path(supplied).resolve()
    if expected != actual:
        raise fail(f"{label} must resolve to the candidate path")
    return expected


def published_candidate(source: pathlib.Path) -> dict[str, Any]:
    try:
        candidate = clean_candidate(source)
    except RuntimeErrorEvidence as error:
        raise fail(str(error)) from error
    upstream = subprocess.run(["git", "rev-parse", "@{upstream}"], cwd=source, capture_output=True, text=True, check=False)
    if upstream.returncode != 0 or upstream.stdout.strip() != candidate["commit"]:
        raise fail("candidate upstream is absent or differs from HEAD")
    candidate["upstream_commit"] = upstream.stdout.strip()
    return candidate


def input_paths(arguments: argparse.Namespace, source: pathlib.Path) -> dict[str, pathlib.Path]:
    runner = (source / "tools/run_cartesian_frames_qualification.py").resolve()
    if pathlib.Path(__file__).resolve() != runner:
        raise fail("runner must execute from the candidate path")
    paths = {
        "profile": canonical(source, "experiments/profiles/cartesian_frames.json", arguments.profile, "profile"),
        "protocol": canonical(source, "docs/decisions/GEOMETRY_CARTESIAN_FRAMES_QUALIFICATION_PROTOCOL.md", arguments.protocol, "protocol"),
        "entry_authority": source / "docs/decisions/GEOMETRY_TRANSFORMATIONS_COORDINATE_FRAMES_ENTRY_DECISION.md",
        "point_vector_authority": source / "docs/decisions/GEOMETRY_POINT_VECTOR_QUALIFICATION_PROTOCOL.md",
        "linear_algebra_authority": source / "docs/decisions/GEOMETRY_MINIMAL_SMALL_LINEAR_ALGEBRA_QUALIFICATION_PROTOCOL.md",
        "foundation_authority": source / "docs/decisions/FOUNDATION_END_TO_END_REGRESSION.md",
        "reproducible_experiment_authority": source / "docs/contracts/APMESH_CORE_REPRODUCIBLE_EXPERIMENT_CONTRACT.md",
        "exporter": canonical(source, "experiments/cartesian_frames_export.cpp", arguments.exporter, "exporter"),
        "validator": canonical(source, "tools/cartesian_frames_evidence.py", arguments.validator, "validator"),
        "runtime": source / "tools/experiment_runtime.py",
        "cmake": source / "CMakeLists.txt",
        "presets": source / "CMakePresets.json",
        "geometry_header": source / "include/apmesh/core/geometry.hpp",
        "geometry_source": source / "src/core/geometry.cpp",
        "focused_contract": source / "tests/cartesian_frames.cpp",
        "runner_contract": source / "tests/cartesian_frames_runner_test.py",
        "retention_contract": source / "tests/cartesian_frames_retention_test.py",
        "evidence_contract": source / "tests/cartesian_frames_evidence_test.py",
        "state": source / "docs/APMESH_CORE_STATE.md",
        "roadmap": source / "docs/APMESH_CORE_ROADMAP.md",
    }
    for path in paths.values():
        if not path.is_file():
            raise fail(f"required input is absent: {path}")
    return paths


def protocol_check(path: pathlib.Path) -> None:
    content = path.read_text(encoding="utf-8")
    required = (
        "Status: PRE-REGISTERED / REPORT-ONLY INFRASTRUCTURE IMPLEMENTED / CF0–CF7 NOT EXECUTED",
        "`CF0`", "`CF1`", "`CF2`", "`CF3`", "`CF4`", "`CF5`", "`CF6`", "`CF7`",
        "execution_requested=false", "exact, versioned prerequisite allowlist", "detached-worktree verification",
    )
    if any(token not in content for token in required):
        raise fail("protocol is not the pre-registered CF0-CF7 authority")


def environment_identity() -> dict[str, Any]:
    try:
        return {"cmake": tool_version("cmake"), "ctest": tool_version("ctest"), "ninja": tool_version("ninja"), "python": tool_version("python")}
    except RuntimeErrorEvidence as error:
        raise fail(str(error)) from error


def ctest_regex(names: list[str]) -> str:
    return "^(?:" + "|".join(re.escape(name) for name in names) + ")$"


def validate_prerequisite_discovery(discovered: list[str], expected: list[str]) -> None:
    if sorted(discovered) != sorted(expected) or len(discovered) != len(set(discovered)):
        raise fail("CTest prerequisite allowlist differs from discovery")


def command_plan(profile: dict[str, Any], source: pathlib.Path) -> list[dict[str, Any]]:
    plan: list[dict[str, Any]] = []
    focused_regex = ctest_regex(FOCUSED_TESTS)
    prerequisite_regex = ctest_regex(profile["exact_prerequisite_tests"])
    for cell in profile["cells"]:
        build = source / "build" / cell["id"]
        exporter = build / "apmesh_core_cartesian_frames_export"
        plan.append({
            "cell": cell["id"], "preset": cell["id"], "compiler": cell["compiler"],
            "library": cell["library"], "build_type": cell["build_type"], "repetitions": profile["repetitions_per_cell"],
            "commands": {
                "configure": ["cmake", "--preset", cell["id"]],
                "build": ["cmake", "--build", "--preset", cell["id"], "--target", "apmesh_core", "apmesh_core.cartesian_frames", "apmesh_core_cartesian_frames_export"],
                "prerequisite_discovery": ["ctest", "--test-dir", str(build), "-N"],
                "focused_ctest": ["ctest", "--test-dir", str(build), "--output-on-failure", "-R", focused_regex],
                "prerequisite_ctest": ["ctest", "--test-dir", str(build), "--output-on-failure", "-R", prerequisite_regex],
                "certificate": [str(exporter), "certificate", f"@OUTPUT_ROOT@/certificates/{cell['id']}-@REPETITION@.json"],
                "negative_outcomes": [sys.executable, str(source / "tools/cartesian_frames_evidence.py"), "--profile", str(source / "experiments/profiles/cartesian_frames.json"), "negative-outcomes", "--certificate", f"@OUTPUT_ROOT@/certificates/{cell['id']}-1.json", "--output", f"@OUTPUT_ROOT@/negatives/{cell['id']}.json"],
                "runtime_dependencies": ["ldd", str(exporter)],
            },
        })
    return plan


def planned_inventories(profile: dict[str, Any], candidate: dict[str, Any]) -> dict[str, Any]:
    cells = []
    artifacts: list[dict[str, str]] = [
        {"path": "prepared-manifest.json", "role": "prepared-manifest"}, {"path": "plan.json", "role": "launch-plan"},
        {"path": "planned-inventories.json", "role": "planned-inventory"}, {"path": "state.json", "role": "lifecycle-state"},
        {"path": "state-history.jsonl", "role": "lifecycle-history"}, {"path": "command-records.json", "role": "command-records"},
        {"path": "certificate-index.json", "role": "certificate-index"}, {"path": "cross-cell-comparison.json", "role": "cross-cell-comparison"},
        {"path": "gate-summary.json", "role": "gate-summary"}, {"path": "terminal-manifest.json", "role": "terminal-manifest"},
        {"path": "detached-verification.json", "role": "detached-verification"}, {"path": "retention-manifest.json", "role": "retention-manifest"},
    ]
    for cell in profile["cells"]:
        name = cell["id"]
        cells.append({"cell": name, "repetitions": profile["repetitions_per_cell"], "prerequisites": profile["exact_prerequisite_tests"]})
        artifacts += [
            {"path": f"cells/{name}/compile_commands.json", "role": "compile-command-inventory"},
            {"path": f"cells/{name}/runtime-dependencies.json", "role": "runtime-dependency-inventory"},
            {"path": f"cells/{name}/prerequisite-discovery.json", "role": "prerequisite-discovery"},
            {"path": f"cells/{name}/prerequisite-preservation.json", "role": "prerequisite-preservation"},
            {"path": f"negatives/{name}.json", "role": "certificate-negative-outcomes"},
        ]
        artifacts += [{"path": f"certificates/{name}-{repetition}.json", "role": "semantic-certificate"} for repetition in range(1, profile["repetitions_per_cell"] + 1)]
    return {"schema_version": 1, "kind": "cartesian-frames-planned-inventories", "candidate_source_inventory": candidate["source_inventory"], "cells": cells, "artifacts": artifacts}


def report_only_plan(profile: dict[str, Any], source: pathlib.Path) -> dict[str, Any]:
    return {"schema_version": 2, "kind": "cartesian-frames-launch-plan", "execution_requested": False, "cells": command_plan(profile, source), "gates": {gate: "NOT_EXECUTED" for gate in profile["gates"]}, "limitations": profile["limitations"]}


def self_check(arguments: argparse.Namespace) -> dict[str, Any]:
    source = pathlib.Path(arguments.source_root).resolve()
    paths = input_paths(arguments, source)
    profile = validate_profile(paths["profile"])
    protocol_check(paths["protocol"])
    return {"schema_version": 2, "kind": "cartesian-frames-runner-self-check", "status": "PASS", "execution_requested": False, "input_hashes": input_identity(paths), "source_checks": validate_source(source), "plan": report_only_plan(profile, source)}


def preparation_seal(output: pathlib.Path) -> dict[str, Any]:
    return {"schema_version": 1, "kind": "cartesian-frames-preparation-seal", "files": [{"path": name, "sha256": sha256_file(output / name)} for name in PREPARATION_FILES]}


def validate_prepared(output: pathlib.Path, *, require_unconsumed: bool) -> dict[str, Any]:
    manifest = read_json(output / "prepared-manifest.json")
    expected = {"schema_version", "kind", "state", "execution_requested", "candidate", "inputs", "environment", "working_directory", "output_root", "plan", "planned_inventories", "gates", "retained_limitations", "prepared_utc"}
    if set(manifest) != expected or manifest["schema_version"] != 1 or manifest["kind"] != "cartesian-frames-prepared-manifest" or manifest["state"] != "PREPARED" or manifest["execution_requested"] is not False:
        raise fail("prepared manifest schema differs")
    expected_plan = {"schema_version": 2, "kind": "cartesian-frames-launch-plan", "cells": manifest["plan"], "gates": manifest["gates"], "limitations": manifest["retained_limitations"]}
    if read_json(output / "plan.json") != expected_plan or read_json(output / "planned-inventories.json") != manifest["planned_inventories"] or read_json(output / "preparation-seal.json") != preparation_seal(output):
        raise fail("prepared chain differs")
    state = read_json(output / "state.json")
    if state.get("state") not in {"PREPARED", "RUNNING", "EXECUTED_PENDING_AUDIT", "BLOCKED"}:
        raise fail("prepared lifecycle state differs")
    if require_unconsumed and (state.get("state") != "PREPARED" or state.get("detail") != {"candidate_commit": manifest["candidate"]["commit"], "execution_requested": False, "prepared_manifest_sha256": sha256_file(output / "prepared-manifest.json")} or (output / "terminal-manifest.json").exists()):
        raise fail("prepared attempt is already consumed")
    return manifest


def prepare(arguments: argparse.Namespace) -> None:
    source = pathlib.Path(arguments.source_root).resolve()
    output = pathlib.Path(arguments.output_root).resolve()
    if output.exists() or source == output or source in output.parents:
        raise fail("output root must be a new external directory")
    paths = input_paths(arguments, source)
    profile = validate_profile(paths["profile"])
    protocol_check(paths["protocol"])
    candidate = published_candidate(source)
    plan = command_plan(profile, source)
    inventories = planned_inventories(profile, candidate)
    output.mkdir(parents=True)
    write_json(output / "profile.json", profile)
    manifest = {"schema_version": 1, "kind": "cartesian-frames-prepared-manifest", "state": "PREPARED", "execution_requested": False, "candidate": candidate, "inputs": input_identity(paths), "environment": environment_identity(), "working_directory": str(source), "output_root": str(output), "plan": plan, "planned_inventories": inventories, "gates": {gate: "NOT_EXECUTED" for gate in profile["gates"]}, "retained_limitations": profile["limitations"], "prepared_utc": utc_now()}
    write_json(output / "prepared-manifest.json", manifest)
    write_json(output / "plan.json", {"schema_version": 2, "kind": "cartesian-frames-launch-plan", "cells": plan, "gates": manifest["gates"], "limitations": manifest["retained_limitations"]})
    write_json(output / "planned-inventories.json", inventories)
    write_state(output, "PREPARED", {"candidate_commit": candidate["commit"], "execution_requested": False, "prepared_manifest_sha256": sha256_file(output / "prepared-manifest.json")})
    write_json(output / "preparation-seal.json", preparation_seal(output))
    validate_prepared(output, require_unconsumed=True)


def verify_retention(root: pathlib.Path) -> dict[str, Any]:
    manifest = validate_prepared(root, require_unconsumed=False)
    terminal = read_json(root / "terminal-manifest.json")
    retention = read_json(root / "retention-manifest.json")
    detached = read_json(root / "detached-verification.json")
    terminal_keys = {"schema_version", "kind", "state", "candidate", "prepared_manifest_sha256", "command_records", "inventories", "limitations"}
    if set(terminal) != terminal_keys or terminal["schema_version"] != 1 or terminal["kind"] != "cartesian-frames-terminal-manifest" or terminal["state"] not in {"EXECUTED_PENDING_AUDIT", "BLOCKED"} or terminal["candidate"] != manifest["candidate"] or terminal["prepared_manifest_sha256"] != sha256_file(root / "prepared-manifest.json"):
        raise fail("terminal manifest differs")
    if read_json(root / "state.json").get("state") != terminal["state"]:
        raise fail("terminal lifecycle state differs")
    retention_keys = {"schema_version", "kind", "candidate_commit", "prepared_manifest_sha256", "required_paths", "files"}
    if set(retention) != retention_keys or retention["schema_version"] != 1 or retention["kind"] != "cartesian-frames-retention" or retention["candidate_commit"] != manifest["candidate"]["commit"] or retention["prepared_manifest_sha256"] != sha256_file(root / "prepared-manifest.json"):
        raise fail("retention manifest identity differs")
    expected_detached = {"schema_version": 1, "kind": "cartesian-frames-detached-verification", "result": "PASS", "candidate_commit": manifest["candidate"]["commit"], "source_inventory_count": len(manifest["candidate"]["source_inventory"])}
    if detached != expected_detached:
        raise fail("detached verification differs")
    listed: set[str] = {"retention-manifest.json"}
    for entry in retention["files"]:
        if not isinstance(entry, dict) or set(entry) != {"path", "role", "sha256", "size"}:
            raise fail("retention entry schema differs")
        relative = entry["path"]
        path = root / pathlib.PurePosixPath(relative)
        if not isinstance(relative, str) or relative in listed or pathlib.PurePosixPath(relative).is_absolute() or ".." in pathlib.PurePosixPath(relative).parts or relative.startswith("build/") or not path.is_file() or path.stat().st_size != entry["size"] or sha256_file(path) != entry["sha256"]:
            raise fail("retention file differs")
        listed.add(relative)
    actual = {relative_path(root, path) for path in root.rglob("*") if path.is_file()}
    if actual != listed or set(retention["required_paths"]) - listed:
        raise fail("retention file set differs")
    return {"schema_version": 1, "kind": "cartesian-frames-retention-verification", "status": "PASS", "files": sorted(listed)}


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source-root", required=True)
    parser.add_argument("--profile", required=True)
    parser.add_argument("--protocol", required=True)
    parser.add_argument("--exporter", required=True)
    parser.add_argument("--validator", required=True)
    commands = parser.add_subparsers(dest="command", required=True)
    for name in ("self-check", "plan"):
        command = commands.add_parser(name)
        command.add_argument("--output", required=True)
    prepare_command = commands.add_parser("prepare")
    prepare_command.add_argument("--output-root", required=True)
    retention_command = commands.add_parser("verify-retention")
    retention_command.add_argument("--evidence-root", required=True)
    retention_command.add_argument("--output", required=True)
    arguments = parser.parse_args()
    try:
        if arguments.command == "prepare":
            prepare(arguments)
            return 0
        if arguments.command == "verify-retention":
            result = verify_retention(pathlib.Path(arguments.evidence_root).resolve())
        else:
            result = self_check(arguments)
            if arguments.command == "plan":
                result = result["plan"]
        write_json(pathlib.Path(arguments.output), result)
    except (EvidenceError, RunnerError, RuntimeErrorEvidence) as error:
        print(f"runner error: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
