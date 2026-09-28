#!/usr/bin/env python3
"""Independent report-only validator for Surface Differential Geometry evidence."""

from __future__ import annotations

import argparse
import copy
import hashlib
import html
import math
import pathlib
import re
import sys
from collections import defaultdict
from typing import Any

from experiment_runtime import read_json, sha256_file, write_json


class EvidenceError(RuntimeError):
    pass


GATES = [f"SDG{i}" for i in range(8)]
CELLS = [
    {"id": "gcc-debug", "compiler": "GCC 13.3.0", "library": "libstdc++", "build_type": "Debug"},
    {"id": "gcc-release", "compiler": "GCC 13.3.0", "library": "libstdc++", "build_type": "Release"},
    {"id": "clang-debug", "compiler": "Clang 18.1.3", "library": "libc++ 18.1.3", "build_type": "Debug"},
    {"id": "clang-release", "compiler": "Clang 18.1.3", "library": "libc++ 18.1.3", "build_type": "Release"},
]
ERRORS = [
    "non_finite_u_parameter", "non_finite_v_parameter",
    "u_parameter_out_of_domain", "v_parameter_out_of_domain",
    "insufficient_continuity", "singular_parameterization",
    "non_representable_result",
]
FAMILIES = [
    "bicubic_bezier", "rational_bicubic_bezier", "bicubic_nurbs",
    "bicubic_nurbs_double_knot", "coons_patch", "rectangular_trim",
    "linear_extrusion", "revolution", "plane", "cylinder", "sphere",
]
CASE_IDS = [
    "metric_plane_identity", "metric_oblique",
    "parameter_non_finite_u", "parameter_non_finite_v",
    "parameter_u_out_of_domain", "parameter_v_out_of_domain",
    "non_representable_metric", "metric_exact_singular",
    "metric_near_singular", "metric_extreme_finite",
    "conditioning_nonorthogonal", "conditioning_parameter_scale",
    "conditioning_spatial_scale", "conditioning_axis_swap",
    "conditioning_u_reversal", "conditioning_v_reversal",
    "second_order_oblique", "second_order_hyperbolic",
    "principal_elliptic", "principal_hyperbolic", "principal_parabolic",
    "principal_umbilic", "principal_near_umbilic", "principal_nonorthogonal",
    "second_order_singular", "second_order_insufficient_continuity",
    "plane_surface", "cylinder_radius_1", "cylinder_radius_2",
    "cylinder_radius_4", "sphere_equator", "sphere_latitude",
    "sphere_pole", "sphere_near_pole", "family_conformance",
]
NEGATIVES = [
    "missing_case", "duplicate_case", "extra_case", "forged_candidate",
    "forged_typed_error", "forged_umbilic", "principal_order_violation",
    "principal_identity_violation", "metric_area_identity_violation",
    "condition_below_one", "forged_orientation_covariance",
    "forged_scale_covariance", "undeclared_policy", "non_finite_success",
    "malformed_signed_zero_policy", "undeclared_claim",
    "incomplete_figure_source_binding",
]
FIGURES = [
    {"id": "sphere_latitude_profile", "source": "sphere-equator-latitude-pole", "format": "svg"},
    {"id": "cylinder_radius_scale", "source": "cylinder-radius-1-2-4", "format": "svg"},
    {"id": "near_singular_conditioning", "source": "metric-near-singular-series", "format": "svg"},
    {"id": "principal_curvature_classes", "source": "elliptic-hyperbolic-parabolic", "format": "svg"},
]
NON_CLAIMS = [
    "surface_representation_qualification", "principal_directions",
    "curvature_line_continuity", "near_umbilic_threshold",
    "meshing_conditioning_threshold", "cone", "torus",
    "periodic_seam_topology", "general_pcurves", "topological_face_binding",
    "boundary_curve_discretization", "physical_sizing",
    "shared_boundary_certification", "triangular_meshing",
    "quadrilateral_meshing", "adaptive_meshing", "anisotropic_tensor_metrics",
    "quad_dominant", "parallel_gpu_simd_equivalence",
    "native_windows_qualification", "arbitrary_precision",
    "third_party_runtime_numerics",
]
FIELD_KEYS = {
    "e", "f", "g", "area_density", "normal", "condition_number",
    "l", "m", "n", "gaussian_curvature", "mean_curvature",
    "maximum_curvature", "minimum_curvature", "is_umbilic",
}
NUMERIC_FIELDS = [
    "e", "f", "g", "area_density", "condition_number", "l", "m", "n",
    "gaussian_curvature", "mean_curvature", "maximum_curvature",
    "minimum_curvature",
]
SHA1 = re.compile(r"^[0-9a-f]{40}$")
SHA256 = re.compile(r"^[0-9a-f]{64}$")


def fail(message: str) -> None:
    raise EvidenceError(message)


def require_keys(value: Any, expected: set[str], context: str) -> dict[str, Any]:
    if not isinstance(value, dict) or set(value) != expected:
        fail(f"{context} keys differ")
    return value


def parse_hex(value: Any, context: str) -> float:
    if not isinstance(value, str):
        fail(f"{context} is not hexadecimal floating text")
    try:
        result = float.fromhex(value)
    except ValueError as error:
        raise EvidenceError(f"{context} is not hexadecimal floating text") from error
    if not math.isfinite(result):
        fail(f"{context} is not finite")
    return result


def git_blob_sha1(path: pathlib.Path) -> str:
    data = path.read_bytes()
    return hashlib.sha1(f"blob {len(data)}\0".encode("ascii") + data).hexdigest()


