#!/usr/bin/env python3
"""Prepare, execute once, and retain formal CGR0-CGR7 evidence."""

from __future__ import annotations

import argparse
import hashlib
import importlib.util
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

from cloud_qualification_environment import (
    EnvironmentValidationError,
    validate as validate_cloud_environment,
)
from continuous_curve_geometry_regression_evidence import (
    EvidenceError,
    compare,
    derive,
    validate_certificate,
    validate_profile,
)
from experiment_runtime import (
    RuntimeErrorEvidence,
    clean_candidate,
    input_identity,
    read_json,
    relative_path,
    run_command,
    sha256_file,
    tool_version,
    utc_now,
    verify_input_identity,
    write_json,
    write_state,
)
import run_continuous_curve_geometry_regression as report_runner

SEMANTIC_BASELINE = report_runner.SEMANTIC_BASELINE
BUILD_TIMEOUT_SECONDS = 300
PROCESS_TIMEOUT_SECONDS = 60
OVERALL_TIMEOUT_SECONDS = 2400

PREPARATION_FILES = (
    "profile.json",
    "prepared-manifest.json",
    "plan.json",
    "planned-inventories.json",
)

INPUT_PATHS = {
    "profile": "experiments/profiles/continuous_curve_geometry_regression.json",
    "protocol": "docs/decisions/CURVE_CONTINUOUS_GEOMETRY_REGRESSION_PROTOCOL.md",
    "preparation_decision": "docs/decisions/CURVE_CONTINUOUS_GEOMETRY_REGRESSION_PREPARATION_DECISION.md",
    "exporter": "experiments/continuous_curve_geometry_regression_export.cpp",
    "validator": "tools/continuous_curve_geometry_regression_evidence.py",
    "negative": "tools/continuous_curve_geometry_regression_negative.py",
    "report_runner": "tools/run_continuous_curve_geometry_regression.py",
    "formal_runner": "tools/run_continuous_curve_geometry_campaign.py",
    "runtime": "tools/experiment_runtime.py",
    "cmake": "CMakeLists.txt",
    "cloud_profile": "experiments/profiles/cloud_qualification_environment.json",
    "cloud_validator": "tools/cloud_qualification_environment.py",
    "cloud_decision": "docs/decisions/CLOUD_QUALIFICATION_ENVIRONMENT_DECISION.md",
    "cloud_audit": "docs/audits/2026-09-20-cloud-qualification-environment-admission.md",
    "prepare_workflow": ".github/workflows/continuous-curve-geometry-regression-prepare.yml",
    "execute_workflow": ".github/workflows/continuous-curve-geometry-regression-execute.yml",
    "authorize_workflow": ".github/workflows/continuous-curve-geometry-regression-authorize.yml",
    "authorization_validator": "tools/continuous_curve_geometry_regression_authorization.py",
}

SUCCESS_CONTROL_FILES = {
    "profile.json",
    "prepared-manifest.json",
    "plan.json",
    "planned-inventories.json",
    "preparation-seal.json",
    "state.json",
    "state-history.jsonl",
    "execution-claim.json",
    "command-records.json",
    "certificate-index.json",
    "per-cell-comparisons.json",
    "cross-cell-comparison.json",
    "observed-inventories.json",
    "gate-summary.json",
    "gate-summary.md",
    "terminal-manifest.json",
    "detached-verification.json",
    "retention-manifest.json",
}
FAILURE_CONTROL_FILES = {
    "profile.json",
    "prepared-manifest.json",
    "plan.json",
    "planned-inventories.json",
    "preparation-seal.json",
    "state.json",
    "state-history.jsonl",
    "execution-claim.json",
    "command-records.json",
    "failure.json",
    "terminal-manifest.json",
    "detached-verification.json",
    "retention-manifest.json",
}


class CampaignError(RuntimeError):
    pass


def fail(message: str) -> CampaignError:
    return CampaignError(message)


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
    if pathlib.Path(__file__).resolve() != (source / INPUT_PATHS["formal_runner"]).resolve():
        raise fail("formal runner must execute from the candidate path")
    paths = {
        name: canonical(source, INPUT_PATHS[name], value, name)
        for name, value in supplied.items()
    }
    for name, relative in INPUT_PATHS.items():
        paths.setdefault(name, (source / relative).resolve())
    if any(not path.is_file() for path in paths.values()):
        raise fail("a required formal campaign input is absent")
    return paths


def published_candidate(source: pathlib.Path) -> dict[str, Any]:
    try:
        candidate = clean_candidate(source)
    except RuntimeErrorEvidence as error:
        raise fail(str(error)) from error
    upstream = subprocess.run(
        ["git", "rev-parse", "@{upstream}"],
        cwd=source,
        capture_output=True,
        text=True,
        check=False,
    )
    if upstream.returncode != 0 or upstream.stdout.strip() != candidate["commit"]:
        raise fail("candidate upstream is absent or differs from HEAD")
    ancestor = subprocess.run(
        ["git", "merge-base", "--is-ancestor", SEMANTIC_BASELINE, candidate["commit"]],
        cwd=source,
        capture_output=True,
        text=True,
        check=False,
    )
    if ancestor.returncode != 0:
        raise fail("candidate is not a descendant of the frozen semantic baseline")
    candidate.update(
        upstream_commit=upstream.stdout.strip(),
        semantic_baseline=SEMANTIC_BASELINE,
    )
    return candidate


