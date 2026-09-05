#!/usr/bin/env python3
"""Prepare or execute the pre-registered Architecture Contract regression.

Without ``--execute``, this entrypoint writes only an external manifest and
command plan. Execution is deliberately opt-in.
"""

from __future__ import annotations

import argparse
import datetime as dt
import hashlib
import json
import os
import pathlib
import shutil
import subprocess
import sys
from dataclasses import dataclass
from typing import Any

from bootstrap_regression import EvidenceError, read_bytes, validate_profile


BUILD_TIMEOUT_SECONDS = 300
PROCESS_TIMEOUT_SECONDS = 30
OVERALL_TIMEOUT_SECONDS = 900


@dataclass(frozen=True)
class Configuration:
    name: str
    compiler: str
    libcxx: bool


CONFIGURATIONS = {
    "gcc-debug": Configuration("gcc-debug", "g++-13", False),
    "gcc-release": Configuration("gcc-release", "g++-13", False),
    "clang-debug": Configuration("clang-debug", "clang++-18", True),
    "clang-release": Configuration("clang-release", "clang++-18", True),
}


def utc_now() -> str:
    return dt.datetime.now(dt.timezone.utc).isoformat(timespec="seconds")


def sha256(path: pathlib.Path) -> str:
    return hashlib.sha256(read_bytes(path)).hexdigest()


def canonical_bytes(value: Any) -> bytes:
    return (json.dumps(value, sort_keys=True, separators=(",", ":"), ensure_ascii=True) + "\n").encode("utf-8")


def run(command: list[str], *, cwd: pathlib.Path, timeout: int, stdout: pathlib.Path, stderr: pathlib.Path) -> dict[str, Any]:
    started = utc_now()
    try:
        completed = subprocess.run(command, cwd=cwd, check=False, capture_output=True, timeout=timeout, text=False)
        stdout.write_bytes(completed.stdout)
        stderr.write_bytes(completed.stderr)
        return {"command": command, "exit_code": completed.returncode, "started_utc": started,
                "ended_utc": utc_now(), "stdout_sha256": sha256(stdout), "stderr_sha256": sha256(stderr)}
    except subprocess.TimeoutExpired as error:
        stdout.write_bytes(error.stdout or b"")
        stderr.write_bytes(error.stderr or b"")
        return {"command": command, "exit_code": None, "timeout_seconds": timeout, "started_utc": started,
                "ended_utc": utc_now(), "stdout_sha256": sha256(stdout), "stderr_sha256": sha256(stderr)}


def require_clean_candidate(source_root: pathlib.Path) -> tuple[str, list[dict[str, str]]]:
    status = subprocess.run(["git", "status", "--porcelain"], cwd=source_root, check=False, capture_output=True, text=True)
    if status.returncode != 0 or status.stdout:
        raise EvidenceError("candidate working tree is not clean")
    revision = subprocess.run(["git", "rev-parse", "HEAD"], cwd=source_root, check=True, capture_output=True, text=True).stdout.strip()
    paths = subprocess.run(["git", "ls-files"], cwd=source_root, check=True, capture_output=True, text=True).stdout.splitlines()
    return revision, [{"path": path, "sha256": sha256(source_root / path)} for path in paths]


def tool_version(command: str) -> dict[str, str]:
    resolved = shutil.which(command)
    if resolved is None:
        raise EvidenceError(f"required tool is unavailable: {command}")
    completed = subprocess.run([resolved, "--version"], check=False, capture_output=True, text=True)
    if completed.returncode != 0:
        raise EvidenceError(f"cannot query tool version: {resolved}")
    return {"path": str(pathlib.Path(resolved).resolve()), "version": (completed.stdout or completed.stderr).splitlines()[0]}


def command_plan(profile: dict[str, Any], source_root: pathlib.Path) -> list[dict[str, Any]]:
    plan: list[dict[str, Any]] = []
    for entry in profile["configurations"]:
        configuration = CONFIGURATIONS.get(entry["name"])
        if configuration is None:
            raise EvidenceError(f"unsupported configuration: {entry['name']}")
        build = "@OUTPUT_ROOT@/cells/" + configuration.name + "/build"
        plan.append({
            "name": configuration.name,
            "configure": ["cmake", "-S", str(source_root), "-B", build, "-G", "Ninja",
                          f"-DCMAKE_CXX_COMPILER={configuration.compiler}", f"-DCMAKE_BUILD_TYPE={entry['build_type']}",
                          f"-DAPMESH_USE_LIBCXX={'ON' if configuration.libcxx else 'OFF'}", "-DBUILD_TESTING=ON",
                          "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"],
            "build": ["cmake", "--build", build],
            "ctest": ["ctest", "--test-dir", build, "-L", "bootstrap", "--output-on-failure"],
            "certificate_processes": profile["certificate_processes_per_configuration"],
            "consumer_validations": profile["consumer_validations_per_configuration"],
        })
    return plan


