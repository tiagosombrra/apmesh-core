#include "apmesh/math/linear_algebra.hpp"

#include "apmesh/core/numeric.hpp"

#include <array>
#include <expected>

namespace apmesh::core {
namespace {

[[nodiscard]] bool finite_entries(const std::array<double, 4>& entries) noexcept {
    for (const double entry : entries) {
        if (!is_finite(entry)) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] bool finite_entries(const std::array<double, 9>& entries) noexcept {
    for (const double entry : entries) {
        if (!is_finite(entry)) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] std::expected<double, LinearAlgebraError> finite_result(
    const double value) noexcept {
    if (!is_finite(value)) {
        return std::unexpected{LinearAlgebraError::non_finite_result};
    }
    return value;
}

} // namespace

std::expected<Mat2, LinearAlgebraError> Mat2::make(
    const std::array<double, 4> row_major_entries) noexcept {
    if (!finite_entries(row_major_entries)) {
        return std::unexpected{LinearAlgebraError::non_finite_input};
    }
    return Mat2{row_major_entries};
}

Mat2 Mat2::zero() noexcept {
    return Mat2{std::array<double, 4>{0.0, 0.0, 0.0, 0.0}};
}

Mat2 Mat2::identity() noexcept {
    return Mat2{std::array<double, 4>{1.0, 0.0, 0.0, 1.0}};
}

std::expected<double, LinearAlgebraError> Mat2::at(
    const std::size_t row,
    const std::size_t column) const noexcept {
    if (row >= 2 || column >= 2) {
        return std::unexpected{LinearAlgebraError::index_out_of_range};
    }
    return entries_[row * 2 + column];
}

std::expected<Mat3, LinearAlgebraError> Mat3::make(
    const std::array<double, 9> row_major_entries) noexcept {
    if (!finite_entries(row_major_entries)) {
        return std::unexpected{LinearAlgebraError::non_finite_input};
    }
    return Mat3{row_major_entries};
}

Mat3 Mat3::zero() noexcept {
    return Mat3{std::array<double, 9>{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}};
}

Mat3 Mat3::identity() noexcept {
    return Mat3{std::array<double, 9>{1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0}};
}

std::expected<double, LinearAlgebraError> Mat3::at(
    const std::size_t row,
    const std::size_t column) const noexcept {
    if (row >= 3 || column >= 3) {
        return std::unexpected{LinearAlgebraError::index_out_of_range};
    }
    return entries_[row * 3 + column];
}

Mat2 transpose(const Mat2& matrix) noexcept {
    return Mat2{std::array<double, 4>{
        matrix.entries_[0], matrix.entries_[2], matrix.entries_[1], matrix.entries_[3]}};
}

Mat3 transpose(const Mat3& matrix) noexcept {
    return Mat3{std::array<double, 9>{
        matrix.entries_[0], matrix.entries_[3], matrix.entries_[6],
        matrix.entries_[1], matrix.entries_[4], matrix.entries_[7],
        matrix.entries_[2], matrix.entries_[5], matrix.entries_[8]}};
}

std::expected<Mat2, LinearAlgebraError> compose(
    const Mat2& lhs,
    const Mat2& rhs) noexcept {
    const std::array<double, 4> entries{
        lhs.entries_[0] * rhs.entries_[0] + lhs.entries_[1] * rhs.entries_[2],
        lhs.entries_[0] * rhs.entries_[1] + lhs.entries_[1] * rhs.entries_[3],
        lhs.entries_[2] * rhs.entries_[0] + lhs.entries_[3] * rhs.entries_[2],
        lhs.entries_[2] * rhs.entries_[1] + lhs.entries_[3] * rhs.entries_[3]};
    if (!finite_entries(entries)) {
        return std::unexpected{LinearAlgebraError::non_finite_result};
    }
    return Mat2{entries};
}

std::expected<Mat3, LinearAlgebraError> compose(
    const Mat3& lhs,
    const Mat3& rhs) noexcept {
    const std::array<double, 9> entries{
        lhs.entries_[0] * rhs.entries_[0] + lhs.entries_[1] * rhs.entries_[3] + lhs.entries_[2] * rhs.entries_[6],
        lhs.entries_[0] * rhs.entries_[1] + lhs.entries_[1] * rhs.entries_[4] + lhs.entries_[2] * rhs.entries_[7],
        lhs.entries_[0] * rhs.entries_[2] + lhs.entries_[1] * rhs.entries_[5] + lhs.entries_[2] * rhs.entries_[8],
        lhs.entries_[3] * rhs.entries_[0] + lhs.entries_[4] * rhs.entries_[3] + lhs.entries_[5] * rhs.entries_[6],
        lhs.entries_[3] * rhs.entries_[1] + lhs.entries_[4] * rhs.entries_[4] + lhs.entries_[5] * rhs.entries_[7],
        lhs.entries_[3] * rhs.entries_[2] + lhs.entries_[4] * rhs.entries_[5] + lhs.entries_[5] * rhs.entries_[8],
        lhs.entries_[6] * rhs.entries_[0] + lhs.entries_[7] * rhs.entries_[3] + lhs.entries_[8] * rhs.entries_[6],
        lhs.entries_[6] * rhs.entries_[1] + lhs.entries_[7] * rhs.entries_[4] + lhs.entries_[8] * rhs.entries_[7],
        lhs.entries_[6] * rhs.entries_[2] + lhs.entries_[7] * rhs.entries_[5] + lhs.entries_[8] * rhs.entries_[8]};
    if (!finite_entries(entries)) {
        return std::unexpected{LinearAlgebraError::non_finite_result};
    }
    return Mat3{entries};
}

std::expected<double, LinearAlgebraError> determinant(const Mat2& matrix) noexcept {
    return finite_result(
        matrix.entries_[0] * matrix.entries_[3] - matrix.entries_[1] * matrix.entries_[2]);
}

std::expected<double, LinearAlgebraError> determinant(const Mat3& matrix) noexcept {
    return finite_result(
        matrix.entries_[0] * (matrix.entries_[4] * matrix.entries_[8] - matrix.entries_[5] * matrix.entries_[7]) -
        matrix.entries_[1] * (matrix.entries_[3] * matrix.entries_[8] - matrix.entries_[5] * matrix.entries_[6]) +
        matrix.entries_[2] * (matrix.entries_[3] * matrix.entries_[7] - matrix.entries_[4] * matrix.entries_[6]));
}

} // namespace apmesh::core
