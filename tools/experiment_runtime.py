#!/usr/bin/env python3
"""Small, strict primitives shared by experiment-contract tooling.

This module deliberately contains no scientific oracle.  It preserves bytes,
identities and process evidence so contract-specific tools can evaluate their
own claims without inventing a second workflow framework.
"""

from __future__ import annotations

import datetime as dt
import hashlib
import json
import os
import pathlib
import shutil
import subprocess
import time
from typing import Any, Iterable


class RuntimeErrorEvidence(RuntimeError):
    """Raised when reproducibility evidence is malformed or differs."""


def utc_now() -> str:
    return dt.datetime.now(dt.timezone.utc).isoformat(timespec="seconds")


def read_bytes(path: pathlib.Path) -> bytes:
    try:
        return path.read_bytes()
    except OSError as error:
        raise RuntimeErrorEvidence(f"cannot read {path}: {error}") from error


def sha256_file(path: pathlib.Path) -> str:
    return hashlib.sha256(read_bytes(path)).hexdigest()


def _reject_duplicate_keys(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    value: dict[str, Any] = {}
    for key, item in pairs:
        if key in value:
            raise RuntimeErrorEvidence(f"duplicate JSON key: {key}")
        value[key] = item
    return value


def read_json(path: pathlib.Path) -> dict[str, Any]:
    raw = read_bytes(path)
    if raw.startswith(b"\xef\xbb\xbf"):
        raise RuntimeErrorEvidence(f"UTF-8 BOM is not accepted: {path}")
    try:
        value = json.loads(raw.decode("utf-8"), object_pairs_hook=_reject_duplicate_keys)
    except (UnicodeDecodeError, json.JSONDecodeError) as error:
        raise RuntimeErrorEvidence(f"invalid JSON: {path}: {error}") from error
    if not isinstance(value, dict):
        raise RuntimeErrorEvidence(f"JSON object required: {path}")
    return value


def canonical_json(value: Any) -> bytes:
    return (json.dumps(value, sort_keys=True, separators=(",", ":"), ensure_ascii=True) + "\n").encode("utf-8")


def write_json(path: pathlib.Path, value: Any) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes(canonical_json(value))


def require_exact_keys(value: dict[str, Any], expected: Iterable[str], context: str) -> None:
    expected_set = set(expected)
    if set(value) != expected_set:
        missing, extra = sorted(expected_set - set(value)), sorted(set(value) - expected_set)
        raise RuntimeErrorEvidence(f"{context} keys differ; missing={missing}, extra={extra}")


def relative_path(root: pathlib.Path, path: pathlib.Path) -> str:
    try:
        return path.resolve().relative_to(root.resolve()).as_posix()
    except ValueError as error:
        raise RuntimeErrorEvidence(f"path escapes root: {path}") from error


def clean_candidate(root: pathlib.Path) -> dict[str, Any]:
    status = subprocess.run(["git", "status", "--porcelain"], cwd=root, capture_output=True, text=True, check=False)
    if status.returncode != 0 or status.stdout:
        raise RuntimeErrorEvidence("candidate working tree is not clean")
    revision = subprocess.run(["git", "rev-parse", "HEAD"], cwd=root, capture_output=True, text=True, check=True).stdout.strip()
    listed = subprocess.run(["git", "ls-files"], cwd=root, capture_output=True, text=True, check=True).stdout.splitlines()
    return {
        "commit": revision,
        "tree_clean": True,
        "source_root": str(root.resolve()),
        "source_inventory": [{"path": item, "sha256": sha256_file(root / item)} for item in listed],
    }


def tool_version(command: str) -> dict[str, str]:
    resolved = shutil.which(command)
    if resolved is None:
        raise RuntimeErrorEvidence(f"required tool is unavailable: {command}")
    completed = subprocess.run([resolved, "--version"], capture_output=True, text=True, check=False)
    if completed.returncode != 0:
        raise RuntimeErrorEvidence(f"cannot query tool version: {resolved}")
    output = (completed.stdout or completed.stderr).splitlines()
    if not output:
        raise RuntimeErrorEvidence(f"empty tool version: {resolved}")
    return {"path": str(pathlib.Path(resolved).resolve()), "version": output[0]}


def input_identity(paths: dict[str, pathlib.Path]) -> dict[str, dict[str, str]]:
    result: dict[str, dict[str, str]] = {}
    for name, path in paths.items():
        if not path.is_file():
            raise RuntimeErrorEvidence(f"required input is absent: {path}")
        result[name] = {"path": str(path.resolve()), "sha256": sha256_file(path)}
    return result


def verify_input_identity(expected: dict[str, dict[str, str]], paths: dict[str, pathlib.Path]) -> None:
    observed = input_identity(paths)
    if observed != expected:
        raise RuntimeErrorEvidence("prepared input identity differs")


def run_command(argv: list[str], cwd: pathlib.Path, logs_root: pathlib.Path, record_id: str,
                timeout_seconds: int, environment_delta: dict[str, str] | None = None) -> dict[str, Any]:
    """Run exactly one declared command and preserve independent stdout/stderr."""
    if not argv or any(not isinstance(part, str) or not part for part in argv):
        raise RuntimeErrorEvidence("command argv must contain non-empty strings")
    if timeout_seconds <= 0:
        raise RuntimeErrorEvidence("command timeout must be positive")
    logs_root.mkdir(parents=True, exist_ok=True)
    stdout_path, stderr_path = logs_root / f"{record_id}.stdout.log", logs_root / f"{record_id}.stderr.log"
    environment = os.environ.copy()
    environment.update(environment_delta or {})
    started = dt.datetime.now(dt.timezone.utc)
    try:
        process = subprocess.Popen(argv, cwd=cwd, env=environment, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    except OSError as error:
        stderr_path.write_text(str(error), encoding="utf-8")
        stdout_path.write_bytes(b"")
        return {
            "schema_version": 1, "kind": "experiment-command-record", "id": record_id, "argv": argv,
            "cwd": str(cwd.resolve()), "environment_delta": environment_delta or {}, "started_utc": utc_now(),
            "ended_utc": utc_now(), "elapsed_seconds": 0.0, "pid": None, "timeout_seconds": timeout_seconds,
            "timed_out": False, "exit_code": None, "launch_error": str(error),
            "stdout": {"path": relative_path(logs_root.parent, stdout_path), "sha256": sha256_file(stdout_path)},
            "stderr": {"path": relative_path(logs_root.parent, stderr_path), "sha256": sha256_file(stderr_path)},
        }
    pid = process.pid
    try:
        stdout, stderr = process.communicate(timeout=timeout_seconds)
        stdout_path.write_bytes(stdout)
        stderr_path.write_bytes(stderr)
        exit_code: int | None = process.returncode
        timed_out = False
    except subprocess.TimeoutExpired:
        process.kill()
        stdout, stderr = process.communicate()
        stdout_path.write_bytes(stdout)
        stderr_path.write_bytes(stderr)
        exit_code, timed_out = None, True
    ended = dt.datetime.now(dt.timezone.utc)
    return {
        "schema_version": 1,
        "kind": "experiment-command-record",
        "id": record_id,
        "argv": argv,
        "cwd": str(cwd.resolve()),
        "environment_delta": environment_delta or {},
        "started_utc": started.isoformat(timespec="seconds"),
        "ended_utc": ended.isoformat(timespec="seconds"),
        "elapsed_seconds": round((ended - started).total_seconds(), 6),
        "pid": pid,
        "timeout_seconds": timeout_seconds,
        "timed_out": timed_out,
        "exit_code": exit_code,
        "launch_error": None,
        "stdout": {"path": relative_path(logs_root.parent, stdout_path), "sha256": sha256_file(stdout_path)},
        "stderr": {"path": relative_path(logs_root.parent, stderr_path), "sha256": sha256_file(stderr_path)},
    }


def write_state(control_root: pathlib.Path, state: str, detail: dict[str, Any]) -> None:
    if state not in {"PREPARED", "RUNNING", "EXECUTED_PENDING_AUDIT", "BLOCKED"}:
        raise RuntimeErrorEvidence(f"unknown lifecycle state: {state}")
    transitions = {"PREPARED": {"RUNNING", "BLOCKED"}, "RUNNING": {"EXECUTED_PENDING_AUDIT", "BLOCKED"},
                   "EXECUTED_PENDING_AUDIT": set(), "BLOCKED": set()}
    state_path = control_root / "state.json"
    if state_path.exists():
        previous = read_json(state_path).get("state")
        if previous not in transitions or state not in transitions[previous]:
            raise RuntimeErrorEvidence(f"invalid lifecycle transition: {previous} -> {state}")
    elif state != "PREPARED":
        raise RuntimeErrorEvidence("first lifecycle state must be PREPARED")
    record = {"schema_version": 1, "kind": "experiment-lifecycle", "state": state, "recorded_utc": utc_now(), "detail": detail}
    write_json(state_path, record)
    history = control_root / "state-history.jsonl"
    with history.open("ab") as stream:
        stream.write(canonical_json(record))


def inventory_files(root: pathlib.Path, entries: list[tuple[str, str, str, dict[str, Any] | None]]) -> list[dict[str, Any]]:
    """Inventory explicit paths only; the inventory itself is intentionally omitted."""
    rows: list[dict[str, Any]] = []
    for relative, role, schema, derivation in entries:
        path = root / relative
        if not path.is_file():
            raise RuntimeErrorEvidence(f"required artifact is absent: {path}")
        row: dict[str, Any] = {"path": relative, "role": role, "schema": schema, "size": path.stat().st_size, "sha256": sha256_file(path)}
        if derivation is not None:
            row["derivation"] = derivation
        rows.append(row)
    return rows


def retention_manifest(destination: pathlib.Path, retained: list[tuple[pathlib.Path, pathlib.Path]],
                       candidate_commit: str, archival_commit: str | None) -> dict[str, Any]:
    records = []
    for source, target in retained:
        rel = relative_path(destination, target)
        records.append({"source_path": str(source.resolve()), "retained_path": rel, "sha256": sha256_file(target), "size": target.stat().st_size})
    return {"schema_version": 1, "kind": "canonical-evidence-retention", "candidate_commit": candidate_commit,
            "archival_commit": archival_commit, "files": sorted(records, key=lambda row: row["retained_path"])}
