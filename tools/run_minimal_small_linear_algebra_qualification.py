#!/usr/bin/env python3
"""Prepare or explicitly execute the revision-bound LA0-LA7 protocol."""

from __future__ import annotations

import argparse
import json
import hashlib
import shutil
import os
import pathlib
import platform
import re
import subprocess
import sys
import tempfile
import time
from typing import Any

TOOL_ROOT = pathlib.Path(__file__).resolve().parent
if str(TOOL_ROOT) not in sys.path:
    sys.path.insert(0, str(TOOL_ROOT))

from experiment_runtime import (  # noqa: E402
    RuntimeErrorEvidence, clean_candidate, input_identity, read_json, relative_path,
    run_command, sha256_file, tool_version, utc_now, verify_input_identity, write_json, write_state,
)
from minimal_small_linear_algebra_evidence import (  # noqa: E402
    EvidenceError, compare_index, validate_profile, output_fields, validate_source_root, dependency_graph,
    NEGATIVE_CASES, validate_negative_outcomes,
)

BUILD_TIMEOUT_SECONDS = 300
PROCESS_TIMEOUT_SECONDS = 45
OVERALL_TIMEOUT_SECONDS = 1800
CONFIGURATIONS = {
    "gcc-debug": ("g++-13", False), "gcc-release": ("g++-13", False),
    "clang-debug": ("clang++-18", True), "clang-release": ("clang++-18", True),
}
TERMINAL_REQUIRED_SUCCESS = {
    "profile.json",
    "preparation-seal.json", "execution-claim.json", "planned-inventories.json",
    "prepared-manifest.json", "plan.json", "state.json", "state-history.jsonl", "command-records.json",
    "source-checks.json", "prerequisite-discovery.json", "observed-inventories.json", "certificate-index.json",
    "cross-cell-comparison.json", "gate-summary.json", "terminal-manifest.json", "detached-verification.json",
    "retention-manifest.json",
}
TERMINAL_REQUIRED_FAILURE = {
    "profile.json",
    "preparation-seal.json", "execution-claim.json", "planned-inventories.json",
    "prepared-manifest.json", "plan.json", "state.json", "state-history.jsonl", "command-records.json",
    "failure.json", "terminal-manifest.json", "detached-verification.json", "retention-manifest.json",
}
SEAL_FAILURE_PATHS = {"retention-error.json", "pre-seal-terminal.json"}


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
    return "^(" + "|".join(re.escape(name) for name in names) + ")$"


def command_plan(profile: dict[str, Any], source: pathlib.Path) -> list[dict[str, Any]]:
    plan: list[dict[str, Any]] = []
    for cell in profile["cells"]:
        compiler, libcxx = CONFIGURATIONS[cell["id"]]
        build = f"@OUTPUT_ROOT@/cells/{cell['id']}/build"
        source_output = f"@OUTPUT_ROOT@/cells/{cell['id']}/source-check.json"
        plan.append({
            "cell": cell["id"],
            "source_check": [sys.executable, str(source / "tools/minimal_small_linear_algebra_evidence.py"), "--profile", str(source / "experiments/profiles/minimal_small_linear_algebra.json"), "validate-source", "--source-root", str(source), "--output", source_output],
            "configure": ["cmake", "-S", str(source), "-B", build, "-G", "Ninja", f"-DCMAKE_CXX_COMPILER={compiler}", f"-DCMAKE_BUILD_TYPE={cell['build_type']}", f"-DAPMESH_USE_LIBCXX={'ON' if libcxx else 'OFF'}", "-DBUILD_TESTING=ON", "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"],
            "build": ["cmake", "--build", build],
            "dependency_discovery": ["ninja", "-C", build, "-t", "deps"],
            "prerequisite_discovery": ["ctest", "--test-dir", build, "-N"],
            "focused_ctest": ["ctest", "--test-dir", build, "--output-on-failure", "-R", "^apmesh_core\\.(minimal_small_linear_algebra|math_header_isolation|minimal_small_linear_algebra_evidence|minimal_small_linear_algebra_runner|minimal_small_linear_algebra_retention)$"],
            "prerequisite_ctest": ["ctest", "--test-dir", build, "--output-on-failure", "-R", ctest_regex(profile["exact_prerequisite_tests"])],
            "exporter": f"{build}/apmesh_core_minimal_small_linear_algebra_export", "repetitions": profile["repetitions_per_cell"],
            "negative_outcomes": [sys.executable, str(source / "tools/minimal_small_linear_algebra_evidence.py"), "--profile", str(source / "experiments/profiles/minimal_small_linear_algebra.json"), "negative-outcomes", "--certificate", f"@OUTPUT_ROOT@/certificates/{cell['id']}-1.json", "--output", f"@OUTPUT_ROOT@/negatives/{cell['id']}"],
            "compile_commands": f"cells/{cell['id']}/build/compile_commands.json",
            "runtime_executables": ["apmesh_core_minimal_small_linear_algebra_export", "apmesh_core.minimal_small_linear_algebra", "apmesh_core.math_header_isolation"],
        })
    return plan


