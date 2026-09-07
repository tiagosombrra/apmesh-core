#!/usr/bin/env python3
"""Focused durable-retention contract for a semantically valid REC package."""

from __future__ import annotations

import argparse
import importlib.util
import json
import pathlib
import sys
import tempfile


def load_tool(path: pathlib.Path, name: str):
    sys.path.insert(0, str(path.parent))
    spec = importlib.util.spec_from_file_location(name, path)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"cannot load {name}")
    module = importlib.util.module_from_spec(spec); sys.modules[spec.name] = module; spec.loader.exec_module(module)
    return module


def write_bundle(evidence, root: pathlib.Path, profile: dict, cell: str, replay: int, commit: str, prepared_sha: str,
                 profile_sha: str, launch_plan_sha: str) -> None:
    root.mkdir(parents=True); logs = root / "logs"; logs.mkdir()
    (logs / "focused.stdout.log").write_text("focused\n", encoding="utf-8")
    (logs / "focused.stderr.log").write_text("", encoding="utf-8")
    record = {"schema_version": 1, "kind": "experiment-command-record", "id": "focused", "argv": ["tool"],
              "cwd": "/synthetic", "environment_delta": {}, "started_utc": "2026-01-01T00:00:00+00:00",
              "ended_utc": "2026-01-01T00:00:01+00:00", "elapsed_seconds": 1.0, "pid": 7, "timeout_seconds": 10,
              "timed_out": False, "exit_code": 0, "launch_error": None,
              "stdout": {"path": "logs/focused.stdout.log", "sha256": evidence.sha256(logs / "focused.stdout.log")},
              "stderr": {"path": "logs/focused.stderr.log", "sha256": evidence.sha256(logs / "focused.stderr.log")}}
    evidence.write_json(root / "manifest.json", {"schema_version": 1, "kind": "reproducible-experiment-bundle-manifest", "cell": cell,
                                                   "replay": replay, "candidate_commit": commit, "profile_sha256": profile_sha,
                                                   "prepared_manifest_sha256": prepared_sha, "launch_plan_sha256": launch_plan_sha})
    evidence.write_json(root / "certificate.json", {"schema_version": 1, "kind": "numeric-contract-certificate", "stable": True})
    evidence.write_json(root / "environment.json", {"schema_version": 1, "kind": "numeric-contract-environment", "stable": True})
    evidence.write_json(root / "execution-record.json", {"schema_version": 1, "kind": "reproducible-experiment-execution", "cell": cell, "replay": replay, "records": [record]})
    summary = {"schema_version": 1, "kind": "reproducible-experiment-summary", "cell": cell, "replay": replay,
               "gates": {gate: "BUNDLE_VALIDATED" for gate in evidence.EXPECTED_GATES},
               "certificate": {"schema_version": 1, "kind": "numeric-contract-certificate", "stable": True},
               "environment": {"schema_version": 1, "kind": "numeric-contract-environment", "stable": True},
               "artifact_roles": profile["required_artifacts"], "retained_limitations": profile["limitations"]}
    evidence.write_json(root / "summary.json", summary); evidence.write_derived(root, [summary])
    execution = json.loads((root / "execution-record.json").read_text(encoding="utf-8"))
    evidence.write_json(root / "artifact-inventory.json", {"schema_version": 1, "kind": "experiment-artifact-inventory", "artifacts": evidence.artifact_inventory(root, profile, execution)})


