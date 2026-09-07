#!/usr/bin/env python3
"""Focused contracts for hash-bound Foundation admission evidence."""

from __future__ import annotations

import argparse
import copy
import importlib.util
import pathlib
import subprocess
import sys
import tempfile


CELLS = ("gcc-debug", "gcc-release", "clang-debug", "clang-release")
BASE = "a" * 40
CANDIDATE = "b" * 40


def load_tool(path: pathlib.Path):
    sys.path.insert(0, str(path.parent))
    spec = importlib.util.spec_from_file_location("foundation_end_to_end_evidence", path)
    if spec is None or spec.loader is None:
        raise RuntimeError("cannot load Foundation evidence tool")
    module = importlib.util.module_from_spec(spec); sys.modules[spec.name] = module; spec.loader.exec_module(module)
    return module


def write_package(tool, root: pathlib.Path, commit: str, source_root: pathlib.Path, changed: str | None = None) -> None:
    tool.write_json(root / "retention-manifest.json", {"schema_version": 1, "kind": "canonical-evidence-retention", "candidate_commit": commit, "archival_commit": None, "files": []})
    bundles = []
    for cell in CELLS:
        for replay in (1, 2):
            relative = f"cells/{cell}/replay-{replay}"; bundles.append({"cell": cell, "replay": replay, "bundle": relative})
            tool.write_json(root / relative / "summary.json", {"schema_version": 1, "kind": "reproducible-experiment-summary", "cell": cell, "replay": replay,
                "certificate": {"stable": True}, "environment": {"stable": True}, "gates": {f"E{index}": "BUNDLE_VALIDATED" for index in range(8)},
                "artifact_roles": ["summary.json"], "retained_limitations": ["synthetic"]})
    prerequisites = {}
    for name, location in (("architecture_contract", "prerequisites/architecture/manifest.json"), ("numeric_contract", "prerequisites/numeric/manifest.json")):
        manifest = root / location; records = []
        for cell in CELLS:
            certificates = [f"cells/{cell}/certificate-{index}.json" for index in range(1, 4)]
            row = {"cell": cell, "certificates": certificates}
            if name == "numeric_contract": row["environments"] = [f"cells/{cell}/environment-{index}.json" for index in range(1, 4)]
            records.append(row)
            for item in certificates: tool.write_json(manifest.parent / item, {"kind": name, "stable": True})
            for item in row.get("environments", []): tool.write_json(manifest.parent / item, {"kind": "numeric-environment", "stable": True})
        tool.write_json(manifest, {"execution": {"records": records}})
        prerequisites[name] = {"candidate_commit": commit, "manifest": location, "manifest_sha256": "0" * 64, "state": "EXECUTED_PENDING_AUDIT"}
    tool.write_json(root / "terminal-manifest.json", {"state": "EXECUTED_PENDING_AUDIT", "candidate_commit": commit, "bundles": bundles,
        "negative_fixtures": [{"fixture": f"N{index}"} for index in range(1, 9)], "prerequisites": prerequisites,
        "replay_comparisons": [{"cell": cell, "claim_fields": "equivalent"} for cell in CELLS], "cross_configuration": {"claim_fields": "equivalent"}})
    inventory = [{"path": "README.md", "sha256": "1" * 64}]
    if changed: inventory.append({"path": changed, "sha256": "2" * 64})
    tool.write_json(root / "control" / "prepared-manifest.json", {"candidate": {"commit": commit, "tree_clean": True, "source_root": str(source_root.resolve()), "source_inventory": inventory}})


def write_authorities(tool, root: pathlib.Path, source_root: pathlib.Path, profile: dict, commit: str) -> pathlib.Path:
    rows = []
    for row in profile["qualified_authorities"]:
        path = source_root / row["path"]; path.parent.mkdir(parents=True, exist_ok=True); path.write_text(f"{row['qualified_marker']}\n", encoding="utf-8")
        updated = dict(row); updated["sha256"] = tool.sha256_file(path); rows.append(updated)
    profile["qualified_authorities"] = rows
    result = root / "authorities.json"; tool.write_json(result, {"schema_version": 1, "kind": "foundation-qualified-authorities", "candidate_commit": commit, "authorities": rows})
    return result