def fields(**updates: Any) -> dict[str, Any]:
    result = {key: None for key in FIELD_KEYS}
    result.update(updates)
    return result


def value_result(**updates: Any) -> dict[str, Any]:
    return {"outcome": "value", "error": None, "fields": fields(**updates)}


def error_result(error: str) -> dict[str, Any]:
    return {"outcome": "error", "error": error, "fields": fields()}


def oracle(case_id: str) -> dict[str, Any]:
    z = [0.0, 0.0, 1.0]
    nz = [0.0, 0.0, -1.0]
    if case_id == "metric_plane_identity":
        return value_result(e=1.0, f=0.0, g=1.0, area_density=1.0, normal=z, condition_number=1.0)
    if case_id == "metric_oblique":
        return value_result(e=4.0, f=2.0, g=10.0, area_density=6.0, normal=z)
    errors = {
        "parameter_non_finite_u": "non_finite_u_parameter",
        "parameter_non_finite_v": "non_finite_v_parameter",
        "parameter_u_out_of_domain": "u_parameter_out_of_domain",
        "parameter_v_out_of_domain": "v_parameter_out_of_domain",
        "non_representable_metric": "non_representable_result",
        "metric_exact_singular": "singular_parameterization",
        "second_order_singular": "singular_parameterization",
        "second_order_insufficient_continuity": "insufficient_continuity",
        "sphere_pole": "singular_parameterization",
    }
    if case_id in errors:
        return error_result(errors[case_id])
    if case_id == "metric_near_singular":
        tiny = math.ldexp(1.0, -500)
        return value_result(e=1.0, f=1.0, g=1.0, area_density=tiny, normal=z, condition_number=math.ldexp(1.0, 501))
    if case_id == "metric_extreme_finite":
        return value_result(e=math.ldexp(1.0, 900), f=0.0, g=math.ldexp(1.0, -900), area_density=1.0, normal=z, condition_number=math.ldexp(1.0, 900))
    kappa = 0.5 * (3.0 + math.sqrt(5.0))
    simple_condition = {
        "conditioning_nonorthogonal": (4.0, 2.0, 2.0, 2.0, z, kappa),
        "conditioning_parameter_scale": (16.0, 0.0, 1.0, 4.0, z, 4.0),
        "conditioning_spatial_scale": (256.0, 128.0, 128.0, 128.0, z, kappa),
        "conditioning_axis_swap": (2.0, 2.0, 4.0, 2.0, nz, kappa),
        "conditioning_u_reversal": (4.0, -2.0, 2.0, 2.0, nz, kappa),
        "conditioning_v_reversal": (4.0, -2.0, 2.0, 2.0, nz, kappa),
    }
    if case_id in simple_condition:
        e, f, g, area, normal, condition = simple_condition[case_id]
        return value_result(e=e, f=f, g=g, area_density=area, normal=normal, condition_number=condition)
    if case_id == "second_order_oblique":
        return value_result(e=4.0, f=2.0, g=10.0, area_density=6.0, normal=z, condition_number=(7.0 + math.sqrt(13.0)) / 6.0, l=4.0, m=-2.0, n=6.0, gaussian_curvature=5.0 / 9.0, mean_curvature=1.0)
    if case_id == "second_order_hyperbolic":
        return value_result(e=1.0, f=0.0, g=1.0, area_density=1.0, normal=z, condition_number=1.0, l=2.0, m=0.0, n=-2.0, gaussian_curvature=-4.0, mean_curvature=0.0)
    principal = {
        "principal_elliptic": (3.0, 1.0, 3.0, 2.0, 3.0, 1.0, False),
        "principal_hyperbolic": (2.0, -4.0, -8.0, -1.0, 2.0, -4.0, False),
        "principal_parabolic": (2.0, 0.0, 0.0, 1.0, 2.0, 0.0, False),
        "principal_umbilic": (2.0, 2.0, 4.0, 2.0, 2.0, 2.0, True),
    }
    if case_id in principal:
        l, n, gaussian, mean, maximum, minimum, umbilic = principal[case_id]
        return value_result(e=1.0, f=0.0, g=1.0, area_density=1.0, normal=z, condition_number=1.0, l=l, m=0.0, n=n, gaussian_curvature=gaussian, mean_curvature=mean, maximum_curvature=maximum, minimum_curvature=minimum, is_umbilic=umbilic)
    if case_id == "principal_near_umbilic":
        near = math.nextafter(2.0, 3.0)
        return value_result(e=1.0, f=0.0, g=1.0, area_density=1.0, normal=z, condition_number=1.0, l=near, m=0.0, n=2.0, gaussian_curvature=near * 2.0, mean_curvature=0.5 * (near + 2.0), maximum_curvature=near, minimum_curvature=2.0, is_umbilic=False)
    if case_id == "principal_nonorthogonal":
        return value_result(e=4.0, f=2.0, g=10.0, area_density=6.0, normal=z, condition_number=(7.0 + math.sqrt(13.0)) / 6.0, l=4.0, m=-2.0, n=6.0, gaussian_curvature=5.0 / 9.0, mean_curvature=1.0, maximum_curvature=5.0 / 3.0, minimum_curvature=1.0 / 3.0, is_umbilic=False)
    if case_id == "plane_surface":
        return value_result(e=1.0, f=0.0, g=1.0, area_density=1.0, normal=z, condition_number=1.0, l=0.0, m=0.0, n=0.0, gaussian_curvature=0.0, mean_curvature=0.0, maximum_curvature=0.0, minimum_curvature=0.0, is_umbilic=True)
    if case_id.startswith("cylinder_radius_"):
        radius = float(case_id.rsplit("_", 1)[1])
        inverse = 1.0 / radius
        return value_result(e=radius * radius, f=0.0, g=1.0, area_density=radius, normal=[1.0, 0.0, 0.0], condition_number=max(radius, inverse), l=-radius, m=0.0, n=0.0, gaussian_curvature=0.0, mean_curvature=-0.5 * inverse, maximum_curvature=0.0, minimum_curvature=-inverse, is_umbilic=False)
    if case_id == "sphere_equator":
        return value_result(e=4.0, f=0.0, g=4.0, area_density=4.0, normal=[1.0, 0.0, 0.0], condition_number=1.0, l=-2.0, m=0.0, n=-2.0, gaussian_curvature=0.25, mean_curvature=-0.5, maximum_curvature=-0.5, minimum_curvature=-0.5, is_umbilic=True)
    if case_id in {"sphere_latitude", "sphere_near_pole"}:
        latitude = 0.5 if case_id == "sphere_latitude" else 0.5 * math.pi - math.ldexp(1.0, -20)
        cosine = math.cos(latitude)
        sine = math.sin(latitude)
        return value_result(e=4.0 * cosine * cosine, f=0.0, g=4.0, area_density=4.0 * cosine, normal=[cosine, 0.0, sine], condition_number=1.0 / cosine, l=-2.0 * cosine * cosine, m=0.0, n=-2.0, gaussian_curvature=0.25, mean_curvature=-0.5, maximum_curvature=-0.5, minimum_curvature=-0.5, is_umbilic=True)
    if case_id == "family_conformance":
        return value_result()
    fail(f"no independent oracle for {case_id}")


