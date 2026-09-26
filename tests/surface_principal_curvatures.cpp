#include "apmesh/core/numeric.hpp"
#include "apmesh/geometry/elementary_surface.hpp"
#include "apmesh/geometry/surface_differential.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <expected>
#include <limits>
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
    const double absolute = 6.0e-12,
    const double relative = 6.0e-12) {
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

apmesh::core::Vector3 vector3(
    const double x,
    const double y,
    const double z) {
    return *apmesh::core::Vector3::make(x, y, z);
}

apmesh::core::SurfaceSecondOrderGeometry3 synthetic_geometry(
    const double e,
    const double f,
    const double g,
    const double area_density,
    const double l,
    const double m,
    const double n,
    const double gaussian,
    const double mean) {
    return {
        .metric_normal = {
            .first_fundamental_form = {
                .e = e,
                .f = f,
                .g = g,
            },
            .area_density = area_density,
            .unit_normal = vector3(0.0, 0.0, 1.0),
        },
        .second_fundamental_form = {
            .l = l,
            .m = m,
            .n = n,
        },
        .gaussian_curvature = gaussian,
        .mean_curvature = mean,
    };
}

bool check_hk(
    const apmesh::core::SurfacePrincipalCurvatures& principal,
    const apmesh::core::SurfaceSecondOrderGeometry3& geometry,
    const double scale = 1.0) {
    return close_scalar(
               principal.maximum_curvature + principal.minimum_curvature,
               2.0 * geometry.mean_curvature,
               scale,
               2.0e-11,
               2.0e-11) &&
           close_scalar(
               principal.maximum_curvature * principal.minimum_curvature,
               geometry.gaussian_curvature,
               scale,
               4.0e-11,
               4.0e-11);
}

struct SyntheticSurface3 {
    apmesh::core::SurfaceFirstDerivatives3 first;
    apmesh::core::SurfaceSecondDerivatives3 second;
    bool fail_second{};

    [[nodiscard]] apmesh::core::SurfaceParameterDomain
    parameter_domain() const noexcept {
        return {
            .u = *apmesh::core::CurveParameterDomain::make(0.0, 1.0),
            .v = *apmesh::core::CurveParameterDomain::make(0.0, 1.0),
        };
    }

    [[nodiscard]] std::expected<
        apmesh::core::Point3,
        apmesh::core::SurfaceError>
    evaluate(const double u, const double v) const noexcept {
        const auto valid = validate(u, v);
        if (!valid.has_value()) {
            return std::unexpected{valid.error()};
        }
        return *apmesh::core::Point3::make(0.0, 0.0, 0.0);
    }

    [[nodiscard]] std::expected<
        apmesh::core::SurfaceFirstDerivatives3,
        apmesh::core::SurfaceError>
    first_derivatives(const double u, const double v) const noexcept {
        const auto valid = validate(u, v);
        if (!valid.has_value()) {
            return std::unexpected{valid.error()};
        }
        return first;
    }

    [[nodiscard]] std::expected<
        apmesh::core::SurfaceSecondDerivatives3,
        apmesh::core::SurfaceError>
    second_derivatives(const double u, const double v) const noexcept {
        const auto valid = validate(u, v);
        if (!valid.has_value()) {
            return std::unexpected{valid.error()};
        }
        if (fail_second) {
            return std::unexpected{
                apmesh::core::SurfaceError::insufficient_continuity};
        }
        return second;
    }

private:
    [[nodiscard]] std::expected<void, apmesh::core::SurfaceError>
    validate(const double u, const double v) const noexcept {
        if (!std::isfinite(u)) {
            return std::unexpected{
                apmesh::core::SurfaceError::non_finite_u_parameter};
        }
        if (!std::isfinite(v)) {
            return std::unexpected{
                apmesh::core::SurfaceError::non_finite_v_parameter};
        }
        if (u < 0.0 || u > 1.0) {
            return std::unexpected{
                apmesh::core::SurfaceError::u_parameter_out_of_domain};
        }
        if (v < 0.0 || v > 1.0) {
            return std::unexpected{
                apmesh::core::SurfaceError::v_parameter_out_of_domain};
        }
        return {};
    }
};

} // namespace

