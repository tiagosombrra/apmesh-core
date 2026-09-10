#!/usr/bin/env python3
"""Strict, report-only evidence checks for the Reproducible Experiment Contract."""

from __future__ import annotations

import argparse
import datetime as dt
import fnmatch
import math
import pathlib
import subprocess
import sys
from typing import Any

from experiment_runtime import (RuntimeErrorEvidence, canonical_json, inventory_files, read_json,
                                require_exact_keys, sha256_file, write_json)
from bootstrap_regression import EvidenceError as BootstrapEvidenceError
from bootstrap_regression import validate_certificate as validate_bootstrap_certificate
from numeric_contract_evidence import EvidenceError as NumericEvidenceError
from numeric_contract_evidence import validate_certificate as validate_numeric_certificate
from numeric_contract_evidence import validate_environment as validate_numeric_environment


class EvidenceError(RuntimeErrorEvidence):
    pass


EXPECTED_CONFIGURATION_NAMES = ("gcc-debug", "gcc-release", "clang-debug", "clang-release")
EXPECTED_GATES = ("E0", "E1", "E2", "E3", "E4", "E5", "E6", "E7")
REQUIRED_ARTIFACTS = (
    "manifest.json", "certificate.json", "environment.json", "execution-record.json", "artifact-inventory.json",
    "summary.json", "report.md", "metrics.csv", "figures/status-matrix.svg",
)
DERIVED_ARTIFACTS = ("report.md", "metrics.csv", "figures/status-matrix.svg")


def read_bytes(path: pathlib.Path) -> bytes:
    from experiment_runtime import read_bytes as read
    return read(path)


def sha256(path: pathlib.Path) -> str:
    return sha256_file(path)


def read_json_object(path: pathlib.Path) -> dict[str, Any]:
    try:
        return read_json(path)
    except RuntimeErrorEvidence as error:
        raise EvidenceError(str(error)) from error


def canonical(value: Any) -> bytes:
    return canonical_json(value)


def write_json_compat(path: pathlib.Path, value: Any) -> None:
    write_json(path, value)


def write_derived(root: pathlib.Path, summaries: list[dict[str, Any]]) -> None:
    rows = sorted(summaries, key=lambda row: (row["cell"], row["replay"]))
    markdown = ["# Reproducible Experiment Contract Evidence", "", "| Cell | Replay | Evidence |", "| --- | ---: | --- |"]
    markdown.extend(f"| {row['cell']} | {row['replay']} | bundle validated |" for row in rows)
    markdown.extend(["", "Qualification: PENDING SCIENTIFIC AUDIT", ""])
    (root / "report.md").write_text("\n".join(markdown), encoding="utf-8", newline="\n")
    (root / "metrics.csv").write_text("cell,replay,evidence_state\n" + "".join(
        f"{row['cell']},{row['replay']},bundle_validated\n" for row in rows), encoding="utf-8", newline="\n")
    (root / "figures").mkdir(parents=True, exist_ok=True)
    svg_rows = "".join(f'<text x="10" y="{30 + index * 20}">{row["cell"]} replay {row["replay"]}: bundle validated</text>' for index, row in enumerate(rows))
    (root / "figures" / "status-matrix.svg").write_text(
        f'<svg xmlns="http://www.w3.org/2000/svg" width="720" height="{50 + len(rows) * 20}"><text x="10" y="15">REC evidence status</text>{svg_rows}</svg>\n', encoding="utf-8", newline="\n")


def derived_bytes(summary: dict[str, Any]) -> dict[str, bytes]:
    # Keep one deterministic derivation definition for writing and validation.
    markdown = ["# Reproducible Experiment Contract Evidence", "", "| Cell | Replay | Evidence |", "| --- | ---: | --- |",
                f"| {summary['cell']} | {summary['replay']} | bundle validated |", "", "Qualification: PENDING SCIENTIFIC AUDIT", ""]
    metrics = f"cell,replay,evidence_state\n{summary['cell']},{summary['replay']},bundle_validated\n"
    svg = f'<svg xmlns="http://www.w3.org/2000/svg" width="720" height="70"><text x="10" y="15">REC evidence status</text><text x="10" y="30">{summary["cell"]} replay {summary["replay"]}: bundle validated</text></svg>\n'
    return {"report.md": "\n".join(markdown).encode("utf-8"), "metrics.csv": metrics.encode("utf-8"), "figures/status-matrix.svg": svg.encode("utf-8")}


def validate_profile(path: pathlib.Path) -> dict[str, Any]:
    try:
        value = read_json(path)
        require_exact_keys(value, {
            "schema_version", "experiment_id", "question", "specimen", "replays_per_cell", "configurations",
            "commands", "required_artifacts", "optional_artifacts", "artifact_schemas", "derivations",
            "claim_fields", "volatile_fields", "equivalence_rules", "gates", "limits_seconds", "limitations",
            "provenance_difference_rules", "gate_conditions", "negative_fixtures", "prerequisites",
        }, "REC profile")
    except RuntimeErrorEvidence as error:
        raise EvidenceError(str(error)) from error
    if value["schema_version"] != 2 or value["experiment_id"] != "foundation-reproducible-experiment-contract" or value["specimen"] != "numeric-contract":
        raise EvidenceError("REC profile identity differs")
    if value["replays_per_cell"] != 2 or value["gates"] != list(EXPECTED_GATES):
        raise EvidenceError("REC profile replay or gate declaration differs")
    configurations = value["configurations"]
    if not isinstance(configurations, list) or tuple(entry.get("name") for entry in configurations if isinstance(entry, dict)) != EXPECTED_CONFIGURATION_NAMES:
        raise EvidenceError("REC profile configuration matrix differs")
    for configuration in configurations:
        if not isinstance(configuration, dict):
            raise EvidenceError("REC configuration schema differs")
        try:
            require_exact_keys(configuration, {"name", "preset", "compiler", "standard_library", "build_type"}, "REC configuration")
        except RuntimeErrorEvidence as error:
            raise EvidenceError(str(error)) from error
        if not all(isinstance(item, str) and item for item in configuration.values()):
            raise EvidenceError("REC configuration values differ")
    if tuple(value["required_artifacts"]) != REQUIRED_ARTIFACTS or not isinstance(value["optional_artifacts"], list):
        raise EvidenceError("REC profile artifact declaration differs")
    if set(value["commands"]) != {"configure", "build", "ctest", "export", "prerequisite_protocols"}:
        raise EvidenceError("REC profile command declaration differs")
    if not all(isinstance(item, str) and item for item in value["commands"].values()):
        raise EvidenceError("REC command declaration values differ")
    if set(value["artifact_schemas"]) != set(REQUIRED_ARTIFACTS):
        raise EvidenceError("REC profile artifact schemas differ")
    if set(value["derivations"]) != set(DERIVED_ARTIFACTS):
        raise EvidenceError("REC profile derivations differ")
    if set(value["equivalence_rules"]) != set(REQUIRED_ARTIFACTS):
        raise EvidenceError("REC profile equivalence rules differ")
    if not all(isinstance(item, str) and item for item in value["claim_fields"] + value["volatile_fields"] + value["limitations"]):
        raise EvidenceError("REC profile string fields differ")
    rules = value["provenance_difference_rules"]
    if not isinstance(rules, dict) or set(rules) != {"replay", "configuration"}:
        raise EvidenceError("REC provenance difference rules differ")
    for scope, entries in rules.items():
        if not isinstance(entries, list) or not entries:
            raise EvidenceError(f"REC {scope} provenance rules differ")
        for entry in entries:
            if not isinstance(entry, dict):
                raise EvidenceError("REC provenance difference rule schema differs")
            try:
                require_exact_keys(entry, {"path", "classification"}, "REC provenance difference rule")
            except RuntimeErrorEvidence as error:
                raise EvidenceError(str(error)) from error
            if not all(isinstance(item, str) and item.startswith("/") for item in (entry["path"],)) or not isinstance(entry["classification"], str) or not entry["classification"].startswith("declared-"):
                raise EvidenceError("REC provenance difference rule values differ")
        volatile_paths = {entry["path"] for entry in entries if entry["classification"] == "declared-volatile-provenance"}
        if volatile_paths != set(value["volatile_fields"]):
            raise EvidenceError(f"REC {scope} volatile provenance rules differ")
    if set(value["gate_conditions"]) != set(EXPECTED_GATES) or set(value["negative_fixtures"]) != {f"N{index}" for index in range(1, 9)}:
        raise EvidenceError("REC profile gate or negative fixture declaration differs")
    for condition in value["gate_conditions"].values():
        if not isinstance(condition, dict):
            raise EvidenceError("REC gate condition schema differs")
        try:
            require_exact_keys(condition, {"pass", "blocked"}, "REC gate condition")
        except RuntimeErrorEvidence as error:
            raise EvidenceError(str(error)) from error
        if not all(isinstance(item, str) and item for item in condition.values()):
            raise EvidenceError("REC gate condition values differ")
    if not all(isinstance(item, str) and item for item in value["artifact_schemas"].values()) or not all(isinstance(item, str) and item for item in value["derivations"].values()) or not all(isinstance(item, str) and item for item in value["equivalence_rules"].values()):
        raise EvidenceError("REC profile declaration values differ")
    for fixture, declaration in value["negative_fixtures"].items():
        if not isinstance(declaration, dict):
            raise EvidenceError("REC negative fixture declaration differs")
        try:
            require_exact_keys(declaration, {"reason_code", "description"}, "REC negative fixture")
        except RuntimeErrorEvidence as error:
            raise EvidenceError(str(error)) from error
        if not all(isinstance(item, str) and item for item in declaration.values()):
            raise EvidenceError("REC negative fixture values differ")
    if value["prerequisites"] != ["architecture-contract", "numeric-contract"]:
        raise EvidenceError("REC prerequisite declaration differs")
    limits = value["limits_seconds"]
    if not isinstance(limits, dict) or set(limits) != {"configure", "build", "ctest", "export", "overall"} or not all(isinstance(item, int) and item > 0 for item in limits.values()):
        raise EvidenceError("REC profile limits differ")
    return value


