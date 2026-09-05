#!/usr/bin/env python3
import argparse
import pathlib
import shutil
import subprocess
import sys
import tempfile


def run(command: list[str], expected_returncode: int) -> None:
    completed = subprocess.run(command, check=False, capture_output=True)
    if completed.returncode != expected_returncode:
        raise RuntimeError(
            f"unexpected exit {completed.returncode} for {' '.join(command)}: "
            f"{completed.stdout!r} {completed.stderr!r}"
        )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--tool", required=True)
    parser.add_argument("--expected", required=True)
    parser.add_argument("--profile", required=True)
    parser.add_argument("--data-dir", required=True)
    arguments = parser.parse_args()

    tool = pathlib.Path(arguments.tool)
    expected = pathlib.Path(arguments.expected)
    data_directory = pathlib.Path(arguments.data_dir)
    with tempfile.TemporaryDirectory() as temporary_directory:
        root = pathlib.Path(temporary_directory)
        actual = root / "actual.json"
        shutil.copyfile(expected, actual)

        run([sys.executable, str(tool), "validate", "--expected", str(expected), "--actual", str(actual)], 0)
        run([sys.executable, str(tool), "validate", "--expected", str(expected), "--actual", str(root / "missing.json")], 1)
        for name in ("malformed.json", "unsupported_schema_version.json", "changed_component.json"):
            run(
                [
                    sys.executable,
                    str(tool),
                    "validate",
                    "--expected",
                    str(expected),
                    "--actual",
                    str(data_directory / name),
                ],
                1,
            )

        run(
            [
                sys.executable,
                str(tool),
                "validate-manifest",
                "--manifest",
                str(data_directory / "manifest_expected.json"),
                "--expected-source-hash",
                "expected-source-hash",
            ],
            0,
        )
        run(
            [
                sys.executable,
                str(tool),
                "validate-manifest",
                "--manifest",
                str(data_directory / "manifest_changed_source_hash.json"),
                "--expected-source-hash",
                "expected-source-hash",
            ],
            1,
        )

        first_plan = root / "first-plan.json"
        second_plan = root / "second-plan.json"
        for output in (first_plan, second_plan):
            run(
                [
                    sys.executable,
                    str(tool),
                    "plan",
                    "--profile",
                    str(arguments.profile),
                    "--source-root",
                    str(tool.parents[1]),
                    "--output",
                    str(output),
                ],
                0,
            )
        if first_plan.read_bytes() != second_plan.read_bytes():
            raise RuntimeError("plan bytes are not deterministic")

        first_report = root / "first-report.md"
        second_report = root / "second-report.md"
        for report in (first_report, second_report):
            run(
                [
                    sys.executable,
                    str(tool),
                    "compare",
                    "--expected",
                    str(expected),
                    "--certificate",
                    str(actual),
                    "--certificate",
                    str(actual),
                    "--report",
                    str(report),
                ],
                0,
            )
        if first_report.read_bytes() != second_report.read_bytes():
            raise RuntimeError("report bytes are not deterministic")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
