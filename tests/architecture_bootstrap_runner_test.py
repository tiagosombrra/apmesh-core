#!/usr/bin/env python3
import argparse
import importlib.util
import json
import pathlib
import subprocess
import sys
import tempfile


def load_runner(path: pathlib.Path):
    spec = importlib.util.spec_from_file_location("architecture_runner", path)
    if spec is None or spec.loader is None:
        raise RuntimeError("cannot load runner")
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
    parser.add_argument("--expected", required=True)
    parser.add_argument("--comparer", required=True)
    arguments = parser.parse_args()
    runner_path = pathlib.Path(arguments.runner)
    runner = load_runner(runner_path)
    profile = json.loads(pathlib.Path(arguments.profile).read_text(encoding="utf-8"))
    plan = runner.command_plan(profile, pathlib.Path("/source"))
    if [cell["name"] for cell in plan] != ["gcc-debug", "gcc-release", "clang-debug", "clang-release"]:
        raise RuntimeError("configuration plan differs")
    if any(not any("@OUTPUT_ROOT@" in argument for argument in cell["configure"]) for cell in plan):
        raise RuntimeError("plan lacks isolated output roots")
    if any(cell["certificate_processes"] != 3 or cell["consumer_validations"] != 1 for cell in plan):
        raise RuntimeError("repeat or consumer count differs")
    with tempfile.TemporaryDirectory() as temporary_directory:
        temporary_root = pathlib.Path(temporary_directory)
        output = temporary_root / "occupied"
        output.mkdir()
        completed = subprocess.run(
            [sys.executable, str(runner_path), "--source-root", ".", "--profile", arguments.profile,
             "--protocol", arguments.protocol, "--expected", arguments.expected, "--comparer", arguments.comparer,
             "--output-root", str(output)],
            check=False,
            capture_output=True,
        )
        if completed.returncode != 1 or b"output root already exists" not in completed.stderr:
            raise RuntimeError("occupied output root is not rejected")
        source = temporary_root / "source"
        source.mkdir()
        (source / "tracked.txt").write_text("candidate\n", encoding="utf-8")
        for command in (["git", "init"], ["git", "config", "user.email", "test@example.invalid"],
                        ["git", "config", "user.name", "Test"], ["git", "add", "tracked.txt"],
                        ["git", "commit", "-m", "candidate"]):
            subprocess.run(command, cwd=source, check=True, capture_output=True)
        prepared = temporary_root / "prepared"
        completed = subprocess.run(
            [sys.executable, str(runner_path), "--source-root", str(source), "--profile", arguments.profile,
             "--protocol", arguments.protocol, "--expected", arguments.expected, "--comparer", arguments.comparer,
             "--output-root", str(prepared)],
            check=False,
            capture_output=True,
        )
        if completed.returncode != 0:
            raise RuntimeError(f"clean candidate preparation failed: {completed.stderr!r}")
        manifest = json.loads((prepared / "manifest.json").read_text(encoding="utf-8"))
        if manifest["state"] != "PREPARED" or manifest["execution_requested"]:
            raise RuntimeError("prepare mode attempted execution")
        if manifest["candidate"]["tree_clean"] is not True or len(manifest["plan"]) != 4:
            raise RuntimeError("prepared manifest lacks candidate or cell evidence")
        prepared_arguments = __import__("argparse").Namespace(
            source_root=str(source), profile=arguments.profile, protocol=arguments.protocol,
            expected=arguments.expected, comparer=arguments.comparer, output_root=str(prepared),
        )
        loaded_root, loaded_manifest = runner.load_prepared(prepared_arguments)
        if loaded_root != prepared.resolve() or loaded_manifest != manifest:
            raise RuntimeError("prepared manifest cannot be consumed unchanged")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
