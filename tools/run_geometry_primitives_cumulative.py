#!/usr/bin/env python3
"""Prepare, execute once, and retain immutable GPR0-GPR7 evidence."""
from __future__ import annotations

import argparse
import hashlib
import json
import os
import pathlib
import re
import shutil
import subprocess
import sys
import tempfile
from typing import Any

TOOL_ROOT = pathlib.Path(__file__).resolve().parent
if str(TOOL_ROOT) not in sys.path:
    sys.path.insert(0, str(TOOL_ROOT))

from experiment_runtime import (RuntimeErrorEvidence, clean_candidate, input_identity,
                                read_json, relative_path, run_command, sha256_file,
                                tool_version, utc_now, verify_input_identity,
                                write_json, write_state)
from geometry_primitives_cumulative_evidence import (EvidenceError, compare,
                                                      validate_certificate,
                                                      validate_negative_outcomes,
                                                      validate_profile)

POST_MERGE_BASELINE = "ca20ad64cd0dd14b869225dae73401ad2b93464c"
BUILD_TIMEOUT_SECONDS, PROCESS_TIMEOUT_SECONDS, OVERALL_TIMEOUT_SECONDS = 300, 45, 1800
PREPARATION_FILES = ("profile.json", "prepared-manifest.json", "plan.json", "planned-inventories.json")
INPUT_PATHS = {"profile": "experiments/profiles/geometry_primitives_cumulative.json", "protocol": "docs/decisions/GEOMETRY_PRIMITIVES_CUMULATIVE_REGRESSION_PROTOCOL.md", "exporter": "experiments/geometry_primitives_cumulative_export.cpp", "validator": "tools/geometry_primitives_cumulative_evidence.py", "runner": "tools/run_geometry_primitives_cumulative.py", "runtime": "tools/experiment_runtime.py", "cmake": "CMakeLists.txt"}
SUCCESS_FILES = {"profile.json", "prepared-manifest.json", "plan.json", "planned-inventories.json", "preparation-seal.json", "state.json", "state-history.jsonl", "execution-claim.json", "command-records.json", "certificate-index.json", "per-cell-comparisons.json", "cross-cell-comparison.json", "observed-inventories.json", "gate-summary.json", "gate-summary.md", "terminal-manifest.json", "detached-verification.json", "retention-manifest.json"}
FAILURE_FILES = {"profile.json", "prepared-manifest.json", "plan.json", "planned-inventories.json", "preparation-seal.json", "state.json", "state-history.jsonl", "execution-claim.json", "command-records.json", "failure.json", "terminal-manifest.json", "detached-verification.json", "retention-manifest.json"}


class RunnerError(RuntimeError):
    pass


def fail(message: str) -> RunnerError:
    return RunnerError(message)


def canonical(source: pathlib.Path, relative: str, supplied: str, label: str) -> pathlib.Path:
    expected, actual = (source / relative).resolve(), pathlib.Path(supplied).resolve()
    if expected != actual:
        raise fail(f"{label} must resolve to the candidate path")
    return expected


def input_paths(arguments: argparse.Namespace, source: pathlib.Path) -> dict[str, pathlib.Path]:
    supplied = {"profile": arguments.profile, "protocol": arguments.protocol, "exporter": arguments.exporter, "validator": arguments.validator}
    if pathlib.Path(__file__).resolve() != (source / INPUT_PATHS["runner"]).resolve():
        raise fail("runner must execute from the candidate path")
    paths = {name: canonical(source, INPUT_PATHS[name], supplied[name], name) for name in supplied}
    paths.update({name: (source / relative).resolve() for name, relative in INPUT_PATHS.items() if name not in paths})
    if any(not path.is_file() for path in paths.values()):
        raise fail("a required candidate input is absent")
    return paths


def published_candidate(source: pathlib.Path) -> dict[str, Any]:
    try:
        candidate = clean_candidate(source)
    except RuntimeErrorEvidence as error:
        raise fail(str(error)) from error
    upstream = subprocess.run(["git", "rev-parse", "@{upstream}"], cwd=source, capture_output=True, text=True, check=False)
    if upstream.returncode != 0 or upstream.stdout.strip() != candidate["commit"]:
        raise fail("candidate upstream is absent or differs from HEAD")
    ancestor = subprocess.run(["git", "merge-base", "--is-ancestor", POST_MERGE_BASELINE, candidate["commit"]], cwd=source, capture_output=True, text=True, check=False)
    if ancestor.returncode != 0:
        raise fail("candidate is not a descendant of the post-merge baseline")
    candidate.update(upstream_commit=upstream.stdout.strip(), post_merge_baseline=POST_MERGE_BASELINE)
    return candidate


