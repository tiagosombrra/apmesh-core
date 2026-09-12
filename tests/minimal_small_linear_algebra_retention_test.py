#!/usr/bin/env python3
"""Focused positive and negative retention checks for LA qualification evidence."""

from __future__ import annotations

import argparse
import hashlib
import importlib.util
import json
import pathlib
import tempfile


def load(path: pathlib.Path):
    spec = importlib.util.spec_from_file_location("la_runner", path)
    if spec is None or spec.loader is None:
        raise RuntimeError("cannot load runner")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def digest(path: pathlib.Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--runner", required=True)
    arguments = parser.parse_args()
    runner = load(pathlib.Path(arguments.runner))
    with tempfile.TemporaryDirectory(prefix="apmesh-core-la-retention-") as temporary:
        root = pathlib.Path(temporary)
        prepared = root / "prepared-manifest.json"; summary = root / "summary.json"
        prepared.write_text("{}\n", encoding="utf-8"); summary.write_text("{}\n", encoding="utf-8")
        manifest = {"schema_version": 1, "kind": "minimal-small-linear-algebra-retention", "candidate_commit": "a" * 40,
                    "prepared_manifest_sha256": digest(prepared), "files": [{"path": "summary.json", "sha256": digest(summary), "size": summary.stat().st_size}]}
        (root / "retention-manifest.json").write_text(json.dumps(manifest), encoding="utf-8")
        runner.verify_retention(argparse.Namespace(output_root=str(root)))
        summary.write_text("tampered\n", encoding="utf-8")
        try:
            runner.verify_retention(argparse.Namespace(output_root=str(root)))
        except runner.RuntimeErrorEvidence:
            return 0
        raise RuntimeError("tampered retention was accepted")


if __name__ == "__main__":
    raise SystemExit(main())
