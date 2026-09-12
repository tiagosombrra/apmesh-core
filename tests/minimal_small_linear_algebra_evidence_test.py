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
    parser.add_argument("--source-root", required=True)
    arguments = parser.parse_args()
    tool = load(pathlib.Path(arguments.tool))
    profile = tool.validate_profile(pathlib.Path(arguments.profile))
    tool.validate_source_root(pathlib.Path(arguments.source_root))
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
        entries = []
        for cell in ("gcc-debug", "gcc-release", "clang-debug", "clang-release"):
            for repetition in range(1, 4):
                copied = root / f"{cell}-{repetition}.json"
                copied.write_bytes(first.read_bytes())
                entries.append({"cell": cell, "repetition": repetition, "path": copied.name, "sha256": tool.sha256(copied)})
        index = root / "certificate-index.json"
        index.write_text(json.dumps({"schema_version": 1, "kind": "minimal-small-linear-algebra-certificate-index", "entries": entries}), encoding="utf-8")
        if tool.compare_index(profile, index)["state"] != "EVIDENCE_COLLECTED_PENDING_AUDIT":
            raise RuntimeError("cross-cell comparison made a scientific decision")
        bad = json.loads(first.read_text(encoding="utf-8"))
        bad["cases"].append(bad["cases"][0])
        duplicate = root / "duplicate.json"; duplicate.write_text(json.dumps(bad), encoding="utf-8")
        try:
            tool.validate_certificate(profile, duplicate)
        except tool.EvidenceError:
            pass
        else:
            raise RuntimeError("duplicate certificate family was accepted")
        forged = json.loads(first.read_text(encoding="utf-8"))
        forged["cases"][0]["observed"]["value"] = ["0x0p+0"]
        forged_path = root / "forged.json"; forged_path.write_text(json.dumps(forged), encoding="utf-8")
        try:
            tool.validate_certificate(profile, forged_path)
        except tool.EvidenceError:
            pass
        else:
            raise RuntimeError("forged certificate observation was accepted")
        wrong_placement = json.loads(first.read_text(encoding="utf-8"))
        target = next(item for item in wrong_placement["cases"] if item["id"] == "mat3_nonfinite_posinf_e8")
        target["inputs"][1] = "0x0p+0"
        wrong_path = root / "wrong-placement.json"; wrong_path.write_text(json.dumps(wrong_placement), encoding="utf-8")
        try:
            tool.validate_certificate(profile, wrong_path)
        except tool.EvidenceError:
            return 0
        raise RuntimeError("forged non-finite placement was accepted")


if __name__ == "__main__":
    raise SystemExit(main())
