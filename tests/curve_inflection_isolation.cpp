#include "apmesh/geometry/curve.hpp"
#include "geometry/detail/curve_inflection_interval.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <expected>
#include <limits>
#include <optional>
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

std::optional<apmesh::core::CubicBezier2> make_curve(
    const std::array<std::array<double, 2>, 4>& coordinates) {
    std::array<apmesh::core::Point2, 4> points{
        *apmesh::core::Point2::make(0.0, 0.0),
        *apmesh::core::Point2::make(0.0, 0.0),
        *apmesh::core::Point2::make(0.0, 0.0),
        *apmesh::core::Point2::make(0.0, 0.0),
    };

    for (std::size_t index = 0; index < coordinates.size(); ++index) {
        const auto point = apmesh::core::Point2::make(
            coordinates[index][0],
            coordinates[index][1]);
        if (!point) {
            return std::nullopt;
        }
        points[index] = *point;
    }

    return apmesh::core::CubicBezier2{
        points[0],
        points[1],
        points[2],
        points[3],
    };
}

template <typename Transform>
std::optional<apmesh::core::CubicBezier2> transform_curve(
    const apmesh::core::CubicBezier2& curve,
    Transform transform) {
    std::array<std::array<double, 2>, 4> coordinates{};
    for (std::size_t index = 0; index < coordinates.size(); ++index) {
        coordinates[index] = transform(curve.control_points()[index]);
    }
    return make_curve(coordinates);
}

bool bracket_contains(
    const apmesh::core::CurveInflectionBracket& bracket,
    const long double parameter) {
    return static_cast<long double>(bracket.lower_parameter) < parameter &&
           parameter < static_cast<long double>(bracket.upper_parameter);
}

bool valid_complete_evidence(
    const apmesh::core::CurveInflectionIsolationEvidence& evidence,
    const double tolerance) {
    if (evidence.result !=
        apmesh::core::CurveInflectionIsolationResult::complete) {
        return false;
    }
    if (evidence.regularity.result !=
        apmesh::core::CurveRegularityResult::regular) {
        return false;
    }
    if (evidence.inflection_count > evidence.brackets.size()) {
        return false;
    }

    for (std::size_t index = 0; index < evidence.inflection_count; ++index) {
        const auto& bracket = evidence.brackets[index];
        if (!std::isfinite(bracket.lower_parameter) ||
            !std::isfinite(bracket.upper_parameter) ||
            bracket.lower_parameter < 0.0 ||
            bracket.upper_parameter > 1.0 ||
            !(bracket.lower_parameter < bracket.upper_parameter) ||
            (bracket.upper_parameter - bracket.lower_parameter) > tolerance) {
            return false;
        }
        if (index > 0 &&
            !(evidence.brackets[index - 1].upper_parameter <=
              bracket.lower_parameter)) {
            return false;
        }
    }
    return true;
}

} // namespace

