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
        (candidate / "tools").mkdir()
        for name in ("run_architecture_bootstrap_regression.py", "run_numeric_contract_regression.py"):
            shutil.copy2(runner_path.parent / name, candidate / "tools" / name)
        for command in (["git", "init"], ["git", "config", "user.email", "test@example.invalid"], ["git", "config", "user.name", "Test"], ["git", "add", "tracked.txt", "tools"], ["git", "commit", "-m", "candidate"]):
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
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
