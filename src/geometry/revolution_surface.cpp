#include "apmesh/geometry/revolution_surface.hpp"

#include <cmath>
#include <expected>
#include <numbers>

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

[[nodiscard]] SurfaceError map_curve_error(const CurveError error) noexcept {
    switch (error) {
    case CurveError::non_finite_parameter:
        return SurfaceError::non_finite_v_parameter;
    case CurveError::parameter_out_of_domain:
        return SurfaceError::v_parameter_out_of_domain;
    case CurveError::insufficient_continuity:
        return SurfaceError::insufficient_continuity;
    case CurveError::non_finite_result:
    case CurveError::singular_parameter:
        return SurfaceError::non_finite_result;
    }
    return SurfaceError::non_finite_result;
}

[[nodiscard]] std::expected<Point3, SurfaceError> rotate_point(
    const Point3& point,
    const AxisPlacement3& axis,
    const double angle) noexcept {
    if (angle == 0.0) {
        return point;
    }

    const auto local = axis.point_to_local(point);
    if (!local) {
        return std::unexpected{SurfaceError::non_finite_result};
    }

    const double sine = std::sin(angle);
    const double cosine = std::cos(angle);
    if (!std::isfinite(sine) || !std::isfinite(cosine)) {
        return std::unexpected{SurfaceError::non_finite_result};
    }

    const long double x =
        static_cast<long double>(cosine) *
            static_cast<long double>(local->x()) -
        static_cast<long double>(sine) *
            static_cast<long double>(local->y());
    const long double y =
        static_cast<long double>(sine) *
            static_cast<long double>(local->x()) +
        static_cast<long double>(cosine) *
            static_cast<long double>(local->y());

    const auto rotated_local = Point3::make(
        static_cast<double>(x),
        static_cast<double>(y),
        local->z());
    if (!rotated_local) {
        return std::unexpected{SurfaceError::non_finite_result};
    }

    const auto world = axis.point_to_world(*rotated_local);
    if (!world) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    return *world;
}

[[nodiscard]] std::expected<Vector3, SurfaceError> rotate_vector(
    const Vector3& vector,
    const AxisPlacement3& axis,
    const double angle) noexcept {
    if (angle == 0.0) {
        return vector;
    }

    const auto local = axis.vector_to_local(vector);
    if (!local) {
        return std::unexpected{SurfaceError::non_finite_result};
    }

    const double sine = std::sin(angle);
    const double cosine = std::cos(angle);
    if (!std::isfinite(sine) || !std::isfinite(cosine)) {
        return std::unexpected{SurfaceError::non_finite_result};
    }

    const long double x =
        static_cast<long double>(cosine) *
            static_cast<long double>(local->x()) -
        static_cast<long double>(sine) *
            static_cast<long double>(local->y());
    const long double y =
        static_cast<long double>(sine) *
            static_cast<long double>(local->x()) +
        static_cast<long double>(cosine) *
            static_cast<long double>(local->y());

    const auto rotated_local = Vector3::make(
        static_cast<double>(x),
        static_cast<double>(y),
        local->z());
    if (!rotated_local) {
        return std::unexpected{SurfaceError::non_finite_result};
    }

    const auto world = axis.vector_to_world(*rotated_local);
    if (!world) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    return *world;
}

[[nodiscard]] std::expected<Vector3, SurfaceError> radial_vector(
    const Point3& point,
    const AxisPlacement3& axis) noexcept {
    const auto value = point - axis.origin();
    if (!value) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    return *value;
}

[[nodiscard]] std::expected<Vector3, SurfaceError> scaled_cross(
    const Vector3& axis_direction,
    const Vector3& vector,
    const double scalar) noexcept {
    const auto cross_value = cross(axis_direction, vector);
    if (!cross_value) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    const auto scaled = *cross_value * scalar;
    if (!scaled) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    return *scaled;
}