def tool_identity(command: str) -> dict[str, str]:
    resolved = shutil.which(command)
    if resolved is None:
        raise fail(f"required tool is unavailable: {command}")
    invoked, executable = pathlib.Path(resolved).absolute(), pathlib.Path(resolved).resolve()
    if not executable.is_file():
        raise fail(f"required tool is not a regular file: {command}")
    try:
        version = tool_version(command)["version"]
    except RuntimeErrorEvidence as error:
        raise fail(str(error)) from error
    return {"command": command, "invocation_path": str(invoked), "resolved_path": str(executable), "sha256": sha256_file(executable), "version": version}


def environment_identity() -> dict[str, Any]:
    return {"python": tool_identity(sys.executable), "cmake": tool_identity("cmake"), "ctest": tool_identity("ctest"), "ninja": tool_identity("ninja"), "gcc": tool_identity("g++-13"), "clang": tool_identity("clang++-18"), "ldd": tool_identity("ldd"), "observations": {name: os.environ.get(name, "") for name in ("LANG", "LC_ALL", "TZ")}}


def declared_allowlist(source: pathlib.Path, expected: list[str]) -> list[str]:
    names = re.findall(r"add_test\s*\(\s*NAME\s+([^\s)]+)", (source / "CMakeLists.txt").read_text(encoding="utf-8"))
    selected = [name for name in names if name in expected]
    if sorted(selected) != sorted(expected) or len(selected) != len(set(selected)):
        raise fail("declared semantic CTest allowlist differs")
    return selected


def protocol_check(path: pathlib.Path) -> None:
    content = path.read_text(encoding="utf-8")
    required = ("## 9. GPR0", "execution_requested=false", "detached-worktree verification", "## 13. Next bounded action")
    if any(token not in content for token in required):
        raise fail("protocol is not the pre-registered GPR0-GPR7 authority")


def ctest_regex(names: list[str]) -> str:
    return "^(?:" + "|".join(re.escape(name) for name in names) + ")$"


def command_plan(profile: dict[str, Any], source: pathlib.Path, declared: list[str]) -> list[dict[str, Any]]:
    targets = ["apmesh_core", "apmesh_core_bootstrap_smoke", "apmesh_core_numeric_contract", "apmesh_core.geometry_primitives", "apmesh_core.minimal_small_linear_algebra", "apmesh_core.math_header_isolation", "apmesh_core.cartesian_frames", "apmesh_core_geometry_primitives_cumulative_export"]
    plan = []
    for cell in profile["cells"]:
        compiler, libcxx = ("g++-13", False) if cell["id"].startswith("gcc-") else ("clang++-18", True)
        build = f"@OUTPUT_ROOT@/cells/{cell['id']}/build"
        plan.append({"cell": cell["id"], "compiler": compiler, "library": cell["library"], "build_type": cell["build_type"], "repetitions": profile["repetitions_per_cell"], "configure": ["cmake", "-S", str(source), "-B", build, "-G", "Ninja", f"-DCMAKE_CXX_COMPILER={compiler}", f"-DCMAKE_BUILD_TYPE={cell['build_type']}", f"-DAPMESH_USE_LIBCXX={'ON' if libcxx else 'OFF'}", "-DBUILD_TESTING=ON", "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"], "build": ["cmake", "--build", build, "--target", *targets], "ctest_discovery": ["ctest", "--test-dir", build, "-N"], "semantic_ctest": ["ctest", "--test-dir", build, "--output-on-failure", "-R", ctest_regex(declared)], "certificate": [f"{build}/apmesh_core_geometry_primitives_cumulative_export", "certificate", f"@OUTPUT_ROOT@/certificates/{cell['id']}-@REPETITION@.json"], "certificate_validation": [sys.executable, str(source / INPUT_PATHS["validator"]), "--profile", str(source / INPUT_PATHS["profile"]), "validate-certificate", "--certificate", f"@OUTPUT_ROOT@/certificates/{cell['id']}-@REPETITION@.json"], "negative_outcomes": [sys.executable, str(source / INPUT_PATHS["validator"]), "--profile", str(source / INPUT_PATHS["profile"]), "negative-outcomes", "--output", f"@OUTPUT_ROOT@/negatives/{cell['id']}.json"], "dependency_inventory": ["ninja", "-C", build, "-t", "deps"], "runtime_dependencies": [f"{build}/apmesh_core_geometry_primitives_cumulative_export"]})
    return plan


