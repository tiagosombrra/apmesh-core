#!/usr/bin/env python3

import argparse
import hashlib
import importlib.util
import json
import pathlib
import subprocess
import sys
import tempfile


def run(command: list[str], expected: int = 0) -> None:
    completed = subprocess.run(command, capture_output=True, text=True, check=False)
    if completed.returncode != expected:
        raise RuntimeError(f"unexpected exit {completed.returncode}: {' '.join(command)}\n{completed.stderr}")


def rejects(action, message: str) -> None:
    try:
        action()
    except Exception:
        return
    raise RuntimeError(message)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--exporter", required=True)
    parser.add_argument("--tool", required=True)
    parser.add_argument("--profile", required=True)
    parser.add_argument("--source-root", required=True)
    arguments = parser.parse_args()
    specification = importlib.util.spec_from_file_location("cartesian_frames_evidence", arguments.tool)
    if specification is None or specification.loader is None:
        raise RuntimeError("evidence module could not be loaded")
    evidence = importlib.util.module_from_spec(specification)
    specification.loader.exec_module(evidence)

    with tempfile.TemporaryDirectory(prefix="apmesh-core-cf-evidence-") as temporary:
        root = pathlib.Path(temporary)
        certificate = root / "certificate.json"
        source_check = root / "source-check.json"
        run([arguments.exporter, "certificate", str(certificate)])
        common = [sys.executable, arguments.tool, "--profile", arguments.profile]
        run([*common, "validate-source", "--source-root", arguments.source_root, "--output", str(source_check)])
        run([*common, "validate-certificate", "--certificate", str(certificate)])
        profile = json.loads(pathlib.Path(arguments.profile).read_text(encoding="utf-8"))
        entries = []
        for cell in profile["cells"]:
            for repetition in range(1, profile["repetitions_per_cell"] + 1):
                copied = root / f"{cell['id']}-{repetition}.json"
                copied.write_bytes(certificate.read_bytes())
                entries.append({"cell": cell["id"], "repetition": repetition, "path": copied.name, "sha256": hashlib.sha256(copied.read_bytes()).hexdigest()})
        index = root / "certificate-index.json"
        index.write_text(json.dumps({"schema_version": 1, "kind": "cartesian-frames-certificate-index", "entries": entries}), encoding="utf-8")
        report = root / "report.md"
        run([*common, "compare", "--index", str(index), "--report", str(report)])
        if "Qualification: EVIDENCE_COLLECTED_PENDING_AUDIT" not in report.read_text(encoding="utf-8"):
            raise RuntimeError("report-only comparer claims scientific closure")
        per_cell = root / "per-cell-comparisons.json"
        per_cell.write_text(json.dumps(evidence.compare_per_cell_certificates(profile, index)), encoding="utf-8")
        evidence.validate_per_cell_comparisons(profile, index, per_cell)
        per_cell_value = json.loads(per_cell.read_text(encoding="utf-8"))
        if [item["cell"] for item in per_cell_value["cells"]] != [cell["id"] for cell in profile["cells"]]:
            raise RuntimeError("per-cell comparison coverage differs")
        per_cell_value["cells"][0]["status"] = "FORGED"
        per_cell.write_text(json.dumps(per_cell_value), encoding="utf-8")
        rejects(lambda: evidence.validate_per_cell_comparisons(profile, index, per_cell), "forged per-cell comparison was accepted")
        summary = evidence.gate_summary(profile, profile["limitations"])
        summary_markdown = root / "gate-summary.md"
        summary_markdown.write_text(evidence.render_gate_summary_markdown(summary), encoding="utf-8")
        evidence.validate_gate_summary_markdown(summary, summary_markdown)
        summary_markdown.write_text("forged summary\n", encoding="utf-8")
        rejects(lambda: evidence.validate_gate_summary_markdown(summary, summary_markdown), "forged Markdown gate summary was accepted")
        collected = root / "collected.json"
        run([*common, "collect", "--index", str(index), "--output", str(collected)])
        if json.loads(collected.read_text(encoding="utf-8"))["status"] != "EVIDENCE_COLLECTED_PENDING_AUDIT":
            raise RuntimeError("collector claims a scientific decision")
        negatives = root / "negative-outcomes.json"
        run([*common, "negative-outcomes", "--certificate", str(certificate), "--output", str(negatives)])
        outcomes = json.loads(negatives.read_text(encoding="utf-8"))["outcomes"]
        if outcomes != [{"id": item, "result": "REJECTED"} for item in profile["certificate_negative_cases"]]:
            raise RuntimeError("negative outcome evidence differs")
        run([*common, "validate-negative-outcomes", "--outcomes", str(negatives)])
        forged_outcomes = json.loads(negatives.read_text(encoding="utf-8"))
        forged_outcomes["outcomes"][0]["result"] = "ACCEPTED"
        forged_outcomes_path = root / "forged-negative-outcomes.json"
        forged_outcomes_path.write_text(json.dumps(forged_outcomes), encoding="utf-8")
        run([*common, "validate-negative-outcomes", "--outcomes", str(forged_outcomes_path)], expected=1)

        mutated = json.loads(certificate.read_text(encoding="utf-8"))
        next(item for item in mutated["cases"] if item["id"] == "frame2_duplicate_basis")["inputs"]["basis"][0] = "0x0p+0"
        forged = root / "forged-adversarial-input.json"
        forged.write_text(json.dumps(mutated), encoding="utf-8")
        run([*common, "validate-certificate", "--certificate", str(forged)], expected=1)
        forged = json.loads(certificate.read_text(encoding="utf-8"))
        forged["cases"][0]["expected"]["value"][0] = "0x1p+100"
        forged_path = root / "forged.json"
        forged_path.write_text(json.dumps(forged), encoding="utf-8")
        run([*common, "validate-certificate", "--certificate", str(forged_path)], expected=1)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
