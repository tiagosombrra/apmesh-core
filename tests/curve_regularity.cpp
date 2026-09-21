#include "apmesh/geometry/curve.hpp"
#include "geometry/detail/curve_regularity_interval.hpp"

#include <array>
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

bool contains(
    const apmesh::core::detail::ClosedInterval& interval,
    const long double exact) {
    return static_cast<long double>(interval.lower) <= exact &&
           exact <= static_cast<long double>(interval.upper);
}

template <typename Value, typename Error>
bool require_error(
    const std::expected<Value, Error>& value,
    const Error expected,
    const std::string_view message) {
    return require(!value.has_value() && value.error() == expected, message);
}

} // namespace

int main() {
    using apmesh::core::CubicBezier2;
    using apmesh::core::CubicBezier3;
    using apmesh::core::CurveRegularityError;
    using apmesh::core::CurveRegularityPolicy;
    using apmesh::core::CurveRegularityResult;
    using apmesh::core::Point2;
    using apmesh::core::Point3;
    namespace detail = apmesh::core::detail;

    bool passed = true;

    const auto one = detail::point_interval(1.0);
    const auto two = detail::point_interval(2.0);
    if (!one || !two) {
        return 1;
    }
    const auto sum = detail::add(*one, *two);
    const auto difference = detail::subtract(*one, *two);
    const auto product = detail::multiply(*one, *two);
    const auto quotient = detail::divide_positive(*two, 3.0);
    const auto middle = detail::midpoint(*one, *two);
    passed = require(sum && contains(*sum, 3.0L), "interval addition lost exact result") && passed;
    passed = require(
                 difference && contains(*difference, -1.0L),
                 "interval subtraction lost exact result") &&
             passed;
    passed = require(
                 product && contains(*product, 2.0L),
                 "interval multiplication lost exact result") &&
             passed;
    passed = require(
                 quotient && contains(*quotient, 2.0L / 3.0L),
                 "interval division lost exact result") &&
             passed;
    passed = require(
                 middle && contains(*middle, 1.5L),
                 "interval midpoint lost exact result") &&
             passed;

    const std::array<detail::ClosedInterval, 5> constant_coefficients{
        *two, *two, *two, *two, *two};
    const auto constant_children =
        detail::subdivide_quartic_midpoint(constant_coefficients);
    passed = require(
                 constant_children.has_value(),
                 "constant quartic subdivision failed") &&
             passed;
    if (constant_children) {
        for (const auto& coefficient : constant_children->first) {
            passed = require(
                         contains(coefficient, 2.0L),
                         "left subdivision lost constant polynomial") &&
                     passed;
        }
        for (const auto& coefficient : constant_children->second) {
            passed = require(
                         contains(coefficient, 2.0L),
                         "right subdivision lost constant polynomial") &&
                     passed;
        }
    }

    const CurveRegularityPolicy deep_policy{
        .max_subdivision_depth = 32,
        .max_processed_nodes = 4096,
    };
    const CurveRegularityPolicy coarse_policy{
        .max_subdivision_depth = 0,
        .max_processed_nodes = 1,
    };

    const auto a20 = Point2::make(0.0, 0.0);
    const auto a21 = Point2::make(1.0, 0.0);
    const auto a22 = Point2::make(2.0, 0.0);
    const auto a23 = Point2::make(3.0, 0.0);
    const auto a30 = Point3::make(0.0, 0.0, 0.0);
    const auto a31 = Point3::make(1.0, 0.0, 0.0);
    const auto a32 = Point3::make(2.0, 0.0, 0.0);
    const auto a33 = Point3::make(3.0, 0.0, 0.0);
    if (!a20 || !a21 || !a22 || !a23 || !a30 || !a31 || !a32 || !a33) {
        return 1;
    }

    const CubicBezier2 line2{*a20, *a21, *a22, *a23};
    const CubicBezier3 line3{*a30, *a31, *a32, *a33};
    const auto line2_result = line2.certify_regularity(deep_policy);
    const auto line3_result = line3.certify_regularity(deep_policy);
    passed = require(
                 line2_result &&
                     line2_result->result == CurveRegularityResult::regular,
                 "regular 2D line did not certify") &&
             passed;
    passed = require(
                 line3_result &&
                     line3_result->result == CurveRegularityResult::regular,
                 "regular 3D line did not certify") &&
             passed;

    const auto singular2 = Point2::make(0.0, 1.0);
    const auto singular2_end = Point2::make(2.0, 1.0);
    if (!singular2 || !singular2_end) {
        return 1;
    }
    const CubicBezier2 endpoint_singular{
        *singular2, *singular2, *singular2_end, *singular2_end};
    const auto endpoint_result = endpoint_singular.certify_regularity(deep_policy);
    passed = require(
                 endpoint_result &&
                     endpoint_result->result == CurveRegularityResult::degenerate &&
                     endpoint_result->processed_nodes == 0,
                 "endpoint singularity was not certified exactly") &&
             passed;

    const auto i0 = Point2::make(0.0, 0.0);
    const auto i1 = Point2::make(1.0, 0.0);
    const auto i2 = Point2::make(0.0, 0.0);
    const auto i3 = Point2::make(1.0, 0.0);
    if (!i0 || !i1 || !i2 || !i3) {
        return 1;
    }
    const CubicBezier2 interior_stationary{*i0, *i1, *i2, *i3};
    const auto stationary_result =
        interior_stationary.certify_regularity(deep_policy);
    passed = require(
                 stationary_result &&
                     stationary_result->result != CurveRegularityResult::regular,
                 "interior stationary fixture was falsely certified regular") &&
             passed;

    constexpr double epsilon = 0x1p-10;
    const auto n0 = Point2::make(0.0, 0.0);
    const auto n1 = Point2::make(1.0, epsilon);
    const auto n2 = Point2::make(1.0, 2.0 * epsilon);
    const auto n3 = Point2::make(0.0, 3.0 * epsilon);
    const auto n30 = Point3::make(0.0, 0.0, 0.0);
    const auto n31 = Point3::make(1.0, epsilon, 0.0);
    const auto n32 = Point3::make(1.0, 2.0 * epsilon, 0.0);
    const auto n33 = Point3::make(0.0, 3.0 * epsilon, 0.0);
    if (!n0 || !n1 || !n2 || !n3 || !n30 || !n31 || !n32 || !n33) {
        return 1;
    }

    const CubicBezier2 near_stationary2{*n0, *n1, *n2, *n3};
    const CubicBezier3 near_stationary3{*n30, *n31, *n32, *n33};
    const auto coarse_result = near_stationary2.certify_regularity(coarse_policy);
    const auto deep_result2 = near_stationary2.certify_regularity(deep_policy);
    const auto deep_result3 = near_stationary3.certify_regularity(deep_policy);
    passed = require(
                 coarse_result &&
                     coarse_result->result == CurveRegularityResult::indeterminate,
                 "coarse policy should remain indeterminate") &&
             passed;
    passed = require(
                 deep_result2 &&
                     deep_result2->result == CurveRegularityResult::regular,
                 "near-stationary 2D regular curve did not certify") &&
             passed;
    passed = require(
                 deep_result3 &&
                     deep_result3->result == CurveRegularityResult::regular,
                 "2D/3D parity regularity result differs") &&
             passed;

    const auto reversed_result =
        near_stationary2.reversed().certify_regularity(deep_policy);
    passed = require(
                 reversed_result && deep_result2 &&
                     reversed_result->result == deep_result2->result,
                 "reversal changed regularity classification") &&
             passed;

    const auto t0 = Point2::make(8.0, -4.0);
    const auto t1 = Point2::make(9.0, -4.0 + epsilon);
    const auto t2 = Point2::make(9.0, -4.0 + 2.0 * epsilon);
    const auto t3 = Point2::make(8.0, -4.0 + 3.0 * epsilon);
    if (!t0 || !t1 || !t2 || !t3) {
        return 1;
    }
    const CubicBezier2 translated{*t0, *t1, *t2, *t3};
    const auto translated_result = translated.certify_regularity(deep_policy);
    passed = require(
                 translated_result && deep_result2 &&
                     translated_result->result == deep_result2->result,
                 "translation changed regularity classification") &&
             passed;

    const auto s0 = Point2::make(0.0, 0.0);
    const auto s1 = Point2::make(2.0, 2.0 * epsilon);
    const auto s2 = Point2::make(2.0, 4.0 * epsilon);
    const auto s3 = Point2::make(0.0, 6.0 * epsilon);
    if (!s0 || !s1 || !s2 || !s3) {
        return 1;
    }
    const CubicBezier2 scaled{*s0, *s1, *s2, *s3};
    const auto scaled_result = scaled.certify_regularity(deep_policy);
    passed = require(
                 scaled_result && deep_result2 &&
                     scaled_result->result == deep_result2->result,
                 "power-of-two scale changed regularity classification") &&
             passed;

    const auto repeat_result = near_stationary2.certify_regularity(deep_policy);
    passed = require(
                 repeat_result && deep_result2 && *repeat_result == *deep_result2,
                 "regularity evidence is not deterministic") &&
             passed;

    const CurveRegularityPolicy zero_nodes{
        .max_subdivision_depth = 1,
        .max_processed_nodes = 0,
    };
    const CurveRegularityPolicy excessive_depth{
        .max_subdivision_depth = 65,
        .max_processed_nodes = 1,
    };
    passed = require_error(
                 near_stationary2.certify_regularity(zero_nodes),
                 CurveRegularityError::invalid_policy,
                 "zero-node policy was not rejected") &&
             passed;
    passed = require_error(
                 near_stationary2.certify_regularity(excessive_depth),
                 CurveRegularityError::invalid_policy,
                 "unsupported depth policy was not rejected") &&
             passed;

    const double maximum = std::numeric_limits<double>::max();
    const auto e0 = Point2::make(maximum, 0.0);
    const auto e1 = Point2::make(-maximum, 0.0);
    const auto e2 = Point2::make(maximum, 1.0);
    const auto e3 = Point2::make(-maximum, 1.0);
    if (!e0 || !e1 || !e2 || !e3) {
        return 1;
    }
    const CubicBezier2 extreme{*e0, *e1, *e2, *e3};
    passed = require_error(
                 extreme.certify_regularity(deep_policy),
                 CurveRegularityError::non_finite_enclosure,
                 "overflowing enclosure did not fail explicitly") &&
             passed;
    passed = require_error(
                 extreme.certify_regularity(zero_nodes),
                 CurveRegularityError::invalid_policy,
                 "policy validation did not precede enclosure construction") &&
             passed;

    return passed ? 0 : 1;
}