def _require_file_set(root: pathlib.Path, profile: dict[str, Any], execution: dict[str, Any]) -> None:
    expected = set(profile["required_artifacts"] + profile["optional_artifacts"])
    for record in execution["records"]:
        for stream in ("stdout", "stderr"):
            expected.add(record[stream]["path"])
    observed = {item.relative_to(root).as_posix() for item in root.rglob("*") if item.is_file()}
    if observed != expected:
        raise EvidenceError(f"bundle artifacts differ; missing={sorted(expected - observed)}, extra={sorted(observed - expected)}")


def _validate_execution(root: pathlib.Path) -> dict[str, Any]:
    value = read_json_object(root / "execution-record.json")
    required = {"schema_version", "kind", "cell", "replay", "records"}
    if set(value) != required or value["schema_version"] != 1 or value["kind"] != "reproducible-experiment-execution" or not isinstance(value["records"], list) or not value["records"]:
        raise EvidenceError("execution record schema differs")
    record_keys = {"schema_version", "kind", "id", "argv", "cwd", "environment_delta", "started_utc", "ended_utc", "elapsed_seconds", "pid", "timeout_seconds", "timed_out", "exit_code", "launch_error", "stdout", "stderr"}
    for record in value["records"]:
        _validate_rec_command_record(record, root, record_keys)
    return value


def _utc_timestamp(value: Any, context: str) -> dt.datetime:
    if not isinstance(value, str):
        raise EvidenceError(f"{context} timestamp differs")
    try:
        timestamp = dt.datetime.fromisoformat(value.replace("Z", "+00:00"))
    except ValueError as error:
        raise EvidenceError(f"{context} timestamp differs") from error
    if timestamp.tzinfo is None or timestamp.utcoffset() != dt.timedelta():
        raise EvidenceError(f"{context} timestamp differs")
    return timestamp


def _positive_integer(value: Any, context: str) -> int:
    if isinstance(value, bool) or not isinstance(value, int) or value <= 0:
        raise EvidenceError(f"{context} differs")
    return value


def _nonnegative_duration(value: Any, context: str) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)) or not math.isfinite(value) or value < 0:
        raise EvidenceError(f"{context} differs")
    return float(value)


def _sha256_text(value: Any, context: str) -> str:
    if not isinstance(value, str) or len(value) != 64 or any(character not in "0123456789abcdef" for character in value):
        raise EvidenceError(f"{context} differs")
    return value


def _validated_path(root: pathlib.Path, value: Any, context: str) -> pathlib.Path:
    if not isinstance(value, str) or not value:
        raise EvidenceError(f"{context} path differs")
    path = pathlib.Path(value)
    if not path.is_absolute():
        path = root / path
    try:
        resolved = path.resolve()
        resolved.relative_to(root.resolve())
    except (OSError, ValueError) as error:
        raise EvidenceError(f"{context} path escapes evidence root") from error
    if not resolved.is_file():
        raise EvidenceError(f"{context} artifact is absent")
    return resolved


def _validate_rec_command_record(record: Any, root: pathlib.Path, record_keys: set[str] | None = None) -> None:
    expected = record_keys or {"schema_version", "kind", "id", "argv", "cwd", "environment_delta", "started_utc", "ended_utc", "elapsed_seconds", "pid", "timeout_seconds", "timed_out", "exit_code", "launch_error", "stdout", "stderr"}
    if not isinstance(record, dict) or set(record) != expected or record["schema_version"] != 1 or record["kind"] != "experiment-command-record":
        raise EvidenceError("command record schema differs")
    if not isinstance(record["id"], str) or not record["id"] or not isinstance(record["argv"], list) or not record["argv"] or any(not isinstance(item, str) or not item for item in record["argv"]):
        raise EvidenceError("command record provenance differs")
    if not isinstance(record["cwd"], str) or not record["cwd"] or not isinstance(record["environment_delta"], dict) or any(not isinstance(key, str) or not isinstance(value, str) for key, value in record["environment_delta"].items()):
        raise EvidenceError("command record provenance differs")
    started, ended = _utc_timestamp(record["started_utc"], "command start"), _utc_timestamp(record["ended_utc"], "command end")
    elapsed = _nonnegative_duration(record["elapsed_seconds"], "command elapsed duration")
    if ended < started or abs(elapsed - (ended - started).total_seconds()) > 1.0:
        raise EvidenceError("command duration chronology differs")
    _positive_integer(record["pid"], "command PID")
    _positive_integer(record["timeout_seconds"], "command timeout")
    for stream in ("stdout", "stderr"):
        item = record[stream]
        if not isinstance(item, dict) or set(item) != {"path", "sha256"}:
            raise EvidenceError("command log declaration differs")
        path = _validated_path(root, item.get("path"), "command log")
        if sha256(path) != _sha256_text(item.get("sha256"), "command log hash"):
            raise EvidenceError("command log identity differs")
    if isinstance(record["exit_code"], bool) or not isinstance(record["exit_code"], int) or record["exit_code"] != 0 or record["timed_out"] is not False or record["launch_error"] is not None:
        raise EvidenceError("accepted bundle command did not succeed")


def _validate_launch_plan(plan: Any, profile: dict[str, Any]) -> dict[tuple[str, int], dict[str, Any]]:
    if not isinstance(plan, dict) or set(plan) != {"schema_version", "kind", "experiment_id", "cells"}:
        raise EvidenceError("launch plan schema differs")
    if plan["schema_version"] != 1 or plan["kind"] != "reproducible-experiment-launch-plan" or plan["experiment_id"] != profile["experiment_id"]:
        raise EvidenceError("launch plan identity differs")
    if not isinstance(plan["cells"], list):
        raise EvidenceError("launch plan cells differ")
    expected = {(configuration["name"], replay) for configuration in profile["configurations"] for replay in (1, 2)}
    rows: dict[tuple[str, int], dict[str, Any]] = {}
    for row in plan["cells"]:
        if not isinstance(row, dict) or set(row) != {"cell", "replay", "configure", "build", "ctest"}:
            raise EvidenceError("launch plan row schema differs")
        key = (row["cell"], row["replay"])
        if key in rows or key not in expected or not all(isinstance(row[name], list) and row[name] and all(isinstance(part, str) and part for part in row[name]) for name in ("configure", "build", "ctest")):
            raise EvidenceError("launch plan row differs")
        rows[key] = row
    if set(rows) != expected:
        raise EvidenceError("launch plan matrix differs")
    return rows


def _replace_scratch_root(argv: list[str], scratch_root: pathlib.Path) -> list[str]:
    return [part.replace("@SCRATCH_ROOT@", str(scratch_root)) for part in argv]


