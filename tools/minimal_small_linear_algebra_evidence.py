#!/usr/bin/env python3
"""Strict, report-only validation for Minimal Small Linear Algebra evidence."""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import math
import pathlib
import re
from typing import Any


class EvidenceError(RuntimeError):
    """Raised when declared LA evidence is incomplete or differs."""


CELLS = ["gcc-debug", "gcc-release", "clang-debug", "clang-release"]
DETERMINANT_NON_CLAIMS = ["predicate", "rank", "degeneracy", "orientation", "incidence", "topology"]
SHA256 = re.compile(r"[0-9a-f]{64}")
NEGATIVE_CASES = (
    "duplicate_case", "forged_permutation", "wrong_nonfinite_placement",
    "incomplete_nonfinite_matrix", "altered_finite_matrix_entry", "wrong_failure_error",
    "undeclared_scale_field", "missing_determinant_nonclaim",
    "undeclared_signed_zero_policy", "nonzero_one_ulp_mismatch",
)
EXACT_HEX_POLICY = {
    "rule": "exact_hex",
    "signed_zero_policy": "normalize_to_positive",
    "proximity_policy": None,
}
EXACT_HEX_COMPARISON = {**EXACT_HEX_POLICY, "exact_match": True}


def reject_duplicates(pairs: list[tuple[str, Any]]) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, value in pairs:
        if key in result:
            raise EvidenceError(f"duplicate JSON key: {key}")
        result[key] = value
    return result


def read_json(path: pathlib.Path) -> Any:
    raw = path.read_bytes()
    if raw.startswith(b"\xef\xbb\xbf"):
        raise EvidenceError(f"UTF-8 BOM is not accepted: {path}")
    try:
        return json.loads(raw.decode("utf-8"), object_pairs_hook=reject_duplicates)
    except (UnicodeDecodeError, json.JSONDecodeError) as error:
        raise EvidenceError(f"invalid JSON: {path}: {error}") from error


def write_json(path: pathlib.Path, value: Any) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, sort_keys=True, separators=(",", ":"), ensure_ascii=True) + "\n", encoding="utf-8")


