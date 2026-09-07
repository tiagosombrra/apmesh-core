#!/usr/bin/env python3
"""Report-only, revision-bound qualification of Foundation evidence."""

from __future__ import annotations

import argparse
import csv
import io
import pathlib
import posixpath
import re
import shutil
import subprocess
import sys
import xml.etree.ElementTree as etree
from typing import Any

from experiment_runtime import (RuntimeErrorEvidence, read_json, relative_path,
                                require_exact_keys, sha256_file, write_json)
from retain_experiment_evidence import verify as verify_retained_package


EXPECTED_GATES = [f"FND{index}" for index in range(8)]
PENDING = "EVIDENCE_COLLECTED_PENDING_AUDIT"
BLOCKED = "BLOCKED"
SHA256 = re.compile(r"[0-9a-f]{64}")
GIT_COMMIT = re.compile(r"[0-9a-f]{40}")
LDD_RESOLVED = re.compile(r"^\s*(?P<soname>\S+)\s+=>\s+(?P<path>\S+)\s+\(")
LDD_DIRECT = re.compile(r"^\s*(?P<path>/\S+)\s+\(")
LDD_PSEUDO = re.compile(r"^\s*(?P<soname>linux-vdso\.so\.1)\s+\(")
LDD_MISSING = re.compile(r"^\s*(?P<soname>\S+)\s+=>\s+not found\s*$")


def _sha(value: Any, context: str) -> str:
    if not isinstance(value, str) or not SHA256.fullmatch(value):
        raise RuntimeErrorEvidence(f"{context} hash differs")
    return value


def _relative(root: pathlib.Path, value: Any, context: str) -> pathlib.Path:
    if not isinstance(value, str) or not value:
        raise RuntimeErrorEvidence(f"{context} path differs")
    path = pathlib.Path(value)
    if path.is_absolute() or any(part in {"", ".", ".."} for part in path.parts):
        raise RuntimeErrorEvidence(f"{context} path differs")
    resolved = (root / path).resolve()
    try:
        resolved.relative_to(root.resolve())
    except ValueError as error:
        raise RuntimeErrorEvidence(f"{context} path escapes root") from error
    return resolved


def _hash_bound_file(root: pathlib.Path, row: Any, context: str) -> pathlib.Path:
    if not isinstance(row, dict):
        raise RuntimeErrorEvidence(f"{context} record differs")
    require_exact_keys(row, {"path", "sha256"}, context)
    path = _relative(root, row["path"], context)
    if not path.is_file() or sha256_file(path) != _sha(row["sha256"], context):
        raise RuntimeErrorEvidence(f"{context} identity differs")
    return path


