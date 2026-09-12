#!/usr/bin/env python3
"""Focused schema, certificate, comparison, and negative checks for LA evidence."""

from __future__ import annotations

import argparse
import importlib.util
import json
import pathlib
import subprocess
import tempfile


def load(path: pathlib.Path):
    spec = importlib.util.spec_from_file_location("la_evidence", path)
    if spec is None or spec.loader is None:
        raise RuntimeError("cannot load evidence tool")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--exporter", required=True)
    parser.add_argument("--profile", required=True)
    parser.add_argument("--tool", required=True)
    arguments = parser.parse_args()
    tool = load(pathlib.Path(arguments.tool))
    profile = tool.validate_profile(pathlib.Path(arguments.profile))
    with tempfile.TemporaryDirectory(prefix="apmesh-core-la-evidence-") as temporary:
        root = pathlib.Path(temporary)
        first = root / "first.json"; second = root / "second.json"; third = root / "third.json"
        for path in (first, second, third):
            completed = subprocess.run([arguments.exporter, str(path)], capture_output=True, text=True, check=False)
            if completed.returncode != 0:
                raise RuntimeError(f"exporter failed: {completed.stderr}")
            tool.validate_certificate(profile, path)
        comparison = tool.compare_certificates(profile, [first, second, third])
        if comparison["state"] != "EVIDENCE_COLLECTED_PENDING_AUDIT":
            raise RuntimeError("comparison made a scientific decision")
        bad = json.loads(first.read_text(encoding="utf-8"))
        bad["cases"].append(bad["cases"][0])
        duplicate = root / "duplicate.json"; duplicate.write_text(json.dumps(bad), encoding="utf-8")
        try:
            tool.validate_certificate(profile, duplicate)
        except tool.EvidenceError:
            return 0
        raise RuntimeError("duplicate certificate family was accepted")


if __name__ == "__main__":
    raise SystemExit(main())
