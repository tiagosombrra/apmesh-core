#!/usr/bin/env python3
"""Focused durable-retention contract for a semantically valid REC package."""

from __future__ import annotations

import argparse
import copy
import importlib.util
import json
import os
import pathlib
import shutil
import subprocess
import sys
import tempfile


def load_tool(path: pathlib.Path, name: str):
    sys.path.insert(0, str(path.parent))
    spec = importlib.util.spec_from_file_location(name, path)
    if spec is None or spec.loader is None:
        raise RuntimeError(f"cannot load {name}")
    module = importlib.util.module_from_spec(spec); sys.modules[spec.name] = module; spec.loader.exec_module(module)
    return module


def write_bundle(evidence, root: pathlib.Path, profile: dict, row: dict, commit: str, prepared_sha: str,
                 profile_sha: str, launch_plan_sha: str, source_root: pathlib.Path,
                 scratch_root: pathlib.Path, evidence_root: pathlib.Path) -> None:
    cell, replay = row["cell"], row["replay"]
    root.mkdir(parents=True); logs = root / "logs"; logs.mkdir()
    declared_bundle = evidence_root / "cells" / cell / f"replay-{replay}"
    exporter = scratch_root / "cells" / cell / f"replay-{replay}" / "build" / "apmesh_core_numeric_contract_export"
    commands = [(stage, [part.replace("@SCRATCH_ROOT@", str(scratch_root)) for part in row[stage]], source_root,
                 profile["limits_seconds"][stage]) for stage in ("configure", "build", "ctest")]
    commands.extend((mode, [str(exporter), mode, str(declared_bundle / filename)], declared_bundle,
                     profile["limits_seconds"]["export"])
                    for mode, filename in (("certificate", "certificate.json"), ("environment", "environment.json")))
    records = []
    for index, (name, argv, cwd, timeout) in enumerate(commands, start=1):
        stdout, stderr = logs / f"{name}.stdout.log", logs / f"{name}.stderr.log"
        stdout.write_text(f"{name}\n", encoding="utf-8"); stderr.write_text("", encoding="utf-8")
        records.append({"schema_version": 1, "kind": "experiment-command-record", "id": name, "argv": argv,
                        "cwd": str(cwd.resolve()), "environment_delta": {}, "started_utc": "2026-01-01T00:00:00+00:00",
                        "ended_utc": "2026-01-01T00:00:01+00:00", "elapsed_seconds": 1.0, "pid": index,
                        "timeout_seconds": timeout, "timed_out": False, "exit_code": 0, "launch_error": None,
                        "stdout": {"path": stdout.relative_to(root).as_posix(), "sha256": evidence.sha256(stdout)},
                        "stderr": {"path": stderr.relative_to(root).as_posix(), "sha256": evidence.sha256(stderr)}})
    evidence.write_json(root / "manifest.json", {"schema_version": 1, "kind": "reproducible-experiment-bundle-manifest", "cell": cell,
                                                   "replay": replay, "candidate_commit": commit, "profile_sha256": profile_sha,
                                                   "prepared_manifest_sha256": prepared_sha, "launch_plan_sha256": launch_plan_sha})
    evidence.write_json(root / "certificate.json", {"schema_version": 1, "kind": "numeric-contract-certificate", "stable": True})
    evidence.write_json(root / "environment.json", {"schema_version": 1, "kind": "numeric-contract-environment", "stable": True})
    evidence.write_json(root / "execution-record.json", {"schema_version": 1, "kind": "reproducible-experiment-execution", "cell": cell, "replay": replay, "records": records})
    summary = {"schema_version": 1, "kind": "reproducible-experiment-summary", "cell": cell, "replay": replay,
               "gates": {gate: "BUNDLE_VALIDATED" for gate in evidence.EXPECTED_GATES},
               "certificate": {"schema_version": 1, "kind": "numeric-contract-certificate", "stable": True},
               "environment": {"schema_version": 1, "kind": "numeric-contract-environment", "stable": True},
               "artifact_roles": profile["required_artifacts"], "retained_limitations": profile["limitations"]}
    evidence.write_json(root / "summary.json", summary); evidence.write_derived(root, [summary])
    execution = json.loads((root / "execution-record.json").read_text(encoding="utf-8"))
    evidence.write_json(root / "artifact-inventory.json", {"schema_version": 1, "kind": "experiment-artifact-inventory", "artifacts": evidence.artifact_inventory(root, profile, execution)})


def require_rejection(callback, error_types: tuple[type[Exception], ...], message: str) -> None:
    try:
        callback()
    except error_types:
        return
    raise RuntimeError(message)


