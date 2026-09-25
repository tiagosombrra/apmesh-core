#include "apmesh/geometry/elementary_surface.hpp"

#include <cmath>
#include <expected>
#include <limits>
#include <numbers>

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


[[nodiscard]] std::expected<double, SurfaceError> finite_double(
    const long double value) noexcept {
    if (!std::isfinite(value) ||
        std::abs(value) >
            static_cast<long double>(std::numeric_limits<double>::max())) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    const double result = static_cast<double>(value);
    if (!std::isfinite(result)) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    return result;
}

[[nodiscard]] std::expected<Point3, SurfaceError> cylinder_local_point(
    const double radius,
    const double u,
    const double v) noexcept {
    const long double angle = static_cast<long double>(u);
    const long double radial = static_cast<long double>(radius);
    const auto x = finite_double(radial * std::cos(angle));
    const auto y = finite_double(radial * std::sin(angle));
    if (!x || !y) {
        return std::unexpected{SurfaceError::non_finite_result};
    }

    const auto point = Point3::make(*x, *y, v);
    if (!point) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    return *point;
}

[[nodiscard]] std::expected<Vector3, SurfaceError> cylinder_local_u(
    const double radius,
    const double u,
    const double sign) noexcept {
    const long double angle = static_cast<long double>(u);
    const long double radial = static_cast<long double>(radius);
    const long double direction = static_cast<long double>(sign);
    const auto x =
        finite_double(-direction * radial * std::sin(angle));
    const auto y =
        finite_double(direction * radial * std::cos(angle));
    if (!x || !y) {
        return std::unexpected{SurfaceError::non_finite_result};
    }

    const auto vector = Vector3::make(*x, *y, 0.0);
    if (!vector) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    return *vector;
}

[[nodiscard]] std::expected<Vector3, SurfaceError> cylinder_local_uu(
    const double radius,
    const double u) noexcept {
    const long double angle = static_cast<long double>(u);
    const long double radial = static_cast<long double>(radius);
    const auto x = finite_double(-radial * std::cos(angle));
    const auto y = finite_double(-radial * std::sin(angle));
    if (!x || !y) {
        return std::unexpected{SurfaceError::non_finite_result};
    }

    const auto vector = Vector3::make(*x, *y, 0.0);
    if (!vector) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    return *vector;
}

