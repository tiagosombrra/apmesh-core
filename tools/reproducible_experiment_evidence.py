#!/usr/bin/env python3
"""Strict, report-only evidence checks for the Reproducible Experiment Contract."""

from __future__ import annotations

import argparse
import fnmatch
import pathlib
import sys
from typing import Any

from experiment_runtime import (RuntimeErrorEvidence, canonical_json, inventory_files, read_json,
                                require_exact_keys, sha256_file, write_json)


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
    (root / "report.md").write_text("\n".join(markdown), encoding="utf-8")
    (root / "metrics.csv").write_text("cell,replay,evidence_state\n" + "".join(
        f"{row['cell']},{row['replay']},bundle_validated\n" for row in rows), encoding="utf-8")
    (root / "figures").mkdir(parents=True, exist_ok=True)
    svg_rows = "".join(f'<text x="10" y="{30 + index * 20}">{row["cell"]} replay {row["replay"]}: bundle validated</text>' for index, row in enumerate(rows))
    (root / "figures" / "status-matrix.svg").write_text(
        f'<svg xmlns="http://www.w3.org/2000/svg" width="720" height="{50 + len(rows) * 20}"><text x="10" y="15">REC evidence status</text>{svg_rows}</svg>\n', encoding="utf-8")


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
        if not isinstance(record, dict) or set(record) != record_keys or record["schema_version"] != 1 or record["kind"] != "experiment-command-record":
            raise EvidenceError("command record schema differs")
        if not isinstance(record["argv"], list) or not isinstance(record["cwd"], str) or not isinstance(record["environment_delta"], dict) or not isinstance(record["elapsed_seconds"], (int, float)):
            raise EvidenceError("command record provenance differs")
        for stream in ("stdout", "stderr"):
            item = record[stream]
            if not isinstance(item, dict) or set(item) != {"path", "sha256"} or not isinstance(item["path"], str):
                raise EvidenceError("command log declaration differs")
            path = root / item["path"]
            if not path.is_file() or sha256(path) != item["sha256"]:
                raise EvidenceError("command log identity differs")
        if record["exit_code"] != 0 or record["timed_out"] is not False or record["launch_error"] is not None:
            raise EvidenceError("accepted bundle command did not succeed")
    return value


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


def _validate_prerequisite(root: pathlib.Path, entry: Any, expected_commit: str) -> None:
    if not isinstance(entry, dict) or set(entry) != {"manifest", "manifest_sha256", "state", "candidate_commit"}:
        raise EvidenceError("terminal prerequisite schema differs")
    path = root / entry["manifest"]
    if not isinstance(entry["manifest"], str) or not path.is_file() or sha256(path) != entry["manifest_sha256"]:
        raise EvidenceError("terminal prerequisite identity differs")
    manifest = read_json_object(path)
    candidate = manifest.get("candidate")
    commit = candidate.get("commit") if isinstance(candidate, dict) else None
    execution = manifest.get("execution")
    if entry["state"] != "EXECUTED_PENDING_AUDIT" or manifest.get("state") != entry["state"] or entry["candidate_commit"] != expected_commit or commit != expected_commit or not isinstance(execution, dict) or not execution:
        raise EvidenceError("terminal prerequisite state or candidate differs")


def _validate_negative_fixture(root: pathlib.Path, row: Any, profile: dict[str, Any]) -> None:
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
    if not isinstance(record, dict) or record.get("exit_code") != 0 or record.get("timed_out") is not False or record.get("launch_error") is not None:
        raise EvidenceError("terminal negative-fixture command differs")


