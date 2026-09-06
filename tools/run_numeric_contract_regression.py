#!/usr/bin/env python3
"""Prepare or explicitly execute the bounded Numeric Contract regression."""

from __future__ import annotations

import argparse
import datetime as dt
import hashlib
import json
import pathlib
import shutil
import subprocess
import sys
from typing import Any

from numeric_contract_evidence import EvidenceError, validate_certificate, validate_environment, validate_profile


BUILD_TIMEOUT_SECONDS = 300
PROCESS_TIMEOUT_SECONDS = 30
OVERALL_TIMEOUT_SECONDS = 1_200
CONFIGURATIONS = {
    "gcc-debug": ("g++-13", False), "gcc-release": ("g++-13", False),
    "clang-debug": ("clang++-18", True), "clang-release": ("clang++-18", True),
}


def read_bytes(path: pathlib.Path) -> bytes:
    try:
        return path.read_bytes()
    except OSError as error:
        raise EvidenceError(f"cannot read {path}: {error}") from error


def sha256(path: pathlib.Path) -> str:
    return hashlib.sha256(read_bytes(path)).hexdigest()


def utc_now() -> str:
    return dt.datetime.now(dt.timezone.utc).isoformat(timespec="seconds")


def write_json(path: pathlib.Path, value: Any) -> None:
    path.write_bytes((json.dumps(value, sort_keys=True, separators=(",", ":"), ensure_ascii=True) + "\n").encode("utf-8"))


def clean_candidate(source_root: pathlib.Path) -> tuple[str, list[dict[str, str]]]:
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
    result = subprocess.run([resolved, "--version"], check=False, capture_output=True, text=True)
    if result.returncode != 0:
        raise EvidenceError(f"cannot query tool version: {resolved}")
    return {"path": str(pathlib.Path(resolved).resolve()), "version": (result.stdout or result.stderr).splitlines()[0]}


def command_plan(profile: dict[str, Any], source_root: pathlib.Path) -> list[dict[str, Any]]:
    plan: list[dict[str, Any]] = []
    for entry in profile["configurations"]:
        name = entry["name"]
        if name not in CONFIGURATIONS:
            raise EvidenceError(f"unsupported configuration: {name}")
        compiler, libcxx = CONFIGURATIONS[name]
        build = f"@OUTPUT_ROOT@/cells/{name}/build"
        plan.append({
            "name": name,
            "configure": ["cmake", "-S", str(source_root), "-B", build, "-G", "Ninja", f"-DCMAKE_CXX_COMPILER={compiler}",
                          f"-DCMAKE_BUILD_TYPE={entry['build_type']}", f"-DAPMESH_USE_LIBCXX={'ON' if libcxx else 'OFF'}",
                          "-DBUILD_TESTING=ON", "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON"],
            "build": ["cmake", "--build", build, "--target", "apmesh_core_numeric_contract_export", "apmesh_core_numeric_contract"],
            "ctest": ["ctest", "--test-dir", build, "-L", "numeric", "--output-on-failure"],
            "repetitions": profile["repetitions_per_configuration"],
        })
    return plan


def execute_command(command: list[str], cwd: pathlib.Path, timeout: int, root: pathlib.Path, name: str) -> dict[str, Any]:
    started = utc_now()
    stdout, stderr = root / f"{name}.stdout.log", root / f"{name}.stderr.log"
    try:
        completed = subprocess.run(command, cwd=cwd, check=False, capture_output=True, timeout=timeout, text=False)
        stdout.write_bytes(completed.stdout)
        stderr.write_bytes(completed.stderr)
        return {"command": command, "exit_code": completed.returncode, "started_utc": started, "ended_utc": utc_now(),
                "stdout_sha256": sha256(stdout), "stderr_sha256": sha256(stderr)}
    except subprocess.TimeoutExpired as error:
        stdout.write_bytes(error.stdout or b"")
        stderr.write_bytes(error.stderr or b"")
        return {"command": command, "exit_code": None, "timeout_seconds": timeout, "started_utc": started, "ended_utc": utc_now(),
                "stdout_sha256": sha256(stdout), "stderr_sha256": sha256(stderr)}


def require_success(record: dict[str, Any], context: str) -> None:
    if record["exit_code"] != 0:
        raise EvidenceError(f"{context} failed")


def required_inputs(arguments: argparse.Namespace) -> dict[str, pathlib.Path]:
    paths = {
        "profile": pathlib.Path(arguments.profile).resolve(),
        "protocol": pathlib.Path(arguments.protocol).resolve(),
        "validator": pathlib.Path(arguments.validator).resolve(),
        "architecture_evidence": pathlib.Path(arguments.architecture_evidence).resolve(),
        "architecture_runner": pathlib.Path(arguments.architecture_runner).resolve(),
        "architecture_profile": pathlib.Path(arguments.architecture_profile).resolve(),
        "architecture_expected": pathlib.Path(arguments.architecture_expected).resolve(),
        "architecture_comparer": pathlib.Path(arguments.architecture_comparer).resolve(),
        "launcher": pathlib.Path(__file__).resolve(),
    }
    for path in paths.values():
        if not path.is_file():
            raise EvidenceError(f"required input is absent: {path}")
    return paths


