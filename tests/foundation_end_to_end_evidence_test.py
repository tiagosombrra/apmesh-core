#!/usr/bin/env python3
"""Integrated contract for revision-bound Foundation admission evidence."""

from __future__ import annotations

import argparse
import copy
import hashlib
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


def run(arguments: list[str], cwd: pathlib.Path | None = None) -> str:
    completed = subprocess.run(arguments, cwd=cwd, check=True, capture_output=True, text=True)
    return completed.stdout.strip()


def sha256(path: pathlib.Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def require_rejection(action, message: str) -> None:
    try:
        action()
    except Exception:
        return
    raise RuntimeError(message)


def write_inputs(tool, path: pathlib.Path, profile: pathlib.Path, package: pathlib.Path,
                 recorded_source: str, commit: str, artifacts: dict[str, pathlib.Path]) -> pathlib.Path:
    tool.write_json(path, {"schema_version": 1, "kind": "foundation-admission-input-manifest", "candidate_commit": commit,
                           "candidate_retention_manifest_sha256": sha256(package / "retention-manifest.json"),
                           "profile_sha256": sha256(profile), "source_root": recorded_source,
                           "artifacts": {name: {"path": item.relative_to(path.parent).as_posix(), "sha256": sha256(item)}
                                         for name, item in artifacts.items()}})
    return path


def require_blocked(tool, gate: str, *arguments, **keywords) -> None:
    result = tool.qualify(*arguments, **keywords)
    if result["state"] != tool.BLOCKED or result["gates"][gate] != tool.BLOCKED:
        raise RuntimeError(f"Foundation qualifier did not block {gate}")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--tool", required=True); parser.add_argument("--profile", required=True); parser.add_argument("--rec-profile", required=True)
    arguments = parser.parse_args(); tool = load_tool(pathlib.Path(arguments.tool))
    repository = pathlib.Path(arguments.profile).resolve().parents[2]
    package = repository / "evidence" / "foundation" / "reproducible-experiment-contract" / "rec-e0-e7-85d215a"
    terminal = tool.read_json(package / "terminal-manifest.json")
    commit = terminal["candidate_commit"]
    prepared = tool.read_json(package / "control" / "prepared-manifest.json")
    recorded_source = prepared["candidate"]["source_root"]
    with tempfile.TemporaryDirectory(prefix="apmesh-core-foundation-contract-") as temporary:
        root, remote, source = pathlib.Path(temporary), pathlib.Path(temporary) / "remote.git", pathlib.Path(temporary) / "source"
        run(["git", "init", "--bare", str(remote)])
        run(["git", "push", str(remote), f"{commit}:refs/heads/foundation/test"], repository)
        run(["git", "clone", "--branch", "foundation/test", str(remote), str(source)])
        profile = copy.deepcopy(tool.read_json(pathlib.Path(arguments.profile)))
        profile["accepted_baseline"] = {"path": str(package), "candidate_commit": commit,
                                        "retention_manifest_sha256": sha256(package / "retention-manifest.json")}
        profile["required_contract_tests"] = [name for name in profile["required_contract_tests"]
                                              if name != "apmesh_core.foundation_end_to_end_evidence"]
        if len(profile["required_contract_tests"]) != 11:
            raise RuntimeError("Foundation fixture contract matrix differs")
        authority_rows = []
        for row in profile["qualified_authorities"]:
            if row["name"] == "reproducible_experiment":
                row = {**row, "qualified_marker": "IMPLEMENTATION CANDIDATE COMPLETE"}
            item = source / row["path"]
            if not item.is_file() or row["qualified_marker"] not in item.read_text(encoding="utf-8"):
                raise RuntimeError("qualified authority fixture is absent")
            authority_rows.append({**row, "sha256": sha256(item)})
        profile["qualified_authorities"] = authority_rows
        profile["scope_policy"]["approved_support_paths"] = [{"path": "README.md", "candidate_sha256": sha256(source / "README.md")}]
        profile_path = root / "foundation-profile.json"; tool.write_json(profile_path, profile)
        authorities = root / "authorities.json"
        tool.write_json(authorities, {"schema_version": 1, "kind": "foundation-qualified-authorities", "candidate_commit": commit,
                                      "authorities": authority_rows})
        observed = tool.collect_observed_build_and_contract_evidence(profile_path, source, commit, root / "observed")
        dependencies_ok, dependency_findings, build_directories = tool.validate_dependencies(observed["dependencies"], profile, commit, root / "observed", source)
        contracts_ok, contract_findings = tool.validate_contract_tests(observed["contract_tests"], profile, commit, root / "observed", build_directories)
        if not dependencies_ok or dependency_findings or not contracts_ok or contract_findings:
            raise RuntimeError("real Foundation evidence was not accepted")
        dependencies = tool.read_json(observed["dependencies"])
        dependencies["cells"][0]["build"]["source_before"]["commit"] = "0" * 40
        tool.write_json(observed["dependencies"], dependencies)
        require_rejection(lambda: tool.validate_dependencies(observed["dependencies"], profile, commit, root / "observed", source),
                          "Foundation accepted a build not bound to its Git revision")
        observed = tool.collect_observed_build_and_contract_evidence(profile_path, source, commit, root / "observed-rebuilt")
        publication = root / "publication.json"; tool.write_json(publication, tool.source_identity(source))
        artifacts = {"publication": publication, "authorities": authorities, **observed}
        inputs = write_inputs(tool, root / "inputs.json", profile_path, package, recorded_source, commit, artifacts)
        output = root / "qualified"
        result = tool.qualify(profile_path, pathlib.Path(arguments.rec_profile), package, package, inputs, output,
                              verification_source_root=source)
        if result["state"] != tool.PENDING or any(value != tool.PENDING for value in result["gates"].values()):
            raise RuntimeError("integrated Foundation qualifier rejected real retained REC evidence")

        forged_publication = {**tool.read_json(publication), "upstream_head": "f" * 40}; tool.write_json(publication, forged_publication)
        inputs = write_inputs(tool, root / "negative-fnd0.json", profile_path, package, recorded_source, commit, artifacts)
        require_blocked(tool, "FND0", profile_path, pathlib.Path(arguments.rec_profile), package, package, inputs, root / "negative-fnd0", verification_source_root=source)
        tool.write_json(publication, tool.source_identity(source))

        invalid_authorities = tool.read_json(authorities); invalid_authorities["authorities"][0]["qualified_marker"] = "missing"
        tool.write_json(authorities, invalid_authorities)
        inputs = write_inputs(tool, root / "negative-fnd1.json", profile_path, package, recorded_source, commit, artifacts)
        require_blocked(tool, "FND1", profile_path, pathlib.Path(arguments.rec_profile), package, package, inputs, root / "negative-fnd1", verification_source_root=source)
        tool.write_json(authorities, {"schema_version": 1, "kind": "foundation-qualified-authorities", "candidate_commit": commit, "authorities": authority_rows})

        incomplete_terminal = {**terminal, "bundles": terminal["bundles"][:-1]}
        if tool._terminal_complete(incomplete_terminal, commit):
            raise RuntimeError("Foundation accepted incomplete cumulative execution")
        if tool._claims_equivalent({"replay_comparisons": [], "cross_configuration": {}}):
            raise RuntimeError("Foundation accepted non-equivalent deterministic claims")
        if not tool._diff({"claim": "baseline"}, {"claim": "candidate"}):
            raise RuntimeError("Foundation accepted an unclassified baseline difference")

        contracts = tool.read_json(observed["contract_tests"]); junit = root / "observed-rebuilt" / contracts["cells"][0]["junit"]["path"]
        junit.write_text("<testsuites><testsuite><testcase name=\"missing\"/></testsuite></testsuites>\n", encoding="utf-8")
        contracts["cells"][0]["junit"]["sha256"] = sha256(junit); tool.write_json(observed["contract_tests"], contracts)
        inputs = write_inputs(tool, root / "negative-fnd3.json", profile_path, package, recorded_source, commit, artifacts)
        require_blocked(tool, "FND3", profile_path, pathlib.Path(arguments.rec_profile), package, package, inputs, root / "negative-fnd3", verification_source_root=source)

        observed = tool.collect_observed_build_and_contract_evidence(profile_path, source, commit, root / "observed-final")
        artifacts = {"publication": publication, "authorities": authorities, **observed}
        inputs = write_inputs(tool, root / "negative-fnd7.json", profile_path, package, recorded_source, commit, artifacts)
        dependencies = tool.read_json(observed["dependencies"]); executable = dependencies["cells"][0]["executables"][0]
        ldd = root / "observed-final" / executable["ldd"]["path"]; ldd.write_text("libundeclared.so => not found\n", encoding="utf-8")
        digest = sha256(ldd); executable["ldd"]["sha256"] = digest; executable["execution"]["stdout"]["sha256"] = digest
        tool.write_json(observed["dependencies"], dependencies)
        inputs = write_inputs(tool, root / "negative-fnd7.json", profile_path, package, recorded_source, commit, artifacts)
        require_blocked(tool, "FND7", profile_path, pathlib.Path(arguments.rec_profile), package, package, inputs, root / "negative-fnd7", verification_source_root=source)

        tool.verify_foundation_retention(output, profile_path, pathlib.Path(arguments.rec_profile), package / "retention-manifest.json", commit,
                                         source, pathlib.Path(recorded_source))
        (output / "inputs" / "raw" / "observed-rebuilt" / "dependencies.json").write_text("{}\n", encoding="utf-8")
        require_rejection(lambda: tool.verify_foundation_retention(output, profile_path, pathlib.Path(arguments.rec_profile), package / "retention-manifest.json", commit,
                                                                     source, pathlib.Path(recorded_source)),
                          "Foundation retention accepted tampered semantic evidence")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
