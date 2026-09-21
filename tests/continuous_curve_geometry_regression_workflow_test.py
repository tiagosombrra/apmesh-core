#!/usr/bin/env python3
"""Static contract for the report-only CGR tooling workflow."""

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
        "name: Continuous Curve Geometry Regression Tooling" in text,
        "workflow identity differs",
    )
    require(
        re.search(
            r'(?m)^on:\n  push:\n    branches:\n      - "curve/continuous-geometry-regression-tooling"\n  workflow_dispatch:\s*$',
            text,
        )
        is not None,
        "workflow trigger differs",
    )
    for forbidden in ("pull_request:", "schedule:", "workflow_run:"):
        require(forbidden not in text, f"workflow exposes forbidden trigger: {forbidden}")

    require("permissions:\n  contents: read" in text, "workflow permissions differ")
    require("cancel-in-progress: true" in text, "tooling workflow concurrency differs")
    require("runs-on: ubuntu-24.04" in text, "tooling runner image differs")
    require("fetch-depth: 0" in text, "semantic baseline history is not available")

    for name in ("GCC 13 Debug", "Clang 18 libc++ Debug"):
        require(name in text, f"tooling matrix cell is absent: {name}")
    require(text.count("build/cgr-") == 2, "tooling build-directory matrix differs")

    for package in (
        "cmake=3.28.3-1build7",
        "ninja-build=1.11.1-2",
        "g++-13=13.3.0-6ubuntu2~24.04.1",
        "clang-18=1:18.1.3-1ubuntu1",
        "libc++-18-dev=1:18.1.3-1ubuntu1",
        "libc++abi-18-dev=1:18.1.3-1ubuntu1",
    ):
        require(package in text, f"admitted package is absent: {package}")

    require(
        "-DAPMESH_ENABLE_QUALIFICATION_TESTS=ON" in text,
        "qualification tests are not explicitly enabled",
    )
    require(
        "--target apmesh_core_continuous_curve_geometry_regression_export" in text,
        "CGR exporter target is not built explicitly",
    )

    expected_tests = (
        "apmesh_core.continuous_curve_geometry_regression_evidence",
        "apmesh_core.continuous_curve_geometry_regression_runner",
        "apmesh_core.continuous_curve_geometry_regression_workflow",
    )
    for test in expected_tests:
        require(test in text, f"focused tooling test is absent: {test}")

    for forbidden in (
        " prepare ",
        " execute ",
        "execution-claim",
        "upload-artifact",
        "download-artifact",
        "EXECUTE_ONCE",
    ):
        require(forbidden not in text, f"report-only workflow contains formal lifecycle material: {forbidden}")

    require("continue-on-error:" not in text, "workflow weakens a tooling failure")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
