#!/usr/bin/env python3
"""Static contract for the authorization-gated reusable CGR executor."""

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

    require(
        "name: Continuous Curve Geometry Regression Execution" in text,
        "workflow identity differs",
    )
    require(
        re.search(r"(?m)^on:\n  workflow_call:\s*$", text) is not None,
        "workflow is not reusable-only",
    )
    for trigger in ("workflow_dispatch", "push", "pull_request", "schedule", "workflow_run"):
        require(
            re.search(rf"(?m)^\s+{trigger}:\s*$", text) is None,
            f"workflow exposes {trigger} trigger",
        )

    for name in (
        "authorization_file",
        "authorization_commit",
        "candidate",
        "preparation_run_id",
        "prepared_artifact_id",
        "prepared_artifact_sha256",
        "prepared_manifest_sha256",
        "preparation_seal_sha256",
    ):
        require(f"      {name}:" in text, f"workflow_call input is absent: {name}")

    require("actions: read" in text and "contents: write" in text, "workflow permissions differ")
    require("cancel-in-progress: false" in text, "formal attempt may be cancelled")
    require(
        "group: continuous-curve-geometry-regression-execution-${{ inputs.prepared_manifest_sha256 }}"
        in text,
        "execution concurrency is not manifest-bound",
    )
    require(
        "CGR_CLAIM_TAG: cgr-execution-claim-${{ inputs.prepared_manifest_sha256 }}" in text,
        "claim tag is not manifest-bound",
    )
    require(
        'test "${CGR_AUTHORIZATION_FILE}" = "experiments/authorizations/continuous-curve-geometry-regression-${CGR_PREPARED_MANIFEST_SHA256}.json"'
        in text,
        "authorization path is not manifest-derived",
    )

    require('test "${GITHUB_REF}" = "refs/heads/main"' in text, "workflow is not protected-main-only")
    require('test "${GITHUB_SHA}" = "${CGR_AUTHORIZATION_COMMIT}"' in text, "caller authorization commit is not bound")
    require(
        "tools/continuous_curve_geometry_regression_authorization.py" in text,
        "committed authorization is not independently revalidated",
    )
    require("authorization input differs" in text, "caller inputs are not compared with authorization JSON")

    require(
        text.count("actions/checkout@3d3c42e5aac5ba805825da76410c181273ba90b1") == 2,
        "authorization and candidate checkouts must both be pinned",
    )
    require("ref: ${{ inputs.authorization_commit }}" in text, "authorization checkout is not exact")
    require("ref: ${{ inputs.candidate }}" in text, "candidate checkout is not exact")
    require(
        'git merge-base --is-ancestor "${CGR_CANDIDATE}" origin/main' in text,
        "published ancestry is not revalidated",
    )
    require(
        'git update-ref refs/remotes/origin/cgr-prepared-candidate "${CGR_CANDIDATE}"' in text,
        "sealed upstream identity is not reconstructed",
    )

    require("Require exact audited artifact metadata" in text, "artifact metadata preflight is absent")
    require("/actions/artifacts/{artifact_id}" in text, "artifact metadata API endpoint is absent")
    require("prepared artifact digest differs" in text, "artifact digest is not fail-closed")
    require("prepared artifact run differs" in text, "artifact run provenance is not checked")
    require("prepared artifact candidate differs" in text, "artifact candidate provenance is not checked")
    require('workflow_run.get("head_branch") != "main"' in text, "artifact branch provenance is not checked")

    require(
        "actions/download-artifact@d3f86a106a0bac45b974a628896c90dbdf5c8093" in text,
        "download action is not pinned",
    )
    require("artifact-ids: ${{ inputs.prepared_artifact_id }}" in text, "authorized artifact ID is not used")
    require("run-id: ${{ inputs.preparation_run_id }}" in text, "authorized preparation run is not used")
    require("github-token: ${{ secrets.GITHUB_TOKEN }}" in text, "cross-run artifact download lacks token")
    require("path: ${{ runner.temp }}/apmesh-cgr-prepared" in text, "prepared restore path differs")

    require("validate-prepared \\" in text, "complete PREPARED preflight is absent")
    require('sha256sum "${OUTPUT_ROOT}/prepared-manifest.json"' in text, "manifest hash is not independently checked")
    require('sha256sum "${OUTPUT_ROOT}/preparation-seal.json"' in text, "seal hash is not independently checked")
    require('test ! -e "${OUTPUT_ROOT}/execution-claim.json"' in text, "workflow does not require an unconsumed package")

    require(text.count('git tag -a "${CGR_CLAIM_TAG}"') == 1, "workflow must create exactly one claim")
    require('git ls-remote --exit-code --tags origin "refs/tags/${CGR_CLAIM_TAG}"' in text, "existing claim is not checked")
    require('git push origin "refs/tags/${CGR_CLAIM_TAG}"' in text, "claim is not persisted remotely")
    require(text.count("            execute \\") == 1, "workflow must expose exactly one execute invocation")

    authorization = text.index("continuous_curve_geometry_regression_authorization.py")
    artifact = text.index("Require exact audited artifact metadata")
    preflight = text.index("validate-prepared \\")
    claim = text.index('git tag -a "${CGR_CLAIM_TAG}"')
    execution = text.index("            execute \\")
    require(
        authorization < artifact < preflight < claim < execution,
        "workflow order is not authorization -> artifact -> preflight -> claim -> execute",
    )

    require("if: always()" in text, "terminal retention is not failure-resilient")
    require('find "${OUTPUT_ROOT}/cells"' in text and "-name build" in text, "build trees are not pruned")
    require("verify-retention \\" in text, "terminal retention is not independently verified")
    require(
        "actions/upload-artifact@ea165f8d65b6e75b540449e92b4886f43607fa02" in text,
        "terminal upload action is not pinned",
    )
    require(
        "name: cgr-terminal-${{ inputs.prepared_manifest_sha256 }}-${{ github.run_id }}" in text,
        "terminal artifact name is not manifest-bound",
    )
    require("continue-on-error:" not in text, "workflow weakens a formal failure")
    require("rerun" not in text.lower(), "workflow contains a retry path")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