def write_json(path: pathlib.Path, value: Any) -> None:
    path.write_bytes(canonical_bytes(value))


def command_result(command: list[str], *, cwd: pathlib.Path, timeout: int, root: pathlib.Path, name: str) -> dict[str, Any]:
    return run(command, cwd=cwd, timeout=timeout, stdout=root / f"{name}.stdout.log", stderr=root / f"{name}.stderr.log")


def directory_snapshot(root: pathlib.Path) -> list[dict[str, str]]:
    if not root.is_dir():
        raise EvidenceError(f"scratch directory is absent: {root}")
    entries: list[dict[str, str]] = []
    for path in sorted(root.rglob("*")):
        if path.is_symlink() or not path.is_file():
            raise EvidenceError(f"scratch contains an unsupported entry: {path}")
        entries.append({"path": str(path.relative_to(root)), "sha256": sha256(path)})
    return entries


def command_result_with_scratch_check(command: list[str], *, cwd: pathlib.Path, timeout: int,
                                      root: pathlib.Path, name: str, scratch: pathlib.Path) -> dict[str, Any]:
    before = directory_snapshot(scratch)
    record = command_result(command, cwd=cwd, timeout=timeout, root=root, name=name)
    after = directory_snapshot(scratch)
    record["scratch"] = {
        "before_sha256": hashlib.sha256(canonical_bytes(before)).hexdigest(),
        "after_sha256": hashlib.sha256(canonical_bytes(after)).hexdigest(),
        "unchanged": before == after,
    }
    if before != after:
        raise EvidenceError(f"{name} modified the scratch directory")
    return record


def require_success(record: dict[str, Any], context: str) -> None:
    if record["exit_code"] != 0:
        raise EvidenceError(f"{context} failed")


def prepare(arguments: argparse.Namespace) -> tuple[pathlib.Path, dict[str, Any]]:
    source_root = pathlib.Path(arguments.source_root).resolve()
    output_root = pathlib.Path(arguments.output_root).resolve()
    if output_root.exists():
        raise EvidenceError(f"output root already exists: {output_root}")
    revision, inventory = require_clean_candidate(source_root)
    profile_path = pathlib.Path(arguments.profile).resolve()
    profile = validate_profile(profile_path)
    output_root.mkdir(parents=True)
    plan = command_plan(profile, source_root)
    manifest = {
        "schema_version": 1, "protocol_version": profile["protocol_version"], "state": "PREPARED",
        "candidate": {"commit": revision, "tree_clean": True, "source_root": str(source_root), "source_inventory": inventory},
        "inputs": {
            "profile": {"path": str(profile_path), "sha256": sha256(profile_path)},
            "protocol": {"path": str(pathlib.Path(arguments.protocol).resolve()), "sha256": sha256(pathlib.Path(arguments.protocol))},
            "expected_certificate": {"path": str(pathlib.Path(arguments.expected).resolve()), "sha256": sha256(pathlib.Path(arguments.expected))},
            "comparer": {"path": str(pathlib.Path(arguments.comparer).resolve()), "sha256": sha256(pathlib.Path(arguments.comparer))},
            "launcher": {"path": str(pathlib.Path(__file__).resolve()), "sha256": sha256(pathlib.Path(__file__).resolve())},
        },
        "environment": {"python": sys.version.splitlines()[0], "python_path": str(pathlib.Path(sys.executable).resolve()),
                        "cmake": tool_version("cmake"), "ninja": tool_version("ninja"), "gcc": tool_version("g++-13"),
                        "clang": tool_version("clang++-18"), "pid": os.getpid(), "working_directory": str(pathlib.Path.cwd())},
        "limits_seconds": {"build": BUILD_TIMEOUT_SECONDS, "process": PROCESS_TIMEOUT_SECONDS, "overall": OVERALL_TIMEOUT_SECONDS},
        "plan": plan, "negative_fixtures": profile["negative_fixtures"], "prepared_utc": utc_now(),
        "execution_requested": False,
    }
    write_json(output_root / "manifest.json", manifest)
    write_json(output_root / "plan.json", {"protocol_version": profile["protocol_version"], "cells": plan})
    return output_root, manifest


