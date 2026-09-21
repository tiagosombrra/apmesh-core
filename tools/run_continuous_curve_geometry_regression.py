#!/usr/bin/env python3
"""Report-only planning and validation for Continuous Curve Geometry Regression."""

from __future__ import annotations

import argparse
import hashlib
import json
import pathlib
import re
import subprocess
import sys
from typing import Any

TOOL_ROOT = pathlib.Path(__file__).resolve().parent
if str(TOOL_ROOT) not in sys.path:
    sys.path.insert(0, str(TOOL_ROOT))

from experiment_runtime import input_identity, read_json, sha256_file, write_json
from continuous_curve_geometry_regression_evidence import EvidenceError, validate_profile

SEMANTIC_BASELINE = "438620efa1f93d29b442e9ba199882a09d2359d9"
INPUT_PATHS = {
    "profile": "experiments/profiles/continuous_curve_geometry_regression.json",
    "protocol": "docs/decisions/CURVE_CONTINUOUS_GEOMETRY_REGRESSION_PROTOCOL.md",
    "exporter": "experiments/continuous_curve_geometry_regression_export.cpp",
    "validator": "tools/continuous_curve_geometry_regression_evidence.py",
    "negative": "tools/continuous_curve_geometry_regression_negative.py",
    "runner": "tools/run_continuous_curve_geometry_regression.py",
    "runtime": "tools/experiment_runtime.py",
    "cmake": "CMakeLists.txt",
}
EXPECTED_DERIVED_FILES = [
    "curve-value-reference.csv",
    "curve-value-reference.svg",
    "curve-differential-speed.csv",
    "curve-differential-speed.svg",
    "curve-arc-length-enclosure.csv",
    "curve-arc-length-enclosure.svg",
    "curve-inverse-bracket.csv",
    "curve-inverse-bracket.svg",
    "derived-evidence.json",
]


class RunnerError(RuntimeError):
    pass


def fail(message: str) -> RunnerError:
    return RunnerError(message)


def canonical(source: pathlib.Path, relative: str, supplied: str, label: str) -> pathlib.Path:
    expected = (source / relative).resolve()
    actual = pathlib.Path(supplied).resolve()
    if expected != actual:
        raise fail(f"{label} must resolve to the candidate path")
    return expected


def input_paths(arguments: argparse.Namespace, source: pathlib.Path) -> dict[str, pathlib.Path]:
    supplied = {
        "profile": arguments.profile,
        "protocol": arguments.protocol,
        "exporter": arguments.exporter,
        "validator": arguments.validator,
        "negative": arguments.negative,
    }
    if pathlib.Path(__file__).resolve() != (source / INPUT_PATHS["runner"]).resolve():
        raise fail("runner must execute from the candidate path")
    paths = {
        name: canonical(source, INPUT_PATHS[name], value, name)
        for name, value in supplied.items()
    }
    for name, relative in INPUT_PATHS.items():
        paths.setdefault(name, (source / relative).resolve())
    if any(not path.is_file() for path in paths.values()):
        raise fail("a required candidate input is absent")
    return paths


def candidate_identity(source: pathlib.Path) -> dict[str, Any]:
    head = subprocess.run(
        ["git", "rev-parse", "HEAD"],
        cwd=source,
        capture_output=True,
        text=True,
        check=False,
    )
    if head.returncode != 0:
        raise fail("candidate commit could not be resolved")
    commit = head.stdout.strip()
    if re.fullmatch(r"[0-9a-f]{40}", commit) is None:
        raise fail("candidate commit identity differs")
    status = subprocess.run(
        ["git", "status", "--porcelain", "--untracked-files=no"],
        cwd=source,
        capture_output=True,
        text=True,
        check=False,
    )
    if status.returncode != 0 or status.stdout:
        raise fail("tracked candidate inputs are not clean")
    ancestor = subprocess.run(
        ["git", "merge-base", "--is-ancestor", SEMANTIC_BASELINE, commit],
        cwd=source,
        capture_output=True,
        text=True,
        check=False,
    )
    if ancestor.returncode != 0:
        raise fail("candidate is not a descendant of the semantic baseline")
    return {"commit": commit, "tree_clean": True, "semantic_baseline": SEMANTIC_BASELINE}


