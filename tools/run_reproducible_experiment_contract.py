#!/usr/bin/env python3
"""Prepare or explicitly execute the revision-bound Foundation REC campaign."""

from __future__ import annotations

import argparse
import pathlib
import sys
import time
from typing import Any

from experiment_runtime import (RuntimeErrorEvidence, clean_candidate, input_identity, read_json,
                                run_command, sha256_file, tool_version, utc_now, verify_input_identity,
                                write_json, write_state)
from reproducible_experiment_evidence import (EvidenceError, REQUIRED_ARTIFACTS, artifact_inventory,
                                               compare_configurations, compare_replays, validate_profile, validate_terminal_manifest,
                                               write_derived)


COMPILERS = {
    "gcc-debug": ("g++-13", False), "gcc-release": ("g++-13", False),
    "clang-debug": ("clang++-18", True), "clang-release": ("clang++-18", True),
}


def required_inputs(arguments: argparse.Namespace) -> dict[str, pathlib.Path]:
    root = pathlib.Path(arguments.source_root).resolve()
    paths = {
        "profile": pathlib.Path(arguments.profile).resolve(),
        "contract": pathlib.Path(arguments.contract).resolve(),
        "protocol": pathlib.Path(arguments.protocol).resolve(),
        "collector": pathlib.Path(arguments.collector).resolve(),
        "launcher": pathlib.Path(__file__).resolve(),
        "runtime": pathlib.Path(__file__).with_name("experiment_runtime.py").resolve(),
        "architecture_runner": root / "tools" / "run_architecture_bootstrap_regression.py",
        "numeric_runner": root / "tools" / "run_numeric_contract_regression.py",
        "numeric_profile": root / "experiments" / "profiles" / "numeric_contract.json",
        "numeric_protocol": root / "docs" / "decisions" / "FOUNDATION_NUMERIC_CONTRACT_QUALIFICATION.md",
        "numeric_validator": root / "tools" / "numeric_contract_evidence.py",
        "architecture_profile": root / "experiments" / "profiles" / "architecture_bootstrap.json",
        "architecture_protocol": root / "docs" / "decisions" / "FOUNDATION_ARCHITECTURE_BOOTSTRAP_REGRESSION.md",
        "architecture_expected": root / "experiments" / "expected" / "bootstrap_certificate.json",
        "architecture_comparer": root / "tools" / "bootstrap_regression.py",
        "negative_fixture_runner": root / "tools" / "reproducible_experiment_negative.py",
    }
    for path in paths.values():
        if not path.is_file():
            raise EvidenceError(f"required input is absent: {path}")
    return paths


def command_plan(profile: dict[str, Any], source_root: pathlib.Path) -> list[dict[str, Any]]:
    plan: list[dict[str, Any]] = []
    for configuration in profile["configurations"]:
        name = configuration["name"]
        compiler, libcxx = COMPILERS[name]
        for replay in range(1, profile["replays_per_cell"] + 1):
            build = f"@SCRATCH_ROOT@/cells/{name}/replay-{replay}/build"
            plan.append({
                "cell": name, "replay": replay,
                "configure": ["cmake", "-S", str(source_root), "-B", build, "-G", "Ninja", f"-DCMAKE_CXX_COMPILER={compiler}", f"-DCMAKE_BUILD_TYPE={configuration['build_type']}", f"-DAPMESH_USE_LIBCXX={'ON' if libcxx else 'OFF'}", "-DBUILD_TESTING=ON", "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"],
                "build": ["cmake", "--build", build, "--target", "apmesh_core_numeric_contract_export", "apmesh_core_numeric_contract"],
                "ctest": ["ctest", "--test-dir", build, "-L", "numeric", "--output-on-failure"],
            })
    return plan


def _tool_environment() -> dict[str, Any]:
    return {"python": {"path": str(pathlib.Path(sys.executable).resolve()), "version": sys.version.splitlines()[0]},
            "cmake": tool_version("cmake"), "ctest": tool_version("ctest"), "ninja": tool_version("ninja"),
            "gcc": tool_version("g++-13"), "clang": tool_version("clang++-18")}


