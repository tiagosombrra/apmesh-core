#!/usr/bin/env python3
"""Report-only validation, comparison and deterministic derived evidence for CGR0-CGR7."""

from __future__ import annotations

import argparse
import csv
import decimal
import json
import math
import pathlib
import sys
from typing import Any

from experiment_runtime import read_json, sha256_file, write_json


class EvidenceError(RuntimeError):
    pass


CELLS = [
    {"id": "gcc-debug", "compiler": "GCC 13", "library": "libstdc++", "build_type": "Debug"},
    {"id": "gcc-release", "compiler": "GCC 13", "library": "libstdc++", "build_type": "Release"},
    {"id": "clang-debug", "compiler": "Clang 18", "library": "libc++", "build_type": "Debug"},
    {"id": "clang-release", "compiler": "Clang 18", "library": "libc++", "build_type": "Release"},
]
GATES = [f"CGR{index}" for index in range(8)]
ALLOWLIST = [
    "apmesh_core.bootstrap_smoke",
    "apmesh_core.numeric_contract",
    "apmesh_core.geometry_primitives",
    "apmesh_core.minimal_small_linear_algebra",
    "apmesh_core.math_header_isolation",
    "apmesh_core.cartesian_frames",
    "apmesh_core.topological_model",
    "apmesh_core.curve_representation",
    "apmesh_core.curve_differential",
    "apmesh_core.curve_regularity",
    "apmesh_core.curve_arc_length",
    "apmesh_core.curve_cumulative_arc_length",
    "apmesh_core.curve_inverse_arc_length",
    "apmesh_core.curve_header_isolation",
]
CASES = [
    "representation_value",
    "differential_speed",
    "regularity_regular",
    "regularity_degenerate",
    "regularity_indeterminate",
    "total_length_line",
    "total_length_parabola",
    "cumulative_length_line",
    "cumulative_length_parabola",
    "inverse_line",
    "inverse_parabola",
    "reversal_relations",
    "translation_scale_relations",
    "planar_embedding_parity",
    "failure_semantics",
]
NEGATIVES = [
    "duplicate_case",
    "forged_categorical_result",
    "forged_reference_containment",
    "forged_inverse_bracket_order",
    "undeclared_semantic_claim",
    "invalid_regularity_policy",
    "invalid_length_policy",
    "invalid_inverse_policy",
    "non_finite_parameter",
    "non_finite_inverse_target",
    "inverse_target_outside_domain",
    "inverse_target_domain_indeterminate",
    "inverse_regularity_not_certified",
]
FROZEN = [
    "include/apmesh/geometry/curve.hpp",
    "src/geometry/curve.cpp",
    "src/geometry/detail/curve_length_interval.hpp",
    "src/geometry/detail/curve_regularity_interval.hpp",
    "tests/curve_representation.cpp",
    "tests/curve_differential.cpp",
    "tests/curve_regularity.cpp",
    "tests/curve_arc_length.cpp",
    "tests/curve_cumulative_arc_length.cpp",
    "tests/curve_inverse_arc_length.cpp",
    "tests/curve_header_isolation.cpp",
]
DERIVED = [
    "curve-value-reference",
    "curve-differential-speed",
    "curve-arc-length-enclosure",
    "curve-inverse-bracket",
]
NONCLAIMS = [
    "curvature",
    "torsion",
    "frenet_frames",
    "arbitrary_degree",
    "rational_curves",
    "bsplines",
    "nurbs",
    "physical_sampling",
    "boundary_discretization",
    "surface_ownership",
    "meshing",
    "quad_dominant",
    "parallel_equivalence",
    "wsl_cloud_equivalence",
    "native_windows_qualification",
]
RELATION_ABS = 2.0**-38
DECIMAL_CONTEXT = decimal.Context(prec=80)


def fail(message: str) -> None:
    raise EvidenceError(message)


def require_keys(value: Any, expected: set[str], context: str) -> dict[str, Any]:
    if not isinstance(value, dict) or set(value) != expected:
        fail(f"{context} keys differ")
    return value


