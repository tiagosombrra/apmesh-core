#!/usr/bin/env python3
"""Prepare, execute once, and retain revision-bound CF0-CF7 evidence."""
from __future__ import annotations

import argparse
import datetime as dt
import hashlib
import json
import os
import pathlib
import platform
import re
import shutil
import subprocess
import sys
import tempfile
import time
from typing import Any

TOOL_ROOT = pathlib.Path(__file__).resolve().parent
if str(TOOL_ROOT) not in sys.path:
    sys.path.insert(0, str(TOOL_ROOT))

from cartesian_frames_evidence import EvidenceError, compare_certificates, compare_per_cell_certificates, gate_summary, render_gate_summary_markdown, validate_certificate, validate_gate_summary_markdown, validate_negative_outcomes, validate_per_cell_comparisons, validate_profile, validate_source  # noqa: E402
from experiment_runtime import RuntimeErrorEvidence, clean_candidate, input_identity, read_json, relative_path, run_command, sha256_file, tool_version, utc_now, verify_input_identity, write_json, write_state  # noqa: E402

BUILD_TIMEOUT_SECONDS = 300
PROCESS_TIMEOUT_SECONDS = 45
OVERALL_TIMEOUT_SECONDS = 1800
FOCUSED_TESTS = ["apmesh_core.cartesian_frames", "apmesh_core.cartesian_frames_evidence", "apmesh_core.cartesian_frames_runner", "apmesh_core.cartesian_frames_retention"]
CONFIGURATIONS = {"gcc-debug": ("g++-13", False), "gcc-release": ("g++-13", False), "clang-debug": ("clang++-18", True), "clang-release": ("clang++-18", True)}
SUCCESS_FILES = {"profile.json", "preparation-seal.json", "execution-claim.json", "planned-inventories.json", "prepared-manifest.json", "plan.json", "state.json", "state-history.jsonl", "command-records.json", "source-checks.json", "prerequisite-discovery.json", "observed-inventories.json", "certificate-index.json", "per-cell-comparisons.json", "cross-cell-comparison.json", "gate-summary.json", "gate-summary.md", "terminal-manifest.json", "detached-verification.json", "retention-manifest.json"}
FAILURE_FILES = {"profile.json", "preparation-seal.json", "execution-claim.json", "planned-inventories.json", "prepared-manifest.json", "plan.json", "state.json", "state-history.jsonl", "command-records.json", "failure.json", "terminal-manifest.json", "detached-verification.json", "retention-manifest.json"}
SEAL_FAILURE_FILES = {"retention-error.json", "pre-seal-terminal.json"}
PREPARATION_FILES = ("prepared-manifest.json", "plan.json", "planned-inventories.json", "profile.json")
SUCCESS_TERMINAL_REFERENCES = {
    "observed_inventories": "observed-inventories.json",
    "certificate_index": "certificate-index.json",
    "per_cell_comparison": "per-cell-comparisons.json",
    "comparison": "cross-cell-comparison.json",
    "gate_summary": "gate-summary.json",
    "gate_summary_markdown": "gate-summary.md",
    "prerequisite_discovery": "prerequisite-discovery.json",
}


class RunnerError(RuntimeError):
    pass


def fail(message: str) -> RunnerError:
    return RunnerError(message)


def _reject_duplicate_compile_command_keys(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise fail("duplicate key in compile command inventory")
        result[key] = value
    return result


def read_compile_commands(path: pathlib.Path) -> list[dict[str, Any]]:
    try:
        value = json.loads(
            path.read_text(encoding="utf-8"),
            object_pairs_hook=_reject_duplicate_compile_command_keys,
        )
    except (OSError, UnicodeDecodeError, json.JSONDecodeError) as error:
        raise fail(f"invalid compile command inventory: {path}") from error
    if not isinstance(value, list):
        raise fail(f"compile command inventory must be a JSON list: {path}")
    return value


def canonical(source: pathlib.Path, relative: str, supplied: str, label: str) -> pathlib.Path:
    expected, actual = (source / relative).resolve(), pathlib.Path(supplied).resolve()
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
        "architecture_authority": source / "docs/contracts/APMESH_CORE_ARCHITECTURE_CONTRACT.md",
        "numeric_authority": source / "docs/contracts/APMESH_CORE_NUMERIC_CONTRACT.md",
        "reproducible_experiment_authority": source / "docs/contracts/APMESH_CORE_REPRODUCIBLE_EXPERIMENT_CONTRACT.md",
        "foundation_authority": source / "docs/decisions/FOUNDATION_END_TO_END_REGRESSION.md",
        "point_vector_authority": source / "docs/decisions/GEOMETRY_POINT_VECTOR_QUALIFICATION_PROTOCOL.md",
        "linear_algebra_authority": source / "docs/decisions/GEOMETRY_MINIMAL_SMALL_LINEAR_ALGEBRA_QUALIFICATION_PROTOCOL.md",
        "exporter": canonical(source, "experiments/cartesian_frames_export.cpp", arguments.exporter, "exporter"),
        "validator": canonical(source, "tools/cartesian_frames_evidence.py", arguments.validator, "validator"),
        "runner": runner, "runtime": source / "tools/experiment_runtime.py", "cmake": source / "CMakeLists.txt", "presets": source / "CMakePresets.json",
        "geometry_header": source / "include/apmesh/core/geometry.hpp", "geometry_source": source / "src/core/geometry.cpp",
        "focused_contract": source / "tests/cartesian_frames.cpp", "runner_contract": source / "tests/cartesian_frames_runner_test.py", "retention_contract": source / "tests/cartesian_frames_retention_test.py", "evidence_contract": source / "tests/cartesian_frames_evidence_test.py",
        "state": source / "docs/APMESH_CORE_STATE.md", "roadmap": source / "docs/APMESH_CORE_ROADMAP.md",
    }
    for path in paths.values():
        if not path.is_file():
            raise fail(f"required input is absent: {path}")
    return paths


def protocol_check(path: pathlib.Path) -> None:
    content = path.read_text(encoding="utf-8")
    required = ("## 7. CF0", "execution_requested=false", "exact, versioned prerequisite allowlist", "detached-worktree verification")
    if any(token not in content for token in required):
        raise fail("protocol is not the pre-registered CF0-CF7 authority")


def environment_identity() -> dict[str, Any]:
    try:
        return {
            "python": tool_identity(sys.executable),
            "platform": platform.platform(),
            "cmake": tool_identity("cmake"),
            "ctest": tool_identity("ctest"),
            "ninja": tool_identity("ninja"),
            "gcc": tool_identity("g++-13"),
            "clang": tool_identity("clang++-18"),
            "ldd": tool_identity("ldd"),
            "observations": {name: os.environ.get(name, "") for name in ("LANG", "LC_ALL", "TZ")},
        }
    except RuntimeErrorEvidence as error:
        raise fail(str(error)) from error