def validate_profile(path: pathlib.Path, source_root: pathlib.Path | None = None) -> dict[str, Any]:
    profile = read_json(path)
    expected_keys = {
        "schema_version", "kind", "protocol", "semantic_baseline",
        "repetitions_per_cell", "cells", "gates", "semantic_ctest_allowlist",
        "frozen_semantic_files", "numeric_policies", "certificate_cases",
        "required_errors", "required_surface_families",
        "certificate_negative_cases", "figure_specs", "non_claims",
        "signed_zero_policy", "tooling_status", "tooling_gate_state",
        "limitations", "frozen_semantic_git_blobs",
    }
    require_keys(profile, expected_keys, "profile")
    if profile["schema_version"] != 1 or profile["kind"] != "surface-differential-geometry-qualification-profile":
        fail("profile identity differs")
    if not SHA1.fullmatch(profile["semantic_baseline"]):
        fail("profile semantic baseline differs")
    if profile["repetitions_per_cell"] != 2 or profile["cells"] != CELLS or profile["gates"] != GATES:
        fail("profile matrix or gates differ")
    allowlist = profile["semantic_ctest_allowlist"]
    if not isinstance(allowlist, list) or len(allowlist) != 42 or len(set(allowlist)) != 42:
        fail("ordinary semantic allowlist differs")
    if profile["required_errors"] != ERRORS or profile["required_surface_families"] != FAMILIES:
        fail("profile error/family coverage differs")
    if profile["certificate_negative_cases"] != NEGATIVES or profile["figure_specs"] != FIGURES or profile["non_claims"] != NON_CLAIMS:
        fail("profile preregistered inventory differs")
    if profile["tooling_status"] != "EVIDENCE_COLLECTED_PENDING_AUDIT" or profile["tooling_gate_state"] != "NOT_EXECUTED":
        fail("tooling terminal-state guard differs")
    if profile["signed_zero_policy"] != {"raw_hex_retained": True, "semantic_projection": "canonicalize_positive_and_negative_zero", "signed_zero_scientific_claim": False}:
        fail("signed-zero policy differs")
    policies = profile["numeric_policies"]
    require_keys(policies, {"binary_strict", "surface_strict"}, "numeric policies")
    for name, policy in policies.items():
        require_keys(policy, {"absolute_tolerance", "relative_tolerance", "reference_scale_rule"}, f"{name} policy")
        parse_hex(policy["absolute_tolerance"], f"{name} absolute tolerance")
        parse_hex(policy["relative_tolerance"], f"{name} relative tolerance")
        if policy["reference_scale_rule"] != "max_one_expected_magnitude":
            fail(f"{name} scale rule differs")
    cases = profile["certificate_cases"]
    if [item.get("id") for item in cases] != CASE_IDS:
        fail("certificate case inventory differs")
    for item in cases:
        require_keys(item, {"id", "category", "comparison_rule", "policy"}, "profile case")
        if item["comparison_rule"] == "exact":
            if item["policy"] is not None:
                fail("exact case declares numeric policy")
        elif item["comparison_rule"] in {"mixed", "proximity"}:
            if item["policy"] not in policies:
                fail("rounded case lacks explicit policy")
        else:
            fail("comparison rule differs")
    frozen = profile["frozen_semantic_git_blobs"]
    if list(frozen) != profile["frozen_semantic_files"] or len(frozen) != 32:
        fail("frozen semantic path inventory differs")
    if any(not SHA1.fullmatch(value) for value in frozen.values()):
        fail("frozen semantic blob identity differs")
    if source_root is not None:
        for relative, expected in frozen.items():
            target = source_root / relative
            if not target.is_file() or git_blob_sha1(target) != expected:
                fail(f"frozen semantic file differs: {relative}")
    return profile