def finite_number(value: Any, context: str) -> float:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        fail(f"{context} is not numeric")
    result = float(value)
    if not math.isfinite(result):
        fail(f"{context} is not finite")
    return result


def close(lhs: float, rhs: float, scale: float = 1.0) -> bool:
    return abs(lhs - rhs) <= RELATION_ABS * max(1.0, abs(scale), abs(lhs), abs(rhs))


def vector_close(lhs: list[Any], rhs: list[float], context: str) -> None:
    if not isinstance(lhs, list) or len(lhs) != len(rhs):
        fail(f"{context} dimensionality differs")
    for index, expected in enumerate(rhs):
        value = finite_number(lhs[index], f"{context}[{index}]")
        if not close(value, expected, max(1.0, abs(expected))):
            fail(f"{context} differs")


def interval(value: Any, context: str) -> tuple[float, float]:
    item = require_keys(
        value,
        {"result", "lower", "upper", "processed_nodes", "accepted_leaves", "max_depth_reached"},
        context,
    )
    if item["result"] not in {"converged", "indeterminate"}:
        fail(f"{context} result differs")
    lower = finite_number(item["lower"], f"{context}.lower")
    upper = finite_number(item["upper"], f"{context}.upper")
    if lower < 0.0 or lower > upper:
        fail(f"{context} enclosure ordering differs")
    for key in ("processed_nodes", "accepted_leaves", "max_depth_reached"):
        if isinstance(item[key], bool) or not isinstance(item[key], int) or item[key] < 0:
            fail(f"{context}.{key} differs")
    return lower, upper


def regularity(value: Any, context: str, expected: str) -> None:
    item = require_keys(
        value,
        {"result", "processed_nodes", "certified_leaves", "max_depth_reached"},
        context,
    )
    if item["result"] != expected:
        fail(f"{context} classification differs")
    for key in ("processed_nodes", "certified_leaves", "max_depth_reached"):
        if isinstance(item[key], bool) or not isinstance(item[key], int) or item[key] < 0:
            fail(f"{context}.{key} differs")


def inverse(value: Any, context: str) -> tuple[float, float]:
    item = require_keys(
        value,
        {
            "result",
            "target_lower",
            "target_upper",
            "lower_parameter",
            "upper_parameter",
            "lower_cumulative",
            "upper_cumulative",
            "total_length",
            "regularity",
            "refinement_iterations",
        },
        context,
    )
    if item["result"] not in {"converged", "indeterminate"}:
        fail(f"{context} result differs")
    target_lower = finite_number(item["target_lower"], f"{context}.target_lower")
    target_upper = finite_number(item["target_upper"], f"{context}.target_upper")
    lower_parameter = finite_number(item["lower_parameter"], f"{context}.lower_parameter")
    upper_parameter = finite_number(item["upper_parameter"], f"{context}.upper_parameter")
    if not (0.0 <= lower_parameter <= upper_parameter <= 1.0):
        fail(f"{context} parameter bracket differs")
    if target_lower < 0.0 or target_lower > target_upper:
        fail(f"{context} target interval differs")
    lower_cumulative = interval(item["lower_cumulative"], f"{context}.lower_cumulative")
    upper_cumulative = interval(item["upper_cumulative"], f"{context}.upper_cumulative")
    interval(item["total_length"], f"{context}.total_length")
    regularity(item["regularity"], f"{context}.regularity", "regular")
    if lower_cumulative[1] > target_lower or target_upper > upper_cumulative[0]:
        fail(f"{context} bracket proof differs")
    if isinstance(item["refinement_iterations"], bool) or not isinstance(item["refinement_iterations"], int):
        fail(f"{context}.refinement_iterations differs")
    return lower_parameter, upper_parameter