def git_blob(source: pathlib.Path, revision: str, relative: str) -> bytes:
    result = subprocess.run(
        ["git", "show", f"{revision}:{relative}"],
        cwd=source,
        capture_output=True,
        check=False,
    )
    if result.returncode != 0:
        raise fail(f"baseline semantic file is unavailable: {relative}")
    return result.stdout


def frozen_semantic_identity(source: pathlib.Path, profile: dict[str, Any]) -> list[dict[str, str]]:
    rows: list[dict[str, str]] = []
    for relative in profile["frozen_semantic_files"]:
        current = (source / relative).read_bytes()
        baseline = git_blob(source, profile["semantic_baseline"], relative)
        current_hash = hashlib.sha256(current).hexdigest()
        baseline_hash = hashlib.sha256(baseline).hexdigest()
        if current_hash != baseline_hash:
            raise fail(f"frozen semantic file drifted from protocol baseline: {relative}")
        rows.append(
            {
                "path": relative,
                "sha256": current_hash,
                "baseline_sha256": baseline_hash,
            }
        )
    return rows


def declared_allowlist(source: pathlib.Path, expected: list[str]) -> list[str]:
    content = (source / "CMakeLists.txt").read_text(encoding="utf-8")
    names = re.findall(r"add_test\s*\(\s*NAME\s+([^\s)]+)", content)
    selected = [name for name in names if name in expected]
    if len(selected) != len(set(selected)):
        raise fail("declared semantic CTest allowlist contains duplicates")
    if set(selected) != set(expected) or len(selected) != len(expected):
        raise fail(
            f"declared semantic CTest allowlist differs: expected={expected}, observed={selected}"
        )
    return list(expected)


def protocol_check(path: pathlib.Path) -> None:
    content = path.read_text(encoding="utf-8")
    required = (
        "Status: **PRE-REGISTERED / INTEGRATED / DOCUMENTATION-ONLY / NO FORMAL EXECUTION AUTHORIZED**",
        "## 4. Frozen semantic baseline",
        SEMANTIC_BASELINE,
        "## 5. Exact semantic CTest allowlist",
        "**56 command records total**",
        "**8 repetitions × 14 tests = 112 individual semantic test executions.**",
        "## 14. CGR0 — Identity and scope",
        "## 21. CGR7 — Evidence integrity and closure",
        "No formal campaign is authorized by this protocol document.",
        "## 22. Formal lifecycle",
    )
    if any(token not in content for token in required):
        raise fail("protocol is not the pre-registered CGR0-CGR7 authority")


def ctest_regex(names: list[str]) -> str:
    return "^(" + "|".join(re.escape(name) for name in names) + ")$"


def command_plan(
    profile: dict[str, Any],
    source: pathlib.Path,
    declared: list[str],
) -> list[dict[str, Any]]:
    plan: list[dict[str, Any]] = []
    for cell in profile["cells"]:
        if cell["id"].startswith("gcc-"):
            compiler = "/usr/bin/g++-13"
            use_libcxx = "OFF"
        else:
            compiler = "/usr/bin/clang++-18"
            use_libcxx = "ON"
        build = f"@REPORT_ROOT@/cells/{cell['id']}/build"
        repetitions: list[dict[str, Any]] = []
        for repetition in range(1, profile["repetitions_per_cell"] + 1):
            certificate = (
                f"@REPORT_ROOT@/certificates/{cell['id']}-{repetition}.json"
            )
            repetitions.append(
                {
                    "repetition": repetition,
                    "build": ["/usr/bin/cmake", "--build", build, "--parallel"],
                    "ctest_discovery": ["/usr/bin/ctest", "--test-dir", build, "-N"],
                    "semantic_ctest": [
                        "/usr/bin/ctest",
                        "--test-dir",
                        build,
                        "--output-on-failure",
                        "-R",
                        ctest_regex(declared),
                    ],
                    "certificate": [
                        f"{build}/apmesh_core_continuous_curve_geometry_regression_export",
                        "certificate",
                        certificate,
                        cell["id"],
                        str(repetition),
                    ],
                    "certificate_validation": [
                        "/usr/bin/python3",
                        str(source / INPUT_PATHS["validator"]),
                        "--profile",
                        str(source / INPUT_PATHS["profile"]),
                        "validate-certificate",
                        "--certificate",
                        certificate,
                    ],
                }
            )
        plan.append(
            {
                "cell": cell["id"],
                "compiler": compiler,
                "library": cell["library"],
                "build_type": cell["build_type"],
                "configure": [
                    "/usr/bin/cmake",
                    "-S",
                    str(source),
                    "-B",
                    build,
                    "-G",
                    "Ninja",
                    "-DCMAKE_MAKE_PROGRAM=/usr/bin/ninja",
                    f"-DCMAKE_CXX_COMPILER={compiler}",
                    f"-DCMAKE_BUILD_TYPE={cell['build_type']}",
                    f"-DAPMESH_USE_LIBCXX={use_libcxx}",
                    "-DBUILD_TESTING=ON",
                    "-DAPMESH_ENABLE_QUALIFICATION_TESTS=ON",
                    "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
                ],
                "repetitions": repetitions,
                "negative_evidence": [
                    "/usr/bin/python3",
                    str(source / INPUT_PATHS["negative"]),
                    "--validator",
                    str(source / INPUT_PATHS["validator"]),
                    "--profile",
                    str(source / INPUT_PATHS["profile"]),
                    "generate",
                    "--certificate",
                    f"@REPORT_ROOT@/certificates/{cell['id']}-1.json",
                    "--output",
                    f"@REPORT_ROOT@/negatives/{cell['id']}.json",
                ],
                "dependency_inventory": [
                    "/usr/bin/ninja",
                    "-C",
                    build,
                    "-t",
                    "deps",
                ],
                "runtime_dependencies": [
                    f"{build}/apmesh_core_continuous_curve_geometry_regression_export"
                ],
            }
        )
    return plan


