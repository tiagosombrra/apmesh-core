#!/usr/bin/env python3
"""Validate and compare report-only Numeric Contract evidence."""

from __future__ import annotations

import argparse
import json
import pathlib
import sys
from typing import Any


class EvidenceError(Exception):
    pass


EXPECTED_CLASSIFICATION = {
    "positive_zero": "zero",
    "negative_zero": "zero",
    "denorm_min": "subnormal",
    "one": "normal",
    "positive_infinity": "infinite",
    "quiet_nan": "not_a_number",
}
EXPECTED_POLICY = {
    "absolute_valid": ("valid", None),
    "negative_absolute": ("error", "negative_absolute_tolerance"),
    "negative_relative": ("error", "negative_relative_tolerance"),
    "zero_scale": ("error", "non_positive_reference_scale"),
    "infinite_scale": ("error", "non_finite_policy"),
}
EXPECTED_PROXIMITY = {
    "absolute_boundary": ("within", None),
    "absolute_outside": ("outside", None),
    "relative_boundary": ("within", None),
    "mixed_boundary": ("within", None),
    "symmetric_forward": ("within", None),
    "symmetric_reverse": ("within", None),
    "near_zero": ("within", None),
    "large_equal": ("within", None),
    "power_two_base": ("within", None),
    "power_two_scaled": ("within", None),
    "nan_input": ("error", "non_finite_input"),
    "overflowing_residual": ("error", "non_finite_intermediate"),
}
UNSAFE_FLOATING_FLAGS = (
    "-ffast-math",
    "-ofast",
    "-fassociative-math",
    "-funsafe-math-optimizations",
    "-ffinite-math-only",
    "-fno-signed-zeros",
    "-fno-trapping-math",
    "/fp:fast",
)


def read_json_value(path: pathlib.Path) -> Any:
    try:
        raw = path.read_bytes()
    except OSError as error:
        raise EvidenceError(f"cannot read {path}: {error}") from error
    if raw.startswith(b"\xef\xbb\xbf"):
        raise EvidenceError(f"UTF-8 BOM is not accepted: {path}")
    try:
        value = json.loads(raw.decode("utf-8"))
    except (UnicodeDecodeError, json.JSONDecodeError) as error:
        raise EvidenceError(f"invalid JSON: {path}: {error}") from error
    return value


def read_json(path: pathlib.Path) -> dict[str, Any]:
    value = read_json_value(path)
    if not isinstance(value, dict):
        raise EvidenceError(f"JSON object required: {path}")
    return value


def require_exact_keys(value: dict[str, Any], keys: set[str], context: str) -> None:
    if set(value) != keys:
        raise EvidenceError(f"schema keys differ for {context}")


def validate_certificate(path: pathlib.Path) -> dict[str, Any]:
    value = read_json(path)
    require_exact_keys(value, {"schema_version", "kind", "classification", "policy", "proximity"}, "certificate")
    if value["schema_version"] != 1 or value["kind"] != "numeric-contract-certificate":
        raise EvidenceError("certificate identity differs")
    for field in ("classification", "policy", "proximity"):
        if not isinstance(value[field], list):
            raise EvidenceError(f"certificate {field} must be a list")

    classification = {entry.get("case"): entry.get("category") for entry in value["classification"] if isinstance(entry, dict)}
    if classification != EXPECTED_CLASSIFICATION:
        raise EvidenceError("certificate classification evidence differs")
    policy = {entry.get("case"): (entry.get("outcome"), entry.get("error")) for entry in value["policy"] if isinstance(entry, dict)}
    if policy != EXPECTED_POLICY:
        raise EvidenceError("certificate policy evidence differs")
    proximity = {entry.get("case"): (entry.get("outcome"), entry.get("error")) for entry in value["proximity"] if isinstance(entry, dict)}
    if proximity != EXPECTED_PROXIMITY:
        raise EvidenceError("certificate proximity evidence differs")
    for entry in value["proximity"]:
        if not isinstance(entry, dict) or set(entry) != {"case", "outcome", "error", "residual_hex", "limit_hex"}:
            raise EvidenceError("certificate proximity row schema differs")
        if entry["outcome"] == "error":
            if entry["residual_hex"] is not None or entry["limit_hex"] is not None:
                raise EvidenceError("failed proximity row has numeric evidence")
        elif not isinstance(entry["residual_hex"], str) or not isinstance(entry["limit_hex"], str):
            raise EvidenceError("successful proximity row lacks numeric evidence")
    return value


def command_text(entry: dict[str, Any]) -> str:
    if isinstance(entry.get("command"), str):
        return entry["command"]
    if isinstance(entry.get("arguments"), list) and all(isinstance(part, str) for part in entry["arguments"]):
        return " ".join(entry["arguments"])
    raise EvidenceError("compile command entry lacks command text")