def planned_inventories(profile: dict[str, Any], candidate: dict[str, Any], plan: list[dict[str, Any]]) -> dict[str, Any]:
    artifacts = [{"path": name, "role": "preparation-control", "required_when": "always"} for name in (*PREPARATION_FILES, "preparation-seal.json", "state.json", "state-history.jsonl", "execution-claim.json", "command-records.json", "terminal-manifest.json", "detached-verification.json", "retention-manifest.json")]
    artifacts += [{"path": name, "role": "terminal-success", "required_when": "success"} for name in ("certificate-index.json", "per-cell-comparisons.json", "cross-cell-comparison.json", "observed-inventories.json", "gate-summary.json", "gate-summary.md")]
    artifacts += [{"path": "failure.json", "role": "terminal-failure", "required_when": "failure"}]
    for cell in plan:
        for stage in ("configure", "build", "ctest-discovery", "semantic-ctest", "negative-outcomes", "dependency-inventory", "ldd-exporter"):
            artifacts += [{"path": f"logs/{cell['cell']}-{stage}.{stream}.log", "role": "command-log", "required_when": "executed"} for stream in ("stdout", "stderr")]
        for item in range(1, cell["repetitions"] + 1):
            artifacts += [{"path": f"certificates/{cell['cell']}-{item}.json", "role": "semantic-certificate", "required_when": "success"}]
            artifacts += [{"path": f"logs/{cell['cell']}-certificate-{item}.{stream}.log", "role": "command-log", "required_when": "executed"} for stream in ("stdout", "stderr")]
            artifacts += [{"path": f"logs/{cell['cell']}-certificate-validation-{item}.{stream}.log", "role": "command-log", "required_when": "executed"} for stream in ("stdout", "stderr")]
        artifacts += [{"path": f"cells/{cell['cell']}/compile_commands.json", "role": "compile-command-inventory", "required_when": "success"}, {"path": f"negatives/{cell['cell']}.json", "role": "negative-outcomes", "required_when": "success"}]
    return {"schema_version": 1, "kind": "geometry-primitives-cumulative-planned-inventories", "candidate_source_inventory": candidate["source_inventory"], "cells": [{"cell": cell["cell"], "repetitions": cell["repetitions"]} for cell in plan], "artifacts": artifacts}


def preparation_seal(output: pathlib.Path) -> dict[str, Any]:
    initial = (output / "state-history.jsonl").read_bytes().splitlines(keepends=True)[0]
    return {"schema_version": 1, "files": {name: sha256_file(output / name) for name in PREPARATION_FILES}, "state_history_sha256": hashlib.sha256(initial).hexdigest()}


def validate_prepared(output: pathlib.Path, *, require_unconsumed: bool = True) -> dict[str, Any]:
    manifest = read_json(output / "prepared-manifest.json")
    keys = {"schema_version", "kind", "state", "execution_requested", "candidate", "inputs", "environment", "working_directory", "output_root", "limits_seconds", "declared_semantic_ctest_allowlist", "plan", "planned_inventories", "gates", "retained_limitations", "prepared_utc"}
    if set(manifest) != keys or manifest["schema_version"] != 1 or manifest["kind"] != "geometry-primitives-cumulative-prepared-manifest" or manifest["state"] != "PREPARED" or manifest["execution_requested"] is not False:
        raise fail("prepared manifest schema differs")
    if manifest["output_root"] != str(output.resolve()) or set(manifest["gates"].values()) != {"NOT_EXECUTED"}:
        raise fail("prepared manifest state differs")
    if read_json(output / "profile.json") != read_json(pathlib.Path(manifest["inputs"]["profile"]["path"])):
        raise fail("prepared profile differs")
    plan = {"schema_version": 1, "kind": "geometry-primitives-cumulative-launch-plan", "execution_requested": False, "cells": manifest["plan"], "gates": manifest["gates"], "limitations": manifest["retained_limitations"]}
    if read_json(output / "plan.json") != plan or read_json(output / "planned-inventories.json") != manifest["planned_inventories"]:
        raise fail("prepared plan or inventory differs")
    seal = read_json(output / "preparation-seal.json")
    history_bytes = (output / "state-history.jsonl").read_bytes().splitlines(keepends=True)
    if set(seal) != {"schema_version", "files", "state_history_sha256"} or seal["schema_version"] != 1 or seal["files"] != {name: sha256_file(output / name) for name in PREPARATION_FILES} or not history_bytes or seal["state_history_sha256"] != hashlib.sha256(history_bytes[0]).hexdigest():
        raise fail("preparation seal differs")
    state, history = read_json(output / "state.json"), (output / "state-history.jsonl").read_bytes().splitlines()
    initial = json.loads(history[0]) if history else {}
    if not history or initial.get("state") != "PREPARED":
        raise fail("prepared lifecycle differs")
    if require_unconsumed and (len(history) != 1 or state != initial or any((output / name).exists() for name in ("execution-claim.json", "terminal-manifest.json", "command-records.json"))):
        raise fail("prepared manifest is already consumed")
    return manifest


