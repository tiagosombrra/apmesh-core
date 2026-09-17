#!/usr/bin/env python3

import argparse
import importlib.util
import json
import pathlib
import subprocess
import sys
import tempfile
from unittest.mock import patch


def run(command: list[str]) -> None:
    completed = subprocess.run(command, capture_output=True, text=True, check=False)
    if completed.returncode != 0:
        raise RuntimeError(f"runner failed: {completed.stderr}")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--runner", required=True)
    parser.add_argument("--source-root", required=True)
    parser.add_argument("--profile", required=True)
    parser.add_argument("--protocol", required=True)
    parser.add_argument("--exporter", required=True)
    parser.add_argument("--validator", required=True)
    arguments = parser.parse_args()
    specification = importlib.util.spec_from_file_location("cartesian_frames_runner", arguments.runner)
    if specification is None or specification.loader is None:
        raise RuntimeError("runner module could not be loaded")
    runner = importlib.util.module_from_spec(specification)
    specification.loader.exec_module(runner)
    common = [sys.executable, arguments.runner, "--source-root", arguments.source_root, "--profile", arguments.profile,
              "--protocol", arguments.protocol, "--exporter", arguments.exporter, "--validator", arguments.validator]
    with tempfile.TemporaryDirectory(prefix="apmesh-core-cf-runner-") as temporary:
        root = pathlib.Path(temporary)
        checked = root / "self-check.json"
        run([*common, "self-check", "--output", str(checked)])
        result = json.loads(checked.read_text(encoding="utf-8"))
        if result["status"] != "PASS" or result["execution_requested"] is not False:
            raise RuntimeError("runner self-check differs")
        plan = root / "plan.json"
        run([*common, "plan", "--output", str(plan)])
        planned = json.loads(plan.read_text(encoding="utf-8"))
        if planned["execution_requested"] is not False or len(planned["cells"]) != 4 or set(planned["gates"].values()) != {"NOT_EXECUTED"}:
            raise RuntimeError("runner plan differs")
        if any(path.name.endswith("manifest.json") for path in root.iterdir()):
            raise RuntimeError("runner created a manifest")
        loaded_profile = json.loads(pathlib.Path(arguments.profile).read_text(encoding="utf-8"))
        runner.validate_prerequisite_discovery(loaded_profile["exact_prerequisite_tests"], loaded_profile["exact_prerequisite_tests"])
        try:
            runner.validate_prerequisite_discovery(loaded_profile["exact_prerequisite_tests"][:-1], loaded_profile["exact_prerequisite_tests"])
        except runner.RunnerError:
            pass
        else:
            raise RuntimeError("runner accepted a missing prerequisite")
        candidate = {"commit": "0123456789abcdef0123456789abcdef01234567", "upstream_commit": "0123456789abcdef0123456789abcdef01234567", "tree_clean": True, "source_root": str(pathlib.Path(arguments.source_root).resolve()), "source_inventory": []}
        prepared = root / "prepared"
        preparation = argparse.Namespace(source_root=arguments.source_root, profile=arguments.profile, protocol=arguments.protocol,
                                         exporter=arguments.exporter, validator=arguments.validator, output_root=str(prepared))
        with patch.object(runner, "published_candidate", return_value=candidate), patch.object(runner, "environment_identity", return_value={"test": "identity"}):
            runner.prepare(preparation)
            runner.validate_prepared(prepared, require_unconsumed=True)
            loaded_root, loaded_manifest, loaded = runner.load_prepared(preparation)
            if loaded_root != prepared.resolve() or loaded_manifest["candidate"] != candidate or loaded != loaded_profile:
                raise RuntimeError("prepared manifest cannot be consumed unchanged")
            (prepared / "execution-claim.json").write_text("{}", encoding="utf-8")
            try:
                runner.load_prepared(preparation)
            except runner.RunnerError:
                pass
            else:
                raise RuntimeError("runner accepted a consumed prepared manifest")
        if json.loads((prepared / "prepared-manifest.json").read_text(encoding="utf-8"))["execution_requested"] is not False:
            raise RuntimeError("prepared manifest enables execution")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