def _validate_execution_against_plan(bundle_root: pathlib.Path, bundle_relative: str, row: dict[str, Any],
                                     profile: dict[str, Any], source_root: pathlib.Path,
                                     scratch_root: pathlib.Path, evidence_root: pathlib.Path) -> None:
    execution = _validate_execution(bundle_root)
    if (execution["cell"], execution["replay"]) != (row["cell"], row["replay"]):
        raise EvidenceError("observed execution identity differs from launch plan")
    declared_bundle = evidence_root / bundle_relative
    exporter = scratch_root / "cells" / row["cell"] / f"replay-{row['replay']}" / "build" / "apmesh_core_numeric_contract_export"
    expected = [
        (stage, _replace_scratch_root(row[stage], scratch_root), source_root, profile["limits_seconds"][stage])
        for stage in ("configure", "build", "ctest")
    ]
    expected.extend((mode, [str(exporter), mode, str(declared_bundle / filename)], declared_bundle,
                     profile["limits_seconds"]["export"])
                    for mode, filename in (("certificate", "certificate.json"), ("environment", "environment.json")))
    if len(execution["records"]) != len(expected):
        raise EvidenceError("observed command count differs from launch plan")
    for record, (record_id, argv, cwd, timeout) in zip(execution["records"], expected, strict=True):
        if record["id"] != record_id or record["argv"] != argv or record["cwd"] != str(cwd.resolve()) or record["environment_delta"] != {} or record["timeout_seconds"] != timeout:
            raise EvidenceError("observed command differs from launch plan")


def _validate_bundle_manifest(root: pathlib.Path, profile: dict[str, Any]) -> dict[str, Any]:
    manifest = read_json_object(root / "manifest.json")
    expected = {"schema_version", "kind", "cell", "replay", "candidate_commit", "profile_sha256",
                "prepared_manifest_sha256", "launch_plan_sha256"}
    if set(manifest) != expected or manifest["schema_version"] != 1 or manifest["kind"] != "reproducible-experiment-bundle-manifest":
        raise EvidenceError("bundle manifest schema differs")
    if not isinstance(manifest["cell"], str) or not isinstance(manifest["replay"], int) or manifest["replay"] not in {1, 2}:
        raise EvidenceError("bundle manifest identity differs")
    if not isinstance(manifest["candidate_commit"], str) or len(manifest["candidate_commit"]) != 40 or any(not isinstance(manifest[name], str) or len(manifest[name]) != 64 for name in ("profile_sha256", "prepared_manifest_sha256", "launch_plan_sha256")):
        raise EvidenceError("bundle manifest hash identity differs")
    if manifest["profile_sha256"] == "0" * 64:
        raise EvidenceError("bundle manifest profile identity differs")
    return manifest


def _validate_summary(root: pathlib.Path, profile: dict[str, Any]) -> dict[str, Any]:
    summary = read_json_object(root / "summary.json")
    expected = {"schema_version", "kind", "cell", "replay", "gates", "certificate", "environment", "artifact_roles", "retained_limitations"}
    if set(summary) != expected or summary["schema_version"] != 1 or summary["kind"] != "reproducible-experiment-summary":
        raise EvidenceError("summary schema differs")
    if not isinstance(summary["cell"], str) or not isinstance(summary["replay"], int) or summary["artifact_roles"] != profile["required_artifacts"]:
        raise EvidenceError("summary identity or role declaration differs")
    if set(summary["gates"]) != set(profile["gates"]) or any(value != "BUNDLE_VALIDATED" for value in summary["gates"].values()):
        raise EvidenceError("summary gate state differs")
    return summary


def _validate_declared_schema(root: pathlib.Path, profile: dict[str, Any]) -> None:
    for relative, declaration in profile["artifact_schemas"].items():
        path = root / relative
        if not path.is_file() or not isinstance(declaration, str) or "/v" not in declaration:
            raise EvidenceError("artifact schema declaration differs")
        if relative.endswith(".json"):
            value = read_json_object(path)
            kind, version = declaration.rsplit("/v", 1)
            if value.get("kind") != kind or value.get("schema_version") != int(version):
                raise EvidenceError(f"artifact schema differs: {relative}")


def _inventory_entries(root: pathlib.Path, profile: dict[str, Any], execution: dict[str, Any]) -> list[tuple[str, str, str, dict[str, Any] | None]]:
    entries: list[tuple[str, str, str, dict[str, Any] | None]] = []
    summary_sha = sha256(root / "summary.json")
    for path in profile["required_artifacts"]:
        if path == "artifact-inventory.json":
            continue
        derivation = {"source_path": "summary.json", "source_sha256": summary_sha} if path in DERIVED_ARTIFACTS else None
        entries.append((path, path, profile["artifact_schemas"][path], derivation))
    for record in execution["records"]:
        entries.extend((record[stream]["path"], f"command-{stream}", "experiment-log/v1", None) for stream in ("stdout", "stderr"))
    return entries


def artifact_inventory(root: pathlib.Path, profile: dict[str, Any], execution: dict[str, Any]) -> list[dict[str, Any]]:
    return inventory_files(root, _inventory_entries(root, profile, execution))


def require_inventory_rows(declared: list[Any], expected: list[dict[str, Any]]) -> None:
    if declared != expected:
        raise EvidenceError("artifact inventory differs")


def validate_bundle(root: pathlib.Path, profile: dict[str, Any]) -> dict[str, Any]:
    for artifact in profile["required_artifacts"]:
        if not (root / artifact).is_file():
            raise EvidenceError(f"required artifact is absent: {root / artifact}")
    bundle_manifest = _validate_bundle_manifest(root, profile)
    execution = _validate_execution(root)
    summary = _validate_summary(root, profile)
    if (execution["cell"], execution["replay"]) != (summary["cell"], summary["replay"]) or (execution["cell"], execution["replay"]) != (bundle_manifest["cell"], bundle_manifest["replay"]):
        raise EvidenceError("execution and summary identity differ")
    _require_file_set(root, profile, execution)
    _validate_declared_schema(root, profile)
    inventory = read_json_object(root / "artifact-inventory.json")
    try:
        require_exact_keys(inventory, {"schema_version", "kind", "artifacts"}, "artifact inventory")
    except RuntimeErrorEvidence as error:
        raise EvidenceError(str(error)) from error
    expected = artifact_inventory(root, profile, execution)
    if inventory["schema_version"] != 1 or inventory["kind"] != "experiment-artifact-inventory":
        raise EvidenceError("artifact inventory schema differs")
    require_inventory_rows(inventory["artifacts"], expected)
    expected_derived = derived_bytes(summary)
    for relative, expected_bytes in expected_derived.items():
        if read_bytes(root / relative) != expected_bytes:
            raise EvidenceError(f"derived artifact differs: {relative}")
    return summary


def semantic_summary(summary: dict[str, Any], profile: dict[str, Any]) -> bytes:
    return canonical({key: summary[key] for key in profile["claim_fields"]})


def _flatten(value: Any, path: str = "") -> dict[str, Any]:
    if isinstance(value, dict):
        result: dict[str, Any] = {}
        for key in sorted(value):
            result.update(_flatten(value[key], f"{path}/{key}"))
        return result
    if isinstance(value, list):
        result: dict[str, Any] = {}
        for index, item in enumerate(value):
            result.update(_flatten(item, f"{path}/{index}"))
        return result
    return {path or "/": value}


def _provenance_differences(baseline: dict[str, Any], observed: dict[str, Any], profile: dict[str, Any], scope: str) -> list[dict[str, Any]]:
    baseline_fields, observed_fields = _flatten(baseline), _flatten(observed)
    if set(baseline_fields) != set(observed_fields):
        raise EvidenceError(f"{scope} provenance structure differs")
    rules = profile["provenance_difference_rules"][scope]
    differences: list[dict[str, Any]] = []
    for path in sorted(baseline_fields):
        if baseline_fields[path] == observed_fields[path]:
            continue
        declaration = next((entry for entry in rules if fnmatch.fnmatchcase(path, entry["path"])), None)
        if declaration is None:
            raise EvidenceError(f"{scope} provenance difference is undeclared: {path}")
        differences.append({"path": path, "classification": declaration["classification"],
                            "baseline": baseline_fields[path], "observed": observed_fields[path]})
    return differences


