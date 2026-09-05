#!/usr/bin/env python3
"""Validate and report bounded AP Mesh Core bootstrap evidence."""

from __future__ import annotations

import argparse
import hashlib
import json
import pathlib
import sys
from typing import Any


EXPECTED_KEYS = ("schema_version", "component", "language")


class EvidenceError(Exception):
    pass


def read_bytes(path: pathlib.Path) -> bytes:
    try:
        return path.read_bytes()
    except OSError as error:
        raise EvidenceError(f"cannot read {path}: {error}") from error


def parse_certificate(path: pathlib.Path) -> tuple[bytes, dict[str, Any]]:
    raw = read_bytes(path)
    if raw.startswith(b"\xef\xbb\xbf"):
        raise EvidenceError(f"certificate has UTF-8 BOM: {path}")
    try:
        decoded = raw.decode("utf-8")
        value = json.loads(decoded)
    except (UnicodeDecodeError, json.JSONDecodeError) as error:
        raise EvidenceError(f"invalid certificate JSON {path}: {error}") from error
    if not isinstance(value, dict) or tuple(value.keys()) != EXPECTED_KEYS:
        raise EvidenceError(f"certificate schema keys differ: {path}")
    if (
        type(value["schema_version"]) is not int
        or type(value["component"]) is not str
        or type(value["language"]) is not str
    ):
        raise EvidenceError(f"certificate field types differ: {path}")
    return raw, value


def validate_certificate(expected_path: pathlib.Path, actual_path: pathlib.Path) -> bytes:
    expected_raw, expected_value = parse_certificate(expected_path)
    actual_raw, actual_value = parse_certificate(actual_path)
    if actual_value != expected_value:
        raise EvidenceError(f"certificate values differ: {actual_path}")
    if actual_raw != expected_raw:
        raise EvidenceError(f"certificate bytes differ: {actual_path}")
    return actual_raw


def validate_manifest(manifest_path: pathlib.Path, expected_source_hash: str) -> None:
    raw = read_bytes(manifest_path)
    try:
        manifest = json.loads(raw.decode("utf-8"))
    except (UnicodeDecodeError, json.JSONDecodeError) as error:
        raise EvidenceError(f"invalid manifest JSON {manifest_path}: {error}") from error
    if not isinstance(manifest, dict) or manifest.get("source_hash") != expected_source_hash:
        raise EvidenceError(f"manifest source hash differs: {manifest_path}")


def validate_profile(profile_path: pathlib.Path) -> dict[str, Any]:
    try:
        profile = json.loads(read_bytes(profile_path).decode("utf-8"))
    except (UnicodeDecodeError, json.JSONDecodeError) as error:
        raise EvidenceError(f"invalid profile JSON {profile_path}: {error}") from error
    required = {
        "protocol_version",
        "certificate_processes_per_configuration",
        "consumer_validations_per_configuration",
        "configurations",
        "negative_fixtures",
    }
    if not isinstance(profile, dict) or set(profile) != required:
        raise EvidenceError("profile keys differ")
    configurations = profile["configurations"]
    names = [entry.get("name") for entry in configurations] if isinstance(configurations, list) else []
    if names != ["gcc-debug", "gcc-release", "clang-debug", "clang-release"]:
        raise EvidenceError("profile configuration order differs")
    if profile["certificate_processes_per_configuration"] != 3:
        raise EvidenceError("profile certificate repeat count differs")
    if profile["consumer_validations_per_configuration"] != 1:
        raise EvidenceError("profile consumer count differs")
    if profile["negative_fixtures"] != [
        "missing_certificate",
        "malformed_json",
        "unsupported_schema_version",
        "changed_component",
        "changed_source_hash",
    ]:
        raise EvidenceError("profile negative-fixture list differs")
    return profile


def write_bytes(path: pathlib.Path, content: bytes) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(content)


def command_validate(arguments: argparse.Namespace) -> int:
    validate_certificate(pathlib.Path(arguments.expected), pathlib.Path(arguments.actual))
    return 0


def command_validate_manifest(arguments: argparse.Namespace) -> int:
    validate_manifest(pathlib.Path(arguments.manifest), arguments.expected_source_hash)
    return 0


def command_plan(arguments: argparse.Namespace) -> int:
    profile = validate_profile(pathlib.Path(arguments.profile))
    source_root = pathlib.Path(arguments.source_root).resolve()
    if not source_root.is_dir():
        raise EvidenceError(f"source root is not a directory: {source_root}")
    plan = {
        "certificate_processes_per_configuration": profile["certificate_processes_per_configuration"],
        "configurations": profile["configurations"],
        "consumer_validations_per_configuration": profile["consumer_validations_per_configuration"],
        "negative_fixtures": profile["negative_fixtures"],
        "protocol_version": profile["protocol_version"],
        "source_root": source_root.as_posix(),
    }
    write_bytes(
        pathlib.Path(arguments.output),
        (json.dumps(plan, separators=(",", ":"), ensure_ascii=True) + "\n").encode("utf-8"),
    )
    return 0


def command_compare(arguments: argparse.Namespace) -> int:
    expected_path = pathlib.Path(arguments.expected)
    rows: list[tuple[str, str]] = []
    for certificate in arguments.certificate:
        path = pathlib.Path(certificate)
        raw = validate_certificate(expected_path, path)
        rows.append((path.name, hashlib.sha256(raw).hexdigest()))
    lines = [
        "# Architecture Bootstrap Certificate Report",
        "",
        "| Certificate | SHA-256 | Result |",
        "| --- | --- | --- |",
    ]
    lines.extend(f"| {name} | `{digest}` | PASS |" for name, digest in rows)
    lines.extend(["", "Overall: PASS", ""])
    write_bytes(pathlib.Path(arguments.report), "\n".join(lines).encode("utf-8"))
    return 0


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    commands = parser.add_subparsers(dest="command", required=True)

    validate = commands.add_parser("validate")
    validate.add_argument("--expected", required=True)
    validate.add_argument("--actual", required=True)
    validate.set_defaults(handler=command_validate)

    validate_manifest_command = commands.add_parser("validate-manifest")
    validate_manifest_command.add_argument("--manifest", required=True)
    validate_manifest_command.add_argument("--expected-source-hash", required=True)
    validate_manifest_command.set_defaults(handler=command_validate_manifest)

    plan = commands.add_parser("plan")
    plan.add_argument("--profile", required=True)
    plan.add_argument("--source-root", required=True)
    plan.add_argument("--output", required=True)
    plan.set_defaults(handler=command_plan)

    compare = commands.add_parser("compare")
    compare.add_argument("--expected", required=True)
    compare.add_argument("--certificate", required=True, action="append")
    compare.add_argument("--report", required=True)
    compare.set_defaults(handler=command_compare)

    return parser.parse_args()


def main() -> int:
    arguments = parse_arguments()
    try:
        return arguments.handler(arguments)
    except EvidenceError as error:
        print(error, file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
