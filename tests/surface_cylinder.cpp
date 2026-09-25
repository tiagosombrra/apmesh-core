#include "apmesh/core/numeric.hpp"
#include "apmesh/geometry/elementary_surface.hpp"
#include "apmesh/geometry/line_segment.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <cstdio>
#include <limits>
#include <numbers>
#include <string_view>

namespace {

using apmesh::core::AxisPlacement3;
using apmesh::core::BoundedCylinderSurface3;
using apmesh::core::BoundedParametricSurface3;
using apmesh::core::CurveParameterDomain;
using apmesh::core::CylinderSurfaceConstructionError;
using apmesh::core::LineSegment3;
using apmesh::core::Point3;
using apmesh::core::ProximityPolicy;
using apmesh::core::ProximityResult;
using apmesh::core::SurfaceError;
using apmesh::core::SurfaceFirstDerivatives3;
using apmesh::core::SurfaceSecondDerivatives3;
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
    const double absolute = 8.0e-13,
    const double relative = 8.0e-13) {
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

Point3 reference_point(
    const AxisPlacement3& placement,
    const double radius,
    const double u,
    const double v) {
    const long double angle = static_cast<long double>(u);
    const long double radial = static_cast<long double>(radius);
    const long double cosine = std::cos(angle);
    const long double sine = std::sin(angle);

    const long double x =
        static_cast<long double>(placement.origin().x()) +
        radial * cosine *
            static_cast<long double>(placement.x_direction().x()) +
        radial * sine *
            static_cast<long double>(placement.y_direction().x()) +
        static_cast<long double>(v) *
            static_cast<long double>(placement.z_direction().x());
    const long double y =
        static_cast<long double>(placement.origin().y()) +
        radial * cosine *
            static_cast<long double>(placement.x_direction().y()) +
        radial * sine *
            static_cast<long double>(placement.y_direction().y()) +
        static_cast<long double>(v) *
            static_cast<long double>(placement.z_direction().y());
    const long double z =
        static_cast<long double>(placement.origin().z()) +
        radial * cosine *
            static_cast<long double>(placement.x_direction().z()) +
        radial * sine *
            static_cast<long double>(placement.y_direction().z()) +
        static_cast<long double>(v) *
            static_cast<long double>(placement.z_direction().z());

    return Point3::make(
               static_cast<double>(x),
               static_cast<double>(y),
               static_cast<double>(z))
        .value();
}

Vector3 reference_u(
    const AxisPlacement3& placement,
    const double radius,
    const double u) {
    const long double angle = static_cast<long double>(u);
    const long double radial = static_cast<long double>(radius);
    const long double cosine = std::cos(angle);
    const long double sine = std::sin(angle);

    return Vector3::make(
               static_cast<double>(
                   -radial * sine *
                       static_cast<long double>(
                           placement.x_direction().x()) +
                   radial * cosine *
                       static_cast<long double>(
                           placement.y_direction().x())),
               static_cast<double>(
                   -radial * sine *
                       static_cast<long double>(
                           placement.x_direction().y()) +
                   radial * cosine *
                       static_cast<long double>(
                           placement.y_direction().y())),
               static_cast<double>(
                   -radial * sine *
                       static_cast<long double>(
                           placement.x_direction().z()) +
                   radial * cosine *
                       static_cast<long double>(
                           placement.y_direction().z())))
        .value();
}

Vector3 reference_v(const AxisPlacement3& placement) {
    return placement.z_direction();
}

Vector3 reference_uu(
    const AxisPlacement3& placement,
    const double radius,
    const double u) {
    const long double angle = static_cast<long double>(u);
    const long double radial = static_cast<long double>(radius);
    const long double cosine = std::cos(angle);
    const long double sine = std::sin(angle);

    return Vector3::make(
               static_cast<double>(
                   -radial * cosine *
                       static_cast<long double>(
                           placement.x_direction().x()) -
                   radial * sine *
                       static_cast<long double>(
                           placement.y_direction().x())),
               static_cast<double>(
                   -radial * cosine *
                       static_cast<long double>(
                           placement.x_direction().y()) -
                   radial * sine *
                       static_cast<long double>(
                           placement.y_direction().y())),
               static_cast<double>(
                   -radial * cosine *
                       static_cast<long double>(
                           placement.x_direction().z()) -
                   radial * sine *
                       static_cast<long double>(
                           placement.y_direction().z())))
        .value();
}

bool check_axial_boundary(
    const BoundedCylinderSurface3& surface,
    const double fixed_u,
    const CurveParameterDomain& v_domain) {
    const auto source =
        surface.evaluate(fixed_u, v_domain.lower());
    const auto target =
        surface.evaluate(fixed_u, v_domain.upper());
    if (!source || !target) {
        return false;
    }

    const LineSegment3 line{*source, *target};
    const auto line_derivative = line.first_derivative(0.5);
    if (!line_derivative) {
        return false;
    }

    constexpr std::array<double, 5> fractions{
        0.0, 0.125, 0.5, 0.875, 1.0};
    const double v_width = v_domain.upper() - v_domain.lower();

    for (const double fraction : fractions) {
        const double v = std::lerp(
            v_domain.lower(), v_domain.upper(), fraction);
        const auto value = surface.evaluate(fixed_u, v);
        const auto line_value = line.evaluate(fraction);
        const auto first = surface.first_derivatives(fixed_u, v);
        if (!value || !line_value || !first ||
            !close_point(*value, *line_value, 512.0) ||
            !close_scalar(
                line_derivative->x(),
                v_width * first->v.x(),
                512.0) ||
            !close_scalar(
                line_derivative->y(),
                v_width * first->v.y(),
                512.0) ||
            !close_scalar(
                line_derivative->z(),
                v_width * first->v.z(),
                512.0)) {
            return false;
        }
    }
    return true;
}

bool check_reference(
    const BoundedCylinderSurface3& surface,
    const AxisPlacement3& placement,
    const double radius,
    const double u,
    const double v) {
    const auto value = surface.evaluate(u, v);
    const auto first = surface.first_derivatives(u, v);
    const auto second = surface.second_derivatives(u, v);
    const auto zero = Vector3::make(0.0, 0.0, 0.0);
    if (!value || !first || !second || !zero) {
        return false;
    }

    return close_point(
               *value,
               reference_point(placement, radius, u, v),
               1024.0) &&
           close_vector(
               first->u,
               reference_u(placement, radius, u),
               1024.0) &&
           close_vector(
               first->v,
               reference_v(placement),
               1024.0) &&
           close_vector(
               second->uu,
               reference_uu(placement, radius, u),
               4096.0) &&
           second->uv == *zero &&
           second->vv == *zero;
}

} // namespace