def decode_result(value: Any, context: str) -> dict[str, Any]:
    result = require_keys(value, {"outcome", "error", "fields"}, context)
    raw_fields = require_keys(result["fields"], FIELD_KEYS, f"{context}.fields")
    decoded = {key: None if raw_fields[key] is None else parse_hex(raw_fields[key], f"{context}.{key}") for key in NUMERIC_FIELDS}
    raw_normal = raw_fields["normal"]
    if raw_normal is None:
        decoded["normal"] = None
    elif isinstance(raw_normal, list) and len(raw_normal) == 3:
        decoded["normal"] = [parse_hex(item, f"{context}.normal") for item in raw_normal]
    else:
        fail(f"{context}.normal differs")
    if raw_fields["is_umbilic"] is not None and not isinstance(raw_fields["is_umbilic"], bool):
        fail(f"{context}.is_umbilic differs")
    decoded["is_umbilic"] = raw_fields["is_umbilic"]
    if result["outcome"] == "error":
        if result["error"] not in ERRORS or any(item is not None for item in decoded.values()):
            fail(f"{context} error payload differs")
    elif result["outcome"] == "value":
        if result["error"] is not None:
            fail(f"{context} value carries error")
    else:
        fail(f"{context} outcome differs")
    return {"outcome": result["outcome"], "error": result["error"], "fields": decoded}


def numeric_map(value: dict[str, Any]) -> dict[str, float]:
    result = {key: value[key] for key in NUMERIC_FIELDS if value[key] is not None}
    if value["normal"] is not None:
        result.update({f"normal_{axis}": value["normal"][index] for index, axis in enumerate("xyz")})
    return result


def policy_limit(profile: dict[str, Any], policy_name: str, expected: float) -> tuple[float, float]:
    policy = profile["numeric_policies"][policy_name]
    absolute = parse_hex(policy["absolute_tolerance"], "absolute tolerance")
    relative = parse_hex(policy["relative_tolerance"], "relative tolerance")
    scale = max(1.0, abs(expected))
    return scale, absolute + relative * scale


def compare_result(profile: dict[str, Any], meta: dict[str, Any], actual: dict[str, Any], expected: dict[str, Any], context: str) -> None:
    if actual["outcome"] != expected["outcome"] or actual["error"] != expected["error"]:
        fail(f"{context} classification differs")
    if expected["outcome"] == "error":
        return
    if actual["fields"]["is_umbilic"] != expected["fields"]["is_umbilic"]:
        fail(f"{context} umbilic state differs")
    wanted = numeric_map(expected["fields"])
    got = numeric_map(actual["fields"])
    if set(wanted) != set(got):
        fail(f"{context} field coverage differs")
    for field, reference in wanted.items():
        observed = got[field]
        if meta["comparison_rule"] == "exact":
            if not (observed == reference or observed == reference == 0.0):
                fail(f"{context}.{field} differs")
        else:
            _, limit = policy_limit(profile, meta["policy"], reference)
            if abs(observed - reference) > limit:
                fail(f"{context}.{field} violates {meta['policy']}")


def validate_comparison_records(profile: dict[str, Any], meta: dict[str, Any], entry: dict[str, Any], expected: dict[str, Any], observed: dict[str, Any]) -> None:
    records = entry["comparisons"]
    if meta["comparison_rule"] == "exact":
        if records:
            fail("exact case contains rounded-comparison evidence")
        return
    wanted = numeric_map(expected["fields"])
    got = numeric_map(observed["fields"])
    if len(records) != len(wanted):
        fail("rounded-comparison evidence count differs")
    seen: set[str] = set()
    for record in records:
        require_keys(record, {"field", "policy", "reference", "observed", "reference_scale", "residual", "limit"}, "comparison record")
        field = record["field"]
        if field in seen or field not in wanted or record["policy"] != meta["policy"]:
            fail("comparison record identity/policy differs")
        seen.add(field)
        reference = parse_hex(record["reference"], "comparison reference")
        observed_value = parse_hex(record["observed"], "comparison observed")
        scale = parse_hex(record["reference_scale"], "comparison scale")
        residual = parse_hex(record["residual"], "comparison residual")
        limit = parse_hex(record["limit"], "comparison limit")
        expected_scale, expected_limit = policy_limit(profile, meta["policy"], wanted[field])
        if reference != wanted[field] or observed_value != got[field] or scale != expected_scale:
            fail("comparison record source binding differs")
        if residual != abs(got[field] - wanted[field]) or limit != expected_limit or residual > limit:
            fail("comparison arithmetic differs")


def validate_observations(case_id: str, observations: Any, profile: dict[str, Any]) -> None:
    if not isinstance(observations, dict):
        fail(f"{case_id} observations differ")
    if case_id == "metric_near_singular":
        if observations.get("law") != "near_singular_power_of_two_series":
            fail("near-singular law differs")
        exponents = [20, 100, 500]
        series = observations.get("series")
        if not isinstance(series, list) or [item.get("separation_exponent") for item in series] != exponents:
            fail("near-singular series differs")
        for item, exponent in zip(series, exponents, strict=True):
            actual = parse_hex(item.get("condition_number"), "near-singular series")
            expected = math.ldexp(1.0, exponent + 1)
            _, limit = policy_limit(profile, "binary_strict", expected)
            if abs(actual - expected) > limit:
                fail("near-singular series violates analytic law")
    if case_id in {"conditioning_parameter_scale", "conditioning_spatial_scale", "conditioning_axis_swap", "conditioning_u_reversal", "conditioning_v_reversal"} and observations.get("law_satisfied") is not True:
        fail(f"{case_id} law flag differs")
    if case_id == "plane_surface" and observations != {"translation_invariant": True, "u_reversal_normal_flipped": True}:
        fail("plane transformation law differs")
    if case_id == "cylinder_radius_2" and observations != {"u_reversal_gaussian_invariant": True, "u_reversal_mean_flipped": True}:
        fail("cylinder orientation law differs")
    if case_id == "sphere_latitude" and observations != {"u_reversal_gaussian_invariant": True, "u_reversal_mean_flipped": True}:
        fail("sphere orientation law differs")
    if case_id == "family_conformance":
        require_keys(observations, {"families", "double_knot_c1"}, "family conformance")
        regular = [name for name in profile["required_surface_families"] if name != "bicubic_nurbs_double_knot"]
        if [item.get("id") for item in observations["families"]] != regular:
            fail("family conformance inventory differs")
        if any(item.get("regular_differential") != "value" for item in observations["families"]):
            fail("family conformance contains failure")
        if observations["double_knot_c1"] != {"id": "bicubic_nurbs_double_knot", "first_order": "value", "second_order": "insufficient_continuity"}:
            fail("double-knot conformance differs")


