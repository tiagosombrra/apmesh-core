#!/usr/bin/env python3
"""Revision-bound CF0-CF7 lifecycle tooling; it never selects a scientific outcome."""
from __future__ import annotations

import argparse
import hashlib
import pathlib
import re
import shutil
import subprocess
import sys
from typing import Any

from experiment_runtime import (RuntimeErrorEvidence, clean_candidate, input_identity,
    read_json, retention_manifest, run_command, sha256_file, tool_version, utc_now,
    verify_input_identity, write_json, write_state)

CONFIGURATIONS = {"gcc-debug": ("g++-13", False), "gcc-release": ("g++-13", False),
                  "clang-debug": ("clang++-18", True), "clang-release": ("clang++-18", True)}
BUILD_TIMEOUT_SECONDS, PROCESS_TIMEOUT_SECONDS = 600, 60
REQUIRED_FOCUSED = ["apmesh_core.coordinate_frames", "apmesh_core.cartesian_frames_evidence",
                    "apmesh_core.cartesian_frames_runner", "apmesh_core.cartesian_frames_retention"]


def fail(message: str) -> RuntimeErrorEvidence: return RuntimeErrorEvidence(message)


def canonical(source: pathlib.Path, relative: str, supplied: str, label: str) -> pathlib.Path:
    expected, actual = (source / relative).resolve(), pathlib.Path(supplied).resolve()
    if expected != actual: raise fail(f"{label} is not the canonical repository input")
    return actual


def published_candidate(source: pathlib.Path) -> dict[str, Any]:
    candidate = clean_candidate(source)
    upstream = subprocess.run(["git", "rev-parse", "@{upstream}"], cwd=source, text=True, capture_output=True, check=False)
    if upstream.returncode or upstream.stdout.strip() != candidate["commit"]:
        raise fail("candidate is not published at its exact upstream revision")
    return candidate


def input_paths(args: argparse.Namespace, source: pathlib.Path) -> dict[str, pathlib.Path]:
    relative = {"profile": "experiments/profiles/cartesian_frames.json", "protocol": "docs/decisions/GEOMETRY_CARTESIAN_FRAMES_QUALIFICATION_PROTOCOL.md", "validator": "tools/cartesian_frames_evidence.py", "runner": "tools/run_cartesian_frames_qualification.py", "runtime": "tools/experiment_runtime.py", "exporter_source": "experiments/cartesian_frames_export.cpp", "focused_contract": "tests/coordinate_frames.cpp", "header": "include/apmesh/core/geometry.hpp", "implementation": "src/core/geometry.cpp", "cmake": "CMakeLists.txt", "entry_authority": "docs/decisions/GEOMETRY_TRANSFORMATIONS_COORDINATE_FRAMES_ENTRY_DECISION.md", "roadmap": "docs/APMESH_CORE_ROADMAP.md", "state": "docs/APMESH_CORE_STATE.md"}
    supplied = {"profile": args.profile, "protocol": args.protocol, "validator": args.validator}
    paths = {name: canonical(source, value, supplied[name], name) if name in supplied else (source / value).resolve() for name, value in relative.items()}
    if any(not path.is_file() for path in paths.values()): raise fail("a required revision-bound input is missing")
    return paths


def validate_profile(path: pathlib.Path) -> dict[str, Any]:
    profile = read_json(path)
    if profile.get("status") != "report_only_infrastructure" or profile.get("repetitions_per_cell") != 3: raise fail("profile is not the fixed report-only three-repeat definition")
    if [item.get("id") for item in profile.get("cells", [])] != ["gcc-debug", "gcc-release", "clang-debug", "clang-release"]: raise fail("profile must declare the exact four-cell matrix")
    if profile.get("gates") != [f"CF{number}" for number in range(8)]: raise fail("profile must declare CF0 through CF7")
    tests = profile.get("exact_prerequisite_tests")
    if not isinstance(tests, list) or len(tests) != len(set(tests)) or len(tests) < 20 or any(not item.startswith("apmesh_core.") for item in tests): raise fail("profile prerequisite allowlist is not exact and complete")
    return profile


def ctest_regex(names: list[str]) -> str: return "^(" + "|".join(name.replace(".", "\\.") for name in names) + ")$"