def tool_identity(command: str) -> dict[str, str]:
    resolved = shutil.which(command)
    if resolved is None:
        raise RuntimeErrorEvidence(f"required tool is unavailable: {command}")
    invoked = pathlib.Path(resolved).absolute()
    executable = invoked.resolve()
    if not executable.is_file():
        raise RuntimeErrorEvidence(f"required tool is not a regular file: {command}")
    version = tool_version(command)["version"]
    return {
        "command": command,
        "invocation_path": str(invoked),
        "resolved_path": str(executable),
        "sha256": sha256_file(executable),
        "version": version,
    }


def ctest_regex(names: list[str]) -> str:
    return "^(?:" + "|".join(re.escape(name) for name in names) + ")$"


def validate_prerequisite_discovery(discovered: list[str], expected: list[str]) -> None:
    if sorted(discovered) != sorted(expected) or len(discovered) != len(set(discovered)):
        raise fail("CTest prerequisite allowlist differs from discovery")


def command_plan(profile: dict[str, Any], source: pathlib.Path) -> list[dict[str, Any]]:
    plan: list[dict[str, Any]] = []
    for cell in profile["cells"]:
        compiler, libcxx = CONFIGURATIONS[cell["id"]]
        build = f"@OUTPUT_ROOT@/cells/{cell['id']}/build"
        exporter = f"{build}/apmesh_core_cartesian_frames_export"
        plan.append({
            "cell": cell["id"], "compiler": compiler, "library": cell["library"], "build_type": cell["build_type"], "repetitions": profile["repetitions_per_cell"],
            "source_check": [sys.executable, str(source / "tools/cartesian_frames_evidence.py"), "--profile", str(source / "experiments/profiles/cartesian_frames.json"), "validate-source", "--source-root", str(source), "--output", f"@OUTPUT_ROOT@/cells/{cell['id']}/source-check.json"],
            "configure": ["cmake", "-S", str(source), "-B", build, "-G", "Ninja", f"-DCMAKE_CXX_COMPILER={compiler}", f"-DCMAKE_BUILD_TYPE={cell['build_type']}", f"-DAPMESH_USE_LIBCXX={'ON' if libcxx else 'OFF'}", "-DBUILD_TESTING=ON", "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"],
            "build": ["cmake", "--build", build, "--target", "apmesh_core", "apmesh_core.cartesian_frames", "apmesh_core_cartesian_frames_export"],
            "prerequisite_discovery": ["ctest", "--test-dir", build, "-N"],
            "focused_ctest": ["ctest", "--test-dir", build, "--output-on-failure", "-R", ctest_regex(FOCUSED_TESTS)],
            "prerequisite_ctest": ["ctest", "--test-dir", build, "--output-on-failure", "-R", ctest_regex(profile["exact_prerequisite_tests"])],
            "certificate": [exporter, "certificate", f"@OUTPUT_ROOT@/certificates/{cell['id']}-@REPETITION@.json"],
            "certificate_validation": [sys.executable, str(source / "tools/cartesian_frames_evidence.py"), "--profile", str(source / "experiments/profiles/cartesian_frames.json"), "validate-certificate", "--certificate", f"@OUTPUT_ROOT@/certificates/{cell['id']}-@REPETITION@.json"],
            "negative_outcomes": [sys.executable, str(source / "tools/cartesian_frames_evidence.py"), "--profile", str(source / "experiments/profiles/cartesian_frames.json"), "negative-outcomes", "--certificate", f"@OUTPUT_ROOT@/certificates/{cell['id']}-1.json", "--output", f"@OUTPUT_ROOT@/negatives/{cell['id']}.json"],
            "runtime_executables": ["apmesh_core_cartesian_frames_export", "apmesh_core.cartesian_frames"],
        })
    return plan


def planned_inventories(profile: dict[str, Any], candidate: dict[str, Any], plan: list[dict[str, Any]]) -> dict[str, Any]:
    artifacts = [{"path": path, "role": "terminal-control", "required_when": "always" if path in SUCCESS_FILES & FAILURE_FILES else "success" if path in SUCCESS_FILES else "failure"} for path in sorted(SUCCESS_FILES | FAILURE_FILES)]
    artifacts += [{"path": path, "role": "seal-failure", "required_when": "seal-failure"} for path in sorted(SEAL_FAILURE_FILES)]
    artifacts += [{"path": path, "role": "seal-snapshot", "required_when": "if-produced"} for path in ("pre-seal-detached.json", "pre-seal-retention.json")]
    cells = []
    for cell in plan:
        name = cell["cell"]
        stages = ["source-check", "configure", "build", "prerequisite-discovery", "focused-ctest", "prerequisite-ctest", "negative-outcomes"]
        stages += [f"certificate-{index}" for index in range(1, cell["repetitions"] + 1)] + [f"certificate-validation-{index}" for index in range(1, cell["repetitions"] + 1)]
        stages += ["ldd-" + name.replace(".", "_") for name in cell["runtime_executables"]]
        for stage in stages:
            artifacts += [{"path": f"logs/{name}-{stage}.{stream}.log", "role": "command-log", "required_when": "command-started"} for stream in ("stdout", "stderr")]
        artifacts += [{"path": f"certificates/{name}-{index}.json", "role": "semantic-certificate", "required_when": "success"} for index in range(1, cell["repetitions"] + 1)]
        artifacts += [{"path": f"cells/{name}/source-check.json", "role": "source-check", "required_when": "success"}, {"path": f"cells/{name}/compile_commands.json", "role": "compile-command-inventory", "required_when": "success"}, {"path": f"negatives/{name}.json", "role": "certificate-negative-outcomes", "required_when": "success"}]
        cells.append({"cell": name, "repetitions": cell["repetitions"], "prerequisites": profile["exact_prerequisite_tests"], "runtime_executables": cell["runtime_executables"]})
    return {"schema_version": 2, "kind": "cartesian-frames-planned-inventories", "candidate_source_inventory": candidate["source_inventory"], "cells": cells, "artifacts": artifacts}


def preparation_seal(output: pathlib.Path) -> dict[str, Any]:
    first = (output / "state-history.jsonl").read_bytes().splitlines(keepends=True)[0]
    return {"schema_version": 2, "files": {name: sha256_file(output / name) for name in PREPARATION_FILES}, "initial_state": json.loads(first), "initial_state_history_sha256": hashlib.sha256(first).hexdigest()}


