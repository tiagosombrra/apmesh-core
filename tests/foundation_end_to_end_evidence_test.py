"""Focused contract for Foundation preparation readiness and closure separation."""

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
    module = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module


def run(arguments: list[str], cwd: pathlib.Path | None = None) -> str:
    completed = subprocess.run(arguments, cwd=cwd, check=False, capture_output=True, text=True)
    if completed.returncode != 0:
        raise RuntimeError(f"fixture command failed: {arguments}: {completed.stderr.strip()}")
    return completed.stdout.strip()


def sha256(path: pathlib.Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def require_rejection(action, message: str) -> None:
    try:
        action()
    except Exception:
        return
    raise RuntimeError(message)


def require_blocked(tool, gate: str, *arguments, **keywords) -> None:
    result = tool.preflight(*arguments, **keywords)
    if result["state"] != tool.BLOCKED or result["gates"][gate] != tool.BLOCKED:
        raise RuntimeError(f"Foundation preflight did not block {gate}")


def bind_profile(tool, profile: dict, source: pathlib.Path, package: pathlib.Path) -> dict:
    profile["accepted_baseline"] = {
        "path": str(package),
        "candidate_commit": tool.read_json(package / "retention-manifest.json")["candidate_commit"],
        "retention_manifest_sha256": sha256(package / "retention-manifest.json"),
    }
    for row in profile["scope_policy"]["approved_support_paths"]:
        item = source / row["path"]
        if not item.is_file():
            raise RuntimeError("Foundation support fixture is absent")
        row["candidate_sha256"] = sha256(item)
    for row in profile["qualified_authorities"]:
        item = source / row["path"]
        if not item.is_file() or row["qualified_marker"] not in item.read_text(encoding="utf-8"):
            raise RuntimeError("Foundation qualified authority fixture is absent")
        row["sha256"] = sha256(item)
    for name in ("runner", "report_only_tool", "report_only_contract"):
        row = profile["preparation_preflight"][name]
        item = source / row["path"]
        if not item.is_file():
            raise RuntimeError("Foundation preflight fixture is absent")
        row["sha256"] = sha256(item)
    return profile


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--tool", required=True)
    parser.add_argument("--profile", required=True)
    parser.add_argument("--rec-profile", required=True)
    arguments = parser.parse_args()
    tool = load_tool(pathlib.Path(arguments.tool))
    repository = pathlib.Path(arguments.profile).resolve().parents[2]
    package = repository / "evidence" / "foundation" / "reproducible-experiment-contract" / "rec-e0-e7-85d215a"
    retained_verifications: list[tuple[pathlib.Path, pathlib.Path]] = []

    def record_retained_verification(observed_package: pathlib.Path, observed_profile: pathlib.Path) -> None:
        retained_verifications.append((observed_package.resolve(), observed_profile.resolve()))

    tool.verify_retained_package = record_retained_verification

    with tempfile.TemporaryDirectory(prefix="apmesh-core-foundation-preflight-") as temporary:
        root = pathlib.Path(temporary)
        remote, source = root / "remote.git", root / "source"
        run(["git", "init", "--bare", str(remote)])
        run(["git", "push", str(remote), "HEAD:refs/heads/foundation/test"], repository)
        run(["git", "-c", "core.longpaths=true", "clone", "--branch", "foundation/test", str(remote), str(source)])

        profile_path = source / "experiments" / "profiles" / "foundation_end_to_end.json"
        profile = bind_profile(tool, copy.deepcopy(tool.read_json(pathlib.Path(arguments.profile))), source, package)
        tool.write_json(profile_path, profile)
        run(["git", "add", "experiments/profiles/foundation_end_to_end.json"], source)
        run(["git", "-c", "user.name=Foundation Contract", "-c", "user.email=foundation@example.invalid",
             "commit", "-m", "test: bind Foundation preflight fixture"], source)
        run(["git", "push"], source)

        rec_profile = source / "experiments" / "profiles" / "reproducible_experiment_contract.json"
        control, evidence = root / "control", root / "evidence"
        result = tool.preflight(profile_path, rec_profile, package, source, control, evidence)
        if (result["state"] != tool.PENDING or result["execution_authorization"] is not False or
                any(result["gates"][gate] != tool.PENDING for gate in tool.PREPARATION_GATES)):
            raise RuntimeError("Foundation preflight did not retain audit-only readiness evidence")
        if retained_verifications != [(package.resolve(), rec_profile.resolve())]:
            raise RuntimeError("Foundation preflight did not request historical REC verification")

        require_rejection(
            lambda: tool.qualify(profile_path, rec_profile, package, package, root / "unused-inputs.json", root / "unused-output"),
            "Foundation accepted the historical REC baseline as its current candidate")

        dirty = source / "preflight-dirty.txt"
        dirty.write_text("dirty\n", encoding="utf-8")
        require_blocked(tool, "FPR0", profile_path, rec_profile, package, source, root / "dirty-control", root / "dirty-evidence")
        dirty.unlink()

        invalid = copy.deepcopy(profile)
        invalid["qualified_authorities"][2]["sha256"] = "0" * 64
        invalid_path = root / "invalid-authority-profile.json"
        tool.write_json(invalid_path, invalid)
        require_blocked(tool, "FPR1", invalid_path, rec_profile, package, source, root / "authority-control", root / "authority-evidence")

        require_blocked(tool, "FPR5", profile_path, rec_profile, package, source, root / "same-root", root / "same-root")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
