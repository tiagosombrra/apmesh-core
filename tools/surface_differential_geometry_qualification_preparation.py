#!/usr/bin/env python3
"""Fail-closed synthetic preparation primitives for Surface Differential Geometry."""

from __future__ import annotations

import argparse
import hashlib
import json
import pathlib
import re
import sys
from typing import Any


PROFILE_KIND = "surface-differential-geometry-qualification-profile"
FIXTURE_KIND = "surface-differential-geometry-preparation-fixture"
MANIFEST_KIND = "surface-differential-geometry-synthetic-preparation-manifest"
LIFECYCLE_KIND = "surface-differential-geometry-synthetic-lifecycle"
STATUS = "SYNTHETIC_PREPARATION_VALIDATED"
REMOTE_REPOSITORY = "https://github.com/tiagosombrra/apmesh-core.git"
RUNNER_IMAGE = "ubuntu-24.04"
ARCHITECTURE = "x86_64"
EXPECTED_INPUT_GIT_BLOBS = {
    "profile": "982d1d13aaeebf2c7e36ae41a0b925db3a060470",
    "protocol": "793314048c2f480b56facac58a7edd3edc55f6ac",
    "exporter": "69b273d05646e8a43458739c5da966f37f70b899",
    "validator": "9606b82da134be2987112244a852e6a502e97fec",
}
EXPECTED_REPETITIONS = 2
EXPECTED_CORE_COMMAND_RECORDS = 56
EXPECTED_ORDINARY_TEST_EXECUTIONS = 336
EXPECTED_GATE_IDS = [f"SDG{index}" for index in range(8)]
FORBIDDEN_ARTIFACTS = {
    "prepared-manifest.json",
    "execution-claim.json",
    "terminal-manifest.json",
    "command-records.json",
    "authorization.json",
    "failure.json",
}
OUTPUT_FILES = {
    "synthetic-preparation-manifest.json",
    "synthetic-state-history.jsonl",
}

_HEX40 = re.compile(r"^[0-9a-f]{40}$")
_HEX64 = re.compile(r"^[0-9a-f]{64}$")


class PreparationError(RuntimeError):
    """Raised when the synthetic preparation contract is violated."""


def fail(message: str) -> PreparationError:
    return PreparationError(message)


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


def read_json(path: pathlib.Path, label: str) -> Any:
    try:
        return json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        raise fail(f"{label} JSON could not be read: {error}") from error


def sha256_bytes(value: bytes) -> str:
    return hashlib.sha256(value).hexdigest()


def sha256_file(path: pathlib.Path) -> str:
    digest = hashlib.sha256()
    try:
        with path.open("rb") as stream:
            for chunk in iter(lambda: stream.read(1024 * 1024), b""):
                digest.update(chunk)
    except OSError as error:
        raise fail(f"required input could not be read: {path}") from error
    return digest.hexdigest()


def git_blob_sha1(path: pathlib.Path) -> str:
    try:
        content = path.read_bytes()
    except OSError as error:
        raise fail(f"required input could not be read: {path}") from error
    header = f"blob {len(content)}\0".encode("ascii")
    return hashlib.sha1(header + content).hexdigest()


def require_exact_keys(value: dict[str, Any], expected: set[str], label: str) -> None:
    if set(value) != expected:
        raise fail(f"{label} keys differ")


def require_hex40(value: Any, label: str) -> str:
    if not isinstance(value, str) or _HEX40.fullmatch(value) is None:
        raise fail(f"{label} is not a lowercase 40-hex identity")
    return value


def require_hex64(value: Any, label: str) -> str:
    if not isinstance(value, str) or _HEX64.fullmatch(value) is None:
        raise fail(f"{label} is not a lowercase SHA-256")
    return value


def require_bool(value: Any, expected: bool, label: str) -> None:
    if value is not expected:
        raise fail(f"{label} differs")


