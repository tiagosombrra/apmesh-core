#!/usr/bin/env python3
"""Focused evidence contracts for Continuous Curve Geometry Regression."""

from __future__ import annotations

import argparse
import hashlib
import json
import pathlib
import subprocess
import sys
import tempfile


def run(command: list[str], expected: int = 0) -> subprocess.CompletedProcess[str]:
    completed = subprocess.run(
        command,
        capture_output=True,
        text=True,
        check=False,
    )
    if completed.returncode != expected:
        raise RuntimeError(
            f"unexpected exit {completed.returncode}: {' '.join(command)}\n"
            f"stdout:\n{completed.stdout}\nstderr:\n{completed.stderr}"
        )
    return completed


def digest_tree(root: pathlib.Path) -> dict[str, str]:
    return {
        path.relative_to(root).as_posix(): hashlib.sha256(path.read_bytes()).hexdigest()
        for path in sorted(root.rglob("*"))
        if path.is_file()
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--exporter", required=True)
    parser.add_argument("--tool", required=True)
    parser.add_argument("--negative", required=True)
    parser.add_argument("--profile", required=True)
    arguments = parser.parse_args()

    common = [sys.executable, arguments.tool, "--profile", arguments.profile]
    negative_common = [
        sys.executable,
        arguments.negative,
        "--validator",
        arguments.tool,
        "--profile",
        arguments.profile,
    ]

    with tempfile.TemporaryDirectory(prefix="apmesh-cgr-evidence-") as temporary:
        root = pathlib.Path(temporary)

        run([*common, "validate-profile"])

        entries = []
        certificates: list[pathlib.Path] = []
        profile = json.loads(pathlib.Path(arguments.profile).read_text(encoding="utf-8"))
        for cell in profile["cells"]:
            for repetition in range(1, profile["repetitions_per_cell"] + 1):
                certificate = root / f"{cell['id']}-{repetition}.json"
                run(
                    [
                        arguments.exporter,
                        "certificate",
                        str(certificate),
                        cell["id"],
                        str(repetition),
                    ]
                )
                run(
                    [
                        *common,
                        "validate-certificate",
                        "--certificate",
                        str(certificate),
                    ]
                )
                certificates.append(certificate)
                entries.append(
                    {
                        "cell": cell["id"],
                        "repetition": repetition,
                        "path": certificate.name,
                        "sha256": hashlib.sha256(certificate.read_bytes()).hexdigest(),
                    }
                )

        if len(certificates) != 8:
            raise RuntimeError("focused certificate count differs")

        index = root / "certificate-index.json"
        index.write_text(
            json.dumps(
                {
                    "schema_version": 1,
                    "kind": "continuous-curve-geometry-certificate-index",
                    "entries": entries,
                },
                indent=2,
                sort_keys=True,
            )
            + "\n",
            encoding="utf-8",
        )

        comparison = root / "comparison.json"
        run(
            [
                *common,
                "compare",
                "--index",
                str(index),
                "--output",
                str(comparison),
            ]
        )
        comparison_value = json.loads(comparison.read_text(encoding="utf-8"))
        if (
            comparison_value["status"] != "EVIDENCE_COLLECTED_PENDING_AUDIT"
            or set(comparison_value["gates"].values()) != {"NOT_EXECUTED"}
            or comparison_value["certificate_count"] != 8
            or comparison_value["same_cell_projection_equal"] is not True
            or comparison_value["cross_cell_categorical_equal"] is not True
            or comparison_value["numeric_relations_independently_validated"] is not True
        ):
            raise RuntimeError("report-only comparison claims or coverage differ")

        first = certificates[0]
        derived_a = root / "derived-a"
        derived_b = root / "derived-b"
        run(
            [
                *common,
                "derive",
                "--certificate",
                str(first),
                "--output-dir",
                str(derived_a),
            ]
        )
        run(
            [
                *common,
                "derive",
                "--certificate",
                str(first),
                "--output-dir",
                str(derived_b),
            ]
        )
        expected_derived = {
            "curve-value-reference.csv",
            "curve-value-reference.svg",
            "curve-differential-speed.csv",
            "curve-differential-speed.svg",
            "curve-arc-length-enclosure.csv",
            "curve-arc-length-enclosure.svg",
            "curve-inverse-bracket.csv",
            "curve-inverse-bracket.svg",
            "derived-evidence.json",
        }
        if set(digest_tree(derived_a)) != expected_derived:
            raise RuntimeError("derived evidence inventory differs")
        if digest_tree(derived_a) != digest_tree(derived_b):
            raise RuntimeError("derived evidence is not byte deterministic")

        negative = root / "negative-evidence.json"
        run(
            [
                *negative_common,
                "generate",
                "--certificate",
                str(first),
                "--output",
                str(negative),
            ]
        )
        run(
            [
                *negative_common,
                "validate",
                "--outcomes",
                str(negative),
            ]
        )
        negative_value = json.loads(negative.read_text(encoding="utf-8"))
        if [item["id"] for item in negative_value["outcomes"]] != profile["certificate_negative_cases"]:
            raise RuntimeError("negative evidence inventory differs")
        if any(item["result"] != "REJECTED" for item in negative_value["outcomes"]):
            raise RuntimeError("negative evidence contains acceptance")

        forged = json.loads(first.read_text(encoding="utf-8"))
        forged["scientific_projection"]["cases"]["regularity_regular"]["result"] = "degenerate"
        forged_path = root / "forged-categorical.json"
        forged_path.write_text(json.dumps(forged), encoding="utf-8")
        run(
            [
                *common,
                "validate-certificate",
                "--certificate",
                str(forged_path),
            ],
            expected=1,
        )

        forged = json.loads(first.read_text(encoding="utf-8"))
        forged["scientific_projection"]["cases"]["inverse_line"]["lower_parameter"] = 0.9
        forged["scientific_projection"]["cases"]["inverse_line"]["upper_parameter"] = 0.1
        forged_path = root / "forged-bracket.json"
        forged_path.write_text(json.dumps(forged), encoding="utf-8")
        run(
            [
                *common,
                "validate-certificate",
                "--certificate",
                str(forged_path),
            ],
            expected=1,
        )

        forged_profile = dict(profile)
        forged_profile["semantic_ctest_allowlist"] = list(profile["semantic_ctest_allowlist"][:-1])
        forged_profile_path = root / "forged-profile.json"
        forged_profile_path.write_text(json.dumps(forged_profile), encoding="utf-8")
        run(
            [
                sys.executable,
                arguments.tool,
                "--profile",
                str(forged_profile_path),
                "validate-profile",
            ],
            expected=1,
        )

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