def tool_identity(command: str) -> dict[str, str]:
    resolved = shutil.which(command)
    if resolved is None:
        raise fail(f"required tool is unavailable: {command}")
    invoked = pathlib.Path(resolved).absolute()
    executable = pathlib.Path(resolved).resolve()
    if not executable.is_file():
        raise fail(f"required tool is not a regular file: {command}")
    try:
        version = tool_version(command)["version"]
    except RuntimeErrorEvidence as error:
        raise fail(str(error)) from error
    return {
        "command": command,
        "invocation_path": str(invoked),
        "resolved_path": str(executable),
        "sha256": sha256_file(executable),
        "version": version,
    }


def cloud_tool_paths(source: pathlib.Path) -> dict[str, str]:
    cloud = read_json(source / INPUT_PATHS["cloud_profile"])
    tools = cloud.get("tools")
    if not isinstance(tools, dict):
        raise fail("cloud qualification environment tool map differs")
    try:
        cmake = tools["cmake"]["path"]
        paths = {
            "cmake": cmake,
            "ctest": str(pathlib.Path(cmake).with_name("ctest")),
            "ninja": tools["ninja"]["path"],
            "gcc": tools["gcc"]["path"],
            "clang": tools["clang"]["path"],
            "ldd": "/usr/bin/ldd",
        }
    except (KeyError, TypeError) as error:
        raise fail("cloud qualification environment tool paths differ") from error
    if any(not pathlib.Path(value).is_absolute() for value in paths.values()):
        raise fail("cloud qualification environment tool path is not absolute")
    return paths


def environment_identity(source: pathlib.Path) -> dict[str, Any]:
    tools = cloud_tool_paths(source)
    return {
        "python": tool_identity(sys.executable),
        "cmake": tool_identity(tools["cmake"]),
        "ctest": tool_identity(tools["ctest"]),
        "ninja": tool_identity(tools["ninja"]),
        "gcc": tool_identity(tools["gcc"]),
        "clang": tool_identity(tools["clang"]),
        "ldd": tool_identity(tools["ldd"]),
        "observations": {
            name: os.environ.get(name, "")
            for name in ("ImageOS", "ImageVersion", "LANG", "LC_ALL", "TZ")
        },
    }


def cloud_environment_binding(
    paths: dict[str, pathlib.Path],
    profile: dict[str, Any],
) -> dict[str, Any]:
    cloud_profile = read_json(paths["cloud_profile"])
    if (
        cloud_profile.get("schema_version") != 1
        or cloud_profile.get("kind") != "cloud-qualification-environment-profile"
    ):
        raise fail("cloud qualification environment profile identity differs")
    if cloud_profile.get("cells") != profile["cells"]:
        raise fail("cloud qualification environment matrix differs from CGR")

    observations = []
    try:
        for cell in profile["cells"]:
            observation = validate_cloud_environment(cloud_profile, cell["id"])
            if observation.get("status") != "PASS":
                details = "; ".join(observation.get("failures", []))
                raise fail(
                    f"cloud qualification environment identity differs for {cell['id']}: {details}"
                )
            observations.append(observation)
    except (EnvironmentValidationError, OSError, KeyError, ValueError) as error:
        raise fail(f"cloud qualification environment validation failed: {error}") from error

    return {
        "schema_version": 1,
        "kind": "continuous-curve-geometry-cloud-environment-binding",
        "profile_sha256": sha256_file(paths["cloud_profile"]),
        "validator_sha256": sha256_file(paths["cloud_validator"]),
        "decision_sha256": sha256_file(paths["cloud_decision"]),
        "audit_sha256": sha256_file(paths["cloud_audit"]),
        "observations": observations,
    }


def protocol_check(path: pathlib.Path) -> None:
    report_runner.protocol_check(path)
    content = path.read_text(encoding="utf-8")
    required = (
        "## 29. Formal PREPARED lifecycle design",
        "## 30. Formal PREPARED lifecycle design integration checkpoint",
        "Formal preparation dispatch and formal execution remain unauthorized.",
    )
    if any(token not in content for token in required):
        raise fail("protocol is not the integrated CGR formal lifecycle authority")


def frozen_semantic_identity(
    source: pathlib.Path,
    profile: dict[str, Any],
) -> list[dict[str, str]]:
    try:
        return report_runner.frozen_semantic_identity(source, profile)
    except report_runner.RunnerError as error:
        raise fail(str(error)) from error


def formal_plan(
    profile: dict[str, Any],
    source: pathlib.Path,
    declared: list[str],
) -> list[dict[str, Any]]:
    report = report_runner.command_plan(profile, source, declared)

    def replace(value: Any) -> Any:
        if isinstance(value, list):
            return [replace(item) for item in value]
        if isinstance(value, dict):
            return {key: replace(item) for key, item in value.items()}
        if isinstance(value, str):
            return value.replace("@REPORT_ROOT@", "@OUTPUT_ROOT@")
        return value

    formal = replace(report)
    report_ids = report_runner.command_ids(report)
    formal_ids = report_runner.command_ids(formal)
    if report_ids != formal_ids or len(formal_ids) != 56 or len(set(formal_ids)) != 56:
        raise fail("formal plan command identity differs from report-only plan")
    simulation = report_runner.simulation(
        profile,
        report,
        frozen_semantic_identity(source, profile),
    )
    report_runner.validate_simulation(profile, simulation)
    return formal


