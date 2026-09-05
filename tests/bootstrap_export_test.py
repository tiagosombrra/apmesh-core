#!/usr/bin/env python3
import argparse
import pathlib
import subprocess
import tempfile


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--exporter", required=True)
    parser.add_argument("--expected", required=True)
    arguments = parser.parse_args()

    expected = pathlib.Path(arguments.expected).read_bytes()
    with tempfile.TemporaryDirectory() as temporary_directory:
        output = pathlib.Path(temporary_directory) / "certificate.json"
        completed = subprocess.run(
            [arguments.exporter, str(output)],
            check=False,
            capture_output=True,
        )
        if completed.returncode != 0:
            return completed.returncode
        if completed.stdout or completed.stderr or output.read_bytes() != expected:
            return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
