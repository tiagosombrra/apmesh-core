#!/usr/bin/env python3
"""Report-only, revision-bound qualification of Foundation evidence."""

from __future__ import annotations

import argparse
import csv
import datetime
import io
import json
import pathlib
import posixpath
import re
import shutil
import subprocess
import sys
import tempfile
import xml.etree.ElementTree as etree
from typing import Any

from experiment_runtime import (RuntimeErrorEvidence, read_json, relative_path,
                                require_exact_keys, run_command, sha256_file, write_json)
from retain_experiment_evidence import verify as verify_retained_package


EXPECTED_GATES = [f"FND{index}" for index in range(8)]
PREPARATION_GATES = [f"FPR{index}" for index in range(7)]
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
                                 "allowed_pseudo_dependencies", "gates", "preparation_preflight", "limitations"},
                       "Foundation profile")
    if profile["schema_version"] != 3 or profile["kind"] != "foundation-end-to-end-profile":
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
    preflight = profile["preparation_preflight"]
    if not isinstance(preflight, dict):
        raise RuntimeErrorEvidence("Foundation preparation preflight differs")
    require_exact_keys(preflight, {"gates", "runner", "report_only_tool", "report_only_contract", "execution_authorization"},
                       "Foundation preparation preflight")
    if preflight["gates"] != PREPARATION_GATES or preflight["execution_authorization"] is not False:
        raise RuntimeErrorEvidence("Foundation preparation preflight differs")
    for name in ("runner", "report_only_tool", "report_only_contract"):
        row = preflight[name]
        if not isinstance(row, dict):
            raise RuntimeErrorEvidence("Foundation preparation preflight differs")
        require_exact_keys(row, {"path", "sha256"}, "Foundation preparation preflight")
        if not isinstance(row["path"], str) or not row["path"]:
            raise RuntimeErrorEvidence("Foundation preparation preflight differs")
        _sha(row["sha256"], "Foundation preparation preflight")
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
    return {"schema_version": 1, "kind": "foundation-candidate-publication",
            "branch": values["branch"], "head": values["head"], "upstream": values["upstream"],
            "upstream_head": values["upstream_head"], "tree_clean": not values["status"]}


def _worktree_scope_changes(baseline_package: pathlib.Path, source_root: pathlib.Path,
                            candidate_commit: str, profile: dict[str, Any],
                            profile_path: pathlib.Path) -> tuple[bool, list[dict[str, Any]]]:
    baseline_commit = profile["accepted_baseline"]["candidate_commit"]
    changed = subprocess.run(["git", "diff", "--name-only", f"{baseline_commit}..{candidate_commit}"], cwd=source_root,
                            capture_output=True, text=True, check=False)
    if changed.returncode != 0:
        raise RuntimeErrorEvidence("Foundation candidate scope cannot be inspected")
    policy = profile["scope_policy"]
    protected, protected_prefixes = set(policy["protected_paths"]), tuple(policy["protected_prefixes"])
    support = {row["path"]: row["candidate_sha256"] for row in policy["approved_support_paths"]}
    retained_prefix = policy["verified_retained_prefix"]["path"].rstrip("/") + "/"
    expected_retention = policy["verified_retained_prefix"]["retention_manifest_sha256"]
    rows: list[dict[str, Any]] = []
    for name in sorted(set(changed.stdout.splitlines())):
        candidate = _relative(source_root, name, "Foundation candidate scope")
        if not candidate.is_file():
            classification = "BLOCKED_MISSING_OR_REMOVED_PATH"
        elif name == policy["profile_path"] and sha256_file(candidate) == sha256_file(profile_path):
            classification = "EXPECTED_PROFILE_IDENTITY"
        elif name in support and sha256_file(candidate) == support[name]:
            classification = "EXPECTED_CHANGE"
        elif name in protected or name.startswith(protected_prefixes):
            classification = "BLOCKED_PROTECTED_CHANGE"
        elif name.startswith(retained_prefix):
            historical = baseline_package / name[len(retained_prefix):]
            if (historical.is_file() and sha256_file(candidate) == sha256_file(historical) and
                    sha256_file(baseline_package / "retention-manifest.json") == expected_retention):
                classification = "EXPECTED_RETAINED_BASELINE"
            else:
                classification = "BLOCKED_RETAINED_BASELINE_CHANGE"
        else:
            classification = "BLOCKED_UNDECLARED_CHANGE"
        rows.append({"path": name, "candidate_sha256": sha256_file(candidate) if candidate.is_file() else None,
                     "classification": classification})
    allowed = {"EXPECTED_PROFILE_IDENTITY", "EXPECTED_CHANGE", "EXPECTED_RETAINED_BASELINE"}
    return all(row["classification"] in allowed for row in rows), rows


def _current_authorities(profile: dict[str, Any], source_root: pathlib.Path) -> bool:
    for row in profile["qualified_authorities"]:
        authority = _relative(source_root, row["path"], "Foundation qualified authority")
        if (not authority.is_file() or sha256_file(authority) != row["sha256"] or
                row["qualified_marker"] not in authority.read_text(encoding="utf-8")):
            return False
    return True


def _preflight_file(source_root: pathlib.Path, declaration: dict[str, Any], context: str) -> bool:
    path = _relative(source_root, declaration["path"], context)
    return path.is_file() and sha256_file(path) == declaration["sha256"]


def _runner_supports_fixed_matrix(source_root: pathlib.Path, rec_profile_path: pathlib.Path,
                                  declaration: dict[str, Any]) -> bool:
    if not _preflight_file(source_root, declaration, "Foundation REC runner"):
        return False
    profile = read_json(rec_profile_path)
    configurations = profile.get("configurations")
    names = [row.get("name") for row in configurations] if isinstance(configurations, list) else []
    if names != ["gcc-debug", "gcc-release", "clang-debug", "clang-release"] or profile.get("replays_per_cell") != 2:
        return False
    source = _relative(source_root, declaration["path"], "Foundation REC runner").read_text(encoding="utf-8")
    return all(token in source for token in ("def prepare", "def load_prepared", "def admit_execution", "--execute"))