def main() -> int:
    parser = argparse.ArgumentParser(); parser.add_argument("--tool", required=True); parser.add_argument("--profile", required=True)
    arguments = parser.parse_args(); tool_path = pathlib.Path(arguments.tool); tool = load_tool(tool_path, "rec_retention")
    evidence = load_tool(tool_path.with_name("reproducible_experiment_evidence.py"), "rec_evidence")
    profile_path = pathlib.Path(arguments.profile); profile = evidence.validate_profile(profile_path)
    with tempfile.TemporaryDirectory() as temporary_directory:
        root = pathlib.Path(temporary_directory); campaign, control = root / "campaign", root / "control"
        destination = root / "evidence" / "foundation" / "rec" / "synthetic"; commit = "a" * 40
        control.mkdir(); profile_sha, launch_plan_sha = evidence.sha256(profile_path), "c" * 64
        prepared = {"state": "PREPARED", "candidate": {"commit": commit}, "profile_sha256": profile_sha, "launch_plan": {"sha256": launch_plan_sha}}
        evidence.write_json(control / "prepared-manifest.json", prepared); prepared_sha = evidence.sha256(control / "prepared-manifest.json")
        evidence.write_json(control / "state.json", {"state": "EXECUTED_PENDING_AUDIT"})
        (control / "state-history.jsonl").write_text('{"state":"PREPARED"}\n{"state":"RUNNING"}\n{"state":"EXECUTED_PENDING_AUDIT"}\n', encoding="utf-8")
        bundles, seals = [], []
        for configuration in profile["configurations"]:
            for replay in (1, 2):
                relative = f"cells/{configuration['name']}/replay-{replay}"; bundle = campaign / relative
                write_bundle(evidence, bundle, profile, configuration["name"], replay, commit, prepared_sha, profile_sha, launch_plan_sha)
                bundles.append({"cell": configuration["name"], "replay": replay, "bundle": relative})
                seals.append({"bundle": relative, "inventory_sha256": evidence.sha256(bundle / "artifact-inventory.json"),
                              "summary_sha256": evidence.sha256(bundle / "summary.json"), "execution_sha256": evidence.sha256(bundle / "execution-record.json")})
        negatives = []
        for fixture, declaration in profile["negative_fixtures"].items():
            response_path = campaign / "negative-fixtures" / fixture / "logs" / f"{fixture}.stdout.log"; response_path.parent.mkdir(parents=True)
            evidence.write_json(response_path, {"schema_version": 1, "kind": "reproducible-experiment-negative-result", "fixture": fixture, "result": "REJECTED", "reason_code": declaration["reason_code"]})
            negatives.append({"fixture": fixture, "expected_reason_code": declaration["reason_code"], "result": "REJECTED", "reason_code": declaration["reason_code"],
                              "record": {"exit_code": 0, "timed_out": False, "launch_error": None},
                              "response": {"path": response_path.relative_to(campaign).as_posix(), "sha256": evidence.sha256(response_path)}})
        prerequisites = {}
        for name, relative in (("numeric_contract", "prerequisites/numeric-contract/manifest.json"), ("architecture_contract", "prerequisites/numeric-contract/architecture/manifest.json")):
            path = campaign / relative; path.parent.mkdir(parents=True); evidence.write_json(path, {"state": "EXECUTED_PENDING_AUDIT", "candidate": {"commit": commit}, "execution": {"records": ["synthetic"]}})
            prerequisites[name] = {"manifest": relative, "manifest_sha256": evidence.sha256(path), "state": "EXECUTED_PENDING_AUDIT", "candidate_commit": commit}
        replay_comparisons = []
        cell_summaries, provenance = {}, {}
        for configuration in profile["configurations"]:
            cell = configuration["name"]; roots = [campaign / "cells" / cell / f"replay-{replay}" for replay in (1, 2)]
            replay_comparisons.append(evidence.compare_replays(roots, profile))
            cell_summaries[cell] = evidence.validate_bundle(roots[0], profile)
            provenance[cell] = json.loads((roots[0] / "execution-record.json").read_text(encoding="utf-8"))
        terminal = {"schema_version": 1, "kind": "reproducible-experiment-terminal-manifest", "state": "EXECUTED_PENDING_AUDIT",
                    "prepared_manifest_sha256": prepared_sha, "candidate_commit": commit, "bundles": bundles, "bundle_seals": seals,
                    "replay_comparisons": replay_comparisons, "cross_configuration": evidence.compare_configurations(cell_summaries, profile, provenance),
                    "negative_fixtures": negatives, "prerequisites": prerequisites,
                    "gate_results": {gate: "EVIDENCE_COLLECTED_PENDING_AUDIT" for gate in evidence.EXPECTED_GATES}}
        evidence.write_json(campaign / "terminal-manifest.json", terminal)
        (campaign / "build" / "CMakeFiles").mkdir(parents=True); (campaign / "build" / "CMakeFiles" / "object.o").write_bytes(b"object")
        tool.assemble(campaign, destination, commit, "archive-sha", control, profile_path)
        tool.verify(destination, profile_path)
        retained = json.loads((destination / "retention-manifest.json").read_text(encoding="utf-8"))
        if not retained["files"][0]["source_path"].startswith(str(campaign)) or (destination / "build").exists():
            raise RuntimeError("retention package did not preserve evidence policy")
        (destination / "cells" / "gcc-debug" / "replay-1" / "summary.json").write_text("tampered\n", encoding="utf-8")
        try:
            tool.verify(destination, profile_path)
        except tool.RuntimeErrorEvidence:
            pass
        else:
            raise RuntimeError("retention verifier accepted tampered evidence")
        blocked_campaign, blocked_control = root / "blocked-campaign", root / "blocked-control"
        blocked_destination = root / "evidence" / "foundation" / "rec" / "blocked-synthetic"
        blocked_control.mkdir(); evidence.write_json(blocked_control / "prepared-manifest.json", prepared)
        blocked_prepared_sha = evidence.sha256(blocked_control / "prepared-manifest.json")
        evidence.write_json(blocked_control / "state.json", {"state": "BLOCKED"})
        (blocked_control / "state-history.jsonl").write_text('{"state":"PREPARED"}\n{"state":"RUNNING"}\n{"state":"BLOCKED"}\n', encoding="utf-8")
        failure = blocked_campaign / "partial" / "execution-record.json"; failure.parent.mkdir(parents=True); evidence.write_json(failure, {"failure": "synthetic"})
        evidence.write_json(blocked_campaign / "terminal-manifest.json", {"schema_version": 1, "kind": "reproducible-experiment-terminal-manifest", "state": "BLOCKED",
                                                                            "prepared_manifest_sha256": blocked_prepared_sha, "candidate_commit": commit,
                                                                            "attempted_bundles": [], "reason": "synthetic failure",
                                                                            "failure_evidence": [{"path": failure.relative_to(blocked_campaign).as_posix(), "sha256": evidence.sha256(failure)}]})
        tool.assemble(blocked_campaign, blocked_destination, commit, "archive-sha", blocked_control, profile_path)
        tool.verify(blocked_destination, profile_path)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