def validate_execution_binding(arguments: argparse.Namespace, source: pathlib.Path, output: pathlib.Path, manifest: dict[str, Any]) -> dict[str, Any]:
    paths, profile, candidate = input_paths(arguments, source), None, published_candidate(source)
    profile = validate_profile(paths["profile"])
    protocol_check(paths["protocol"])
    if manifest["candidate"] != candidate or manifest["environment"] != environment_identity() or manifest["working_directory"] != str(source) or manifest["output_root"] != str(output):
        raise fail("prepared candidate or environment differs")
    verify_input_identity(manifest["inputs"], paths)
    declared = declared_allowlist(source, profile["semantic_ctest_allowlist"])
    plan = command_plan(profile, source, declared)
    if manifest["declared_semantic_ctest_allowlist"] != declared or manifest["plan"] != plan or manifest["planned_inventories"] != planned_inventories(profile, candidate, plan) or manifest["gates"] != {gate: "NOT_EXECUTED" for gate in profile["gates"]}:
        raise fail("prepared manifest binding differs")
    return profile


def prepare(arguments: argparse.Namespace) -> None:
    source, output = pathlib.Path(arguments.source_root).resolve(), pathlib.Path(arguments.output_root).resolve()
    if output.exists() or source == output or source in output.parents:
        raise fail("output root must be a new external directory")
    paths = input_paths(arguments, source)
    profile, candidate = validate_profile(paths["profile"]), published_candidate(source)
    protocol_check(paths["protocol"])
    declared = declared_allowlist(source, profile["semantic_ctest_allowlist"])
    plan = command_plan(profile, source, declared)
    output.mkdir(parents=True); shutil.copyfile(paths["profile"], output / "profile.json")
    manifest = {"schema_version": 1, "kind": "geometry-primitives-cumulative-prepared-manifest", "state": "PREPARED", "execution_requested": False, "candidate": candidate, "inputs": input_identity(paths), "environment": environment_identity(), "working_directory": str(source), "output_root": str(output), "limits_seconds": {"build": BUILD_TIMEOUT_SECONDS, "process": PROCESS_TIMEOUT_SECONDS, "overall": OVERALL_TIMEOUT_SECONDS}, "declared_semantic_ctest_allowlist": declared, "plan": plan, "planned_inventories": planned_inventories(profile, candidate, plan), "gates": {gate: "NOT_EXECUTED" for gate in profile["gates"]}, "retained_limitations": profile["limitations"], "prepared_utc": utc_now()}
    write_json(output / "prepared-manifest.json", manifest)
    write_json(output / "plan.json", {"schema_version": 1, "kind": "geometry-primitives-cumulative-launch-plan", "execution_requested": False, "cells": plan, "gates": manifest["gates"], "limitations": manifest["retained_limitations"]})
    write_json(output / "planned-inventories.json", manifest["planned_inventories"])
    write_state(output, "PREPARED", {"candidate_commit": candidate["commit"], "execution_requested": False, "prepared_manifest_sha256": sha256_file(output / "prepared-manifest.json")})
    write_json(output / "preparation-seal.json", preparation_seal(output)); validate_prepared(output)