def validate_prepared(output: pathlib.Path, *, require_unconsumed: bool) -> dict[str, Any]:
    manifest = read_json(output / "prepared-manifest.json")
    keys = {"schema_version", "kind", "state", "execution_requested", "candidate", "inputs", "environment", "working_directory", "output_root", "limits_seconds", "plan", "planned_inventories", "gates", "retained_limitations", "prepared_utc"}
    if set(manifest) != keys or manifest["schema_version"] != 2 or manifest["kind"] != "cartesian-frames-prepared-manifest" or manifest["state"] != "PREPARED" or manifest["execution_requested"] is not False:
        raise fail("prepared manifest schema differs")
    seal = read_json(output / "preparation-seal.json")
    if set(seal) != {"schema_version", "files", "initial_state", "initial_state_history_sha256"} or seal["schema_version"] != 2 or seal["files"] != {name: sha256_file(output / name) for name in PREPARATION_FILES}:
        raise fail("prepared bytes differ from seal")
    history = (output / "state-history.jsonl").read_bytes().splitlines()
    if not history or hashlib.sha256(history[0] + b"\n").hexdigest() != seal["initial_state_history_sha256"]:
        raise fail("prepared lifecycle history differs")
    initial, state = json.loads(history[0]), read_json(output / "state.json")
    if initial != seal["initial_state"] or initial["state"] != "PREPARED" or state != json.loads(history[-1]):
        raise fail("prepared lifecycle state differs")
    if require_unconsumed and (state["state"] != "PREPARED" or (output / "execution-claim.json").exists() or (output / "terminal-manifest.json").exists()):
        raise fail("prepared attempt is already consumed")
    return manifest


def prepare(arguments: argparse.Namespace) -> None:
    source, output = pathlib.Path(arguments.source_root).resolve(), pathlib.Path(arguments.output_root).resolve()
    if output.exists() or source == output or source in output.parents:
        raise fail("output root must be a new external directory")
    paths = input_paths(arguments, source)
    profile = validate_profile(paths["profile"])
    protocol_check(paths["protocol"])
    candidate, plan = published_candidate(source), command_plan(profile, source)
    output.mkdir(parents=True)
    shutil.copyfile(paths["profile"], output / "profile.json")
    manifest = {"schema_version": 2, "kind": "cartesian-frames-prepared-manifest", "state": "PREPARED", "execution_requested": False, "candidate": candidate, "inputs": input_identity(paths), "environment": environment_identity(), "working_directory": str(source), "output_root": str(output), "limits_seconds": {"build": BUILD_TIMEOUT_SECONDS, "process": PROCESS_TIMEOUT_SECONDS, "overall": OVERALL_TIMEOUT_SECONDS}, "plan": plan, "planned_inventories": planned_inventories(profile, candidate, plan), "gates": {gate: "NOT_EXECUTED" for gate in profile["gates"]}, "retained_limitations": profile["limitations"], "prepared_utc": utc_now()}
    write_json(output / "prepared-manifest.json", manifest)
    write_json(output / "plan.json", {"schema_version": 2, "kind": "cartesian-frames-launch-plan", "cells": plan, "gates": manifest["gates"], "limitations": manifest["retained_limitations"]})
    write_json(output / "planned-inventories.json", manifest["planned_inventories"])
    write_state(output, "PREPARED", {"candidate_commit": candidate["commit"], "execution_requested": False, "prepared_manifest_sha256": sha256_file(output / "prepared-manifest.json")})
    write_json(output / "preparation-seal.json", preparation_seal(output))
    validate_prepared(output, require_unconsumed=True)


def validate_execution_binding(arguments: argparse.Namespace, source: pathlib.Path, output: pathlib.Path, manifest: dict[str, Any]) -> dict[str, Any]:
    paths, profile = input_paths(arguments, source), validate_profile(canonical(source, "experiments/profiles/cartesian_frames.json", arguments.profile, "profile"))
    if manifest["candidate"] != published_candidate(source) or manifest["environment"] != environment_identity() or manifest["working_directory"] != str(source) or manifest["output_root"] != str(output):
        raise fail("prepared candidate or environment differs")
    verify_input_identity(manifest["inputs"], paths)
    plan = command_plan(profile, source)
    if manifest["limits_seconds"] != {"build": BUILD_TIMEOUT_SECONDS, "process": PROCESS_TIMEOUT_SECONDS, "overall": OVERALL_TIMEOUT_SECONDS} or manifest["plan"] != plan or manifest["planned_inventories"] != planned_inventories(profile, manifest["candidate"], plan) or manifest["gates"] != {gate: "NOT_EXECUTED" for gate in profile["gates"]} or manifest["retained_limitations"] != profile["limitations"] or read_json(output / "profile.json") != profile:
        raise fail("prepared manifest plan differs")
    return profile


def load_prepared(arguments: argparse.Namespace) -> tuple[pathlib.Path, dict[str, Any], dict[str, Any]]:
    source, output = pathlib.Path(arguments.source_root).resolve(), pathlib.Path(arguments.output_root).resolve()
    manifest = validate_prepared(output, require_unconsumed=True)
    return output, manifest, validate_execution_binding(arguments, source, output, manifest)


def replace_root(value: Any, output: pathlib.Path, repetition: int | None = None) -> Any:
    if isinstance(value, list):
        return [replace_root(item, output, repetition) for item in value]
    if isinstance(value, dict):
        return {key: replace_root(item, output, repetition) for key, item in value.items()}
    if isinstance(value, str):
        value = value.replace("@OUTPUT_ROOT@", str(output))
        return value.replace("@REPETITION@", str(repetition)) if repetition is not None else value
    return value