def validate_profile(path: pathlib.Path) -> dict[str, Any]:
    profile = read_json(path)
    require_exact_keys(profile, {"schema_version", "kind", "accepted_baseline", "configurations", "claim_fields",
                                 "required_executables", "required_contract_tests", "scope_policy",
                                 "qualified_authorities", "allowed_runtime_dependencies", "allowed_runtime_prefixes",
                                 "allowed_pseudo_dependencies", "gates", "limitations"},
                       "Foundation profile")
    if profile["schema_version"] != 2 or profile["kind"] != "foundation-end-to-end-profile":
        raise RuntimeErrorEvidence("Foundation profile identity differs")
    baseline = profile["accepted_baseline"]
    if not isinstance(baseline, dict):
        raise RuntimeErrorEvidence("Foundation baseline declaration differs")
    require_exact_keys(baseline, {"path", "candidate_commit", "retention_manifest_sha256"}, "Foundation baseline")
    if (not isinstance(baseline["path"], str) or not baseline["path"] or
            not isinstance(baseline["candidate_commit"], str) or not GIT_COMMIT.fullmatch(baseline["candidate_commit"]) or
            not isinstance(baseline["retention_manifest_sha256"], str) or not SHA256.fullmatch(baseline["retention_manifest_sha256"])):
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
    for name in ("claim_fields", "required_executables", "required_contract_tests", "allowed_runtime_prefixes",
                 "allowed_pseudo_dependencies", "limitations"):
        values = profile[name]
        if not isinstance(values, list) or not values or len(values) != len(set(values)) or any(not isinstance(value, str) or not value for value in values):
            raise RuntimeErrorEvidence(f"Foundation {name} declaration differs")
    scope = profile["scope_policy"]
    if not isinstance(scope, dict):
        raise RuntimeErrorEvidence("Foundation scope policy differs")
    require_exact_keys(scope, {"protected_paths", "protected_prefixes", "approved_support_paths", "profile_path", "verified_retained_prefix"}, "Foundation scope policy")
    protected = scope["protected_paths"]
    if not isinstance(protected, list) or not protected or len(protected) != len(set(protected)) or any(not isinstance(value, str) or not value or "*" in value for value in protected):
        raise RuntimeErrorEvidence("Foundation scope policy differs")
    prefixes = scope["protected_prefixes"]
    if not isinstance(prefixes, list) or not prefixes or len(prefixes) != len(set(prefixes)) or any(not isinstance(value, str) or not value.endswith("/") or "*" in value for value in prefixes):
        raise RuntimeErrorEvidence("Foundation scope policy differs")
    if not isinstance(scope["profile_path"], str) or not scope["profile_path"] or "*" in scope["profile_path"]:
        raise RuntimeErrorEvidence("Foundation scope policy differs")
    support = scope["approved_support_paths"]
    if not isinstance(support, list) or not support:
        raise RuntimeErrorEvidence("Foundation scope policy differs")
    seen_support: set[str] = set()
    for row in support:
        if not isinstance(row, dict):
            raise RuntimeErrorEvidence("Foundation scope policy differs")
        require_exact_keys(row, {"path", "candidate_sha256"}, "Foundation support path")
        if not isinstance(row["path"], str) or not row["path"] or "*" in row["path"] or row["path"] in seen_support:
            raise RuntimeErrorEvidence("Foundation scope policy differs")
        _sha(row["candidate_sha256"], "Foundation support path")
        seen_support.add(row["path"])
    retained = scope["verified_retained_prefix"]
    if not isinstance(retained, dict):
        raise RuntimeErrorEvidence("Foundation retained-prefix policy differs")
    require_exact_keys(retained, {"path", "retention_manifest_sha256"}, "Foundation retained-prefix policy")
    if not isinstance(retained["path"], str) or not retained["path"] or "/**" in retained["path"] or not SHA256.fullmatch(retained["retention_manifest_sha256"]):
        raise RuntimeErrorEvidence("Foundation retained-prefix policy differs")
    authorities = profile["qualified_authorities"]
    if not isinstance(authorities, list) or len(authorities) != 3:
        raise RuntimeErrorEvidence("Foundation authority policy differs")
    observed: set[str] = set()
    for row in authorities:
        if not isinstance(row, dict):
            raise RuntimeErrorEvidence("Foundation authority policy differs")
        require_exact_keys(row, {"name", "path", "sha256", "qualified_marker"}, "Foundation authority policy")
        if (not isinstance(row["name"], str) or row["name"] in observed or not isinstance(row["path"], str) or
                not row["path"] or not isinstance(row["qualified_marker"], str) or not row["qualified_marker"]):
            raise RuntimeErrorEvidence("Foundation authority policy differs")
        _sha(row["sha256"], "Foundation authority policy")
        observed.add(row["name"])
    if observed != {"architecture", "numeric", "reproducible_experiment"}:
        raise RuntimeErrorEvidence("Foundation authority policy differs")
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
        cell, certificates = row["cell"], row.get("certificates")
        if not isinstance(cell, str) or not isinstance(certificates, list) or not certificates:
            raise RuntimeErrorEvidence("Foundation prerequisite certificate matrix differs")
        claim: dict[str, Any] = {"certificates": [read_json(manifest_path.parent / item) for item in certificates]}
        environments = row.get("environments")
        if environments is not None:
            if not isinstance(environments, list) or not environments:
                raise RuntimeErrorEvidence("Foundation prerequisite environment matrix differs")
            claim["environments"] = [read_json(manifest_path.parent / item) for item in environments]
        projection[cell] = claim
    if set(projection) != {"gcc-debug", "gcc-release", "clang-debug", "clang-release"}:
        raise RuntimeErrorEvidence("Foundation prerequisite configuration matrix differs")
    return projection


def package_claims(package: pathlib.Path, terminal: dict[str, Any], claim_fields: list[str]) -> dict[str, Any]:
    return {"rec": _bundle_claims(package, terminal, claim_fields),
            "architecture": _prerequisite_claims(package, terminal, "architecture_contract"),
            "numeric": _prerequisite_claims(package, terminal, "numeric_contract")}