def replace_root(value: Any, output: pathlib.Path, repetition: int | None = None) -> Any:
    if isinstance(value, list):
        return [replace_root(item, output, repetition) for item in value]
    if isinstance(value, str):
        result = value.replace("@OUTPUT_ROOT@", str(output))
        return result.replace("@REPETITION@", str(repetition)) if repetition is not None else result
    return value


def require_success(record: dict[str, Any], context: str) -> None:
    if record.get("exit_code") != 0 or record.get("timed_out") or record.get("launch_error") is not None:
        raise fail(f"{context} failed")


def discovered_tests_from_log(output: pathlib.Path, record: dict[str, Any]) -> list[str]:
    text = (output / record["stdout"]["path"]).read_text(encoding="utf-8", errors="strict")
    return re.findall(r"Test #\d+: ([^\s]+)", text)


def write_records(output: pathlib.Path, records: list[dict[str, Any]]) -> None:
    write_json(output / "command-records.json", {"schema_version": 1, "kind": "geometry-primitives-cumulative-command-records", "records": records})


def write_terminal(output: pathlib.Path, manifest: dict[str, Any], state: str, extra: dict[str, str]) -> None:
    write_json(output / "terminal-manifest.json", {"schema_version": 1, "kind": "geometry-primitives-cumulative-terminal-manifest", "candidate": manifest["candidate"], "prepared_manifest_sha256": sha256_file(output / "prepared-manifest.json"), "state": state, "command_records": "command-records.json", "retention_manifest": "retention-manifest.json", **extra})


def verify_detached_candidate(source: pathlib.Path, candidate: dict[str, Any]) -> dict[str, Any]:
    with tempfile.TemporaryDirectory(prefix="apmesh-core-gpr-detached-") as temporary:
        worktree = pathlib.Path(temporary) / "candidate"
        added = subprocess.run(["git", "worktree", "add", "--detach", str(worktree), candidate["commit"]], cwd=source, capture_output=True, text=True, check=False)
        if added.returncode != 0:
            raise fail("detached candidate worktree could not be created")
        try:
            observed = clean_candidate(worktree)
            if observed["commit"] != candidate["commit"] or observed["source_inventory"] != candidate["source_inventory"]:
                raise fail("detached candidate identity differs")
        finally:
            removed = subprocess.run(["git", "worktree", "remove", "--force", str(worktree)], cwd=source, capture_output=True, text=True, check=False)
            if removed.returncode != 0:
                raise fail("detached candidate worktree could not be removed")
    return {"result": "PASS", "candidate_commit": candidate["commit"], "source_inventory_count": len(candidate["source_inventory"])}


def write_retention_inventory(output: pathlib.Path, manifest: dict[str, Any], required_paths: set[str]) -> None:
    missing = sorted(path for path in required_paths if path != "retention-manifest.json" and not (output / path).is_file())
    if missing:
        raise fail("required retained artifact is absent")
    files = []
    for path in sorted((item for item in output.rglob("*") if item.is_file()), key=lambda item: item.as_posix()):
        relative = relative_path(output, path)
        if relative == "retention-manifest.json" or "/build/" in relative:
            continue
        files.append({"path": relative, "sha256": sha256_file(path), "size": path.stat().st_size})
    write_json(output / "retention-manifest.json", {"schema_version": 1, "kind": "geometry-primitives-cumulative-retention", "candidate_commit": manifest["candidate"]["commit"], "prepared_manifest_sha256": sha256_file(output / "prepared-manifest.json"), "required_paths": sorted(required_paths), "files": files})