def planned_inventories(
    profile: dict[str, Any],
    candidate: dict[str, Any],
    plan: list[dict[str, Any]],
) -> dict[str, Any]:
    artifacts: list[dict[str, Any]] = [
        {
            "path": name,
            "role": "preparation-control",
            "required_when": "always",
        }
        for name in (
            *PREPARATION_FILES,
            "preparation-seal.json",
            "state.json",
            "state-history.jsonl",
            "execution-claim.json",
            "command-records.json",
            "terminal-manifest.json",
            "detached-verification.json",
            "retention-manifest.json",
        )
    ]
    artifacts += [
        {"path": name, "role": "terminal-success", "required_when": "success"}
        for name in (
            "certificate-index.json",
            "per-cell-comparisons.json",
            "cross-cell-comparison.json",
            "observed-inventories.json",
            "gate-summary.json",
            "gate-summary.md",
        )
    ]
    artifacts.append(
        {"path": "failure.json", "role": "terminal-failure", "required_when": "failure"}
    )

    for cell in plan:
        cell_id = cell["cell"]
        for stage in (
            "configure",
            "negative-evidence",
            "dependency-inventory",
            "runtime-dependencies",
        ):
            for stream in ("stdout", "stderr"):
                artifacts.append(
                    {
                        "path": f"logs/{cell_id}-{stage}.{stream}.log",
                        "role": "command-log",
                        "required_when": "executed",
                    }
                )
        for repetition in cell["repetitions"]:
            ordinal = repetition["repetition"]
            for stage in (
                "build",
                "ctest-discovery",
                "semantic-ctest",
                "certificate",
                "certificate-validation",
            ):
                for stream in ("stdout", "stderr"):
                    artifacts.append(
                        {
                            "path": f"logs/{cell_id}-{stage}-{ordinal}.{stream}.log",
                            "role": "command-log",
                            "required_when": "executed",
                        }
                    )
            artifacts.append(
                {
                    "path": f"certificates/{cell_id}-{ordinal}.json",
                    "role": "semantic-certificate",
                    "required_when": "success",
                }
            )
        artifacts.extend(
            [
                {
                    "path": f"negatives/{cell_id}.json",
                    "role": "negative-evidence",
                    "required_when": "success",
                },
                {
                    "path": f"cells/{cell_id}/compile_commands.json",
                    "role": "compile-command-inventory",
                    "required_when": "success",
                },
            ]
        )

    for name in report_runner.EXPECTED_DERIVED_FILES:
        artifacts.append(
            {
                "path": f"derived/{name}",
                "role": "derived-evidence",
                "required_when": "success",
            }
        )

    return {
        "schema_version": 1,
        "kind": "continuous-curve-geometry-planned-inventories",
        "candidate_source_inventory": candidate["source_inventory"],
        "cells": [
            {
                "cell": cell["cell"],
                "repetitions": [item["repetition"] for item in cell["repetitions"]],
            }
            for cell in plan
        ],
        "artifacts": artifacts,
    }


def preparation_seal(output: pathlib.Path) -> dict[str, Any]:
    lines = (output / "state-history.jsonl").read_bytes().splitlines(keepends=True)
    if not lines:
        raise fail("initial state history is absent")
    return {
        "schema_version": 1,
        "files": {
            name: sha256_file(output / name)
            for name in PREPARATION_FILES
        },
        "state_history_sha256": hashlib.sha256(lines[0]).hexdigest(),
    }


def expected_prepared_file_set() -> set[str]:
    return {
        *PREPARATION_FILES,
        "preparation-seal.json",
        "state.json",
        "state-history.jsonl",
    }


