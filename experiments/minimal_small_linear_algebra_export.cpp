#include "apmesh/core/geometry.hpp"
#include "apmesh/math/linear_algebra.hpp"

#include <array>
#include <bit>
#include <cmath>
#include <concepts>
#include <cstdint>
#include <expected>
#include <fstream>
#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

using apmesh::core::GeometryError;
using apmesh::core::LinearAlgebraError;
using apmesh::core::Mat2;
using apmesh::core::Mat3;
using apmesh::core::Point2;
using apmesh::core::Point3;
using apmesh::core::Vector2;
using apmesh::core::Vector3;
using apmesh::core::apply;
using apmesh::core::compose;
using apmesh::core::determinant;
using apmesh::core::transpose;

template <typename Matrix, typename Operand>
concept Applicable = requires(const Matrix& matrix, const Operand& operand) {
    apply(matrix, operand);
};

template <typename Left, typename Right>
concept Composable = requires(const Left& left, const Right& right) {
    compose(left, right);
};

static_assert(!Applicable<Mat2, Point2>);
static_assert(!Applicable<Mat3, Point3>);
static_assert(!Applicable<Mat2, Vector3>);
static_assert(!Applicable<Mat3, Vector2>);
static_assert(!Composable<Mat2, Mat3>);
static_assert(!Composable<Mat3, Mat2>);

using Values = std::vector<double>;

struct Result {
    std::string outcome;
    std::string error;
    Values value;
};

std::string hex_value(const double value) {
    if (std::isnan(value)) return "nan";
    if (value == std::numeric_limits<double>::infinity()) return "inf";
    if (value == -std::numeric_limits<double>::infinity()) return "-inf";
    std::ostringstream output;
    output.imbue(std::locale::classic());
    output << std::hexfloat << value;
    return output.str();
}

void write_values(std::ostream& output, const Values& values) {
    output << '[';
    bool first = true;
    for (const double value : values) {
        if (!first) output << ',';
        first = false;
        output << '"' << hex_value(value) << '"';
    }
    output << ']';
}

void write_result(std::ostream& output, const Result& result) {
    output << "\"outcome\":\"" << result.outcome << "\",\"error\":";
    if (result.error.empty()) output << "null";
    else output << '"' << result.error << '"';
    output << ",\"value\":";
    if (result.outcome == "value") write_values(output, result.value);
    else output << "null";
}

Result value(Values entries) {
    return {"value", {}, std::move(entries)};
}

Result error(const std::string_view name) {
    return {"error", std::string{name}, {}};
}

Result compile_time_rejection() {
    return {"compile_time_rejection", {}, {}};
}

bool canonical_exact_match(const Result& expected, const Result& observed) {
    if (expected.outcome != observed.outcome || expected.error != observed.error
        || expected.value.size() != observed.value.size()) return false;
    for (std::size_t index = 0; index < expected.value.size(); ++index) {
        const double left = expected.value[index];
        const double right = observed.value[index];
        if (left == 0.0 && right == 0.0) continue;
        if (std::bit_cast<std::uint64_t>(left) != std::bit_cast<std::uint64_t>(right)) return false;
    }
    return true;
}

std::string error_name(const LinearAlgebraError value) {
    switch (value) {
    case LinearAlgebraError::non_finite_input: return "non_finite_input";
    case LinearAlgebraError::non_finite_result: return "non_finite_result";
    case LinearAlgebraError::index_out_of_range: return "index_out_of_range";
    }
    return "unknown";
}

std::string error_name(const GeometryError value) {
    switch (value) {
    case GeometryError::non_finite_input: return "non_finite_input";
    case GeometryError::non_finite_result: return "non_finite_result";
    default: return "unknown";
    }
}

Result result_of(const std::expected<double, LinearAlgebraError>& input) {
    if (!input) return error(error_name(input.error()));
    return value({*input});
}

