#!/usr/bin/env python3
"""Assemble or verify a compact canonical experiment-evidence package."""

from __future__ import annotations

import argparse
import json
import pathlib
import shutil
import sys

from experiment_runtime import RuntimeErrorEvidence, read_json, relative_path, retention_manifest, sha256_file, write_json
from reproducible_experiment_evidence import (EvidenceError, validate_blocked_terminal_manifest, validate_profile,
                                              validate_terminal_manifest)


EXCLUDED_PARTS = {"build", "CMakeFiles", ".ninja_deps", ".ninja_log"}


def retained_files(source: pathlib.Path) -> list[pathlib.Path]:
    files: list[pathlib.Path] = []
    for path in sorted(source.rglob("*")):
        if path.is_file() and not any(part in EXCLUDED_PARTS for part in path.relative_to(source).parts):
            files.append(path)
    if not files:
        raise RuntimeErrorEvidence("campaign evidence is empty")
    return files


def _require_canonical_destination(destination: pathlib.Path) -> None:
    if len(destination.parts) < 4 or destination.parents[2].name != "evidence":
        raise RuntimeErrorEvidence("canonical destination must be evidence/<stage>/<contract>/<campaign-id>")


def _validate_campaign(source: pathlib.Path, control_root: pathlib.Path, profile: pathlib.Path,
                       candidate_commit: str) -> None:
    prepared_path = control_root / "prepared-manifest.json"
    prepared = read_json(prepared_path)
    if prepared.get("state") != "PREPARED" or prepared.get("candidate", {}).get("commit") != candidate_commit:
        raise RuntimeErrorEvidence("prepared campaign identity differs")
    profile_data = validate_profile(profile)
    if prepared.get("profile_sha256") != sha256_file(profile):
        raise RuntimeErrorEvidence("prepared campaign profile identity differs")
    state = read_json(control_root / "state.json")
    history_path = control_root / "state-history.jsonl"
    try:
        history = [json.loads(line) for line in history_path.read_text(encoding="utf-8").splitlines() if line]
    except (OSError, UnicodeDecodeError, json.JSONDecodeError) as error:
        raise RuntimeErrorEvidence("campaign lifecycle history is invalid") from error
    if not history or state.get("state") != history[-1].get("state") or [entry.get("state") for entry in history] not in (["PREPARED", "RUNNING", "EXECUTED_PENDING_AUDIT"], ["PREPARED", "RUNNING", "BLOCKED"]):
        raise RuntimeErrorEvidence("campaign lifecycle history differs")
    launch_plan = prepared.get("launch_plan")
    if not isinstance(launch_plan, dict) or not isinstance(launch_plan.get("sha256"), str):
        raise RuntimeErrorEvidence("prepared campaign launch-plan identity differs")
    launch_plan_path = control_root / "launch-plan.json"
    if not launch_plan_path.is_file() or sha256_file(launch_plan_path) != launch_plan["sha256"]:
        raise RuntimeErrorEvidence("prepared campaign launch-plan bytes differ")
    launch_plan_data = read_json(launch_plan_path)
    candidate = prepared.get("candidate")
    envelope = prepared.get("execution_envelope")
    if not isinstance(candidate, dict) or not isinstance(candidate.get("source_root"), str) or not isinstance(envelope, dict) or not isinstance(envelope.get("scratch_root"), str) or not isinstance(prepared.get("evidence_root"), str):
        raise RuntimeErrorEvidence("prepared campaign execution envelope differs")
    terminal = read_json(source / "terminal-manifest.json")
    if terminal.get("state") == "EXECUTED_PENDING_AUDIT":
        validate_terminal_manifest(source, profile_data, sha256_file(prepared_path), candidate_commit,
                                   prepared["profile_sha256"], launch_plan["sha256"], launch_plan_data,
                                   pathlib.Path(candidate["source_root"]), pathlib.Path(envelope["scratch_root"]),
                                   pathlib.Path(prepared["evidence_root"]))
    elif terminal.get("state") == "BLOCKED":
        validate_blocked_terminal_manifest(source, sha256_file(prepared_path), candidate_commit)
    else:
        raise RuntimeErrorEvidence("campaign terminal state differs")