def validate_certificate_data(profile: dict[str, Any], certificate: Any) -> dict[str, Any]:
    value = require_keys(certificate, {"schema_version", "kind", "candidate_identity", "claim", "signed_zero_policy", "non_claims", "figure_sources", "cases"}, "certificate")
    if value["schema_version"] != 1 or value["kind"] != "surface-differential-geometry-qualification-certificate":
        fail("certificate identity differs")
    if value["candidate_identity"] != "REPORT_ONLY_TOOLING_UNPREPARED" or value["claim"] != "pointwise_local_surface_differential_geometry":
        fail("certificate candidate/claim differs")
    if value["signed_zero_policy"] != profile["signed_zero_policy"] or value["non_claims"] != profile["non_claims"]:
        fail("certificate policy/non-claims differ")
    if value["figure_sources"] != [item["source"] for item in profile["figure_specs"]]:
        fail("figure source binding differs")
    if not isinstance(value["cases"], list) or len(value["cases"]) != 35:
        fail("certificate case cardinality differs")
    metadata = {item["id"]: item for item in profile["certificate_cases"]}
    seen: set[str] = set()
    errors: set[str] = set()
    projection: list[dict[str, Any]] = []
    for entry in value["cases"]:
        require_keys(entry, {"id", "category", "comparison_rule", "policy", "reference", "observed", "comparisons", "observations"}, "certificate case")
        case_id = entry["id"]
        if case_id in seen or case_id not in metadata:
            fail("certificate case identity differs")
        seen.add(case_id)
        meta = metadata[case_id]
        if any(entry[key] != meta[key] for key in ("category", "comparison_rule", "policy")):
            fail(f"{case_id} metadata differs")
        expected = oracle(case_id)
        reference = decode_result(entry["reference"], f"{case_id}.reference")
        observed = decode_result(entry["observed"], f"{case_id}.observed")
        compare_result(profile, meta, reference, expected, f"{case_id}.reference")
        compare_result(profile, meta, observed, expected, f"{case_id}.observed")
        validate_comparison_records(profile, meta, entry, expected, observed)
        validate_observations(case_id, entry["observations"], profile)
        if observed["error"] is not None:
            errors.add(observed["error"])
        field = observed["fields"]
        if field["maximum_curvature"] is not None:
            if field["maximum_curvature"] < field["minimum_curvature"]:
                fail(f"{case_id} principal ordering differs")
            policy = meta["policy"] or "binary_strict"
            if field["gaussian_curvature"] is not None:
                _, limit = policy_limit(profile, policy, field["gaussian_curvature"])
                if abs(field["maximum_curvature"] * field["minimum_curvature"] - field["gaussian_curvature"]) > limit:
                    fail(f"{case_id} principal Gaussian identity differs")
            if field["mean_curvature"] is not None:
                target = 2.0 * field["mean_curvature"]
                _, limit = policy_limit(profile, policy, target)
                if abs(field["maximum_curvature"] + field["minimum_curvature"] - target) > limit:
                    fail(f"{case_id} principal mean identity differs")
        if field["e"] is not None and field["area_density"] is not None:
            determinant = field["e"] * field["g"] - field["f"] * field["f"]
            area_squared = field["area_density"] * field["area_density"]
            policy = meta["policy"] or "binary_strict"
            _, limit = policy_limit(profile, policy, max(abs(determinant), abs(area_squared)))
            if abs(determinant - area_squared) > limit:
                fail(f"{case_id} metric/area identity differs")
        if field["condition_number"] is not None and field["condition_number"] < 1.0:
            fail(f"{case_id} condition number is below one")
        projection.append({"id": case_id, "outcome": observed["outcome"], "error": observed["error"], "fields": observed["fields"], "observations": entry["observations"]})
    if seen != set(CASE_IDS) or errors != set(ERRORS):
        fail("certificate case/error coverage differs")
    return {"schema_version": 1, "kind": "surface-differential-geometry-scientific-projection", "claim": value["claim"], "signed_zero_policy": value["signed_zero_policy"], "non_claims": value["non_claims"], "cases": projection}


def validate_certificate(profile: dict[str, Any], path: pathlib.Path) -> dict[str, Any]:
    return validate_certificate_data(profile, read_json(path))


def canonical(value: Any) -> Any:
    if isinstance(value, dict):
        return {key: canonical(item) for key, item in sorted(value.items())}
    if isinstance(value, list):
        return [canonical(item) for item in value]
    if isinstance(value, float) and value == 0.0:
        return 0.0
    return value


def discrete(projection: dict[str, Any]) -> list[dict[str, Any]]:
    return [{"id": item["id"], "outcome": item["outcome"], "error": item["error"], "is_umbilic": item["fields"]["is_umbilic"], "observations": item["observations"]} for item in projection["cases"]]


