#!/usr/bin/env python3
"""Execute one pre-registered REC rejection fixture in disposable storage."""

from __future__ import annotations

import argparse
import pathlib
import subprocess
import sys
import tempfile

from experiment_runtime import RuntimeErrorEvidence, clean_candidate, inventory_files, run_command, verify_input_identity, write_state
from reproducible_experiment_evidence import EvidenceError, require_inventory_rows, validate_bundle, validate_profile
from run_reproducible_experiment_contract import require_empty_evidence_root


FIXTURES = {
    "N1": "dirty candidate is rejected", "N2": "changed declared input is rejected", "N3": "non-empty evidence root is rejected",
    "N4": "unsupported profile schema is rejected", "N5": "missing required artifact is rejected", "N6": "tampered artifact inventory is rejected",
    "N7": "failed command preserves explicit evidence", "N8": "terminal lifecycle state cannot be reused",
}


def expect_rejection(callback, expected: tuple[type[BaseException], ...]) -> None:
    try:
        callback()
    except expected:
        return
    raise RuntimeError("negative fixture was accepted")


def execute_fixture(fixture: str, profile_path: pathlib.Path) -> None:
    profile = validate_profile(profile_path)
    with tempfile.TemporaryDirectory() as temporary_directory:
        root = pathlib.Path(temporary_directory)
        if fixture == "N1":
            candidate = root / "candidate"; candidate.mkdir(); (candidate / "tracked.txt").write_text("x\n", encoding="utf-8")
            for command in (["git", "init"], ["git", "config", "user.email", "fixture@example.invalid"], ["git", "config", "user.name", "Fixture"], ["git", "add", "tracked.txt"], ["git", "commit", "-m", "fixture"]):
                subprocess.run(command, cwd=candidate, check=True, capture_output=True)
            (candidate / "dirty.txt").write_text("dirty\n", encoding="utf-8")
            expect_rejection(lambda: clean_candidate(candidate), (RuntimeErrorEvidence,))
        elif fixture == "N2":
            item = root / "input.txt"; item.write_text("one\n", encoding="utf-8")
            expected = {"input": {"path": str(item.resolve()), "sha256": "0" * 64}}
            expect_rejection(lambda: verify_input_identity(expected, {"input": item}), (RuntimeErrorEvidence,))
        elif fixture == "N3":
            evidence = root / "evidence"; evidence.mkdir(); (evidence / "old.txt").write_text("old\n", encoding="utf-8")
            expect_rejection(lambda: require_empty_evidence_root(evidence), (EvidenceError,))
        elif fixture == "N4":
            malformed = root / "profile.json"; malformed.write_text('{"schema_version":99}\n', encoding="utf-8")
            expect_rejection(lambda: validate_profile(malformed), (EvidenceError, RuntimeErrorEvidence))
        elif fixture == "N5":
            bundle = root / "bundle"; bundle.mkdir()
            expect_rejection(lambda: validate_bundle(bundle, profile), (EvidenceError, RuntimeErrorEvidence))
        elif fixture == "N6":
            artifact = root / "artifact.txt"; artifact.write_text("original\n", encoding="utf-8")
            expected = inventory_files(root, [("artifact.txt", "fixture", "plain-text/v1", None)])
            altered = [dict(expected[0])]; altered[0]["sha256"] = "0" * 64
            expect_rejection(lambda: require_inventory_rows(altered, expected), (EvidenceError,))
        elif fixture == "N7":
            record = run_command([sys.executable, "-c", "raise SystemExit(7)"], root, root / "logs", "negative", 10)
            if record["exit_code"] != 7 or record["launch_error"] is not None:
                raise RuntimeError("failed command was not recorded")
        elif fixture == "N8":
            control = root / "control"; control.mkdir(); write_state(control, "PREPARED", {}); write_state(control, "BLOCKED", {})
            expect_rejection(lambda: write_state(control, "PREPARED", {}), (RuntimeErrorEvidence,))
        else:
            raise RuntimeError("unknown negative fixture")


def main() -> int:
    parser = argparse.ArgumentParser(); parser.add_argument("--fixture", choices=sorted(FIXTURES), required=True); parser.add_argument("--profile", required=True)
    arguments = parser.parse_args()
    try:
        execute_fixture(arguments.fixture, pathlib.Path(arguments.profile))
        return 0
    except (RuntimeError, RuntimeErrorEvidence, EvidenceError) as error:
        print(error, file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
