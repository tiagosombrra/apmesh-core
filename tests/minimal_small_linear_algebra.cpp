#include "apmesh/core/geometry.hpp"
#include "apmesh/math/linear_algebra.hpp"

#include <array>
#include <cstddef>
#include <concepts>
#include <expected>
#include <iostream>
#include <limits>
#include <string_view>
#include <type_traits>
#include <utility>

namespace {

template <typename Matrix, typename Vector>
concept Applicable = requires(const Matrix& matrix, const Vector& vector) {
    apmesh::core::apply(matrix, vector);
};

template <typename Left, typename Right>
concept Composable = requires(const Left& left, const Right& right) {
    apmesh::core::compose(left, right);
};

bool require(const bool condition, const std::string_view message) {
    if (!condition) {
        std::cerr << message << '\n';
    }
    return condition;
}

template <typename Value, typename Error>
bool require_value(
    const std::expected<Value, Error>& value,
    const std::string_view message) {
    return require(value.has_value(), message);
}

template <typename Value, typename Error>
bool require_error(
    const std::expected<Value, Error>& value,
    const Error expected,
    const std::string_view message) {
    return require(!value.has_value() && value.error() == expected, message);
}

template <typename Matrix>
bool require_matrix(
    const Matrix& actual,
    const Matrix& expected,
    const std::size_t dimension,
    const std::string_view message) {
    for (std::size_t row = 0; row < dimension; ++row) {
        for (std::size_t column = 0; column < dimension; ++column) {
            const auto actual_entry = actual.at(row, column);
            const auto expected_entry = expected.at(row, column);
            if (!actual_entry || !expected_entry || *actual_entry != *expected_entry) {
                return require(false, message);
            }
        }
    }
    return true;
}

} // namespace

