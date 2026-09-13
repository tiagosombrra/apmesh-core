#!/usr/bin/env python3
"""Focused schema, certificate, comparison, and negative checks for LA evidence."""

from __future__ import annotations

import argparse
import importlib.util
import json
import pathlib
import subprocess
import tempfile
import copy
import shutil


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
        # Every rejected constructor preserves all entries, including the finite ones.
        nonfinite_rows = [row for row in tool.read_json(first)["cases"] if "_nonfinite_" in row["id"]]
        if len(nonfinite_rows) != 39 or any(len(row["inputs"]) != row["dimension"] ** 2 or row["input_layout"] != "matrix_row_major" for row in nonfinite_rows):
            raise RuntimeError("complete non-finite matrix coverage differs")
        negative_root = root / "negatives"
        tool.generate_negative_outcomes(profile, first, negative_root)
        negative_path = negative_root / "negative-outcomes.json"
        validator_hash = tool.sha256(pathlib.Path(arguments.tool))
        def revalidate(): return tool.validate_negative_outcomes(profile, negative_path, tool.sha256(first), validator_hash)
        revalidate()
        saved = negative_path.read_bytes()
        for kind in ("missing", "duplicate", "reason", "outcome", "validator"):
            report = tool.read_json(negative_path)
            if kind == "missing": report["entries"].pop()
            elif kind == "duplicate": report["entries"][-1] = report["entries"][0]
            elif kind == "reason": report["entries"][0]["expected_reason"] = "unrelated error"
            elif kind == "outcome": report["entries"][0]["observed"]["outcome"] = "PASS"
            else: report["validator_sha256"] = "0" * 64
            tool.write_json(negative_path, report)
            try: revalidate()
            except tool.EvidenceError: pass
            else: raise RuntimeError(f"tampered negative report accepted: {kind}")
            negative_path.write_bytes(saved)
        # Rehashing an unrelated rejection does not make it the declared mutation.
        report = tool.read_json(negative_path); entry = report["entries"][0]
        artifact = negative_root / entry["path"]; original_artifact = artifact.read_bytes()
        tool.write_json(artifact, {})
        entry["sha256"] = tool.sha256(artifact); tool.write_json(negative_path, report)
        try: revalidate()
        except tool.EvidenceError: pass
        else: raise RuntimeError("rehashed unrelated negative accepted")
        artifact.write_bytes(original_artifact); negative_path.write_bytes(saved)
        relocated_negatives = root / "relocated-negatives"
        shutil.copytree(negative_root, relocated_negatives)
        tool.validate_negative_outcomes(profile, relocated_negatives / negative_path.name, tool.sha256(first), validator_hash)
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
        for case_id, field in (("mat2_swap_application", "observed"), ("mat3_cycle_application", "observed"), ("scale_k_1", "output_fields"), ("mat2_transpose_composition", "observed")):
            mutated = copy.deepcopy(tool.read_json(first))
            row = next(item for item in mutated["cases"] if item["id"] == case_id)
            if field == "output_fields": row[field][-1] = "wrong.law"
            else: row[field]["value"][-1] = "0x1.1p+20"
            path = root / (case_id + "-negative.json"); tool.write_json(path, mutated)
            try: tool.validate_certificate(profile, path)
            except tool.EvidenceError: pass
            else: raise RuntimeError(f"forged field accepted: {case_id}")
        project = root / "dependency-project"
        for folder in ("include", "src"):
            shutil.copytree(pathlib.Path(arguments.source_root) / folder, project / folder)
        shutil.copyfile(pathlib.Path(arguments.source_root) / "CMakeLists.txt", project / "CMakeLists.txt")
        tool.validate_source_root(project)
        bridge = project / "include/apmesh/math/bridge.hpp"
        bridge.write_text('#include "../core/geometry.hpp"\n', encoding="utf-8")
        header = project / "include/apmesh/math/linear_algebra.hpp"
        header.write_text(header.read_text(encoding="utf-8") + '\n#include "bridge.hpp"\n', encoding="utf-8")
        try: tool.validate_source_root(project)
        except tool.EvidenceError: pass
        else: raise RuntimeError("transitive relative geometry dependency accepted")
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
        target["inputs"][8] = "0x0p+0"
        wrong_path = root / "wrong-placement.json"; wrong_path.write_text(json.dumps(wrong_placement), encoding="utf-8")
        try:
            tool.validate_certificate(profile, wrong_path)
        except tool.EvidenceError:
            return 0
        raise RuntimeError("forged non-finite placement was accepted")


if __name__ == "__main__":
    raise SystemExit(main())
