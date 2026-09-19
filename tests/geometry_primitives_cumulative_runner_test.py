#!/usr/bin/env python3
"""Focused preparation-only contracts for the GPR manifest runner."""
from __future__ import annotations

import argparse
import importlib.util
import json
import pathlib
import subprocess
import sys
import tempfile
from unittest.mock import patch


def load(path: str):
    specification = importlib.util.spec_from_file_location("gpr_runner", path)
    if specification is None or specification.loader is None:
        raise RuntimeError("runner module could not be loaded")
    module = importlib.util.module_from_spec(specification)
    specification.loader.exec_module(module)
    return module


def rejects(action, message: str) -> None:
    try:
        action()
    except RuntimeError:
        return
    raise RuntimeError(message)


def main() -> int:
    parser = argparse.ArgumentParser()
    for name in ("runner", "source-root", "profile", "protocol", "exporter", "certificate-exporter", "validator"):
        parser.add_argument(f"--{name}", required=True)
    arguments = parser.parse_args()
    runner = load(arguments.runner)
    source = pathlib.Path(arguments.source_root).resolve()
    profile = json.loads(pathlib.Path(arguments.profile).read_text(encoding="utf-8"))
    if sorted(runner.declared_allowlist(source, profile["semantic_ctest_allowlist"])) != sorted(profile["semantic_ctest_allowlist"]):
        raise RuntimeError("declared allowlist differs")
    common = [sys.executable, arguments.runner, "--source-root", arguments.source_root, "--profile", arguments.profile, "--protocol", arguments.protocol, "--exporter", arguments.exporter, "--validator", arguments.validator]
    with tempfile.TemporaryDirectory(prefix="apmesh-core-gpr-runner-") as temporary:
        root = pathlib.Path(temporary)
        self_check = root / "self-check.json"
        result = subprocess.run([*common, "self-check", "--output", str(self_check)], capture_output=True, text=True, check=False)
        if result.returncode != 0 or json.loads(self_check.read_text(encoding="utf-8"))["execution_requested"] is not False:
            raise RuntimeError("runner self-check differs")
        discovery = root / "discovery"
        (discovery / "logs").mkdir(parents=True)
        discovery_record = {"stdout": {"path": "logs/ctest.stdout.log"}}
        discovery_log = discovery / discovery_record["stdout"]["path"]
        discovery_log.write_text("  Test #1: alpha\n  Test  #2: beta\n Test    #10: gamma\n", encoding="utf-8")
        if runner.discovered_tests_from_log(discovery, discovery_record) != ["alpha", "beta", "gamma"]:
            raise RuntimeError("variable CTest spacing was not accepted")
        discovery_log.write_text("Test #1 alpha\nprefix Test #2: beta\nTest 2: gamma\nTest #3:\ndelta\n", encoding="utf-8")
        if runner.discovered_tests_from_log(discovery, discovery_record):
            raise RuntimeError("malformed CTest discovery was accepted")
        candidate = {"commit": "0123456789abcdef0123456789abcdef01234567", "upstream_commit": "0123456789abcdef0123456789abcdef01234567", "post_merge_baseline": runner.POST_MERGE_BASELINE, "tree_clean": True, "source_root": str(source), "source_inventory": []}
        output = root / "prepared"
        prepared = argparse.Namespace(source_root=arguments.source_root, profile=arguments.profile, protocol=arguments.protocol, exporter=arguments.exporter, validator=arguments.validator, output_root=str(output))
        with patch.object(runner, "published_candidate", return_value=candidate), patch.object(runner, "environment_identity", return_value={"test": "identity"}):
            runner.prepare(prepared)
            manifest = runner.validate_prepared(output)
            if manifest["execution_requested"] is not False or set(manifest["gates"].values()) != {"NOT_EXECUTED"}:
                raise RuntimeError("prepared manifest claims execution")
            runner.validate_execution_binding(prepared, source, output, manifest)
            saved = (output / "prepared-manifest.json").read_bytes()
            forged = json.loads(saved); forged["execution_requested"] = True
            (output / "prepared-manifest.json").write_text(json.dumps(forged), encoding="utf-8")
            rejects(lambda: runner.validate_prepared(output), "enabled execution was accepted")
            (output / "prepared-manifest.json").write_bytes(saved)
            (output / "execution-claim.json").write_text("{}", encoding="utf-8")
            rejects(lambda: runner.validate_prepared(output), "consumed manifest was accepted")
            (output / "execution-claim.json").unlink()
        existing = root / "existing"; existing.mkdir()
        rejected = argparse.Namespace(source_root=arguments.source_root, profile=arguments.profile, protocol=arguments.protocol, exporter=arguments.exporter, validator=arguments.validator, output_root=str(existing))
        rejects(lambda: runner.prepare(rejected), "existing output root was accepted")
        failed = root / "failed"
        failed_args = argparse.Namespace(source_root=arguments.source_root, profile=arguments.profile, protocol=arguments.protocol, exporter=arguments.exporter, validator=arguments.validator, output_root=str(failed))

        def failed_command(argv, cwd, logs_root, record_id, timeout_seconds, environment_delta=None):
            logs_root.mkdir(parents=True, exist_ok=True)
            stdout, stderr = logs_root / f"{record_id}.stdout.log", logs_root / f"{record_id}.stderr.log"
            stdout.write_bytes(b""); stderr.write_bytes(b"intentional focused command failure\n")
            return {"schema_version": 1, "kind": "experiment-command-record", "id": record_id, "argv": argv, "cwd": str(pathlib.Path(cwd).resolve()), "environment_delta": environment_delta or {}, "started_utc": "2026-01-01T00:00:00+00:00", "ended_utc": "2026-01-01T00:00:00+00:00", "elapsed_seconds": 0.0, "pid": 1, "timeout_seconds": timeout_seconds, "timed_out": False, "exit_code": 1, "launch_error": None, "stdout": {"path": f"logs/{record_id}.stdout.log", "sha256": runner.sha256_file(stdout)}, "stderr": {"path": f"logs/{record_id}.stderr.log", "sha256": runner.sha256_file(stderr)}}

        detached = {"result": "PASS", "candidate_commit": candidate["commit"], "source_inventory_count": 0}
        with patch.object(runner, "published_candidate", return_value=candidate), patch.object(runner, "environment_identity", return_value={"test": "identity"}), patch.object(runner, "run_command", side_effect=failed_command), patch.object(runner, "verify_detached_candidate", return_value=detached):
            runner.prepare(failed_args)
            if runner.execute(failed_args) != 1:
                raise RuntimeError("focused terminal failure was not reported")
            if runner.read_json(failed / "state.json")["state"] != "BLOCKED" or runner.verify_retention(failed)["status"] != "PASS":
                raise RuntimeError("focused terminal failure was not retained")
            before = {path.relative_to(failed).as_posix(): path.read_bytes() for path in failed.rglob("*") if path.is_file()}
            if runner.execute(failed_args) != 1:
                raise RuntimeError("consumed failed attempt was not rejected")
            after = {path.relative_to(failed).as_posix(): path.read_bytes() for path in failed.rglob("*") if path.is_file()}
            if after != before:
                raise RuntimeError("consumed failed attempt was reused")

        succeeded = root / "succeeded"
        success_args = argparse.Namespace(source_root=arguments.source_root, profile=arguments.profile, protocol=arguments.protocol, exporter=arguments.exporter, validator=arguments.validator, output_root=str(succeeded))

        def successful_command(argv, cwd, logs_root, record_id, timeout_seconds, environment_delta=None):
            logs_root.mkdir(parents=True, exist_ok=True)
            stdout, stderr = logs_root / f"{record_id}.stdout.log", logs_root / f"{record_id}.stderr.log"
            stdout.write_bytes(b""); stderr.write_bytes(b"")
            if "ctest-discovery" in record_id:
                stdout.write_text("\n".join(f"  Test  #{index}: {name}" for index, name in enumerate(profile["semantic_ctest_allowlist"], 1)), encoding="utf-8")
            if "certificate-" in record_id and "validation" not in record_id:
                destination = pathlib.Path(argv[-1]); destination.parent.mkdir(parents=True, exist_ok=True)
                completed = subprocess.run([arguments.certificate_exporter, "certificate", str(destination)], capture_output=True, text=True, check=False)
                if completed.returncode != 0:
                    raise RuntimeError(completed.stderr)
            if "negative-outcomes" in record_id:
                destination = pathlib.Path(argv[-1]); destination.parent.mkdir(parents=True, exist_ok=True)
                destination.write_text(json.dumps({"schema_version": 1, "kind": "geometry-primitives-cumulative-negative-outcomes", "outcomes": [{"id": item, "result": "REJECTED"} for item in profile["certificate_negative_cases"]]}), encoding="utf-8")
            if "configure" in record_id:
                build = succeeded / "cells" / record_id.removesuffix("-configure") / "build"; build.mkdir(parents=True, exist_ok=True)
                (build / "compile_commands.json").write_text("[]", encoding="utf-8")
            return {"schema_version": 1, "kind": "experiment-command-record", "id": record_id, "argv": argv, "cwd": str(pathlib.Path(cwd).resolve()), "environment_delta": environment_delta or {}, "started_utc": "2026-01-01T00:00:00+00:00", "ended_utc": "2026-01-01T00:00:00+00:00", "elapsed_seconds": 0.0, "pid": 1, "timeout_seconds": timeout_seconds, "timed_out": False, "exit_code": 0, "launch_error": None, "stdout": {"path": f"logs/{record_id}.stdout.log", "sha256": runner.sha256_file(stdout)}, "stderr": {"path": f"logs/{record_id}.stderr.log", "sha256": runner.sha256_file(stderr)}}

        with patch.object(runner, "published_candidate", return_value=candidate), patch.object(runner, "environment_identity", return_value={"test": "identity"}), patch.object(runner, "run_command", side_effect=successful_command), patch.object(runner, "verify_detached_candidate", return_value=detached):
            runner.prepare(success_args)
            if runner.execute(success_args) != 0 or runner.verify_retention(succeeded)["status"] != "PASS":
                raise RuntimeError("focused successful terminal package was not retained")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