def command_ids(plan: list[dict[str, Any]]) -> list[str]:
    identifiers: list[str] = []
    for cell in plan:
        name = cell["cell"]
        identifiers.append(f"{name}-configure")
        for repetition in cell["repetitions"]:
            ordinal = repetition["repetition"]
            for stage in (
                "build",
                "ctest-discovery",
                "semantic-ctest",
                "certificate",
                "certificate-validation",
            ):
                identifiers.append(f"{name}-{stage}-{ordinal}")
        identifiers.extend(
            [
                f"{name}-negative-evidence",
                f"{name}-dependency-inventory",
                f"{name}-runtime-dependencies",
            ]
        )
    return identifiers


def simulation(
    profile: dict[str, Any],
    plan: list[dict[str, Any]],
    frozen: list[dict[str, str]],
) -> dict[str, Any]:
    identifiers = command_ids(plan)
    logs = [
        f"logs/{identifier}.{stream}.log"
        for identifier in identifiers
        for stream in ("stdout", "stderr")
    ]
    certificate_slots = [
        {
            "cell": cell["id"],
            "repetition": repetition,
            "path": f"certificates/{cell['id']}-{repetition}.json",
        }
        for cell in profile["cells"]
        for repetition in range(1, profile["repetitions_per_cell"] + 1)
    ]
    return {
        "schema_version": 1,
        "kind": "continuous-curve-geometry-report-only-simulation",
        "execution_requested": False,
        "formal_preparation": False,
        "formal_execution": False,
        "gates": {gate: "NOT_EXECUTED" for gate in profile["gates"]},
        "command_record_count": len(identifiers),
        "command_ids": identifiers,
        "command_log_count": len(logs),
        "command_logs": logs,
        "discovery_record_count": 8,
        "semantic_ctest_record_count": 8,
        "semantic_test_execution_count": 112,
        "certificate_count": len(certificate_slots),
        "certificate_slots": certificate_slots,
        "derived_evidence_files": EXPECTED_DERIVED_FILES,
        "frozen_semantic_files": frozen,
    }