def _external_empty_root(path: pathlib.Path, source_root: pathlib.Path) -> bool:
    resolved = path.resolve()
    try:
        resolved.relative_to(source_root.resolve())
    except ValueError:
        return not resolved.exists() or not any(resolved.iterdir())
    return False


def preflight(profile_path: pathlib.Path, rec_profile_path: pathlib.Path, baseline_package: pathlib.Path,
              source_root: pathlib.Path, control_root: pathlib.Path, evidence_root: pathlib.Path) -> dict[str, Any]:
    """Collect FPR0–FPR5 evidence without creating a manifest or running a campaign."""
    profile = validate_profile(profile_path)
    declared = profile["accepted_baseline"]
    baseline_path = pathlib.Path(declared["path"])
    if not baseline_path.is_absolute():
        baseline_path = profile_path.resolve().parents[2] / baseline_path
    if (baseline_package.resolve() != baseline_path.resolve() or
            sha256_file(baseline_package / "retention-manifest.json") != declared["retention_manifest_sha256"]):
        raise RuntimeErrorEvidence("accepted Foundation baseline identity differs")
    baseline_retention, _ = _package_identity(baseline_package, rec_profile_path)
    publication = source_identity(source_root)
    candidate_commit = publication["head"]
    if not GIT_COMMIT.fullmatch(candidate_commit):
        raise RuntimeErrorEvidence("Foundation preparation candidate differs")
    scope_ok, scope_changes = _worktree_scope_changes(baseline_package, source_root, candidate_commit, profile, profile_path)
    current_candidate = candidate_commit != baseline_retention["candidate_commit"]
    fpr0 = (publication["tree_clean"] and publication["head"] == publication["upstream_head"] and
            bool(publication["branch"]) and current_candidate and scope_ok)
    fpr1 = _current_authorities(profile, source_root)
    fpr2 = baseline_retention.get("candidate_commit") == declared["candidate_commit"]
    fpr3 = _runner_supports_fixed_matrix(source_root, rec_profile_path, profile["preparation_preflight"]["runner"])
    fpr4 = (_preflight_file(source_root, profile["preparation_preflight"]["report_only_tool"], "Foundation report-only tool") and
            _preflight_file(source_root, profile["preparation_preflight"]["report_only_contract"], "Foundation report-only contract"))
    fpr5 = (control_root.resolve() != evidence_root.resolve() and
            _external_empty_root(control_root, source_root) and _external_empty_root(evidence_root, source_root))
    conditions = {"FPR0": fpr0, "FPR1": fpr1, "FPR2": fpr2, "FPR3": fpr3, "FPR4": fpr4, "FPR5": fpr5}
    gates = {name: PENDING if passed else BLOCKED for name, passed in conditions.items()}
    gates["FPR6"] = PENDING if all(conditions.values()) else BLOCKED
    return {"schema_version": 1, "kind": "foundation-preparation-preflight-summary",
            "state": PENDING if all(conditions.values()) else BLOCKED,
            "candidate_commit": candidate_commit, "baseline_commit": baseline_retention["candidate_commit"],
            "publication": publication, "scope_changes": scope_changes, "gates": gates,
            "execution_authorization": False, "limitations": profile["limitations"]}


def build_source_state(source_root: pathlib.Path) -> dict[str, Any]:
    """Capture the clean tracked source consumed by one build/CTest cell."""
    status = subprocess.run(["git", "status", "--porcelain"], cwd=source_root, capture_output=True, text=True, check=False)
    head = subprocess.run(["git", "rev-parse", "HEAD"], cwd=source_root, capture_output=True, text=True, check=False)
    tracked = subprocess.run(["git", "ls-files"], cwd=source_root, capture_output=True, text=True, check=False)
    if status.returncode != 0 or head.returncode != 0 or tracked.returncode != 0 or status.stdout.strip() or not GIT_COMMIT.fullmatch(head.stdout.strip()):
        raise RuntimeErrorEvidence("Foundation build source is not a clean revision")
    inventory = []
    for name in tracked.stdout.splitlines():
        path = source_root / name
        if not path.is_file():
            raise RuntimeErrorEvidence("Foundation build source inventory differs")
        inventory.append({"path": name, "sha256": sha256_file(path)})
    return {"commit": head.stdout.strip(), "tree_clean": True, "source_inventory": inventory}


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


def _observed_record(record: dict[str, Any], log_parent: pathlib.Path, evidence_root: pathlib.Path) -> dict[str, Any]:
    """Project the common runtime record into the Foundation evidence schema."""
    return {"argv": record["argv"], "exit_code": record["exit_code"], "pid": record["pid"],
            "started_utc": record["started_utc"], "finished_utc": record["ended_utc"],
            "stdout": {"path": relative_path(evidence_root, log_parent / record["stdout"]["path"]), "sha256": record["stdout"]["sha256"]},
            "stderr": {"path": relative_path(evidence_root, log_parent / record["stderr"]["path"]), "sha256": record["stderr"]["sha256"]}}


