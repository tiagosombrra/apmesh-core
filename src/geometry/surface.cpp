#include "apmesh/geometry/surface.hpp"

#include <array>
#include <cmath>
#include <expected>
#include <utility>

namespace apmesh::core {
namespace {

[[nodiscard]] SurfaceParameterDomain unit_square_domain() noexcept {
    return SurfaceParameterDomain{
        .u = *CurveParameterDomain::make(0.0, 1.0),
        .v = *CurveParameterDomain::make(0.0, 1.0),
    };
}

[[nodiscard]] std::expected<void, SurfaceError> validate_parameters(
    const double u,
    const double v) noexcept {
    if (!std::isfinite(u)) {
        return std::unexpected{SurfaceError::non_finite_u_parameter};
    }
    if (!std::isfinite(v)) {
        return std::unexpected{SurfaceError::non_finite_v_parameter};
    }
    if (u < 0.0 || u > 1.0) {
        return std::unexpected{SurfaceError::u_parameter_out_of_domain};
    }
    if (v < 0.0 || v > 1.0) {
        return std::unexpected{SurfaceError::v_parameter_out_of_domain};
    }
    return {};
}

[[nodiscard]] std::expected<Point3, SurfaceError> interpolate(
    const Point3& lhs,
    const Point3& rhs,
    const double parameter) noexcept {
    const auto point = Point3::make(
        std::lerp(lhs.x(), rhs.x(), parameter),
        std::lerp(lhs.y(), rhs.y(), parameter),
        std::lerp(lhs.z(), rhs.z(), parameter));
    if (!point.has_value()) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    return *point;
}

[[nodiscard]] std::expected<Vector3, SurfaceError> interpolate(
    const Vector3& lhs,
    const Vector3& rhs,
    const double parameter) noexcept {
    const auto vector = Vector3::make(
        std::lerp(lhs.x(), rhs.x(), parameter),
        std::lerp(lhs.y(), rhs.y(), parameter),
        std::lerp(lhs.z(), rhs.z(), parameter));
    if (!vector.has_value()) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    return *vector;
}

[[nodiscard]] std::expected<Vector3, SurfaceError> difference(
    const Point3& lhs,
    const Point3& rhs) noexcept {
    const auto result = lhs - rhs;
    if (!result.has_value()) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    return *result;
}

[[nodiscard]] std::expected<Vector3, SurfaceError> difference(
    const Vector3& lhs,
    const Vector3& rhs) noexcept {
    const auto result = lhs - rhs;
    if (!result.has_value()) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    return *result;
}

[[nodiscard]] std::expected<Vector3, SurfaceError> scale_vector(
    const Vector3& vector,
    const double scalar) noexcept {
    const auto result = vector * scalar;
    if (!result.has_value()) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    return *result;
}

template <typename Value>
[[nodiscard]] std::expected<Value, SurfaceError> evaluate_cubic(
    const std::array<Value, 4>& controls,
    const double parameter) noexcept {
    const auto q0 = interpolate(controls[0], controls[1], parameter);
    const auto q1 = interpolate(controls[1], controls[2], parameter);
    const auto q2 = interpolate(controls[2], controls[3], parameter);
    if (!q0 || !q1 || !q2) {
        return std::unexpected{SurfaceError::non_finite_result};
    }

    const auto r0 = interpolate(*q0, *q1, parameter);
    const auto r1 = interpolate(*q1, *q2, parameter);
    if (!r0 || !r1) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    return interpolate(*r0, *r1, parameter);
}

[[nodiscard]] std::expected<Vector3, SurfaceError> evaluate_quadratic(
    const std::array<Vector3, 3>& controls,
    const double parameter) noexcept {
    const auto q0 = interpolate(controls[0], controls[1], parameter);
    const auto q1 = interpolate(controls[1], controls[2], parameter);
    if (!q0 || !q1) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    return interpolate(*q0, *q1, parameter);
}

[[nodiscard]] std::expected<Vector3, SurfaceError> evaluate_linear(
    const std::array<Vector3, 2>& controls,
    const double parameter) noexcept {
    return interpolate(controls[0], controls[1], parameter);
}

template <typename Value>
[[nodiscard]] std::expected<std::array<Vector3, 3>, SurfaceError>
first_derivative_controls(
    const std::array<Value, 4>& controls) noexcept {
    std::array<Vector3, 3> result{
        *Vector3::make(0.0, 0.0, 0.0),
        *Vector3::make(0.0, 0.0, 0.0),
        *Vector3::make(0.0, 0.0, 0.0),
    };
    for (std::size_t index = 0; index < result.size(); ++index) {
        const auto delta =
            difference(controls[index + 1U], controls[index]);
        if (!delta.has_value()) {
            return std::unexpected{delta.error()};
        }
        const auto scaled = scale_vector(*delta, 3.0);
        if (!scaled.has_value()) {
            return std::unexpected{scaled.error()};
        }
        result[index] = *scaled;
    }
    return result;
}

[[nodiscard]] std::expected<std::array<Vector3, 2>, SurfaceError>
second_derivative_controls(
    const std::array<Vector3, 3>& first_controls) noexcept {
    std::array<Vector3, 2> result{
        *Vector3::make(0.0, 0.0, 0.0),
        *Vector3::make(0.0, 0.0, 0.0),
    };
    for (std::size_t index = 0; index < result.size(); ++index) {
        const auto delta =
            difference(first_controls[index + 1U], first_controls[index]);
        if (!delta.has_value()) {
            return std::unexpected{delta.error()};
        }
        const auto scaled = scale_vector(*delta, 2.0);
        if (!scaled.has_value()) {
            return std::unexpected{scaled.error()};
        }
        result[index] = *scaled;
    }
    return result;
}

template <typename Value>
[[nodiscard]] std::expected<Vector3, SurfaceError> evaluate_first_derivative(
    const std::array<Value, 4>& controls,
    const double parameter) noexcept {
    const auto derivative_controls =
        first_derivative_controls(controls);
    if (!derivative_controls.has_value()) {
        return std::unexpected{derivative_controls.error()};
    }
    return evaluate_quadratic(*derivative_controls, parameter);
}

[[nodiscard]] std::expected<Vector3, SurfaceError>
evaluate_second_derivative(
    const std::array<Point3, 4>& controls,
    const double parameter) noexcept {
    const auto first = first_derivative_controls(controls);
    if (!first.has_value()) {
        return std::unexpected{first.error()};
    }
    const auto second = second_derivative_controls(*first);
    if (!second.has_value()) {
        return std::unexpected{second.error()};
    }
    return evaluate_linear(*second, parameter);
}

[[nodiscard]] std::expected<std::array<Point3, 4>, SurfaceError>
evaluate_v_rows(
    const BicubicBezierPatch3::ControlNet& controls,
    const double v) noexcept {
    std::array<Point3, 4> rows{
        controls[0][0],
        controls[1][0],
        controls[2][0],
        controls[3][0],
    };
    for (std::size_t u_index = 0; u_index < rows.size(); ++u_index) {
        const auto value = evaluate_cubic(controls[u_index], v);
        if (!value.has_value()) {
            return std::unexpected{value.error()};
        }
        rows[u_index] = *value;
    }
    return rows;
}

[[nodiscard]] std::expected<std::array<Vector3, 4>, SurfaceError>
evaluate_v_first_rows(
    const BicubicBezierPatch3::ControlNet& controls,
    const double v) noexcept {
    const auto zero = *Vector3::make(0.0, 0.0, 0.0);
    std::array<Vector3, 4> rows{zero, zero, zero, zero};
    for (std::size_t u_index = 0; u_index < rows.size(); ++u_index) {
        const auto value =
            evaluate_first_derivative(controls[u_index], v);
        if (!value.has_value()) {
            return std::unexpected{value.error()};
        }
        rows[u_index] = *value;
    }
    return rows;
}

[[nodiscard]] std::expected<std::array<Vector3, 4>, SurfaceError>
evaluate_v_second_rows(
    const BicubicBezierPatch3::ControlNet& controls,
    const double v) noexcept {
    const auto zero = *Vector3::make(0.0, 0.0, 0.0);
    std::array<Vector3, 4> rows{zero, zero, zero, zero};
    for (std::size_t u_index = 0; u_index < rows.size(); ++u_index) {
        const auto value =
            evaluate_second_derivative(controls[u_index], v);
        if (!value.has_value()) {
            return std::unexpected{value.error()};
        }
        rows[u_index] = *value;
    }
    return rows;
}

} // namespace

const BicubicBezierPatch3::ControlNet&
BicubicBezierPatch3::control_points() const noexcept {
    return control_points_;
}

SurfaceParameterDomain
BicubicBezierPatch3::parameter_domain() const noexcept {
    return unit_square_domain();
}

std::expected<Point3, SurfaceError>
BicubicBezierPatch3::evaluate(
    const double u,
    const double v) const noexcept {
    const auto valid = validate_parameters(u, v);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }

