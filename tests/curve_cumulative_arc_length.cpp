#include "apmesh/geometry/curve.hpp"

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

template <typename Value, typename Error>
bool require_error(
    const std::expected<Value, Error>& value,
    const Error expected,
    const std::string_view message) {
    return require(!value.has_value() && value.error() == expected, message);
}

bool ordered_finite(const apmesh::core::CurveLengthEvidence& evidence) {
    return std::isfinite(evidence.lower_length) &&
           std::isfinite(evidence.upper_length) &&
           evidence.lower_length >= 0.0 &&
           evidence.lower_length <= evidence.upper_length;
}

bool contains(
    const apmesh::core::CurveLengthEvidence& evidence,
    const long double value) {
    return static_cast<long double>(evidence.lower_length) <= value &&
           value <= static_cast<long double>(evidence.upper_length);
}

bool overlaps(
    const apmesh::core::CurveLengthEvidence& evidence,
    const long double lower,
    const long double upper) {
    return static_cast<long double>(evidence.lower_length) <= upper &&
           lower <= static_cast<long double>(evidence.upper_length);
}

long double parabola_prefix_length(const long double parameter) {
    return parameter * std::sqrt(1.0L + 4.0L * parameter * parameter) / 2.0L +
           std::asinh(2.0L * parameter) / 4.0L;
}

} // namespace