def architecture_regression_commands(arguments: argparse.Namespace, output_root: pathlib.Path) -> tuple[list[str], list[str]]:
    common = [sys.executable, str(pathlib.Path(arguments.architecture_runner).resolve()),
              "--source-root", str(pathlib.Path(arguments.source_root).resolve()),
              "--profile", str(pathlib.Path(arguments.architecture_profile).resolve()),
              "--protocol", str(pathlib.Path(arguments.architecture_evidence).resolve()),
              "--expected", str(pathlib.Path(arguments.architecture_expected).resolve()),
              "--comparer", str(pathlib.Path(arguments.architecture_comparer).resolve()),
              "--output-root", str(output_root / "architecture")]
    return common, [*common, "--execute"]


def prepare(arguments: argparse.Namespace) -> tuple[pathlib.Path, dict[str, Any]]:
    source_root, output_root = pathlib.Path(arguments.source_root).resolve(), pathlib.Path(arguments.output_root).resolve()
    if output_root.exists():
        raise EvidenceError(f"output root already exists: {output_root}")
    revision, inventory = clean_candidate(source_root)
    inputs = required_inputs(arguments)
    profile = validate_profile(inputs["profile"])
    output_root.mkdir(parents=True)
    architecture_prepare, architecture_execute = architecture_regression_commands(arguments, output_root)
    manifest = {
        "schema_version": 1, "protocol_version": profile["protocol_version"], "state": "PREPARED", "execution_requested": False,
        "candidate": {"commit": revision, "tree_clean": True, "source_root": str(source_root), "source_inventory": inventory},
        "inputs": {key: {"path": str(path), "sha256": sha256(path)} for key, path in inputs.items()},
        "environment": {"python": sys.version.splitlines()[0], "cmake": tool_version("cmake"), "ninja": tool_version("ninja"),
                        "gcc": tool_version("g++-13"), "clang": tool_version("clang++-18")},
        "limits_seconds": {"build": BUILD_TIMEOUT_SECONDS, "process": PROCESS_TIMEOUT_SECONDS, "overall": OVERALL_TIMEOUT_SECONDS},
        "plan": command_plan(profile, source_root),
        "architecture_preservation_plan": {"prepare": architecture_prepare, "execute": architecture_execute},
        "prepared_utc": utc_now(),
    }
    write_json(output_root / "manifest.json", manifest)
    write_json(output_root / "plan.json", {"protocol_version": profile["protocol_version"], "cells": manifest["plan"]})
    return output_root, manifest


def load_prepared(arguments: argparse.Namespace) -> tuple[pathlib.Path, dict[str, Any]]:
    output_root = pathlib.Path(arguments.output_root).resolve()
    try:
        manifest = json.loads(read_bytes(output_root / "manifest.json").decode("utf-8"))
    except (OSError, UnicodeDecodeError, json.JSONDecodeError) as error:
        raise EvidenceError("prepared manifest is invalid or absent") from error
    if not isinstance(manifest, dict) or manifest.get("state") != "PREPARED" or manifest.get("execution_requested") is not False:
        raise EvidenceError("manifest is not an executable PREPARED manifest")
    source_root = pathlib.Path(arguments.source_root).resolve()
    revision, inventory = clean_candidate(source_root)
    candidate = manifest.get("candidate")
    if not isinstance(candidate, dict) or candidate != {"commit": revision, "tree_clean": True, "source_root": str(source_root), "source_inventory": inventory}:
        raise EvidenceError("prepared manifest candidate differs")
    for key, path in required_inputs(arguments).items():
        if manifest.get("inputs", {}).get(key) != {"path": str(path), "sha256": sha256(path)}:
            raise EvidenceError(f"prepared manifest input differs: {key}")
    return output_root, manifest