Result result_of(const std::expected<Mat2, LinearAlgebraError>& input);
Result result_of(const std::expected<Mat3, LinearAlgebraError>& input);
Result result_of(const std::expected<Vector2, GeometryError>& input);
Result result_of(const std::expected<Vector3, GeometryError>& input);

Values entries(const Mat2& matrix) {
    Values result;
    for (std::size_t row = 0; row < 2; ++row) {
        for (std::size_t column = 0; column < 2; ++column) result.push_back(*matrix.at(row, column));
    }
    return result;
}

Values entries(const Mat3& matrix) {
    Values result;
    for (std::size_t row = 0; row < 3; ++row) {
        for (std::size_t column = 0; column < 3; ++column) result.push_back(*matrix.at(row, column));
    }
    return result;
}

Values entries(const Vector2& vector) { return {vector.x(), vector.y()}; }
Values entries(const Vector3& vector) { return {vector.x(), vector.y(), vector.z()}; }

Result result_of(const std::expected<Mat2, LinearAlgebraError>& input) {
    if (!input) return error(error_name(input.error()));
    return value(entries(*input));
}

Result result_of(const std::expected<Mat3, LinearAlgebraError>& input) {
    if (!input) return error(error_name(input.error()));
    return value(entries(*input));
}

Result result_of(const std::expected<Vector2, GeometryError>& input) {
    if (!input) return error(error_name(input.error()));
    return value(entries(*input));
}

Result result_of(const std::expected<Vector3, GeometryError>& input) {
    if (!input) return error(error_name(input.error()));
    return value(entries(*input));
}

Values join(Values first, const Values& second) {
    first.insert(first.end(), second.begin(), second.end());
    return first;
}

Values join(Values first, const std::initializer_list<double> second) {
    first.insert(first.end(), second.begin(), second.end());
    return first;
}

std::vector<std::string> output_fields(const int dimension, const std::string_view operation,
                                     const Result& expected) {
    std::vector<std::string> fields;
    const auto matrix = [&fields](const std::string& prefix, const int size) {
        for (int row = 0; row < size; ++row)
            for (int column = 0; column < size; ++column)
                fields.push_back(prefix + ".r" + std::to_string(row) + "c" + std::to_string(column));
    };
    const auto vector = [&fields](const std::string& prefix, const int size) {
        for (int axis = 0; axis < size; ++axis) fields.push_back(prefix + "." + std::string(1, "xyz"[axis]));
    };
    if (expected.outcome != "value") return fields;
    if (operation == "power_two_scale_laws") {
        vector("mat2.scaled_application", 2); vector("mat3.scaled_application", 3);
        matrix("mat2.scaled_transpose", 2); matrix("mat3.scaled_transpose", 3);
        fields.push_back("mat2.scaled_determinant"); fields.push_back("mat3.scaled_determinant");
        matrix("mat2.left_scaled_composition", 2); matrix("mat3.left_scaled_composition", 3);
        matrix("mat2.right_scaled_composition", 2); matrix("mat3.right_scaled_composition", 3);
    } else if (operation == "matrix_vector_application") vector("result", dimension);
    else if (operation == "determinant" || operation == "checked_access") fields.push_back("scalar");
    else if (operation == "noncommuting_composition") { matrix("AB", dimension); matrix("BA", dimension); }
    else if (operation == "transpose_composition") { matrix("transpose_AB", dimension); matrix("transpose_B_transpose_A", dimension); }
    else matrix("result", dimension);
    return fields;
}