def verify_retention(output: pathlib.Path) -> dict[str, Any]:
    manifest = validate_prepared(output, require_unconsumed=False)
    terminal, retention = read_json(output / "terminal-manifest.json"), read_json(output / "retention-manifest.json")
    if terminal.get("candidate") != manifest["candidate"] or terminal.get("prepared_manifest_sha256") != sha256_file(output / "prepared-manifest.json") or terminal.get("state") not in {"EXECUTED_PENDING_AUDIT", "BLOCKED"}:
        raise fail("terminal manifest differs")
    expected = set(FAILURE_FILES)
    if terminal["state"] == "EXECUTED_PENDING_AUDIT":
        expected = set(SUCCESS_FILES) | {row["path"] for row in manifest["planned_inventories"]["artifacts"] if row["required_when"] in {"always", "success", "executed"}}
    if set(retention) != {"schema_version", "kind", "candidate_commit", "prepared_manifest_sha256", "required_paths", "files"} or retention.get("schema_version") != 1 or retention.get("kind") != "geometry-primitives-cumulative-retention" or retention.get("candidate_commit") != manifest["candidate"]["commit"] or retention.get("prepared_manifest_sha256") != sha256_file(output / "prepared-manifest.json") or set(retention.get("required_paths", [])) != expected:
        raise fail("retention manifest differs")
    listed = {"retention-manifest.json"}
    for entry in retention.get("files", []):
        path = (output / entry.get("path", "")).resolve()
        if set(entry) != {"path", "sha256", "size"} or entry["path"] in listed or output.resolve() not in path.parents or not path.is_file() or sha256_file(path) != entry["sha256"] or path.stat().st_size != entry["size"]:
            raise fail("retained file differs")
        listed.add(entry["path"])
    actual = {relative_path(output, path) for path in output.rglob("*") if path.is_file() and "/build/" not in relative_path(output, path)}
    if actual != listed:
        raise fail("retained file set differs")
    records = read_json(output / "command-records.json").get("records")
    if not isinstance(records, list):
        raise fail("command records differ")
    if terminal["state"] == "BLOCKED":
        failure = read_json(output / "failure.json")
        if failure.get("records") != records or not failure.get("message"):
            raise fail("failure evidence differs")
    else:
        profile = validate_profile(output / "profile.json")
        index = output / "certificate-index.json"
        if read_json(output / "cross-cell-comparison.json") != compare(profile, index):
            raise fail("cross-cell comparison differs")
        per_cell = read_json(output / "per-cell-comparisons.json")
        if [row.get("cell") for row in per_cell.get("cells", [])] != [cell["id"] for cell in profile["cells"]]:
            raise fail("per-cell comparison differs")
        summary = read_json(output / "gate-summary.json")
        if set(summary.get("gates", {}).values()) != {"EVIDENCE_COLLECTED_PENDING_AUDIT"}:
            raise fail("gate summary claims closure")
    detached = read_json(output / "detached-verification.json")
    if detached != {"schema_version": 1, "kind": "geometry-primitives-cumulative-detached-verification", "result": "PASS", "candidate_commit": manifest["candidate"]["commit"], "source_inventory_count": len(manifest["candidate"]["source_inventory"])}:
        raise fail("detached verification differs")
    return {"schema_version": 1, "kind": "geometry-primitives-cumulative-retention-verification", "status": "PASS", "files": sorted(listed)}


def seal_output(output: pathlib.Path, manifest: dict[str, Any], required_paths: set[str]) -> None:
    detached = verify_detached_candidate(pathlib.Path(manifest["candidate"]["source_root"]), manifest["candidate"])
    write_json(output / "detached-verification.json", {"schema_version": 1, "kind": "geometry-primitives-cumulative-detached-verification", **detached})
    write_retention_inventory(output, manifest, required_paths)
    verify_retention(output)


def per_cell_comparisons(profile: dict[str, Any], entries: list[dict[str, Any]], output: pathlib.Path) -> dict[str, Any]:
    cells = []
    for cell in profile["cells"]:
        selected = [entry for entry in entries if entry["cell"] == cell["id"]]
        projections = [validate_certificate(profile, output / entry["path"]) for entry in selected]
        if len(projections) != profile["repetitions_per_cell"] or any(item != projections[0] for item in projections[1:]):
            raise fail("per-cell semantic projection differs")
        cells.append({"cell": cell["id"], "certificate_count": len(projections), "status": "EVIDENCE_COLLECTED_PENDING_AUDIT", "semantic_projection": projections[0]})
    return {"schema_version": 1, "kind": "geometry-primitives-cumulative-per-cell-comparisons", "cells": cells}


def gate_summary(profile: dict[str, Any], limitations: list[str]) -> dict[str, Any]:
    return {"schema_version": 1, "kind": "geometry-primitives-cumulative-gate-summary", "status": "EVIDENCE_COLLECTED_PENDING_AUDIT", "gates": {gate: "EVIDENCE_COLLECTED_PENDING_AUDIT" for gate in profile["gates"]}, "limitations": limitations}


