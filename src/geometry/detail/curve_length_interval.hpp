#pragma once

#include "curve_regularity_interval.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <expected>
#include <utility>

namespace apmesh::core::detail {

template <std::size_t Dimension>
[[nodiscard]] inline std::expected<IntervalVector<Dimension>, IntervalError>
add_vectors(
    const IntervalVector<Dimension>& lhs,
    const IntervalVector<Dimension>& rhs) noexcept {
    IntervalVector<Dimension> result{};
    for (std::size_t index = 0; index < Dimension; ++index) {
        if (lhs[index].lower == 0.0 && lhs[index].upper == 0.0 &&
            rhs[index].lower == 0.0 && rhs[index].upper == 0.0) {
            result[index] = ClosedInterval{0.0, 0.0};
            continue;
        }
        const auto value = add(lhs[index], rhs[index]);
        if (!value) {
            return std::unexpected{value.error()};
        }
        result[index] = *value;
    }
    return result;
}

template <std::size_t Dimension>
[[nodiscard]] inline std::expected<IntervalVector<Dimension>, IntervalError>
scale_vector(
    const IntervalVector<Dimension>& value,
    const double scalar) noexcept {
    IntervalVector<Dimension> result{};
    for (std::size_t index = 0; index < Dimension; ++index) {
        if (value[index].lower == 0.0 && value[index].upper == 0.0) {
            result[index] = ClosedInterval{0.0, 0.0};
            continue;
        }
        const auto component = multiply(value[index], scalar);
        if (!component) {
            return std::unexpected{component.error()};
        }
        result[index] = *component;
    }
    return result;
}

[[nodiscard]] inline std::pair<double, double> absolute_bounds(
    const ClosedInterval& value) noexcept {
    if (value.lower <= 0.0 && value.upper >= 0.0) {
        return {0.0, std::max(std::abs(value.lower), std::abs(value.upper))};
    }
    const double lower = std::min(std::abs(value.lower), std::abs(value.upper));
    const double upper = std::max(std::abs(value.lower), std::abs(value.upper));
    return {lower, upper};
}

template <std::size_t Dimension>
[[nodiscard]] inline std::expected<ClosedInterval, IntervalError>
euclidean_norm_bounds(
    const IntervalVector<Dimension>& value) noexcept {
    static_assert(Dimension == 2 || Dimension == 3);

    std::array<double, Dimension> lower_components{};
    std::array<double, Dimension> upper_components{};
    for (std::size_t index = 0; index < Dimension; ++index) {
        const auto [lower, upper] = absolute_bounds(value[index]);
        lower_components[index] = lower;
        upper_components[index] = upper;
    }

    double lower_raw = 0.0;
    double upper_raw = 0.0;
    if constexpr (Dimension == 2) {
        lower_raw = std::hypot(lower_components[0], lower_components[1]);
        upper_raw = std::hypot(upper_components[0], upper_components[1]);
    } else {
        if (lower_components[2] == 0.0 && upper_components[2] == 0.0) {
            lower_raw = std::hypot(lower_components[0], lower_components[1]);
            upper_raw = std::hypot(upper_components[0], upper_components[1]);
        } else {
            lower_raw = std::hypot(
                lower_components[0],
                lower_components[1],
                lower_components[2]);
            upper_raw = std::hypot(
                upper_components[0],
                upper_components[1],
                upper_components[2]);
        }
    }

    if (!std::isfinite(lower_raw) || !std::isfinite(upper_raw)) {
        return std::unexpected{IntervalError::non_finite_bound};
    }

    double lower = 0.0;
    if (lower_raw > 0.0) {
        const auto widened_lower = widen_down(lower_raw);
        if (!widened_lower) {
            return std::unexpected{widened_lower.error()};
        }
        lower = std::max(0.0, *widened_lower);
    }

    double upper = 0.0;
    if (upper_raw > 0.0) {
        const auto widened_upper = widen_up(upper_raw);
        if (!widened_upper) {
            return std::unexpected{widened_upper.error()};
        }
        upper = *widened_upper;
    }

    if (lower > upper) {
        return std::unexpected{IntervalError::non_finite_bound};
    }
    return ClosedInterval{lower, upper};
}

template <std::size_t Dimension>
[[nodiscard]] inline std::expected<
    std::pair<std::array<IntervalVector<Dimension>, 3>,
              std::array<IntervalVector<Dimension>, 3>>,
    IntervalError>
subdivide_cubic_edges_midpoint(
    const std::array<IntervalVector<Dimension>, 3>& edges) noexcept {
    const auto left0 = scale_vector(edges[0], 0.5);
    const auto left0_quarter = scale_vector(edges[0], 0.25);
    const auto middle1_quarter = scale_vector(edges[1], 0.25);
    const auto right2_quarter = scale_vector(edges[2], 0.25);
    const auto right2 = scale_vector(edges[2], 0.5);
    const auto outer0_eighth = scale_vector(edges[0], 0.125);
    const auto outer2_eighth = scale_vector(edges[2], 0.125);
    if (!left0 || !left0_quarter || !middle1_quarter ||
        !right2_quarter || !right2 || !outer0_eighth || !outer2_eighth) {
        return std::unexpected{IntervalError::non_finite_bound};
    }

    const auto left1 = add_vectors(*left0_quarter, *middle1_quarter);
    const auto right1 = add_vectors(*middle1_quarter, *right2_quarter);
    const auto middle_partial = add_vectors(*outer0_eighth, *middle1_quarter);
    if (!left1 || !right1 || !middle_partial) {
        return std::unexpected{IntervalError::non_finite_bound};
    }
    const auto middle = add_vectors(*middle_partial, *outer2_eighth);
    if (!middle) {
        return std::unexpected{IntervalError::non_finite_bound};
    }

    return std::pair{
        std::array<IntervalVector<Dimension>, 3>{*left0, *left1, *middle},
        std::array<IntervalVector<Dimension>, 3>{*middle, *right1, *right2},
    };
}

} // namespace apmesh::core::detail