def validate_simulation(profile: dict[str, Any], value: dict[str, Any]) -> None:
    expected_keys = {
        "schema_version",
        "kind",
        "execution_requested",
        "formal_preparation",
        "formal_execution",
        "gates",
        "command_record_count",
        "command_ids",
        "command_log_count",
        "command_logs",
        "discovery_record_count",
        "semantic_ctest_record_count",
        "semantic_test_execution_count",
        "certificate_count",
        "certificate_slots",
        "derived_evidence_files",
        "frozen_semantic_files",
    }
    if set(value) != expected_keys:
        raise fail("report-only simulation schema differs")
    if value["schema_version"] != 1 or value["kind"] != "continuous-curve-geometry-report-only-simulation":
        raise fail("report-only simulation identity differs")
    if value["execution_requested"] is not False or value["formal_preparation"] is not False or value["formal_execution"] is not False:
        raise fail("report-only simulation claims a formal lifecycle")
    if value["gates"] != {gate: "NOT_EXECUTED" for gate in profile["gates"]}:
        raise fail("report-only simulation claims a CGR gate result")
    identifiers = value["command_ids"]
    logs = value["command_logs"]
    if value["command_record_count"] != 56 or not isinstance(identifiers, list) or len(identifiers) != 56 or len(set(identifiers)) != 56:
        raise fail("report-only command cardinality differs")
    if value["command_log_count"] != 112 or not isinstance(logs, list) or len(logs) != 112 or len(set(logs)) != 112:
        raise fail("report-only command-log cardinality differs")
    if value["discovery_record_count"] != 8 or value["semantic_ctest_record_count"] != 8:
        raise fail("report-only repetition evidence cardinality differs")
    if value["semantic_test_execution_count"] != 112:
        raise fail("report-only semantic test cardinality differs")
    slots = value["certificate_slots"]
    if value["certificate_count"] != 8 or not isinstance(slots, list) or len(slots) != 8:
        raise fail("report-only certificate cardinality differs")
    observed_slots = [(item["cell"], item["repetition"]) for item in slots]
    expected_slots = [
        (cell["id"], repetition)
        for cell in profile["cells"]
        for repetition in range(1, profile["repetitions_per_cell"] + 1)
    ]
    if observed_slots != expected_slots:
        raise fail("report-only certificate slots differ")
    if value["derived_evidence_files"] != EXPECTED_DERIVED_FILES:
        raise fail("derived evidence plan differs")
    frozen = value["frozen_semantic_files"]
    if not isinstance(frozen, list) or [item.get("path") for item in frozen] != profile["frozen_semantic_files"]:
        raise fail("frozen semantic inventory differs")
    if any(item.get("sha256") != item.get("baseline_sha256") for item in frozen):
        raise fail("frozen semantic baseline differs")


def self_check(arguments: argparse.Namespace) -> dict[str, Any]:
    source = pathlib.Path(arguments.source_root).resolve()
    paths = input_paths(arguments, source)
    profile = validate_profile(paths["profile"])
    protocol_check(paths["protocol"])
    candidate = candidate_identity(source)
    declared = declared_allowlist(source, profile["semantic_ctest_allowlist"])
    frozen = frozen_semantic_identity(source, profile)
    plan = command_plan(profile, source, declared)
    report = simulation(profile, plan, frozen)
    validate_simulation(profile, report)
    return {
        "schema_version": 1,
        "kind": "continuous-curve-geometry-regression-runner-self-check",
        "status": "PASS",
        "execution_requested": False,
        "formal_preparation": False,
        "formal_execution": False,
        "candidate": candidate,
        "input_hashes": input_identity(paths),
        "declared_semantic_ctest_allowlist": declared,
        "frozen_semantic_files": frozen,
        "plan": {
            "schema_version": 1,
            "kind": "continuous-curve-geometry-report-only-plan",
            "execution_requested": False,
            "cells": plan,
            "gates": {gate: "NOT_EXECUTED" for gate in profile["gates"]},
            "limitations": profile["limitations"],
        },
        "simulation": report,
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    for name in ("source-root", "profile", "protocol", "exporter", "validator", "negative"):
        parser.add_argument(f"--{name}", required=True)
    commands = parser.add_subparsers(dest="command", required=True)
    commands.add_parser("self-check").add_argument("--output", required=True)
    commands.add_parser("plan").add_argument("--output", required=True)
    commands.add_parser("simulate").add_argument("--output", required=True)
    validate = commands.add_parser("validate-simulation")
    validate.add_argument("--simulation", required=True)
    arguments = parser.parse_args()

    try:
        result = self_check(arguments)
        if arguments.command == "self-check":
            write_json(pathlib.Path(arguments.output), result)
            return 0
        if arguments.command == "plan":
            write_json(pathlib.Path(arguments.output), result["plan"])
            return 0
        if arguments.command == "simulate":
            write_json(pathlib.Path(arguments.output), result["simulation"])
            return 0
        profile = validate_profile(pathlib.Path(arguments.profile))
        validate_simulation(profile, read_json(pathlib.Path(arguments.simulation)))
        return 0
    except (RunnerError, EvidenceError, OSError, ValueError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
