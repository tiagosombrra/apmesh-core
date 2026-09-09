#!/usr/bin/env python3
"""Verify the exact CTest set required by the Point/Vector PV6 gate."""

from __future__ import annotations

import argparse
import json
import pathlib
import subprocess


EXPECTED = {
    "apmesh_core.bootstrap_smoke",
    "apmesh_core.bootstrap_export",
    "apmesh_core.bootstrap_tool",
    "apmesh_core.architecture_bootstrap_runner",
    "apmesh_core.numeric_contract",
    "apmesh_core.numeric_contract_evidence",
    "apmesh_core.numeric_contract_runner",
    "apmesh_core.reproducible_experiment_evidence",
    "apmesh_core.reproducible_experiment_runner",
    "apmesh_core.reproducible_experiment_retention",
    "apmesh_core.reproducible_experiment_negatives",
}


def labels(test: dict) -> list[str]:
    for property_value in test.get("properties", []):
        if property_value.get("name") == "LABELS":
            value = property_value.get("value")
            return value if isinstance(value, list) and all(isinstance(item, str) for item in value) else []
    return []


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--ctest", required=True)
    parser.add_argument("--build-dir", required=True)
    arguments = parser.parse_args()

    completed = subprocess.run(
        [arguments.ctest, "--test-dir", str(pathlib.Path(arguments.build_dir).resolve()), "--show-only=json-v1"],
        check=False,
        capture_output=True,
        text=True,
    )
    if completed.returncode != 0:
        raise RuntimeError(f"CTest discovery failed: {completed.stderr.strip()}")
    payload = json.loads(completed.stdout)
    tests = payload.get("tests")
    if not isinstance(tests, list):
        raise RuntimeError("CTest discovery schema lacks tests")
    observed = {test.get("name") for test in tests if isinstance(test, dict) and "foundation-preservation" in labels(test)}
    if observed != EXPECTED:
        raise RuntimeError(f"PV6 preservation test set differs: expected={sorted(EXPECTED)}, observed={sorted(observed)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