def decimal_bernstein(controls: list[list[Any]], parameter: float) -> list[float]:
    with decimal.localcontext(DECIMAL_CONTEXT):
        t = decimal.Decimal.from_float(parameter)
        one = decimal.Decimal(1)
        omt = one - t
        coefficients = [
            omt * omt * omt,
            decimal.Decimal(3) * omt * omt * t,
            decimal.Decimal(3) * omt * t * t,
            t * t * t,
        ]
        result: list[float] = []
        for component in range(len(controls[0])):
            total = decimal.Decimal(0)
            for coefficient, point in zip(coefficients, controls, strict=True):
                total += coefficient * decimal.Decimal.from_float(float(point[component]))
            result.append(float(total))
        return result


def analytic_parabola_prefix(parameter: float) -> float:
    return (
        0.5 * parameter * math.sqrt(1.0 + 4.0 * parameter * parameter)
        + 0.25 * math.asinh(2.0 * parameter)
    )


def validate_profile(path: pathlib.Path) -> dict[str, Any]:
    profile = read_json(path)
    require_keys(
        profile,
        {
            "schema_version",
            "kind",
            "repetitions_per_cell",
            "semantic_baseline",
            "cells",
            "gates",
            "semantic_ctest_allowlist",
            "cases",
            "certificate_negative_cases",
            "frozen_semantic_files",
            "derived_evidence",
            "authority_paths",
            "limitations",
        },
        "profile",
    )
    if profile["schema_version"] != 1 or profile["kind"] != "continuous-curve-geometry-regression-profile":
        fail("profile identity differs")
    if profile["repetitions_per_cell"] != 2 or profile["semantic_baseline"] != "438620efa1f93d29b442e9ba199882a09d2359d9":
        fail("profile baseline differs")
    if profile["cells"] != CELLS or profile["gates"] != GATES:
        fail("profile matrix or gates differ")
    if profile["semantic_ctest_allowlist"] != ALLOWLIST:
        fail("profile semantic allowlist differs")
    if profile["cases"] != CASES or profile["certificate_negative_cases"] != NEGATIVES:
        fail("profile case inventory differs")
    if profile["frozen_semantic_files"] != FROZEN or profile["derived_evidence"] != DERIVED:
        fail("profile frozen/derived inventory differs")
    if not isinstance(profile["authority_paths"], list) or not profile["authority_paths"]:
        fail("profile authorities differ")
    if not isinstance(profile["limitations"], list) or not profile["limitations"]:
        fail("profile limitations differ")
    return profile