def _source_inventory(package: pathlib.Path) -> dict[str, str]:
    prepared = read_json(package / "control" / "prepared-manifest.json")
    rows = prepared.get("candidate", {}).get("source_inventory")
    if not isinstance(rows, list) or not rows:
        raise RuntimeErrorEvidence("Foundation candidate source inventory differs")
    result: dict[str, str] = {}
    for row in rows:
        if not isinstance(row, dict) or set(row) != {"path", "sha256"} or not isinstance(row["path"], str) or row["path"] in result:
            raise RuntimeErrorEvidence("Foundation candidate source inventory differs")
        result[row["path"]] = _sha(row["sha256"], "Foundation candidate source inventory")
    return result


def source_changes(baseline_package: pathlib.Path, candidate_package: pathlib.Path,
                   profile: dict[str, Any], profile_path: pathlib.Path) -> tuple[bool, list[dict[str, Any]]]:
    baseline, candidate = _source_inventory(baseline_package), _source_inventory(candidate_package)
    policy = profile["scope_policy"]
    protected, protected_prefixes = set(policy["protected_paths"]), tuple(policy["protected_prefixes"])
    support = {row["path"]: row["candidate_sha256"] for row in policy["approved_support_paths"]}
    retained_prefix = policy["verified_retained_prefix"]["path"].rstrip("/") + "/"
    expected_retention = policy["verified_retained_prefix"]["retention_manifest_sha256"]
    rows: list[dict[str, Any]] = []
    for path in sorted(set(baseline) | set(candidate)):
        if baseline.get(path) == candidate.get(path):
            continue
        if path == policy["profile_path"] and candidate.get(path) == sha256_file(profile_path):
            classification = "EXPECTED_PROFILE_IDENTITY"
        elif path in support and candidate.get(path) == support[path]:
            classification = "EXPECTED_CHANGE"
        elif path in protected or path.startswith(protected_prefixes):
            classification = "BLOCKED_PROTECTED_CHANGE"
        elif path.startswith(retained_prefix):
            canonical = baseline_package / path[len(retained_prefix):]
            if canonical.is_file() and sha256_file(canonical) == candidate[path] and sha256_file(baseline_package / "retention-manifest.json") == expected_retention:
                classification = "EXPECTED_RETAINED_BASELINE"
            else:
                classification = "INVESTIGATION_REQUIRED"
        else:
            classification = "INVESTIGATION_REQUIRED"
        rows.append({"path": path, "baseline_sha256": baseline.get(path), "candidate_sha256": candidate.get(path),
                     "classification": classification})
    return all(row["classification"] in {"EXPECTED_CHANGE", "EXPECTED_PROFILE_IDENTITY", "EXPECTED_RETAINED_BASELINE"} for row in rows), rows


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
        return [difference for index, (left, right) in enumerate(zip(baseline, candidate))
                for difference in _diff(left, right, f"{path}/{index}")]
    return [] if baseline == candidate else [{"path": path or "/", "classification": "REGRESSION", "baseline": baseline, "candidate": candidate}]


def source_identity(source_root: pathlib.Path) -> dict[str, Any]:
    commands = {"status": ["git", "status", "--porcelain"], "head": ["git", "rev-parse", "HEAD"],
                "branch": ["git", "branch", "--show-current"],
                "upstream": ["git", "rev-parse", "--abbrev-ref", "--symbolic-full-name", "@{u}"],
                "upstream_head": ["git", "rev-parse", "@{u}"]}
    values: dict[str, str] = {}
    for name, command in commands.items():
        completed = subprocess.run(command, cwd=source_root, capture_output=True, text=True, check=False)
        if completed.returncode != 0:
            raise RuntimeErrorEvidence(f"Foundation publication command failed: {name}")
        values[name] = completed.stdout.strip()
    return {"branch": values["branch"], "head": values["head"], "upstream": values["upstream"],
            "upstream_head": values["upstream_head"], "tree_clean": not values["status"]}


