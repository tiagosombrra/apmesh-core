#!/usr/bin/env python3
"""Run each REC negative fixture independently without a formal campaign."""

from __future__ import annotations

import argparse
import pathlib
import subprocess
import sys


def main() -> int:
    parser = argparse.ArgumentParser(); parser.add_argument("--tool", required=True); parser.add_argument("--profile", required=True)
    arguments = parser.parse_args()
    for fixture in (f"N{number}" for number in range(1, 9)):
        completed = subprocess.run([sys.executable, arguments.tool, "--fixture", fixture, "--profile", arguments.profile], capture_output=True, text=True, check=False)
        if completed.returncode != 0:
            raise RuntimeError(f"negative fixture {fixture} failed: {completed.stderr}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