def planned_command_records(manifest: dict[str, Any], output: pathlib.Path) -> list[dict[str, Any]]:
    """Return the exact command-record provenance expected from the sealed plan."""
    planned: list[dict[str, Any]] = []

    def append(cell: str, stage: str, argv: list[str], timeout_seconds: int) -> None:
        planned.append({
            "id": f"{cell}-{stage}",
            "argv": argv,
            "cwd": manifest["working_directory"],
            "timeout_seconds": timeout_seconds,
        })

    for cell in manifest["plan"]:
        name = cell["cell"]
        for stage in ("source_check", "configure", "build"):
            append(name, stage.replace("_", "-"), replace_root(cell[stage], output), BUILD_TIMEOUT_SECONDS)
        append(name, "prerequisite-discovery", replace_root(cell["prerequisite_discovery"], output), PROCESS_TIMEOUT_SECONDS)
        for stage in ("focused_ctest", "prerequisite_ctest"):
            append(name, stage.replace("_", "-"), replace_root(cell[stage], output), BUILD_TIMEOUT_SECONDS)
        for repetition in range(1, cell["repetitions"] + 1):
            append(name, f"certificate-{repetition}", replace_root(cell["certificate"], output, repetition), PROCESS_TIMEOUT_SECONDS)
            append(name, f"certificate-validation-{repetition}", replace_root(cell["certificate_validation"], output, repetition), PROCESS_TIMEOUT_SECONDS)
        append(name, "negative-outcomes", replace_root(cell["negative_outcomes"], output), PROCESS_TIMEOUT_SECONDS)
    for cell in manifest["plan"]:
        for executable_name in cell["runtime_executables"]:
            append(cell["cell"], f"ldd-{executable_name.replace('.', '_')}", ["ldd", str(output / "cells" / cell["cell"] / "build" / executable_name)], PROCESS_TIMEOUT_SECONDS)
    return planned


def parse_timestamp(value: Any, context: str) -> dt.datetime:
    if not isinstance(value, str):
        raise fail(f"{context} timestamp differs")
    try:
        parsed = dt.datetime.fromisoformat(value)
    except ValueError as error:
        raise fail(f"{context} timestamp differs") from error
    if parsed.tzinfo is None:
        raise fail(f"{context} timestamp lacks timezone")
    return parsed


def verify_command_records(root: pathlib.Path, manifest: dict[str, Any], state: str) -> list[dict[str, Any]]:
    records_document = read_json(root / "command-records.json")
    if set(records_document) != {"schema_version", "kind", "records"} or records_document.get("schema_version") != 2 or records_document.get("kind") != "cartesian-frames-command-records" or not isinstance(records_document.get("records"), list):
        raise fail("command records differ")
    records = records_document["records"]
    expected = planned_command_records(manifest, root)
    if len(records) > len(expected) or [row.get("id") if isinstance(row, dict) else None for row in records] != [row["id"] for row in expected[:len(records)]]:
        raise fail("command record sequence differs from sealed plan")
    if state == "EXECUTED_PENDING_AUDIT" and len(records) != len(expected):
        raise fail("successful command record sequence is incomplete")
    expected_keys = {"schema_version", "kind", "id", "argv", "cwd", "environment_delta", "started_utc", "ended_utc", "elapsed_seconds", "pid", "timeout_seconds", "timed_out", "exit_code", "launch_error", "stdout", "stderr"}
    for index, (record, specification) in enumerate(zip(records, expected)):
        if not isinstance(record, dict) or set(record) != expected_keys or record.get("schema_version") != 1 or record.get("kind") != "experiment-command-record" or record.get("id") != specification["id"] or record.get("argv") != specification["argv"] or record.get("cwd") != specification["cwd"] or record.get("environment_delta") != {} or record.get("timeout_seconds") != specification["timeout_seconds"]:
            raise fail("command record provenance differs")
        started, ended = parse_timestamp(record.get("started_utc"), record["id"]), parse_timestamp(record.get("ended_utc"), record["id"])
        if started > ended or not isinstance(record.get("elapsed_seconds"), (int, float)) or isinstance(record.get("elapsed_seconds"), bool) or record["elapsed_seconds"] < 0:
            raise fail("command record timing differs")
        timed_out, launch_error = record.get("timed_out"), record.get("launch_error")
        if not isinstance(timed_out, bool):
            raise fail("command record timeout state differs")
        if launch_error is None:
            if not isinstance(record.get("pid"), int) or record["pid"] <= 0 or (timed_out and record.get("exit_code") is not None) or (not timed_out and not isinstance(record.get("exit_code"), int)):
                raise fail("command record process state differs")
        elif not isinstance(launch_error, str) or record.get("pid") is not None or record.get("exit_code") is not None or timed_out:
            raise fail("command record launch failure differs")
        for stream in ("stdout", "stderr"):
            item = record.get(stream)
            expected_path = f"logs/{record['id']}.{stream}.log"
            if not isinstance(item, dict) or set(item) != {"path", "sha256"} or item.get("path") != expected_path or not isinstance(item.get("sha256"), str) or len(item["sha256"]) != 64 or not (root / expected_path).is_file() or sha256_file(root / expected_path) != item["sha256"]:
                raise fail("command record log binding differs")
        if index < len(records) - 1:
            require_success(record, f"retained command {record['id']}")
    if state == "EXECUTED_PENDING_AUDIT" and any(record["exit_code"] != 0 or record["timed_out"] or record["launch_error"] is not None for record in records):
        raise fail("successful terminal includes failed command")
    return records


def require_success(record: dict[str, Any], context: str) -> None:
    if record["exit_code"] != 0 or record["timed_out"] or record["launch_error"] is not None:
        raise fail(f"{context} failed")


def discovered_test_names(record: dict[str, Any], output: pathlib.Path) -> list[str]:
    path = output / record["stdout"]["path"]
    if not path.is_file() or sha256_file(path) != record["stdout"]["sha256"]:
        raise fail("CTest discovery stdout differs")
    names = re.findall(r"(?m)^\s*Test\s+#\d+:\s+(.+?)\s*$", path.read_text(encoding="utf-8", errors="strict"))
    if len(names) != len(set(names)):
        raise fail("CTest discovery contains duplicates")
    return names


def verify_detached_candidate(source: pathlib.Path, candidate: dict[str, Any]) -> dict[str, Any]:
    with tempfile.TemporaryDirectory(prefix="apmesh-core-cf-detached-") as temporary:
        detached = pathlib.Path(temporary) / "candidate"
        added = subprocess.run(["git", "worktree", "add", "--detach", str(detached), candidate["commit"]], cwd=source, capture_output=True, text=True, check=False)
        if added.returncode != 0:
            raise fail("detached candidate worktree could not be created")
        try:
            observed = clean_candidate(detached)
            expected = [{"path": row["path"], "sha256": row["sha256"]} for row in candidate["source_inventory"]]
            actual = [{"path": row["path"], "sha256": row["sha256"]} for row in observed["source_inventory"]]
            if observed["commit"] != candidate["commit"] or actual != expected:
                raise fail("detached candidate identity differs")
            return {"result": "PASS", "candidate_commit": candidate["commit"], "source_inventory_count": len(actual)}
        finally:
            removed = subprocess.run(["git", "worktree", "remove", "--force", str(detached)], cwd=source, capture_output=True, text=True, check=False)
            if removed.returncode != 0:
                raise fail("detached candidate worktree could not be removed")


