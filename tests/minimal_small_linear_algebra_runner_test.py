#!/usr/bin/env python3
"""Disposable-repository lifecycle tests; never prepares a scientific candidate."""
from __future__ import annotations

import argparse
import importlib.util
import pathlib
import shutil
import subprocess
import tempfile
import os
import copy
from unittest.mock import patch


def load(path):
    spec = importlib.util.spec_from_file_location("la_runner", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def git(root, *argv):
    result = subprocess.run(["git", "-c", "user.name=Contract Test", "-c", "user.email=contract@example.invalid", *argv], cwd=root, capture_output=True, text=True)
    if result.returncode: raise RuntimeError(result.stderr)
    return result.stdout.strip()


def sandbox(parent, original):
    """Actual local commit + local upstream, deliberately failing CMake input."""
    seed = parent / "seed"; seed.mkdir()
    for folder in ("include", "src", "tools", "tests", "experiments", "docs"):
        shutil.copytree(original / folder, seed / folder, ignore=shutil.ignore_patterns("__pycache__", "*.pyc"))
    for name in ("CMakeLists.txt", "CMakePresets.json"):
        shutil.copyfile(original / name, seed / name)
    (seed / ".gitignore").write_text("__pycache__/\n*.pyc\n", encoding="utf-8")
    (seed / ".gitattributes").write_text("* text=auto eol=lf\n", encoding="utf-8")
    with (seed / "CMakeLists.txt").open("a", encoding="utf-8") as stream:
        stream.write('\nmessage(FATAL_ERROR "LA_TEST_EXPECTED_CONFIGURE_FAILURE")\n')
    git(seed, "init", "-b", "test-candidate")
    git(seed, "add", ".")
    git(seed, "commit", "-m", "test: disposable lifecycle fixture")
    root = parent / "candidate"
    git(parent, "clone", "--quiet", str(seed), str(root))
    runner = load(root / "tools/run_minimal_small_linear_algebra_qualification.py")
    args = argparse.Namespace(source_root=str(root), output_root=str(parent / "attempt"),
                              profile=str(root / "experiments/profiles/minimal_small_linear_algebra.json"),
                              protocol=str(root / "docs/decisions/GEOMETRY_MINIMAL_SMALL_LINEAR_ALGEBRA_QUALIFICATION_PROTOCOL.md"))
    return runner, args, root


def rejects(action, message):
    try: action()
    except RuntimeError: return
    raise RuntimeError(message)


def sealing_failure_checks(runner, args, root, parent):
    """Faults at sealing boundaries, not fabricated qualification successes."""
    for stage in ("detached-verification", "inventory", "verification"):
        args = argparse.Namespace(**vars(args)); args.output_root = str(parent / ("seal-" + stage))
        output = pathlib.Path(args.output_root)
        runner.prepare(args)
        _, manifest, _ = runner.load_prepared(args)
        runner.write_json(output / "execution-claim.json", {"candidate_commit": manifest["candidate"]["commit"], "prepared_manifest_sha256": runner.sha256_file(output / "prepared-manifest.json"), "pid": os.getpid(), "started_utc": runner.utc_now()})
        runner.write_state(output, "RUNNING", {"candidate_commit": manifest["candidate"]["commit"]})
        cell = manifest["plan"][0]
        record = runner.run_command(runner.replace_root(cell["source_check"], output), root, output / "logs", "gcc-debug-source_check", 30)
        runner.require_success(record, "disposable source check")
        runner.write_command_records(output, [record])
        # Deliberately incomplete success candidate: real verifier must reject it.
        runner.write_terminal(output, manifest, "EXECUTED_PENDING_AUDIT", {})
        runner.write_state(output, "EXECUTED_PENDING_AUDIT", {"candidate_commit": manifest["candidate"]["commit"]})
        original_writer = runner.write_retention_inventory
        def inventory_fault(out, prepared, required):
            if required == runner.TERMINAL_REQUIRED_SUCCESS: raise OSError("injected inventory write failure")
            return original_writer(out, prepared, required)
        with patch.object(runner, "verify_detached_candidate", wraps=runner.verify_detached_candidate) as detached, patch.object(runner, "write_retention_inventory", wraps=original_writer) as inventory:
            if stage == "detached-verification": detached.side_effect = OSError("injected detached I/O failure")
            if stage == "inventory": inventory.side_effect = inventory_fault
            try: runner.seal_output(output, root, manifest, runner.TERMINAL_REQUIRED_SUCCESS)
            except (RuntimeError, OSError): pass
            else: raise RuntimeError("incomplete or failed sealing was accepted")
            if detached.call_count != 1: raise RuntimeError("detached verification was retried")
            if sum(call.args[2] == runner.TERMINAL_REQUIRED_SUCCESS for call in inventory.call_args_list) > 1: raise RuntimeError("success sealing was retried")
        if runner.read_json(output / "retention-error.json")["stage"] != stage: raise RuntimeError("wrong sealing failure exercised")
        if runner.read_json(output / "state.json")["state"] != "BLOCKED": raise RuntimeError("sealing failure is not BLOCKED")
        runner.verify_retention(args)
        relocated = parent / ("relocated-seal-" + stage)
        shutil.copytree(output, relocated)
        runner.verify_retention(argparse.Namespace(output_root=str(relocated)))
        # The preserved error cannot be promoted or rebound by recomputing hashes.
        for name, field, value in (("detached-verification.json", "result", "PASS"), ("retention-error.json", "candidate_commit", "0" * 40), ("retention-error.json", "retry_attempted", True), ("pre-seal-terminal.json", "candidate", {})):
            path = output / name; saved = path.read_bytes()
            retention = output / "retention-manifest.json"; saved_retention = retention.read_bytes()
            changed = runner.read_json(path); changed[field] = value; runner.write_json(path, changed)
            original_writer(output, manifest, runner.TERMINAL_REQUIRED_FAILURE | runner.SEAL_FAILURE_PATHS)
            rejects(lambda: runner.verify_retention(args), f"rehashed seal failure mutation accepted: {name}/{field}")
            path.write_bytes(saved); retention.write_bytes(saved_retention)
        rejects(lambda: runner.execute(args), "seal-failed attempt was reused")


def negative_command_checks(runner, root, original_build, parent, profile):
    """Use the planned CLI and real exporter, without preparing a campaign."""
    output = parent / "negative-command-test"; (output / "certificates").mkdir(parents=True)
    cell = runner.command_plan(profile, root)[0]
    certificate = output / "certificates/gcc-debug-1.json"
    exporter = original_build / "apmesh_core_minimal_small_linear_algebra_export"
    record = runner.run_command([str(exporter), str(certificate)], root, output / "logs", "focused-export", 30)
    runner.require_success(record, "focused negative baseline")
    record = runner.run_command(runner.replace_root(cell["negative_outcomes"], output), root, output / "logs", "gcc-debug-negative-outcomes", 30)
    runner.require_success(record, "focused negative CLI")
    # Minimal arguments for the same binding verifier called by verify_retention().
    manifest = {"output_root": str(output), "inputs": {"validator": {"sha256": runner.sha256_file(root / "tools/minimal_small_linear_algebra_evidence.py")}}}
    runner.validate_negative_cell(output, manifest, cell, [record], profile)
    relocated = parent / "negative-command-relocated"; shutil.copytree(output, relocated)
    runner.validate_negative_cell(relocated, manifest, cell, [record], profile)
    rejects(lambda: runner.validate_negative_cell(output, manifest, cell, [], profile), "missing negative command accepted")
    for field, value in (("argv", ["wrong-command"]), ("exit_code", 1)):
        altered = copy.deepcopy(record); altered[field] = value
        rejects(lambda: runner.validate_negative_cell(output, manifest, cell, [altered], profile), "invalid negative command accepted")
    altered_manifest = copy.deepcopy(manifest); altered_manifest["inputs"]["validator"]["sha256"] = "0" * 64
    rejects(lambda: runner.validate_negative_cell(output, altered_manifest, cell, [record], profile), "unbound negative validator accepted")


def lifecycle_checks(original, original_build):
    with tempfile.TemporaryDirectory(prefix="apmesh-la-lifecycle-test-") as temporary:
        runner, args, root = sandbox(pathlib.Path(temporary), original)
        output = pathlib.Path(args.output_root)
        runner.prepare(args)
        runner.load_prepared(args)
        inventory = runner.read_json(output / "planned-inventories.json")
        conditions = {row["path"]: row["required_when"] for row in inventory["artifacts"]}
        for name in runner.TERMINAL_REQUIRED_SUCCESS & runner.TERMINAL_REQUIRED_FAILURE:
            if conditions[name] != "always": raise RuntimeError("common terminal artifact is not unconditional")
        for name in runner.TERMINAL_REQUIRED_FAILURE - runner.TERMINAL_REQUIRED_SUCCESS:
            if conditions[name] != "failure": raise RuntimeError("failure-only artifact condition differs")
        for name in runner.SEAL_FAILURE_PATHS:
            if conditions[name] != "seal-failure": raise RuntimeError("seal failure condition differs")
        for name in ("prepared-manifest.json", "plan.json", "planned-inventories.json", "state.json", "state-history.jsonl", "preparation-seal.json"):
            path = output / name; original_bytes = path.read_bytes()
            path.write_bytes(original_bytes.replace(b'"schema_version":', b'"altered_schema_version":', 1))
            rejects(lambda: runner.load_prepared(args), f"mutation accepted: {name}")
            path.write_bytes(original_bytes)
        path = output / "execution-claim.json"; path.write_text("{}", encoding="utf-8")
        rejects(lambda: runner.load_prepared(args), "consumed attempt accepted")
        path.unlink()
        dirty = root / "untracked.txt"; dirty.write_text("dirty", encoding="utf-8")
        rejects(lambda: runner.load_prepared(args), "dirty candidate accepted")
        dirty.unlink()
        git(root, "config", "--unset", "branch.test-candidate.remote")
        rejects(lambda: runner.load_prepared(args), "unpublished candidate accepted")
        git(root, "config", "branch.test-candidate.remote", "origin")
        profile = runner.validate_profile(pathlib.Path(args.profile))
        rejects(lambda: runner.validate_prerequisite_discovery(profile["exact_prerequisite_tests"][:-1], profile["exact_prerequisite_tests"]), "missing prerequisite accepted")
        rejects(lambda: runner.validate_prerequisite_discovery(profile["exact_prerequisite_tests"] + ["unexpected"], profile["exact_prerequisite_tests"]), "additional prerequisite accepted")
        rejects(lambda: runner.execute(args), "intentional configure failure did not fail")
        terminal = runner.read_json(output / "terminal-manifest.json")
        if terminal["state"] != "BLOCKED": raise RuntimeError("failed execution is not terminal")
        records = runner.read_json(output / "command-records.json")["records"]
        if len(records) != 2 or records[-1]["exit_code"] == 0 or not records[-1]["pid"]:
            raise RuntimeError("real configure failure provenance is absent")
        stderr = (output / records[-1]["stderr"]["path"]).read_text(encoding="utf-8")
        if "LA_TEST_EXPECTED_CONFIGURE_FAILURE" not in stderr: raise RuntimeError("wrong failure exercised")
        runner.verify_retention(args)
        relocated = pathlib.Path(temporary) / "relocated"
        shutil.copytree(output, relocated, ignore=shutil.ignore_patterns("build"))
        runner.verify_retention(argparse.Namespace(output_root=str(relocated)))
        before = {path.relative_to(output).as_posix(): path.read_bytes() for path in output.rglob("*") if path.is_file()}
        rejects(lambda: runner.execute(args), "failed attempt was reexecuted")
        after = {path.relative_to(output).as_posix(): path.read_bytes() for path in output.rglob("*") if path.is_file()}
        if before != after: raise RuntimeError("reuse mutated terminal evidence")
        for name in ("terminal-manifest.json", "detached-verification.json", "failure.json", "command-records.json"):
            path = output / name; data = path.read_bytes(); path.write_text("{}", encoding="utf-8")
            rejects(lambda: runner.verify_retention(args), f"retained mutation accepted: {name}")
            path.write_bytes(data)
        # Recomputed file hashes do not excuse contradictions in candidate, lifecycle or logs.
        for name, field, value in (("terminal-manifest.json", "candidate", {}), ("detached-verification.json", "candidate_commit", "0" * 40), ("failure.json", "records", [])):
            path = output / name; data = path.read_bytes()
            retention_path = output / "retention-manifest.json"; retention_bytes = retention_path.read_bytes()
            altered = runner.read_json(path); altered[field] = value; runner.write_json(path, altered)
            retained = runner.read_json(retention_path)
            for row in retained["files"]:
                if row["path"] == name: row.update(sha256=runner.sha256_file(path), size=path.stat().st_size)
            runner.write_json(retention_path, retained)
            rejects(lambda: runner.verify_retention(args), f"rehashed inconsistent evidence accepted: {name}")
            path.write_bytes(data); retention_path.write_bytes(retention_bytes)
        # Exercise compiler dependency parsing with actual Ninja output in the invoking build.
        if not (original_build / "build.ninja").is_file(): raise RuntimeError("focused build root is absent")
        if original_build.is_dir():
            record = runner.run_command(["ninja", "-C", str(original_build), "-t", "deps"], original, pathlib.Path(temporary) / "dep-logs", "observed-deps", 30)
            runner.require_success(record, "focused dependency discovery")
            runner.validate_observed_dependencies(record, pathlib.Path(temporary))
            path = pathlib.Path(temporary) / record["stdout"]["path"]
            path.write_text(path.read_text(encoding="utf-8").replace("src/math/linear_algebra.cpp.o:", "src/math/missing.cpp.o:"), encoding="utf-8")
            record["stdout"]["sha256"] = runner.sha256_file(path)
            rejects(lambda: runner.validate_observed_dependencies(record, pathlib.Path(temporary)), "missing compiled math dependency accepted")
        sealing_failure_checks(runner, args, root, pathlib.Path(temporary))
        negative_command_checks(runner, root, original_build, pathlib.Path(temporary), profile)
        print("PASS: preparation mutation, publication, discovery, actual subprocess failure, conditional inventories, sealing failures, structured negative CLI/binding, relocated retention and single use")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--runner", required=True); parser.add_argument("--source-root", required=True)
    parser.add_argument("--profile", required=True); parser.add_argument("--protocol", required=True)
    parser.add_argument("--build-root", required=True)
    args = parser.parse_args()
    lifecycle_checks(pathlib.Path(args.source_root), pathlib.Path(args.build_root))
    return 0


if __name__ == "__main__": raise SystemExit(main())