int main() {
    using apmesh::core::CubicBezier2;
    using apmesh::core::CurveInflectionError;
    using apmesh::core::CurveInflectionIsolationPolicy;
    using apmesh::core::CurveInflectionIsolationResult;
    using apmesh::core::CurveRegularityPolicy;
    using apmesh::core::CurveRegularityResult;
    namespace detail = apmesh::core::detail;

    bool passed = true;

    const auto positive = detail::point_interval(2.0);
    const auto negative = detail::point_interval(-2.0);
    const auto exact_zero = detail::point_interval(0.0);
    if (!positive || !negative || !exact_zero) {
        return 1;
    }

    const detail::QuadraticIntervalCoefficients one_variation{
        *positive,
        *exact_zero,
        *negative,
    };
    const auto variation = detail::quadratic_sign_variation(one_variation);
    passed = require(
                 variation.resolved && !variation.all_zero &&
                     variation.variations == 1,
                 "quadratic sign variation differs") &&
             passed;

    const detail::QuadraticIntervalCoefficients all_zero{
        *exact_zero,
        *exact_zero,
        *exact_zero,
    };
    const auto zero_variation = detail::quadratic_sign_variation(all_zero);
    passed = require(
                 zero_variation.resolved &&
                     zero_variation.all_zero &&
                     zero_variation.variations == 0,
                 "identically-zero quadratic evidence differs") &&
             passed;

    const auto midpoint_children =
        detail::subdivide_quadratic_midpoint(one_variation);
    passed = require(
                 midpoint_children &&
                     detail::interval_contains_zero(
                         midpoint_children->shared_boundary_value),
                 "internal midpoint zero obligation was not retained") &&
             passed;

    const CurveRegularityPolicy regularity_policy{
        .max_subdivision_depth = 32,
        .max_processed_nodes = 4096,
    };
    constexpr double bracket_tolerance = 0x1p-12;
    const CurveInflectionIsolationPolicy policy{
        .regularity_policy = regularity_policy,
        .parameter_tolerance = bracket_tolerance,
        .max_subdivision_depth = 24,
        .max_processed_nodes = 4096,
    };

    const auto line = make_curve({{
        {0.0, 0.0},
        {1.0, 0.0},
        {2.0, 0.0},
        {3.0, 0.0},
    }});
    const auto no_inflection = make_curve({{
        {0.0, 0.0},
        {1.0, 0.0},
        {1.0, 1.0},
        {0.0, 2.0},
    }});
    const auto one_inflection = make_curve({{
        {0.0, 0.0},
        {1.0, 0.0},
        {1.0, 1.0},
        {2.0, 2.0},
    }});
    const auto two_inflections = make_curve({{
        {0.0, 0.0},
        {1.0, 0.0},
        {1.0, 1.0},
        {0.0, -3.0},
    }});
    const auto endpoint_zero = make_curve({{
        {0.0, 0.0},
        {1.0, 0.0},
        {2.0, 0.0},
        {3.0, 1.0},
    }});
    const auto midpoint_root = make_curve({{
        {0.0, 0.0},
        {1.0, 0.0},
        {1.0, 1.0},
        {2.0, 1.0},
    }});
    if (!line || !no_inflection || !one_inflection || !two_inflections ||
        !endpoint_zero || !midpoint_root) {
        return 1;
    }

    const auto line_result = line->isolate_simple_inflections(policy);
    passed = require(
                 line_result &&
                     valid_complete_evidence(*line_result, bracket_tolerance) &&
                     line_result->inflection_count == 0,
                 "regular straight line did not complete with zero simple inflections") &&
             passed;

    const auto no_inflection_result =
        no_inflection->isolate_simple_inflections(policy);
    passed = require(
                 no_inflection_result &&
                     valid_complete_evidence(
                         *no_inflection_result,
                         bracket_tolerance) &&
                     no_inflection_result->inflection_count == 0 &&
                     no_inflection_result->root_free_leaves > 0,
                 "root-free planar cubic did not certify complete") &&
             passed;

    const auto one_result = one_inflection->isolate_simple_inflections(policy);
    const long double one_reference =
        (std::sqrt(5.0L) - 1.0L) / 2.0L;
    passed = require(
                 one_result &&
                     valid_complete_evidence(*one_result, bracket_tolerance) &&
                     one_result->inflection_count == 1 &&
                     bracket_contains(one_result->brackets[0], one_reference),
                 "analytic one-inflection bracket differs") &&
             passed;

    const auto two_result = two_inflections->isolate_simple_inflections(policy);
    const long double sqrt_three = std::sqrt(3.0L);
    const std::array<long double, 2> two_reference{
        (3.0L - sqrt_three) / 6.0L,
        (3.0L + sqrt_three) / 6.0L,
    };
    passed = require(
                 two_result &&
                     valid_complete_evidence(*two_result, bracket_tolerance) &&
                     two_result->inflection_count == 2 &&
                     bracket_contains(two_result->brackets[0], two_reference[0]) &&
                     bracket_contains(two_result->brackets[1], two_reference[1]),
                 "analytic two-inflection brackets differ") &&
             passed;

    const auto endpoint_result =
        endpoint_zero->isolate_simple_inflections(policy);
    passed = require(
                 endpoint_result &&
                     valid_complete_evidence(*endpoint_result, bracket_tolerance) &&
                     endpoint_result->inflection_count == 0,
                 "physical endpoint zero was reported as an interior inflection") &&
             passed;

    const auto midpoint_result =
        midpoint_root->isolate_simple_inflections(policy);
    passed = require(
                 midpoint_result &&
                     midpoint_result->result ==
                         CurveInflectionIsolationResult::indeterminate &&
                     midpoint_result->inflection_count == 0,
                 "internal subdivision-boundary root was silently lost") &&
             passed;

    const CurveInflectionIsolationPolicy coarse_policy{
        .regularity_policy = regularity_policy,
        .parameter_tolerance = bracket_tolerance,
        .max_subdivision_depth = 0,
        .max_processed_nodes = 1,
    };
    const auto coarse_result =
        one_inflection->isolate_simple_inflections(coarse_policy);
    passed = require(
                 coarse_result &&
                     coarse_result->result ==
                         CurveInflectionIsolationResult::indeterminate,
                 "coarse isolation policy guessed an inflection bracket") &&
             passed;

    const CurveInflectionIsolationPolicy regularity_indeterminate_policy{
        .regularity_policy = CurveRegularityPolicy{
            .max_subdivision_depth = 0,
            .max_processed_nodes = 1,
        },
        .parameter_tolerance = bracket_tolerance,
        .max_subdivision_depth = 24,
        .max_processed_nodes = 4096,
    };
    const auto near_stationary = make_curve({{
        {0.0, 0.0},
        {1.0, 0x1p-10},
        {1.0, 0x1p-9},
        {0.0, 3.0 * 0x1p-10},
    }});
    if (!near_stationary) {
        return 1;
    }
    const auto regularity_indeterminate =
        near_stationary->isolate_simple_inflections(
            regularity_indeterminate_policy);
    passed = require(
                 regularity_indeterminate &&
                     regularity_indeterminate->result ==
                         CurveInflectionIsolationResult::indeterminate &&
                     regularity_indeterminate->regularity.result ==
                         CurveRegularityResult::indeterminate,
                 "regularity-indeterminate prerequisite was not preserved") &&
             passed;

    const auto singular = make_curve({{
        {0.0, 0.0},
        {0.0, 0.0},
        {1.0, 0.0},
        {2.0, 1.0},
    }});
    if (!singular) {
        return 1;
    }
    passed = require_error(
                 singular->isolate_simple_inflections(policy),
                 CurveInflectionError::curve_not_regular,
                 "globally degenerate curve was accepted for inflection isolation") &&
             passed;

    const CurveInflectionIsolationPolicy zero_tolerance{
        .regularity_policy = regularity_policy,
        .parameter_tolerance = 0.0,
        .max_subdivision_depth = 24,
        .max_processed_nodes = 4096,
    };
    const CurveInflectionIsolationPolicy zero_nodes{
        .regularity_policy = regularity_policy,
        .parameter_tolerance = bracket_tolerance,
        .max_subdivision_depth = 24,
        .max_processed_nodes = 0,
    };
    const CurveInflectionIsolationPolicy excessive_depth{
        .regularity_policy = regularity_policy,
        .parameter_tolerance = bracket_tolerance,
        .max_subdivision_depth = 65,
        .max_processed_nodes = 4096,
    };
    passed = require_error(
                 one_inflection->isolate_simple_inflections(zero_tolerance),
                 CurveInflectionError::invalid_policy,
                 "zero bracket tolerance was accepted") &&
             passed;
    passed = require_error(
                 one_inflection->isolate_simple_inflections(zero_nodes),
                 CurveInflectionError::invalid_policy,
                 "zero processed-node budget was accepted") &&
             passed;
    passed = require_error(
                 one_inflection->isolate_simple_inflections(excessive_depth),
                 CurveInflectionError::invalid_policy,
                 "unsupported inflection subdivision depth was accepted") &&
             passed;

    const auto reversed = one_inflection->reversed();
    const auto reversed_result = reversed.isolate_simple_inflections(policy);
    passed = require(
                 one_result && reversed_result &&
                     reversed_result->result ==
                         CurveInflectionIsolationResult::complete &&
                     reversed_result->inflection_count ==
                         one_result->inflection_count &&
                     reversed_result->brackets[0].lower_parameter ==
                         1.0 - one_result->brackets[0].upper_parameter &&
                     reversed_result->brackets[0].upper_parameter ==
                         1.0 - one_result->brackets[0].lower_parameter,
                 "reversal bracket covariance differs") &&
             passed;

    const auto translated = transform_curve(
        *one_inflection,
        [](const apmesh::core::Point2& point) {
            return std::array<double, 2>{
                point.x() + 8.0,
                point.y() - 4.0,
            };
        });
    const auto reflected = transform_curve(
        *one_inflection,
        [](const apmesh::core::Point2& point) {
            return std::array<double, 2>{
                point.x(),
                -point.y(),
            };
        });
    const auto scaled = transform_curve(
        *one_inflection,
        [](const apmesh::core::Point2& point) {
            return std::array<double, 2>{
                point.x() * 8.0,
                point.y() * 8.0,
            };
        });
    if (!translated || !reflected || !scaled) {
        return 1;
    }

    const auto translated_result =
        translated->isolate_simple_inflections(policy);
    const auto reflected_result =
        reflected->isolate_simple_inflections(policy);
    const auto scaled_result =
        scaled->isolate_simple_inflections(policy);
    passed = require(
                 one_result && translated_result &&
                     translated_result->result ==
                         CurveInflectionIsolationResult::complete &&
                     translated_result->inflection_count == 1 &&
                     translated_result->brackets[0] ==
                         one_result->brackets[0],
                 "translation changed inflection bracket") &&
             passed;
    passed = require(
                 one_result && reflected_result &&
                     reflected_result->result ==
                         CurveInflectionIsolationResult::complete &&
                     reflected_result->inflection_count == 1 &&
                     reflected_result->brackets[0] ==
                         one_result->brackets[0],
                 "reflection changed inflection bracket") &&
             passed;
    passed = require(
                 one_result && scaled_result &&
                     scaled_result->result ==
                         CurveInflectionIsolationResult::complete &&
                     scaled_result->inflection_count == 1 &&
                     scaled_result->brackets[0] ==
                         one_result->brackets[0],
                 "power-of-two scale changed inflection bracket") &&
             passed;

    const auto repeat_result =
        one_inflection->isolate_simple_inflections(policy);
    passed = require(
                 one_result && repeat_result &&
                     *repeat_result == *one_result,
                 "inflection isolation evidence is not deterministic") &&
             passed;

    const double maximum = std::numeric_limits<double>::max();
    const auto extreme = make_curve({{
        {maximum, 0.0},
        {-maximum, 0.0},
        {maximum, 1.0},
        {-maximum, 1.0},
    }});
    if (!extreme) {
        return 1;
    }
    passed = require_error(
                 extreme->isolate_simple_inflections(policy),
                 CurveInflectionError::non_finite_enclosure,
                 "non-finite enclosure did not fail explicitly") &&
             passed;

    return passed ? 0 : 1;
}