def validate_publication(path: pathlib.Path, candidate_commit: str, source_root: pathlib.Path) -> bool:
    publication = read_json(path)
    require_exact_keys(publication, {"schema_version", "kind", "branch", "head", "upstream", "upstream_head", "tree_clean"}, "Foundation publication")
    if publication["schema_version"] != 1 or publication["kind"] != "foundation-candidate-publication":
        raise RuntimeErrorEvidence("Foundation publication schema differs")
    observed = source_identity(source_root)
    return publication == observed and observed["tree_clean"] is True and observed["head"] == candidate_commit and observed["upstream_head"] == candidate_commit


def _parse_ldd(path: pathlib.Path) -> tuple[list[dict[str, str]], list[str]]:
    dependencies: list[dict[str, str]] = []; unresolved: list[str] = []
    for raw in path.read_text(encoding="utf-8").splitlines():
        line = raw.strip()
        if not line:
            continue
        missing = LDD_MISSING.match(line)
        if missing:
            unresolved.append(missing.group("soname")); continue
        pseudo = LDD_PSEUDO.match(line)
        if pseudo:
            dependencies.append({"soname": pseudo.group("soname"), "resolved_path": pseudo.group("soname")}); continue
        resolved = LDD_RESOLVED.match(line)
        if resolved:
            dependencies.append({"soname": resolved.group("soname"), "resolved_path": resolved.group("path")}); continue
        direct = LDD_DIRECT.match(line)
        if direct:
            dependencies.append({"soname": pathlib.PurePosixPath(direct.group("path")).name, "resolved_path": direct.group("path")}); continue
        raise RuntimeErrorEvidence("Foundation dependency raw ldd output differs")
    return dependencies, unresolved


def validate_dependencies(path: pathlib.Path, profile: dict[str, Any], candidate_commit: str, root: pathlib.Path) -> tuple[bool, list[dict[str, str]]]:
    inventory = read_json(path)
    require_exact_keys(inventory, {"schema_version", "kind", "candidate_commit", "cells"}, "Foundation dependency inventory")
    if inventory["schema_version"] != 2 or inventory["kind"] != "foundation-runtime-dependencies" or inventory["candidate_commit"] != candidate_commit:
        raise RuntimeErrorEvidence("Foundation dependency inventory identity differs")
    cells = inventory["cells"]
    if not isinstance(cells, list) or {row.get("name") for row in cells if isinstance(row, dict)} != set(profile["configurations"]) or len(cells) != 4:
        raise RuntimeErrorEvidence("Foundation dependency cell matrix differs")
    findings: list[dict[str, str]] = []; required = set(profile["required_executables"])
    for cell in cells:
        require_exact_keys(cell, {"name", "executables"}, "Foundation dependency cell")
        executables = cell["executables"]
        if not isinstance(executables, list) or {row.get("name") for row in executables if isinstance(row, dict)} != required or len(executables) != len(required):
            raise RuntimeErrorEvidence("Foundation executable dependency matrix differs")
        allowed = set(profile["allowed_runtime_dependencies"][profile["configurations"][cell["name"]]])
        for executable in executables:
            require_exact_keys(executable, {"name", "path", "sha256", "ldd"}, "Foundation executable dependency")
            executable_path = _relative(root, executable["path"], "Foundation executable")
            if not executable_path.is_file() or sha256_file(executable_path) != _sha(executable["sha256"], "Foundation executable"):
                raise RuntimeErrorEvidence("Foundation executable identity differs")
            ldd_path = _hash_bound_file(root, executable["ldd"], "Foundation executable ldd")
            dependencies, unresolved = _parse_ldd(ldd_path); seen: set[str] = set()
            for item in dependencies:
                soname, resolved = item["soname"], item["resolved_path"]
                if soname in seen:
                    raise RuntimeErrorEvidence("Foundation dependency rows differ")
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
            findings.extend({"cell": cell["name"], "executable": executable["name"], "dependency": item, "classification": "REGRESSION"} for item in unresolved)
    return not findings, findings


def _contract_names(discovery: pathlib.Path) -> set[str]:
    data = read_json(discovery); tests = data.get("tests")
    if not isinstance(tests, list):
        raise RuntimeErrorEvidence("Foundation CTest discovery differs")
    names: set[str] = set()
    for test in tests:
        if not isinstance(test, dict) or not isinstance(test.get("name"), str):
            raise RuntimeErrorEvidence("Foundation CTest discovery differs")
        labels: list[str] = []
        for prop in test.get("properties", []):
            if isinstance(prop, dict) and prop.get("name") == "LABELS":
                value = prop.get("value"); labels = value if isinstance(value, list) else [value]
        if "contract" in labels:
            names.add(test["name"])
    return names


