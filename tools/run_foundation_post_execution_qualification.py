#!/usr/bin/env python3
"""Assemble report-only Foundation evidence from one completed REC campaign.

This is deliberately an operational bridge, not a new Foundation experiment.
It consumes an already executed REC terminal manifest, creates the candidate
retention package and revision-bound admission inputs, then invokes the
existing Foundation qualifier from a clean checkout of that exact revision.
"""

from __future__ import annotations

import argparse
import importlib.util
import json
import pathlib
import subprocess
import sys
from types import ModuleType
from typing import Any


def _load_module(name: str, path: pathlib.Path) -> ModuleType:
    tools_root = str(path.parent)
    if tools_root not in sys.path:
        sys.path.insert(0, tools_root)
    specification = importlib.util.spec_from_file_location(name, path)
    if specification is None or specification.loader is None:
        raise RuntimeError(f"cannot load qualified tool: {path}")
    module = importlib.util.module_from_spec(specification)
    sys.modules[name] = module
    specification.loader.exec_module(module)
    return module


def _require_empty_destination(path: pathlib.Path, context: str) -> None:
    if path.exists():
        if path.is_dir() and not any(path.iterdir()):
            return
        raise RuntimeError(f"{context} already exists or is not empty: {path}")


def _git_value(source_root: pathlib.Path, command: list[str], context: str) -> str:
    completed = subprocess.run(command, cwd=source_root, capture_output=True, text=True, check=False)
    if completed.returncode != 0:
        raise RuntimeError(f"Foundation verification checkout command failed: {context}")
    return completed.stdout.strip()


def _create_clean_checkout(recorded_source: pathlib.Path, branch: str, candidate_commit: str,
                           destination: pathlib.Path) -> pathlib.Path:
    _require_empty_destination(destination, "Foundation verification checkout")
    cloned = subprocess.run(["git", "clone", "--branch", branch, "--single-branch", str(recorded_source), str(destination)],
                            capture_output=True, text=True, check=False)
    if cloned.returncode != 0:
        raise RuntimeError("Foundation verification checkout could not be created")
    head = _git_value(destination, ["git", "rev-parse", "HEAD"], "head")
    status = _git_value(destination, ["git", "status", "--porcelain"], "status")
    upstream = _git_value(destination, ["git", "rev-parse", "@{u}"], "upstream")
    if head != candidate_commit or upstream != candidate_commit or status:
        raise RuntimeError("Foundation verification checkout identity differs")
    return destination


def _artifact(root: pathlib.Path, path: pathlib.Path, runtime: ModuleType) -> dict[str, str]:
    return {"path": runtime.relative_path(root, path), "sha256": runtime.sha256_file(path)}


