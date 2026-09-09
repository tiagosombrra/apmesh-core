#!/usr/bin/env python3

import argparse
import importlib.util
import json
import pathlib
import subprocess
import sys
import tempfile


def run(command: list[str], cwd: pathlib.Path) -> None:
    subprocess.run(command, cwd=cwd, check=True, capture_output=True)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--runner", required=True)
    parser.add_argument("--profile", required=True)
    parser.add_argument("--protocol", required=True)
    parser.add_argument("--exporter", required=True)
    parser.add_argument("--validator", required=True)
    arguments = parser.parse_args()

    with tempfile.TemporaryDirectory() as temporary_directory:
        root = pathlib.Path(temporary_directory)
        source = root / "candidate"
        source.mkdir()
        required = {
            "CMakeLists.txt": "cmake_minimum_required(VERSION 3.25)\n",
            "include/apmesh/core/geometry.hpp": "#pragma once\n",
            "src/core/geometry.cpp": "namespace apmesh::core {}\n",
            "tests/geometry_primitives.cpp": "int main() { return 0; }\n",
        }
        for relative, content in required.items():
            path = source / relative
            path.parent.mkdir(parents=True, exist_ok=True)
            path.write_text(content, encoding="utf-8")
        run(["git", "init"], source)
        run(["git", "config", "user.email", "test@example.invalid"], source)
        run(["git", "config", "user.name", "Test"], source)
        run(["git", "add", "."], source)
        run(["git", "commit", "-m", "candidate"], source)
        run(["git", "branch", "-M", "candidate"], source)
        remote = root / "remote.git"
        run(["git", "init", "--bare", str(remote)], root)
        run(["git", "remote", "add", "origin", str(remote)], source)
        run(["git", "push", "-u", "origin", "candidate"], source)

        common = [
            "--source-root", str(source), "--profile", arguments.profile,
            "--protocol", arguments.protocol, "--exporter", arguments.exporter,
            "--validator", arguments.validator,
        ]
        occupied = root / "occupied"
        occupied.mkdir()
        rejected = subprocess.run([sys.executable, arguments.runner, *common, "--output-root", str(occupied)], check=False, capture_output=True)
        if rejected.returncode != 1 or b"output root already exists" not in rejected.stderr:
            raise RuntimeError("occupied output root was not rejected")

        prepared = root / "prepared"
        completed = subprocess.run([sys.executable, arguments.runner, *common, "--output-root", str(prepared)], check=False, capture_output=True)
        if completed.returncode != 0:
            raise RuntimeError(f"PREPARED manifest creation failed: {completed.stderr!r}")
        manifest = json.loads((prepared / "manifest.json").read_text(encoding="utf-8"))
        if manifest["state"] != "PREPARED" or manifest["execution_requested"] is not False or len(manifest["plan"]) != 4:
            raise RuntimeError("launcher did not produce a four-cell PREPARED-only manifest")
        if (prepared / "state.json").read_text(encoding="utf-8").find('"PREPARED"') < 0:
            raise RuntimeError("PREPARED lifecycle state is absent")

        (source / "dirty.txt").write_text("dirty\n", encoding="utf-8")
        blocked = subprocess.run([sys.executable, arguments.runner, *common, "--output-root", str(prepared), "--execute"], check=False, capture_output=True)
        if blocked.returncode != 1 or b"candidate working tree is not clean" not in blocked.stderr:
            raise RuntimeError("launcher did not block changed candidate before execution")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
