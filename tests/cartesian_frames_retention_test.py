#!/usr/bin/env python3

import argparse
import importlib.util
import json
import pathlib
import tempfile
from unittest.mock import patch


def rejects(action, message: str) -> None:
    try:
        action()
    except Exception:
        return
    raise RuntimeError(message)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--runner", required=True)
    parser.add_argument("--source-root", required=True)
    parser.add_argument("--profile", required=True)
    parser.add_argument("--protocol", required=True)
    parser.add_argument("--exporter", required=True)
    parser.add_argument("--validator", required=True)
    arguments = parser.parse_args()
    specification = importlib.util.spec_from_file_location("cartesian_frames_runner_retention", arguments.runner)
    if specification is None or specification.loader is None:
        raise RuntimeError("runner module could not be loaded")
    runner = importlib.util.module_from_spec(specification)
    specification.loader.exec_module(runner)
    profile = json.loads(pathlib.Path(arguments.profile).read_text(encoding="utf-8"))
    if profile["retention_negative_cases"] != ["missing_retained_hash"]:
        raise RuntimeError("retention negative profile differs")

    with tempfile.TemporaryDirectory(prefix="apmesh-core-cf-retention-") as temporary:
        root = pathlib.Path(temporary) / "evidence"
        source = pathlib.Path(arguments.source_root).resolve()
        candidate = {"commit": "0123456789abcdef0123456789abcdef01234567", "upstream_commit": "0123456789abcdef0123456789abcdef01234567", "tree_clean": True, "source_root": str(source), "source_inventory": []}
        preparation = argparse.Namespace(source_root=str(source), profile=arguments.profile, protocol=arguments.protocol,
                                         exporter=arguments.exporter, validator=arguments.validator, output_root=str(root))
        with patch.object(runner, "published_candidate", return_value=candidate), patch.object(runner, "environment_identity", return_value={"test": "identity"}):
            runner.prepare(preparation)
        manifest = runner.read_json(root / "prepared-manifest.json")
        runner.write_json(root / "command-records.json", {"schema_version": 1, "records": []})
        runner.write_json(root / "observed-inventories.json", {"schema_version": 1, "inventories": []})
        runner.write_state(root, "RUNNING", {"candidate_commit": candidate["commit"]})
        runner.write_state(root, "EXECUTED_PENDING_AUDIT", {"candidate_commit": candidate["commit"]})
        runner.write_json(root / "terminal-manifest.json", {"schema_version": 1, "kind": "cartesian-frames-terminal-manifest", "state": "EXECUTED_PENDING_AUDIT", "candidate": candidate, "prepared_manifest_sha256": runner.sha256_file(root / "prepared-manifest.json"), "command_records": "command-records.json", "inventories": "observed-inventories.json", "limitations": manifest["retained_limitations"]})
        runner.write_json(root / "detached-verification.json", {"schema_version": 1, "kind": "cartesian-frames-detached-verification", "result": "PASS", "candidate_commit": candidate["commit"], "source_inventory_count": 0})
        files = []
        for path in sorted((path for path in root.rglob("*") if path.is_file()), key=lambda item: item.as_posix()):
            relative = runner.relative_path(root, path)
            files.append({"path": relative, "role": "retained-evidence", "sha256": runner.sha256_file(path), "size": path.stat().st_size})
        runner.write_json(root / "retention-manifest.json", {"schema_version": 1, "kind": "cartesian-frames-retention", "candidate_commit": candidate["commit"], "prepared_manifest_sha256": runner.sha256_file(root / "prepared-manifest.json"), "required_paths": [entry["path"] for entry in files], "files": files})
        verified = runner.verify_retention(root)
        if verified["status"] != "PASS":
            raise RuntimeError("retention verification did not pass")
        retained = root / files[0]["path"]
        retained.write_bytes(retained.read_bytes() + b"x")
        rejects(lambda: runner.verify_retention(root), "missing retained hash mutation was accepted")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