def _junit_passed(path: pathlib.Path) -> tuple[set[str], set[str]]:
    try:
        root = etree.parse(path).getroot()
    except etree.ParseError as error:
        raise RuntimeErrorEvidence("Foundation CTest JUnit differs") from error
    passed: set[str] = set(); failed: set[str] = set()
    for node in root.iter("testcase"):
        name = node.attrib.get("name")
        if not name:
            raise RuntimeErrorEvidence("Foundation CTest JUnit differs")
        (failed if any(child.tag in {"failure", "error", "skipped"} for child in node) else passed).add(name)
    return passed, failed


def validate_contract_tests(path: pathlib.Path, profile: dict[str, Any], candidate_commit: str, root: pathlib.Path) -> tuple[bool, list[dict[str, Any]]]:
    inventory = read_json(path)
    require_exact_keys(inventory, {"schema_version", "kind", "candidate_commit", "cells"}, "Foundation contract-test inventory")
    if inventory["schema_version"] != 2 or inventory["kind"] != "foundation-contract-tests" or inventory["candidate_commit"] != candidate_commit:
        raise RuntimeErrorEvidence("Foundation contract-test inventory identity differs")
    cells = inventory["cells"]; expected = set(profile["configurations"])
    if not isinstance(cells, list) or {row.get("name") for row in cells if isinstance(row, dict)} != expected or len(cells) != len(expected):
        raise RuntimeErrorEvidence("Foundation contract-test cell matrix differs")
    required = set(profile["required_contract_tests"]); findings: list[dict[str, Any]] = []
    for cell in cells:
        require_exact_keys(cell, {"name", "command", "discovery", "junit", "exit_code"}, "Foundation contract-test cell")
        command = cell["command"]
        if not isinstance(command, list) or not command or any(not isinstance(part, str) or not part for part in command) or pathlib.Path(command[0]).name != "ctest" or "-L" not in command or "contract" not in command or "--output-junit" not in command:
            raise RuntimeErrorEvidence("Foundation contract-test command differs")
        discovered = _contract_names(_hash_bound_file(root, cell["discovery"], "Foundation CTest discovery"))
        passed, failed = _junit_passed(_hash_bound_file(root, cell["junit"], "Foundation CTest JUnit"))
        missing, not_passed = sorted(required - discovered), sorted(required - passed)
        if cell["exit_code"] != 0 or failed or missing or not_passed or passed != discovered:
            findings.append({"cell": cell["name"], "classification": "REGRESSION", "missing": missing,
                             "not_passed": not_passed, "failed": sorted(failed), "exit_code": cell["exit_code"]})
    return not findings, findings


def validate_authorities(path: pathlib.Path, profile: dict[str, Any], candidate_commit: str,
                         source_root: pathlib.Path, candidate_terminal: dict[str, Any]) -> bool:
    record = read_json(path)
    require_exact_keys(record, {"schema_version", "kind", "candidate_commit", "authorities"}, "Foundation authority record")
    if record["schema_version"] != 1 or record["kind"] != "foundation-qualified-authorities" or record["candidate_commit"] != candidate_commit:
        raise RuntimeErrorEvidence("Foundation authority record differs")
    expected = {row["name"]: row for row in profile["qualified_authorities"]}; rows = record["authorities"]
    if not isinstance(rows, list) or {row.get("name") for row in rows if isinstance(row, dict)} != set(expected) or len(rows) != len(expected):
        raise RuntimeErrorEvidence("Foundation authority record differs")
    for row in rows:
        require_exact_keys(row, {"name", "path", "sha256", "qualified_marker"}, "Foundation authority record")
        declared = expected[row["name"]]
        if row != declared:
            return False
        source = _relative(source_root, row["path"], "Foundation authority")
        if not source.is_file() or sha256_file(source) != row["sha256"] or row["qualified_marker"] not in source.read_text(encoding="utf-8"):
            return False
    prerequisites = candidate_terminal.get("prerequisites")
    return (isinstance(prerequisites, dict) and set(prerequisites) == {"architecture_contract", "numeric_contract"} and
            all(isinstance(item, dict) and item.get("candidate_commit") == candidate_commit for item in prerequisites.values()))


