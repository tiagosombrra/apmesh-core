#include "apmesh/geometry/curve.hpp"

#include <algorithm>\n#include <cmath>
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

double width(const apmesh::core::CurveLengthEvidence& evidence) {
    return evidence.upper_length - evidence.lower_length;
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
    const auto line2_length = line2.arc_length_enclosure(zero_tolerance_policy);
    const auto line3_length = line3.arc_length_enclosure(zero_tolerance_policy);
    passed = require(
                 line2_length &&
                     line2_length->result == CurveLengthResult::converged &&
                     line2_length->lower_length == 3.0 &&
                     line2_length->upper_length == 3.0,
                 "axis-aligned 2D line did not return exact length") &&
             passed;
    passed = require(
                 line3_length &&
                     line3_length->result == CurveLengthResult::converged &&
                     line3_length->lower_length == 3.0 &&
                     line3_length->upper_length == 3.0,
                 "axis-aligned 3D line did not return exact length") &&
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
    const auto constant2_length =
        constant2.arc_length_enclosure(zero_tolerance_policy);
    const auto constant3_length =
        constant3.arc_length_enclosure(zero_tolerance_policy);
    passed = require(
                 constant2_length &&
                     constant2_length->result == CurveLengthResult::converged &&
                     constant2_length->lower_length == 0.0 &&
                     constant2_length->upper_length == 0.0,
                 "constant 2D curve did not return exact zero length") &&
             passed;
    passed = require(
                 constant3_length &&
                     constant3_length->result == CurveLengthResult::converged &&
                     constant3_length->lower_length == 0.0 &&
                     constant3_length->upper_length == 0.0,
                 "constant 3D curve did not return exact zero length") &&
             passed;

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
    const long double parabola_reference =
        std::sqrt(5.0L) / 2.0L + std::asinh(2.0L) / 4.0L;
    const auto parabola2_length =
        parabola2.arc_length_enclosure(tight_policy);
    const auto parabola3_length =
        parabola3.arc_length_enclosure(tight_policy);
    passed = require(
                 parabola2_length &&
                     parabola2_length->result == CurveLengthResult::converged &&
                     ordered_finite(*parabola2_length) &&
                     contains(*parabola2_length, parabola_reference),
                 "2D degree-elevated parabola enclosure missed analytic length") &&
             passed;
    passed = require(
                 parabola3_length &&
                     parabola3_length->result == CurveLengthResult::converged &&
                     ordered_finite(*parabola3_length) &&
                     contains(*parabola3_length, parabola_reference),
                 "3D embedded parabola enclosure missed analytic length") &&
             passed;
    passed = require(
                 parabola2_length && parabola3_length &&
                     *parabola2_length == *parabola3_length,
                 "2D/3D planar embedding changed length evidence") &&
             passed;

    const auto reversed_length =
        parabola2.reversed().arc_length_enclosure(tight_policy);
    passed = require(
                 reversed_length &&
                     reversed_length->result == CurveLengthResult::converged &&
                     contains(*reversed_length, parabola_reference) &&
                     width(*reversed_length) <=
                         std::max(
                             tight_policy.absolute_tolerance,
                             tight_policy.relative_tolerance *
                                 reversed_length->upper_length),
                 "reversal did not retain a converged containing enclosure") &&
             passed;

    const auto t0 = Point2::make(8.0, -4.0);
    const auto t1 = Point2::make(9.0, -2.0);
    const auto t2 = Point2::make(11.0, -3.0);
    const auto t3 = Point2::make(12.0, -4.0);
    const auto b0 = Point2::make(0.0, 0.0);
    const auto b1 = Point2::make(1.0, 2.0);
    const auto b2 = Point2::make(3.0, 1.0);
    const auto b3 = Point2::make(4.0, 0.0);
    if (!t0 || !t1 || !t2 || !t3 || !b0 || !b1 || !b2 || !b3) {
        return 1;
    }
    const CubicBezier2 base_curve{*b0, *b1, *b2, *b3};
    const CubicBezier2 translated_curve{*t0, *t1, *t2, *t3};
    const auto base_length = base_curve.arc_length_enclosure(tight_policy);
    const auto translated_length =
        translated_curve.arc_length_enclosure(tight_policy);
    passed = require(
                 base_length && translated_length &&
                     *base_length == *translated_length,
                 "exact translation changed arc-length evidence") &&
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
    const auto scaled_length =
        scaled_curve.arc_length_enclosure(scaled_policy);
    passed = require(
                 base_length && scaled_length &&
                     base_length->result == scaled_length->result &&
                     scaled_length->lower_length <=
                         2.0 * base_length->upper_length &&
                     scaled_length->upper_length >=
                         2.0 * base_length->lower_length,
                 "power-of-two scale covariance enclosure differs") &&
             passed;

    const auto zero_tolerance =
        parabola2.arc_length_enclosure(zero_tolerance_policy);
    passed = require(
                 zero_tolerance &&
                     zero_tolerance->result == CurveLengthResult::indeterminate &&
                     ordered_finite(*zero_tolerance),
                 "zero-tolerance curved request did not remain indeterminate") &&
             passed;

    const auto coarse_length =
        parabola2.arc_length_enclosure(coarse_policy);
    passed = require(
                 coarse_length &&
                     coarse_length->result == CurveLengthResult::indeterminate &&
                     ordered_finite(*coarse_length),
                 "coarse resource budget did not retain a valid enclosure") &&
             passed;
    passed = require(
                 coarse_length && parabola2_length &&
                     width(*parabola2_length) <= width(*coarse_length),
                 "increased resources did not produce a no-wider enclosure") &&
             passed;

    const auto loop0 = Point2::make(0.0, 0.0);
    const auto loop1 = Point2::make(1.0, 0.0);
    const auto loop2 = Point2::make(1.0, 1.0);
    const auto loop3 = Point2::make(0.0, 0.0);
    if (!loop0 || !loop1 || !loop2 || !loop3) {
        return 1;
    }
    const CubicBezier2 loop_like{*loop0, *loop1, *loop2, *loop3};
    const auto loop_length = loop_like.arc_length_enclosure(tight_policy);
    passed = require(
                 loop_length && ordered_finite(*loop_length) &&
                     loop_length->upper_length > 0.0 &&
                     loop_length->lower_length > 0.0,
                 "zero endpoint chord was misclassified as zero total length") &&
             passed;

    const auto st0 = Point2::make(0.0, 0.0);
    const auto st1 = Point2::make(1.0, 0.0);
    const auto st2 = Point2::make(1.0, 0.0);
    const auto st3 = Point2::make(0.0, 0.0);
    if (!st0 || !st1 || !st2 || !st3) {
        return 1;
    }
    const CubicBezier2 stationary{*st0, *st1, *st2, *st3};
    const auto stationary_length =
        stationary.arc_length_enclosure(tight_policy);
    passed = require(
                 stationary_length && ordered_finite(*stationary_length) &&
                     stationary_length->upper_length > 0.0,
                 "stationary cubic incorrectly required regularity for length") &&
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
                 extreme.arc_length_enclosure(tight_policy),
                 CurveLengthError::non_finite_enclosure,
                 "un-enclosable extreme geometry did not fail explicitly") &&
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
    const auto subnormal_length =
        subnormal.arc_length_enclosure(coarse_policy);
    passed = require(
                 subnormal_length && ordered_finite(*subnormal_length),
                 "subnormal-scale geometry did not retain a valid enclosure") &&
             passed;

    const CurveLengthPolicy nan_absolute{
        .absolute_tolerance = std::numeric_limits<double>::quiet_NaN(),
        .relative_tolerance = 0.0,
        .max_subdivision_depth = 1,
        .max_processed_nodes = 1,
    };
    const CurveLengthPolicy infinite_relative{
        .absolute_tolerance = 0.0,
        .relative_tolerance = std::numeric_limits<double>::infinity(),
        .max_subdivision_depth = 1,
        .max_processed_nodes = 1,
    };
    const CurveLengthPolicy negative_absolute{
        .absolute_tolerance = -1.0,
        .relative_tolerance = 0.0,
        .max_subdivision_depth = 1,
        .max_processed_nodes = 1,
    };
    const CurveLengthPolicy zero_nodes{
        .absolute_tolerance = 0.0,
        .relative_tolerance = 0.0,
        .max_subdivision_depth = 1,
        .max_processed_nodes = 0,
    };
    const CurveLengthPolicy excessive_depth{
        .absolute_tolerance = 0.0,
        .relative_tolerance = 0.0,
        .max_subdivision_depth = 65,
        .max_processed_nodes = 1,
    };

    for (const CurveLengthPolicy policy : {
             nan_absolute,
             infinite_relative,
             negative_absolute,
             zero_nodes,
             excessive_depth}) {
        passed = require_error(
                     parabola2.arc_length_enclosure(policy),
                     CurveLengthError::invalid_policy,
                     "invalid arc-length policy was accepted") &&
                 passed;
    }

    const auto repeat_length =
        parabola2.arc_length_enclosure(tight_policy);
    passed = require(
                 repeat_length && parabola2_length &&
                     *repeat_length == *parabola2_length,
                 "arc-length evidence is not deterministic") &&
             passed;

    return passed ? 0 : 1;
}