def _build_provenance(row: Any, profile: dict[str, Any], candidate_commit: str, root: pathlib.Path,
                      candidate_source_root: pathlib.Path, recorded_root: pathlib.Path | None = None,
                      recorded_source_root: pathlib.Path | None = None) -> tuple[pathlib.Path, dict[str, str]]:
    if not isinstance(row, dict):
        raise RuntimeErrorEvidence("Foundation build provenance differs")
    require_exact_keys(row, {"candidate_commit", "source_root", "source_before", "source_after", "build_directory", "configure", "build", "outputs"},
                       "Foundation build provenance")
    command_root = (recorded_root or root).resolve()
    command_source = (recorded_source_root or candidate_source_root).resolve()
    if row["candidate_commit"] != candidate_commit or row["source_root"] != str(command_source):
        raise RuntimeErrorEvidence("Foundation build provenance identity differs")
    for name in ("source_before", "source_after"):
        state = row[name]
        if not isinstance(state, dict) or set(state) != {"commit", "tree_clean", "source_inventory"} or state["commit"] != candidate_commit or state["tree_clean"] is not True:
            raise RuntimeErrorEvidence("Foundation build source state differs")
        inventory = state["source_inventory"]
        if (not isinstance(inventory, list) or not inventory or any(not isinstance(item, dict) or set(item) != {"path", "sha256"} or
                not isinstance(item["path"], str) or not item["path"] or _sha(item["sha256"], "Foundation build source") != item["sha256"] for item in inventory)):
            raise RuntimeErrorEvidence("Foundation build source inventory differs")
    if row["source_before"] != row["source_after"] or row["source_after"] != build_source_state(candidate_source_root):
        raise RuntimeErrorEvidence("Foundation build source changed during collection")
    build_directory = _relative(root, row["build_directory"], "Foundation build directory")
    configure, build = row["configure"], row["build"]
    _execution_record(root, configure, configure.get("argv", []), "Foundation configure")
    _execution_record(root, build, build.get("argv", []), "Foundation build")
    expected_build = command_root / pathlib.PurePosixPath(row["build_directory"])
    source_index = configure["argv"].index("-S") if "-S" in configure["argv"] else -1
    build_index = configure["argv"].index("-B") if "-B" in configure["argv"] else -1
    if (not configure["argv"] or pathlib.Path(configure["argv"][0]).name != "cmake" or source_index < 0 or build_index < 0 or
            source_index + 1 >= len(configure["argv"]) or build_index + 1 >= len(configure["argv"]) or
            configure["argv"][source_index + 1] != str(command_source) or configure["argv"][build_index + 1] != str(expected_build) or
            not build["argv"] or pathlib.Path(build["argv"][0]).name != "cmake" or build["argv"][1:3] != ["--build", str(expected_build)]):
        raise RuntimeErrorEvidence("Foundation build provenance command differs")
    outputs = row["outputs"]
    if not isinstance(outputs, list) or {item.get("name") for item in outputs if isinstance(item, dict)} != set(profile["required_executables"]):
        raise RuntimeErrorEvidence("Foundation build output matrix differs")
    hashes: dict[str, str] = {}
    for item in outputs:
        require_exact_keys(item, {"name", "path", "sha256"}, "Foundation build output")
        output = _relative(root, item["path"], "Foundation build output")
        if output.parent != build_directory or not output.is_file() or sha256_file(output) != _sha(item["sha256"], "Foundation build output"):
            raise RuntimeErrorEvidence("Foundation build output identity differs")
        hashes[item["name"]] = item["sha256"]
    return build_directory, hashes


def validate_dependencies(path: pathlib.Path, profile: dict[str, Any], candidate_commit: str, root: pathlib.Path,
                          candidate_source_root: pathlib.Path, recorded_root: pathlib.Path | None = None,
                          recorded_source_root: pathlib.Path | None = None) -> tuple[bool, list[dict[str, str]], dict[str, pathlib.Path]]:
    inventory = read_json(path)
    require_exact_keys(inventory, {"schema_version", "kind", "candidate_commit", "cells"}, "Foundation dependency inventory")
    if inventory["schema_version"] != 4 or inventory["kind"] != "foundation-runtime-dependencies" or inventory["candidate_commit"] != candidate_commit:
        raise RuntimeErrorEvidence("Foundation dependency inventory identity differs")
    cells = inventory["cells"]
    if not isinstance(cells, list) or {row.get("name") for row in cells if isinstance(row, dict)} != set(profile["configurations"]) or len(cells) != 4:
        raise RuntimeErrorEvidence("Foundation dependency cell matrix differs")
    findings: list[dict[str, str]] = []; build_directories: dict[str, pathlib.Path] = {}; required = set(profile["required_executables"])
    for cell in cells:
        require_exact_keys(cell, {"name", "build", "executables"}, "Foundation dependency cell")
        build_directory, built_hashes = _build_provenance(cell["build"], profile, candidate_commit, root, candidate_source_root,
                                                           recorded_root, recorded_source_root)
        build_directories[cell["name"]] = build_directory
        executables = cell["executables"]
        if not isinstance(executables, list) or {row.get("name") for row in executables if isinstance(row, dict)} != required or len(executables) != len(required):
            raise RuntimeErrorEvidence("Foundation executable dependency matrix differs")
        allowed = set(profile["allowed_runtime_dependencies"][profile["configurations"][cell["name"]]])
        for executable in executables:
            require_exact_keys(executable, {"name", "path", "sha256", "ldd", "execution"}, "Foundation executable dependency")
            executable_path = _relative(root, executable["path"], "Foundation executable")
            if (executable_path.parent != build_directory or not executable_path.is_file() or
                    sha256_file(executable_path) != _sha(executable["sha256"], "Foundation executable") or
                    built_hashes.get(executable["name"]) != executable["sha256"]):
                raise RuntimeErrorEvidence("Foundation executable identity differs")
            ldd_path = _hash_bound_file(root, executable["ldd"], "Foundation executable ldd")
            ldd_argv = executable["execution"].get("argv", []) if isinstance(executable.get("execution"), dict) else []
            if (not isinstance(ldd_argv, list) or len(ldd_argv) != 2 or pathlib.Path(ldd_argv[0]).name != "ldd" or
                    ldd_argv[1] != str((recorded_root or root).resolve() / pathlib.PurePosixPath(executable["path"]))):
                raise RuntimeErrorEvidence("Foundation executable ldd command differs")
            _execution_record(root, executable["execution"], ldd_argv, "Foundation executable ldd")
            if _hash_bound_file(root, executable["execution"]["stdout"], "Foundation executable ldd stdout") != ldd_path:
                raise RuntimeErrorEvidence("Foundation executable ldd output differs")
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
    return not findings, findings, build_directories


