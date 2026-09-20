#!/usr/bin/env python3
"""Static contract for protected-main generic TMR authorization-as-code."""

from __future__ import annotations

import argparse
import pathlib
import re


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
    require('      - "experiments/authorizations/topological-model-tmr-*.json"' in text, "generic authorization path trigger differs")
    for trigger in ("workflow_dispatch", "pull_request", "schedule", "workflow_run"):
        require(re.search(rf"(?m)^\s+{trigger}:\s*$", text) is None, f"authorization workflow exposes {trigger} trigger")

    require("permissions:\n  contents: read" in text, "validator permissions differ")
    require("cancel-in-progress: false" in text, "authorization workflow may cancel a formal authorization")
    require("group: topology-tmr-authorization" in text, "generic authorization concurrency differs")
    require("actions/checkout@3d3c42e5aac5ba805825da76410c181273ba90b1" in text, "checkout action is not pinned")
    require("fetch-depth: 0" in text, "authorization history is not fully fetched")

    require('test "${GITHUB_REF}" = "refs/heads/main"' in text, "authorization is not restricted to main")
    require('test "$(git rev-parse HEAD)" = "${GITHUB_SHA}"' in text, "authorization commit is not bound to the push SHA")
    whole_diff = 'git diff --name-status "${BEFORE_SHA}" "${GITHUB_SHA}"'
    require(whole_diff in text, "authorization addition is not whole-commit diff verified")
    require(
        whole_diff + " -- " not in text,
        "authorization diff is path-filtered and may hide unrelated changes",
    )
    require(
        "-- experiments/authorizations" not in text,
        "authorization diff remains restricted to the authorization directory",
    )
    require('if [ "${count}" -ne 1 ]' in text, "authorization commit may change multiple files")
    require('if [ "${status}" != "A" ]' in text, "authorization must be a newly added file")
    require("topological-model-tmr-[0-9a-f]{64}" in text, "authorization filename is not manifest-hash constrained")
    require("id: authorization_path" in text, "authorization path discovery step is absent")

    require("tools/topological_model_tmr_authorization.py" in text, "authorization validator is not invoked")
    require('--github-output "${GITHUB_OUTPUT}"' in text, "validated authorization outputs are not sealed into job outputs")
    require("prepared_artifact_sha256:" in text, "artifact digest is not propagated")
    require('git ls-remote --exit-code --tags origin "refs/tags/${TMR_CLAIM_TAG}"' in text, "pre-existing manifest claim is not rejected")

    require("uses: ./.github/workflows/topological-model-tmr-execute.yml" in text, "controller does not call the reusable executor")
    require("actions: read" in text and "contents: write" in text, "executor call permissions differ")
    for field in (
        "authorization_file",
        "candidate",
        "preparation_run_id",
        "prepared_artifact_id",
        "prepared_artifact_sha256",
        "prepared_manifest_sha256",
        "preparation_seal_sha256",
    ):
        expected = field + ": ${{ needs.authorize.outputs." + field + " }}"
        require(expected in text, f"validated authorization output is not passed: {field}")
    require("authorization_commit: ${{ github.sha }}" in text, "authorization commit is not passed to executor")

    legacy_manifest = "d8a7984a3aba3988b970ee734cc5240a035069731a4951ae5ae0f1b4616c8dfd"
    require(legacy_manifest not in text, "controller remains hard-bound to first manifest")
    require("            execute \\" not in text, "authorization controller must not contain a direct execute path")
    require("continue-on-error:" not in text, "authorization workflow weakens failure semantics")
    require("rerun" not in text.lower(), "authorization workflow contains a retry path")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
