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
        report = root / "report.md"
        run([arguments.exporter, "certificate", str(certificate)], 0)
        run([sys.executable, arguments.tool, "validate-profile", "--profile", arguments.profile], 0)
        run([sys.executable, arguments.tool, "validate-certificate", "--certificate", str(certificate)], 0)
        run([sys.executable, arguments.tool, "validate-compile-commands", "--compile-commands", arguments.compile_commands], 0)
        run([sys.executable, arguments.tool, "compare", "--profile", arguments.profile, "--certificate", str(certificate), "--report", str(report)], 0)
        if "Qualification: EVIDENCE_COLLECTED_PENDING_AUDIT" not in report.read_text(encoding="utf-8"):
            raise RuntimeError("report confuses report-only evidence with scientific qualification")

        changed = json.loads(certificate.read_text(encoding="utf-8"))
        changed["cases"][0]["outcome"] = "error"
        changed["cases"][0]["error"] = "non_finite_input"
        changed["cases"][0]["value"] = None
        changed_certificate = root / "changed-certificate.json"
        changed_certificate.write_text(json.dumps(changed), encoding="utf-8")
        run([sys.executable, arguments.tool, "validate-certificate", "--certificate", str(changed_certificate)], 1)

        commands = json.loads(pathlib.Path(arguments.compile_commands).read_text(encoding="utf-8"))
        for entry in commands:
            if str(entry.get("file", "")).replace("\\", "/").endswith("src/core/geometry.cpp"):
                entry["command"] = str(entry.get("command", "")) + " -ffast-math"
        unsafe = root / "unsafe-compile-commands.json"
        unsafe.write_text(json.dumps(commands), encoding="utf-8")
        run([sys.executable, arguments.tool, "validate-compile-commands", "--compile-commands", str(unsafe)], 1)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