def _contract_names(discovery: pathlib.Path) -> set[str]:
    try:
        data = json.loads(discovery.read_text(encoding="utf-8")); tests = data.get("tests")
    except (OSError, json.JSONDecodeError):
        tests = None
    if isinstance(tests, list):
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
    labels: set[str] = set(); names = set()
    for line in discovery.read_text(encoding="utf-8").splitlines():
        label_match = re.fullmatch(r"Labels:\s*(.*)", line)
        test_match = re.fullmatch(r"\s*Test\s+#\d+:\s*(\S+)\s*", line)
        if label_match:
            labels = {label for label in re.split(r"[;\s]+", label_match.group(1)) if label}
        elif test_match:
            if "contract" in labels:
                names.add(test_match.group(1))
            labels = set()
    if not names:
        raise RuntimeErrorEvidence("Foundation CTest discovery differs")
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


def _execution_record(root: pathlib.Path, row: Any, expected_argv: list[str], context: str) -> None:
    """Validate the completed, hash-bound command which emitted raw evidence."""
    if not isinstance(row, dict):
        raise RuntimeErrorEvidence(f"{context} execution record differs")
    require_exact_keys(row, {"argv", "exit_code", "pid", "started_utc", "finished_utc", "stdout", "stderr"},
                       f"{context} execution record")
    if row["argv"] != expected_argv or row["exit_code"] != 0 or not isinstance(row["pid"], int) or row["pid"] <= 0:
        raise RuntimeErrorEvidence(f"{context} execution identity differs")
    try:
        started = datetime.datetime.fromisoformat(row["started_utc"].replace("Z", "+00:00"))
        finished = datetime.datetime.fromisoformat(row["finished_utc"].replace("Z", "+00:00"))
    except (AttributeError, ValueError) as error:
        raise RuntimeErrorEvidence(f"{context} execution timestamp differs") from error
    if started.tzinfo is None or finished.tzinfo is None or finished < started:
        raise RuntimeErrorEvidence(f"{context} execution timestamp differs")
    _hash_bound_file(root, row["stdout"], f"{context} stdout")
    _hash_bound_file(root, row["stderr"], f"{context} stderr")


def validate_contract_tests(path: pathlib.Path, profile: dict[str, Any], candidate_commit: str, root: pathlib.Path,
                            build_directories: dict[str, pathlib.Path], recorded_root: pathlib.Path | None = None) -> tuple[bool, list[dict[str, Any]]]:
    inventory = read_json(path)
    require_exact_keys(inventory, {"schema_version", "kind", "candidate_commit", "cells"}, "Foundation contract-test inventory")
    if inventory["schema_version"] != 4 or inventory["kind"] != "foundation-contract-tests" or inventory["candidate_commit"] != candidate_commit:
        raise RuntimeErrorEvidence("Foundation contract-test inventory identity differs")
    cells = inventory["cells"]; expected = set(profile["configurations"])
    if not isinstance(cells, list) or {row.get("name") for row in cells if isinstance(row, dict)} != expected or len(cells) != len(expected):
        raise RuntimeErrorEvidence("Foundation contract-test cell matrix differs")
    required = set(profile["required_contract_tests"]); findings: list[dict[str, Any]] = []
    for cell in cells:
        require_exact_keys(cell, {"name", "command", "discovery", "discovery_execution", "junit", "exit_code", "execution"}, "Foundation contract-test cell")
        command = cell["command"]
        if not isinstance(command, list) or not command or any(not isinstance(part, str) or not part for part in command) or pathlib.Path(command[0]).name != "ctest" or "-L" not in command or "contract" not in command or "--output-junit" not in command:
            raise RuntimeErrorEvidence("Foundation contract-test command differs")
        _execution_record(root, cell["execution"], command, "Foundation CTest")
        build_directory = build_directories.get(cell["name"])
        if build_directory is None or "--test-dir" not in command or command.index("--test-dir") + 1 >= len(command):
            raise RuntimeErrorEvidence("Foundation CTest build provenance differs")
        command_build = (recorded_root or root).resolve() / build_directory.relative_to(root.resolve())
        if command[command.index("--test-dir") + 1] != str(command_build):
            raise RuntimeErrorEvidence("Foundation CTest build provenance differs")
        discovery_command = ["ctest", "--test-dir", str(command_build), "-N", "-V"]
        _execution_record(root, cell["discovery_execution"], discovery_command, "Foundation CTest discovery")
        discovery_path = _hash_bound_file(root, cell["discovery"], "Foundation CTest discovery")
        if sha256_file(_hash_bound_file(root, cell["discovery_execution"]["stdout"], "Foundation CTest discovery stdout")) != sha256_file(discovery_path):
            raise RuntimeErrorEvidence("Foundation CTest discovery output differs")
        discovered = _contract_names(discovery_path)
        junit = _hash_bound_file(root, cell["junit"], "Foundation CTest JUnit")
        output_index = command.index("--output-junit") + 1
        recorded_junit = (recorded_root or root).resolve() / junit.relative_to(root.resolve())
        if output_index >= len(command) or pathlib.Path(command[output_index]).resolve() != recorded_junit:
            raise RuntimeErrorEvidence("Foundation CTest output differs")
        passed, failed = _junit_passed(junit)
        missing, not_passed = sorted(required - discovered), sorted(required - passed)
        if cell["exit_code"] != 0 or failed or missing or not_passed or passed != discovered:
            findings.append({"cell": cell["name"], "classification": "REGRESSION", "missing": missing,
                             "not_passed": not_passed, "failed": sorted(failed), "exit_code": cell["exit_code"]})
    return not findings, findings