int main() {
    static_assert(BoundedParametricSurface3<BoundedCylinderSurface3>);

    bool passed = true;

    const auto origin = Point3::make(7.0, -11.0, 5.0);
    const auto axis = Vector3::make(1.0, 2.0, 3.0);
    const auto x_reference = Vector3::make(4.0, -2.0, 1.0);
    const auto u_domain = CurveParameterDomain::make(-0.7, 2.4);
    const auto v_domain = CurveParameterDomain::make(-3.75, 1.5);
    if (!origin || !axis || !x_reference || !u_domain || !v_domain) {
        return 1;
    }

    const auto placement =
        AxisPlacement3::make(*origin, *axis, *x_reference);
    if (!placement) {
        return 1;
    }

    constexpr double radius = 2.75;
    const auto cylinder = BoundedCylinderSurface3::make(
        *placement, radius, *u_domain, *v_domain);
    if (!cylinder) {
        return 1;
    }

    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double infinity = std::numeric_limits<double>::infinity();
    const double full_revolution =
        2.0 * std::numbers::pi_v<double>;
    const auto full_domain =
        CurveParameterDomain::make(0.0, full_revolution);
    const auto wide_domain =
        CurveParameterDomain::make(0.0, full_revolution + 0.25);
    if (!full_domain || !wide_domain) {
        return 1;
    }

    const auto nan_radius = BoundedCylinderSurface3::make(
        *placement, nan, *u_domain, *v_domain);
    const auto infinite_radius = BoundedCylinderSurface3::make(
        *placement, infinity, *u_domain, *v_domain);
    const auto zero_radius = BoundedCylinderSurface3::make(
        *placement, 0.0, *u_domain, *v_domain);
    const auto negative_radius = BoundedCylinderSurface3::make(
        *placement, -1.0, *u_domain, *v_domain);
    const auto full = BoundedCylinderSurface3::make(
        *placement, radius, *full_domain, *v_domain);
    const auto wide = BoundedCylinderSurface3::make(
        *placement, radius, *wide_domain, *v_domain);

    passed = require(
                 !nan_radius &&
                     nan_radius.error() ==
                         CylinderSurfaceConstructionError::
                             non_finite_radius &&
                     !infinite_radius &&
                     infinite_radius.error() ==
                         CylinderSurfaceConstructionError::
                             non_finite_radius &&
                     !zero_radius &&
                     zero_radius.error() ==
                         CylinderSurfaceConstructionError::
                             non_positive_radius &&
                     !negative_radius &&
                     negative_radius.error() ==
                         CylinderSurfaceConstructionError::
                             non_positive_radius &&
                     !full &&
                     full.error() ==
                         CylinderSurfaceConstructionError::
                             full_or_multiple_revolution_not_admitted &&
                     !wide &&
                     wide.error() ==
                         CylinderSurfaceConstructionError::
                             full_or_multiple_revolution_not_admitted,
                 "bounded cylinder construction validation differs") &&
             passed;

    passed = require(
                 cylinder->axis_placement() == *placement &&
                     cylinder->radius() == radius &&
                     cylinder->u_domain() == *u_domain &&
                     cylinder->v_domain() == *v_domain &&
                     cylinder->parameter_domain().u == *u_domain &&
                     cylinder->parameter_domain().v == *v_domain &&
                     !cylinder->u_is_reversed() &&
                     !cylinder->v_is_reversed(),
                 "bounded cylinder stored representation differs") &&
             passed;

    const auto u_nan = cylinder->evaluate(nan, nan);
    const auto v_nan = cylinder->evaluate(0.0, nan);
    const auto v_nan_before_u_domain =
        cylinder->evaluate(u_domain->lower() - 1.0, nan);
    const auto u_low = cylinder->first_derivatives(
        u_domain->lower() - 1.0, 0.0);
    const auto v_high = cylinder->second_derivatives(
        0.0, v_domain->upper() + 1.0);

    passed = require(
                 !u_nan &&
                     u_nan.error() ==
                         SurfaceError::non_finite_u_parameter &&
                     !v_nan &&
                     v_nan.error() ==
                         SurfaceError::non_finite_v_parameter &&
                     !v_nan_before_u_domain &&
                     v_nan_before_u_domain.error() ==
                         SurfaceError::non_finite_v_parameter &&
                     !u_low &&
                     u_low.error() ==
                         SurfaceError::u_parameter_out_of_domain &&
                     !v_high &&
                     v_high.error() ==
                         SurfaceError::v_parameter_out_of_domain,
                 "bounded cylinder parameter validation order differs") &&
             passed;

    constexpr std::array<std::array<double, 2>, 8> samples{{
        {-0.7, -3.75},
        {-0.7, 1.5},
        {2.4, -3.75},
        {2.4, 1.5},
        {-0.25, -2.0},
        {0.5, 0.25},
        {1.25, -1.0},
        {2.0, 1.0},
    }};

    for (const auto& sample : samples) {
        passed = require(
                     check_reference(
                         *cylinder,
                         *placement,
                         radius,
                         sample[0],
                         sample[1]),
                     "bounded cylinder independent analytic oracle differs") &&
                 passed;
    }

    passed = require(
                 check_axial_boundary(
                     *cylinder, u_domain->lower(), *v_domain) &&
                     check_axial_boundary(
                         *cylinder, u_domain->upper(), *v_domain),
                 "bounded cylinder axial LineSegment3 parity differs") &&
             passed;

    constexpr std::array<double, 5> circular_parameters{
        -0.7, -0.1, 0.5, 1.3, 2.4};
    for (const double u : circular_parameters) {
        const auto lower_value =
            cylinder->evaluate(u, v_domain->lower());
        const auto upper_value =
            cylinder->evaluate(u, v_domain->upper());
        passed = require(
                     lower_value && upper_value &&
                         close_point(
                             *lower_value,
                             reference_point(
                                 *placement,
                                 radius,
                                 u,
                                 v_domain->lower()),
                             1024.0) &&
                         close_point(
                             *upper_value,
                             reference_point(
                                 *placement,
                                 radius,
                                 u,
                                 v_domain->upper()),
                             1024.0),
                     "bounded cylinder circular-boundary oracle differs") &&
                 passed;
    }

    constexpr double identity_u = 0.75;
    constexpr double identity_v = -1.25;
    const auto value =
        cylinder->evaluate(identity_u, identity_v);
    const auto first =
        cylinder->first_derivatives(identity_u, identity_v);
    const auto second =
        cylinder->second_derivatives(identity_u, identity_v);
    if (!value || !first || !second) {
        return 1;
    }

    const auto orthogonality =
        apmesh::core::dot(first->u, first->v);
    const auto u_norm = apmesh::core::norm(first->u);
    const auto v_norm = apmesh::core::norm(first->v);
    const auto orientation =
        apmesh::core::cross(first->u, first->v);
    const auto radial = -second->uu;

    passed = require(
                 orthogonality && u_norm && v_norm && orientation &&
                     close_scalar(*orthogonality, 0.0, 1.0) &&
                     close_scalar(*u_norm, radius, radius) &&
                     close_scalar(*v_norm, 1.0, 1.0) &&
                     close_vector(*orientation, radial, 4096.0) &&
                     close_scalar(
                         radial.x(),
                         value->x() -
                             reference_point(
                                 *placement, 0.0, identity_u, identity_v)
                                 .x(),
                         4096.0) &&
                     second->uv ==
                         Vector3::make(0.0, 0.0, 0.0).value() &&
                     second->vv ==
                         Vector3::make(0.0, 0.0, 0.0).value(),
                 "bounded cylinder derivative identities differ") &&
             passed;

    const auto u_reversed = cylinder->u_reversed();
    const auto v_reversed = cylinder->v_reversed();
    const auto both_reversed = u_reversed.v_reversed();
    const auto mapped_u =
        reversed_parameter(*u_domain, identity_u);
    const auto mapped_v =
        reversed_parameter(*v_domain, identity_v);
    if (!mapped_u || !mapped_v) {
        return 1;
    }

    const auto u_value =
        u_reversed.evaluate(*mapped_u, identity_v);
    const auto v_value =
        v_reversed.evaluate(identity_u, *mapped_v);
    const auto both_value =
        both_reversed.evaluate(*mapped_u, *mapped_v);
    const auto u_first =
        u_reversed.first_derivatives(*mapped_u, identity_v);
    const auto v_first =
        v_reversed.first_derivatives(identity_u, *mapped_v);
    const auto both_first =
        both_reversed.first_derivatives(*mapped_u, *mapped_v);
    const auto u_second =
        u_reversed.second_derivatives(*mapped_u, identity_v);
    const auto v_second =
        v_reversed.second_derivatives(identity_u, *mapped_v);
    const auto both_second =
        both_reversed.second_derivatives(*mapped_u, *mapped_v);

    passed = require(
                 u_reversed.u_is_reversed() &&
                     !u_reversed.v_is_reversed() &&
                     !v_reversed.u_is_reversed() &&
                     v_reversed.v_is_reversed() &&
                     u_reversed.u_reversed() == *cylinder &&
                     v_reversed.v_reversed() == *cylinder &&
                     both_reversed.u_reversed().v_reversed() == *cylinder &&
                     u_value && v_value && both_value &&
                     close_point(*value, *u_value, 1024.0) &&
                     close_point(*value, *v_value, 1024.0) &&
                     close_point(*value, *both_value, 1024.0) &&
                     u_first && v_first && both_first &&
                     close_vector(u_first->u, -first->u, 1024.0) &&
                     close_vector(u_first->v, first->v, 1024.0) &&
                     close_vector(v_first->u, first->u, 1024.0) &&
                     close_vector(v_first->v, -first->v, 1024.0) &&
                     close_vector(both_first->u, -first->u, 1024.0) &&
                     close_vector(both_first->v, -first->v, 1024.0) &&
                     u_second && v_second && both_second &&
                     close_vector(u_second->uu, second->uu, 4096.0) &&
                     close_vector(v_second->uu, second->uu, 4096.0) &&
                     close_vector(both_second->uu, second->uu, 4096.0),
                 "bounded cylinder reversal covariance differs") &&
             passed;

    const auto translated_origin =
        Point3::make(
            origin->x() + 8.0,
            origin->y() - 6.0,
            origin->z() + 4.0);
    if (!translated_origin) {
        return 1;
    }
    const auto translated_placement = AxisPlacement3::make(
        *translated_origin, *axis, *x_reference);
    if (!translated_placement) {
        return 1;
    }
    const auto translated = BoundedCylinderSurface3::make(
        *translated_placement,
        radius,
        *u_domain,
        *v_domain);
    if (!translated) {
        return 1;
    }

    const auto translated_value =
        translated->evaluate(identity_u, identity_v);
    const auto translated_first =
        translated->first_derivatives(identity_u, identity_v);
    const auto translated_second =
        translated->second_derivatives(identity_u, identity_v);

    passed = require(
                 translated_value && translated_first && translated_second &&
                     close_scalar(
                         translated_value->x() - value->x(),
                         8.0,
                         32.0) &&
                     close_scalar(
                         translated_value->y() - value->y(),
                         -6.0,
                         32.0) &&
                     close_scalar(
                         translated_value->z() - value->z(),
                         4.0,
                         32.0) &&
                     close_vector(
                         translated_first->u, first->u, 1024.0) &&
                     close_vector(
                         translated_first->v, first->v, 1024.0) &&
                     close_vector(
                         translated_second->uu, second->uu, 4096.0),
                 "bounded cylinder translation covariance differs") &&
             passed;

    const auto identity_origin = Point3::make(1.0, -2.0, 3.0);
    const auto doubled_origin = Point3::make(2.0, -4.0, 6.0);
    const auto z_axis = Vector3::make(0.0, 0.0, 1.0);
    const auto x_axis = Vector3::make(1.0, 0.0, 0.0);
    const auto scale_u = CurveParameterDomain::make(-0.5, 1.0);
    const auto scale_v = CurveParameterDomain::make(-1.0, 3.0);
    const auto doubled_v = CurveParameterDomain::make(-2.0, 6.0);
    if (!identity_origin || !doubled_origin || !z_axis || !x_axis ||
        !scale_u || !scale_v || !doubled_v) {
        return 1;
    }

    const auto identity_placement =
        AxisPlacement3::make(*identity_origin, *z_axis, *x_axis);
    const auto doubled_placement =
        AxisPlacement3::make(*doubled_origin, *z_axis, *x_axis);
    if (!identity_placement || !doubled_placement) {
        return 1;
    }

    const auto scale_surface = BoundedCylinderSurface3::make(
        *identity_placement, 2.0, *scale_u, *scale_v);
    const auto doubled_surface = BoundedCylinderSurface3::make(
        *doubled_placement, 4.0, *scale_u, *doubled_v);
    if (!scale_surface || !doubled_surface) {
        return 1;
    }

    const auto scale_value = scale_surface->evaluate(0.25, 0.5);
    const auto doubled_value = doubled_surface->evaluate(0.25, 1.0);
    const auto scale_first =
        scale_surface->first_derivatives(0.25, 0.5);
    const auto doubled_first =
        doubled_surface->first_derivatives(0.25, 1.0);
    const auto scale_second =
        scale_surface->second_derivatives(0.25, 0.5);
    const auto doubled_second =
        doubled_surface->second_derivatives(0.25, 1.0);

    passed = require(
                 scale_value && doubled_value &&
                     scale_first && doubled_first &&
                     scale_second && doubled_second &&
                     close_scalar(
                         doubled_value->x(), 2.0 * scale_value->x(), 2048.0) &&
                     close_scalar(
                         doubled_value->y(), 2.0 * scale_value->y(), 2048.0) &&
                     close_scalar(
                         doubled_value->z(), 2.0 * scale_value->z(), 2048.0) &&
                     close_vector(
                         doubled_first->u,
                         Vector3::make(
                             2.0 * scale_first->u.x(),
                             2.0 * scale_first->u.y(),
                             2.0 * scale_first->u.z())
                             .value(),
                         4096.0) &&
                     close_vector(
                         doubled_first->v,
                         scale_first->v,
                         1024.0) &&
                     close_vector(
                         doubled_second->uu,
                         Vector3::make(
                             2.0 * scale_second->uu.x(),
                             2.0 * scale_second->uu.y(),
                             2.0 * scale_second->uu.z())
                             .value(),
                         8192.0),
                 "bounded cylinder power-of-two scale covariance differs") &&
             passed;

    const double maximum = std::numeric_limits<double>::max();
    const auto extreme_origin =
        Point3::make(maximum / 8.0, 0.0, 0.0);
    const auto extreme_u =
        CurveParameterDomain::make(0.25, 0.5);
    const auto extreme_v =
        CurveParameterDomain::make(
            -maximum / 32.0, maximum / 32.0);
    if (!extreme_origin || !extreme_u || !extreme_v) {
        return 1;
    }
    const auto extreme_placement =
        AxisPlacement3::make(*extreme_origin, *z_axis, *x_axis);
    if (!extreme_placement) {
        return 1;
    }

    const auto extreme_surface = BoundedCylinderSurface3::make(
        *extreme_placement,
        maximum / 32.0,
        *extreme_u,
        *extreme_v);
    if (!extreme_surface) {
        return 1;
    }
    const auto extreme_value =
        extreme_surface->evaluate(0.375, maximum / 64.0);
    const auto extreme_first =
        extreme_surface->first_derivatives(
            0.375, maximum / 64.0);
    const auto extreme_second =
        extreme_surface->second_derivatives(
            0.375, maximum / 64.0);

    passed = require(
                 extreme_value && extreme_first && extreme_second &&
                     std::isfinite(extreme_value->x()) &&
                     std::isfinite(extreme_value->y()) &&
                     std::isfinite(extreme_value->z()) &&
                     std::isfinite(extreme_first->u.x()) &&
                     std::isfinite(extreme_second->uu.x()),
                 "bounded cylinder representable extreme query failed") &&
             passed;

    const auto overflow_origin =
        Point3::make(maximum / 2.0, 0.0, 0.0);
    const auto overflow_u =
        CurveParameterDomain::make(-0.1, 0.1);
    const auto overflow_v =
        CurveParameterDomain::make(-1.0, 1.0);
    if (!overflow_origin || !overflow_u || !overflow_v) {
        return 1;
    }
    const auto overflow_placement =
        AxisPlacement3::make(*overflow_origin, *z_axis, *x_axis);
    if (!overflow_placement) {
        return 1;
    }
    const auto overflow_surface = BoundedCylinderSurface3::make(
        *overflow_placement,
        maximum,
        *overflow_u,
        *overflow_v);
    if (!overflow_surface) {
        return 1;
    }
    const auto overflow_value =
        overflow_surface->evaluate(0.0, 0.0);

    passed = require(
                 !overflow_value &&
                     overflow_value.error() ==
                         SurfaceError::non_finite_result,
                 "bounded cylinder unrepresentable value did not fail") &&
             passed;

    const auto repeat_value_a =
        cylinder->evaluate(identity_u, identity_v);
    const auto repeat_value_b =
        cylinder->evaluate(identity_u, identity_v);
    const auto repeat_first_a =
        cylinder->first_derivatives(identity_u, identity_v);
    const auto repeat_first_b =
        cylinder->first_derivatives(identity_u, identity_v);
    const auto repeat_second_a =
        cylinder->second_derivatives(identity_u, identity_v);
    const auto repeat_second_b =
        cylinder->second_derivatives(identity_u, identity_v);
    const auto repeat_failure_a =
        cylinder->evaluate(u_domain->upper() + 1.0, identity_v);
    const auto repeat_failure_b =
        cylinder->evaluate(u_domain->upper() + 1.0, identity_v);

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
                 "bounded cylinder repeated evidence is not deterministic") &&
             passed;

    return passed ? 0 : 1;
}