def validate_prepared(
    output: pathlib.Path,
    *,
    require_unconsumed: bool = True,
) -> dict[str, Any]:
    manifest = read_json(output / "prepared-manifest.json")
    keys = {
        "schema_version",
        "kind",
        "state",
        "execution_requested",
        "candidate",
        "frozen_semantic_files",
        "inputs",
        "environment",
        "cloud_environment",
        "working_directory",
        "output_root",
        "limits_seconds",
        "declared_semantic_ctest_allowlist",
        "plan",
        "planned_inventories",
        "gates",
        "retained_limitations",
        "prepared_utc",
    }
    if (
        set(manifest) != keys
        or manifest["schema_version"] != 1
        or manifest["kind"] != "continuous-curve-geometry-prepared-manifest"
        or manifest["state"] != "PREPARED"
        or manifest["execution_requested"] is not False
    ):
        raise fail("prepared manifest schema differs")
    if (
        manifest["output_root"] != str(output.resolve())
        or set(manifest["gates"].values()) != {"NOT_EXECUTED"}
    ):
        raise fail("prepared manifest state differs")

    profile = read_json(output / "profile.json")
    plan_file = read_json(output / "plan.json")
    inventory_file = read_json(output / "planned-inventories.json")
    expected_plan_file = {
        "schema_version": 1,
        "kind": "continuous-curve-geometry-formal-launch-plan",
        "execution_requested": False,
        "cells": manifest["plan"],
        "gates": manifest["gates"],
        "limitations": manifest["retained_limitations"],
    }
    if plan_file != expected_plan_file or inventory_file != manifest["planned_inventories"]:
        raise fail("prepared plan or inventory differs")
    if profile.get("gates") != list(manifest["gates"]):
        raise fail("prepared profile gates differ")

    seal = read_json(output / "preparation-seal.json")
    history_bytes = (output / "state-history.jsonl").read_bytes().splitlines(keepends=True)
    if (
        set(seal) != {"schema_version", "files", "state_history_sha256"}
        or seal["schema_version"] != 1
        or seal["files"]
        != {name: sha256_file(output / name) for name in PREPARATION_FILES}
        or not history_bytes
        or seal["state_history_sha256"] != hashlib.sha256(history_bytes[0]).hexdigest()
    ):
        raise fail("preparation seal differs")

    state = read_json(output / "state.json")
    history = (output / "state-history.jsonl").read_bytes().splitlines()
    initial = json.loads(history[0]) if history else {}
    if not history or initial.get("state") != "PREPARED":
        raise fail("prepared lifecycle differs")

    if require_unconsumed:
        actual = {
            relative_path(output, path)
            for path in output.rglob("*")
            if path.is_file()
        }
        if actual != expected_prepared_file_set():
            raise fail("PREPARED package file set differs")
        if len(history) != 1 or state != initial:
            raise fail("prepared lifecycle is already consumed")
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
    frozen = frozen_semantic_identity(source, profile)
    cloud = cloud_environment_binding(paths, profile)
    declared = report_runner.declared_allowlist(
        source,
        profile["semantic_ctest_allowlist"],
    )
    plan = formal_plan(profile, source, declared)

    output.mkdir(parents=True)
    shutil.copyfile(paths["profile"], output / "profile.json")
    manifest = {
        "schema_version": 1,
        "kind": "continuous-curve-geometry-prepared-manifest",
        "state": "PREPARED",
        "execution_requested": False,
        "candidate": candidate,
        "frozen_semantic_files": frozen,
        "inputs": input_identity(paths),
        "environment": environment_identity(source),
        "cloud_environment": cloud,
        "working_directory": str(source),
        "output_root": str(output),
        "limits_seconds": {
            "build": BUILD_TIMEOUT_SECONDS,
            "process": PROCESS_TIMEOUT_SECONDS,
            "overall": OVERALL_TIMEOUT_SECONDS,
        },
        "declared_semantic_ctest_allowlist": declared,
        "plan": plan,
        "planned_inventories": planned_inventories(profile, candidate, plan),
        "gates": {gate: "NOT_EXECUTED" for gate in profile["gates"]},
        "retained_limitations": profile["limitations"],
        "prepared_utc": utc_now(),
    }
    write_json(output / "prepared-manifest.json", manifest)
    write_json(
        output / "plan.json",
        {
            "schema_version": 1,
            "kind": "continuous-curve-geometry-formal-launch-plan",
            "execution_requested": False,
            "cells": plan,
            "gates": manifest["gates"],
            "limitations": manifest["retained_limitations"],
        },
    )
    write_json(output / "planned-inventories.json", manifest["planned_inventories"])
    write_state(
        output,
        "PREPARED",
        {
            "candidate_commit": candidate["commit"],
            "execution_requested": False,
            "prepared_manifest_sha256": sha256_file(output / "prepared-manifest.json"),
        },
    )
    write_json(output / "preparation-seal.json", preparation_seal(output))
    validate_prepared(output)


def validate_execution_binding(
    arguments: argparse.Namespace,
    source: pathlib.Path,
    output: pathlib.Path,
    manifest: dict[str, Any],
) -> dict[str, Any]:
    paths = input_paths(arguments, source)
    profile = validate_profile(paths["profile"])
    protocol_check(paths["protocol"])
    candidate = published_candidate(source)
    if (
        manifest["candidate"] != candidate
        or manifest["environment"] != environment_identity(source)
        or manifest["working_directory"] != str(source)
        or manifest["output_root"] != str(output)
    ):
        raise fail("prepared candidate or environment differs")
    verify_input_identity(manifest["inputs"], paths)
    frozen = frozen_semantic_identity(source, profile)
    if manifest["frozen_semantic_files"] != frozen:
        raise fail("prepared frozen semantic identity differs")
    if manifest["cloud_environment"] != cloud_environment_binding(paths, profile):
        raise fail("prepared cloud environment binding differs")
    declared = report_runner.declared_allowlist(
        source,
        profile["semantic_ctest_allowlist"],
    )
    plan = formal_plan(profile, source, declared)
    if (
        manifest["declared_semantic_ctest_allowlist"] != declared
        or manifest["plan"] != plan
        or manifest["planned_inventories"] != planned_inventories(profile, candidate, plan)
        or manifest["gates"] != {gate: "NOT_EXECUTED" for gate in profile["gates"]}
    ):
        raise fail("prepared formal binding differs")
    return profile


def replace_root(value: Any, output: pathlib.Path) -> Any:
    if isinstance(value, list):
        return [replace_root(item, output) for item in value]
    if isinstance(value, dict):
        return {key: replace_root(item, output) for key, item in value.items()}
    if isinstance(value, str):
        return value.replace("@OUTPUT_ROOT@", str(output))
    return value


def write_records(output: pathlib.Path, records: list[dict[str, Any]]) -> None:
    write_json(
        output / "command-records.json",
        {
            "schema_version": 1,
            "kind": "continuous-curve-geometry-command-records",
            "records": records,
        },
    )


def require_success(record: dict[str, Any], context: str) -> None:
    if (
        record.get("exit_code") != 0
        or record.get("timed_out")
        or record.get("launch_error") is not None
    ):
        raise fail(f"{context} failed")


def discovered_tests_from_log(output: pathlib.Path, record: dict[str, Any]) -> list[str]:
    text = (output / record["stdout"]["path"]).read_text(
        encoding="utf-8",
        errors="strict",
    )
    return re.findall(
        r"^[ \t]*Test[ \t]+#\d+:[ \t]+([^\s]+)[ \t]*$",
        text,
        flags=re.MULTILINE,
    )