def planned_inventories(plan: list[dict[str, Any]], candidate: dict[str, Any]) -> dict[str, Any]:
    artifacts = [{"path": path, "role": "terminal-control", "required_when": "always" if path in TERMINAL_REQUIRED_SUCCESS & TERMINAL_REQUIRED_FAILURE else "success" if path in TERMINAL_REQUIRED_SUCCESS else "failure"}
                 for path in sorted(TERMINAL_REQUIRED_SUCCESS | TERMINAL_REQUIRED_FAILURE)]
    artifacts += [{"path": path, "role": "seal-failure", "required_when": "seal-failure"} for path in sorted(SEAL_FAILURE_PATHS)]
    artifacts += [{"path": path, "role": "seal-snapshot", "required_when": "if-produced"} for path in ("pre-seal-detached.json", "pre-seal-retention.json")]
    cells = []
    for cell in plan:
        name = cell["cell"]
        stages = ["source_check", "configure", "build", "dependency_discovery", "prerequisite-discovery", "focused_ctest", "prerequisite_ctest", "negative-outcomes"]
        stages += [f"certificate-{repeat}" for repeat in range(1, cell["repetitions"] + 1)]
        stages += ["ldd-" + executable.replace(".", "_") for executable in cell["runtime_executables"]]
        for stage in stages:
            artifacts += [{"path": f"logs/{name}-{stage}.{stream}.log", "role": "command-log", "required_when": "command-started"} for stream in ("stdout", "stderr")]
        artifacts += [{"path": f"certificates/{name}-{repeat}.json", "role": "certificate", "required_when": "success"} for repeat in range(1, cell["repetitions"] + 1)]
        artifacts += [{"path": f"negatives/{name}/{file}.json", "role": "negative-evidence", "required_when": "success"} for file in ("baseline", "negative-outcomes", *NEGATIVE_CASES)]
        artifacts += [{"path": f"cells/{name}/{file}", "role": role, "required_when": "success"} for file, role in (("source-check.json", "source-check"), ("compile_commands.json", "compile-inventory"), ("dependency-graph.json", "module-graph"))]
        cells.append({"cell": name, "compile_commands": cell["compile_commands"], "runtime_executables": cell["runtime_executables"]})
    return {"schema_version": 3, "kind": "minimal-small-linear-algebra-planned-inventories", "source": candidate["source_inventory"], "cells": cells, "artifacts": artifacts}


def preparation_files() -> tuple[str, ...]:
    return ("prepared-manifest.json", "plan.json", "planned-inventories.json", "profile.json")


def validate_preparation_chain(output: pathlib.Path, *, require_prepared: bool) -> dict[str, Any]:
    seal = read_json(output / "preparation-seal.json")
    if set(seal) != {"schema_version", "files", "initial_state", "initial_state_sha256"} or seal["schema_version"] != 1:
        raise fail("preparation seal schema differs")
    expected = {path: sha256_file(output / path) for path in preparation_files()}
    if seal["files"] != expected: raise fail("prepared bytes differ from seal")
    history_bytes = (output / "state-history.jsonl").read_bytes().splitlines(keepends=True)
    if not history_bytes or hashlib.sha256(history_bytes[0]).hexdigest() != seal["initial_state_sha256"]:
        raise fail("initial state history differs from preparation seal")
    history = [json.loads(row) for row in history_bytes]
    state = read_json(output / "state.json"); manifest = read_json(output / "prepared-manifest.json")
    if history[0] != seal["initial_state"] or state != history[-1]: raise fail("state and history differ")
    if history[0]["state"] != "PREPARED" or history[0]["detail"] != {"candidate_commit": manifest["candidate"]["commit"], "execution_requested": False, "prepared_manifest_sha256": expected["prepared-manifest.json"]}:
        raise fail("initial state identity differs")
    transitions = {"PREPARED": {"RUNNING"}, "RUNNING": {"BLOCKED", "EXECUTED_PENDING_AUDIT"}, "EXECUTED_PENDING_AUDIT": {"BLOCKED"}, "BLOCKED": set()}
    for previous, current in zip(history, history[1:]):
        if current["state"] not in transitions[previous["state"]] or current["detail"].get("candidate_commit") != manifest["candidate"]["commit"]:
            raise fail("invalid retained lifecycle")
    if require_prepared and (len(history) != 1 or state["state"] != "PREPARED" or (output / "execution-claim.json").exists()):
        raise fail("prepared attempt is already consumed")
    if read_json(output / "plan.json") != {"schema_version": 2, "kind": "minimal-small-linear-algebra-launch-plan", "cells": manifest["plan"]}:
        raise fail("plan.json differs from prepared manifest")
    if read_json(output / "planned-inventories.json") != manifest["planned_inventories"]:
        raise fail("planned inventories differ from manifest")
    if sha256_file(output / "profile.json") != manifest["inputs"]["profile"]["sha256"]:
        raise fail("retained profile identity differs")
    return manifest


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
    manifest = {"schema_version": 4, "kind": "minimal-small-linear-algebra-prepared-manifest", "state": "PREPARED", "execution_requested": False, "candidate": candidate, "inputs": input_identity(paths), "environment": environment, "working_directory": str(source), "output_root": str(output), "limits_seconds": {"build": BUILD_TIMEOUT_SECONDS, "process": PROCESS_TIMEOUT_SECONDS, "overall": OVERALL_TIMEOUT_SECONDS}, "plan": plan, "planned_inventories": planned_inventories(plan, candidate), "certificate_fields": {case: output_fields(case) for case in profile["cases"]}, "expected_certificate_slots": expected_slots(profile), "gates": {gate: "NOT_EXECUTED" for gate in profile["gates"]}, "retained_limitations": profile["limitations"], "prepared_utc": utc_now()}
    write_json(output / "prepared-manifest.json", manifest)
    write_json(output / "plan.json", {"schema_version": 2, "kind": "minimal-small-linear-algebra-launch-plan", "cells": plan})
    write_state(output, "PREPARED", {"candidate_commit": candidate["commit"], "execution_requested": False, "prepared_manifest_sha256": sha256_file(output / "prepared-manifest.json")})
    write_json(output / "planned-inventories.json", manifest["planned_inventories"])
    shutil.copyfile(paths["profile"], output / "profile.json")
    write_json(output / "preparation-seal.json", {"schema_version": 1, "files": {path: sha256_file(output / path) for path in preparation_files()}, "initial_state": read_json(output / "state.json"), "initial_state_sha256": sha256_file(output / "state-history.jsonl")})
    validate_preparation_chain(output, require_prepared=True)


