#!/usr/bin/env python3
"""Focused contract for generic repository-resident TMR execution authorization."""

from __future__ import annotations

import argparse
import importlib.util
import json
import pathlib
import tempfile


def require(condition: bool, message: str) -> None:
    if not condition:
        raise RuntimeError(message)


def load_module(path: pathlib.Path):
    spec = importlib.util.spec_from_file_location("tmr_authorization", path)
    if spec is None or spec.loader is None:
        raise RuntimeError("authorization validator module could not be loaded")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def expect_rejection(module, path: pathlib.Path, root: pathlib.Path) -> None:
    try:
        module.validate_authorization(path, root)
    except module.AuthorizationError:
        return
    raise RuntimeError("invalid authorization was accepted")


def audit_value(value: dict) -> dict:
    return {
        "schema_version": 1,
        "kind": "topological-model-tmr-corrected-preparation-audit",
        "decision": "PASS",
        "lifecycle_state": "PREPARED",
        "execution_requested": False,
        "candidate_commit": value["candidate"],
        "preparation": {
            "run_id": value["preparation_run_id"],
            "artifact_id": value["prepared_artifact_id"],
            "artifact_sha256": value["prepared_artifact_sha256"],
            "prepared_manifest_sha256": value["prepared_manifest_sha256"],
            "preparation_seal_sha256": value["preparation_seal_sha256"],
        },
        "execution_absence": {
            "execution_claim": False,
            "terminal_manifest": False,
            "command_records": False,
            "certificate_index": False,
            "failure_record": False,
            "all_gates": "NOT_EXECUTED",
        },
        "formal_execution_authorized": False,
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--tool", required=True)
    arguments = parser.parse_args()

    module = load_module(pathlib.Path(arguments.tool))

    with tempfile.TemporaryDirectory(prefix="apmesh-tmr-authorization-") as temporary:
        root = pathlib.Path(temporary)
        authorization_dir = root / "experiments" / "authorizations"
        audit_dir = root / "docs" / "audits"
        authorization_dir.mkdir(parents=True)
        audit_dir.mkdir(parents=True)

        value = {
            "schema_version": 1,
            "kind": module.KIND,
            "authorization": module.AUTHORIZATION,
            "candidate": "1" * 40,
            "preparation_run_id": 123456,
            "prepared_artifact_id": 654321,
            "prepared_artifact_sha256": "2" * 64,
            "prepared_manifest_sha256": "3" * 64,
            "preparation_seal_sha256": "4" * 64,
            "preparation_audit": "docs/audits/2026-09-20-topological-model-tmr-corrected-preparation-audit.json",
            "execution_workflow": module.EXECUTION_WORKFLOW,
            "terminal_audit_required": True,
        }

        audit = root / value["preparation_audit"]
        audit.write_text(
            json.dumps(audit_value(value), indent=2, sort_keys=True) + "\n",
            encoding="utf-8",
        )

        manifest = value["prepared_manifest_sha256"]
        authorization = authorization_dir / f"topological-model-tmr-{manifest}.json"
        authorization.write_text(
            json.dumps(value, indent=2, sort_keys=True) + "\n",
            encoding="utf-8",
        )

        observed = module.validate_authorization(authorization, root)
        require(observed == value, "valid authorization projection differs")

        github_output = root / "github-output.txt"
        module.write_github_output(github_output, authorization, root, observed)
        rows = dict(
            line.split("=", 1)
            for line in github_output.read_text(encoding="utf-8").splitlines()
        )
        require(
            rows["authorization_file"]
            == f"experiments/authorizations/topological-model-tmr-{manifest}.json",
            "authorization output path differs",
        )
        require(rows["candidate"] == value["candidate"], "candidate output differs")
        require(
            rows["preparation_run_id"] == str(value["preparation_run_id"]),
            "preparation run output differs",
        )
        require(
            rows["prepared_artifact_id"] == str(value["prepared_artifact_id"]),
            "artifact id output differs",
        )
        require(
            rows["prepared_artifact_sha256"] == value["prepared_artifact_sha256"],
            "artifact digest output differs",
        )
        require(
            rows["prepared_manifest_sha256"] == manifest,
            "manifest output differs",
        )
        require(
            rows["preparation_seal_sha256"] == value["preparation_seal_sha256"],
            "seal output differs",
        )
        require(
            rows["claim_tag"] == f"tmr-execution-claim-{manifest}",
            "claim output differs",
        )

        wrong_candidate = dict(value)
        wrong_candidate["candidate"] = "0" * 40
        authorization.write_text(
            json.dumps(wrong_candidate, indent=2, sort_keys=True) + "\n",
            encoding="utf-8",
        )
        expect_rejection(module, authorization, root)

        extra = dict(value)
        extra["retry"] = True
        authorization.write_text(
            json.dumps(extra, indent=2, sort_keys=True) + "\n",
            encoding="utf-8",
        )
        expect_rejection(module, authorization, root)

        authorization.write_text(
            json.dumps(value, indent=2, sort_keys=True) + "\n",
            encoding="utf-8",
        )
        wrong_name = authorization_dir / "topological-model-tmr-wrong.json"
        wrong_name.write_text(authorization.read_text(encoding="utf-8"), encoding="utf-8")
        expect_rejection(module, wrong_name, root)

        bad_audit = audit_value(value)
        bad_audit["preparation"]["artifact_sha256"] = "5" * 64
        audit.write_text(
            json.dumps(bad_audit, indent=2, sort_keys=True) + "\n",
            encoding="utf-8",
        )
        expect_rejection(module, authorization, root)

        audit.write_text(
            json.dumps(audit_value(value), indent=2, sort_keys=True) + "\n",
            encoding="utf-8",
        )
        bad_audit = audit_value(value)
        bad_audit["formal_execution_authorized"] = True
        audit.write_text(
            json.dumps(bad_audit, indent=2, sort_keys=True) + "\n",
            encoding="utf-8",
        )
        expect_rejection(module, authorization, root)

        audit.write_text(
            json.dumps(audit_value(value), indent=2, sort_keys=True) + "\n",
            encoding="utf-8",
        )
        audit.unlink()
        expect_rejection(module, authorization, root)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
