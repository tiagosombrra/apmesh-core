#!/usr/bin/env python3
"""Focused report-only contracts for Topological Model cumulative evidence."""

from __future__ import annotations

import argparse
import hashlib
import json
import pathlib
import subprocess
import sys
import tempfile


def run(command: list[str], expected: int = 0) -> None:
    completed = subprocess.run(command, capture_output=True, text=True, check=False)
    if completed.returncode != expected:
        raise RuntimeError(
            f"unexpected exit {completed.returncode}: {' '.join(command)}\n"
            f"{completed.stdout}\n{completed.stderr}"
        )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--exporter", required=True)
    parser.add_argument("--tool", required=True)
    parser.add_argument("--profile", required=True)
    arguments = parser.parse_args()

    common = [sys.executable, arguments.tool, "--profile", arguments.profile]
    with tempfile.TemporaryDirectory(prefix="apmesh-core-tmr-evidence-") as temporary:
        root = pathlib.Path(temporary)
        certificate = root / "certificate.json"

        run([arguments.exporter, "certificate", str(certificate)])
        run([*common, "validate-profile"])
        run([*common, "validate-certificate", "--certificate", str(certificate)])

        profile = json.loads(pathlib.Path(arguments.profile).read_text(encoding="utf-8"))
        forged_profile = dict(profile)
        forged_profile["limitations"] = list(profile["limitations"])
        forged_profile["limitations"][0] = "Cloud execution silently replaces the pre-registered WSL scope"
        forged_profile_path = root / "forged-profile.json"
        forged_profile_path.write_text(json.dumps(forged_profile), encoding="utf-8")
        run(
            [sys.executable, arguments.tool, "--profile", str(forged_profile_path), "validate-profile"],
            expected=1,
        )

        entries = []
        for cell in profile["cells"]:
            for repetition in range(1, profile["repetitions_per_cell"] + 1):
                copied = root / f"{cell['id']}-{repetition}.json"
                copied.write_bytes(certificate.read_bytes())
                entries.append({
                    "cell": cell["id"],
                    "repetition": repetition,
                    "path": copied.name,
                    "sha256": hashlib.sha256(copied.read_bytes()).hexdigest(),
                })

        index = root / "certificate-index.json"
        index.write_text(json.dumps({
            "schema_version": 1,
            "kind": "topological-model-cumulative-certificate-index",
            "entries": entries,
        }), encoding="utf-8")

        comparison = root / "comparison.json"
        run([*common, "compare", "--index", str(index), "--output", str(comparison)])
        observed = json.loads(comparison.read_text(encoding="utf-8"))
        if (
            observed["status"] != "EVIDENCE_COLLECTED_PENDING_AUDIT"
            or set(observed["gates"].values()) != {"NOT_EXECUTED"}
            or observed["certificate_count"] != 8
        ):
            raise RuntimeError("report-only comparison claims a scientific gate result")

        collected = root / "collected.json"
        run([*common, "collect", "--index", str(index), "--output", str(collected)])
        if json.loads(collected.read_text(encoding="utf-8"))["status"] != "EVIDENCE_COLLECTED_PENDING_AUDIT":
            raise RuntimeError("collector claims scientific closure")

        outcomes = root / "negative-outcomes.json"
        run([*common, "negative-outcomes", "--output", str(outcomes)])
        run([*common, "validate-negative-outcomes", "--outcomes", str(outcomes)])
        forged_outcomes = json.loads(outcomes.read_text(encoding="utf-8"))
        forged_outcomes["outcomes"][0]["result"] = "ACCEPTED"
        forged_outcomes_path = root / "forged-negative-outcomes.json"
        forged_outcomes_path.write_text(json.dumps(forged_outcomes), encoding="utf-8")
        run([*common, "validate-negative-outcomes", "--outcomes", str(forged_outcomes_path)], expected=1)

        forged = json.loads(certificate.read_text(encoding="utf-8"))
        snapshot_case = next(item for item in forged["cases"] if item["id"] == "canonical_snapshot")
        snapshot_case["observed"]["snapshot"] = "forged\n"
        forged_path = root / "forged-snapshot.json"
        forged_path.write_text(json.dumps(forged), encoding="utf-8")
        run([*common, "validate-certificate", "--certificate", str(forged_path)], expected=1)

        forged = json.loads(certificate.read_text(encoding="utf-8"))
        incidence_case = next(item for item in forged["cases"] if item["id"] == "incidence_bijection")
        incidence_case["observed"]["exact_bijection"] = False
        forged_path = root / "forged-incidence.json"
        forged_path.write_text(json.dumps(forged), encoding="utf-8")
        run([*common, "validate-certificate", "--certificate", str(forged_path)], expected=1)

        forged = json.loads(certificate.read_text(encoding="utf-8"))
        forged["cases"].append(forged["cases"][0])
        forged_path = root / "duplicate-case.json"
        forged_path.write_text(json.dumps(forged), encoding="utf-8")
        run([*common, "validate-certificate", "--certificate", str(forged_path)], expected=1)

        forged = json.loads(certificate.read_text(encoding="utf-8"))
        forged["non_claims"] = list(forged["non_claims"]) + ["qualified_manifoldness"]
        forged_path = root / "forged-non-claim.json"
        forged_path.write_text(json.dumps(forged), encoding="utf-8")
        run([*common, "validate-certificate", "--certificate", str(forged_path)], expected=1)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