def load_prepared(arguments: argparse.Namespace) -> tuple[pathlib.Path, dict[str, Any], dict[str, Any]]:
    source, output = pathlib.Path(arguments.source_root).resolve(), pathlib.Path(arguments.output_root).resolve()
    manifest = validate_preparation_chain(output, require_prepared=True); profile = validate_profile(canonical(source, "experiments/profiles/minimal_small_linear_algebra.json", arguments.profile, "profile"))
    required = {"schema_version", "kind", "state", "execution_requested", "candidate", "inputs", "environment", "working_directory", "output_root", "limits_seconds", "plan", "planned_inventories", "expected_certificate_slots", "gates", "retained_limitations", "prepared_utc"}
    required.add("certificate_fields")
    if set(manifest) != required or manifest["schema_version"] != 4 or manifest["kind"] != "minimal-small-linear-algebra-prepared-manifest" or manifest["state"] != "PREPARED" or manifest["execution_requested"] is not False:
        raise fail("prepared manifest schema differs")
    if manifest["candidate"] != published_candidate(source) or manifest["environment"] != environment_identity() or manifest["working_directory"] != str(source) or manifest["output_root"] != str(output):
        raise fail("prepared candidate or environment differs")
    verify_input_identity(manifest["inputs"], inputs(arguments, source))
    plan = command_plan(profile, source)
    if manifest["limits_seconds"] != {"build": BUILD_TIMEOUT_SECONDS, "process": PROCESS_TIMEOUT_SECONDS, "overall": OVERALL_TIMEOUT_SECONDS} or manifest["certificate_fields"] != {case: output_fields(case) for case in profile["cases"]}:
        raise fail("prepared limits or fields differ")
    if manifest["plan"] != plan or manifest["planned_inventories"] != planned_inventories(plan, manifest["candidate"]) or manifest["expected_certificate_slots"] != expected_slots(profile) or manifest["gates"] != {gate: "NOT_EXECUTED" for gate in profile["gates"]} or manifest["retained_limitations"] != profile["limitations"]:
        raise fail("prepared manifest plan differs")
    return output, manifest, profile


def require_success(record: dict[str, Any], context: str) -> None:
    if record["exit_code"] != 0 or record["timed_out"] or record["launch_error"] is not None:
        raise fail(f"{context} failed")


def validate_observed_dependencies(record: dict[str, Any], output: pathlib.Path) -> None:
    path = output / record["stdout"]["path"]
    if sha256_file(path) != record["stdout"]["sha256"]: raise fail("dependency log hash differs")
    blocks = path.read_text(encoding="utf-8").split("\n\n")
    math = [block for block in blocks if block.startswith("CMakeFiles/apmesh_core.dir/src/math/linear_algebra.cpp.o:")]
    geometry = [block for block in blocks if block.startswith("CMakeFiles/apmesh_core.dir/src/core/geometry.cpp.o:")]
    if len(math) != 1 or len(geometry) != 1 or "deps not found" in math[0] + geometry[0]:
        raise fail("observed object dependency inventory is absent")
    if "/apmesh/math/linear_algebra.hpp" not in math[0] or "/apmesh/math/linear_algebra.hpp" not in geometry[0]:
        raise fail("observed math/adapter dependency is absent")
    if "/apmesh/core/geometry.hpp" in math[0]: raise fail("observed math depends on geometry")