def execute(arguments: argparse.Namespace) -> int:
    source, output = pathlib.Path(arguments.source_root).resolve(), pathlib.Path(arguments.output_root).resolve()
    records: list[dict[str, Any]] = []
    entries: list[dict[str, Any]] = []
    try:
        manifest = validate_prepared(output, require_unconsumed=True)
        profile = validate_execution_binding(arguments, source, output, manifest)
        write_json(output / "execution-claim.json", {"schema_version": 1, "kind": "geometry-primitives-cumulative-execution-claim", "candidate_commit": manifest["candidate"]["commit"], "prepared_manifest_sha256": sha256_file(output / "prepared-manifest.json"), "pid": os.getpid(), "started_utc": utc_now()})
        write_state(output, "RUNNING", {"candidate_commit": manifest["candidate"]["commit"]})
        discoveries, inventories = [], []
        for cell in manifest["plan"]:
            name = cell["cell"]
            for stage in ("configure", "build", "ctest_discovery", "semantic_ctest"):
                record = run_command(replace_root(cell[stage], output), source, output / "logs", f"{name}-{stage.replace('_', '-')}", BUILD_TIMEOUT_SECONDS)
                records.append(record); require_success(record, f"{name} {stage}")
                if stage == "ctest_discovery":
                    observed = discovered_tests_from_log(output, record)
                    selected = [item for item in observed if item in manifest["declared_semantic_ctest_allowlist"]]
                    if sorted(selected) != sorted(manifest["declared_semantic_ctest_allowlist"]) or len(selected) != len(set(selected)):
                        raise fail("observed semantic CTest allowlist differs")
                    discoveries.append({"cell": name, "record_id": record["id"], "discovered_allowlist": selected})
            for repetition in range(1, cell["repetitions"] + 1):
                for stage in ("certificate", "certificate_validation"):
                    record = run_command(replace_root(cell[stage], output, repetition), source, output / "logs", f"{name}-{stage.replace('_', '-')}-{repetition}", PROCESS_TIMEOUT_SECONDS)
                    records.append(record); require_success(record, f"{name} {stage}")
                certificate = output / "certificates" / f"{name}-{repetition}.json"
                entries.append({"cell": name, "repetition": repetition, "path": relative_path(output, certificate), "sha256": sha256_file(certificate)})
            for stage, timeout in (("negative_outcomes", PROCESS_TIMEOUT_SECONDS), ("dependency_inventory", PROCESS_TIMEOUT_SECONDS)):
                record = run_command(replace_root(cell[stage], output), source, output / "logs", f"{name}-{stage.replace('_', '-')}", timeout)
                records.append(record); require_success(record, f"{name} {stage}")
            validate_negative_outcomes(profile, output / "negatives" / f"{name}.json")
            runtime = run_command(["ldd", replace_root(cell["runtime_dependencies"][0], output)], source, output / "logs", f"{name}-ldd-exporter", PROCESS_TIMEOUT_SECONDS)
            records.append(runtime); require_success(runtime, f"{name} runtime dependency")
            if "not found" in ((output / runtime["stdout"]["path"]).read_text(encoding="utf-8") + (output / runtime["stderr"]["path"]).read_text(encoding="utf-8")):
                raise fail("unresolved runtime dependency")
            build = output / "cells" / name / "build"; compile_commands = build / "compile_commands.json"; retained = output / "cells" / name / "compile_commands.json"
            if not compile_commands.is_file():
                raise fail("compile command inventory is absent")
            retained.parent.mkdir(parents=True, exist_ok=True); shutil.copyfile(compile_commands, retained)
            inventories.append({"cell": name, "compile_commands": {"path": relative_path(output, retained), "sha256": sha256_file(retained)}, "dependency_record_id": records[-2]["id"], "runtime_record_id": runtime["id"]})
        write_records(output, records)
        write_json(output / "certificate-index.json", {"schema_version": 1, "kind": "geometry-primitives-cumulative-certificate-index", "entries": entries})
        write_json(output / "per-cell-comparisons.json", per_cell_comparisons(profile, entries, output))
        write_json(output / "cross-cell-comparison.json", compare(profile, output / "certificate-index.json"))
        write_json(output / "observed-inventories.json", {"schema_version": 1, "kind": "geometry-primitives-cumulative-observed-inventories", "candidate_commit": manifest["candidate"]["commit"], "discoveries": discoveries, "cells": inventories})
        summary = gate_summary(profile, manifest["retained_limitations"])
        write_json(output / "gate-summary.json", summary)
        (output / "gate-summary.md").write_text("# GPR0-GPR7 execution evidence\n\nStatus: EVIDENCE_COLLECTED_PENDING_AUDIT\n", encoding="utf-8")
        write_terminal(output, manifest, "EXECUTED_PENDING_AUDIT", {"certificate_index": "certificate-index.json", "per_cell_comparisons": "per-cell-comparisons.json", "cross_cell_comparison": "cross-cell-comparison.json", "gate_summary": "gate-summary.json"})
        write_state(output, "EXECUTED_PENDING_AUDIT", {"candidate_commit": manifest["candidate"]["commit"], "comparison": "cross-cell-comparison.json"})
        required = set(SUCCESS_FILES) | {row["path"] for row in manifest["planned_inventories"]["artifacts"] if row["required_when"] in {"always", "success", "executed"}}
        seal_output(output, manifest, required)
        return 0
    except Exception as error:
        if (output / "state.json").is_file() and read_json(output / "state.json").get("state") == "BLOCKED":
            return 1
        write_records(output, records)
        manifest = read_json(output / "prepared-manifest.json")
        write_json(output / "failure.json", {"schema_version": 1, "kind": "geometry-primitives-cumulative-failure", "message": str(error), "records": records, "partial_certificate_slots": entries})
        if read_json(output / "state.json").get("state") == "PREPARED":
            write_state(output, "RUNNING", {"candidate_commit": manifest["candidate"]["commit"]})
        write_terminal(output, manifest, "BLOCKED", {"failure": "failure.json"})
        write_state(output, "BLOCKED", {"candidate_commit": manifest["candidate"]["commit"], "closure_failure": True, "failure": "failure.json"})
        seal_output(output, manifest, set(FAILURE_FILES))
        return 1


