#include "apmesh/geometry/revolution_surface.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <cstdio>
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
    const double absolute = 1.2e-11,
    const double relative = 1.2e-11) {
    const double magnitude =
        std::max({1.0, std::abs(lhs), std::abs(rhs), std::abs(scale)});
    return std::abs(lhs - rhs) <=
           absolute * std::abs(scale) + relative * magnitude;
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

struct LVec3 {
    long double x{};
    long double y{};
    long double z{};
};

LVec3 operator+(const LVec3& lhs, const LVec3& rhs) {
    return {lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
}

LVec3 operator-(const LVec3& lhs, const LVec3& rhs) {
    return {lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
}

LVec3 operator*(const LVec3& value, const long double scalar) {
    return {value.x * scalar, value.y * scalar, value.z * scalar};
}

long double dot(const LVec3& lhs, const LVec3& rhs) {
    return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
}

LVec3 cross(const LVec3& lhs, const LVec3& rhs) {
    return {
        lhs.y * rhs.z - lhs.z * rhs.y,
        lhs.z * rhs.x - lhs.x * rhs.z,
        lhs.x * rhs.y - lhs.y * rhs.x,
    };
}

LVec3 as_long(const apmesh::core::Point3& point) {
    return {
        static_cast<long double>(point.x()),
        static_cast<long double>(point.y()),
        static_cast<long double>(point.z()),
    };
}

LVec3 as_long(const apmesh::core::Vector3& vector) {
    return {
        static_cast<long double>(vector.x()),
        static_cast<long double>(vector.y()),
        static_cast<long double>(vector.z()),
    };
}

struct CurveJet {
    LVec3 value;
    LVec3 d1;
    LVec3 d2;
};

CurveJet reference_curve_jet(
    const std::array<apmesh::core::Point3, 4>& points,
    const long double t) {
    const long double q = 1.0L - t;
    const long double b0 = q * q * q;
    const long double b1 = 3.0L * t * q * q;
    const long double b2 = 3.0L * t * t * q;
    const long double b3 = t * t * t;

    const auto p0 = as_long(points[0]);
    const auto p1 = as_long(points[1]);
    const auto p2 = as_long(points[2]);
    const auto p3 = as_long(points[3]);

    const LVec3 value{
        b0 * p0.x + b1 * p1.x + b2 * p2.x + b3 * p3.x,
        b0 * p0.y + b1 * p1.y + b2 * p2.y + b3 * p3.y,
        b0 * p0.z + b1 * p1.z + b2 * p2.z + b3 * p3.z,
    };

    const LVec3 d1{
        3.0L * (q * q * (p1.x - p0.x) +
                2.0L * q * t * (p2.x - p1.x) +
                t * t * (p3.x - p2.x)),
        3.0L * (q * q * (p1.y - p0.y) +
                2.0L * q * t * (p2.y - p1.y) +
                t * t * (p3.y - p2.y)),
        3.0L * (q * q * (p1.z - p0.z) +
                2.0L * q * t * (p2.z - p1.z) +
                t * t * (p3.z - p2.z)),
    };

    const LVec3 d2{
        6.0L * (q * (p2.x - 2.0L * p1.x + p0.x) +
                t * (p3.x - 2.0L * p2.x + p1.x)),
        6.0L * (q * (p2.y - 2.0L * p1.y + p0.y) +
                t * (p3.y - 2.0L * p2.y + p1.y)),
        6.0L * (q * (p2.z - 2.0L * p1.z + p0.z) +
                t * (p3.z - 2.0L * p2.z + p1.z)),
    };

    return {value, d1, d2};
}

LVec3 rotate_vector_reference(
    const LVec3& value,
    const LVec3& axis,
    const long double angle) {
    const long double sine = std::sin(angle);
    const long double cosine = std::cos(angle);
    const auto axis_cross = cross(axis, value);
    const long double projection = dot(axis, value);
    return value * cosine +
           axis_cross * sine +
           axis * (projection * (1.0L - cosine));
}

LVec3 rotate_point_reference(
    const LVec3& point,
    const LVec3& origin,
    const LVec3& axis,
    const long double angle) {
    return origin +
           rotate_vector_reference(point - origin, axis, angle);
}

struct SurfaceJet {
    LVec3 value;
    LVec3 du;
    LVec3 dv;
    LVec3 duu;
    LVec3 duv;
    LVec3 dvv;
};

SurfaceJet reference_surface_jet(
    const std::array<apmesh::core::Point3, 4>& points,
    const apmesh::core::AxisPlacement3& placement,
    const double sweep,
    const double u,
    const double v) {
    const auto curve =
        reference_curve_jet(points, static_cast<long double>(v));
    const auto origin = as_long(placement.origin());
    const auto axis = as_long(placement.z_direction());
    const long double alpha = static_cast<long double>(sweep);
    const long double angle = static_cast<long double>(u) * alpha;

    const auto value =
        rotate_point_reference(curve.value, origin, axis, angle);
    const auto rotated_d1 =
        rotate_vector_reference(curve.d1, axis, angle);
    const auto rotated_d2 =
        rotate_vector_reference(curve.d2, axis, angle);
    const auto radius = value - origin;
    const auto axis_cross_radius = cross(axis, radius);

    return {
        value,
        axis_cross_radius * alpha,
        rotated_d1,
        cross(axis, axis_cross_radius) * (alpha * alpha),
        cross(axis, rotated_d1) * alpha,
        rotated_d2,
    };
}

bool close_point_reference(
    const apmesh::core::Point3& actual,
    const LVec3& expected,
    const double scale) {
    return close_scalar(actual.x(), static_cast<double>(expected.x), scale) &&
           close_scalar(actual.y(), static_cast<double>(expected.y), scale) &&
           close_scalar(actual.z(), static_cast<double>(expected.z), scale);
}

bool close_vector_reference(
    const apmesh::core::Vector3& actual,
    const LVec3& expected,
    const double scale) {
    return close_scalar(actual.x(), static_cast<double>(expected.x), scale) &&
           close_scalar(actual.y(), static_cast<double>(expected.y), scale) &&
           close_scalar(actual.z(), static_cast<double>(expected.z), scale);
}

std::array<apmesh::core::Point3, 4> linear_controls(
    const apmesh::core::Point3& start,
    const apmesh::core::Point3& end) {
    const auto delta = end - start;
    if (!delta) {
        return {start, start, end, end};
    }
    const auto one_third = *delta / 3.0;
    const auto two_thirds = *delta * (2.0 / 3.0);
    if (!one_third || !two_thirds) {
        return {start, start, end, end};
    }
    const auto p1 = start + *one_third;
    const auto p2 = start + *two_thirds;
    if (!p1 || !p2) {
        return {start, start, end, end};
    }
    return {start, *p1, *p2, end};
}

} // namespace

int main() {
    using apmesh::core::AxisPlacement3;
    using apmesh::core::BoundedParametricSurface3;
    using apmesh::core::CubicBezier3;
    using apmesh::core::CubicBezierRevolutionSurface3;
    using apmesh::core::Point3;
    using apmesh::core::RevolutionSurfaceConstructionError;
    using apmesh::core::SurfaceError;
    using apmesh::core::Vector3;

    static_assert(
        BoundedParametricSurface3<CubicBezierRevolutionSurface3>);

    bool passed = true;

    const auto p0 = Point3::make(2.0, -1.0, 0.5);
    const auto p1 = Point3::make(3.0, 1.5, 2.0);
    const auto p2 = Point3::make(1.0, 4.0, -0.5);
    const auto p3 = Point3::make(-1.0, 2.0, 3.5);
    const auto axis_origin = Point3::make(0.5, -2.0, 1.0);
    const auto axis_main = Vector3::make(1.0, 2.0, 3.0);
    const auto axis_x = Vector3::make(2.0, -1.0, 1.0);
    if (!p0 || !p1 || !p2 || !p3 ||
        !axis_origin || !axis_main || !axis_x) {
        return 1;
    }

    const CubicBezier3 generatrix{*p0, *p1, *p2, *p3};
    const auto axis =
        AxisPlacement3::make(*axis_origin, *axis_main, *axis_x);
    if (!axis) {
        return 1;
    }

    constexpr double sweep = 1.25;
    const auto surface =
        CubicBezierRevolutionSurface3::make(generatrix, *axis, sweep);
    if (!surface) {
        return 1;
    }

    passed = require(
                 surface->start_curve() == generatrix &&
                     surface->axis_placement() == *axis &&
                     surface->sweep_angle() == sweep,
                 "revolution stored representation differs") &&
             passed;

    const auto domain = surface->parameter_domain();
    passed = require(
                 domain.u.lower() == 0.0 &&
                     domain.u.upper() == 1.0 &&
                     domain.v.lower() == 0.0 &&
                     domain.v.upper() == 1.0,
                 "revolution parameter domain differs") &&
             passed;

    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double infinity = std::numeric_limits<double>::infinity();
    const auto non_finite =
        CubicBezierRevolutionSurface3::make(generatrix, *axis, nan);
    const auto zero =
        CubicBezierRevolutionSurface3::make(generatrix, *axis, 0.0);
    const auto full = CubicBezierRevolutionSurface3::make(
        generatrix,
        *axis,
        2.0 * std::numbers::pi_v<double>);
    const auto negative_full = CubicBezierRevolutionSurface3::make(
        generatrix,
        *axis,
        -2.0 * std::numbers::pi_v<double>);
    const auto excessive =
        CubicBezierRevolutionSurface3::make(generatrix, *axis, 7.0);
    const auto negative =
        CubicBezierRevolutionSurface3::make(generatrix, *axis, -1.0);

    passed = require(
                 !non_finite &&
                     non_finite.error() ==
                         RevolutionSurfaceConstructionError::
                             non_finite_sweep_angle &&
                     !zero &&
                     zero.error() ==
                         RevolutionSurfaceConstructionError::
                             zero_sweep_angle &&
                     !full &&
                     full.error() ==
                         RevolutionSurfaceConstructionError::
                             full_or_multiple_revolution_not_admitted &&
                     !negative_full &&
                     negative_full.error() ==
                         RevolutionSurfaceConstructionError::
                             full_or_multiple_revolution_not_admitted &&
                     !excessive &&
                     excessive.error() ==
                         RevolutionSurfaceConstructionError::
                             full_or_multiple_revolution_not_admitted &&
                     negative.has_value(),
                 "revolution sweep validation differs") &&
             passed;

    const auto u_nan = surface->evaluate(nan, nan);
    const auto v_nan = surface->evaluate(0.5, nan);
    const auto v_nan_before_u_domain = surface->evaluate(-0.1, nan);
    const auto u_low = surface->evaluate(-0.1, 0.5);
    const auto v_high = surface->evaluate(0.5, 1.1);
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
                 "revolution parameter validation order differs") &&
             passed;

    constexpr std::array<std::array<double, 2>, 8> samples{{
        {0.0, 0.0},
        {0.0, 0.6},
        {0.125, 0.25},
        {0.35, 0.75},
        {0.5, 0.5},
        {0.825, 0.2},
        {1.0, 0.4},
        {1.0, 1.0},
    }};

    for (const auto& sample : samples) {
        const double u = sample[0];
        const double v = sample[1];
        const auto reference = reference_surface_jet(
            generatrix.control_points(), *axis, sweep, u, v);
        const auto value = surface->evaluate(u, v);
        const auto first = surface->first_derivatives(u, v);
        const auto second = surface->second_derivatives(u, v);

        passed = require(
                     value && first && second &&
                         close_point_reference(
                             *value, reference.value, 512.0) &&
                         close_vector_reference(
                             first->u, reference.du, 2048.0) &&
                         close_vector_reference(
                             first->v, reference.dv, 2048.0) &&
                         close_vector_reference(
                             second->uu, reference.duu, 8192.0) &&
                         close_vector_reference(
                             second->uv, reference.duv, 8192.0) &&
                         close_vector_reference(
                             second->vv, reference.dvv, 8192.0),
                     "revolution independent Bernstein/Rodrigues oracle differs") &&
                 passed;
    }

    constexpr std::array<double, 5> boundary_parameters{
        0.0, 0.2, 0.5, 0.8, 1.0};
    for (const double v : boundary_parameters) {
        const auto start_value = surface->evaluate(0.0, v);
        const auto curve_value = generatrix.evaluate(v);
        const auto end_value = surface->evaluate(1.0, v);
        const auto stored_end = surface->end_curve().evaluate(v);
        passed = require(
                     start_value && curve_value &&
                         *start_value == *curve_value &&
                         end_value && stored_end &&
                         *end_value == *stored_end,
                     "revolution U-boundary parity differs") &&
                 passed;
    }

    const auto identity_axis = AxisPlacement3::identity();
    const auto c0 = Point3::make(2.0, 0.0, -1.0);
    const auto c3 = Point3::make(2.0, 0.0, 3.0);
    if (!c0 || !c3) {
        return 1;
    }
    const auto cylinder_controls = linear_controls(*c0, *c3);
    const CubicBezier3 cylinder_curve{
        cylinder_controls[0],
        cylinder_controls[1],
        cylinder_controls[2],
        cylinder_controls[3]};
    constexpr double cylinder_sweep =
        std::numbers::pi_v<double> / 2.0;
    const auto cylinder = CubicBezierRevolutionSurface3::make(
        cylinder_curve, identity_axis, cylinder_sweep);
    if (!cylinder) {
        return 1;
    }

    constexpr double cu = 0.4;
    constexpr double cv = 0.35;
    const double theta = cu * cylinder_sweep;
    const double z = -1.0 + 4.0 * cv;
    const auto cylinder_value = cylinder->evaluate(cu, cv);
    const auto cylinder_first = cylinder->first_derivatives(cu, cv);
    const auto cylinder_second = cylinder->second_derivatives(cu, cv);
    passed = require(
                 cylinder_value && cylinder_first && cylinder_second &&
                     close_scalar(
                         cylinder_value->x(),
                         2.0 * std::cos(theta),
                         128.0) &&
                     close_scalar(
                         cylinder_value->y(),
                         2.0 * std::sin(theta),
                         128.0) &&
                     close_scalar(cylinder_value->z(), z, 128.0) &&
                     close_scalar(
                         cylinder_first->u.x(),
                         -2.0 * cylinder_sweep * std::sin(theta),
                         512.0) &&
                     close_scalar(
                         cylinder_first->u.y(),
                         2.0 * cylinder_sweep * std::cos(theta),
                         512.0) &&
                     close_scalar(cylinder_first->u.z(), 0.0, 512.0) &&
                     close_scalar(cylinder_first->v.x(), 0.0, 512.0) &&
                     close_scalar(cylinder_first->v.y(), 0.0, 512.0) &&
                     close_scalar(cylinder_first->v.z(), 4.0, 512.0) &&
                     close_scalar(
                         cylinder_second->uu.x(),
                         -2.0 * cylinder_sweep * cylinder_sweep *
                             std::cos(theta),
                         2048.0) &&
                     close_scalar(
                         cylinder_second->uu.y(),
                         -2.0 * cylinder_sweep * cylinder_sweep *
                             std::sin(theta),
                         2048.0) &&
                     close_vector(
                         cylinder_second->uv,
                         *Vector3::make(0.0, 0.0, 0.0),
                         2048.0) &&
                     close_vector(
                         cylinder_second->vv,
                         *Vector3::make(0.0, 0.0, 0.0),
                         2048.0),
                 "revolution cylinder analytic fixture differs") &&
             passed;

    const auto a0 = Point3::make(1.0, 0.0, 0.5);
    const auto a3 = Point3::make(3.0, 0.0, 0.5);
    if (!a0 || !a3) {
        return 1;
    }
    const auto annular_controls = linear_controls(*a0, *a3);
    const CubicBezier3 annular_curve{
        annular_controls[0],
        annular_controls[1],
        annular_controls[2],
        annular_controls[3]};
    constexpr double annular_sweep = -0.75;
    const auto annular = CubicBezierRevolutionSurface3::make(
        annular_curve, identity_axis, annular_sweep);
    if (!annular) {
        return 1;
    }
    constexpr double au = 0.6;
    constexpr double av = 0.25;
    const double annular_theta = au * annular_sweep;
    const double radius = 1.0 + 2.0 * av;
    const auto annular_value = annular->evaluate(au, av);
    const auto annular_first = annular->first_derivatives(au, av);
    const auto annular_second = annular->second_derivatives(au, av);
    passed = require(
                 annular_value && annular_first && annular_second &&
                     close_scalar(
                         annular_value->x(),
                         radius * std::cos(annular_theta),
                         128.0) &&
                     close_scalar(
                         annular_value->y(),
                         radius * std::sin(annular_theta),
                         128.0) &&
                     close_scalar(annular_value->z(), 0.5, 128.0) &&
                     close_scalar(
                         annular_first->v.x(),
                         2.0 * std::cos(annular_theta),
                         512.0) &&
                     close_scalar(
                         annular_first->v.y(),
                         2.0 * std::sin(annular_theta),
                         512.0) &&
                     close_scalar(
                         annular_second->uv.x(),
                         -2.0 * annular_sweep *
                             std::sin(annular_theta),
                         2048.0) &&
                     close_scalar(
                         annular_second->uv.y(),
                         2.0 * annular_sweep *
                             std::cos(annular_theta),
                         2048.0),
                 "revolution annular-sector analytic fixture differs") &&
             passed;

    const auto axis_x_alternate = Vector3::make(-1.0, 3.0, -1.0);
    if (!axis_x_alternate) {
        return 1;
    }
    const auto alternate_axis = AxisPlacement3::make(
        *axis_origin, *axis_main, *axis_x_alternate);
    if (!alternate_axis) {
        return 1;
    }
    const auto alternate_surface =
        CubicBezierRevolutionSurface3::make(
            generatrix, *alternate_axis, sweep);
    if (!alternate_surface) {
        return 1;
    }
    const auto axis_ref_value = surface->evaluate(0.45, 0.55);
    const auto alternate_value =
        alternate_surface->evaluate(0.45, 0.55);
    const auto axis_ref_first =
        surface->first_derivatives(0.45, 0.55);
    const auto alternate_first =
        alternate_surface->first_derivatives(0.45, 0.55);
    const auto axis_ref_second =
        surface->second_derivatives(0.45, 0.55);
    const auto alternate_second =
        alternate_surface->second_derivatives(0.45, 0.55);
    passed = require(
                 axis_ref_value && alternate_value &&
                     close_point(
                         *axis_ref_value, *alternate_value, 512.0) &&
                     axis_ref_first && alternate_first &&
                     close_vector(
                         axis_ref_first->u, alternate_first->u, 2048.0) &&
                     close_vector(
                         axis_ref_first->v, alternate_first->v, 2048.0) &&
                     axis_ref_second && alternate_second &&
                     close_vector(
                         axis_ref_second->uu,
                         alternate_second->uu,
                         8192.0) &&
                     close_vector(
                         axis_ref_second->uv,
                         alternate_second->uv,
                         8192.0) &&
                     close_vector(
                         axis_ref_second->vv,
                         alternate_second->vv,
                         8192.0),
                 "revolution depends on axis X/Y reference") &&
             passed;

    const auto positive =
        CubicBezierRevolutionSurface3::make(
            cylinder_curve, identity_axis, 0.8);
    const auto negative_sweep =
        CubicBezierRevolutionSurface3::make(
            cylinder_curve, identity_axis, -0.8);
    if (!positive || !negative_sweep) {
        return 1;
    }
    const auto positive_origin =
        positive->first_derivatives(0.0, 0.4);
    const auto negative_origin =
        negative_sweep->first_derivatives(0.0, 0.4);
    const auto positive_value =
        positive->evaluate(0.0, 0.4);
    const auto negative_value =
        negative_sweep->evaluate(0.0, 0.4);
    passed = require(
                 positive_origin && negative_origin &&
                     positive_value && negative_value &&
                     *positive_value == *negative_value &&
                     close_scalar(
                         positive_origin->u.x(),
                         -negative_origin->u.x(),
                         512.0) &&
                     close_scalar(
                         positive_origin->u.y(),
                         -negative_origin->u.y(),
                         512.0) &&
                     close_scalar(
                         positive_origin->u.z(),
                         -negative_origin->u.z(),
                         512.0) &&
                     close_vector(
                         positive_origin->v,
                         negative_origin->v,
                         512.0),
                 "signed revolution sweep covariance differs") &&
             passed;

    const auto u_reversed = surface->u_reversed();
    const auto v_reversed = surface->v_reversed();
    passed = require(
                 u_reversed.u_reversed() == *surface &&
                     v_reversed.v_reversed() == *surface,
                 "revolution reversal is not exact involution") &&
             passed;

    constexpr double ru = 0.3;
    constexpr double rv = 0.65;
    const auto original_value = surface->evaluate(ru, rv);
    const auto original_first = surface->first_derivatives(ru, rv);
    const auto original_second = surface->second_derivatives(ru, rv);
    const auto u_value = u_reversed.evaluate(1.0 - ru, rv);
    const auto u_first =
        u_reversed.first_derivatives(1.0 - ru, rv);
    const auto u_second =
        u_reversed.second_derivatives(1.0 - ru, rv);
    const auto v_value = v_reversed.evaluate(ru, 1.0 - rv);
    const auto v_first =
        v_reversed.first_derivatives(ru, 1.0 - rv);
    const auto v_second =
        v_reversed.second_derivatives(ru, 1.0 - rv);

    passed = require(
                 original_value && original_first && original_second &&
                     u_value && u_first && u_second &&
                     v_value && v_first && v_second &&
                     close_point(*u_value, *original_value, 512.0) &&
                     close_scalar(
                         u_first->u.x(),
                         -original_first->u.x(),
                         2048.0) &&
                     close_scalar(
                         u_first->u.y(),
                         -original_first->u.y(),
                         2048.0) &&
                     close_scalar(
                         u_first->u.z(),
                         -original_first->u.z(),
                         2048.0) &&
                     close_vector(
                         u_first->v, original_first->v, 2048.0) &&
                     close_vector(
                         u_second->uu, original_second->uu, 8192.0) &&
                     close_scalar(
                         u_second->uv.x(),
                         -original_second->uv.x(),
                         8192.0) &&
                     close_scalar(
                         u_second->uv.y(),
                         -original_second->uv.y(),
                         8192.0) &&
                     close_scalar(
                         u_second->uv.z(),
                         -original_second->uv.z(),
                         8192.0) &&
                     close_vector(
                         u_second->vv, original_second->vv, 8192.0) &&
                     close_point(*v_value, *original_value, 512.0) &&
                     close_vector(
                         v_first->u, original_first->u, 2048.0) &&
                     close_scalar(
                         v_first->v.x(),
                         -original_first->v.x(),
                         2048.0) &&
                     close_scalar(
                         v_first->v.y(),
                         -original_first->v.y(),
                         2048.0) &&
                     close_scalar(
                         v_first->v.z(),
                         -original_first->v.z(),
                         2048.0) &&
                     close_vector(
                         v_second->uu, original_second->uu, 8192.0) &&
                     close_scalar(
                         v_second->uv.x(),
                         -original_second->uv.x(),
                         8192.0) &&
                     close_scalar(
                         v_second->uv.y(),
                         -original_second->uv.y(),
                         8192.0) &&
                     close_scalar(
                         v_second->uv.z(),
                         -original_second->uv.z(),
                         8192.0) &&
                     close_vector(
                         v_second->vv, original_second->vv, 8192.0),
                 "revolution reversal covariance differs") &&
             passed;

    const auto on_axis0 = Point3::make(0.0, 0.0, -1.0);
    const auto on_axis3 = Point3::make(0.0, 0.0, 2.0);
    if (!on_axis0 || !on_axis3) {
        return 1;
    }
    const auto on_axis_controls = linear_controls(*on_axis0, *on_axis3);
    const CubicBezier3 on_axis_curve{
        on_axis_controls[0],
        on_axis_controls[1],
        on_axis_controls[2],
        on_axis_controls[3]};
    const auto on_axis_surface =
        CubicBezierRevolutionSurface3::make(
            on_axis_curve, identity_axis, 0.75);
    if (!on_axis_surface) {
        return 1;
    }
    const auto on_axis_first =
        on_axis_surface->first_derivatives(0.4, 0.6);
    passed = require(
                 on_axis_first &&
                     on_axis_first->u ==
                         *Vector3::make(0.0, 0.0, 0.0),
                 "revolution rejected or altered on-axis degeneracy") &&
             passed;

    const auto constant_point = Point3::make(2.0, 0.0, 1.0);
    if (!constant_point) {
        return 1;
    }
    const CubicBezier3 constant_curve{
        *constant_point,
        *constant_point,
        *constant_point,
        *constant_point};
    const auto constant_surface =
        CubicBezierRevolutionSurface3::make(
            constant_curve, identity_axis, 0.9);
    if (!constant_surface) {
        return 1;
    }
    const auto constant_first =
        constant_surface->first_derivatives(0.5, 0.5);
    const auto constant_second =
        constant_surface->second_derivatives(0.5, 0.5);
    passed = require(
                 constant_first && constant_second &&
                     constant_first->v ==
                         *Vector3::make(0.0, 0.0, 0.0) &&
                     constant_second->uv ==
                         *Vector3::make(0.0, 0.0, 0.0) &&
                     constant_second->vv ==
                         *Vector3::make(0.0, 0.0, 0.0),
                 "revolution constant-generatrix semantics differ") &&
             passed;

    const auto translation = Vector3::make(4.0, -8.0, 2.0);
    if (!translation) {
        return 1;
    }
    auto translated_controls = generatrix.control_points();
    for (std::size_t index = 0; index < translated_controls.size(); ++index) {
        const auto translated =
            generatrix.control_points()[index] + *translation;
        if (!translated) {
            return 1;
        }
        translated_controls[index] = *translated;
    }
    const auto translated_origin = *axis_origin + *translation;
    if (!translated_origin) {
        return 1;
    }
    const auto translated_axis =
        AxisPlacement3::make(*translated_origin, *axis_main, *axis_x);
    if (!translated_axis) {
        return 1;
    }
    const CubicBezier3 translated_curve{
        translated_controls[0],
        translated_controls[1],
        translated_controls[2],
        translated_controls[3]};
    const auto translated_surface =
        CubicBezierRevolutionSurface3::make(
            translated_curve, *translated_axis, sweep);
    if (!translated_surface) {
        return 1;
    }
    const auto base_translate_value = surface->evaluate(0.4, 0.6);
    const auto shifted_value =
        translated_surface->evaluate(0.4, 0.6);
    const auto shifted_first =
        translated_surface->first_derivatives(0.4, 0.6);
    const auto base_translate_first =
        surface->first_derivatives(0.4, 0.6);
    passed = require(
                 base_translate_value && shifted_value &&
                     close_scalar(
                         shifted_value->x() - base_translate_value->x(),
                         translation->x(),
                         1024.0) &&
                     close_scalar(
                         shifted_value->y() - base_translate_value->y(),
                         translation->y(),
                         1024.0) &&
                     close_scalar(
                         shifted_value->z() - base_translate_value->z(),
                         translation->z(),
                         1024.0) &&
                     shifted_first && base_translate_first &&
                     close_vector(
                         shifted_first->u,
                         base_translate_first->u,
                         4096.0) &&
                     close_vector(
                         shifted_first->v,
                         base_translate_first->v,
                         4096.0),
                 "revolution translation covariance differs") &&
             passed;

    constexpr double scale = 2.0;
    const auto scaled_origin = Point3::make(
        scale * axis_origin->x(),
        scale * axis_origin->y(),
        scale * axis_origin->z());
    if (!scaled_origin) {
        return 1;
    }
    auto scaled_controls = generatrix.control_points();
    for (std::size_t index = 0; index < scaled_controls.size(); ++index) {
        const auto& point = generatrix.control_points()[index];
        const auto scaled_point = Point3::make(
            scale * point.x(), scale * point.y(), scale * point.z());
        if (!scaled_point) {
            return 1;
        }
        scaled_controls[index] = *scaled_point;
    }
    const auto scaled_axis =
        AxisPlacement3::make(*scaled_origin, *axis_main, *axis_x);
    const CubicBezier3 scaled_curve{
        scaled_controls[0],
        scaled_controls[1],
        scaled_controls[2],
        scaled_controls[3]};
    const auto scaled_surface =
        scaled_axis
            ? CubicBezierRevolutionSurface3::make(
                  scaled_curve, *scaled_axis, sweep)
            : std::expected<
                  CubicBezierRevolutionSurface3,
                  RevolutionSurfaceConstructionError>{
                  std::unexpected{
                      RevolutionSurfaceConstructionError::
                          non_finite_rotated_control}};
    if (!scaled_surface) {
        return 1;
    }
    const auto scale_value = surface->evaluate(0.3, 0.7);
    const auto scaled_value = scaled_surface->evaluate(0.3, 0.7);
    const auto scale_first = surface->first_derivatives(0.3, 0.7);
    const auto scaled_first =
        scaled_surface->first_derivatives(0.3, 0.7);
    const auto scale_second = surface->second_derivatives(0.3, 0.7);
    const auto scaled_second =
        scaled_surface->second_derivatives(0.3, 0.7);
    passed = require(
                 scale_value && scaled_value &&
                     close_scalar(
                         scaled_value->x(),
                         scale * scale_value->x(),
                         1024.0) &&
                     close_scalar(
                         scaled_value->y(),
                         scale * scale_value->y(),
                         1024.0) &&
                     close_scalar(
                         scaled_value->z(),
                         scale * scale_value->z(),
                         1024.0) &&
                     scale_first && scaled_first &&
                     close_scalar(
                         scaled_first->u.x(),
                         scale * scale_first->u.x(),
                         4096.0) &&
                     close_scalar(
                         scaled_first->v.y(),
                         scale * scale_first->v.y(),
                         4096.0) &&
                     scale_second && scaled_second &&
                     close_scalar(
                         scaled_second->uu.z(),
                         scale * scale_second->uu.z(),
                         16384.0) &&
                     close_scalar(
                         scaled_second->uv.x(),
                         scale * scale_second->uv.x(),
                         16384.0) &&
                     close_scalar(
                         scaled_second->vv.y(),
                         scale * scale_second->vv.y(),
                         16384.0),
                 "revolution power-of-two scale covariance differs") &&
             passed;

    constexpr double large = 1.0e140;
    const auto large0 = Point3::make(large + 2.0e130, large, large);
    const auto large1 = Point3::make(
        large + 2.0e130, large, large + 1.0e130);
    const auto large2 = Point3::make(
        large + 2.0e130, large, large + 2.0e130);
    const auto large3 = Point3::make(
        large + 2.0e130, large, large + 3.0e130);
    const auto large_origin = Point3::make(large, large, large);
    if (!large0 || !large1 || !large2 || !large3 || !large_origin) {
        return 1;
    }
    const auto large_axis = AxisPlacement3::make(
        *large_origin,
        *Vector3::make(0.0, 0.0, 1.0),
        *Vector3::make(1.0, 0.0, 0.0));
    const CubicBezier3 large_curve{
        *large0, *large1, *large2, *large3};
    const auto large_surface =
        large_axis
            ? CubicBezierRevolutionSurface3::make(
                  large_curve, *large_axis, 0.5)
            : std::expected<
                  CubicBezierRevolutionSurface3,
                  RevolutionSurfaceConstructionError>{
                  std::unexpected{
                      RevolutionSurfaceConstructionError::
                          non_finite_rotated_control}};
    const auto large_value =
        large_surface ? large_surface->evaluate(0.5, 0.5)
                      : std::expected<Point3, SurfaceError>{
                            std::unexpected{
                                SurfaceError::non_finite_result}};
    passed = require(
                 large_surface && large_value &&
                     std::isfinite(large_value->x()) &&
                     std::isfinite(large_value->y()) &&
                     std::isfinite(large_value->z()),
                 "revolution extreme finite fixture failed unexpectedly") &&
             passed;

    const double maximum = std::numeric_limits<double>::max();
    const auto huge0 = Point3::make(maximum, maximum, 0.0);
    const auto huge1 = Point3::make(maximum, maximum, 1.0);
    const auto huge2 = Point3::make(maximum, maximum, 2.0);
    const auto huge3 = Point3::make(maximum, maximum, 3.0);
    if (!huge0 || !huge1 || !huge2 || !huge3) {
        return 1;
    }
    const CubicBezier3 huge_curve{
        *huge0, *huge1, *huge2, *huge3};
    const auto huge_surface =
        CubicBezierRevolutionSurface3::make(
            huge_curve, identity_axis, std::numbers::pi_v<double> / 4.0);
    passed = require(
                 !huge_surface &&
                     huge_surface.error() ==
                         RevolutionSurfaceConstructionError::
                             non_finite_rotated_control,
                 "revolution non-representable end curve did not fail explicitly") &&
             passed;

    const auto repeat_value_a = surface->evaluate(0.375, 0.625);
    const auto repeat_value_b = surface->evaluate(0.375, 0.625);
    const auto repeat_first_a =
        surface->first_derivatives(0.375, 0.625);
    const auto repeat_first_b =
        surface->first_derivatives(0.375, 0.625);
    const auto repeat_second_a =
        surface->second_derivatives(0.375, 0.625);
    const auto repeat_second_b =
        surface->second_derivatives(0.375, 0.625);
    const auto repeat_failure_a = surface->evaluate(1.1, 0.5);
    const auto repeat_failure_b = surface->evaluate(1.1, 0.5);
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
                 "revolution repeated evidence is not deterministic") &&
             passed;

    (void)infinity;
    return passed ? 0 : 1;
}