def discovered_test_names(record: dict[str, Any], output: pathlib.Path) -> list[str]:
    stdout = output / record["stdout"]["path"]
    if not stdout.is_file() or sha256_file(stdout) != record["stdout"]["sha256"]:
        raise fail("CTest discovery stdout differs")
    names = re.findall(r"(?m)^\s*Test\s+#\d+:\s+(.+?)\s*$", stdout.read_text(encoding="utf-8", errors="strict"))
    if len(names) != len(set(names)):
        raise fail("CTest discovery contains duplicate test names")
    return names


def validate_prerequisite_discovery(discovered: list[str], expected: list[str]) -> None:
    if set(discovered) != set(expected) or len(discovered) != len(expected):
        raise fail("CTest prerequisite allowlist differs from discovery")


def verify_detached_candidate(source: pathlib.Path, candidate: dict[str, Any]) -> dict[str, Any]:
    with tempfile.TemporaryDirectory(prefix="apmesh-core-la-detached-") as temporary:
        detached = pathlib.Path(temporary) / "candidate"
        added = subprocess.run(["git", "worktree", "add", "--detach", str(detached), candidate["commit"]], cwd=source, capture_output=True, text=True, check=False)
        if added.returncode != 0: raise fail("detached candidate worktree could not be created")
        try:
            observed = clean_candidate(detached)
            expected_inventory = [{"path": row["path"], "sha256": row["sha256"]} for row in candidate["source_inventory"]]
            actual_inventory = [{"path": row["path"], "sha256": row["sha256"]} for row in observed["source_inventory"]]
            if observed["commit"] != candidate["commit"] or actual_inventory != expected_inventory: raise fail("detached candidate identity differs")
            return {"result": "PASS", "candidate_commit": candidate["commit"], "source_inventory_count": len(actual_inventory)}
        finally:
            removed = subprocess.run(["git", "worktree", "remove", "--force", str(detached)], cwd=source, capture_output=True, text=True, check=False)
            if removed.returncode != 0: raise fail("detached candidate worktree could not be removed")


def observed_inventories(output: pathlib.Path, plan: list[dict[str, Any]], source: pathlib.Path, records: list[dict[str, Any]], deadline: float) -> dict[str, Any]:
    cells: list[dict[str, Any]] = []
    for cell in plan:
        if time.monotonic() >= deadline: raise fail("overall qualification timeout")
        build = output / "cells" / cell["cell"] / "build"; compile_commands = build / "compile_commands.json"
        if not compile_commands.is_file(): raise fail(f"compile commands are absent: {cell['cell']}")
        retained_compile = output / "cells" / cell["cell"] / "compile_commands.json"
        shutil.copyfile(compile_commands, retained_compile)
        write_json(output / "cells" / cell["cell"] / "dependency-graph.json", {"schema_version": 1, "edges": dependency_graph(source)})
        dependencies: list[dict[str, Any]] = []
        for executable_name in cell["runtime_executables"]:
            executable = build / executable_name
            if not executable.is_file(): raise fail(f"runtime executable is absent: {cell['cell']}:{executable_name}")
            record = run_command(["ldd", str(executable)], source, output / "logs", f"{cell['cell']}-ldd-{executable_name.replace('.', '_')}", PROCESS_TIMEOUT_SECONDS)
            records.append(record); require_success(record, f"{cell['cell']} ldd {executable_name}")
            if "not found" in (output / record["stdout"]["path"]).read_text(encoding="utf-8"):
                raise fail(f"unresolved runtime dependency: {cell['cell']}:{executable_name}")
            dependencies.append({"executable": relative_path(output, executable), "sha256": sha256_file(executable), "ldd_record_id": record["id"], "ldd_stdout_sha256": record["stdout"]["sha256"]})
        cells.append({"cell": cell["cell"], "compile_commands": {"path": relative_path(output, retained_compile), "sha256": sha256_file(retained_compile)}, "runtime_dependencies": dependencies})
    return {"schema_version": 1, "kind": "minimal-small-linear-algebra-observed-inventories", "cells": cells}


def write_command_records(output: pathlib.Path, records: list[dict[str, Any]]) -> None:
    write_json(output / "command-records.json", {"schema_version": 2, "kind": "minimal-small-linear-algebra-command-records", "records": records})


def write_terminal(output: pathlib.Path, manifest: dict[str, Any], state: str, extra: dict[str, Any]) -> None:
    value = {"schema_version": 3, "kind": "minimal-small-linear-algebra-terminal-manifest", "state": state, "candidate": manifest["candidate"], "prepared_manifest_sha256": sha256_file(output / "prepared-manifest.json"), "records": "command-records.json", "retention_manifest": "retention-manifest.json", **extra}
    write_json(output / "terminal-manifest.json", value)


