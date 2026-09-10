#!/usr/bin/env python3

import argparse
import pathlib
import subprocess
import sys
import tempfile


def run(command: list[str], cwd: pathlib.Path) -> None:
    subprocess.run(command, cwd=cwd, check=True, capture_output=True)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--runner", required=True)
    parser.add_argument("--runtime", required=True)
    arguments = parser.parse_args()
    sys.path.insert(0, str(pathlib.Path(arguments.runner).resolve().parent))
    import run_geometry_point_vector_qualification as runner
    from experiment_runtime import RuntimeErrorEvidence, clean_candidate, read_json, sha256_file, write_json, write_state

    with tempfile.TemporaryDirectory() as temporary_directory:
        root = pathlib.Path(temporary_directory)
        source = root / "candidate"; source.mkdir()
        (source / "tracked.txt").write_text("candidate\n", encoding="utf-8")
        run(["git", "init"], source)
        run(["git", "config", "user.email", "test@example.invalid"], source)
        run(["git", "config", "user.name", "Test"], source)
        run(["git", "add", "tracked.txt"], source)
        run(["git", "commit", "-m", "candidate"], source)
        candidate = clean_candidate(source)
        candidate["upstream_commit"] = candidate["commit"]
        output = root / "retained"
        output.mkdir()
        planned_compile_commands = output / "cells" / "gcc-debug" / "build" / "compile_commands.json"
        planned_compile_commands.parent.mkdir(parents=True)
        planned_compile_commands.write_text("[]\n", encoding="utf-8")
        planned_executable = output / "cells" / "gcc-debug" / "build" / "apmesh_core_geometry_point_vector_export"
        planned_executable.write_text("fixture\n", encoding="utf-8")
        dependency = {
            "executable": "apmesh_core_geometry_point_vector_export",
            "path": "cells/gcc-debug/build/apmesh_core_geometry_point_vector_export",
            "sha256": sha256_file(planned_executable),
            "ldd_record": {"id": "ldd-apmesh_core_geometry_point_vector_export", "exit_code": 0,
                           "timed_out": False, "launch_error": None},
        }
        inventories = [{"cell": "gcc-debug", "compile_commands": {"path": "cells/gcc-debug/build/compile_commands.json",
                                                                          "sha256": sha256_file(planned_compile_commands)},
                        "runtime_dependencies": [dependency]}]
        runtime_inventory = {"schema_version": 1, "kind": "geometry-point-vector-runtime-dependencies",
                             "candidate_commit": candidate["commit"], "cells": [{"cell": "gcc-debug", "dependencies": [dependency]}]}
        write_json(output / "runtime-dependencies.json", runtime_inventory)
        prepared = {"schema_version": 3, "kind": "geometry-point-vector-prepared-manifest", "candidate": candidate,
                    "planned_inventories": {"schema_version": 1, "kind": "geometry-point-vector-planned-inventories",
                                              "cells": [{"cell": "gcc-debug", "compile_commands_path": "cells/gcc-debug/build/compile_commands.json",
                                                         "runtime_dependency_executables": ["apmesh_core_geometry_point_vector_export"]}]}}
        write_json(output / "prepared-manifest.json", prepared)
        write_state(output, "PREPARED", {"candidate_commit": candidate["commit"]})
        write_state(output, "RUNNING", {"candidate_commit": candidate["commit"]})
        terminal = {
            "schema_version": 3,
            "kind": "geometry-point-vector-terminal-manifest",
            "state": "EXECUTED_PENDING_AUDIT",
            "candidate": candidate,
            "prepared_manifest_sha256": sha256_file(output / "prepared-manifest.json"),
            "execution": {"observed_inventories": inventories,
                          "runtime_dependencies": {"path": "runtime-dependencies.json", "sha256": sha256_file(output / "runtime-dependencies.json")}},
            "retention_manifest": "retention-manifest.json",
        }
        write_json(output / "terminal-manifest.json", terminal)
        write_state(output, "EXECUTED_PENDING_AUDIT", {"candidate_commit": candidate["commit"]})
        runner.seal_output(output, source, prepared)
        runner.verify_retention(output, source)

        legacy_dependency = {
            **dependency,
            "ldd_record": {"stage": "ldd-apmesh_core_geometry_point_vector_export", "exit_code": 0,
                           "timed_out": False, "launch_error": None},
        }
        legacy_inventories = [{"cell": "gcc-debug", "compile_commands": inventories[0]["compile_commands"],
                               "runtime_dependencies": [legacy_dependency]}]
        legacy_runtime_inventory = {"schema_version": 1, "kind": "geometry-point-vector-runtime-dependencies",
                                    "candidate_commit": candidate["commit"],
                                    "cells": [{"cell": "gcc-debug", "dependencies": [legacy_dependency]}]}
        try:
            runner.verify_observed_inventories(output, prepared, legacy_inventories, legacy_runtime_inventory,
                                               output / "runtime-dependencies.json")
        except RuntimeErrorEvidence:
            pass
        else:
            raise RuntimeError("runtime dependency verifier accepted legacy stage instead of command id")

        (output / "unexpected.txt").write_text("tamper\n", encoding="utf-8")
        try:
            runner.verify_retention(output, source)
        except runner.RuntimeErrorEvidence:
            pass
        else:
            raise RuntimeError("retention verifier accepted an unsealed artifact")

        closure = root / "closure"; closure.mkdir()
        write_state(closure, "PREPARED", {})
        write_state(closure, "RUNNING", {})
        write_state(closure, "EXECUTED_PENDING_AUDIT", {})
        try:
            write_state(closure, "BLOCKED", {"reason": "missing closure marker"})
        except RuntimeErrorEvidence:
            pass
        else:
            raise RuntimeError("closure failure transition did not require an explicit marker")
        write_state(closure, "BLOCKED", {"reason": "simulated retention failure", "closure_failure": True})
        if read_json(closure / "state.json")["state"] != "BLOCKED":
            raise RuntimeError("explicit retention failure did not produce a BLOCKED lifecycle state")
        return 0
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
