#!/usr/bin/env python3
from __future__ import annotations

import argparse
import hashlib
import json
import pathlib
import subprocess
import sys
import tempfile


def verify(runner: str, package: pathlib.Path, expected: int) -> None:
    result = subprocess.run([sys.executable, runner, "verify-retention", "--package", str(package)], text=True, capture_output=True, check=False)
    if result.returncode != expected:
        raise RuntimeError(result.stderr)


def main() -> int:
    parser = argparse.ArgumentParser(); parser.add_argument("--runner", required=True); args = parser.parse_args()
    with tempfile.TemporaryDirectory() as directory:
        root = pathlib.Path(directory); artifact = root / "certificate.json"; artifact.write_text("{}\n", encoding="utf-8")
        package = root / "package.json"; package.write_text(json.dumps({"status": "sealed", "formal_manifest": None, "entries": [{"path": str(artifact), "sha256": hashlib.sha256(artifact.read_bytes()).hexdigest()}]}), encoding="utf-8")
        verify(args.runner, package, 0)
        artifact.write_text("mutated\n", encoding="utf-8")
        verify(args.runner, package, 1)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
