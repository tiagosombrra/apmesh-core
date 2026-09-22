#include "apmesh/geometry/parametric_curve.hpp"
#include "apmesh/geometry/curve.hpp"

#include "apmesh/core/numeric.hpp"

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
    const double reference_scale = 1.0) {
    const apmesh::core::ProximityPolicy policy{
        .absolute_tolerance = 2.0e-14 * reference_scale,
        .relative_tolerance = 2.0e-14,
        .reference_scale = reference_scale,
    };
    const auto comparison =
        apmesh::core::compare_proximity(lhs, rhs, policy);
    return comparison.has_value() &&
           comparison->result == apmesh::core::ProximityResult::within;
}

bool close_point(
    const apmesh::core::Point2& lhs,
    const apmesh::core::Point2& rhs,
    const double reference_scale = 1.0) {
    return close_scalar(lhs.x(), rhs.x(), reference_scale) &&
           close_scalar(lhs.y(), rhs.y(), reference_scale);
}

bool close_point(
    const apmesh::core::Point3& lhs,
    const apmesh::core::Point3& rhs,
    const double reference_scale = 1.0) {
    return close_scalar(lhs.x(), rhs.x(), reference_scale) &&
           close_scalar(lhs.y(), rhs.y(), reference_scale) &&
           close_scalar(lhs.z(), rhs.z(), reference_scale);
}

bool close_vector(
    const apmesh::core::Vector2& lhs,
    const apmesh::core::Vector2& rhs,
    const double reference_scale = 1.0) {
    return close_scalar(lhs.x(), rhs.x(), reference_scale) &&
           close_scalar(lhs.y(), rhs.y(), reference_scale);
}

bool close_vector(
    const apmesh::core::Vector3& lhs,
    const apmesh::core::Vector3& rhs,
    const double reference_scale = 1.0) {
    return close_scalar(lhs.x(), rhs.x(), reference_scale) &&
           close_scalar(lhs.y(), rhs.y(), reference_scale) &&
           close_scalar(lhs.z(), rhs.z(), reference_scale);
}

struct IncompleteCurve {};

} // namespace

