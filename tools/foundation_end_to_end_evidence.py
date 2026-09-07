#!/usr/bin/env python3
"""Report-only qualification of the cumulative Foundation evidence package."""

from __future__ import annotations

import argparse
import csv
import fnmatch
import io
import pathlib
import posixpath
import re
import sys
from typing import Any

from experiment_runtime import (RuntimeErrorEvidence, canonical_json, read_json,
                                require_exact_keys, sha256_file, write_json)
from retain_experiment_evidence import verify as verify_retained_package


EXPECTED_GATES = [f"FND{index}" for index in range(8)]
PENDING = "EVIDENCE_COLLECTED_PENDING_AUDIT"
SHA256 = re.compile(r"[0-9a-f]{64}")
GIT_COMMIT = re.compile(r"[0-9a-f]{40}")


def validate_profile(path: pathlib.Path) -> dict[str, Any]:
    profile = read_json(path)
    require_exact_keys(profile, {"schema_version", "kind", "accepted_baseline", "configurations", "claim_fields",
                                 "required_executables", "required_contract_tests", "allowed_scope_changes",
                                 "allowed_runtime_dependencies", "allowed_runtime_prefixes",
                                 "allowed_pseudo_dependencies", "gates", "limitations"},
                       "Foundation profile")
    if profile["schema_version"] != 1 or profile["kind"] != "foundation-end-to-end-profile":
        raise RuntimeErrorEvidence("Foundation profile identity differs")
    baseline = profile["accepted_baseline"]
    if not isinstance(baseline, dict):
        raise RuntimeErrorEvidence("Foundation baseline declaration differs")
    require_exact_keys(baseline, {"path", "candidate_commit", "retention_manifest_sha256"}, "Foundation baseline")
    if not all(isinstance(baseline[key], str) and baseline[key] for key in baseline) or not GIT_COMMIT.fullmatch(baseline["candidate_commit"]) or not SHA256.fullmatch(baseline["retention_manifest_sha256"]):
        raise RuntimeErrorEvidence("Foundation baseline declaration differs")
    configurations = profile["configurations"]
    expected_configurations = {"gcc-debug", "gcc-release", "clang-debug", "clang-release"}
    if not isinstance(configurations, dict) or set(configurations) != expected_configurations or any(not isinstance(value, str) or not value for value in configurations.values()):
        raise RuntimeErrorEvidence("Foundation configuration matrix differs")
    dependencies = profile["allowed_runtime_dependencies"]
    if not isinstance(dependencies, dict) or set(dependencies) != set(configurations.values()):
        raise RuntimeErrorEvidence("Foundation dependency policy differs")
    for values in dependencies.values():
        if not isinstance(values, list) or not values or values != sorted(set(values)) or any(not isinstance(value, str) or not value for value in values):
            raise RuntimeErrorEvidence("Foundation dependency policy differs")
    if profile["gates"] != EXPECTED_GATES:
        raise RuntimeErrorEvidence("Foundation gate matrix differs")
    for name in ("claim_fields", "required_executables", "required_contract_tests", "allowed_scope_changes",
                 "allowed_runtime_prefixes", "allowed_pseudo_dependencies", "limitations"):
        values = profile[name]
        if not isinstance(values, list) or not values or len(values) != len(set(values)) or any(not isinstance(value, str) or not value for value in values):
            raise RuntimeErrorEvidence(f"Foundation {name} declaration differs")
    return profile


def _package_identity(package: pathlib.Path, rec_profile: pathlib.Path) -> tuple[dict[str, Any], dict[str, Any]]:
    verify_retained_package(package, rec_profile)
    retention = read_json(package / "retention-manifest.json")
    terminal = read_json(package / "terminal-manifest.json")
    if retention.get("candidate_commit") != terminal.get("candidate_commit"):
        raise RuntimeErrorEvidence("retained package candidate identity differs")
    return retention, terminal


