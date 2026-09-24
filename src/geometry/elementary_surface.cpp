#include "apmesh/geometry/elementary_surface.hpp"

#include <cmath>
#include <expected>

namespace apmesh::core {
namespace {

[[nodiscard]] std::expected<void, SurfaceError> validate_parameters(
    const CurveParameterDomain& u_domain,
    const CurveParameterDomain& v_domain,
    const double u,
    const double v) noexcept {
    if (!std::isfinite(u)) {
        return std::unexpected{SurfaceError::non_finite_u_parameter};
    }
    if (!std::isfinite(v)) {
        return std::unexpected{SurfaceError::non_finite_v_parameter};
    }

    const auto u_contained = u_domain.contains(u);
    if (!u_contained.has_value() || !*u_contained) {
        return std::unexpected{SurfaceError::u_parameter_out_of_domain};
    }

    const auto v_contained = v_domain.contains(v);
    if (!v_contained.has_value() || !*v_contained) {
        return std::unexpected{SurfaceError::v_parameter_out_of_domain};
    }
    return {};
}

[[nodiscard]] std::expected<double, SurfaceError> effective_parameter(
    const CurveParameterDomain& domain,
    const double parameter,
    const bool reversed) noexcept {
    if (!reversed) {
        return parameter;
    }
    const auto value = reversed_parameter(domain, parameter);
    if (!value.has_value()) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    return *value;
}

[[nodiscard]] Vector3 zero_vector3() noexcept {
    return *Vector3::make(0.0, 0.0, 0.0);
}

} // namespace

const AxisPlacement3&
BoundedPlaneSurface3::axis_placement() const noexcept {
    return placement_;
}

const CurveParameterDomain&
BoundedPlaneSurface3::u_domain() const noexcept {
    return u_domain_;
}

const CurveParameterDomain&
BoundedPlaneSurface3::v_domain() const noexcept {
    return v_domain_;
}

bool BoundedPlaneSurface3::u_is_reversed() const noexcept {
    return u_reversed_;
}

bool BoundedPlaneSurface3::v_is_reversed() const noexcept {
    return v_reversed_;
}

SurfaceParameterDomain
BoundedPlaneSurface3::parameter_domain() const noexcept {
    return SurfaceParameterDomain{
        .u = u_domain_,
        .v = v_domain_,
    };
}

std::expected<Point3, SurfaceError>
BoundedPlaneSurface3::evaluate(
    const double u,
    const double v) const noexcept {
    const auto valid =
        validate_parameters(u_domain_, v_domain_, u, v);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }

    const auto effective_u =
        effective_parameter(u_domain_, u, u_reversed_);
    const auto effective_v =
        effective_parameter(v_domain_, v, v_reversed_);
    if (!effective_u.has_value() || !effective_v.has_value()) {
        return std::unexpected{SurfaceError::non_finite_result};
    }

    const auto local =
        Point3::make(*effective_u, *effective_v, 0.0);
    if (!local.has_value()) {
        return std::unexpected{SurfaceError::non_finite_result};
    }

    const auto world = placement_.point_to_world(*local);
    if (!world.has_value()) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    return *world;
}

std::expected<SurfaceFirstDerivatives3, SurfaceError>
BoundedPlaneSurface3::first_derivatives(
    const double u,
    const double v) const noexcept {
    const auto valid =
        validate_parameters(u_domain_, v_domain_, u, v);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }

    return SurfaceFirstDerivatives3{
        .u = u_reversed_ ? -placement_.x_direction()
                         : placement_.x_direction(),
        .v = v_reversed_ ? -placement_.y_direction()
                         : placement_.y_direction(),
    };
}

std::expected<SurfaceSecondDerivatives3, SurfaceError>
BoundedPlaneSurface3::second_derivatives(
    const double u,
    const double v) const noexcept {
    const auto valid =
        validate_parameters(u_domain_, v_domain_, u, v);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }

    const auto zero = zero_vector3();
    return SurfaceSecondDerivatives3{
        .uu = zero,
        .uv = zero,
        .vv = zero,
    };
}

BoundedPlaneSurface3
BoundedPlaneSurface3::u_reversed() const noexcept {
    return BoundedPlaneSurface3{
        placement_,
        u_domain_,
        v_domain_,
        !u_reversed_,
        v_reversed_};
}

BoundedPlaneSurface3
BoundedPlaneSurface3::v_reversed() const noexcept {
    return BoundedPlaneSurface3{
        placement_,
        u_domain_,
        v_domain_,
        u_reversed_,
        !v_reversed_};
}

} // namespace apmesh::core