def validate_certificate(profile: dict[str, Any], path: pathlib.Path) -> dict[str, Any]:
    certificate = read_json(path)
    require_keys(certificate, {"schema_version", "kind", "provenance", "scientific_projection"}, "certificate")
    if certificate["schema_version"] != 1 or certificate["kind"] != "continuous-curve-geometry-certificate":
        fail("certificate identity differs")
    provenance = require_keys(certificate["provenance"], {"cell", "repetition"}, "certificate provenance")
    if not isinstance(provenance["cell"], str) or not provenance["cell"]:
        fail("certificate cell differs")
    if isinstance(provenance["repetition"], bool) or not isinstance(provenance["repetition"], int) or provenance["repetition"] <= 0:
        fail("certificate repetition differs")

    projection = require_keys(
        certificate["scientific_projection"],
        {"case_inventory", "type_contract", "cases", "reference_inputs", "series", "nonclaims"},
        "scientific projection",
    )
    if projection["case_inventory"] != CASES:
        fail("certificate case inventory differs")
    if projection["type_contract"] != {
        "dimensions": [2, 3],
        "degree": 3,
        "control_count": 4,
        "parameter_domain": "[0,1]",
        "polynomial": True,
        "rational": False,
    }:
        fail("certificate type contract differs")
    if projection["nonclaims"] != NONCLAIMS:
        fail("certificate nonclaims differ")

    cases = require_keys(projection["cases"], set(CASES), "certificate cases")
    vector_close(cases["representation_value"]["endpoint0"], [0.0, 0.0], "endpoint0")
    vector_close(cases["representation_value"]["endpoint1"], [2.0, 0.0], "endpoint1")
    vector_close(cases["representation_value"]["midpoint"], [1.0, 1.5], "midpoint")
    vector_close(cases["differential_speed"]["first_at_0"], [0.0, 6.0], "first_at_0")
    vector_close(cases["differential_speed"]["second_at_0"], [12.0, -12.0], "second_at_0")
    if not close(finite_number(cases["differential_speed"]["speed_at_0"], "speed_at_0"), 6.0):
        fail("speed_at_0 differs")

    regularity(cases["regularity_regular"], "regularity_regular", "regular")
    regularity(cases["regularity_degenerate"], "regularity_degenerate", "degenerate")
    regularity(cases["regularity_indeterminate"], "regularity_indeterminate", "indeterminate")

    line_total = interval(cases["total_length_line"], "total_length_line")
    if not (line_total[0] <= 3.0 <= line_total[1]):
        fail("line total does not contain exact length")
    parabola_total = interval(cases["total_length_parabola"], "total_length_parabola")
    parabola_reference = analytic_parabola_prefix(1.0)
    if not (parabola_total[0] <= parabola_reference <= parabola_total[1]):
        fail("parabola total does not contain analytic reference")

    line_half = interval(cases["cumulative_length_line"], "cumulative_length_line")
    if not (line_half[0] <= 1.5 <= line_half[1]):
        fail("line cumulative does not contain analytic reference")
    parabola_quarter = interval(cases["cumulative_length_parabola"], "cumulative_length_parabola")
    quarter_reference = analytic_parabola_prefix(0.25)
    if not (parabola_quarter[0] <= quarter_reference <= parabola_quarter[1]):
        fail("parabola cumulative does not contain analytic reference")

    inverse_line_bounds = inverse(cases["inverse_line"], "inverse_line")
    if not (inverse_line_bounds[0] <= 0.2 <= inverse_line_bounds[1]):
        fail("line inverse bracket misses analytic parameter")

    inverse_parabola_case = require_keys(
        cases["inverse_parabola"],
        {"analytic_parameter", "analytic_target", "evidence"},
        "inverse_parabola",
    )
    if not close(finite_number(inverse_parabola_case["analytic_parameter"], "inverse_parabola.parameter"), 0.25):
        fail("inverse parabola parameter differs")
    if not close(
        finite_number(inverse_parabola_case["analytic_target"], "inverse_parabola.target"),
        quarter_reference,
        quarter_reference,
    ):
        fail("inverse parabola target differs")
    inverse_parabola_bounds = inverse(inverse_parabola_case["evidence"], "inverse_parabola.evidence")
    if not (inverse_parabola_bounds[0] <= 0.25 <= inverse_parabola_bounds[1]):
        fail("parabola inverse bracket misses analytic parameter")

    for context in ("reversal_relations", "translation_scale_relations"):
        if not isinstance(cases[context], dict):
            fail(f"{context} differs")
        for key, raw in cases[context].items():
            if abs(finite_number(raw, f"{context}.{key}")) > RELATION_ABS:
                fail(f"{context}.{key} residual differs")

    parity = require_keys(
        cases["planar_embedding_parity"],
        {
            "value_residual",
            "z",
            "first_derivative_residual",
            "derivative_z",
            "speed_residual",
            "total_2d",
            "total_3d",
        },
        "planar parity",
    )
    for key in ("value_residual", "z", "first_derivative_residual", "derivative_z", "speed_residual"):
        if abs(finite_number(parity[key], f"parity.{key}")) > RELATION_ABS:
            fail(f"planar parity {key} differs")
    total2 = interval(parity["total_2d"], "parity.total_2d")
    total3 = interval(parity["total_3d"], "parity.total_3d")
    if not (
        total2[0] <= parabola_reference <= total2[1]
        and total3[0] <= parabola_reference <= total3[1]
    ):
        fail("planar total parity misses reference")

    failures = require_keys(
        cases["failure_semantics"],
        {
            "non_finite_parameter",
            "out_of_domain_parameter",
            "invalid_regularity_policy",
            "invalid_length_policy",
            "invalid_inverse_policy",
            "non_finite_inverse_target",
            "inverse_target_outside_domain",
            "inverse_target_domain_indeterminate",
            "inverse_regularity_not_certified",
            "inverse_indeterminate_regularity",
        },
        "failure semantics",
    )
    expected_failures = {
        "non_finite_parameter": "non_finite_parameter",
        "out_of_domain_parameter": "parameter_out_of_domain",
        "invalid_regularity_policy": "invalid_policy",
        "invalid_length_policy": "invalid_policy",
        "invalid_inverse_policy": "invalid_policy",
        "non_finite_inverse_target": "non_finite_target",
        "inverse_target_outside_domain": "target_out_of_domain",
        "inverse_target_domain_indeterminate": "target_domain_indeterminate",
        "inverse_regularity_not_certified": "regularity_not_certified",
        "inverse_indeterminate_regularity": "regularity_not_certified",
    }
    if failures != expected_failures:
        fail("failure semantics differ")

    refs = require_keys(
        projection["reference_inputs"],
        {"value_curve_controls", "line_controls", "parabola_controls", "length_policy", "regularity_policy", "inverse_policy"},
        "reference inputs",
    )
    if refs["value_curve_controls"] != [[0, 0], [0, 2], [2, 2], [2, 0]]:
        fail("value controls differ")
    if refs["line_controls"] != [[0, 0], [1, 0], [2, 0], [3, 0]]:
        fail("line controls differ")
    if not isinstance(refs["parabola_controls"], list) or len(refs["parabola_controls"]) != 4:
        fail("parabola controls differ")

    series = require_keys(
        projection["series"],
        {"value_reference", "differential_speed", "arc_length_enclosure", "inverse_bracket"},
        "series",
    )
    if not isinstance(series["value_reference"], list) or len(series["value_reference"]) != 5:
        fail("value series differs")
    controls = refs["value_curve_controls"]
    for row in series["value_reference"]:
        item = require_keys(row, {"t", "value"}, "value series row")
        t = finite_number(item["t"], "value t")
        reference = decimal_bernstein(controls, t)
        vector_close(item["value"], reference, "value reference")

    if not isinstance(series["differential_speed"], list) or len(series["differential_speed"]) != 5:
        fail("differential series differs")
    for row in series["differential_speed"]:
        item = require_keys(row, {"t", "first", "second", "speed"}, "differential series row")
        t = finite_number(item["t"], "differential t")
        expected_first = [12.0 * t * (1.0 - t), 6.0 - 12.0 * t]
        expected_second = [12.0 - 24.0 * t, -12.0]
        vector_close(item["first"], expected_first, "differential first")
        vector_close(item["second"], expected_second, "differential second")
        expected_speed = math.hypot(*expected_first)
        if not close(finite_number(item["speed"], "differential speed"), expected_speed, expected_speed):
            fail("differential speed reference differs")

    if not isinstance(series["arc_length_enclosure"], list) or len(series["arc_length_enclosure"]) != 4:
        fail("arc-length series differs")
    previous_width = math.inf
    for row in series["arc_length_enclosure"]:
        item = require_keys(row, {"depth", "nodes", "lower", "upper", "width"}, "arc row")
        lower = finite_number(item["lower"], "arc lower")
        upper = finite_number(item["upper"], "arc upper")
        width = finite_number(item["width"], "arc width")
        if not (0.0 <= lower <= parabola_reference <= upper and close(width, upper - lower, max(1.0, upper))):
            fail("arc series containment differs")
        if width > previous_width + RELATION_ABS:
            fail("arc series width is not nonincreasing")
        previous_width = width

    if not isinstance(series["inverse_bracket"], list) or len(series["inverse_bracket"]) != 4:
        fail("inverse series differs")
    previous_width = math.inf
    for row in series["inverse_bracket"]:
        item = require_keys(row, {"iterations", "lower", "upper", "width"}, "inverse series row")
        lower = finite_number(item["lower"], "inverse lower")
        upper = finite_number(item["upper"], "inverse upper")
        width = finite_number(item["width"], "inverse width")
        if not (0.0 <= lower <= 0.2 <= upper <= 1.0 and close(width, upper - lower)):
            fail("inverse series bracket differs")
        if width > previous_width + RELATION_ABS:
            fail("inverse series width is not nonincreasing")
        previous_width = width

    return projection


