#!/usr/bin/env python3
"""Strict report-only validation for Point/Vector qualification evidence."""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import pathlib
import re
import sys
import tempfile
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
    "compiled_contract": True, "point_vector_conversion": False,
    "point_plus_point": False, "scalar_times_point": False,
    "vector_scaling": True, "point_displacement_vector": True,
}
SHA256 = re.compile(r"^[0-9a-f]{64}$")
NORMALIZE_THREE_FOUR_POLICY = {"relative_limit": "0x1p-52", "absolute_limit": "0x0p+0"}


def reject_duplicate_keys(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
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
        return json.loads(raw.decode("utf-8"), object_pairs_hook=reject_duplicate_keys)
    except (UnicodeDecodeError, json.JSONDecodeError) as error:
        raise EvidenceError(f"invalid JSON: {path}: {error}") from error


def write_json(path: pathlib.Path, value: Any) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_bytes((json.dumps(value, sort_keys=True, separators=(",", ":"), ensure_ascii=True) + "\n").encode("utf-8"))


def sha256(path: pathlib.Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def require_keys(value: dict[str, Any], expected: set[str], context: str) -> None:
    if set(value) != expected:
        raise EvidenceError(f"{context} keys differ; missing={sorted(expected - set(value))}, extra={sorted(set(value) - expected)}")


def as_hex(value: Any, context: str, *, finite: bool = True) -> float:
    if not isinstance(value, str):
        raise EvidenceError(f"{context} must be hexadecimal floating text")
    try:
        parsed = float.fromhex(value)
    except ValueError as error:
        raise EvidenceError(f"{context} is not hexadecimal floating text") from error
    if finite and not math.isfinite(parsed):
        raise EvidenceError(f"{context} must be finite")
    return parsed


def hexes(values: list[float]) -> list[str]:
    return [value.hex() for value in values]


def invalid_ids() -> list[str]:
    result: list[str] = []
    for kind, dimensions in (("point2", "xy"), ("vector2", "xy"), ("point3", "xyz"), ("vector3", "xyz")):
        for invalid in ("nan", "positive_infinity", "negative_infinity"):
            result.extend(f"{kind}_{invalid}_{axis}" for axis in dimensions)
    return result


def extrema_ids() -> list[str]:
    return [f"{kind}_{suffix}" for kind in ("point2", "point3", "vector2", "vector3")
            for suffix in ("denorm_min", "min", "lowest", "max")]


def power_ids() -> list[str]:
    return [f"power_two_{marker}_{operation}" for marker in ("m500", "m100", "p0", "p100", "p500")
            for operation in ("norm", "normalize")]


def required_case_ids() -> list[str]:
    return [
        "point2_finite", "point3_finite", "vector2_finite", "vector3_finite",
        *invalid_ids(), *extrema_ids(),
        "affine_translation", "affine_inverse_translation", "point_displacement",
        "dot_basis_same", "dot_basis_distinct", "cross_basis", "cross_antisymmetry", "cross_parallel",
        "norm_three_four", "normalize_three_four", "normalize_positive_zero", "normalize_mixed_zero",
        "norm_subnormal", "normalize_subnormal", *power_ids(),
        "overflow_vector_add", "overflow_vector_subtract", "overflow_vector_scale", "overflow_vector_divide",
        "overflow_point_translate", "overflow_point_reverse_translate", "overflow_point_displacement",
        "overflow_dot", "overflow_norm", "overflow_cross", "division_by_zero", "scale_infinite", "divide_nan",
    ]


def metadata_for(case_id: str) -> tuple[str, str, str]:
    if case_id.startswith("point2_"):
        return "construct_point2", "finite_construction", "exact"
    if case_id.startswith("point3_"):
        return "construct_point3", "finite_construction", "exact"
    if case_id.startswith("vector2_"):
        return "construct_vector2", "finite_construction", "exact"
    if case_id.startswith("vector3_"):
        return "construct_vector3", "finite_construction", "exact"
    if case_id in {"normalize_positive_zero", "normalize_mixed_zero"}:
        return "normalize", "failure_classification", "exact"
    if case_id.startswith("overflow_"):
        operation = {
            "overflow_vector_add": "vector_addition", "overflow_vector_subtract": "vector_subtraction",
            "overflow_vector_scale": "vector_scaling", "overflow_vector_divide": "vector_division",
            "overflow_point_translate": "point_vector_addition",
            "overflow_point_reverse_translate": "point_vector_subtraction",
            "overflow_point_displacement": "point_point_subtraction", "overflow_dot": "dot",
            "overflow_norm": "norm", "overflow_cross": "cross",
        }[case_id]
        return operation, "failure_classification", "exact"
    if case_id == "division_by_zero":
        return "vector_division", "failure_classification", "exact"
    if case_id == "scale_infinite":
        return "vector_scaling", "failure_classification", "exact"
    if case_id == "divide_nan":
        return "vector_division", "failure_classification", "exact"
    operations = {
        "affine_translation": "point_vector_addition",
        "affine_inverse_translation": "point_vector_subtraction",
        "point_displacement": "point_point_subtraction",
        "dot_basis_same": "dot", "dot_basis_distinct": "dot",
        "cross_basis": "cross", "cross_antisymmetry": "cross", "cross_parallel": "cross",
        "norm_three_four": "norm", "normalize_three_four": "normalize",
        "norm_subnormal": "norm", "normalize_subnormal": "normalize",
    }
    if case_id.startswith("power_two_"):
        return ("norm" if case_id.endswith("_norm") else "normalize"), "norm_normalization_scale", "exact"
    if case_id in operations:
        rule = "proximity" if case_id == "normalize_three_four" else "exact"
        category = "norm_normalization_scale" if case_id.startswith(("norm_", "normalize_")) else "affine_vector_algebra"
        return operations[case_id], category, rule
    raise EvidenceError(f"unknown fixed case: {case_id}")


def validate_profile(path: pathlib.Path) -> dict[str, Any]:
    value = read_json(path)
    if not isinstance(value, dict):
        raise EvidenceError("profile must be a JSON object")
    require_keys(value, {"protocol_version", "kind", "repetitions_per_configuration", "configurations", "gates",
                         "claim_fields", "volatile_fields", "focused_ctest_label", "foundation_ctest_label",
                         "cases", "limitations"}, "profile")
    expected_configurations = [
        {"name": name, "compiler": compiler, "standard_library": library, "build_type": build}
        for name, compiler, library, build in CONFIGURATIONS
    ]
    if value["protocol_version"] != 2 or value["kind"] != "geometry-point-vector-qualification-profile":
        raise EvidenceError("profile identity differs")
    if value["repetitions_per_configuration"] != 3 or value["configurations"] != expected_configurations:
        raise EvidenceError("profile execution matrix differs")
    if value["gates"] != [f"PV{number}" for number in range(8)]:
        raise EvidenceError("profile gates differ")
    if value["focused_ctest_label"] != "geometry" or value["foundation_ctest_label"] != "foundation-preservation":
        raise EvidenceError("profile CTest labels differ")
    if not all(isinstance(value[key], list) for key in ("claim_fields", "volatile_fields", "limitations", "cases")):
        raise EvidenceError("profile lists are malformed")
    expected_ids = required_case_ids()
    if len(value["cases"]) != len(expected_ids):
        raise EvidenceError("profile case count differs")
    seen: set[str] = set()
    for entry in value["cases"]:
        if not isinstance(entry, dict):
            raise EvidenceError("profile case is malformed")
        require_keys(entry, {"id", "operation", "claim_category", "comparison_rule"}, "profile case")
        case_id = entry["id"]
        if not isinstance(case_id, str) or case_id in seen:
            raise EvidenceError("profile case identity differs")
        seen.add(case_id)
        if tuple(entry[key] for key in ("operation", "claim_category", "comparison_rule")) != metadata_for(case_id):
            raise EvidenceError(f"profile case metadata differs: {case_id}")
    if seen != set(expected_ids):
        raise EvidenceError("profile cases differ")
    return value


def validate_compile_commands(path: pathlib.Path) -> None:
    value = read_json(path)
    if not isinstance(value, list):
        raise EvidenceError("compile commands must be a list")
    commands: list[str] = []
    for entry in value:
        if not isinstance(entry, dict):
            continue
        if not str(entry.get("file", "")).replace("\\", "/").endswith("src/core/geometry.cpp"):
            continue
        command = entry.get("command")
        commands.append(command if isinstance(command, str) else " ".join(entry.get("arguments", [])))
    if len(commands) != 1:
        raise EvidenceError("geometry source compile command is absent or ambiguous")
    lowered = commands[0].lower()
    if "-std=c++23" not in lowered or any(flag in lowered for flag in UNSAFE_FLOATING_FLAGS):
        raise EvidenceError("geometry source compile command violates the numeric envelope")


def expected_result(case_id: str, inputs: list[str]) -> tuple[str, str | None, list[str] | None]:
    if case_id in invalid_ids() or case_id in {"scale_infinite", "divide_nan"}:
        return "error", "non_finite_input", None
    if case_id in {"normalize_positive_zero", "normalize_mixed_zero"}:
        return "error", "zero_length", None
    if case_id == "division_by_zero":
        return "error", "division_by_zero", None
    if case_id.startswith("overflow_"):
        return "error", "non_finite_result", None
    fixed = {
        "affine_translation": [4.0, 2.0], "affine_inverse_translation": [1.0, -2.0],
        "point_displacement": [0.0, 0.0], "dot_basis_same": [1.0], "dot_basis_distinct": [0.0],
        "cross_basis": [0.0, 0.0, 1.0], "cross_antisymmetry": [0.0, 0.0, -1.0],
        "cross_parallel": [0.0, 0.0, 0.0], "norm_three_four": [5.0],
        "normalize_three_four": [0.6, 0.8], "norm_subnormal": [float.fromhex("0x0.0000000000001p-1022")],
        "normalize_subnormal": [1.0, 0.0],
    }
    if case_id in fixed:
        return "value", None, hexes(fixed[case_id])
    if case_id.startswith("power_two_"):
        exponent = {"m500": -500, "m100": -100, "p0": 0, "p100": 100, "p500": 500}[case_id.split("_")[2]]
        return "value", None, hexes([math.ldexp(1.0, exponent)] if case_id.endswith("_norm") else [1.0, 0.0])
    return "value", None, inputs


def validate_result(value: Any, expected: tuple[str, str | None, list[str] | None], context: str, *, approximate: bool) -> None:
    if not isinstance(value, dict):
        raise EvidenceError(f"{context} is malformed")
    require_keys(value, {"outcome", "error", "value"}, context)
    outcome, error, components = expected
    if value["outcome"] != outcome or value["error"] != error:
        raise EvidenceError(f"{context} classification differs")
    if components is None:
        if value["value"] is not None:
            raise EvidenceError(f"{context} error contains a value")
        return
    if not isinstance(value["value"], list) or len(value["value"]) != len(components):
        raise EvidenceError(f"{context} value arity differs")
    for index, (observed, wanted) in enumerate(zip(value["value"], components, strict=True)):
        observed_float = as_hex(observed, f"{context}[{index}]")
        wanted_float = as_hex(wanted, f"{context} expected[{index}]")
        if approximate:
            if not math.isclose(observed_float, wanted_float, rel_tol=2.0 ** -52, abs_tol=2.0 ** -52):
                raise EvidenceError(f"{context}[{index}] violates the declared proximity bound")
        elif observed_float != wanted_float:
            raise EvidenceError(f"{context}[{index}] differs")


def validate_normalize_three_four_proximity(comparison: dict[str, Any], expected: dict[str, Any], observed: dict[str, Any]) -> None:
    if comparison["exact_match"] is not False or comparison["policy"] != NORMALIZE_THREE_FOUR_POLICY:
        raise EvidenceError("normalize_three_four proximity policy differs")
    if expected["outcome"] != "value" or observed["outcome"] != "value":
        raise EvidenceError("normalize_three_four proximity requires successful values")
    expected_values = [as_hex(value, "normalize_three_four expected") for value in expected["value"]]
    observed_values = [as_hex(value, "normalize_three_four observed") for value in observed["value"]]
    if len(expected_values) != 2 or len(observed_values) != 2:
        raise EvidenceError("normalize_three_four proximity arity differs")
    reference_scale = math.hypot(*expected_values)
    residual = math.hypot(*(actual - reference for actual, reference in zip(observed_values, expected_values, strict=True)))
    absolute_limit = as_hex(NORMALIZE_THREE_FOUR_POLICY["absolute_limit"], "normalize_three_four absolute limit")
    relative_limit = as_hex(NORMALIZE_THREE_FOUR_POLICY["relative_limit"], "normalize_three_four relative limit")
    limit = absolute_limit + relative_limit * reference_scale
    if not all(math.isfinite(value) for value in (reference_scale, residual, limit)):
        raise EvidenceError("normalize_three_four proximity calculation is non-finite")
    recorded = {
        "reference_scale": as_hex(comparison["reference_scale"], "normalize_three_four comparison reference_scale"),
        "residual": as_hex(comparison["residual"], "normalize_three_four comparison residual"),
        "limit": as_hex(comparison["limit"], "normalize_three_four comparison limit"),
    }
    expected_fields = {"reference_scale": reference_scale, "residual": residual, "limit": limit}
    if any(recorded[key] != value for key, value in expected_fields.items()):
        raise EvidenceError("normalize_three_four proximity evidence differs")
    if residual > limit:
        raise EvidenceError("normalize_three_four violates the declared proximity bound")


def validate_certificate(path: pathlib.Path, profile: dict[str, Any] | None = None) -> dict[str, Any]:
    value = read_json(path)
    if not isinstance(value, dict):
        raise EvidenceError("certificate must be a JSON object")
    require_keys(value, {"schema_version", "kind", "environment", "separation", "cases"}, "certificate")
    if value["schema_version"] != 2 or value["kind"] != "geometry-point-vector-certificate":
        raise EvidenceError("certificate identity differs")
    if value["environment"] != {"double_radix": 2, "double_digits": 53, "iec559": True} or value["separation"] != SEPARATION:
        raise EvidenceError("certificate environment or type separation differs")
    profile = profile or {"cases": [{"id": case_id, "operation": metadata_for(case_id)[0],
                                     "claim_category": metadata_for(case_id)[1], "comparison_rule": metadata_for(case_id)[2]}
                                    for case_id in required_case_ids()]}
    expected_metadata = {entry["id"]: entry for entry in profile["cases"]}
    cases: dict[str, dict[str, Any]] = {}
    for entry in value["cases"]:
        if not isinstance(entry, dict):
            raise EvidenceError("certificate case is malformed")
        require_keys(entry, {"id", "operation", "claim_category", "inputs", "expected", "observed", "comparison"}, "certificate case")
        case_id = entry["id"]
        if not isinstance(case_id, str) or case_id in cases or not isinstance(entry["inputs"], list) or not all(isinstance(item, str) for item in entry["inputs"]):
            raise EvidenceError("certificate case identity differs")
        if case_id not in expected_metadata:
            raise EvidenceError(f"certificate includes undeclared case: {case_id}")
        metadata = expected_metadata[case_id]
        if any(entry[key] != metadata[key] for key in ("operation", "claim_category")):
            raise EvidenceError(f"certificate case metadata differs: {case_id}")
        expected = expected_result(case_id, entry["inputs"])
        approximate = metadata["comparison_rule"] == "proximity"
        validate_result(entry["expected"], expected, f"{case_id} expected", approximate=False)
        validate_result(entry["observed"], expected, f"{case_id} observed", approximate=approximate)
        comparison = entry["comparison"]
        if not isinstance(comparison, dict):
            raise EvidenceError(f"{case_id} comparison is malformed")
        require_keys(comparison, {"rule", "exact_match", "policy", "reference_scale", "residual", "limit"}, f"{case_id} comparison")
        if comparison["rule"] != metadata["comparison_rule"]:
            raise EvidenceError(f"{case_id} comparison rule differs")
        if approximate:
            if case_id != "normalize_three_four":
                raise EvidenceError(f"unexpected proximity case: {case_id}")
            validate_normalize_three_four_proximity(comparison, entry["expected"], entry["observed"])
        elif comparison != {"rule": "exact", "exact_match": True, "policy": None, "reference_scale": None, "residual": None, "limit": None}:
            raise EvidenceError(f"{case_id} exact comparison differs")
        cases[case_id] = entry
    if set(cases) != set(expected_metadata):
        raise EvidenceError("certificate cases differ")
    return value


def semantic_bytes(value: dict[str, Any]) -> bytes:
    return (json.dumps(value, sort_keys=True, separators=(",", ":"), ensure_ascii=True) + "\n").encode("utf-8")


def compare(profile_path: pathlib.Path, index_path: pathlib.Path, report: pathlib.Path) -> None:
    profile = validate_profile(profile_path)
    index = read_json(index_path)
    if not isinstance(index, dict):
        raise EvidenceError("certificate index is malformed")
    require_keys(index, {"schema_version", "kind", "entries"}, "certificate index")
    if index["schema_version"] != 1 or index["kind"] != "geometry-point-vector-certificate-index" or not isinstance(index["entries"], list):
        raise EvidenceError("certificate index identity differs")
    expected_slots = {(entry["name"], repetition) for entry in profile["configurations"]
                      for repetition in range(1, profile["repetitions_per_configuration"] + 1)}
    entries: dict[tuple[str, int], dict[str, Any]] = {}
    for entry in index["entries"]:
        if not isinstance(entry, dict):
            raise EvidenceError("certificate index entry is malformed")
        require_keys(entry, {"cell", "repetition", "path", "sha256"}, "certificate index entry")
        key = (entry["cell"], entry["repetition"])
        if key in entries or key not in expected_slots or not isinstance(entry["path"], str) or not SHA256.fullmatch(str(entry["sha256"])):
            raise EvidenceError("certificate index slot differs")
        path = (index_path.parent / entry["path"]).resolve()
        try:
            path.relative_to(index_path.parent.resolve())
        except ValueError as error:
            raise EvidenceError("certificate path escapes index root") from error
        if not path.is_file() or sha256(path) != entry["sha256"]:
            raise EvidenceError("certificate index hash differs")
        entries[key] = entry
    if set(entries) != expected_slots:
        raise EvidenceError("certificate index matrix differs")
    certificates = [validate_certificate((index_path.parent / entries[slot]["path"]).resolve(), profile)
                    for slot in sorted(expected_slots)]
    reference = semantic_bytes(certificates[0])
    if any(semantic_bytes(value) != reference for value in certificates[1:]):
        raise EvidenceError("certificate semantic evidence differs")
    summary = {"schema_version": 1, "kind": "geometry-point-vector-comparison",
               "configuration_count": len(profile["configurations"]),
               "repetitions_per_configuration": profile["repetitions_per_configuration"],
               "certificate_count": len(certificates), "result": "PASS",
               "qualification": "EVIDENCE_COLLECTED_PENDING_AUDIT"}
    write_json(report.with_suffix(".json"), summary)
    report.parent.mkdir(parents=True, exist_ok=True)
    report.write_text("# Geometry Point/Vector Evidence Report\n\nEvidence comparison: PASS\nQualification: EVIDENCE_COLLECTED_PENDING_AUDIT\n", encoding="utf-8")


def negative_self_check(profile: pathlib.Path, certificate: pathlib.Path, compile_commands: pathlib.Path, output: pathlib.Path) -> None:
    validated = validate_profile(profile)
    validate_certificate(certificate, validated)
    validate_compile_commands(compile_commands)
    with tempfile.TemporaryDirectory(prefix="apmesh-core-pv-negative-") as temporary:
        root = pathlib.Path(temporary)
        altered_certificate = read_json(certificate)
        altered_certificate["cases"].append(altered_certificate["cases"][0])
        altered_certificate_path = root / "duplicate-certificate.json"
        write_json(altered_certificate_path, altered_certificate)
        certificate_blocked = False
        try:
            validate_certificate(altered_certificate_path, validated)
        except EvidenceError:
            certificate_blocked = True
        altered_commands = read_json(compile_commands)
        for entry in altered_commands:
            if str(entry.get("file", "")).replace("\\", "/").endswith("src/core/geometry.cpp"):
                entry["command"] = str(entry.get("command", "")) + " -ffast-math"
        altered_commands_path = root / "unsafe-compile-commands.json"
        write_json(altered_commands_path, altered_commands)
        commands_blocked = False
        try:
            validate_compile_commands(altered_commands_path)
        except EvidenceError:
            commands_blocked = True
        altered_proximity = read_json(certificate)
        for case in altered_proximity["cases"]:
            if case["id"] == "normalize_three_four":
                case["comparison"]["residual"] = "0x1p+0"
                break
        else:
            raise EvidenceError("normalize_three_four fixture is absent")
        altered_proximity_path = root / "forged-proximity-certificate.json"
        write_json(altered_proximity_path, altered_proximity)
        proximity_blocked = False
        try:
            validate_certificate(altered_proximity_path, validated)
        except EvidenceError:
            proximity_blocked = True
    if not certificate_blocked or not commands_blocked or not proximity_blocked:
        raise EvidenceError("negative self-check did not reject malformed evidence")
    write_json(output, {"schema_version": 1, "kind": "geometry-point-vector-negative-fixtures",
                        "outcomes": [{"id": "duplicate_case", "result": "REJECTED"},
                                     {"id": "unsafe_compile_flag", "result": "REJECTED"},
                                     {"id": "forged_proximity", "result": "REJECTED"}]})


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    commands = parser.add_subparsers(dest="command", required=True)
    profile = commands.add_parser("validate-profile"); profile.add_argument("--profile", required=True)
    certificate = commands.add_parser("validate-certificate"); certificate.add_argument("--certificate", required=True); certificate.add_argument("--profile")
    compile_commands = commands.add_parser("validate-compile-commands"); compile_commands.add_argument("--compile-commands", required=True)
    comparison = commands.add_parser("compare"); comparison.add_argument("--profile", required=True); comparison.add_argument("--index", required=True); comparison.add_argument("--report", required=True)
    negative = commands.add_parser("negative-self-check"); negative.add_argument("--profile", required=True); negative.add_argument("--certificate", required=True); negative.add_argument("--compile-commands", required=True); negative.add_argument("--output", required=True)
    return parser.parse_args()


def main() -> int:
    arguments = parse_arguments()
    try:
        if arguments.command == "validate-profile":
            validate_profile(pathlib.Path(arguments.profile))
        elif arguments.command == "validate-certificate":
            profile = validate_profile(pathlib.Path(arguments.profile)) if arguments.profile else None
            validate_certificate(pathlib.Path(arguments.certificate), profile)
        elif arguments.command == "validate-compile-commands":
            validate_compile_commands(pathlib.Path(arguments.compile_commands))
        elif arguments.command == "compare":
            compare(pathlib.Path(arguments.profile), pathlib.Path(arguments.index), pathlib.Path(arguments.report))
        else:
            negative_self_check(pathlib.Path(arguments.profile), pathlib.Path(arguments.certificate),
                                pathlib.Path(arguments.compile_commands), pathlib.Path(arguments.output))
        return 0
    except EvidenceError as error:
        print(error, file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