def write_records(output: pathlib.Path, records: list[dict[str, Any]]) -> None:
    write_json(output / "command-records.json", {"schema_version": 2, "kind": "cartesian-frames-command-records", "records": records})


def write_terminal(output: pathlib.Path, manifest: dict[str, Any], state: str, extra: dict[str, Any]) -> None:
    write_json(output / "terminal-manifest.json", {"schema_version": 2, "kind": "cartesian-frames-terminal-manifest", "state": state, "candidate": manifest["candidate"], "prepared_manifest_sha256": sha256_file(output / "prepared-manifest.json"), "command_records": "command-records.json", "observed_inventories": "observed-inventories.json", "retention_manifest": "retention-manifest.json", **extra})


def write_retention_inventory(output: pathlib.Path, manifest: dict[str, Any], required_paths: set[str]) -> None:
    files = sorted(path for path in output.rglob("*") if path.is_file() and path.name != "retention-manifest.json" and "/build/" not in path.relative_to(output).as_posix())
    write_json(output / "retention-manifest.json", {"schema_version": 2, "kind": "cartesian-frames-retention", "candidate_commit": manifest["candidate"]["commit"], "prepared_manifest_sha256": sha256_file(output / "prepared-manifest.json"), "required_paths": sorted(required_paths), "files": [{"path": relative_path(output, path), "sha256": sha256_file(path), "size": path.stat().st_size} for path in files]})


def validate_success_terminal_references(terminal: dict[str, Any]) -> None:
    if any(terminal.get(key) != value for key, value in SUCCESS_TERMINAL_REFERENCES.items()):
        raise fail("successful terminal references differ")