def _terminal_complete(terminal: dict[str, Any], candidate_commit: str) -> bool:
    return (terminal.get("state") == "EXECUTED_PENDING_AUDIT" and terminal.get("candidate_commit") == candidate_commit and
            isinstance(terminal.get("bundles"), list) and len(terminal["bundles"]) == 8 and
            isinstance(terminal.get("negative_fixtures"), list) and len(terminal["negative_fixtures"]) == 8 and
            isinstance(terminal.get("prerequisites"), dict) and all(value.get("candidate_commit") == candidate_commit for value in terminal["prerequisites"].values()))


def _claims_equivalent(terminal: dict[str, Any]) -> bool:
    replays, cross = terminal.get("replay_comparisons"), terminal.get("cross_configuration")
    return (isinstance(replays, list) and len(replays) == 4 and all(row.get("claim_fields") == "equivalent" for row in replays) and
            isinstance(cross, dict) and cross.get("claim_fields") == "equivalent")


def _load_inputs(path: pathlib.Path, profile_path: pathlib.Path, candidate_retention: pathlib.Path,
                 candidate_commit: str, candidate_source_root: pathlib.Path) -> tuple[pathlib.Path, dict[str, pathlib.Path]]:
    manifest = read_json(path)
    require_exact_keys(manifest, {"schema_version", "kind", "candidate_commit", "candidate_retention_manifest_sha256", "profile_sha256", "source_root", "artifacts"}, "Foundation admission inputs")
    if (manifest["schema_version"] != 1 or manifest["kind"] != "foundation-admission-input-manifest" or
            manifest["candidate_commit"] != candidate_commit or manifest["candidate_retention_manifest_sha256"] != sha256_file(candidate_retention) or
            manifest["profile_sha256"] != sha256_file(profile_path) or not isinstance(manifest["source_root"], str)):
        raise RuntimeErrorEvidence("Foundation admission input identity differs")
    if pathlib.Path(manifest["source_root"]).resolve() != candidate_source_root.resolve():
        raise RuntimeErrorEvidence("Foundation admission source identity differs")
    artifacts = manifest["artifacts"]; expected = {"publication", "authorities", "dependencies", "contract_tests"}
    if not isinstance(artifacts, dict) or set(artifacts) != expected:
        raise RuntimeErrorEvidence("Foundation admission artifact matrix differs")
    root = path.parent.resolve()
    return root, {name: _hash_bound_file(root, artifacts[name], f"Foundation {name} artifact") for name in sorted(expected)}


def _write_text_outputs(output: pathlib.Path, summary: dict[str, Any]) -> None:
    write_json(output / "summary.json", summary)
    write_json(output / "certificate.json", {"schema_version": 1, "kind": "foundation-end-to-end-certificate",
                                               "state": summary["state"], "candidate_commit": summary["candidate_commit"],
                                               "baseline_commit": summary["baseline_commit"], "gates": summary["gates"],
                                               "limitations": summary["limitations"]})
    report = ["# Foundation End-to-End Evidence", "", f"State: `{summary['state']}`", "", "| Gate | Evidence status |", "| --- | --- |"]
    report.extend(f"| {gate} | {status} |" for gate, status in summary["gates"].items())
    report.extend(["", f"Baseline differences: {len(summary['baseline_comparison']['differences'])}", f"Dependency findings: {len(summary['dependency_findings'])}", ""])
    (output / "report.md").write_text("\n".join(report), encoding="utf-8", newline="\n")
    stream = io.StringIO(newline=""); writer = csv.writer(stream, lineterminator="\n")
    writer.writerow(["gate", "status"]); writer.writerows(summary["gates"].items())
    (output / "metrics.csv").write_text(stream.getvalue(), encoding="utf-8", newline="\n")
    colors = {PENDING: "#2f855a", BLOCKED: "#c53030"}
    cells = "".join(f'<rect x="{20 + index * 90}" y="25" width="75" height="35" fill="{colors[status]}"/><text x="{57 + index * 90}" y="47" text-anchor="middle" fill="white">{gate}</text>' for index, (gate, status) in enumerate(summary["gates"].items()))
    figures = output / "figures"; figures.mkdir()
    (figures / "status-matrix.svg").write_text(f'<svg xmlns="http://www.w3.org/2000/svg" width="760" height="85" viewBox="0 0 760 85"><title>Foundation gate evidence</title>{cells}</svg>\n', encoding="utf-8", newline="\n")


