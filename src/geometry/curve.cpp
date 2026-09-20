#include "apmesh/geometry/curve.hpp"

#include "apmesh/core/numeric.hpp"

#include <array>
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

[[nodiscard]] std::expected<Vector2, CurveError> interpolate(
    const Vector2& lhs,
    const Vector2& rhs,
    const double parameter) noexcept {
    const auto vector = Vector2::make(
        std::lerp(lhs.x(), rhs.x(), parameter),
        std::lerp(lhs.y(), rhs.y(), parameter));
    if (!vector.has_value()) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return *vector;
}

[[nodiscard]] std::expected<Vector3, CurveError> interpolate(
    const Vector3& lhs,
    const Vector3& rhs,
    const double parameter) noexcept {
    const auto vector = Vector3::make(
        std::lerp(lhs.x(), rhs.x(), parameter),
        std::lerp(lhs.y(), rhs.y(), parameter),
        std::lerp(lhs.z(), rhs.z(), parameter));
    if (!vector.has_value()) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return *vector;
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

template <typename Vector>
[[nodiscard]] std::expected<Vector, CurveError> scale_vector(
    const Vector& vector,
    const double scalar) noexcept {
    const auto scaled = vector * scalar;
    if (!scaled.has_value()) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return *scaled;
}

template <typename Point, typename Vector>
[[nodiscard]] std::expected<std::array<Vector, 3>, CurveError>
first_derivative_controls(const std::array<Point, 4>& points) noexcept {
    std::array<Vector, 3> controls{};
    for (std::size_t index = 0; index < controls.size(); ++index) {
        const auto delta = points[index + 1] - points[index];
        if (!delta.has_value()) {
            return std::unexpected{CurveError::non_finite_result};
        }
        const auto scaled = scale_vector(*delta, 3.0);
        if (!scaled.has_value()) {
            return std::unexpected{scaled.error()};
        }
        controls[index] = *scaled;
    }
    return controls;
}

template <typename Vector>
[[nodiscard]] std::expected<std::array<Vector, 2>, CurveError>
second_derivative_controls(const std::array<Vector, 3>& first_controls) noexcept {
    std::array<Vector, 2> controls{};
    for (std::size_t index = 0; index < controls.size(); ++index) {
        const auto delta = first_controls[index + 1] - first_controls[index];
        if (!delta.has_value()) {
            return std::unexpected{CurveError::non_finite_result};
        }
        const auto scaled = scale_vector(*delta, 2.0);
        if (!scaled.has_value()) {
            return std::unexpected{scaled.error()};
        }
        controls[index] = *scaled;
    }
    return controls;
}

template <typename Vector>
[[nodiscard]] std::expected<Vector, CurveError> evaluate_quadratic(
    const std::array<Vector, 3>& controls,
    const double parameter) noexcept {
    const auto q0 = interpolate(controls[0], controls[1], parameter);
    const auto q1 = interpolate(controls[1], controls[2], parameter);
    if (!q0 || !q1) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return interpolate(*q0, *q1, parameter);
}

template <typename Vector>
[[nodiscard]] std::expected<Vector, CurveError> evaluate_linear(
    const std::array<Vector, 2>& controls,
    const double parameter) noexcept {
    return interpolate(controls[0], controls[1], parameter);
}

template <typename Point, typename Vector>
[[nodiscard]] std::expected<Vector, CurveError> evaluate_first_derivative(
    const std::array<Point, 4>& points,
    const double parameter) noexcept {
    const auto valid = validate_parameter(parameter);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }

    const auto controls = first_derivative_controls<Point, Vector>(points);
    if (!controls.has_value()) {
        return std::unexpected{controls.error()};
    }
    return evaluate_quadratic(*controls, parameter);
}

template <typename Point, typename Vector>
[[nodiscard]] std::expected<Vector, CurveError> evaluate_second_derivative(
    const std::array<Point, 4>& points,
    const double parameter) noexcept {
    const auto valid = validate_parameter(parameter);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }

    const auto first_controls = first_derivative_controls<Point, Vector>(points);
    if (!first_controls.has_value()) {
        return std::unexpected{first_controls.error()};
    }
    const auto second_controls = second_derivative_controls(*first_controls);
    if (!second_controls.has_value()) {
        return std::unexpected{second_controls.error()};
    }
    return evaluate_linear(*second_controls, parameter);
}

template <typename Vector>
[[nodiscard]] std::expected<double, CurveError> vector_speed(
    const Vector& vector) noexcept {
    const auto value = norm(vector);
    if (!value.has_value()) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return *value;
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

std::expected<Vector2, CurveError> CubicBezier2::first_derivative(
    const double parameter) const noexcept {
    return evaluate_first_derivative<Point2, Vector2>(control_points_, parameter);
}

std::expected<Vector2, CurveError> CubicBezier2::second_derivative(
    const double parameter) const noexcept {
    return evaluate_second_derivative<Point2, Vector2>(control_points_, parameter);
}

std::expected<double, CurveError> CubicBezier2::speed(
    const double parameter) const noexcept {
    const auto derivative = first_derivative(parameter);
    if (!derivative.has_value()) {
        return std::unexpected{derivative.error()};
    }
    return vector_speed(*derivative);
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

std::expected<Vector3, CurveError> CubicBezier3::first_derivative(
    const double parameter) const noexcept {
    return evaluate_first_derivative<Point3, Vector3>(control_points_, parameter);
}

std::expected<Vector3, CurveError> CubicBezier3::second_derivative(
    const double parameter) const noexcept {
    return evaluate_second_derivative<Point3, Vector3>(control_points_, parameter);
}

std::expected<double, CurveError> CubicBezier3::speed(
    const double parameter) const noexcept {
    const auto derivative = first_derivative(parameter);
    if (!derivative.has_value()) {
        return std::unexpected{derivative.error()};
    }
    return vector_speed(*derivative);
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