[[nodiscard]] std::expected<Vector3, SurfaceError> world_vector(
    const AxisPlacement3& placement,
    const Vector3& local) noexcept {
    const auto value = placement.vector_to_world(local);
    if (!value) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    return *value;
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


std::expected<BoundedCylinderSurface3, CylinderSurfaceConstructionError>
BoundedCylinderSurface3::make(
    const AxisPlacement3& placement,
    const double radius,
    const CurveParameterDomain& u_domain,
    const CurveParameterDomain& v_domain) noexcept {
    if (!std::isfinite(radius)) {
        return std::unexpected{
            CylinderSurfaceConstructionError::non_finite_radius};
    }
    if (radius <= 0.0) {
        return std::unexpected{
            CylinderSurfaceConstructionError::non_positive_radius};
    }

    const long double angular_width =
        static_cast<long double>(u_domain.upper()) -
        static_cast<long double>(u_domain.lower());
    const long double full_revolution =
        static_cast<long double>(
            2.0 * std::numbers::pi_v<double>);
    if (!std::isfinite(angular_width) ||
        angular_width >= full_revolution) {
        return std::unexpected{
            CylinderSurfaceConstructionError::
                full_or_multiple_revolution_not_admitted};
    }

    return BoundedCylinderSurface3{
        placement,
        radius,
        u_domain,
        v_domain,
        false,
        false};
}

const AxisPlacement3&
BoundedCylinderSurface3::axis_placement() const noexcept {
    return placement_;
}

double BoundedCylinderSurface3::radius() const noexcept {
    return radius_;
}

const CurveParameterDomain&
BoundedCylinderSurface3::u_domain() const noexcept {
    return u_domain_;
}

const CurveParameterDomain&
BoundedCylinderSurface3::v_domain() const noexcept {
    return v_domain_;
}

bool BoundedCylinderSurface3::u_is_reversed() const noexcept {
    return u_reversed_;
}

bool BoundedCylinderSurface3::v_is_reversed() const noexcept {
    return v_reversed_;
}

SurfaceParameterDomain
BoundedCylinderSurface3::parameter_domain() const noexcept {
    return SurfaceParameterDomain{
        .u = u_domain_,
        .v = v_domain_,
    };
}

std::expected<Point3, SurfaceError>
BoundedCylinderSurface3::evaluate(
    const double u,
    const double v) const noexcept {
    const auto valid =
        validate_parameters(u_domain_, v_domain_, u, v);
    if (!valid) {
        return std::unexpected{valid.error()};
    }

    const auto effective_u =
        effective_parameter(u_domain_, u, u_reversed_);
    const auto effective_v =
        effective_parameter(v_domain_, v, v_reversed_);
    if (!effective_u || !effective_v) {
        return std::unexpected{SurfaceError::non_finite_result};
    }

    const auto local =
        cylinder_local_point(radius_, *effective_u, *effective_v);
    if (!local) {
        return std::unexpected{local.error()};
    }
    const auto world = placement_.point_to_world(*local);
    if (!world) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    return *world;
}

std::expected<SurfaceFirstDerivatives3, SurfaceError>
BoundedCylinderSurface3::first_derivatives(
    const double u,
    const double v) const noexcept {
    const auto valid =
        validate_parameters(u_domain_, v_domain_, u, v);
    if (!valid) {
        return std::unexpected{valid.error()};
    }

    const auto effective_u =
        effective_parameter(u_domain_, u, u_reversed_);
    if (!effective_u) {
        return std::unexpected{SurfaceError::non_finite_result};
    }

    const auto local_u = cylinder_local_u(
        radius_,
        *effective_u,
        u_reversed_ ? -1.0 : 1.0);
    const auto local_v = Vector3::make(
        0.0, 0.0, v_reversed_ ? -1.0 : 1.0);
    if (!local_u || !local_v) {
        return std::unexpected{SurfaceError::non_finite_result};
    }

    const auto world_u = world_vector(placement_, *local_u);
    const auto world_v = world_vector(placement_, *local_v);
    if (!world_u || !world_v) {
        return std::unexpected{SurfaceError::non_finite_result};
    }

    return SurfaceFirstDerivatives3{
        .u = *world_u,
        .v = *world_v,
    };
}

std::expected<SurfaceSecondDerivatives3, SurfaceError>
BoundedCylinderSurface3::second_derivatives(
    const double u,
    const double v) const noexcept {
    const auto valid =
        validate_parameters(u_domain_, v_domain_, u, v);
    if (!valid) {
        return std::unexpected{valid.error()};
    }

    const auto effective_u =
        effective_parameter(u_domain_, u, u_reversed_);
    if (!effective_u) {
        return std::unexpected{SurfaceError::non_finite_result};
    }

    const auto local_uu =
        cylinder_local_uu(radius_, *effective_u);
    if (!local_uu) {
        return std::unexpected{local_uu.error()};
    }

    const auto world_uu = world_vector(placement_, *local_uu);
    if (!world_uu) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    const auto zero = zero_vector3();

    return SurfaceSecondDerivatives3{
        .uu = *world_uu,
        .uv = zero,
        .vv = zero,
    };
}

BoundedCylinderSurface3
BoundedCylinderSurface3::u_reversed() const noexcept {
    return BoundedCylinderSurface3{
        placement_,
        radius_,
        u_domain_,
        v_domain_,
        !u_reversed_,
        v_reversed_};
}

BoundedCylinderSurface3
BoundedCylinderSurface3::v_reversed() const noexcept {
    return BoundedCylinderSurface3{
        placement_,
        radius_,
        u_domain_,
        v_domain_,
        u_reversed_,
        !v_reversed_};
}

} // namespace apmesh::core
