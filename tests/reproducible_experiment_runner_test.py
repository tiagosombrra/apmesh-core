#!/usr/bin/env python3
"""Focused preparation, identity, root-separation and lifecycle contracts."""

from __future__ import annotations

import argparse
import importlib.util
import json
import pathlib
import shutil
import subprocess
import sys
import tempfile


def load_runner(path: pathlib.Path):
    sys.path.insert(0, str(path.parent))
    spec = importlib.util.spec_from_file_location("rec_runner", path)
    if spec is None or spec.loader is None:
        raise RuntimeError("cannot load REC runner")
    module = importlib.util.module_from_spec(spec); sys.modules[spec.name] = module; spec.loader.exec_module(module)
    return module


def must_reject(callback, error_type, message: str) -> None:
    try:
        callback()
    except error_type:
        return
    raise RuntimeError(message)


def main() -> int:
    parser = argparse.ArgumentParser()
    for name in ("runner", "profile", "contract", "protocol", "collector"):
        parser.add_argument(f"--{name}", required=True)
    arguments = parser.parse_args(); runner_path = pathlib.Path(arguments.runner); runner = load_runner(runner_path)
    profile = json.loads(pathlib.Path(arguments.profile).read_text(encoding="utf-8"))
    if len(runner.command_plan(profile, pathlib.Path("/source"))) != 8:
        raise RuntimeError("REC plan does not define eight independent replay cells")
    with tempfile.TemporaryDirectory() as temporary_directory:
        root, candidate = pathlib.Path(temporary_directory), pathlib.Path(temporary_directory) / "candidate"
        candidate.mkdir(); (candidate / "tracked.txt").write_text("candidate\n", encoding="utf-8")
        source_repository = runner_path.parent.parent
        for relative in (
            "tools/run_architecture_bootstrap_regression.py", "tools/run_numeric_contract_regression.py",
            "tools/numeric_contract_evidence.py", "tools/bootstrap_regression.py", "tools/reproducible_experiment_negative.py",
            "experiments/profiles/numeric_contract.json", "experiments/profiles/architecture_bootstrap.json",
            "docs/decisions/FOUNDATION_NUMERIC_CONTRACT_QUALIFICATION.md",
            "docs/decisions/FOUNDATION_ARCHITECTURE_BOOTSTRAP_REGRESSION.md",
            "experiments/expected/bootstrap_certificate.json",
        ):
            target = candidate / relative; target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(source_repository / relative, target)
        for command in (["git", "init"], ["git", "config", "user.email", "test@example.invalid"], ["git", "config", "user.name", "Test"], ["git", "add", "tracked.txt", "tools", "experiments", "docs"], ["git", "commit", "-m", "candidate"]):
            subprocess.run(command, cwd=candidate, check=True, capture_output=True)
        control, evidence = root / "control", root / "evidence"
        common = ["--source-root", str(candidate), "--profile", arguments.profile, "--contract", arguments.contract,
                  "--protocol", arguments.protocol, "--collector", arguments.collector, "--control-root", str(control), "--evidence-root", str(evidence)]
        completed = subprocess.run([sys.executable, str(runner_path), *common], check=False, capture_output=True)
        if completed.returncode != 0:
            raise RuntimeError(f"REC preparation failed: {completed.stderr!r}")
        manifest = json.loads((control / "prepared-manifest.json").read_text(encoding="utf-8"))
        if manifest["state"] != "PREPARED" or manifest["execution_requested"] is not False or len(manifest["planned_cells"]) != 8 or evidence.exists():
            raise RuntimeError("REC preparation did not preserve control/evidence separation")
        parsed = argparse.Namespace(source_root=str(candidate), profile=arguments.profile, contract=arguments.contract, protocol=arguments.protocol,
                                    collector=arguments.collector, control_root=str(control), evidence_root=str(evidence))
        runner.load_prepared(parsed); runner.admit_execution(parsed)
        # N2: changed profile bytes; N1: dirty candidate; N3: nonempty evidence root; N8: terminal reuse.
        modified_profile = root / "profile.json"; modified_profile.write_bytes(pathlib.Path(arguments.profile).read_bytes() + b"\n")
        changed_input = argparse.Namespace(**{**vars(parsed), "profile": str(modified_profile)})
        must_reject(lambda: runner.load_prepared(changed_input), (runner.EvidenceError, runner.RuntimeErrorEvidence), "REC accepted changed declared input")
        (candidate / "dirty.txt").write_text("dirty\n", encoding="utf-8")
        must_reject(lambda: runner.load_prepared(parsed), (runner.EvidenceError, runner.RuntimeErrorEvidence), "REC accepted dirty candidate")
        (candidate / "dirty.txt").unlink(); evidence.mkdir(); (evidence / "old.txt").write_text("old\n", encoding="utf-8")
        must_reject(lambda: runner.admit_execution(parsed), (runner.EvidenceError, runner.RuntimeErrorEvidence), "REC accepted nonempty evidence root")
        (evidence / "old.txt").unlink(); evidence.rmdir()
        runner.write_state(control, "BLOCKED", {"reason": "synthetic negative fixture"})
        must_reject(lambda: runner.admit_execution(parsed), (runner.EvidenceError, runner.RuntimeErrorEvidence), "REC reused terminal state")
        # N7: a failed command remains explicit rather than being retried or rewritten.
        record = runner.run_command([sys.executable, "-c", "raise SystemExit(7)"], root, root / "logs", "failing", 10)
        if record["exit_code"] != 7 or record["timed_out"] or record["pid"] is None:
            raise RuntimeError("REC command record did not preserve failed-process evidence")
        failure_control, failure_evidence = root / "failure-control", root / "failure-evidence"
        failure_arguments = argparse.Namespace(**{**vars(parsed), "control_root": str(failure_control), "evidence_root": str(failure_evidence)})
        runner.prepare(failure_arguments)
        original_write_bundle = runner._write_bundle
        def synthetic_failure(bundle, *unused):
            bundle.mkdir(parents=True)
            runner.write_json(bundle / "execution-record.json", {"synthetic": "recorded-before-failure"})
            raise runner.EvidenceError("synthetic command failure")
        runner._write_bundle = synthetic_failure
        try:
            if runner.execute(failure_arguments) != 1:
                raise RuntimeError("REC synthetic failure was not blocked")
        finally:
            runner._write_bundle = original_write_bundle
        terminal = json.loads((failure_evidence / "terminal-manifest.json").read_text(encoding="utf-8"))
        if terminal.get("state") != "BLOCKED" or not terminal.get("failure_evidence"):
            raise RuntimeError("REC blocked terminal did not retain failure evidence")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