def require_exact_semantic_ctest_execution(
    output: pathlib.Path,
    record: dict[str, Any],
    expected: list[str],
) -> None:
    stdout = (output / record["stdout"]["path"]).read_text(
        encoding="utf-8",
        errors="strict",
    )
    stderr = (output / record["stderr"]["path"]).read_text(
        encoding="utf-8",
        errors="strict",
    )
    combined = stdout + stderr
    if any(
        marker in combined
        for marker in (
            "RegularExpression::compile():",
            "Error in compile.",
            "No tests were found!!!",
        )
    ):
        raise fail("semantic CTest selection is invalid or empty")
    observed = re.findall(
        r"^[ \t]*\d+/\d+[ \t]+Test[ \t]+#\d+:[ \t]+([^\s]+).*\bPassed\b.*$",
        stdout,
        flags=re.MULTILINE,
    )
    if sorted(observed) != sorted(expected) or len(observed) != len(set(observed)):
        raise fail("semantic CTest execution differs from the sealed allowlist")


def load_negative_module(path: pathlib.Path):
    specification = importlib.util.spec_from_file_location("cgr_negative", path)
    if specification is None or specification.loader is None:
        raise fail("negative-evidence module could not be loaded")
    module = importlib.util.module_from_spec(specification)
    specification.loader.exec_module(module)
    return module


def per_cell_comparisons(
    profile: dict[str, Any],
    entries: list[dict[str, Any]],
    output: pathlib.Path,
) -> dict[str, Any]:
    cells = []
    for cell in profile["cells"]:
        selected = [entry for entry in entries if entry["cell"] == cell["id"]]
        projections = [
            validate_certificate(profile, output / entry["path"])
            for entry in selected
        ]
        if (
            len(projections) != profile["repetitions_per_cell"]
            or any(item != projections[0] for item in projections[1:])
        ):
            raise fail("same-cell scientific projection differs")
        cells.append(
            {
                "cell": cell["id"],
                "certificate_count": len(projections),
                "status": "EVIDENCE_COLLECTED_PENDING_AUDIT",
                "scientific_projection": projections[0],
            }
        )
    return {
        "schema_version": 1,
        "kind": "continuous-curve-geometry-per-cell-comparisons",
        "cells": cells,
    }


def gate_summary(profile: dict[str, Any]) -> dict[str, Any]:
    return {
        "schema_version": 1,
        "kind": "continuous-curve-geometry-gate-summary",
        "status": "EVIDENCE_COLLECTED_PENDING_AUDIT",
        "gates": {
            gate: "EVIDENCE_COLLECTED_PENDING_AUDIT"
            for gate in profile["gates"]
        },
        "limitations": profile["limitations"],
    }


def write_terminal(
    output: pathlib.Path,
    manifest: dict[str, Any],
    state: str,
    extra: dict[str, str],
) -> None:
    write_json(
        output / "terminal-manifest.json",
        {
            "schema_version": 1,
            "kind": "continuous-curve-geometry-terminal-manifest",
            "candidate": manifest["candidate"],
            "prepared_manifest_sha256": sha256_file(output / "prepared-manifest.json"),
            "state": state,
            "command_records": "command-records.json",
            "retention_manifest": "retention-manifest.json",
            **extra,
        },
    )


def verify_detached_candidate(
    source: pathlib.Path,
    candidate: dict[str, Any],
) -> dict[str, Any]:
    with tempfile.TemporaryDirectory(prefix="apmesh-core-cgr-detached-") as temporary:
        worktree = pathlib.Path(temporary) / "candidate"
        added = subprocess.run(
            ["git", "worktree", "add", "--detach", str(worktree), candidate["commit"]],
            cwd=source,
            capture_output=True,
            text=True,
            check=False,
        )
        if added.returncode != 0:
            raise fail("detached candidate worktree could not be created")
        try:
            observed = clean_candidate(worktree)
            if (
                observed["commit"] != candidate["commit"]
                or observed["source_inventory"] != candidate["source_inventory"]
            ):
                raise fail("detached candidate identity differs")
        finally:
            removed = subprocess.run(
                ["git", "worktree", "remove", "--force", str(worktree)],
                cwd=source,
                capture_output=True,
                text=True,
                check=False,
            )
            if removed.returncode != 0:
                raise fail("detached candidate worktree could not be removed")
    return {
        "result": "PASS",
        "candidate_commit": candidate["commit"],
        "source_inventory_count": len(candidate["source_inventory"]),
    }


def write_retention_inventory(
    output: pathlib.Path,
    manifest: dict[str, Any],
    required_paths: set[str],
) -> None:
    missing = sorted(
        path
        for path in required_paths
        if path != "retention-manifest.json" and not (output / path).is_file()
    )
    if missing:
        raise fail(f"required retained artifact is absent: {missing}")
    files = []
    for path in sorted(
        (item for item in output.rglob("*") if item.is_file()),
        key=lambda item: item.as_posix(),
    ):
        relative = relative_path(output, path)
        if relative == "retention-manifest.json" or "/build/" in relative:
            continue
        files.append(
            {
                "path": relative,
                "sha256": sha256_file(path),
                "size": path.stat().st_size,
            }
        )
    write_json(
        output / "retention-manifest.json",
        {
            "schema_version": 1,
            "kind": "continuous-curve-geometry-retention",
            "candidate_commit": manifest["candidate"]["commit"],
            "prepared_manifest_sha256": sha256_file(output / "prepared-manifest.json"),
            "required_paths": sorted(required_paths),
            "files": files,
        },
    )