def write_prerequisites(evidence, architecture_runner, numeric_runner, numeric_evidence, campaign: pathlib.Path,
                        source_root: pathlib.Path, source_inventory: list[dict[str, str]], commit: str) -> dict:
    architecture = campaign / "prerequisites" / "numeric-contract" / "architecture" / "manifest.json"
    numeric = campaign / "prerequisites" / "numeric-contract" / "manifest.json"
    numeric_root = numeric.parent
    if numeric_root.exists():
        shutil.rmtree(numeric_root)
    architecture.parent.mkdir(parents=True, exist_ok=True)

    input_identity = {"path": str(source_root / "input"), "sha256": evidence.sha256(source_root / "input")}

    def artifact(root: pathlib.Path, name: str, payload: dict, anchor: pathlib.Path) -> dict:
        path = root / name; path.parent.mkdir(parents=True, exist_ok=True)
        evidence.write_json(path, payload)
        return {"path": path.relative_to(anchor).as_posix(), "sha256": evidence.sha256(path)}

    def command(root: pathlib.Path, name: str, anchor: pathlib.Path, argv: list[str], cwd: pathlib.Path, *, exit_code: int = 0) -> dict:
        logs = root / "logs"; logs.mkdir(parents=True, exist_ok=True)
        stdout, stderr = logs / f"{name}.stdout.log", logs / f"{name}.stderr.log"
        stdout.write_text(f"{name}\n", encoding="utf-8"); stderr.write_text("", encoding="utf-8")
        return {"schema_version": 1, "kind": "qualified-command-record", "id": name,
                "command": argv, "cwd": str(cwd.resolve()), "pid": 41,
                "timeout_seconds": 30, "started_utc": "2026-01-01T00:00:00+00:00",
                "ended_utc": "2026-01-01T00:00:01+00:00", "elapsed_seconds": 1.0,
                "timed_out": False, "exit_code": exit_code, "launch_error": None,
                "stdout": {"path": stdout.relative_to(anchor).as_posix(), "sha256": evidence.sha256(stdout)},
                "stderr": {"path": stderr.relative_to(anchor).as_posix(), "sha256": evidence.sha256(stderr)}}

    architecture_profile = json.loads((pathlib.Path(__file__).parents[1] / "experiments" / "profiles" / "architecture_bootstrap.json").read_text(encoding="utf-8"))
    numeric_profile = json.loads((pathlib.Path(__file__).parents[1] / "experiments" / "profiles" / "numeric_contract.json").read_text(encoding="utf-8"))
    architecture_plan = architecture_runner.command_plan(architecture_profile, source_root)
    numeric_plan = numeric_runner.command_plan(numeric_profile, source_root)
    expected_source = pathlib.Path(__file__).parents[1] / "experiments" / "expected" / "bootstrap_certificate.json"
    architecture_expected = architecture.parent / "expected-certificate.json"; shutil.copy2(expected_source, architecture_expected)
    architecture_cells, numeric_cells = [], []
    for name in evidence.EXPECTED_CONFIGURATION_NAMES:
        architecture_cell = architecture.parent / "cells" / name
        architecture_certificates = []
        for index in range(1, 4):
            certificate = architecture_cell / f"certificate-{index}.json"; certificate.parent.mkdir(parents=True, exist_ok=True); shutil.copy2(architecture_expected, certificate)
            architecture_certificates.append({"path": certificate.relative_to(architecture.parent).as_posix(), "sha256": evidence.sha256(certificate)})
        compile_commands = artifact(architecture_cell, "compile_commands.json", {"commands": []}, architecture.parent)
        architecture_plan_row = next(row for row in architecture_plan if row["name"] == name)
        build_root, scratch_root, consumer_root = architecture_cell / "build", architecture_cell / "scratch", architecture_cell / "consumer"
        configuration = architecture_runner.CONFIGURATIONS[name]
        architecture_commands = {stage: ([part.replace("@OUTPUT_ROOT@", str(architecture.parent)) for part in architecture_plan_row[stage]], architecture.parent) for stage in ("configure", "build", "ctest")}
        architecture_commands.update({"scratch-smoke": ([str(build_root / "apmesh_core_bootstrap_smoke")], scratch_root),
                                      **{f"export-{index}": ([str(build_root / "apmesh_core_bootstrap_export"), str(architecture_cell / f"certificate-{index}.json")], architecture_cell) for index in range(1, 4)},
                                      **{f"validate-{index}": ([sys.executable, str(source_root / "comparer.py"), "validate", "--expected", str(source_root / "expected.json"), "--actual", str(architecture_cell / f"certificate-{index}.json")], architecture_cell) for index in range(1, 4)},
                                      "consumer-configure": (["cmake", "-S", str(source_root / "tests" / "consumer"), "-B", str(consumer_root), "-G", "Ninja", f"-DCMAKE_CXX_COMPILER={configuration.compiler}", f"-DAPMESH_CORE_SOURCE_DIR={source_root}", "-DBUILD_TESTING=OFF", f"-DAPMESH_USE_LIBCXX={'ON' if configuration.libcxx else 'OFF'}"], architecture_cell),
                                      "consumer-build": (["cmake", "--build", str(consumer_root), "--target", "apmesh_core_external_consumer", "apmesh_core_unrelated_target"], architecture_cell),
                                      "consumer": ([str(consumer_root / "apmesh_core_external_consumer")], scratch_root),
                                      "unrelated": ([str(consumer_root / "apmesh_core_unrelated_target")], scratch_root)})
        architecture_records = [command(architecture_cell, stage, architecture.parent, argv, cwd) for stage, (argv, cwd) in architecture_commands.items()]
        architecture_records.append({"stage": "compile-commands", **compile_commands})
        architecture_cells.append({"cell": name, "state": "PASS", "certificates": [item["path"] for item in architecture_certificates],
                                   "certificate_artifacts": architecture_certificates, "records": architecture_records})

        numeric_cell = numeric_root / "cells" / name
        certificate_payload = {"schema_version": 1, "kind": "numeric-contract-certificate",
                               "classification": [{"case": case, "category": category} for case, category in numeric_evidence.EXPECTED_CLASSIFICATION.items()],
                               "policy": [{"case": case, "outcome": outcome, "error": error} for case, (outcome, error) in numeric_evidence.EXPECTED_POLICY.items()],
                               "proximity": [{"case": case, "outcome": outcome, "error": error,
                                              "residual_hex": None if outcome == "error" else float.hex(numeric_evidence.EXPECTED_PROXIMITY_VALUES[case][0]),
                                              "limit_hex": None if outcome == "error" else float.hex(numeric_evidence.EXPECTED_PROXIMITY_VALUES[case][1])}
                                             for case, (outcome, error) in numeric_evidence.EXPECTED_PROXIMITY.items()],
                               "separation": {"proximity_result_values": ["within", "outside"], "identity_conversion": False, "predicate_sign": False}}
        environment_payload = {"schema_version": 1, "kind": "numeric-contract-environment", "double": {"radix": 2, "digits": 53, "is_iec559": True, "subnormal_supported": True, "round_style": "to_nearest"}, "active_rounding": "to_nearest"}
        numeric_certificates = [artifact(numeric_cell, f"certificate-{index}.json", certificate_payload, numeric_root) for index in range(1, 4)]
        numeric_environments = [artifact(numeric_cell, f"environment-{index}.json", environment_payload, numeric_root) for index in range(1, 4)]
        compile_commands = artifact(numeric_cell, "compile_commands.json", [{"file": str(source_root / "src" / "core" / "numeric.cpp"), "command": "g++-13 -std=c++23 -c src/core/numeric.cpp"}], numeric_root)
        numeric_plan_row = next(row for row in numeric_plan if row["name"] == name)
        numeric_build = numeric_cell / "build"; exporter = numeric_build / "apmesh_core_numeric_contract_export"
        numeric_commands = {stage: ([part.replace("@OUTPUT_ROOT@", str(numeric_root)) for part in numeric_plan_row[stage]], numeric_root) for stage in ("configure", "build")}
        numeric_commands.update({f"ctest-{index}": ([part.replace("@OUTPUT_ROOT@", str(numeric_root)) for part in numeric_plan_row["ctest"]], numeric_root) for index in range(1, 4)})
        numeric_commands.update({f"certificate-{index}": ([str(exporter), "certificate", str(numeric_cell / f"certificate-{index}.json")], numeric_cell) for index in range(1, 4)})
        numeric_commands.update({f"environment-{index}": ([str(exporter), "environment", str(numeric_cell / f"environment-{index}.json")], numeric_cell) for index in range(1, 4)})
        numeric_cells.append({"cell": name, "state": "PASS", "certificates": [item["path"] for item in numeric_certificates],
                              "environments": [item["path"] for item in numeric_environments],
                              "certificate_artifacts": numeric_certificates, "environment_artifacts": numeric_environments,
                              "compile_commands_artifact": compile_commands,
                              "records": [command(numeric_cell, stage, numeric_root, argv, cwd) for stage, (argv, cwd) in numeric_commands.items()]})

    architecture_inputs = {"profile": input_identity, "protocol": input_identity,
                           "expected_certificate": {"path": str(source_root / "expected.json"), "sha256": evidence.sha256(source_root / "expected.json")},
                           "comparer": {"path": str(source_root / "comparer.py"), "sha256": evidence.sha256(source_root / "comparer.py")}, "launcher": input_identity}
    architecture_certificates = [str(architecture.parent / certificate)
                                 for row in architecture_cells for certificate in row["certificates"]]
    architecture_globals = []
    for name, suffix in (("compare-one", "one"), ("compare-two", "two")):
        argv = [sys.executable, architecture_inputs["comparer"]["path"], "compare", "--expected",
                architecture_inputs["expected_certificate"]["path"],
                *sum((["--certificate", certificate] for certificate in architecture_certificates), []),
                "--report", str(architecture.parent / "reports" / suffix / "certificate-report.md")]
        architecture_globals.append({"global": name, "state": "PASS",
                                     "record": command(architecture.parent, name, architecture.parent, argv, architecture.parent)})
    fixtures = source_root / "tests" / "data" / "bootstrap_regression"
    architecture_negatives = [
        [sys.executable, architecture_inputs["comparer"]["path"], "validate", "--expected",
         architecture_inputs["expected_certificate"]["path"], "--actual", str(architecture.parent / "missing.json")],
        *[[sys.executable, architecture_inputs["comparer"]["path"], "validate", "--expected",
           architecture_inputs["expected_certificate"]["path"], "--actual", str(fixtures / filename)]
          for filename in ("malformed.json", "unsupported_schema_version.json", "changed_component.json")],
        [sys.executable, architecture_inputs["comparer"]["path"], "validate-manifest", "--manifest",
         str(fixtures / "manifest_changed_source_hash.json"), "--expected-source-hash", "expected-source-hash"],
    ]
    evidence.write_json(architecture, {"schema_version": 1, "protocol_version": 1, "state": "EXECUTED_PENDING_AUDIT", "execution_requested": False,
                                       "candidate": {"commit": commit, "tree_clean": True, "source_root": str(source_root), "source_inventory": source_inventory},
                                       "inputs": architecture_inputs,
                                       "evidence_inputs": {"expected_certificate": {"path": "expected-certificate.json", "sha256": evidence.sha256(architecture_expected)}},
                                       "environment": {}, "limits_seconds": {}, "plan": architecture_plan, "negative_fixtures": architecture_profile["negative_fixtures"], "prepared_utc": "2026-01-01T00:00:00+00:00",
                                       "requirements": {str(number): "EVIDENCE_COLLECTED_PENDING_AUDIT" for number in range(1, 9)},
                                       "execution": {"records": architecture_cells + architecture_globals,
                                                     "negative_records": [command(architecture.parent, f"negative-{index}", architecture.parent, argv, architecture.parent, exit_code=1)
                                                                          for index, argv in enumerate(architecture_negatives, start=1)]}})
    report = numeric_root / "report.md"; report.write_text("qualified report\n", encoding="utf-8")
    numeric_inputs = {name: input_identity for name in ("profile", "protocol", "validator", "architecture_runner", "architecture_profile", "architecture_evidence", "architecture_expected", "architecture_comparer", "launcher")}
    numeric_certificates = [str(numeric_root / certificate) for row in numeric_cells for certificate in row["certificates"]]
    numeric_environments = [str(numeric_root / environment) for row in numeric_cells for environment in row["environments"]]
    numeric_comparison = [sys.executable, numeric_inputs["validator"]["path"], "compare",
                          *sum((["--certificate", certificate] for certificate in numeric_certificates), []),
                          *sum((["--environment", environment] for environment in numeric_environments), []),
                          "--report", str(numeric_root / "report.md")]
    preservation_plan = {"prepare": ["architecture", "prepare"], "execute": ["architecture", "execute"]}
    evidence.write_json(numeric, {"schema_version": 1, "protocol_version": 1, "state": "EXECUTED_PENDING_AUDIT", "execution_requested": False,
                                       "candidate": {"commit": commit, "tree_clean": True, "source_root": str(source_root), "source_inventory": source_inventory},
                                   "inputs": numeric_inputs,
                                   "environment": {}, "limits_seconds": {}, "plan": numeric_plan, "architecture_preservation_plan": preservation_plan, "prepared_utc": "2026-01-01T00:00:00+00:00",
                                   "gates": {f"N{number}": "EVIDENCE_COLLECTED_PENDING_AUDIT" for number in range(8)},
                                   "execution": {"records": numeric_cells, "comparison": command(numeric_root, "compare", numeric_root, numeric_comparison, numeric_root),
                                                 "architecture_preservation": {"prepare": command(numeric_root, "architecture-prepare", numeric_root, preservation_plan["prepare"], numeric_root), "execute": command(numeric_root, "architecture-execute", numeric_root, preservation_plan["execute"], numeric_root),
                                                                               "manifest": str(architecture)},
                                                 "report_sha256": evidence.sha256(report)}})
    return {"numeric_contract": {"manifest": numeric.relative_to(campaign).as_posix(), "manifest_sha256": evidence.sha256(numeric),
                                  "state": "EXECUTED_PENDING_AUDIT", "candidate_commit": commit},
            "architecture_contract": {"manifest": architecture.relative_to(campaign).as_posix(), "manifest_sha256": evidence.sha256(architecture),
                                        "state": "EXECUTED_PENDING_AUDIT", "candidate_commit": commit}}