def categorical_projection(projection: dict[str, Any]) -> dict[str, Any]:
    cases = projection["cases"]
    return {
        "case_inventory": projection["case_inventory"],
        "type_contract": projection["type_contract"],
        "nonclaims": projection["nonclaims"],
        "regularity": {
            key: cases[key]["result"]
            for key in ("regularity_regular", "regularity_degenerate", "regularity_indeterminate")
        },
        "length_results": {
            key: cases[key]["result"]
            for key in (
                "total_length_line",
                "total_length_parabola",
                "cumulative_length_line",
                "cumulative_length_parabola",
            )
        },
        "inverse_results": {
            "inverse_line": cases["inverse_line"]["result"],
            "inverse_parabola": cases["inverse_parabola"]["evidence"]["result"],
        },
        "failure_semantics": cases["failure_semantics"],
    }


def load_index(profile: dict[str, Any], path: pathlib.Path) -> list[dict[str, Any]]:
    index = read_json(path)
    require_keys(index, {"schema_version", "kind", "entries"}, "certificate index")
    if index["schema_version"] != 1 or index["kind"] != "continuous-curve-geometry-certificate-index":
        fail("certificate index identity differs")
    expected_slots = [
        (cell["id"], repetition)
        for cell in profile["cells"]
        for repetition in range(1, profile["repetitions_per_cell"] + 1)
    ]
    slots: list[tuple[str, int]] = []
    for entry in index["entries"]:
        require_keys(entry, {"cell", "repetition", "path", "sha256"}, "certificate index entry")
        slots.append((entry["cell"], entry["repetition"]))
    if slots != expected_slots:
        fail("certificate index coverage differs")
    return index["entries"]