def command_plan(profile: dict[str, Any], source: pathlib.Path) -> list[dict[str, Any]]:
    plan: list[dict[str, Any]] = []
    for cell in profile["cells"]:
        compiler, libcxx = CONFIGURATIONS[cell["id"]]; build = f"@OUTPUT_ROOT@/cells/{cell['id']}/build"
        plan.append({"cell": cell["id"],
                     "configure": ["cmake", "-S", str(source), "-B", build, "-G", "Ninja", f"-DCMAKE_CXX_COMPILER={compiler}", f"-DCMAKE_BUILD_TYPE={cell['build_type']}", f"-DAPMESH_USE_LIBCXX={'ON' if libcxx else 'OFF'}", "-DBUILD_TESTING=ON", "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"],
                     "build": ["cmake", "--build", build], "discovery": ["ctest", "--test-dir", build, "-N"],
                     "focused_ctest": ["ctest", "--test-dir", build, "--output-on-failure", "-R", ctest_regex(REQUIRED_FOCUSED)],
                     "prerequisite_ctest": ["ctest", "--test-dir", build, "--output-on-failure", "-R", ctest_regex(profile["exact_prerequisite_tests"])],
                     "exporter": f"{build}/apmesh_core_cartesian_frames_export", "repetitions": 3})
    return plan


def expected_slots(profile: dict[str, Any]) -> list[dict[str, Any]]:
    return [{"cell": cell["id"], "repetition": repeat} for cell in profile["cells"] for repeat in range(1, 4)]


def planned_inventory(plan: list[dict[str, Any]], candidate: dict[str, Any]) -> dict[str, Any]:
    artifacts = [{"path": name, "role": "control", "required_when": "always"} for name in ("prepared-manifest.json", "plan.json", "planned-inventory.json", "state.json", "state-history.jsonl", "preparation-seal.json")]
    artifacts += [{"path": name, "role": role, "required_when": "after-execution"} for name, role in (("terminal-manifest.json", "terminal"), ("command-records.json", "commands"), ("retention-manifest.json", "retention"), ("detached-verification.json", "detached"))]
    for cell in plan:
        for repeat in range(1, 4): artifacts.append({"path": f"certificates/{cell['cell']}-{repeat}.json", "role": "semantic-certificate", "required_when": "success"})
        for stage in ("configure", "build", "discovery", "focused_ctest", "prerequisite_ctest"):
            artifacts += [{"path": f"logs/{cell['cell']}-{stage}.{stream}.log", "role": "command-log", "required_when": "started"} for stream in ("stdout", "stderr")]
    return {"schema_version": 1, "kind": "cartesian-frames-planned-inventory", "candidate_source": candidate["source_inventory"], "artifacts": artifacts}


def preparation_files() -> tuple[str, ...]: return ("prepared-manifest.json", "plan.json", "planned-inventory.json", "profile.json")


def seal_preparation(root: pathlib.Path) -> None:
    write_json(root / "preparation-seal.json", {"schema_version": 1, "kind": "cartesian-frames-preparation-seal", "files": {name: sha256_file(root / name) for name in preparation_files()}})


def validate_prepared(root: pathlib.Path, require_unused: bool = True) -> dict[str, Any]:
    manifest, seal, state = read_json(root / "prepared-manifest.json"), read_json(root / "preparation-seal.json"), read_json(root / "state.json")
    if manifest.get("state") != "PREPARED" or manifest.get("execution_requested") is not False: raise fail("prepared manifest lifecycle differs")
    if seal.get("files") != {name: sha256_file(root / name) for name in preparation_files()}: raise fail("prepared input seal differs")
    permitted = {"PREPARED"} if require_unused else {"PREPARED", "RUNNING", "EXECUTED_PENDING_AUDIT", "BLOCKED"}
    if state.get("state") not in permitted or state.get("detail", {}).get("candidate_commit") != manifest.get("candidate", {}).get("commit"): raise fail("prepared lifecycle identity differs")
    if read_json(root / "plan.json") != {"schema_version": 1, "kind": "cartesian-frames-launch-plan", "cells": manifest["plan"]}: raise fail("prepared plan differs")
    if read_json(root / "planned-inventory.json") != manifest["planned_inventory"]: raise fail("planned inventory differs")
    if require_unused and (root / "terminal-manifest.json").exists(): raise fail("prepared attempt is already consumed")
    return manifest


