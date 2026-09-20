#!/usr/bin/env python3
"""Validate repository-resident one-shot Topological Model TMR authorization."""

from __future__ import annotations

import argparse
import json
import pathlib
import re
from typing import Any


SCHEMA_KEYS = {
    "schema_version",
    "kind",
    "authorization",
    "candidate",
    "preparation_run_id",
    "prepared_artifact_id",
    "prepared_artifact_sha256",
    "prepared_manifest_sha256",
    "preparation_seal_sha256",
    "preparation_audit",
    "execution_workflow",
    "terminal_audit_required",
}

KIND = "topological-model-tmr-execution-authorization"
AUTHORIZATION = "EXECUTE_ONCE"
EXECUTION_WORKFLOW = ".github/workflows/topological-model-tmr-execute.yml"
AUDIT_STATUS = "PASS / PREPARED / NOT EXECUTED"

_SHA256 = re.compile(r"^[0-9a-f]{64}$")
_COMMIT = re.compile(r"^[0-9a-f]{40}$")
_AUDIT_PATH = re.compile(
    r"^docs/audits/[A-Za-z0-9._-]+topological-model-tmr[A-Za-z0-9._-]*preparation-audit\.(?:json|md)$"
)


class AuthorizationError(RuntimeError):
    pass


def fail(message: str) -> AuthorizationError:
    return AuthorizationError(message)


def read_object(path: pathlib.Path, label: str) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        raise fail(f"{label} JSON could not be read: {error}") from error
    if not isinstance(value, dict):
        raise fail(f"{label} root must be a JSON object")
    return value


def read_json(path: pathlib.Path) -> dict[str, Any]:
    return read_object(path, "authorization")


def require_sha256(value: Any, field: str) -> str:
    if not isinstance(value, str) or _SHA256.fullmatch(value) is None:
        raise fail(f"authorization field is not a lowercase SHA-256: {field}")
    return value


def require_positive_int(value: Any, field: str) -> int:
    if isinstance(value, bool) or not isinstance(value, int) or value <= 0:
        raise fail(f"authorization field is not a positive integer: {field}")
    return value


def validate_machine_readable_audit(
    audit: pathlib.Path,
    value: dict[str, Any],
) -> None:
    observed = read_object(audit, "preparation audit")
    preparation = observed.get("preparation")
    execution_absence = observed.get("execution_absence")
    if not isinstance(preparation, dict) or not isinstance(execution_absence, dict):
        raise fail("preparation audit structure differs")

    expected = {
        "decision": "PASS",
        "lifecycle_state": "PREPARED",
        "execution_requested": False,
        "candidate_commit": value["candidate"],
        "formal_execution_authorized": False,
    }
    for key, item in expected.items():
        if observed.get(key) != item:
            raise fail(f"preparation audit field differs: {key}")

    preparation_expected = {
        "run_id": value["preparation_run_id"],
        "artifact_id": value["prepared_artifact_id"],
        "artifact_sha256": value["prepared_artifact_sha256"],
        "prepared_manifest_sha256": value["prepared_manifest_sha256"],
        "preparation_seal_sha256": value["preparation_seal_sha256"],
    }
    for key, item in preparation_expected.items():
        if preparation.get(key) != item:
            raise fail(f"preparation audit binding differs: preparation.{key}")

    absence_expected = {
        "execution_claim": False,
        "terminal_manifest": False,
        "command_records": False,
        "certificate_index": False,
        "failure_record": False,
        "all_gates": "NOT_EXECUTED",
    }
    for key, item in absence_expected.items():
        if execution_absence.get(key) != item:
            raise fail(f"preparation audit execution absence differs: {key}")


def validate_legacy_markdown_audit(
    audit: pathlib.Path,
    value: dict[str, Any],
) -> None:
    content = audit.read_text(encoding="utf-8")
    required = (
        AUDIT_STATUS,
        value["candidate"],
        str(value["preparation_run_id"]),
        str(value["prepared_artifact_id"]),
        value["prepared_artifact_sha256"],
        value["prepared_manifest_sha256"],
        value["preparation_seal_sha256"],
    )
    if any(token not in content for token in required):
        raise fail("legacy preparation audit does not bind the exact authorized PREPARED identity")


