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
    double scale = 0.0;

    for (std::size_t index = 0; index < Dimension; ++index) {
        const auto [lower, upper] = absolute_bounds(value[index]);
        if (!std::isfinite(lower) || !std::isfinite(upper)) {
            return std::unexpected{IntervalError::non_finite_bound};
        }
        lower_components[index] = lower;
        upper_components[index] = upper;
        scale = std::max(scale, upper);
    }

    if (scale == 0.0) {
        return ClosedInterval{0.0, 0.0};
    }

    double lower_sum = 0.0;
    double upper_sum = 0.0;

    for (std::size_t index = 0; index < Dimension; ++index) {
        double normalized_lower = 0.0;
        if (lower_components[index] > 0.0) {
            const double raw = lower_components[index] / scale;
            if (!std::isfinite(raw)) {
                return std::unexpected{IntervalError::non_finite_bound};
            }
            normalized_lower =
                raw == 0.0 ? 0.0 : std::max(0.0, std::nextafter(raw, 0.0));
        }

        double normalized_upper = 0.0;
        if (upper_components[index] > 0.0) {
            const double raw = upper_components[index] / scale;
            if (!std::isfinite(raw)) {
                return std::unexpected{IntervalError::non_finite_bound};
            }
            normalized_upper = std::nextafter(
                raw,
                std::numeric_limits<double>::infinity());
            if (!std::isfinite(normalized_upper)) {
                return std::unexpected{IntervalError::non_finite_bound};
            }
        }

        double square_lower = 0.0;
        if (normalized_lower > 0.0) {
            const double raw = normalized_lower * normalized_lower;
            if (!std::isfinite(raw)) {
                return std::unexpected{IntervalError::non_finite_bound};
            }
            square_lower =
                raw == 0.0 ? 0.0 : std::max(0.0, std::nextafter(raw, 0.0));
        }

        double square_upper = 0.0;
        if (normalized_upper > 0.0) {
            const double raw = normalized_upper * normalized_upper;
            if (!std::isfinite(raw)) {
                return std::unexpected{IntervalError::non_finite_bound};
            }
            square_upper = std::nextafter(
                raw,
                std::numeric_limits<double>::infinity());
            if (!std::isfinite(square_upper)) {
                return std::unexpected{IntervalError::non_finite_bound};
            }
        }

        if (square_lower > 0.0) {
            if (lower_sum == 0.0) {
                lower_sum = square_lower;
            } else {
                const double raw = lower_sum + square_lower;
                if (!std::isfinite(raw)) {
                    return std::unexpected{IntervalError::non_finite_bound};
                }
                lower_sum = std::max(0.0, std::nextafter(raw, 0.0));
            }
        }

        if (square_upper > 0.0) {
            if (upper_sum == 0.0) {
                upper_sum = square_upper;
            } else {
                const double raw = upper_sum + square_upper;
                if (!std::isfinite(raw)) {
                    return std::unexpected{IntervalError::non_finite_bound};
                }
                upper_sum = std::nextafter(
                    raw,
                    std::numeric_limits<double>::infinity());
                if (!std::isfinite(upper_sum)) {
                    return std::unexpected{IntervalError::non_finite_bound};
                }
            }
        }
    }

    double root_lower = 0.0;
    if (lower_sum > 0.0) {
        const double raw = std::sqrt(lower_sum);
        if (!std::isfinite(raw)) {
            return std::unexpected{IntervalError::non_finite_bound};
        }
        root_lower = std::max(0.0, std::nextafter(raw, 0.0));
    }

    double root_upper = 0.0;
    if (upper_sum > 0.0) {
        const double raw = std::sqrt(upper_sum);
        if (!std::isfinite(raw)) {
            return std::unexpected{IntervalError::non_finite_bound};
        }
        root_upper = std::nextafter(
            raw,
            std::numeric_limits<double>::infinity());
        if (!std::isfinite(root_upper)) {
            return std::unexpected{IntervalError::non_finite_bound};
        }
    }

    double lower = 0.0;
    if (root_lower > 0.0) {
        const double raw = root_lower * scale;
        if (!std::isfinite(raw)) {
            return std::unexpected{IntervalError::non_finite_bound};
        }
        lower = raw == 0.0 ? 0.0 : std::max(0.0, std::nextafter(raw, 0.0));
    }

    double upper = 0.0;
    if (root_upper > 0.0) {
        const double raw = root_upper * scale;
        if (!std::isfinite(raw)) {
            return std::unexpected{IntervalError::non_finite_bound};
        }
        upper = std::nextafter(
            raw,
            std::numeric_limits<double>::infinity());
        if (!std::isfinite(upper)) {
            return std::unexpected{IntervalError::non_finite_bound};
        }
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