def main() -> int:
    parser = argparse.ArgumentParser(); parser.add_argument("--tool", required=True); parser.add_argument("--profile", required=True)
    arguments = parser.parse_args(); tool_path = pathlib.Path(arguments.tool); tool = load_tool(tool_path, "rec_retention")
    runner = load_tool(tool_path.with_name("run_reproducible_experiment_contract.py"), "rec_runner")
    evidence = load_tool(tool_path.with_name("reproducible_experiment_evidence.py"), "rec_evidence")
    architecture_runner = load_tool(tool_path.with_name("run_architecture_bootstrap_regression.py"), "rec_architecture_runner")
    numeric_runner = load_tool(tool_path.with_name("run_numeric_contract_regression.py"), "rec_numeric_runner")
    numeric_evidence = load_tool(tool_path.with_name("numeric_contract_evidence.py"), "rec_numeric_evidence")
    profile_path = pathlib.Path(arguments.profile); profile = evidence.validate_profile(profile_path)
    with tempfile.TemporaryDirectory() as temporary_directory:
        root = pathlib.Path(temporary_directory); campaign, control, source = root / "campaign", root / "control", root / "source"
        destination = root / "evidence" / "foundation" / "rec" / "synthetic"
        command_root = root / "child-pid"; command_root.mkdir()
        architecture_record = architecture_runner.run([sys.executable, "-c", "print('architecture')"], cwd=command_root, timeout=10,
                                                     stdout=command_root / "architecture.stdout.log", stderr=command_root / "architecture.stderr.log")
        numeric_record = numeric_runner.execute_command([sys.executable, "-c", "print('numeric')"], command_root, 10, command_root, "numeric")
        if architecture_record["pid"] == os.getpid() or numeric_record["pid"] == os.getpid() or architecture_record["exit_code"] != 0 or numeric_record["exit_code"] != 0:
            raise RuntimeError("REC prerequisite runners did not retain child-process identity")
        control.mkdir(); source.mkdir(); scratch = control / "scratch"; profile_sha = evidence.sha256(profile_path)
        expected_source = pathlib.Path(__file__).parents[1] / "experiments" / "expected" / "bootstrap_certificate.json"
        (source / "CMakeLists.txt").write_text("cmake_minimum_required(VERSION 3.25)\n", encoding="utf-8")
        (source / "input").write_text("revision-bound input\n", encoding="utf-8")
        shutil.copy2(expected_source, source / "expected.json")
        (source / "comparer.py").write_text("# revision-bound comparer placeholder\n", encoding="utf-8")
        for command in (("git", "init"), ("git", "config", "user.email", "rec@example.invalid"),
                        ("git", "config", "user.name", "REC Contract"), ("git", "add", "."),
                        ("git", "commit", "-m", "synthetic candidate")):
            subprocess.run(command, cwd=source, check=True, capture_output=True)
        commit = subprocess.run(("git", "rev-parse", "HEAD"), cwd=source, check=True, capture_output=True, text=True).stdout.strip()
        source_inventory = [{"path": path, "sha256": evidence.sha256(source / path)}
                            for path in subprocess.run(("git", "ls-files"), cwd=source, check=True, capture_output=True, text=True).stdout.splitlines()]
        plan = {"schema_version": 1, "kind": "reproducible-experiment-launch-plan", "experiment_id": profile["experiment_id"],
                "cells": runner.command_plan(profile, source)}
        evidence.write_json(control / "launch-plan.json", plan); launch_plan_sha = evidence.sha256(control / "launch-plan.json")
        prepared = {"state": "PREPARED", "candidate": {"commit": commit, "source_root": str(source)},
                    "evidence_root": str(campaign), "profile_sha256": profile_sha,
                    "launch_plan": {"path": str(control / "launch-plan.json"), "sha256": launch_plan_sha},
                    "execution_envelope": {"scratch_root": str(scratch), "exporter_paths": []}}
        evidence.write_json(control / "prepared-manifest.json", prepared); prepared_sha = evidence.sha256(control / "prepared-manifest.json")
        evidence.write_json(control / "state.json", {"state": "EXECUTED_PENDING_AUDIT"})
        (control / "state-history.jsonl").write_text('{"state":"PREPARED"}\n{"state":"RUNNING"}\n{"state":"EXECUTED_PENDING_AUDIT"}\n', encoding="utf-8")
        bundles, seals = [], []
        for row in plan["cells"]:
                relative = f"cells/{row['cell']}/replay-{row['replay']}"; bundle = campaign / relative
                write_bundle(evidence, bundle, profile, row, commit, prepared_sha, profile_sha, launch_plan_sha, source, scratch, campaign)
                bundles.append({"cell": row["cell"], "replay": row["replay"], "bundle": relative})
                seals.append({"bundle": relative, "inventory_sha256": evidence.sha256(bundle / "artifact-inventory.json"),
                              "summary_sha256": evidence.sha256(bundle / "summary.json"), "execution_sha256": evidence.sha256(bundle / "execution-record.json")})
        negatives = []
        for fixture, declaration in profile["negative_fixtures"].items():
            response_path = campaign / "negative-fixtures" / fixture / "logs" / f"{fixture}.stdout.log"; response_path.parent.mkdir(parents=True)
            evidence.write_json(response_path, {"schema_version": 1, "kind": "reproducible-experiment-negative-result", "fixture": fixture, "result": "REJECTED", "reason_code": declaration["reason_code"]})
            stderr_path = response_path.with_name(f"{fixture}.stderr.log"); stderr_path.write_text("", encoding="utf-8")
            record = {"schema_version": 1, "kind": "experiment-command-record", "id": fixture,
                      "argv": [sys.executable, str(source / "tools" / "reproducible_experiment_negative.py"), "--fixture", fixture,
                               "--profile", str(source / "experiments" / "profiles" / "reproducible_experiment_contract.json")],
                      "cwd": str(source.resolve()), "environment_delta": {}, "started_utc": "2026-01-01T00:00:00+00:00",
                      "ended_utc": "2026-01-01T00:00:01+00:00", "elapsed_seconds": 1.0, "pid": 100 + len(negatives),
                      "timeout_seconds": 30, "timed_out": False, "exit_code": 0, "launch_error": None,
                      "stdout": {"path": "logs/" + response_path.name, "sha256": evidence.sha256(response_path)},
                      "stderr": {"path": "logs/" + stderr_path.name, "sha256": evidence.sha256(stderr_path)}}
            negatives.append({"fixture": fixture, "expected_reason_code": declaration["reason_code"], "result": "REJECTED", "reason_code": declaration["reason_code"],
                              "record": record,
                              "response": {"path": response_path.relative_to(campaign).as_posix(), "sha256": evidence.sha256(response_path)}})
        prerequisites = write_prerequisites(evidence, architecture_runner, numeric_runner, numeric_evidence, campaign, source, source_inventory, commit)
        replay_comparisons = []
        cell_summaries, provenance = {}, {}
        for configuration in profile["configurations"]:
            cell = configuration["name"]; roots = [campaign / "cells" / cell / f"replay-{replay}" for replay in (1, 2)]
            replay_comparisons.append(evidence.compare_replays(roots, profile))
            cell_summaries[cell] = evidence.validate_bundle(roots[0], profile)
            provenance[cell] = json.loads((roots[0] / "execution-record.json").read_text(encoding="utf-8"))
        terminal = {"schema_version": 1, "kind": "reproducible-experiment-terminal-manifest", "state": "EXECUTED_PENDING_AUDIT",
                    "prepared_manifest_sha256": prepared_sha, "candidate_commit": commit, "bundles": bundles, "bundle_seals": seals,
                    "replay_comparisons": replay_comparisons, "cross_configuration": evidence.compare_configurations(cell_summaries, profile, provenance),
                    "negative_fixtures": negatives, "prerequisites": prerequisites,
                    "gate_results": {gate: "EVIDENCE_COLLECTED_PENDING_AUDIT" for gate in evidence.EXPECTED_GATES}}
        evidence.write_json(campaign / "terminal-manifest.json", terminal)
        tool._validate_campaign(campaign, control, profile_path, commit)
        original_terminal = json.loads((campaign / "terminal-manifest.json").read_text(encoding="utf-8"))
        terminal = copy.deepcopy(original_terminal)
        # E2/E3: a successful but undeclared command cannot substitute for the launch plan.
        first_bundle = campaign / terminal["bundles"][0]["bundle"]
        execution_path, inventory_path = first_bundle / "execution-record.json", first_bundle / "artifact-inventory.json"
        original_execution, original_inventory = execution_path.read_bytes(), inventory_path.read_bytes()
        execution = json.loads(original_execution.decode("utf-8")); execution["records"][0]["argv"] = ["cmake", "--unexpected"]
        evidence.write_json(execution_path, execution)
        evidence.write_json(inventory_path, {"schema_version": 1, "kind": "experiment-artifact-inventory",
                                             "artifacts": evidence.artifact_inventory(first_bundle, profile, execution)})
        terminal["bundle_seals"][0]["inventory_sha256"] = evidence.sha256(inventory_path)
        terminal["bundle_seals"][0]["execution_sha256"] = evidence.sha256(execution_path)
        evidence.write_json(campaign / "terminal-manifest.json", terminal)
        require_rejection(lambda: tool._validate_campaign(campaign, control, profile_path, commit), (tool.RuntimeErrorEvidence, evidence.EvidenceError), "REC accepted an observed command outside the launch plan")
        execution_path.write_bytes(original_execution); inventory_path.write_bytes(original_inventory)
        # E3: command chronology, PID, and duration are evidence, not advisory metadata.
        for field, value in (("started_utc", "not-a-timestamp"), ("ended_utc", "2026-01-01T00:00:03+00:00"), ("pid", 0), ("elapsed_seconds", -1.0)):
            terminal = copy.deepcopy(original_terminal)
            execution = json.loads(original_execution.decode("utf-8")); execution["records"][0][field] = value
            evidence.write_json(execution_path, execution)
            evidence.write_json(inventory_path, {"schema_version": 1, "kind": "experiment-artifact-inventory",
                                                 "artifacts": evidence.artifact_inventory(first_bundle, profile, execution)})
            terminal["bundle_seals"][0]["inventory_sha256"] = evidence.sha256(inventory_path)
            terminal["bundle_seals"][0]["execution_sha256"] = evidence.sha256(execution_path)
            evidence.write_json(campaign / "terminal-manifest.json", terminal)
            require_rejection(lambda: tool._validate_campaign(campaign, control, profile_path, commit), (tool.RuntimeErrorEvidence, evidence.EvidenceError), f"REC accepted invalid command {field}")
        execution_path.write_bytes(original_execution); inventory_path.write_bytes(original_inventory)
        # E3/E7: negative-fixture evidence requires complete child-process provenance.
        terminal = copy.deepcopy(original_terminal)
        terminal["negative_fixtures"][0]["record"]["pid"] = 0
        evidence.write_json(campaign / "terminal-manifest.json", terminal)
        require_rejection(lambda: tool._validate_campaign(campaign, control, profile_path, commit), (tool.RuntimeErrorEvidence, evidence.EvidenceError), "REC accepted incomplete negative-fixture provenance")
        terminal = copy.deepcopy(original_terminal)
        # E5/E6: terminal comparisons are recomputed rather than accepted by shape alone.
        terminal["replay_comparisons"][0]["differences"] = []
        evidence.write_json(campaign / "terminal-manifest.json", terminal)
        require_rejection(lambda: tool._validate_campaign(campaign, control, profile_path, commit), (tool.RuntimeErrorEvidence, evidence.EvidenceError), "REC accepted stale replay comparison")
        terminal = copy.deepcopy(original_terminal)
        # E4: a duplicate seal cannot replace the seal for a different bundle.
        terminal["bundle_seals"][1]["bundle"] = terminal["bundle_seals"][0]["bundle"]
        evidence.write_json(campaign / "terminal-manifest.json", terminal)
        require_rejection(lambda: tool._validate_campaign(campaign, control, profile_path, commit), (tool.RuntimeErrorEvidence, evidence.EvidenceError), "REC accepted non-bijective bundle seals")
        terminal = copy.deepcopy(original_terminal)
        # E7: a shallow prerequisite manifest is not complete terminal evidence.
        numeric_path = campaign / prerequisites["numeric_contract"]["manifest"]
        evidence.write_json(numeric_path, {"state": "EXECUTED_PENDING_AUDIT", "candidate": {"commit": commit}, "execution": {"records": ["synthetic"]}})
        terminal["prerequisites"]["numeric_contract"]["manifest_sha256"] = evidence.sha256(numeric_path)
        evidence.write_json(campaign / "terminal-manifest.json", terminal)
        require_rejection(lambda: tool._validate_campaign(campaign, control, profile_path, commit), (tool.RuntimeErrorEvidence, evidence.EvidenceError), "REC accepted shallow Numeric prerequisite")
        write_prerequisites(evidence, architecture_runner, numeric_runner, numeric_evidence, campaign, source, source_inventory, commit)
        # E7: prerequisite artifacts and command matrices must be complete and hash-bound.
        original_numeric = numeric_path.read_bytes()
        numeric_manifest = json.loads(original_numeric.decode("utf-8"))
        numeric_manifest["execution"]["records"][0]["certificate_artifacts"][0]["sha256"] = "0" * 64
        evidence.write_json(numeric_path, numeric_manifest)
        terminal = copy.deepcopy(original_terminal)
        terminal["prerequisites"]["numeric_contract"]["manifest_sha256"] = evidence.sha256(numeric_path)
        evidence.write_json(campaign / "terminal-manifest.json", terminal)
        require_rejection(lambda: tool._validate_campaign(campaign, control, profile_path, commit), (tool.RuntimeErrorEvidence, evidence.EvidenceError), "REC accepted an unbound Numeric artifact")
        numeric_path.write_bytes(original_numeric)
        numeric_manifest = json.loads(original_numeric.decode("utf-8"))
        numeric_manifest["execution"]["records"][0]["records"][0]["command"] = ["unexpected-command"]
        evidence.write_json(numeric_path, numeric_manifest)
        terminal = copy.deepcopy(original_terminal)
        terminal["prerequisites"]["numeric_contract"]["manifest_sha256"] = evidence.sha256(numeric_path)
        evidence.write_json(campaign / "terminal-manifest.json", terminal)
        require_rejection(lambda: tool._validate_campaign(campaign, control, profile_path, commit), (tool.RuntimeErrorEvidence, evidence.EvidenceError), "REC accepted a Numeric command outside its plan")
        numeric_path.write_bytes(original_numeric)
        numeric_manifest = json.loads(original_numeric.decode("utf-8"))
        numeric_manifest["execution"]["architecture_preservation"]["manifest"] = str(campaign / "unlinked" / "manifest.json")
        evidence.write_json(numeric_path, numeric_manifest)
        terminal = copy.deepcopy(original_terminal)
        terminal["prerequisites"]["numeric_contract"]["manifest_sha256"] = evidence.sha256(numeric_path)
        evidence.write_json(campaign / "terminal-manifest.json", terminal)
        require_rejection(lambda: tool._validate_campaign(campaign, control, profile_path, commit), (tool.RuntimeErrorEvidence, evidence.EvidenceError), "REC accepted an unlinked Architecture prerequisite")
        numeric_path.write_bytes(original_numeric)
        numeric_manifest = json.loads(original_numeric.decode("utf-8"))
        certificate_path = numeric_path.parent / numeric_manifest["execution"]["records"][0]["certificate_artifacts"][0]["path"]
        original_certificate = certificate_path.read_bytes()
        certificate_path.write_text('{"schema_version":1,"kind":"numeric-contract-certificate"}\n', encoding="utf-8")
        numeric_manifest["execution"]["records"][0]["certificate_artifacts"][0]["sha256"] = evidence.sha256(certificate_path)
        evidence.write_json(numeric_path, numeric_manifest)
        terminal = copy.deepcopy(original_terminal)
        terminal["prerequisites"]["numeric_contract"]["manifest_sha256"] = evidence.sha256(numeric_path)
        evidence.write_json(campaign / "terminal-manifest.json", terminal)
        require_rejection(lambda: tool._validate_campaign(campaign, control, profile_path, commit), (tool.RuntimeErrorEvidence, evidence.EvidenceError), "REC accepted a semantically invalid Numeric certificate")
        certificate_path.write_bytes(original_certificate); numeric_path.write_bytes(original_numeric)
        architecture_path = campaign / prerequisites["architecture_contract"]["manifest"]
        original_architecture = architecture_path.read_bytes()
        architecture_manifest = json.loads(original_architecture.decode("utf-8"))
        architecture_manifest["execution"]["records"][0]["records"] = architecture_manifest["execution"]["records"][0]["records"][1:]
        evidence.write_json(architecture_path, architecture_manifest)
        terminal = copy.deepcopy(original_terminal)
        terminal["prerequisites"]["architecture_contract"]["manifest_sha256"] = evidence.sha256(architecture_path)
        evidence.write_json(campaign / "terminal-manifest.json", terminal)
        require_rejection(lambda: tool._validate_campaign(campaign, control, profile_path, commit), (tool.RuntimeErrorEvidence, evidence.EvidenceError), "REC accepted an incomplete Architecture command matrix")
        architecture_path.write_bytes(original_architecture)
        original_input = (source / "input").read_bytes()
        (source / "input").write_text("tampered revision-bound input\n", encoding="utf-8")
        evidence.write_json(campaign / "terminal-manifest.json", original_terminal)
        require_rejection(lambda: tool._validate_campaign(campaign, control, profile_path, commit), (tool.RuntimeErrorEvidence, evidence.EvidenceError), "REC accepted a changed prerequisite input identity")
        (source / "input").write_bytes(original_input)
        evidence.write_json(campaign / "terminal-manifest.json", original_terminal)
        terminal = original_terminal
        evidence.write_json(campaign / "terminal-manifest.json", terminal)
        (campaign / "build" / "CMakeFiles").mkdir(parents=True); (campaign / "build" / "CMakeFiles" / "object.o").write_bytes(b"object")
        tool.assemble(campaign, destination, commit, "archive-sha", control, profile_path)
        tool.verify(destination, profile_path)
        retained = json.loads((destination / "retention-manifest.json").read_text(encoding="utf-8"))
        if not retained["files"][0]["source_path"].startswith(str(campaign)) or (destination / "build").exists():
            raise RuntimeError("retention package did not preserve evidence policy")
        (destination / "cells" / "gcc-debug" / "replay-1" / "summary.json").write_text("tampered\n", encoding="utf-8")
        try:
            tool.verify(destination, profile_path)
        except tool.RuntimeErrorEvidence:
            pass
        else:
            raise RuntimeError("retention verifier accepted tampered evidence")
        blocked_campaign, blocked_control = root / "blocked-campaign", root / "blocked-control"
        blocked_destination = root / "evidence" / "foundation" / "rec" / "blocked-synthetic"
        blocked_control.mkdir(); evidence.write_json(blocked_control / "prepared-manifest.json", prepared)
        shutil.copy2(control / "launch-plan.json", blocked_control / "launch-plan.json")
        blocked_prepared_sha = evidence.sha256(blocked_control / "prepared-manifest.json")
        evidence.write_json(blocked_control / "state.json", {"state": "BLOCKED"})
        (blocked_control / "state-history.jsonl").write_text('{"state":"PREPARED"}\n{"state":"RUNNING"}\n{"state":"BLOCKED"}\n', encoding="utf-8")
        failure = blocked_campaign / "partial" / "execution-record.json"; failure.parent.mkdir(parents=True); evidence.write_json(failure, {"failure": "synthetic"})
        evidence.write_json(blocked_campaign / "terminal-manifest.json", {"schema_version": 1, "kind": "reproducible-experiment-terminal-manifest", "state": "BLOCKED",
                                                                            "prepared_manifest_sha256": blocked_prepared_sha, "candidate_commit": commit,
                                                                            "attempted_bundles": [], "reason": "synthetic failure",
                                                                            "failure_evidence": [{"path": failure.relative_to(blocked_campaign).as_posix(), "sha256": evidence.sha256(failure)}]})
        tool.assemble(blocked_campaign, blocked_destination, commit, "archive-sha", blocked_control, profile_path)
        tool.verify(blocked_destination, profile_path)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