def compare_index(profile: dict[str, Any], index_path: pathlib.Path) -> dict[str, Any]:
    index = read_json(index_path)
    require_keys(index, {"schema_version", "kind", "entries"}, "index")
    expected_slots = [(cell["id"], rep) for cell in profile["cells"] for rep in range(1, 3)]
    entries = index["entries"]
    if index["schema_version"] != 1 or index["kind"] != "surface-differential-geometry-certificate-index" or [(item.get("cell"), item.get("repetition")) for item in entries] != expected_slots:
        fail("certificate index differs")
    per_cell: dict[str, list[dict[str, Any]]] = defaultdict(list)
    all_discrete: list[list[dict[str, Any]]] = []
    for item in entries:
        require_keys(item, {"cell", "repetition", "path", "sha256"}, "index entry")
        path = (index_path.parent / item["path"]).resolve()
        if not SHA256.fullmatch(item["sha256"]) or not path.is_file() or sha256_file(path) != item["sha256"]:
            fail("certificate index hash binding differs")
        projection = canonical(validate_certificate(profile, path))
        per_cell[item["cell"]].append(projection)
        all_discrete.append(discrete(projection))
    if any(len(values) != 2 or values[0] != values[1] for values in per_cell.values()):
        fail("same-cell projection differs")
    if any(value != all_discrete[0] for value in all_discrete[1:]):
        fail("cross-cell discrete projection differs")
    return {"schema_version": 1, "kind": "surface-differential-geometry-tooling-comparison", "status": profile["tooling_status"], "gates": {gate: "NOT_EXECUTED" for gate in GATES}, "certificate_count": len(entries), "cells": [cell["id"] for cell in CELLS], "repetitions_per_cell": 2, "same_cell_deterministic": True, "cross_cell_equivalent_under_profile": True}


def case_by_id(certificate: dict[str, Any], case_id: str) -> dict[str, Any]:
    for item in certificate["cases"]:
        if item["id"] == case_id:
            return item
    fail(f"case missing for mutation: {case_id}")


def mutate(identifier: str, certificate: dict[str, Any]) -> dict[str, Any]:
    value = copy.deepcopy(certificate)
    if identifier == "missing_case": value["cases"].pop()
    elif identifier == "duplicate_case": value["cases"].append(copy.deepcopy(value["cases"][0]))
    elif identifier == "extra_case":
        item = copy.deepcopy(value["cases"][0]); item["id"] = "undeclared_extra_case"; value["cases"].append(item)
    elif identifier == "forged_candidate": value["candidate_identity"] = "PREPARED"
    elif identifier == "forged_typed_error": case_by_id(value, "sphere_pole")["observed"]["error"] = "insufficient_continuity"
    elif identifier == "forged_umbilic": case_by_id(value, "principal_umbilic")["observed"]["fields"]["is_umbilic"] = False
    elif identifier == "principal_order_violation":
        field = case_by_id(value, "principal_elliptic")["observed"]["fields"]; field["maximum_curvature"], field["minimum_curvature"] = field["minimum_curvature"], field["maximum_curvature"]
    elif identifier == "principal_identity_violation": case_by_id(value, "principal_elliptic")["observed"]["fields"]["maximum_curvature"] = float.hex(4.0)
    elif identifier == "metric_area_identity_violation": case_by_id(value, "metric_oblique")["observed"]["fields"]["area_density"] = float.hex(7.0)
    elif identifier == "condition_below_one": case_by_id(value, "conditioning_nonorthogonal")["observed"]["fields"]["condition_number"] = float.hex(0.5)
    elif identifier == "forged_orientation_covariance": case_by_id(value, "plane_surface")["observations"]["u_reversal_normal_flipped"] = False
    elif identifier == "forged_scale_covariance": case_by_id(value, "conditioning_spatial_scale")["observations"]["law_satisfied"] = False
    elif identifier == "undeclared_policy": case_by_id(value, "metric_near_singular")["policy"] = "undeclared"
    elif identifier == "non_finite_success": case_by_id(value, "metric_plane_identity")["observed"]["fields"]["e"] = "inf"
    elif identifier == "malformed_signed_zero_policy": value["signed_zero_policy"]["semantic_projection"] = "preserve_signed_zero"
    elif identifier == "undeclared_claim": value["claim"] = "surface_meshing_quality"
    elif identifier == "incomplete_figure_source_binding": value["figure_sources"].pop()
    else: fail(f"unknown negative mutation: {identifier}")
    return value


def negative_outcomes(profile: dict[str, Any], certificate_path: pathlib.Path) -> dict[str, Any]:
    certificate = read_json(certificate_path)
    validate_certificate_data(profile, certificate)
    outcomes = []
    for identifier in NEGATIVES:
        try:
            validate_certificate_data(profile, mutate(identifier, certificate))
        except EvidenceError:
            outcomes.append({"id": identifier, "result": "REJECTED"})
        else:
            fail(f"negative mutation accepted: {identifier}")
    return {"schema_version": 1, "kind": "surface-differential-geometry-negative-outcomes", "status": profile["tooling_status"], "outcomes": outcomes}


def case_field(case: dict[str, Any], name: str) -> float:
    value = decode_result(case["observed"], "figure source")["fields"][name]
    if value is None:
        fail(f"figure source lacks {name}")
    return value


