#include "apmesh/core/numeric.hpp"
#include "apmesh/geometry/elementary_surface.hpp"
#include "apmesh/geometry/line_segment.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <cstdio>
#include <limits>
#include <string_view>

namespace {

using apmesh::core::AxisPlacement3;
using apmesh::core::BoundedParametricSurface3;
using apmesh::core::BoundedPlaneSurface3;
using apmesh::core::CurveParameterDomain;
using apmesh::core::LineSegment3;
using apmesh::core::Point3;
using apmesh::core::ProximityPolicy;
using apmesh::core::ProximityResult;
using apmesh::core::SurfaceError;
using apmesh::core::Vector3;
using apmesh::core::reversed_parameter;

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
    const double absolute = 3.0e-13,
    const double relative = 3.0e-13) {
    const double reference = std::max(1.0, std::abs(scale));
    const auto comparison = apmesh::core::compare_proximity(
        lhs,
        rhs,
        ProximityPolicy{
            .absolute_tolerance = absolute * reference,
            .relative_tolerance = relative,
            .reference_scale = reference,
        });
    return comparison.has_value() &&
           comparison->result == ProximityResult::within;
}

bool close_point(
    const Point3& lhs,
    const Point3& rhs,
    const double scale = 1.0) {
    return close_scalar(lhs.x(), rhs.x(), scale) &&
           close_scalar(lhs.y(), rhs.y(), scale) &&
           close_scalar(lhs.z(), rhs.z(), scale);
}

bool close_vector(
    const Vector3& lhs,
    const Vector3& rhs,
    const double scale = 1.0) {
    return close_scalar(lhs.x(), rhs.x(), scale) &&
           close_scalar(lhs.y(), rhs.y(), scale) &&
           close_scalar(lhs.z(), rhs.z(), scale);
}

Point3 analytic_point(
    const AxisPlacement3& placement,
    const double u,
    const double v) {
    const long double x =
        static_cast<long double>(placement.origin().x()) +
        static_cast<long double>(u) *
            static_cast<long double>(placement.x_direction().x()) +
        static_cast<long double>(v) *
            static_cast<long double>(placement.y_direction().x());
    const long double y =
        static_cast<long double>(placement.origin().y()) +
        static_cast<long double>(u) *
            static_cast<long double>(placement.x_direction().y()) +
        static_cast<long double>(v) *
            static_cast<long double>(placement.y_direction().y());
    const long double z =
        static_cast<long double>(placement.origin().z()) +
        static_cast<long double>(u) *
            static_cast<long double>(placement.x_direction().z()) +
        static_cast<long double>(v) *
            static_cast<long double>(placement.y_direction().z());

    return Point3::make(
               static_cast<double>(x),
               static_cast<double>(y),
               static_cast<double>(z))
        .value();
}

double unit_parameter(
    const double parameter,
    const CurveParameterDomain& domain) {
    return (parameter - domain.lower()) /
           (domain.upper() - domain.lower());
}

bool check_boundary(
    const BoundedPlaneSurface3& surface,
    const AxisPlacement3& placement,
    const bool vary_u,
    const double fixed_parameter,
    const CurveParameterDomain& varying_domain) {
    const Point3 source =
        vary_u
            ? analytic_point(
                  placement, varying_domain.lower(), fixed_parameter)
            : analytic_point(
                  placement, fixed_parameter, varying_domain.lower());
    const Point3 target =
        vary_u
            ? analytic_point(
                  placement, varying_domain.upper(), fixed_parameter)
            : analytic_point(
                  placement, fixed_parameter, varying_domain.upper());

    const LineSegment3 line{source, target};
    constexpr std::array<double, 5> fractions{
        0.0, 0.125, 0.5, 0.875, 1.0};

    for (const double fraction : fractions) {
        const double parameter = std::lerp(
            varying_domain.lower(),
            varying_domain.upper(),
            fraction);
        const auto surface_value =
            vary_u
                ? surface.evaluate(parameter, fixed_parameter)
                : surface.evaluate(fixed_parameter, parameter);
        const auto line_value = line.evaluate(fraction);
        if (!surface_value || !line_value ||
            !close_point(*surface_value, *line_value, 256.0)) {
            return false;
        }

        const double mapped =
            unit_parameter(parameter, varying_domain);
        if (!close_scalar(mapped, fraction, 1.0, 1.0e-14, 1.0e-14)) {
            return false;
        }
    }
    return true;
}

} // namespace