def verify_retention(root: pathlib.Path) -> dict[str, Any]:
    manifest = validate_prepared(root, require_unconsumed=False)
    terminal, retention = read_json(root / "terminal-manifest.json"), read_json(root / "retention-manifest.json")
    terminal_base = {"schema_version", "kind", "state", "candidate", "prepared_manifest_sha256", "command_records", "observed_inventories", "retention_manifest"}
    terminal_success = terminal_base | {"certificate_index", "per_cell_comparison", "comparison", "gate_summary", "gate_summary_markdown", "prerequisite_discovery"}
    terminal_failure = terminal_base | {"failure"}
    if (terminal.get("state") == "EXECUTED_PENDING_AUDIT" and set(terminal) != terminal_success) or (terminal.get("state") == "BLOCKED" and not set(terminal).issubset(terminal_failure | {"retention_failure"})) or terminal.get("kind") != "cartesian-frames-terminal-manifest" or terminal.get("schema_version") != 2 or terminal.get("candidate") != manifest["candidate"] or terminal.get("prepared_manifest_sha256") != sha256_file(root / "prepared-manifest.json") or terminal.get("state") not in {"EXECUTED_PENDING_AUDIT", "BLOCKED"} or terminal.get("command_records") != "command-records.json" or terminal.get("retention_manifest") != "retention-manifest.json" or read_json(root / "state.json").get("state") != terminal.get("state"):
        raise fail("terminal manifest differs")
    if terminal["state"] == "EXECUTED_PENDING_AUDIT":
        validate_success_terminal_references(terminal)
    claim = read_json(root / "execution-claim.json")
    if set(claim) != {"schema_version", "kind", "candidate_commit", "prepared_manifest_sha256", "pid", "started_utc"} or claim.get("schema_version") != 1 or claim.get("kind") != "cartesian-frames-execution-claim" or claim.get("candidate_commit") != manifest["candidate"]["commit"] or claim.get("prepared_manifest_sha256") != sha256_file(root / "prepared-manifest.json") or not isinstance(claim.get("pid"), int) or claim["pid"] <= 0:
        raise fail("execution claim differs")
    parse_timestamp(claim.get("started_utc"), "execution claim")
    expected = set(SUCCESS_FILES if terminal["state"] == "EXECUTED_PENDING_AUDIT" else FAILURE_FILES)
    sealing_failed = terminal.get("retention_failure") is not None
    if sealing_failed:
        if terminal["state"] != "BLOCKED" or terminal["retention_failure"] != "retention-error.json":
            raise fail("retention failure terminal differs")
        expected |= SEAL_FAILURE_FILES
    if set(retention) != {"schema_version", "kind", "candidate_commit", "prepared_manifest_sha256", "required_paths", "files"} or retention.get("schema_version") != 2 or retention.get("kind") != "cartesian-frames-retention" or retention.get("candidate_commit") != manifest["candidate"]["commit"] or retention.get("prepared_manifest_sha256") != sha256_file(root / "prepared-manifest.json") or not isinstance(retention.get("files"), list) or not isinstance(retention.get("required_paths"), list) or set(retention["required_paths"]) != expected:
        raise fail("retention manifest differs")
    listed = {"retention-manifest.json"}
    for entry in retention.get("files", []):
        path = (root / entry.get("path", "")).resolve()
        if not isinstance(entry, dict) or set(entry) != {"path", "sha256", "size"} or root.resolve() not in path.parents or entry["path"] in listed or not path.is_file() or path.stat().st_size != entry["size"] or sha256_file(path) != entry["sha256"]:
            raise fail("retention file differs")
        listed.add(entry["path"])
    actual = {relative_path(root, path) for path in root.rglob("*") if path.is_file() and "/build/" not in path.relative_to(root).as_posix()}
    if actual != listed:
        raise fail("retention file set differs")
    records = verify_command_records(root, manifest, terminal["state"])
    record_by_id = {record["id"]: record for record in records}
    detached = read_json(root / "detached-verification.json")
    if sealing_failed:
        error = read_json(root / "retention-error.json")
        if set(error) != {"schema_version", "kind", "candidate_commit", "stage", "error_type", "message", "retry_attempted", "snapshots"} or error.get("schema_version") != 1 or error.get("kind") != "cartesian-frames-retention-failure" or error.get("candidate_commit") != manifest["candidate"]["commit"] or error.get("stage") not in {"detached-verification", "inventory", "verification"} or not isinstance(error.get("error_type"), str) or not error["error_type"] or not isinstance(error.get("message"), str) or not error["message"] or error.get("retry_attempted") is not False or not isinstance(error.get("snapshots"), dict):
            raise fail("retention failure evidence differs")
        for name, digest in error["snapshots"].items():
            if name not in {"pre-seal-terminal.json", "pre-seal-detached.json", "pre-seal-retention.json"} or name not in listed or not isinstance(digest, str) or sha256_file(root / name) != digest:
                raise fail("retention failure snapshot differs")
        if detached != {"schema_version": 1, "kind": "cartesian-frames-detached-verification", "result": "BLOCKED", "candidate_commit": manifest["candidate"]["commit"], "retention_failure": "retention-error.json"}:
            raise fail("sealed failure detached verification differs")
    else:
        observed = verify_detached_candidate(pathlib.Path(manifest["candidate"]["source_root"]), manifest["candidate"])
        if detached != {"schema_version": 1, "kind": "cartesian-frames-detached-verification", **observed}:
            raise fail("detached retention verification differs")
    conditions = {"always", "success" if terminal["state"] == "EXECUTED_PENDING_AUDIT" else "failure"}
    if sealing_failed:
        conditions.add("seal-failure")
    required_artifacts = {row["path"] for row in manifest["planned_inventories"]["artifacts"] if row["required_when"] in conditions}
    if not required_artifacts.issubset(listed):
        raise fail("planned artifacts are incomplete")
    if terminal["state"] == "BLOCKED":
        failure = read_json(root / "failure.json")
        if set(failure) != {"schema_version", "kind", "message", "records", "partial_certificate_slots"} or failure.get("schema_version") != 1 or failure.get("kind") != "cartesian-frames-failure" or not isinstance(failure.get("message"), str) or not failure["message"] or failure.get("records") != records or not isinstance(failure.get("partial_certificate_slots"), list) or terminal.get("failure") != "failure.json":
            raise fail("terminal failure evidence differs")
    else:
        profile = validate_profile(root / "profile.json")
        if sha256_file(root / "profile.json") != manifest["inputs"]["profile"]["sha256"]:
            raise fail("retained profile identity differs")
        validate_per_cell_comparisons(profile, root / "certificate-index.json", root / "per-cell-comparisons.json")
        if read_json(root / "cross-cell-comparison.json") != compare_certificates(profile, root / "certificate-index.json"):
            raise fail("cross-cell comparison recomputation differs")
        source_checks = read_json(root / "source-checks.json")
        expected_cells = [cell["cell"] for cell in manifest["plan"]]
        if set(source_checks) != {"schema_version", "kind", "cells"} or source_checks.get("schema_version") != 1 or source_checks.get("kind") != "cartesian-frames-source-checks" or not isinstance(source_checks.get("cells"), list) or [item.get("cell") if isinstance(item, dict) else None for item in source_checks["cells"]] != expected_cells:
            raise fail("retained source check index differs")
        discoveries = read_json(root / "prerequisite-discovery.json")
        if set(discoveries) != {"schema_version", "kind", "cells"} or discoveries.get("schema_version") != 1 or discoveries.get("kind") != "cartesian-frames-prerequisite-discovery" or not isinstance(discoveries.get("cells"), list) or [item.get("cell") if isinstance(item, dict) else None for item in discoveries["cells"]] != expected_cells:
            raise fail("retained prerequisite discovery differs")
        inventories = read_json(root / "observed-inventories.json")
        if set(inventories) != {"schema_version", "kind", "candidate_commit", "cells"} or inventories.get("schema_version") != 1 or inventories.get("kind") != "cartesian-frames-observed-inventories" or inventories.get("candidate_commit") != manifest["candidate"]["commit"] or not isinstance(inventories.get("cells"), list) or [item.get("cell") if isinstance(item, dict) else None for item in inventories["cells"]] != expected_cells:
            raise fail("retained observed inventory differs")
        for cell, source_entry, discovery, inventory in zip(manifest["plan"], source_checks["cells"], discoveries["cells"], inventories["cells"]):
            name = cell["cell"]
            source_path = f"cells/{name}/source-check.json"
            if set(source_entry) != {"cell", "path", "sha256"} or source_entry.get("path") != source_path or source_entry.get("sha256") != sha256_file(root / source_path) or read_json(root / source_path) != validate_source(pathlib.Path(manifest["working_directory"])):
                raise fail("retained source check differs")
            discovery_id = f"{name}-prerequisite-discovery"
            if set(discovery) != {"cell", "record_id", "discovered_allowlist"} or discovery.get("record_id") != discovery_id or discovery_id not in record_by_id or not isinstance(discovery.get("discovered_allowlist"), list):
                raise fail("retained prerequisite discovery binding differs")
            require_success(record_by_id[discovery_id], f"retained prerequisite discovery {name}")
            discovered = discovered_test_names(record_by_id[discovery_id], root)
            expected_allowlist = [item for item in discovered if item in profile["exact_prerequisite_tests"]]
            if discovery["discovered_allowlist"] != expected_allowlist:
                raise fail("retained prerequisite discovery result differs")
            validate_prerequisite_discovery(expected_allowlist, profile["exact_prerequisite_tests"])
            prerequisite_id = f"{name}-prerequisite-ctest"
            if prerequisite_id not in record_by_id:
                raise fail("retained prerequisite execution is absent")
            require_success(record_by_id[prerequisite_id], f"retained prerequisite execution {name}")
            if set(inventory) != {"cell", "compile_commands", "runtime_dependencies"} or set(inventory.get("compile_commands", {})) != {"path", "sha256"} or inventory["compile_commands"].get("path") != f"cells/{name}/compile_commands.json" or inventory["compile_commands"].get("sha256") != sha256_file(root / inventory["compile_commands"]["path"]):
                raise fail("retained compile inventory differs")
            compile_commands = read_compile_commands(root / inventory["compile_commands"]["path"])
            if not isinstance(compile_commands, list) or not compile_commands:
                raise fail("retained compile commands differ")
            compiler = cell["compiler"]
            if any(not isinstance(item, dict) or not isinstance(item.get("file"), str) or not isinstance(item.get("command"), str) or compiler not in item["command"] or not pathlib.Path(item["file"]).resolve().is_relative_to(pathlib.Path(manifest["working_directory"]).resolve()) for item in compile_commands):
                raise fail("retained compile command provenance differs")
            expected_dependencies = cell["runtime_executables"]
            dependencies = inventory.get("runtime_dependencies")
            if not isinstance(dependencies, list) or [item.get("executable") if isinstance(item, dict) else None for item in dependencies] != expected_dependencies:
                raise fail("retained runtime inventory differs")
            for dependency in dependencies:
                executable = dependency["executable"]
                record_id = f"{name}-ldd-{executable.replace('.', '_')}"
                expected_path = f"cells/{name}/build/{executable}"
                if set(dependency) != {"executable", "path", "sha256", "ldd_record_id", "ldd_stdout_sha256"} or dependency.get("path") != expected_path or dependency.get("ldd_record_id") != record_id or not isinstance(dependency.get("sha256"), str) or len(dependency["sha256"]) != 64 or record_id not in record_by_id:
                    raise fail("retained runtime dependency binding differs")
                record = record_by_id[record_id]
                require_success(record, f"retained runtime dependency {record_id}")
                if record["stdout"]["sha256"] != dependency.get("ldd_stdout_sha256") or "not found" in ((root / record["stdout"]["path"]).read_text(encoding="utf-8", errors="strict") + (root / record["stderr"]["path"]).read_text(encoding="utf-8", errors="strict")):
                    raise fail("retained runtime dependency result differs")
            negative_path = root / "negatives" / f"{name}.json"
            negative_id = f"{name}-negative-outcomes"
            if negative_id not in record_by_id:
                raise fail("retained negative outcome command is absent")
            require_success(record_by_id[negative_id], f"retained negative outcomes {name}")
            validate_negative_outcomes(profile, negative_path)
        expected_summary = gate_summary(profile, manifest["retained_limitations"])
        if read_json(root / "gate-summary.json") != expected_summary:
            raise fail("retained gate summary differs")
        validate_gate_summary_markdown(expected_summary, root / "gate-summary.md")
    return {"schema_version": 1, "kind": "cartesian-frames-retention-verification", "status": "PASS", "files": sorted(listed)}