def execute(arguments: argparse.Namespace) -> dict[str, Any]:
    evidence_root = pathlib.Path(arguments.evidence_root).resolve()
    control_root = pathlib.Path(arguments.control_root).resolve()
    retained_package = pathlib.Path(arguments.retained_package).resolve()
    admission_root = pathlib.Path(arguments.admission_root).resolve()
    output = pathlib.Path(arguments.output).resolve()
    verification_root = pathlib.Path(arguments.verification_source_root).resolve()

    for path, context in ((retained_package, "candidate REC retention package"),
                          (admission_root, "Foundation admission root"),
                          (output, "Foundation qualification output"),
                          (verification_root, "Foundation verification checkout")):
        _require_empty_destination(path, context)

    prepared_path = control_root / "prepared-manifest.json"
    terminal_path = evidence_root / "terminal-manifest.json"
    if not prepared_path.is_file() or not terminal_path.is_file():
        raise RuntimeError("completed REC control or terminal manifest is absent")
    prepared = json.loads(prepared_path.read_text(encoding="utf-8"))
    terminal = json.loads(terminal_path.read_text(encoding="utf-8"))
    candidate = prepared.get("candidate")
    if not isinstance(candidate, dict):
        raise RuntimeError("completed REC candidate record differs")
    candidate_commit = candidate.get("commit")
    recorded_source_value = candidate.get("source_root")
    if (not isinstance(candidate_commit, str) or len(candidate_commit) != 40 or
            not isinstance(recorded_source_value, str) or not recorded_source_value):
        raise RuntimeError("completed REC candidate identity differs")
    if terminal.get("state") != "EXECUTED_PENDING_AUDIT" or terminal.get("candidate_commit") != candidate_commit:
        raise RuntimeError("completed REC terminal state differs")

    recorded_source = pathlib.Path(recorded_source_value).resolve()
    if not recorded_source.is_dir():
        raise RuntimeError("completed REC recorded source root is absent")
    branch = _git_value(recorded_source, ["git", "branch", "--show-current"], "branch")
    if not branch:
        raise RuntimeError("completed REC recorded source branch is absent")
    verification_source = _create_clean_checkout(recorded_source, branch, candidate_commit, verification_root)

    foundation = _load_module("qualified_foundation_end_to_end_evidence",
                              verification_source / "tools" / "foundation_end_to_end_evidence.py")
    retention = _load_module("foundation_retention_assembler",
                             pathlib.Path(__file__).with_name("retain_experiment_evidence.py"))
    runtime = _load_module("qualified_experiment_runtime", verification_source / "tools" / "experiment_runtime.py")

    profile_path = verification_source / "experiments" / "profiles" / "foundation_end_to_end.json"
    rec_profile_path = verification_source / "experiments" / "profiles" / "reproducible_experiment_contract.json"
    profile = foundation.validate_profile(profile_path)
    baseline_package = pathlib.Path(profile["accepted_baseline"]["path"])
    if not baseline_package.is_absolute():
        baseline_package = verification_source / baseline_package

    retention.assemble(evidence_root, retained_package, candidate_commit, None, control_root, rec_profile_path,
                       verification_source_root=verification_source)
    retention.verify(retained_package, rec_profile_path)

    observed = foundation.collect_observed_build_and_contract_evidence(profile_path, verification_source,
                                                                         candidate_commit, admission_root)
    publication_path = admission_root / "publication.json"
    authorities_path = admission_root / "authorities.json"
    runtime.write_json(publication_path, foundation.source_identity(verification_source))
    runtime.write_json(authorities_path, {"schema_version": 1, "kind": "foundation-qualified-authorities",
                                          "candidate_commit": candidate_commit,
                                          "authorities": profile["qualified_authorities"]})
    manifest_path = admission_root / "admission-input-manifest.json"
    runtime.write_json(manifest_path, {"schema_version": 1, "kind": "foundation-admission-input-manifest",
                                       "candidate_commit": candidate_commit,
                                       "candidate_retention_manifest_sha256": runtime.sha256_file(retained_package / "retention-manifest.json"),
                                       "profile_sha256": runtime.sha256_file(profile_path),
                                       "source_root": str(recorded_source),
                                       "artifacts": {"publication": _artifact(admission_root, publication_path, runtime),
                                                     "authorities": _artifact(admission_root, authorities_path, runtime),
                                                     "dependencies": _artifact(admission_root, observed["dependencies"], runtime),
                                                     "contract_tests": _artifact(admission_root, observed["contract_tests"], runtime)}})
    summary = foundation.qualify(profile_path, rec_profile_path, baseline_package, retained_package, manifest_path, output,
                                 verification_source_root=verification_source)
    return {"schema_version": 1, "kind": "foundation-post-execution-qualification",
            "candidate_commit": candidate_commit, "state": summary["state"], "gates": summary["gates"],
            "retained_package": str(retained_package), "admission_manifest": str(manifest_path),
            "qualification_output": str(output), "verification_source_root": str(verification_source)}


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--evidence-root", required=True)
    parser.add_argument("--control-root", required=True)
    parser.add_argument("--retained-package", required=True)
    parser.add_argument("--admission-root", required=True)
    parser.add_argument("--output", required=True)
    parser.add_argument("--verification-source-root", required=True)
    return parser.parse_args()


def main() -> int:
    try:
        print(json.dumps(execute(parse_arguments()), sort_keys=True))
        return 0
    except (OSError, RuntimeError, ValueError, json.JSONDecodeError) as error:
        print(error, file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
