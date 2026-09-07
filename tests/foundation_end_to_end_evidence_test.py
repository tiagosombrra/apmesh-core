#!/usr/bin/env python3
"""Focused contracts for the report-only Foundation end-to-end qualifier."""

from __future__ import annotations

import argparse
import copy
import importlib.util
import pathlib
import sys
import tempfile


CELLS = ("gcc-debug", "gcc-release", "clang-debug", "clang-release")


def load_tool(path: pathlib.Path):
    sys.path.insert(0, str(path.parent))
    spec = importlib.util.spec_from_file_location("foundation_end_to_end_evidence", path)
    if spec is None or spec.loader is None:
        raise RuntimeError("cannot load Foundation evidence tool")
    module = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module


def write_package(tool, root: pathlib.Path, commit: str, stable: bool = True) -> None:
    tool.write_json(root / "retention-manifest.json", {"schema_version": 1, "kind": "canonical-evidence-retention",
                                                        "candidate_commit": commit, "archival_commit": None, "files": []})
    bundles = []
    for cell in CELLS:
        for replay in (1, 2):
            relative = f"cells/{cell}/replay-{replay}"
            bundles.append({"cell": cell, "replay": replay, "bundle": relative})
            tool.write_json(root / relative / "summary.json", {
                "schema_version": 1, "kind": "reproducible-experiment-summary", "cell": cell, "replay": replay,
                "certificate": {"stable": stable}, "environment": {"stable": True},
                "gates": {f"E{index}": "BUNDLE_VALIDATED" for index in range(8)},
                "artifact_roles": ["summary.json"], "retained_limitations": ["synthetic"],
            })
    prerequisites = {}
    for name, location in (("architecture_contract", "prerequisites/architecture/manifest.json"),
                           ("numeric_contract", "prerequisites/numeric/manifest.json")):
        manifest_path = root / location
        records = []
        for cell in CELLS:
            certificates = [f"cells/{cell}/certificate-{index}.json" for index in range(1, 4)]
            row = {"cell": cell, "certificates": certificates}
            if name == "numeric_contract":
                row["environments"] = [f"cells/{cell}/environment-{index}.json" for index in range(1, 4)]
            records.append(row)
            for item in certificates:
                tool.write_json(manifest_path.parent / item, {"kind": name, "stable": stable})
            for item in row.get("environments", []):
                tool.write_json(manifest_path.parent / item, {"kind": "numeric-environment", "stable": True})
        tool.write_json(manifest_path, {"execution": {"records": records}})
        prerequisites[name] = {"candidate_commit": commit, "manifest": location,
                               "manifest_sha256": "0" * 64, "state": "EXECUTED_PENDING_AUDIT"}
    tool.write_json(root / "terminal-manifest.json", {
        "state": "EXECUTED_PENDING_AUDIT", "candidate_commit": commit, "bundles": bundles,
        "negative_fixtures": [{"fixture": f"N{index}"} for index in range(1, 9)],
        "prerequisites": prerequisites,
        "replay_comparisons": [{"cell": cell, "claim_fields": "equivalent"} for cell in CELLS],
        "cross_configuration": {"claim_fields": "equivalent"},
    })
    inventory = [{"path": "README.md", "sha256": "2" * 64}]
    if commit.startswith("b"):
        inventory.append({"path": "tools/foundation_end_to_end_evidence.py", "sha256": "3" * 64})
    tool.write_json(root / "control" / "prepared-manifest.json", {
        "candidate": {"commit": commit, "tree_clean": True, "source_root": "/synthetic",
                      "source_inventory": inventory}
    })


def write_dependencies(tool, path: pathlib.Path, profile: dict, commit: str,
                       unknown: bool = False, outside_system: bool = False) -> None:
    cells = []
    for cell, family in profile["configurations"].items():
        allowed = profile["allowed_runtime_dependencies"][family]
        dependencies = [{"soname": allowed[0], "resolved_path": f"/opt/{allowed[0]}" if outside_system else f"/lib/{allowed[0]}"}]
        if unknown and cell == "gcc-debug":
            dependencies.append({"soname": "libunexpected.so", "resolved_path": "/opt/libunexpected.so"})
        cells.append({"name": cell, "executables": [
            {"name": name, "path": f"/tmp/{cell}/{name}", "sha256": "1" * 64,
             "dependencies": dependencies, "unresolved": []}
            for name in profile["required_executables"]
        ]})
    tool.write_json(path, {"schema_version": 1, "kind": "foundation-runtime-dependencies",
                           "candidate_commit": commit, "cells": cells})


def write_contract_tests(tool, path: pathlib.Path, profile: dict, commit: str, missing: bool = False) -> None:
    tests = list(profile["required_contract_tests"])
    cells = []
    for cell in CELLS:
        observed = tests[1:] if missing and cell == "gcc-debug" else tests
        cells.append({"name": cell, "command": ["ctest", "--test-dir", f"/tmp/{cell}", "-L", "contract"],
                      "discovered": observed, "passed": observed, "failed": [], "exit_code": 0})
    tool.write_json(path, {"schema_version": 1, "kind": "foundation-contract-tests",
                           "candidate_commit": commit, "cells": cells})