def compare_replays(roots: list[pathlib.Path], profile: dict[str, Any]) -> dict[str, Any]:
    if len(roots) != 2:
        raise EvidenceError("exactly two replay bundles are required")
    summaries = [validate_bundle(root, profile) for root in roots]
    if summaries[0]["cell"] != summaries[1]["cell"] or {item["replay"] for item in summaries} != {1, 2}:
        raise EvidenceError("REC comparison requires replay 1 and 2 of one cell")
    if semantic_summary(summaries[0], profile) != semantic_summary(summaries[1], profile):
        raise EvidenceError("REC claim fields differ between replays")
    first_execution, second_execution = (read_json_object(root / "execution-record.json") for root in roots)
    return {"cell": summaries[0]["cell"], "replays": [1, 2], "claim_fields": "equivalent",
            "differences": _provenance_differences(first_execution, second_execution, profile, "replay")}


def compare_configurations(cell_summaries: dict[str, dict[str, Any]], profile: dict[str, Any],
                           provenance_by_cell: dict[str, dict[str, Any]] | None = None) -> dict[str, Any]:
    if tuple(sorted(cell_summaries)) != tuple(sorted(EXPECTED_CONFIGURATION_NAMES)):
        raise EvidenceError("cross-configuration comparison requires all four cells")
    baseline_name = EXPECTED_CONFIGURATION_NAMES[0]
    baseline = semantic_summary(cell_summaries[baseline_name], profile)
    differences = []
    for name in EXPECTED_CONFIGURATION_NAMES[1:]:
        if semantic_summary(cell_summaries[name], profile) != baseline:
            differences.append({"cell": name, "classification": "claim-field-difference"})
    if differences:
        raise EvidenceError(f"cross-configuration claim fields differ: {differences}")
    provenance_differences = []
    if provenance_by_cell is not None:
        if set(provenance_by_cell) != set(EXPECTED_CONFIGURATION_NAMES):
            raise EvidenceError("cross-configuration provenance matrix differs")
        baseline_provenance = provenance_by_cell[baseline_name]
        for name in EXPECTED_CONFIGURATION_NAMES[1:]:
            for difference in _provenance_differences(baseline_provenance, provenance_by_cell[name], profile, "configuration"):
                provenance_differences.append({"cell": name, **difference})
    return {"baseline": baseline_name, "cells": list(EXPECTED_CONFIGURATION_NAMES), "claim_fields": "equivalent", "differences": provenance_differences}


def _require_completed_cells(records: Any, context: str) -> list[dict[str, Any]]:
    if not isinstance(records, list):
        raise EvidenceError(f"{context} cell matrix differs")
    cells = [row for row in records if isinstance(row, dict) and "cell" in row]
    if {row.get("cell") for row in cells} != set(EXPECTED_CONFIGURATION_NAMES) or len(cells) != len(EXPECTED_CONFIGURATION_NAMES):
        raise EvidenceError(f"{context} cell matrix differs")
    for row in cells:
        if not isinstance(row, dict) or row.get("state") != "PASS" or not isinstance(row.get("records"), list) or not row["records"]:
            raise EvidenceError(f"{context} cell execution differs")
    return cells


_QUALIFIED_COMMAND_KEYS = {"schema_version", "kind", "id", "command", "cwd", "pid", "timeout_seconds", "started_utc", "exit_code", "ended_utc", "elapsed_seconds", "timed_out", "launch_error", "stdout", "stderr"}


def _validate_qualified_command(root: pathlib.Path, record: Any, context: str, *, expected_success: bool) -> None:
    allowed = _QUALIFIED_COMMAND_KEYS | {"stage", "scratch"}
    if not isinstance(record, dict) or not _QUALIFIED_COMMAND_KEYS.issubset(record) or not set(record).issubset(allowed) or record.get("schema_version") != 1 or record.get("kind") != "qualified-command-record":
        raise EvidenceError(f"{context} command schema differs")
    if not isinstance(record["id"], str) or not record["id"] or not isinstance(record["command"], list) or not record["command"] or any(not isinstance(part, str) or not part for part in record["command"]):
        raise EvidenceError(f"{context} command provenance differs")
    if not isinstance(record["cwd"], str) or not record["cwd"]:
        raise EvidenceError(f"{context} command provenance differs")
    started, ended = _utc_timestamp(record["started_utc"], f"{context} command start"), _utc_timestamp(record["ended_utc"], f"{context} command end")
    elapsed = _nonnegative_duration(record["elapsed_seconds"], f"{context} command duration")
    if ended < started or abs(elapsed - (ended - started).total_seconds()) > 1.0:
        raise EvidenceError(f"{context} command chronology differs")
    _positive_integer(record["pid"], f"{context} command PID")
    _positive_integer(record["timeout_seconds"], f"{context} command timeout")
    if "stage" in record and (not isinstance(record["stage"], str) or not record["stage"]):
        raise EvidenceError(f"{context} command stage differs")
    if "scratch" in record:
        scratch = record["scratch"]
        if not isinstance(scratch, dict) or set(scratch) != {"before_sha256", "after_sha256", "unchanged"} or scratch["unchanged"] is not True:
            raise EvidenceError(f"{context} scratch evidence differs")
        _sha256_text(scratch["before_sha256"], f"{context} scratch before hash")
        _sha256_text(scratch["after_sha256"], f"{context} scratch after hash")
    for stream in ("stdout", "stderr"):
        item = record[stream]
        if not isinstance(item, dict) or set(item) != {"path", "sha256"}:
            raise EvidenceError(f"{context} command log differs")
        path = _validated_path(root, item.get("path"), f"{context} command log")
        if sha256(path) != _sha256_text(item.get("sha256"), f"{context} command log hash"):
            raise EvidenceError(f"{context} command log identity differs")
    if not isinstance(record["exit_code"], int) or isinstance(record["exit_code"], bool) or record["timed_out"] is not False or record["launch_error"] is not None:
        raise EvidenceError(f"{context} command outcome differs")
    if (record["exit_code"] == 0) != expected_success:
        raise EvidenceError(f"{context} command outcome differs")


def _validate_hashed_artifacts(root: pathlib.Path, declared: Any, count: int, context: str) -> None:
    if not isinstance(declared, list) or len(declared) != count:
        raise EvidenceError(f"{context} artifact matrix differs")
    seen: set[pathlib.Path] = set()
    for item in declared:
        if not isinstance(item, dict) or set(item) != {"path", "sha256"}:
            raise EvidenceError(f"{context} artifact schema differs")
        path = _validated_path(root, item.get("path"), f"{context} artifact")
        if path in seen or sha256(path) != _sha256_text(item.get("sha256"), f"{context} artifact hash"):
            raise EvidenceError(f"{context} artifact identity differs")
        seen.add(path)


def _validated_hashed_artifact(root: pathlib.Path, item: Any, context: str) -> pathlib.Path:
    if not isinstance(item, dict) or set(item) != {"path", "sha256"}:
        raise EvidenceError(f"{context} artifact schema differs")
    path = _validated_path(root, item.get("path"), context)
    if sha256(path) != _sha256_text(item.get("sha256"), f"{context} hash"):
        raise EvidenceError(f"{context} artifact identity differs")
    return path


def _relative_source_path(source_root: pathlib.Path, value: Any, context: str) -> pathlib.Path:
    if not isinstance(value, str) or not value:
        raise EvidenceError(f"{context} path differs")
    path = pathlib.Path(value)
    if path.is_absolute():
        try:
            return path.resolve().relative_to(source_root.resolve())
        except (OSError, ValueError) as error:
            raise EvidenceError(f"{context} path escapes candidate source") from error
    if any(part in {"", ".", ".."} for part in path.parts):
        raise EvidenceError(f"{context} path differs")
    return path