[[nodiscard]] std::expected<CubicBezier3, RevolutionSurfaceConstructionError>
rotated_curve(
    const CubicBezier3& curve,
    const AxisPlacement3& axis,
    const double angle) noexcept {
    const auto& controls = curve.control_points();

    const auto p0 = rotate_point(controls[0], axis, angle);
    const auto p1 = rotate_point(controls[1], axis, angle);
    const auto p2 = rotate_point(controls[2], axis, angle);
    const auto p3 = rotate_point(controls[3], axis, angle);

    if (!p0 || !p1 || !p2 || !p3) {
        return std::unexpected{
            RevolutionSurfaceConstructionError::non_finite_rotated_control};
    }

    return CubicBezier3{*p0, *p1, *p2, *p3};
}

} // namespace

std::expected<CubicBezierRevolutionSurface3, RevolutionSurfaceConstructionError>
CubicBezierRevolutionSurface3::make(
    const CubicBezier3& generatrix,
    const AxisPlacement3& axis,
    const double sweep_angle) noexcept {
    if (!std::isfinite(sweep_angle)) {
        return std::unexpected{
            RevolutionSurfaceConstructionError::non_finite_sweep_angle};
    }
    if (sweep_angle == 0.0) {
        return std::unexpected{
            RevolutionSurfaceConstructionError::zero_sweep_angle};
    }

    constexpr double full_revolution =
        2.0 * std::numbers::pi_v<double>;
    if (std::abs(sweep_angle) >= full_revolution) {
        return std::unexpected{
            RevolutionSurfaceConstructionError::
                full_or_multiple_revolution_not_admitted};
    }

    const auto end_curve = rotated_curve(generatrix, axis, sweep_angle);
    if (!end_curve) {
        return std::unexpected{end_curve.error()};
    }

    return CubicBezierRevolutionSurface3{
        generatrix,
        *end_curve,
        axis,
        sweep_angle};
}

const CubicBezier3&
CubicBezierRevolutionSurface3::start_curve() const noexcept {
    return start_curve_;
}

const CubicBezier3&
CubicBezierRevolutionSurface3::end_curve() const noexcept {
    return end_curve_;
}

const AxisPlacement3&
CubicBezierRevolutionSurface3::axis_placement() const noexcept {
    return axis_;
}

double CubicBezierRevolutionSurface3::sweep_angle() const noexcept {
    return sweep_angle_;
}

SurfaceParameterDomain
CubicBezierRevolutionSurface3::parameter_domain() const noexcept {
    return unit_square_domain();
}

std::expected<Point3, SurfaceError>
CubicBezierRevolutionSurface3::evaluate(
    const double u,
    const double v) const noexcept {
    const auto valid = validate_parameters(u, v);
    if (!valid) {
        return std::unexpected{valid.error()};
    }

    if (u == 0.0) {
        const auto value = start_curve_.evaluate(v);
        if (!value) {
            return std::unexpected{map_curve_error(value.error())};
        }
        return *value;
    }
    if (u == 1.0) {
        const auto value = end_curve_.evaluate(v);
        if (!value) {
            return std::unexpected{map_curve_error(value.error())};
        }
        return *value;
    }

    const auto value = start_curve_.evaluate(v);
    if (!value) {
        return std::unexpected{map_curve_error(value.error())};
    }
    return rotate_point(*value, axis_, u * sweep_angle_);
}

