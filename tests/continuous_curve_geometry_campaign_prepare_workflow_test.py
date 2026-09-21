#!/usr/bin/env python3
"""Static contract for the preparation-only formal CGR workflow."""

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

    text = pathlib.Path(arguments.workflow).read_text(encoding="utf-8")

    require(
        "name: Continuous Curve Geometry Regression Preparation" in text,
        "workflow identity differs",
    )
    require(
        re.search(r"(?m)^on:\n  workflow_dispatch:\s*$", text) is not None,
        "workflow is not manual-only",
    )
    for trigger in ("push", "pull_request", "schedule", "workflow_run"):
        require(
            re.search(rf"(?m)^\s+{trigger}:\s*$", text) is None,
            f"workflow exposes {trigger} trigger",
        )
    require(
        'if [ "${GITHUB_REF}" != "refs/heads/main" ]; then' in text,
        "workflow is not restricted to main",
    )
    require("fetch-depth: 0" in text, "workflow does not fetch candidate ancestry")
    require(
        'OUTPUT_ROOT: ${{ runner.temp }}/apmesh-cgr-prepared' in text,
        "workflow output is not external runner temp",
    )
    require(
        'test ! -e "${OUTPUT_ROOT}"' in text,
        "workflow does not require a new output root",
    )
    require(
        re.search(r"(?m)^\s+prepare \\\s*$", text) is not None,
        "workflow does not invoke prepare",
    )
    require(
        re.search(r"(?m)^\s+validate-prepared \\\s*$", text) is not None,
        "workflow does not validate PREPARED state",
    )
    require(
        re.search(r"(?m)^\s+execute(?:\s|\\|$)", text) is None,
        "workflow exposes formal execution",
    )
    for absent in (
        "execution-claim.json",
        "terminal-manifest.json",
        "command-records.json",
        "certificate-index.json",
        "failure.json",
    ):
        require(
            f'test ! -e "${{OUTPUT_ROOT}}/{absent}"' in text,
            f"workflow does not reject PREPARED-forbidden artifact: {absent}",
        )
    require(
        "actions/checkout@3d3c42e5aac5ba805825da76410c181273ba90b1" in text,
        "checkout action is not pinned",
    )
    require(
        "actions/upload-artifact@ea165f8d65b6e75b540449e92b4886f43607fa02" in text,
        "artifact action is not pinned",
    )
    require(
        'name: cgr-prepared-${{ github.sha }}' in text,
        "prepared artifact name differs",
    )
    require(
        'path: ${{ runner.temp }}/apmesh-cgr-prepared' in text,
        "artifact path differs",
    )
    require("if-no-files-found: error" in text, "missing artifact does not fail closed")
    require("overwrite: false" in text, "artifact overwrite is permitted")
    require(
        "/usr/bin/python3 tools/run_continuous_curve_geometry_campaign.py" in text,
        "workflow does not use the formal CGR runner",
    )
    require("--negative tools/continuous_curve_geometry_regression_negative.py" in text, "negative evidence input is not sealed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