def prepare(args: argparse.Namespace) -> None:
    source, output = pathlib.Path(args.source_root).resolve(), pathlib.Path(args.output_root).resolve()
    if output.exists(): raise fail("output root already exists")
    paths = input_paths(args, source); profile = validate_profile(paths["profile"]); candidate = published_candidate(source); plan = command_plan(profile, source)
    output.mkdir(parents=True); shutil.copyfile(paths["profile"], output / "profile.json")
    manifest = {"schema_version": 1, "kind": "cartesian-frames-prepared-manifest", "state": "PREPARED", "execution_requested": False, "candidate": candidate, "inputs": input_identity(paths), "environment": {"cmake": tool_version("cmake"), "ctest": tool_version("ctest"), "ninja": tool_version("ninja"), "python": tool_version("python3")}, "working_directory": str(source), "output_root": str(output), "plan": plan, "expected_slots": expected_slots(profile), "planned_inventory": planned_inventory(plan, candidate), "gates": {gate: "NOT_EXECUTED" for gate in profile["gates"]}, "limitations": profile["limitations"], "prepared_utc": utc_now()}
    write_json(output / "prepared-manifest.json", manifest); write_json(output / "plan.json", {"schema_version": 1, "kind": "cartesian-frames-launch-plan", "cells": plan}); write_json(output / "planned-inventory.json", manifest["planned_inventory"])
    write_state(output, "PREPARED", {"candidate_commit": candidate["commit"], "execution_requested": False}); seal_preparation(output)


def replace_root(argv: list[str], root: pathlib.Path) -> list[str]: return [part.replace("@OUTPUT_ROOT@", str(root)) for part in argv]
def require_success(record: dict[str, Any], stage: str) -> None:
    if record.get("exit_code") != 0 or record.get("timed_out") or record.get("launch_error"): raise fail(f"{stage} failed")


def execute(args: argparse.Namespace) -> None:
    root = pathlib.Path(args.output_root).resolve(); manifest = validate_prepared(root); source = pathlib.Path(manifest["working_directory"]); paths = input_paths(args, source); verify_input_identity(manifest["inputs"], paths)
    write_state(root, "RUNNING", {"candidate_commit": manifest["candidate"]["commit"]}); records: list[dict[str, Any]] = []
    try:
        for cell in manifest["plan"]:
            for stage in ("configure", "build", "discovery", "focused_ctest", "prerequisite_ctest"):
                record = run_command(replace_root(cell[stage], root), source, root / "logs", f"{cell['cell']}-{stage}", BUILD_TIMEOUT_SECONDS); records.append(record); require_success(record, f"{cell['cell']}:{stage}")
            for repeat in range(1, 4):
                certificate = root / "certificates" / f"{cell['cell']}-{repeat}.json"; record = run_command([replace_root([cell["exporter"]], root)[0], str(certificate)], source, root / "logs", f"{cell['cell']}-certificate-{repeat}", PROCESS_TIMEOUT_SECONDS); records.append(record); require_success(record, f"{cell['cell']}:certificate-{repeat}")
                record = run_command([sys.executable, str(paths["validator"]), "validate-certificate", "--profile", str(paths["profile"]), "--certificate", str(certificate)], source, root / "logs", f"{cell['cell']}-certificate-{repeat}-validate", PROCESS_TIMEOUT_SECONDS); records.append(record); require_success(record, f"{cell['cell']}:certificate-{repeat}-validate")
        index = {"certificates": [{"cell": cell["cell"], "repetition": repeat, "path": str(root / "certificates" / f"{cell['cell']}-{repeat}.json")} for cell in manifest["plan"] for repeat in range(1, 4)]}
        write_json(root / "certificate-index.json", index)
        record = run_command([sys.executable, str(paths["validator"]), "compare", "--profile", str(paths["profile"]), "--index", str(root / "certificate-index.json"), "--report", str(root / "cross-cell-comparison.json")], source, root / "logs", "cross-cell-compare", PROCESS_TIMEOUT_SECONDS); records.append(record); require_success(record, "cross-cell-comparison")
        write_json(root / "command-records.json", {"schema_version": 1, "kind": "cartesian-frames-command-records", "records": records}); write_json(root / "terminal-manifest.json", {"schema_version": 1, "kind": "cartesian-frames-terminal-manifest", "state": "EXECUTED_PENDING_AUDIT", "candidate": manifest["candidate"], "prepared_manifest_sha256": sha256_file(root / "prepared-manifest.json"), "record_count": len(records), "execution_requested": True}); write_state(root, "EXECUTED_PENDING_AUDIT", {"candidate_commit": manifest["candidate"]["commit"]})
    except RuntimeErrorEvidence as error:
        write_json(root / "command-records.json", {"schema_version": 1, "kind": "cartesian-frames-command-records", "records": records}); write_json(root / "terminal-manifest.json", {"schema_version": 1, "kind": "cartesian-frames-terminal-manifest", "state": "BLOCKED", "candidate": manifest["candidate"], "prepared_manifest_sha256": sha256_file(root / "prepared-manifest.json"), "record_count": len(records), "failure": str(error)}); write_state(root, "BLOCKED", {"candidate_commit": manifest["candidate"]["commit"], "failure": str(error)}); raise