def _seal_output(output: pathlib.Path, profile_path: pathlib.Path, candidate_retention: pathlib.Path,
                 inputs_manifest: pathlib.Path, inputs: dict[str, pathlib.Path], candidate_commit: str) -> None:
    inputs_root = output / "inputs"; inputs_root.mkdir(); copied: dict[str, dict[str, str]] = {}
    manifest_target = inputs_root / "admission-input-manifest.json"; shutil.copy2(inputs_manifest, manifest_target)
    for role, path in inputs.items():
        suffix = path.suffix or ".bin"; target = inputs_root / f"{role}{suffix}"
        shutil.copy2(path, target); copied[role] = {"path": relative_path(output, target), "sha256": sha256_file(target)}
    write_json(output / "foundation-evidence-manifest.json", {"schema_version": 1, "kind": "foundation-end-to-end-evidence-binding", "candidate_commit": candidate_commit,
        "profile_sha256": sha256_file(profile_path), "candidate_retention_manifest_sha256": sha256_file(candidate_retention),
        "input_manifest": {"path": relative_path(output, manifest_target), "sha256": sha256_file(manifest_target)}, "artifacts": copied})
    files = sorted(path for path in output.rglob("*") if path.is_file())
    write_json(output / "artifact-inventory.json", {"schema_version": 1, "kind": "foundation-end-to-end-artifact-inventory",
        "artifacts": [{"path": relative_path(output, path), "sha256": sha256_file(path)} for path in files]})
    sealed = sorted(path for path in output.rglob("*") if path.is_file())
    write_json(output / "retention-manifest.json", {"schema_version": 1, "kind": "foundation-end-to-end-retention", "candidate_commit": candidate_commit,
        "files": [{"path": relative_path(output, path), "sha256": sha256_file(path), "size": path.stat().st_size} for path in sealed]})


def verify_foundation_retention(output: pathlib.Path, profile_path: pathlib.Path, candidate_retention: pathlib.Path,
                                candidate_commit: str) -> None:
    manifest = read_json(output / "retention-manifest.json")
    require_exact_keys(manifest, {"schema_version", "kind", "candidate_commit", "files"}, "Foundation retention")
    if manifest["schema_version"] != 1 or manifest["kind"] != "foundation-end-to-end-retention" or manifest["candidate_commit"] != candidate_commit or not isinstance(manifest["files"], list):
        raise RuntimeErrorEvidence("Foundation retention manifest differs")
    expected = {"retention-manifest.json"}
    for row in manifest["files"]:
        if not isinstance(row, dict):
            raise RuntimeErrorEvidence("Foundation retention manifest differs")
        require_exact_keys(row, {"path", "sha256", "size"}, "Foundation retention entry")
        path = _relative(output, row["path"], "Foundation retention entry")
        if not path.is_file() or path.stat().st_size != row["size"] or sha256_file(path) != _sha(row["sha256"], "Foundation retention entry"):
            raise RuntimeErrorEvidence("Foundation retained artifact differs")
        expected.add(row["path"])
    if {relative_path(output, path) for path in output.rglob("*") if path.is_file()} != expected:
        raise RuntimeErrorEvidence("Foundation retained artifact set differs")
    binding = read_json(output / "foundation-evidence-manifest.json")
    require_exact_keys(binding, {"schema_version", "kind", "candidate_commit", "profile_sha256", "candidate_retention_manifest_sha256", "input_manifest", "artifacts"}, "Foundation evidence binding")
    if (binding["schema_version"] != 1 or binding["kind"] != "foundation-end-to-end-evidence-binding" or binding["candidate_commit"] != candidate_commit or
            binding["profile_sha256"] != sha256_file(profile_path) or binding["candidate_retention_manifest_sha256"] != sha256_file(candidate_retention)):
        raise RuntimeErrorEvidence("Foundation evidence binding differs")
    _hash_bound_file(output, binding["input_manifest"], "Foundation retained input manifest")
    if not isinstance(binding["artifacts"], dict) or set(binding["artifacts"]) != {"publication", "authorities", "dependencies", "contract_tests"}:
        raise RuntimeErrorEvidence("Foundation retained input matrix differs")
    for role, row in binding["artifacts"].items():
        _hash_bound_file(output, row, f"Foundation retained {role}")