def seal_output(output: pathlib.Path, manifest: dict[str, Any], required_paths: set[str]) -> None:
    stage = "detached-verification"
    try:
        detached = verify_detached_candidate(pathlib.Path(manifest["candidate"]["source_root"]), manifest["candidate"])
        write_json(output / "detached-verification.json", {"schema_version": 1, "kind": "cartesian-frames-detached-verification", **detached})
        stage = "inventory"
        write_retention_inventory(output, manifest, required_paths)
        stage = "verification"
        verify_retention(output)
    except Exception as error:
        snapshots = {}
        for original, snapshot in (("terminal-manifest.json", "pre-seal-terminal.json"), ("detached-verification.json", "pre-seal-detached.json"), ("retention-manifest.json", "pre-seal-retention.json")):
            path = output / original
            if path.is_file():
                (output / snapshot).write_bytes(path.read_bytes())
                snapshots[snapshot] = sha256_file(output / snapshot)
        if not (output / "failure.json").exists():
            write_json(output / "failure.json", {"schema_version": 1, "kind": "cartesian-frames-failure", "message": str(error), "records": read_json(output / "command-records.json")["records"], "partial_certificate_slots": []})
        write_json(output / "retention-error.json", {"schema_version": 1, "kind": "cartesian-frames-retention-failure", "candidate_commit": manifest["candidate"]["commit"], "stage": stage, "error_type": type(error).__name__, "message": str(error), "retry_attempted": False, "snapshots": snapshots})
        write_terminal(output, manifest, "BLOCKED", {"failure": "failure.json", "retention_failure": "retention-error.json"})
        if read_json(output / "state.json")["state"] != "BLOCKED":
            write_state(output, "BLOCKED", {"candidate_commit": manifest["candidate"]["commit"], "closure_failure": True, "failure": "failure.json"})
        write_json(output / "detached-verification.json", {"schema_version": 1, "kind": "cartesian-frames-detached-verification", "result": "BLOCKED", "candidate_commit": manifest["candidate"]["commit"], "retention_failure": "retention-error.json"})
        write_retention_inventory(output, manifest, FAILURE_FILES | SEAL_FAILURE_FILES)
        raise fail(f"retention sealing failed at {stage}: {error}") from error