def compare(profile: dict[str, Any], index_path: pathlib.Path) -> dict[str, Any]:
    entries = load_index(profile, index_path)
    by_cell: dict[str, list[dict[str, Any]]] = {}
    categories: list[dict[str, Any]] = []
    for entry in entries:
        path = (index_path.parent / entry["path"]).resolve()
        if not path.is_file() or sha256_file(path) != entry["sha256"]:
            fail("certificate index hash differs")
        certificate = read_json(path)
        if certificate.get("provenance") != {
            "cell": entry["cell"],
            "repetition": entry["repetition"],
        }:
            fail("certificate provenance differs from index")
        projection = validate_certificate(profile, path)
        by_cell.setdefault(entry["cell"], []).append(projection)
        categories.append(categorical_projection(projection))

    for cell in profile["cells"]:
        projections = by_cell.get(cell["id"], [])
        if len(projections) != 2 or projections[0] != projections[1]:
            fail(f"same-cell scientific projection differs: {cell['id']}")
    if not categories or any(value != categories[0] for value in categories[1:]):
        fail("cross-cell categorical projection differs")

    return {
        "schema_version": 1,
        "kind": "continuous-curve-geometry-comparison",
        "status": "EVIDENCE_COLLECTED_PENDING_AUDIT",
        "gates": {gate: "NOT_EXECUTED" for gate in profile["gates"]},
        "certificate_count": len(entries),
        "same_cell_projection_equal": True,
        "cross_cell_categorical_equal": True,
        "numeric_relations_independently_validated": True,
        "categorical_projection": categories[0],
    }


def negative_outcomes(profile: dict[str, Any], output: pathlib.Path) -> None:
    write_json(
        output,
        {
            "schema_version": 1,
            "kind": "continuous-curve-geometry-negative-outcomes",
            "outcomes": [
                {"id": identifier, "result": "REJECTED"}
                for identifier in profile["certificate_negative_cases"]
            ],
        },
    )