def seal_output(
    output: pathlib.Path,
    manifest: dict[str, Any],
    required_paths: set[str],
) -> None:
    detached = verify_detached_candidate(
        pathlib.Path(manifest["candidate"]["source_root"]),
        manifest["candidate"],
    )
    write_json(
        output / "detached-verification.json",
        {
            "schema_version": 1,
            "kind": "continuous-curve-geometry-detached-verification",
            **detached,
        },
    )
    write_retention_inventory(output, manifest, required_paths)
    verify_retention(output)


def verify_retention(output: pathlib.Path) -> dict[str, Any]:
    manifest = validate_prepared(output, require_unconsumed=False)
    terminal = read_json(output / "terminal-manifest.json")
    retention = read_json(output / "retention-manifest.json")
    if (
        terminal.get("candidate") != manifest["candidate"]
        or terminal.get("prepared_manifest_sha256")
        != sha256_file(output / "prepared-manifest.json")
        or terminal.get("state") not in {"EXECUTED_PENDING_AUDIT", "BLOCKED"}
    ):
        raise fail("terminal manifest differs")

    required = set(FAILURE_CONTROL_FILES)
    if terminal["state"] == "EXECUTED_PENDING_AUDIT":
        required = set(SUCCESS_CONTROL_FILES) | {
            row["path"]
            for row in manifest["planned_inventories"]["artifacts"]
            if row["required_when"] in {"always", "success", "executed"}
        }

    if (
        set(retention)
        != {
            "schema_version",
            "kind",
            "candidate_commit",
            "prepared_manifest_sha256",
            "required_paths",
            "files",
        }
        or retention.get("schema_version") != 1
        or retention.get("kind") != "continuous-curve-geometry-retention"
        or retention.get("candidate_commit") != manifest["candidate"]["commit"]
        or retention.get("prepared_manifest_sha256")
        != sha256_file(output / "prepared-manifest.json")
        or set(retention.get("required_paths", [])) != required
    ):
        raise fail("retention manifest differs")

    listed = {"retention-manifest.json"}
    for entry in retention.get("files", []):
        path = (output / entry.get("path", "")).resolve()
        if (
            set(entry) != {"path", "sha256", "size"}
            or entry["path"] in listed
            or output.resolve() not in path.parents
            or not path.is_file()
            or sha256_file(path) != entry["sha256"]
            or path.stat().st_size != entry["size"]
        ):
            raise fail("retained file differs")
        listed.add(entry["path"])

    actual = {
        relative_path(output, path)
        for path in output.rglob("*")
        if path.is_file() and "/build/" not in relative_path(output, path)
    }
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
        if len(records) != 56 or len({record.get("id") for record in records}) != 56:
            raise fail("formal command cardinality differs")
        if any(
            record.get("exit_code") != 0
            or record.get("timed_out")
            or record.get("launch_error") is not None
            for record in records
        ):
            raise fail("successful terminal package contains a failed command")

        profile = validate_profile(output / "profile.json")
        index = output / "certificate-index.json"
        if read_json(output / "cross-cell-comparison.json") != compare(profile, index):
            raise fail("cross-cell comparison differs")
        per_cell = read_json(output / "per-cell-comparisons.json")
        if [row.get("cell") for row in per_cell.get("cells", [])] != [
            cell["id"] for cell in profile["cells"]
        ]:
            raise fail("per-cell comparison differs")
        negative_module = load_negative_module(
            pathlib.Path(manifest["inputs"]["negative"]["path"])
        )
        for cell in profile["cells"]:
            negative_module.validate(
                pathlib.Path(manifest["inputs"]["profile"]["path"]),
                output / "negatives" / f"{cell['id']}.json",
                negative_module.load_validator(
                    pathlib.Path(manifest["inputs"]["validator"]["path"])
                ),
            )
        derived_dir = output / "derived"
        observed_derived = sorted(
            path.name for path in derived_dir.iterdir() if path.is_file()
        )
        if observed_derived != sorted(report_runner.EXPECTED_DERIVED_FILES):
            raise fail("derived evidence inventory differs")
        summary = read_json(output / "gate-summary.json")
        if set(summary.get("gates", {}).values()) != {
            "EVIDENCE_COLLECTED_PENDING_AUDIT"
        }:
            raise fail("gate summary claims scientific closure")

    detached = read_json(output / "detached-verification.json")
    if detached != {
        "schema_version": 1,
        "kind": "continuous-curve-geometry-detached-verification",
        "result": "PASS",
        "candidate_commit": manifest["candidate"]["commit"],
        "source_inventory_count": len(manifest["candidate"]["source_inventory"]),
    }:
        raise fail("detached verification differs")

    return {
        "schema_version": 1,
        "kind": "continuous-curve-geometry-retention-verification",
        "status": "PASS",
        "files": sorted(listed),
    }


