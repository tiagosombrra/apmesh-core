#pragma once

#include <array>
#include <cstddef>
#include <expected>

namespace apmesh::core {

enum class LinearAlgebraError {
    non_finite_input,
    non_finite_result,
    index_out_of_range,
};

class Mat2 {
public:
    [[nodiscard]] static std::expected<Mat2, LinearAlgebraError> make(
        std::array<double, 4> row_major_entries) noexcept;

    [[nodiscard]] static Mat2 zero() noexcept;
    [[nodiscard]] static Mat2 identity() noexcept;

    [[nodiscard]] std::expected<double, LinearAlgebraError> at(
        std::size_t row,
        std::size_t column) const noexcept;

    [[nodiscard]] bool operator==(const Mat2&) const noexcept = default;

private:
    constexpr explicit Mat2(std::array<double, 4> row_major_entries) noexcept
        : entries_(row_major_entries) {}

    std::array<double, 4> entries_;

    friend Mat2 transpose(const Mat2&) noexcept;
    friend std::expected<Mat2, LinearAlgebraError> compose(
        const Mat2&,
        const Mat2&) noexcept;
    friend std::expected<double, LinearAlgebraError> determinant(const Mat2&) noexcept;
};

class Mat3 {
public:
    [[nodiscard]] static std::expected<Mat3, LinearAlgebraError> make(
        std::array<double, 9> row_major_entries) noexcept;

    [[nodiscard]] static Mat3 zero() noexcept;
    [[nodiscard]] static Mat3 identity() noexcept;

    [[nodiscard]] std::expected<double, LinearAlgebraError> at(
        std::size_t row,
        std::size_t column) const noexcept;

    [[nodiscard]] bool operator==(const Mat3&) const noexcept = default;

private:
    constexpr explicit Mat3(std::array<double, 9> row_major_entries) noexcept
        : entries_(row_major_entries) {}

    std::array<double, 9> entries_;

    friend Mat3 transpose(const Mat3&) noexcept;
    friend std::expected<Mat3, LinearAlgebraError> compose(
        const Mat3&,
        const Mat3&) noexcept;
    friend std::expected<double, LinearAlgebraError> determinant(const Mat3&) noexcept;
};

[[nodiscard]] Mat2 transpose(const Mat2& matrix) noexcept;
[[nodiscard]] Mat3 transpose(const Mat3& matrix) noexcept;

[[nodiscard]] std::expected<Mat2, LinearAlgebraError> compose(
    const Mat2& lhs,
    const Mat2& rhs) noexcept;
[[nodiscard]] std::expected<Mat3, LinearAlgebraError> compose(
    const Mat3& lhs,
    const Mat3& rhs) noexcept;

[[nodiscard]] std::expected<double, LinearAlgebraError> determinant(
    const Mat2& matrix) noexcept;
[[nodiscard]] std::expected<double, LinearAlgebraError> determinant(
    const Mat3& matrix) noexcept;

} // namespace apmesh::core
