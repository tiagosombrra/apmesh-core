#!/usr/bin/env python3
"""Single focused tooling contract for TMR evidence and runner lifecycle."""
from __future__ import annotations

import argparse
import hashlib
import importlib.util
import json
import pathlib
import subprocess
import sys
import tempfile
from unittest.mock import patch


def load(path: str):
    specification = importlib.util.spec_from_file_location("tmr_runner", path)
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


def run_checked(command: list[str], expected: int = 0) -> None:
    completed = subprocess.run(command, capture_output=True, text=True, check=False)
    if completed.returncode != expected:
        raise RuntimeError(
            f"unexpected exit {completed.returncode}: {' '.join(command)}\\n"
            f"{completed.stdout}\\n{completed.stderr}"
        )


def exercise_evidence_contract(exporter: str, tool: str, profile_path: str) -> None:
    common = [sys.executable, tool, "--profile", profile_path]
    with tempfile.TemporaryDirectory(prefix="apmesh-core-tmr-evidence-") as temporary:
        root = pathlib.Path(temporary)
        certificate = root / "certificate.json"

        run_checked([exporter, "certificate", str(certificate)])
        run_checked([*common, "validate-profile"])
        run_checked([*common, "validate-certificate", "--certificate", str(certificate)])

        profile = json.loads(pathlib.Path(profile_path).read_text(encoding="utf-8"))
        forged_profile = dict(profile)
        forged_profile["limitations"] = list(profile["limitations"])
        forged_profile["limitations"][0] = "Cloud execution silently replaces the pre-registered WSL scope"
        forged_profile_path = root / "forged-profile.json"
        forged_profile_path.write_text(json.dumps(forged_profile), encoding="utf-8")
        run_checked(
            [sys.executable, tool, "--profile", str(forged_profile_path), "validate-profile"],
            expected=1,
        )

        entries = []
        for cell in profile["cells"]:
            for repetition in range(1, profile["repetitions_per_cell"] + 1):
                copied = root / f"{cell['id']}-{repetition}.json"
                copied.write_bytes(certificate.read_bytes())
                entries.append({
                    "cell": cell["id"],
                    "repetition": repetition,
                    "path": copied.name,
                    "sha256": hashlib.sha256(copied.read_bytes()).hexdigest(),
                })

        index = root / "certificate-index.json"
        index.write_text(json.dumps({
            "schema_version": 1,
            "kind": "topological-model-cumulative-certificate-index",
            "entries": entries,
        }), encoding="utf-8")

        comparison = root / "comparison.json"
        run_checked([*common, "compare", "--index", str(index), "--output", str(comparison)])
        observed = json.loads(comparison.read_text(encoding="utf-8"))
        if (
            observed["status"] != "EVIDENCE_COLLECTED_PENDING_AUDIT"
            or set(observed["gates"].values()) != {"NOT_EXECUTED"}
            or observed["certificate_count"] != 8
        ):
            raise RuntimeError("report-only comparison claims a scientific gate result")

        collected = root / "collected.json"
        run_checked([*common, "collect", "--index", str(index), "--output", str(collected)])
        if json.loads(collected.read_text(encoding="utf-8"))["status"] != "EVIDENCE_COLLECTED_PENDING_AUDIT":
            raise RuntimeError("collector claims scientific closure")

        outcomes = root / "negative-outcomes.json"
        run_checked([*common, "negative-outcomes", "--output", str(outcomes)])
        run_checked([*common, "validate-negative-outcomes", "--outcomes", str(outcomes)])
        forged_outcomes = json.loads(outcomes.read_text(encoding="utf-8"))
        forged_outcomes["outcomes"][0]["result"] = "ACCEPTED"
        forged_outcomes_path = root / "forged-negative-outcomes.json"
        forged_outcomes_path.write_text(json.dumps(forged_outcomes), encoding="utf-8")
        run_checked([*common, "validate-negative-outcomes", "--outcomes", str(forged_outcomes_path)], expected=1)

        forged = json.loads(certificate.read_text(encoding="utf-8"))
        snapshot_case = next(item for item in forged["cases"] if item["id"] == "canonical_snapshot")
        snapshot_case["observed"]["snapshot"] = "forged\\n"
        forged_path = root / "forged-snapshot.json"
        forged_path.write_text(json.dumps(forged), encoding="utf-8")
        run_checked([*common, "validate-certificate", "--certificate", str(forged_path)], expected=1)

        forged = json.loads(certificate.read_text(encoding="utf-8"))
        incidence_case = next(item for item in forged["cases"] if item["id"] == "incidence_bijection")
        incidence_case["observed"]["exact_bijection"] = False
        forged_path = root / "forged-incidence.json"
        forged_path.write_text(json.dumps(forged), encoding="utf-8")
        run_checked([*common, "validate-certificate", "--certificate", str(forged_path)], expected=1)

        forged = json.loads(certificate.read_text(encoding="utf-8"))
        forged["cases"].append(forged["cases"][0])
        forged_path = root / "duplicate-case.json"
        forged_path.write_text(json.dumps(forged), encoding="utf-8")
        run_checked([*common, "validate-certificate", "--certificate", str(forged_path)], expected=1)

        forged = json.loads(certificate.read_text(encoding="utf-8"))
        forged["non_claims"] = list(forged["non_claims"]) + ["qualified_manifoldness"]
        forged_path = root / "forged-non-claim.json"
        forged_path.write_text(json.dumps(forged), encoding="utf-8")
        run_checked([*common, "validate-certificate", "--certificate", str(forged_path)], expected=1)