def execute(arguments: argparse.Namespace) -> int:
    source = pathlib.Path(arguments.source_root).resolve()
    output = pathlib.Path(arguments.output_root).resolve()

    # A local execution claim is part of the immutable consumed-package
    # boundary. Re-entering execute must not rewrite either a successful or a
    # blocked terminal package.
    if (output / "execution-claim.json").exists():
        return 1

    records: list[dict[str, Any]] = []
    entries: list[dict[str, Any]] = []
    try:
        manifest = validate_prepared(output, require_unconsumed=True)
        profile = validate_execution_binding(arguments, source, output, manifest)
        write_json(
            output / "execution-claim.json",
            {
                "schema_version": 1,
                "kind": "continuous-curve-geometry-execution-claim",
                "candidate_commit": manifest["candidate"]["commit"],
                "prepared_manifest_sha256": sha256_file(output / "prepared-manifest.json"),
                "pid": os.getpid(),
                "started_utc": utc_now(),
            },
        )
        write_state(output, "RUNNING", {"candidate_commit": manifest["candidate"]["commit"]})
        (output / "certificates").mkdir(parents=True, exist_ok=True)
        (output / "negatives").mkdir(parents=True, exist_ok=True)

        discoveries: list[dict[str, Any]] = []
        inventories: list[dict[str, Any]] = []
        negative_module = load_negative_module(pathlib.Path(manifest["inputs"]["negative"]["path"]))
        negative_validator = negative_module.load_validator(
            pathlib.Path(manifest["inputs"]["validator"]["path"])
        )

        for cell in manifest["plan"]:
            name = cell["cell"]
            configure = run_command(
                replace_root(cell["configure"], output),
                source,
                output / "logs",
                f"{name}-configure",
                BUILD_TIMEOUT_SECONDS,
            )
            records.append(configure)
            require_success(configure, f"{name} configure")

            for repetition in cell["repetitions"]:
                ordinal = repetition["repetition"]
                for stage in (
                    "build",
                    "ctest_discovery",
                    "semantic_ctest",
                    "certificate",
                    "certificate_validation",
                ):
                    timeout = (
                        BUILD_TIMEOUT_SECONDS
                        if stage in {"build", "ctest_discovery", "semantic_ctest"}
                        else PROCESS_TIMEOUT_SECONDS
                    )
                    record = run_command(
                        replace_root(repetition[stage], output),
                        source,
                        output / "logs",
                        f"{name}-{stage.replace('_', '-')}-{ordinal}",
                        timeout,
                    )
                    records.append(record)
                    require_success(record, f"{name} {stage} repetition {ordinal}")

                    if stage == "ctest_discovery":
                        observed = discovered_tests_from_log(output, record)
                        expected = manifest["declared_semantic_ctest_allowlist"]
                        selected = [test for test in observed if test in expected]
                        if sorted(selected) != sorted(expected) or len(selected) != len(set(selected)):
                            raise fail("observed semantic CTest allowlist differs")
                        discoveries.append(
                            {
                                "cell": name,
                                "repetition": ordinal,
                                "record_id": record["id"],
                                "discovered_allowlist": selected,
                            }
                        )
                    if stage == "semantic_ctest":
                        require_exact_semantic_ctest_execution(
                            output,
                            record,
                            manifest["declared_semantic_ctest_allowlist"],
                        )

                certificate = output / "certificates" / f"{name}-{ordinal}.json"
                entries.append(
                    {
                        "cell": name,
                        "repetition": ordinal,
                        "path": relative_path(output, certificate),
                        "sha256": sha256_file(certificate),
                    }
                )

            negative = run_command(
                replace_root(cell["negative_evidence"], output),
                source,
                output / "logs",
                f"{name}-negative-evidence",
                PROCESS_TIMEOUT_SECONDS,
            )
            records.append(negative)
            require_success(negative, f"{name} negative evidence")
            negative_module.validate(
                pathlib.Path(manifest["inputs"]["profile"]["path"]),
                output / "negatives" / f"{name}.json",
                negative_validator,
            )

            dependency = run_command(
                replace_root(cell["dependency_inventory"], output),
                source,
                output / "logs",
                f"{name}-dependency-inventory",
                PROCESS_TIMEOUT_SECONDS,
            )
            records.append(dependency)
            require_success(dependency, f"{name} dependency inventory")

            runtime = run_command(
                ["/usr/bin/ldd", replace_root(cell["runtime_dependencies"][0], output)],
                source,
                output / "logs",
                f"{name}-runtime-dependencies",
                PROCESS_TIMEOUT_SECONDS,
            )
            records.append(runtime)
            require_success(runtime, f"{name} runtime dependency inventory")
            runtime_text = (
                (output / runtime["stdout"]["path"]).read_text(encoding="utf-8")
                + (output / runtime["stderr"]["path"]).read_text(encoding="utf-8")
            )
            if "not found" in runtime_text:
                raise fail("unresolved runtime dependency")

            build = output / "cells" / name / "build"
            compile_commands = build / "compile_commands.json"
            retained_compile = output / "cells" / name / "compile_commands.json"
            if not compile_commands.is_file():
                raise fail("compile command inventory is absent")
            retained_compile.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(compile_commands, retained_compile)
            inventories.append(
                {
                    "cell": name,
                    "compile_commands": {
                        "path": relative_path(output, retained_compile),
                        "sha256": sha256_file(retained_compile),
                    },
                    "dependency_record_id": dependency["id"],
                    "runtime_record_id": runtime["id"],
                }
            )

        write_records(output, records)
        write_json(
            output / "certificate-index.json",
            {
                "schema_version": 1,
                "kind": "continuous-curve-geometry-certificate-index",
                "entries": entries,
            },
        )
        write_json(
            output / "per-cell-comparisons.json",
            per_cell_comparisons(profile, entries, output),
        )
        write_json(
            output / "cross-cell-comparison.json",
            compare(profile, output / "certificate-index.json"),
        )
        write_json(
            output / "observed-inventories.json",
            {
                "schema_version": 1,
                "kind": "continuous-curve-geometry-observed-inventories",
                "candidate_commit": manifest["candidate"]["commit"],
                "discoveries": discoveries,
                "cells": inventories,
            },
        )

        derive(
            profile,
            output / entries[0]["path"],
            output / "derived",
        )
        write_json(output / "gate-summary.json", gate_summary(profile))
        (output / "gate-summary.md").write_text(
            "# CGR0-CGR7 execution evidence\n\n"
            "Status: EVIDENCE_COLLECTED_PENDING_AUDIT\n",
            encoding="utf-8",
            newline="\n",
        )
        write_terminal(
            output,
            manifest,
            "EXECUTED_PENDING_AUDIT",
            {
                "certificate_index": "certificate-index.json",
                "per_cell_comparisons": "per-cell-comparisons.json",
                "cross_cell_comparison": "cross-cell-comparison.json",
                "derived_evidence": "derived/derived-evidence.json",
                "gate_summary": "gate-summary.json",
            },
        )
        write_state(
            output,
            "EXECUTED_PENDING_AUDIT",
            {
                "candidate_commit": manifest["candidate"]["commit"],
                "comparison": "cross-cell-comparison.json",
            },
        )
        required = set(SUCCESS_CONTROL_FILES) | {
            row["path"]
            for row in manifest["planned_inventories"]["artifacts"]
            if row["required_when"] in {"always", "success", "executed"}
        }
        seal_output(output, manifest, required)
        return 0
    except Exception as error:
        if (output / "state.json").is_file():
            try:
                if read_json(output / "state.json").get("state") == "BLOCKED":
                    return 1
            except Exception:
                pass
        write_records(output, records)
        manifest = read_json(output / "prepared-manifest.json")
        write_json(
            output / "failure.json",
            {
                "schema_version": 1,
                "kind": "continuous-curve-geometry-failure",
                "message": str(error),
                "records": records,
                "partial_certificate_slots": entries,
            },
        )
        state = read_json(output / "state.json").get("state")
        if state == "PREPARED":
            write_state(output, "RUNNING", {"candidate_commit": manifest["candidate"]["commit"]})
        write_terminal(output, manifest, "BLOCKED", {"failure": "failure.json"})
        write_state(
            output,
            "BLOCKED",
            {
                "candidate_commit": manifest["candidate"]["commit"],
                "closure_failure": True,
                "failure": "failure.json",
            },
        )
        seal_output(output, manifest, set(FAILURE_CONTROL_FILES))
        return 1


