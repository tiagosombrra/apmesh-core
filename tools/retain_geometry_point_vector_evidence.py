#!/usr/bin/env python3
"""Retain and verify the minimal canonical Point/Vector evidence package."""

from __future__ import annotations

import argparse
import hashlib
import json
import pathlib
import shutil
import sys
from typing import Any


ROOT_FILES = {
    "certificate-index.json",
    "detached-verification.json",
    "execution-progress.json",
    "plan.json",
    "prepared-manifest.json",
    "report.json",
    "report.md",
    "retention-manifest.json",
    "runtime-dependencies.json",
    "state-history.jsonl",
    "state.json",
    "terminal-manifest.json",
}
GATES = tuple(f"PV{number}" for number in range(8))
KIND = "geometry-point-vector-canonical-evidence"


class EvidenceError(RuntimeError):
    pass


def read_json(path: pathlib.Path) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, UnicodeDecodeError, json.JSONDecodeError) as error:
        raise EvidenceError(f"invalid JSON: {path}") from error
    if not isinstance(value, dict):
        raise EvidenceError(f"JSON object required: {path}")
    return value


def write_json(path: pathlib.Path, value: dict[str, Any]) -> None:
    path.write_text(json.dumps(value, sort_keys=True, indent=2) + "\n", encoding="utf-8")


def sha256(path: pathlib.Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def retained_path(relative: pathlib.PurePosixPath) -> pathlib.PurePosixPath | None:
    if len(relative.parts) == 1 and relative.name in ROOT_FILES:
        return relative
    if len(relative.parts) >= 3 and relative.parts[0] == "cells":
        cell = relative.parts[1]
        if relative.name in {"certificate-1.json", "certificate-2.json", "certificate-3.json", "negative-fixtures.json"}:
            return relative
        if "logs" in relative.parts:
            return relative
        if relative.parts[-2:] == ("build", "compile_commands.json"):
            return pathlib.PurePosixPath("inventories") / cell / "compile_commands.json"
    return None


def selected_files(source: pathlib.Path) -> list[tuple[pathlib.Path, pathlib.PurePosixPath, pathlib.PurePosixPath]]:
    selected: list[tuple[pathlib.Path, pathlib.PurePosixPath, pathlib.PurePosixPath]] = []
    for path in sorted(source.rglob("*")):
        if not path.is_file():
            continue
        relative = pathlib.PurePosixPath(path.relative_to(source).as_posix())
        target = retained_path(relative)
        if target is not None:
            selected.append((path, relative, target))
    missing = ROOT_FILES - {relative.name for _, relative, _ in selected if len(relative.parts) == 1}
    if missing:
        raise EvidenceError(f"source package misses root files: {sorted(missing)}")
    if len([item for item in selected if item[1].name in {"certificate-1.json", "certificate-2.json", "certificate-3.json"}]) != 12:
        raise EvidenceError("source package does not contain twelve certificates")
    if len([item for item in selected if item[2].parts[0] == "inventories"]) != 4:
        raise EvidenceError("source package does not contain four compile-command inventories")
    return selected


def validate_source(source: pathlib.Path, candidate_commit: str) -> None:
    prepared = read_json(source / "prepared-manifest.json")
    terminal = read_json(source / "terminal-manifest.json")
    if prepared.get("kind") != "geometry-point-vector-prepared-manifest":
        raise EvidenceError("prepared manifest kind differs")
    if terminal.get("kind") != "geometry-point-vector-terminal-manifest":
        raise EvidenceError("terminal manifest kind differs")
    if prepared.get("candidate", {}).get("commit") != candidate_commit or terminal.get("candidate") != prepared.get("candidate"):
        raise EvidenceError("candidate identity differs")
    if terminal.get("state") != "EXECUTED_PENDING_AUDIT":
        raise EvidenceError("terminal state differs")
    if terminal.get("prepared_manifest_sha256") != sha256(source / "prepared-manifest.json"):
        raise EvidenceError("prepared manifest hash differs")
    if terminal.get("gates") != {gate: "EVIDENCE_COLLECTED_PENDING_AUDIT" for gate in GATES}:
        raise EvidenceError("terminal gate state differs")


def assemble(source: pathlib.Path, destination: pathlib.Path, candidate_commit: str, archival_revision: str) -> None:
    if destination.exists():
        raise EvidenceError(f"destination already exists: {destination}")
    if destination.parts[-4:-1] != ("evidence", "geometry-primitives", "point-vector"):
        raise EvidenceError("destination must be evidence/geometry-primitives/point-vector/<campaign-id>")
    validate_source(source, candidate_commit)
    selected = selected_files(source)
    destination.mkdir(parents=True)
    rows: list[dict[str, Any]] = []
    for source_path, source_relative, destination_relative in selected:
        target = destination / destination_relative
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source_path, target)
        rows.append({"source_path": source_relative.as_posix(), "retained_path": destination_relative.as_posix(),
                     "sha256": sha256(source_path), "size": source_path.stat().st_size})
    manifest = {
        "schema_version": 1,
        "kind": KIND,
        "candidate_commit": candidate_commit,
        "archival_revision": archival_revision,
        "source_retention_manifest_sha256": sha256(source / "retention-manifest.json"),
        "files": rows,
    }
    write_json(destination / "canonical-evidence-manifest.json", manifest)


