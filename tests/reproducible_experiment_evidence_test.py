#!/usr/bin/env python3
"""Focused positive and negative evidence contracts for REC tooling."""

from __future__ import annotations

import argparse
import importlib.util
import json
import pathlib
import shutil
import sys
import tempfile


def load_module(path: pathlib.Path):
    sys.path.insert(0, str(path.parent))
    spec = importlib.util.spec_from_file_location("rec_evidence", path)
    if spec is None or spec.loader is None:
        raise RuntimeError("cannot load REC evidence module")
    module = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module


def write_bundle(module, root: pathlib.Path, profile: dict, cell: str, replay: int, stable: bool = True) -> None:
    if root.exists():
        shutil.rmtree(root)
    root.mkdir(parents=True)
    logs = root / "logs"; logs.mkdir()
    (logs / "focused.stdout.log").write_text("focused output\n", encoding="utf-8")
    (logs / "focused.stderr.log").write_text("", encoding="utf-8")
    records = [{
        "schema_version": 1, "kind": "experiment-command-record", "id": "focused", "argv": ["tool", "argument"],
        "cwd": "/synthetic", "environment_delta": {}, "started_utc": "2026-01-01T00:00:00+00:00",
        "ended_utc": "2026-01-01T00:00:01+00:00", "elapsed_seconds": 1.0, "pid": 17, "timeout_seconds": 10,
        "timed_out": False, "exit_code": 0,
        "stdout": {"path": "logs/focused.stdout.log", "sha256": module.sha256(logs / "focused.stdout.log")},
        "stderr": {"path": "logs/focused.stderr.log", "sha256": module.sha256(logs / "focused.stderr.log")},
    }]
    module.write_json(root / "certificate.json", {"kind": "numeric-contract-certificate", "stable": stable})
    module.write_json(root / "environment.json", {"kind": "numeric-contract-environment", "stable": stable})
    module.write_json(root / "execution-record.json", {"schema_version": 1, "kind": "reproducible-experiment-execution", "cell": cell, "replay": replay, "records": records})
    summary = {"schema_version": 1, "kind": "reproducible-experiment-summary", "cell": cell, "replay": replay,
               "gates": {gate: "EVIDENCE_COLLECTED_PENDING_AUDIT" for gate in module.EXPECTED_GATES},
               "certificate": {"kind": "numeric-contract-certificate", "stable": stable},
               "environment": {"kind": "numeric-contract-environment", "stable": stable},
               "artifact_roles": profile["required_artifacts"], "retained_limitations": profile["limitations"]}
    module.write_json(root / "summary.json", summary)
    module.write_derived(root, [summary])
    execution = json.loads((root / "execution-record.json").read_text(encoding="utf-8"))
    module.write_json(root / "artifact-inventory.json", {"schema_version": 1, "kind": "experiment-artifact-inventory", "artifacts": module.artifact_inventory(root, profile, execution)})


def must_reject(callback, message: str) -> None:
    try:
        callback()
    except module_error_types:
        return
    raise RuntimeError(message)


def main() -> int:
    parser = argparse.ArgumentParser(); parser.add_argument("--tool", required=True); parser.add_argument("--profile", required=True)
    arguments = parser.parse_args(); module = load_module(pathlib.Path(arguments.tool))
    global module_error_types
    module_error_types = (module.EvidenceError,)
    profile = module.validate_profile(pathlib.Path(arguments.profile))
    with tempfile.TemporaryDirectory() as temporary_directory:
        root = pathlib.Path(temporary_directory)
        first, second = root / "one", root / "two"
        write_bundle(module, first, profile, "gcc-debug", 1); write_bundle(module, second, profile, "gcc-debug", 2)
        module.validate_bundle(first, profile)
        if module.compare_replays([first, second], profile)["claim_fields"] != "equivalent":
            raise RuntimeError("REC comparer did not record replay equivalence")
        # N5: required artifact absence.
        (second / "metrics.csv").unlink(); must_reject(lambda: module.validate_bundle(second, profile), "REC accepted missing artifact")
        write_bundle(module, second, profile, "gcc-debug", 2)
        # N6: a changed artifact cannot be excused by a stale inventory.
        (second / "certificate.json").write_text('{"kind":"numeric-contract-certificate","stable":false}\n', encoding="utf-8")
        must_reject(lambda: module.validate_bundle(second, profile), "REC accepted stale inventory")
        write_bundle(module, second, profile, "gcc-debug", 2, stable=False)
        must_reject(lambda: module.compare_replays([first, second], profile), "REC accepted changed claim field")
        # N4: duplicate/unknown fields are rejected by strict profile loading.
        malformed = root / "malformed.json"
        malformed.write_text('{"schema_version":2,"schema_version":2}\n', encoding="utf-8")
        must_reject(lambda: module.validate_profile(malformed), "REC accepted duplicate profile keys")
        cells = {}
        for index, name in enumerate(module.EXPECTED_CONFIGURATION_NAMES):
            bundle = root / "cells" / name / "replay-1"; write_bundle(module, bundle, profile, name, 1)
            cells[name] = module.validate_bundle(bundle, profile)
        if module.compare_configurations(cells, profile)["claim_fields"] != "equivalent":
            raise RuntimeError("REC cross-configuration comparison did not record equivalence")
        cells["clang-release"] = dict(cells["clang-release"]); cells["clang-release"]["certificate"] = {"changed": True}
        must_reject(lambda: module.compare_configurations(cells, profile), "REC accepted cross-configuration claim difference")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