def _prepared_paths(control_root: pathlib.Path) -> tuple[pathlib.Path, pathlib.Path]:
    return control_root / "prepared-manifest.json", control_root / "launch-plan.json"


def prepare(arguments: argparse.Namespace) -> None:
    source_root, control_root, evidence_root = pathlib.Path(arguments.source_root).resolve(), pathlib.Path(arguments.control_root).resolve(), pathlib.Path(arguments.evidence_root).resolve()
    if control_root.exists():
        raise EvidenceError(f"control root already exists: {control_root}")
    if evidence_root.exists():
        raise EvidenceError(f"evidence root must not exist during preparation: {evidence_root}")
    candidate = clean_candidate(source_root)
    inputs = required_inputs(arguments)
    profile = validate_profile(inputs["profile"])
    plan = {"schema_version": 1, "kind": "reproducible-experiment-launch-plan", "experiment_id": profile["experiment_id"], "cells": command_plan(profile, source_root)}
    control_root.mkdir(parents=True)
    plan_path = control_root / "launch-plan.json"
    write_json(plan_path, plan)
    manifest = {
        "schema_version": 2, "kind": "reproducible-experiment-prepared-manifest", "state": "PREPARED", "execution_requested": False,
        "candidate": candidate, "evidence_root": str(evidence_root), "inputs": input_identity(inputs), "tools": _tool_environment(),
        "profile_sha256": sha256_file(inputs["profile"]), "launch_plan": {"path": str(plan_path), "sha256": sha256_file(plan_path)},
        "planned_cells": [{"cell": row["cell"], "replay": row["replay"]} for row in plan["cells"]],
        "prerequisites": profile["prerequisites"],
        "execution_envelope": {"scratch_root": str(control_root / "scratch"), "exporter_paths": [
            str(control_root / "scratch" / "cells" / row["cell"] / f"replay-{row['replay']}" / "build" / "apmesh_core_numeric_contract_export") for row in plan["cells"]]},
        "prepared_utc": utc_now(),
    }
    manifest_path = control_root / "prepared-manifest.json"
    write_json(manifest_path, manifest)
    write_state(control_root, "PREPARED", {"prepared_manifest_sha256": sha256_file(manifest_path), "execution_requested": False})


def load_prepared(arguments: argparse.Namespace) -> tuple[pathlib.Path, pathlib.Path, dict[str, Any], dict[str, Any]]:
    source_root, control_root, evidence_root = pathlib.Path(arguments.source_root).resolve(), pathlib.Path(arguments.control_root).resolve(), pathlib.Path(arguments.evidence_root).resolve()
    manifest_path, plan_path = _prepared_paths(control_root)
    manifest, plan = read_json(manifest_path), read_json(plan_path)
    expected_manifest = {"schema_version", "kind", "state", "execution_requested", "candidate", "evidence_root", "inputs", "tools", "profile_sha256", "launch_plan", "planned_cells", "prerequisites", "execution_envelope", "prepared_utc"}
    if set(manifest) != expected_manifest or manifest["schema_version"] != 2 or manifest["kind"] != "reproducible-experiment-prepared-manifest" or manifest["state"] != "PREPARED" or manifest["execution_requested"] is not False:
        raise EvidenceError("manifest is not a PREPARED executable plan")
    if manifest["candidate"] != clean_candidate(source_root):
        raise EvidenceError("prepared candidate differs")
    if manifest["evidence_root"] != str(evidence_root) or manifest["launch_plan"] != {"path": str(plan_path), "sha256": sha256_file(plan_path)}:
        raise EvidenceError("prepared root or launch plan differs")
    inputs = required_inputs(arguments)
    try:
        verify_input_identity(manifest["inputs"], inputs)
    except RuntimeErrorEvidence as error:
        raise EvidenceError(str(error)) from error
    profile = validate_profile(inputs["profile"])
    expected_plan = {"schema_version": 1, "kind": "reproducible-experiment-launch-plan", "experiment_id": profile["experiment_id"], "cells": command_plan(profile, source_root)}
    if plan != expected_plan or manifest["profile_sha256"] != sha256_file(inputs["profile"]) or manifest["planned_cells"] != [{"cell": row["cell"], "replay": row["replay"]} for row in plan["cells"]]:
        raise EvidenceError("prepared plan differs")
    if manifest["tools"] != _tool_environment() or manifest["prerequisites"] != profile["prerequisites"]:
        raise EvidenceError("prepared tool or prerequisite identity differs")
    expected_exporters = [str(control_root / "scratch" / "cells" / row["cell"] / f"replay-{row['replay']}" / "build" / "apmesh_core_numeric_contract_export") for row in plan["cells"]]
    if manifest["execution_envelope"] != {"scratch_root": str(control_root / "scratch"), "exporter_paths": expected_exporters}:
        raise EvidenceError("prepared executable envelope differs")
    return control_root, evidence_root, manifest, plan