def collect_observed_build_and_contract_evidence(profile_path: pathlib.Path, source_root: pathlib.Path,
                                                 candidate_commit: str, output_root: pathlib.Path) -> dict[str, pathlib.Path]:
    """Collect real per-cell build, CTest, and `ldd` evidence for a later admission.

    This helper deliberately has no CLI entrypoint: collecting evidence is not
    the formal Foundation regression.  The formal runner may call it only after
    its separate PREPARED gate.  The focused contract uses a disposable CMake
    project to prove the recorded evidence comes from executed commands.
    """
    profile = validate_profile(profile_path)
    if output_root.exists() and any(output_root.iterdir()):
        raise RuntimeErrorEvidence("Foundation observed evidence root is not empty")
    output_root.mkdir(parents=True, exist_ok=True)
    ldd = shutil.which("ldd")
    if ldd is None:
        raise RuntimeErrorEvidence("Foundation ldd is unavailable")
    dependency_cells: list[dict[str, Any]] = []; contract_cells: list[dict[str, Any]] = []
    for name, family in profile["configurations"].items():
        build_directory = output_root / "build" / name
        logs = output_root / "logs" / name
        source_before = build_source_state(source_root)
        if source_before["commit"] != candidate_commit:
            raise RuntimeErrorEvidence("Foundation observed build candidate differs")
        compiler = "g++-13" if family == "gcc" else "clang++-18"
        configure_argv = ["cmake", "-S", str(source_root.resolve()), "-B", str(build_directory), "-G", "Ninja",
                          f"-DCMAKE_CXX_COMPILER={compiler}", f"-DCMAKE_BUILD_TYPE={'Release' if name.endswith('release') else 'Debug'}",
                          "-DBUILD_TESTING=ON", "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"]
        if family == "clang-libcxx":
            configure_argv.append("-DAPMESH_USE_LIBCXX=ON")
        configure = run_command(configure_argv, source_root, logs, "configure", 120)
        build_argv = ["cmake", "--build", str(build_directory), "--target", *profile["required_executables"]]
        build = run_command(build_argv, source_root, logs, "build", 120)
        if any(record["exit_code"] != 0 or record["timed_out"] or record["launch_error"] is not None for record in (configure, build)):
            raise RuntimeErrorEvidence(f"Foundation observed build failed: {name}")
        outputs = []
        executable_rows = []
        for executable_name in profile["required_executables"]:
            executable = build_directory / executable_name
            if not executable.is_file():
                raise RuntimeErrorEvidence(f"Foundation observed executable is absent: {executable_name}")
            identity = {"name": executable_name, "path": relative_path(output_root, executable), "sha256": sha256_file(executable)}
            outputs.append(identity)
            ldd_record = run_command(["ldd", str(executable)], source_root, logs, f"ldd-{executable_name}", 30)
            if ldd_record["exit_code"] != 0 or ldd_record["timed_out"] or ldd_record["launch_error"] is not None:
                raise RuntimeErrorEvidence(f"Foundation observed ldd failed: {executable_name}")
            ldd_output = logs.parent / ldd_record["stdout"]["path"]
            executable_rows.append({**identity, "ldd": {"path": relative_path(output_root, ldd_output), "sha256": sha256_file(ldd_output)},
                                    "execution": _observed_record(ldd_record, logs.parent, output_root)})
        discovery_record = run_command(["ctest", "--test-dir", str(build_directory), "-N", "-V"], source_root, logs, "ctest-discovery", 30)
        discovery = output_root / "ctest" / f"{name}-discovery.txt"; discovery.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(logs.parent / discovery_record["stdout"]["path"], discovery)
        junit = output_root / "ctest" / f"{name}-result.xml"
        ctest_argv = ["ctest", "--test-dir", str(build_directory), "-L", "contract", "--output-junit", str(junit)]
        ctest_record = run_command(ctest_argv, source_root, logs, "ctest-contract", 60)
        if any(record["exit_code"] != 0 or record["timed_out"] or record["launch_error"] is not None for record in (discovery_record, ctest_record)) or not junit.is_file():
            raise RuntimeErrorEvidence(f"Foundation observed CTest failed: {name}")
        source_after = build_source_state(source_root)
        if source_before != source_after:
            raise RuntimeErrorEvidence("Foundation observed source changed during collection")
        dependency_cells.append({"name": name, "build": {"candidate_commit": candidate_commit, "source_root": str(source_root.resolve()),
                                 "source_before": source_before, "source_after": source_after, "build_directory": relative_path(output_root, build_directory), "configure": _observed_record(configure, logs.parent, output_root),
                                 "build": _observed_record(build, logs.parent, output_root), "outputs": outputs}, "executables": executable_rows})
        contract_cells.append({"name": name, "command": ctest_argv, "discovery": {"path": relative_path(output_root, discovery), "sha256": sha256_file(discovery)},
                               "discovery_execution": _observed_record(discovery_record, logs.parent, output_root), "junit": {"path": relative_path(output_root, junit), "sha256": sha256_file(junit)},
                               "exit_code": ctest_record["exit_code"], "execution": _observed_record(ctest_record, logs.parent, output_root)})
    dependencies = output_root / "dependencies.json"; contracts = output_root / "contract-tests.json"
    write_json(dependencies, {"schema_version": 4, "kind": "foundation-runtime-dependencies", "candidate_commit": candidate_commit, "cells": dependency_cells})
    write_json(contracts, {"schema_version": 4, "kind": "foundation-contract-tests", "candidate_commit": candidate_commit, "cells": contract_cells})
    return {"dependencies": dependencies, "contract_tests": contracts}


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


