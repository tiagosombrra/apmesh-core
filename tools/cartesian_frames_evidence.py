#!/usr/bin/env python3
"""Independent, report-only validation and comparison for Cartesian frames."""
from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
import pathlib
import subprocess
import sys
from typing import Any


class EvidenceError(RuntimeError):
    pass


def reject_duplicates(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise EvidenceError(f"duplicate JSON key: {key}")
        result[key] = value
    return result


def read_json(path: pathlib.Path) -> dict[str, Any]:
    try:
        value = json.loads(path.read_text(encoding="utf-8"), object_pairs_hook=reject_duplicates)
    except (OSError, json.JSONDecodeError) as error:
        raise EvidenceError(f"cannot read {path}: {error}") from error
    if not isinstance(value, dict):
        raise EvidenceError(f"{path}: expected JSON object")
    return value


def write_json(path: pathlib.Path, value: Any) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def sha256(path: pathlib.Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def require_keys(value: dict[str, Any], expected: set[str], where: str) -> None:
    if set(value) != expected:
        raise EvidenceError(f"{where}: expected keys {sorted(expected)}, got {sorted(value)}")


def canonical_hex(number: float) -> str:
    if not math.isfinite(number):
        raise EvidenceError("non-finite certificate value")
    if number == 0.0:
        return "0x0.0p+0"
    return number.hex()


def values(items: list[Any], where: str) -> list[str]:
    if not isinstance(items, list):
        raise EvidenceError(f"{where}: value must be list")
    output: list[str] = []
    for item in items:
        if not isinstance(item, str):
            raise EvidenceError(f"{where}: values must be hex strings")
        try:
            output.append(canonical_hex(float.fromhex(item)))
        except ValueError as error:
            raise EvidenceError(f"{where}: invalid hex {item}") from error
    return output


EXPECTED: dict[str, tuple[str, str | None, list[str] | None]] = {
    "identity2_point": ("value", None, ["0x1.0000000000000p+0", "0x1.0000000000000p+1"]),
    "identity2_vector": ("value", None, ["0x1.0000000000000p+0", "0x1.0000000000000p+1"]),
    "quarter2_axis_x": ("value", None, ["0x0.0p+0", "0x1.0000000000000p+1"]),
    "quarter2_axis_y": ("value", None, ["-0x1.0000000000000p+1", "0x0.0p+0"]),
    "cycle3_axis_x": ("value", None, ["0x0.0p+0", "0x0.0p+0", "0x1.0000000000000p-1"]),
    "cycle3_axis_y": ("value", None, ["0x1.0000000000000p-1", "0x0.0p+0", "0x0.0p+0"]),
    "cycle3_axis_z": ("value", None, ["0x0.0p+0", "0x1.0000000000000p-1", "0x0.0p+0"]),
    "reflection2_vector": ("value", None, ["-0x1.0000000000000p+0", "0x1.0000000000000p+1"]),
    "scale_m8": ("value", None, ["0x1.0000000000000p-8", "0x0.0p+0"]),
    "scale_m1": ("value", None, ["0x1.0000000000000p-1", "0x0.0p+0"]),
    "scale_0": ("value", None, ["0x1.0000000000000p+0", "0x0.0p+0"]),
    "scale_1": ("value", None, ["0x1.0000000000000p+1", "0x0.0p+0"]),
    "scale_8": ("value", None, ["0x1.0000000000000p+8", "0x0.0p+0"]),
    "roundtrip2_point": ("value", None, ["0x1.0000000000000p+0", "0x1.0000000000000p+1"]),
    "roundtrip2_vector": ("value", None, ["0x1.0000000000000p+0", "0x1.0000000000000p+1"]),
    "roundtrip3_point": ("value", None, ["0x1.0000000000000p+0", "0x1.0000000000000p+1", "0x1.8000000000000p+1"]),
    "roundtrip3_vector": ("value", None, ["0x1.0000000000000p+0", "0x1.0000000000000p+1", "0x1.8000000000000p+1"]),
    "affine2": ("value", None, ["0x0.0p+0", "0x0.0p+0"]),
    "difference2": ("value", None, ["-0x1.0000000000000p+2", "0x1.0000000000000p+1"]),
    "metric_scale2": ("value", None, ["0x1.0000000000000p+2"]),
    "compile_time_dimension_separation": ("compile_time_rejection", None, []),
    "reject_duplicate_basis2": ("error", "invalid_frame", None),
    "reject_nonunit_basis2": ("error", "invalid_frame", None),
    "reject_missing_basis2": ("error", "invalid_frame", None),
    "mat2_nonfinite_rejection": ("error", "non_finite_input", None),
    "reject_scale_high": ("error", "scale_out_of_range", None),
    "reject_scale_low": ("error", "scale_out_of_range", None),
    "overflow_vector2": ("error", "non_finite_result", None),
}


def validate_profile(path: pathlib.Path) -> dict[str, Any]:
    profile = read_json(path)
    require_keys(profile, {"schema_version", "kind", "status", "repetitions_per_cell", "cells", "gates", "cases", "negative_cases", "scale_exponents", "exact_prerequisite_tests", "equivalence", "limitations"}, "profile")
    if profile["schema_version"] != 1 or profile["kind"] != "cartesian-frames-qualification-profile":
        raise EvidenceError("profile schema/kind mismatch")
    if profile["status"] != "report_only_infrastructure" or profile["repetitions_per_cell"] != 3:
        raise EvidenceError("profile is not the fixed report-only 3-repeat definition")
    if profile["gates"] != [f"CF{i}" for i in range(8)]:
        raise EvidenceError("profile gates must be CF0..CF7")
    if profile["cases"] != list(EXPECTED):
        raise EvidenceError("profile case list differs from independent oracle")
    if profile["scale_exponents"] != [-8, -1, 0, 1, 8]:
        raise EvidenceError("profile scale exponents differ")
    if profile["equivalence"] != {"rule": "exact_hex", "signed_zero_policy": "normalize_to_positive"}:
        raise EvidenceError("profile equivalence policy differs")
    cells = profile["cells"]
    if not isinstance(cells, list) or [cell.get("id") for cell in cells] != ["gcc-debug", "gcc-release", "clang-debug", "clang-release"]:
        raise EvidenceError("profile must enumerate the fixed four-cell matrix")
    return profile


def validate_certificate(profile: dict[str, Any], path: pathlib.Path) -> dict[str, Any]:
    certificate = read_json(path)
    require_keys(certificate, {"schema_version", "kind", "signed_zero_policy", "cases"}, "certificate")
    if certificate["schema_version"] != 1 or certificate["kind"] != "cartesian-frames-certificate":
        raise EvidenceError("certificate schema/kind mismatch")
    if certificate["signed_zero_policy"] != profile["equivalence"]["signed_zero_policy"]:
        raise EvidenceError("signed-zero policy mismatch")
    cases = certificate["cases"]
    if not isinstance(cases, list) or len(cases) != len(EXPECTED):
        raise EvidenceError("certificate case count mismatch")
    observed_ids: list[str] = []
    canonical_cases: list[dict[str, Any]] = []
    for case in cases:
        if not isinstance(case, dict):
            raise EvidenceError("certificate case is not object")
        require_keys(case, {"id", "outcome", "error", "value"}, "certificate case")
        case_id = case["id"]
        if not isinstance(case_id, str) or case_id in observed_ids or case_id not in EXPECTED:
            raise EvidenceError("duplicate or unknown certificate case")
        observed_ids.append(case_id)
        outcome, error, expected = EXPECTED[case_id]
        if case["outcome"] != outcome or case["error"] != error:
            raise EvidenceError(f"{case_id}: outcome/error differs from independent oracle")
        if expected is None:
            if case["value"] is not None:
                raise EvidenceError(f"{case_id}: error case has value")
            canonical = None
        else:
            canonical = values(case["value"], case_id)
            if canonical != expected:
                raise EvidenceError(f"{case_id}: exact value differs from independent oracle")
        canonical_cases.append({"id": case_id, "outcome": outcome, "error": error, "value": canonical})
    if observed_ids != profile["cases"]:
        raise EvidenceError("certificate order or coverage differs from profile")
    return {"schema_version": 1, "kind": "cartesian-frames-canonical-certificate", "cases": canonical_cases}


def compare(profile: dict[str, Any], index_path: pathlib.Path, report_path: pathlib.Path) -> None:
    index = read_json(index_path)
    require_keys(index, {"certificates"}, "comparison index")
    raw_paths = index["certificates"]
    if not isinstance(raw_paths, list) or not raw_paths:
        raise EvidenceError("comparison needs at least one certificate")
    projections = [validate_certificate(profile, pathlib.Path(item)) for item in raw_paths]
    first = json.dumps(projections[0], sort_keys=True, separators=(",", ":"))
    if any(json.dumps(item, sort_keys=True, separators=(",", ":")) != first for item in projections[1:]):
        raise EvidenceError("certificate projections differ")
    write_json(report_path, {"schema_version": 1, "kind": "cartesian-frames-comparison", "status": "report_only", "certificate_count": len(projections), "equivalent": True})


def negative_self_check(profile: dict[str, Any], certificate_path: pathlib.Path, output: pathlib.Path) -> None:
    certificate = read_json(certificate_path)
    mutations: list[tuple[str, dict[str, Any]]] = []
    duplicate = copy.deepcopy(certificate); duplicate["cases"].append(copy.deepcopy(duplicate["cases"][0])); mutations.append(("duplicate_case", duplicate))
    forged = copy.deepcopy(certificate); forged["cases"][0]["value"][0] = "0x1.0000000000001p+0"; mutations.append(("forged_value", forged))
    forged_error = copy.deepcopy(certificate); forged_error["cases"][-1]["error"] = "invalid_frame"; mutations.append(("forged_error", forged_error))
    missing = copy.deepcopy(certificate); missing["cases"].pop(); mutations.append(("missing_case", missing))
    wrong_policy = copy.deepcopy(certificate); wrong_policy["signed_zero_policy"] = "preserve"; mutations.append(("wrong_signed_zero_policy", wrong_policy))
    rejected: list[str] = []
    for name, mutation in mutations:
        candidate = output.parent / f"{name}.json"; write_json(candidate, mutation)
        try:
            validate_certificate(profile, candidate)
        except EvidenceError:
            rejected.append(name)
        else:
            raise EvidenceError(f"negative mutation accepted: {name}")
    write_json(output, {"schema_version": 1, "kind": "cartesian-frames-negative-outcomes", "rejected": rejected, "baseline_sha256": sha256(certificate_path), "validator_sha256": sha256(pathlib.Path(__file__))})


def main() -> int:
    parser = argparse.ArgumentParser()
    sub = parser.add_subparsers(dest="command", required=True)
    for name in ("validate-profile", "validate-certificate"):
        item = sub.add_parser(name); item.add_argument("--profile", type=pathlib.Path, required=True)
        if name == "validate-certificate": item.add_argument("--certificate", type=pathlib.Path, required=True)
    item = sub.add_parser("compare"); item.add_argument("--profile", type=pathlib.Path, required=True); item.add_argument("--index", type=pathlib.Path, required=True); item.add_argument("--report", type=pathlib.Path, required=True)
    item = sub.add_parser("negative-self-check"); item.add_argument("--profile", type=pathlib.Path, required=True); item.add_argument("--certificate", type=pathlib.Path, required=True); item.add_argument("--output", type=pathlib.Path, required=True)
    args = parser.parse_args()
    try:
        profile = validate_profile(args.profile)
        if args.command == "validate-certificate": validate_certificate(profile, args.certificate)
        elif args.command == "compare": compare(profile, args.index, args.report)
        elif args.command == "negative-self-check": negative_self_check(profile, args.certificate, args.output)
    except EvidenceError as error:
        print(f"cartesian-frames evidence: {error}", file=sys.stderr); return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