def _bundle_claims(package: pathlib.Path, terminal: dict[str, Any], claim_fields: list[str]) -> dict[str, Any]:
    rows: dict[str, Any] = {}
    bundles = terminal.get("bundles")
    if not isinstance(bundles, list) or len(bundles) != 8:
        raise RuntimeErrorEvidence("Foundation REC bundle matrix differs")
    for row in bundles:
        if not isinstance(row, dict) or set(row) != {"cell", "replay", "bundle"}:
            raise RuntimeErrorEvidence("Foundation REC bundle identity differs")
        key = f"{row['cell']}/replay-{row['replay']}"
        summary = read_json(package / row["bundle"] / "summary.json")
        if key in rows or any(field not in summary for field in claim_fields):
            raise RuntimeErrorEvidence("Foundation REC claim projection differs")
        rows[key] = {field: summary[field] for field in claim_fields}
    return rows


def _prerequisite_claims(package: pathlib.Path, terminal: dict[str, Any], name: str) -> dict[str, Any]:
    prerequisites = terminal.get("prerequisites")
    if not isinstance(prerequisites, dict) or name not in prerequisites:
        raise RuntimeErrorEvidence("Foundation prerequisite linkage differs")
    entry = prerequisites[name]
    if not isinstance(entry, dict) or entry.get("candidate_commit") != terminal.get("candidate_commit"):
        raise RuntimeErrorEvidence("Foundation prerequisite candidate differs")
    manifest_path = package / entry["manifest"]
    manifest = read_json(manifest_path)
    rows = manifest.get("execution", {}).get("records")
    if not isinstance(rows, list):
        raise RuntimeErrorEvidence("Foundation prerequisite record matrix differs")
    projection: dict[str, Any] = {}
    for row in rows:
        if not isinstance(row, dict) or "cell" not in row:
            continue
        cell = row["cell"]
        certificates = row.get("certificates")
        if not isinstance(cell, str) or not isinstance(certificates, list) or not certificates:
            raise RuntimeErrorEvidence("Foundation prerequisite certificate matrix differs")
        cell_claims: dict[str, Any] = {"certificates": [read_json(manifest_path.parent / item) for item in certificates]}
        environments = row.get("environments")
        if environments is not None:
            if not isinstance(environments, list) or not environments:
                raise RuntimeErrorEvidence("Foundation prerequisite environment matrix differs")
            cell_claims["environments"] = [read_json(manifest_path.parent / item) for item in environments]
        projection[cell] = cell_claims
    if set(projection) != {"gcc-debug", "gcc-release", "clang-debug", "clang-release"}:
        raise RuntimeErrorEvidence("Foundation prerequisite configuration matrix differs")
    return projection


def package_claims(package: pathlib.Path, terminal: dict[str, Any], claim_fields: list[str]) -> dict[str, Any]:
    return {
        "rec": _bundle_claims(package, terminal, claim_fields),
        "architecture": _prerequisite_claims(package, terminal, "architecture_contract"),
        "numeric": _prerequisite_claims(package, terminal, "numeric_contract"),
    }


def source_changes(baseline_package: pathlib.Path, candidate_package: pathlib.Path,
                   allowed_patterns: list[str]) -> tuple[bool, list[dict[str, Any]]]:
    def inventory(package: pathlib.Path) -> dict[str, str]:
        prepared = read_json(package / "control" / "prepared-manifest.json")
        rows = prepared.get("candidate", {}).get("source_inventory")
        if not isinstance(rows, list) or not rows:
            raise RuntimeErrorEvidence("Foundation candidate source inventory differs")
        result: dict[str, str] = {}
        for row in rows:
            if not isinstance(row, dict) or set(row) != {"path", "sha256"} or not isinstance(row["path"], str) or not SHA256.fullmatch(row["sha256"]) or row["path"] in result:
                raise RuntimeErrorEvidence("Foundation candidate source inventory differs")
            result[row["path"]] = row["sha256"]
        return result

    baseline, candidate = inventory(baseline_package), inventory(candidate_package)
    changes: list[dict[str, Any]] = []
    for path in sorted(set(baseline) | set(candidate)):
        if baseline.get(path) == candidate.get(path):
            continue
        allowed = any(fnmatch.fnmatchcase(path, pattern) for pattern in allowed_patterns)
        changes.append({"path": path, "baseline_sha256": baseline.get(path), "candidate_sha256": candidate.get(path),
                        "classification": "EXPECTED_CHANGE" if allowed else "INVESTIGATION_REQUIRED"})
    return all(row["classification"] == "EXPECTED_CHANGE" for row in changes), changes