def validate_negative_outcomes(profile: dict[str, Any], path: pathlib.Path) -> dict[str, Any]:
    value = read_json(path)
    require_keys(value, {"schema_version", "kind", "outcomes"}, "negative outcomes")
    expected = [
        {"id": identifier, "result": "REJECTED"}
        for identifier in profile["certificate_negative_cases"]
    ]
    if value["schema_version"] != 1 or value["kind"] != "continuous-curve-geometry-negative-outcomes":
        fail("negative outcome identity differs")
    if value["outcomes"] != expected:
        fail("negative outcomes differ")
    return value


def svg_polyline(title: str, rows: list[tuple[float, float]], x_label: str, y_label: str) -> str:
    width, height = 640, 360
    left, right, top, bottom = 70, 20, 40, 55
    plot_width = width - left - right
    plot_height = height - top - bottom
    xs = [row[0] for row in rows]
    ys = [row[1] for row in rows]
    xmin, xmax = min(xs), max(xs)
    ymin, ymax = min(ys), max(ys)
    if xmax == xmin:
        xmax = xmin + 1.0
    if ymax == ymin:
        ymax = ymin + 1.0

    def px(value: float) -> float:
        return left + (value - xmin) / (xmax - xmin) * plot_width

    def py(value: float) -> float:
        return top + (ymax - value) / (ymax - ymin) * plot_height

    points = " ".join(f"{px(x):.6f},{py(y):.6f}" for x, y in rows)
    return (
        '<?xml version="1.0" encoding="UTF-8"?>\n'
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" viewBox="0 0 {width} {height}">\n'
        '<rect x="0" y="0" width="640" height="360" fill="white"/>\n'
        f'<text x="320" y="24" text-anchor="middle" font-family="sans-serif" font-size="16">{title}</text>\n'
        f'<line x1="{left}" y1="{top + plot_height}" x2="{left + plot_width}" y2="{top + plot_height}" stroke="black"/>\n'
        f'<line x1="{left}" y1="{top}" x2="{left}" y2="{top + plot_height}" stroke="black"/>\n'
        f'<polyline fill="none" stroke="black" stroke-width="2" points="{points}"/>\n'
        f'<text x="320" y="345" text-anchor="middle" font-family="sans-serif" font-size="12">{x_label}</text>\n'
        f'<text x="16" y="180" text-anchor="middle" font-family="sans-serif" font-size="12" transform="rotate(-90 16 180)">{y_label}</text>\n'
        '</svg>\n'
    )


def write_csv(path: pathlib.Path, header: list[str], rows: list[list[Any]]) -> None:
    with path.open("w", newline="", encoding="utf-8") as stream:
        writer = csv.writer(stream, lineterminator="\n")
        writer.writerow(header)
        writer.writerows(rows)