def write_dependencies(tool, root: pathlib.Path, profile: dict, commit: str) -> pathlib.Path:
    cells = []
    for cell, family in profile["configurations"].items():
        allowed = profile["allowed_runtime_dependencies"][family]; executables = []
        for name in profile["required_executables"]:
            binary = root / "bins" / cell / name; binary.parent.mkdir(parents=True, exist_ok=True); binary.write_bytes(f"{cell}/{name}".encode())
            raw = root / "ldd" / f"{cell}-{name}.txt"; raw.parent.mkdir(parents=True, exist_ok=True)
            raw.write_text(f"linux-vdso.so.1 (0x0000)\n{allowed[0]} => /lib/{allowed[0]} (0x0000)\n", encoding="utf-8")
            stderr = root / "ldd" / f"{cell}-{name}.stderr"; stderr.write_text("", encoding="utf-8")
            executables.append({"name": name, "path": binary.relative_to(root).as_posix(), "sha256": tool.sha256_file(binary),
                                "ldd": {"path": raw.relative_to(root).as_posix(), "sha256": tool.sha256_file(raw)},
                                "execution": {"argv": ["ldd", str(binary)], "exit_code": 0, "pid": 1,
                                              "started_utc": "2026-01-01T00:00:00Z", "finished_utc": "2026-01-01T00:00:01Z",
                                              "stdout": {"path": raw.relative_to(root).as_posix(), "sha256": tool.sha256_file(raw)},
                                              "stderr": {"path": stderr.relative_to(root).as_posix(), "sha256": tool.sha256_file(stderr)}}})
        cells.append({"name": cell, "executables": executables})
    result = root / "dependencies.json"; tool.write_json(result, {"schema_version": 3, "kind": "foundation-runtime-dependencies", "candidate_commit": commit, "cells": cells}); return result


def write_contracts(tool, root: pathlib.Path, profile: dict, commit: str) -> pathlib.Path:
    names = list(profile["required_contract_tests"]); cells = []
    for cell in CELLS:
        discovery = root / "ctest" / f"{cell}-discover.json"; discovery.parent.mkdir(parents=True, exist_ok=True)
        tool.write_json(discovery, {"tests": [{"name": name, "properties": [{"name": "LABELS", "value": ["contract"]}]} for name in names]})
        junit = root / "ctest" / f"{cell}-result.xml"; junit.write_text("<testsuites><testsuite>" + "".join(f'<testcase name="{name}"/>' for name in names) + "</testsuite></testsuites>\n", encoding="utf-8")
        stdout = root / "ctest" / f"{cell}-stdout.txt"; stdout.write_text("ctest completed\n", encoding="utf-8")
        stderr = root / "ctest" / f"{cell}-stderr.txt"; stderr.write_text("", encoding="utf-8")
        command = ["ctest", "--test-dir", f"build/{cell}", "-L", "contract", "--output-junit", str(junit)]
        cells.append({"name": cell, "command": command,
                      "discovery": {"path": discovery.relative_to(root).as_posix(), "sha256": tool.sha256_file(discovery)},
                      "junit": {"path": junit.relative_to(root).as_posix(), "sha256": tool.sha256_file(junit)}, "exit_code": 0,
                      "execution": {"argv": command, "exit_code": 0, "pid": 1,
                                    "started_utc": "2026-01-01T00:00:00Z", "finished_utc": "2026-01-01T00:00:01Z",
                                    "stdout": {"path": stdout.relative_to(root).as_posix(), "sha256": tool.sha256_file(stdout)},
                                    "stderr": {"path": stderr.relative_to(root).as_posix(), "sha256": tool.sha256_file(stderr)}}})
    result = root / "contract-tests.json"; tool.write_json(result, {"schema_version": 3, "kind": "foundation-contract-tests", "candidate_commit": commit, "cells": cells}); return result


