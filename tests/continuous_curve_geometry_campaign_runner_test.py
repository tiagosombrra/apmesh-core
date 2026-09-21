#!/usr/bin/env python3
"""Focused contracts for the formal Continuous Curve Geometry campaign runner."""

from __future__ import annotations

import argparse
import importlib.util
import json
import pathlib
import subprocess
import sys
import tempfile
from unittest.mock import patch


def load(path: pathlib.Path):
    specification = importlib.util.spec_from_file_location("cgr_campaign", path)
    if specification is None or specification.loader is None:
        raise RuntimeError("formal CGR runner module could not be loaded")
    module = importlib.util.module_from_spec(specification)
    specification.loader.exec_module(module)
    return module


def rejects(action, message: str) -> None:
    try:
        action()
    except RuntimeError:
        return
    raise RuntimeError(message)


def snapshot(root: pathlib.Path) -> dict[str, bytes]:
    return {
        path.relative_to(root).as_posix(): path.read_bytes()
        for path in sorted(root.rglob("*"))
        if path.is_file()
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--runner", required=True)
    parser.add_argument("--source-root", required=True)
    parser.add_argument("--profile", required=True)
    parser.add_argument("--protocol", required=True)
    parser.add_argument("--exporter", required=True)
    parser.add_argument("--certificate-exporter", required=True)
    parser.add_argument("--validator", required=True)
    parser.add_argument("--negative", required=True)
    arguments = parser.parse_args()

    runner = load(pathlib.Path(arguments.runner))
    source = pathlib.Path(arguments.source_root).resolve()
    profile = json.loads(pathlib.Path(arguments.profile).read_text(encoding="utf-8"))

    declared = runner.report_runner.declared_allowlist(
        source,
        profile["semantic_ctest_allowlist"],
    )
    report_plan = runner.report_runner.command_plan(profile, source, declared)
    formal_plan = runner.formal_plan(profile, source, declared)

    if runner.report_runner.command_ids(report_plan) != runner.report_runner.command_ids(formal_plan):
        raise RuntimeError("formal plan command identities differ from report-only authority")
    if len(runner.report_runner.command_ids(formal_plan)) != 56:
        raise RuntimeError("formal plan command cardinality differs")

    self_check_args = argparse.Namespace(
        source_root=arguments.source_root,
        profile=arguments.profile,
        protocol=arguments.protocol,
        exporter=arguments.exporter,
        validator=arguments.validator,
        negative=arguments.negative,
    )
    checked = runner.self_check(self_check_args)
    if (
        checked["status"] != "PASS"
        or checked["execution_requested"] is not False
        or checked["formal_preparation"] is not False
        or checked["formal_execution"] is not False
        or set(checked["plan"]["gates"].values()) != {"NOT_EXECUTED"}
    ):
        raise RuntimeError("formal self-check claims execution or gate closure")

    candidate = {
        "commit": "0123456789abcdef0123456789abcdef01234567",
        "upstream_commit": "0123456789abcdef0123456789abcdef01234567",
        "semantic_baseline": runner.SEMANTIC_BASELINE,
        "tree_clean": True,
        "source_root": str(source),
        "source_inventory": [],
    }
    fake_cloud_binding = {
        "schema_version": 1,
        "kind": "continuous-curve-geometry-cloud-environment-binding",
        "profile_sha256": "profile",
        "validator_sha256": "validator",
        "decision_sha256": "decision",
        "audit_sha256": "audit",
        "observations": [
            {"cell": cell["id"], "status": "PASS"}
            for cell in profile["cells"]
        ],
    }
    detached = {
        "result": "PASS",
        "candidate_commit": candidate["commit"],
        "source_inventory_count": 0,
    }

    def args_for(output: pathlib.Path) -> argparse.Namespace:
        return argparse.Namespace(
            source_root=arguments.source_root,
            profile=arguments.profile,
            protocol=arguments.protocol,
            exporter=arguments.exporter,
            validator=arguments.validator,
            negative=arguments.negative,
            output_root=str(output),
        )

    with tempfile.TemporaryDirectory(prefix="apmesh-cgr-campaign-") as temporary:
        root = pathlib.Path(temporary)

        prepared = root / "prepared"
        prepared_args = args_for(prepared)
        with (
            patch.object(runner, "published_candidate", return_value=candidate),
            patch.object(runner, "environment_identity", return_value={"test": "identity"}),
            patch.object(runner, "cloud_environment_binding", return_value=fake_cloud_binding),
        ):
            runner.prepare(prepared_args)
            manifest = runner.validate_prepared(prepared)
            if set(path.name for path in prepared.iterdir() if path.is_file()) != {
                "profile.json",
                "plan.json",
                "planned-inventories.json",
                "prepared-manifest.json",
                "preparation-seal.json",
                "state.json",
                "state-history.jsonl",
            }:
                raise RuntimeError("PREPARED package does not contain exactly seven files")
            if (
                manifest["execution_requested"] is not False
                or set(manifest["gates"].values()) != {"NOT_EXECUTED"}
                or manifest["candidate"] != candidate
                or manifest["cloud_environment"] != fake_cloud_binding
            ):
                raise RuntimeError("PREPARED manifest binding differs")
            if [item["path"] for item in manifest["frozen_semantic_files"]] != profile["frozen_semantic_files"]:
                raise RuntimeError("frozen semantic inventory differs")
            if any(
                item["sha256"] != item["baseline_sha256"]
                for item in manifest["frozen_semantic_files"]
            ):
                raise RuntimeError("frozen semantic file differs from baseline")

            runner.validate_execution_binding(prepared_args, source, prepared, manifest)

            planned = manifest["planned_inventories"]["artifacts"]
            logs = {item["path"] for item in planned if item["role"] == "command-log"}
            certificates = {
                item["path"] for item in planned if item["role"] == "semantic-certificate"
            }
            derived = {
                item["path"] for item in planned if item["role"] == "derived-evidence"
            }
            if len(logs) != 112 or len(certificates) != 8:
                raise RuntimeError("planned formal evidence cardinality differs")
            if derived != {
                f"derived/{name}" for name in runner.report_runner.EXPECTED_DERIVED_FILES
            }:
                raise RuntimeError("planned derived evidence differs")

            saved_manifest = (prepared / "prepared-manifest.json").read_bytes()
            forged = json.loads(saved_manifest)
            forged["execution_requested"] = True
            (prepared / "prepared-manifest.json").write_text(
                json.dumps(forged),
                encoding="utf-8",
            )
            rejects(
                lambda: runner.validate_prepared(prepared),
                "mutated PREPARED manifest was accepted",
            )
            (prepared / "prepared-manifest.json").write_bytes(saved_manifest)

            (prepared / "execution-claim.json").write_text("{}\n", encoding="utf-8")
            rejects(
                lambda: runner.validate_prepared(prepared),
                "consumed PREPARED package was accepted as unconsumed",
            )
            (prepared / "execution-claim.json").unlink()

        cloud_drift = root / "cloud-drift"
        with (
            patch.object(runner, "published_candidate", return_value=candidate),
            patch.object(runner, "environment_identity", return_value={"test": "identity"}),
            patch.object(
                runner,
                "cloud_environment_binding",
                side_effect=runner.CampaignError("intentional cloud drift"),
            ),
        ):
            rejects(
                lambda: runner.prepare(args_for(cloud_drift)),
                "cloud drift did not fail preparation",
            )

        existing = root / "existing"
        existing.mkdir()
        with (
            patch.object(runner, "published_candidate", return_value=candidate),
            patch.object(runner, "environment_identity", return_value={"test": "identity"}),
            patch.object(runner, "cloud_environment_binding", return_value=fake_cloud_binding),
        ):
            rejects(
                lambda: runner.prepare(args_for(existing)),
                "existing output root was accepted",
            )

        def simulated_command(
            output_root: pathlib.Path,
            fail_record_id: str | None = None,
        ):
            def command(
                argv,
                cwd,
                logs_root,
                record_id,
                timeout_seconds,
                environment_delta=None,
            ):
                logs_root.mkdir(parents=True, exist_ok=True)
                stdout = logs_root / f"{record_id}.stdout.log"
                stderr = logs_root / f"{record_id}.stderr.log"
                stdout.write_text("", encoding="utf-8")
                stderr.write_text("", encoding="utf-8")

                if "ctest-discovery" in record_id:
                    stdout.write_text(
                        "\n".join(
                            f"  Test  #{index}: {name}"
                            for index, name in enumerate(
                                profile["semantic_ctest_allowlist"],
                                1,
                            )
                        )
                        + "\n",
                        encoding="utf-8",
                    )

                if "semantic-ctest" in record_id:
                    stdout.write_text(
                        "\n".join(
                            f" {index}/14 Test #{index}: {name} .... Passed 0.00 sec"
                            for index, name in enumerate(
                                profile["semantic_ctest_allowlist"],
                                1,
                            )
                        )
                        + "\n",
                        encoding="utf-8",
                    )

                if "certificate-" in record_id and "validation" not in record_id:
                    completed = subprocess.run(
                        [arguments.certificate_exporter, *argv[1:]],
                        capture_output=True,
                        text=True,
                        check=False,
                    )
                    if completed.returncode != 0:
                        raise RuntimeError(completed.stderr)

                if record_id.endswith("-negative-evidence"):
                    completed = subprocess.run(
                        argv,
                        capture_output=True,
                        text=True,
                        check=False,
                    )
                    if completed.returncode != 0:
                        raise RuntimeError(completed.stderr)

                if record_id.endswith("-configure"):
                    cell_id = record_id.removesuffix("-configure")
                    build = output_root / "cells" / cell_id / "build"
                    build.mkdir(parents=True, exist_ok=True)
                    (build / "compile_commands.json").write_text(
                        "[]\n",
                        encoding="utf-8",
                    )

                failed = record_id == fail_record_id
                if failed:
                    stderr.write_text(
                        "intentional focused mid-campaign failure\n",
                        encoding="utf-8",
                    )

                return {
                    "schema_version": 1,
                    "kind": "experiment-command-record",
                    "id": record_id,
                    "argv": argv,
                    "cwd": str(pathlib.Path(cwd).resolve()),
                    "environment_delta": environment_delta or {},
                    "started_utc": "2026-01-01T00:00:00+00:00",
                    "ended_utc": "2026-01-01T00:00:00+00:00",
                    "elapsed_seconds": 0.0,
                    "pid": 1,
                    "timeout_seconds": timeout_seconds,
                    "timed_out": False,
                    "exit_code": 1 if failed else 0,
                    "launch_error": None,
                    "stdout": {
                        "path": f"logs/{record_id}.stdout.log",
                        "sha256": runner.sha256_file(stdout),
                    },
                    "stderr": {
                        "path": f"logs/{record_id}.stderr.log",
                        "sha256": runner.sha256_file(stderr),
                    },
                }

            return command

        failed = root / "failed"
        failed_args = args_for(failed)
        with (
            patch.object(runner, "published_candidate", return_value=candidate),
            patch.object(runner, "environment_identity", return_value={"test": "identity"}),
            patch.object(runner, "cloud_environment_binding", return_value=fake_cloud_binding),
            patch.object(
                runner,
                "run_command",
                side_effect=simulated_command(
                    failed,
                    "gcc-debug-semantic-ctest-2",
                ),
            ),
            patch.object(runner, "verify_detached_candidate", return_value=detached),
        ):
            runner.prepare(failed_args)
            if runner.execute(failed_args) != 1:
                raise RuntimeError("synthetic mid-campaign failure was not reported")
            if runner.read_json(failed / "state.json")["state"] != "BLOCKED":
                raise RuntimeError("synthetic failure did not close as BLOCKED")
            if runner.verify_retention(failed)["status"] != "PASS":
                raise RuntimeError("blocked terminal retention did not verify")

            failure_ids = [
                record["id"]
                for record in runner.read_json(failed / "command-records.json")["records"]
            ]
            if failure_ids[-1] != "gcc-debug-semantic-ctest-2":
                raise RuntimeError("synthetic failure occurred at a different command")
            if "gcc-debug-certificate-2" in failure_ids:
                raise RuntimeError("runner continued after semantic failure")
            if any(item.startswith("gcc-release-") for item in failure_ids):
                raise RuntimeError("runner continued into a later cell after failure")

            before = snapshot(failed)
            if runner.execute(failed_args) != 1:
                raise RuntimeError("consumed blocked package was not rejected")
            if snapshot(failed) != before:
                raise RuntimeError("consumed blocked package was mutated")

        succeeded = root / "succeeded"
        success_args = args_for(succeeded)
        with (
            patch.object(runner, "published_candidate", return_value=candidate),
            patch.object(runner, "environment_identity", return_value={"test": "identity"}),
            patch.object(runner, "cloud_environment_binding", return_value=fake_cloud_binding),
            patch.object(runner, "run_command", side_effect=simulated_command(succeeded)),
            patch.object(runner, "verify_detached_candidate", return_value=detached),
        ):
            runner.prepare(success_args)
            if runner.execute(success_args) != 0:
                raise RuntimeError("synthetic successful execution failed")
            if runner.verify_retention(succeeded)["status"] != "PASS":
                raise RuntimeError("successful terminal retention did not verify")

        records = runner.read_json(succeeded / "command-records.json")["records"]
        ids = [record["id"] for record in records]
        if len(ids) != 56 or len(set(ids)) != 56:
            raise RuntimeError("successful command-record cardinality differs")
        if any(
            record["exit_code"] != 0
            or record["timed_out"]
            or record["launch_error"] is not None
            for record in records
        ):
            raise RuntimeError("successful synthetic package contains failed commands")

        semantic = [record for record in records if "-semantic-ctest-" in record["id"]]
        discoveries = [
            record for record in records if "-ctest-discovery-" in record["id"]
        ]
        certificates = runner.read_json(succeeded / "certificate-index.json")["entries"]
        if len(semantic) != 8 or len(discoveries) != 8 or len(certificates) != 8:
            raise RuntimeError("successful repeated scientific evidence cardinality differs")

        for record in semantic:
            runner.require_exact_semantic_ctest_execution(
                succeeded,
                record,
                profile["semantic_ctest_allowlist"],
            )

        observed = runner.read_json(succeeded / "observed-inventories.json")
        if len(observed["discoveries"]) != 8:
            raise RuntimeError("observed discovery inventory differs")
        if {
            (item["cell"], item["repetition"])
            for item in observed["discoveries"]
        } != {
            (cell["id"], repetition)
            for cell in profile["cells"]
            for repetition in (1, 2)
        }:
            raise RuntimeError("observed repetition identity differs")

        planned_logs = {
            item["path"]
            for item in runner.read_json(succeeded / "prepared-manifest.json")[
                "planned_inventories"
            ]["artifacts"]
            if item["role"] == "command-log"
        }
        actual_logs = {
            path.relative_to(succeeded).as_posix()
            for path in (succeeded / "logs").iterdir()
            if path.is_file()
        }
        if len(planned_logs) != 112 or actual_logs != planned_logs:
            raise RuntimeError("successful command-log inventory differs")

        derived = {
            path.name
            for path in (succeeded / "derived").iterdir()
            if path.is_file()
        }
        if derived != set(runner.report_runner.EXPECTED_DERIVED_FILES):
            raise RuntimeError("successful derived evidence inventory differs")

        gate_summary = runner.read_json(succeeded / "gate-summary.json")
        if set(gate_summary["gates"].values()) != {
            "EVIDENCE_COLLECTED_PENDING_AUDIT"
        }:
            raise RuntimeError("formal runner claims CGR gate closure")

        before = snapshot(succeeded)
        if runner.execute(success_args) != 1:
            raise RuntimeError("consumed successful package was not rejected")
        if snapshot(succeeded) != before:
            raise RuntimeError("consumed successful package was mutated")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
