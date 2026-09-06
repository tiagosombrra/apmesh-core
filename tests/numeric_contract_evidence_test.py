#!/usr/bin/env python3
import argparse
import json
import pathlib
import subprocess
import sys
import tempfile


def run(command: list[str], expected_returncode: int) -> None:
    completed = subprocess.run(command, check=False, capture_output=True, text=True)
    if completed.returncode != expected_returncode:
        raise RuntimeError(f"unexpected exit {completed.returncode}: {' '.join(command)}\n{completed.stderr}")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--exporter", required=True)
    parser.add_argument("--tool", required=True)
    parser.add_argument("--profile", required=True)
    parser.add_argument("--compile-commands", required=True)
    arguments = parser.parse_args()

    with tempfile.TemporaryDirectory() as temporary_directory:
        root = pathlib.Path(temporary_directory)
        certificate = root / "certificate.json"
        environment = root / "environment.json"
        report = root / "report.md"
        run([arguments.exporter, "certificate", str(certificate)], 0)
        run([arguments.exporter, "environment", str(environment)], 0)
        run([sys.executable, arguments.tool, "validate-profile", "--profile", arguments.profile], 0)
        run([sys.executable, arguments.tool, "validate-certificate", "--certificate", str(certificate)], 0)
        run([sys.executable, arguments.tool, "validate-environment", "--environment", str(environment), "--compile-commands", arguments.compile_commands], 0)
        run([sys.executable, arguments.tool, "compare", "--certificate", str(certificate), "--environment", str(environment), "--report", str(report)], 0)
        report_text = report.read_text(encoding="utf-8")
        if "Evidence comparison: PASS" not in report_text or "Qualification: PENDING SCIENTIFIC AUDIT" not in report_text:
            raise RuntimeError("comparison report confuses evidence with qualification")

        changed_certificate = json.loads(certificate.read_text(encoding="utf-8"))
        changed_certificate["classification"][0]["category"] = "normal"
        changed_path = root / "changed-certificate.json"
        changed_path.write_text(json.dumps(changed_certificate), encoding="utf-8")
        run([sys.executable, arguments.tool, "validate-certificate", "--certificate", str(changed_path)], 1)

        unsafe_commands = json.loads(pathlib.Path(arguments.compile_commands).read_text(encoding="utf-8"))
        for entry in unsafe_commands:
            if str(entry.get("file", "")).endswith("src/core/numeric.cpp"):
                entry["command"] = str(entry.get("command", "")) + " -ffast-math"
        unsafe_path = root / "unsafe-compile-commands.json"
        unsafe_path.write_text(json.dumps(unsafe_commands), encoding="utf-8")
        run([sys.executable, arguments.tool, "validate-environment", "--environment", str(environment), "--compile-commands", str(unsafe_path)], 1)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