def validate_profile(profile: dict[str, Any]) -> None:
    if profile.get("schema_version") != 1 or profile.get("kind") != PROFILE_KIND:
        raise fail("qualification profile identity differs")

    cells = profile.get("cells")
    if not isinstance(cells, list) or len(cells) != 4:
        raise fail("qualification profile cell matrix differs")

    repetitions = profile.get("repetitions_per_cell")
    if repetitions != EXPECTED_REPETITIONS:
        raise fail("qualification profile repetition count differs")

    gates = profile.get("gates")
    if gates != EXPECTED_GATE_IDS:
        raise fail("qualification profile gate inventory differs")

    allowlist = profile.get("semantic_ctest_allowlist")
    if (
        not isinstance(allowlist, list)
        or len(allowlist) != 42
        or len(allowlist) != len(set(allowlist))
        or any(not isinstance(name, str) or not name for name in allowlist)
    ):
        raise fail("qualification profile semantic allowlist differs")

    frozen = profile.get("frozen_semantic_git_blobs")
    if not isinstance(frozen, dict) or len(frozen) != 32:
        raise fail("qualification profile frozen semantic inventory differs")
    for path, blob in frozen.items():
        if not isinstance(path, str) or not path or pathlib.PurePosixPath(path).is_absolute():
            raise fail("qualification profile frozen path differs")
        require_hex40(blob, f"frozen blob {path}")


def input_identities(
    profile_path: pathlib.Path,
    protocol_path: pathlib.Path,
    exporter_path: pathlib.Path,
    validator_path: pathlib.Path,
) -> dict[str, dict[str, str]]:
    paths = {
        "profile": profile_path,
        "protocol": protocol_path,
        "exporter": exporter_path,
        "validator": validator_path,
    }
    result: dict[str, dict[str, str]] = {}
    for name, path in paths.items():
        if not path.is_file():
            raise fail(f"required preparation input is absent: {path}")
        observed_blob = git_blob_sha1(path)
        if observed_blob != EXPECTED_INPUT_GIT_BLOBS[name]:
            raise fail(f"frozen preparation input Git blob differs: {name}")
        result[name] = {
            "git_blob": observed_blob,
            "sha256": sha256_file(path),
        }
    return result


def validate_inventory(entries: Any, count: Any, digest: Any) -> None:
    if not isinstance(entries, list) or not entries:
        raise fail("tracked-source inventory is empty or malformed")
    if isinstance(count, bool) or not isinstance(count, int) or count != len(entries):
        raise fail("tracked-source inventory count differs")

    paths: list[str] = []
    for entry in entries:
        if not isinstance(entry, dict):
            raise fail("tracked-source inventory entry is not an object")
        require_exact_keys(entry, {"path", "git_blob", "sha256"}, "tracked-source entry")
        path = entry["path"]
        if (
            not isinstance(path, str)
            or not path
            or pathlib.PurePosixPath(path).is_absolute()
            or ".." in pathlib.PurePosixPath(path).parts
        ):
            raise fail("tracked-source inventory path differs")
        require_hex40(entry["git_blob"], f"tracked-source git blob {path}")
        require_hex64(entry["sha256"], f"tracked-source SHA-256 {path}")
        paths.append(path)

    if paths != sorted(paths) or len(paths) != len(set(paths)):
        raise fail("tracked-source inventory is not canonical and unique")

    require_hex64(digest, "tracked-source inventory digest")
    if digest != sha256_bytes(canonical_json(entries)):
        raise fail("tracked-source inventory digest differs")