def _transitive_input_paths(root: pathlib.Path, inputs: dict[str, pathlib.Path]) -> list[pathlib.Path]:
    """Return every raw artifact consumed by the two evidence inventories."""
    paths = set(inputs.values())
    dependencies = read_json(inputs["dependencies"])
    dependency_root = inputs["dependencies"].parent
    for cell in dependencies["cells"]:
        build = cell["build"]
        for stage in ("configure", "build"):
            for stream in ("stdout", "stderr"):
                paths.add(_hash_bound_file(dependency_root, build[stage][stream], f"Foundation retained {stage} {stream}"))
        for output in build["outputs"]:
            paths.add(_relative(dependency_root, output["path"], "Foundation retained build output"))
        for executable in cell["executables"]:
            paths.add(_relative(dependency_root, executable["path"], "Foundation retained executable"))
            paths.add(_hash_bound_file(dependency_root, executable["ldd"], "Foundation retained ldd"))
            for stream in ("stdout", "stderr"):
                paths.add(_hash_bound_file(dependency_root, executable["execution"][stream], f"Foundation retained ldd {stream}"))
    contracts = read_json(inputs["contract_tests"])
    contract_root = inputs["contract_tests"].parent
    for cell in contracts["cells"]:
        for key in ("discovery", "junit"):
            paths.add(_hash_bound_file(contract_root, cell[key], f"Foundation retained CTest {key}"))
        for record_name in ("discovery_execution", "execution"):
            for stream in ("stdout", "stderr"):
                paths.add(_hash_bound_file(contract_root, cell[record_name][stream], f"Foundation retained CTest {record_name} {stream}"))
    return sorted(paths)


def _copy_transitive(source_root: pathlib.Path, paths: list[pathlib.Path], destination: pathlib.Path,
                     retention_root: pathlib.Path) -> list[dict[str, str]]:
    copied: list[dict[str, str]] = []
    for source in paths:
        target = destination / relative_path(source_root, source)
        target.parent.mkdir(parents=True, exist_ok=True); shutil.copy2(source, target)
        copied.append({"path": relative_path(retention_root, target), "sha256": sha256_file(target)})
    return copied


def _seal_output(output: pathlib.Path, profile_path: pathlib.Path, rec_profile: pathlib.Path, candidate_retention: pathlib.Path,
                 candidate_package: pathlib.Path, inputs_manifest: pathlib.Path, input_root: pathlib.Path,
                 inputs: dict[str, pathlib.Path], candidate_commit: str) -> None:
    inputs_root = output / "inputs"; inputs_root.mkdir(); copied: dict[str, dict[str, str]] = {}
    manifest_target = inputs_root / "admission-input-manifest.json"; shutil.copy2(inputs_manifest, manifest_target)
    for role, path in inputs.items():
        suffix = path.suffix or ".bin"; target = inputs_root / f"{role}{suffix}"
        shutil.copy2(path, target); copied[role] = {"path": relative_path(output, target), "sha256": sha256_file(target)}
    profile_target = inputs_root / "foundation-profile.json"; shutil.copy2(profile_path, profile_target)
    rec_profile_target = inputs_root / "reproducible-experiment-profile.json"; shutil.copy2(rec_profile, rec_profile_target)
    package_target = inputs_root / "candidate-rec-package"; shutil.copytree(candidate_package, package_target)
    transitive = _copy_transitive(input_root, _transitive_input_paths(input_root, inputs), inputs_root / "raw", output)
    write_json(output / "foundation-evidence-manifest.json", {"schema_version": 4, "kind": "foundation-end-to-end-evidence-binding", "candidate_commit": candidate_commit,
        "profile_sha256": sha256_file(profile_path), "candidate_retention_manifest_sha256": sha256_file(candidate_retention),
        "input_root": str(input_root.resolve()),
        "profile": {"path": relative_path(output, profile_target), "sha256": sha256_file(profile_target)},
        "rec_profile": {"path": relative_path(output, rec_profile_target), "sha256": sha256_file(rec_profile_target)},
        "candidate_rec_package": {"path": relative_path(output, package_target / "retention-manifest.json"), "sha256": sha256_file(package_target / "retention-manifest.json")},
        "input_manifest": {"path": relative_path(output, manifest_target), "sha256": sha256_file(manifest_target)},
        "artifacts": copied, "transitive_artifacts": transitive})
    files = sorted(path for path in output.rglob("*") if path.is_file())
    write_json(output / "artifact-inventory.json", {"schema_version": 1, "kind": "foundation-end-to-end-artifact-inventory",
        "artifacts": [{"path": relative_path(output, path), "sha256": sha256_file(path)} for path in files]})
    sealed = sorted(path for path in output.rglob("*") if path.is_file())
    write_json(output / "retention-manifest.json", {"schema_version": 1, "kind": "foundation-end-to-end-retention", "candidate_commit": candidate_commit,
        "files": [{"path": relative_path(output, path), "sha256": sha256_file(path), "size": path.stat().st_size} for path in sealed]})