def self_check(arguments: argparse.Namespace) -> dict[str, Any]:
    source = pathlib.Path(arguments.source_root).resolve(); paths = input_paths(arguments, source); profile = validate_profile(paths["profile"])
    protocol_check(paths["protocol"])
    declared, plan = declared_allowlist(source, profile["semantic_ctest_allowlist"]), None
    plan = command_plan(profile, source, declared)
    if len(plan) != 4 or any(cell["repetitions"] != 3 for cell in plan):
        raise fail("fixed matrix differs")
    return {"schema_version": 1, "kind": "geometry-primitives-cumulative-runner-self-check", "status": "PASS", "execution_requested": False, "input_hashes": input_identity(paths), "declared_semantic_ctest_allowlist": declared, "plan": {"schema_version": 1, "kind": "geometry-primitives-cumulative-launch-plan", "execution_requested": False, "cells": plan, "gates": {gate: "NOT_EXECUTED" for gate in profile["gates"]}, "limitations": profile["limitations"]}}


def main() -> int:
    parser = argparse.ArgumentParser()
    for name in ("source-root", "profile", "protocol", "exporter", "validator"):
        parser.add_argument(f"--{name}", required=True)
    commands = parser.add_subparsers(dest="command", required=True)
    commands.add_parser("self-check").add_argument("--output", required=True)
    commands.add_parser("plan").add_argument("--output", required=True)
    commands.add_parser("prepare").add_argument("--output-root", required=True)
    commands.add_parser("validate-prepared").add_argument("--output-root", required=True)
    commands.add_parser("execute").add_argument("--output-root", required=True)
    commands.add_parser("verify-retention").add_argument("--output-root", required=True)
    arguments = parser.parse_args()
    try:
        if arguments.command == "prepare":
            prepare(arguments); return 0
        if arguments.command == "validate-prepared":
            output = pathlib.Path(arguments.output_root).resolve(); manifest = validate_prepared(output); validate_execution_binding(arguments, pathlib.Path(arguments.source_root).resolve(), output, manifest); return 0
        if arguments.command == "execute":
            return execute(arguments)
        if arguments.command == "verify-retention":
            print(json.dumps(verify_retention(pathlib.Path(arguments.output_root).resolve()), sort_keys=True)); return 0
        result = self_check(arguments); write_json(pathlib.Path(arguments.output), result if arguments.command == "self-check" else result["plan"]); return 0
    except (RunnerError, RuntimeErrorEvidence, EvidenceError, OSError, ValueError) as error:
        print(f"ERROR: {error}", file=sys.stderr); return 2


if __name__ == "__main__":
    raise SystemExit(main())