def self_check(arguments: argparse.Namespace) -> dict[str, Any]:
    source = pathlib.Path(arguments.source_root).resolve()
    paths = input_paths(arguments, source)
    profile = validate_profile(paths["profile"])
    protocol_check(paths["protocol"])
    declared = report_runner.declared_allowlist(
        source,
        profile["semantic_ctest_allowlist"],
    )
    frozen = frozen_semantic_identity(source, profile)
    plan = formal_plan(profile, source, declared)
    identifiers = report_runner.command_ids(plan)
    if len(identifiers) != 56 or len(set(identifiers)) != 56:
        raise fail("formal command cardinality differs")
    return {
        "schema_version": 1,
        "kind": "continuous-curve-geometry-formal-runner-self-check",
        "status": "PASS",
        "execution_requested": False,
        "formal_preparation": False,
        "formal_execution": False,
        "input_hashes": input_identity(paths),
        "declared_semantic_ctest_allowlist": declared,
        "frozen_semantic_files": frozen,
        "plan": {
            "schema_version": 1,
            "kind": "continuous-curve-geometry-formal-launch-plan",
            "execution_requested": False,
            "cells": plan,
            "gates": {gate: "NOT_EXECUTED" for gate in profile["gates"]},
            "limitations": profile["limitations"],
        },
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    for name in (
        "source-root",
        "profile",
        "protocol",
        "exporter",
        "validator",
        "negative",
    ):
        parser.add_argument(f"--{name}", required=True)
    commands = parser.add_subparsers(dest="command", required=True)
    commands.add_parser("self-check").add_argument("--output", required=True)
    commands.add_parser("prepare").add_argument("--output-root", required=True)
    commands.add_parser("validate-prepared").add_argument("--output-root", required=True)
    commands.add_parser("execute").add_argument("--output-root", required=True)
    commands.add_parser("verify-retention").add_argument("--output-root", required=True)
    arguments = parser.parse_args()

    try:
        if arguments.command == "prepare":
            prepare(arguments)
            return 0
        if arguments.command == "validate-prepared":
            output = pathlib.Path(arguments.output_root).resolve()
            manifest = validate_prepared(output)
            validate_execution_binding(
                arguments,
                pathlib.Path(arguments.source_root).resolve(),
                output,
                manifest,
            )
            return 0
        if arguments.command == "execute":
            return execute(arguments)
        if arguments.command == "verify-retention":
            print(
                json.dumps(
                    verify_retention(pathlib.Path(arguments.output_root).resolve()),
                    sort_keys=True,
                )
            )
            return 0
        result = self_check(arguments)
        write_json(pathlib.Path(arguments.output), result)
        return 0
    except (
        CampaignError,
        RuntimeErrorEvidence,
        EvidenceError,
        report_runner.RunnerError,
        OSError,
        ValueError,
        json.JSONDecodeError,
    ) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