def verify_foundation_retention(output: pathlib.Path, profile_path: pathlib.Path, rec_profile: pathlib.Path,
                                candidate_retention: pathlib.Path, candidate_commit: str,
                                candidate_source_root: pathlib.Path, recorded_source_root: pathlib.Path | None = None) -> None:
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
    require_exact_keys(binding, {"schema_version", "kind", "candidate_commit", "profile_sha256", "candidate_retention_manifest_sha256", "input_root", "profile", "rec_profile", "candidate_rec_package", "input_manifest", "artifacts", "transitive_artifacts"}, "Foundation evidence binding")
    if (binding["schema_version"] != 4 or binding["kind"] != "foundation-end-to-end-evidence-binding" or binding["candidate_commit"] != candidate_commit or
            binding["profile_sha256"] != sha256_file(profile_path) or binding["candidate_retention_manifest_sha256"] != sha256_file(candidate_retention)):
        raise RuntimeErrorEvidence("Foundation evidence binding differs")
    if not isinstance(binding["input_root"], str) or not pathlib.Path(binding["input_root"]).is_absolute():
        raise RuntimeErrorEvidence("Foundation retained input root differs")
    _hash_bound_file(output, binding["profile"], "Foundation retained profile")
    retained_rec_profile = _hash_bound_file(output, binding["rec_profile"], "Foundation retained REC profile")
    if sha256_file(retained_rec_profile) != sha256_file(rec_profile):
        raise RuntimeErrorEvidence("Foundation retained REC profile differs")
    _hash_bound_file(output, binding["candidate_rec_package"], "Foundation retained REC package")
    _hash_bound_file(output, binding["input_manifest"], "Foundation retained input manifest")
    if not isinstance(binding["artifacts"], dict) or set(binding["artifacts"]) != {"publication", "authorities", "dependencies", "contract_tests"}:
        raise RuntimeErrorEvidence("Foundation retained input matrix differs")
    for role, row in binding["artifacts"].items():
        _hash_bound_file(output, row, f"Foundation retained {role}")
    if not isinstance(binding["transitive_artifacts"], list) or not binding["transitive_artifacts"]:
        raise RuntimeErrorEvidence("Foundation retained transitive artifacts differ")
    for row in binding["transitive_artifacts"]:
        _hash_bound_file(output, row, "Foundation retained transitive artifact")
    # Retention is only meaningful if the candidate can be reconstructed in a
    # clean detached worktree, rather than being validated through the mutable
    # checkout that produced the evidence.
    with tempfile.TemporaryDirectory(prefix="apmesh-core-foundation-retention-") as temporary:
        detached = pathlib.Path(temporary) / "candidate"
        created = subprocess.run(["git", "worktree", "add", "--detach", str(detached), candidate_commit], cwd=candidate_source_root,
                                 capture_output=True, text=True, check=False)
        if created.returncode != 0:
            raise RuntimeErrorEvidence("Foundation detached retention worktree could not be created")
        try:
            head = subprocess.run(["git", "rev-parse", "HEAD"], cwd=detached, capture_output=True, text=True, check=False)
            status = subprocess.run(["git", "status", "--porcelain"], cwd=detached, capture_output=True, text=True, check=False)
            if head.returncode != 0 or head.stdout.strip() != candidate_commit or status.returncode != 0 or status.stdout.strip():
                raise RuntimeErrorEvidence("Foundation detached retention worktree differs")
            retained_profile = validate_profile(_hash_bound_file(output, binding["profile"], "Foundation retained profile"))
            retained_input = read_json(_hash_bound_file(output, binding["input_manifest"], "Foundation retained input manifest"))
            require_exact_keys(retained_input, {"schema_version", "kind", "candidate_commit", "candidate_retention_manifest_sha256", "profile_sha256", "source_root", "artifacts"},
                               "Foundation retained admission input")
            expected_recorded_source = (recorded_source_root or candidate_source_root).resolve()
            if retained_input["candidate_commit"] != candidate_commit or retained_input["source_root"] != str(expected_recorded_source):
                raise RuntimeErrorEvidence("Foundation retained admission identity differs")
            raw_root = output / "inputs" / "raw"
            artifacts = retained_input.get("artifacts")
            if not isinstance(artifacts, dict) or set(artifacts) != {"publication", "authorities", "dependencies", "contract_tests"}:
                raise RuntimeErrorEvidence("Foundation retained admission artifacts differ")
            retained = {role: _hash_bound_file(raw_root, row, f"Foundation retained raw {role}") for role, row in artifacts.items()}
            retained_package = _hash_bound_file(output, binding["candidate_rec_package"], "Foundation retained REC package").parent
            terminal = read_json(retained_package / "terminal-manifest.json")
            if not validate_authorities(retained["authorities"], retained_profile, candidate_commit, detached, terminal):
                raise RuntimeErrorEvidence("Foundation retained authorities are not qualified")
            recorded_input_root = pathlib.Path(binding["input_root"])
            dependency_recorded_root = recorded_input_root / pathlib.PurePosixPath(artifacts["dependencies"]["path"]).parent
            contract_recorded_root = recorded_input_root / pathlib.PurePosixPath(artifacts["contract_tests"]["path"]).parent
            dependencies_ok, dependency_findings, build_directories = validate_dependencies(retained["dependencies"], retained_profile,
                                                                                              candidate_commit, retained["dependencies"].parent, detached,
                                                                                              dependency_recorded_root, candidate_source_root)
            contract_ok, contract_findings = validate_contract_tests(retained["contract_tests"], retained_profile, candidate_commit,
                                                                       retained["contract_tests"].parent, build_directories, contract_recorded_root)
            if not dependencies_ok or dependency_findings or not contract_ok or contract_findings:
                raise RuntimeErrorEvidence("Foundation retained semantic evidence differs")
            # The retained copy is sealed below ``inputs/``.  Recreate it at a
            # canonical evidence path only after the clean-worktree build
            # evidence was checked; the REC verifier intentionally rejects
            # non-canonical destinations.
            semantic_package = (detached / "evidence" / "foundation" / "reproducible-experiment-contract" /
                                "retained-rec-semantic-check")
            shutil.copytree(retained_package, semantic_package)
            verify_retained_package(semantic_package, retained_rec_profile)
        finally:
            removed = subprocess.run(["git", "worktree", "remove", "--force", str(detached)], cwd=candidate_source_root,
                                     capture_output=True, text=True, check=False)
            if removed.returncode != 0:
                raise RuntimeErrorEvidence("Foundation detached retention worktree could not be removed")