def execute(output_root: pathlib.Path, manifest: dict[str, Any]) -> int:
    started, records = dt.datetime.now(dt.timezone.utc), []
    try:
        for cell in manifest["plan"]:
            if (dt.datetime.now(dt.timezone.utc) - started).total_seconds() > OVERALL_TIMEOUT_SECONDS:
                raise EvidenceError("overall timeout")
            cell_root = output_root / "cells" / cell["name"]
            cell_root.mkdir(parents=True)
            cell_records: list[dict[str, Any]] = []
            for stage in ("configure", "build"):
                command = [part.replace("@OUTPUT_ROOT@", str(output_root)) for part in cell[stage]]
                record = execute_command(command, output_root, BUILD_TIMEOUT_SECONDS, cell_root, stage)
                cell_records.append(record)
                require_success(record, f"{cell['name']} {stage}")
            ctest_command = [part.replace("@OUTPUT_ROOT@", str(output_root)) for part in cell["ctest"]]
            for repeat in range(cell["repetitions"]):
                record = execute_command(ctest_command, output_root, PROCESS_TIMEOUT_SECONDS, cell_root, f"ctest-{repeat + 1}")
                cell_records.append(record)
                require_success(record, f"{cell['name']} ctest repetition {repeat + 1}")
            build_root, exporter = cell_root / "build", cell_root / "build" / "apmesh_core_numeric_contract_export"
            certificates, environments = [], []
            for repeat in range(cell["repetitions"]):
                certificate, environment = cell_root / f"certificate-{repeat + 1}.json", cell_root / f"environment-{repeat + 1}.json"
                for mode, artifact in (("certificate", certificate), ("environment", environment)):
                    record = execute_command([str(exporter), mode, str(artifact)], cell_root, PROCESS_TIMEOUT_SECONDS, cell_root, f"{mode}-{repeat + 1}")
                    cell_records.append(record)
                    require_success(record, f"{cell['name']} {mode} {repeat + 1}")
                validate_certificate(certificate)
                validate_environment(environment, build_root / "compile_commands.json")
                certificates.append(str(certificate)); environments.append(str(environment))
            records.append({"cell": cell["name"], "state": "PASS", "certificates": certificates, "environments": environments, "records": cell_records})
        certificates = [path for record in records for path in record["certificates"]]
        environments = [path for record in records for path in record["environments"]]
        report = output_root / "report.md"
        command = [sys.executable, manifest["inputs"]["validator"]["path"], "compare", *sum((["--certificate", path] for path in certificates), []), *sum((["--environment", path] for path in environments), []), "--report", str(report)]
        comparison = execute_command(command, output_root, PROCESS_TIMEOUT_SECONDS, output_root, "compare")
        require_success(comparison, "cross-cell comparison")
        prepared_architecture, execute_architecture = architecture_regression_commands(
            argparse.Namespace(
                source_root=manifest["candidate"]["source_root"],
                architecture_runner=manifest["inputs"]["architecture_runner"]["path"],
                architecture_profile=manifest["inputs"]["architecture_profile"]["path"],
                architecture_evidence=manifest["inputs"]["architecture_evidence"]["path"],
                architecture_expected=manifest["inputs"]["architecture_expected"]["path"],
                architecture_comparer=manifest["inputs"]["architecture_comparer"]["path"],
            ), output_root)
        architecture_prepare = execute_command(prepared_architecture, output_root, PROCESS_TIMEOUT_SECONDS, output_root, "architecture-prepare")
        require_success(architecture_prepare, "architecture regression preparation")
        architecture_execute = execute_command(execute_architecture, output_root, OVERALL_TIMEOUT_SECONDS, output_root, "architecture-execute")
        require_success(architecture_execute, "architecture preservation regression")
        manifest["state"] = "EXECUTED_PENDING_AUDIT"
        manifest["execution"] = {"started_utc": started.isoformat(timespec="seconds"), "ended_utc": utc_now(), "records": records, "comparison": comparison,
                                 "architecture_preservation": {"prepare": architecture_prepare, "execute": architecture_execute,
                                                                 "manifest": str(output_root / "architecture" / "manifest.json")},
                                 "report_sha256": sha256(report)}
        manifest["gates"] = {gate: "EVIDENCE_COLLECTED_PENDING_AUDIT" for gate in ("N0", "N1", "N2", "N3", "N4", "N5", "N6", "N7")}
        write_json(output_root / "manifest.json", manifest)
        return 0
    except EvidenceError as error:
        manifest["state"] = "BLOCKED"
        manifest["execution"] = {"started_utc": started.isoformat(timespec="seconds"), "ended_utc": utc_now(), "records": records, "reason": str(error)}
        write_json(output_root / "manifest.json", manifest)
        return 1


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    for name in ("source-root", "profile", "protocol", "validator", "architecture-evidence", "architecture-runner", "architecture-profile", "architecture-expected", "architecture-comparer", "output-root"):
        parser.add_argument(f"--{name}", required=True)
    parser.add_argument("--execute", action="store_true")
    return parser.parse_args()


def main() -> int:
    arguments = parse_arguments()
    try:
        if arguments.execute:
            root, manifest = load_prepared(arguments)
            return execute(root, manifest)
        prepare(arguments)
        return 0
    except EvidenceError as error:
        print(error, file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
