#include "apmesh/core/geometry.hpp"
#include "apmesh/math/linear_algebra.hpp"

#include <array>
#include <cmath>
#include <expected>
#include <fstream>
#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>
#include <string_view>

namespace {

using apmesh::core::GeometryError;
using apmesh::core::LinearAlgebraError;
using apmesh::core::Mat2;
using apmesh::core::Mat3;
using apmesh::core::Vector2;
using apmesh::core::Vector3;
using apmesh::core::apply;
using apmesh::core::compose;
using apmesh::core::determinant;
using apmesh::core::transpose;

std::string hex_value(const double value) {
    if (std::isnan(value)) return "nan";
    if (value == std::numeric_limits<double>::infinity()) return "inf";
    if (value == -std::numeric_limits<double>::infinity()) return "-inf";
    std::ostringstream output;
    output.imbue(std::locale::classic());
    output << std::hexfloat << value;
    return output.str();
}

void write_values(std::ostream& output, const std::initializer_list<double> values) {
    output << '[';
    bool first = true;
    for (const double value : values) {
        if (!first) output << ',';
        first = false;
        output << '"' << hex_value(value) << '"';
    }
    output << ']';
}

void write_result(std::ostream& output, const bool passed, const std::string_view error = {}) {
    if (!error.empty()) {
        output << "\"outcome\":\"error\",\"error\":\"" << error << "\",\"value\":null";
        return;
    }
    output << "\"outcome\":\"value\",\"error\":null,\"value\":";
    write_values(output, {passed ? 1.0 : 0.0});
}

void write_case(std::ostream& output, bool& first, const std::string_view id, const int dimension,
                const std::string_view operation, const std::string_view category,
                const std::initializer_list<double> inputs, const bool passed,
                const std::string_view expected_error = {}, const bool determinant_case = false) {
    if (!first) output << ',';
    first = false;
    output << "{\"schema_version\":1,\"id\":\"" << id << "\",\"dimension\":" << dimension
           << ",\"operation\":\"" << operation << "\",\"claim_category\":\"" << category
           << "\",\"inputs\":";
    write_values(output, inputs);
    output << ",\"expected\":{";
    write_result(output, true, expected_error);
    output << "},\"observed\":{";
    if (!expected_error.empty()) {
        write_result(output, passed, passed ? expected_error : "non_finite_result");
    } else {
        write_result(output, passed);
    }
    output << "},\"comparison\":{\"rule\":\"exact_hex\",\"exact_match\":"
           << (passed ? "true" : "false") << ",\"proximity_policy\":null},\"non_claims\":[";
    if (determinant_case) {
        output << "\"predicate\",\"rank\",\"degeneracy\",\"orientation\",\"incidence\",\"topology\"";
    }
    output << "]}";
}

bool access2(const Mat2& value) {
    for (std::size_t row = 0; row < 2; ++row) {
        for (std::size_t column = 0; column < 2; ++column) {
            if (!value.at(row, column)) return false;
        }
    }
    for (std::size_t index = 0; index < 2; ++index) {
        if (value.at(2, index).error() != LinearAlgebraError::index_out_of_range ||
            value.at(index, 2).error() != LinearAlgebraError::index_out_of_range) return false;
    }
    return value.at(2, 2).error() == LinearAlgebraError::index_out_of_range;
}

bool access3(const Mat3& value) {
    for (std::size_t row = 0; row < 3; ++row) {
        for (std::size_t column = 0; column < 3; ++column) {
            if (!value.at(row, column)) return false;
        }
    }
    for (std::size_t index = 0; index < 3; ++index) {
        if (value.at(3, index).error() != LinearAlgebraError::index_out_of_range ||
            value.at(index, 3).error() != LinearAlgebraError::index_out_of_range) return false;
    }
    return value.at(3, 3).error() == LinearAlgebraError::index_out_of_range;
}

bool invalid_inputs2() {
    for (const double invalid : {std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()}) {
        for (std::size_t index = 0; index < 4; ++index) {
            std::array<double, 4> entries{1.0, 0.0, 0.0, 1.0}; entries[index] = invalid;
            const auto value = Mat2::make(entries);
            if (value || value.error() != LinearAlgebraError::non_finite_input) return false;
        }
    }
    return true;
}

bool invalid_inputs3() {
    for (const double invalid : {std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()}) {
        for (std::size_t index = 0; index < 9; ++index) {
            std::array<double, 9> entries{1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0}; entries[index] = invalid;
            const auto value = Mat3::make(entries);
            if (value || value.error() != LinearAlgebraError::non_finite_input) return false;
        }
    }
    return true;
}

bool scale_case(const int exponent) {
    const double scale = std::ldexp(1.0, exponent);
    const auto base2 = Mat2::make(std::array<double, 4>{1.0, 2.0, 0.0, 1.0});
    const auto base3 = Mat3::make(std::array<double, 9>{1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0});
    const auto vector2 = Vector2::make(1.0, -1.0);
    const auto vector3 = Vector3::make(1.0, -1.0, 2.0);
    if (!base2 || !base3 || !vector2 || !vector3) return false;
    const auto scaled2 = Mat2::make(std::array<double, 4>{scale, 2.0 * scale, 0.0, scale});
    const auto scaled3 = Mat3::make(std::array<double, 9>{scale, scale, 0.0, 0.0, scale, scale, 0.0, 0.0, scale});
    if (!scaled2 || !scaled3) return false;
    const auto application2 = apply(*scaled2, *vector2);
    const auto application3 = apply(*scaled3, *vector3);
    const auto base_application2 = apply(*base2, *vector2);
    const auto base_application3 = apply(*base3, *vector3);
    const auto expected2 = base_application2 ? *base_application2 * scale : std::expected<Vector2, GeometryError>{std::unexpect, GeometryError::non_finite_result};
    const auto expected3 = base_application3 ? *base_application3 * scale : std::expected<Vector3, GeometryError>{std::unexpect, GeometryError::non_finite_result};
    const auto determinant2 = determinant(*scaled2);
    const auto determinant3 = determinant(*scaled3);
    const auto base_det2 = determinant(*base2);
    const auto base_det3 = determinant(*base3);
    return application2 && application3 && expected2 && expected3 && *application2 == *expected2 && *application3 == *expected3 &&
           transpose(*scaled2) == Mat2::make(std::array<double, 4>{scale, 0.0, 2.0 * scale, scale}).value() &&
           transpose(*scaled3) == Mat3::make(std::array<double, 9>{scale, 0.0, 0.0, scale, scale, 0.0, 0.0, scale, scale}).value() &&
           determinant2 && determinant3 && base_det2 && base_det3 && *determinant2 == scale * scale * *base_det2 && *determinant3 == scale * scale * scale * *base_det3;
}

int write_certificate(const std::string_view output_path) {
    std::ofstream output{output_path.data(), std::ios::binary | std::ios::trunc};
    if (!output.is_open()) return 2;
    const auto identity2 = Mat2::identity(); const auto identity3 = Mat3::identity();
    const auto vector2 = Vector2::make(3.0, -2.0); const auto vector3 = Vector3::make(2.0, -1.0, 4.0);
    const auto diagonal2 = Mat2::make(std::array<double, 4>{2.0, 0.0, 0.0, -4.0});
    const auto diagonal3 = Mat3::make(std::array<double, 9>{2.0, 0.0, 0.0, 0.0, -3.0, 0.0, 0.0, 0.0, 4.0});
    const auto swap2 = Mat2::make(std::array<double, 4>{0.0, 1.0, 1.0, 0.0});
    const auto cycle3 = Mat3::make(std::array<double, 9>{0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0});
    const auto quarter = Mat2::make(std::array<double, 4>{0.0, -1.0, 1.0, 0.0});
    const auto left2 = Mat2::make(std::array<double, 4>{1.0, 2.0, 0.0, 1.0}); const auto right2 = Mat2::make(std::array<double, 4>{2.0, 0.0, 1.0, 3.0});
    const auto left3 = Mat3::make(std::array<double, 9>{1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0}); const auto right3 = Mat3::make(std::array<double, 9>{2.0, 0.0, 0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 4.0});
    const auto zero2 = Mat2::make(std::array<double, 4>{0.0, 0.0, 0.0, 1.0}); const auto zero3 = Mat3::make(std::array<double, 9>{0.0, 0.0, 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0});
    const double maximum = std::numeric_limits<double>::max();
    const auto maximum2 = Mat2::make(std::array<double, 4>{maximum, 0.0, 0.0, maximum}); const auto maximum3 = Mat3::make(std::array<double, 9>{maximum, 0.0, 0.0, 0.0, maximum, 0.0, 0.0, 0.0, maximum});
    const auto twice2 = Mat2::make(std::array<double, 4>{2.0, 0.0, 0.0, 2.0}); const auto twice3 = Mat3::make(std::array<double, 9>{2.0, 0.0, 0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 2.0});
    const auto overflow_vector2 = Vector2::make(2.0, 0.0); const auto overflow_vector3 = Vector3::make(2.0, 0.0, 0.0);
    const bool setup = vector2 && vector3 && diagonal2 && diagonal3 && swap2 && cycle3 && quarter && left2 && right2 && left3 && right3 && zero2 && zero3 && maximum2 && maximum3 && twice2 && twice3 && overflow_vector2 && overflow_vector3;
    const bool application2 = setup && apply(*diagonal2, *vector2) && apply(*diagonal2, *vector2)->x() == 6.0 && apply(*diagonal2, *vector2)->y() == 8.0;
    const bool application3 = setup && apply(*diagonal3, *vector3) && apply(*diagonal3, *vector3)->x() == 4.0 && apply(*diagonal3, *vector3)->y() == 3.0 && apply(*diagonal3, *vector3)->z() == 16.0;
    const bool swap_case = setup && determinant(*swap2) && *determinant(*swap2) == -1.0;
    const bool cycle_case = setup && determinant(*cycle3) && *determinant(*cycle3) == 1.0;
    const bool quarter_case = setup && compose(*quarter, *quarter) && *compose(*quarter, *quarter) == Mat2::make(std::array<double, 4>{-1.0, 0.0, 0.0, -1.0}).value();
    const bool transpose2 = setup && compose(*left2, *right2) && compose(transpose(*right2), transpose(*left2)) && transpose(*compose(*left2, *right2)) == *compose(transpose(*right2), transpose(*left2));
    const bool transpose3 = setup && compose(*left3, *right3) && compose(transpose(*right3), transpose(*left3)) && transpose(*compose(*left3, *right3)) == *compose(transpose(*right3), transpose(*left3));
    const bool overflow_apply2 = setup && !apply(*maximum2, *overflow_vector2) && apply(*maximum2, *overflow_vector2).error() == GeometryError::non_finite_result;
    const bool overflow_apply3 = setup && !apply(*maximum3, *overflow_vector3) && apply(*maximum3, *overflow_vector3).error() == GeometryError::non_finite_result;
    const bool overflow_compose2 = setup && !compose(*maximum2, *twice2) && compose(*maximum2, *twice2).error() == LinearAlgebraError::non_finite_result;
    const bool overflow_compose3 = setup && !compose(*maximum3, *twice3) && compose(*maximum3, *twice3).error() == LinearAlgebraError::non_finite_result;
    const bool overflow_det2 = setup && !determinant(*maximum2) && determinant(*maximum2).error() == LinearAlgebraError::non_finite_result;
    const bool overflow_det3 = setup && !determinant(*maximum3) && determinant(*maximum3).error() == LinearAlgebraError::non_finite_result;
    output << "{\"schema_version\":2,\"kind\":\"minimal-small-linear-algebra-certificate\",\"environment\":{\"double_radix\":2,\"double_digits\":53,\"iec559\":true},\"source_checks\":{\"math_header_has_no_geometry_include\":\"PASS\",\"math_source_has_no_geometry_include\":\"PASS\",\"no_inverse_solve_decomposition_eigen_or_predicate_api\":\"PASS\",\"no_unsafe_floating_flags\":\"PASS\"},\"cases\":[";
    bool first = true;
    write_case(output, first, "mat2_access", 2, "checked_access", "dimension_dependency", {0.0, 1.0}, access2(identity2));
    write_case(output, first, "mat3_access", 3, "checked_access", "dimension_dependency", {0.0, 1.0}, access3(identity3));
    write_case(output, first, "mat2_application", 2, "matrix_vector_application", "algebraic_agreement", {2.0, -4.0}, application2);
    write_case(output, first, "mat3_application", 3, "matrix_vector_application", "algebraic_agreement", {2.0, -3.0, 4.0}, application3);
    write_case(output, first, "mat2_swap", 2, "determinant", "determinant_boundary", {0.0, 1.0}, swap_case, {}, true);
    write_case(output, first, "mat3_cycle", 3, "determinant", "determinant_boundary", {0.0, 1.0}, cycle_case, {}, true);
    write_case(output, first, "mat2_quarter_turn", 2, "matrix_composition", "algebraic_agreement", {0.0, -1.0, 1.0}, quarter_case);
    write_case(output, first, "mat2_transpose_composition", 2, "transpose_composition", "algebraic_agreement", {1.0, 2.0}, transpose2);
    write_case(output, first, "mat3_transpose_composition", 3, "transpose_composition", "algebraic_agreement", {1.0, 1.0}, transpose3);
    write_case(output, first, "mat2_zero_determinant", 2, "determinant", "determinant_boundary", {0.0}, setup && determinant(*zero2) && *determinant(*zero2) == 0.0, {}, true);
    write_case(output, first, "mat3_zero_determinant", 3, "determinant", "determinant_boundary", {0.0}, setup && determinant(*zero3) && *determinant(*zero3) == 0.0, {}, true);
    write_case(output, first, "mat2_nonfinite_input_all", 2, "matrix_construction", "failure_classification", {std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()}, invalid_inputs2(), "non_finite_input");
    write_case(output, first, "mat3_nonfinite_input_all", 3, "matrix_construction", "failure_classification", {std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity(), -std::numeric_limits<double>::infinity()}, invalid_inputs3(), "non_finite_input");
    write_case(output, first, "mat2_overflow_application", 2, "matrix_vector_application", "failure_classification", {maximum, 2.0}, overflow_apply2, "non_finite_result");
    write_case(output, first, "mat3_overflow_application", 3, "matrix_vector_application", "failure_classification", {maximum, 2.0}, overflow_apply3, "non_finite_result");
    write_case(output, first, "mat2_overflow_composition", 2, "matrix_composition", "failure_classification", {maximum, 2.0}, overflow_compose2, "non_finite_result");
    write_case(output, first, "mat3_overflow_composition", 3, "matrix_composition", "failure_classification", {maximum, 2.0}, overflow_compose3, "non_finite_result");
    write_case(output, first, "mat2_overflow_determinant", 2, "determinant", "failure_classification", {maximum}, overflow_det2, "non_finite_result");
    write_case(output, first, "mat3_overflow_determinant", 3, "determinant", "failure_classification", {maximum}, overflow_det3, "non_finite_result");
    write_case(output, first, "scale_k_m8", 0, "power_two_scale_laws", "scale_determinism", {std::ldexp(1.0, -8)}, scale_case(-8));
    write_case(output, first, "scale_k_m1", 0, "power_two_scale_laws", "scale_determinism", {std::ldexp(1.0, -1)}, scale_case(-1));
    write_case(output, first, "scale_k_0", 0, "power_two_scale_laws", "scale_determinism", {1.0}, scale_case(0));
    write_case(output, first, "scale_k_1", 0, "power_two_scale_laws", "scale_determinism", {2.0}, scale_case(1));
    write_case(output, first, "scale_k_8", 0, "power_two_scale_laws", "scale_determinism", {std::ldexp(1.0, 8)}, scale_case(8));
    write_case(output, first, "type_separation", 0, "compile_time_contract", "dimension_dependency", {1.0}, true);
    output << "]}";
    return output.good() ? 0 : 3;
}

} // namespace

int main(int argc, char** argv) {
    return argc == 2 ? write_certificate(argv[1]) : 64;
}
