#!/usr/bin/env python3
"""Reject fabricated retained evidence without a campaign."""
import argparse
import pathlib
import tempfile
from minimal_small_linear_algebra_runner_test import load, rejects


def main():
    parser = argparse.ArgumentParser(); parser.add_argument("--runner", required=True)
    args = parser.parse_args(); runner = load(args.runner)
    with tempfile.TemporaryDirectory(prefix="apmesh-la-retention-negative-") as temporary:
        root = pathlib.Path(temporary)
        for path in runner.TERMINAL_REQUIRED_FAILURE:
            runner.write_json(root / path, {})
        rejects(lambda: runner.verify_retention(argparse.Namespace(output_root=str(root))), "empty fabricated package accepted")
    # The positive retained package is produced by a real failing child in the runner contract.
    print("PASS: fabricated terminal retention rejected")
    return 0


if __name__ == "__main__": raise SystemExit(main())
