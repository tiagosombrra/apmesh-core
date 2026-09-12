#!/usr/bin/env python3
"""Disposable-repository lifecycle tests; never prepares a scientific candidate."""
from __future__ import annotations

import argparse
import importlib.util
import pathlib
import shutil
import subprocess
import tempfile


def load(path):
    spec = importlib.util.spec_from_file_location("la_runner", path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def git(root, *argv):
    result = subprocess.run(["git", "-c", "user.name=Contract Test", "-c", "user.email=contract@example.invalid", *argv], cwd=root, capture_output=True, text=True)
    if result.returncode: raise RuntimeError(result.stderr)
    return result.stdout.strip()


def sandbox(parent, original):
    """Actual local commit + local upstream, deliberately failing CMake input."""
    seed = parent / "seed"; seed.mkdir()
    for folder in ("include", "src", "tools", "tests", "experiments", "docs"):
        shutil.copytree(original / folder, seed / folder, ignore=shutil.ignore_patterns("__pycache__", "*.pyc"))
    for name in ("CMakeLists.txt", "CMakePresets.json"):
        shutil.copyfile(original / name, seed / name)
    (seed / ".gitignore").write_text("__pycache__/\n*.pyc\n", encoding="utf-8")
    (seed / ".gitattributes").write_text("* text=auto eol=lf\n", encoding="utf-8")
    with (seed / "CMakeLists.txt").open("a", encoding="utf-8") as stream:
        stream.write('\nmessage(FATAL_ERROR "LA_TEST_EXPECTED_CONFIGURE_FAILURE")\n')
    git(seed, "init", "-b", "test-candidate")
    git(seed, "add", ".")
    git(seed, "commit", "-m", "test: disposable lifecycle fixture")
    root = parent / "candidate"
    git(parent, "clone", "--quiet", str(seed), str(root))
    runner = load(root / "tools/run_minimal_small_linear_algebra_qualification.py")
    args = argparse.Namespace(source_root=str(root), output_root=str(parent / "attempt"),
                              profile=str(root / "experiments/profiles/minimal_small_linear_algebra.json"),
                              protocol=str(root / "docs/decisions/GEOMETRY_MINIMAL_SMALL_LINEAR_ALGEBRA_QUALIFICATION_PROTOCOL.md"))
    return runner, args, root


def rejects(action, message):
    try: action()
    except RuntimeError: return
    raise RuntimeError(message)


def lifecycle_checks(original, original_build):
    with tempfile.TemporaryDirectory(prefix="apmesh-la-lifecycle-test-") as temporary:
        runner, args, root = sandbox(pathlib.Path(temporary), original)
        output = pathlib.Path(args.output_root)
        runner.prepare(args)
        runner.load_prepared(args)
        for name in ("prepared-manifest.json", "plan.json", "planned-inventories.json", "state.json", "state-history.jsonl", "preparation-seal.json"):
            path = output / name; original_bytes = path.read_bytes()
            path.write_bytes(original_bytes.replace(b'"schema_version":', b'"altered_schema_version":', 1))
            rejects(lambda: runner.load_prepared(args), f"mutation accepted: {name}")
            path.write_bytes(original_bytes)
        path = output / "execution-claim.json"; path.write_text("{}", encoding="utf-8")
        rejects(lambda: runner.load_prepared(args), "consumed attempt accepted")
        path.unlink()
        dirty = root / "untracked.txt"; dirty.write_text("dirty", encoding="utf-8")
        rejects(lambda: runner.load_prepared(args), "dirty candidate accepted")
        dirty.unlink()
        git(root, "config", "--unset", "branch.test-candidate.remote")
        rejects(lambda: runner.load_prepared(args), "unpublished candidate accepted")
        git(root, "config", "branch.test-candidate.remote", "origin")
        profile = runner.validate_profile(pathlib.Path(args.profile))
        rejects(lambda: runner.validate_prerequisite_discovery(profile["exact_prerequisite_tests"][:-1], profile["exact_prerequisite_tests"]), "missing prerequisite accepted")
        rejects(lambda: runner.validate_prerequisite_discovery(profile["exact_prerequisite_tests"] + ["unexpected"], profile["exact_prerequisite_tests"]), "additional prerequisite accepted")
        rejects(lambda: runner.execute(args), "intentional configure failure did not fail")
        terminal = runner.read_json(output / "terminal-manifest.json")
        if terminal["state"] != "BLOCKED": raise RuntimeError("failed execution is not terminal")
        records = runner.read_json(output / "command-records.json")["records"]
        if len(records) != 2 or records[-1]["exit_code"] == 0 or not records[-1]["pid"]:
            raise RuntimeError("real configure failure provenance is absent")
        stderr = (output / records[-1]["stderr"]["path"]).read_text(encoding="utf-8")
        if "LA_TEST_EXPECTED_CONFIGURE_FAILURE" not in stderr: raise RuntimeError("wrong failure exercised")
        runner.verify_retention(args)
        relocated = pathlib.Path(temporary) / "relocated"
        shutil.copytree(output, relocated, ignore=shutil.ignore_patterns("build"))
        runner.verify_retention(argparse.Namespace(output_root=str(relocated)))
        before = {path.relative_to(output).as_posix(): path.read_bytes() for path in output.rglob("*") if path.is_file()}
        rejects(lambda: runner.execute(args), "failed attempt was reexecuted")
        after = {path.relative_to(output).as_posix(): path.read_bytes() for path in output.rglob("*") if path.is_file()}
        if before != after: raise RuntimeError("reuse mutated terminal evidence")
        for name in ("terminal-manifest.json", "detached-verification.json", "failure.json", "command-records.json"):
            path = output / name; data = path.read_bytes(); path.write_text("{}", encoding="utf-8")
            rejects(lambda: runner.verify_retention(args), f"retained mutation accepted: {name}")
            path.write_bytes(data)
        # Recomputed file hashes do not excuse contradictions in candidate, lifecycle or logs.
        for name, field, value in (("terminal-manifest.json", "candidate", {}), ("detached-verification.json", "candidate_commit", "0" * 40), ("failure.json", "records", [])):
            path = output / name; data = path.read_bytes()
            retention_path = output / "retention-manifest.json"; retention_bytes = retention_path.read_bytes()
            altered = runner.read_json(path); altered[field] = value; runner.write_json(path, altered)
            retained = runner.read_json(retention_path)
            for row in retained["files"]:
                if row["path"] == name: row.update(sha256=runner.sha256_file(path), size=path.stat().st_size)
            runner.write_json(retention_path, retained)
            rejects(lambda: runner.verify_retention(args), f"rehashed inconsistent evidence accepted: {name}")
            path.write_bytes(data); retention_path.write_bytes(retention_bytes)
        # Exercise compiler dependency parsing with actual Ninja output in the invoking build.
        if not (original_build / "build.ninja").is_file(): raise RuntimeError("focused build root is absent")
        if original_build.is_dir():
            record = runner.run_command(["ninja", "-C", str(original_build), "-t", "deps"], original, pathlib.Path(temporary) / "dep-logs", "observed-deps", 30)
            runner.require_success(record, "focused dependency discovery")
            runner.validate_observed_dependencies(record, pathlib.Path(temporary))
            path = pathlib.Path(temporary) / record["stdout"]["path"]
            path.write_text(path.read_text(encoding="utf-8").replace("src/math/linear_algebra.cpp.o:", "src/math/missing.cpp.o:"), encoding="utf-8")
            record["stdout"]["sha256"] = runner.sha256_file(path)
            rejects(lambda: runner.validate_observed_dependencies(record, pathlib.Path(temporary)), "missing compiled math dependency accepted")
        print("PASS: preparation mutation, publication, discovery, actual subprocess failure, terminal retention and single use")


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--runner", required=True); parser.add_argument("--source-root", required=True)
    parser.add_argument("--profile", required=True); parser.add_argument("--protocol", required=True)
    parser.add_argument("--build-root", required=True)
    args = parser.parse_args()
    lifecycle_checks(pathlib.Path(args.source_root), pathlib.Path(args.build_root))
    return 0


if __name__ == "__main__": raise SystemExit(main())