void write_case(std::ostream& output, bool& first, const std::string_view id,
                const int dimension, const std::string_view operation,
                const std::string_view category, const std::string_view input_layout,
                const Values& inputs, const Result& expected, const Result& observed,
                const bool determinant_case = false) {
    if (!first) output << ',';
    first = false;
    output << "{\"schema_version\":4,\"id\":\"" << id << "\",\"dimension\":" << dimension
           << ",\"operation\":\"" << operation << "\",\"claim_category\":\"" << category
           << "\",\"input_layout\":\"" << input_layout << "\",\"inputs\":";
    write_values(output, inputs);
    output << ",\"output_fields\":[";
    bool first_field = true;
    for (const auto& field : output_fields(dimension, operation, expected)) {
        if (!first_field) output << ',';
        first_field = false; output << '"' << field << '"';
    }
    output << ']';
    output << ",\"expected\":{";
    write_result(output, expected);
    output << "},\"observed\":{";
    write_result(output, observed);
    output << "},\"comparison\":{\"rule\":\"exact_hex\",\"signed_zero_policy\":\"normalize_to_positive\",\"exact_match\":"
           << (canonical_exact_match(expected, observed) ? "true" : "false")
           << ",\"proximity_policy\":null},\"non_claims\":[";
    if (determinant_case) {
        output << "\"predicate\",\"rank\",\"degeneracy\",\"orientation\",\"incidence\",\"topology\"";
    }
    output << "]}";
}

void write_access_cases(std::ostream& output, bool& first, const Mat2& matrix) {
    const Values matrix_values = entries(matrix);
    for (std::size_t row = 0; row < 2; ++row) {
        for (std::size_t column = 0; column < 2; ++column) {
            const Values input = join(matrix_values, {static_cast<double>(row), static_cast<double>(column)});
            write_case(output, first, std::string{"mat2_access_r"} + std::to_string(row) + "c" + std::to_string(column), 2,
                       "checked_access", "dimension_dependency", "matrix_row_major,row,column", input,
                       value({matrix_values[row * 2 + column]}), result_of(matrix.at(row, column)));
        }
    }
    write_case(output, first, "mat2_access_row_oob", 2, "checked_access", "dimension_dependency",
               "matrix_row_major,row,column", join(matrix_values, {2.0, 0.0}), error("index_out_of_range"), result_of(matrix.at(2, 0)));
    write_case(output, first, "mat2_access_column_oob", 2, "checked_access", "dimension_dependency",
               "matrix_row_major,row,column", join(matrix_values, {0.0, 2.0}), error("index_out_of_range"), result_of(matrix.at(0, 2)));
}

void write_access_cases(std::ostream& output, bool& first, const Mat3& matrix) {
    const Values matrix_values = entries(matrix);
    for (std::size_t row = 0; row < 3; ++row) {
        for (std::size_t column = 0; column < 3; ++column) {
            const Values input = join(matrix_values, {static_cast<double>(row), static_cast<double>(column)});
            write_case(output, first, std::string{"mat3_access_r"} + std::to_string(row) + "c" + std::to_string(column), 3,
                       "checked_access", "dimension_dependency", "matrix_row_major,row,column", input,
                       value({matrix_values[row * 3 + column]}), result_of(matrix.at(row, column)));
        }
    }
    write_case(output, first, "mat3_access_row_oob", 3, "checked_access", "dimension_dependency",
               "matrix_row_major,row,column", join(matrix_values, {3.0, 0.0}), error("index_out_of_range"), result_of(matrix.at(3, 0)));
    write_case(output, first, "mat3_access_column_oob", 3, "checked_access", "dimension_dependency",
               "matrix_row_major,row,column", join(matrix_values, {0.0, 3.0}), error("index_out_of_range"), result_of(matrix.at(0, 3)));
}