def _write_outputs(output: pathlib.Path, summary: dict[str, Any], profile_path: pathlib.Path, rec_profile: pathlib.Path,
                   candidate_retention: pathlib.Path, candidate_package: pathlib.Path, inputs_manifest: pathlib.Path,
                   input_root: pathlib.Path, inputs: dict[str, pathlib.Path], candidate_source_root: pathlib.Path,
                   recorded_source_root: pathlib.Path | None = None) -> bool:
    if output.exists() and any(output.iterdir()):
        raise RuntimeErrorEvidence("Foundation qualifier output root is not empty")
    output.mkdir(parents=True, exist_ok=True)
    _write_text_outputs(output, summary)
    _seal_output(output, profile_path, rec_profile, candidate_retention, candidate_package, inputs_manifest, input_root, inputs, summary["candidate_commit"])
    verify_foundation_retention(output, profile_path, rec_profile, candidate_retention, summary["candidate_commit"], candidate_source_root,
                                recorded_source_root)
    return True


def qualify(profile_path: pathlib.Path, rec_profile: pathlib.Path, baseline_package: pathlib.Path,
            candidate_package: pathlib.Path, inputs_manifest: pathlib.Path, output: pathlib.Path,
            verification_source_root: pathlib.Path | None = None) -> dict[str, Any]:
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
    if candidate_commit == baseline_retention["candidate_commit"]:
        raise RuntimeErrorEvidence("historical Foundation baseline cannot be the current candidate")
    candidate_source = read_json(candidate_package / "control" / "prepared-manifest.json").get("candidate", {}).get("source_root")
    if not isinstance(candidate_source, str) or not candidate_source:
        raise RuntimeErrorEvidence("Foundation candidate source differs")
    recorded_source_root = pathlib.Path(candidate_source)
    source_root = (verification_source_root or recorded_source_root).resolve()
    input_root, inputs = _load_inputs(inputs_manifest, profile_path, candidate_package / "retention-manifest.json", candidate_commit, recorded_source_root)
    publication_ok = validate_publication(inputs["publication"], candidate_commit, source_root)
    authorities_ok = validate_authorities(inputs["authorities"], profile, candidate_commit, source_root, candidate_terminal)
    dependencies_ok, dependency_findings, build_directories = validate_dependencies(inputs["dependencies"], profile, candidate_commit,
                                                                                     inputs["dependencies"].parent, source_root)
    contract_tests_ok, contract_test_findings = validate_contract_tests(inputs["contract_tests"], profile, candidate_commit,
                                                                         inputs["contract_tests"].parent, build_directories)
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
    if not all(conditions.values()):
        summary["gates"]["FND6"] = BLOCKED
        if output.exists() and any(output.iterdir()):
            raise RuntimeErrorEvidence("Foundation qualifier output root is not empty")
        output.mkdir(parents=True, exist_ok=True)
        _write_text_outputs(output, summary)
        return summary
    retention_verified = _write_outputs(output, summary, profile_path, rec_profile, candidate_package / "retention-manifest.json", candidate_package,
                                        inputs_manifest, input_root, inputs, source_root, recorded_source_root)
    conditions["FND6"] = retention_verified
    if not all(conditions.values()):
        summary["state"] = BLOCKED
    return summary


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--preflight", action="store_true")
    parser.add_argument("--profile", required=True); parser.add_argument("--rec-profile", required=True)
    parser.add_argument("--baseline-package", required=True); parser.add_argument("--candidate-package")
    parser.add_argument("--inputs-manifest"); parser.add_argument("--output")
    parser.add_argument("--source-root"); parser.add_argument("--control-root"); parser.add_argument("--evidence-root")
    return parser.parse_args()


def main() -> int:
    arguments = parse_arguments()
    try:
        if arguments.preflight:
            required = {"source root": arguments.source_root, "control root": arguments.control_root,
                        "evidence root": arguments.evidence_root}
            missing = [name for name, value in required.items() if value is None]
            if missing:
                raise RuntimeErrorEvidence(f"Foundation preflight argument is absent: {', '.join(missing)}")
            summary = preflight(pathlib.Path(arguments.profile), pathlib.Path(arguments.rec_profile),
                                pathlib.Path(arguments.baseline_package), pathlib.Path(arguments.source_root),
                                pathlib.Path(arguments.control_root), pathlib.Path(arguments.evidence_root))
            print(json.dumps(summary, sort_keys=True))
            return 0 if summary["state"] == PENDING else 1
        required = {"candidate package": arguments.candidate_package, "inputs manifest": arguments.inputs_manifest,
                    "output": arguments.output}
        missing = [name for name, value in required.items() if value is None]
        if missing:
            raise RuntimeErrorEvidence(f"Foundation qualification argument is absent: {', '.join(missing)}")
        summary = qualify(pathlib.Path(arguments.profile), pathlib.Path(arguments.rec_profile), pathlib.Path(arguments.baseline_package),
                          pathlib.Path(arguments.candidate_package), pathlib.Path(arguments.inputs_manifest), pathlib.Path(arguments.output))
        return 0 if summary["state"] == PENDING else 1
    except (OSError, RuntimeErrorEvidence) as error:
        print(error, file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
