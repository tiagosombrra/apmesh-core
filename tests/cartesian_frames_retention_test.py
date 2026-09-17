#!/usr/bin/env python3

import argparse
import importlib.util
import json
import pathlib
import subprocess
import tempfile
from unittest.mock import patch


def rejects(action, message: str) -> None:
    try:
        action()
    except Exception:
        return
    raise RuntimeError(message)


def git(root: pathlib.Path, *arguments: str) -> None:
    completed = subprocess.run(["git", *arguments], cwd=root, capture_output=True, text=True, check=False)
    if completed.returncode != 0:
        raise RuntimeError(f"git command failed: {' '.join(arguments)}: {completed.stderr}")


def main() -> int:
    parser = argparse.ArgumentParser()
    for name in ("runner", "source-root", "profile", "protocol", "exporter", "validator"):
        parser.add_argument(f"--{name}", required=True)
    arguments = parser.parse_args()
    specification = importlib.util.spec_from_file_location("cartesian_frames_runner_retention", arguments.runner)
    if specification is None or specification.loader is None:
        raise RuntimeError("runner module could not be loaded")
    runner = importlib.util.module_from_spec(specification)
    specification.loader.exec_module(runner)
    profile = json.loads(pathlib.Path(arguments.profile).read_text(encoding="utf-8"))
    if profile["retention_negative_cases"] != ["missing_retained_hash", "rehashed_failure_records", "forged_detached_verification"]:
        raise RuntimeError("retention negative profile differs")

    with tempfile.TemporaryDirectory(prefix="apmesh-core-cf-retention-") as temporary:
        temporary_root = pathlib.Path(temporary)
        candidate_root = temporary_root / "candidate"
        candidate_root.mkdir()
        git(candidate_root, "init")
        git(candidate_root, "config", "user.email", "retention@example.invalid")
        git(candidate_root, "config", "user.name", "Retention Contract")
        (candidate_root / "tracked.txt").write_text("detached verification source\n", encoding="utf-8")
        git(candidate_root, "add", "tracked.txt")
        git(candidate_root, "commit", "-m", "candidate")
        candidate = runner.clean_candidate(candidate_root)
        candidate["upstream_commit"] = candidate["commit"]
        evidence_root = temporary_root / "evidence"
        preparation = argparse.Namespace(source_root=arguments.source_root, profile=arguments.profile, protocol=arguments.protocol,
                                         exporter=arguments.exporter, validator=arguments.validator, output_root=str(evidence_root))
        with patch.object(runner, "published_candidate", return_value=candidate), patch.object(runner, "environment_identity", return_value={"test": "identity"}):
            runner.prepare(preparation)
        manifest = runner.read_json(evidence_root / "prepared-manifest.json")
        runner.write_json(evidence_root / "execution-claim.json", {
            "schema_version": 1, "kind": "cartesian-frames-execution-claim", "candidate_commit": candidate["commit"],
            "prepared_manifest_sha256": runner.sha256_file(evidence_root / "prepared-manifest.json"), "pid": 1,
            "started_utc": "2026-01-01T00:00:00+00:00",
        })
        records: list[dict] = []
        runner.write_records(evidence_root, records)
        runner.write_json(evidence_root / "failure.json", {
            "schema_version": 1, "kind": "cartesian-frames-failure", "message": "intentional focused partial failure",
            "records": records, "partial_certificate_slots": [],
        })
        runner.write_state(evidence_root, "RUNNING", {"candidate_commit": candidate["commit"]})
        runner.write_state(evidence_root, "BLOCKED", {"candidate_commit": candidate["commit"], "closure_failure": True, "failure": "failure.json"})
        runner.write_terminal(evidence_root, manifest, "BLOCKED", {"failure": "failure.json"})
        runner.seal_output(evidence_root, manifest, runner.FAILURE_FILES)
        if runner.verify_retention(evidence_root)["status"] != "PASS":
            raise RuntimeError("retention verification did not pass")

        retained = evidence_root / "failure.json"
        retained.write_bytes(retained.read_bytes() + b"x")
        rejects(lambda: runner.verify_retention(evidence_root), "missing retained hash mutation was accepted")

        runner.write_json(retained, {
            "schema_version": 1, "kind": "cartesian-frames-failure", "message": "intentional focused partial failure",
            "records": [{"id": "forged"}], "partial_certificate_slots": [],
        })
        runner.write_retention_inventory(evidence_root, manifest, runner.FAILURE_FILES)
        rejects(lambda: runner.verify_retention(evidence_root), "rehashed semantic failure mutation was accepted")

        runner.write_json(retained, {
            "schema_version": 1, "kind": "cartesian-frames-failure", "message": "intentional focused partial failure",
            "records": records, "partial_certificate_slots": [],
        })
        runner.write_retention_inventory(evidence_root, manifest, runner.FAILURE_FILES)
        detached = runner.read_json(evidence_root / "detached-verification.json")
        detached["source_inventory_count"] += 1
        runner.write_json(evidence_root / "detached-verification.json", detached)
        runner.write_retention_inventory(evidence_root, manifest, runner.FAILURE_FILES)
        rejects(lambda: runner.verify_retention(evidence_root), "forged detached verification was accepted")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
