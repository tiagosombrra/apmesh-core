#include "apmesh/core/numeric.hpp"
#include "apmesh/geometry/coons_surface.hpp"
#include "apmesh/geometry/elementary_surface.hpp"
#include "apmesh/geometry/extrusion_surface.hpp"
#include "apmesh/geometry/nurbs_surface.hpp"
#include "apmesh/geometry/revolution_surface.hpp"
#include "apmesh/geometry/surface.hpp"
#include "apmesh/geometry/surface_differential.hpp"
#include "apmesh/geometry/trimmed_surface.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <expected>
#include <limits>
#include <numbers>
#include <string_view>
#include <vector>

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
    const double absolute = 3.0e-12,
    const double relative = 3.0e-12) {
    const double reference =
        std::max({1.0, std::abs(lhs), std::abs(rhs), std::abs(scale)});
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

struct SyntheticSurface3 {
    apmesh::core::SurfaceFirstDerivatives3 first;

    [[nodiscard]] apmesh::core::SurfaceParameterDomain
    parameter_domain() const noexcept {
        return {
            .u = *apmesh::core::CurveParameterDomain::make(0.0, 1.0),
            .v = *apmesh::core::CurveParameterDomain::make(0.0, 1.0),
        };
    }

    [[nodiscard]] std::expected<apmesh::core::Point3, apmesh::core::SurfaceError>
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
        const auto zero = *apmesh::core::Vector3::make(0.0, 0.0, 0.0);
        return apmesh::core::SurfaceSecondDerivatives3{
            .uu = zero,
            .uv = zero,
            .vv = zero,
        };
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

template <apmesh::core::BoundedParametricSurface3 Surface>
bool regular_conditioning_conformance(
    const Surface& surface,
    const double u,
    const double v) {
    const auto result =
        apmesh::core::surface_metric_conditioning(surface, u, v);
    return result.has_value() &&
           std::isfinite(result->condition_number) &&
           result->condition_number >= 1.0;
}

apmesh::core::BicubicBezierPatch3::ControlNet planar_control_net() {
    using apmesh::core::Point3;
    return {{
        {{
            *Point3::make(0.0, 0.0, 0.0),
            *Point3::make(0.0, 1.0 / 3.0, 0.0),
            *Point3::make(0.0, 2.0 / 3.0, 0.0),
            *Point3::make(0.0, 1.0, 0.0),
        }},
        {{
            *Point3::make(1.0 / 3.0, 0.0, 0.0),
            *Point3::make(1.0 / 3.0, 1.0 / 3.0, 0.0),
            *Point3::make(1.0 / 3.0, 2.0 / 3.0, 0.0),
            *Point3::make(1.0 / 3.0, 1.0, 0.0),
        }},
        {{
            *Point3::make(2.0 / 3.0, 0.0, 0.0),
            *Point3::make(2.0 / 3.0, 1.0 / 3.0, 0.0),
            *Point3::make(2.0 / 3.0, 2.0 / 3.0, 0.0),
            *Point3::make(2.0 / 3.0, 1.0, 0.0),
        }},
        {{
            *Point3::make(1.0, 0.0, 0.0),
            *Point3::make(1.0, 1.0 / 3.0, 0.0),
            *Point3::make(1.0, 2.0 / 3.0, 0.0),
            *Point3::make(1.0, 1.0, 0.0),
        }},
    }};
}

apmesh::core::RationalBicubicBezierPatch3::WeightNet unit_weights() {
    return {{
        {{1.0, 1.0, 1.0, 1.0}},
        {{1.0, 1.0, 1.0, 1.0}},
        {{1.0, 1.0, 1.0, 1.0}},
        {{1.0, 1.0, 1.0, 1.0}},
    }};
}

} // namespace

int main() {
    using apmesh::core::AxisPlacement3;
    using apmesh::core::BoundedPlaneSurface3;
    using apmesh::core::CurveParameterDomain;
    using apmesh::core::SurfaceDifferentialError;
    using apmesh::core::SurfaceMetricConditioning;
    using apmesh::core::SurfaceMetricNormal3;
    using apmesh::core::Vector3;
    using apmesh::core::surface_metric_conditioning;

    static_assert(apmesh::core::BoundedParametricSurface3<SyntheticSurface3>);

    bool passed = true;

    const auto unit_domain = CurveParameterDomain::make(0.0, 1.0);
    const auto positive_z = Vector3::make(0.0, 0.0, 1.0);
    if (!unit_domain || !positive_z) {
        return 1;
    }

    const BoundedPlaneSurface3 plane{
        AxisPlacement3::identity(), *unit_domain, *unit_domain};
    const auto plane_condition =
        surface_metric_conditioning(plane, 0.25, 0.75);
    passed = require(
                 plane_condition &&
                     plane_condition->condition_number == 1.0,
                 "isotropic plane conditioning differs") &&
             passed;

    const double nan = std::numeric_limits<double>::quiet_NaN();
    const auto non_finite_u =
        surface_metric_conditioning(plane, nan, nan);
    const auto non_finite_v =
        surface_metric_conditioning(plane, 0.5, nan);
    const auto u_out =
        surface_metric_conditioning(plane, -0.1, 0.5);
    const auto v_out =
        surface_metric_conditioning(plane, 0.5, 1.1);
    passed = require(
                 !non_finite_u &&
                     non_finite_u.error() ==
                         SurfaceDifferentialError::non_finite_u_parameter &&
                     !non_finite_v &&
                     non_finite_v.error() ==
                         SurfaceDifferentialError::non_finite_v_parameter &&
                     !u_out &&
                     u_out.error() ==
                         SurfaceDifferentialError::u_parameter_out_of_domain &&
                     !v_out &&
                     v_out.error() ==
                         SurfaceDifferentialError::v_parameter_out_of_domain,
                 "conditioning parameter error propagation differs") &&
             passed;

    const auto diagonal_u = Vector3::make(4.0, 0.0, 0.0);
    const auto diagonal_v = Vector3::make(0.0, 1.0, 0.0);
    const auto oblique_u = Vector3::make(2.0, 0.0, 0.0);
    const auto oblique_v = Vector3::make(1.0, 1.0, 0.0);
    if (!diagonal_u || !diagonal_v || !oblique_u || !oblique_v) {
        return 1;
    }

    const SyntheticSurface3 diagonal{
        .first = {.u = *diagonal_u, .v = *diagonal_v},
    };
    const auto diagonal_condition =
        surface_metric_conditioning(diagonal, 0.5, 0.5);
    passed = require(
                 diagonal_condition &&
                     diagonal_condition->condition_number == 4.0,
                 "diagonal anisotropic conditioning differs") &&
             passed;

    const SyntheticSurface3 oblique{
        .first = {.u = *oblique_u, .v = *oblique_v},
    };
    const auto oblique_condition =
        surface_metric_conditioning(oblique, 0.5, 0.5);
    const double expected_oblique =
        0.5 * (3.0 + std::sqrt(5.0));
    passed = require(
                 oblique_condition &&
                     close_scalar(
                         oblique_condition->condition_number,
                         expected_oblique,
                         expected_oblique),
                 "non-orthogonal analytic conditioning differs") &&
             passed;

    const SyntheticSurface3 swapped{
        .first = {.u = *oblique_v, .v = *oblique_u},
    };
    const SyntheticSurface3 u_reversed{
        .first = {.u = -*oblique_u, .v = *oblique_v},
    };
    const SyntheticSurface3 v_reversed{
        .first = {.u = *oblique_u, .v = -*oblique_v},
    };
    const SyntheticSurface3 both_reversed{
        .first = {.u = -*oblique_u, .v = -*oblique_v},
    };
    const auto swapped_condition =
        surface_metric_conditioning(swapped, 0.5, 0.5);
    const auto u_reversed_condition =
        surface_metric_conditioning(u_reversed, 0.5, 0.5);
    const auto v_reversed_condition =
        surface_metric_conditioning(v_reversed, 0.5, 0.5);
    const auto both_reversed_condition =
        surface_metric_conditioning(both_reversed, 0.5, 0.5);
    passed = require(
                 oblique_condition && swapped_condition &&
                     u_reversed_condition && v_reversed_condition &&
                     both_reversed_condition &&
                     close_scalar(
                         swapped_condition->condition_number,
                         oblique_condition->condition_number) &&
                     close_scalar(
                         u_reversed_condition->condition_number,
                         oblique_condition->condition_number) &&
                     close_scalar(
                         v_reversed_condition->condition_number,
                         oblique_condition->condition_number) &&
                     close_scalar(
                         both_reversed_condition->condition_number,
                         oblique_condition->condition_number),
                 "axis-swap/reversal conditioning invariance differs") &&
             passed;

    const auto scaled_oblique_u = *oblique_u * 8.0;
    const auto scaled_oblique_v = *oblique_v * 8.0;
    if (!scaled_oblique_u || !scaled_oblique_v) {
        return 1;
    }
    const SyntheticSurface3 uniformly_scaled{
        .first = {.u = *scaled_oblique_u, .v = *scaled_oblique_v},
    };
    const auto uniformly_scaled_condition =
        surface_metric_conditioning(uniformly_scaled, 0.5, 0.5);
    passed = require(
                 oblique_condition && uniformly_scaled_condition &&
                     close_scalar(
                         uniformly_scaled_condition->condition_number,
                         oblique_condition->condition_number),
                 "uniform spatial scale changed conditioning") &&
             passed;

    const auto unit_u = Vector3::make(1.0, 0.0, 0.0);
    const auto unit_v = Vector3::make(0.0, 1.0, 0.0);
    const auto stretched_u = Vector3::make(4.0, 0.0, 0.0);
    if (!unit_u || !unit_v || !stretched_u) {
        return 1;
    }
    const SyntheticSurface3 unit_parameterization{
        .first = {.u = *unit_u, .v = *unit_v},
    };
    const SyntheticSurface3 nonuniform_parameterization{
        .first = {.u = *stretched_u, .v = *unit_v},
    };
    const auto unit_parameter_condition =
        surface_metric_conditioning(unit_parameterization, 0.5, 0.5);
    const auto nonuniform_parameter_condition =
        surface_metric_conditioning(
            nonuniform_parameterization, 0.5, 0.5);
    passed = require(
                 unit_parameter_condition &&
                     nonuniform_parameter_condition &&
                     unit_parameter_condition->condition_number == 1.0 &&
                     nonuniform_parameter_condition->condition_number == 4.0,
                 "non-uniform parameter scaling did not change conditioning") &&
             passed;

    const auto parallel = Vector3::make(2.0, 0.0, 0.0);
    if (!parallel) {
        return 1;
    }
    const SyntheticSurface3 singular{
        .first = {.u = *unit_u, .v = *parallel},
    };
    const auto singular_condition =
        surface_metric_conditioning(singular, 0.5, 0.5);
    passed = require(
                 !singular_condition &&
                     singular_condition.error() ==
                         SurfaceDifferentialError::singular_parameterization,
                 "exact singular conditioning was not rejected") &&
             passed;

    const double tiny = std::ldexp(1.0, -500);
    const auto near_v = Vector3::make(1.0, tiny, 0.0);
    if (!near_v) {
        return 1;
    }
    const SyntheticSurface3 near_singular{
        .first = {.u = *unit_u, .v = *near_v},
    };
    const auto near_condition =
        surface_metric_conditioning(near_singular, 0.5, 0.5);
    const double expected_near = std::ldexp(1.0, 501);
    passed = require(
                 near_condition &&
                     near_condition->condition_number == expected_near,
                 "near-singular regular conditioning differs") &&
             passed;

    const double huge = std::ldexp(1.0, 450);
    const double small = std::ldexp(1.0, -450);
    const auto extreme_u = Vector3::make(huge, 0.0, 0.0);
    const auto extreme_v = Vector3::make(0.0, small, 0.0);
    if (!extreme_u || !extreme_v) {
        return 1;
    }
    const SyntheticSurface3 extreme{
        .first = {.u = *extreme_u, .v = *extreme_v},
    };
    const auto extreme_condition =
        surface_metric_conditioning(extreme, 0.5, 0.5);
    const double expected_extreme = std::ldexp(1.0, 900);
    passed = require(
                 extreme_condition &&
                     extreme_condition->condition_number == expected_extreme,
                 "extreme finite conditioning differs") &&
             passed;

    const SurfaceMetricNormal3 nonrepresentable_metric{
        .first_fundamental_form = {
            .e = 1.0,
            .f = 1.0,
            .g = 1.0,
        },
        .area_density = std::numeric_limits<double>::denorm_min(),
        .unit_normal = *positive_z,
    };
    const auto nonrepresentable =
        surface_metric_conditioning(nonrepresentable_metric);
    passed = require(
                 !nonrepresentable &&
                     nonrepresentable.error() ==
                         SurfaceDifferentialError::non_representable_result,
                 "unrepresentable conditioning did not fail explicitly") &&
             passed;

    const auto translated_origin =
        apmesh::core::Point3::make(8.0, -6.0, 4.0);
    const auto reflected_axis = Vector3::make(0.0, 0.0, -1.0);
    const auto x_reference = Vector3::make(1.0, 0.0, 0.0);
    if (!translated_origin || !reflected_axis || !x_reference) {
        return 1;
    }
    const auto translated_placement = AxisPlacement3::make(
        *translated_origin,
        *positive_z,
        *x_reference);
    const auto reflected_placement = AxisPlacement3::make(
        *apmesh::core::Point3::make(0.0, 0.0, 0.0),
        *reflected_axis,
        *x_reference);
    if (!translated_placement || !reflected_placement) {
        return 1;
    }
    const BoundedPlaneSurface3 translated_plane{
        *translated_placement, *unit_domain, *unit_domain};
    const BoundedPlaneSurface3 reflected_plane{
        *reflected_placement, *unit_domain, *unit_domain};
    const auto translated_condition =
        surface_metric_conditioning(translated_plane, 0.25, 0.75);
    const auto reflected_condition =
        surface_metric_conditioning(reflected_plane, 0.25, 0.75);
    passed = require(
                 translated_condition && reflected_condition &&
                     translated_condition->condition_number == 1.0 &&
                     reflected_condition->condition_number == 1.0,
                 "translation/signed-frame conditioning invariance differs") &&
             passed;

    const auto cylinder_u = CurveParameterDomain::make(-0.5, 1.0);
    const auto cylinder_v = CurveParameterDomain::make(-1.0, 2.0);
    if (!cylinder_u || !cylinder_v) {
        return 1;
    }
    constexpr double cylinder_radius = 2.5;
    const auto cylinder = apmesh::core::BoundedCylinderSurface3::make(
        AxisPlacement3::identity(),
        cylinder_radius,
        *cylinder_u,
        *cylinder_v);
    if (!cylinder) {
        return 1;
    }
    constexpr double cylinder_parameter = 0.4;
    const auto cylinder_condition =
        surface_metric_conditioning(
            *cylinder, cylinder_parameter, 0.25);
    passed = require(
                 cylinder_condition &&
                     close_scalar(
                         cylinder_condition->condition_number,
                         cylinder_radius,
                         cylinder_radius),
                 "cylinder analytic conditioning differs") &&
             passed;

    const auto cylinder_u_reversed = cylinder->u_reversed();
    const auto cylinder_v_reversed = cylinder->v_reversed();
    const auto mapped_cylinder_u =
        apmesh::core::reversed_parameter(
            *cylinder_u, cylinder_parameter);
    const auto mapped_cylinder_v =
        apmesh::core::reversed_parameter(
            *cylinder_v, 0.25);
    if (!mapped_cylinder_u || !mapped_cylinder_v) {
        return 1;
    }
    const auto cylinder_u_condition =
        surface_metric_conditioning(
            cylinder_u_reversed, *mapped_cylinder_u, 0.25);
    const auto cylinder_v_condition =
        surface_metric_conditioning(
            cylinder_v_reversed, cylinder_parameter, *mapped_cylinder_v);
    passed = require(
                 cylinder_condition &&
                     cylinder_u_condition &&
                     cylinder_v_condition &&
                     close_scalar(
                         cylinder_u_condition->condition_number,
                         cylinder_condition->condition_number) &&
                     close_scalar(
                         cylinder_v_condition->condition_number,
                         cylinder_condition->condition_number),
                 "cylinder reversal conditioning invariance differs") &&
             passed;

    const double half_pi =
        0.5 * std::numbers::pi_v<double>;
    const auto sphere_u = CurveParameterDomain::make(-0.5, 0.5);
    const auto sphere_v = CurveParameterDomain::make(-half_pi, half_pi);
    if (!sphere_u || !sphere_v) {
        return 1;
    }
    constexpr double sphere_radius = 2.0;
    const auto sphere = apmesh::core::BoundedSphereSurface3::make(
        AxisPlacement3::identity(),
        sphere_radius,
        *sphere_u,
        *sphere_v);
    if (!sphere) {
        return 1;
    }
    constexpr double sphere_latitude = 0.3;
    const auto sphere_condition =
        surface_metric_conditioning(*sphere, 0.2, sphere_latitude);
    const double expected_sphere =
        1.0 / std::cos(sphere_latitude);
    const auto sphere_equator =
        surface_metric_conditioning(*sphere, 0.2, 0.0);
    const auto sphere_pole =
        surface_metric_conditioning(*sphere, 0.0, half_pi);
    passed = require(
                 sphere_condition &&
                     close_scalar(
                         sphere_condition->condition_number,
                         expected_sphere,
                         expected_sphere) &&
                     sphere_equator &&
                     sphere_equator->condition_number == 1.0 &&
                     !sphere_pole &&
                     sphere_pole.error() ==
                         SurfaceDifferentialError::singular_parameterization,
                 "sphere regular/polar conditioning differs") &&
             passed;

    const auto net = planar_control_net();
    const apmesh::core::BicubicBezierPatch3 bezier{net};
    const auto rational =
        apmesh::core::RationalBicubicBezierPatch3::make(
            net, unit_weights());
    if (!rational) {
        return 1;
    }

    std::vector<apmesh::core::Point3> flat_controls;
    flat_controls.reserve(16U);
    for (const auto& row : net) {
        flat_controls.insert(
            flat_controls.end(), row.begin(), row.end());
    }
    const auto nurbs = apmesh::core::BicubicNURBSSurface3::make(
        flat_controls,
        std::vector<double>(16U, 1.0),
        4U,
        4U,
        {},
        {},
        0.0,
        1.0,
        0.0,
        1.0);
    if (!nurbs) {
        return 1;
    }

    const apmesh::core::CubicBezier3 bottom{
        net[0][0], net[1][0], net[2][0], net[3][0]};
    const apmesh::core::CubicBezier3 top{
        net[0][3], net[1][3], net[2][3], net[3][3]};
    const apmesh::core::CubicBezier3 left{
        net[0][0], net[0][1], net[0][2], net[0][3]};
    const apmesh::core::CubicBezier3 right{
        net[3][0], net[3][1], net[3][2], net[3][3]};
    const auto coons = apmesh::core::CubicBezierCoonsPatch3::make(
        bottom, top, left, right);
    if (!coons) {
        return 1;
    }

    const auto trimmed =
        apmesh::core::RectangularTrimmedSurface3<
            apmesh::core::BicubicBezierPatch3>::make(
                bezier, 0.1, 0.9, 0.2, 0.8);
    if (!trimmed) {
        return 1;
    }

    const auto extrusion_displacement =
        Vector3::make(0.0, 1.0, 0.0);
    if (!extrusion_displacement) {
        return 1;
    }
    const auto extrusion =
        apmesh::core::CubicBezierLinearExtrusionSurface3::make(
            bottom, *extrusion_displacement);
    if (!extrusion) {
        return 1;
    }

    const apmesh::core::CubicBezier3 generatrix{
        *apmesh::core::Point3::make(2.0, 0.0, 0.0),
        *apmesh::core::Point3::make(2.0, 0.0, 1.0 / 3.0),
        *apmesh::core::Point3::make(2.0, 0.0, 2.0 / 3.0),
        *apmesh::core::Point3::make(2.0, 0.0, 1.0)};
    const auto revolution =
        apmesh::core::CubicBezierRevolutionSurface3::make(
            generatrix, AxisPlacement3::identity(), 1.0);
    if (!revolution) {
        return 1;
    }

    passed = require(
                 regular_conditioning_conformance(bezier, 0.4, 0.6) &&
                     regular_conditioning_conformance(
                         *rational, 0.4, 0.6) &&
                     regular_conditioning_conformance(
                         *nurbs, 0.4, 0.6) &&
                     regular_conditioning_conformance(
                         *coons, 0.4, 0.6) &&
                     regular_conditioning_conformance(
                         *trimmed, 0.4, 0.5) &&
                     regular_conditioning_conformance(
                         *extrusion, 0.4, 0.6) &&
                     regular_conditioning_conformance(
                         *revolution, 0.4, 0.5) &&
                     regular_conditioning_conformance(
                         plane, 0.4, 0.6) &&
                     regular_conditioning_conformance(
                         *cylinder, 0.4, 0.6) &&
                     regular_conditioning_conformance(
                         *sphere, 0.2, 0.3),
                 "integrated surface-family conditioning conformance differs") &&
             passed;

    const auto repeat_success_a =
        surface_metric_conditioning(oblique, 0.5, 0.5);
    const auto repeat_success_b =
        surface_metric_conditioning(oblique, 0.5, 0.5);
    const auto repeat_failure_a =
        surface_metric_conditioning(singular, 0.5, 0.5);
    const auto repeat_failure_b =
        surface_metric_conditioning(singular, 0.5, 0.5);
    passed = require(
                 repeat_success_a && repeat_success_b &&
                     *repeat_success_a == *repeat_success_b &&
                     !repeat_failure_a && !repeat_failure_b &&
                     repeat_failure_a.error() ==
                         repeat_failure_b.error(),
                 "conditioning results are not deterministic") &&
             passed;

    return passed ? 0 : 1;
}