def validate_fixture(
    fixture: dict[str, Any],
    profile: dict[str, Any],
    expected_inputs: dict[str, dict[str, str]],
) -> None:
    require_exact_keys(
        fixture,
        {
            "schema_version",
            "kind",
            "synthetic_fixture",
            "candidate",
            "inputs",
            "frozen_semantic_git_blobs",
            "semantic_ctest_allowlist",
            "tracked_source_inventory",
            "tracked_source_count",
            "tracked_source_inventory_sha256",
            "environment",
            "plan",
            "lifecycle",
            "capabilities",
            "limitations",
        },
        "fixture",
    )
    if fixture["schema_version"] != 1 or fixture["kind"] != FIXTURE_KIND:
        raise fail("fixture identity differs")
    require_bool(fixture["synthetic_fixture"], True, "synthetic fixture marker")

    candidate = fixture["candidate"]
    if not isinstance(candidate, dict):
        raise fail("candidate record is not an object")
    require_exact_keys(
        candidate,
        {
            "commit",
            "upstream_commit",
            "tree_clean",
            "published",
            "divergent",
            "moving",
            "remote_repository",
            "tracked_source_paths",
        },
        "candidate",
    )
    commit = require_hex40(candidate["commit"], "candidate commit")
    upstream = require_hex40(candidate["upstream_commit"], "candidate upstream")
    if commit != upstream:
        raise fail("candidate and upstream commit differ")
    require_bool(candidate["tree_clean"], True, "candidate clean state")
    require_bool(candidate["published"], True, "candidate published state")
    require_bool(candidate["divergent"], False, "candidate divergence state")
    require_bool(candidate["moving"], False, "candidate moving state")
    if candidate["remote_repository"] != REMOTE_REPOSITORY:
        raise fail("candidate remote repository identity differs")

    tracked_source_paths = candidate["tracked_source_paths"]
    if (
        not isinstance(tracked_source_paths, list)
        or not tracked_source_paths
        or tracked_source_paths != sorted(tracked_source_paths)
        or len(tracked_source_paths) != len(set(tracked_source_paths))
        or any(
            not isinstance(path, str)
            or not path
            or pathlib.PurePosixPath(path).is_absolute()
            or ".." in pathlib.PurePosixPath(path).parts
            for path in tracked_source_paths
        )
    ):
        raise fail("candidate tracked-source path inventory differs")

    inputs = fixture["inputs"]
    if not isinstance(inputs, dict) or inputs != expected_inputs:
        raise fail("profile/protocol/exporter/validator identity differs")

    if fixture["frozen_semantic_git_blobs"] != profile["frozen_semantic_git_blobs"]:
        raise fail("frozen semantic Git-blob identity differs")

    allowlist = fixture["semantic_ctest_allowlist"]
    if (
        allowlist != profile["semantic_ctest_allowlist"]
        or len(allowlist) != len(set(allowlist))
    ):
        raise fail("ordinary semantic CTest allowlist differs")

    validate_inventory(
        fixture["tracked_source_inventory"],
        fixture["tracked_source_count"],
        fixture["tracked_source_inventory_sha256"],
    )
    observed_inventory_paths = [
        entry["path"] for entry in fixture["tracked_source_inventory"]
    ]
    if observed_inventory_paths != tracked_source_paths:
        raise fail("tracked-source inventory is incomplete or contains extra paths")

    environment = fixture["environment"]
    if not isinstance(environment, dict):
        raise fail("environment record is not an object")
    require_exact_keys(
        environment,
        {"runner_image", "architecture", "cells", "repetitions_per_cell"},
        "environment",
    )
    if environment["runner_image"] != RUNNER_IMAGE:
        raise fail("runner image differs")
    if environment["architecture"] != ARCHITECTURE:
        raise fail("runner architecture differs")
    if environment["cells"] != profile["cells"]:
        raise fail("compiler/library/build cell matrix differs")
    if environment["repetitions_per_cell"] != EXPECTED_REPETITIONS:
        raise fail("environment repetition count differs")

    plan = fixture["plan"]
    if not isinstance(plan, dict):
        raise fail("plan record is not an object")
    require_exact_keys(
        plan,
        {
            "core_command_records",
            "ordinary_semantic_test_executions",
            "gates",
            "execution_requested",
            "formal_execution_authorized",
            "qualification_result",
        },
        "plan",
    )
    if plan["core_command_records"] != EXPECTED_CORE_COMMAND_RECORDS:
        raise fail("core command cardinality differs")
    if plan["ordinary_semantic_test_executions"] != EXPECTED_ORDINARY_TEST_EXECUTIONS:
        raise fail("ordinary semantic execution cardinality differs")
    expected_gates = {gate: "NOT_EXECUTED" for gate in EXPECTED_GATE_IDS}
    if plan["gates"] != expected_gates:
        raise fail("SDG gate state differs")
    require_bool(plan["execution_requested"], False, "execution requested state")
    require_bool(
        plan["formal_execution_authorized"],
        False,
        "formal execution authorization state",
    )
    if plan["qualification_result"] is not None:
        raise fail("qualification result must be absent")

    lifecycle = fixture["lifecycle"]
    if not isinstance(lifecycle, dict):
        raise fail("lifecycle record is not an object")
    require_exact_keys(
        lifecycle,
        {
            "state",
            "consumed",
            "execution_claim_present",
            "real_prepared_package",
        },
        "lifecycle",
    )
    if lifecycle["state"] != STATUS:
        raise fail("synthetic lifecycle state differs")
    require_bool(lifecycle["consumed"], False, "consumed state")
    require_bool(
        lifecycle["execution_claim_present"],
        False,
        "execution claim presence",
    )
    require_bool(
        lifecycle["real_prepared_package"],
        False,
        "real PREPARED-package marker",
    )

    capabilities = fixture["capabilities"]
    if not isinstance(capabilities, dict):
        raise fail("capability record is not an object")
    require_exact_keys(
        capabilities,
        {
            "prepare_real_candidate",
            "dispatch_workflow",
            "authorize_execution",
            "execute_formal_campaign",
            "set_qualified_status",
        },
        "capabilities",
    )
    for name, value in capabilities.items():
        require_bool(value, False, f"capability {name}")

    limitations = fixture["limitations"]
    if (
        not isinstance(limitations, list)
        or not limitations
        or any(not isinstance(item, str) or not item for item in limitations)
        or len(limitations) != len(set(limitations))
    ):
        raise fail("fixture limitations differ")


