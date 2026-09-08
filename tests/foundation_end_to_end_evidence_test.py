#!/usr/bin/env python3
"""End-to-end focused contract for observed Foundation admission evidence."""

from __future__ import annotations

import argparse
import copy
import importlib.util
import pathlib
import subprocess
import sys
import tempfile


def load_tool(path: pathlib.Path):
    sys.path.insert(0, str(path.parent))
    spec = importlib.util.spec_from_file_location("foundation_end_to_end_evidence", path)
    if spec is None or spec.loader is None:
        raise RuntimeError("cannot load Foundation evidence tool")
    module = importlib.util.module_from_spec(spec); sys.modules[spec.name] = module; spec.loader.exec_module(module)
    return module


def git(source: pathlib.Path, *arguments: str) -> str:
    completed = subprocess.run(["git", *arguments], cwd=source, check=True, capture_output=True, text=True)
    return completed.stdout.strip()


def require_rejection(callback, message: str) -> None:
    try:
        callback()
    except Exception:
        return
    raise RuntimeError(message)


def write_project(source: pathlib.Path, executables: list[str], tests: list[str]) -> None:
    (source / "main.cpp").write_text("int main() { return 0; }\n", encoding="utf-8")
    lines = ["cmake_minimum_required(VERSION 3.25)", "project(foundation_evidence LANGUAGES CXX)", "include(CTest)"]
    lines.extend(f"add_executable({name} main.cpp)" for name in executables)
    for name in tests:
        lines.extend((f"add_test(NAME {name} COMMAND ${{CMAKE_COMMAND}} -E true)", f"set_tests_properties({name} PROPERTIES LABELS contract)"))
    (source / "CMakeLists.txt").write_text("\n".join(lines) + "\n", encoding="utf-8")


def write_authorities(tool, root: pathlib.Path, source: pathlib.Path, profile: dict, commit: str) -> pathlib.Path:
    rows = []
    for row in profile["qualified_authorities"]:
        path = source / row["path"]; path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(f"{row['qualified_marker']}\n", encoding="utf-8")
        updated = dict(row); updated["sha256"] = tool.sha256_file(path); rows.append(updated)
    profile["qualified_authorities"] = rows
    result = root / "authorities.json"
    tool.write_json(result, {"schema_version": 1, "kind": "foundation-qualified-authorities", "candidate_commit": commit, "authorities": rows})
    return result


def write_candidate_package(tool, root: pathlib.Path, commit: str, source: pathlib.Path) -> pathlib.Path:
    tool.write_json(root / "retention-manifest.json", {"schema_version": 1, "kind": "canonical-evidence-retention", "candidate_commit": commit, "archival_commit": None, "files": []})
    prerequisites = {name: {"candidate_commit": commit, "manifest": f"prerequisites/{name}.json", "state": "EXECUTED_PENDING_AUDIT"}
                     for name in ("architecture_contract", "numeric_contract")}
    tool.write_json(root / "terminal-manifest.json", {"state": "EXECUTED_PENDING_AUDIT", "candidate_commit": commit,
                    "bundles": [{"cell": cell, "replay": replay, "bundle": "unused"} for cell in ("gcc-debug", "gcc-release", "clang-debug", "clang-release") for replay in (1, 2)],
                    "negative_fixtures": [{"fixture": f"N{index}"} for index in range(8)], "prerequisites": prerequisites,
                    "replay_comparisons": [{"cell": cell, "claim_fields": "equivalent"} for cell in ("gcc-debug", "gcc-release", "clang-debug", "clang-release")],
                    "cross_configuration": {"claim_fields": "equivalent"}})
    tool.write_json(root / "control" / "prepared-manifest.json", {"candidate": {"commit": commit, "tree_clean": True, "source_root": str(source.resolve()), "source_inventory": [{"path": "README.md", "sha256": "0" * 64}]}})
    return root


