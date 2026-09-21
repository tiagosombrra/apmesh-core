#include "apmesh/geometry/curve.hpp"

#include <cmath>
#include <cstdio>
#include <expected>
#include <limits>
#include <numeric>
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

bool contains_parameter(
    const apmesh::core::CurveInverseLengthEvidence& evidence,
    const double parameter) {
    return evidence.lower_parameter <= parameter &&
           parameter <= evidence.upper_parameter;
}

bool has_bracket_proof(
    const apmesh::core::CurveInverseLengthEvidence& evidence) {
    return evidence.lower_parameter >= 0.0 &&
           evidence.lower_parameter <= evidence.upper_parameter &&
           evidence.upper_parameter <= 1.0 &&
           evidence.lower_cumulative.upper_length <=
               evidence.target_lower_length &&
           evidence.target_upper_length <=
               evidence.upper_cumulative.lower_length;
}

long double parabola_prefix_length(const long double parameter) {
    return 0.5L * parameter *
               std::sqrt(1.0L + 4.0L * parameter * parameter) +
           0.25L * std::asinh(2.0L * parameter);
}

} // namespace

int main() {
    using apmesh::core::CubicBezier2;
    using apmesh::core::CubicBezier3;
    using apmesh::core::CurveInverseLengthError;
    using apmesh::core::CurveInverseLengthPolicy;
    using apmesh::core::CurveInverseLengthResult;
    using apmesh::core::CurveLengthPolicy;
    using apmesh::core::CurveRegularityPolicy;
    using apmesh::core::Point2;
    using apmesh::core::Point3;

    bool passed = true;

    const CurveInverseLengthPolicy policy{
        .length_policy = CurveLengthPolicy{
            .absolute_tolerance = 0x1p-40,
            .relative_tolerance = 0x1p-40,
            .max_subdivision_depth = 32,
            .max_processed_nodes = 8192,
        },
        .regularity_policy = CurveRegularityPolicy{
            .max_subdivision_depth = 32,
            .max_processed_nodes = 4096,
        },
        .parameter_tolerance = 0x1p-5,
        .max_refinement_iterations = 64,
    };

    const auto p20 = Point2::make(0.0, 0.0);
    const auto p21 = Point2::make(1.0, 0.0);
    const auto p22 = Point2::make(2.0, 0.0);
    const auto p23 = Point2::make(3.0, 0.0);
    const auto p30 = Point3::make(0.0, 0.0, 0.0);
    const auto p31 = Point3::make(1.0, 0.0, 0.0);
    const auto p32 = Point3::make(2.0, 0.0, 0.0);
    const auto p33 = Point3::make(3.0, 0.0, 0.0);
    if (!p20 || !p21 || !p22 || !p23 || !p30 || !p31 || !p32 || !p33) {
        return 1;
    }

    const CubicBezier2 line2{*p20, *p21, *p22, *p23};
    const CubicBezier3 line3{*p30, *p31, *p32, *p33};

    const auto absolute_zero = line2.inverse_arc_length_bracket(0.0, policy);
    passed = require(
                 absolute_zero &&
                     absolute_zero->result == CurveInverseLengthResult::converged &&
                     absolute_zero->lower_parameter == 0.0 &&
                     absolute_zero->upper_parameter == 0.0 &&
                     absolute_zero->target_lower_length == 0.0 &&
                     absolute_zero->target_upper_length == 0.0,
                 "absolute zero did not map exactly to parameter zero") &&
             passed;

    const auto fraction_zero =
        line2.inverse_arc_length_fraction_bracket(0.0, policy);
    passed = require(
                 fraction_zero &&
                     fraction_zero->lower_parameter == 0.0 &&
                     fraction_zero->upper_parameter == 0.0,
                 "fraction zero did not map exactly to parameter zero") &&
             passed;

    const auto fraction_one =
        line2.inverse_arc_length_fraction_bracket(1.0, policy);
    passed = require(
                 fraction_one &&
                     fraction_one->lower_parameter == 1.0 &&
                     fraction_one->upper_parameter == 1.0 &&
                     fraction_one->target_lower_length ==
                         fraction_one->total_length.lower_length &&
                     fraction_one->target_upper_length ==
                         fraction_one->total_length.upper_length,
                 "fraction one did not retain exact endpoint identity") &&
             passed;

    const auto exact_total = line2.inverse_arc_length_bracket(3.0, policy);
    passed = require(
                 exact_total &&
                     exact_total->lower_parameter == 1.0 &&
                     exact_total->upper_parameter == 1.0,
                 "exact certified total did not map to parameter one") &&
             passed;

    const auto straight_absolute =
        line2.inverse_arc_length_bracket(0.6, policy);
    passed = require(
                 straight_absolute &&
                     contains_parameter(*straight_absolute, 0.2) &&
                     has_bracket_proof(*straight_absolute),
                 "straight absolute target lost the analytic inverse bracket") &&
             passed;

    const auto straight_fraction =
        line2.inverse_arc_length_fraction_bracket(0.2, policy);
    passed = require(
                 straight_fraction &&
                     contains_parameter(*straight_fraction, 0.2) &&
                     has_bracket_proof(*straight_fraction),
                 "straight fraction target lost the analytic inverse bracket") &&
             passed;

    const auto line3_absolute =
        line3.inverse_arc_length_bracket(0.6, policy);
    passed = require(
                 line3_absolute &&
                     line3_absolute->result == straight_absolute->result &&
                     line3_absolute->lower_parameter ==
                         straight_absolute->lower_parameter &&
                     line3_absolute->upper_parameter ==
                         straight_absolute->upper_parameter,
                 "2D/3D inverse bracket parity differs") &&
             passed;

    const auto r20 = Point2::make(8.0, -4.0);
    const auto r21 = Point2::make(9.0, -4.0);
    const auto r22 = Point2::make(10.0, -4.0);
    const auto r23 = Point2::make(11.0, -4.0);
    if (!r20 || !r21 || !r22 || !r23) {
        return 1;
    }
    const CubicBezier2 translated{*r20, *r21, *r22, *r23};
    const auto translated_result =
        translated.inverse_arc_length_bracket(0.6, policy);
    passed = require(
                 translated_result &&
                     translated_result->lower_parameter ==
                         straight_absolute->lower_parameter &&
                     translated_result->upper_parameter ==
                         straight_absolute->upper_parameter,
                 "exact translation changed inverse bracket semantics") &&
             passed;

    const auto s20 = Point2::make(0.0, 0.0);
    const auto s21 = Point2::make(2.0, 0.0);
    const auto s22 = Point2::make(4.0, 0.0);
    const auto s23 = Point2::make(6.0, 0.0);
    if (!s20 || !s21 || !s22 || !s23) {
        return 1;
    }
    const CubicBezier2 scaled{*s20, *s21, *s22, *s23};
    const auto scaled_result =
        scaled.inverse_arc_length_bracket(1.2, policy);
    passed = require(
                 scaled_result &&
                     scaled_result->lower_parameter ==
                         straight_absolute->lower_parameter &&
                     scaled_result->upper_parameter ==
                         straight_absolute->upper_parameter,
                 "power-of-two scale changed inverse bracket semantics") &&
             passed;

    const auto reversed_result =
        line2.reversed().inverse_arc_length_bracket(2.4, policy);
    passed = require(
                 reversed_result &&
                     contains_parameter(*reversed_result, 0.8) &&
                     has_bracket_proof(*reversed_result),
                 "reversal lost the corresponding inverse bracket") &&
             passed;

    const auto q0 = Point2::make(0.0, 0.0);
    const auto q1 = Point2::make(1.0 / 3.0, 0.0);
    const auto q2 = Point2::make(2.0 / 3.0, 1.0 / 3.0);
    const auto q3 = Point2::make(1.0, 1.0);
    if (!q0 || !q1 || !q2 || !q3) {
        return 1;
    }
    const CubicBezier2 parabola{*q0, *q1, *q2, *q3};
    constexpr double known_parameter = 0.25;
    const double analytic_target =
        static_cast<double>(parabola_prefix_length(known_parameter));
    const auto parabola_result =
        parabola.inverse_arc_length_bracket(analytic_target, policy);
    passed = require(
                 parabola_result &&
                     contains_parameter(*parabola_result, known_parameter) &&
                     has_bracket_proof(*parabola_result),
                 "analytic parabola target escaped the certified bracket") &&
             passed;

    passed = require_error(
                 line2.inverse_arc_length_bracket(-1.0, policy),
                 CurveInverseLengthError::target_out_of_domain,
                 "negative absolute target was accepted") &&
             passed;
    passed = require_error(
                 line2.inverse_arc_length_bracket(4.0, policy),
                 CurveInverseLengthError::target_out_of_domain,
                 "above-total absolute target was accepted") &&
             passed;
    passed = require_error(
                 line2.inverse_arc_length_bracket(
                     std::numeric_limits<double>::quiet_NaN(),
                     policy),
                 CurveInverseLengthError::non_finite_target,
                 "NaN absolute target was accepted") &&
             passed;
    passed = require_error(
                 line2.inverse_arc_length_fraction_bracket(
                     std::numeric_limits<double>::infinity(),
                     policy),
                 CurveInverseLengthError::non_finite_target,
                 "infinite normalized target was accepted") &&
             passed;
    passed = require_error(
                 line2.inverse_arc_length_fraction_bracket(1.25, policy),
                 CurveInverseLengthError::target_out_of_domain,
                 "out-of-domain normalized target was accepted") &&
             passed;

    CurveInverseLengthPolicy coarse_length = policy;
    coarse_length.length_policy = CurveLengthPolicy{
        .absolute_tolerance = 0.0,
        .relative_tolerance = 0.0,
        .max_subdivision_depth = 0,
        .max_processed_nodes = 1,
    };
    const auto coarse_total =
        parabola.arc_length_enclosure(coarse_length.length_policy);
    if (!coarse_total ||
        !(coarse_total->lower_length < coarse_total->upper_length)) {
        return 1;
    }
    const double uncertain_target =
        std::midpoint(
            coarse_total->lower_length,
            coarse_total->upper_length);
    passed = require_error(
                 parabola.inverse_arc_length_bracket(
                     uncertain_target,
                     coarse_length),
                 CurveInverseLengthError::target_domain_indeterminate,
                 "uncertain total membership was treated as proven") &&
             passed;

    const auto constant_point = Point2::make(2.0, 3.0);
    if (!constant_point) {
        return 1;
    }
    const CubicBezier2 constant{
        *constant_point,
        *constant_point,
        *constant_point,
        *constant_point};
    passed = require_error(
                 constant.inverse_arc_length_bracket(0.0, policy),
                 CurveInverseLengthError::regularity_not_certified,
                 "degenerate curve received an inverse claim") &&
             passed;

    constexpr double epsilon = 0x1p-10;
    const auto n0 = Point2::make(0.0, 0.0);
    const auto n1 = Point2::make(1.0, epsilon);
    const auto n2 = Point2::make(1.0, 2.0 * epsilon);
    const auto n3 = Point2::make(0.0, 3.0 * epsilon);
    if (!n0 || !n1 || !n2 || !n3) {
        return 1;
    }
    const CubicBezier2 near_stationary{*n0, *n1, *n2, *n3};
    CurveInverseLengthPolicy coarse_regularity = policy;
    coarse_regularity.regularity_policy = CurveRegularityPolicy{
        .max_subdivision_depth = 0,
        .max_processed_nodes = 1,
    };
    passed = require_error(
                 near_stationary.inverse_arc_length_fraction_bracket(
                     0.5,
                     coarse_regularity),
                 CurveInverseLengthError::regularity_not_certified,
                 "indeterminate regularity received an inverse claim") &&
             passed;

    const auto midpoint_prefix =
        parabola.cumulative_arc_length_enclosure(
            0.5,
            coarse_length.length_policy);
    if (!midpoint_prefix ||
        !(midpoint_prefix->lower_length <
          midpoint_prefix->upper_length)) {
        return 1;
    }
    const double ambiguous_target =
        std::midpoint(
            midpoint_prefix->lower_length,
            midpoint_prefix->upper_length);
    const auto ambiguous =
        parabola.inverse_arc_length_bracket(
            ambiguous_target,
            coarse_length);
    passed = require(
                 ambiguous &&
                     ambiguous->result ==
                         CurveInverseLengthResult::indeterminate &&
                     ambiguous->lower_parameter == 0.0 &&
                     ambiguous->upper_parameter == 1.0 &&
                     ambiguous->refinement_iterations == 1 &&
                     has_bracket_proof(*ambiguous),
                 "ambiguous midpoint did not retain the valid current bracket") &&
             passed;

    CurveInverseLengthPolicy one_iteration = policy;
    one_iteration.parameter_tolerance = 0.0;
    one_iteration.max_refinement_iterations = 1;
    const auto exhausted =
        line2.inverse_arc_length_bracket(0.6, one_iteration);
    passed = require(
                 exhausted &&
                     exhausted->result ==
                         CurveInverseLengthResult::indeterminate &&
                     exhausted->refinement_iterations == 1 &&
                     contains_parameter(*exhausted, 0.2) &&
                     has_bracket_proof(*exhausted),
                 "iteration exhaustion did not retain a valid bracket") &&
             passed;

    CurveInverseLengthPolicy zero_tolerance = policy;
    zero_tolerance.parameter_tolerance = 0.0;
    zero_tolerance.max_refinement_iterations = 64;
    const auto zero_tolerance_result =
        line2.inverse_arc_length_bracket(0.6, zero_tolerance);
    passed = require(
                 zero_tolerance_result &&
                     zero_tolerance_result->result ==
                         CurveInverseLengthResult::indeterminate &&
                     zero_tolerance_result->lower_parameter <
                         zero_tolerance_result->upper_parameter &&
                     has_bracket_proof(*zero_tolerance_result),
                 "zero parameter tolerance fabricated an exact inverse") &&
             passed;

    CurveInverseLengthPolicy invalid = policy;
    invalid.parameter_tolerance = -1.0;
    passed = require_error(
                 line2.inverse_arc_length_bracket(0.6, invalid),
                 CurveInverseLengthError::invalid_policy,
                 "negative parameter tolerance was accepted") &&
             passed;
    invalid = policy;
    invalid.max_refinement_iterations = 0;
    passed = require_error(
                 line2.inverse_arc_length_bracket(0.6, invalid),
                 CurveInverseLengthError::invalid_policy,
                 "zero refinement budget was accepted") &&
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
                 extreme.inverse_arc_length_bracket(1.0, policy),
                 CurveInverseLengthError::non_finite_enclosure,
                 "non-finite inverse enclosure did not fail explicitly") &&
             passed;

    const auto repeat_a =
        line2.inverse_arc_length_bracket(0.6, policy);
    const auto repeat_b =
        line2.inverse_arc_length_bracket(0.6, policy);
    passed = require(
                 repeat_a && repeat_b && *repeat_a == *repeat_b,
                 "inverse arc-length evidence is not deterministic") &&
             passed;

    return passed ? 0 : 1;
}
