#pragma once

#include "apmesh/core/geometry.hpp"

#include <cmath>
#include <concepts>
#include <expected>
#include <numeric>

namespace apmesh::core {

enum class CurveError {
    non_finite_parameter,
    parameter_out_of_domain,
    non_finite_result,
    singular_parameter,
    insufficient_continuity,
};

enum class CurveParameterDomainError {
    non_finite_lower_bound,
    non_finite_upper_bound,
    reversed_interval,
    zero_width_interval,
};

class CurveParameterDomain {
public:
    [[nodiscard]] static std::expected<CurveParameterDomain, CurveParameterDomainError>
    make(double lower, double upper) noexcept {
        if (!std::isfinite(lower)) {
            return std::unexpected{
                CurveParameterDomainError::non_finite_lower_bound};
        }
        if (!std::isfinite(upper)) {
            return std::unexpected{
                CurveParameterDomainError::non_finite_upper_bound};
        }
        if (lower > upper) {
            return std::unexpected{CurveParameterDomainError::reversed_interval};
        }
        if (lower == upper) {
            return std::unexpected{CurveParameterDomainError::zero_width_interval};
        }
        return CurveParameterDomain{lower, upper};
    }

    [[nodiscard]] double lower() const noexcept {
        return lower_;
    }

    [[nodiscard]] double upper() const noexcept {
        return upper_;
    }

    [[nodiscard]] std::expected<bool, CurveError> contains(
        const double parameter) const noexcept {
        if (!std::isfinite(parameter)) {
            return std::unexpected{CurveError::non_finite_parameter};
        }
        return parameter >= lower_ && parameter <= upper_;
    }

    [[nodiscard]] bool operator==(const CurveParameterDomain&) const noexcept = default;

private:
    constexpr CurveParameterDomain(
        const double lower,
        const double upper) noexcept
        : lower_(lower), upper_(upper) {}

    double lower_{};
    double upper_{};
};

[[nodiscard]] inline std::expected<double, CurveError> reversed_parameter(
    const CurveParameterDomain& domain,
    const double parameter) noexcept {
    const auto contained = domain.contains(parameter);
    if (!contained.has_value()) {
        return std::unexpected{contained.error()};
    }
    if (!*contained) {
        return std::unexpected{CurveError::parameter_out_of_domain};
    }

    const double lower = domain.lower();
    const double upper = domain.upper();

    if (parameter == lower) {
        return upper;
    }
    if (parameter == upper) {
        return lower;
    }

    const double midpoint = std::midpoint(lower, upper);
    const double reversed =
        parameter <= midpoint
            ? upper - (parameter - lower)
            : lower + (upper - parameter);

    if (!std::isfinite(reversed)) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return reversed;
}

template <typename Curve>
concept BoundedParametricCurve2 =
    requires(const Curve& curve, const double parameter) {
        {
            curve.parameter_domain()
        } -> std::same_as<CurveParameterDomain>;
        {
            curve.evaluate(parameter)
        } -> std::same_as<std::expected<Point2, CurveError>>;
        {
            curve.first_derivative(parameter)
        } -> std::same_as<std::expected<Vector2, CurveError>>;
        {
            curve.second_derivative(parameter)
        } -> std::same_as<std::expected<Vector2, CurveError>>;
        {
            curve.reversed()
        } -> std::same_as<Curve>;
    };

template <typename Curve>
concept BoundedParametricCurve3 =
    requires(const Curve& curve, const double parameter) {
        {
            curve.parameter_domain()
        } -> std::same_as<CurveParameterDomain>;
        {
            curve.evaluate(parameter)
        } -> std::same_as<std::expected<Point3, CurveError>>;
        {
            curve.first_derivative(parameter)
        } -> std::same_as<std::expected<Vector3, CurveError>>;
        {
            curve.second_derivative(parameter)
        } -> std::same_as<std::expected<Vector3, CurveError>>;
        {
            curve.reversed()
        } -> std::same_as<Curve>;
    };

} // namespace apmesh::core