def write_retention_inventory(output: pathlib.Path, manifest: dict[str, Any], required_paths: set[str]) -> None:
    """Byte inventory only: writing this file never asserts scientific qualification."""
    files = sorted(path for path in output.rglob("*") if path.is_file() and path.name != "retention-manifest.json" and "/build/" not in path.relative_to(output).as_posix())
    manifest_data = {"schema_version": 3, "kind": "minimal-small-linear-algebra-retention", "candidate_commit": manifest["candidate"]["commit"], "prepared_manifest_sha256": sha256_file(output / "prepared-manifest.json"), "required_paths": sorted(required_paths), "files": [{"path": relative_path(output, path), "sha256": sha256_file(path), "size": path.stat().st_size} for path in files]}
    write_json(output / "retention-manifest.json", manifest_data)


def seal_output(output: pathlib.Path, source: pathlib.Path, manifest: dict[str, Any], required_paths: set[str]) -> None:
    stage = "detached-verification"
    try:
        detached = verify_detached_candidate(source, manifest["candidate"])
        write_json(output / "detached-verification.json", {"schema_version": 1, "kind": "minimal-small-linear-algebra-detached-verification", **detached})
        stage = "inventory"
        write_retention_inventory(output, manifest, required_paths)
        stage = "verification"
        verify_retention(argparse.Namespace(output_root=str(output)))
    except Exception as error:
        # Preserve the failed attempt. This is not a second detached check or a retry
        # of success sealing: it writes an explicitly BLOCKED diagnostic archive.
        snapshots = {}
        for original, snapshot in (("terminal-manifest.json", "pre-seal-terminal.json"), ("detached-verification.json", "pre-seal-detached.json"), ("retention-manifest.json", "pre-seal-retention.json")):
            path = output / original
            if path.is_file():
                with (output / snapshot).open("xb") as stream: stream.write(path.read_bytes())
                snapshots[snapshot] = sha256_file(output / snapshot)
        failure = {"schema_version": 1, "kind": "minimal-small-linear-algebra-seal-failure", "candidate_commit": manifest["candidate"]["commit"], "stage": stage, "error_type": type(error).__name__, "message": str(error), "retry_attempted": False, "snapshots": snapshots}
        write_json(output / "retention-error.json", failure)
        if not (output / "failure.json").exists():
            write_json(output / "failure.json", {"schema_version": 2, "kind": "minimal-small-linear-algebra-failure", "message": str(error), "records": read_json(output / "command-records.json")["records"], "partial_certificate_slots": []})
        write_terminal(output, manifest, "BLOCKED", {"failure": "failure.json", "retention_failure": "retention-error.json"})
        if read_json(output / "state.json")["state"] != "BLOCKED":
            write_state(output, "BLOCKED", {"candidate_commit": manifest["candidate"]["commit"], "closure_failure": True, "failure": "failure.json"})
        write_json(output / "detached-verification.json", {"schema_version": 1, "kind": "minimal-small-linear-algebra-detached-verification", "result": "BLOCKED", "candidate_commit": manifest["candidate"]["commit"], "retention_failure": "retention-error.json"})
        write_retention_inventory(output, manifest, TERMINAL_REQUIRED_FAILURE | SEAL_FAILURE_PATHS)
        raise