int main() {
    static_assert(BoundedParametricSurface3<BoundedPlaneSurface3>);

    bool passed = true;

    const auto origin = Point3::make(7.0, -11.0, 5.0);
    const auto main_direction = Vector3::make(1.0, 2.0, 3.0);
    const auto x_reference = Vector3::make(4.0, -2.0, 1.0);
    const auto u_domain = CurveParameterDomain::make(-2.5, 4.25);
    const auto v_domain = CurveParameterDomain::make(-3.75, 1.5);
    if (!origin || !main_direction || !x_reference ||
        !u_domain || !v_domain) {
        return 1;
    }

    const auto placement =
        AxisPlacement3::make(*origin, *main_direction, *x_reference);
    if (!placement) {
        return 1;
    }

    const BoundedPlaneSurface3 surface{
        *placement, *u_domain, *v_domain};

    passed = require(
                 surface.axis_placement() == *placement &&
                     surface.u_domain() == *u_domain &&
                     surface.v_domain() == *v_domain &&
                     !surface.u_is_reversed() &&
                     !surface.v_is_reversed() &&
                     surface.parameter_domain().u == *u_domain &&
                     surface.parameter_domain().v == *v_domain,
                 "bounded plane stored representation/domain differs") &&
             passed;

    const double nan = std::numeric_limits<double>::quiet_NaN();
    const auto u_nan = surface.evaluate(nan, nan);
    const auto v_nan = surface.evaluate(0.0, nan);
    const auto v_nan_before_u_domain =
        surface.evaluate(u_domain->lower() - 1.0, nan);
    const auto u_low =
        surface.first_derivatives(u_domain->lower() - 1.0, 0.0);
    const auto v_high =
        surface.second_derivatives(0.0, v_domain->upper() + 1.0);

    passed = require(
                 !u_nan &&
                     u_nan.error() == SurfaceError::non_finite_u_parameter &&
                     !v_nan &&
                     v_nan.error() == SurfaceError::non_finite_v_parameter &&
                     !v_nan_before_u_domain &&
                     v_nan_before_u_domain.error() ==
                         SurfaceError::non_finite_v_parameter &&
                     !u_low &&
                     u_low.error() ==
                         SurfaceError::u_parameter_out_of_domain &&
                     !v_high &&
                     v_high.error() ==
                         SurfaceError::v_parameter_out_of_domain,
                 "bounded plane parameter validation order differs") &&
             passed;

    constexpr std::array<std::array<double, 2>, 7> samples{{
        {-2.5, -3.75},
        {-2.5, 1.5},
        {4.25, -3.75},
        {4.25, 1.5},
        {-1.0, -2.0},
        {0.75, 0.25},
        {3.0, 1.0},
    }};

    const auto zero = Vector3::make(0.0, 0.0, 0.0);
    if (!zero) {
        return 1;
    }

    for (const auto& sample : samples) {
        const double u = sample[0];
        const double v = sample[1];
        const Point3 reference =
            analytic_point(*placement, u, v);
        const auto value = surface.evaluate(u, v);
        const auto first = surface.first_derivatives(u, v);
        const auto second = surface.second_derivatives(u, v);

        passed = require(
                     value &&
                         close_point(*value, reference, 256.0) &&
                         first &&
                         first->u == placement->x_direction() &&
                         first->v == placement->y_direction() &&
                         second &&
                         second->uu == *zero &&
                         second->uv == *zero &&
                         second->vv == *zero,
                     "bounded plane analytic oracle/partials differ") &&
                 passed;
    }

    passed = require(
                 check_boundary(
                     surface,
                     *placement,
                     false,
                     u_domain->lower(),
                     *v_domain) &&
                     check_boundary(
                         surface,
                         *placement,
                         false,
                         u_domain->upper(),
                         *v_domain) &&
                     check_boundary(
                         surface,
                         *placement,
                         true,
                         v_domain->lower(),
                         *u_domain) &&
                     check_boundary(
                         surface,
                         *placement,
                         true,
                         v_domain->upper(),
                         *u_domain),
                 "bounded plane boundary LineSegment3 parity differs") &&
             passed;

    const auto u_reversed = surface.u_reversed();
    const auto v_reversed = surface.v_reversed();
    const auto both_reversed = u_reversed.v_reversed();

    passed = require(
                 u_reversed.u_is_reversed() &&
                     !u_reversed.v_is_reversed() &&
                     !v_reversed.u_is_reversed() &&
                     v_reversed.v_is_reversed() &&
                     both_reversed.u_is_reversed() &&
                     both_reversed.v_is_reversed() &&
                     u_reversed.u_reversed() == surface &&
                     v_reversed.v_reversed() == surface &&
                     both_reversed.u_reversed().v_reversed() == surface,
                 "bounded plane reversal storage/involution differs") &&
             passed;

    constexpr double reversal_u = 0.75;
    constexpr double reversal_v = -1.25;
    const auto mapped_u =
        reversed_parameter(*u_domain, reversal_u);
    const auto mapped_v =
        reversed_parameter(*v_domain, reversal_v);
    if (!mapped_u || !mapped_v) {
        return 1;
    }

    const auto base_value =
        surface.evaluate(reversal_u, reversal_v);
    const auto u_reverse_value =
        u_reversed.evaluate(*mapped_u, reversal_v);
    const auto v_reverse_value =
        v_reversed.evaluate(reversal_u, *mapped_v);
    const auto both_reverse_value =
        both_reversed.evaluate(*mapped_u, *mapped_v);
    const auto base_first =
        surface.first_derivatives(reversal_u, reversal_v);
    const auto u_reverse_first =
        u_reversed.first_derivatives(*mapped_u, reversal_v);
    const auto v_reverse_first =
        v_reversed.first_derivatives(reversal_u, *mapped_v);
    const auto both_reverse_first =
        both_reversed.first_derivatives(*mapped_u, *mapped_v);

    passed = require(
                 base_value && u_reverse_value && v_reverse_value &&
                     both_reverse_value &&
                     close_point(*base_value, *u_reverse_value, 256.0) &&
                     close_point(*base_value, *v_reverse_value, 256.0) &&
                     close_point(*base_value, *both_reverse_value, 256.0) &&
                     base_first && u_reverse_first &&
                     v_reverse_first && both_reverse_first &&
                     u_reverse_first->u == -base_first->u &&
                     u_reverse_first->v == base_first->v &&
                     v_reverse_first->u == base_first->u &&
                     v_reverse_first->v == -base_first->v &&
                     both_reverse_first->u == -base_first->u &&
                     both_reverse_first->v == -base_first->v,
                 "bounded plane reversal covariance differs") &&
             passed;

    const auto base_orientation =
        apmesh::core::cross(base_first->u, base_first->v);
    const auto u_orientation =
        apmesh::core::cross(
            u_reverse_first->u, u_reverse_first->v);
    const auto v_orientation =
        apmesh::core::cross(
            v_reverse_first->u, v_reverse_first->v);
    const auto both_orientation =
        apmesh::core::cross(
            both_reverse_first->u, both_reverse_first->v);

    passed = require(
                 base_orientation && u_orientation &&
                     v_orientation && both_orientation &&
                     close_vector(
                         *base_orientation,
                         placement->z_direction()) &&
                     close_vector(
                         *u_orientation,
                         -placement->z_direction()) &&
                     close_vector(
                         *v_orientation,
                         -placement->z_direction()) &&
                     close_vector(
                         *both_orientation,
                         placement->z_direction()),
                 "bounded plane reversal orientation differs") &&
             passed;

    const auto translated_origin =
        Point3::make(
            origin->x() + 8.0,
            origin->y() - 6.0,
            origin->z() + 4.0);
    if (!translated_origin) {
        return 1;
    }
    const auto translated_placement =
        AxisPlacement3::make(
            *translated_origin, *main_direction, *x_reference);
    if (!translated_placement) {
        return 1;
    }

    const BoundedPlaneSurface3 translated{
        *translated_placement, *u_domain, *v_domain};
    const auto translated_value =
        translated.evaluate(reversal_u, reversal_v);
    const auto translated_first =
        translated.first_derivatives(reversal_u, reversal_v);

    passed = require(
                 base_value && translated_value &&
                     close_scalar(
                         translated_value->x() - base_value->x(),
                         8.0,
                         16.0) &&
                     close_scalar(
                         translated_value->y() - base_value->y(),
                         -6.0,
                         16.0) &&
                     close_scalar(
                         translated_value->z() - base_value->z(),
                         4.0,
                         16.0) &&
                     base_first && translated_first &&
                     close_vector(base_first->u, translated_first->u) &&
                     close_vector(base_first->v, translated_first->v),
                 "bounded plane translation covariance differs") &&
             passed;

    const auto scale_origin = Point3::make(1.0, -2.0, 3.0);
    const auto scaled_origin = Point3::make(2.0, -4.0, 6.0);
    const auto z_axis = Vector3::make(0.0, 0.0, 1.0);
    const auto x_axis = Vector3::make(1.0, 0.0, 0.0);
    const auto scale_u = CurveParameterDomain::make(-2.0, 4.0);
    const auto scale_v = CurveParameterDomain::make(-1.0, 3.0);
    const auto scaled_u = CurveParameterDomain::make(-4.0, 8.0);
    const auto scaled_v = CurveParameterDomain::make(-2.0, 6.0);
    if (!scale_origin || !scaled_origin || !z_axis || !x_axis ||
        !scale_u || !scale_v || !scaled_u || !scaled_v) {
        return 1;
    }

    const auto scale_placement =
        AxisPlacement3::make(*scale_origin, *z_axis, *x_axis);
    const auto doubled_placement =
        AxisPlacement3::make(*scaled_origin, *z_axis, *x_axis);
    if (!scale_placement || !doubled_placement) {
        return 1;
    }

    const BoundedPlaneSurface3 scale_surface{
        *scale_placement, *scale_u, *scale_v};
    const BoundedPlaneSurface3 doubled_surface{
        *doubled_placement, *scaled_u, *scaled_v};

    const auto scale_value = scale_surface.evaluate(1.5, 0.5);
    const auto doubled_value = doubled_surface.evaluate(3.0, 1.0);
    const auto scale_first =
        scale_surface.first_derivatives(1.5, 0.5);
    const auto doubled_first =
        doubled_surface.first_derivatives(3.0, 1.0);

    passed = require(
                 scale_value && doubled_value &&
                     close_scalar(
                         doubled_value->x(), 2.0 * scale_value->x()) &&
                     close_scalar(
                         doubled_value->y(), 2.0 * scale_value->y()) &&
                     close_scalar(
                         doubled_value->z(), 2.0 * scale_value->z()) &&
                     scale_first && doubled_first &&
                     scale_first->u == doubled_first->u &&
                     scale_first->v == doubled_first->v,
                 "bounded plane power-of-two coordinate scaling differs") &&
             passed;

    const double maximum = std::numeric_limits<double>::max();
    const auto extreme_origin =
        Point3::make(maximum / 4.0, 0.0, 0.0);
    const auto extreme_u =
        CurveParameterDomain::make(0.0, maximum / 8.0);
    const auto extreme_v =
        CurveParameterDomain::make(-1.0, 1.0);
    if (!extreme_origin || !extreme_u || !extreme_v) {
        return 1;
    }

    const auto extreme_placement =
        AxisPlacement3::make(*extreme_origin, *z_axis, *x_axis);
    if (!extreme_placement) {
        return 1;
    }
    const BoundedPlaneSurface3 extreme_surface{
        *extreme_placement, *extreme_u, *extreme_v};
    const auto extreme_value =
        extreme_surface.evaluate(maximum / 8.0, 0.0);

    passed = require(
                 extreme_value &&
                     std::isfinite(extreme_value->x()) &&
                     std::isfinite(extreme_value->y()) &&
                     std::isfinite(extreme_value->z()),
                 "bounded plane representable extreme query failed") &&
             passed;

    const auto overflowing_origin =
        Point3::make(maximum / 2.0, 0.0, 0.0);
    const auto overflowing_u =
        CurveParameterDomain::make(0.0, maximum);
    if (!overflowing_origin || !overflowing_u) {
        return 1;
    }
    const auto overflowing_placement =
        AxisPlacement3::make(
            *overflowing_origin, *z_axis, *x_axis);
    if (!overflowing_placement) {
        return 1;
    }

    const BoundedPlaneSurface3 overflowing_surface{
        *overflowing_placement, *overflowing_u, *extreme_v};
    const auto overflowing_value =
        overflowing_surface.evaluate(maximum, 0.0);

    passed = require(
                 !overflowing_value &&
                     overflowing_value.error() ==
                         SurfaceError::non_finite_result,
                 "bounded plane unrepresentable query did not fail explicitly") &&
             passed;

    const auto repeat_value_a =
        surface.evaluate(reversal_u, reversal_v);
    const auto repeat_value_b =
        surface.evaluate(reversal_u, reversal_v);
    const auto repeat_first_a =
        surface.first_derivatives(reversal_u, reversal_v);
    const auto repeat_first_b =
        surface.first_derivatives(reversal_u, reversal_v);
    const auto repeat_second_a =
        surface.second_derivatives(reversal_u, reversal_v);
    const auto repeat_second_b =
        surface.second_derivatives(reversal_u, reversal_v);
    const auto repeat_failure_a =
        surface.evaluate(u_domain->upper() + 1.0, 0.0);
    const auto repeat_failure_b =
        surface.evaluate(u_domain->upper() + 1.0, 0.0);

    passed = require(
                 repeat_value_a && repeat_value_b &&
                     *repeat_value_a == *repeat_value_b &&
                     repeat_first_a && repeat_first_b &&
                     *repeat_first_a == *repeat_first_b &&
                     repeat_second_a && repeat_second_b &&
                     *repeat_second_a == *repeat_second_b &&
                     !repeat_failure_a && !repeat_failure_b &&
                     repeat_failure_a.error() ==
                         repeat_failure_b.error(),
                 "bounded plane repeated evidence is not deterministic") &&
             passed;

    return passed ? 0 : 1;
}
