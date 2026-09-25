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
#include <limits>
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
    const double absolute = 2.0e-12,
    const double relative = 2.0e-12) {
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

bool close_vector(
    const apmesh::core::Vector3& lhs,
    const apmesh::core::Vector3& rhs,
    const double scale = 1.0) {
    return close_scalar(lhs.x(), rhs.x(), scale) &&
           close_scalar(lhs.y(), rhs.y(), scale) &&
           close_scalar(lhs.z(), rhs.z(), scale);
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
bool regular_conformance(
    const Surface& surface,
    const double u,
    const double v) {
    const auto result = apmesh::core::surface_metric_normal(surface, u, v);
    return result.has_value() &&
           std::isfinite(result->first_fundamental_form.e) &&
           std::isfinite(result->first_fundamental_form.f) &&
           std::isfinite(result->first_fundamental_form.g) &&
           std::isfinite(result->area_density) &&
           result->area_density > 0.0;
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
    using apmesh::core::Vector3;
    using apmesh::core::surface_metric_normal;

    static_assert(apmesh::core::BoundedParametricSurface3<SyntheticSurface3>);

    bool passed = true;

    const auto unit_domain = CurveParameterDomain::make(0.0, 1.0);
    if (!unit_domain) {
        return 1;
    }

    const BoundedPlaneSurface3 plane{
        AxisPlacement3::identity(), *unit_domain, *unit_domain};
    const auto plane_metric = surface_metric_normal(plane, 0.25, 0.75);
    const auto positive_z = Vector3::make(0.0, 0.0, 1.0);
    if (!positive_z) {
        return 1;
    }

    passed = require(
                 plane_metric &&
                     plane_metric->first_fundamental_form.e == 1.0 &&
                     plane_metric->first_fundamental_form.f == 0.0 &&
                     plane_metric->first_fundamental_form.g == 1.0 &&
                     plane_metric->area_density == 1.0 &&
                     plane_metric->unit_normal == *positive_z,
                 "plane first fundamental form/normal differs") &&
             passed;

    const double nan = std::numeric_limits<double>::quiet_NaN();
    const auto non_finite_u = surface_metric_normal(plane, nan, nan);
    const auto non_finite_v = surface_metric_normal(plane, 0.5, nan);
    const auto u_out = surface_metric_normal(plane, -0.1, 0.5);
    const auto v_out = surface_metric_normal(plane, 0.5, 1.1);
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
                 "surface differential error propagation differs") &&
             passed;

    const auto u = Vector3::make(2.0, 0.0, 0.0);
    const auto v = Vector3::make(1.0, 3.0, 0.0);
    if (!u || !v) {
        return 1;
    }
    const SyntheticSurface3 oblique{
        .first = {.u = *u, .v = *v},
    };
    const auto oblique_metric = surface_metric_normal(oblique, 0.5, 0.5);
    passed = require(
                 oblique_metric &&
                     close_scalar(
                         oblique_metric->first_fundamental_form.e, 4.0) &&
                     close_scalar(
                         oblique_metric->first_fundamental_form.f, 2.0) &&
                     close_scalar(
                         oblique_metric->first_fundamental_form.g, 10.0) &&
                     close_scalar(oblique_metric->area_density, 6.0) &&
                     close_vector(
                         oblique_metric->unit_normal, *positive_z),
                 "oblique first fundamental form differs") &&
             passed;

    const SyntheticSurface3 u_reversed{
        .first = {.u = -*u, .v = *v},
    };
    const SyntheticSurface3 v_reversed{
        .first = {.u = *u, .v = -*v},
    };
    const SyntheticSurface3 both_reversed{
        .first = {.u = -*u, .v = -*v},
    };
    const auto u_metric = surface_metric_normal(u_reversed, 0.5, 0.5);
    const auto v_metric = surface_metric_normal(v_reversed, 0.5, 0.5);
    const auto both_metric =
        surface_metric_normal(both_reversed, 0.5, 0.5);
    passed = require(
                 u_metric && v_metric && both_metric &&
                     close_scalar(
                         u_metric->first_fundamental_form.e, 4.0) &&
                     close_scalar(
                         u_metric->first_fundamental_form.f, -2.0) &&
                     close_scalar(
                         u_metric->first_fundamental_form.g, 10.0) &&
                     close_scalar(u_metric->area_density, 6.0) &&
                     close_vector(
                         u_metric->unit_normal, -*positive_z) &&
                     close_scalar(
                         v_metric->first_fundamental_form.f, -2.0) &&
                     close_vector(
                         v_metric->unit_normal, -*positive_z) &&
                     close_scalar(
                         both_metric->first_fundamental_form.f, 2.0) &&
                     close_vector(
                         both_metric->unit_normal, *positive_z),
                 "surface differential reversal covariance differs") &&
             passed;

    const auto parallel = Vector3::make(4.0, 0.0, 0.0);
    if (!parallel) {
        return 1;
    }
    const SyntheticSurface3 singular{
        .first = {.u = *u, .v = *parallel},
    };
    const auto singular_metric =
        surface_metric_normal(singular, 0.5, 0.5);
    passed = require(
                 !singular_metric &&
                     singular_metric.error() ==
                         SurfaceDifferentialError::singular_parameterization,
                 "exact singular parameterization was not rejected") &&
             passed;

    const double tiny = std::ldexp(1.0, -500);
    const auto near_v = Vector3::make(1.0, tiny, 0.0);
    const auto near_u = Vector3::make(1.0, 0.0, 0.0);
    if (!near_u || !near_v) {
        return 1;
    }
    const SyntheticSurface3 near_degenerate{
        .first = {.u = *near_u, .v = *near_v},
    };
    const auto near_metric =
        surface_metric_normal(near_degenerate, 0.5, 0.5);
    passed = require(
                 near_metric &&
                     near_metric->area_density == tiny &&
                     close_vector(
                         near_metric->unit_normal, *positive_z),
                 "near-degenerate regular surface was rejected") &&
             passed;

    const auto scaled_u = *u * 2.0;
    const auto scaled_v = *v * 2.0;
    if (!scaled_u || !scaled_v) {
        return 1;
    }
    const SyntheticSurface3 scaled{
        .first = {.u = *scaled_u, .v = *scaled_v},
    };
    const auto scaled_metric = surface_metric_normal(scaled, 0.5, 0.5);
    passed = require(
                 oblique_metric && scaled_metric &&
                     close_scalar(
                         scaled_metric->first_fundamental_form.e,
                         4.0 * oblique_metric->first_fundamental_form.e) &&
                     close_scalar(
                         scaled_metric->first_fundamental_form.f,
                         4.0 * oblique_metric->first_fundamental_form.f) &&
                     close_scalar(
                         scaled_metric->first_fundamental_form.g,
                         4.0 * oblique_metric->first_fundamental_form.g) &&
                     close_scalar(
                         scaled_metric->area_density,
                         4.0 * oblique_metric->area_density) &&
                     close_vector(
                         scaled_metric->unit_normal,
                         oblique_metric->unit_normal),
                 "surface differential power-of-two scale covariance differs") &&
             passed;

    const auto cylinder_u = CurveParameterDomain::make(-0.5, 1.0);
    const auto cylinder_v = CurveParameterDomain::make(-1.0, 2.0);
    if (!cylinder_u || !cylinder_v) {
        return 1;
    }
    constexpr double radius = 2.5;
    const auto cylinder = apmesh::core::BoundedCylinderSurface3::make(
        AxisPlacement3::identity(),
        radius,
        *cylinder_u,
        *cylinder_v);
    if (!cylinder) {
        return 1;
    }
    constexpr double cylinder_parameter = 0.4;
    const auto cylinder_metric =
        surface_metric_normal(*cylinder, cylinder_parameter, 0.25);
    const auto cylinder_normal = Vector3::make(
        std::cos(cylinder_parameter),
        std::sin(cylinder_parameter),
        0.0);
    if (!cylinder_normal) {
        return 1;
    }
    passed = require(
                 cylinder_metric &&
                     close_scalar(
                         cylinder_metric->first_fundamental_form.e,
                         radius * radius,
                         radius * radius) &&
                     close_scalar(
                         cylinder_metric->first_fundamental_form.f,
                         0.0) &&
                     close_scalar(
                         cylinder_metric->first_fundamental_form.g,
                         1.0) &&
                     close_scalar(
                         cylinder_metric->area_density,
                         radius,
                         radius) &&
                     close_vector(
                         cylinder_metric->unit_normal,
                         *cylinder_normal,
                         16.0),
                 "cylinder analytic metric/normal differs") &&
             passed;

    const auto reflected_axis = Vector3::make(0.0, 0.0, -1.0);
    const auto x_reference = Vector3::make(1.0, 0.0, 0.0);
    const auto origin = apmesh::core::Point3::make(0.0, 0.0, 0.0);
    if (!reflected_axis || !x_reference || !origin) {
        return 1;
    }
    const auto reflected_placement = AxisPlacement3::make(
        *origin, *reflected_axis, *x_reference);
    if (!reflected_placement) {
        return 1;
    }
    const BoundedPlaneSurface3 reflected_plane{
        *reflected_placement, *unit_domain, *unit_domain};
    const auto reflected_metric =
        surface_metric_normal(reflected_plane, 0.25, 0.75);
    passed = require(
                 reflected_metric &&
                     reflected_metric->first_fundamental_form ==
                         plane_metric->first_fundamental_form &&
                     reflected_metric->area_density ==
                         plane_metric->area_density &&
                     close_vector(
                         reflected_metric->unit_normal,
                         reflected_placement->z_direction()),
                 "signed-frame metric/normal covariance differs") &&
             passed;

    const auto translated_origin =
        apmesh::core::Point3::make(8.0, -6.0, 4.0);
    const auto positive_axis = Vector3::make(0.0, 0.0, 1.0);
    if (!translated_origin || !positive_axis) {
        return 1;
    }
    const auto translated_placement = AxisPlacement3::make(
        *translated_origin, *positive_axis, *x_reference);
    if (!translated_placement) {
        return 1;
    }
    const BoundedPlaneSurface3 translated_plane{
        *translated_placement, *unit_domain, *unit_domain};
    const auto translated_metric =
        surface_metric_normal(translated_plane, 0.25, 0.75);
    passed = require(
                 translated_metric && plane_metric &&
                     *translated_metric == *plane_metric,
                 "translation changed surface differential properties") &&
             passed;

    const double huge = std::ldexp(1.0, 600);
    const auto huge_u = Vector3::make(huge, 0.0, 0.0);
    const auto huge_v = Vector3::make(0.0, huge, 0.0);
    if (!huge_u || !huge_v) {
        return 1;
    }
    const SyntheticSurface3 unrepresentable{
        .first = {.u = *huge_u, .v = *huge_v},
    };
    const auto unrepresentable_metric =
        surface_metric_normal(unrepresentable, 0.5, 0.5);
    passed = require(
                 !unrepresentable_metric &&
                     unrepresentable_metric.error() ==
                         SurfaceDifferentialError::non_representable_result,
                 "unrepresentable metric did not fail explicitly") &&
             passed;

    return passed ? 0 : 1;
}
