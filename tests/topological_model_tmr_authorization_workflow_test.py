#!/usr/bin/env python3
"""Static contract for protected-main TMR authorization-as-code."""

from __future__ import annotations

import argparse
import pathlib
import re


MANIFEST_SHA256 = "d8a7984a3aba3988b970ee734cc5240a035069731a4951ae5ae0f1b4616c8dfd"
AUTHORIZATION_FILE = (
    "experiments/authorizations/topological-model-tmr-"
    f"{MANIFEST_SHA256}.json"
)


def require(condition: bool, message: str) -> None:
    if not condition:
        raise RuntimeError(message)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--workflow", required=True)
    arguments = parser.parse_args()

    text = pathlib.Path(arguments.workflow).read_text(encoding="utf-8")

    require("name: Topological Model TMR Authorization" in text, "workflow identity differs")
    require(re.search(r"(?m)^on:\n  push:\n    branches:\n      - main\n    paths:\n", text) is not None, "authorization workflow trigger differs")
    require(f'      - "{AUTHORIZATION_FILE}"' in text, "authorization path trigger differs")
    for trigger in ("workflow_dispatch", "pull_request", "schedule", "workflow_run"):
        require(re.search(rf"(?m)^\s+{trigger}:\s*$", text) is None, f"authorization workflow exposes {trigger} trigger")

    require("permissions:\n  contents: read" in text, "validator permissions differ")
    require("cancel-in-progress: false" in text, "authorization workflow may cancel a formal authorization")
    require(AUTHORIZATION_FILE in text, "authorization file identity differs")
    require("actions/checkout@3d3c42e5aac5ba805825da76410c181273ba90b1" in text, "checkout action is not pinned")
    require("fetch-depth: 0" in text, "authorization history is not fully fetched")

    require('test "${GITHUB_REF}" = "refs/heads/main"' in text, "authorization is not restricted to main")
    require('test "$(git rev-parse HEAD)" = "${GITHUB_SHA}"' in text, "authorization commit is not bound to the push SHA")
    require('git diff --name-status "${BEFORE_SHA}" "${GITHUB_SHA}"' in text, "authorization addition is not diff-verified")
    require("printf 'A\\t%s'" in text, "authorization must be a newly added file")
    require("tools/topological_model_tmr_authorization.py" in text, "authorization validator is not invoked")
    require('--github-output "${GITHUB_OUTPUT}"' in text, "validated authorization outputs are not sealed into job outputs")
    require('git ls-remote --exit-code --tags origin "refs/tags/${TMR_CLAIM_TAG}"' in text, "pre-existing claim is not rejected")

    require("uses: ./.github/workflows/topological-model-tmr-execute.yml" in text, "controller does not call the reusable executor")
    require("actions: read" in text and "contents: write" in text, "executor call permissions differ")
    require("authorization_file: ${{ needs.authorize.outputs.authorization_file }}" in text, "authorization file is not passed from validation")
    require("authorization_commit: ${{ github.sha }}" in text, "authorization commit is not passed to executor")
    require("candidate: ${{ needs.authorize.outputs.candidate }}" in text, "candidate is not passed from validation")
    require("preparation_run_id: ${{ needs.authorize.outputs.preparation_run_id }}" in text, "preparation run is not passed from validation")
    require("prepared_artifact_id: ${{ needs.authorize.outputs.prepared_artifact_id }}" in text, "artifact is not passed from validation")
    require("prepared_manifest_sha256: ${{ needs.authorize.outputs.prepared_manifest_sha256 }}" in text, "manifest identity is not passed from validation")
    require("preparation_seal_sha256: ${{ needs.authorize.outputs.preparation_seal_sha256 }}" in text, "seal identity is not passed from validation")

    require("            execute \\" not in text, "authorization controller must not contain a direct execute path")
    require("continue-on-error:" not in text, "authorization workflow weakens failure semantics")
    require("rerun" not in text.lower(), "authorization workflow contains a retry path")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