def _validated_source_inventory(candidate: dict[str, Any], expected_commit: str, context: str,
                                allowed_untracked_root: pathlib.Path | None = None,
                                verification_source_root: pathlib.Path | None = None) -> tuple[pathlib.Path, dict[pathlib.Path, str], pathlib.Path]:
    recorded_source_root = pathlib.Path(candidate["source_root"])
    source_root = verification_source_root if verification_source_root is not None else recorded_source_root
    if not source_root.is_dir():
        raise EvidenceError(f"{context} source root is absent")
    status_command = ["git", "status", "--porcelain"]
    allowed_prefix: str | None = None
    if allowed_untracked_root is not None:
        try:
            allowed_relative = allowed_untracked_root.resolve().relative_to(source_root.resolve()).as_posix()
        except (OSError, ValueError) as error:
            raise EvidenceError(f"{context} allowed retention path escapes source root") from error
        if not allowed_relative:
            raise EvidenceError(f"{context} allowed retention path differs")
        allowed_prefix = f"{allowed_relative.rstrip('/')}/"
        status_command.append("--untracked-files=all")
    status = subprocess.run(status_command, cwd=source_root, capture_output=True, text=True, check=False)
    revision = subprocess.run(["git", "rev-parse", "HEAD"], cwd=source_root, capture_output=True, text=True, check=False)
    listed = subprocess.run(["git", "ls-files"], cwd=source_root, capture_output=True, text=True, check=False)
    status_lines = [line for line in status.stdout.splitlines() if line]
    if allowed_prefix is not None:
        allowed_status = all(line.startswith("?? ") and line[3:].replace("\\", "/").startswith(allowed_prefix)
                             for line in status_lines)
    else:
        allowed_status = not status_lines
    if status.returncode != 0 or not allowed_status or revision.returncode != 0 or revision.stdout.strip() != expected_commit or listed.returncode != 0:
        raise EvidenceError(f"{context} revision-bound source differs")
    inventory = candidate["source_inventory"]
    rows: dict[pathlib.Path, str] = {}
    for row in inventory:
        relative = _relative_source_path(source_root, row["path"], f"{context} source")
        if relative in rows:
            raise EvidenceError(f"{context} source inventory differs")
        path = source_root / relative
        if not path.is_file() or sha256(path) != row["sha256"]:
            raise EvidenceError(f"{context} source inventory differs")
        rows[relative] = row["sha256"]
    tracked = {pathlib.PurePosixPath(item).as_posix() for item in listed.stdout.splitlines() if item}
    if {path.as_posix() for path in rows} != tracked:
        raise EvidenceError(f"{context} source inventory differs")
    return source_root, rows, recorded_source_root


def _validate_prerequisite_candidate(manifest: dict[str, Any], expected_commit: str, context: str,
                                    allowed_untracked_root: pathlib.Path | None = None,
                                    verification_source_root: pathlib.Path | None = None) -> tuple[pathlib.Path, dict[pathlib.Path, str], pathlib.Path]:
    candidate = manifest.get("candidate")
    if not isinstance(candidate, dict) or set(candidate) != {"commit", "tree_clean", "source_root", "source_inventory"} or candidate["commit"] != expected_commit or candidate["tree_clean"] is not True or not isinstance(candidate["source_root"], str) or not candidate["source_root"]:
        raise EvidenceError(f"{context} candidate identity differs")
    inventory = candidate["source_inventory"]
    if not isinstance(inventory, list) or not inventory or any(not isinstance(row, dict) or set(row) != {"path", "sha256"} or not isinstance(row["path"], str) or not row["path"] or _sha256_text(row["sha256"], f"{context} source hash") != row["sha256"] for row in inventory):
        raise EvidenceError(f"{context} source inventory differs")
    return _validated_source_inventory(candidate, expected_commit, context, allowed_untracked_root, verification_source_root)


def _validate_prerequisite_inputs(manifest: dict[str, Any], names: set[str], source_root: pathlib.Path,
                                  inventory: dict[pathlib.Path, str], context: str,
                                  recorded_source_root: pathlib.Path | None = None) -> None:
    inputs = manifest.get("inputs")
    if not isinstance(inputs, dict) or set(inputs) != names:
        raise EvidenceError(f"{context} input identity differs")
    for name, item in inputs.items():
        if not isinstance(item, dict) or set(item) != {"path", "sha256"} or not isinstance(item["path"], str) or not item["path"]:
            raise EvidenceError(f"{context} input identity differs")
        declared_hash = _sha256_text(item["sha256"], f"{context} input hash")
        relative = _relative_source_path(recorded_source_root or source_root, item["path"], f"{context} input")
        path = source_root / relative
        if inventory.get(relative) != declared_hash or not path.is_file() or sha256(path) != declared_hash:
            raise EvidenceError(f"{context} input identity differs")


def _replace_output_root(command: list[Any], root: pathlib.Path, context: str) -> list[str]:
    if not isinstance(command, list) or not command or any(not isinstance(part, str) or not part for part in command):
        raise EvidenceError(f"{context} plan command differs")
    return [part.replace("@OUTPUT_ROOT@", str(root)) for part in command]


def _require_exact_commands(records: list[dict[str, Any]], expected: dict[str, tuple[list[str], pathlib.Path]], context: str) -> None:
    by_id = {record.get("id"): record for record in records}
    if set(by_id) != set(expected) or len(by_id) != len(records):
        raise EvidenceError(f"{context} command matrix differs")
    for identifier, (argv, cwd) in expected.items():
        record = by_id[identifier]
        if record["command"] != argv or record["cwd"] != str(cwd.resolve()):
            raise EvidenceError(f"{context} observed command differs from plan")


def _require_command_ids(records: list[dict[str, Any]], required: set[str], context: str) -> None:
    identifiers = {record.get("id") for record in records}
    if not required.issubset(identifiers):
        raise EvidenceError(f"{context} command matrix differs")