def execute(arguments: argparse.Namespace) -> None:
    output, manifest, profile = load_prepared(arguments)
    # Exclusive creation prevents concurrent consumers or reuse after a partial failure.
    with (output / "execution-claim.json").open("x", encoding="utf-8") as stream:
        json.dump({"candidate_commit": manifest["candidate"]["commit"], "prepared_manifest_sha256": sha256_file(output / "prepared-manifest.json"), "pid": os.getpid(), "started_utc": utc_now()}, stream)
    source = pathlib.Path(arguments.source_root).resolve(); write_state(output, "RUNNING", {"candidate_commit": manifest["candidate"]["commit"], "prepared_manifest_sha256": sha256_file(output / "prepared-manifest.json")})
    records: list[dict[str, Any]] = []; entries: list[dict[str, Any]] = []; source_checks: list[dict[str, Any]] = []; discovery: list[dict[str, Any]] = []
    deadline = time.monotonic() + OVERALL_TIMEOUT_SECONDS
    try:
        for cell in manifest["plan"]:
            for stage in ("source_check", "configure", "build", "dependency_discovery"):
                if time.monotonic() >= deadline: raise fail("overall qualification timeout")
                record = run_command(replace_root(cell[stage], output), source, output / "logs", f"{cell['cell']}-{stage}", BUILD_TIMEOUT_SECONDS)
                records.append(record); require_success(record, f"{cell['cell']} {stage}")
                if stage == "dependency_discovery": validate_observed_dependencies(record, output)
            source_path = output / "cells" / cell["cell"] / "source-check.json"; source_value = read_json(source_path)
            if source_value != {"schema_version": 1, "kind": "minimal-small-linear-algebra-source-check", "checks": {check: "PASS" for check in profile["source_checks"]}}: raise fail(f"source check artifact differs: {cell['cell']}")
            source_checks.append({"cell": cell["cell"], "path": relative_path(output, source_path), "sha256": sha256_file(source_path)})
            record = run_command(replace_root(cell["prerequisite_discovery"], output), source, output / "logs", f"{cell['cell']}-prerequisite-discovery", PROCESS_TIMEOUT_SECONDS)
            records.append(record); require_success(record, f"{cell['cell']} prerequisite discovery")
            names = discovered_test_names(record, output); validate_prerequisite_discovery([name for name in names if name in profile["exact_prerequisite_tests"]], profile["exact_prerequisite_tests"])
            discovery.append({"cell": cell["cell"], "record_id": record["id"], "discovered_allowlist": [name for name in names if name in profile["exact_prerequisite_tests"]]})
            for stage in ("focused_ctest", "prerequisite_ctest"):
                if time.monotonic() >= deadline: raise fail("overall qualification timeout")
                record = run_command(replace_root(cell[stage], output), source, output / "logs", f"{cell['cell']}-{stage}", BUILD_TIMEOUT_SECONDS)
                records.append(record); require_success(record, f"{cell['cell']} {stage}")
            exporter = pathlib.Path(cell["exporter"].replace("@OUTPUT_ROOT@", str(output)))
            for repetition in range(1, profile["repetitions_per_cell"] + 1):
                if time.monotonic() >= deadline: raise fail("overall qualification timeout")
                certificate = output / "certificates" / f"{cell['cell']}-{repetition}.json"; certificate.parent.mkdir(parents=True, exist_ok=True)
                record = run_command([str(exporter), str(certificate)], source, output / "logs", f"{cell['cell']}-certificate-{repetition}", PROCESS_TIMEOUT_SECONDS)
                records.append(record); require_success(record, f"{cell['cell']} certificate {repetition}")
                entries.append({"cell": cell["cell"], "repetition": repetition, "path": relative_path(output, certificate), "sha256": sha256_file(certificate)})
            record = run_command(replace_root(cell["negative_outcomes"], output), source, output / "logs", f"{cell['cell']}-negative-outcomes", PROCESS_TIMEOUT_SECONDS)
            records.append(record); require_success(record, f"{cell['cell']} negative outcomes")
        inventories = observed_inventories(output, manifest["plan"], source, records, deadline)
        write_command_records(output, records); write_json(output / "source-checks.json", {"schema_version": 1, "kind": "minimal-small-linear-algebra-source-checks", "cells": source_checks}); write_json(output / "prerequisite-discovery.json", {"schema_version": 1, "kind": "minimal-small-linear-algebra-prerequisite-discovery", "cells": discovery})
        write_json(output / "observed-inventories.json", inventories); write_json(output / "certificate-index.json", {"schema_version": 1, "kind": "minimal-small-linear-algebra-certificate-index", "entries": entries})
        comparison = compare_index(profile, output / "certificate-index.json"); write_json(output / "cross-cell-comparison.json", comparison)
        gate_summary = {"schema_version": 1, "kind": "minimal-small-linear-algebra-gate-summary", "state": "EVIDENCE_COLLECTED_PENDING_AUDIT", "gates": {gate: "EVIDENCE_COLLECTED_PENDING_AUDIT" for gate in profile["gates"]}, "limitations": manifest["retained_limitations"]}
        write_json(output / "gate-summary.json", gate_summary)
        write_terminal(output, manifest, "EXECUTED_PENDING_AUDIT", {"comparison": "cross-cell-comparison.json", "gate_summary": "gate-summary.json", "source_checks": "source-checks.json", "prerequisite_discovery": "prerequisite-discovery.json", "observed_inventories": "observed-inventories.json"})
        write_state(output, "EXECUTED_PENDING_AUDIT", {"candidate_commit": manifest["candidate"]["commit"], "comparison": "cross-cell-comparison.json"})
    except Exception as error:
        write_command_records(output, records)
        write_json(output / "failure.json", {"schema_version": 2, "kind": "minimal-small-linear-algebra-failure", "message": str(error), "records": records, "partial_certificate_slots": entries})
        write_terminal(output, manifest, "BLOCKED", {"failure": "failure.json"})
        write_state(output, "BLOCKED", {"candidate_commit": manifest["candidate"]["commit"], "closure_failure": True, "failure": "failure.json"})
        seal_output(output, source, manifest, TERMINAL_REQUIRED_FAILURE)
        raise
    seal_output(output, source, manifest, TERMINAL_REQUIRED_SUCCESS)


