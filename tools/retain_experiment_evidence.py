#!/usr/bin/env python3
"""Assemble or verify a compact canonical experiment-evidence package."""

from __future__ import annotations

import argparse
import pathlib
import shutil
import sys

from experiment_runtime import RuntimeErrorEvidence, read_json, relative_path, retention_manifest, sha256_file, write_json


EXCLUDED_PARTS = {"build", "CMakeFiles", ".ninja_deps", ".ninja_log"}


def retained_files(source: pathlib.Path) -> list[pathlib.Path]:
    files: list[pathlib.Path] = []
    for path in sorted(source.rglob("*")):
        if path.is_file() and not any(part in EXCLUDED_PARTS for part in path.relative_to(source).parts):
            files.append(path)
    if not files:
        raise RuntimeErrorEvidence("campaign evidence is empty")
    return files


def assemble(source: pathlib.Path, destination: pathlib.Path, candidate_commit: str, archival_commit: str | None) -> None:
    if destination.exists():
        raise RuntimeErrorEvidence(f"canonical destination already exists: {destination}")
    files = retained_files(source)
    destination.mkdir(parents=True)
    copied: list[tuple[pathlib.Path, pathlib.Path]] = []
    for item in files:
        target = destination / item.relative_to(source)
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(item, target)
        copied.append((item, target))
    write_json(destination / "retention-manifest.json", retention_manifest(destination, copied, candidate_commit, archival_commit))


def verify(destination: pathlib.Path) -> None:
    manifest = read_json(destination / "retention-manifest.json")
    if manifest.get("kind") != "canonical-evidence-retention" or not isinstance(manifest.get("files"), list):
        raise RuntimeErrorEvidence("retention manifest schema differs")
    for row in manifest["files"]:
        if not isinstance(row, dict) or set(row) != {"source_path", "retained_path", "sha256", "size"}:
            raise RuntimeErrorEvidence("retention manifest file schema differs")
        path = destination / row["retained_path"]
        if relative_path(destination, path) != row["retained_path"] or not path.is_file():
            raise RuntimeErrorEvidence("retained file is absent")
        if path.stat().st_size != row["size"] or sha256_file(path) != row["sha256"]:
            raise RuntimeErrorEvidence("retained file identity differs")


def main() -> int:
    parser = argparse.ArgumentParser()
    commands = parser.add_subparsers(dest="command", required=True)
    assemble_parser = commands.add_parser("assemble")
    assemble_parser.add_argument("--source", required=True); assemble_parser.add_argument("--destination", required=True)
    assemble_parser.add_argument("--candidate-commit", required=True); assemble_parser.add_argument("--archival-commit")
    verify_parser = commands.add_parser("verify"); verify_parser.add_argument("--destination", required=True)
    arguments = parser.parse_args()
    try:
        if arguments.command == "assemble":
            assemble(pathlib.Path(arguments.source), pathlib.Path(arguments.destination), arguments.candidate_commit, arguments.archival_commit)
        else:
            verify(pathlib.Path(arguments.destination))
        return 0
    except RuntimeErrorEvidence as error:
        print(error, file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