int main() {
    using apmesh::core::AxisPlacement3;
    using apmesh::core::BoundedCylinderSurface3;
    using apmesh::core::BoundedPlaneSurface3;
    using apmesh::core::CurveParameterDomain;
    using apmesh::core::Point3;
    using apmesh::core::SurfaceDifferentialError;
    using apmesh::core::SurfaceFirstDerivatives3;
    using apmesh::core::SurfacePrincipalCurvatures;
    using apmesh::core::SurfaceSecondDerivatives3;
    using apmesh::core::Vector3;
    using apmesh::core::reversed_parameter;
    using apmesh::core::surface_principal_curvatures;

    static_assert(apmesh::core::BoundedParametricSurface3<SyntheticSurface3>);

    bool passed = true;

    const auto elliptic = synthetic_geometry(
        1.0, 0.0, 1.0, 1.0,
        3.0, 0.0, 1.0,
        3.0, 2.0);
    const auto elliptic_values = surface_principal_curvatures(elliptic);
    passed = require(
                 elliptic_values &&
                     elliptic_values->maximum_curvature == 3.0 &&
                     elliptic_values->minimum_curvature == 1.0 &&
                     !elliptic_values->is_umbilic &&
                     check_hk(*elliptic_values, elliptic, 3.0),
                 "elliptic principal curvatures differ") &&
             passed;

    const auto hyperbolic = synthetic_geometry(
        1.0, 0.0, 1.0, 1.0,
        2.0, 0.0, -4.0,
        -8.0, -1.0);
    const auto hyperbolic_values = surface_principal_curvatures(hyperbolic);
    passed = require(
                 hyperbolic_values &&
                     hyperbolic_values->maximum_curvature == 2.0 &&
                     hyperbolic_values->minimum_curvature == -4.0 &&
                     !hyperbolic_values->is_umbilic &&
                     check_hk(*hyperbolic_values, hyperbolic, 8.0),
                 "hyperbolic principal curvatures differ") &&
             passed;

    const auto parabolic = synthetic_geometry(
        1.0, 0.0, 1.0, 1.0,
        2.0, 0.0, 0.0,
        0.0, 1.0);
    const auto parabolic_values = surface_principal_curvatures(parabolic);
    passed = require(
                 parabolic_values &&
                     parabolic_values->maximum_curvature == 2.0 &&
                     parabolic_values->minimum_curvature == 0.0 &&
                     !parabolic_values->is_umbilic &&
                     check_hk(*parabolic_values, parabolic, 2.0),
                 "parabolic principal curvatures differ") &&
             passed;

    const auto exact_umbilic = synthetic_geometry(
        1.0, 0.0, 1.0, 1.0,
        2.0, 0.0, 2.0,
        4.0, 2.0);
    const auto exact_umbilic_values =
        surface_principal_curvatures(exact_umbilic);
    passed = require(
                 exact_umbilic_values &&
                     exact_umbilic_values->maximum_curvature == 2.0 &&
                     exact_umbilic_values->minimum_curvature == 2.0 &&
                     exact_umbilic_values->is_umbilic &&
                     check_hk(
                         *exact_umbilic_values, exact_umbilic, 4.0),
                 "exact nonzero umbilic state differs") &&
             passed;

    const double next_one = std::nextafter(1.0, 2.0);
    const auto near_umbilic = synthetic_geometry(
        1.0, 0.0, 1.0, 1.0,
        1.0, 0.0, next_one,
        next_one, 0.5 * (1.0 + next_one));
    const auto near_umbilic_values =
        surface_principal_curvatures(near_umbilic);
    passed = require(
                 near_umbilic_values &&
                     !near_umbilic_values->is_umbilic &&
                     near_umbilic_values->maximum_curvature >
                         near_umbilic_values->minimum_curvature &&
                     close_scalar(
                         near_umbilic_values->maximum_curvature,
                         next_one,
                         1.0,
                         1.0e-15,
                         1.0e-15) &&
                     close_scalar(
                         near_umbilic_values->minimum_curvature,
                         1.0,
                         1.0,
                         1.0e-15,
                         1.0e-15),
                 "near-umbilic exact-distinction semantics differ") &&
             passed;

    const double root_three = std::sqrt(3.0);
    const double expected_max = 1.0 + root_three / 3.0;
    const double expected_min = 1.0 - root_three / 3.0;
    const auto non_orthogonal = synthetic_geometry(
        2.0, 1.0, 2.0, root_three,
        3.0, 1.0, 1.0,
        2.0 / 3.0, 1.0);
    const auto non_orthogonal_values =
        surface_principal_curvatures(non_orthogonal);
    passed = require(
                 non_orthogonal_values &&
                     close_scalar(
                         non_orthogonal_values->maximum_curvature,
                         expected_max,
                         2.0) &&
                     close_scalar(
                         non_orthogonal_values->minimum_curvature,
                         expected_min,
                         2.0) &&
                     !non_orthogonal_values->is_umbilic &&
                     check_hk(
                         *non_orthogonal_values, non_orthogonal, 2.0),
                 "non-orthogonal generalized eigenproblem differs") &&
             passed;

    const auto u_reversed = synthetic_geometry(
        1.0, -0.0, 1.0, 1.0,
        -3.0, 0.0, -1.0,
        3.0, -2.0);
    const auto v_reversed = u_reversed;
    const auto both_reversed = elliptic;
    const auto u_values = surface_principal_curvatures(u_reversed);
    const auto v_values = surface_principal_curvatures(v_reversed);
    const auto both_values = surface_principal_curvatures(both_reversed);
    passed = require(
                 elliptic_values && u_values && v_values && both_values &&
                     u_values->maximum_curvature ==
                         -elliptic_values->minimum_curvature &&
                     u_values->minimum_curvature ==
                         -elliptic_values->maximum_curvature &&
                     v_values->maximum_curvature ==
                         -elliptic_values->minimum_curvature &&
                     v_values->minimum_curvature ==
                         -elliptic_values->maximum_curvature &&
                     *both_values == *elliptic_values,
                 "principal-curvature reversal law differs") &&
             passed;

    constexpr double lambda = 8.0;
    const auto scale_base = synthetic_geometry(
        1.0, 0.0, 1.0, 1.0,
        2.0, 0.0, -1.0,
        -2.0, 0.5);
    const auto scale_transformed = synthetic_geometry(
        lambda * lambda,
        0.0,
        lambda * lambda,
        lambda * lambda,
        lambda * 2.0,
        0.0,
        -lambda,
        -2.0 / (lambda * lambda),
        0.5 / lambda);
    const auto base_values = surface_principal_curvatures(scale_base);
    const auto scaled_values =
        surface_principal_curvatures(scale_transformed);
    passed = require(
                 base_values && scaled_values &&
                     close_scalar(
                         scaled_values->maximum_curvature,
                         base_values->maximum_curvature / lambda,
                         2.0) &&
                     close_scalar(
                         scaled_values->minimum_curvature,
                         base_values->minimum_curvature / lambda,
                         2.0) &&
                     scaled_values->is_umbilic ==
                         base_values->is_umbilic,
                 "principal-curvature scale covariance differs") &&
             passed;

    const double metric_extreme = std::ldexp(1.0, 600);
    const double second_extreme = std::ldexp(1.0, 300);
    const auto extreme = synthetic_geometry(
        metric_extreme,
        0.0,
        metric_extreme,
        metric_extreme,
        second_extreme,
        0.0,
        -0.5 * second_extreme,
        -0.5 * std::ldexp(1.0, -600),
        0.25 * std::ldexp(1.0, -300));
    const auto extreme_values = surface_principal_curvatures(extreme);
    passed = require(
                 extreme_values &&
                     close_scalar(
                         extreme_values->maximum_curvature,
                         std::ldexp(1.0, -300),
                         1.0) &&
                     close_scalar(
                         extreme_values->minimum_curvature,
                         -std::ldexp(1.0, -301),
                         1.0),
                 "extreme finite principal curvatures differ") &&
             passed;

    const double tiny_metric = std::ldexp(1.0, -900);
    const double huge_second = std::ldexp(1.0, 900);
    const auto unrepresentable_geometry = synthetic_geometry(
        tiny_metric,
        0.0,
        tiny_metric,
        tiny_metric,
        huge_second,
        0.0,
        huge_second,
        0.0,
        0.0);
    const auto unrepresentable =
        surface_principal_curvatures(unrepresentable_geometry);
    passed = require(
                 !unrepresentable &&
                     unrepresentable.error() ==
                         SurfaceDifferentialError::non_representable_result,
                 "unrepresentable principal curvatures did not fail") &&
             passed;

    const auto unit_domain = CurveParameterDomain::make(0.0, 1.0);
    if (!unit_domain) {
        return 1;
    }
    const BoundedPlaneSurface3 plane{
        AxisPlacement3::identity(), *unit_domain, *unit_domain};
    const auto plane_values =
        surface_principal_curvatures(plane, 0.25, 0.75);
    passed = require(
                 plane_values &&
                     plane_values->maximum_curvature == 0.0 &&
                     plane_values->minimum_curvature == 0.0 &&
                     plane_values->is_umbilic,
                 "plane principal curvatures differ") &&
             passed;

    const auto cylinder_u = CurveParameterDomain::make(-0.5, 1.0);
    const auto cylinder_v = CurveParameterDomain::make(-1.0, 2.0);
    if (!cylinder_u || !cylinder_v) {
        return 1;
    }
    constexpr double radius = 2.5;
    const auto cylinder = BoundedCylinderSurface3::make(
        AxisPlacement3::identity(), radius, *cylinder_u, *cylinder_v);
    if (!cylinder) {
        return 1;
    }

    constexpr double u = 0.4;
    constexpr double v = 0.25;
    const auto cylinder_values = surface_principal_curvatures(*cylinder, u, v);
    passed = require(
                 cylinder_values &&
                     close_scalar(
                         cylinder_values->maximum_curvature, 0.0) &&
                     close_scalar(
                         cylinder_values->minimum_curvature,
                         -1.0 / radius,
                         1.0 / radius) &&
                     !cylinder_values->is_umbilic,
                 "cylinder principal curvatures differ") &&
             passed;

    const auto mapped_u = reversed_parameter(*cylinder_u, u);
    const auto mapped_v = reversed_parameter(*cylinder_v, v);
    if (!mapped_u || !mapped_v) {
        return 1;
    }
    const auto cylinder_u_reversed = surface_principal_curvatures(
        cylinder->u_reversed(), *mapped_u, v);
    const auto cylinder_v_reversed = surface_principal_curvatures(
        cylinder->v_reversed(), u, *mapped_v);
    const auto cylinder_both_reversed = surface_principal_curvatures(
        cylinder->u_reversed().v_reversed(), *mapped_u, *mapped_v);
    passed = require(
                 cylinder_values &&
                     cylinder_u_reversed &&
                     cylinder_v_reversed &&
                     cylinder_both_reversed &&
                     close_scalar(
                         cylinder_u_reversed->maximum_curvature,
                         -cylinder_values->minimum_curvature,
                         1.0 / radius) &&
                     close_scalar(
                         cylinder_u_reversed->minimum_curvature,
                         -cylinder_values->maximum_curvature,
                         1.0 / radius) &&
                     close_scalar(
                         cylinder_v_reversed->maximum_curvature,
                         -cylinder_values->minimum_curvature,
                         1.0 / radius) &&
                     close_scalar(
                         cylinder_v_reversed->minimum_curvature,
                         -cylinder_values->maximum_curvature,
                         1.0 / radius) &&
                     close_scalar(
                         cylinder_both_reversed->maximum_curvature,
                         cylinder_values->maximum_curvature,
                         1.0 / radius) &&
                     close_scalar(
                         cylinder_both_reversed->minimum_curvature,
                         cylinder_values->minimum_curvature,
                         1.0 / radius),
                 "cylinder reversal principal-curvature law differs") &&
             passed;

    const auto translated_origin = Point3::make(8.0, -6.0, 4.0);
    const auto z_axis = Vector3::make(0.0, 0.0, 1.0);
    const auto x_axis = Vector3::make(1.0, 0.0, 0.0);
    if (!translated_origin || !z_axis || !x_axis) {
        return 1;
    }
    const auto translated_placement =
        AxisPlacement3::make(*translated_origin, *z_axis, *x_axis);
    if (!translated_placement) {
        return 1;
    }
    const auto translated_cylinder = BoundedCylinderSurface3::make(
        *translated_placement, radius, *cylinder_u, *cylinder_v);
    if (!translated_cylinder) {
        return 1;
    }
    const auto translated_values =
        surface_principal_curvatures(*translated_cylinder, u, v);
    passed = require(
                 translated_values && cylinder_values &&
                     close_scalar(
                         translated_values->maximum_curvature,
                         cylinder_values->maximum_curvature,
                         1.0 / radius) &&
                     close_scalar(
                         translated_values->minimum_curvature,
                         cylinder_values->minimum_curvature,
                         1.0 / radius) &&
                     translated_values->is_umbilic ==
                         cylinder_values->is_umbilic,
                 "translation changed principal curvatures") &&
             passed;

    const auto rotated_origin = Point3::make(0.0, 0.0, 0.0);
    const auto rotated_z = Vector3::make(0.0, 1.0, 0.0);
    const auto rotated_x = Vector3::make(0.0, 0.0, 1.0);
    if (!rotated_origin || !rotated_z || !rotated_x) {
        return 1;
    }
    const auto rotated_placement =
        AxisPlacement3::make(*rotated_origin, *rotated_z, *rotated_x);
    if (!rotated_placement) {
        return 1;
    }
    const auto rotated_cylinder = BoundedCylinderSurface3::make(
        *rotated_placement, radius, *cylinder_u, *cylinder_v);
    if (!rotated_cylinder) {
        return 1;
    }
    const auto rotated_values =
        surface_principal_curvatures(*rotated_cylinder, u, v);
    passed = require(
                 rotated_values && cylinder_values &&
                     close_scalar(
                         rotated_values->maximum_curvature,
                         cylinder_values->maximum_curvature,
                         1.0 / radius) &&
                     close_scalar(
                         rotated_values->minimum_curvature,
                         cylinder_values->minimum_curvature,
                         1.0 / radius) &&
                     rotated_values->is_umbilic ==
                         cylinder_values->is_umbilic,
                 "signed Cartesian frame transform changed principal curvatures") &&
             passed;

    const SurfaceFirstDerivatives3 synthetic_first{
        .u = vector3(1.0, 0.0, 0.0),
        .v = vector3(0.0, 1.0, 0.0),
    };
    const SurfaceSecondDerivatives3 synthetic_second{
        .uu = vector3(0.0, 0.0, 2.0),
        .uv = vector3(0.0, 0.0, 0.0),
        .vv = vector3(0.0, 0.0, -1.0),
    };
    const SyntheticSurface3 valid_surface{
        .first = synthetic_first,
        .second = synthetic_second,
    };
    const SyntheticSurface3 insufficient_surface{
        .first = synthetic_first,
        .second = synthetic_second,
        .fail_second = true,
    };
    const double nan = std::numeric_limits<double>::quiet_NaN();
    const auto non_finite =
        surface_principal_curvatures(valid_surface, nan, nan);
    const auto out_of_domain =
        surface_principal_curvatures(valid_surface, 0.5, 2.0);
    const auto insufficient =
        surface_principal_curvatures(insufficient_surface, 0.5, 0.5);
    passed = require(
                 !non_finite &&
                     non_finite.error() ==
                         SurfaceDifferentialError::non_finite_u_parameter &&
                     !out_of_domain &&
                     out_of_domain.error() ==
                         SurfaceDifferentialError::v_parameter_out_of_domain &&
                     !insufficient &&
                     insufficient.error() ==
                         SurfaceDifferentialError::insufficient_continuity,
                 "principal-curvature error propagation differs") &&
             passed;

    const auto repeat_a = surface_principal_curvatures(non_orthogonal);
    const auto repeat_b = surface_principal_curvatures(non_orthogonal);
    const auto repeat_failure_a =
        surface_principal_curvatures(insufficient_surface, 0.5, 0.5);
    const auto repeat_failure_b =
        surface_principal_curvatures(insufficient_surface, 0.5, 0.5);
    passed = require(
                 repeat_a && repeat_b && *repeat_a == *repeat_b &&
                     !repeat_failure_a && !repeat_failure_b &&
                     repeat_failure_a.error() == repeat_failure_b.error(),
                 "principal-curvature evidence is not deterministic") &&
             passed;

    return passed ? 0 : 1;
}