def _validate_architecture_prerequisite(root: pathlib.Path, execution_root: pathlib.Path, manifest: dict[str, Any], expected_commit: str,
                                        allowed_untracked_root: pathlib.Path | None = None,
                                        verification_source_root: pathlib.Path | None = None) -> None:
    candidate, execution, requirements = manifest.get("candidate"), manifest.get("execution"), manifest.get("requirements")
    if manifest.get("state") != "EXECUTED_PENDING_AUDIT":
        raise EvidenceError("Architecture prerequisite state or candidate differs")
    source_root, inventory, recorded_source_root = _validate_prerequisite_candidate(manifest, expected_commit, "Architecture prerequisite", allowed_untracked_root, verification_source_root)
    _validate_prerequisite_inputs(manifest, {"profile", "protocol", "expected_certificate", "comparer", "launcher"}, source_root, inventory, "Architecture prerequisite", recorded_source_root)
    if not isinstance(requirements, dict) or requirements != {str(number): "EVIDENCE_COLLECTED_PENDING_AUDIT" for number in range(1, 9)}:
        raise EvidenceError("Architecture prerequisite requirement matrix differs")
    if not isinstance(execution, dict):
        raise EvidenceError("Architecture prerequisite execution differs")
    plan = manifest.get("plan")
    if not isinstance(plan, list) or {row.get("name") for row in plan if isinstance(row, dict)} != set(EXPECTED_CONFIGURATION_NAMES) or len(plan) != len(EXPECTED_CONFIGURATION_NAMES):
        raise EvidenceError("Architecture prerequisite plan matrix differs")
    plan_by_name = {row["name"]: row for row in plan}
    evidence_inputs = manifest.get("evidence_inputs")
    if not isinstance(evidence_inputs, dict) or set(evidence_inputs) != {"expected_certificate"}:
        raise EvidenceError("Architecture prerequisite retained input differs")
    expected_certificate = _validated_hashed_artifact(root, evidence_inputs["expected_certificate"], "Architecture prerequisite expected certificate")
    rows = _require_completed_cells(execution.get("records"), "Architecture prerequisite")
    globals_ = [row for row in execution["records"] if isinstance(row, dict) and "global" in row]
    if {row.get("global") for row in globals_} != {"compare-one", "compare-two"} or len(globals_) != 2:
        raise EvidenceError("Architecture prerequisite global matrix differs")
    for row in rows:
        if set(row) != {"cell", "state", "certificates", "certificate_artifacts", "records"} or not isinstance(row["certificates"], list) or len(row["certificates"]) != 3:
            raise EvidenceError("Architecture prerequisite certificate matrix differs")
        _validate_hashed_artifacts(root, row["certificate_artifacts"], 3, "Architecture prerequisite certificate")
        if [item["path"] for item in row["certificate_artifacts"]] != row["certificates"]:
            raise EvidenceError("Architecture prerequisite certificate linkage differs")
        compile_records = [item for item in row["records"] if isinstance(item, dict) and item.get("stage") == "compile-commands"]
        commands = [item for item in row["records"] if isinstance(item, dict) and item.get("kind") == "qualified-command-record"]
        if len(compile_records) != 1 or len(commands) != len(row["records"]) - 1:
            raise EvidenceError("Architecture prerequisite command matrix differs")
        plan_row = plan_by_name[row["cell"]]
        if set(plan_row) != {"name", "configure", "build", "ctest", "certificate_processes", "consumer_validations"} or plan_row["certificate_processes"] != 3 or plan_row["consumer_validations"] != 1:
            raise EvidenceError("Architecture prerequisite plan row differs")
        cell_root, build_root, scratch_root = execution_root / "cells" / row["cell"], execution_root / "cells" / row["cell"] / "build", execution_root / "cells" / row["cell"] / "scratch"
        expected = {stage: (_replace_output_root(plan_row[stage], execution_root, "Architecture prerequisite"), execution_root) for stage in ("configure", "build", "ctest")}
        compiler, libcxx = {"gcc-debug": ("g++-13", False), "gcc-release": ("g++-13", False), "clang-debug": ("clang++-18", True), "clang-release": ("clang++-18", True)}[row["cell"]]
        consumer_root = cell_root / "consumer"
        expected.update({"scratch-smoke": ([str(build_root / "apmesh_core_bootstrap_smoke")], scratch_root),
                         **{f"export-{index}": ([str(build_root / "apmesh_core_bootstrap_export"), str(cell_root / f"certificate-{index}.json")], cell_root) for index in range(1, 4)},
                         **{f"validate-{index}": ([sys.executable, manifest["inputs"]["comparer"]["path"], "validate", "--expected", manifest["inputs"]["expected_certificate"]["path"], "--actual", str(cell_root / f"certificate-{index}.json")], cell_root) for index in range(1, 4)},
                         "consumer-configure": (["cmake", "-S", str(recorded_source_root / "tests" / "consumer"), "-B", str(consumer_root), "-G", "Ninja", f"-DCMAKE_CXX_COMPILER={compiler}", f"-DAPMESH_CORE_SOURCE_DIR={recorded_source_root}", "-DBUILD_TESTING=OFF", f"-DAPMESH_USE_LIBCXX={'ON' if libcxx else 'OFF'}"], cell_root),
                         "consumer-build": (["cmake", "--build", str(consumer_root), "--target", "apmesh_core_external_consumer", "apmesh_core_unrelated_target"], cell_root),
                         "consumer": ([str(consumer_root / "apmesh_core_external_consumer")], scratch_root),
                         "unrelated": ([str(consumer_root / "apmesh_core_unrelated_target")], scratch_root)})
        _require_exact_commands(commands, expected, "Architecture prerequisite")
        compile_record = compile_records[0]
        if set(compile_record) != {"stage", "path", "sha256"}:
            raise EvidenceError("Architecture prerequisite compile command schema differs")
        compile_path = _validated_path(root, compile_record["path"], "Architecture prerequisite compile command")
        if sha256(compile_path) != _sha256_text(compile_record["sha256"], "Architecture prerequisite compile command hash"):
            raise EvidenceError("Architecture prerequisite compile command identity differs")
        for record in commands:
            _validate_qualified_command(root, record, "Architecture prerequisite", expected_success=True)
        for artifact in row["certificate_artifacts"]:
            try:
                validate_bootstrap_certificate(expected_certificate, _validated_hashed_artifact(root, artifact, "Architecture prerequisite certificate"))
            except BootstrapEvidenceError as error:
                raise EvidenceError(f"Architecture prerequisite certificate semantics differ: {error}") from error
    for row in globals_:
        if set(row) != {"global", "state", "record"} or row["state"] != "PASS":
            raise EvidenceError("Architecture prerequisite global schema differs")
        _validate_qualified_command(root, row["record"], "Architecture prerequisite global", expected_success=True)
    all_certificates = [str(execution_root / certificate) for row in rows for certificate in row["certificates"]]
    expected_global = {
        name: [sys.executable, manifest["inputs"]["comparer"]["path"], "compare", "--expected", manifest["inputs"]["expected_certificate"]["path"],
               *sum((["--certificate", certificate] for certificate in all_certificates), []), "--report", str(execution_root / "reports" / suffix / "certificate-report.md")]
        for name, suffix in (("compare-one", "one"), ("compare-two", "two"))
    }
    for row in globals_:
        if row["record"]["command"] != expected_global[row["global"]] or row["record"]["cwd"] != str(execution_root):
            raise EvidenceError("Architecture prerequisite global command differs")
    negatives = execution.get("negative_records")
    if not isinstance(negatives, list) or len(negatives) != 5:
        raise EvidenceError("Architecture prerequisite negative evidence differs")
    fixtures = recorded_source_root / "tests" / "data" / "bootstrap_regression"
    expected_negatives = [
        [sys.executable, manifest["inputs"]["comparer"]["path"], "validate", "--expected", manifest["inputs"]["expected_certificate"]["path"], "--actual", str(execution_root / "missing.json")],
        [sys.executable, manifest["inputs"]["comparer"]["path"], "validate", "--expected", manifest["inputs"]["expected_certificate"]["path"], "--actual", str(fixtures / "malformed.json")],
        [sys.executable, manifest["inputs"]["comparer"]["path"], "validate", "--expected", manifest["inputs"]["expected_certificate"]["path"], "--actual", str(fixtures / "unsupported_schema_version.json")],
        [sys.executable, manifest["inputs"]["comparer"]["path"], "validate", "--expected", manifest["inputs"]["expected_certificate"]["path"], "--actual", str(fixtures / "changed_component.json")],
        [sys.executable, manifest["inputs"]["comparer"]["path"], "validate-manifest", "--manifest", str(fixtures / "manifest_changed_source_hash.json"), "--expected-source-hash", "expected-source-hash"],
    ]
    for index, record in enumerate(negatives, start=1):
        _validate_qualified_command(root, record, "Architecture prerequisite negative", expected_success=False)
        if record["id"] != f"negative-{index}" or record["command"] != expected_negatives[index - 1] or record["cwd"] != str(execution_root):
            raise EvidenceError("Architecture prerequisite negative command differs")