    if (u == 0.0 && v == 0.0) {
        return control_points_[0][0];
    }
    if (u == 0.0 && v == 1.0) {
        return control_points_[0][3];
    }
    if (u == 1.0 && v == 0.0) {
        return control_points_[3][0];
    }
    if (u == 1.0 && v == 1.0) {
        return control_points_[3][3];
    }

    const auto rows = evaluate_v_rows(control_points_, v);
    if (!rows.has_value()) {
        return std::unexpected{rows.error()};
    }
    return evaluate_cubic(*rows, u);
}

std::expected<SurfaceFirstDerivatives3, SurfaceError>
BicubicBezierPatch3::first_derivatives(
    const double u,
    const double v) const noexcept {
    const auto valid = validate_parameters(u, v);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }

    const auto rows = evaluate_v_rows(control_points_, v);
    const auto v_rows = evaluate_v_first_rows(control_points_, v);
    if (!rows || !v_rows) {
        return std::unexpected{SurfaceError::non_finite_result};
    }

    const auto du = evaluate_first_derivative(*rows, u);
    const auto dv = evaluate_cubic(*v_rows, u);
    if (!du || !dv) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    return SurfaceFirstDerivatives3{.u = *du, .v = *dv};
}

std::expected<SurfaceSecondDerivatives3, SurfaceError>
BicubicBezierPatch3::second_derivatives(
    const double u,
    const double v) const noexcept {
    const auto valid = validate_parameters(u, v);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }

    const auto rows = evaluate_v_rows(control_points_, v);
    const auto v_first_rows = evaluate_v_first_rows(control_points_, v);
    const auto v_second_rows = evaluate_v_second_rows(control_points_, v);
    if (!rows || !v_first_rows || !v_second_rows) {
        return std::unexpected{SurfaceError::non_finite_result};
    }

    const auto duu = evaluate_second_derivative(*rows, u);
    const auto duv = evaluate_first_derivative(*v_first_rows, u);
    const auto dvv = evaluate_cubic(*v_second_rows, u);
    if (!duu || !duv || !dvv) {
        return std::unexpected{SurfaceError::non_finite_result};
    }

    return SurfaceSecondDerivatives3{
        .uu = *duu,
        .uv = *duv,
        .vv = *dvv,
    };
}

BicubicBezierPatch3
BicubicBezierPatch3::u_reversed() const noexcept {
    ControlNet reversed = control_points_;
    for (std::size_t u_index = 0; u_index < 4U; ++u_index) {
        reversed[u_index] = control_points_[3U - u_index];
    }
    return BicubicBezierPatch3{reversed};
}

BicubicBezierPatch3
BicubicBezierPatch3::v_reversed() const noexcept {
    ControlNet reversed = control_points_;
    for (std::size_t u_index = 0; u_index < 4U; ++u_index) {
        for (std::size_t v_index = 0; v_index < 4U; ++v_index) {
            reversed[u_index][v_index] =
                control_points_[u_index][3U - v_index];
        }
    }
    return BicubicBezierPatch3{reversed};
}

} // namespace apmesh::core