def build_manifest(fixture: dict[str, Any]) -> dict[str, Any]:
    return {
        "schema_version": 1,
        "kind": MANIFEST_KIND,
        "status": STATUS,
        "synthetic_fixture": True,
        "candidate": fixture["candidate"],
        "inputs": fixture["inputs"],
        "frozen_semantic_git_blobs": fixture["frozen_semantic_git_blobs"],
        "semantic_ctest_allowlist": fixture["semantic_ctest_allowlist"],
        "tracked_source_inventory": fixture["tracked_source_inventory"],
        "tracked_source_count": fixture["tracked_source_count"],
        "tracked_source_inventory_sha256": fixture[
            "tracked_source_inventory_sha256"
        ],
        "environment": fixture["environment"],
        "plan": fixture["plan"],
        "lifecycle": fixture["lifecycle"],
        "capabilities": fixture["capabilities"],
        "limitations": fixture["limitations"],
        "source_fixture_sha256": sha256_bytes(canonical_json(fixture)),
    }


def lifecycle_record() -> dict[str, Any]:
    return {
        "schema_version": 1,
        "kind": LIFECYCLE_KIND,
        "state": STATUS,
        "formal_preparation": False,
        "formal_execution_authorized": False,
        "real_prepared_package": False,
    }


def validate_manifest(
    manifest: dict[str, Any],
    fixture: dict[str, Any],
) -> None:
    expected = build_manifest(fixture)
    if manifest != expected:
        raise fail("synthetic preparation manifest differs")
    if manifest["status"] != STATUS:
        raise fail("synthetic manifest status differs")
    if manifest["lifecycle"]["real_prepared_package"] is not False:
        raise fail("synthetic manifest claims a real PREPARED package")
    if manifest["plan"]["execution_requested"] is not False:
        raise fail("synthetic manifest requests execution")
    if manifest["plan"]["formal_execution_authorized"] is not False:
        raise fail("synthetic manifest authorizes execution")
    if manifest["plan"]["qualification_result"] is not None:
        raise fail("synthetic manifest claims a qualification result")


