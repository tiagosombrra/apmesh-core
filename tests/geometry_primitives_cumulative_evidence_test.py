#!/usr/bin/env python3

import argparse
import hashlib
import json
import pathlib
import subprocess
import sys
import tempfile


def run(command: list[str], expected: int = 0) -> None:
    completed = subprocess.run(command, capture_output=True, text=True, check=False)
    if completed.returncode != expected:
        raise RuntimeError(f"unexpected exit {completed.returncode}: {' '.join(command)}\n{completed.stderr}")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--exporter", required=True)
    parser.add_argument("--tool", required=True)
    parser.add_argument("--profile", required=True)
    arguments = parser.parse_args()
    common = [sys.executable, arguments.tool, "--profile", arguments.profile]
    with tempfile.TemporaryDirectory(prefix="apmesh-core-gpr-evidence-") as temporary:
        root = pathlib.Path(temporary)
        certificate = root / "certificate.json"
        run([arguments.exporter, "certificate", str(certificate)])
        run([*common, "validate-profile"])
        run([*common, "validate-certificate", "--certificate", str(certificate)])

        profile = json.loads(pathlib.Path(arguments.profile).read_text(encoding="utf-8"))
        forged_profile = dict(profile)
        forged_profile["limitations"] = list(profile["limitations"])
        forged_profile["limitations"][1] = "Report-only development workflow; no prepared manifest or formal execution"
        forged_profile_path = root / "stale-limitations-profile.json"
        forged_profile_path.write_text(json.dumps(forged_profile), encoding="utf-8")
        run([sys.executable, arguments.tool, "--profile", str(forged_profile_path), "validate-profile"], expected=1)
        entries = []
        for cell in profile["cells"]:
            for repetition in range(1, profile["repetitions_per_cell"] + 1):
                copied = root / f"{cell['id']}-{repetition}.json"
                copied.write_bytes(certificate.read_bytes())
                entries.append({
                    "cell": cell["id"], "repetition": repetition, "path": copied.name,
                    "sha256": hashlib.sha256(copied.read_bytes()).hexdigest(),
                })
        index = root / "certificate-index.json"
        index.write_text(json.dumps({
            "schema_version": 1,
            "kind": "geometry-primitives-cumulative-certificate-index",
            "entries": entries,
        }), encoding="utf-8")
        comparison = root / "comparison.json"
        run([*common, "compare", "--index", str(index), "--output", str(comparison)])
        value = json.loads(comparison.read_text(encoding="utf-8"))
        if value["status"] != "EVIDENCE_COLLECTED_PENDING_AUDIT" or set(value["gates"].values()) != {"NOT_EXECUTED"} or value["certificate_count"] != 12:
            raise RuntimeError("report-only comparison claims a gate result")
        collected = root / "collected.json"
        run([*common, "collect", "--index", str(index), "--output", str(collected)])
        if json.loads(collected.read_text(encoding="utf-8"))["status"] != "EVIDENCE_COLLECTED_PENDING_AUDIT":
            raise RuntimeError("collector claims scientific closure")

        outcomes = root / "negative-outcomes.json"
        run([*common, "negative-outcomes", "--output", str(outcomes)])
        run([*common, "validate-negative-outcomes", "--outcomes", str(outcomes)])
        forged_outcomes = json.loads(outcomes.read_text(encoding="utf-8"))
        forged_outcomes["outcomes"][0]["result"] = "ACCEPTED"
        forged_outcomes_path = root / "forged-negative-outcomes.json"
        forged_outcomes_path.write_text(json.dumps(forged_outcomes), encoding="utf-8")
        run([*common, "validate-negative-outcomes", "--outcomes", str(forged_outcomes_path)], expected=1)

        forged = json.loads(certificate.read_text(encoding="utf-8"))
        next(item for item in forged["cases"] if item["id"] == "mat2_compose_apply")["observed"]["values"][0] = "0x1p+20"
        forged_path = root / "forged-integrated-result.json"
        forged_path.write_text(json.dumps(forged), encoding="utf-8")
        run([*common, "validate-certificate", "--certificate", str(forged_path)], expected=1)
        forged = json.loads(certificate.read_text(encoding="utf-8"))
        forged["cases"].append(forged["cases"][0])
        forged_path = root / "duplicate-case.json"
        forged_path.write_text(json.dumps(forged), encoding="utf-8")
        run([*common, "validate-certificate", "--certificate", str(forged_path)], expected=1)
        forged = json.loads(certificate.read_text(encoding="utf-8"))
        forged["non_claims"] = ["orientation", "predicate", "coincidence", "incidence", "topology", "meshing"]
        forged_path = root / "forged-non-claim.json"
        forged_path.write_text(json.dumps(forged), encoding="utf-8")
        run([*common, "validate-certificate", "--certificate", str(forged_path)], expected=1)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