def verify(destination: pathlib.Path, candidate_commit: str) -> None:
    manifest = read_json(destination / "canonical-evidence-manifest.json")
    if (manifest.get("schema_version") != 1 or manifest.get("kind") != KIND or
            manifest.get("candidate_commit") != candidate_commit or not isinstance(manifest.get("files"), list)):
        raise EvidenceError("canonical manifest identity differs")
    expected = {"canonical-evidence-manifest.json"}
    observed_compile_commands: set[str] = set()
    for row in manifest["files"]:
        if not isinstance(row, dict) or set(row) != {"source_path", "retained_path", "sha256", "size"}:
            raise EvidenceError("canonical manifest row differs")
        target = destination / str(row["retained_path"])
        if not target.is_file() or target.stat().st_size != row["size"] or sha256(target) != row["sha256"]:
            raise EvidenceError("retained file identity differs")
        if "/build/" in str(row["retained_path"]) or target.name in {"CMakeCache.txt", ".ninja_deps", ".ninja_log"}:
            raise EvidenceError("scratch build artifact retained")
        if target.suffix in {".o", ".a"} or target.name.startswith("apmesh_core"):
            raise EvidenceError("reproducible binary retained")
        expected.add(str(row["retained_path"]))
        if str(row["retained_path"]).startswith("inventories/"):
            observed_compile_commands.add(str(row["retained_path"]))
    actual = {path.relative_to(destination).as_posix() for path in destination.rglob("*") if path.is_file()}
    if actual != expected:
        raise EvidenceError("canonical file set differs")
    if len(observed_compile_commands) != 4:
        raise EvidenceError("canonical compile-command inventory differs")
    validate_source(destination, candidate_commit)
    index = read_json(destination / "certificate-index.json")
    entries = index.get("entries")
    if not isinstance(entries, list) or len(entries) != 12:
        raise EvidenceError("canonical certificate matrix differs")
    for entry in entries:
        if not isinstance(entry, dict) or not isinstance(entry.get("path"), str) or not isinstance(entry.get("sha256"), str):
            raise EvidenceError("canonical certificate index differs")
        certificate = destination / entry["path"]
        if not certificate.is_file() or sha256(certificate) != entry["sha256"]:
            raise EvidenceError("canonical certificate hash differs")
    if manifest.get("source_retention_manifest_sha256") != sha256(destination / "retention-manifest.json"):
        raise EvidenceError("source retention manifest hash differs")


def main() -> int:
    parser = argparse.ArgumentParser()
    commands = parser.add_subparsers(dest="command", required=True)
    for name in ("assemble", "verify"):
        command = commands.add_parser(name)
        command.add_argument("--destination", required=True)
        command.add_argument("--candidate-commit", required=True)
    assemble_parser = commands.choices["assemble"]
    assemble_parser.add_argument("--source", required=True)
    assemble_parser.add_argument("--archival-revision", required=True)
    arguments = parser.parse_args()
    try:
        destination = pathlib.Path(arguments.destination)
        if arguments.command == "assemble":
            assemble(pathlib.Path(arguments.source), destination, arguments.candidate_commit, arguments.archival_revision)
        else:
            verify(destination, arguments.candidate_commit)
    except EvidenceError as error:
        print(error, file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
