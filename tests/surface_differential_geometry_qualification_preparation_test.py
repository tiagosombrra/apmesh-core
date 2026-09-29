#!/usr/bin/env python3
"""Focused contract for fail-closed Surface Differential Geometry preparation primitives."""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import pathlib
import shutil
import subprocess
import sys
import tempfile
from typing import Any, Callable


REMOTE_REPOSITORY = "https://github.com/tiagosombrra/apmesh-core.git"
STATUS = "SYNTHETIC_PREPARATION_VALIDATED"


def run(
    command: list[str],
    expect_success: bool = True,
) -> subprocess.CompletedProcess[str]:
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
        raise RuntimeError(
            "negative command unexpectedly succeeded: " + " ".join(command)
        )
    return result


def canonical_json(value: Any) -> bytes:
    return (
        json.dumps(
            value,
            sort_keys=True,
            separators=(",", ":"),
            ensure_ascii=False,
        )
        + "\n"
    ).encode("utf-8")


def sha256_file(path: pathlib.Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def git_blob_sha1(path: pathlib.Path) -> str:
    content = path.read_bytes()
    header = f"blob {len(content)}\0".encode("ascii")
    return hashlib.sha1(header + content).hexdigest()


def write_json(path: pathlib.Path, value: Any) -> None:
    path.write_text(
        json.dumps(value, sort_keys=True, indent=2, ensure_ascii=False) + "\n",
        encoding="utf-8",
        newline="\n",
    )


def read_json(path: pathlib.Path) -> Any:
    return json.loads(path.read_text(encoding="utf-8"))


def fixture_value(
    profile: dict[str, Any],
    protocol: pathlib.Path,
    profile_path: pathlib.Path,
    exporter: pathlib.Path,
    validator: pathlib.Path,
) -> dict[str, Any]:
    inventory = [
        {
            "path": "CMakeLists.txt",
            "git_blob": "1" * 40,
            "sha256": "2" * 64,
        },
        {
            "path": "docs/decisions/SURFACE_DIFFERENTIAL_GEOMETRY_QUALIFICATION_PROTOCOL.md",
            "git_blob": "3" * 40,
            "sha256": "4" * 64,
        },
        {
            "path": "src/geometry/surface_differential.cpp",
            "git_blob": "5" * 40,
            "sha256": "6" * 64,
        },
    ]
    return {
        "schema_version": 1,
        "kind": "surface-differential-geometry-preparation-fixture",
        "synthetic_fixture": True,
        "candidate": {
            "commit": "a" * 40,
            "upstream_commit": "a" * 40,
            "tree_clean": True,
            "published": True,
            "divergent": False,
            "moving": False,
            "remote_repository": REMOTE_REPOSITORY,
        },
        "inputs": {
            "profile": {
                "git_blob": git_blob_sha1(profile_path),
                "sha256": sha256_file(profile_path),
            },
            "protocol": {
                "git_blob": git_blob_sha1(protocol),
                "sha256": sha256_file(protocol),
            },
            "exporter": {
                "git_blob": git_blob_sha1(exporter),
                "sha256": sha256_file(exporter),
            },
            "validator": {
                "git_blob": git_blob_sha1(validator),
                "sha256": sha256_file(validator),
            },
        },
        "frozen_semantic_git_blobs": copy.deepcopy(
            profile["frozen_semantic_git_blobs"]
        ),
        "semantic_ctest_allowlist": list(profile["semantic_ctest_allowlist"]),
        "tracked_source_inventory": inventory,
        "tracked_source_count": len(inventory),
        "tracked_source_inventory_sha256": hashlib.sha256(
            canonical_json(inventory)
        ).hexdigest(),
        "environment": {
            "runner_image": "ubuntu-24.04",
            "architecture": "x86_64",
            "cells": copy.deepcopy(profile["cells"]),
            "repetitions_per_cell": profile["repetitions_per_cell"],
        },
        "plan": {
            "core_command_records": 56,
            "ordinary_semantic_test_executions": 336,
            "gates": {gate: "NOT_EXECUTED" for gate in profile["gates"]},
            "execution_requested": False,
            "formal_execution_authorized": False,
            "qualification_result": None,
        },
        "lifecycle": {
            "state": STATUS,
            "consumed": False,
            "execution_claim_present": False,
            "real_prepared_package": False,
        },
        "capabilities": {
            "prepare_real_candidate": False,
            "dispatch_workflow": False,
            "authorize_execution": False,
            "execute_formal_campaign": False,
            "set_qualified_status": False,
        },
        "limitations": [
            "synthetic-only preparation infrastructure validation",
            "no real PREPARED package is created",
            "formal execution remains separately authorized",
        ],
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--tool", required=True)
    parser.add_argument("--profile", required=True)
    parser.add_argument("--protocol", required=True)
    parser.add_argument("--exporter", required=True)
    parser.add_argument("--validator", required=True)
    args = parser.parse_args()

    tool = pathlib.Path(args.tool)
    profile_path = pathlib.Path(args.profile)
    protocol = pathlib.Path(args.protocol)
    exporter = pathlib.Path(args.exporter)
    validator = pathlib.Path(args.validator)
    profile = read_json(profile_path)

    run([sys.executable, "-m", "py_compile", str(tool)])

    help_result = run([sys.executable, str(tool), "--help"])
    expected_commands = "{validate-fixture,simulate-fixture,validate-simulated}"
    if expected_commands not in help_result.stdout:
        raise RuntimeError("preparation CLI command surface differs")

    base = [
        sys.executable,
        str(tool),
        "--profile",
        str(profile_path),
        "--protocol",
        str(protocol),
        "--exporter",
        str(exporter),
        "--validator",
        str(validator),
    ]

    with tempfile.TemporaryDirectory(prefix="apmesh-sdg-preparation-") as temporary:
        root = pathlib.Path(temporary)
        fixture = fixture_value(profile, protocol, profile_path, exporter, validator)
        fixture_path = root / "fixture.json"
        write_json(fixture_path, fixture)

        run(base + ["validate-fixture", "--fixture", str(fixture_path)])

        output_a = root / "output-a"
        output_b = root / "output-b"
        run(
            base
            + [
                "simulate-fixture",
                "--fixture",
                str(fixture_path),
                "--output-root",
                str(output_a),
            ]
        )
        run(
            base
            + [
                "simulate-fixture",
                "--fixture",
                str(fixture_path),
                "--output-root",
                str(output_b),
            ]
        )
        run(
            base
            + [
                "validate-simulated",
                "--fixture",
                str(fixture_path),
                "--output-root",
                str(output_a),
            ]
        )

        expected_files = {
            "synthetic-preparation-manifest.json",
            "synthetic-state-history.jsonl",
        }
        if {path.name for path in output_a.iterdir()} != expected_files:
            raise RuntimeError("synthetic preparation output inventory differs")
        if {path.name for path in output_b.iterdir()} != expected_files:
            raise RuntimeError("second synthetic output inventory differs")

        for name in expected_files:
            if (output_a / name).read_bytes() != (output_b / name).read_bytes():
                raise RuntimeError("synthetic preparation output is not deterministic")

        manifest = read_json(output_a / "synthetic-preparation-manifest.json")
        if manifest["status"] != STATUS:
            raise RuntimeError("synthetic manifest status differs")
        if manifest["lifecycle"]["real_prepared_package"] is not False:
            raise RuntimeError("synthetic manifest claims a real PREPARED package")
        if manifest["plan"]["execution_requested"] is not False:
            raise RuntimeError("synthetic manifest requests execution")
        if manifest["plan"]["formal_execution_authorized"] is not False:
            raise RuntimeError("synthetic manifest authorizes execution")
        if manifest["plan"]["qualification_result"] is not None:
            raise RuntimeError("synthetic manifest claims a qualification result")
        if set(manifest["plan"]["gates"].values()) != {"NOT_EXECUTED"}:
            raise RuntimeError("synthetic manifest advances an SDG gate")

        def rejected(name: str, mutate: Callable[[dict[str, Any]], None]) -> None:
            candidate = copy.deepcopy(fixture)
            mutate(candidate)
            target = root / f"negative-{name}.json"
            write_json(target, candidate)
            run(
                base + ["validate-fixture", "--fixture", str(target)],
                expect_success=False,
            )

        rejected(
            "candidate-upstream",
            lambda value: value["candidate"].__setitem__(
                "upstream_commit", "b" * 40
            ),
        )
        rejected(
            "dirty",
            lambda value: value["candidate"].__setitem__("tree_clean", False),
        )
        rejected(
            "unpublished",
            lambda value: value["candidate"].__setitem__("published", False),
        )
        rejected(
            "divergent",
            lambda value: value["candidate"].__setitem__("divergent", True),
        )
        rejected(
            "moving",
            lambda value: value["candidate"].__setitem__("moving", True),
        )
        rejected(
            "remote",
            lambda value: value["candidate"].__setitem__(
                "remote_repository", "https://example.invalid/apmesh-core.git"
            ),
        )
        rejected(
            "input-identity",
            lambda value: value["inputs"]["profile"].__setitem__(
                "sha256", "0" * 64
            ),
        )

        altered_profile_path = root / "altered-profile.json"
        altered_profile = copy.deepcopy(profile)
        altered_profile["tooling_status"] = "ALTERED_SYNTHETIC_PROFILE"
        write_json(altered_profile_path, altered_profile)
        run(
            [
                sys.executable,
                str(tool),
                "--profile",
                str(altered_profile_path),
                "--protocol",
                str(protocol),
                "--exporter",
                str(exporter),
                "--validator",
                str(validator),
                "validate-fixture",
                "--fixture",
                str(fixture_path),
            ],
            expect_success=False,
        )

        frozen_path = next(iter(fixture["frozen_semantic_git_blobs"]))
        rejected(
            "frozen-blob",
            lambda value: value["frozen_semantic_git_blobs"].__setitem__(
                frozen_path, "f" * 40
            ),
        )
        rejected(
            "allowlist-missing",
            lambda value: value["semantic_ctest_allowlist"].pop(),
        )
        rejected(
            "allowlist-extra",
            lambda value: value["semantic_ctest_allowlist"].append(
                "apmesh_core.synthetic_extra"
            ),
        )
        rejected(
            "allowlist-duplicate",
            lambda value: value["semantic_ctest_allowlist"].append(
                value["semantic_ctest_allowlist"][0]
            ),
        )

        rejected(
            "inventory-count",
            lambda value: value.__setitem__(
                "tracked_source_count", value["tracked_source_count"] + 1
            ),
        )

        def duplicate_inventory(value: dict[str, Any]) -> None:
            value["tracked_source_inventory"][1]["path"] = value[
                "tracked_source_inventory"
            ][0]["path"]
            value["tracked_source_inventory_sha256"] = hashlib.sha256(
                canonical_json(value["tracked_source_inventory"])
            ).hexdigest()

        rejected("inventory-duplicate", duplicate_inventory)
        rejected(
            "inventory-digest",
            lambda value: value.__setitem__(
                "tracked_source_inventory_sha256", "0" * 64
            ),
        )

        rejected(
            "runner-image",
            lambda value: value["environment"].__setitem__(
                "runner_image", "ubuntu-22.04"
            ),
        )
        rejected(
            "architecture",
            lambda value: value["environment"].__setitem__(
                "architecture", "aarch64"
            ),
        )

        def wrong_compiler(value: dict[str, Any]) -> None:
            value["environment"]["cells"][0]["compiler"] = "GCC 12.0.0"

        rejected("compiler-cell", wrong_compiler)
        rejected(
            "repetitions",
            lambda value: value["environment"].__setitem__(
                "repetitions_per_cell", 1
            ),
        )
        rejected(
            "core-cardinality",
            lambda value: value["plan"].__setitem__("core_command_records", 55),
        )
        rejected(
            "test-cardinality",
            lambda value: value["plan"].__setitem__(
                "ordinary_semantic_test_executions", 335
            ),
        )

        def advance_gate(value: dict[str, Any]) -> None:
            value["plan"]["gates"]["SDG0"] = "PASS"

        rejected("gate-advanced", advance_gate)
        rejected(
            "execution-requested",
            lambda value: value["plan"].__setitem__("execution_requested", True),
        )
        rejected(
            "execution-authorized",
            lambda value: value["plan"].__setitem__(
                "formal_execution_authorized", True
            ),
        )
        rejected(
            "qualified",
            lambda value: value["plan"].__setitem__(
                "qualification_result", "QUALIFIED"
            ),
        )
        rejected(
            "consumed",
            lambda value: value["lifecycle"].__setitem__("consumed", True),
        )
        rejected(
            "execution-claim",
            lambda value: value["lifecycle"].__setitem__(
                "execution_claim_present", True
            ),
        )
        rejected(
            "real-prepared",
            lambda value: value["lifecycle"].__setitem__(
                "real_prepared_package", True
            ),
        )
        rejected(
            "prepared-state",
            lambda value: value["lifecycle"].__setitem__("state", "PREPARED"),
        )
        rejected(
            "dispatch-capability",
            lambda value: value["capabilities"].__setitem__(
                "dispatch_workflow", True
            ),
        )
        rejected(
            "authorize-capability",
            lambda value: value["capabilities"].__setitem__(
                "authorize_execution", True
            ),
        )
        rejected(
            "execute-capability",
            lambda value: value["capabilities"].__setitem__(
                "execute_formal_campaign", True
            ),
        )
        rejected(
            "qualify-capability",
            lambda value: value["capabilities"].__setitem__(
                "set_qualified_status", True
            ),
        )
        rejected(
            "prepare-capability",
            lambda value: value["capabilities"].__setitem__(
                "prepare_real_candidate", True
            ),
        )

        noncanonical = root / "noncanonical"
        shutil.copytree(output_a, noncanonical)
        noncanonical_manifest = read_json(
            noncanonical / "synthetic-preparation-manifest.json"
        )
        write_json(
            noncanonical / "synthetic-preparation-manifest.json",
            noncanonical_manifest,
        )
        run(
            base
            + [
                "validate-simulated",
                "--fixture",
                str(fixture_path),
                "--output-root",
                str(noncanonical),
            ],
            expect_success=False,
        )

        extra_artifact = root / "extra-artifact"
        shutil.copytree(output_a, extra_artifact)
        (extra_artifact / "execution-claim.json").write_text(
            "{}\n",
            encoding="utf-8",
        )
        run(
            base
            + [
                "validate-simulated",
                "--fixture",
                str(fixture_path),
                "--output-root",
                str(extra_artifact),
            ],
            expect_success=False,
        )

        extra_history = root / "extra-history"
        shutil.copytree(output_a, extra_history)
        with (extra_history / "synthetic-state-history.jsonl").open(
            "a",
            encoding="utf-8",
        ) as stream:
            stream.write("{}\n")
        run(
            base
            + [
                "validate-simulated",
                "--fixture",
                str(fixture_path),
                "--output-root",
                str(extra_history),
            ],
            expect_success=False,
        )

        existing_output = root / "existing-output"
        existing_output.mkdir()
        run(
            base
            + [
                "simulate-fixture",
                "--fixture",
                str(fixture_path),
                "--output-root",
                str(existing_output),
            ],
            expect_success=False,
        )

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