def assemble(source: pathlib.Path, destination: pathlib.Path, candidate_commit: str, archival_commit: str | None,
             control_root: pathlib.Path, profile: pathlib.Path) -> None:
    if destination.exists():
        raise RuntimeErrorEvidence(f"canonical destination already exists: {destination}")
    _require_canonical_destination(destination)
    terminal = read_json(source / "terminal-manifest.json")
    if terminal.get("kind") != "reproducible-experiment-terminal-manifest" or terminal.get("state") not in {"EXECUTED_PENDING_AUDIT", "BLOCKED"}:
        raise RuntimeErrorEvidence("campaign terminal manifest is absent or invalid")
    for name in ("prepared-manifest.json", "launch-plan.json", "state.json", "state-history.jsonl"):
        if not (control_root / name).is_file():
            raise RuntimeErrorEvidence(f"control evidence is absent: {name}")
    _validate_campaign(source, control_root, profile, candidate_commit)
    files = retained_files(source)
    destination.mkdir(parents=True)
    copied: list[tuple[pathlib.Path, pathlib.Path]] = []
    for item in files:
        target = destination / item.relative_to(source)
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(item, target)
        copied.append((item, target))
    for name in ("prepared-manifest.json", "launch-plan.json", "state.json", "state-history.jsonl"):
        source_item, target = control_root / name, destination / "control" / name
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source_item, target)
        copied.append((source_item, target))
    write_json(destination / "retention-manifest.json", retention_manifest(destination, copied, candidate_commit, archival_commit))


def verify(destination: pathlib.Path, profile: pathlib.Path) -> None:
    _require_canonical_destination(destination)
    manifest = read_json(destination / "retention-manifest.json")
    if manifest.get("kind") != "canonical-evidence-retention" or not isinstance(manifest.get("files"), list):
        raise RuntimeErrorEvidence("retention manifest schema differs")
    expected_paths = {"retention-manifest.json"}
    for row in manifest["files"]:
        if not isinstance(row, dict) or set(row) != {"source_path", "retained_path", "sha256", "size"}:
            raise RuntimeErrorEvidence("retention manifest file schema differs")
        path = destination / row["retained_path"]
        if relative_path(destination, path) != row["retained_path"] or not path.is_file():
            raise RuntimeErrorEvidence("retained file is absent")
        if path.stat().st_size != row["size"] or sha256_file(path) != row["sha256"]:
            raise RuntimeErrorEvidence("retained file identity differs")
        expected_paths.add(row["retained_path"])
    observed_paths = {path.relative_to(destination).as_posix() for path in destination.rglob("*") if path.is_file()}
    if observed_paths != expected_paths:
        raise RuntimeErrorEvidence("canonical package contains unmanifested or absent files")
    candidate_commit = manifest.get("candidate_commit")
    if not isinstance(candidate_commit, str):
        raise RuntimeErrorEvidence("retention candidate identity differs")
    _validate_campaign(destination, destination / "control", profile, candidate_commit)


def main() -> int:
    parser = argparse.ArgumentParser()
    commands = parser.add_subparsers(dest="command", required=True)
    assemble_parser = commands.add_parser("assemble")
    assemble_parser.add_argument("--source", required=True); assemble_parser.add_argument("--destination", required=True)
    assemble_parser.add_argument("--candidate-commit", required=True); assemble_parser.add_argument("--archival-commit"); assemble_parser.add_argument("--control-root", required=True); assemble_parser.add_argument("--profile", required=True)
    verify_parser = commands.add_parser("verify"); verify_parser.add_argument("--destination", required=True); verify_parser.add_argument("--profile", required=True)
    arguments = parser.parse_args()
    try:
        if arguments.command == "assemble":
            assemble(pathlib.Path(arguments.source), pathlib.Path(arguments.destination), arguments.candidate_commit, arguments.archival_commit, pathlib.Path(arguments.control_root), pathlib.Path(arguments.profile))
        else:
            verify(pathlib.Path(arguments.destination), pathlib.Path(arguments.profile))
        return 0
    except (RuntimeErrorEvidence, EvidenceError) as error:
        print(error, file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
