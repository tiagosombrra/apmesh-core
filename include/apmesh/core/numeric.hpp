#pragma once

#include <expected>

namespace apmesh::core {

enum class FloatingCategory {
    zero,
    subnormal,
    normal,
    infinite,
    not_a_number,
};

enum class NumericError {
    non_finite_input,
    non_finite_policy,
    negative_absolute_tolerance,
    negative_relative_tolerance,
    non_positive_reference_scale,
    non_finite_intermediate,
};

enum class ProximityResult {
    within,
    outside,
};

struct ProximityPolicy {
    double absolute_tolerance{};
    double relative_tolerance{};
    double reference_scale{};
};

struct ProximityEvidence {
    ProximityResult result{};
    double residual{};
    double limit{};
};

[[nodiscard]] FloatingCategory classify_floating(double value) noexcept;
[[nodiscard]] bool is_finite(double value) noexcept;

[[nodiscard]] std::expected<void, NumericError> validate_proximity_policy(
    const ProximityPolicy& policy) noexcept;

[[nodiscard]] std::expected<ProximityEvidence, NumericError> compare_proximity(
    double lhs,
    double rhs,
    const ProximityPolicy& policy) noexcept;

} // namespace apmesh::core