def execute(arguments: argparse.Namespace) -> int:
    source, output = pathlib.Path(arguments.source_root).resolve(), pathlib.Path(arguments.output_root).resolve()
    manifest = validate_prepared(output, require_unconsumed=True)
    with (output / "execution-claim.json").open("x", encoding="utf-8") as stream:
        json.dump({"schema_version": 1, "kind": "cartesian-frames-execution-claim", "candidate_commit": manifest["candidate"]["commit"], "prepared_manifest_sha256": sha256_file(output / "prepared-manifest.json"), "pid": os.getpid(), "started_utc": utc_now()}, stream, sort_keys=True)
    records, entries, discoveries = [], [], []
    deadline = time.monotonic() + OVERALL_TIMEOUT_SECONDS
    try:
        write_state(output, "RUNNING", {"candidate_commit": manifest["candidate"]["commit"], "prepared_manifest_sha256": sha256_file(output / "prepared-manifest.json")})
        profile = validate_execution_binding(arguments, source, output, manifest)
        for cell in manifest["plan"]:
            for stage in ("source_check", "configure", "build"):
                if time.monotonic() >= deadline:
                    raise fail("overall qualification timeout")
                record = run_command(replace_root(cell[stage], output), source, output / "logs", f"{cell['cell']}-{stage.replace('_', '-')}", BUILD_TIMEOUT_SECONDS)
                records.append(record); require_success(record, f"{cell['cell']} {stage}")
            source_check = output / "cells" / cell["cell"] / "source-check.json"
            if read_json(source_check) != validate_source(source):
                raise fail(f"source check differs: {cell['cell']}")
            discovery = run_command(replace_root(cell["prerequisite_discovery"], output), source, output / "logs", f"{cell['cell']}-prerequisite-discovery", PROCESS_TIMEOUT_SECONDS)
            records.append(discovery); require_success(discovery, f"{cell['cell']} prerequisite discovery")
            names = discovered_test_names(discovery, output)
            allowlist = [name for name in names if name in profile["exact_prerequisite_tests"]]
            validate_prerequisite_discovery(allowlist, profile["exact_prerequisite_tests"])
            discoveries.append({"cell": cell["cell"], "record_id": discovery["id"], "discovered_allowlist": allowlist})
            for stage in ("focused_ctest", "prerequisite_ctest"):
                record = run_command(replace_root(cell[stage], output), source, output / "logs", f"{cell['cell']}-{stage.replace('_', '-')}", BUILD_TIMEOUT_SECONDS)
                records.append(record); require_success(record, f"{cell['cell']} {stage}")
            for repetition in range(1, cell["repetitions"] + 1):
                certificate = output / "certificates" / f"{cell['cell']}-{repetition}.json"; certificate.parent.mkdir(parents=True, exist_ok=True)
                record = run_command(replace_root(cell["certificate"], output, repetition), source, output / "logs", f"{cell['cell']}-certificate-{repetition}", PROCESS_TIMEOUT_SECONDS)
                records.append(record); require_success(record, f"{cell['cell']} certificate {repetition}")
                record = run_command(replace_root(cell["certificate_validation"], output, repetition), source, output / "logs", f"{cell['cell']}-certificate-validation-{repetition}", PROCESS_TIMEOUT_SECONDS)
                records.append(record); require_success(record, f"{cell['cell']} certificate validation {repetition}")
                validate_certificate(profile, certificate)
                entries.append({"cell": cell["cell"], "repetition": repetition, "path": relative_path(output, certificate), "sha256": sha256_file(certificate)})
            record = run_command(replace_root(cell["negative_outcomes"], output), source, output / "logs", f"{cell['cell']}-negative-outcomes", PROCESS_TIMEOUT_SECONDS)
            records.append(record); require_success(record, f"{cell['cell']} negative outcomes")
        inventories = []
        for cell in manifest["plan"]:
            build = output / "cells" / cell["cell"] / "build"; compiled = build / "compile_commands.json"
            if not compiled.is_file():
                raise fail(f"compile commands are absent: {cell['cell']}")
            retained = output / "cells" / cell["cell"] / "compile_commands.json"; shutil.copyfile(compiled, retained)
            dependencies = []
            for executable_name in cell["runtime_executables"]:
                executable = build / executable_name
                record = run_command(["ldd", str(executable)], source, output / "logs", f"{cell['cell']}-ldd-{executable_name.replace('.', '_')}", PROCESS_TIMEOUT_SECONDS)
                records.append(record); require_success(record, f"{cell['cell']} ldd {executable_name}")
                if "not found" in ((output / record["stdout"]["path"]).read_text(encoding="utf-8") + (output / record["stderr"]["path"]).read_text(encoding="utf-8")):
                    raise fail(f"unresolved runtime dependency: {cell['cell']}:{executable_name}")
                dependencies.append({"executable": executable_name, "path": relative_path(output, executable), "sha256": sha256_file(executable), "ldd_record_id": record["id"], "ldd_stdout_sha256": record["stdout"]["sha256"]})
            inventories.append({"cell": cell["cell"], "compile_commands": {"path": relative_path(output, retained), "sha256": sha256_file(retained)}, "runtime_dependencies": dependencies})
        write_records(output, records)
        write_json(output / "source-checks.json", {"schema_version": 1, "kind": "cartesian-frames-source-checks", "cells": [{"cell": cell["cell"], "path": f"cells/{cell['cell']}/source-check.json", "sha256": sha256_file(output / "cells" / cell["cell"] / "source-check.json")} for cell in manifest["plan"]]})
        write_json(output / "prerequisite-discovery.json", {"schema_version": 1, "kind": "cartesian-frames-prerequisite-discovery", "cells": discoveries})
        write_json(output / "observed-inventories.json", {"schema_version": 1, "kind": "cartesian-frames-observed-inventories", "candidate_commit": manifest["candidate"]["commit"], "cells": inventories})
        write_json(output / "certificate-index.json", {"schema_version": 1, "kind": "cartesian-frames-certificate-index", "entries": entries})
        write_json(output / "per-cell-comparisons.json", compare_per_cell_certificates(profile, output / "certificate-index.json"))
        write_json(output / "cross-cell-comparison.json", compare_certificates(profile, output / "certificate-index.json"))
        summary = gate_summary(profile, manifest["retained_limitations"])
        write_json(output / "gate-summary.json", summary)
        (output / "gate-summary.md").write_text(render_gate_summary_markdown(summary), encoding="utf-8")
        write_terminal(output, manifest, "EXECUTED_PENDING_AUDIT", SUCCESS_TERMINAL_REFERENCES)
        write_state(output, "EXECUTED_PENDING_AUDIT", {"candidate_commit": manifest["candidate"]["commit"], "comparison": "cross-cell-comparison.json"})
        seal_output(output, manifest, SUCCESS_FILES)
        return 0
    except Exception as error:
        if (output / "state.json").is_file() and read_json(output / "state.json").get("state") == "BLOCKED":
            # seal_output already retained the terminal failure; another seal would be a hidden retry.
            return 1
        write_records(output, records)
        write_json(output / "failure.json", {"schema_version": 1, "kind": "cartesian-frames-failure", "message": str(error), "records": records, "partial_certificate_slots": entries})
        write_terminal(output, manifest, "BLOCKED", {"failure": "failure.json"})
        write_state(output, "BLOCKED", {"candidate_commit": manifest["candidate"]["commit"], "closure_failure": True, "failure": "failure.json"})
        seal_output(output, manifest, FAILURE_FILES)
        return 1


def self_check(arguments: argparse.Namespace) -> dict[str, Any]:
    source = pathlib.Path(arguments.source_root).resolve(); paths = input_paths(arguments, source); profile = validate_profile(paths["profile"]); protocol_check(paths["protocol"]); plan = command_plan(profile, source)
    if len(plan) != 4:
        raise fail("fixed qualification matrix differs")
    validate_prerequisite_discovery(list(profile["exact_prerequisite_tests"]), profile["exact_prerequisite_tests"])
    return {"schema_version": 3, "kind": "cartesian-frames-runner-self-check", "status": "PASS", "execution_requested": False, "input_hashes": input_identity(paths), "source_checks": validate_source(source), "plan": {"schema_version": 2, "kind": "cartesian-frames-launch-plan", "execution_requested": False, "cells": plan, "gates": {gate: "NOT_EXECUTED" for gate in profile["gates"]}, "limitations": profile["limitations"]}}


def main() -> int:
    parser = argparse.ArgumentParser()
    for name in ("source-root", "profile", "protocol", "exporter", "validator"):
        parser.add_argument(f"--{name}", required=True)
    commands = parser.add_subparsers(dest="command", required=True)
    for name in ("self-check", "plan"):
        commands.add_parser(name).add_argument("--output", required=True)
    for name in ("prepare", "execute", "verify-retention"):
        commands.add_parser(name).add_argument("--output-root", required=True)
    arguments = parser.parse_args()
    try:
        if arguments.command == "prepare":
            prepare(arguments); return 0
        if arguments.command == "execute":
            return execute(arguments)
        if arguments.command == "verify-retention":
            print(json.dumps(verify_retention(pathlib.Path(arguments.output_root).resolve()), sort_keys=True)); return 0
        result = self_check(arguments)
        write_json(pathlib.Path(arguments.output), result if arguments.command == "self-check" else result["plan"]); return 0
    except (RunnerError, RuntimeErrorEvidence, EvidenceError) as error:
        print(f"ERROR: {error}", file=sys.stderr); return 2


if __name__ == "__main__":
    raise SystemExit(main())