void write_nonfinite_cases(std::ostream& output, bool& first, const int dimension) {
    const std::array<std::pair<std::string_view, double>, 3> invalid_values{{
        {"nan", std::numeric_limits<double>::quiet_NaN()},
        {"posinf", std::numeric_limits<double>::infinity()},
        {"neginf", -std::numeric_limits<double>::infinity()},
    }};
    const std::size_t count = dimension == 2 ? 4U : 9U;
    for (const auto& [name, invalid] : invalid_values) {
        for (std::size_t index = 0; index < count; ++index) {
            const std::string id = "mat" + std::to_string(dimension) + "_nonfinite_" + std::string(name) + "_e" + std::to_string(index);
            if (dimension == 2) {
                std::array<double, 4> input{1.0, 0.0, 0.0, 1.0}; input[index] = invalid;
                write_case(output, first, id, 2, "matrix_construction", "failure_classification", "matrix_row_major",
                           Values(input.begin(), input.end()), error("non_finite_input"), result_of(Mat2::make(input)));
            } else {
                std::array<double, 9> input{1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0}; input[index] = invalid;
                write_case(output, first, id, 3, "matrix_construction", "failure_classification", "matrix_row_major",
                           Values(input.begin(), input.end()), error("non_finite_input"), result_of(Mat3::make(input)));
            }
        }
    }
}

Values scaled_results(const double scale) {
    return {
        -scale, -scale,
        0.0, scale, 2.0 * scale,
        scale, 0.0, 2.0 * scale, scale,
        scale, 0.0, 0.0, scale, scale, 0.0, 0.0, scale, scale,
        scale * scale, scale * scale * scale,
        scale, 4.0 * scale, 0.0, scale,
        scale, 2.0 * scale, scale, 0.0, scale, 2.0 * scale, 0.0, 0.0, scale,
        scale, 4.0 * scale, 0.0, scale,
        scale, 2.0 * scale, scale, 0.0, scale, 2.0 * scale, 0.0, 0.0, scale,
    };
}

Result observed_scale_results(const double scale) {
    const auto base2 = Mat2::make(std::array<double, 4>{1.0, 2.0, 0.0, 1.0});
    const auto base3 = Mat3::make(std::array<double, 9>{1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0});
    const auto scaled2 = Mat2::make(std::array<double, 4>{scale, 2.0 * scale, 0.0, scale});
    const auto scaled3 = Mat3::make(std::array<double, 9>{scale, scale, 0.0, 0.0, scale, scale, 0.0, 0.0, scale});
    const auto vector2 = Vector2::make(1.0, -1.0);
    const auto vector3 = Vector3::make(1.0, -1.0, 2.0);
    if (!base2 || !base3 || !scaled2 || !scaled3 || !vector2 || !vector3) return error("non_finite_result");
    const auto application2 = apply(*scaled2, *vector2);
    const auto application3 = apply(*scaled3, *vector3);
    const auto determinant2 = determinant(*scaled2);
    const auto determinant3 = determinant(*scaled3);
    const auto composition2 = compose(*scaled2, *base2);
    const auto composition3 = compose(*scaled3, *base3);
    const auto right_composition2 = compose(*base2, *scaled2);
    const auto right_composition3 = compose(*base3, *scaled3);
    if (!application2 || !application3 || !determinant2 || !determinant3 || !composition2 || !composition3 || !right_composition2 || !right_composition3) return error("non_finite_result");
    Values result = entries(*application2);
    result = join(std::move(result), entries(*application3));
    result = join(std::move(result), entries(transpose(*scaled2)));
    result = join(std::move(result), entries(transpose(*scaled3)));
    result.push_back(*determinant2); result.push_back(*determinant3);
    result = join(std::move(result), entries(*composition2));
    result = join(std::move(result), entries(*composition3));
    result = join(std::move(result), entries(*right_composition2));
    result = join(std::move(result), entries(*right_composition3));
    return value(std::move(result));
}

