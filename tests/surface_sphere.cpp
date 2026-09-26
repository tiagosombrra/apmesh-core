#include "apmesh/core/numeric.hpp"
#include "apmesh/geometry/elementary_surface.hpp"
#include "apmesh/geometry/surface_differential.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <expected>
#include <limits>
#include <numbers>
#include <string_view>

namespace {

bool require(const bool condition, const std::string_view message) {
    if (!condition) {
        std::fputs(message.data(), stderr);
        std::fputc('\n', stderr);
    }
    return condition;
}

bool close_scalar(
    const double lhs,
    const double rhs,
    const double scale = 1.0,
    const double absolute = 8.0e-12,
    const double relative = 8.0e-12) {
    const double reference = std::max(1.0, std::abs(scale));
    const auto comparison = apmesh::core::compare_proximity(
        lhs,
        rhs,
        apmesh::core::ProximityPolicy{
            .absolute_tolerance = absolute * reference,
            .relative_tolerance = relative,
            .reference_scale = reference,
        });
    return comparison.has_value() &&
           comparison->result == apmesh::core::ProximityResult::within;
}

bool close_point(
    const apmesh::core::Point3& lhs,
    const apmesh::core::Point3& rhs,
    const double scale = 1.0) {
    return close_scalar(lhs.x(), rhs.x(), scale) &&
           close_scalar(lhs.y(), rhs.y(), scale) &&
           close_scalar(lhs.z(), rhs.z(), scale);
}

bool close_vector(
    const apmesh::core::Vector3& lhs,
    const apmesh::core::Vector3& rhs,
    const double scale = 1.0) {
    return close_scalar(lhs.x(), rhs.x(), scale) &&
           close_scalar(lhs.y(), rhs.y(), scale) &&
           close_scalar(lhs.z(), rhs.z(), scale);
}

struct OracleTrig {
    long double sine{};
    long double cosine{};
};

OracleTrig latitude_trig(const double v) {
    const double half_pi = 0.5 * std::numbers::pi_v<double>;
    if (v == half_pi) {
        return {1.0L, 0.0L};
    }
    if (v == -half_pi) {
        return {-1.0L, 0.0L};
    }
    const long double angle = static_cast<long double>(v);
    return {std::sin(angle), std::cos(angle)};
}

OracleTrig longitude_trig(const double u) {
    const long double angle = static_cast<long double>(u);
    return {std::sin(angle), std::cos(angle)};
}

std::expected<double, apmesh::core::GeometryError> oracle_double(
    const long double value) {
    if (!std::isfinite(value) ||
        std::abs(value) >
            static_cast<long double>(std::numeric_limits<double>::max())) {
        return std::unexpected{
            apmesh::core::GeometryError::non_finite_result};
    }
    const double converted = static_cast<double>(value);
    if (!std::isfinite(converted)) {
        return std::unexpected{
            apmesh::core::GeometryError::non_finite_result};
    }
    return converted;
}

std::expected<apmesh::core::Point3, apmesh::core::GeometryError>
oracle_world_point(
    const apmesh::core::AxisPlacement3& placement,
    const long double x,
    const long double y,
    const long double z) {
    const long double wx =
        static_cast<long double>(placement.origin().x()) +
        x * static_cast<long double>(placement.x_direction().x()) +
        y * static_cast<long double>(placement.y_direction().x()) +
        z * static_cast<long double>(placement.z_direction().x());
    const long double wy =
        static_cast<long double>(placement.origin().y()) +
        x * static_cast<long double>(placement.x_direction().y()) +
        y * static_cast<long double>(placement.y_direction().y()) +
        z * static_cast<long double>(placement.z_direction().y());
    const long double wz =
        static_cast<long double>(placement.origin().z()) +
        x * static_cast<long double>(placement.x_direction().z()) +
        y * static_cast<long double>(placement.y_direction().z()) +
        z * static_cast<long double>(placement.z_direction().z());

    const auto dx = oracle_double(wx);
    const auto dy = oracle_double(wy);
    const auto dz = oracle_double(wz);
    if (!dx || !dy || !dz) {
        return std::unexpected{
            apmesh::core::GeometryError::non_finite_result};
    }
    return apmesh::core::Point3::make(*dx, *dy, *dz);
}

std::expected<apmesh::core::Vector3, apmesh::core::GeometryError>
oracle_world_vector(
    const apmesh::core::AxisPlacement3& placement,
    const long double x,
    const long double y,
    const long double z) {
    const long double wx =
        x * static_cast<long double>(placement.x_direction().x()) +
        y * static_cast<long double>(placement.y_direction().x()) +
        z * static_cast<long double>(placement.z_direction().x());
    const long double wy =
        x * static_cast<long double>(placement.x_direction().y()) +
        y * static_cast<long double>(placement.y_direction().y()) +
        z * static_cast<long double>(placement.z_direction().y());
    const long double wz =
        x * static_cast<long double>(placement.x_direction().z()) +
        y * static_cast<long double>(placement.y_direction().z()) +
        z * static_cast<long double>(placement.z_direction().z());

    const auto dx = oracle_double(wx);
    const auto dy = oracle_double(wy);
    const auto dz = oracle_double(wz);
    if (!dx || !dy || !dz) {
        return std::unexpected{
            apmesh::core::GeometryError::non_finite_result};
    }
    return apmesh::core::Vector3::make(*dx, *dy, *dz);
}

struct SphereOracle {
    apmesh::core::Point3 value;
    apmesh::core::Vector3 du;
    apmesh::core::Vector3 dv;
    apmesh::core::Vector3 duu;
    apmesh::core::Vector3 duv;
    apmesh::core::Vector3 dvv;
};

std::expected<SphereOracle, apmesh::core::GeometryError> sphere_oracle(
    const apmesh::core::AxisPlacement3& placement,
    const double radius,
    const double u,
    const double v,
    const double u_sign = 1.0,
    const double v_sign = 1.0) {
    const long double r = static_cast<long double>(radius);
    const long double us = static_cast<long double>(u_sign);
    const long double vs = static_cast<long double>(v_sign);
    const OracleTrig ut = longitude_trig(u);
    const OracleTrig vt = latitude_trig(v);

    const long double x = r * vt.cosine * ut.cosine;
    const long double y = r * vt.cosine * ut.sine;
    const long double z = r * vt.sine;

    const auto value = oracle_world_point(placement, x, y, z);
    const auto du = oracle_world_vector(
        placement,
        -us * r * vt.cosine * ut.sine,
        us * r * vt.cosine * ut.cosine,
        0.0L);
    const auto dv = oracle_world_vector(
        placement,
        -vs * r * vt.sine * ut.cosine,
        -vs * r * vt.sine * ut.sine,
        vs * r * vt.cosine);
    const auto duu = oracle_world_vector(
        placement,
        -r * vt.cosine * ut.cosine,
        -r * vt.cosine * ut.sine,
        0.0L);
    const auto duv = oracle_world_vector(
        placement,
        us * vs * r * vt.sine * ut.sine,
        -us * vs * r * vt.sine * ut.cosine,
        0.0L);
    const auto dvv = oracle_world_vector(
        placement,
        -r * vt.cosine * ut.cosine,
        -r * vt.cosine * ut.sine,
        -r * vt.sine);

    if (!value || !du || !dv || !duu || !duv || !dvv) {
        return std::unexpected{
            apmesh::core::GeometryError::non_finite_result};
    }

    return SphereOracle{
        .value = *value,
        .du = *du,
        .dv = *dv,
        .duu = *duu,
        .duv = *duv,
        .dvv = *dvv,
    };
}

} // namespace