def load_prepared(arguments: argparse.Namespace) -> tuple[pathlib.Path, dict[str, Any]]:
    output_root = pathlib.Path(arguments.output_root).resolve()
    manifest_path = output_root / "manifest.json"
    if not manifest_path.is_file():
        raise EvidenceError(f"prepared manifest is absent: {manifest_path}")
    try:
        manifest = json.loads(read_bytes(manifest_path).decode("utf-8"))
    except (UnicodeDecodeError, json.JSONDecodeError) as error:
        raise EvidenceError(f"prepared manifest is invalid: {manifest_path}") from error
    if not isinstance(manifest, dict) or manifest.get("state") != "PREPARED":
        raise EvidenceError("manifest is not in PREPARED state")
    if manifest.get("execution_requested") is not False:
        raise EvidenceError("prepared manifest has an invalid execution request")
    source_root = pathlib.Path(arguments.source_root).resolve()
    revision, inventory = require_clean_candidate(source_root)
    candidate = manifest.get("candidate")
    if not isinstance(candidate, dict) or candidate.get("commit") != revision:
        raise EvidenceError("prepared manifest candidate commit differs")
    if candidate.get("source_root") != str(source_root) or candidate.get("source_inventory") != inventory:
        raise EvidenceError("prepared manifest candidate inventory differs")
    expected_inputs = {
        "profile": pathlib.Path(arguments.profile).resolve(),
        "protocol": pathlib.Path(arguments.protocol).resolve(),
        "expected_certificate": pathlib.Path(arguments.expected).resolve(),
        "comparer": pathlib.Path(arguments.comparer).resolve(),
        "launcher": pathlib.Path(__file__).resolve(),
    }
    inputs = manifest.get("inputs")
    if not isinstance(inputs, dict):
        raise EvidenceError("prepared manifest inputs are absent")
    for key, path in expected_inputs.items():
        entry = inputs.get(key)
        if not isinstance(entry, dict) or entry.get("path") != str(path) or entry.get("sha256") != sha256(path):
            raise EvidenceError(f"prepared manifest input differs: {key}")
    return output_root, manifest