def verify_retention(arguments: argparse.Namespace) -> None:
    root = pathlib.Path(arguments.output_root).resolve(); retention = read_json(root / "retention-manifest.json")
    manifest = validate_preparation_chain(root, require_prepared=False)
    required = {"schema_version", "kind", "candidate_commit", "prepared_manifest_sha256", "required_paths", "files"}
    if not isinstance(retention, dict) or set(retention) != required or retention["schema_version"] != 3 or retention["kind"] != "minimal-small-linear-algebra-retention" or not isinstance(retention["files"], list) or not isinstance(retention["required_paths"], list):
        raise fail("retention manifest schema differs")
    prepared = root / "prepared-manifest.json"
    if not prepared.is_file() or retention["prepared_manifest_sha256"] != sha256_file(prepared): raise fail("retention prepared binding differs")
    terminal = read_json(root / "terminal-manifest.json")
    state = read_json(root / "state.json")
    claim = read_json(root / "execution-claim.json")
    if retention["candidate_commit"] != manifest["candidate"]["commit"] or terminal.get("candidate") != manifest["candidate"] or terminal.get("prepared_manifest_sha256") != retention["prepared_manifest_sha256"] or state["state"] != terminal["state"]:
        raise fail("retained candidate or terminal identity differs")
    if claim.get("candidate_commit") != retention["candidate_commit"] or claim.get("prepared_manifest_sha256") != retention["prepared_manifest_sha256"] or not isinstance(claim.get("pid"), int) or claim["pid"] <= 0:
        raise fail("execution claim identity differs")
    terminal_state = terminal.get("state")
    expected_required = TERMINAL_REQUIRED_SUCCESS if terminal_state == "EXECUTED_PENDING_AUDIT" else TERMINAL_REQUIRED_FAILURE if terminal_state == "BLOCKED" else None
    sealing_failure = terminal.get("retention_failure")
    if sealing_failure is not None:
        if terminal_state != "BLOCKED" or sealing_failure != "retention-error.json": raise fail("seal failure terminal differs")
        expected_required = TERMINAL_REQUIRED_FAILURE | SEAL_FAILURE_PATHS
    if expected_required is None or set(retention["required_paths"]) != expected_required:
        raise fail("retention terminal requirement set differs")
    actual: set[str] = set(); listed: set[str] = {"retention-manifest.json"}
    for entry in retention["files"]:
        if not isinstance(entry, dict) or set(entry) != {"path", "sha256", "size"}: raise fail("retention entry schema differs")
        path = (root / entry["path"]).resolve()
        if root not in path.parents or not path.is_file() or path.stat().st_size != entry["size"] or sha256_file(path) != entry["sha256"]: raise fail("retention entry differs")
        if entry["path"] in listed: raise fail("duplicate retained path")
        listed.add(entry["path"])
    for path in root.rglob("*"):
        if path.is_file() and "/build/" not in path.relative_to(root).as_posix(): actual.add(path.relative_to(root).as_posix())
    if actual != listed or not set(retention["required_paths"]).issubset(actual): raise fail("retention file set differs")
    records = read_json(root / "command-records.json")["records"]
    if len({row["id"] for row in records}) != len(records): raise fail("duplicate command identity")
    for row in records:
        for stream in ("stdout", "stderr"):
            if row[stream]["path"] not in listed or sha256_file(root / row[stream]["path"]) != row[stream]["sha256"]:
                raise fail("command log binding differs")
    detached = read_json(root / "detached-verification.json")
    if sealing_failure:
        validate_sealing_failure(root, manifest, detached)
    elif detached.get("result") != "PASS" or detached.get("candidate_commit") != manifest["candidate"]["commit"] or detached.get("source_inventory_count") != len(manifest["candidate"]["source_inventory"]):
        raise fail("detached candidate evidence differs")
    conditions = {"always", "success" if terminal_state == "EXECUTED_PENDING_AUDIT" else "failure"}
    if sealing_failure: conditions.add("seal-failure")
    required_artifacts = {row["path"] for row in manifest["planned_inventories"]["artifacts"] if row["required_when"] in conditions}
    if not required_artifacts.issubset(listed): raise fail("planned conditional artifacts are incomplete")
    if terminal_state == "BLOCKED":
        failure = read_json(root / "failure.json")
        if terminal.get("failure") != "failure.json" or not failure.get("message") or failure.get("records") != records: raise fail("terminal failure evidence differs")
    else:
        profile_path = root / "profile.json"
        if read_json(root / "cross-cell-comparison.json") != compare_index(validate_profile(profile_path), root / "certificate-index.json"):
            raise fail("retained comparison recomputation differs")
        profile = validate_profile(profile_path)
        for cell in manifest["plan"]:
            validate_negative_cell(root, manifest, cell, records, profile)


