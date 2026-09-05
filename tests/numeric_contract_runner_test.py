#!/usr/bin/env python3
"""Focused no-execution checks for the Numeric Contract formal runner."""

from __future__ import annotations

import argparse
import importlib.util
import json
import pathlib
import subprocess
import sys
import tempfile


def load_runner(path: pathlib.Path):
    spec = importlib.util.spec_from_file_location("numeric_runner", path)
    if spec is None or spec.loader is None:
        raise RuntimeError("cannot load numeric runner")
    module = importlib.util.module_from_spec(spec)
    sys.path.insert(0, str(path.parent))
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--runner", required=True)
    parser.add_argument("--profile", required=True)
    parser.add_argument("--protocol", required=True)
    parser.add_argument("--validator", required=True)
    parser.add_argument("--architecture-evidence", required=True)
    arguments = parser.parse_args()
    runner_path = pathlib.Path(arguments.runner)
    runner = load_runner(runner_path)
    profile = json.loads(pathlib.Path(arguments.profile).read_text(encoding="utf-8"))
    plan = runner.command_plan(profile, pathlib.Path("/source"))
    if [cell["name"] for cell in plan] != ["gcc-debug", "gcc-release", "clang-debug", "clang-release"]:
        raise RuntimeError("numeric configuration plan differs")
    if any(cell["repetitions"] != 3 or "@OUTPUT_ROOT@" not in " ".join(cell["configure"]) for cell in plan):
        raise RuntimeError("numeric plan repetition or isolation differs")
    with tempfile.TemporaryDirectory() as temporary_directory:
        root = pathlib.Path(temporary_directory)
        candidate = root / "candidate"
        candidate.mkdir()
        (candidate / "tracked.txt").write_text("candidate\n", encoding="utf-8")
        for command in (["git", "init"], ["git", "config", "user.email", "test@example.invalid"],
                        ["git", "config", "user.name", "Test"], ["git", "add", "tracked.txt"],
                        ["git", "commit", "-m", "candidate"]):
            subprocess.run(command, cwd=candidate, check=True, capture_output=True)
        common = ["--source-root", str(candidate), "--profile", arguments.profile, "--protocol", arguments.protocol,
                  "--validator", arguments.validator, "--architecture-evidence", arguments.architecture_evidence]
        occupied = root / "occupied"
        occupied.mkdir()
        rejected = subprocess.run([sys.executable, str(runner_path), *common, "--output-root", str(occupied)], check=False, capture_output=True)
        if rejected.returncode != 1 or b"output root already exists" not in rejected.stderr:
            raise RuntimeError("occupied output root was not rejected")
        prepared = root / "prepared"
        completed = subprocess.run([sys.executable, str(runner_path), *common, "--output-root", str(prepared)], check=False, capture_output=True)
        if completed.returncode != 0:
            raise RuntimeError(f"clean numeric preparation failed: {completed.stderr!r}")
        manifest = json.loads((prepared / "manifest.json").read_text(encoding="utf-8"))
        if manifest["state"] != "PREPARED" or manifest["execution_requested"] is not False or len(manifest["plan"]) != 4:
            raise RuntimeError("preparation attempted execution or lacks cells")
        parsed = argparse.Namespace(source_root=str(candidate), profile=arguments.profile, protocol=arguments.protocol,
                                    validator=arguments.validator, architecture_evidence=arguments.architecture_evidence,
                                    output_root=str(prepared))
        loaded_root, loaded_manifest = runner.load_prepared(parsed)
        if loaded_root != prepared.resolve() or loaded_manifest != manifest:
            raise RuntimeError("prepared numeric manifest cannot be consumed unchanged")
        (candidate / "changed.txt").write_text("changed\n", encoding="utf-8")
        try:
            runner.load_prepared(parsed)
        except runner.EvidenceError:
            pass
        else:
            raise RuntimeError("dirty candidate was accepted for execution")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