def verify_retention(args: argparse.Namespace) -> None:
    root = pathlib.Path(args.output_root).resolve(); manifest = validate_prepared(root, require_unused=False); terminal = read_json(root / "terminal-manifest.json")
    if terminal.get("candidate") != manifest.get("candidate") or terminal.get("state") not in {"BLOCKED", "EXECUTED_PENDING_AUDIT"}: raise fail("terminal identity differs")
    required = [(root / name, root / name) for name in ("prepared-manifest.json", "plan.json", "planned-inventory.json", "state.json", "state-history.jsonl", "preparation-seal.json", "terminal-manifest.json", "command-records.json")]
    retained = retention_manifest(root, required, manifest["candidate"]["commit"], None); write_json(root / "retention-manifest.json", retained)
    if any(not (root / row["retained_path"]).is_file() or sha256_file(root / row["retained_path"]) != row["sha256"] for row in retained["files"]): raise fail("retention inventory hash differs")


def verify_simple_retention(package: pathlib.Path) -> None:
    """Focused negative contract for SHA-bound retention entries."""
    record = read_json(package)
    if record.get("status") != "sealed" or record.get("formal_manifest") is not None or not isinstance(record.get("entries"), list):
        raise fail("simple retention schema differs")
    for entry in record["entries"]:
        path = pathlib.Path(entry.get("path", ""))
        if not path.is_file() or sha256_file(path) != entry.get("sha256"):
            raise fail("simple retention hash differs")


def self_check(args: argparse.Namespace) -> None:
    source = pathlib.Path(args.source_root).resolve(); paths = input_paths(args, source); profile = validate_profile(paths["profile"]); plan = command_plan(profile, source)
    if len(plan) != 4 or len(expected_slots(profile)) != 12: raise fail("fixed execution matrix differs")
    if "PREPARED" not in paths["protocol"].read_text(encoding="utf-8"): raise fail("protocol lacks prepared lifecycle")
    selector = ctest_regex(profile["exact_prerequisite_tests"])
    if any(re.fullmatch(selector, name) is None for name in profile["exact_prerequisite_tests"]): raise fail("allowlist regex is not exact")
    record = {"schema_version": 1, "kind": "cartesian-frames-runner-self-check", "status": "pass", "formal_manifest": None, "execution_requested": False, "matrix_slots": 12}
    if args.output_root: write_json(pathlib.Path(args.output_root), record)


def main() -> int:
    parser = argparse.ArgumentParser(); parser.add_argument("--source-root", required=True); parser.add_argument("--profile", required=True); parser.add_argument("--protocol", required=True); parser.add_argument("--validator", required=True); parser.add_argument("--exporter", required=True); parser.add_argument("--output-root"); parser.add_argument("--package")
    commands = parser.add_subparsers(dest="command", required=True)
    for name in ("self-check", "prepare", "execute"): commands.add_parser(name)
    commands.add_parser("verify-retention")
    raw = sys.argv[1:]
    if raw and raw[0] in {"self-check", "prepare", "execute", "verify-retention"}:
        raw = raw[1:] + [raw[0]]
    args = parser.parse_args(raw)
    try:
        if args.command == "self-check": self_check(args)
        elif args.command == "prepare":
            if not args.output_root: raise fail("prepare requires --output-root")
            prepare(args)
        elif args.command == "execute":
            if not args.output_root: raise fail("execute requires --output-root")
            execute(args)
        else:
            if args.package: verify_simple_retention(pathlib.Path(args.package))
            else:
                if not args.output_root: raise fail("verify-retention requires --output-root")
                verify_retention(args)
    except RuntimeErrorEvidence as error:
        print(f"cartesian-frames runner: {error}", file=sys.stderr); return 2
    return 0


if __name__ == "__main__": raise SystemExit(main())
