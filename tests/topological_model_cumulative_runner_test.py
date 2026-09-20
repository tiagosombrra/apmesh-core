#!/usr/bin/env python3
"""Focused contracts for the Topological Model cumulative manifest runner."""
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


def main() -> int:
    parser = argparse.ArgumentParser()
    for name in (
        "runner",
        "source-root",
        "profile",
        "protocol",
        "exporter",
        "certificate-exporter",
        "validator",
    ):
        parser.add_argument(f"--{name}", required=True)
    arguments = parser.parse_args()

    runner = load(arguments.runner)
    source = pathlib.Path(arguments.source_root).resolve()
    profile = json.loads(pathlib.Path(arguments.profile).read_text(encoding="utf-8"))

    if sorted(runner.declared_allowlist(source, profile["semantic_ctest_allowlist"])) != sorted(
        profile["semantic_ctest_allowlist"]
    ):
        raise RuntimeError("declared allowlist differs")

    expression = runner.ctest_regex(profile["semantic_ctest_allowlist"])
    if "(?:" in expression or not expression.startswith("^(") or not expression.endswith(")$"):
        raise RuntimeError("semantic CTest expression is not CTest-compatible")

    # The active protocol no longer uses the historical transient Section 13 title.
    runner.protocol_check(pathlib.Path(arguments.protocol))

    common = [
        sys.executable,
        arguments.runner,
        "--source-root",
        arguments.source_root,
        "--profile",
        arguments.profile,
        "--protocol",
        arguments.protocol,
        "--exporter",
        arguments.exporter,
        "--validator",
        arguments.validator,
    ]
    binding_args = argparse.Namespace(
        source_root=arguments.source_root,
        profile=arguments.profile,
        protocol=arguments.protocol,
        exporter=arguments.exporter,
        validator=arguments.validator,
    )
    binding_paths = runner.input_paths(binding_args, source)

    def passing_cloud_observation(cloud_profile, cell):
        return {
            "schema_version": 1,
            "kind": "cloud-qualification-environment-observation",
            "cell": cell,
            "status": "PASS",
            "runner": cloud_profile["runner"],
            "tools": cloud_profile["tools"],
            "historical_wsl_reference": cloud_profile["historical_wsl_reference"],
            "limitations": cloud_profile["limitations"],
            "failures": [],
        }

    with patch.object(runner, "validate_cloud_environment", side_effect=passing_cloud_observation):
        binding = runner.cloud_environment_binding(binding_paths, profile)
        if (
            binding["kind"] != "topological-model-cumulative-cloud-environment-binding"
            or len(binding["observations"]) != 4
        ):
            raise RuntimeError("cloud environment binding differs")
        if any(item["status"] != "PASS" for item in binding["observations"]):
            raise RuntimeError("cloud environment binding did not retain PASS observations")

    with patch.object(
        runner,
        "validate_cloud_environment",
        return_value={"status": "BLOCKED", "failures": ["intentional image drift"]},
    ):
        rejects(
            lambda: runner.cloud_environment_binding(binding_paths, profile),
            "cloud environment drift was accepted",
        )

    fake_cloud_binding = {
        "schema_version": 1,
        "kind": "topological-model-cumulative-cloud-environment-binding",
        "profile_sha256": "profile",
        "validator_sha256": "validator",
        "supplement_sha256": "supplement",
        "decision_sha256": "decision",
        "audit_sha256": "audit",
        "observations": [
            {"cell": cell["id"], "status": "PASS"} for cell in profile["cells"]
        ],
    }

    with tempfile.TemporaryDirectory(prefix="apmesh-core-tmr-runner-") as temporary:
        root = pathlib.Path(temporary)

        broken_protocol = root / "broken-protocol.md"
        broken_protocol.write_text(
            pathlib.Path(arguments.protocol)
            .read_text(encoding="utf-8")
            .replace(
                "Every cell must discover, build, and execute exactly once per repetition:",
                "Every cell executes the semantic contracts:",
                1,
            ),
            encoding="utf-8",
        )
        rejects(
            lambda: runner.protocol_check(broken_protocol),
            "protocol repetition invariant was not guarded",
        )

        self_check = root / "self-check.json"
        result = subprocess.run(
            [*common, "self-check", "--output", str(self_check)],
            capture_output=True,
            text=True,
            check=False,
        )
        if (
            result.returncode != 0
            or json.loads(self_check.read_text(encoding="utf-8"))["execution_requested"]
            is not False
        ):
            raise RuntimeError("runner self-check differs")

        self_check_value = json.loads(self_check.read_text(encoding="utf-8"))
        if any(
            "-DAPMESH_ENABLE_QUALIFICATION_TESTS=ON" not in cell["configure"]
            for cell in self_check_value["plan"]["cells"]
        ):
            raise RuntimeError("runner does not explicitly opt in to qualification tooling")
        if any(cell["repetitions"] != 2 for cell in self_check_value["plan"]["cells"]):
            raise RuntimeError("runner repetition count differs")
        if len(self_check_value["declared_semantic_ctest_allowlist"]) != 7:
            raise RuntimeError("runner semantic allowlist cardinality differs")

        for cell in self_check_value["plan"]["cells"]:
            expected_compiler = (
                "/usr/bin/g++-13"
                if cell["cell"].startswith("gcc-")
                else "/usr/bin/clang++-18"
            )
            if (
                cell["configure"][0] != "/usr/bin/cmake"
                or cell["build"][0] != "/usr/bin/cmake"
            ):
                raise RuntimeError("runner does not seal the admitted CMake path")
            if (
                cell["ctest_discovery"][0] != "/usr/bin/ctest"
                or cell["semantic_ctest"][0] != "/usr/bin/ctest"
            ):
                raise RuntimeError(
                    "runner does not seal the CTest path from the admitted CMake package"
                )
            if cell["dependency_inventory"][0] != "/usr/bin/ninja":
                raise RuntimeError("runner does not seal the admitted Ninja path")
            if "-DCMAKE_MAKE_PROGRAM=/usr/bin/ninja" not in cell["configure"]:
                raise RuntimeError("runner configure does not bind the admitted Ninja path")
            if (
                f"-DCMAKE_CXX_COMPILER={expected_compiler}" not in cell["configure"]
                or cell["compiler"] != expected_compiler
            ):
                raise RuntimeError("runner does not seal the admitted compiler path")

        discovery = root / "discovery"
        (discovery / "logs").mkdir(parents=True)
        discovery_record = {"stdout": {"path": "logs/ctest.stdout.log"}}
        discovery_log = discovery / discovery_record["stdout"]["path"]
        discovery_log.write_text(
            "  Test #1: alpha\n  Test  #2: beta\n Test    #10: gamma\n",
            encoding="utf-8",
        )
        if runner.discovered_tests_from_log(discovery, discovery_record) != [
            "alpha",
            "beta",
            "gamma",
        ]:
            raise RuntimeError("variable CTest spacing was not accepted")
        discovery_log.write_text(
            "Test #1 alpha\nprefix Test #2: beta\nTest 2: gamma\nTest #3:\ndelta\n",
            encoding="utf-8",
        )
        if runner.discovered_tests_from_log(discovery, discovery_record):
            raise RuntimeError("malformed CTest discovery was accepted")

        semantic_record = {
            "stdout": {"path": "logs/semantic.stdout.log"},
            "stderr": {"path": "logs/semantic.stderr.log"},
        }
        semantic_stdout = discovery / semantic_record["stdout"]["path"]
        semantic_stderr = discovery / semantic_record["stderr"]["path"]
        semantic_stdout.write_text(
            "\n".join(
                f" {index}/7 Test #{index}: {name} .... Passed 0.00 sec"
                for index, name in enumerate(profile["semantic_ctest_allowlist"], 1)
            ),
            encoding="utf-8",
        )
        semantic_stderr.write_text("", encoding="utf-8")
        runner.require_exact_semantic_ctest_execution(
            discovery,
            semantic_record,
            profile["semantic_ctest_allowlist"],
        )
        semantic_stdout.write_text(
            "RegularExpression::compile(): ?+* follows nothing.\n",
            encoding="utf-8",
        )
        semantic_stderr.write_text("No tests were found!!!\n", encoding="utf-8")
        rejects(
            lambda: runner.require_exact_semantic_ctest_execution(
                discovery,
                semantic_record,
                profile["semantic_ctest_allowlist"],
            ),
            "empty semantic CTest selection was accepted",
        )

        candidate = {
            "commit": "0123456789abcdef0123456789abcdef01234567",
            "upstream_commit": "0123456789abcdef0123456789abcdef01234567",
            "post_merge_baseline": runner.POST_MERGE_BASELINE,
            "tree_clean": True,
            "source_root": str(source),
            "source_inventory": [],
        }

        output = root / "prepared"
        prepared = argparse.Namespace(
            source_root=arguments.source_root,
            profile=arguments.profile,
            protocol=arguments.protocol,
            exporter=arguments.exporter,
            validator=arguments.validator,
            output_root=str(output),
        )
        with (
            patch.object(runner, "published_candidate", return_value=candidate),
            patch.object(runner, "environment_identity", return_value={"test": "identity"}),
            patch.object(
                runner,
                "cloud_environment_binding",
                return_value=fake_cloud_binding,
            ),
        ):
            runner.prepare(prepared)
            manifest = runner.validate_prepared(output)
            if (
                manifest["execution_requested"] is not False
                or set(manifest["gates"].values()) != {"NOT_EXECUTED"}
            ):
                raise RuntimeError("prepared manifest claims execution")
            if manifest["cloud_environment"] != fake_cloud_binding:
                raise RuntimeError("prepared manifest did not seal cloud environment binding")
            runner.validate_execution_binding(prepared, source, output, manifest)

            planned_logs = {
                item["path"]
                for item in manifest["planned_inventories"]["artifacts"]
                if item["role"] == "command-log"
            }
            if len(planned_logs) != 112:
                raise RuntimeError("planned command-log cardinality differs")
            for cell in profile["cells"]:
                cell_id = cell["id"]
                for repetition in (1, 2):
                    for stage in (
                        "build",
                        "ctest-discovery",
                        "semantic-ctest",
                        "certificate",
                        "certificate-validation",
                    ):
                        for stream in ("stdout", "stderr"):
                            expected = f"logs/{cell_id}-{stage}-{repetition}.{stream}.log"
                            if expected not in planned_logs:
                                raise RuntimeError(
                                    f"repetition-scoped planned log is absent: {expected}"
                                )

            saved = (output / "prepared-manifest.json").read_bytes()
            forged = json.loads(saved)
            forged["execution_requested"] = True
            (output / "prepared-manifest.json").write_text(
                json.dumps(forged),
                encoding="utf-8",
            )
            rejects(
                lambda: runner.validate_prepared(output),
                "enabled execution was accepted",
            )
            (output / "prepared-manifest.json").write_bytes(saved)
            (output / "execution-claim.json").write_text("{}", encoding="utf-8")
            rejects(
                lambda: runner.validate_prepared(output),
                "consumed manifest was accepted",
            )
            (output / "execution-claim.json").unlink()

        existing = root / "existing"
        existing.mkdir()
        rejected = argparse.Namespace(
            source_root=arguments.source_root,
            profile=arguments.profile,
            protocol=arguments.protocol,
            exporter=arguments.exporter,
            validator=arguments.validator,
            output_root=str(existing),
        )
        rejects(
            lambda: runner.prepare(rejected),
            "existing output root was accepted",
        )

        def simulated_command(output_root: pathlib.Path, fail_record_id: str | None = None):
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
                stdout.write_bytes(b"")
                stderr.write_bytes(b"")

                if "ctest-discovery" in record_id:
                    stdout.write_text(
                        "\n".join(
                            f"  Test  #{index}: {name}"
                            for index, name in enumerate(
                                profile["semantic_ctest_allowlist"],
                                1,
                            )
                        ),
                        encoding="utf-8",
                    )

                if "semantic-ctest" in record_id:
                    stdout.write_text(
                        "\n".join(
                            f" {index}/7 Test #{index}: {name} .... Passed 0.00 sec"
                            for index, name in enumerate(
                                profile["semantic_ctest_allowlist"],
                                1,
                            )
                        ),
                        encoding="utf-8",
                    )

                if "certificate-" in record_id and "validation" not in record_id:
                    destination = pathlib.Path(argv[-1])
                    if not destination.parent.is_dir():
                        raise RuntimeError(
                            "runner did not create certificate output directory"
                        )
                    completed = subprocess.run(
                        [
                            arguments.certificate_exporter,
                            "certificate",
                            str(destination),
                        ],
                        capture_output=True,
                        text=True,
                        check=False,
                    )
                    if completed.returncode != 0:
                        raise RuntimeError(completed.stderr)

                if "negative-outcomes" in record_id:
                    destination = pathlib.Path(argv[-1])
                    if not destination.parent.is_dir():
                        raise RuntimeError(
                            "runner did not create negative-output directory"
                        )
                    destination.write_text(
                        json.dumps(
                            {
                                "schema_version": 1,
                                "kind": "topological-model-cumulative-negative-outcomes",
                                "outcomes": [
                                    {"id": item, "result": "REJECTED"}
                                    for item in profile["certificate_negative_cases"]
                                ],
                            }
                        ),
                        encoding="utf-8",
                    )

                if record_id.endswith("-configure"):
                    cell_id = record_id.removesuffix("-configure")
                    build = output_root / "cells" / cell_id / "build"
                    build.mkdir(parents=True, exist_ok=True)
                    (build / "compile_commands.json").write_text(
                        "[]",
                        encoding="utf-8",
                    )

                failed = record_id == fail_record_id
                if failed:
                    stderr.write_text(
                        "intentional focused second-repetition failure\n",
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

        detached = {
            "result": "PASS",
            "candidate_commit": candidate["commit"],
            "source_inventory_count": 0,
        }

        failed = root / "failed"
        failed_args = argparse.Namespace(
            source_root=arguments.source_root,
            profile=arguments.profile,
            protocol=arguments.protocol,
            exporter=arguments.exporter,
            validator=arguments.validator,
            output_root=str(failed),
        )
        with (
            patch.object(runner, "published_candidate", return_value=candidate),
            patch.object(runner, "environment_identity", return_value={"test": "identity"}),
            patch.object(
                runner,
                "cloud_environment_binding",
                return_value=fake_cloud_binding,
            ),
            patch.object(
                runner,
                "run_command",
                side_effect=simulated_command(
                    failed,
                    "gcc-debug-semantic-ctest-2",
                ),
            ),
            patch.object(
                runner,
                "verify_detached_candidate",
                return_value=detached,
            ),
        ):
            runner.prepare(failed_args)
            if runner.execute(failed_args) != 1:
                raise RuntimeError("focused second-repetition failure was not reported")
            if (
                runner.read_json(failed / "state.json")["state"] != "BLOCKED"
                or runner.verify_retention(failed)["status"] != "PASS"
            ):
                raise RuntimeError("focused terminal failure was not retained")

            failure_records = runner.read_json(failed / "command-records.json")["records"]
            failure_ids = [record["id"] for record in failure_records]
            if failure_ids[-1] != "gcc-debug-semantic-ctest-2":
                raise RuntimeError("second-repetition failure did not occur at the sealed point")
            if "gcc-debug-certificate-2" in failure_ids:
                raise RuntimeError("runner continued after second-repetition semantic failure")
            if any(record_id.startswith("gcc-release-") for record_id in failure_ids):
                raise RuntimeError("runner did not fail closed before the next cell")

            before = {
                path.relative_to(failed).as_posix(): path.read_bytes()
                for path in failed.rglob("*")
                if path.is_file()
            }
            if runner.execute(failed_args) != 1:
                raise RuntimeError("consumed failed attempt was not rejected")
            after = {
                path.relative_to(failed).as_posix(): path.read_bytes()
                for path in failed.rglob("*")
                if path.is_file()
            }
            if after != before:
                raise RuntimeError("consumed failed attempt was reused")

        succeeded = root / "succeeded"
        success_args = argparse.Namespace(
            source_root=arguments.source_root,
            profile=arguments.profile,
            protocol=arguments.protocol,
            exporter=arguments.exporter,
            validator=arguments.validator,
            output_root=str(succeeded),
        )
        with (
            patch.object(runner, "published_candidate", return_value=candidate),
            patch.object(runner, "environment_identity", return_value={"test": "identity"}),
            patch.object(
                runner,
                "cloud_environment_binding",
                return_value=fake_cloud_binding,
            ),
            patch.object(
                runner,
                "run_command",
                side_effect=simulated_command(succeeded),
            ),
            patch.object(
                runner,
                "verify_detached_candidate",
                return_value=detached,
            ),
        ):
            runner.prepare(success_args)
            if (
                runner.execute(success_args) != 0
                or runner.verify_retention(succeeded)["status"] != "PASS"
            ):
                raise RuntimeError("focused successful terminal package was not retained")

        records = runner.read_json(succeeded / "command-records.json")["records"]
        ids = [record["id"] for record in records]
        if len(ids) != 56 or len(set(ids)) != 56:
            raise RuntimeError("corrected command-record cardinality differs")

        repetition_scoped = (
            "build",
            "ctest-discovery",
            "semantic-ctest",
            "certificate",
            "certificate-validation",
        )
        cell_scoped = (
            "configure",
            "negative-outcomes",
            "dependency-inventory",
            "ldd-exporter",
        )

        for cell in profile["cells"]:
            cell_id = cell["id"]
            cell_ids = [record_id for record_id in ids if record_id.startswith(f"{cell_id}-")]
            if len(cell_ids) != 14:
                raise RuntimeError(f"command cardinality differs for {cell_id}")

            for stage in repetition_scoped:
                expected = {
                    f"{cell_id}-{stage}-1",
                    f"{cell_id}-{stage}-2",
                }
                observed = {
                    record_id
                    for record_id in cell_ids
                    if record_id in expected
                }
                if observed != expected:
                    raise RuntimeError(
                        f"repetition-scoped records differ for {cell_id} {stage}"
                    )

            for stage in cell_scoped:
                expected = f"{cell_id}-{stage}"
                if cell_ids.count(expected) != 1:
                    raise RuntimeError(
                        f"cell-scoped record differs for {cell_id} {stage}"
                    )

        observed_inventories = runner.read_json(
            succeeded / "observed-inventories.json"
        )
        discoveries = observed_inventories["discoveries"]
        if len(discoveries) != 8:
            raise RuntimeError("discovery evidence cardinality differs")
        for cell in profile["cells"]:
            cell_id = cell["id"]
            selected = [item for item in discoveries if item["cell"] == cell_id]
            if {item["repetition"] for item in selected} != {1, 2}:
                raise RuntimeError(f"discovery repetition identity differs for {cell_id}")
            for item in selected:
                if item["record_id"] != (
                    f"{cell_id}-ctest-discovery-{item['repetition']}"
                ):
                    raise RuntimeError("discovery record identity differs")
                if sorted(item["discovered_allowlist"]) != sorted(
                    profile["semantic_ctest_allowlist"]
                ):
                    raise RuntimeError("discovery semantic allowlist differs")

        semantic_records = [
            record for record in records if "-semantic-ctest-" in record["id"]
        ]
        if len(semantic_records) != 8:
            raise RuntimeError("semantic CTest record cardinality differs")
        for record in semantic_records:
            runner.require_exact_semantic_ctest_execution(
                succeeded,
                record,
                profile["semantic_ctest_allowlist"],
            )

        actual_logs = {
            path.relative_to(succeeded).as_posix()
            for path in (succeeded / "logs").iterdir()
            if path.is_file()
        }
        success_manifest = runner.read_json(succeeded / "prepared-manifest.json")
        planned_logs = {
            item["path"]
            for item in success_manifest["planned_inventories"]["artifacts"]
            if item["role"] == "command-log"
        }
        if actual_logs != planned_logs or len(planned_logs) != 112:
            raise RuntimeError("retained command-log inventory differs from the plan")

        certificate_entries = runner.read_json(
            succeeded / "certificate-index.json"
        )["entries"]
        if len(certificate_entries) != 8:
            raise RuntimeError("certificate cardinality differs")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