def inputs_manifest(tool, root: pathlib.Path, profile_path: pathlib.Path, candidate: pathlib.Path, source_root: pathlib.Path,
                    commit: str, artifacts: dict[str, pathlib.Path]) -> pathlib.Path:
    result = root / "inputs.json"
    tool.write_json(result, {"schema_version": 1, "kind": "foundation-admission-input-manifest", "candidate_commit": commit,
        "candidate_retention_manifest_sha256": tool.sha256_file(candidate / "retention-manifest.json"), "profile_sha256": tool.sha256_file(profile_path),
        "source_root": str(source_root.resolve()), "artifacts": {name: {"path": path.relative_to(root).as_posix(), "sha256": tool.sha256_file(path)} for name, path in artifacts.items()}})
    return result


def assert_blocked(tool, *arguments, gate: str) -> None:
    result = tool.qualify(*arguments)
    if result["state"] != tool.BLOCKED or result["gates"][gate] != tool.BLOCKED:
        raise RuntimeError(f"Foundation qualifier did not block {gate}")


def git(source: pathlib.Path, *arguments: str) -> str:
    completed = subprocess.run(["git", *arguments], cwd=source, check=True, capture_output=True, text=True)
    return completed.stdout.strip()


def main() -> int:
    global BASE, CANDIDATE
    parser = argparse.ArgumentParser(); parser.add_argument("--tool", required=True); parser.add_argument("--profile", required=True)
    arguments = parser.parse_args(); tool = load_tool(pathlib.Path(arguments.tool))
    with tempfile.TemporaryDirectory() as temporary:
        root = pathlib.Path(temporary); source = root / "source"; source.mkdir(); baseline, candidate = root / "baseline", root / "candidate"
        remote = root / "remote.git"
        subprocess.run(["git", "init", "--bare", str(remote)], check=True, capture_output=True, text=True)
        git(source, "init", "-b", "foundation/test"); git(source, "config", "user.email", "foundation@test.invalid"); git(source, "config", "user.name", "Foundation Test")
        (source / "README.md").write_text("foundation\n", encoding="utf-8"); git(source, "add", "README.md"); git(source, "commit", "-m", "base")
        BASE = git(source, "rev-parse", "HEAD")
        git(source, "remote", "add", "origin", str(remote)); git(source, "push", "-u", "origin", "foundation/test")
        # The actual candidate commit is created after the authority documents
        # are populated below; source_identity is never replaced in this test.
        write_package(tool, baseline, BASE, source); write_package(tool, candidate, CANDIDATE, source)
        profile = copy.deepcopy(tool.read_json(pathlib.Path(arguments.profile)))
        profile["accepted_baseline"] = {"path": baseline.as_posix(), "candidate_commit": BASE, "retention_manifest_sha256": tool.sha256_file(baseline / "retention-manifest.json")}
        profile["scope_policy"]["approved_support_paths"] = [{"path": "tools/foundation_end_to_end_evidence.py", "candidate_sha256": "2" * 64}]
        profile["scope_policy"]["verified_retained_prefix"]["retention_manifest_sha256"] = tool.sha256_file(baseline / "retention-manifest.json")
        profile_path = root / "profile.json"; write_authorities(tool, root, source, profile, CANDIDATE)
        git(source, "add", "."); git(source, "commit", "-m", "authorities"); CANDIDATE = git(source, "rev-parse", "HEAD"); git(source, "push")
        write_package(tool, candidate, CANDIDATE, source); authorities = write_authorities(tool, root, source, profile, CANDIDATE); tool.write_json(profile_path, profile)
        dependencies = write_dependencies(tool, root, profile, CANDIDATE); contracts = write_contracts(tool, root, profile, CANDIDATE)
        publication = root / "publication.json"; published = {"schema_version": 1, "kind": "foundation-candidate-publication", "branch": "foundation/test", "head": CANDIDATE, "upstream": "origin/foundation/test", "upstream_head": CANDIDATE, "tree_clean": True}; tool.write_json(publication, published)
        tool.verify_retained_package = lambda package, declared_profile: None
        artifact_map = {"publication": publication, "authorities": authorities, "dependencies": dependencies, "contract_tests": contracts}
        manifest = inputs_manifest(tool, root, profile_path, candidate, source, CANDIDATE, artifact_map)
        args = (profile_path, root / "rec-profile.json", baseline, candidate, manifest, root / "pass")
        result = tool.qualify(*args)
        if result["state"] != tool.PENDING or any(value != tool.PENDING for value in result["gates"].values()):
            raise RuntimeError("Foundation qualifier rejected valid hash-bound evidence")
        tool.verify_foundation_retention(root / "pass", profile_path, candidate / "retention-manifest.json", CANDIDATE, source)

        tampered = root / "pass" / "summary.json"; tampered.write_text("{}\n", encoding="utf-8")
        try: tool.verify_foundation_retention(root / "pass", profile_path, candidate / "retention-manifest.json", CANDIDATE, source)
        except tool.RuntimeErrorEvidence: pass
        else: raise RuntimeError("Foundation retention accepted tampered output")

        tool.write_json(publication, {**published, "upstream_head": "c" * 40})
        manifest = inputs_manifest(tool, root, profile_path, candidate, source, CANDIDATE, artifact_map)
        assert_blocked(tool, profile_path, root / "rec-profile.json", baseline, candidate, manifest, root / "publication-blocked", gate="FND0")
        tool.write_json(publication, published)

        modified = source / profile["qualified_authorities"][0]["path"]; modified.write_text("not qualified\n", encoding="utf-8")
        manifest = inputs_manifest(tool, root, profile_path, candidate, source, CANDIDATE, artifact_map)
        assert_blocked(tool, profile_path, root / "rec-profile.json", baseline, candidate, manifest, root / "authority-blocked", gate="FND1")
        modified.write_text(profile["qualified_authorities"][0]["qualified_marker"] + "\n", encoding="utf-8")

        junit = root / "ctest" / "gcc-debug-result.xml"; junit.write_text("<testsuites><testsuite><testcase name=\"missing\"/></testsuite></testsuites>\n", encoding="utf-8")
        contract = tool.read_json(contracts); contract["cells"][0]["junit"]["sha256"] = tool.sha256_file(junit); tool.write_json(contracts, contract)
        manifest = inputs_manifest(tool, root, profile_path, candidate, source, CANDIDATE, artifact_map)
        assert_blocked(tool, profile_path, root / "rec-profile.json", baseline, candidate, manifest, root / "ctest-blocked", gate="FND3")
        contracts = write_contracts(tool, root, profile, CANDIDATE); artifact_map["contract_tests"] = contracts

        binary = root / "bins" / "gcc-debug" / profile["required_executables"][0]; binary.write_bytes(b"tampered")
        manifest = inputs_manifest(tool, root, profile_path, candidate, source, CANDIDATE, artifact_map)
        try: tool.qualify(profile_path, root / "rec-profile.json", baseline, candidate, manifest, root / "dependency-blocked")
        except tool.RuntimeErrorEvidence: pass
        else: raise RuntimeError("Foundation qualifier accepted mismatched executable hash")
        dependencies = write_dependencies(tool, root, profile, CANDIDATE); artifact_map["dependencies"] = dependencies

        prepared = tool.read_json(candidate / "control" / "prepared-manifest.json"); prepared["candidate"]["source_inventory"].append({"path": "src/core/numeric.cpp", "sha256": "4" * 64}); tool.write_json(candidate / "control" / "prepared-manifest.json", prepared)
        manifest = inputs_manifest(tool, root, profile_path, candidate, source, CANDIDATE, artifact_map)
        assert_blocked(tool, profile_path, root / "rec-profile.json", baseline, candidate, manifest, root / "scope-blocked", gate="FND0")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
