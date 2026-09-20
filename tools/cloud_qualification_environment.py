#!/usr/bin/env python3
"""Validate the bounded GitHub-hosted cloud qualification environment.

This tool validates environment identity only. It makes no scientific claim
and does not run qualification campaigns.
"""

from __future__ import annotations

import argparse
import json
import os
import pathlib
import platform
import re
import subprocess
import sys
from typing import Any


class EnvironmentValidationError(RuntimeError):
    pass


def load_json(path: pathlib.Path) -> dict[str, Any]:
    with path.open("r", encoding="utf-8") as stream:
        value = json.load(stream)
    if not isinstance(value, dict):
        raise EnvironmentValidationError(f"JSON object required: {path}")
    return value


def command_stdout(argv: list[str]) -> str:
    completed = subprocess.run(argv, capture_output=True, text=True, check=False)
    if completed.returncode != 0:
        raise EnvironmentValidationError(
            f"command failed ({completed.returncode}): {' '.join(argv)}\n"
            f"{completed.stderr.strip()}"
        )
    return completed.stdout.strip()


def package_version(name: str) -> str:
    return command_stdout(["/usr/bin/dpkg-query", "-W", "-f=${Version}", name])


def os_release() -> dict[str, str]:
    result: dict[str, str] = {}
    for raw in pathlib.Path("/etc/os-release").read_text(encoding="utf-8").splitlines():
        if not raw or raw.startswith("#") or "=" not in raw:
            continue
        key, value = raw.split("=", 1)
        result[key] = value.strip().strip('"')
    return result


def tool_version(path: str, kind: str) -> str:
    first = command_stdout([path, "--version"]).splitlines()[0]
    if kind == "cmake":
        match = re.search(r"cmake version ([0-9.]+)", first)
    elif kind == "ninja":
        match = re.fullmatch(r"([0-9.]+)", first)
    elif kind == "gcc":
        match = re.search(r"\) ([0-9]+\.[0-9]+\.[0-9]+)$", first)
    elif kind == "clang":
        match = re.search(r"clang version ([0-9]+\.[0-9]+\.[0-9]+)", first)
    else:
        raise EnvironmentValidationError(f"unknown tool kind: {kind}")
    if match is None:
        raise EnvironmentValidationError(f"cannot parse {kind} version from: {first}")
    return match.group(1)


def expect(observed: Any, expected: Any, field: str, failures: list[str]) -> None:
    if observed != expected:
        failures.append(f"{field}: expected {expected!r}, observed {observed!r}")


def validate(profile: dict[str, Any], cell: str) -> dict[str, Any]:
    if profile.get("schema_version") != 1 or profile.get("kind") != "cloud-qualification-environment-profile":
        raise EnvironmentValidationError("unsupported environment profile")

    cells = {entry["id"] for entry in profile.get("cells", [])}
    if cell not in cells:
        raise EnvironmentValidationError(f"undeclared cell: {cell}")

    runner = profile["runner"]
    tools = profile["tools"]
    release = os_release()
    failures: list[str] = []

    observed_runner = {
        "label": "ubuntu-24.04",
        "image_os": os.environ.get("ImageOS"),
        "image_version": os.environ.get("ImageVersion"),
        "os_id": release.get("ID"),
        "os_version_id": release.get("VERSION_ID"),
        "os_pretty_name": release.get("PRETTY_NAME"),
        "architecture": platform.machine(),
        "kernel": platform.release(),
    }

    for key in ("label", "image_os", "image_version", "os_id", "os_version_id", "architecture"):
        expect(observed_runner.get(key), runner.get(key), f"runner.{key}", failures)

    observed_tools: dict[str, Any] = {}
    for kind in ("cmake", "ninja", "gcc", "clang"):
        spec = tools[kind]
        path = spec["path"]
        if not pathlib.Path(path).is_file():
            failures.append(f"tools.{kind}.path: missing {path}")
            continue
        observed_tools[kind] = {
            "path": str(pathlib.Path(path).resolve()),
            "version": tool_version(path, kind),
            "package": spec["package"],
            "package_version": package_version(spec["package"]),
        }
        expect(observed_tools[kind]["version"], spec["version"], f"tools.{kind}.version", failures)
        expect(
            observed_tools[kind]["package_version"],
            spec["package_version"],
            f"tools.{kind}.package_version",
            failures,
        )

    for kind in ("libcxx_dev", "libcxxabi_dev"):
        spec = tools[kind]
        observed_tools[kind] = {
            "package": spec["package"],
            "package_version": package_version(spec["package"]),
        }
        expect(
            observed_tools[kind]["package_version"],
            spec["package_version"],
            f"tools.{kind}.package_version",
            failures,
        )

    return {
        "schema_version": 1,
        "kind": "cloud-qualification-environment-observation",
        "cell": cell,
        "status": "PASS" if not failures else "BLOCKED",
        "runner": observed_runner,
        "tools": observed_tools,
        "historical_wsl_reference": profile["historical_wsl_reference"],
        "limitations": profile["limitations"],
        "failures": failures,
    }


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--profile", type=pathlib.Path, required=True)
    parser.add_argument("--cell", required=True)
    parser.add_argument("--output", type=pathlib.Path, required=True)
    args = parser.parse_args()

    try:
        profile = load_json(args.profile)
        result = validate(profile, args.cell)
        args.output.parent.mkdir(parents=True, exist_ok=True)
        args.output.write_text(
            json.dumps(result, sort_keys=True, indent=2) + "\n",
            encoding="utf-8",
        )
        if result["status"] != "PASS":
            for failure in result["failures"]:
                print(f"BLOCKED: {failure}", file=sys.stderr)
            return 1
        print(f"PASS: cloud qualification environment identity for {args.cell}")
        return 0
    except (OSError, KeyError, ValueError, EnvironmentValidationError) as error:
        print(f"BLOCKED: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