std::expected<SurfaceFirstDerivatives3, SurfaceError>
CubicBezierRevolutionSurface3::first_derivatives(
    const double u,
    const double v) const noexcept {
    const auto valid = validate_parameters(u, v);
    if (!valid) {
        return std::unexpected{valid.error()};
    }

    std::expected<Point3, CurveError> curve_value =
        u == 1.0 ? end_curve_.evaluate(v) : start_curve_.evaluate(v);
    std::expected<Vector3, CurveError> curve_d1 =
        u == 1.0 ? end_curve_.first_derivative(v)
                 : start_curve_.first_derivative(v);

    if (!curve_value) {
        return std::unexpected{map_curve_error(curve_value.error())};
    }
    if (!curve_d1) {
        return std::unexpected{map_curve_error(curve_d1.error())};
    }

    Point3 surface_point = *curve_value;
    Vector3 dv = *curve_d1;

    if (u != 0.0 && u != 1.0) {
        const double angle = u * sweep_angle_;
        const auto rotated_point = rotate_point(*curve_value, axis_, angle);
        const auto rotated_d1 = rotate_vector(*curve_d1, axis_, angle);
        if (!rotated_point || !rotated_d1) {
            return std::unexpected{SurfaceError::non_finite_result};
        }
        surface_point = *rotated_point;
        dv = *rotated_d1;
    }

    const auto radius = radial_vector(surface_point, axis_);
    if (!radius) {
        return std::unexpected{radius.error()};
    }
    const auto du = scaled_cross(
        axis_.z_direction(), *radius, sweep_angle_);
    if (!du) {
        return std::unexpected{du.error()};
    }

    return SurfaceFirstDerivatives3{
        .u = *du,
        .v = dv,
    };
}

std::expected<SurfaceSecondDerivatives3, SurfaceError>
CubicBezierRevolutionSurface3::second_derivatives(
    const double u,
    const double v) const noexcept {
    const auto valid = validate_parameters(u, v);
    if (!valid) {
        return std::unexpected{valid.error()};
    }

    std::expected<Point3, CurveError> curve_value =
        u == 1.0 ? end_curve_.evaluate(v) : start_curve_.evaluate(v);
    std::expected<Vector3, CurveError> curve_d1 =
        u == 1.0 ? end_curve_.first_derivative(v)
                 : start_curve_.first_derivative(v);
    std::expected<Vector3, CurveError> curve_d2 =
        u == 1.0 ? end_curve_.second_derivative(v)
                 : start_curve_.second_derivative(v);

    if (!curve_value) {
        return std::unexpected{map_curve_error(curve_value.error())};
    }
    if (!curve_d1) {
        return std::unexpected{map_curve_error(curve_d1.error())};
    }
    if (!curve_d2) {
        return std::unexpected{map_curve_error(curve_d2.error())};
    }

    Point3 surface_point = *curve_value;
    Vector3 dv = *curve_d1;
    Vector3 dvv = *curve_d2;

    if (u != 0.0 && u != 1.0) {
        const double angle = u * sweep_angle_;
        const auto rotated_point = rotate_point(*curve_value, axis_, angle);
        const auto rotated_d1 = rotate_vector(*curve_d1, axis_, angle);
        const auto rotated_d2 = rotate_vector(*curve_d2, axis_, angle);
        if (!rotated_point || !rotated_d1 || !rotated_d2) {
            return std::unexpected{SurfaceError::non_finite_result};
        }
        surface_point = *rotated_point;
        dv = *rotated_d1;
        dvv = *rotated_d2;
    }

    const auto radius = radial_vector(surface_point, axis_);
    if (!radius) {
        return std::unexpected{radius.error()};
    }

    const auto first_cross = cross(axis_.z_direction(), *radius);
    if (!first_cross) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    const auto second_cross = cross(axis_.z_direction(), *first_cross);
    if (!second_cross) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    const auto duu =
        *second_cross * (sweep_angle_ * sweep_angle_);
    if (!duu) {
        return std::unexpected{SurfaceError::non_finite_result};
    }

    const auto duv = scaled_cross(
        axis_.z_direction(), dv, sweep_angle_);
    if (!duv) {
        return std::unexpected{duv.error()};
    }

    return SurfaceSecondDerivatives3{
        .uu = *duu,
        .uv = *duv,
        .vv = dvv,
    };
}

CubicBezierRevolutionSurface3
CubicBezierRevolutionSurface3::u_reversed() const noexcept {
    return CubicBezierRevolutionSurface3{
        end_curve_,
        start_curve_,
        axis_,
        -sweep_angle_};
}

CubicBezierRevolutionSurface3
CubicBezierRevolutionSurface3::v_reversed() const noexcept {
    return CubicBezierRevolutionSurface3{
        start_curve_.reversed(),
        end_curve_.reversed(),
        axis_,
        sweep_angle_};
}

} // namespace apmesh::core
