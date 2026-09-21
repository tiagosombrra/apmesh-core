#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <expected>
#include <limits>

namespace apmesh::core::detail {

enum class IntervalError {
    non_finite_bound,
};

struct ClosedInterval {
    double lower{};
    double upper{};

    [[nodiscard]] bool operator==(const ClosedInterval&) const noexcept = default;
};

[[nodiscard]] inline std::expected<ClosedInterval, IntervalError> point_interval(
    const double value) noexcept {
    if (!std::isfinite(value)) {
        return std::unexpected{IntervalError::non_finite_bound};
    }
    return ClosedInterval{value, value};
}

[[nodiscard]] inline std::expected<double, IntervalError> widen_down(
    const double value) noexcept {
    if (!std::isfinite(value)) {
        return std::unexpected{IntervalError::non_finite_bound};
    }
    const double widened = std::nextafter(
        value,
        -std::numeric_limits<double>::infinity());
    if (!std::isfinite(widened)) {
        return std::unexpected{IntervalError::non_finite_bound};
    }
    return widened;
}

[[nodiscard]] inline std::expected<double, IntervalError> widen_up(
    const double value) noexcept {
    if (!std::isfinite(value)) {
        return std::unexpected{IntervalError::non_finite_bound};
    }
    const double widened = std::nextafter(
        value,
        std::numeric_limits<double>::infinity());
    if (!std::isfinite(widened)) {
        return std::unexpected{IntervalError::non_finite_bound};
    }
    return widened;
}

[[nodiscard]] inline std::expected<ClosedInterval, IntervalError> widened(
    const double lower,
    const double upper) noexcept {
    const auto widened_lower = widen_down(lower);
    const auto widened_upper = widen_up(upper);
    if (!widened_lower || !widened_upper) {
        return std::unexpected{IntervalError::non_finite_bound};
    }
    return ClosedInterval{*widened_lower, *widened_upper};
}

[[nodiscard]] inline std::expected<ClosedInterval, IntervalError> add(
    const ClosedInterval& lhs,
    const ClosedInterval& rhs) noexcept {
    return widened(lhs.lower + rhs.lower, lhs.upper + rhs.upper);
}

[[nodiscard]] inline std::expected<ClosedInterval, IntervalError> subtract(
    const ClosedInterval& lhs,
    const ClosedInterval& rhs) noexcept {
    return widened(lhs.lower - rhs.upper, lhs.upper - rhs.lower);
}

[[nodiscard]] inline std::expected<ClosedInterval, IntervalError> multiply(
    const ClosedInterval& lhs,
    const ClosedInterval& rhs) noexcept {
    const std::array<double, 4> products{
        lhs.lower * rhs.lower,
        lhs.lower * rhs.upper,
        lhs.upper * rhs.lower,
        lhs.upper * rhs.upper,
    };
    for (const double product : products) {
        if (!std::isfinite(product)) {
            return std::unexpected{IntervalError::non_finite_bound};
        }
    }
    const auto [minimum, maximum] = std::minmax_element(
        products.begin(),
        products.end());
    return widened(*minimum, *maximum);
}

[[nodiscard]] inline std::expected<ClosedInterval, IntervalError> multiply(
    const ClosedInterval& value,
    const double scalar) noexcept {
    const auto scalar_interval = point_interval(scalar);
    if (!scalar_interval) {
        return std::unexpected{scalar_interval.error()};
    }
    return multiply(value, *scalar_interval);
}

[[nodiscard]] inline std::expected<ClosedInterval, IntervalError> divide_positive(
    const ClosedInterval& value,
    const double positive_scalar) noexcept {
    if (!(positive_scalar > 0.0) || !std::isfinite(positive_scalar)) {
        return std::unexpected{IntervalError::non_finite_bound};
    }
    return widened(
        value.lower / positive_scalar,
        value.upper / positive_scalar);
}

[[nodiscard]] inline std::expected<ClosedInterval, IntervalError> midpoint(
    const ClosedInterval& lhs,
    const ClosedInterval& rhs) noexcept {
    const auto half_lhs = multiply(lhs, 0.5);
    const auto half_rhs = multiply(rhs, 0.5);
    if (!half_lhs || !half_rhs) {
        return std::unexpected{IntervalError::non_finite_bound};
    }
    return add(*half_lhs, *half_rhs);
}

template <std::size_t Dimension>
using IntervalVector = std::array<ClosedInterval, Dimension>;

template <std::size_t Dimension>
[[nodiscard]] inline std::expected<ClosedInterval, IntervalError> dot(
    const IntervalVector<Dimension>& lhs,
    const IntervalVector<Dimension>& rhs) noexcept {
    auto total = point_interval(0.0);
    if (!total) {
        return std::unexpected{total.error()};
    }

    for (std::size_t index = 0; index < Dimension; ++index) {
        const auto product = multiply(lhs[index], rhs[index]);
        if (!product) {
            return std::unexpected{product.error()};
        }
        total = add(*total, *product);
        if (!total) {
            return std::unexpected{total.error()};
        }
    }
    return total;
}

[[nodiscard]] inline std::expected<
    std::pair<std::array<ClosedInterval, 5>, std::array<ClosedInterval, 5>>,
    IntervalError>
subdivide_quartic_midpoint(
    const std::array<ClosedInterval, 5>& coefficients) noexcept {
    std::array<std::array<ClosedInterval, 5>, 5> levels{};
    levels[0] = coefficients;

    for (std::size_t level = 1; level < 5; ++level) {
        for (std::size_t index = 0; index < 5 - level; ++index) {
            const auto value = midpoint(
                levels[level - 1][index],
                levels[level - 1][index + 1]);
            if (!value) {
                return std::unexpected{value.error()};
            }
            levels[level][index] = *value;
        }
    }

    std::array<ClosedInterval, 5> left{};
    std::array<ClosedInterval, 5> right{};
    for (std::size_t index = 0; index < 5; ++index) {
        left[index] = levels[index][0];
        right[index] = levels[4 - index][index];
    }
    return std::pair{left, right};
}

[[nodiscard]] inline bool strictly_positive(
    const std::array<ClosedInterval, 5>& coefficients) noexcept {
    return std::all_of(
        coefficients.begin(),
        coefficients.end(),
        [](const ClosedInterval& coefficient) {
            return coefficient.lower > 0.0;
        });
}

} // namespace apmesh::core::detail
