#include "apmesh/geometry/extrusion_surface.hpp"

#include <cmath>
#include <expected>

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

[[nodiscard]] SurfaceError map_curve_error(
    const CurveError error) noexcept {
    switch (error) {
    case CurveError::non_finite_parameter:
        return SurfaceError::non_finite_u_parameter;
    case CurveError::parameter_out_of_domain:
        return SurfaceError::u_parameter_out_of_domain;
    case CurveError::insufficient_continuity:
        return SurfaceError::insufficient_continuity;
    case CurveError::non_finite_result:
    case CurveError::singular_parameter:
        return SurfaceError::non_finite_result;
    }
    return SurfaceError::non_finite_result;
}

[[nodiscard]] Vector3 zero_vector3() noexcept {
    return *Vector3::make(0.0, 0.0, 0.0);
}

} // namespace

std::expected<
    CubicBezierLinearExtrusionSurface3,
    LinearExtrusionSurfaceConstructionError>
CubicBezierLinearExtrusionSurface3::make(
    const CubicBezier3& basis_curve,
    const Vector3& extrusion_displacement) noexcept {
    const auto& controls = basis_curve.control_points();

    const auto p0 = controls[0] + extrusion_displacement;
    const auto p1 = controls[1] + extrusion_displacement;
    const auto p2 = controls[2] + extrusion_displacement;
    const auto p3 = controls[3] + extrusion_displacement;

    if (!p0 || !p1 || !p2 || !p3) {
        return std::unexpected{
            LinearExtrusionSurfaceConstructionError::
                non_finite_extruded_control};
    }

    return CubicBezierLinearExtrusionSurface3{
        basis_curve,
        CubicBezier3{*p0, *p1, *p2, *p3},
        extrusion_displacement};
}

const CubicBezier3&
CubicBezierLinearExtrusionSurface3::basis_curve() const noexcept {
    return basis_curve_;
}

const CubicBezier3&
CubicBezierLinearExtrusionSurface3::end_curve() const noexcept {
    return end_curve_;
}

const Vector3&
CubicBezierLinearExtrusionSurface3::extrusion_displacement() const noexcept {
    return extrusion_displacement_;
}

SurfaceParameterDomain
CubicBezierLinearExtrusionSurface3::parameter_domain() const noexcept {
    return unit_square_domain();
}

std::expected<Point3, SurfaceError>
CubicBezierLinearExtrusionSurface3::evaluate(
    const double u,
    const double v) const noexcept {
    const auto valid = validate_parameters(u, v);
    if (!valid) {
        return std::unexpected{valid.error()};
    }

    if (v == 0.0) {
        const auto value = basis_curve_.evaluate(u);
        if (!value) {
            return std::unexpected{map_curve_error(value.error())};
        }
        return *value;
    }
    if (v == 1.0) {
        const auto value = end_curve_.evaluate(u);
        if (!value) {
            return std::unexpected{map_curve_error(value.error())};
        }
        return *value;
    }

    const auto basis_value = basis_curve_.evaluate(u);
    if (!basis_value) {
        return std::unexpected{map_curve_error(basis_value.error())};
    }
    const auto offset = extrusion_displacement_ * v;
    if (!offset) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    const auto value = *basis_value + *offset;
    if (!value) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    return *value;
}

std::expected<SurfaceFirstDerivatives3, SurfaceError>
CubicBezierLinearExtrusionSurface3::first_derivatives(
    const double u,
    const double v) const noexcept {
    const auto valid = validate_parameters(u, v);
    if (!valid) {
        return std::unexpected{valid.error()};
    }

    const auto du = basis_curve_.first_derivative(u);
    if (!du) {
        return std::unexpected{map_curve_error(du.error())};
    }
    return SurfaceFirstDerivatives3{
        .u = *du,
        .v = extrusion_displacement_,
    };
}

std::expected<SurfaceSecondDerivatives3, SurfaceError>
CubicBezierLinearExtrusionSurface3::second_derivatives(
    const double u,
    const double v) const noexcept {
    const auto valid = validate_parameters(u, v);
    if (!valid) {
        return std::unexpected{valid.error()};
    }

    const auto duu = basis_curve_.second_derivative(u);
    if (!duu) {
        return std::unexpected{map_curve_error(duu.error())};
    }
    const auto zero = zero_vector3();
    return SurfaceSecondDerivatives3{
        .uu = *duu,
        .uv = zero,
        .vv = zero,
    };
}

CubicBezierLinearExtrusionSurface3
CubicBezierLinearExtrusionSurface3::u_reversed() const noexcept {
    return CubicBezierLinearExtrusionSurface3{
        basis_curve_.reversed(),
        end_curve_.reversed(),
        extrusion_displacement_};
}

CubicBezierLinearExtrusionSurface3
CubicBezierLinearExtrusionSurface3::v_reversed() const noexcept {
    return CubicBezierLinearExtrusionSurface3{
        end_curve_,
        basis_curve_,
        -extrusion_displacement_};
}

} // namespace apmesh::core
