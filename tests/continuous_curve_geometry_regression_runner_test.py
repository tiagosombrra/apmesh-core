#!/usr/bin/env python3
"""Focused report-only runner contracts for CGR0-CGR7 tooling."""

from __future__ import annotations

import argparse
import importlib.util
import json
import pathlib
import subprocess
import sys
import tempfile


def load(path: pathlib.Path):
    specification = importlib.util.spec_from_file_location("cgr_runner", path)
    if specification is None or specification.loader is None:
        raise RuntimeError("CGR runner module could not be loaded")
    module = importlib.util.module_from_spec(specification)
    specification.loader.exec_module(module)
    return module


def run(command: list[str], expected: int = 0) -> subprocess.CompletedProcess[str]:
    completed = subprocess.run(
        command,
        capture_output=True,
        text=True,
        check=False,
    )
    if completed.returncode != expected:
        raise RuntimeError(
            f"unexpected exit {completed.returncode}: {' '.join(command)}\n"
            f"stdout:\n{completed.stdout}\nstderr:\n{completed.stderr}"
        )
    return completed


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--runner", required=True)
    parser.add_argument("--source-root", required=True)
    parser.add_argument("--profile", required=True)
    parser.add_argument("--protocol", required=True)
    parser.add_argument("--exporter", required=True)
    parser.add_argument("--validator", required=True)
    parser.add_argument("--negative", required=True)
    arguments = parser.parse_args()

    runner_path = pathlib.Path(arguments.runner)
    module = load(runner_path)
    profile = json.loads(pathlib.Path(arguments.profile).read_text(encoding="utf-8"))
    source = pathlib.Path(arguments.source_root).resolve()

    declared = module.declared_allowlist(source, profile["semantic_ctest_allowlist"])
    if declared != profile["semantic_ctest_allowlist"]:
        raise RuntimeError("declared allowlist differs")
    expression = module.ctest_regex(declared)
    if "(?:" in expression or not expression.startswith("^(") or not expression.endswith(")$"):
        raise RuntimeError("CTest expression is not compatible with the admitted syntax")

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
        "--negative",
        arguments.negative,
    ]

    with tempfile.TemporaryDirectory(prefix="apmesh-cgr-runner-") as temporary:
        root = pathlib.Path(temporary)

        self_check = root / "self-check.json"
        run([*common, "self-check", "--output", str(self_check)])
        value = json.loads(self_check.read_text(encoding="utf-8"))
        if (
            value["status"] != "PASS"
            or value["execution_requested"] is not False
            or value["formal_preparation"] is not False
            or value["formal_execution"] is not False
            or value["candidate"]["semantic_baseline"] != profile["semantic_baseline"]
        ):
            raise RuntimeError("report-only self-check differs")

        frozen = value["frozen_semantic_files"]
        if [item["path"] for item in frozen] != profile["frozen_semantic_files"]:
            raise RuntimeError("frozen semantic inventory differs")
        if any(item["sha256"] != item["baseline_sha256"] for item in frozen):
            raise RuntimeError("frozen semantic baseline drifted")

        plan_path = root / "plan.json"
        run([*common, "plan", "--output", str(plan_path)])
        plan = json.loads(plan_path.read_text(encoding="utf-8"))
        if (
            plan["execution_requested"] is not False
            or set(plan["gates"].values()) != {"NOT_EXECUTED"}
            or len(plan["cells"]) != 4
        ):
            raise RuntimeError("report-only plan differs")
        for cell in plan["cells"]:
            if len(cell["repetitions"]) != 2:
                raise RuntimeError("repetition count differs")
            if cell["configure"][0] != "/usr/bin/cmake":
                raise RuntimeError("CMake path is not sealed")
            if cell["configure"][2] != str(source):
                raise RuntimeError("source root differs")
            if "-DAPMESH_ENABLE_QUALIFICATION_TESTS=ON" not in cell["configure"]:
                raise RuntimeError("qualification tooling is not explicitly enabled")
            if cell["compiler"] not in {"/usr/bin/g++-13", "/usr/bin/clang++-18"}:
                raise RuntimeError("compiler path differs")
            for repetition in cell["repetitions"]:
                if repetition["build"][0] != "/usr/bin/cmake":
                    raise RuntimeError("build path differs")
                if repetition["ctest_discovery"][0] != "/usr/bin/ctest":
                    raise RuntimeError("CTest discovery path differs")
                if repetition["semantic_ctest"][0] != "/usr/bin/ctest":
                    raise RuntimeError("semantic CTest path differs")

        simulation = root / "simulation.json"
        run([*common, "simulate", "--output", str(simulation)])
        simulated = json.loads(simulation.read_text(encoding="utf-8"))
        if simulated["command_record_count"] != 56:
            raise RuntimeError("command cardinality differs")
        if simulated["command_log_count"] != 112:
            raise RuntimeError("command-log cardinality differs")
        if simulated["discovery_record_count"] != 8:
            raise RuntimeError("discovery cardinality differs")
        if simulated["semantic_ctest_record_count"] != 8:
            raise RuntimeError("semantic CTest cardinality differs")
        if simulated["semantic_test_execution_count"] != 112:
            raise RuntimeError("individual semantic test cardinality differs")
        if simulated["certificate_count"] != 8:
            raise RuntimeError("certificate cardinality differs")
        if len(set(simulated["command_ids"])) != 56:
            raise RuntimeError("command identifiers are not unique")
        if len(set(simulated["command_logs"])) != 112:
            raise RuntimeError("command logs are not unique")
        if set(simulated["gates"].values()) != {"NOT_EXECUTED"}:
            raise RuntimeError("report-only simulation claims CGR closure")

        run(
            [
                *common,
                "validate-simulation",
                "--simulation",
                str(simulation),
            ]
        )

        forged = dict(simulated)
        forged["formal_execution"] = True
        forged_path = root / "forged-execution.json"
        forged_path.write_text(json.dumps(forged), encoding="utf-8")
        run(
            [
                *common,
                "validate-simulation",
                "--simulation",
                str(forged_path),
            ],
            expected=2,
        )

        forged = dict(simulated)
        forged["command_ids"] = list(simulated["command_ids"][:-1])
        forged_path = root / "forged-cardinality.json"
        forged_path.write_text(json.dumps(forged), encoding="utf-8")
        run(
            [
                *common,
                "validate-simulation",
                "--simulation",
                str(forged_path),
            ],
            expected=2,
        )

        help_result = run([sys.executable, arguments.runner, "--help"])
        help_text = help_result.stdout
        if "prepare" in help_text or "execute" in help_text:
            raise RuntimeError("report-only runner exposes a formal lifecycle command")
        for command in ("self-check", "plan", "simulate", "validate-simulation"):
            if command not in help_text:
                raise RuntimeError(f"runner command is absent: {command}")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