def simulate_fixture(
    fixture: dict[str, Any],
    output_root: pathlib.Path,
) -> None:
    if output_root.exists():
        raise fail("synthetic output root already exists")
    output_root.mkdir(parents=True)

    manifest = build_manifest(fixture)
    (output_root / "synthetic-preparation-manifest.json").write_bytes(
        canonical_json(manifest)
    )
    (output_root / "synthetic-state-history.jsonl").write_bytes(
        canonical_json(lifecycle_record())
    )


def validate_simulated(output_root: pathlib.Path, fixture: dict[str, Any]) -> None:
    if not output_root.is_dir():
        raise fail("synthetic output root is absent")

    entries = list(output_root.iterdir())
    names = {path.name for path in entries}
    if names != OUTPUT_FILES or any(not path.is_file() for path in entries):
        raise fail("synthetic output inventory differs")
    if names & FORBIDDEN_ARTIFACTS:
        raise fail("formal execution/preparation artifact appeared")

    manifest_path = output_root / "synthetic-preparation-manifest.json"
    state_path = output_root / "synthetic-state-history.jsonl"

    manifest_bytes = manifest_path.read_bytes()
    manifest = read_json(manifest_path, "synthetic manifest")
    if manifest_bytes != canonical_json(manifest):
        raise fail("synthetic manifest is not canonical")
    validate_manifest(manifest, fixture)

    state_bytes = state_path.read_bytes()
    expected_state = canonical_json(lifecycle_record())
    if state_bytes != expected_state:
        raise fail("synthetic state history differs")


def load_context(
    arguments: argparse.Namespace,
) -> tuple[dict[str, Any], dict[str, dict[str, str]]]:
    profile_path = pathlib.Path(arguments.profile)
    protocol_path = pathlib.Path(arguments.protocol)
    exporter_path = pathlib.Path(arguments.exporter)
    validator_path = pathlib.Path(arguments.validator)

    profile = read_json(profile_path, "qualification profile")
    if not isinstance(profile, dict):
        raise fail("qualification profile root must be an object")
    validate_profile(profile)
    identities = input_identities(
        profile_path,
        protocol_path,
        exporter_path,
        validator_path,
    )
    return profile, identities


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description=(
            "Validate synthetic fail-closed Surface Differential Geometry "
            "preparation fixtures without creating a real PREPARED package."
        )
    )
    parser.add_argument("--profile", required=True)
    parser.add_argument("--protocol", required=True)
    parser.add_argument("--exporter", required=True)
    parser.add_argument("--validator", required=True)

    subparsers = parser.add_subparsers(dest="command", required=True)

    validate_fixture_parser = subparsers.add_parser("validate-fixture")
    validate_fixture_parser.add_argument("--fixture", required=True)

    simulate_parser = subparsers.add_parser("simulate-fixture")
    simulate_parser.add_argument("--fixture", required=True)
    simulate_parser.add_argument("--output-root", required=True)

    validate_simulated_parser = subparsers.add_parser("validate-simulated")
    validate_simulated_parser.add_argument("--fixture", required=True)
    validate_simulated_parser.add_argument("--output-root", required=True)

    return parser


def main() -> int:
    parser = build_parser()
    arguments = parser.parse_args()

    try:
        profile, identities = load_context(arguments)
        fixture = read_json(pathlib.Path(arguments.fixture), "preparation fixture")
        if not isinstance(fixture, dict):
            raise fail("preparation fixture root must be an object")
        validate_fixture(fixture, profile, identities)

        if arguments.command == "validate-fixture":
            return 0

        output_root = pathlib.Path(arguments.output_root)
        if arguments.command == "simulate-fixture":
            simulate_fixture(fixture, output_root)
            validate_simulated(output_root, fixture)
            return 0

        if arguments.command == "validate-simulated":
            validate_simulated(output_root, fixture)
            return 0

        raise fail("unknown command")
    except (PreparationError, OSError, ValueError) as error:
        print(f"BLOCKED: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