def _diff(baseline: Any, candidate: Any, path: str = "") -> list[dict[str, Any]]:
    if type(baseline) is not type(candidate):
        return [{"path": path or "/", "classification": "REGRESSION", "baseline": baseline, "candidate": candidate}]
    if isinstance(baseline, dict):
        differences: list[dict[str, Any]] = []
        for key in sorted(set(baseline) | set(candidate)):
            child = f"{path}/{key}"
            if key not in baseline or key not in candidate:
                differences.append({"path": child, "classification": "REGRESSION", "baseline": baseline.get(key), "candidate": candidate.get(key)})
            else:
                differences.extend(_diff(baseline[key], candidate[key], child))
        return differences
    if isinstance(baseline, list):
        if len(baseline) != len(candidate):
            return [{"path": path or "/", "classification": "REGRESSION", "baseline": baseline, "candidate": candidate}]
        differences: list[dict[str, Any]] = []
        for index, (left, right) in enumerate(zip(baseline, candidate)):
            differences.extend(_diff(left, right, f"{path}/{index}"))
        return differences
    return [] if baseline == candidate else [{"path": path or "/", "classification": "REGRESSION", "baseline": baseline, "candidate": candidate}]


def validate_publication(path: pathlib.Path, candidate_commit: str) -> bool:
    publication = read_json(path)
    require_exact_keys(publication, {"schema_version", "kind", "branch", "head", "upstream", "upstream_head", "tree_clean"}, "Foundation publication")
    if publication["schema_version"] != 1 or publication["kind"] != "foundation-candidate-publication":
        raise RuntimeErrorEvidence("Foundation publication schema differs")
    return (publication["tree_clean"] is True and publication["head"] == candidate_commit and
            publication["upstream_head"] == candidate_commit and
            isinstance(publication["branch"], str) and bool(publication["branch"]) and
            isinstance(publication["upstream"], str) and bool(publication["upstream"]))


