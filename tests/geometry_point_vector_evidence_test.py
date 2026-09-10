#!/usr/bin/env python3

import argparse
import hashlib
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
        run([sys.executable, arguments.tool, "validate-certificate", "--profile", arguments.profile, "--certificate", str(certificate)], 0)
        run([sys.executable, arguments.tool, "validate-compile-commands", "--compile-commands", arguments.compile_commands], 0)
        profile = json.loads(pathlib.Path(arguments.profile).read_text(encoding="utf-8"))
        entries = []
        for configuration in profile["configurations"]:
            for repetition in range(1, profile["repetitions_per_configuration"] + 1):
                copied = root / f"{configuration['name']}-{repetition}.json"
                copied.write_bytes(certificate.read_bytes())
                entries.append({"cell": configuration["name"], "repetition": repetition,
                                "path": copied.name, "sha256": hashlib.sha256(copied.read_bytes()).hexdigest()})
        index = root / "certificate-index.json"
        index.write_text(json.dumps({"schema_version": 1, "kind": "geometry-point-vector-certificate-index", "entries": entries}), encoding="utf-8")
        run([sys.executable, arguments.tool, "compare", "--profile", arguments.profile, "--index", str(index), "--report", str(report)], 0)
        if "Qualification: EVIDENCE_COLLECTED_PENDING_AUDIT" not in report.read_text(encoding="utf-8"):
            raise RuntimeError("report confuses report-only evidence with scientific qualification")

        changed = json.loads(certificate.read_text(encoding="utf-8"))
        changed["cases"][0]["observed"] = {"outcome": "error", "error": "non_finite_input", "value": None}
        changed_certificate = root / "changed-certificate.json"
        changed_certificate.write_text(json.dumps(changed), encoding="utf-8")
        run([sys.executable, arguments.tool, "validate-certificate", "--profile", arguments.profile, "--certificate", str(changed_certificate)], 1)

        forged_proximity = json.loads(certificate.read_text(encoding="utf-8"))
        next(case for case in forged_proximity["cases"] if case["id"] == "normalize_three_four")["comparison"]["limit"] = "0x1p+0"
        forged_proximity_certificate = root / "forged-proximity-certificate.json"
        forged_proximity_certificate.write_text(json.dumps(forged_proximity), encoding="utf-8")
        run([sys.executable, arguments.tool, "validate-certificate", "--profile", arguments.profile,
             "--certificate", str(forged_proximity_certificate)], 1)

        commands = json.loads(pathlib.Path(arguments.compile_commands).read_text(encoding="utf-8"))
        for entry in commands:
            if str(entry.get("file", "")).replace("\\", "/").endswith("src/core/geometry.cpp"):
                entry["command"] = str(entry.get("command", "")) + " -ffast-math"
        unsafe = root / "unsafe-compile-commands.json"
        unsafe.write_text(json.dumps(commands), encoding="utf-8")
        run([sys.executable, arguments.tool, "validate-compile-commands", "--compile-commands", str(unsafe)], 1)
        negative = root / "negative-fixtures.json"
        run([sys.executable, arguments.tool, "negative-self-check", "--profile", arguments.profile,
             "--certificate", str(certificate), "--compile-commands", arguments.compile_commands,
             "--output", str(negative)], 0)
        if json.loads(negative.read_text(encoding="utf-8"))["outcomes"] != [
            {"id": "duplicate_case", "result": "REJECTED"}, {"id": "unsafe_compile_flag", "result": "REJECTED"},
            {"id": "forged_proximity", "result": "REJECTED"}
        ]:
            raise RuntimeError("negative fixture evidence differs")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
