#!/usr/bin/env python3
"""Static contract for the manual preparation-only TMR workflow."""

from __future__ import annotations

import argparse
import pathlib
import re


def require(condition: bool, message: str) -> None:
    if not condition:
        raise RuntimeError(message)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--workflow", required=True)
    arguments = parser.parse_args()

    path = pathlib.Path(arguments.workflow)
    text = path.read_text(encoding="utf-8")

    require("name: Topological Model TMR Preparation" in text, "workflow identity differs")
    require(re.search(r"(?m)^on:\n  workflow_dispatch:\s*$", text) is not None, "workflow is not manual-only")
    require(re.search(r"(?m)^\s+push:\s*$", text) is None, "workflow has a push trigger")
    require(re.search(r"(?m)^\s+pull_request:\s*$", text) is None, "workflow has a pull-request trigger")
    require(re.search(r"(?m)^\s+schedule:\s*$", text) is None, "workflow has a scheduled trigger")
    require('if [ "${GITHUB_REF}" != "refs/heads/main" ]; then' in text, "workflow is not restricted to main")
    require("fetch-depth: 0" in text, "workflow does not fetch candidate ancestry")
    require('OUTPUT_ROOT: ${{ runner.temp }}/apmesh-tmr-prepared' in text, "workflow output is not external runner temp")
    require('test ! -e "${OUTPUT_ROOT}"' in text, "workflow does not require a new output root")
    require(re.search(r"(?m)^\s+prepare \\\s*$", text) is not None, "workflow does not invoke prepare")
    require(re.search(r"(?m)^\s+validate-prepared \\\s*$", text) is not None, "workflow does not validate PREPARED state")
    require(re.search(r"(?m)^\s+execute(?:\s|\\|$)", text) is None, "workflow exposes formal execution")
    require('test ! -e "${OUTPUT_ROOT}/execution-claim.json"' in text, "workflow does not reject execution evidence")
    require('test ! -e "${OUTPUT_ROOT}/terminal-manifest.json"' in text, "workflow does not reject terminal evidence")
    require("actions/checkout@3d3c42e5aac5ba805825da76410c181273ba90b1" in text, "checkout action is not pinned")
    require("actions/upload-artifact@ea165f8d65b6e75b540449e92b4886f43607fa02" in text, "artifact action is not pinned")
    require('path: ${{ runner.temp }}/apmesh-tmr-prepared' in text, "artifact path differs from prepared root")
    require("if-no-files-found: error" in text, "missing artifact does not fail closed")
    require("overwrite: false" in text, "artifact overwrite is permitted")
    require("/usr/bin/python3 tools/run_topological_model_cumulative.py" in text, "workflow does not use the TMR runner")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