def validate_audit(root: pathlib.Path, value: dict[str, Any]) -> None:
    relative = value["preparation_audit"]
    if not isinstance(relative, str) or _AUDIT_PATH.fullmatch(relative) is None:
        raise fail("preparation audit path differs from the admitted audit namespace")

    audit = (root / relative).resolve()
    audit_root = (root / "docs" / "audits").resolve()
    if audit.parent != audit_root or not audit.is_file():
        raise fail("preparation audit authority is absent or outside docs/audits")

    if audit.suffix == ".json":
        validate_machine_readable_audit(audit, value)
    else:
        validate_legacy_markdown_audit(audit, value)


def validate_authorization(
    authorization: pathlib.Path,
    source_root: pathlib.Path,
) -> dict[str, Any]:
    root = source_root.resolve()
    path = authorization.resolve()
    expected_directory = (root / "experiments" / "authorizations").resolve()

    if path.parent != expected_directory:
        raise fail("authorization must live directly in experiments/authorizations")

    value = read_json(path)
    if set(value) != SCHEMA_KEYS:
        raise fail("authorization schema differs")

    if value["schema_version"] != 1:
        raise fail("authorization schema version differs")
    if value["kind"] != KIND:
        raise fail("authorization kind differs")
    if value["authorization"] != AUTHORIZATION:
        raise fail("authorization decision differs")
    if not isinstance(value["candidate"], str) or _COMMIT.fullmatch(value["candidate"]) is None:
        raise fail("candidate is not a lowercase 40-hex commit")
    require_positive_int(value["preparation_run_id"], "preparation_run_id")
    require_positive_int(value["prepared_artifact_id"], "prepared_artifact_id")
    require_sha256(value["prepared_artifact_sha256"], "prepared_artifact_sha256")
    manifest = require_sha256(value["prepared_manifest_sha256"], "prepared_manifest_sha256")
    require_sha256(value["preparation_seal_sha256"], "preparation_seal_sha256")
    if value["execution_workflow"] != EXECUTION_WORKFLOW:
        raise fail("execution workflow differs")
    if value["terminal_audit_required"] is not True:
        raise fail("terminal audit requirement differs")

    expected_name = f"topological-model-tmr-{manifest}.json"
    if path.name != expected_name:
        raise fail("authorization filename does not match the prepared-manifest identity")

    validate_audit(root, value)
    return value


def write_github_output(
    output: pathlib.Path,
    authorization: pathlib.Path,
    source_root: pathlib.Path,
    value: dict[str, Any],
) -> None:
    root = source_root.resolve()
    relative = authorization.resolve().relative_to(root).as_posix()
    manifest = value["prepared_manifest_sha256"]
    rows = {
        "authorization_file": relative,
        "candidate": value["candidate"],
        "preparation_run_id": str(value["preparation_run_id"]),
        "prepared_artifact_id": str(value["prepared_artifact_id"]),
        "prepared_artifact_sha256": value["prepared_artifact_sha256"],
        "prepared_manifest_sha256": manifest,
        "preparation_seal_sha256": value["preparation_seal_sha256"],
        "claim_tag": f"tmr-execution-claim-{manifest}",
    }
    with output.open("a", encoding="utf-8", newline="\n") as stream:
        for key, item in rows.items():
            stream.write(f"{key}={item}\n")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source-root", default=".")
    commands = parser.add_subparsers(dest="command", required=True)
    validate = commands.add_parser("validate")
    validate.add_argument("--authorization", required=True)
    validate.add_argument("--github-output")
    arguments = parser.parse_args()

    root = pathlib.Path(arguments.source_root)
    authorization = pathlib.Path(arguments.authorization)
    value = validate_authorization(authorization, root)

    if arguments.github_output:
        write_github_output(
            pathlib.Path(arguments.github_output),
            authorization,
            root,
            value,
        )

    print(
        json.dumps(
            {
                "status": "PASS",
                "authorization": value["authorization"],
                "candidate": value["candidate"],
                "prepared_manifest_sha256": value["prepared_manifest_sha256"],
            },
            sort_keys=True,
        )
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