int main() {
    using apmesh::core::CubicBezier2;
    using apmesh::core::CubicBezier3;
    using apmesh::core::CurveLengthError;
    using apmesh::core::CurveLengthPolicy;
    using apmesh::core::CurveLengthResult;
    using apmesh::core::Point2;
    using apmesh::core::Point3;

    bool passed = true;

    const CurveLengthPolicy tight_policy{
        .absolute_tolerance = 1.0e-8,
        .relative_tolerance = 1.0e-8,
        .max_subdivision_depth = 24,
        .max_processed_nodes = 32768,
    };
    const CurveLengthPolicy zero_tolerance_policy{
        .absolute_tolerance = 0.0,
        .relative_tolerance = 0.0,
        .max_subdivision_depth = 8,
        .max_processed_nodes = 512,
    };
    const CurveLengthPolicy coarse_policy{
        .absolute_tolerance = 0.0,
        .relative_tolerance = 0.0,
        .max_subdivision_depth = 24,
        .max_processed_nodes = 1,
    };

    const auto l20 = Point2::make(0.0, 0.0);
    const auto l21 = Point2::make(1.0, 0.0);
    const auto l22 = Point2::make(2.0, 0.0);
    const auto l23 = Point2::make(3.0, 0.0);
    const auto l30 = Point3::make(0.0, 0.0, 0.0);
    const auto l31 = Point3::make(1.0, 0.0, 0.0);
    const auto l32 = Point3::make(2.0, 0.0, 0.0);
    const auto l33 = Point3::make(3.0, 0.0, 0.0);
    if (!l20 || !l21 || !l22 || !l23 || !l30 || !l31 || !l32 || !l33) {
        return 1;
    }

    const CubicBezier2 line2{*l20, *l21, *l22, *l23};
    const CubicBezier3 line3{*l30, *l31, *l32, *l33};

    const auto zero2 =
        line2.cumulative_arc_length_enclosure(0.0, zero_tolerance_policy);
    const auto zero3 =
        line3.cumulative_arc_length_enclosure(0.0, zero_tolerance_policy);
    passed = require(
                 zero2 && zero3 &&
                     zero2->result == CurveLengthResult::converged &&
                     zero3->result == CurveLengthResult::converged &&
                     zero2->lower_length == 0.0 &&
                     zero2->upper_length == 0.0 &&
                     zero3->lower_length == 0.0 &&
                     zero3->upper_length == 0.0,
                 "t=0 did not return exact zero cumulative length") &&
             passed;

    const auto total2 = line2.arc_length_enclosure(zero_tolerance_policy);
    const auto endpoint2 =
        line2.cumulative_arc_length_enclosure(1.0, zero_tolerance_policy);
    const auto total3 = line3.arc_length_enclosure(zero_tolerance_policy);
    const auto endpoint3 =
        line3.cumulative_arc_length_enclosure(1.0, zero_tolerance_policy);
    passed = require(
                 total2 && endpoint2 && *total2 == *endpoint2 &&
                     total3 && endpoint3 && *total3 == *endpoint3,
                 "t=1 cumulative evidence differs from total-length evidence") &&
             passed;

    const auto half_line2 =
        line2.cumulative_arc_length_enclosure(0.5, tight_policy);
    const auto half_line3 =
        line3.cumulative_arc_length_enclosure(0.5, tight_policy);
    passed = require(
                 half_line2 && half_line3 &&
                     half_line2->result == CurveLengthResult::converged &&
                     half_line3->result == CurveLengthResult::converged &&
                     contains(*half_line2, 1.5L) &&
                     contains(*half_line3, 1.5L),
                 "axis-line cumulative enclosure missed exact analytic S(0.5)") &&
             passed;

    const auto quarter_line =
        line2.cumulative_arc_length_enclosure(0.25, tight_policy);
    const auto three_quarter_line =
        line2.cumulative_arc_length_enclosure(0.75, tight_policy);
    passed = require(
                 quarter_line && half_line2 && three_quarter_line &&
                     quarter_line->upper_length <= half_line2->lower_length &&
                     half_line2->upper_length <= three_quarter_line->lower_length,
                 "safe line fixture contradicted cumulative monotonicity") &&
             passed;

    const auto constant2_point = Point2::make(-4.0, 8.0);
    const auto constant3_point = Point3::make(2.0, -4.0, 8.0);
    if (!constant2_point || !constant3_point) {
        return 1;
    }
    const CubicBezier2 constant2{
        *constant2_point, *constant2_point, *constant2_point, *constant2_point};
    const CubicBezier3 constant3{
        *constant3_point, *constant3_point, *constant3_point, *constant3_point};
    for (const double parameter : {0.0, 0.25, 0.5, 0.75, 1.0}) {
        const auto cumulative2 =
            constant2.cumulative_arc_length_enclosure(parameter, zero_tolerance_policy);
        const auto cumulative3 =
            constant3.cumulative_arc_length_enclosure(parameter, zero_tolerance_policy);
        passed = require(
                     cumulative2 && cumulative3 &&
                         cumulative2->result == CurveLengthResult::converged &&
                         cumulative3->result == CurveLengthResult::converged &&
                         cumulative2->lower_length == 0.0 &&
                         cumulative2->upper_length == 0.0 &&
                         cumulative3->lower_length == 0.0 &&
                         cumulative3->upper_length == 0.0,
                     "constant curve cumulative length was not exact zero") &&
                 passed;
    }

    const auto p0 = Point2::make(0.0, 0.0);
    const auto p1 = Point2::make(1.0 / 3.0, 0.0);
    const auto p2 = Point2::make(2.0 / 3.0, 1.0 / 3.0);
    const auto p3 = Point2::make(1.0, 1.0);
    const auto p30 = Point3::make(0.0, 0.0, 0.0);
    const auto p31 = Point3::make(1.0 / 3.0, 0.0, 0.0);
    const auto p32 = Point3::make(2.0 / 3.0, 1.0 / 3.0, 0.0);
    const auto p33 = Point3::make(1.0, 1.0, 0.0);
    if (!p0 || !p1 || !p2 || !p3 || !p30 || !p31 || !p32 || !p33) {
        return 1;
    }

    const CubicBezier2 parabola2{*p0, *p1, *p2, *p3};
    const CubicBezier3 parabola3{*p30, *p31, *p32, *p33};
    const double prefix_parameter = 0.5;
    const long double prefix_reference =
        parabola_prefix_length(static_cast<long double>(prefix_parameter));
    const auto parabola2_prefix =
        parabola2.cumulative_arc_length_enclosure(prefix_parameter, tight_policy);
    const auto parabola3_prefix =
        parabola3.cumulative_arc_length_enclosure(prefix_parameter, tight_policy);
    passed = require(
                 parabola2_prefix &&
                     parabola2_prefix->result == CurveLengthResult::converged &&
                     ordered_finite(*parabola2_prefix) &&
                     contains(*parabola2_prefix, prefix_reference),
                 "2D parabola prefix enclosure missed analytic integral") &&
             passed;
    passed = require(
                 parabola3_prefix &&
                     parabola3_prefix->result == CurveLengthResult::converged &&
                     ordered_finite(*parabola3_prefix) &&
                     contains(*parabola3_prefix, prefix_reference),
                 "3D parabola prefix enclosure missed analytic integral") &&
             passed;
    passed = require(
                 parabola2_prefix && parabola3_prefix &&
                     *parabola2_prefix == *parabola3_prefix,
                 "2D/3D planar embedding changed cumulative evidence") &&
             passed;

    const double reverse_parameter = 0.375;
    const auto total_parabola =
        parabola2.arc_length_enclosure(tight_policy);
    const auto forward_complement =
        parabola2.cumulative_arc_length_enclosure(
            1.0 - reverse_parameter,
            tight_policy);
    const auto reversed_prefix =
        parabola2.reversed().cumulative_arc_length_enclosure(
            reverse_parameter,
            tight_policy);
    if (total_parabola && forward_complement && reversed_prefix) {
        const long double difference_lower = std::max(
            0.0L,
            static_cast<long double>(total_parabola->lower_length) -
                static_cast<long double>(forward_complement->upper_length));
        const long double difference_upper = std::max(
            0.0L,
            static_cast<long double>(total_parabola->upper_length) -
                static_cast<long double>(forward_complement->lower_length));
        passed = require(
                     overlaps(*reversed_prefix, difference_lower, difference_upper),
                     "reversal cumulative enclosures contradict L-S(1-t)") &&
                 passed;
    } else {
        passed = require(false, "reversal cumulative evidence was unavailable") && passed;
    }

    const auto b0 = Point2::make(0.0, 0.0);
    const auto b1 = Point2::make(1.0, 2.0);
    const auto b2 = Point2::make(3.0, 1.0);
    const auto b3 = Point2::make(4.0, 0.0);
    const auto t0 = Point2::make(8.0, -4.0);
    const auto t1 = Point2::make(9.0, -2.0);
    const auto t2 = Point2::make(11.0, -3.0);
    const auto t3 = Point2::make(12.0, -4.0);
    if (!b0 || !b1 || !b2 || !b3 || !t0 || !t1 || !t2 || !t3) {
        return 1;
    }
    const CubicBezier2 base_curve{*b0, *b1, *b2, *b3};
    const CubicBezier2 translated_curve{*t0, *t1, *t2, *t3};
    const auto base_prefix =
        base_curve.cumulative_arc_length_enclosure(0.625, tight_policy);
    const auto translated_prefix =
        translated_curve.cumulative_arc_length_enclosure(0.625, tight_policy);
    passed = require(
                 base_prefix && translated_prefix &&
                     *base_prefix == *translated_prefix,
                 "exact translation changed cumulative evidence") &&
             passed;

    const auto s0 = Point2::make(0.0, 0.0);
    const auto s1 = Point2::make(2.0, 4.0);
    const auto s2 = Point2::make(6.0, 2.0);
    const auto s3 = Point2::make(8.0, 0.0);
    if (!s0 || !s1 || !s2 || !s3) {
        return 1;
    }
    const CubicBezier2 scaled_curve{*s0, *s1, *s2, *s3};
    const CurveLengthPolicy scaled_policy{
        .absolute_tolerance = 2.0 * tight_policy.absolute_tolerance,
        .relative_tolerance = tight_policy.relative_tolerance,
        .max_subdivision_depth = tight_policy.max_subdivision_depth,
        .max_processed_nodes = tight_policy.max_processed_nodes,
    };
    const auto scaled_prefix =
        scaled_curve.cumulative_arc_length_enclosure(0.625, scaled_policy);
    passed = require(
                 base_prefix && scaled_prefix &&
                     base_prefix->result == scaled_prefix->result &&
                     scaled_prefix->lower_length <=
                         2.0 * base_prefix->upper_length &&
                     scaled_prefix->upper_length >=
                         2.0 * base_prefix->lower_length,
                 "power-of-two scale cumulative enclosure differs") &&
             passed;

    const auto st0 = Point2::make(0.0, 0.0);
    const auto st1 = Point2::make(1.0, 0.0);
    const auto st2 = Point2::make(1.0, 0.0);
    const auto st3 = Point2::make(0.0, 0.0);
    if (!st0 || !st1 || !st2 || !st3) {
        return 1;
    }
    const CubicBezier2 stationary{*st0, *st1, *st2, *st3};
    const auto stationary_prefix =
        stationary.cumulative_arc_length_enclosure(0.75, tight_policy);
    passed = require(
                 stationary_prefix && ordered_finite(*stationary_prefix) &&
                     stationary_prefix->upper_length > 0.0,
                 "stationary cubic incorrectly required regularity for cumulative length") &&
             passed;

    const auto loop0 = Point2::make(0.0, 0.0);
    const auto loop1 = Point2::make(1.0, 0.0);
    const auto loop2 = Point2::make(1.0, 1.0);
    const auto loop3 = Point2::make(0.0, 0.0);
    if (!loop0 || !loop1 || !loop2 || !loop3) {
        return 1;
    }
    const CubicBezier2 loop_like{*loop0, *loop1, *loop2, *loop3};
    const auto loop_prefix =
        loop_like.cumulative_arc_length_enclosure(0.75, tight_policy);
    passed = require(
                 loop_prefix && ordered_finite(*loop_prefix) &&
                     loop_prefix->upper_length > 0.0,
                 "endpoint-coincident curve was reduced to endpoint displacement") &&
             passed;

    const auto zero_tolerance_prefix =
        parabola2.cumulative_arc_length_enclosure(0.5, zero_tolerance_policy);
    passed = require(
                 zero_tolerance_prefix &&
                     zero_tolerance_prefix->result ==
                         CurveLengthResult::indeterminate &&
                     ordered_finite(*zero_tolerance_prefix),
                 "zero-tolerance curved prefix did not remain indeterminate") &&
             passed;

    const auto coarse_prefix =
        parabola2.cumulative_arc_length_enclosure(0.5, coarse_policy);
    passed = require(
                 coarse_prefix &&
                     coarse_prefix->result == CurveLengthResult::indeterminate &&
                     ordered_finite(*coarse_prefix),
                 "coarse resource budget did not retain a valid cumulative enclosure") &&
             passed;

    passed = require_error(
                 parabola2.cumulative_arc_length_enclosure(
                     std::numeric_limits<double>::quiet_NaN(),
                     tight_policy),
                 CurveLengthError::non_finite_parameter,
                 "NaN cumulative parameter was accepted") &&
             passed;
    passed = require_error(
                 parabola2.cumulative_arc_length_enclosure(
                     std::numeric_limits<double>::infinity(),
                     tight_policy),
                 CurveLengthError::non_finite_parameter,
                 "infinite cumulative parameter was accepted") &&
             passed;
    passed = require_error(
                 parabola2.cumulative_arc_length_enclosure(-0.25, tight_policy),
                 CurveLengthError::parameter_out_of_domain,
                 "below-domain cumulative parameter was accepted") &&
             passed;
    passed = require_error(
                 parabola2.cumulative_arc_length_enclosure(1.25, tight_policy),
                 CurveLengthError::parameter_out_of_domain,
                 "above-domain cumulative parameter was accepted") &&
             passed;

    const CurveLengthPolicy invalid_policy{
        .absolute_tolerance = -1.0,
        .relative_tolerance = 0.0,
        .max_subdivision_depth = 8,
        .max_processed_nodes = 128,
    };
    passed = require_error(
                 parabola2.cumulative_arc_length_enclosure(0.5, invalid_policy),
                 CurveLengthError::invalid_policy,
                 "invalid cumulative length policy was accepted") &&
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
                 extreme.cumulative_arc_length_enclosure(0.5, tight_policy),
                 CurveLengthError::non_finite_enclosure,
                 "un-enclosable cumulative extreme geometry did not fail explicitly") &&
             passed;

    const double denormal = std::numeric_limits<double>::denorm_min();
    const auto d0 = Point2::make(0.0, 0.0);
    const auto d1 = Point2::make(denormal, 0.0);
    const auto d2 = Point2::make(2.0 * denormal, denormal);
    const auto d3 = Point2::make(3.0 * denormal, denormal);
    if (!d0 || !d1 || !d2 || !d3) {
        return 1;
    }
    const CubicBezier2 subnormal{*d0, *d1, *d2, *d3};
    const auto subnormal_prefix =
        subnormal.cumulative_arc_length_enclosure(0.5, coarse_policy);
    passed = require(
                 subnormal_prefix && ordered_finite(*subnormal_prefix),
                 "subnormal-scale cumulative geometry did not retain evidence") &&
             passed;

    const auto repeat_a =
        parabola2.cumulative_arc_length_enclosure(0.5, tight_policy);
    const auto repeat_b =
        parabola2.cumulative_arc_length_enclosure(0.5, tight_policy);
    passed = require(
                 repeat_a && repeat_b && *repeat_a == *repeat_b,
                 "cumulative arc-length evidence is not deterministic") &&
             passed;

    return passed ? 0 : 1;
}
