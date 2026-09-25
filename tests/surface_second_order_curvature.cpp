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
#include <cstdint>
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
    const double absolute = 4.0e-12,
    const double relative = 4.0e-12) {
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

std::vector<apmesh::core::Point3> planar_nurbs_points(
    const std::size_t u_count,
    const std::size_t v_count) {
    std::vector<apmesh::core::Point3> points;
    points.reserve(u_count * v_count);
    for (std::size_t i = 0; i < u_count; ++i) {
        for (std::size_t j = 0; j < v_count; ++j) {
            points.push_back(*apmesh::core::Point3::make(
                static_cast<double>(i),
                static_cast<double>(j),
                0.0));
        }
    }
    return points;
}

template <apmesh::core::BoundedParametricSurface3 Surface>
bool regular_second_order_conformance(
    const Surface& surface,
    const double u,
    const double v) {
    const auto result =
        apmesh::core::surface_second_order_geometry(surface, u, v);
    return result.has_value() &&
           std::isfinite(result->second_fundamental_form.l) &&
           std::isfinite(result->second_fundamental_form.m) &&
           std::isfinite(result->second_fundamental_form.n) &&
           std::isfinite(result->gaussian_curvature) &&
           std::isfinite(result->mean_curvature);
}

} // namespace

int main() {
    using apmesh::core::AxisPlacement3;
    using apmesh::core::BicubicBezierPatch3;
    using apmesh::core::BicubicNURBSSurface3;
    using apmesh::core::BoundedCylinderSurface3;
    using apmesh::core::BoundedPlaneSurface3;
    using apmesh::core::CubicBezier3;
    using apmesh::core::CubicBezierCoonsPatch3;
    using apmesh::core::CubicBezierLinearExtrusionSurface3;
    using apmesh::core::CubicBezierRevolutionSurface3;
    using apmesh::core::CurveParameterDomain;
    using apmesh::core::Point3;
    using apmesh::core::RationalBicubicBezierPatch3;
    using apmesh::core::SurfaceDifferentialError;
    using apmesh::core::SurfaceFirstDerivatives3;
    using apmesh::core::SurfaceSecondDerivatives3;
    using apmesh::core::Vector3;
    using apmesh::core::reversed_parameter;
    using apmesh::core::surface_second_order_geometry;

    static_assert(
        apmesh::core::BoundedParametricSurface3<SyntheticSurface3>);

    bool passed = true;

    const SurfaceFirstDerivatives3 general_first{
        .u = vector3(2.0, 0.0, 0.0),
        .v = vector3(1.0, 3.0, 0.0),
    };
    const SurfaceSecondDerivatives3 general_second{
        .uu = vector3(0.0, 0.0, 4.0),
        .uv = vector3(0.0, 0.0, -2.0),
        .vv = vector3(0.0, 0.0, 6.0),
    };

    const auto direct =
        surface_second_order_geometry(general_first, general_second);
    passed = require(
                 direct &&
                     close_scalar(
                         direct->metric_normal.first_fundamental_form.e, 4.0) &&
                     close_scalar(
                         direct->metric_normal.first_fundamental_form.f, 2.0) &&
                     close_scalar(
                         direct->metric_normal.first_fundamental_form.g, 10.0) &&
                     close_scalar(direct->metric_normal.area_density, 6.0) &&
                     close_scalar(
                         direct->second_fundamental_form.l, 4.0) &&
                     close_scalar(
                         direct->second_fundamental_form.m, -2.0) &&
                     close_scalar(
                         direct->second_fundamental_form.n, 6.0) &&
                     close_scalar(
                         direct->gaussian_curvature, 5.0 / 9.0) &&
                     close_scalar(direct->mean_curvature, 1.0),
                 "general second fundamental form / K / H differs") &&
             passed;

    const SurfaceFirstDerivatives3 saddle_first{
        .u = vector3(1.0, 0.0, 0.0),
        .v = vector3(0.0, 1.0, 0.0),
    };
    const SurfaceSecondDerivatives3 saddle_second{
        .uu = vector3(0.0, 0.0, 2.0),
        .uv = vector3(0.0, 0.0, 0.0),
        .vv = vector3(0.0, 0.0, -2.0),
    };
    const auto saddle =
        surface_second_order_geometry(saddle_first, saddle_second);
    passed = require(
                 saddle &&
                     close_scalar(
                         saddle->second_fundamental_form.l, 2.0) &&
                     close_scalar(
                         saddle->second_fundamental_form.m, 0.0) &&
                     close_scalar(
                         saddle->second_fundamental_form.n, -2.0) &&
                     close_scalar(saddle->gaussian_curvature, -4.0) &&
                     close_scalar(saddle->mean_curvature, 0.0),
                 "hyperbolic-paraboloid local curvature fixture differs") &&
             passed;

    const SyntheticSurface3 general_surface{
        .first = general_first,
        .second = general_second,
    };
    const auto templated =
        surface_second_order_geometry(general_surface, 0.25, 0.75);
    passed = require(
                 templated && direct && *templated == *direct,
                 "surface template and direct derivative APIs differ") &&
             passed;

    const double nan = std::numeric_limits<double>::quiet_NaN();
    const auto non_finite =
        surface_second_order_geometry(general_surface, nan, nan);
    const auto out_of_domain =
        surface_second_order_geometry(general_surface, 0.5, 1.1);
    const SyntheticSurface3 insufficient_surface{
        .first = general_first,
        .second = general_second,
        .fail_second = true,
    };
    const auto insufficient =
        surface_second_order_geometry(insufficient_surface, 0.5, 0.5);
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
                 "second-order error propagation differs") &&
             passed;

    const SurfaceFirstDerivatives3 singular_first{
        .u = vector3(1.0, 0.0, 0.0),
        .v = vector3(2.0, 0.0, 0.0),
    };
    const auto singular =
        surface_second_order_geometry(singular_first, general_second);
    passed = require(
                 !singular &&
                     singular.error() ==
                         SurfaceDifferentialError::singular_parameterization,
                 "exact singular parameterization was not rejected") &&
             passed;

    const double near_tiny = std::ldexp(1.0, -500);
    const SurfaceFirstDerivatives3 near_first{
        .u = vector3(1.0, 0.0, 0.0),
        .v = vector3(1.0, near_tiny, 0.0),
    };
    const SurfaceSecondDerivatives3 zero_second{
        .uu = vector3(0.0, 0.0, 0.0),
        .uv = vector3(0.0, 0.0, 0.0),
        .vv = vector3(0.0, 0.0, 0.0),
    };
    const auto near_regular =
        surface_second_order_geometry(near_first, zero_second);
    passed = require(
                 near_regular &&
                     near_regular->gaussian_curvature == 0.0 &&
                     near_regular->mean_curvature == 0.0,
                 "near-singular regular parameterization was rejected") &&
             passed;

    const double overflow_tiny = std::ldexp(1.0, -600);
    const SurfaceFirstDerivatives3 overflow_first{
        .u = vector3(1.0, 0.0, 0.0),
        .v = vector3(1.0, overflow_tiny, 0.0),
    };
    const SurfaceSecondDerivatives3 overflow_second{
        .uu = vector3(0.0, 0.0, 1.0),
        .uv = vector3(0.0, 0.0, 0.0),
        .vv = vector3(0.0, 0.0, 1.0),
    };
    const auto unrepresentable =
        surface_second_order_geometry(overflow_first, overflow_second);
    passed = require(
                 !unrepresentable &&
                     unrepresentable.error() ==
                         SurfaceDifferentialError::non_representable_result,
                 "unrepresentable curvature did not fail explicitly") &&
             passed;

    const SurfaceFirstDerivatives3 scaled_first{
        .u = vector3(4.0, 0.0, 0.0),
        .v = vector3(2.0, 6.0, 0.0),
    };
    const SurfaceSecondDerivatives3 scaled_second{
        .uu = vector3(0.0, 0.0, 8.0),
        .uv = vector3(0.0, 0.0, -4.0),
        .vv = vector3(0.0, 0.0, 12.0),
    };
    const auto scaled =
        surface_second_order_geometry(scaled_first, scaled_second);
    passed = require(
                 direct && scaled &&
                     close_scalar(
                         scaled->second_fundamental_form.l,
                         2.0 * direct->second_fundamental_form.l) &&
                     close_scalar(
                         scaled->second_fundamental_form.m,
                         2.0 * direct->second_fundamental_form.m) &&
                     close_scalar(
                         scaled->second_fundamental_form.n,
                         2.0 * direct->second_fundamental_form.n) &&
                     close_scalar(
                         scaled->gaussian_curvature,
                         direct->gaussian_curvature / 4.0) &&
                     close_scalar(
                         scaled->mean_curvature,
                         direct->mean_curvature / 2.0),
                 "second-order power-of-two scale covariance differs") &&
             passed;

    const auto unit_domain = CurveParameterDomain::make(0.0, 1.0);
    if (!unit_domain) {
        return 1;
    }

    const BoundedPlaneSurface3 plane{
        AxisPlacement3::identity(), *unit_domain, *unit_domain};
    const auto plane_geometry =
        surface_second_order_geometry(plane, 0.25, 0.75);
    passed = require(
                 plane_geometry &&
                     plane_geometry->second_fundamental_form.l == 0.0 &&
                     plane_geometry->second_fundamental_form.m == 0.0 &&
                     plane_geometry->second_fundamental_form.n == 0.0 &&
                     plane_geometry->gaussian_curvature == 0.0 &&
                     plane_geometry->mean_curvature == 0.0,
                 "plane second-order geometry differs") &&
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
    const BoundedPlaneSurface3 translated_plane{
        *translated_placement, *unit_domain, *unit_domain};
    const auto translated_plane_geometry =
        surface_second_order_geometry(translated_plane, 0.25, 0.75);
    passed = require(
                 plane_geometry && translated_plane_geometry &&
                     *plane_geometry == *translated_plane_geometry,
                 "translation changed plane second-order geometry") &&
             passed;

    const auto cylinder_u = CurveParameterDomain::make(-0.5, 1.0);
    const auto cylinder_v = CurveParameterDomain::make(-1.0, 2.0);
    if (!cylinder_u || !cylinder_v) {
        return 1;
    }
    constexpr double radius = 2.5;
    const auto cylinder = BoundedCylinderSurface3::make(
        AxisPlacement3::identity(),
        radius,
        *cylinder_u,
        *cylinder_v);
    if (!cylinder) {
        return 1;
    }

    constexpr double cylinder_u_value = 0.4;
    constexpr double cylinder_v_value = 0.25;
    const auto cylinder_geometry = surface_second_order_geometry(
        *cylinder, cylinder_u_value, cylinder_v_value);
    passed = require(
                 cylinder_geometry &&
                     close_scalar(
                         cylinder_geometry->second_fundamental_form.l,
                         -radius,
                         radius) &&
                     close_scalar(
                         cylinder_geometry->second_fundamental_form.m,
                         0.0) &&
                     close_scalar(
                         cylinder_geometry->second_fundamental_form.n,
                         0.0) &&
                     close_scalar(
                         cylinder_geometry->gaussian_curvature,
                         0.0) &&
                     close_scalar(
                         cylinder_geometry->mean_curvature,
                         -1.0 / (2.0 * radius)),
                 "cylinder analytic K/H sign convention differs") &&
             passed;

    const auto mapped_u =
        reversed_parameter(*cylinder_u, cylinder_u_value);
    const auto mapped_v =
        reversed_parameter(*cylinder_v, cylinder_v_value);
    if (!mapped_u || !mapped_v) {
        return 1;
    }
    const auto u_reversed_geometry = surface_second_order_geometry(
        cylinder->u_reversed(), *mapped_u, cylinder_v_value);
    const auto v_reversed_geometry = surface_second_order_geometry(
        cylinder->v_reversed(), cylinder_u_value, *mapped_v);
    const auto both_reversed_geometry = surface_second_order_geometry(
        cylinder->u_reversed().v_reversed(), *mapped_u, *mapped_v);

    passed = require(
                 cylinder_geometry &&
                     u_reversed_geometry &&
                     v_reversed_geometry &&
                     both_reversed_geometry &&
                     close_scalar(
                         u_reversed_geometry->gaussian_curvature,
                         cylinder_geometry->gaussian_curvature) &&
                     close_scalar(
                         v_reversed_geometry->gaussian_curvature,
                         cylinder_geometry->gaussian_curvature) &&
                     close_scalar(
                         both_reversed_geometry->gaussian_curvature,
                         cylinder_geometry->gaussian_curvature) &&
                     close_scalar(
                         u_reversed_geometry->mean_curvature,
                         -cylinder_geometry->mean_curvature) &&
                     close_scalar(
                         v_reversed_geometry->mean_curvature,
                         -cylinder_geometry->mean_curvature) &&
                     close_scalar(
                         both_reversed_geometry->mean_curvature,
                         cylinder_geometry->mean_curvature) &&
                     close_scalar(
                         u_reversed_geometry->second_fundamental_form.l,
                         -cylinder_geometry->second_fundamental_form.l) &&
                     close_scalar(
                         v_reversed_geometry->second_fundamental_form.l,
                         -cylinder_geometry->second_fundamental_form.l) &&
                     close_scalar(
                         both_reversed_geometry->second_fundamental_form.l,
                         cylinder_geometry->second_fundamental_form.l),
                 "second-order reversal covariance differs") &&
             passed;

    constexpr std::size_t c1_u_count = 7U;
    constexpr std::size_t c1_v_count = 6U;
    const std::vector<double> c1_u_knots{0.0, 1.5};
    const std::vector<double> c1_v_knots{-0.5, 1.0};
    const std::vector<std::uint8_t> c1_u_mult{2U, 1U};
    const std::vector<std::uint8_t> c1_v_mult{1U, 1U};
    const auto c1_points =
        planar_nurbs_points(c1_u_count, c1_v_count);
    const std::vector<double> c1_weights(
        c1_u_count * c1_v_count, 1.0);
    const auto c1_surface = BicubicNURBSSurface3::make(
        c1_points,
        c1_weights,
        c1_u_count,
        c1_v_count,
        c1_u_knots,
        c1_v_knots,
        c1_u_mult,
        c1_v_mult,
        -2.0,
        3.0,
        -1.5,
        2.5);
    if (!c1_surface) {
        return 1;
    }

    const auto c1_first_metric =
        apmesh::core::surface_metric_normal(*c1_surface, 0.0, 0.25);
    const auto c1_curvature =
        surface_second_order_geometry(*c1_surface, 0.0, 0.25);
    const auto c1_off_knot =
        surface_second_order_geometry(*c1_surface, 0.25, 0.25);
    passed = require(
                 c1_first_metric &&
                     !c1_curvature &&
                     c1_curvature.error() ==
                         SurfaceDifferentialError::insufficient_continuity &&
                     c1_off_knot,
                 "C1 NURBS curvature continuity semantics differ") &&
             passed;

    const auto net = planar_control_net();
    const BicubicBezierPatch3 bezier{net};
    const auto rational =
        RationalBicubicBezierPatch3::make(net, unit_weights());
    if (!rational) {
        return 1;
    }

    std::vector<Point3> flat_controls;
    flat_controls.reserve(16U);
    for (const auto& row : net) {
        flat_controls.insert(
            flat_controls.end(), row.begin(), row.end());
    }
    const auto nurbs = BicubicNURBSSurface3::make(
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

    const CubicBezier3 bottom{
        net[0][0], net[1][0], net[2][0], net[3][0]};
    const CubicBezier3 top{
        net[0][3], net[1][3], net[2][3], net[3][3]};
    const CubicBezier3 left{
        net[0][0], net[0][1], net[0][2], net[0][3]};
    const CubicBezier3 right{
        net[3][0], net[3][1], net[3][2], net[3][3]};
    const auto coons =
        CubicBezierCoonsPatch3::make(bottom, top, left, right);
    if (!coons) {
        return 1;
    }

    const auto trimmed =
        apmesh::core::RectangularTrimmedSurface3<
            BicubicBezierPatch3>::make(
                bezier, 0.1, 0.9, 0.2, 0.8);
    if (!trimmed) {
        return 1;
    }

    const auto extrusion_direction = Vector3::make(0.0, 0.0, 2.0);
    if (!extrusion_direction) {
        return 1;
    }
    const auto extrusion =
        CubicBezierLinearExtrusionSurface3::make(
            bottom, *extrusion_direction);
    if (!extrusion) {
        return 1;
    }

    const auto revolution_p0 = Point3::make(2.0, 0.0, -1.0);
    const auto revolution_p1 = Point3::make(2.0, 0.0, -1.0 / 3.0);
    const auto revolution_p2 = Point3::make(2.0, 0.0, 1.0 / 3.0);
    const auto revolution_p3 = Point3::make(2.0, 0.0, 1.0);
    if (!revolution_p0 || !revolution_p1 ||
        !revolution_p2 || !revolution_p3) {
        return 1;
    }
    const CubicBezier3 generatrix{
        *revolution_p0,
        *revolution_p1,
        *revolution_p2,
        *revolution_p3,
    };
    const auto revolution =
        CubicBezierRevolutionSurface3::make(
            generatrix, AxisPlacement3::identity(), 0.75);
    if (!revolution) {
        return 1;
    }

    passed = require(
                 regular_second_order_conformance(bezier, 0.3, 0.6) &&
                     regular_second_order_conformance(*rational, 0.3, 0.6) &&
                     regular_second_order_conformance(*nurbs, 0.3, 0.6) &&
                     regular_second_order_conformance(*coons, 0.3, 0.6) &&
                     regular_second_order_conformance(*trimmed, 0.4, 0.5) &&
                     regular_second_order_conformance(*extrusion, 0.3, 0.6) &&
                     regular_second_order_conformance(*revolution, 0.3, 0.6),
                 "cross-family second-order conformance differs") &&
             passed;

    const auto repeat_a =
        surface_second_order_geometry(*cylinder, 0.4, 0.25);
    const auto repeat_b =
        surface_second_order_geometry(*cylinder, 0.4, 0.25);
    const auto failure_a =
        surface_second_order_geometry(*c1_surface, 0.0, 0.25);
    const auto failure_b =
        surface_second_order_geometry(*c1_surface, 0.0, 0.25);
    passed = require(
                 repeat_a && repeat_b && *repeat_a == *repeat_b &&
                     !failure_a && !failure_b &&
                     failure_a.error() == failure_b.error(),
                 "second-order curvature evidence is not deterministic") &&
             passed;

    return passed ? 0 : 1;
}
