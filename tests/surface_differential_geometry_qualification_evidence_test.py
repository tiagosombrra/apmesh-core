#!/usr/bin/env python3
"""Focused development test for Surface Differential Geometry report-only tooling."""

from __future__ import annotations

import argparse
import hashlib
import json
import pathlib
import shutil
import subprocess
import sys
import tempfile
from typing import Any


def run(command: list[str], expect_success: bool = True) -> subprocess.CompletedProcess[str]:
    result = subprocess.run(
        command,
        check=False,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    if expect_success and result.returncode != 0:
        raise RuntimeError(
            "command failed:\n"
            + " ".join(command)
            + "\nstdout:\n"
            + result.stdout
            + "\nstderr:\n"
            + result.stderr
        )
    if not expect_success and result.returncode == 0:
        raise RuntimeError("negative command unexpectedly succeeded: " + " ".join(command))
    return result


def sha256(path: pathlib.Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def write_json(path: pathlib.Path, value: Any) -> None:
    path.write_text(
        json.dumps(value, sort_keys=True, indent=2, ensure_ascii=False) + "\n",
        encoding="utf-8",
        newline="\n",
    )


def read_json(path: pathlib.Path) -> Any:
    return json.loads(path.read_text(encoding="utf-8"))


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--exporter", required=True)
    parser.add_argument("--tool", required=True)
    parser.add_argument("--profile", required=True)
    parser.add_argument("--source-root", required=True)
    args = parser.parse_args()

    exporter = pathlib.Path(args.exporter)
    tool = pathlib.Path(args.tool)
    profile = pathlib.Path(args.profile)
    source_root = pathlib.Path(args.source_root)

    run([sys.executable, "-m", "py_compile", str(tool)])

    base = [
        sys.executable,
        str(tool),
        "--profile",
        str(profile),
        "--source-root",
        str(source_root),
    ]
    run(base + ["validate-profile"])

    with tempfile.TemporaryDirectory(prefix="apmesh-sdg-tooling-") as temporary:
        root = pathlib.Path(temporary)
        certificate = root / "certificate.json"
        run([str(exporter), "certificate", str(certificate)])
        run(base + ["validate-certificate", "--certificate", str(certificate)])

        profile_value = read_json(profile)
        if len(profile_value["semantic_ctest_allowlist"]) != 42:
            raise RuntimeError("ordinary semantic allowlist is not 42 tests")
        if len(profile_value["frozen_semantic_git_blobs"]) != 32:
            raise RuntimeError("frozen semantic inventory is not 32 blobs")
        if len(profile_value["certificate_cases"]) != 35:
            raise RuntimeError("certificate inventory is not 35 cases")
        if len(profile_value["certificate_negative_cases"]) != 17:
            raise RuntimeError("negative inventory is not 17 cases")
        if len(profile_value["figure_specs"]) != 4:
            raise RuntimeError("figure inventory is not four figures")

        broken_profile = root / "broken-profile.json"
        broken = json.loads(json.dumps(profile_value))
        for item in broken["certificate_cases"]:
            if item["comparison_rule"] == "mixed":
                item["policy"] = None
                break
        write_json(broken_profile, broken)
        run(
            [
                sys.executable,
                str(tool),
                "--profile",
                str(broken_profile),
                "validate-profile",
            ],
            expect_success=False,
        )

        index_entries = []
        for cell in profile_value["cells"]:
            for repetition in range(1, profile_value["repetitions_per_cell"] + 1):
                target = root / f"{cell['id']}-r{repetition}.json"
                shutil.copyfile(certificate, target)
                index_entries.append(
                    {
                        "cell": cell["id"],
                        "repetition": repetition,
                        "path": target.name,
                        "sha256": sha256(target),
                    }
                )
        index_path = root / "index.json"
        write_json(
            index_path,
            {
                "schema_version": 1,
                "kind": "surface-differential-geometry-certificate-index",
                "entries": index_entries,
            },
        )
        comparison = root / "comparison.json"
        run(base + ["compare", "--index", str(index_path), "--output", str(comparison)])
        comparison_value = read_json(comparison)
        if comparison_value["certificate_count"] != 8:
            raise RuntimeError("comparison did not retain eight certificate slots")
        if comparison_value["same_cell_deterministic"] is not True:
            raise RuntimeError("same-cell projection is not deterministic")
        if comparison_value["cross_cell_equivalent_under_profile"] is not True:
            raise RuntimeError("cross-cell projection is not equivalent")
        if set(comparison_value["gates"].values()) != {"NOT_EXECUTED"}:
            raise RuntimeError("comparison advanced a formal gate")

        negatives = root / "negatives.json"
        run(
            base
            + [
                "negative-outcomes",
                "--certificate",
                str(certificate),
                "--output",
                str(negatives),
            ]
        )
        negative_value = read_json(negatives)
        expected_negative_ids = profile_value["certificate_negative_cases"]
        if [item["id"] for item in negative_value["outcomes"]] != expected_negative_ids:
            raise RuntimeError("negative outcome inventory differs")
        if any(item["result"] != "REJECTED" for item in negative_value["outcomes"]):
            raise RuntimeError("negative mutation was not rejected")

        figures_a = root / "figures-a"
        figures_b = root / "figures-b"
        manifest_a = figures_a / "manifest.json"
        manifest_b = figures_b / "manifest.json"
        run(
            base
            + [
                "figures",
                "--certificate",
                str(certificate),
                "--output-dir",
                str(figures_a),
                "--manifest",
                str(manifest_a),
            ]
        )
        run(
            base
            + [
                "figures",
                "--certificate",
                str(certificate),
                "--output-dir",
                str(figures_b),
                "--manifest",
                str(manifest_b),
            ]
        )
        first_manifest = read_json(manifest_a)
        second_manifest = read_json(manifest_b)
        if len(first_manifest["entries"]) != 4:
            raise RuntimeError("figure manifest does not contain four figures")
        first_hashes = {
            item["id"]: (item["source_sha256"], item["figure_sha256"])
            for item in first_manifest["entries"]
        }
        second_hashes = {
            item["id"]: (item["source_sha256"], item["figure_sha256"])
            for item in second_manifest["entries"]
        }
        if first_hashes != second_hashes:
            raise RuntimeError("figure generation is not deterministic")

        report_json = root / "report.json"
        report_md = root / "report.md"
        run(
            base
            + [
                "report",
                "--certificate",
                str(certificate),
                "--comparison",
                str(comparison),
                "--negatives",
                str(negatives),
                "--figures",
                str(manifest_a),
                "--output-json",
                str(report_json),
                "--output-md",
                str(report_md),
            ]
        )
        report = read_json(report_json)
        if report["status"] != "EVIDENCE_COLLECTED_PENDING_AUDIT":
            raise RuntimeError("report status differs")
        if set(report["gates"].values()) != {"NOT_EXECUTED"}:
            raise RuntimeError("report advanced a formal gate")
        if report["prepared_candidate"] or report["formal_execution"] or report["stage_qualified"]:
            raise RuntimeError("report advanced a formal lifecycle state")

        forged_comparison = root / "forged-comparison.json"
        forged = read_json(comparison)
        forged["gates"]["SDG0"] = "PASS"
        write_json(forged_comparison, forged)
        run(
            base
            + [
                "report",
                "--certificate",
                str(certificate),
                "--comparison",
                str(forged_comparison),
                "--negatives",
                str(negatives),
                "--figures",
                str(manifest_a),
                "--output-json",
                str(root / "forged-report.json"),
                "--output-md",
                str(root / "forged-report.md"),
            ],
            expect_success=False,
        )

        markdown = report_md.read_text(encoding="utf-8")
        if "No formal qualification gates were executed." not in markdown:
            raise RuntimeError("human-readable report lost its formal-state guard")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
