#!/usr/bin/env python3
"""Report-only CF0-CF7 tooling. This program intentionally has no execute path."""
from __future__ import annotations

import argparse
import hashlib
import json
import pathlib
import subprocess
import sys
from typing import Any

from experiment_runtime import RuntimeErrorEvidence, canonical_json, clean_candidate, input_identity, write_json


def fail(message: str) -> RuntimeErrorEvidence:
    return RuntimeErrorEvidence(message)


def run_validator(argv: list[str], cwd: pathlib.Path) -> None:
    result = subprocess.run([sys.executable, *argv], cwd=cwd, text=True, capture_output=True, check=False)
    if result.returncode:
        raise fail(f"validator failed: {result.stderr.strip()}")


def require_protocol(path: pathlib.Path) -> None:
    text = path.read_text(encoding="utf-8")
    required = ("CF0", "CF1", "CF2", "CF3", "CF4", "CF5", "CF6", "CF7", "NOT EXECUTED", "report-only")
    if any(token not in text for token in required):
        raise fail("protocol does not define the fixed report-only CF0-CF7 boundary")


def source_paths(source: pathlib.Path, args: argparse.Namespace) -> dict[str, pathlib.Path]:
    paths = {"profile": args.profile.resolve(), "protocol": args.protocol.resolve(), "validator": args.validator.resolve(), "exporter": args.exporter.resolve(), "runtime": (source / "tools" / "experiment_runtime.py").resolve(), "frames_header": (source / "include" / "apmesh" / "core" / "geometry.hpp").resolve(), "frames_source": (source / "src" / "core" / "geometry.cpp").resolve()}
    if any(not path.is_file() for path in paths.values()):
        raise fail("report-only CF infrastructure input is missing")
    return paths


def report_plan(args: argparse.Namespace) -> None:
    source = args.source_root.resolve(); candidate = clean_candidate(source)
    if not candidate["clean"]:
        raise fail("report plan requires a clean source candidate")
    paths = source_paths(source, args); require_protocol(paths["protocol"])
    run_validator([str(paths["validator"]), "validate-profile", "--profile", str(paths["profile"])], source)
    output = {"schema_version": 1, "kind": "cartesian-frames-report-plan", "status": "report_only_not_prepared", "candidate": candidate, "input_identity": input_identity(paths), "formal_manifest": None, "execution_requested": False, "next_required_boundary": "separate authorization before any prepared manifest"}
    if args.output: write_json(args.output, output)
    else: print(json.dumps(output, sort_keys=True))


def self_check(args: argparse.Namespace) -> None:
    source = args.source_root.resolve(); paths = source_paths(source, args); require_protocol(paths["protocol"])
    run_validator([str(paths["validator"]), "validate-profile", "--profile", str(paths["profile"])], source)
    certificate = args.output.parent / "certificate.json" if args.output else source / ".cartesian_frames_self_check.json"
    result = subprocess.run([str(paths["exporter"]), str(certificate)], cwd=source, text=True, capture_output=True, check=False)
    if result.returncode: raise fail(f"exporter failed: {result.stderr.strip()}")
    run_validator([str(paths["validator"]), "validate-certificate", "--profile", str(paths["profile"]), "--certificate", str(certificate)], source)
    negative = certificate.with_name("negative-outcomes.json")
    run_validator([str(paths["validator"]), "negative-self-check", "--profile", str(paths["profile"]), "--certificate", str(certificate), "--output", str(negative)], source)
    report = {"schema_version": 1, "kind": "cartesian-frames-report-only-self-check", "status": "pass", "certificate_sha256": hashlib.sha256(certificate.read_bytes()).hexdigest(), "negative_outcomes_sha256": hashlib.sha256(negative.read_bytes()).hexdigest(), "formal_manifest": None, "execution_requested": False}
    if args.output: write_json(args.output, report)
    else: print(json.dumps(report, sort_keys=True))
    if not args.output:
        certificate.unlink(missing_ok=True); negative.unlink(missing_ok=True)


def verify_retention(args: argparse.Namespace) -> None:
    package = json.loads(args.package.read_text(encoding="utf-8"))
    if package.get("status") != "sealed" or package.get("formal_manifest") is not None:
        raise fail("retention package is not a sealed report-only package")
    entries = package.get("entries")
    if not isinstance(entries, list) or not entries:
        raise fail("retention package has no entries")
    for entry in entries:
        path = pathlib.Path(entry["path"]); expected = entry["sha256"]
        if not path.is_file() or hashlib.sha256(path.read_bytes()).hexdigest() != expected:
            raise fail("retention hash mismatch")


def main() -> int:
    parser = argparse.ArgumentParser(); sub = parser.add_subparsers(dest="command", required=True)
    common = argparse.ArgumentParser(add_help=False)
    common.add_argument("--source-root", type=pathlib.Path, required=True); common.add_argument("--profile", type=pathlib.Path, required=True); common.add_argument("--protocol", type=pathlib.Path, required=True); common.add_argument("--validator", type=pathlib.Path, required=True); common.add_argument("--exporter", type=pathlib.Path, required=True); common.add_argument("--output", type=pathlib.Path)
    sub.add_parser("report-plan", parents=[common]); sub.add_parser("self-check", parents=[common])
    retention = sub.add_parser("verify-retention"); retention.add_argument("--package", type=pathlib.Path, required=True)
    args = parser.parse_args()
    try:
        if args.command == "report-plan": report_plan(args)
        elif args.command == "self-check": self_check(args)
        else: verify_retention(args)
    except (RuntimeErrorEvidence, OSError, json.JSONDecodeError) as error:
        print(f"cartesian-frames runner: {error}", file=sys.stderr); return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
