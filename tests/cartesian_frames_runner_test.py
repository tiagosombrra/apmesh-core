#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import pathlib
import subprocess
import sys
import tempfile


def invoke(args: argparse.Namespace, command: str, output: pathlib.Path) -> None:
    result = subprocess.run([sys.executable, args.runner, command, "--source-root", args.source_root, "--profile", args.profile, "--protocol", args.protocol, "--validator", args.validator, "--exporter", args.exporter, "--output", str(output)], text=True, capture_output=True, check=False)
    if result.returncode:
        raise RuntimeError(result.stderr)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--runner", required=True); parser.add_argument("--source-root", required=True); parser.add_argument("--profile", required=True); parser.add_argument("--protocol", required=True); parser.add_argument("--validator", required=True); parser.add_argument("--exporter", required=True)
    args = parser.parse_args()
    with tempfile.TemporaryDirectory() as directory:
        root = pathlib.Path(directory); check = root / "check.json"
        invoke(args, "self-check", check)
        record = json.loads(check.read_text(encoding="utf-8"))
        if record.get("status") != "pass" or record.get("formal_manifest") is not None or record.get("execution_requested") is not False:
            raise RuntimeError("self-check violated the report-only boundary")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
