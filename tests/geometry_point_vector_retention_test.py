#!/usr/bin/env python3

import argparse
import pathlib
import subprocess
import sys
import tempfile


def run(command: list[str], cwd: pathlib.Path) -> None:
    subprocess.run(command, cwd=cwd, check=True, capture_output=True)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--runner", required=True)
    parser.add_argument("--runtime", required=True)
    arguments = parser.parse_args()
    sys.path.insert(0, str(pathlib.Path(arguments.runner).resolve().parent))
    import run_geometry_point_vector_qualification as runner
    from experiment_runtime import clean_candidate, sha256_file, write_json, write_state

    with tempfile.TemporaryDirectory() as temporary_directory:
        root = pathlib.Path(temporary_directory)
        source = root / "candidate"; source.mkdir()
        (source / "tracked.txt").write_text("candidate\n", encoding="utf-8")
        run(["git", "init"], source)
        run(["git", "config", "user.email", "test@example.invalid"], source)
        run(["git", "config", "user.name", "Test"], source)
        run(["git", "add", "tracked.txt"], source)
        run(["git", "commit", "-m", "candidate"], source)
        candidate = clean_candidate(source)
        candidate["upstream_commit"] = candidate["commit"]
        output = root / "retained"
        output.mkdir()
        prepared = {"kind": "geometry-point-vector-prepared-manifest", "candidate": candidate}
        write_json(output / "prepared-manifest.json", prepared)
        write_state(output, "PREPARED", {"candidate_commit": candidate["commit"]})
        write_state(output, "RUNNING", {"candidate_commit": candidate["commit"]})
        terminal = {
            "kind": "geometry-point-vector-terminal-manifest",
            "candidate": candidate,
            "prepared_manifest_sha256": sha256_file(output / "prepared-manifest.json"),
            "retention_manifest": "retention-manifest.json",
        }
        write_json(output / "terminal-manifest.json", terminal)
        write_state(output, "EXECUTED_PENDING_AUDIT", {"candidate_commit": candidate["commit"]})
        runner.seal_output(output, source, prepared)
        runner.verify_retention(output, source)
        (output / "unexpected.txt").write_text("tamper\n", encoding="utf-8")
        try:
            runner.verify_retention(output, source)
        except runner.RuntimeErrorEvidence:
            return 0
        raise RuntimeError("retention verifier accepted an unsealed artifact")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
