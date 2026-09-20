#include "apmesh/geometry/curve.hpp"

#include "apmesh/core/numeric.hpp"

#include <cmath>
#include <expected>

namespace apmesh::core {
namespace {

[[nodiscard]] std::expected<void, CurveError> validate_parameter(
    const double parameter) noexcept {
    if (!is_finite(parameter)) {
        return std::unexpected{CurveError::non_finite_parameter};
    }
    if (parameter < 0.0 || parameter > 1.0) {
        return std::unexpected{CurveError::parameter_out_of_domain};
    }
    return {};
}

[[nodiscard]] std::expected<Point2, CurveError> interpolate(
    const Point2& lhs,
    const Point2& rhs,
    const double parameter) noexcept {
    const auto point = Point2::make(
        std::lerp(lhs.x(), rhs.x(), parameter),
        std::lerp(lhs.y(), rhs.y(), parameter));
    if (!point.has_value()) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return *point;
}

[[nodiscard]] std::expected<Point3, CurveError> interpolate(
    const Point3& lhs,
    const Point3& rhs,
    const double parameter) noexcept {
    const auto point = Point3::make(
        std::lerp(lhs.x(), rhs.x(), parameter),
        std::lerp(lhs.y(), rhs.y(), parameter),
        std::lerp(lhs.z(), rhs.z(), parameter));
    if (!point.has_value()) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return *point;
}

template <typename Point>
[[nodiscard]] std::expected<Point, CurveError> evaluate_de_casteljau(
    const std::array<Point, 4>& points,
    const double parameter) noexcept {
    const auto q0 = interpolate(points[0], points[1], parameter);
    const auto q1 = interpolate(points[1], points[2], parameter);
    const auto q2 = interpolate(points[2], points[3], parameter);
    if (!q0 || !q1 || !q2) {
        return std::unexpected{CurveError::non_finite_result};
    }

    const auto r0 = interpolate(*q0, *q1, parameter);
    const auto r1 = interpolate(*q1, *q2, parameter);
    if (!r0 || !r1) {
        return std::unexpected{CurveError::non_finite_result};
    }

    return interpolate(*r0, *r1, parameter);
}

} // namespace

const std::array<Point2, 4>& CubicBezier2::control_points() const noexcept {
    return control_points_;
}

std::expected<Point2, CurveError> CubicBezier2::evaluate(
    const double parameter) const noexcept {
    const auto valid = validate_parameter(parameter);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }
    return evaluate_de_casteljau(control_points_, parameter);
}

CubicBezier2 CubicBezier2::reversed() const noexcept {
    return CubicBezier2{
        control_points_[3],
        control_points_[2],
        control_points_[1],
        control_points_[0],
    };
}

const std::array<Point3, 4>& CubicBezier3::control_points() const noexcept {
    return control_points_;
}

std::expected<Point3, CurveError> CubicBezier3::evaluate(
    const double parameter) const noexcept {
    const auto valid = validate_parameter(parameter);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }
    return evaluate_de_casteljau(control_points_, parameter);
}

CubicBezier3 CubicBezier3::reversed() const noexcept {
    return CubicBezier3{
        control_points_[3],
        control_points_[2],
        control_points_[1],
        control_points_[0],
    };
}

} // namespace apmesh::core
