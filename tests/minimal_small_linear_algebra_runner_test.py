#!/usr/bin/env python3
"""Focused no-manifest runner check for LA qualification infrastructure."""

from __future__ import annotations

import argparse
import subprocess
import sys


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--runner", required=True)
    parser.add_argument("--source-root", required=True)
    parser.add_argument("--profile", required=True)
    parser.add_argument("--protocol", required=True)
    arguments = parser.parse_args()
    completed = subprocess.run([
        sys.executable, arguments.runner, "--source-root", arguments.source_root,
        "--profile", arguments.profile, "--protocol", arguments.protocol, "self-check",
    ], capture_output=True, text=True, check=False)
    if completed.returncode != 0:
        raise RuntimeError(f"runner self-check failed: {completed.stderr}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
