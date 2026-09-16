#!/usr/bin/env python3
"""Launch a sealed Cartesian Frames manifest through the pinned WSL environment."""
from __future__ import annotations

import argparse
import hashlib
import json
import pathlib
import shutil
import subprocess
import sys

DISTRO = "Ubuntu-24.04"
RUNNER = "tools/run_cartesian_frames_qualification.py"


def require(condition: bool, message: str) -> None:
    if not condition:
        raise RuntimeError(message)


def sha256_file(path: pathlib.Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def run_wsl(*argv: str) -> subprocess.CompletedProcess[str]:
    command = ["wsl.exe", "-d", DISTRO, "--exec", *argv]
    return subprocess.run(command, check=True, capture_output=True, text=True)


def wsl_path(path: pathlib.Path) -> str:
    return run_wsl("wslpath", "-a", str(path)).stdout.strip()


def read_json(path: pathlib.Path) -> dict:
    return json.loads(path.read_text(encoding="utf-8"))


def validate_manifest(manifest_path: pathlib.Path, expected_sha256: str,
                      expected_candidate: str) -> tuple[dict, dict, list[str]]:
    require(shutil.which("wsl.exe") is not None, "wsl.exe is unavailable")
    require(manifest_path.is_file(), "manifest is absent")
    actual_sha256 = sha256_file(manifest_path)
    require(actual_sha256 == expected_sha256.lower(), "manifest SHA-256 differs")
    manifest = read_json(manifest_path)
    require(manifest["state"] == "PREPARED" and manifest["execution_requested"] is False,
            "manifest is not unused PREPARED evidence")
    require(manifest["candidate"]["commit"] == expected_candidate,
            "manifest candidate differs")
    control = manifest_path.parent.resolve()
    require(wsl_path(control) == manifest["control_root"], "control root differs")
    source = manifest["working_directory"]
    require(pathlib.PurePosixPath(source, RUNNER).as_posix() == source + "/" + RUNNER,
            "noncanonical runner path")
    head = run_wsl("git", "-C", source, "rev-parse", "HEAD").stdout.strip()
    upstream = run_wsl("git", "-C", source, "rev-parse", "@{upstream}").stdout.strip()
    require(head == expected_candidate and upstream == expected_candidate,
            "candidate checkout differs")
    require(not run_wsl("git", "-C", source, "status", "--porcelain=v1").stdout.strip(),
            "candidate checkout is dirty")
    validation_code = (
        "import pathlib,sys; "
        "sys.path.insert(0,str(pathlib.Path(sys.argv[1])/'tools')); "
        "import run_cartesian_frames_qualification as r; "
        "r.validate_prepared(pathlib.Path(sys.argv[2])); print('PREPARED_VALID')"
    )
    prepared = run_wsl("python3", "-c", validation_code, source, manifest["control_root"])
    require(prepared.stdout.strip() == "PREPARED_VALID", "manifest validation differs")
    environment_code = (
        "import json,pathlib,platform; "
        "values=dict(line.split('=',1) for line in pathlib.Path('/etc/os-release').read_text().splitlines() "
        "if '=' in line); "
        "print(json.dumps({'id':values.get('ID'),'version_id':values.get('VERSION_ID','').strip(chr(34)),"
        "'platform':platform.platform()}))"
    )
    environment = json.loads(run_wsl("python3", "-c", environment_code).stdout)
    require(environment.get("id") == "ubuntu" and environment.get("version_id") == "24.04",
            "WSL distribution differs")
    command = ["wsl.exe", "-d", DISTRO, "--exec", "python3",
               str(pathlib.PurePosixPath(source, RUNNER)), "execute",
               "--output-root", manifest["control_root"]]
    return manifest, environment, command


def main() -> int:
    parser = argparse.ArgumentParser(allow_abbrev=False)
    parser.add_argument("--manifest", required=True)
    parser.add_argument("--expected-manifest-sha256", required=True)
    parser.add_argument("--expected-candidate", required=True)
    parser.add_argument("--execute", action="store_true")
    args = parser.parse_args()
    try:
        manifest_path = pathlib.Path(args.manifest).resolve()
        manifest, environment, command = validate_manifest(
            manifest_path, args.expected_manifest_sha256, args.expected_candidate)
        record = {
            "status": "PREPARED_VERIFIED",
            "manifest_sha256": args.expected_manifest_sha256.lower(),
            "candidate_commit": manifest["candidate"]["commit"],
            "execution_requested": args.execute,
            "environment": environment,
            "command": command,
        }
        if not args.execute:
            print(json.dumps(record, sort_keys=True))
            return 0
        completed = subprocess.run(command, check=False)
        record["exit_code"] = completed.returncode
        record["status"] = "EXECUTION_FINISHED" if completed.returncode == 0 else "EXECUTION_FAILED"
        print(json.dumps(record, sort_keys=True))
        return completed.returncode
    except Exception as error:
        print(json.dumps({"status": "PREFLIGHT_FAILED", "error": str(error)}), file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
