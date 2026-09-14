#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import pathlib
import subprocess
import sys
import tempfile


def run(argv: list[str]) -> None:
    result = subprocess.run(argv, text=True, capture_output=True, check=False)
    if result.returncode:
        raise RuntimeError(result.stderr)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--exporter", required=True)
    parser.add_argument("--profile", required=True)
    parser.add_argument("--tool", required=True)
    args = parser.parse_args()
    with tempfile.TemporaryDirectory() as directory:
        root = pathlib.Path(directory); certificate = root / "certificate.json"; negatives = root / "negatives.json"; index = root / "index.json"; report = root / "report.json"
        run([args.exporter, str(certificate)])
        run([sys.executable, args.tool, "validate-profile", "--profile", args.profile])
        run([sys.executable, args.tool, "validate-certificate", "--profile", args.profile, "--certificate", str(certificate)])
        run([sys.executable, args.tool, "negative-self-check", "--profile", args.profile, "--certificate", str(certificate), "--output", str(negatives)])
        index.write_text(json.dumps({"certificates": [str(certificate), str(certificate)]}), encoding="utf-8")
        run([sys.executable, args.tool, "compare", "--profile", args.profile, "--index", str(index), "--report", str(report)])
        if json.loads(report.read_text(encoding="utf-8")).get("equivalent") is not True:
            raise RuntimeError("comparison did not record equivalence")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