def write_svg(path: pathlib.Path, title: str, series: list[tuple[str, list[tuple[float, float]]]], note: str) -> None:
    width, height, left, top, right, bottom = 760, 420, 70, 50, 30, 60
    points = [point for _, values in series for point in values]
    xs = [point[0] for point in points]; ys = [point[1] for point in points]
    xmin, xmax, ymin, ymax = min(xs), max(xs), min(ys), max(ys)
    if xmin == xmax: xmin, xmax = xmin - 1.0, xmax + 1.0
    if ymin == ymax: ymin, ymax = ymin - 1.0, ymax + 1.0
    def sx(value: float) -> float: return left + (value - xmin) / (xmax - xmin) * (width - left - right)
    def sy(value: float) -> float: return height - bottom - (value - ymin) / (ymax - ymin) * (height - top - bottom)
    colors = ["#1f2937", "#2563eb", "#059669", "#dc2626"]
    body = [
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" viewBox="0 0 {width} {height}">',
        '<rect width="100%" height="100%" fill="white"/>',
        f'<text x="{left}" y="28" font-family="sans-serif" font-size="18">{html.escape(title)}</text>',
        f'<line x1="{left}" y1="{height-bottom}" x2="{width-right}" y2="{height-bottom}" stroke="#6b7280"/>',
        f'<line x1="{left}" y1="{top}" x2="{left}" y2="{height-bottom}" stroke="#6b7280"/>',
    ]
    for index, (name, values) in enumerate(series):
        color = colors[index % len(colors)]
        coords = " ".join(f"{sx(x):.6f},{sy(y):.6f}" for x, y in values)
        if len(values) > 1: body.append(f'<polyline points="{coords}" fill="none" stroke="{color}" stroke-width="2"/>')
        for x, y in values: body.append(f'<circle cx="{sx(x):.6f}" cy="{sy(y):.6f}" r="3" fill="{color}"/>')
        body.append(f'<text x="{width-right-160}" y="{top+18*index}" font-family="sans-serif" font-size="12" fill="{color}">{html.escape(name)}</text>')
    body.append(f'<text x="{left}" y="{height-18}" font-family="sans-serif" font-size="12">{html.escape(note)}</text>')
    body.append("</svg>")
    path.write_text("\n".join(body) + "\n", encoding="utf-8", newline="\n")


def generate_figures(profile: dict[str, Any], certificate_path: pathlib.Path, output_dir: pathlib.Path) -> dict[str, Any]:
    certificate = read_json(certificate_path)
    validate_certificate_data(profile, certificate)
    cases = {item["id"]: item for item in certificate["cases"]}
    output_dir.mkdir(parents=True, exist_ok=True)
    sphere = {
        "equator": {key: case_field(cases["sphere_equator"], key) for key in ("gaussian_curvature", "mean_curvature", "condition_number")},
        "latitude": {key: case_field(cases["sphere_latitude"], key) for key in ("gaussian_curvature", "mean_curvature", "condition_number")},
        "near_pole": {key: case_field(cases["sphere_near_pole"], key) for key in ("gaussian_curvature", "mean_curvature", "condition_number")},
        "pole_error": cases["sphere_pole"]["observed"]["error"],
    }
    cylinder = {str(radius): {key: case_field(cases[f"cylinder_radius_{radius}"], key) for key in ("mean_curvature", "minimum_curvature", "condition_number")} for radius in (1, 2, 4)}
    near = {"series": cases["metric_near_singular"]["observations"]["series"], "exact_singular_error": cases["metric_exact_singular"]["observed"]["error"]}
    principal_ids = ["principal_elliptic", "principal_hyperbolic", "principal_parabolic"]
    principal = {identifier: {key: case_field(cases[identifier], key) for key in ("maximum_curvature", "minimum_curvature", "gaussian_curvature", "mean_curvature")} for identifier in principal_ids}
    data = {
        "sphere_latitude_profile": (sphere, [("K", [(0.0, sphere["equator"]["gaussian_curvature"]), (0.5, sphere["latitude"]["gaussian_curvature"])]), ("H", [(0.0, sphere["equator"]["mean_curvature"]), (0.5, sphere["latitude"]["mean_curvature"])]), ("kappa", [(0.0, sphere["equator"]["condition_number"]), (0.5, sphere["latitude"]["condition_number"])])], f"pole={sphere['pole_error']}; near-pole retained in source"),
        "cylinder_radius_scale": (cylinder, [("H", [(float(radius), cylinder[str(radius)]["mean_curvature"]) for radius in (1, 2, 4)]), ("kmin", [(float(radius), cylinder[str(radius)]["minimum_curvature"]) for radius in (1, 2, 4)]), ("kappa", [(float(radius), cylinder[str(radius)]["condition_number"]) for radius in (1, 2, 4)])], "K=0 for all retained radii"),
        "near_singular_conditioning": (near, [("log2(kappa)", [(float(item["separation_exponent"]), math.log2(parse_hex(item["condition_number"], "series"))) for item in near["series"]])], "exact singular endpoint retained as typed failure"),
        "principal_curvature_classes": (principal, [("kmax", [(float(index), principal[name]["maximum_curvature"]) for index, name in enumerate(principal_ids)]), ("kmin", [(float(index), principal[name]["minimum_curvature"]) for index, name in enumerate(principal_ids)])], "x: 0 elliptic, 1 hyperbolic, 2 parabolic"),
    }
    entries = []
    for spec in profile["figure_specs"]:
        source, series, note = data[spec["id"]]
        source_path = output_dir / f"{spec['id']}.source.json"
        figure_path = output_dir / f"{spec['id']}.svg"
        write_json(source_path, {"schema_version": 1, "kind": "surface-differential-geometry-figure-source", "id": spec["id"], "source_binding": spec["source"], "data": source})
        write_svg(figure_path, spec["id"].replace("_", " "), series, note)
        entries.append({"id": spec["id"], "source_binding": spec["source"], "source_path": source_path.name, "source_sha256": sha256_file(source_path), "figure_path": figure_path.name, "figure_sha256": sha256_file(figure_path)})
    return {"schema_version": 1, "kind": "surface-differential-geometry-figure-manifest", "status": profile["tooling_status"], "entries": entries}