def sha256(path: pathlib.Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest()


def require_keys(value: dict[str, Any], expected: set[str], context: str) -> None:
    if set(value) != expected:
        raise EvidenceError(f"{context} keys differ; missing={sorted(expected - set(value))}, extra={sorted(set(value) - expected)}")


def as_float(value: Any, context: str, *, finite: bool = True) -> float:
    if not isinstance(value, str):
        raise EvidenceError(f"{context} must be hexadecimal text")
    special = {"nan": math.nan, "inf": math.inf, "-inf": -math.inf}
    try:
        result = special[value] if value in special else float.fromhex(value)
    except ValueError as error:
        raise EvidenceError(f"{context} is not hexadecimal floating text") from error
    if finite and not math.isfinite(result):
        raise EvidenceError(f"{context} must be finite")
    return result


def required_cases() -> list[str]:
    cases = ["mat2_zero", "mat2_identity", "mat3_zero", "mat3_identity"]
    cases += [f"mat2_access_r{row}c{column}" for row in range(2) for column in range(2)]
    cases += ["mat2_access_row_oob", "mat2_access_column_oob"]
    cases += [f"mat3_access_r{row}c{column}" for row in range(3) for column in range(3)]
    cases += ["mat3_access_row_oob", "mat3_access_column_oob"]
    cases += ["mat2_diagonal_application", "mat3_diagonal_application", "mat2_swap_determinant", "mat3_cycle_determinant"]
    cases += ["mat2_swap_application", "mat3_cycle_application"]
    cases += ["mat2_quarter_turn_application", "mat2_quarter_turn_square", "mat2_noncommuting_composition", "mat3_noncommuting_composition"]
    cases += ["mat2_transpose_composition", "mat3_transpose_composition", "mat2_zero_row_determinant", "mat3_zero_row_determinant"]
    cases += ["type_reject_mat2_point2", "type_reject_mat3_point3", "type_reject_mat2_vector3", "type_reject_mat2_mat3"]
    cases += ["type_reject_mat3_vector2", "type_reject_mat3_mat2"]
    for dimension, count in ((2, 4), (3, 9)):
        for kind in ("nan", "posinf", "neginf"):
            cases += [f"mat{dimension}_nonfinite_{kind}_e{index}" for index in range(count)]
    cases += [
        "mat2_overflow_application", "mat3_overflow_application", "mat2_overflow_composition",
        "mat3_overflow_composition", "mat2_overflow_determinant", "mat3_overflow_determinant",
    ]
    cases += ["scale_k_m8", "scale_k_m1", "scale_k_0", "scale_k_1", "scale_k_8"]
    return cases


def metadata(case_id: str) -> tuple[int, str, str, str]:
    if case_id in {"mat2_zero", "mat2_identity"}:
        return 2, case_id.removeprefix("mat2_"), "algebraic_agreement", "none"
    if case_id in {"mat3_zero", "mat3_identity"}:
        return 3, case_id.removeprefix("mat3_"), "algebraic_agreement", "none"
    match = re.fullmatch(r"mat([23])_access_r([0-2])c([0-2])", case_id)
    if match:
        return int(match.group(1)), "checked_access", "dimension_dependency", "matrix_row_major,row,column"
    match = re.fullmatch(r"mat([23])_access_(row|column)_oob", case_id)
    if match:
        return int(match.group(1)), "checked_access", "dimension_dependency", "matrix_row_major,row,column"
    match = re.fullmatch(r"mat([23])_nonfinite_(nan|posinf|neginf)_e[0-8]", case_id)
    if match:
        return int(match.group(1)), "matrix_construction", "failure_classification", "matrix_row_major"
    match = re.fullmatch(r"mat([23])_overflow_(application|composition|determinant)", case_id)
    if match:
        dimension, kind = int(match.group(1)), match.group(2)
        operation = {"application": "matrix_vector_application", "composition": "matrix_composition", "determinant": "determinant"}[kind]
        layout = {"application": "matrix_row_major,vector", "composition": "lhs_row_major,rhs_row_major", "determinant": "matrix_row_major"}[kind]
        return dimension, operation, "failure_classification", layout
    known = {
        "mat2_swap_application": (2, "matrix_vector_application", "algebraic_agreement", "matrix_row_major,vector"),
        "mat3_cycle_application": (3, "matrix_vector_application", "algebraic_agreement", "matrix_row_major,vector"),
        "type_reject_mat3_vector2": (3, "compile_time_rejection", "dimension_dependency", "Mat3,Vector2"),
        "type_reject_mat3_mat2": (3, "compile_time_rejection", "dimension_dependency", "Mat3,Mat2"),
        "mat2_diagonal_application": (2, "matrix_vector_application", "algebraic_agreement", "matrix_row_major,vector"),
        "mat3_diagonal_application": (3, "matrix_vector_application", "algebraic_agreement", "matrix_row_major,vector"),
        "mat2_swap_determinant": (2, "determinant", "determinant_boundary", "matrix_row_major"),
        "mat3_cycle_determinant": (3, "determinant", "determinant_boundary", "matrix_row_major"),
        "mat2_quarter_turn_application": (2, "matrix_vector_application", "algebraic_agreement", "matrix_row_major,vector"),
        "mat2_quarter_turn_square": (2, "matrix_composition", "algebraic_agreement", "lhs_row_major,rhs_row_major"),
        "mat2_noncommuting_composition": (2, "noncommuting_composition", "algebraic_agreement", "lhs_row_major,rhs_row_major"),
        "mat3_noncommuting_composition": (3, "noncommuting_composition", "algebraic_agreement", "lhs_row_major,rhs_row_major"),
        "mat2_transpose_composition": (2, "transpose_composition", "algebraic_agreement", "lhs_row_major,rhs_row_major"),
        "mat3_transpose_composition": (3, "transpose_composition", "algebraic_agreement", "lhs_row_major,rhs_row_major"),
        "mat2_zero_row_determinant": (2, "determinant", "determinant_boundary", "matrix_row_major"),
        "mat3_zero_row_determinant": (3, "determinant", "determinant_boundary", "matrix_row_major"),
        "type_reject_mat2_point2": (2, "compile_time_rejection", "dimension_dependency", "Mat2,Point2"),
        "type_reject_mat3_point3": (3, "compile_time_rejection", "dimension_dependency", "Mat3,Point3"),
        "type_reject_mat2_vector3": (2, "compile_time_rejection", "dimension_dependency", "Mat2,Vector3"),
        "type_reject_mat2_mat3": (2, "compile_time_rejection", "dimension_dependency", "Mat2,Mat3"),
    }
    if case_id in known:
        return known[case_id]
    if re.fullmatch(r"scale_k_(?:m8|m1|0|1|8)", case_id):
        return 0, "power_two_scale_laws", "scale_determinism", "power_of_two_scale"
    raise EvidenceError(f"unknown case id: {case_id}")


def validate_profile(path: pathlib.Path) -> dict[str, Any]:
    profile = read_json(path)
    if not isinstance(profile, dict):
        raise EvidenceError("profile must be an object")
    expected = {"schema_version", "kind", "repetitions_per_cell", "cells", "gates", "cases", "negative_cases", "scale_exponents", "exact_prerequisite_tests", "source_checks", "equivalence", "volatile_fields", "limitations"}
    require_keys(profile, expected, "profile")
    if profile["schema_version"] != 4 or profile["kind"] != "minimal-small-linear-algebra-qualification-profile":
        raise EvidenceError("profile identity differs")
    if profile["repetitions_per_cell"] != 3 or profile["gates"] != [f"LA{number}" for number in range(8)]:
        raise EvidenceError("profile repetitions or gates differ")
    if not isinstance(profile["cells"], list) or [item.get("id") for item in profile["cells"] if isinstance(item, dict)] != CELLS:
        raise EvidenceError("profile cell matrix differs")
    for item in profile["cells"]:
        if not isinstance(item, dict):
            raise EvidenceError("profile cell is malformed")
        require_keys(item, {"id", "compiler", "library", "build_type"}, "profile cell")
    for key in ("cases", "exact_prerequisite_tests", "source_checks", "volatile_fields", "limitations"):
        if not isinstance(profile[key], list) or not profile[key] or any(not isinstance(item, str) or not item for item in profile[key]):
            raise EvidenceError(f"profile {key} differs")
    if profile["cases"] != required_cases() or len(set(profile["cases"])) != len(profile["cases"]):
        raise EvidenceError("profile does not enumerate the fixed case matrix")
    if profile["negative_cases"] != list(NEGATIVE_CASES):
        raise EvidenceError("profile does not enumerate the fixed negative matrix")
    if len(set(profile["exact_prerequisite_tests"])) != len(profile["exact_prerequisite_tests"]):
        raise EvidenceError("profile prerequisite names are duplicated")
    if profile["scale_exponents"] != [-8, -1, 0, 1, 8] or profile["equivalence"] != EXACT_HEX_POLICY:
        raise EvidenceError("profile equivalence differs")
    return profile


def validate_source_root(source_root: pathlib.Path) -> dict[str, str]:
    paths = [source_root / "include/apmesh/math/linear_algebra.hpp", source_root / "src/math/linear_algebra.cpp", source_root / "CMakeLists.txt"]
    if any(not path.is_file() for path in paths):
        raise EvidenceError("source check input is absent")
    header, source, cmake = (path.read_text(encoding="utf-8") for path in paths)
    if "apmesh/core/geometry.hpp" in header or "apmesh/core/geometry.hpp" in source:
        raise EvidenceError("math depends on geometry")
    if re.search(r"\b(inverse|solve|decomposition|eigen|predicate)\b", header + "\n" + source, re.IGNORECASE):
        raise EvidenceError("excluded linear algebra capability appears in math")
    if any(flag in cmake.lower() for flag in ("-ffast-math", "-ofast", "-fassociative-math", "-funsafe-math-optimizations", "-ffinite-math-only")):
        raise EvidenceError("unsafe floating flag appears in CMake")
    dependency_graph(source_root)
    library = re.search(r"add_library\(apmesh_core\s+STATIC\s+([^)]*)\)", cmake)
    expected_sources = {"src/core/bootstrap.cpp", "src/core/numeric.cpp", "src/core/geometry.cpp", "src/math/linear_algebra.cpp"}
    if not library or set(library.group(1).split()) != expected_sources:
        raise EvidenceError("core target source boundary differs")
    if re.search(r"target_link_libraries\(apmesh_core\s", cmake):
        raise EvidenceError("core target has an undeclared link dependency")
    return {
        "math_header_has_no_geometry_include": "PASS",
        "math_source_has_no_geometry_include": "PASS",
        "no_inverse_solve_decomposition_eigen_or_predicate_api": "PASS",
        "no_unsafe_floating_flags": "PASS",
        "transitive_module_dependencies": "PASS",
        "single_core_target_no_external_link": "PASS",
    }


def dependency_graph(root: pathlib.Path) -> dict[str, list[str]]:
    """Resolve actual project includes transitively, including relative includes."""
    graph: dict[str, list[str]] = {}
    for folder in ("include", "src"):
        for path in sorted((root / folder).rglob("*")):
            if path.suffix not in {".hpp", ".h", ".cpp"}: continue
            text = re.sub(r"/\*.*?\*/|//[^\n]*", "", path.read_text(encoding="utf-8"), flags=re.S)
            edges = []
            for name in re.findall(r'^\s*#\s*include\s*[<"]([^>"]+)[>"]', text, re.M):
                candidates = [path.parent / name, root / "include" / name]
                found = next((item.resolve() for item in candidates if item.is_file()), None)
                if found is not None:
                    try: edges.append(found.relative_to(root.resolve()).as_posix())
                    except ValueError: raise EvidenceError("include escapes project boundary")
                elif name.startswith("apmesh/"): raise EvidenceError(f"unresolved project include: {name}")
            graph[path.relative_to(root).as_posix()] = sorted(edges)
    for start in graph:
        if "/math/" not in start and not start.endswith("numeric.cpp") and not start.endswith("numeric.hpp"): continue
        seen: set[str] = set(); pending = list(graph[start])
        while pending:
            node = pending.pop()
            if node in seen: continue
            seen.add(node); pending.extend(graph.get(node, []))
        if any("geometry" in node or any(f"/{part}/" in node for part in ("topology", "mesh", "sizing")) for node in seen):
            raise EvidenceError(f"prohibited transitive dependency: {start}")
    if "include/apmesh/math/linear_algebra.hpp" not in graph.get("include/apmesh/core/geometry.hpp", []):
        raise EvidenceError("Geometry adapter does not depend on math")
    return graph


def output_fields(case_id: str) -> list[str]:
    dimension, operation, _, _ = metadata(case_id)
    matrix = lambda prefix, size: [f"{prefix}.r{row}c{column}" for row in range(size) for column in range(size)]
    vector = lambda prefix, size: [f"{prefix}.{axis}" for axis in "xyz"[:size]]
    if "_overflow_" in case_id or "_nonfinite_" in case_id or "_oob" in case_id or case_id.startswith("type_reject_"): return []
    if operation == "power_two_scale_laws":
        return (vector("mat2.scaled_application", 2) + vector("mat3.scaled_application", 3)
                + matrix("mat2.scaled_transpose", 2) + matrix("mat3.scaled_transpose", 3)
                + ["mat2.scaled_determinant", "mat3.scaled_determinant"]
                + [field for side in ("left", "right") for size in (2, 3) for field in matrix(f"mat{size}.{side}_scaled_composition", size)])
    if operation == "matrix_vector_application": return vector("result", dimension)
    if operation in {"determinant", "checked_access"}: return ["scalar"]
    if operation == "noncommuting_composition": return matrix("AB", dimension) + matrix("BA", dimension)
    if operation == "transpose_composition": return matrix("transpose_AB", dimension) + matrix("transpose_B_transpose_A", dimension)
    return matrix("result", dimension)


def parse_values(value: Any, context: str, *, finite: bool = True) -> list[float]:
    if not isinstance(value, list):
        raise EvidenceError(f"{context} must be an array")
    return [as_float(item, f"{context}[{index}]", finite=finite) for index, item in enumerate(value)]


def parse_result(value: Any, context: str) -> tuple[str, str | None, list[float] | None]:
    if not isinstance(value, dict):
        raise EvidenceError(f"{context} is malformed")
    require_keys(value, {"outcome", "error", "value"}, context)
    if value["outcome"] == "value":
        if value["error"] is not None:
            raise EvidenceError(f"{context} value error differs")
        return "value", None, parse_values(value["value"], context)
    if value["outcome"] == "error":
        if value["error"] not in {"non_finite_input", "non_finite_result", "index_out_of_range"} or value["value"] is not None:
            raise EvidenceError(f"{context} error differs")
        return "error", value["error"], None
    if value["outcome"] == "compile_time_rejection" and value["error"] is None and value["value"] is None:
        return "compile_time_rejection", None, None
    raise EvidenceError(f"{context} result kind differs")


def matmul(left: list[float], right: list[float], dimension: int) -> list[float]:
    return [sum(left[row * dimension + index] * right[index * dimension + column] for index in range(dimension)) for row in range(dimension) for column in range(dimension)]


def matvec(matrix: list[float], vector: list[float], dimension: int) -> list[float]:
    return [sum(matrix[row * dimension + index] * vector[index] for index in range(dimension)) for row in range(dimension)]


def transpose_values(matrix: list[float], dimension: int) -> list[float]:
    return [matrix[column * dimension + row] for row in range(dimension) for column in range(dimension)]


def determinant_value(matrix: list[float], dimension: int) -> float:
    if dimension == 2:
        return matrix[0] * matrix[3] - matrix[1] * matrix[2]
    return matrix[0] * (matrix[4] * matrix[8] - matrix[5] * matrix[7]) - matrix[1] * (matrix[3] * matrix[8] - matrix[5] * matrix[6]) + matrix[2] * (matrix[3] * matrix[7] - matrix[4] * matrix[6])


def expected_result(case_id: str, inputs: list[float]) -> tuple[str, str | None, list[float] | None]:
    diagonal2 = [2., 0., 0., -4.]; diagonal3 = [2., 0., 0., 0., -3., 0., 0., 0., 4.]
    swap = [0., 1., 1., 0.]; cycle = [0., 1., 0., 0., 0., 1., 1., 0., 0.]
    left2 = [1., 2., 0., 1.]; right2 = [2., 0., 1., 3.]
    left3 = [1., 1., 0., 0., 1., 1., 0., 0., 1.]; right3 = [2., 0., 0., 0., 3., 0., 0., 0., 4.]
    quarter = [0., -1., 1., 0.]
    fixed = {
        "mat2_zero": [], "mat2_identity": [], "mat3_zero": [], "mat3_identity": [],
        "mat2_diagonal_application": diagonal2 + [3., -2.], "mat3_diagonal_application": diagonal3 + [2., -1., 4.],
        "mat2_swap_application": swap + [3., -2.], "mat3_cycle_application": cycle + [2., -1., 4.],
        "mat2_swap_determinant": swap, "mat3_cycle_determinant": cycle,
        "mat2_zero_row_determinant": [0., 0., 0., 1.], "mat3_zero_row_determinant": [0., 0., 0., 1., 2., 3., 4., 5., 6.],
        "mat2_quarter_turn_application": quarter + [1., 0.], "mat2_quarter_turn_square": quarter + quarter,
        "mat2_noncommuting_composition": left2 + right2, "mat3_noncommuting_composition": left3 + right3,
        "mat2_transpose_composition": left2 + right2, "mat3_transpose_composition": left3 + right3,
    }
    if case_id in fixed and inputs != fixed[case_id]: raise EvidenceError(f"fixed analytic inputs differ: {case_id}")
    if case_id == "mat2_zero": return "value", None, [0.0] * 4
    if case_id == "mat2_identity": return "value", None, [1.0, 0.0, 0.0, 1.0]
    if case_id == "mat3_zero": return "value", None, [0.0] * 9
    if case_id == "mat3_identity": return "value", None, [1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0]
    access = re.fullmatch(r"mat([23])_access_r([0-2])c([0-2])", case_id)
    if access:
        dimension, row, column = (int(access.group(1)), int(access.group(2)), int(access.group(3)))
        expected_input = ([1.0 if row_ == column_ else 0.0 for row_ in range(dimension) for column_ in range(dimension)] + [float(row), float(column)])
        if inputs != expected_input: raise EvidenceError(f"access inputs differ: {case_id}")
        return "value", None, [expected_input[row * dimension + column]]
    oob = re.fullmatch(r"mat([23])_access_(row|column)_oob", case_id)
    if oob:
        dimension, axis = int(oob.group(1)), oob.group(2)
        matrix = [1.0 if row == column else 0.0 for row in range(dimension) for column in range(dimension)]
        expected_input = matrix + ([float(dimension), 0.0] if axis == "row" else [0.0, float(dimension)])
        if inputs != expected_input: raise EvidenceError(f"out-of-range inputs differ: {case_id}")
        return "error", "index_out_of_range", None
    if case_id in {"mat2_diagonal_application", "mat3_diagonal_application", "mat2_swap_application", "mat3_cycle_application"}:
        dimension = int(case_id[3]); matrix, vector = inputs[:dimension * dimension], inputs[dimension * dimension:]
        if len(inputs) != dimension * dimension + dimension: raise EvidenceError(f"application inputs differ: {case_id}")
        return "value", None, matvec(matrix, vector, dimension)
    if case_id in {"mat2_swap_determinant", "mat3_cycle_determinant", "mat2_zero_row_determinant", "mat3_zero_row_determinant"}:
        dimension = int(case_id[3]);
        if len(inputs) != dimension * dimension: raise EvidenceError(f"determinant inputs differ: {case_id}")
        return "value", None, [determinant_value(inputs, dimension)]
    if case_id == "mat2_quarter_turn_application":
        if len(inputs) != 6: raise EvidenceError("quarter-turn application inputs differ")
        return "value", None, matvec(inputs[:4], inputs[4:], 2)
    if case_id == "mat2_quarter_turn_square":
        if len(inputs) != 8: raise EvidenceError("quarter-turn square inputs differ")
        return "value", None, matmul(inputs[:4], inputs[4:], 2)
    if case_id in {"mat2_noncommuting_composition", "mat3_noncommuting_composition"}:
        dimension = int(case_id[3]); split = dimension * dimension
        if len(inputs) != split * 2: raise EvidenceError(f"composition inputs differ: {case_id}")
        return "value", None, matmul(inputs[:split], inputs[split:], dimension) + matmul(inputs[split:], inputs[:split], dimension)
    if case_id in {"mat2_transpose_composition", "mat3_transpose_composition"}:
        dimension = int(case_id[3]); split = dimension * dimension
        if len(inputs) != split * 2: raise EvidenceError(f"transpose inputs differ: {case_id}")
        return "value", None, (transpose_values(matmul(inputs[:split], inputs[split:], dimension), dimension)
                              + matmul(transpose_values(inputs[split:], dimension), transpose_values(inputs[:split], dimension), dimension))
    if case_id.startswith("type_reject_"):
        if inputs: raise EvidenceError(f"compile-time inputs differ: {case_id}")
        return "compile_time_rejection", None, None
    nonfinite = re.fullmatch(r"mat([23])_nonfinite_(nan|posinf|neginf)_e([0-8])", case_id)
    if nonfinite:
        dimension, kind, index = int(nonfinite.group(1)), nonfinite.group(2), int(nonfinite.group(3))
        expected = {"nan": math.nan, "posinf": math.inf, "neginf": -math.inf}[kind]
        matrix = [float(row == column) for row in range(dimension) for column in range(dimension)]
        if index >= len(matrix): raise EvidenceError(f"non-finite placement differs: {case_id}")
        matrix[index] = expected
        if len(inputs) != len(matrix) or not all((math.isnan(wanted) and math.isnan(actual)) or actual == wanted for actual, wanted in zip(inputs, matrix)):
            raise EvidenceError(f"non-finite placement differs: {case_id}")
        return "error", "non_finite_input", None
    overflow = re.fullmatch(r"mat[23]_overflow_(application|composition|determinant)", case_id)
    if overflow:
        dimension = int(case_id[3])
        maximum = [sys_float_max() if row == col else 0. for row in range(dimension) for col in range(dimension)]
        suffix = ([2.] + [0.] * (dimension - 1) if overflow.group(1) == "application" else
                  [2. if row == col else 0. for row in range(dimension) for col in range(dimension)] if overflow.group(1) == "composition" else [])
        if inputs != maximum + suffix: raise EvidenceError(f"overflow inputs differ: {case_id}")
        return "error", "non_finite_result", None
    scale = re.fullmatch(r"scale_k_(m?)([018])", case_id)
    if scale:
        exponent = -int(scale.group(2)) if scale.group(1) else int(scale.group(2))
        factor = math.ldexp(1.0, exponent)
        if inputs != [factor]: raise EvidenceError(f"scale input differs: {case_id}")
        return "value", None, scale_results(factor)
    raise EvidenceError(f"independent oracle does not recognize {case_id}")


def sys_float_max() -> float:
    return float.fromhex("0x1.fffffffffffffp+1023")


def scale_results(scale: float) -> list[float]:
    bases = {2: [1., 2., 0., 1.], 3: [1., 1., 0., 0., 1., 1., 0., 0., 1.]}
    vectors = {2: [1., -1.], 3: [1., -1., 2.]}
    # Independent component formulas on the unscaled operands implement the RHS laws.
    applications = [scale * x for n in (2, 3) for x in matvec(bases[n], vectors[n], n)]
    transposes = [scale * x for n in (2, 3) for x in transpose_values(bases[n], n)]
    determinants = [scale ** n * determinant_value(bases[n], n) for n in (2, 3)]
    compositions = [scale * x for n in (2, 3) for x in matmul(bases[n], bases[n], n)]
    return applications + transposes + determinants + compositions + compositions


def canonical_hex(value: float) -> str:
    """Canonical exact numeric representation; signed zero is not a claim."""
    if not math.isfinite(value):
        raise EvidenceError("canonical hexadecimal comparison requires finite values")
    return "0x0p+0" if value == 0.0 else value.hex()


def result_matches(actual: tuple[str, str | None, list[float] | None], expected: tuple[str, str | None, list[float] | None]) -> bool:
    if actual[:2] != expected[:2]: return False
    if actual[2] is None or expected[2] is None: return actual[2] == expected[2]
    return len(actual[2]) == len(expected[2]) and all(
        canonical_hex(left) == canonical_hex(right)
        for left, right in zip(actual[2], expected[2], strict=True)
    )


def validate_certificate(profile: dict[str, Any], path: pathlib.Path) -> dict[str, Any]:
    certificate = read_json(path)
    if not isinstance(certificate, dict): raise EvidenceError("certificate must be an object")
    require_keys(certificate, {"schema_version", "kind", "environment", "source_checks", "cases"}, "certificate")
    if certificate["schema_version"] != 4 or certificate["kind"] != "minimal-small-linear-algebra-certificate": raise EvidenceError("certificate identity differs")
    if certificate["environment"] != {"double_radix": 2, "double_digits": 53, "iec559": True}: raise EvidenceError("certificate environment differs")
    if certificate["source_checks"] != {"mode": "external_command_required"}: raise EvidenceError("certificate source-check authority differs")
    if not isinstance(certificate["cases"], list): raise EvidenceError("certificate cases differ")
    seen: list[str] = []
    for row in certificate["cases"]:
        if not isinstance(row, dict): raise EvidenceError("certificate case is malformed")
        require_keys(row, {"schema_version", "id", "dimension", "operation", "claim_category", "input_layout", "inputs", "output_fields", "expected", "observed", "comparison", "non_claims"}, "certificate case")
        case_id = row["id"]
        if not isinstance(case_id, str) or case_id in seen: raise EvidenceError("certificate case identity differs")
        if (row["schema_version"], row["dimension"], row["operation"], row["claim_category"], row["input_layout"]) != (4, *metadata(case_id)):
            raise EvidenceError(f"certificate metadata differs: {case_id}")
        inputs = parse_values(row["inputs"], f"{case_id} inputs", finite=False)
        expected = expected_result(case_id, inputs)
        declared_expected = parse_result(row["expected"], f"{case_id} expected")
        observed = parse_result(row["observed"], f"{case_id} observed")
        if row["output_fields"] != output_fields(case_id) or len(row["output_fields"]) != len(observed[2] or []):
            raise EvidenceError(f"certificate output fields differ: {case_id}")
        if not result_matches(declared_expected, expected) or not result_matches(observed, expected):
            raise EvidenceError(f"certificate independent oracle differs: {case_id}")
        if row["comparison"] != EXACT_HEX_COMPARISON:
            raise EvidenceError(f"certificate comparison differs: {case_id}")
        non_claims = DETERMINANT_NON_CLAIMS if metadata(case_id)[1] in {"determinant", "power_two_scale_laws"} else []
        if row["non_claims"] != non_claims: raise EvidenceError(f"certificate non-claims differ: {case_id}")
        seen.append(case_id)
    if seen != profile["cases"]: raise EvidenceError("certificate case order or set differs")
    return certificate


def negative_mutation(baseline: dict[str, Any], case: str) -> tuple[dict[str, Any], str]:
    """One declared mutation and one exact expected rejection, independent of output labels."""
    altered = copy.deepcopy(baseline)
    rows = {row["id"]: row for row in altered["cases"]}
    nonfinite_id = "mat3_nonfinite_posinf_e8"
    row = rows[nonfinite_id]
    if case == "duplicate_case":
        altered["cases"].append(copy.deepcopy(altered["cases"][0]))
        reason = "certificate case identity differs"
    elif case == "forged_permutation":
        rows["mat3_cycle_application"]["observed"]["value"][0] = "0x1.1p+20"
        reason = "certificate independent oracle differs: mat3_cycle_application"
    elif case in {"wrong_nonfinite_placement", "incomplete_nonfinite_matrix", "altered_finite_matrix_entry"}:
        if case == "wrong_nonfinite_placement": row["inputs"][0], row["inputs"][8] = row["inputs"][8], row["inputs"][0]
        elif case == "incomplete_nonfinite_matrix": row["inputs"] = row["inputs"][-2:]
        else: row["inputs"][1] = "0x1p+0"
        reason = f"non-finite placement differs: {nonfinite_id}"
    elif case == "wrong_failure_error":
        row["observed"]["error"] = "non_finite_result"
        reason = f"certificate independent oracle differs: {nonfinite_id}"
    elif case == "undeclared_scale_field":
        rows["scale_k_1"]["output_fields"][-1] = "wrong.law"
        reason = "certificate output fields differ: scale_k_1"
    elif case == "missing_determinant_nonclaim":
        rows["mat2_swap_determinant"]["non_claims"] = []
        reason = "certificate non-claims differ: mat2_swap_determinant"
    elif case == "undeclared_signed_zero_policy":
        rows["mat2_quarter_turn_square"]["comparison"].pop("signed_zero_policy")
        reason = "certificate comparison differs: mat2_quarter_turn_square"
    elif case == "nonzero_one_ulp_mismatch":
        rows["mat2_quarter_turn_square"]["observed"]["value"][0] = "-0x1.0000000000001p+0"
        reason = "certificate independent oracle differs: mat2_quarter_turn_square"
    else: raise EvidenceError("unknown negative case")
    return altered, reason


def negative_outcome(profile: dict[str, Any], path: pathlib.Path, reason: str) -> dict[str, str]:
    try: validate_certificate(profile, path)
    except EvidenceError as error:
        if str(error) != reason: raise EvidenceError("negative rejected for an undeclared reason") from error
        return {"outcome": "REJECTED", "error_type": "EvidenceError", "reason": str(error)}
    raise EvidenceError("negative certificate was accepted")


def generate_negative_outcomes(profile: dict[str, Any], certificate: pathlib.Path, output: pathlib.Path) -> None:
    baseline = validate_certificate(profile, certificate)
    output.mkdir(parents=True, exist_ok=False)
    (output / "baseline.json").write_bytes(certificate.read_bytes())
    entries = []
    for case in NEGATIVE_CASES:
        altered, reason = negative_mutation(baseline, case)
        path = output / f"{case}.json"; write_json(path, altered)
        entries.append({"id": case, "path": path.name, "sha256": sha256(path),
                        "expected_reason": reason, "observed": negative_outcome(profile, path, reason)})
    write_json(output / "negative-outcomes.json", {
        "schema_version": 1, "kind": "minimal-small-linear-algebra-negative-outcomes",
        "baseline_sha256": sha256(certificate), "validator_sha256": sha256(pathlib.Path(__file__)),
        "entries": entries,
    })
    validate_negative_outcomes(profile, output / "negative-outcomes.json", sha256(certificate), sha256(pathlib.Path(__file__)))


def validate_negative_outcomes(profile: dict[str, Any], path: pathlib.Path, baseline_hash: str, validator_hash: str) -> dict[str, Any]:
    report = read_json(path)
    if not isinstance(report, dict): raise EvidenceError("negative outcomes must be an object")
    require_keys(report, {"schema_version", "kind", "baseline_sha256", "validator_sha256", "entries"}, "negative outcomes")
    if (report["schema_version"], report["kind"], report["baseline_sha256"], report["validator_sha256"]) != (1, "minimal-small-linear-algebra-negative-outcomes", baseline_hash, validator_hash):
        raise EvidenceError("negative outcomes identity differs")
    baseline_path = path.parent / "baseline.json"
    if sha256(baseline_path) != baseline_hash: raise EvidenceError("negative baseline binding differs")
    baseline = validate_certificate(profile, baseline_path)
    entries = report["entries"]
    if not isinstance(entries, list) or [entry.get("id") for entry in entries if isinstance(entry, dict)] != list(NEGATIVE_CASES):
        raise EvidenceError("negative outcome set differs")
    for entry in entries:
        require_keys(entry, {"id", "path", "sha256", "expected_reason", "observed"}, "negative outcome")
        if entry["path"] != f"{entry['id']}.json": raise EvidenceError("negative artifact path differs")
        artifact = path.parent / entry["path"]
        altered, reason = negative_mutation(baseline, entry["id"])
        if sha256(artifact) != entry["sha256"] or read_json(artifact) != altered or entry["expected_reason"] != reason:
            raise EvidenceError("negative mutation binding differs")
        if entry["observed"] != negative_outcome(profile, artifact, reason):
            raise EvidenceError("negative outcome recomputation differs")
    return report


def canonical_result_projection(result: dict[str, Any]) -> dict[str, Any]:
    projected = copy.deepcopy(result)
    if projected["outcome"] == "value":
        projected["value"] = [canonical_hex(as_float(value, "projection value")) for value in projected["value"]]
    return projected


def projection(certificate: dict[str, Any]) -> dict[str, Any]:
    cases = []
    for row in certificate["cases"]:
        projected = copy.deepcopy(row)
        projected["expected"] = canonical_result_projection(row["expected"])
        projected["observed"] = canonical_result_projection(row["observed"])
        cases.append(projected)
    return {"environment": certificate["environment"], "source_checks": certificate["source_checks"], "cases": cases}


def compare_certificates(profile: dict[str, Any], paths: list[pathlib.Path]) -> dict[str, Any]:
    if len(paths) != profile["repetitions_per_cell"]: raise EvidenceError("repeat count differs")
    certificates = [validate_certificate(profile, path) for path in paths]
    baseline = projection(certificates[0])
    if any(projection(certificate) != baseline for certificate in certificates[1:]): raise EvidenceError("within-cell certificate projections differ")
    return {"schema_version": 4, "kind": "minimal-small-linear-algebra-cell-comparison", "state": "EVIDENCE_COLLECTED_PENDING_AUDIT", "certificate_sha256": [sha256(path) for path in paths], "projection": baseline}


def compare_index(profile: dict[str, Any], index_path: pathlib.Path) -> dict[str, Any]:
    index = read_json(index_path)
    if not isinstance(index, dict): raise EvidenceError("certificate index is malformed")
    require_keys(index, {"schema_version", "kind", "entries"}, "certificate index")
    if index["schema_version"] != 1 or index["kind"] != "minimal-small-linear-algebra-certificate-index" or not isinstance(index["entries"], list): raise EvidenceError("certificate index identity differs")
    slots = [(cell, repeat) for cell in CELLS for repeat in range(1, profile["repetitions_per_cell"] + 1)]
    paths: dict[tuple[str, int], pathlib.Path] = {}
    for entry in index["entries"]:
        if not isinstance(entry, dict): raise EvidenceError("certificate index entry differs")
        require_keys(entry, {"cell", "repetition", "path", "sha256"}, "certificate index entry")
        slot = (entry["cell"], entry["repetition"]); path = (index_path.parent / entry["path"]).resolve()
        if slot not in slots or slot in paths or not isinstance(entry["sha256"], str) or not SHA256.fullmatch(entry["sha256"]) or not path.is_file() or sha256(path) != entry["sha256"]:
            raise EvidenceError("certificate index binding differs")
        paths[slot] = path
    if list(paths) != slots: raise EvidenceError("certificate index slots differ")
    cells = {cell: compare_certificates(profile, [paths[(cell, repeat)] for repeat in range(1, profile["repetitions_per_cell"] + 1)]) for cell in CELLS}
    baseline = cells[CELLS[0]]["projection"]
    if any(item["projection"] != baseline for item in cells.values()): raise EvidenceError("cross-cell certificate projections differ")
    return {"schema_version": 4, "kind": "minimal-small-linear-algebra-cross-cell-comparison", "state": "EVIDENCE_COLLECTED_PENDING_AUDIT", "cells": cells, "projection": baseline}


def main() -> int:
    parser = argparse.ArgumentParser(); parser.add_argument("--profile", required=True)
    commands = parser.add_subparsers(dest="command", required=True)
    commands.add_parser("validate-profile")
    certificate = commands.add_parser("validate-certificate"); certificate.add_argument("--certificate", required=True)
    negatives = commands.add_parser("negative-outcomes"); negatives.add_argument("--certificate", required=True); negatives.add_argument("--output", required=True)
    comparison = commands.add_parser("compare"); comparison.add_argument("--certificate", action="append", required=True); comparison.add_argument("--output", required=True)
    index = commands.add_parser("compare-index"); index.add_argument("--index", required=True); index.add_argument("--output", required=True)
    source = commands.add_parser("validate-source"); source.add_argument("--source-root", required=True); source.add_argument("--output", required=False)
    arguments = parser.parse_args(); profile = validate_profile(pathlib.Path(arguments.profile))
    if arguments.command == "validate-profile": return 0
    if arguments.command == "validate-certificate": validate_certificate(profile, pathlib.Path(arguments.certificate)); return 0
    if arguments.command == "negative-outcomes": generate_negative_outcomes(profile, pathlib.Path(arguments.certificate), pathlib.Path(arguments.output)); return 0
    if arguments.command == "compare": write_json(pathlib.Path(arguments.output), compare_certificates(profile, [pathlib.Path(path) for path in arguments.certificate])); return 0
    if arguments.command == "compare-index": write_json(pathlib.Path(arguments.output), compare_index(profile, pathlib.Path(arguments.index))); return 0
    result = validate_source_root(pathlib.Path(arguments.source_root))
    if arguments.output: write_json(pathlib.Path(arguments.output), {"schema_version": 1, "kind": "minimal-small-linear-algebra-source-check", "checks": result})
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
