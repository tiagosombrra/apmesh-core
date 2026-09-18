#!/usr/bin/env python3
"""Focused preparation-only contracts for the GPR manifest runner."""
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
    specification = importlib.util.spec_from_file_location("gpr_runner", path)
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
    for name in ("runner", "source-root", "profile", "protocol", "exporter", "validator"):
        parser.add_argument(f"--{name}", required=True)
    arguments = parser.parse_args()
    runner = load(arguments.runner)
    source = pathlib.Path(arguments.source_root).resolve()
    profile = json.loads(pathlib.Path(arguments.profile).read_text(encoding="utf-8"))
    if sorted(runner.declared_allowlist(source, profile["semantic_ctest_allowlist"])) != sorted(profile["semantic_ctest_allowlist"]):
        raise RuntimeError("declared allowlist differs")
    common = [sys.executable, arguments.runner, "--source-root", arguments.source_root, "--profile", arguments.profile, "--protocol", arguments.protocol, "--exporter", arguments.exporter, "--validator", arguments.validator]
    with tempfile.TemporaryDirectory(prefix="apmesh-core-gpr-runner-") as temporary:
        root = pathlib.Path(temporary)
        self_check = root / "self-check.json"
        result = subprocess.run([*common, "self-check", "--output", str(self_check)], capture_output=True, text=True, check=False)
        if result.returncode != 0 or json.loads(self_check.read_text(encoding="utf-8"))["execution_requested"] is not False:
            raise RuntimeError("runner self-check differs")
        candidate = {"commit": "0123456789abcdef0123456789abcdef01234567", "upstream_commit": "0123456789abcdef0123456789abcdef01234567", "post_merge_baseline": runner.POST_MERGE_BASELINE, "tree_clean": True, "source_root": str(source), "source_inventory": []}
        output = root / "prepared"
        prepared = argparse.Namespace(source_root=arguments.source_root, profile=arguments.profile, protocol=arguments.protocol, exporter=arguments.exporter, validator=arguments.validator, output_root=str(output))
        with patch.object(runner, "published_candidate", return_value=candidate), patch.object(runner, "environment_identity", return_value={"test": "identity"}):
            runner.prepare(prepared)
            manifest = runner.validate_prepared(output)
            if manifest["execution_requested"] is not False or set(manifest["gates"].values()) != {"NOT_EXECUTED"}:
                raise RuntimeError("prepared manifest claims execution")
            runner.validate_execution_binding(prepared, source, output, manifest)
            saved = (output / "prepared-manifest.json").read_bytes()
            forged = json.loads(saved); forged["execution_requested"] = True
            (output / "prepared-manifest.json").write_text(json.dumps(forged), encoding="utf-8")
            rejects(lambda: runner.validate_prepared(output), "enabled execution was accepted")
            (output / "prepared-manifest.json").write_bytes(saved)
            (output / "execution-claim.json").write_text("{}", encoding="utf-8")
            rejects(lambda: runner.validate_prepared(output), "consumed manifest was accepted")
            (output / "execution-claim.json").unlink()
        existing = root / "existing"; existing.mkdir()
        rejected = argparse.Namespace(source_root=arguments.source_root, profile=arguments.profile, protocol=arguments.protocol, exporter=arguments.exporter, validator=arguments.validator, output_root=str(existing))
        rejects(lambda: runner.prepare(rejected), "existing output root was accepted")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
