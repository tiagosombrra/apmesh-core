#pragma once

#include "curve_regularity_interval.hpp"

#include <array>
#include <cstddef>
#include <expected>
#include <utility>

namespace apmesh::core::detail {

using QuadraticIntervalCoefficients = std::array<ClosedInterval, 3>;

enum class CertifiedCoefficientSign {
    negative,
    zero,
    positive,
    unknown,
};

struct QuadraticSignVariation {
    bool resolved{};
    bool all_zero{};
    std::size_t variations{};
};

struct QuadraticSubdivision {
    QuadraticIntervalCoefficients left{};
    QuadraticIntervalCoefficients right{};
    ClosedInterval shared_boundary_value{};
};

[[nodiscard]] inline bool interval_is_exact_zero(
    const ClosedInterval& value) noexcept {
    return value.lower == 0.0 && value.upper == 0.0;
}

[[nodiscard]] inline bool interval_contains_zero(
    const ClosedInterval& value) noexcept {
    return value.lower <= 0.0 && value.upper >= 0.0;
}

[[nodiscard]] inline CertifiedCoefficientSign certified_coefficient_sign(
    const ClosedInterval& value) noexcept {
    if (value.lower > 0.0) {
        return CertifiedCoefficientSign::positive;
    }
    if (value.upper < 0.0) {
        return CertifiedCoefficientSign::negative;
    }
    if (interval_is_exact_zero(value)) {
        return CertifiedCoefficientSign::zero;
    }
    return CertifiedCoefficientSign::unknown;
}

[[nodiscard]] inline std::expected<ClosedInterval, IntervalError>
inflection_multiply(
    const ClosedInterval& lhs,
    const ClosedInterval& rhs) noexcept {
    if (interval_is_exact_zero(lhs) || interval_is_exact_zero(rhs)) {
        return ClosedInterval{0.0, 0.0};
    }
    return multiply(lhs, rhs);
}

[[nodiscard]] inline std::expected<ClosedInterval, IntervalError>
inflection_multiply(
    const ClosedInterval& value,
    const double scalar) noexcept {
    if (interval_is_exact_zero(value) || scalar == 0.0) {
        return ClosedInterval{0.0, 0.0};
    }
    return multiply(value, scalar);
}

[[nodiscard]] inline std::expected<ClosedInterval, IntervalError>
inflection_subtract(
    const ClosedInterval& lhs,
    const ClosedInterval& rhs) noexcept {
    if (interval_is_exact_zero(lhs) && interval_is_exact_zero(rhs)) {
        return ClosedInterval{0.0, 0.0};
    }
    return subtract(lhs, rhs);
}

[[nodiscard]] inline std::expected<ClosedInterval, IntervalError>
inflection_difference_scaled(
    const double next,
    const double current,
    const double scalar) noexcept {
    if (next == current) {
        return ClosedInterval{0.0, 0.0};
    }

    const auto next_interval = point_interval(next);
    const auto current_interval = point_interval(current);
    if (!next_interval || !current_interval) {
        return std::unexpected{IntervalError::non_finite_bound};
    }

    const auto difference = subtract(*next_interval, *current_interval);
    if (!difference) {
        return std::unexpected{difference.error()};
    }
    return inflection_multiply(*difference, scalar);
}

[[nodiscard]] inline std::expected<ClosedInterval, IntervalError>
inflection_determinant(
    const IntervalVector<2>& lhs,
    const IntervalVector<2>& rhs) noexcept {
    const auto lhs_x_rhs_y = inflection_multiply(lhs[0], rhs[1]);
    const auto lhs_y_rhs_x = inflection_multiply(lhs[1], rhs[0]);
    if (!lhs_x_rhs_y || !lhs_y_rhs_x) {
        return std::unexpected{IntervalError::non_finite_bound};
    }
    return inflection_subtract(*lhs_x_rhs_y, *lhs_y_rhs_x);
}

[[nodiscard]] inline std::expected<ClosedInterval, IntervalError>
inflection_midpoint(
    const ClosedInterval& lhs,
    const ClosedInterval& rhs) noexcept {
    if (lhs.lower == lhs.upper &&
        rhs.lower == rhs.upper &&
        lhs.lower == rhs.lower) {
        return lhs;
    }
    return midpoint(lhs, rhs);
}

[[nodiscard]] inline std::expected<QuadraticSubdivision, IntervalError>
subdivide_quadratic_midpoint(
    const QuadraticIntervalCoefficients& coefficients) noexcept {
    const auto q01 = inflection_midpoint(coefficients[0], coefficients[1]);
    const auto q12 = inflection_midpoint(coefficients[1], coefficients[2]);
    if (!q01 || !q12) {
        return std::unexpected{IntervalError::non_finite_bound};
    }

    const auto middle = inflection_midpoint(*q01, *q12);
    if (!middle) {
        return std::unexpected{IntervalError::non_finite_bound};
    }

    return QuadraticSubdivision{
        .left = QuadraticIntervalCoefficients{
            coefficients[0],
            *q01,
            *middle,
        },
        .right = QuadraticIntervalCoefficients{
            *middle,
            *q12,
            coefficients[2],
        },
        .shared_boundary_value = *middle,
    };
}

[[nodiscard]] inline QuadraticSignVariation quadratic_sign_variation(
    const QuadraticIntervalCoefficients& coefficients) noexcept {
    CertifiedCoefficientSign previous = CertifiedCoefficientSign::zero;
    bool have_previous = false;
    std::size_t variations = 0;

    for (const ClosedInterval& coefficient : coefficients) {
        const CertifiedCoefficientSign sign =
            certified_coefficient_sign(coefficient);
        if (sign == CertifiedCoefficientSign::unknown) {
            return QuadraticSignVariation{
                .resolved = false,
                .all_zero = false,
                .variations = 0,
            };
        }
        if (sign == CertifiedCoefficientSign::zero) {
            continue;
        }

        if (have_previous && sign != previous) {
            ++variations;
        }
        previous = sign;
        have_previous = true;
    }

    return QuadraticSignVariation{
        .resolved = true,
        .all_zero = !have_previous,
        .variations = variations,
    };
}

} // namespace apmesh::core::detail