def validate_terminal_manifest(root: pathlib.Path, profile: dict[str, Any], prepared_manifest_sha256: str,
                               candidate_commit: str, profile_sha256: str, launch_plan_sha256: str) -> dict[str, Any]:
    terminal = read_json_object(root / "terminal-manifest.json")
    required = {"schema_version", "kind", "state", "prepared_manifest_sha256", "candidate_commit", "bundles", "bundle_seals", "replay_comparisons", "cross_configuration", "negative_fixtures", "prerequisites", "gate_results"}
    if set(terminal) != required or terminal["schema_version"] != 1 or terminal["kind"] != "reproducible-experiment-terminal-manifest" or terminal["state"] != "EXECUTED_PENDING_AUDIT":
        raise EvidenceError("terminal manifest schema differs")
    if terminal["prepared_manifest_sha256"] != prepared_manifest_sha256:
        raise EvidenceError("terminal prepared-manifest identity differs")
    if terminal["candidate_commit"] != candidate_commit:
        raise EvidenceError("terminal candidate identity differs")
    if not isinstance(terminal["replay_comparisons"], list) or {row.get("cell") for row in terminal["replay_comparisons"] if isinstance(row, dict)} != set(EXPECTED_CONFIGURATION_NAMES):
        raise EvidenceError("terminal replay-comparison matrix differs")
    for row in terminal["replay_comparisons"]:
        if not isinstance(row, dict) or set(row) != {"cell", "replays", "claim_fields", "differences"} or row["replays"] != [1, 2] or row["claim_fields"] != "equivalent" or not isinstance(row["differences"], list):
            raise EvidenceError("terminal replay-comparison schema differs")
    cross = terminal["cross_configuration"]
    if not isinstance(cross, dict) or set(cross) != {"baseline", "cells", "claim_fields", "differences"} or cross["baseline"] != EXPECTED_CONFIGURATION_NAMES[0] or cross["cells"] != list(EXPECTED_CONFIGURATION_NAMES) or cross["claim_fields"] != "equivalent" or not isinstance(cross["differences"], list):
        raise EvidenceError("terminal cross-configuration evidence differs")
    expected_cells = {(configuration["name"], replay) for configuration in profile["configurations"] for replay in (1, 2)}
    observed_cells = {(entry.get("cell"), entry.get("replay")) for entry in terminal["bundles"] if isinstance(entry, dict)}
    if observed_cells != expected_cells or len(terminal["bundles"]) != len(expected_cells):
        raise EvidenceError("terminal bundle matrix differs")
    if len(terminal["bundle_seals"]) != len(expected_cells):
        raise EvidenceError("terminal bundle seals differ")
    if not isinstance(terminal["negative_fixtures"], list) or {row.get("fixture") for row in terminal["negative_fixtures"] if isinstance(row, dict)} != {f"N{number}" for number in range(1, 9)}:
        raise EvidenceError("terminal negative-fixture evidence differs")
    for row in terminal["negative_fixtures"]:
        _validate_negative_fixture(root, row, profile)
    if not isinstance(terminal["prerequisites"], dict) or set(terminal["prerequisites"]) != {"numeric_contract", "architecture_contract"}:
        raise EvidenceError("terminal prerequisite evidence differs")
    for entry in terminal["prerequisites"].values():
        _validate_prerequisite(root, entry, candidate_commit)
    if not isinstance(terminal["gate_results"], dict) or set(terminal["gate_results"]) != set(EXPECTED_GATES) or any(value != "EVIDENCE_COLLECTED_PENDING_AUDIT" for value in terminal["gate_results"].values()):
        raise EvidenceError("terminal gate-result evidence differs")
    by_bundle = {entry["bundle"]: entry for entry in terminal["bundles"]}
    for seal in terminal["bundle_seals"]:
        if not isinstance(seal, dict) or set(seal) != {"bundle", "inventory_sha256", "summary_sha256", "execution_sha256"} or seal["bundle"] not in by_bundle:
            raise EvidenceError("terminal seal schema differs")
        bundle = root / seal["bundle"]
        validate_bundle(bundle, profile)
        bundle_manifest = _validate_bundle_manifest(bundle, profile)
        if bundle_manifest["candidate_commit"] != candidate_commit or bundle_manifest["prepared_manifest_sha256"] != prepared_manifest_sha256 or bundle_manifest["profile_sha256"] != profile_sha256 or bundle_manifest["launch_plan_sha256"] != launch_plan_sha256:
            raise EvidenceError("bundle terminal identity differs")
        for name, filename in (("inventory_sha256", "artifact-inventory.json"), ("summary_sha256", "summary.json"), ("execution_sha256", "execution-record.json")):
            if seal[name] != sha256(bundle / filename):
                raise EvidenceError("terminal seal identity differs")
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