def validate_dependencies(path: pathlib.Path, profile: dict[str, Any], candidate_commit: str) -> tuple[bool, list[dict[str, str]]]:
    inventory = read_json(path)
    require_exact_keys(inventory, {"schema_version", "kind", "candidate_commit", "cells"}, "Foundation dependency inventory")
    if inventory["schema_version"] != 1 or inventory["kind"] != "foundation-runtime-dependencies" or inventory["candidate_commit"] != candidate_commit:
        raise RuntimeErrorEvidence("Foundation dependency inventory identity differs")
    cells = inventory["cells"]
    if not isinstance(cells, list) or {row.get("name") for row in cells if isinstance(row, dict)} != set(profile["configurations"]) or len(cells) != 4:
        raise RuntimeErrorEvidence("Foundation dependency cell matrix differs")
    findings: list[dict[str, str]] = []
    required = set(profile["required_executables"])
    for cell in cells:
        require_exact_keys(cell, {"name", "executables"}, "Foundation dependency cell")
        executables = cell["executables"]
        if not isinstance(executables, list) or {row.get("name") for row in executables if isinstance(row, dict)} != required or len(executables) != len(required):
            raise RuntimeErrorEvidence("Foundation executable dependency matrix differs")
        allowed = set(profile["allowed_runtime_dependencies"][profile["configurations"][cell["name"]]])
        for executable in executables:
            require_exact_keys(executable, {"name", "path", "sha256", "dependencies", "unresolved"}, "Foundation executable dependency")
            if not isinstance(executable["path"], str) or not executable["path"] or not isinstance(executable["sha256"], str) or not SHA256.fullmatch(executable["sha256"]):
                raise RuntimeErrorEvidence("Foundation executable identity differs")
            if not isinstance(executable["unresolved"], list) or any(not isinstance(value, str) or not value for value in executable["unresolved"]):
                raise RuntimeErrorEvidence("Foundation unresolved dependency evidence differs")
            if executable["unresolved"]:
                findings.extend({"cell": cell["name"], "executable": executable["name"], "dependency": value, "classification": "REGRESSION"} for value in executable["unresolved"])
            dependencies = executable["dependencies"]
            if not isinstance(dependencies, list):
                raise RuntimeErrorEvidence("Foundation dependency rows differ")
            seen: set[str] = set()
            for dependency in dependencies:
                if not isinstance(dependency, dict):
                    raise RuntimeErrorEvidence("Foundation dependency row differs")
                require_exact_keys(dependency, {"soname", "resolved_path"}, "Foundation dependency row")
                soname, resolved = dependency["soname"], dependency["resolved_path"]
                if not isinstance(soname, str) or not soname or soname in seen or not isinstance(resolved, str) or not resolved:
                    raise RuntimeErrorEvidence("Foundation dependency row differs")
                seen.add(soname)
                if soname not in allowed:
                    findings.append({"cell": cell["name"], "executable": executable["name"], "dependency": soname, "classification": "REGRESSION"})
                elif soname in profile["allowed_pseudo_dependencies"]:
                    if resolved != soname:
                        findings.append({"cell": cell["name"], "executable": executable["name"], "dependency": soname, "classification": "INVESTIGATION_REQUIRED"})
                else:
                    normalized = posixpath.normpath(resolved)
                    if normalized != resolved or not normalized.startswith("/") or not any(normalized.startswith(prefix) for prefix in profile["allowed_runtime_prefixes"]):
                        findings.append({"cell": cell["name"], "executable": executable["name"], "dependency": soname, "classification": "INVESTIGATION_REQUIRED"})
    return not findings, findings


def validate_contract_tests(path: pathlib.Path, profile: dict[str, Any], candidate_commit: str) -> tuple[bool, list[dict[str, Any]]]:
    inventory = read_json(path)
    require_exact_keys(inventory, {"schema_version", "kind", "candidate_commit", "cells"}, "Foundation contract-test inventory")
    if inventory["schema_version"] != 1 or inventory["kind"] != "foundation-contract-tests" or inventory["candidate_commit"] != candidate_commit:
        raise RuntimeErrorEvidence("Foundation contract-test inventory identity differs")
    cells = inventory["cells"]
    expected_cells = set(profile["configurations"])
    if not isinstance(cells, list) or {row.get("name") for row in cells if isinstance(row, dict)} != expected_cells or len(cells) != len(expected_cells):
        raise RuntimeErrorEvidence("Foundation contract-test cell matrix differs")
    required = set(profile["required_contract_tests"])
    findings: list[dict[str, Any]] = []
    for cell in cells:
        require_exact_keys(cell, {"name", "command", "discovered", "passed", "failed", "exit_code"}, "Foundation contract-test cell")
        command = cell["command"]
        discovered, passed, failed = cell["discovered"], cell["passed"], cell["failed"]
        if not isinstance(command, list) or not command or any(not isinstance(value, str) or not value for value in command) or pathlib.Path(command[0]).name != "ctest" or "-L" not in command or "contract" not in command:
            raise RuntimeErrorEvidence("Foundation contract-test command differs")
        for name, values in (("discovered", discovered), ("passed", passed), ("failed", failed)):
            if not isinstance(values, list) or len(values) != len(set(values)) or any(not isinstance(value, str) or not value for value in values):
                raise RuntimeErrorEvidence(f"Foundation contract-test {name} evidence differs")
        missing = sorted(required - set(discovered)); not_passed = sorted(required - set(passed))
        if cell["exit_code"] != 0 or failed or missing or not_passed or set(passed) != set(discovered):
            findings.append({"cell": cell["name"], "classification": "REGRESSION", "missing": missing,
                             "not_passed": not_passed, "failed": failed, "exit_code": cell["exit_code"]})
    return not findings, findings


