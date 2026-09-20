#!/usr/bin/env python3
"""Validate repository-resident one-shot Topological Model TMR authorization."""

from __future__ import annotations

import argparse
import json
import pathlib
from typing import Any


EXPECTED = {
    "schema_version": 1,
    "kind": "topological-model-tmr-execution-authorization",
    "authorization": "EXECUTE_ONCE",
    "candidate": "e5eda2663d6ff4b93ce1205660ff04d432acb9c0",
    "preparation_run_id": 35524700979,
    "prepared_artifact_id": 10609500629,
    "prepared_artifact_sha256": "2dec472689c62e813c3ec80896163a71f9d055ca1bd8cfeadfa7943408aefa72",
    "prepared_manifest_sha256": "d8a7984a3aba3988b970ee734cc5240a035069731a4951ae5ae0f1b4616c8dfd",
    "preparation_seal_sha256": "982e1441f08bc3f11c3cffcf73113ce69a07e2066924ad01442b3ab94eeeb71e",
    "preparation_audit": "docs/audits/2026-09-20-topological-model-tmr-preparation-audit.md",
    "execution_workflow": ".github/workflows/topological-model-tmr-execute.yml",
    "terminal_audit_required": True,
}


class AuthorizationError(RuntimeError):
    pass


def fail(message: str) -> AuthorizationError:
    return AuthorizationError(message)


def read_json(path: pathlib.Path) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        raise fail(f"authorization JSON could not be read: {error}") from error
    if not isinstance(value, dict):
        raise fail("authorization root must be a JSON object")
    return value


def validate_authorization(
    authorization: pathlib.Path,
    source_root: pathlib.Path,
) -> dict[str, Any]:
    root = source_root.resolve()
    path = authorization.resolve()
    expected_directory = (root / "experiments" / "authorizations").resolve()

    if path.parent != expected_directory:
        raise fail("authorization must live directly in experiments/authorizations")

    expected_name = (
        "topological-model-tmr-"
        f"{EXPECTED['prepared_manifest_sha256']}.json"
    )
    if path.name != expected_name:
        raise fail("authorization filename does not match the prepared-manifest identity")

    value = read_json(path)
    if set(value) != set(EXPECTED):
        raise fail("authorization schema differs")

    for key, expected in EXPECTED.items():
        if value.get(key) != expected:
            raise fail(f"authorization field differs: {key}")

    audit = (root / value["preparation_audit"]).resolve()
    if root not in audit.parents or not audit.is_file():
        raise fail("preparation audit authority is absent or outside the source root")

    return value


def write_github_output(
    output: pathlib.Path,
    authorization: pathlib.Path,
    source_root: pathlib.Path,
    value: dict[str, Any],
) -> None:
    root = source_root.resolve()
    relative = authorization.resolve().relative_to(root).as_posix()
    claim = f"tmr-execution-claim-{value['prepared_manifest_sha256']}"
    rows = {
        "authorization_file": relative,
        "candidate": value["candidate"],
        "preparation_run_id": str(value["preparation_run_id"]),
        "prepared_artifact_id": str(value["prepared_artifact_id"]),
        "prepared_manifest_sha256": value["prepared_manifest_sha256"],
        "preparation_seal_sha256": value["preparation_seal_sha256"],
        "claim_tag": claim,
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