int main() {
    using apmesh::core::GeometryError;
    using apmesh::core::LinearAlgebraError;
    using apmesh::core::Mat2;
    using apmesh::core::Mat3;
    using apmesh::core::Point2;
    using apmesh::core::Vector2;
    using apmesh::core::Vector3;

    static_assert(!std::is_same_v<Mat2, Mat3>);
    static_assert(Applicable<Mat2, Vector2>);
    static_assert(Applicable<Mat3, Vector3>);
    static_assert(!Applicable<Mat2, Point2>);
    static_assert(!Applicable<Mat2, Vector3>);
    static_assert(Composable<Mat2, Mat2>);
    static_assert(Composable<Mat3, Mat3>);
    static_assert(!Composable<Mat2, Mat3>);

    bool passed = true;

    const auto identity2 = Mat2::identity();
    const auto identity3 = Mat3::identity();
    const auto expected_zero2 = Mat2::make(std::array<double, 4>{0.0, 0.0, 0.0, 0.0});
    const auto expected_zero3 = Mat3::make(std::array<double, 9>{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0});
    passed = require(
                 expected_zero2 && require_matrix(Mat2::zero(), *expected_zero2, 2, "Mat2 zero did not contain exact zeros"),
                 "Mat2 zero construction failed") &&
             passed;
    passed = require(
                 expected_zero3 && require_matrix(Mat3::zero(), *expected_zero3, 3, "Mat3 zero did not contain exact zeros"),
                 "Mat3 zero construction failed") &&
             passed;
    passed = require_matrix(transpose(transpose(identity2)), identity2, 2, "Mat2 double transpose failed") && passed;
    passed = require_matrix(transpose(transpose(identity3)), identity3, 3, "Mat3 double transpose failed") && passed;
    passed = require_error(identity2.at(2, 0), LinearAlgebraError::index_out_of_range, "Mat2 row overflow was accepted") && passed;
    passed = require_error(identity2.at(0, 2), LinearAlgebraError::index_out_of_range, "Mat2 column overflow was accepted") && passed;
    passed = require_error(identity3.at(3, 0), LinearAlgebraError::index_out_of_range, "Mat3 row overflow was accepted") && passed;
    passed = require_error(identity3.at(0, 3), LinearAlgebraError::index_out_of_range, "Mat3 column overflow was accepted") && passed;

    const auto vector2 = Vector2::make(0.5, 0.25);
    const auto vector3 = Vector3::make(0.5, -0.5, 0.25);
    passed = require_value(vector2, "Vector2 setup failed") && passed;
    passed = require_value(vector3, "Vector3 setup failed") && passed;
    if (vector2 && vector3) {
        const auto applied2 = apply(identity2, *vector2);
        const auto applied3 = apply(identity3, *vector3);
        passed = require_value(applied2, "Mat2 identity application failed") && passed;
        passed = require_value(applied3, "Mat3 identity application failed") && passed;
        passed = require(applied2 && *applied2 == *vector2, "Mat2 identity application changed vector") && passed;
        passed = require(applied3 && *applied3 == *vector3, "Mat3 identity application changed vector") && passed;
    }

    const auto diagonal2 = Mat2::make(std::array<double, 4>{4.0, 0.0, 0.0, -8.0});
    const auto diagonal3 = Mat3::make(std::array<double, 9>{2.0, 0.0, 0.0, 0.0, 4.0, 0.0, 0.0, 0.0, -8.0});
    passed = require_value(diagonal2, "Mat2 diagonal construction failed") && passed;
    passed = require_value(diagonal3, "Mat3 diagonal construction failed") && passed;
    if (diagonal2 && diagonal3 && vector2 && vector3) {
        const auto scaled2 = apply(*diagonal2, *vector2);
        const auto scaled3 = apply(*diagonal3, *vector3);
        const auto expected2 = Vector2::make(2.0, -2.0);
        const auto expected3 = Vector3::make(1.0, -2.0, -2.0);
        passed = require(scaled2 && expected2 && *scaled2 == *expected2, "Mat2 diagonal application failed") && passed;
        passed = require(scaled3 && expected3 && *scaled3 == *expected3, "Mat3 diagonal application failed") && passed;
    }

    const auto swap2 = Mat2::make(std::array<double, 4>{0.0, 1.0, 1.0, 0.0});
    const auto cyclic3 = Mat3::make(std::array<double, 9>{0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0});
    passed = require_value(swap2, "Mat2 permutation construction failed") && passed;
    passed = require_value(cyclic3, "Mat3 permutation construction failed") && passed;
    if (swap2 && cyclic3) {
        const auto determinant2 = determinant(*swap2);
        const auto determinant3 = determinant(*cyclic3);
        const auto permuted2 = vector2 ? apply(*swap2, *vector2) : std::expected<Vector2, GeometryError>{std::unexpected{GeometryError::non_finite_result}};
        const auto permuted3 = vector3 ? apply(*cyclic3, *vector3) : std::expected<Vector3, GeometryError>{std::unexpected{GeometryError::non_finite_result}};
        const auto expected2 = Vector2::make(0.25, 0.5);
        const auto expected3 = Vector3::make(-0.5, 0.25, 0.5);
        passed = require(determinant2 && *determinant2 == -1.0, "Mat2 permutation determinant failed") && passed;
        passed = require(determinant3 && *determinant3 == 1.0, "Mat3 permutation determinant failed") && passed;
        passed = require(permuted2 && expected2 && *permuted2 == *expected2, "Mat2 permutation application failed") && passed;
        passed = require(permuted3 && expected3 && *permuted3 == *expected3, "Mat3 permutation application failed") && passed;
    }

    const auto quarter_turn = Mat2::make(std::array<double, 4>{0.0, -1.0, 1.0, 0.0});
    const auto basis2 = Vector2::make(1.0, 0.0);
    passed = require_value(quarter_turn, "Quarter-turn construction failed") && passed;
    passed = require_value(basis2, "Mat2 basis setup failed") && passed;
    if (quarter_turn && basis2) {
        const auto rotated = apply(*quarter_turn, *basis2);
        const auto expected = Vector2::make(0.0, 1.0);
        const auto half_turn = compose(*quarter_turn, *quarter_turn);
        const auto expected_half_turn = Mat2::make(std::array<double, 4>{-1.0, 0.0, 0.0, -1.0});
        passed = require(rotated && expected && *rotated == *expected, "Quarter-turn basis application failed") && passed;
        passed = require(half_turn && expected_half_turn && *half_turn == *expected_half_turn, "Quarter-turn composition failed") && passed;
    }

    const auto left = Mat2::make(std::array<double, 4>{1.0, 2.0, 0.0, 1.0});
    const auto right = Mat2::make(std::array<double, 4>{2.0, 0.0, 0.0, 3.0});
    passed = require_value(left, "Mat2 composition left setup failed") && passed;
    passed = require_value(right, "Mat2 composition right setup failed") && passed;
    if (left && right) {
        const auto product = compose(*left, *right);
        const auto reverse_transpose = compose(transpose(*right), transpose(*left));
        const auto expected_product = Mat2::make(std::array<double, 4>{2.0, 6.0, 0.0, 3.0});
        passed = require_value(product, "Mat2 composition failed") && passed;
        passed = require_value(reverse_transpose, "Mat2 transpose composition failed") && passed;
        passed = require(product && expected_product && *product == *expected_product, "Mat2 composition formula failed") && passed;
        passed = require(
                     product && reverse_transpose &&
                         require_matrix(transpose(*product), *reverse_transpose, 2, "Mat2 transpose composition law failed"),
                     "Mat2 transpose composition setup failed") &&
                 passed;
    }

    const auto left3 = Mat3::make(std::array<double, 9>{1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0});
    const auto right3 = Mat3::make(std::array<double, 9>{2.0, 0.0, 0.0, 0.0, 3.0, 0.0, 0.0, 0.0, 4.0});
    const auto expected_product3 = Mat3::make(std::array<double, 9>{2.0, 3.0, 0.0, 0.0, 3.0, 4.0, 0.0, 0.0, 4.0});
    passed = require(left3 && right3 && expected_product3, "Mat3 composition setup failed") && passed;
    if (left3 && right3 && expected_product3) {
        const auto product3 = compose(*left3, *right3);
        const auto reverse_transpose3 = compose(transpose(*right3), transpose(*left3));
        passed = require(product3 && *product3 == *expected_product3, "Mat3 composition formula failed") && passed;
        passed = require(
                     product3 && reverse_transpose3 &&
                         require_matrix(transpose(*product3), *reverse_transpose3, 3, "Mat3 transpose composition law failed"),
                     "Mat3 transpose composition setup failed") &&
                 passed;
    }

    const auto zero_row3 = Mat3::make(std::array<double, 9>{0.0, 0.0, 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0});
    passed = require_value(zero_row3, "Zero-row Mat3 construction failed") && passed;
    if (zero_row3) {
        const auto determinant3 = determinant(*zero_row3);
        passed = require(determinant3 && *determinant3 == 0.0, "Zero-row determinant was not algebraic zero") && passed;
    }

    const std::array<double, 3> invalid_values{
        std::numeric_limits<double>::quiet_NaN(),
        std::numeric_limits<double>::infinity(),
        -std::numeric_limits<double>::infinity()};
    for (const double invalid : invalid_values) {
        for (std::size_t index = 0; index < 4; ++index) {
            std::array<double, 4> entries{1.0, 0.0, 0.0, 1.0};
            entries[index] = invalid;
            passed = require_error(Mat2::make(entries), LinearAlgebraError::non_finite_input, "Mat2 accepted invalid entry") && passed;
        }
        for (std::size_t index = 0; index < 9; ++index) {
            std::array<double, 9> entries{1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0};
            entries[index] = invalid;
            passed = require_error(Mat3::make(entries), LinearAlgebraError::non_finite_input, "Mat3 accepted invalid entry") && passed;
        }
    }

    const double maximum = std::numeric_limits<double>::max();
    const auto maximum2 = Mat2::make(std::array<double, 4>{maximum, 0.0, 0.0, maximum});
    const auto twice_identity2 = Mat2::make(std::array<double, 4>{2.0, 0.0, 0.0, 2.0});
    const auto maximum3 = Mat3::make(std::array<double, 9>{maximum, 0.0, 0.0, 0.0, maximum, 0.0, 0.0, 0.0, maximum});
    const auto twice_identity3 = Mat3::make(std::array<double, 9>{2.0, 0.0, 0.0, 0.0, 2.0, 0.0, 0.0, 0.0, 2.0});
    const auto overflow_vector2 = Vector2::make(2.0, 0.0);
    const auto overflow_vector3 = Vector3::make(2.0, 0.0, 0.0);
    passed = require(maximum2 && twice_identity2 && maximum3 && twice_identity3 && overflow_vector2 && overflow_vector3, "Overflow setup failed") && passed;
    if (maximum2 && twice_identity2 && maximum3 && twice_identity3 && overflow_vector2 && overflow_vector3) {
        passed = require_error(compose(*maximum2, *twice_identity2), LinearAlgebraError::non_finite_result, "Mat2 composition overflow was accepted") && passed;
        passed = require_error(compose(*maximum3, *twice_identity3), LinearAlgebraError::non_finite_result, "Mat3 composition overflow was accepted") && passed;
        passed = require_error(determinant(*maximum2), LinearAlgebraError::non_finite_result, "Mat2 determinant overflow was accepted") && passed;
        passed = require_error(determinant(*maximum3), LinearAlgebraError::non_finite_result, "Mat3 determinant overflow was accepted") && passed;
        passed = require_error(apply(*maximum2, *overflow_vector2), GeometryError::non_finite_result, "Mat2 application overflow was accepted") && passed;
        passed = require_error(apply(*maximum3, *overflow_vector3), GeometryError::non_finite_result, "Mat3 application overflow was accepted") && passed;
    }

    const auto scale2 = Mat2::make(std::array<double, 4>{8.0, 0.0, 0.0, -16.0});
    const auto scale3 = Mat3::make(std::array<double, 9>{4.0, 0.0, 0.0, 0.0, 8.0, 0.0, 0.0, 0.0, -16.0});
    if (diagonal2 && diagonal3 && scale2 && scale3 && vector2 && vector3) {
        const auto scaled_application2 = apply(*scale2, *vector2);
        const auto base_application2 = apply(*diagonal2, *vector2);
        const auto scaled_application3 = apply(*scale3, *vector3);
        const auto base_application3 = apply(*diagonal3, *vector3);
        const auto scale_two_base2 = base_application2 ? *base_application2 * 2.0 : std::expected<Vector2, GeometryError>{std::unexpected{GeometryError::non_finite_result}};
        const auto scale_two_base3 = base_application3 ? *base_application3 * 2.0 : std::expected<Vector3, GeometryError>{std::unexpected{GeometryError::non_finite_result}};
        passed = require(scaled_application2 && scale_two_base2 && *scaled_application2 == *scale_two_base2, "Mat2 power-of-two application law failed") && passed;
        passed = require(scaled_application3 && scale_two_base3 && *scaled_application3 == *scale_two_base3, "Mat3 power-of-two application law failed") && passed;

        const auto determinant_base2 = determinant(*diagonal2);
        const auto determinant_scaled2 = determinant(*scale2);
        const auto determinant_base3 = determinant(*diagonal3);
        const auto determinant_scaled3 = determinant(*scale3);
        passed = require(determinant_base2 && determinant_scaled2 && *determinant_scaled2 == 4.0 * *determinant_base2, "Mat2 determinant scale law failed") && passed;
        passed = require(determinant_base3 && determinant_scaled3 && *determinant_scaled3 == 8.0 * *determinant_base3, "Mat3 determinant scale law failed") && passed;
    }

    const auto scaled_left2 = Mat2::make(std::array<double, 4>{2.0, 4.0, 0.0, 2.0});
    const auto scaled_right2 = Mat2::make(std::array<double, 4>{4.0, 0.0, 0.0, 6.0});
    const auto scaled_product2 = Mat2::make(std::array<double, 4>{4.0, 12.0, 0.0, 6.0});
    passed = require(left && right && scaled_left2 && scaled_right2 && scaled_product2, "Mat2 scale composition setup failed") && passed;
    if (left && right && scaled_left2 && scaled_right2 && scaled_product2) {
        const auto left_scaled_product = compose(*scaled_left2, *right);
        const auto right_scaled_product = compose(*left, *scaled_right2);
        const auto scaled_transpose = Mat2::make(std::array<double, 4>{2.0, 0.0, 4.0, 2.0});
        passed = require(left_scaled_product && *left_scaled_product == *scaled_product2, "Mat2 left scale composition law failed") && passed;
        passed = require(right_scaled_product && *right_scaled_product == *scaled_product2, "Mat2 right scale composition law failed") && passed;
        passed = require(scaled_transpose && transpose(*scaled_left2) == *scaled_transpose, "Mat2 transpose scale law failed") && passed;
    }

    const auto scaled_left3 = Mat3::make(std::array<double, 9>{2.0, 2.0, 0.0, 0.0, 2.0, 2.0, 0.0, 0.0, 2.0});
    const auto scaled_right3 = Mat3::make(std::array<double, 9>{4.0, 0.0, 0.0, 0.0, 6.0, 0.0, 0.0, 0.0, 8.0});
    const auto scaled_product3 = Mat3::make(std::array<double, 9>{4.0, 6.0, 0.0, 0.0, 6.0, 8.0, 0.0, 0.0, 8.0});
    passed = require(left3 && right3 && scaled_left3 && scaled_right3 && scaled_product3, "Mat3 scale composition setup failed") && passed;
    if (left3 && right3 && scaled_left3 && scaled_right3 && scaled_product3) {
        const auto left_scaled_product3 = compose(*scaled_left3, *right3);
        const auto right_scaled_product3 = compose(*left3, *scaled_right3);
        const auto scaled_transpose3 = Mat3::make(std::array<double, 9>{2.0, 0.0, 0.0, 2.0, 2.0, 0.0, 0.0, 2.0, 2.0});
        passed = require(left_scaled_product3 && *left_scaled_product3 == *scaled_product3, "Mat3 left scale composition law failed") && passed;
        passed = require(right_scaled_product3 && *right_scaled_product3 == *scaled_product3, "Mat3 right scale composition law failed") && passed;
        passed = require(scaled_transpose3 && transpose(*scaled_left3) == *scaled_transpose3, "Mat3 transpose scale law failed") && passed;
    }

    return passed ? 0 : 1;
}