def main() -> int:
    parser = argparse.ArgumentParser(); parser.add_argument("--tool", required=True); parser.add_argument("--profile", required=True)
    arguments = parser.parse_args(); tool = load_tool(pathlib.Path(arguments.tool))
    with tempfile.TemporaryDirectory(prefix="apmesh-core-foundation-contract-") as temporary:
        root = pathlib.Path(temporary); source, remote = root / "source", root / "remote.git"; source.mkdir()
        subprocess.run(["git", "init", "--bare", str(remote)], check=True, capture_output=True, text=True)
        git(source, "init", "-b", "foundation/test"); git(source, "config", "user.email", "foundation@test.invalid"); git(source, "config", "user.name", "Foundation Test")
        profile = copy.deepcopy(tool.read_json(pathlib.Path(arguments.profile)))
        profile["required_executables"] = ["foundation_probe_one", "foundation_probe_two"]
        profile["required_contract_tests"] = ["foundation.contract.one", "foundation.contract.two"]
        write_project(source, profile["required_executables"], profile["required_contract_tests"])
        (source / "README.md").write_text("Foundation evidence fixture\n", encoding="utf-8")
        write_authorities(tool, root, source, profile, "pending")
        git(source, "add", "."); git(source, "commit", "-m", "Foundation evidence fixture")
        commit = git(source, "rev-parse", "HEAD"); git(source, "remote", "add", "origin", str(remote)); git(source, "push", "-u", "origin", "foundation/test")
        authorities = write_authorities(tool, root, source, profile, commit)
        profile_path = root / "profile.json"; tool.write_json(profile_path, profile)
        observed = tool.collect_observed_build_and_contract_evidence(profile_path, source, commit, root / "observed")
        dependencies_ok, dependency_findings, build_directories = tool.validate_dependencies(observed["dependencies"], profile, commit, root / "observed", source)
        contracts_ok, contract_findings = tool.validate_contract_tests(observed["contract_tests"], profile, commit, root / "observed", build_directories)
        if not dependencies_ok or dependency_findings or not contracts_ok or contract_findings:
            raise RuntimeError("observed Foundation evidence was not accepted")

        publication = root / "publication.json"; tool.write_json(publication, tool.source_identity(source))
        if not tool.validate_publication(publication, commit, source):
            raise RuntimeError("actual Git publication identity was not accepted")
        terminal = tool.read_json(write_candidate_package(tool, root / "candidate", commit, source) / "terminal-manifest.json")
        if not tool.validate_authorities(authorities, profile, commit, source, terminal):
            raise RuntimeError("actual Foundation authorities were not accepted")

        dependencies = tool.read_json(observed["dependencies"]); first = dependencies["cells"][0]["executables"][0]
        binary = root / "observed" / first["path"]; original = binary.read_bytes(); binary.write_bytes(b"tampered")
        require_rejection(lambda: tool.validate_dependencies(observed["dependencies"], profile, commit, root / "observed", source), "Foundation accepted a binary not produced by the recorded build")
        binary.write_bytes(original)
        contracts = tool.read_json(observed["contract_tests"]); contracts["cells"][0]["discovery_execution"]["argv"] = ["ctest", "--unexpected"]
        tool.write_json(observed["contract_tests"], contracts)
        require_rejection(lambda: tool.validate_contract_tests(observed["contract_tests"], profile, commit, root / "observed", build_directories), "Foundation accepted undeclared CTest discovery")

        observed = tool.collect_observed_build_and_contract_evidence(profile_path, source, commit, root / "observed-rebuilt")
        dependencies_ok, _, build_directories = tool.validate_dependencies(observed["dependencies"], profile, commit, root / "observed-rebuilt", source)
        contracts_ok, _ = tool.validate_contract_tests(observed["contract_tests"], profile, commit, root / "observed-rebuilt", build_directories)
        if not dependencies_ok or not contracts_ok:
            raise RuntimeError("recollected evidence was not accepted")

        candidate = write_candidate_package(tool, root / "candidate-retained", commit, source)
        input_manifest = root / "inputs.json"; artifacts = {"publication": publication, "authorities": authorities, **observed}
        tool.write_json(input_manifest, {"schema_version": 1, "kind": "foundation-admission-input-manifest", "candidate_commit": commit,
                        "candidate_retention_manifest_sha256": tool.sha256_file(candidate / "retention-manifest.json"), "profile_sha256": tool.sha256_file(profile_path),
                        "source_root": str(source.resolve()), "artifacts": {name: {"path": path.relative_to(root).as_posix(), "sha256": tool.sha256_file(path)} for name, path in artifacts.items()}})
        summary = {"schema_version": 1, "kind": "foundation-end-to-end-evidence-summary", "state": tool.PENDING, "candidate_commit": commit,
                   "baseline_commit": commit, "baseline_comparison": {"classification": "NO_CHANGE", "differences": []}, "scope_changes": [],
                   "dependency_findings": [], "contract_test_findings": [], "gates": {gate: tool.PENDING for gate in tool.EXPECTED_GATES}, "limitations": profile["limitations"]}
        output = root / "retained"; tool._write_outputs(output, summary, profile_path, candidate / "retention-manifest.json", candidate,
                                                          input_manifest, root, artifacts, source)
        tool.verify_foundation_retention(output, profile_path, candidate / "retention-manifest.json", commit, source)
        (output / "inputs" / "raw" / "observed-rebuilt" / "dependencies.json").write_text("{}\n", encoding="utf-8")
        require_rejection(lambda: tool.verify_foundation_retention(output, profile_path, candidate / "retention-manifest.json", commit, source), "Foundation retention accepted tampered transitive evidence")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
