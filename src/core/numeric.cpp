#include "apmesh/core/numeric.hpp"

#include <cmath>
#include <expected>

namespace apmesh::core {

FloatingCategory classify_floating(const double value) noexcept {
    switch (std::fpclassify(value)) {
    case FP_ZERO:
        return FloatingCategory::zero;
    case FP_SUBNORMAL:
        return FloatingCategory::subnormal;
    case FP_NORMAL:
        return FloatingCategory::normal;
    case FP_INFINITE:
        return FloatingCategory::infinite;
    case FP_NAN:
        return FloatingCategory::not_a_number;
    default:
        return FloatingCategory::not_a_number;
    }
}

bool is_finite(const double value) noexcept {
    return std::isfinite(value);
}

std::expected<void, NumericError> validate_proximity_policy(
    const ProximityPolicy& policy) noexcept {
    if (!std::isfinite(policy.absolute_tolerance) ||
        !std::isfinite(policy.relative_tolerance) ||
        !std::isfinite(policy.reference_scale)) {
        return std::unexpected{NumericError::non_finite_policy};
    }
    if (policy.absolute_tolerance < 0.0) {
        return std::unexpected{NumericError::negative_absolute_tolerance};
    }
    if (policy.relative_tolerance < 0.0) {
        return std::unexpected{NumericError::negative_relative_tolerance};
    }
    if (policy.reference_scale <= 0.0) {
        return std::unexpected{NumericError::non_positive_reference_scale};
    }

    return {};
}

std::expected<ProximityEvidence, NumericError> compare_proximity(
    const double lhs,
    const double rhs,
    const ProximityPolicy& policy) noexcept {
    if (!is_finite(lhs) || !is_finite(rhs)) {
        return std::unexpected{NumericError::non_finite_input};
    }
    if (const auto validated_policy = validate_proximity_policy(policy);
        !validated_policy.has_value()) {
        return std::unexpected{validated_policy.error()};
    }

    const double residual = std::abs(lhs - rhs);
    const double limit = policy.absolute_tolerance +
                         policy.relative_tolerance * policy.reference_scale;
    if (!std::isfinite(residual) || !std::isfinite(limit)) {
        return std::unexpected{NumericError::non_finite_intermediate};
    }

    return ProximityEvidence{
        .result = residual <= limit ? ProximityResult::within : ProximityResult::outside,
        .residual = residual,
        .limit = limit,
    };
}

} // namespace apmesh::core