int main() {
    using apmesh::core::AxisPlacement3;
    using apmesh::core::BoundedParametricSurface3;
    using apmesh::core::BoundedSphereSurface3;
    using apmesh::core::CurveParameterDomain;
    using apmesh::core::Point3;
    using apmesh::core::SphereSurfaceConstructionError;
    using apmesh::core::SurfaceDifferentialError;
    using apmesh::core::SurfaceError;
    using apmesh::core::Vector3;
    using apmesh::core::reversed_parameter;
    using apmesh::core::surface_metric_normal;
    using apmesh::core::surface_principal_curvatures;
    using apmesh::core::surface_second_order_geometry;

    static_assert(BoundedParametricSurface3<BoundedSphereSurface3>);

    bool passed = true;

    const double pi = std::numbers::pi_v<double>;
    const double half_pi = 0.5 * pi;

    const auto u_domain = CurveParameterDomain::make(-0.75, 1.25);
    const auto v_domain = CurveParameterDomain::make(-0.6, 0.9);
    if (!u_domain || !v_domain) {
        return 1;
    }

    const auto sphere = BoundedSphereSurface3::make(
        AxisPlacement3::identity(),
        2.5,
        *u_domain,
        *v_domain);
    if (!sphere) {
        return 1;
    }

    passed = require(
                 sphere->radius() == 2.5 &&
                     sphere->axis_placement() == AxisPlacement3::identity() &&
                     sphere->u_domain() == *u_domain &&
                     sphere->v_domain() == *v_domain &&
                     !sphere->u_is_reversed() &&
                     !sphere->v_is_reversed() &&
                     sphere->parameter_domain().u == *u_domain &&
                     sphere->parameter_domain().v == *v_domain,
                 "sphere stored representation differs") &&
             passed;

    const auto nan_radius = BoundedSphereSurface3::make(
        AxisPlacement3::identity(),
        std::numeric_limits<double>::quiet_NaN(),
        *u_domain,
        *v_domain);
    const auto zero_radius = BoundedSphereSurface3::make(
        AxisPlacement3::identity(), 0.0, *u_domain, *v_domain);
    const auto negative_radius = BoundedSphereSurface3::make(
        AxisPlacement3::identity(), -1.0, *u_domain, *v_domain);

    const auto full_u = CurveParameterDomain::make(0.0, 2.0 * pi);
    const auto bad_v = CurveParameterDomain::make(-0.5, half_pi + 0.25);
    if (!full_u || !bad_v) {
        return 1;
    }

    const auto full_sphere = BoundedSphereSurface3::make(
        AxisPlacement3::identity(), 1.0, *full_u, *v_domain);
    const auto latitude_out = BoundedSphereSurface3::make(
        AxisPlacement3::identity(), 1.0, *u_domain, *bad_v);

    passed = require(
                 !nan_radius &&
                     nan_radius.error() ==
                         SphereSurfaceConstructionError::non_finite_radius &&
                     !zero_radius &&
                     zero_radius.error() ==
                         SphereSurfaceConstructionError::non_positive_radius &&
                     !negative_radius &&
                     negative_radius.error() ==
                         SphereSurfaceConstructionError::non_positive_radius &&
                     !full_sphere &&
                     full_sphere.error() ==
                         SphereSurfaceConstructionError::
                             full_or_multiple_revolution_not_admitted &&
                     !latitude_out &&
                     latitude_out.error() ==
                         SphereSurfaceConstructionError::latitude_out_of_range,
                 "sphere construction validation differs") &&
             passed;

    const std::array<std::array<double, 2>, 5> parameters{{
        {-0.75, -0.6},
        {-0.2, -0.1},
        {0.0, 0.0},
        {0.45, 0.35},
        {1.25, 0.9},
    }};

    for (const auto& parameter : parameters) {
        const double u = parameter[0];
        const double v = parameter[1];
        const auto oracle =
            sphere_oracle(AxisPlacement3::identity(), 2.5, u, v);
        const auto value = sphere->evaluate(u, v);
        const auto first = sphere->first_derivatives(u, v);
        const auto second = sphere->second_derivatives(u, v);
        passed = require(
                     oracle && value && first && second &&
                         close_point(*value, oracle->value, 4.0) &&
                         close_vector(first->u, oracle->du, 4.0) &&
                         close_vector(first->v, oracle->dv, 4.0) &&
                         close_vector(second->uu, oracle->duu, 4.0) &&
                         close_vector(second->uv, oracle->duv, 4.0) &&
                         close_vector(second->vv, oracle->dvv, 4.0),
                     "sphere independent analytic oracle differs") &&
                 passed;
    }

    const auto nan_u = sphere->evaluate(
        std::numeric_limits<double>::quiet_NaN(), 0.0);
    const auto nan_v = sphere->evaluate(
        0.0, std::numeric_limits<double>::quiet_NaN());
    const auto below_u = sphere->first_derivatives(-0.8, 0.0);
    const auto above_v = sphere->second_derivatives(0.0, 0.95);
    passed = require(
                 !nan_u &&
                     nan_u.error() == SurfaceError::non_finite_u_parameter &&
                     !nan_v &&
                     nan_v.error() == SurfaceError::non_finite_v_parameter &&
                     !below_u &&
                     below_u.error() == SurfaceError::u_parameter_out_of_domain &&
                     !above_v &&
                     above_v.error() == SurfaceError::v_parameter_out_of_domain,
                 "sphere query error order differs") &&
             passed;

    const auto origin = Point3::make(3.0, -4.0, 5.0);
    const auto main_direction = Vector3::make(1.0, 2.0, 3.0);
    const auto x_reference = Vector3::make(2.0, -1.0, 0.5);
    if (!origin || !main_direction || !x_reference) {
        return 1;
    }
    const auto placement = AxisPlacement3::make(
        *origin, *main_direction, *x_reference);
    if (!placement) {
        return 1;
    }

    const auto placed_sphere = BoundedSphereSurface3::make(
        *placement, 3.0, *u_domain, *v_domain);
    if (!placed_sphere) {
        return 1;
    }

    constexpr double placed_u = 0.4;
    constexpr double placed_v = -0.3;
    const auto placed_oracle =
        sphere_oracle(*placement, 3.0, placed_u, placed_v);
    const auto placed_value =
        placed_sphere->evaluate(placed_u, placed_v);
    const auto placed_first =
        placed_sphere->first_derivatives(placed_u, placed_v);
    const auto placed_second =
        placed_sphere->second_derivatives(placed_u, placed_v);
    passed = require(
                 placed_oracle && placed_value && placed_first &&
                     placed_second &&
                     close_point(*placed_value, placed_oracle->value, 8.0) &&
                     close_vector(placed_first->u, placed_oracle->du, 8.0) &&
                     close_vector(placed_first->v, placed_oracle->dv, 8.0) &&
                     close_vector(placed_second->uu, placed_oracle->duu, 8.0) &&
                     close_vector(placed_second->uv, placed_oracle->duv, 8.0) &&
                     close_vector(placed_second->vv, placed_oracle->dvv, 8.0),
                 "arbitrary-placement sphere oracle differs") &&
             passed;

    const auto translated_origin = Point3::make(8.0, -6.0, 4.0);
    const auto canonical_z = Vector3::make(0.0, 0.0, 1.0);
    const auto canonical_x = Vector3::make(1.0, 0.0, 0.0);
    if (!translated_origin || !canonical_z || !canonical_x) {
        return 1;
    }
    const auto translated_placement = AxisPlacement3::make(
        *translated_origin, *canonical_z, *canonical_x);
    if (!translated_placement) {
        return 1;
    }
    const auto translated_sphere = BoundedSphereSurface3::make(
        *translated_placement, 2.5, *u_domain, *v_domain);
    const auto scaled_sphere = BoundedSphereSurface3::make(
        AxisPlacement3::identity(), 20.0, *u_domain, *v_domain);
    if (!translated_sphere || !scaled_sphere) {
        return 1;
    }

    const auto covariance_base = sphere->evaluate(0.4, 0.2);
    const auto covariance_translated = translated_sphere->evaluate(0.4, 0.2);
    const auto covariance_scaled = scaled_sphere->evaluate(0.4, 0.2);
    const auto base_covariance_first = sphere->first_derivatives(0.4, 0.2);
    const auto translated_covariance_first =
        translated_sphere->first_derivatives(0.4, 0.2);
    const auto scaled_covariance_first =
        scaled_sphere->first_derivatives(0.4, 0.2);
    const auto base_covariance_second =
        sphere->second_derivatives(0.4, 0.2);
    const auto translated_covariance_second =
        translated_sphere->second_derivatives(0.4, 0.2);
    const auto scaled_covariance_second =
        scaled_sphere->second_derivatives(0.4, 0.2);

    passed = require(
                 covariance_base && covariance_translated && covariance_scaled &&
                     close_scalar(
                         covariance_translated->x() - covariance_base->x(),
                         8.0,
                         16.0) &&
                     close_scalar(
                         covariance_translated->y() - covariance_base->y(),
                         -6.0,
                         16.0) &&
                     close_scalar(
                         covariance_translated->z() - covariance_base->z(),
                         4.0,
                         16.0) &&
                     close_scalar(
                         covariance_scaled->x(),
                         8.0 * covariance_base->x(),
                         32.0) &&
                     close_scalar(
                         covariance_scaled->y(),
                         8.0 * covariance_base->y(),
                         32.0) &&
                     close_scalar(
                         covariance_scaled->z(),
                         8.0 * covariance_base->z(),
                         32.0) &&
                     base_covariance_first && translated_covariance_first &&
                     scaled_covariance_first &&
                     close_vector(
                         base_covariance_first->u,
                         translated_covariance_first->u,
                         16.0) &&
                     close_vector(
                         base_covariance_first->v,
                         translated_covariance_first->v,
                         16.0) &&
                     close_scalar(
                         scaled_covariance_first->u.x(),
                         8.0 * base_covariance_first->u.x(),
                         64.0) &&
                     close_scalar(
                         scaled_covariance_first->u.y(),
                         8.0 * base_covariance_first->u.y(),
                         64.0) &&
                     close_scalar(
                         scaled_covariance_first->u.z(),
                         8.0 * base_covariance_first->u.z(),
                         64.0) &&
                     close_scalar(
                         scaled_covariance_first->v.x(),
                         8.0 * base_covariance_first->v.x(),
                         64.0) &&
                     close_scalar(
                         scaled_covariance_first->v.y(),
                         8.0 * base_covariance_first->v.y(),
                         64.0) &&
                     close_scalar(
                         scaled_covariance_first->v.z(),
                         8.0 * base_covariance_first->v.z(),
                         64.0) &&
                     base_covariance_second && translated_covariance_second &&
                     scaled_covariance_second &&
                     close_vector(
                         base_covariance_second->uu,
                         translated_covariance_second->uu,
                         32.0) &&
                     close_vector(
                         base_covariance_second->uv,
                         translated_covariance_second->uv,
                         32.0) &&
                     close_vector(
                         base_covariance_second->vv,
                         translated_covariance_second->vv,
                         32.0) &&
                     close_scalar(
                         scaled_covariance_second->uu.x(),
                         8.0 * base_covariance_second->uu.x(),
                         128.0) &&
                     close_scalar(
                         scaled_covariance_second->uu.y(),
                         8.0 * base_covariance_second->uu.y(),
                         128.0) &&
                     close_scalar(
                         scaled_covariance_second->uu.z(),
                         8.0 * base_covariance_second->uu.z(),
                         128.0) &&
                     close_scalar(
                         scaled_covariance_second->uv.x(),
                         8.0 * base_covariance_second->uv.x(),
                         128.0) &&
                     close_scalar(
                         scaled_covariance_second->uv.y(),
                         8.0 * base_covariance_second->uv.y(),
                         128.0) &&
                     close_scalar(
                         scaled_covariance_second->uv.z(),
                         8.0 * base_covariance_second->uv.z(),
                         128.0) &&
                     close_scalar(
                         scaled_covariance_second->vv.x(),
                         8.0 * base_covariance_second->vv.x(),
                         128.0) &&
                     close_scalar(
                         scaled_covariance_second->vv.y(),
                         8.0 * base_covariance_second->vv.y(),
                         128.0) &&
                     close_scalar(
                         scaled_covariance_second->vv.z(),
                         8.0 * base_covariance_second->vv.z(),
                         128.0),
                 "sphere translation/coordinate-scale covariance differs") &&
             passed;

    const auto u_reversed = sphere->u_reversed();
    const auto v_reversed = sphere->v_reversed();
    const auto both_reversed = u_reversed.v_reversed();
    const auto mapped_u = reversed_parameter(*u_domain, 0.2);
    const auto mapped_v = reversed_parameter(*v_domain, 0.3);
    if (!mapped_u || !mapped_v) {
        return 1;
    }

    const auto base_value = sphere->evaluate(0.2, 0.3);
    const auto base_first = sphere->first_derivatives(0.2, 0.3);
    const auto base_second = sphere->second_derivatives(0.2, 0.3);
    const auto ur_value = u_reversed.evaluate(*mapped_u, 0.3);
    const auto ur_first = u_reversed.first_derivatives(*mapped_u, 0.3);
    const auto ur_second = u_reversed.second_derivatives(*mapped_u, 0.3);
    const auto vr_value = v_reversed.evaluate(0.2, *mapped_v);
    const auto vr_first = v_reversed.first_derivatives(0.2, *mapped_v);
    const auto vr_second = v_reversed.second_derivatives(0.2, *mapped_v);
    const auto br_value = both_reversed.evaluate(*mapped_u, *mapped_v);
    const auto br_first =
        both_reversed.first_derivatives(*mapped_u, *mapped_v);
    const auto br_second =
        both_reversed.second_derivatives(*mapped_u, *mapped_v);

    passed = require(
                 base_value && base_first && base_second &&
                     ur_value && ur_first && ur_second &&
                     vr_value && vr_first && vr_second &&
                     br_value && br_first && br_second &&
                     close_point(*base_value, *ur_value, 4.0) &&
                     close_vector(base_first->u, -ur_first->u, 4.0) &&
                     close_vector(base_first->v, ur_first->v, 4.0) &&
                     close_vector(base_second->uu, ur_second->uu, 4.0) &&
                     close_vector(base_second->uv, -ur_second->uv, 4.0) &&
                     close_vector(base_second->vv, ur_second->vv, 4.0) &&
                     close_point(*base_value, *vr_value, 4.0) &&
                     close_vector(base_first->u, vr_first->u, 4.0) &&
                     close_vector(base_first->v, -vr_first->v, 4.0) &&
                     close_vector(base_second->uu, vr_second->uu, 4.0) &&
                     close_vector(base_second->uv, -vr_second->uv, 4.0) &&
                     close_vector(base_second->vv, vr_second->vv, 4.0) &&
                     close_point(*base_value, *br_value, 4.0) &&
                     close_vector(base_first->u, -br_first->u, 4.0) &&
                     close_vector(base_first->v, -br_first->v, 4.0) &&
                     close_vector(base_second->uu, br_second->uu, 4.0) &&
                     close_vector(base_second->uv, br_second->uv, 4.0) &&
                     close_vector(base_second->vv, br_second->vv, 4.0) &&
                     u_reversed.u_reversed() == *sphere &&
                     v_reversed.v_reversed() == *sphere,
                 "sphere reversal covariance differs") &&
             passed;

    const auto polar_u = CurveParameterDomain::make(-0.5, 0.5);
    const auto polar_v = CurveParameterDomain::make(-half_pi, half_pi);
    if (!polar_u || !polar_v) {
        return 1;
    }
    const auto unit_sphere = BoundedSphereSurface3::make(
        AxisPlacement3::identity(), 1.0, *polar_u, *polar_v);
    if (!unit_sphere) {
        return 1;
    }

    const auto north_a = unit_sphere->evaluate(-0.4, half_pi);
    const auto north_b = unit_sphere->evaluate(0.4, half_pi);
    const auto south_a = unit_sphere->evaluate(-0.4, -half_pi);
    const auto north_first =
        unit_sphere->first_derivatives(0.0, half_pi);
    const auto north_second =
        unit_sphere->second_derivatives(0.0, half_pi);
    const auto north_point = Point3::make(0.0, 0.0, 1.0);
    const auto south_point = Point3::make(0.0, 0.0, -1.0);
    if (!north_point || !south_point) {
        return 1;
    }

    passed = require(
                 north_a && north_b && south_a &&
                     *north_a == *north_point &&
                     *north_b == *north_point &&
                     *south_a == *south_point &&
                     north_first &&
                     north_first->u.x() == 0.0 &&
                     north_first->u.y() == 0.0 &&
                     north_first->u.z() == 0.0 &&
                     north_second &&
                     north_second->uu.x() == 0.0 &&
                     north_second->uu.y() == 0.0 &&
                     north_second->uu.z() == 0.0,
                 "canonical sphere pole semantics differ") &&
             passed;

    const auto pole_metric =
        surface_metric_normal(*unit_sphere, 0.0, half_pi);
    const auto pole_second =
        surface_second_order_geometry(*unit_sphere, 0.0, half_pi);
    const auto pole_principal =
        surface_principal_curvatures(*unit_sphere, 0.0, half_pi);
    passed = require(
                 !pole_metric &&
                     pole_metric.error() ==
                         SurfaceDifferentialError::singular_parameterization &&
                     !pole_second &&
                     pole_second.error() ==
                         SurfaceDifferentialError::singular_parameterization &&
                     !pole_principal &&
                     pole_principal.error() ==
                         SurfaceDifferentialError::singular_parameterization,
                 "canonical sphere pole differential singularity differs") &&
             passed;

    const auto polar_v_reversed = unit_sphere->v_reversed();
    const auto reversed_north =
        polar_v_reversed.evaluate(0.25, -half_pi);
    const auto reversed_north_first =
        polar_v_reversed.first_derivatives(0.25, -half_pi);
    const auto reversed_north_metric =
        surface_metric_normal(polar_v_reversed, 0.25, -half_pi);
    passed = require(
                 reversed_north &&
                     *reversed_north == *north_point &&
                     reversed_north_first &&
                     reversed_north_first->u.x() == 0.0 &&
                     reversed_north_first->u.y() == 0.0 &&
                     reversed_north_first->u.z() == 0.0 &&
                     !reversed_north_metric &&
                     reversed_north_metric.error() ==
                         SurfaceDifferentialError::singular_parameterization &&
                     polar_v_reversed.v_reversed() == *unit_sphere,
                 "sphere polar V-reversal semantics differ") &&
             passed;

    const auto equator_metric =
        surface_metric_normal(*unit_sphere, 0.0, 0.0);
    const auto equator_geometry =
        surface_second_order_geometry(*unit_sphere, 0.0, 0.0);
    const auto equator_principal =
        surface_principal_curvatures(*unit_sphere, 0.0, 0.0);
    passed = require(
                 equator_metric &&
                     equator_metric->first_fundamental_form.e == 1.0 &&
                     equator_metric->first_fundamental_form.f == 0.0 &&
                     equator_metric->first_fundamental_form.g == 1.0 &&
                     equator_metric->area_density == 1.0 &&
                     equator_metric->unit_normal.x() == 1.0 &&
                     equator_metric->unit_normal.y() == 0.0 &&
                     equator_metric->unit_normal.z() == 0.0 &&
                     equator_geometry &&
                     equator_geometry->gaussian_curvature == 1.0 &&
                     equator_geometry->mean_curvature == -1.0 &&
                     equator_principal &&
                     equator_principal->maximum_curvature == -1.0 &&
                     equator_principal->minimum_curvature == -1.0 &&
                     equator_principal->is_umbilic,
                 "canonical sphere differential oracle differs") &&
             passed;

    const double near_north =
        std::nextafter(half_pi, -std::numeric_limits<double>::infinity());
    const auto near_metric =
        surface_metric_normal(*unit_sphere, 0.0, near_north);
    passed = require(
                 near_metric.has_value(),
                 "representable near-pole sphere parameter was rejected") &&
             passed;

    const auto radius_eight = BoundedSphereSurface3::make(
        AxisPlacement3::identity(), 8.0, *polar_u, *polar_v);
    if (!radius_eight) {
        return 1;
    }
    const auto scaled_geometry =
        surface_second_order_geometry(*radius_eight, 0.0, 0.0);
    const auto scaled_principal =
        surface_principal_curvatures(*radius_eight, 0.0, 0.0);
    passed = require(
                 scaled_geometry && scaled_principal &&
                     close_scalar(
                         scaled_geometry->gaussian_curvature,
                         1.0 / 64.0,
                         1.0) &&
                     close_scalar(
                         scaled_geometry->mean_curvature,
                         -1.0 / 8.0,
                         1.0) &&
                     close_scalar(
                         scaled_principal->maximum_curvature,
                         -1.0 / 8.0,
                         1.0) &&
                     close_scalar(
                         scaled_principal->minimum_curvature,
                         -1.0 / 8.0,
                         1.0) &&
                     scaled_principal->is_umbilic,
                 "sphere curvature scale covariance differs") &&
             passed;

    const double large_radius =
        std::numeric_limits<double>::max() / 8.0;
    const auto extreme = BoundedSphereSurface3::make(
        AxisPlacement3::identity(),
        large_radius,
        *polar_u,
        *v_domain);
    if (!extreme) {
        return 1;
    }
    const auto extreme_value = extreme->evaluate(0.0, 0.0);
    const auto extreme_first = extreme->first_derivatives(0.0, 0.0);
    const auto extreme_second = extreme->second_derivatives(0.0, 0.0);
    passed = require(
                 extreme_value && extreme_first && extreme_second &&
                     std::isfinite(extreme_value->x()) &&
                     std::isfinite(extreme_first->u.y()) &&
                     std::isfinite(extreme_second->uu.x()),
                 "sphere extreme finite jet introduced avoidable overflow") &&
             passed;

    const double maximum = std::numeric_limits<double>::max();
    const auto huge_origin = Point3::make(maximum, 0.0, 0.0);
    const auto z_axis = Vector3::make(0.0, 0.0, 1.0);
    const auto x_axis = Vector3::make(1.0, 0.0, 0.0);
    if (!huge_origin || !z_axis || !x_axis) {
        return 1;
    }
    const auto huge_placement =
        AxisPlacement3::make(*huge_origin, *z_axis, *x_axis);
    if (!huge_placement) {
        return 1;
    }
    const auto unrepresentable_sphere = BoundedSphereSurface3::make(
        *huge_placement, maximum, *polar_u, *v_domain);
    if (!unrepresentable_sphere) {
        return 1;
    }
    const auto unrepresentable =
        unrepresentable_sphere->evaluate(0.0, 0.0);
    passed = require(
                 !unrepresentable &&
                     unrepresentable.error() ==
                         SurfaceError::non_finite_result,
                 "sphere unrepresentable world point did not fail explicitly") &&
             passed;

    const auto repeat_value_a = sphere->evaluate(0.4, 0.2);
    const auto repeat_value_b = sphere->evaluate(0.4, 0.2);
    const auto repeat_first_a = sphere->first_derivatives(0.4, 0.2);
    const auto repeat_first_b = sphere->first_derivatives(0.4, 0.2);
    const auto repeat_failure_a = sphere->evaluate(2.0, 0.0);
    const auto repeat_failure_b = sphere->evaluate(2.0, 0.0);
    passed = require(
                 repeat_value_a && repeat_value_b &&
                     *repeat_value_a == *repeat_value_b &&
                     repeat_first_a && repeat_first_b &&
                     *repeat_first_a == *repeat_first_b &&
                     !repeat_failure_a && !repeat_failure_b &&
                     repeat_failure_a.error() == repeat_failure_b.error(),
                 "sphere repeated success/failure evidence is not deterministic") &&
             passed;

    return passed ? 0 : 1;
}