def _terminal_complete(terminal: dict[str, Any], candidate_commit: str) -> bool:
    return (terminal.get("state") == "EXECUTED_PENDING_AUDIT" and terminal.get("candidate_commit") == candidate_commit and
            isinstance(terminal.get("bundles"), list) and len(terminal["bundles"]) == 8 and
            isinstance(terminal.get("negative_fixtures"), list) and len(terminal["negative_fixtures"]) == 8 and
            isinstance(terminal.get("prerequisites"), dict) and
            all(value.get("candidate_commit") == candidate_commit for value in terminal["prerequisites"].values()))


def _claims_equivalent(terminal: dict[str, Any]) -> bool:
    replays = terminal.get("replay_comparisons")
    cross = terminal.get("cross_configuration")
    return (isinstance(replays, list) and len(replays) == 4 and all(row.get("claim_fields") == "equivalent" for row in replays) and
            isinstance(cross, dict) and cross.get("claim_fields") == "equivalent")


def _write_outputs(output: pathlib.Path, summary: dict[str, Any]) -> None:
    if output.exists() and any(output.iterdir()):
        raise RuntimeErrorEvidence("Foundation qualifier output root is not empty")
    output.mkdir(parents=True, exist_ok=True)
    write_json(output / "summary.json", summary)
    write_json(output / "certificate.json", {"schema_version": 1, "kind": "foundation-end-to-end-certificate",
                                               "state": summary["state"], "candidate_commit": summary["candidate_commit"],
                                               "baseline_commit": summary["baseline_commit"], "gates": summary["gates"],
                                               "limitations": summary["limitations"]})
    report = ["# Foundation End-to-End Evidence", "", f"State: `{summary['state']}`", "", "| Gate | Evidence status |", "| --- | --- |"]
    report.extend(f"| {gate} | {status} |" for gate, status in summary["gates"].items())
    report.extend(["", f"Baseline differences: {len(summary['baseline_comparison']['differences'])}",
                   f"Dependency findings: {len(summary['dependency_findings'])}", ""])
    (output / "report.md").write_text("\n".join(report), encoding="utf-8", newline="\n")
    stream = io.StringIO(newline="")
    writer = csv.writer(stream, lineterminator="\n"); writer.writerow(["gate", "status"])
    writer.writerows(summary["gates"].items())
    (output / "metrics.csv").write_text(stream.getvalue(), encoding="utf-8", newline="\n")
    colors = {PENDING: "#2f855a", "BLOCKED": "#c53030"}
    cells = "".join(f'<rect x="{20 + index * 90}" y="25" width="75" height="35" fill="{colors[status]}"/><text x="{57 + index * 90}" y="47" text-anchor="middle" fill="white">{gate}</text>' for index, (gate, status) in enumerate(summary["gates"].items()))
    svg = f'<svg xmlns="http://www.w3.org/2000/svg" width="760" height="85" viewBox="0 0 760 85"><title>Foundation gate evidence</title>{cells}</svg>\n'
    figures = output / "figures"; figures.mkdir(); (figures / "status-matrix.svg").write_text(svg, encoding="utf-8", newline="\n")
    artifacts = ["summary.json", "certificate.json", "report.md", "metrics.csv", "figures/status-matrix.svg"]
    write_json(output / "artifact-inventory.json", {"schema_version": 1, "kind": "foundation-end-to-end-artifact-inventory",
                                                      "artifacts": [{"path": item, "sha256": sha256_file(output / item)} for item in artifacts]})