def admit_execution(arguments: argparse.Namespace) -> tuple[pathlib.Path, pathlib.Path, dict[str, Any], dict[str, Any]]:
    control_root, evidence_root, manifest, plan = load_prepared(arguments)
    state = read_json(control_root / "state.json")
    if state.get("state") != "PREPARED":
        raise EvidenceError("terminal or running control state cannot be reused")
    require_empty_evidence_root(evidence_root)
    return control_root, evidence_root, manifest, plan


def require_empty_evidence_root(evidence_root: pathlib.Path) -> None:
    if evidence_root.exists() and any(evidence_root.iterdir()):
        raise EvidenceError("evidence root is not empty")


def _replace_root(argv: list[str], scratch_root: pathlib.Path) -> list[str]:
    return [part.replace("@SCRATCH_ROOT@", str(scratch_root)) for part in argv]


def _command_or_blocked(argv: list[str], cwd: pathlib.Path, bundle: pathlib.Path, name: str, timeout: int,
                        records: list[dict[str, Any]]) -> bool:
    record = run_command(argv, cwd, bundle / "logs", name, timeout)
    records.append(record)
    return record["exit_code"] == 0 and not record["timed_out"] and record["launch_error"] is None


def _write_bundle(bundle: pathlib.Path, row: dict[str, Any], profile: dict[str, Any], source_root: pathlib.Path,
                  scratch_root: pathlib.Path, deadline: float) -> None:
    records: list[dict[str, Any]] = []
    bundle.mkdir(parents=True)
    failure: str | None = None
    for stage in ("configure", "build", "ctest"):
        if time.monotonic() >= deadline:
            failure = "overall timeout"
            break
        if not _command_or_blocked(_replace_root(row[stage], scratch_root), source_root, bundle, stage, profile["limits_seconds"][stage], records):
            failure = f"command failed: {stage}"
            break
    exporter = scratch_root / "cells" / row["cell"] / f"replay-{row['replay']}" / "build" / "apmesh_core_numeric_contract_export"
    for mode, filename in (("certificate", "certificate.json"), ("environment", "environment.json")):
        if failure is not None:
            break
        if time.monotonic() >= deadline:
            failure = "overall timeout"
            break
        if not _command_or_blocked([str(exporter), mode, str(bundle / filename)], bundle, bundle, mode, profile["limits_seconds"]["export"], records):
            failure = f"command failed: {mode}"
            break
    write_json(bundle / "execution-record.json", {"schema_version": 1, "kind": "reproducible-experiment-execution", "cell": row["cell"], "replay": row["replay"], "records": records})
    if failure is not None:
        raise EvidenceError(failure)
    certificate, environment = read_json(bundle / "certificate.json"), read_json(bundle / "environment.json")
    summary = {"schema_version": 1, "kind": "reproducible-experiment-summary", "cell": row["cell"], "replay": row["replay"],
               "gates": {gate: "EVIDENCE_COLLECTED_PENDING_AUDIT" for gate in profile["gates"]}, "certificate": certificate,
               "environment": environment, "artifact_roles": profile["required_artifacts"], "retained_limitations": profile["limitations"]}
    write_json(bundle / "summary.json", summary); write_derived(bundle, [summary])
    execution = read_json(bundle / "execution-record.json")
    write_json(bundle / "artifact-inventory.json", {"schema_version": 1, "kind": "experiment-artifact-inventory", "artifacts": artifact_inventory(bundle, profile, execution)})