def validate_negative_cell(root: pathlib.Path, manifest: dict[str, Any], cell: dict[str, Any], records: list[dict[str, Any]], profile: dict[str, Any]) -> None:
    name = cell["cell"]
    matches = [row for row in records if row["id"] == f"{name}-negative-outcomes"]
    if len(matches) != 1 or matches[0]["argv"] != replace_root(cell["negative_outcomes"], pathlib.Path(manifest["output_root"])):
        raise fail("negative command provenance differs")
    require_success(matches[0], f"{name} retained negatives")
    validate_negative_outcomes(profile, root / "negatives" / name / "negative-outcomes.json", sha256_file(root / "certificates" / f"{name}-1.json"), manifest["inputs"]["validator"]["sha256"])


def validate_sealing_failure(root: pathlib.Path, manifest: dict[str, Any], detached: dict[str, Any]) -> None:
    failure = read_json(root / "retention-error.json")
    keys = {"schema_version", "kind", "candidate_commit", "stage", "error_type", "message", "retry_attempted", "snapshots"}
    if set(failure) != keys or failure["schema_version"] != 1 or failure["kind"] != "minimal-small-linear-algebra-seal-failure" or failure["candidate_commit"] != manifest["candidate"]["commit"] or failure["stage"] not in {"detached-verification", "inventory", "verification"} or failure["retry_attempted"] is not False or not failure["message"] or not failure["error_type"]:
        raise fail("seal failure evidence differs")
    expected_detached = {"schema_version": 1, "kind": "minimal-small-linear-algebra-detached-verification", "result": "BLOCKED", "candidate_commit": manifest["candidate"]["commit"], "retention_failure": "retention-error.json"}
    if detached != expected_detached: raise fail("failed seal was presented as detached success")
    snapshots = failure["snapshots"]
    allowed = {"pre-seal-terminal.json", "pre-seal-detached.json", "pre-seal-retention.json"}
    if not isinstance(snapshots, dict) or set(snapshots) != {name for name in allowed if (root / name).is_file()} or "pre-seal-terminal.json" not in snapshots:
        raise fail("seal snapshot inventory differs")
    for name, digest in snapshots.items():
        if sha256_file(root / name) != digest: raise fail("seal snapshot binding differs")
    previous = read_json(root / "pre-seal-terminal.json")
    if previous.get("candidate") != manifest["candidate"] or previous.get("prepared_manifest_sha256") != sha256_file(root / "prepared-manifest.json") or previous.get("state") not in {"BLOCKED", "EXECUTED_PENDING_AUDIT"} or "retention_failure" in previous:
        raise fail("pre-seal terminal differs")
    if failure["stage"] in {"inventory", "verification"}:
        if "pre-seal-detached.json" not in snapshots: raise fail("pre-seal detached evidence is absent")
        prior = read_json(root / "pre-seal-detached.json")
        if prior.get("result") != "PASS" or prior.get("candidate_commit") != manifest["candidate"]["commit"] or prior.get("source_inventory_count") != len(manifest["candidate"]["source_inventory"]):
            raise fail("pre-seal detached evidence differs")
    if failure["stage"] == "verification" and "pre-seal-retention.json" not in snapshots: raise fail("failed retention seal is absent")


def self_check(arguments: argparse.Namespace) -> None:
    source = pathlib.Path(arguments.source_root).resolve(); profile = validate_profile(canonical(source, "experiments/profiles/minimal_small_linear_algebra.json", arguments.profile, "profile")); inputs(arguments, source)
    plan = command_plan(profile, source)
    if len(plan) != 4 or len(expected_slots(profile)) != 12 or any("prerequisite_discovery" not in cell for cell in plan): raise fail("fixed qualification matrix differs")
    if "." not in profile["exact_prerequisite_tests"][0] or "\\." not in ctest_regex(profile["exact_prerequisite_tests"]): raise fail("prerequisite selector is not exact")
    validate_prerequisite_discovery(list(profile["exact_prerequisite_tests"]), profile["exact_prerequisite_tests"])


def main() -> int:
    parser = argparse.ArgumentParser(); parser.add_argument("--source-root", required=True); parser.add_argument("--profile", required=True); parser.add_argument("--protocol", required=True); parser.add_argument("--output-root", required=False)
    commands = parser.add_subparsers(dest="command", required=True); commands.add_parser("self-check"); commands.add_parser("prepare"); commands.add_parser("execute"); commands.add_parser("verify-retention")
    arguments = parser.parse_args()
    try:
        if arguments.command == "self-check": self_check(arguments)
        elif arguments.command == "prepare":
            if not arguments.output_root: raise fail("prepare requires --output-root")
            prepare(arguments)
        elif arguments.command == "execute":
            if not arguments.output_root: raise fail("execute requires --output-root")
            execute(arguments)
        else:
            if not arguments.output_root: raise fail("verify-retention requires --output-root")
            verify_retention(arguments)
        return 0
    except (RuntimeErrorEvidence, EvidenceError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