def qualify(profile_path: pathlib.Path, rec_profile: pathlib.Path, baseline_package: pathlib.Path,
            candidate_package: pathlib.Path, publication_path: pathlib.Path,
            dependencies_path: pathlib.Path, contract_tests_path: pathlib.Path,
            output: pathlib.Path) -> dict[str, Any]:
    profile = validate_profile(profile_path)
    baseline_manifest = baseline_package / "retention-manifest.json"
    declared = profile["accepted_baseline"]
    declared_path = pathlib.Path(declared["path"])
    if not declared_path.is_absolute():
        declared_path = profile_path.resolve().parents[2] / declared_path
    if baseline_package.resolve() != declared_path.resolve() or sha256_file(baseline_manifest) != declared["retention_manifest_sha256"]:
        raise RuntimeErrorEvidence("accepted Foundation baseline identity differs")
    baseline_retention, baseline_terminal = _package_identity(baseline_package, rec_profile)
    candidate_retention, candidate_terminal = _package_identity(candidate_package, rec_profile)
    if baseline_retention.get("candidate_commit") != declared["candidate_commit"]:
        raise RuntimeErrorEvidence("accepted Foundation baseline candidate differs")
    candidate_commit = candidate_retention.get("candidate_commit")
    if not isinstance(candidate_commit, str) or not GIT_COMMIT.fullmatch(candidate_commit):
        raise RuntimeErrorEvidence("Foundation candidate commit differs")
    publication_ok = validate_publication(publication_path, candidate_commit)
    dependencies_ok, dependency_findings = validate_dependencies(dependencies_path, profile, candidate_commit)
    contract_tests_ok, contract_test_findings = validate_contract_tests(contract_tests_path, profile, candidate_commit)
    scope_ok, scope_changes_rows = source_changes(baseline_package, candidate_package, profile["allowed_scope_changes"])
    baseline_claims = package_claims(baseline_package, baseline_terminal, profile["claim_fields"])
    candidate_claims = package_claims(candidate_package, candidate_terminal, profile["claim_fields"])
    differences = _diff(baseline_claims, candidate_claims)
    terminal_complete = _terminal_complete(candidate_terminal, candidate_commit)
    claims_equivalent = _claims_equivalent(candidate_terminal)
    conditions = {
        "FND0": publication_ok and scope_ok,
        "FND1": True,
        "FND2": terminal_complete,
        "FND3": contract_tests_ok,
        "FND4": claims_equivalent,
        "FND5": not differences,
        "FND6": True,
        "FND7": dependencies_ok,
    }
    gates = {gate: PENDING if conditions[gate] else "BLOCKED" for gate in EXPECTED_GATES}
    summary = {
        "schema_version": 1,
        "kind": "foundation-end-to-end-evidence-summary",
        "state": PENDING if all(conditions.values()) else "BLOCKED",
        "candidate_commit": candidate_commit,
        "baseline_commit": baseline_retention["candidate_commit"],
        "baseline_comparison": {"classification": "NO_CHANGE" if not differences else "REGRESSION", "differences": differences},
        "scope_changes": scope_changes_rows,
        "dependency_findings": dependency_findings,
        "contract_test_findings": contract_test_findings,
        "gates": gates,
        "limitations": profile["limitations"],
    }
    _write_outputs(output, summary)
    return summary


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--profile", required=True); parser.add_argument("--rec-profile", required=True)
    parser.add_argument("--baseline-package", required=True); parser.add_argument("--candidate-package", required=True)
    parser.add_argument("--publication", required=True); parser.add_argument("--dependencies", required=True)
    parser.add_argument("--contract-tests", required=True)
    parser.add_argument("--output", required=True)
    return parser.parse_args()


def main() -> int:
    arguments = parse_arguments()
    try:
        summary = qualify(pathlib.Path(arguments.profile), pathlib.Path(arguments.rec_profile),
                          pathlib.Path(arguments.baseline_package), pathlib.Path(arguments.candidate_package),
                          pathlib.Path(arguments.publication), pathlib.Path(arguments.dependencies),
                          pathlib.Path(arguments.contract_tests), pathlib.Path(arguments.output))
        return 0 if summary["state"] == PENDING else 1
    except (OSError, RuntimeErrorEvidence) as error:
        print(error, file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
