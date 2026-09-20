#!/usr/bin/env python3
"""Static contract for the one-shot manual TMR execution workflow."""

from __future__ import annotations

import argparse
import pathlib
import re


CANDIDATE = "e5eda2663d6ff4b93ce1205660ff04d432acb9c0"
PREPARATION_RUN = "35524700979"
ARTIFACT_ID = "10609500629"
MANIFEST_SHA256 = "d8a7984a3aba3988b970ee734cc5240a035069731a4951ae5ae0f1b4616c8dfd"
SEAL_SHA256 = "982e1441f08bc3f11c3cffcf73113ce69a07e2066924ad01442b3ab94eeeb71e"
CLAIM_TAG = f"tmr-execution-claim-{MANIFEST_SHA256}"


def require(condition: bool, message: str) -> None:
    if not condition:
        raise RuntimeError(message)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--workflow", required=True)
    arguments = parser.parse_args()

    text = pathlib.Path(arguments.workflow).read_text(encoding="utf-8")

    require("name: Topological Model TMR Execution" in text, "workflow identity differs")
    require(re.search(r"(?m)^on:\n  workflow_dispatch:\s*$", text) is not None, "workflow is not manual-only")
    for trigger in ("push", "pull_request", "schedule", "workflow_run"):
        require(re.search(rf"(?m)^\s+{trigger}:\s*$", text) is None, f"workflow exposes {trigger} trigger")

    require("actions: read" in text and "contents: write" in text, "workflow permissions differ")
    require("cancel-in-progress: false" in text, "workflow concurrency may cancel a formal attempt")
    require(CANDIDATE in text, "candidate identity differs")
    require(PREPARATION_RUN in text, "preparation run identity differs")
    require(ARTIFACT_ID in text, "prepared artifact identity differs")
    require(MANIFEST_SHA256 in text, "prepared manifest identity differs")
    require(SEAL_SHA256 in text, "preparation seal identity differs")
    require(CLAIM_TAG in text, "one-time claim identity differs")

    require('if [ "${GITHUB_REF}" != "refs/heads/main" ]; then' in text, "workflow is not restricted to main dispatch")
    require("actions/checkout@3d3c42e5aac5ba805825da76410c181273ba90b1" in text, "checkout action is not pinned")
    require(f"ref: {CANDIDATE}" in text, "workflow does not check out exact candidate")
    require('git merge-base --is-ancestor "${TMR_CANDIDATE}" origin/main' in text, "published ancestry is not revalidated")
    require('git update-ref refs/remotes/origin/tmr-prepared-candidate "${TMR_CANDIDATE}"' in text, "sealed upstream identity is not reconstructed locally")
    require("git branch --set-upstream-to=origin/tmr-prepared-candidate tmr-execution-candidate" in text, "sealed upstream is not attached")

    require("actions/download-artifact@d3f86a106a0bac45b974a628896c90dbdf5c8093" in text, "download action is not pinned")
    require(f'artifact-ids: "{ARTIFACT_ID}"' in text, "workflow does not download exact artifact ID")
    require(f'run-id: "{PREPARATION_RUN}"' in text, "workflow does not download from exact preparation run")
    require("github-token: ${{ secrets.GITHUB_TOKEN }}" in text, "cross-run artifact download lacks repository token")
    require("path: ${{ runner.temp }}/apmesh-tmr-prepared" in text, "prepared package restore path differs")
    require("merge-multiple: true" in text, "artifact IDs are not merged directly into the sealed output root")

    require("validate-prepared \\" in text, "workflow does not preflight the complete PREPARED binding")
    require('sha256sum "${OUTPUT_ROOT}/prepared-manifest.json"' in text, "prepared manifest hash is not independently checked")
    require('sha256sum "${OUTPUT_ROOT}/preparation-seal.json"' in text, "preparation seal hash is not independently checked")
    require('test ! -e "${OUTPUT_ROOT}/execution-claim.json"' in text, "workflow does not require an unconsumed package")

    require(text.count('git tag -a "${TMR_CLAIM_TAG}"') == 1, "workflow must create exactly one remote execution claim")
    require('git ls-remote --exit-code --tags origin "refs/tags/${TMR_CLAIM_TAG}"' in text, "existing claim is not checked")
    require('git push origin "refs/tags/${TMR_CLAIM_TAG}"' in text, "execution claim is not persisted remotely")
    require(text.count("            execute \\") == 1, "workflow must expose exactly one execute invocation")

    preflight = text.index("validate-prepared \\")
    claim = text.index('git tag -a "${TMR_CLAIM_TAG}"')
    execution = text.index("            execute \\")
    require(preflight < claim < execution, "workflow order is not preflight -> claim -> execute")

    require("if: always()" in text, "terminal retention is not failure-resilient")
    require('find "${OUTPUT_ROOT}/cells"' in text and "-name build" in text, "reproducible build trees are not pruned")
    require("verify-retention \\" in text, "terminal retention is not independently verified")
    require("actions/upload-artifact@ea165f8d65b6e75b540449e92b4886f43607fa02" in text, "terminal upload action is not pinned")
    require("path: ${{ runner.temp }}/apmesh-tmr-prepared" in text, "terminal artifact path differs")
    require("if-no-files-found: error" in text and "overwrite: false" in text, "terminal artifact retention is not fail-closed")

    require("continue-on-error:" not in text, "workflow weakens a formal execution failure")
    require("rerun" not in text.lower(), "workflow contains a retry path")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
