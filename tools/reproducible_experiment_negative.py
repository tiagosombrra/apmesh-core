#!/usr/bin/env python3
"""Execute one pre-registered REC rejection fixture in disposable storage."""

from __future__ import annotations

import argparse
import json
import pathlib
import subprocess
import sys
import tempfile

from experiment_runtime import RuntimeErrorEvidence, clean_candidate, inventory_files, run_command, verify_input_identity, write_state
from reproducible_experiment_evidence import EvidenceError, require_inventory_rows, validate_bundle, validate_profile
from run_reproducible_experiment_contract import require_empty_evidence_root


FIXTURES = {
    "N1": ("dirty-candidate", "candidate working tree is not clean"),
    "N2": ("input-identity-mismatch", "prepared input identity differs"),
    "N3": ("evidence-root-not-empty", "evidence root is not empty"),
    "N4": ("profile-schema-invalid", "REC profile"),
    "N5": ("required-artifact-absent", "required artifact is absent"),
    "N6": ("inventory-tampered", "artifact inventory differs"),
    "N7": ("failed-command-recorded-blocked", "failed command was not recorded"),
    "N8": ("terminal-lifecycle-reuse", "invalid lifecycle transition"),
}


def expect_rejection(callback, expected: tuple[type[BaseException], ...], message: str) -> None:
    try:
        callback()
    except expected as error:
        if message not in str(error):
            raise RuntimeError(f"negative fixture rejected for an unexpected reason: {error}") from error
        return
    raise RuntimeError("negative fixture was accepted")


def execute_fixture(fixture: str, profile_path: pathlib.Path) -> str:
    profile = validate_profile(profile_path)
    with tempfile.TemporaryDirectory() as temporary_directory:
        root = pathlib.Path(temporary_directory)
        if fixture == "N1":
            candidate = root / "candidate"; candidate.mkdir(); (candidate / "tracked.txt").write_text("x\n", encoding="utf-8")
            for command in (["git", "init"], ["git", "config", "user.email", "fixture@example.invalid"], ["git", "config", "user.name", "Fixture"], ["git", "add", "tracked.txt"], ["git", "commit", "-m", "fixture"]):
                subprocess.run(command, cwd=candidate, check=True, capture_output=True)
            (candidate / "dirty.txt").write_text("dirty\n", encoding="utf-8")
            expect_rejection(lambda: clean_candidate(candidate), (RuntimeErrorEvidence,), FIXTURES[fixture][1])
        elif fixture == "N2":
            item = root / "input.txt"; item.write_text("one\n", encoding="utf-8")
            expected = {"input": {"path": str(item.resolve()), "sha256": "0" * 64}}
            expect_rejection(lambda: verify_input_identity(expected, {"input": item}), (RuntimeErrorEvidence,), FIXTURES[fixture][1])
        elif fixture == "N3":
            evidence = root / "evidence"; evidence.mkdir(); (evidence / "old.txt").write_text("old\n", encoding="utf-8")
            expect_rejection(lambda: require_empty_evidence_root(evidence), (EvidenceError,), FIXTURES[fixture][1])
        elif fixture == "N4":
            malformed = root / "profile.json"; malformed.write_text('{"schema_version":99}\n', encoding="utf-8")
            expect_rejection(lambda: validate_profile(malformed), (EvidenceError, RuntimeErrorEvidence), FIXTURES[fixture][1])
        elif fixture == "N5":
            bundle = root / "bundle"; bundle.mkdir()
            expect_rejection(lambda: validate_bundle(bundle, profile), (EvidenceError, RuntimeErrorEvidence), FIXTURES[fixture][1])
        elif fixture == "N6":
            artifact = root / "artifact.txt"; artifact.write_text("original\n", encoding="utf-8")
            expected = inventory_files(root, [("artifact.txt", "fixture", "plain-text/v1", None)])
            altered = [dict(expected[0])]; altered[0]["sha256"] = "0" * 64
            expect_rejection(lambda: require_inventory_rows(altered, expected), (EvidenceError,), FIXTURES[fixture][1])
        elif fixture == "N7":
            record = run_command([sys.executable, "-c", "raise SystemExit(7)"], root, root / "logs", "negative", 10)
            if record["exit_code"] != 7 or record["launch_error"] is not None:
                raise RuntimeError(FIXTURES[fixture][1])
            control = root / "control"; control.mkdir()
            write_state(control, "PREPARED", {}); write_state(control, "RUNNING", {})
            write_state(control, "BLOCKED", {"reason": "simulated command failure", "record": record})
            state = json.loads((control / "state.json").read_text(encoding="utf-8"))
            history = (control / "state-history.jsonl").read_text(encoding="utf-8")
            if state.get("state") != "BLOCKED" or "simulated command failure" not in history:
                raise RuntimeError(FIXTURES[fixture][1])
        elif fixture == "N8":
            control = root / "control"; control.mkdir(); write_state(control, "PREPARED", {}); write_state(control, "BLOCKED", {})
            expect_rejection(lambda: write_state(control, "PREPARED", {}), (RuntimeErrorEvidence,), FIXTURES[fixture][1])
        else:
            raise RuntimeError("unknown negative fixture")
    return FIXTURES[fixture][0]


def main() -> int:
    parser = argparse.ArgumentParser(); parser.add_argument("--fixture", choices=sorted(FIXTURES), required=True); parser.add_argument("--profile", required=True)
    arguments = parser.parse_args()
    try:
        reason_code = execute_fixture(arguments.fixture, pathlib.Path(arguments.profile))
        print(json.dumps({"schema_version": 1, "kind": "reproducible-experiment-negative-result", "fixture": arguments.fixture,
                          "result": "REJECTED", "reason_code": reason_code}, sort_keys=True))
        return 0
    except (RuntimeError, RuntimeErrorEvidence, EvidenceError) as error:
        print(error, file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