def _validate_numeric_prerequisite(root: pathlib.Path, execution_root: pathlib.Path, manifest: dict[str, Any], expected_commit: str,
                                   allowed_untracked_root: pathlib.Path | None = None,
                                   verification_source_root: pathlib.Path | None = None) -> None:
    candidate, execution, gates = manifest.get("candidate"), manifest.get("execution"), manifest.get("gates")
    if manifest.get("state") != "EXECUTED_PENDING_AUDIT":
        raise EvidenceError("Numeric prerequisite state or candidate differs")
    source_root, inventory, recorded_source_root = _validate_prerequisite_candidate(manifest, expected_commit, "Numeric prerequisite", allowed_untracked_root, verification_source_root)
    _validate_prerequisite_inputs(manifest, {"profile", "protocol", "validator", "architecture_runner", "architecture_profile", "architecture_evidence", "architecture_expected", "architecture_comparer", "launcher"}, source_root, inventory, "Numeric prerequisite", recorded_source_root)
    if not isinstance(gates, dict) or gates != {f"N{number}": "EVIDENCE_COLLECTED_PENDING_AUDIT" for number in range(8)}:
        raise EvidenceError("Numeric prerequisite gate matrix differs")
    if not isinstance(execution, dict):
        raise EvidenceError("Numeric prerequisite execution differs")
    plan = manifest.get("plan")
    if not isinstance(plan, list) or {row.get("name") for row in plan if isinstance(row, dict)} != set(EXPECTED_CONFIGURATION_NAMES) or len(plan) != len(EXPECTED_CONFIGURATION_NAMES):
        raise EvidenceError("Numeric prerequisite plan matrix differs")
    plan_by_name = {row["name"]: row for row in plan}
    rows = _require_completed_cells(execution.get("records"), "Numeric prerequisite")
    for row in rows:
        if set(row) != {"cell", "state", "certificates", "environments", "certificate_artifacts", "environment_artifacts", "compile_commands_artifact", "records"} or not isinstance(row.get("certificates"), list) or len(row["certificates"]) != 3 or not isinstance(row.get("environments"), list) or len(row["environments"]) != 3:
            raise EvidenceError("Numeric prerequisite repeated evidence differs")
        _validate_hashed_artifacts(root, row["certificate_artifacts"], 3, "Numeric prerequisite certificate")
        _validate_hashed_artifacts(root, row["environment_artifacts"], 3, "Numeric prerequisite environment")
        if [item["path"] for item in row["certificate_artifacts"]] != row["certificates"] or [item["path"] for item in row["environment_artifacts"]] != row["environments"]:
            raise EvidenceError("Numeric prerequisite artifact linkage differs")
        plan_row = plan_by_name[row["cell"]]
        if set(plan_row) != {"name", "configure", "build", "ctest", "repetitions"} or plan_row["repetitions"] != 3:
            raise EvidenceError("Numeric prerequisite plan row differs")
        cell_root, build_root = execution_root / "cells" / row["cell"], execution_root / "cells" / row["cell"] / "build"
        expected = {stage: (_replace_output_root(plan_row[stage], execution_root, "Numeric prerequisite"), execution_root) for stage in ("configure", "build")}
        expected.update({f"ctest-{index}": (_replace_output_root(plan_row["ctest"], execution_root, "Numeric prerequisite"), execution_root) for index in range(1, 4)})
        exporter = build_root / "apmesh_core_numeric_contract_export"
        expected.update({f"certificate-{index}": ([str(exporter), "certificate", str(cell_root / f"certificate-{index}.json")], cell_root) for index in range(1, 4)})
        expected.update({f"environment-{index}": ([str(exporter), "environment", str(cell_root / f"environment-{index}.json")], cell_root) for index in range(1, 4)})
        for record in row["records"]:
            _validate_qualified_command(root, record, "Numeric prerequisite", expected_success=True)
        _require_exact_commands(row["records"], expected, "Numeric prerequisite")
        compile_commands = _validated_hashed_artifact(root, row["compile_commands_artifact"], "Numeric prerequisite compile commands")
        for artifact in row["certificate_artifacts"]:
            try:
                validate_numeric_certificate(_validated_hashed_artifact(root, artifact, "Numeric prerequisite certificate"))
            except NumericEvidenceError as error:
                raise EvidenceError(f"Numeric prerequisite certificate semantics differ: {error}") from error
        for artifact in row["environment_artifacts"]:
            try:
                validate_numeric_environment(_validated_hashed_artifact(root, artifact, "Numeric prerequisite environment"), compile_commands)
            except NumericEvidenceError as error:
                raise EvidenceError(f"Numeric prerequisite environment semantics differ: {error}") from error
    comparison = execution.get("comparison")
    _validate_qualified_command(root, comparison, "Numeric prerequisite comparison", expected_success=True)
    all_certificates = [str(execution_root / certificate) for row in rows for certificate in row["certificates"]]
    all_environments = [str(execution_root / environment) for row in rows for environment in row["environments"]]
    expected_comparison = [sys.executable, manifest["inputs"]["validator"]["path"], "compare",
                           *sum((["--certificate", certificate] for certificate in all_certificates), []),
                           *sum((["--environment", environment] for environment in all_environments), []),
                           "--report", str(execution_root / "report.md")]
    if comparison["id"] != "compare" or comparison["command"] != expected_comparison or comparison["cwd"] != str(execution_root):
        raise EvidenceError("Numeric prerequisite comparison command differs")
    report = root / "report.md"
    if not report.is_file() or sha256(report) != _sha256_text(execution.get("report_sha256"), "Numeric prerequisite report hash"):
        raise EvidenceError("Numeric prerequisite report differs")
    preservation = execution.get("architecture_preservation")
    if not isinstance(preservation, dict) or set(preservation) != {"prepare", "execute", "manifest"} or not isinstance(preservation["manifest"], str):
        raise EvidenceError("Numeric prerequisite Architecture linkage differs")
    preservation_plan = manifest.get("architecture_preservation_plan")
    if not isinstance(preservation_plan, dict) or set(preservation_plan) != {"prepare", "execute"}:
        raise EvidenceError("Numeric prerequisite Architecture plan differs")
    for name in ("prepare", "execute"):
        _validate_qualified_command(root, preservation[name], "Numeric prerequisite Architecture execution", expected_success=True)
        if preservation[name]["id"] != f"architecture-{name}" or preservation[name]["command"] != preservation_plan[name] or preservation[name]["cwd"] != str(execution_root):
            raise EvidenceError("Numeric prerequisite Architecture command differs")


def _validate_prerequisite(root: pathlib.Path, evidence_root: pathlib.Path, name: str, entry: Any, expected_commit: str,
                           allowed_untracked_root: pathlib.Path | None = None,
                           verification_source_root: pathlib.Path | None = None) -> None:
    if not isinstance(entry, dict) or set(entry) != {"manifest", "manifest_sha256", "state", "candidate_commit"}:
        raise EvidenceError("terminal prerequisite schema differs")
    if not isinstance(entry["manifest"], str):
        raise EvidenceError("terminal prerequisite schema differs")
    path = root / entry["manifest"]
    try:
        path.resolve().relative_to(root.resolve())
    except (TypeError, ValueError) as error:
        raise EvidenceError("terminal prerequisite path escapes evidence root") from error
    if not path.is_file() or sha256(path) != entry["manifest_sha256"]:
        raise EvidenceError("terminal prerequisite identity differs")
    manifest = read_json_object(path)
    if entry["state"] != "EXECUTED_PENDING_AUDIT" or entry["candidate_commit"] != expected_commit:
        raise EvidenceError("terminal prerequisite identity differs")
    original_root = evidence_root / pathlib.PurePosixPath(entry["manifest"]).parent
    if name == "architecture_contract":
        _validate_architecture_prerequisite(path.parent, original_root, manifest, expected_commit, allowed_untracked_root, verification_source_root)
    elif name == "numeric_contract":
        _validate_numeric_prerequisite(path.parent, original_root, manifest, expected_commit, allowed_untracked_root, verification_source_root)
    else:
        raise EvidenceError("terminal prerequisite name differs")


def _validate_negative_fixture(root: pathlib.Path, row: Any, profile: dict[str, Any], source_root: pathlib.Path) -> None:
    expected = {"fixture", "expected_reason_code", "result", "reason_code", "record", "response"}
    if not isinstance(row, dict) or set(row) != expected or row["fixture"] not in profile["negative_fixtures"]:
        raise EvidenceError("terminal negative-fixture schema differs")
    declaration = profile["negative_fixtures"][row["fixture"]]
    if row["expected_reason_code"] != declaration["reason_code"] or row["result"] != "REJECTED" or row["reason_code"] != declaration["reason_code"]:
        raise EvidenceError("terminal negative-fixture result differs")
    response = row["response"]
    if not isinstance(response, dict) or set(response) != {"path", "sha256"}:
        raise EvidenceError("terminal negative-fixture response schema differs")
    response_path = root / response["path"]
    if not response_path.is_file() or sha256(response_path) != response["sha256"]:
        raise EvidenceError("terminal negative-fixture response identity differs")
    payload = read_json_object(response_path)
    if payload != {"schema_version": 1, "kind": "reproducible-experiment-negative-result", "fixture": row["fixture"], "result": "REJECTED", "reason_code": row["reason_code"]}:
        raise EvidenceError("terminal negative-fixture response differs")
    record = row["record"]
    fixture_root = root / "negative-fixtures" / row["fixture"]
    _validate_rec_command_record(record, fixture_root)
    expected_command = [sys.executable, str(source_root / "tools" / "reproducible_experiment_negative.py"), "--fixture", row["fixture"],
                        "--profile", str(source_root / "experiments" / "profiles" / "reproducible_experiment_contract.json")]
    if record["id"] != row["fixture"] or record["argv"] != expected_command or record["cwd"] != str(source_root.resolve()) or record["environment_delta"] != {}:
        raise EvidenceError("terminal negative-fixture command differs")
    expected_response = pathlib.PurePosixPath("negative-fixtures") / row["fixture"] / pathlib.PurePosixPath(record["stdout"]["path"])
    if pathlib.PurePosixPath(response["path"]) != expected_response:
        raise EvidenceError("terminal negative-fixture response linkage differs")


