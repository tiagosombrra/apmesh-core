#!/usr/bin/env python3
"""Validate and compare report-only Point/Vector qualification evidence."""

from __future__ import annotations

import argparse
import json
import math
import pathlib
import re
import sys
from typing import Any


class EvidenceError(RuntimeError):
    pass


UNSAFE_FLOATING_FLAGS = (
    "-ffast-math", "-ofast", "-fassociative-math", "-funsafe-math-optimizations",
    "-ffinite-math-only", "-fno-signed-zeros", "-fno-trapping-math", "/fp:fast",
)

CONFIGURATIONS = [
    ("gcc-debug", "GCC 13", "libstdc++", "Debug"),
    ("gcc-release", "GCC 13", "libstdc++", "Release"),
    ("clang-debug", "Clang 18", "libc++", "Debug"),
    ("clang-release", "Clang 18", "libc++", "Release"),
]

SEPARATION = {
    "compiled_contract": True,
    "point_vector_conversion": False,
    "point_plus_point": False,
    "scalar_times_point": False,
    "vector_scaling": True,
    "point_displacement_vector": True,
}


def reject_duplicate_keys(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    value: dict[str, Any] = {}
    for key, item in pairs:
        if key in value:
            raise EvidenceError(f"duplicate JSON key: {key}")
        value[key] = item
    return value


def read_json(path: pathlib.Path) -> Any:
    try:
        raw = path.read_bytes()
    except OSError as error:
        raise EvidenceError(f"cannot read {path}: {error}") from error
    if raw.startswith(b"\xef\xbb\xbf"):
        raise EvidenceError(f"UTF-8 BOM is not accepted: {path}")
    try:
        return json.loads(raw.decode("utf-8"), object_pairs_hook=reject_duplicate_keys)
    except (UnicodeDecodeError, json.JSONDecodeError) as error:
        raise EvidenceError(f"invalid JSON: {path}: {error}") from error


def require_keys(value: dict[str, Any], expected: set[str], context: str) -> None:
    if set(value) != expected:
        raise EvidenceError(f"{context} keys differ; missing={sorted(expected - set(value))}, extra={sorted(set(value) - expected)}")


def as_hex(value: Any, context: str) -> float:
    if not isinstance(value, str):
        raise EvidenceError(f"{context} must be a hexadecimal floating string")
    try:
        parsed = float.fromhex(value)
    except ValueError as error:
        raise EvidenceError(f"{context} is not hexadecimal floating text") from error
    if not math.isfinite(parsed):
        raise EvidenceError(f"{context} must be finite")
    return parsed


def require_values(value: Any, expected: list[float], context: str, *, approximate: bool = False) -> None:
    if not isinstance(value, list) or len(value) != len(expected):
        raise EvidenceError(f"{context} value arity differs")
    for index, (observed, wanted) in enumerate(zip(value, expected, strict=True)):
        parsed = as_hex(observed, f"{context}[{index}]")
        if approximate:
            if not math.isclose(parsed, wanted, rel_tol=2.0 ** -52, abs_tol=2.0 ** -52):
                raise EvidenceError(f"{context}[{index}] violates the declared proximity bound")
        elif parsed != wanted:
            raise EvidenceError(f"{context}[{index}] differs")


def row(cases: dict[str, dict[str, Any]], case_id: str) -> dict[str, Any]:
    try:
        return cases[case_id]
    except KeyError as error:
        raise EvidenceError(f"certificate omits required case: {case_id}") from error


def require_value(cases: dict[str, dict[str, Any]], case_id: str, expected: list[float], *, approximate: bool = False) -> None:
    value = row(cases, case_id)
    if value["outcome"] != "value" or value["error"] is not None:
        raise EvidenceError(f"{case_id} must be a successful value")
    require_values(value["value"], expected, case_id, approximate=approximate)


def require_error(cases: dict[str, dict[str, Any]], case_id: str, expected: str) -> None:
    value = row(cases, case_id)
    if value["outcome"] != "error" or value["error"] != expected or value["value"] is not None:
        raise EvidenceError(f"{case_id} error classification differs")


def validate_profile(path: pathlib.Path) -> dict[str, Any]:
    value = read_json(path)
    if not isinstance(value, dict):
        raise EvidenceError("profile must be a JSON object")
    require_keys(value, {"protocol_version", "kind", "repetitions_per_configuration", "configurations", "gates", "claim_fields", "volatile_fields", "focused_ctest_label", "foundation_ctest_label", "limitations"}, "profile")
    configurations = value["configurations"]
    expected_configurations = [
        {"name": name, "compiler": compiler, "standard_library": library, "build_type": build}
        for name, compiler, library, build in CONFIGURATIONS
    ]
    if value["protocol_version"] != 1 or value["kind"] != "geometry-point-vector-qualification-profile":
        raise EvidenceError("profile identity differs")
    if value["repetitions_per_configuration"] != 3 or configurations != expected_configurations:
        raise EvidenceError("profile execution matrix differs")
    if value["gates"] != ["PV0", "PV1", "PV2", "PV3", "PV4", "PV5", "PV6", "PV7"]:
        raise EvidenceError("profile gates differ")
    if value["focused_ctest_label"] != "geometry" or value["foundation_ctest_label"] != "bootstrap|numeric|reproducibility|foundation":
        raise EvidenceError("profile CTest labels differ")
    if not isinstance(value["claim_fields"], list) or not isinstance(value["volatile_fields"], list) or not isinstance(value["limitations"], list):
        raise EvidenceError("profile lists are malformed")
    return value


def validate_compile_commands(path: pathlib.Path) -> None:
    value = read_json(path)
    if not isinstance(value, list):
        raise EvidenceError("compile commands must be a list")
    commands = []
    for entry in value:
        if not isinstance(entry, dict):
            continue
        file_name = str(entry.get("file", "")).replace("\\", "/")
        if not file_name.endswith("src/core/geometry.cpp"):
            continue
        command = entry.get("command")
        if isinstance(command, str):
            commands.append(command)
        elif isinstance(entry.get("arguments"), list) and all(isinstance(part, str) for part in entry["arguments"]):
            commands.append(" ".join(entry["arguments"]))
    if len(commands) != 1:
        raise EvidenceError("geometry source compile command is absent or ambiguous")
    lowered = commands[0].lower()
    if "-std=c++23" not in lowered or any(flag in lowered for flag in UNSAFE_FLOATING_FLAGS):
        raise EvidenceError("geometry source compile command violates the numeric envelope")


def validate_certificate(path: pathlib.Path) -> dict[str, Any]:
    value = read_json(path)
    if not isinstance(value, dict):
        raise EvidenceError("certificate must be a JSON object")
    require_keys(value, {"schema_version", "kind", "environment", "separation", "cases"}, "certificate")
    if value["schema_version"] != 1 or value["kind"] != "geometry-point-vector-certificate":
        raise EvidenceError("certificate identity differs")
    if value["environment"] != {"double_radix": 2, "double_digits": 53, "iec559": True}:
        raise EvidenceError("certificate numeric environment differs")
    if value["separation"] != SEPARATION:
        raise EvidenceError("certificate type-separation evidence differs")
    if not isinstance(value["cases"], list):
        raise EvidenceError("certificate cases must be a list")
    cases: dict[str, dict[str, Any]] = {}
    for entry in value["cases"]:
        if not isinstance(entry, dict):
            raise EvidenceError("certificate case is not an object")
        require_keys(entry, {"id", "inputs", "outcome", "error", "value"}, "certificate case")
        case_id = entry["id"]
        if not isinstance(case_id, str) or case_id in cases or not isinstance(entry["inputs"], list):
            raise EvidenceError("certificate case identity differs")
        if not all(isinstance(component, str) for component in entry["inputs"]):
            raise EvidenceError(f"{case_id} inputs are malformed")
        cases[case_id] = entry

    fixed_values = {
        "point2_finite": [1.0, -2.0], "point3_finite": [1.0, 2.0, 3.0],
        "vector2_finite": [3.0, 4.0], "vector3_finite": [1.0, 0.0, 0.0],
        "affine_translation": [4.0, 2.0], "affine_inverse_translation": [1.0, -2.0],
        "point_displacement": [0.0, 0.0], "dot_basis_same": [1.0], "dot_basis_distinct": [0.0],
        "cross_basis": [0.0, 0.0, 1.0], "cross_antisymmetry": [0.0, 0.0, -1.0],
        "cross_parallel": [0.0, 0.0, 0.0], "norm_three_four": [5.0],
        "norm_subnormal": [float.fromhex("0x0.0000000000001p-1022")],
        "normalize_subnormal": [1.0, 0.0],
    }
    for case_id, expected in fixed_values.items():
        require_value(cases, case_id, expected)
    require_value(cases, "normalize_three_four", [0.6, 0.8], approximate=True)
    for case_id in ("normalize_positive_zero", "normalize_mixed_zero"):
        require_error(cases, case_id, "zero_length")

    invalid_pattern = re.compile(r"^(point2|vector2|point3|vector3)_(nan|positive_infinity|negative_infinity)_(x|y|z)$")
    invalid_cases = [case_id for case_id in cases if invalid_pattern.match(case_id)]
    if len(invalid_cases) != 30:
        raise EvidenceError("certificate invalid-construction coverage differs")
    for case_id in invalid_cases:
        require_error(cases, case_id, "non_finite_input")

    extrema_pattern = re.compile(r"^(point2|vector2|point3|vector3)_(denorm_min|min|lowest|max)$")
    extrema_cases = [case_id for case_id in cases if extrema_pattern.match(case_id)]
    if len(extrema_cases) != 16:
        raise EvidenceError("certificate finite-envelope coverage differs")
    expected_extrema = {
        "denorm_min": float.fromhex("0x0.0000000000001p-1022"),
        "min": float.fromhex("0x1p-1022"),
        "lowest": -float.fromhex("0x1.fffffffffffffp+1023"),
        "max": float.fromhex("0x1.fffffffffffffp+1023"),
    }
    for case_id in extrema_cases:
        match = extrema_pattern.match(case_id)
        assert match is not None
        suffix = match.group(2)
        expected = expected_extrema[suffix]
        require_value(cases, case_id, [expected] + [0.0] * (2 if "3" in case_id else 1))

    power_pattern = re.compile(r"^power_two_(m500|m100|p0|p100|p500)_(norm|normalize)$")
    power_cases = [case_id for case_id in cases if power_pattern.match(case_id)]
    if len(power_cases) != 10:
        raise EvidenceError("certificate power-of-two coverage differs")
    exponents = {"m500": -500, "m100": -100, "p0": 0, "p100": 100, "p500": 500}
    for case_id in power_cases:
        match = power_pattern.match(case_id)
        assert match is not None
        scale = math.ldexp(1.0, exponents[match.group(1)])
        require_value(cases, case_id, [scale] if match.group(2) == "norm" else [1.0, 0.0])

    for case_id in (
        "overflow_vector_add", "overflow_vector_subtract", "overflow_vector_scale", "overflow_vector_divide",
        "overflow_point_translate", "overflow_point_reverse_translate", "overflow_point_displacement",
        "overflow_dot", "overflow_norm", "overflow_cross",
    ):
        require_error(cases, case_id, "non_finite_result")
    require_error(cases, "division_by_zero", "division_by_zero")
    require_error(cases, "scale_infinite", "non_finite_input")
    require_error(cases, "divide_nan", "non_finite_input")

    expected_count = 4 + 30 + 16 + 14 + 10 + 13
    if len(cases) != expected_count:
        raise EvidenceError("certificate includes undeclared or missing cases")
    return value


def semantic_bytes(value: dict[str, Any]) -> bytes:
    return (json.dumps(value, sort_keys=True, separators=(",", ":"), ensure_ascii=True) + "\n").encode("utf-8")


def compare(profile: pathlib.Path, certificates: list[pathlib.Path], report: pathlib.Path) -> None:
    validate_profile(profile)
    if not certificates:
        raise EvidenceError("at least one certificate is required")
    values = [validate_certificate(path) for path in certificates]
    reference = semantic_bytes(values[0])
    if any(semantic_bytes(value) != reference for value in values[1:]):
        raise EvidenceError("certificate semantic evidence differs")
    lines = [
        "# Geometry Point/Vector Evidence Report", "",
        "| Certificate | Schema | Semantic result |", "| --- | --- | --- |",
    ]
    lines.extend(f"| {path.name} | valid | equivalent |" for path in certificates)
    lines.extend(["", "Evidence comparison: PASS", "Qualification: EVIDENCE_COLLECTED_PENDING_AUDIT", ""])
    report.parent.mkdir(parents=True, exist_ok=True)
    report.write_bytes("\n".join(lines).encode("utf-8"))


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    commands = parser.add_subparsers(dest="command", required=True)
    profile = commands.add_parser("validate-profile")
    profile.add_argument("--profile", required=True)
    certificate = commands.add_parser("validate-certificate")
    certificate.add_argument("--certificate", required=True)
    compile_commands = commands.add_parser("validate-compile-commands")
    compile_commands.add_argument("--compile-commands", required=True)
    comparison = commands.add_parser("compare")
    comparison.add_argument("--profile", required=True)
    comparison.add_argument("--certificate", required=True, action="append")
    comparison.add_argument("--report", required=True)
    return parser.parse_args()


def main() -> int:
    arguments = parse_arguments()
    try:
        if arguments.command == "validate-profile":
            validate_profile(pathlib.Path(arguments.profile))
        elif arguments.command == "validate-certificate":
            validate_certificate(pathlib.Path(arguments.certificate))
        elif arguments.command == "validate-compile-commands":
            validate_compile_commands(pathlib.Path(arguments.compile_commands))
        else:
            compare(pathlib.Path(arguments.profile), [pathlib.Path(item) for item in arguments.certificate], pathlib.Path(arguments.report))
        return 0
    except EvidenceError as error:
        print(error, file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