def execute(output_root: pathlib.Path, manifest: dict[str, Any]) -> int:
    started = dt.datetime.now(dt.timezone.utc)
    records: list[dict[str, Any]] = []
    source_root = pathlib.Path(manifest["candidate"]["source_root"])
    comparer = manifest["inputs"]["comparer"]["path"]
    expected = manifest["inputs"]["expected_certificate"]["path"]
    try:
        for cell in manifest["plan"]:
            if (dt.datetime.now(dt.timezone.utc) - started).total_seconds() > OVERALL_TIMEOUT_SECONDS:
                raise EvidenceError("overall timeout")
            cell_root = output_root / "cells" / cell["name"]
            cell_root.mkdir(parents=True)
            cell_records: list[dict[str, Any]] = []
            for stage_name in ("configure", "build", "ctest"):
                command = [part.replace("@OUTPUT_ROOT@", str(output_root)) for part in cell[stage_name]]
                timeout = BUILD_TIMEOUT_SECONDS if stage_name != "ctest" else PROCESS_TIMEOUT_SECONDS
                record = command_result(command, cwd=output_root, timeout=timeout, root=cell_root, name=stage_name)
                record["stage"] = stage_name
                cell_records.append(record)
                require_success(record, f"{cell['name']} {stage_name}")
            build_root = cell_root / "build"
            export_path = build_root / "apmesh_core_bootstrap_export"
            scratch_root = cell_root / "scratch"
            scratch_root.mkdir()
            record = command_result_with_scratch_check(
                [str(build_root / "apmesh_core_bootstrap_smoke")], cwd=scratch_root,
                timeout=PROCESS_TIMEOUT_SECONDS, root=cell_root, name="scratch-smoke", scratch=scratch_root,
            )
            cell_records.append(record)
            require_success(record, f"{cell['name']} scratch smoke")
            certificates: list[str] = []
            for repeat in range(cell["certificate_processes"]):
                certificate = cell_root / f"certificate-{repeat + 1}.json"
                record = command_result([str(export_path), str(certificate)], cwd=cell_root, timeout=PROCESS_TIMEOUT_SECONDS, root=cell_root, name=f"export-{repeat + 1}")
                cell_records.append(record)
                require_success(record, f"{cell['name']} certificate {repeat + 1}")
                record = command_result([sys.executable, comparer, "validate", "--expected", expected, "--actual", str(certificate)], cwd=cell_root, timeout=PROCESS_TIMEOUT_SECONDS, root=cell_root, name=f"validate-{repeat + 1}")
                cell_records.append(record)
                require_success(record, f"{cell['name']} certificate validation {repeat + 1}")
                certificates.append(str(certificate))
            configuration = CONFIGURATIONS[cell["name"]]
            consumer_root = cell_root / "consumer"
            consumer_configure = ["cmake", "-S", str(source_root / "tests" / "consumer"), "-B", str(consumer_root), "-G", "Ninja", f"-DCMAKE_CXX_COMPILER={configuration.compiler}", f"-DAPMESH_CORE_SOURCE_DIR={source_root}", "-DBUILD_TESTING=OFF", f"-DAPMESH_USE_LIBCXX={'ON' if configuration.libcxx else 'OFF'}"]
            for stage_name, command in (("consumer-configure", consumer_configure), ("consumer-build", ["cmake", "--build", str(consumer_root), "--target", "apmesh_core_external_consumer", "apmesh_core_unrelated_target"]), ("consumer", [str(consumer_root / "apmesh_core_external_consumer")]), ("unrelated", [str(consumer_root / "apmesh_core_unrelated_target")])):
                timeout = BUILD_TIMEOUT_SECONDS if "build" in stage_name or "configure" in stage_name else PROCESS_TIMEOUT_SECONDS
                if stage_name in ("consumer", "unrelated"):
                    record = command_result_with_scratch_check(
                        command, cwd=scratch_root, timeout=timeout, root=cell_root,
                        name=stage_name, scratch=scratch_root,
                    )
                else:
                    record = command_result(command, cwd=cell_root, timeout=timeout, root=cell_root, name=stage_name)
                cell_records.append(record)
                require_success(record, f"{cell['name']} {stage_name}")
            compile_commands = build_root / "compile_commands.json"
            if not compile_commands.is_file():
                raise EvidenceError(f"{cell['name']} compile commands are absent")
            cell_records.append({"stage": "compile-commands", "sha256": sha256(compile_commands)})
            records.append({"cell": cell["name"], "state": "PASS", "certificates": certificates, "records": cell_records})

        all_certificates = [certificate for record in records for certificate in record["certificates"]]
        report_one = output_root / "reports" / "one" / "certificate-report.md"
        report_two = output_root / "reports" / "two" / "certificate-report.md"
        for name, report in (("compare-one", report_one), ("compare-two", report_two)):
            record = command_result([sys.executable, comparer, "compare", "--expected", expected, *sum((["--certificate", certificate] for certificate in all_certificates), []), "--report", str(report)], cwd=output_root, timeout=PROCESS_TIMEOUT_SECONDS, root=output_root, name=name)
            require_success(record, name)
            records.append({"global": name, "state": "PASS", "record": record})
        if read_bytes(report_one) != read_bytes(report_two):
            raise EvidenceError("repeated certificate reports differ")
        fixtures = source_root / "tests" / "data" / "bootstrap_regression"
        negative_commands = [
            [sys.executable, comparer, "validate", "--expected", expected, "--actual", str(output_root / "missing.json")],
            [sys.executable, comparer, "validate", "--expected", expected, "--actual", str(fixtures / "malformed.json")],
            [sys.executable, comparer, "validate", "--expected", expected, "--actual", str(fixtures / "unsupported_schema_version.json")],
            [sys.executable, comparer, "validate", "--expected", expected, "--actual", str(fixtures / "changed_component.json")],
            [sys.executable, comparer, "validate-manifest", "--manifest", str(fixtures / "manifest_changed_source_hash.json"), "--expected-source-hash", "expected-source-hash"],
        ]
        negative_records: list[dict[str, Any]] = []
        for index, command in enumerate(negative_commands, start=1):
            record = command_result(command, cwd=output_root, timeout=PROCESS_TIMEOUT_SECONDS, root=output_root, name=f"negative-{index}")
            if record["exit_code"] == 0:
                raise EvidenceError(f"negative fixture {index} was accepted")
            negative_records.append(record)
        manifest["state"] = "EXECUTED_PENDING_AUDIT"
        manifest["execution"] = {"started_utc": started.isoformat(timespec="seconds"), "ended_utc": utc_now(), "records": records, "negative_records": negative_records}
        manifest["requirements"] = {str(number): "EVIDENCE_COLLECTED_PENDING_AUDIT" for number in range(1, 9)}
        write_json(output_root / "manifest.json", manifest)
        return 0
    except EvidenceError as error:
        manifest["state"] = "BLOCKED"
        manifest["execution"] = {"started_utc": started.isoformat(timespec="seconds"), "ended_utc": utc_now(), "records": records, "reason": str(error)}
        write_json(output_root / "manifest.json", manifest)
        return 1


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source-root", required=True)
    parser.add_argument("--profile", required=True)
    parser.add_argument("--protocol", required=True)
    parser.add_argument("--expected", required=True)
    parser.add_argument("--comparer", required=True)
    parser.add_argument("--output-root", required=True)
    parser.add_argument("--execute", action="store_true")
    return parser.parse_args()


def main() -> int:
    arguments = parse_arguments()
    try:
        if arguments.execute:
            output_root, manifest = load_prepared(arguments)
            return execute(output_root, manifest)
        prepare(arguments)
        return 0
    except EvidenceError as error:
        print(error, file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