def _run_qualified_prerequisites(source_root: pathlib.Path, evidence_root: pathlib.Path, profile: dict[str, Any], deadline: float) -> dict[str, Any]:
    """Invoke the existing qualified protocols without absorbing their oracles."""
    numeric_root = evidence_root / "prerequisites" / "numeric-contract"
    command = [
        sys.executable, str(source_root / "tools" / "run_numeric_contract_regression.py"),
        "--source-root", str(source_root),
        "--profile", str(source_root / "experiments" / "profiles" / "numeric_contract.json"),
        "--protocol", str(source_root / "docs" / "decisions" / "FOUNDATION_NUMERIC_CONTRACT_QUALIFICATION.md"),
        "--validator", str(source_root / "tools" / "numeric_contract_evidence.py"),
        "--architecture-evidence", str(source_root / "docs" / "decisions" / "FOUNDATION_ARCHITECTURE_BOOTSTRAP_REGRESSION.md"),
        "--architecture-runner", str(source_root / "tools" / "run_architecture_bootstrap_regression.py"),
        "--architecture-profile", str(source_root / "experiments" / "profiles" / "architecture_bootstrap.json"),
        "--architecture-expected", str(source_root / "experiments" / "expected" / "bootstrap_certificate.json"),
        "--architecture-comparer", str(source_root / "tools" / "bootstrap_regression.py"),
        "--output-root", str(numeric_root),
    ]
    if time.monotonic() >= deadline:
        raise EvidenceError("overall timeout before prerequisites")
    prepare_record = run_command(command, source_root, evidence_root / "prerequisite-logs", "numeric-contract-prepare", max(1, int(deadline - time.monotonic())))
    if prepare_record["exit_code"] != 0 or prepare_record["timed_out"] or prepare_record["launch_error"] is not None:
        raise EvidenceError("qualified Numeric prerequisite preparation failed")
    execute_record = run_command([*command, "--execute"], source_root, evidence_root / "prerequisite-logs", "numeric-contract-execute", max(1, int(deadline - time.monotonic())))
    if execute_record["exit_code"] != 0 or execute_record["timed_out"] or execute_record["launch_error"] is not None:
        raise EvidenceError("qualified Numeric prerequisite protocol failed")
    manifest_path = numeric_root / "manifest.json"
    manifest = read_json(manifest_path)
    if manifest.get("state") != "EXECUTED_PENDING_AUDIT":
        raise EvidenceError("qualified Numeric prerequisite terminal state differs")
    architecture_manifest = numeric_root / "architecture" / "manifest.json"
    if not architecture_manifest.is_file() or read_json(architecture_manifest).get("state") != "EXECUTED_PENDING_AUDIT":
        raise EvidenceError("qualified Architecture prerequisite terminal state differs")
    return {"numeric_contract": {"prepare_record": prepare_record, "execute_record": execute_record,
            "manifest": str(manifest_path.relative_to(evidence_root)), "manifest_sha256": sha256_file(manifest_path)},
            "architecture_contract": {"manifest": str(architecture_manifest.relative_to(evidence_root)), "manifest_sha256": sha256_file(architecture_manifest)}}


def _run_negative_fixtures(source_root: pathlib.Path, evidence_root: pathlib.Path, profile: dict[str, Any], deadline: float) -> list[dict[str, Any]]:
    records = []
    tool = source_root / "tools" / "reproducible_experiment_negative.py"
    for number in range(1, 9):
        if time.monotonic() >= deadline:
            raise EvidenceError("overall timeout during negative fixtures")
        fixture = f"N{number}"
        record = run_command([sys.executable, str(tool), "--fixture", fixture, "--profile", str(source_root / "experiments" / "profiles" / "reproducible_experiment_contract.json")], source_root, evidence_root / "negative-fixtures" / fixture / "logs", fixture, max(1, int(deadline - time.monotonic())))
        if record["exit_code"] != 0 or record["timed_out"] or record["launch_error"] is not None:
            raise EvidenceError(f"negative fixture failed: {fixture}")
        records.append({"fixture": fixture, "expected": profile["negative_fixtures"][fixture], "result": "REJECTED", "record": record})
    return records