def validate_compile_commands(path: pathlib.Path) -> None:
    value = read_json_value(path)
    if not isinstance(value, list):
        raise EvidenceError("compile commands must be a list")
    numeric_commands = [command_text(entry) for entry in value if isinstance(entry, dict) and str(entry.get("file", "")).endswith("src/core/numeric.cpp")]
    if len(numeric_commands) != 1:
        raise EvidenceError("numeric source compile command is absent or ambiguous")
    lowered = numeric_commands[0].lower()
    if "-std=c++23" not in lowered:
        raise EvidenceError("numeric source is not compiled as strict C++23")
    if any(flag in lowered for flag in UNSAFE_FLOATING_FLAGS):
        raise EvidenceError("numeric source uses an unsafe floating-point flag")


def validate_environment(path: pathlib.Path, compile_commands: pathlib.Path) -> dict[str, Any]:
    value = read_json(path)
    require_exact_keys(value, {"schema_version", "kind", "double", "active_rounding"}, "environment")
    if value["schema_version"] != 1 or value["kind"] != "numeric-contract-environment":
        raise EvidenceError("environment identity differs")
    scalar = value["double"]
    if not isinstance(scalar, dict):
        raise EvidenceError("environment double evidence is absent")
    require_exact_keys(scalar, {"radix", "digits", "is_iec559", "subnormal_supported", "round_style"}, "environment double")
    if scalar != {"radix": 2, "digits": 53, "is_iec559": True, "subnormal_supported": True, "round_style": "to_nearest"}:
        raise EvidenceError("environment binary64 assumption differs")
    if value["active_rounding"] != "to_nearest":
        raise EvidenceError("active rounding mode differs")
    validate_compile_commands(compile_commands)
    return value


def validate_profile(path: pathlib.Path) -> dict[str, Any]:
    value = read_json(path)
    require_exact_keys(value, {"protocol_version", "repetitions_per_configuration", "configurations", "gates"}, "profile")
    expected_names = ["gcc-debug", "gcc-release", "clang-debug", "clang-release"]
    names = [entry.get("name") for entry in value["configurations"]] if isinstance(value["configurations"], list) else []
    if value["protocol_version"] != 1 or value["repetitions_per_configuration"] != 3 or names != expected_names or value["gates"] != ["N0", "N1", "N2", "N3", "N4", "N5", "N6", "N7"]:
        raise EvidenceError("numeric profile differs")
    return value


def semantic_bytes(value: dict[str, Any]) -> bytes:
    return (json.dumps(value, sort_keys=True, separators=(",", ":"), ensure_ascii=True) + "\n").encode("utf-8")


def compare(certificates: list[pathlib.Path], environments: list[pathlib.Path], report: pathlib.Path) -> None:
    if not certificates or len(certificates) != len(environments):
        raise EvidenceError("equal non-empty certificate and environment sets are required")
    certificate_values = [validate_certificate(path) for path in certificates]
    environment_values = [read_json(path) for path in environments]
    reference_certificate = semantic_bytes(certificate_values[0])
    reference_environment = semantic_bytes(environment_values[0])
    if any(semantic_bytes(value) != reference_certificate for value in certificate_values[1:]):
        raise EvidenceError("certificate semantic evidence differs across cells")
    if any(semantic_bytes(value) != reference_environment for value in environment_values[1:]):
        raise EvidenceError("environment semantic evidence differs across cells")
    lines = [
        "# Numeric Contract Evidence Report",
        "",
        "| Evidence pair | Certificate | Environment | Result |",
        "| --- | --- | --- | --- |",
    ]
    for certificate, environment in zip(certificates, environments, strict=True):
        lines.append(f"| {certificate.parent.name}/{certificate.name} | valid | equivalent | PASS |")
    lines.extend(["", "Overall: PASS", ""])
    report.parent.mkdir(parents=True, exist_ok=True)
    report.write_bytes("\n".join(lines).encode("utf-8"))


def parse_arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser()
    commands = parser.add_subparsers(dest="command", required=True)

    certificate = commands.add_parser("validate-certificate")
    certificate.add_argument("--certificate", required=True)
    certificate.set_defaults(handler=lambda args: validate_certificate(pathlib.Path(args.certificate)))

    environment = commands.add_parser("validate-environment")
    environment.add_argument("--environment", required=True)
    environment.add_argument("--compile-commands", required=True)
    environment.set_defaults(handler=lambda args: validate_environment(pathlib.Path(args.environment), pathlib.Path(args.compile_commands)))

    profile = commands.add_parser("validate-profile")
    profile.add_argument("--profile", required=True)
    profile.set_defaults(handler=lambda args: validate_profile(pathlib.Path(args.profile)))

    comparison = commands.add_parser("compare")
    comparison.add_argument("--certificate", required=True, action="append")
    comparison.add_argument("--environment", required=True, action="append")
    comparison.add_argument("--report", required=True)
    comparison.set_defaults(handler=lambda args: compare([pathlib.Path(path) for path in args.certificate], [pathlib.Path(path) for path in args.environment], pathlib.Path(args.report)))
    return parser.parse_args()


def main() -> int:
    arguments = parse_arguments()
    try:
        arguments.handler(arguments)
        return 0
    except EvidenceError as error:
        print(error, file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