def main() -> int:
    parser = argparse.ArgumentParser()
    for name in ("runner", "source-root", "profile", "protocol", "exporter", "certificate-exporter", "validator"):
        parser.add_argument(f"--{name}", required=True)
    arguments = parser.parse_args()
    exercise_evidence_contract(arguments.certificate_exporter, arguments.validator, arguments.profile)
    runner = load(arguments.runner)
    source = pathlib.Path(arguments.source_root).resolve()
    profile = json.loads(pathlib.Path(arguments.profile).read_text(encoding="utf-8"))
    if sorted(runner.declared_allowlist(source, profile["semantic_ctest_allowlist"])) != sorted(profile["semantic_ctest_allowlist"]):
        raise RuntimeError("declared allowlist differs")
    expression = runner.ctest_regex(profile["semantic_ctest_allowlist"])
    if "(?:" in expression or not expression.startswith("^(") or not expression.endswith(")$"):
        raise RuntimeError("semantic CTest expression is not CTest-compatible")
    common = [sys.executable, arguments.runner, "--source-root", arguments.source_root, "--profile", arguments.profile, "--protocol", arguments.protocol, "--exporter", arguments.exporter, "--validator", arguments.validator]
    with tempfile.TemporaryDirectory(prefix="apmesh-core-tmr-runner-") as temporary:
        root = pathlib.Path(temporary)
        self_check = root / "self-check.json"
        result = subprocess.run([*common, "self-check", "--output", str(self_check)], capture_output=True, text=True, check=False)
        if result.returncode != 0 or json.loads(self_check.read_text(encoding="utf-8"))["execution_requested"] is not False:
            raise RuntimeError("runner self-check differs")
        self_check_value = json.loads(self_check.read_text(encoding="utf-8"))
        if any("-DAPMESH_ENABLE_QUALIFICATION_TESTS=ON" not in cell["configure"] for cell in self_check_value["plan"]["cells"]):
            raise RuntimeError("runner does not explicitly opt in to qualification tooling")
        if any(cell["repetitions"] != 2 for cell in self_check_value["plan"]["cells"]):
            raise RuntimeError("runner repetition count differs")
        if len(self_check_value["declared_semantic_ctest_allowlist"]) != 7:
            raise RuntimeError("runner semantic allowlist cardinality differs")
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
        semantic_record = {"stdout": {"path": "logs/semantic.stdout.log"}, "stderr": {"path": "logs/semantic.stderr.log"}}
        semantic_stdout, semantic_stderr = discovery / semantic_record["stdout"]["path"], discovery / semantic_record["stderr"]["path"]
        semantic_stdout.write_text("\n".join(f" {index}/7 Test #{index}: {name} .... Passed 0.00 sec" for index, name in enumerate(profile["semantic_ctest_allowlist"], 1)), encoding="utf-8")
        semantic_stderr.write_text("", encoding="utf-8")
        runner.require_exact_semantic_ctest_execution(discovery, semantic_record, profile["semantic_ctest_allowlist"])
        semantic_stdout.write_text("RegularExpression::compile(): ?+* follows nothing.\n", encoding="utf-8")
        semantic_stderr.write_text("No tests were found!!!\n", encoding="utf-8")
        rejects(lambda: runner.require_exact_semantic_ctest_execution(discovery, semantic_record, profile["semantic_ctest_allowlist"]), "empty semantic CTest selection was accepted")
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
            if "semantic-ctest" in record_id:
                stdout.write_text("\n".join(f" {index}/7 Test #{index}: {name} .... Passed 0.00 sec" for index, name in enumerate(profile["semantic_ctest_allowlist"], 1)), encoding="utf-8")
            if "certificate-" in record_id and "validation" not in record_id:
                destination = pathlib.Path(argv[-1])
                if not destination.parent.is_dir():
                    raise RuntimeError("runner did not create certificate output directory")
                completed = subprocess.run([arguments.certificate_exporter, "certificate", str(destination)], capture_output=True, text=True, check=False)
                if completed.returncode != 0:
                    raise RuntimeError(completed.stderr)
            if "negative-outcomes" in record_id:
                destination = pathlib.Path(argv[-1])
                if not destination.parent.is_dir():
                    raise RuntimeError("runner did not create negative-output directory")
                destination.write_text(json.dumps({"schema_version": 1, "kind": "topological-model-cumulative-negative-outcomes", "outcomes": [{"id": item, "result": "REJECTED"} for item in profile["certificate_negative_cases"]]}), encoding="utf-8")
            if "configure" in record_id:
                build = succeeded / "cells" / record_id.removesuffix("-configure") / "build"; build.mkdir(parents=True, exist_ok=True)
                (build / "compile_commands.json").write_text("[]", encoding="utf-8")
            return {"schema_version": 1, "kind": "experiment-command-record", "id": record_id, "argv": argv, "cwd": str(pathlib.Path(cwd).resolve()), "environment_delta": environment_delta or {}, "started_utc": "2026-01-01T00:00:00+00:00", "ended_utc": "2026-01-01T00:00:00+00:00", "elapsed_seconds": 0.0, "pid": 1, "timeout_seconds": timeout_seconds, "timed_out": False, "exit_code": 0, "launch_error": None, "stdout": {"path": f"logs/{record_id}.stdout.log", "sha256": runner.sha256_file(stdout)}, "stderr": {"path": f"logs/{record_id}.stderr.log", "sha256": runner.sha256_file(stderr)}}

        with patch.object(runner, "published_candidate", return_value=candidate), patch.object(runner, "environment_identity", return_value={"test": "identity"}), patch.object(runner, "run_command", side_effect=successful_command), patch.object(runner, "verify_detached_candidate", return_value=detached):
            runner.prepare(success_args)
            if runner.execute(success_args) != 0 or runner.verify_retention(succeeded)["status"] != "PASS":
                raise RuntimeError("focused successful terminal package was not retained")
            records = runner.read_json(succeeded / "command-records.json")["records"]
            semantic_records = [record for record in records if "-semantic-ctest-" in record["id"]]
            expected_semantic_records = len(profile["cells"]) * profile["repetitions_per_cell"]
            if len(semantic_records) != expected_semantic_records:
                raise RuntimeError("semantic CTest was not executed exactly once in every repetition")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