def _write_outputs(output: pathlib.Path, summary: dict[str, Any], profile_path: pathlib.Path,
                   candidate_retention: pathlib.Path, inputs_manifest: pathlib.Path, inputs: dict[str, pathlib.Path]) -> bool:
    if output.exists() and any(output.iterdir()):
        raise RuntimeErrorEvidence("Foundation qualifier output root is not empty")
    output.mkdir(parents=True, exist_ok=True)
    _write_text_outputs(output, summary)
    _seal_output(output, profile_path, candidate_retention, inputs_manifest, inputs, summary["candidate_commit"])
    verify_foundation_retention(output, profile_path, candidate_retention, summary["candidate_commit"])
    return True


def qualify(profile_path: pathlib.Path, rec_profile: pathlib.Path, baseline_package: pathlib.Path,
            candidate_package: pathlib.Path, inputs_manifest: pathlib.Path, output: pathlib.Path) -> dict[str, Any]:
    profile = validate_profile(profile_path); baseline_manifest = baseline_package / "retention-manifest.json"; declared = profile["accepted_baseline"]
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
    candidate_source = read_json(candidate_package / "control" / "prepared-manifest.json").get("candidate", {}).get("source_root")
    if not isinstance(candidate_source, str) or not candidate_source:
        raise RuntimeErrorEvidence("Foundation candidate source differs")
    input_root, inputs = _load_inputs(inputs_manifest, profile_path, candidate_package / "retention-manifest.json", candidate_commit, pathlib.Path(candidate_source))
    publication_ok = validate_publication(inputs["publication"], candidate_commit, pathlib.Path(candidate_source))
    authorities_ok = validate_authorities(inputs["authorities"], profile, candidate_commit, pathlib.Path(candidate_source), candidate_terminal)
    dependencies_ok, dependency_findings = validate_dependencies(inputs["dependencies"], profile, candidate_commit, input_root)
    contract_tests_ok, contract_test_findings = validate_contract_tests(inputs["contract_tests"], profile, candidate_commit, input_root)
    scope_ok, scope_changes_rows = source_changes(baseline_package, candidate_package, profile, profile_path)
    differences = _diff(package_claims(baseline_package, baseline_terminal, profile["claim_fields"]), package_claims(candidate_package, candidate_terminal, profile["claim_fields"]))
    conditions = {"FND0": publication_ok and scope_ok, "FND1": authorities_ok, "FND2": _terminal_complete(candidate_terminal, candidate_commit),
                  "FND3": contract_tests_ok, "FND4": _claims_equivalent(candidate_terminal), "FND5": not differences,
                  "FND7": dependencies_ok}
    gates = {gate: PENDING if gate == "FND6" or conditions[gate] else BLOCKED for gate in EXPECTED_GATES}
    summary = {"schema_version": 1, "kind": "foundation-end-to-end-evidence-summary", "state": PENDING if all(conditions.values()) else BLOCKED,
               "candidate_commit": candidate_commit, "baseline_commit": baseline_retention["candidate_commit"],
               "baseline_comparison": {"classification": "NO_CHANGE" if not differences else "REGRESSION", "differences": differences},
               "scope_changes": scope_changes_rows, "dependency_findings": dependency_findings, "contract_test_findings": contract_test_findings,
               "gates": gates, "limitations": profile["limitations"]}
    retention_verified = _write_outputs(output, summary, profile_path, candidate_package / "retention-manifest.json", inputs_manifest, inputs)
    conditions["FND6"] = retention_verified
    if not all(conditions.values()):
        summary["state"] = BLOCKED
    return summary


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--profile", required=True); parser.add_argument("--rec-profile", required=True)
    parser.add_argument("--baseline-package", required=True); parser.add_argument("--candidate-package", required=True)
    parser.add_argument("--inputs-manifest", required=True); parser.add_argument("--output", required=True)
    return parser.parse_args()


def main() -> int:
    arguments = parse_arguments()
    try:
        summary = qualify(pathlib.Path(arguments.profile), pathlib.Path(arguments.rec_profile), pathlib.Path(arguments.baseline_package),
                          pathlib.Path(arguments.candidate_package), pathlib.Path(arguments.inputs_manifest), pathlib.Path(arguments.output))
        return 0 if summary["state"] == PENDING else 1
    except (OSError, RuntimeErrorEvidence) as error:
        print(error, file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