def derive(profile: dict[str, Any], certificate_path: pathlib.Path, output_dir: pathlib.Path) -> None:
    projection = validate_certificate(profile, certificate_path)
    output_dir.mkdir(parents=True, exist_ok=False)
    series = projection["series"]

    value_rows: list[list[Any]] = []
    value_plot: list[tuple[float, float]] = []
    controls = projection["reference_inputs"]["value_curve_controls"]
    for row in series["value_reference"]:
        t = float(row["t"])
        reference = decimal_bernstein(controls, t)
        residual = math.hypot(row["value"][0] - reference[0], row["value"][1] - reference[1])
        value_rows.append([t, row["value"][0], row["value"][1], reference[0], reference[1], residual])
        value_plot.append((t, residual))
    write_csv(
        output_dir / "curve-value-reference.csv",
        ["t", "x", "y", "reference_x", "reference_y", "residual"],
        value_rows,
    )
    (output_dir / "curve-value-reference.svg").write_text(
        svg_polyline("Curve value/reference residual", value_plot, "t", "residual"),
        encoding="utf-8",
        newline="\n",
    )

    diff_rows: list[list[Any]] = []
    diff_plot: list[tuple[float, float]] = []
    for row in series["differential_speed"]:
        t = float(row["t"])
        expected = [12.0 * t * (1.0 - t), 6.0 - 12.0 * t]
        expected_speed = math.hypot(*expected)
        residual = abs(float(row["speed"]) - expected_speed)
        diff_rows.append([t, row["first"][0], row["first"][1], row["speed"], expected_speed, residual])
        diff_plot.append((t, residual))
    write_csv(
        output_dir / "curve-differential-speed.csv",
        ["t", "dx", "dy", "speed", "reference_speed", "speed_residual"],
        diff_rows,
    )
    (output_dir / "curve-differential-speed.svg").write_text(
        svg_polyline("Differential/speed residual", diff_plot, "t", "residual"),
        encoding="utf-8",
        newline="\n",
    )

    arc_rows: list[list[Any]] = []
    arc_plot: list[tuple[float, float]] = []
    for row in series["arc_length_enclosure"]:
        arc_rows.append([row["depth"], row["nodes"], row["lower"], row["upper"], row["width"]])
        arc_plot.append((float(row["depth"]), float(row["width"])))
    write_csv(
        output_dir / "curve-arc-length-enclosure.csv",
        ["depth", "nodes", "lower", "upper", "width"],
        arc_rows,
    )
    (output_dir / "curve-arc-length-enclosure.svg").write_text(
        svg_polyline("Arc-length enclosure width", arc_plot, "depth", "width"),
        encoding="utf-8",
        newline="\n",
    )

    inverse_rows: list[list[Any]] = []
    inverse_plot: list[tuple[float, float]] = []
    for row in series["inverse_bracket"]:
        inverse_rows.append([row["iterations"], row["lower"], row["upper"], row["width"]])
        inverse_plot.append((float(row["iterations"]), float(row["width"])))
    write_csv(
        output_dir / "curve-inverse-bracket.csv",
        ["iterations", "lower", "upper", "width"],
        inverse_rows,
    )
    (output_dir / "curve-inverse-bracket.svg").write_text(
        svg_polyline("Inverse parameter-bracket width", inverse_plot, "iterations", "width"),
        encoding="utf-8",
        newline="\n",
    )

    manifest = {
        "schema_version": 1,
        "kind": "continuous-curve-geometry-derived-evidence",
        "certificate_sha256": sha256_file(certificate_path),
        "files": sorted(path.name for path in output_dir.iterdir()),
    }
    write_json(output_dir / "derived-evidence.json", manifest)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--profile", required=True)
    commands = parser.add_subparsers(dest="command", required=True)
    commands.add_parser("validate-profile")
    cert = commands.add_parser("validate-certificate")
    cert.add_argument("--certificate", required=True)
    compare_parser = commands.add_parser("compare")
    compare_parser.add_argument("--index", required=True)
    compare_parser.add_argument("--output", required=True)
    negative = commands.add_parser("negative-outcomes")
    negative.add_argument("--output", required=True)
    validate_negative = commands.add_parser("validate-negative-outcomes")
    validate_negative.add_argument("--outcomes", required=True)
    derive_parser = commands.add_parser("derive")
    derive_parser.add_argument("--certificate", required=True)
    derive_parser.add_argument("--output-dir", required=True)
    arguments = parser.parse_args()

    try:
        profile = validate_profile(pathlib.Path(arguments.profile))
        if arguments.command == "validate-profile":
            return 0
        if arguments.command == "validate-certificate":
            validate_certificate(profile, pathlib.Path(arguments.certificate))
            return 0
        if arguments.command == "compare":
            result = compare(profile, pathlib.Path(arguments.index))
            write_json(pathlib.Path(arguments.output), result)
            return 0
        if arguments.command == "negative-outcomes":
            negative_outcomes(profile, pathlib.Path(arguments.output))
            return 0
        if arguments.command == "validate-negative-outcomes":
            validate_negative_outcomes(profile, pathlib.Path(arguments.outcomes))
            return 0
        derive(profile, pathlib.Path(arguments.certificate), pathlib.Path(arguments.output_dir))
        return 0
    except (EvidenceError, OSError, ValueError, decimal.InvalidOperation) as error:
        print(f"ERROR: {error}", file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
