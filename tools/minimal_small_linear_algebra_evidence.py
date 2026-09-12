#!/usr/bin/env python3
"""Strict report-only schemas and comparison for LA0-LA7 evidence."""

from __future__ import annotations

import argparse
import hashlib
import json
import pathlib
import sys
from typing import Any


class EvidenceError(RuntimeError):
    pass


def _reject_duplicates(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise EvidenceError(f"duplicate JSON key: {key}")
        result[key] = value
    return result


def read_json(path: pathlib.Path) -> Any:
    try:
        raw = path.read_bytes()
    except OSError as error:
        raise EvidenceError(f"cannot read {path}: {error}") from error
    if raw.startswith(b"\xef\xbb\xbf"):
        raise EvidenceError(f"UTF-8 BOM is not accepted: {path}")
    try:
        return json.loads(raw.decode("utf-8"), object_pairs_hook=_reject_duplicates)
    except (UnicodeDecodeError, json.JSONDecodeError) as error:
        raise EvidenceError(f"invalid JSON: {path}: {error}") from error


def write_json(path: pathlib.Path, value: Any) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes((json.dumps(value, sort_keys=True, separators=(",", ":"), ensure_ascii=True) + "\n").encode("utf-8"))


def sha256(path: pathlib.Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def _exact_keys(value: dict[str, Any], expected: set[str], context: str) -> None:
    if set(value) != expected:
        raise EvidenceError(f"{context} keys differ; missing={sorted(expected - set(value))}, extra={sorted(set(value) - expected)}")


def validate_profile(path: pathlib.Path) -> dict[str, Any]:
    profile = read_json(path)
    if not isinstance(profile, dict):
        raise EvidenceError("profile must be an object")
    _exact_keys(profile, {"schema_version", "kind", "repetitions_per_cell", "cells", "gates", "case_families", "exact_prerequisite_tests", "volatile_fields", "limitations"}, "profile")
    if profile["schema_version"] != 1 or profile["kind"] != "minimal-small-linear-algebra-qualification-profile":
        raise EvidenceError("profile identity differs")
    if profile["repetitions_per_cell"] != 3 or profile["gates"] != [f"LA{index}" for index in range(8)]:
        raise EvidenceError("profile repetitions or gates differ")
    expected_cells = ["gcc-debug", "gcc-release", "clang-debug", "clang-release"]
    if not isinstance(profile["cells"], list) or [cell.get("id") for cell in profile["cells"] if isinstance(cell, dict)] != expected_cells:
        raise EvidenceError("profile cell matrix differs")
    for cell in profile["cells"]:
        if not isinstance(cell, dict):
            raise EvidenceError("profile cell is not an object")
        _exact_keys(cell, {"id", "compiler", "library", "build_type"}, "profile cell")
    for key in ("case_families", "exact_prerequisite_tests", "volatile_fields", "limitations"):
        if not isinstance(profile[key], list) or not profile[key] or any(not isinstance(item, str) or not item for item in profile[key]):
            raise EvidenceError(f"profile {key} differs")
    if len(set(profile["case_families"])) != len(profile["case_families"]) or len(set(profile["exact_prerequisite_tests"])) != len(profile["exact_prerequisite_tests"]):
        raise EvidenceError("profile contains duplicate fixed entries")
    return profile


def validate_certificate(profile: dict[str, Any], path: pathlib.Path) -> dict[str, Any]:
    certificate = read_json(path)
    if not isinstance(certificate, dict):
        raise EvidenceError("certificate must be an object")
    _exact_keys(certificate, {"schema_version", "kind", "environment", "cases"}, "certificate")
    if certificate["schema_version"] != 1 or certificate["kind"] != "minimal-small-linear-algebra-certificate":
        raise EvidenceError("certificate identity differs")
    if not isinstance(certificate["environment"], dict):
        raise EvidenceError("certificate environment differs")
    _exact_keys(certificate["environment"], {"double_radix", "double_digits", "iec559"}, "certificate environment")
    if certificate["environment"] != {"double_radix": 2, "double_digits": 53, "iec559": True}:
        raise EvidenceError("certificate floating environment differs")
    if not isinstance(certificate["cases"], list):
        raise EvidenceError("certificate cases differ")
    expected_families = profile["case_families"]
    observed: dict[str, dict[str, Any]] = {}
    for row in certificate["cases"]:
        if not isinstance(row, dict):
            raise EvidenceError("certificate case is not an object")
        _exact_keys(row, {"family", "status", "expected", "observed", "non_claims"}, "certificate case")
        family = row["family"]
        if not isinstance(family, str) or family in observed:
            raise EvidenceError("certificate family is duplicate or invalid")
        if row["status"] != "PASS" or not isinstance(row["expected"], dict) or not isinstance(row["observed"], dict):
            raise EvidenceError("certificate case is not a complete passing observation")
        if not isinstance(row["non_claims"], list) or any(not isinstance(item, str) for item in row["non_claims"]):
            raise EvidenceError("certificate non-claims differ")
        observed[family] = row
    if list(observed) != expected_families:
        raise EvidenceError("certificate case families differ")
    determinant = observed["determinant_boundary"]
    required_non_claims = {"predicate", "rank", "degeneracy", "orientation", "incidence", "topology"}
    if not required_non_claims.issubset(set(determinant["non_claims"])):
        raise EvidenceError("determinant non-claims differ")
    return certificate


def semantic_projection(profile: dict[str, Any], certificate: dict[str, Any]) -> dict[str, Any]:
    del profile
    return {"environment": certificate["environment"], "cases": certificate["cases"]}


def compare_certificates(profile: dict[str, Any], paths: list[pathlib.Path]) -> dict[str, Any]:
    if len(paths) != 3:
        raise EvidenceError("exactly three certificate paths are required")
    certificates = [validate_certificate(profile, path) for path in paths]
    baseline = semantic_projection(profile, certificates[0])
    if any(semantic_projection(profile, certificate) != baseline for certificate in certificates[1:]):
        raise EvidenceError("certificate semantic projections differ")
    return {"schema_version": 1, "kind": "minimal-small-linear-algebra-comparison", "state": "EVIDENCE_COLLECTED_PENDING_AUDIT", "certificate_sha256": [sha256(path) for path in paths], "projection": baseline}


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    subparsers = parser.add_subparsers(dest="command", required=True)
    profile_parser = subparsers.add_parser("validate-profile")
    profile_parser.add_argument("--profile", required=True)
    certificate_parser = subparsers.add_parser("validate-certificate")
    certificate_parser.add_argument("--profile", required=True)
    certificate_parser.add_argument("--certificate", required=True)
    compare_parser = subparsers.add_parser("compare")
    compare_parser.add_argument("--profile", required=True)
    compare_parser.add_argument("--certificate", action="append", required=True)
    compare_parser.add_argument("--output", required=True)
    arguments = parser.parse_args()
    try:
        profile = validate_profile(pathlib.Path(arguments.profile))
        if arguments.command == "validate-profile":
            return 0
        if arguments.command == "validate-certificate":
            validate_certificate(profile, pathlib.Path(arguments.certificate))
            return 0
        result = compare_certificates(profile, [pathlib.Path(path) for path in arguments.certificate])
        write_json(pathlib.Path(arguments.output), result)
        return 0
    except EvidenceError as error:
        print(f"evidence error: {error}", file=sys.stderr)
        return 2


if __name__ == "__main__":
    raise SystemExit(main())