int write_certificate(const std::string_view output_path) {
    std::ofstream output{output_path.data(), std::ios::binary | std::ios::trunc};
    if (!output.is_open()) return 2;

    const Mat2 zero2 = Mat2::zero(); const Mat2 identity2 = Mat2::identity();
    const Mat3 zero3 = Mat3::zero(); const Mat3 identity3 = Mat3::identity();
    const auto diagonal2 = Mat2::make(std::array<double, 4>{2.0, 0.0, 0.0, -4.0});
    const auto diagonal3 = Mat3::make(std::array<double, 9>{2.0, 0.0, 0.0, 0.0, -3.0, 0.0, 0.0, 0.0, 4.0});
    const auto vector2 = Vector2::make(3.0, -2.0); const auto vector3 = Vector3::make(2.0, -1.0, 4.0);
    const auto swap2 = Mat2::make(std::array<double, 4>{0.0, 1.0, 1.0, 0.0});
    const auto cycle3 = Mat3::make(std::array<double, 9>{0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0});
    const auto quarter = Mat2::make(std::array<double, 4>{0.0, -1.0, 1.0, 0.0});
    const auto left2 = Mat2::make(std::array<double, 4>{1.0, 2.0, 0.0, 1.0}); const auto right2 = Mat2::make(std::array<double, 4>{2.0, 0.0, 1.0, 3.0});
    const auto left3 = Mat3::make(std::array<double, 9>{1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0}); const auto right3 = Mat3::make(std::array<double, 9>{2.0, 0.0, 0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 4.0});
    const auto zero_row2 = Mat2::make(std::array<double, 4>{0.0, 0.0, 0.0, 1.0}); const auto zero_row3 = Mat3::make(std::array<double, 9>{0.0, 0.0, 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0});
    const double maximum = std::numeric_limits<double>::max();
    const auto maximum2 = Mat2::make(std::array<double, 4>{maximum, 0.0, 0.0, maximum}); const auto maximum3 = Mat3::make(std::array<double, 9>{maximum, 0.0, 0.0, 0.0, maximum, 0.0, 0.0, 0.0, maximum});
    const auto twice2 = Mat2::make(std::array<double, 4>{2.0, 0.0, 0.0, 2.0}); const auto twice3 = Mat3::make(std::array<double, 9>{2.0, 0.0, 0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 2.0});
    const auto overflow_vector2 = Vector2::make(2.0, 0.0); const auto overflow_vector3 = Vector3::make(2.0, 0.0, 0.0);
    if (!diagonal2 || !diagonal3 || !vector2 || !vector3 || !swap2 || !cycle3 || !quarter || !left2 || !right2 || !left3 || !right3 || !zero_row2 || !zero_row3 || !maximum2 || !maximum3 || !twice2 || !twice3 || !overflow_vector2 || !overflow_vector3) return 3;

    output << "{\"schema_version\":4,\"kind\":\"minimal-small-linear-algebra-certificate\",\"environment\":{\"double_radix\":2,\"double_digits\":53,\"iec559\":true},\"source_checks\":{\"mode\":\"external_command_required\"},\"cases\":[";
    bool first = true;
    write_case(output, first, "mat2_zero", 2, "zero", "algebraic_agreement", "none", {}, value({0.0, 0.0, 0.0, 0.0}), value(entries(zero2)));
    write_case(output, first, "mat2_identity", 2, "identity", "algebraic_agreement", "none", {}, value({1.0, 0.0, 0.0, 1.0}), value(entries(identity2)));
    write_case(output, first, "mat3_zero", 3, "zero", "algebraic_agreement", "none", {}, value({0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}), value(entries(zero3)));
    write_case(output, first, "mat3_identity", 3, "identity", "algebraic_agreement", "none", {}, value({1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0}), value(entries(identity3)));
    write_access_cases(output, first, identity2); write_access_cases(output, first, identity3);

    write_case(output, first, "mat2_diagonal_application", 2, "matrix_vector_application", "algebraic_agreement", "matrix_row_major,vector", join(entries(*diagonal2), entries(*vector2)), value({6.0, 8.0}), result_of(apply(*diagonal2, *vector2)));
    write_case(output, first, "mat3_diagonal_application", 3, "matrix_vector_application", "algebraic_agreement", "matrix_row_major,vector", join(entries(*diagonal3), entries(*vector3)), value({4.0, 3.0, 16.0}), result_of(apply(*diagonal3, *vector3)));
    write_case(output, first, "mat2_swap_determinant", 2, "determinant", "determinant_boundary", "matrix_row_major", entries(*swap2), value({-1.0}), result_of(determinant(*swap2)), true);
    write_case(output, first, "mat3_cycle_determinant", 3, "determinant", "determinant_boundary", "matrix_row_major", entries(*cycle3), value({1.0}), result_of(determinant(*cycle3)), true);
    write_case(output, first, "mat2_swap_application", 2, "matrix_vector_application", "algebraic_agreement", "matrix_row_major,vector", join(entries(*swap2), entries(*vector2)), value({-2.0, 3.0}), result_of(apply(*swap2, *vector2)));
    write_case(output, first, "mat3_cycle_application", 3, "matrix_vector_application", "algebraic_agreement", "matrix_row_major,vector", join(entries(*cycle3), entries(*vector3)), value({-1.0, 4.0, 2.0}), result_of(apply(*cycle3, *vector3)));
    const auto basis2 = Vector2::make(1.0, 0.0);
    write_case(output, first, "mat2_quarter_turn_application", 2, "matrix_vector_application", "algebraic_agreement", "matrix_row_major,vector", join(entries(*quarter), entries(*basis2)), value({0.0, 1.0}), result_of(apply(*quarter, *basis2)));
    write_case(output, first, "mat2_quarter_turn_square", 2, "matrix_composition", "algebraic_agreement", "lhs_row_major,rhs_row_major", join(entries(*quarter), entries(*quarter)), value({-1.0, 0.0, 0.0, -1.0}), result_of(compose(*quarter, *quarter)));
    const auto left_right2 = compose(*left2, *right2); const auto right_left2 = compose(*right2, *left2);
    const auto left_right3 = compose(*left3, *right3); const auto right_left3 = compose(*right3, *left3);
    if (!left_right2 || !right_left2 || !left_right3 || !right_left3) return 3;
    write_case(output, first, "mat2_noncommuting_composition", 2, "noncommuting_composition", "algebraic_agreement", "lhs_row_major,rhs_row_major", join(entries(*left2), entries(*right2)), value({4.0, 6.0, 1.0, 3.0, 2.0, 4.0, 1.0, 5.0}), value(join(entries(*left_right2), entries(*right_left2))));
    write_case(output, first, "mat3_noncommuting_composition", 3, "noncommuting_composition", "algebraic_agreement", "lhs_row_major,rhs_row_major", join(entries(*left3), entries(*right3)), value({2.0, 3.0, 0.0, 0.0, 3.0, 4.0, 0.0, 0.0, 4.0, 2.0, 2.0, 0.0, 0.0, 3.0, 3.0, 0.0, 0.0, 4.0}), value(join(entries(*left_right3), entries(*right_left3))));
    const auto transposed_product2 = compose(transpose(*right2), transpose(*left2));
    const auto transposed_product3 = compose(transpose(*right3), transpose(*left3));
    if (!transposed_product2 || !transposed_product3) return 3;
    write_case(output, first, "mat2_transpose_composition", 2, "transpose_composition", "algebraic_agreement", "lhs_row_major,rhs_row_major", join(entries(*left2), entries(*right2)), value({4.0, 1.0, 6.0, 3.0, 4.0, 1.0, 6.0, 3.0}), value(join(entries(transpose(*left_right2)), entries(*transposed_product2))));
    write_case(output, first, "mat3_transpose_composition", 3, "transpose_composition", "algebraic_agreement", "lhs_row_major,rhs_row_major", join(entries(*left3), entries(*right3)), value({2.0, 0.0, 0.0, 3.0, 3.0, 0.0, 0.0, 4.0, 4.0, 2.0, 0.0, 0.0, 3.0, 3.0, 0.0, 0.0, 4.0, 4.0}), value(join(entries(transpose(*left_right3)), entries(*transposed_product3))));
    write_case(output, first, "mat2_zero_row_determinant", 2, "determinant", "determinant_boundary", "matrix_row_major", entries(*zero_row2), value({0.0}), result_of(determinant(*zero_row2)), true);
    write_case(output, first, "mat3_zero_row_determinant", 3, "determinant", "determinant_boundary", "matrix_row_major", entries(*zero_row3), value({0.0}), result_of(determinant(*zero_row3)), true);
    write_case(output, first, "type_reject_mat2_point2", 2, "compile_time_rejection", "dimension_dependency", "Mat2,Point2", {}, compile_time_rejection(), compile_time_rejection());
    write_case(output, first, "type_reject_mat3_point3", 3, "compile_time_rejection", "dimension_dependency", "Mat3,Point3", {}, compile_time_rejection(), compile_time_rejection());
    write_case(output, first, "type_reject_mat2_vector3", 2, "compile_time_rejection", "dimension_dependency", "Mat2,Vector3", {}, compile_time_rejection(), compile_time_rejection());
    write_case(output, first, "type_reject_mat2_mat3", 2, "compile_time_rejection", "dimension_dependency", "Mat2,Mat3", {}, compile_time_rejection(), compile_time_rejection());
    write_case(output, first, "type_reject_mat3_vector2", 3, "compile_time_rejection", "dimension_dependency", "Mat3,Vector2", {}, compile_time_rejection(), compile_time_rejection());
    write_case(output, first, "type_reject_mat3_mat2", 3, "compile_time_rejection", "dimension_dependency", "Mat3,Mat2", {}, compile_time_rejection(), compile_time_rejection());
    write_nonfinite_cases(output, first, 2); write_nonfinite_cases(output, first, 3);
    write_case(output, first, "mat2_overflow_application", 2, "matrix_vector_application", "failure_classification", "matrix_row_major,vector", join(entries(*maximum2), entries(*overflow_vector2)), error("non_finite_result"), result_of(apply(*maximum2, *overflow_vector2)));
    write_case(output, first, "mat3_overflow_application", 3, "matrix_vector_application", "failure_classification", "matrix_row_major,vector", join(entries(*maximum3), entries(*overflow_vector3)), error("non_finite_result"), result_of(apply(*maximum3, *overflow_vector3)));
    write_case(output, first, "mat2_overflow_composition", 2, "matrix_composition", "failure_classification", "lhs_row_major,rhs_row_major", join(entries(*maximum2), entries(*twice2)), error("non_finite_result"), result_of(compose(*maximum2, *twice2)));
    write_case(output, first, "mat3_overflow_composition", 3, "matrix_composition", "failure_classification", "lhs_row_major,rhs_row_major", join(entries(*maximum3), entries(*twice3)), error("non_finite_result"), result_of(compose(*maximum3, *twice3)));
    write_case(output, first, "mat2_overflow_determinant", 2, "determinant", "failure_classification", "matrix_row_major", entries(*maximum2), error("non_finite_result"), result_of(determinant(*maximum2)), true);
    write_case(output, first, "mat3_overflow_determinant", 3, "determinant", "failure_classification", "matrix_row_major", entries(*maximum3), error("non_finite_result"), result_of(determinant(*maximum3)), true);
    for (const int exponent : {-8, -1, 0, 1, 8}) {
        const double scale = std::ldexp(1.0, exponent);
        const std::string id = exponent < 0 ? "scale_k_m" + std::to_string(-exponent) : "scale_k_" + std::to_string(exponent);
        write_case(output, first, id, 0, "power_two_scale_laws", "scale_determinism", "power_of_two_scale", {scale}, value(scaled_results(scale)), observed_scale_results(scale), true);
    }
    output << "]}";
    return output.good() ? 0 : 4;
}

} // namespace

int main(int argc, char** argv) {
    return argc == 2 ? write_certificate(argv[1]) : 64;
}