int main() {
    using apmesh::core::BoundedParametricCurve2;
    using apmesh::core::BoundedParametricCurve3;
    using apmesh::core::CubicBezier2;
    using apmesh::core::CubicBezier3;
    using apmesh::core::CurveError;
    using apmesh::core::CurveParameterDomain;
    using apmesh::core::CurveParameterDomainError;
    using apmesh::core::Point2;
    using apmesh::core::Point3;
    using apmesh::core::reversed_parameter;

    static_assert(BoundedParametricCurve2<CubicBezier2>);
    static_assert(BoundedParametricCurve3<CubicBezier3>);
    static_assert(!BoundedParametricCurve2<IncompleteCurve>);
    static_assert(!BoundedParametricCurve3<IncompleteCurve>);

    bool passed = true;

    const auto non_finite_lower = CurveParameterDomain::make(
        std::numeric_limits<double>::quiet_NaN(),
        1.0);
    passed = require(
                 !non_finite_lower &&
                     non_finite_lower.error() ==
                         CurveParameterDomainError::non_finite_lower_bound,
                 "non-finite lower bound was not rejected") &&
             passed;

    const auto non_finite_upper = CurveParameterDomain::make(
        0.0,
        std::numeric_limits<double>::infinity());
    passed = require(
                 !non_finite_upper &&
                     non_finite_upper.error() ==
                         CurveParameterDomainError::non_finite_upper_bound,
                 "non-finite upper bound was not rejected") &&
             passed;

    const auto reversed_interval = CurveParameterDomain::make(2.0, -1.0);
    passed = require(
                 !reversed_interval &&
                     reversed_interval.error() ==
                         CurveParameterDomainError::reversed_interval,
                 "reversed interval was not rejected") &&
             passed;

    const auto zero_width = CurveParameterDomain::make(-0.0, 0.0);
    passed = require(
                 !zero_width &&
                     zero_width.error() ==
                         CurveParameterDomainError::zero_width_interval,
                 "zero-width interval was not rejected") &&
             passed;

    const auto general_domain = CurveParameterDomain::make(-2.0, 6.0);
    if (!general_domain) {
        return 1;
    }

    const auto lower_contained = general_domain->contains(-2.0);
    const auto upper_contained = general_domain->contains(6.0);
    const auto outside_contained = general_domain->contains(6.5);
    const auto non_finite_contained = general_domain->contains(
        std::numeric_limits<double>::quiet_NaN());
    passed = require(lower_contained && *lower_contained,
                     "lower endpoint containment differs") &&
             passed;
    passed = require(upper_contained && *upper_contained,
                     "upper endpoint containment differs") &&
             passed;
    passed = require(outside_contained && !*outside_contained,
                     "finite out-of-domain containment differs") &&
             passed;
    passed = require(
                 !non_finite_contained &&
                     non_finite_contained.error() ==
                         CurveError::non_finite_parameter,
                 "non-finite containment did not preserve typed failure") &&
             passed;

    const auto reversed_lower = reversed_parameter(*general_domain, -2.0);
    const auto reversed_upper = reversed_parameter(*general_domain, 6.0);
    const auto reversed_interior = reversed_parameter(*general_domain, 1.5);
    const auto reversed_outside = reversed_parameter(*general_domain, 8.0);
    const auto reversed_non_finite = reversed_parameter(
        *general_domain,
        std::numeric_limits<double>::infinity());
    passed = require(reversed_lower && *reversed_lower == 6.0,
                     "reversal did not exchange lower endpoint") &&
             passed;
    passed = require(reversed_upper && *reversed_upper == -2.0,
                     "reversal did not exchange upper endpoint") &&
             passed;
    passed = require(reversed_interior && *reversed_interior == 2.5,
                     "non-normalized interval reversal differs") &&
             passed;
    passed = require(
                 !reversed_outside &&
                     reversed_outside.error() ==
                         CurveError::parameter_out_of_domain,
                 "out-of-domain reversal did not fail explicitly") &&
             passed;
    passed = require(
                 !reversed_non_finite &&
                     reversed_non_finite.error() ==
                         CurveError::non_finite_parameter,
                 "non-finite reversal did not fail explicitly") &&
             passed;

    const auto extreme_domain = CurveParameterDomain::make(
        -std::numeric_limits<double>::max(),
        std::numeric_limits<double>::max());
    if (!extreme_domain) {
        return 1;
    }
    const auto extreme_midpoint = reversed_parameter(*extreme_domain, 0.0);
    passed = require(
                 extreme_midpoint && std::isfinite(*extreme_midpoint) &&
                     *extreme_midpoint == 0.0,
                 "extreme finite reversal introduced overflow") &&
             passed;

    const auto p20 = Point2::make(0.0, 0.0);
    const auto p21 = Point2::make(1.0, 3.0);
    const auto p22 = Point2::make(4.0, -2.0);
    const auto p23 = Point2::make(6.0, 1.0);
    const auto p30 = Point3::make(0.0, 0.0, 0.0);
    const auto p31 = Point3::make(1.0, 3.0, -1.0);
    const auto p32 = Point3::make(4.0, -2.0, 2.0);
    const auto p33 = Point3::make(6.0, 1.0, 4.0);
    if (!p20 || !p21 || !p22 || !p23 || !p30 || !p31 || !p32 || !p33) {
        return 1;
    }

    const CubicBezier2 curve2{*p20, *p21, *p22, *p23};
    const CubicBezier3 curve3{*p30, *p31, *p32, *p33};

    const auto domain2 = curve2.parameter_domain();
    const auto domain3 = curve3.parameter_domain();
    passed = require(
                 domain2.lower() == 0.0 && domain2.upper() == 1.0 &&
                     domain3.lower() == 0.0 && domain3.upper() == 1.0,
                 "cubic Bezier domain is not exactly [0,1]") &&
             passed;

    const auto map0 = reversed_parameter(domain2, 0.0);
    const auto map1 = reversed_parameter(domain2, 1.0);
    const auto map_quarter = reversed_parameter(domain2, 0.25);
    passed = require(map0 && *map0 == 1.0 && map1 && *map1 == 0.0 &&
                         map_quarter && *map_quarter == 0.75,
                     "cubic Bezier reversal parameter mapping differs") &&
             passed;

    const auto reversed2 = curve2.reversed();
    const auto reversed3 = curve3.reversed();
    passed = require(reversed2.reversed() == curve2,
                     "2D reversal involution differs") &&
             passed;
    passed = require(reversed3.reversed() == curve3,
                     "3D reversal involution differs") &&
             passed;

    constexpr double parameter = 0.25;
    constexpr double reverse_parameter_value = 0.75;

    const auto value2 = curve2.evaluate(parameter);
    const auto reverse_value2 = reversed2.evaluate(reverse_parameter_value);
    const auto value3 = curve3.evaluate(parameter);
    const auto reverse_value3 = reversed3.evaluate(reverse_parameter_value);
    passed = require(value2 && reverse_value2 &&
                         close_point(*value2, *reverse_value2, 8.0),
                     "2D reversal evaluation covariance differs") &&
             passed;
    passed = require(value3 && reverse_value3 &&
                         close_point(*value3, *reverse_value3, 8.0),
                     "3D reversal evaluation covariance differs") &&
             passed;

    const auto d12 = curve2.first_derivative(parameter);
    const auto reverse_d12 =
        reversed2.first_derivative(reverse_parameter_value);
    const auto d13 = curve3.first_derivative(parameter);
    const auto reverse_d13 =
        reversed3.first_derivative(reverse_parameter_value);
    passed = require(
                 d12 && reverse_d12 &&
                     close_scalar(reverse_d12->x(), -d12->x(), 32.0) &&
                     close_scalar(reverse_d12->y(), -d12->y(), 32.0),
                 "2D first-derivative reversal covariance differs") &&
             passed;
    passed = require(
                 d13 && reverse_d13 &&
                     close_scalar(reverse_d13->x(), -d13->x(), 32.0) &&
                     close_scalar(reverse_d13->y(), -d13->y(), 32.0) &&
                     close_scalar(reverse_d13->z(), -d13->z(), 32.0),
                 "3D first-derivative reversal covariance differs") &&
             passed;

    const auto d22 = curve2.second_derivative(parameter);
    const auto reverse_d22 =
        reversed2.second_derivative(reverse_parameter_value);
    const auto d23 = curve3.second_derivative(parameter);
    const auto reverse_d23 =
        reversed3.second_derivative(reverse_parameter_value);
    passed = require(d22 && reverse_d22 &&
                         close_vector(*d22, *reverse_d22, 64.0),
                     "2D second-derivative reversal covariance differs") &&
             passed;
    passed = require(d23 && reverse_d23 &&
                         close_vector(*d23, *reverse_d23, 64.0),
                     "3D second-derivative reversal covariance differs") &&
             passed;

    const auto nan_value = curve2.evaluate(
        std::numeric_limits<double>::quiet_NaN());
    const auto below_value = curve2.evaluate(-0.25);
    const auto above_derivative = curve3.first_derivative(1.25);
    passed = require(
                 !nan_value &&
                     nan_value.error() == CurveError::non_finite_parameter,
                 "existing non-finite query failure changed") &&
             passed;
    passed = require(
                 !below_value &&
                     below_value.error() == CurveError::parameter_out_of_domain,
                 "existing below-domain query failure changed") &&
             passed;
    passed = require(
                 !above_derivative &&
                     above_derivative.error() ==
                         CurveError::parameter_out_of_domain,
                 "existing derivative domain failure changed") &&
             passed;

    return passed ? 0 : 1;
}