def validate_figures(profile: dict[str, Any], path: pathlib.Path) -> None:
    value = read_json(path)
    require_keys(value, {"schema_version", "kind", "status", "entries"}, "figure manifest")
    if value["status"] != profile["tooling_status"] or len(value["entries"]) != 4:
        fail("figure manifest differs")
    specs = {item["id"]: item for item in profile["figure_specs"]}
    for entry in value["entries"]:
        require_keys(entry, {"id", "source_binding", "source_path", "source_sha256", "figure_path", "figure_sha256"}, "figure entry")
        if entry["id"] not in specs or entry["source_binding"] != specs[entry["id"]]["source"]:
            fail("figure source binding differs")
        for path_key, hash_key in (("source_path", "source_sha256"), ("figure_path", "figure_sha256")):
            artifact = path.parent / entry[path_key]
            if not artifact.is_file() or not SHA256.fullmatch(entry[hash_key]) or sha256_file(artifact) != entry[hash_key]:
                fail("figure artifact hash binding differs")


def make_report(profile: dict[str, Any], certificate: pathlib.Path, comparison: pathlib.Path, negatives: pathlib.Path, figures: pathlib.Path) -> dict[str, Any]:
    validate_certificate(profile, certificate)
    comparison_value = read_json(comparison)
    if comparison_value.get("status") != profile["tooling_status"] or comparison_value.get("gates") != {gate: "NOT_EXECUTED" for gate in GATES}:
        fail("comparison terminal-state guard differs")
    negative_value = read_json(negatives)
    expected_negatives = [{"id": identifier, "result": "REJECTED"} for identifier in NEGATIVES]
    if negative_value.get("status") != profile["tooling_status"] or negative_value.get("outcomes") != expected_negatives:
        fail("negative artifact differs")
    validate_figures(profile, figures)
    return {"schema_version": 1, "kind": "surface-differential-geometry-tooling-report", "status": profile["tooling_status"], "gates": {gate: "NOT_EXECUTED" for gate in GATES}, "prepared_candidate": False, "formal_execution": False, "stage_qualified": False, "artifacts": {"certificate_sha256": sha256_file(certificate), "comparison_sha256": sha256_file(comparison), "negative_outcomes_sha256": sha256_file(negatives), "figures_manifest_sha256": sha256_file(figures)}, "limitations": profile["limitations"]}


def write_markdown(path: pathlib.Path, report: dict[str, Any]) -> None:
    lines = ["# Surface Differential Geometry report-only tooling", "", f"Status: {report['status']}", "", "Development evidence only. No formal qualification gates were executed.", "", "## Gate state", ""]
    lines.extend(f"- {gate}: {state}" for gate, state in report["gates"].items())
    lines.extend(["", "## Formal-state guards", "", f"- prepared candidate: {str(report['prepared_candidate']).lower()}", f"- formal execution: {str(report['formal_execution']).lower()}", f"- stage qualified: {str(report['stage_qualified']).lower()}", "", "## Artifact hashes", ""])
    lines.extend(f"- {key}: {digest}" for key, digest in report["artifacts"].items())
    path.write_text("\n".join(lines) + "\n", encoding="utf-8", newline="\n")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--profile", required=True)
    parser.add_argument("--source-root")
    commands = parser.add_subparsers(dest="command", required=True)
    commands.add_parser("validate-profile")
    cert = commands.add_parser("validate-certificate"); cert.add_argument("--certificate", required=True)
    compare = commands.add_parser("compare"); compare.add_argument("--index", required=True); compare.add_argument("--output", required=True)
    negative = commands.add_parser("negative-outcomes"); negative.add_argument("--certificate", required=True); negative.add_argument("--output", required=True)
    figures = commands.add_parser("figures"); figures.add_argument("--certificate", required=True); figures.add_argument("--output-dir", required=True); figures.add_argument("--manifest", required=True)
    report = commands.add_parser("report"); report.add_argument("--certificate", required=True); report.add_argument("--comparison", required=True); report.add_argument("--negatives", required=True); report.add_argument("--figures", required=True); report.add_argument("--output-json", required=True); report.add_argument("--output-md", required=True)
    args = parser.parse_args()
    try:
        profile = validate_profile(pathlib.Path(args.profile), pathlib.Path(args.source_root) if args.source_root else None)
        if args.command == "validate-profile":
            return 0
        if args.command == "validate-certificate":
            validate_certificate(profile, pathlib.Path(args.certificate)); return 0
        if args.command == "compare":
            write_json(pathlib.Path(args.output), compare_index(profile, pathlib.Path(args.index))); return 0
        if args.command == "negative-outcomes":
            write_json(pathlib.Path(args.output), negative_outcomes(profile, pathlib.Path(args.certificate))); return 0
        if args.command == "figures":
            manifest = generate_figures(profile, pathlib.Path(args.certificate), pathlib.Path(args.output_dir)); write_json(pathlib.Path(args.manifest), manifest); validate_figures(profile, pathlib.Path(args.manifest)); return 0
        result = make_report(profile, pathlib.Path(args.certificate), pathlib.Path(args.comparison), pathlib.Path(args.negatives), pathlib.Path(args.figures))
        write_json(pathlib.Path(args.output_json), result); write_markdown(pathlib.Path(args.output_md), result); return 0
    except (EvidenceError, OSError, ValueError, KeyError, TypeError) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
