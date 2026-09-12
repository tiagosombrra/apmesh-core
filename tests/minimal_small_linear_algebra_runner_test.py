#!/usr/bin/env python3
"""Focused no-manifest runner check for LA qualification infrastructure."""

from __future__ import annotations

import argparse
import importlib.util
import subprocess
import sys


def load(path: str):
    spec = importlib.util.spec_from_file_location("la_runner", path)
    if spec is None or spec.loader is None:
        raise RuntimeError("cannot load runner")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--runner", required=True)
    parser.add_argument("--source-root", required=True)
    parser.add_argument("--profile", required=True)
    parser.add_argument("--protocol", required=True)
    arguments = parser.parse_args()
    completed = subprocess.run([
        sys.executable, arguments.runner, "--source-root", arguments.source_root,
        "--profile", arguments.profile, "--protocol", arguments.protocol, "self-check",
    ], capture_output=True, text=True, check=False)
    if completed.returncode != 0:
        raise RuntimeError(f"runner self-check failed: {completed.stderr}")
    runner = load(arguments.runner)
    profile = runner.validate_profile(__import__("pathlib").Path(arguments.profile))
    try:
        runner.validate_prerequisite_discovery(profile["exact_prerequisite_tests"][:-1], profile["exact_prerequisite_tests"])
    except runner.RuntimeErrorEvidence:
        return 0
    raise RuntimeError("missing prerequisite was accepted")


if __name__ == "__main__":
    raise SystemExit(main())