def main() -> int:
    parser = argparse.ArgumentParser(); parser.add_argument("--tool", required=True); parser.add_argument("--profile", required=True)
    arguments = parser.parse_args(); tool = load_tool(pathlib.Path(arguments.tool))
    with tempfile.TemporaryDirectory() as temporary_directory:
        root = pathlib.Path(temporary_directory); baseline, candidate = root / "baseline", root / "candidate"
        baseline_commit, candidate_commit = "a" * 40, "b" * 40
        write_package(tool, baseline, baseline_commit); write_package(tool, candidate, candidate_commit)
        profile = tool.read_json(pathlib.Path(arguments.profile)); profile = copy.deepcopy(profile)
        profile["accepted_baseline"] = {"path": baseline.as_posix(), "candidate_commit": baseline_commit,
                                         "retention_manifest_sha256": tool.sha256_file(baseline / "retention-manifest.json")}
        profile_path = root / "profile.json"; tool.write_json(profile_path, profile)
        rec_profile = root / "rec-profile.json"; tool.write_json(rec_profile, {"synthetic": True})
        publication = root / "publication.json"
        tool.write_json(publication, {"schema_version": 1, "kind": "foundation-candidate-publication",
                                      "branch": "foundation/end-to-end", "head": candidate_commit,
                                      "upstream": "origin/foundation/end-to-end", "upstream_head": candidate_commit,
                                      "tree_clean": True})
        dependencies = root / "dependencies.json"; write_dependencies(tool, dependencies, profile, candidate_commit)
        contract_tests = root / "contract-tests.json"; write_contract_tests(tool, contract_tests, profile, candidate_commit)
        tool.verify_retained_package = lambda package, declared_profile: None
        result = tool.qualify(profile_path, rec_profile, baseline, candidate, publication, dependencies, contract_tests, root / "pass")
        if result["state"] != tool.PENDING or any(value != tool.PENDING for value in result["gates"].values()):
            raise RuntimeError("Foundation qualifier did not collect complete passing evidence")
        required = {"summary.json", "certificate.json", "report.md", "metrics.csv", "figures/status-matrix.svg", "artifact-inventory.json"}
        observed = {path.relative_to(root / "pass").as_posix() for path in (root / "pass").rglob("*") if path.is_file()}
        if observed != required:
            raise RuntimeError("Foundation qualifier artifact set differs")

        candidate_summary = candidate / "cells/gcc-debug/replay-1/summary.json"
        changed = tool.read_json(candidate_summary); changed["certificate"]["stable"] = False; tool.write_json(candidate_summary, changed)
        result = tool.qualify(profile_path, rec_profile, baseline, candidate, publication, dependencies, contract_tests, root / "claim-regression")
        if result["state"] != "BLOCKED" or result["gates"]["FND5"] != "BLOCKED" or not result["baseline_comparison"]["differences"]:
            raise RuntimeError("Foundation qualifier accepted a claim regression")
        changed["certificate"]["stable"] = True; tool.write_json(candidate_summary, changed)

        write_dependencies(tool, dependencies, profile, candidate_commit, unknown=True)
        result = tool.qualify(profile_path, rec_profile, baseline, candidate, publication, dependencies, contract_tests, root / "dependency-regression")
        if result["state"] != "BLOCKED" or result["gates"]["FND7"] != "BLOCKED" or not result["dependency_findings"]:
            raise RuntimeError("Foundation qualifier accepted an undeclared dependency")
        write_dependencies(tool, dependencies, profile, candidate_commit)

        write_dependencies(tool, dependencies, profile, candidate_commit, outside_system=True)
        result = tool.qualify(profile_path, rec_profile, baseline, candidate, publication, dependencies, contract_tests, root / "dependency-location-investigation")
        if result["state"] != "BLOCKED" or result["gates"]["FND7"] != "BLOCKED" or result["dependency_findings"][0]["classification"] != "INVESTIGATION_REQUIRED":
            raise RuntimeError("Foundation qualifier accepted a non-system dependency resolution")
        write_dependencies(tool, dependencies, profile, candidate_commit)

        write_contract_tests(tool, contract_tests, profile, candidate_commit, missing=True)
        result = tool.qualify(profile_path, rec_profile, baseline, candidate, publication, dependencies, contract_tests, root / "contract-test-regression")
        if result["state"] != "BLOCKED" or result["gates"]["FND3"] != "BLOCKED" or not result["contract_test_findings"]:
            raise RuntimeError("Foundation qualifier accepted incomplete contract tests")
        write_contract_tests(tool, contract_tests, profile, candidate_commit)

        invalid_publication = tool.read_json(publication); invalid_publication["upstream_head"] = "c" * 40
        tool.write_json(publication, invalid_publication)
        result = tool.qualify(profile_path, rec_profile, baseline, candidate, publication, dependencies, contract_tests, root / "publication-regression")
        if result["state"] != "BLOCKED" or result["gates"]["FND0"] != "BLOCKED":
            raise RuntimeError("Foundation qualifier accepted an unaligned candidate")

        tool.write_json(publication, {"schema_version": 1, "kind": "foundation-candidate-publication",
                                      "branch": "foundation/end-to-end", "head": candidate_commit,
                                      "upstream": "origin/foundation/end-to-end", "upstream_head": candidate_commit,
                                      "tree_clean": True})
        prepared_path = candidate / "control/prepared-manifest.json"; prepared = tool.read_json(prepared_path)
        prepared["candidate"]["source_inventory"].append({"path": "src/core/numeric.cpp", "sha256": "4" * 64})
        tool.write_json(prepared_path, prepared)
        result = tool.qualify(profile_path, rec_profile, baseline, candidate, publication, dependencies, contract_tests, root / "scope-investigation")
        protected = next((row for row in result["scope_changes"] if row["path"] == "src/core/numeric.cpp"), None)
        if result["state"] != "BLOCKED" or result["gates"]["FND0"] != "BLOCKED" or protected is None or protected["classification"] != "INVESTIGATION_REQUIRED":
            raise RuntimeError("Foundation qualifier accepted an undeclared scientific source change")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