def validate_terminal_manifest(root: pathlib.Path, profile: dict[str, Any], prepared_manifest_sha256: str,
                               candidate_commit: str, profile_sha256: str, launch_plan_sha256: str,
                               launch_plan: dict[str, Any], source_root: pathlib.Path,
                               scratch_root: pathlib.Path, evidence_root: pathlib.Path,
                               allowed_untracked_root: pathlib.Path | None = None,
                               verification_source_root: pathlib.Path | None = None) -> dict[str, Any]:
    terminal = read_json_object(root / "terminal-manifest.json")
    required = {"schema_version", "kind", "state", "prepared_manifest_sha256", "candidate_commit", "bundles", "bundle_seals", "replay_comparisons", "cross_configuration", "negative_fixtures", "prerequisites", "gate_results"}
    if set(terminal) != required or terminal["schema_version"] != 1 or terminal["kind"] != "reproducible-experiment-terminal-manifest" or terminal["state"] != "EXECUTED_PENDING_AUDIT":
        raise EvidenceError("terminal manifest schema differs")
    if terminal["prepared_manifest_sha256"] != prepared_manifest_sha256:
        raise EvidenceError("terminal prepared-manifest identity differs")
    if terminal["candidate_commit"] != candidate_commit:
        raise EvidenceError("terminal candidate identity differs")
    plan_rows = _validate_launch_plan(launch_plan, profile)
    expected_cells = {(configuration["name"], replay) for configuration in profile["configurations"] for replay in (1, 2)}
    observed_cells = {(entry.get("cell"), entry.get("replay")) for entry in terminal["bundles"] if isinstance(entry, dict)}
    if observed_cells != expected_cells or len(terminal["bundles"]) != len(expected_cells):
        raise EvidenceError("terminal bundle matrix differs")
    by_cell: dict[tuple[str, int], dict[str, Any]] = {}
    by_bundle: dict[str, dict[str, Any]] = {}
    for entry in terminal["bundles"]:
        if not isinstance(entry, dict) or set(entry) != {"cell", "replay", "bundle"} or not isinstance(entry["bundle"], str):
            raise EvidenceError("terminal bundle schema differs")
        key = (entry["cell"], entry["replay"])
        if key in by_cell or entry["bundle"] in by_bundle:
            raise EvidenceError("terminal bundle identity is not one-to-one")
        bundle = root / entry["bundle"]
        try:
            bundle.resolve().relative_to(root.resolve())
        except ValueError as error:
            raise EvidenceError("terminal bundle path escapes evidence root") from error
        by_cell[key], by_bundle[entry["bundle"]] = entry, entry
    if set(by_cell) != expected_cells or set(by_cell) != set(plan_rows):
        raise EvidenceError("terminal bundle and launch-plan matrices differ")
    if not isinstance(terminal["bundle_seals"], list) or len(terminal["bundle_seals"]) != len(expected_cells):
        raise EvidenceError("terminal bundle seals differ")
    seals_by_bundle: dict[str, dict[str, Any]] = {}
    for seal in terminal["bundle_seals"]:
        if not isinstance(seal, dict) or set(seal) != {"bundle", "inventory_sha256", "summary_sha256", "execution_sha256"} or not isinstance(seal["bundle"], str) or seal["bundle"] in seals_by_bundle:
            raise EvidenceError("terminal seal schema differs")
        seals_by_bundle[seal["bundle"]] = seal
    if set(seals_by_bundle) != set(by_bundle):
        raise EvidenceError("terminal bundle and seal identities differ")
    if not isinstance(terminal["negative_fixtures"], list) or {row.get("fixture") for row in terminal["negative_fixtures"] if isinstance(row, dict)} != {f"N{number}" for number in range(1, 9)}:
        raise EvidenceError("terminal negative-fixture evidence differs")
    for row in terminal["negative_fixtures"]:
        _validate_negative_fixture(root, row, profile, source_root)
    if not isinstance(terminal["prerequisites"], dict) or set(terminal["prerequisites"]) != {"numeric_contract", "architecture_contract"}:
        raise EvidenceError("terminal prerequisite evidence differs")
    for name, entry in terminal["prerequisites"].items():
        _validate_prerequisite(root, evidence_root, name, entry, candidate_commit, allowed_untracked_root, verification_source_root)
    numeric_manifest = read_json_object(root / terminal["prerequisites"]["numeric_contract"]["manifest"])
    reported_architecture = numeric_manifest["execution"]["architecture_preservation"]["manifest"]
    expected_architecture = evidence_root / terminal["prerequisites"]["architecture_contract"]["manifest"]
    if not isinstance(reported_architecture, str) or pathlib.Path(reported_architecture).resolve() != expected_architecture.resolve():
        raise EvidenceError("Numeric prerequisite Architecture linkage differs")
    if not isinstance(terminal["gate_results"], dict) or set(terminal["gate_results"]) != set(EXPECTED_GATES) or any(value != "EVIDENCE_COLLECTED_PENDING_AUDIT" for value in terminal["gate_results"].values()):
        raise EvidenceError("terminal gate-result evidence differs")
    replay_roots: dict[str, list[pathlib.Path]] = {name: [] for name in EXPECTED_CONFIGURATION_NAMES}
    cell_summaries: dict[str, dict[str, Any]] = {}
    provenance_by_cell: dict[str, dict[str, Any]] = {}
    for key, entry in sorted(by_cell.items()):
        cell, replay = key
        bundle = root / entry["bundle"]
        _validate_execution_against_plan(bundle, entry["bundle"], plan_rows[key], profile, source_root, scratch_root, evidence_root)
        bundle_manifest = _validate_bundle_manifest(bundle, profile)
        if (bundle_manifest["cell"], bundle_manifest["replay"]) != key:
            raise EvidenceError("terminal bundle path and manifest identity differ")
        replay_roots[cell].append(bundle)
        if replay == 1:
            cell_summaries[cell] = read_json_object(bundle / "summary.json")
            provenance_by_cell[cell] = read_json_object(bundle / "execution-record.json")
        seal = seals_by_bundle[entry["bundle"]]
        validate_bundle(bundle, profile)
        if bundle_manifest["candidate_commit"] != candidate_commit or bundle_manifest["prepared_manifest_sha256"] != prepared_manifest_sha256 or bundle_manifest["profile_sha256"] != profile_sha256 or bundle_manifest["launch_plan_sha256"] != launch_plan_sha256:
            raise EvidenceError("bundle terminal identity differs")
        for name, filename in (("inventory_sha256", "artifact-inventory.json"), ("summary_sha256", "summary.json"), ("execution_sha256", "execution-record.json")):
            if seal[name] != sha256(bundle / filename):
                raise EvidenceError("terminal seal identity differs")
    expected_replays = [compare_replays(sorted(replay_roots[name], key=lambda item: _validate_bundle_manifest(item, profile)["replay"]), profile)
                        for name in EXPECTED_CONFIGURATION_NAMES]
    if terminal["replay_comparisons"] != expected_replays:
        raise EvidenceError("terminal replay comparisons are not independently reproducible")
    expected_cross = compare_configurations(cell_summaries, profile, provenance_by_cell)
    if terminal["cross_configuration"] != expected_cross:
        raise EvidenceError("terminal cross-configuration comparison is not independently reproducible")
    return terminal


def validate_blocked_terminal_manifest(root: pathlib.Path, prepared_manifest_sha256: str,
                                       candidate_commit: str) -> dict[str, Any]:
    terminal = read_json_object(root / "terminal-manifest.json")
    expected = {"schema_version", "kind", "state", "prepared_manifest_sha256", "candidate_commit",
                "attempted_bundles", "reason", "failure_evidence"}
    if set(terminal) != expected or terminal["schema_version"] != 1 or terminal["kind"] != "reproducible-experiment-terminal-manifest" or terminal["state"] != "BLOCKED":
        raise EvidenceError("blocked terminal manifest schema differs")
    if terminal["prepared_manifest_sha256"] != prepared_manifest_sha256 or terminal["candidate_commit"] != candidate_commit or not isinstance(terminal["reason"], str) or not terminal["reason"]:
        raise EvidenceError("blocked terminal identity differs")
    if not isinstance(terminal["failure_evidence"], list) or not terminal["failure_evidence"]:
        raise EvidenceError("blocked terminal failure evidence is absent")
    for entry in terminal["failure_evidence"]:
        if not isinstance(entry, dict) or set(entry) != {"path", "sha256"} or not isinstance(entry["path"], str):
            raise EvidenceError("blocked terminal failure evidence schema differs")
        path = root / entry["path"]
        if not path.is_file() or sha256(path) != entry["sha256"]:
            raise EvidenceError("blocked terminal failure evidence identity differs")
    return terminal


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    commands = parser.add_subparsers(dest="command", required=True)
    profile = commands.add_parser("validate-profile"); profile.add_argument("--profile", required=True)
    bundle = commands.add_parser("validate-bundle"); bundle.add_argument("--profile", required=True); bundle.add_argument("--bundle", required=True)
    replay = commands.add_parser("compare-replays"); replay.add_argument("--profile", required=True); replay.add_argument("--bundle", required=True, action="append")
    return parser.parse_args()


def main() -> int:
    arguments = parse_arguments()
    try:
        if arguments.command == "validate-profile":
            validate_profile(pathlib.Path(arguments.profile))
        elif arguments.command == "validate-bundle":
            validate_bundle(pathlib.Path(arguments.bundle), validate_profile(pathlib.Path(arguments.profile)))
        else:
            compare_replays([pathlib.Path(path) for path in arguments.bundle], validate_profile(pathlib.Path(arguments.profile)))
        return 0
    except RuntimeErrorEvidence as error:
        print(error, file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
