#!/usr/bin/env python3
"""Focused contract for the repository-resident TMR execution authorization."""

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


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--tool", required=True)
    arguments = parser.parse_args()

    module = load_module(pathlib.Path(arguments.tool))

    with tempfile.TemporaryDirectory(prefix="apmesh-tmr-authorization-") as temporary:
        root = pathlib.Path(temporary)
        authorization_dir = root / "experiments" / "authorizations"
        audit = root / module.EXPECTED["preparation_audit"]
        authorization_dir.mkdir(parents=True)
        audit.parent.mkdir(parents=True)
        audit.write_text("prepared package audit authority\n", encoding="utf-8")

        manifest = module.EXPECTED["prepared_manifest_sha256"]
        authorization = authorization_dir / f"topological-model-tmr-{manifest}.json"
        authorization.write_text(
            json.dumps(module.EXPECTED, indent=2, sort_keys=True) + "\n",
            encoding="utf-8",
        )

        observed = module.validate_authorization(authorization, root)
        require(observed == module.EXPECTED, "valid authorization projection differs")

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
        require(rows["candidate"] == module.EXPECTED["candidate"], "candidate output differs")
        require(
            rows["preparation_run_id"] == str(module.EXPECTED["preparation_run_id"]),
            "preparation run output differs",
        )
        require(
            rows["prepared_artifact_id"] == str(module.EXPECTED["prepared_artifact_id"]),
            "artifact output differs",
        )
        require(
            rows["prepared_manifest_sha256"] == manifest,
            "manifest output differs",
        )
        require(
            rows["preparation_seal_sha256"] == module.EXPECTED["preparation_seal_sha256"],
            "seal output differs",
        )
        require(
            rows["claim_tag"] == f"tmr-execution-claim-{manifest}",
            "claim output differs",
        )

        wrong_candidate = dict(module.EXPECTED)
        wrong_candidate["candidate"] = "0" * 40
        authorization.write_text(
            json.dumps(wrong_candidate, indent=2, sort_keys=True) + "\n",
            encoding="utf-8",
        )
        expect_rejection(module, authorization, root)

        extra = dict(module.EXPECTED)
        extra["retry"] = True
        authorization.write_text(
            json.dumps(extra, indent=2, sort_keys=True) + "\n",
            encoding="utf-8",
        )
        expect_rejection(module, authorization, root)

        authorization.write_text(
            json.dumps(module.EXPECTED, indent=2, sort_keys=True) + "\n",
            encoding="utf-8",
        )
        wrong_name = authorization_dir / "topological-model-tmr-wrong.json"
        wrong_name.write_text(authorization.read_text(encoding="utf-8"), encoding="utf-8")
        expect_rejection(module, wrong_name, root)

        audit.unlink()
        expect_rejection(module, authorization, root)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