def execute(arguments: argparse.Namespace) -> int:
    control_root, evidence_root, manifest, plan = admit_execution(arguments)
    profile = validate_profile(pathlib.Path(arguments.profile))
    evidence_root.mkdir(parents=True, exist_ok=False)
    write_state(control_root, "RUNNING", {"prepared_manifest_sha256": sha256_file(control_root / "prepared-manifest.json"), "evidence_root": str(evidence_root)})
    completed: list[dict[str, Any]] = []
    deadline = time.monotonic() + profile["limits_seconds"]["overall"]
    try:
        scratch_root = control_root / "scratch"
        for row in plan["cells"]:
            bundle = evidence_root / "cells" / row["cell"] / f"replay-{row['replay']}"
            completed.append({"cell": row["cell"], "replay": row["replay"], "bundle": str(bundle.relative_to(evidence_root))})
            _write_bundle(bundle, row, profile, pathlib.Path(arguments.source_root).resolve(), scratch_root, deadline)
        replay_reports = []
        cell_summaries: dict[str, dict[str, Any]] = {}
        provenance_by_cell: dict[str, dict[str, Any]] = {}
        for name in (item["name"] for item in profile["configurations"]):
            bundles = [evidence_root / "cells" / name / f"replay-{number}" for number in (1, 2)]
            replay_reports.append(compare_replays(bundles, profile))
            cell_summaries[name] = read_json(bundles[0] / "summary.json")
            provenance_by_cell[name] = read_json(bundles[0] / "execution-record.json")
        cross = compare_configurations(cell_summaries, profile, provenance_by_cell)
        negative_fixtures = _run_negative_fixtures(pathlib.Path(arguments.source_root).resolve(), evidence_root, profile, deadline)
        prerequisite_evidence = _run_qualified_prerequisites(pathlib.Path(arguments.source_root).resolve(), evidence_root, profile, deadline)
        seals = []
        for bundle in completed:
            root = evidence_root / bundle["bundle"]
            seals.append({"bundle": bundle["bundle"], "inventory_sha256": sha256_file(root / "artifact-inventory.json"),
                          "summary_sha256": sha256_file(root / "summary.json"), "execution_sha256": sha256_file(root / "execution-record.json")})
        terminal = {"schema_version": 1, "kind": "reproducible-experiment-terminal-manifest", "state": "EXECUTED_PENDING_AUDIT",
                    "prepared_manifest_sha256": sha256_file(control_root / "prepared-manifest.json"), "bundles": completed,
                    "bundle_seals": seals, "replay_comparisons": replay_reports, "cross_configuration": cross,
                    "negative_fixtures": negative_fixtures, "prerequisites": prerequisite_evidence}
        write_json(evidence_root / "terminal-manifest.json", terminal)
        validate_terminal_manifest(evidence_root, profile, terminal["prepared_manifest_sha256"])
        write_state(control_root, "EXECUTED_PENDING_AUDIT", {"terminal_manifest": str(evidence_root / "terminal-manifest.json"), "terminal_sha256": sha256_file(evidence_root / "terminal-manifest.json")})
        return 0
    except (EvidenceError, RuntimeErrorEvidence) as error:
        terminal = {"schema_version": 1, "kind": "reproducible-experiment-terminal-manifest", "state": "BLOCKED",
                    "prepared_manifest_sha256": sha256_file(control_root / "prepared-manifest.json"), "attempted_bundles": completed, "reason": str(error)}
        write_json(evidence_root / "terminal-manifest.json", terminal)
        write_state(control_root, "BLOCKED", {"terminal_manifest": str(evidence_root / "terminal-manifest.json"), "reason": str(error)})
        return 1


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    for name in ("source-root", "profile", "contract", "protocol", "collector", "control-root", "evidence-root"):
        parser.add_argument(f"--{name}", required=True)
    parser.add_argument("--execute", action="store_true")
    return parser.parse_args()


def main() -> int:
    arguments = parse_arguments()
    try:
        if arguments.execute:
            return execute(arguments)
        prepare(arguments)
        return 0
    except (EvidenceError, RuntimeErrorEvidence, OSError) as error:
        print(error, file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
