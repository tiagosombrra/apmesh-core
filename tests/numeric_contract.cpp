#include "apmesh/core/numeric.hpp"

#include <cmath>
#include <cstdio>
#include <limits>
#include <string_view>
#include <type_traits>

namespace {

bool require(const bool condition, const std::string_view message) {
    if (!condition) {
        std::fputs(message.data(), stderr);
        std::fputc('\n', stderr);
    }
    return condition;
}

template <typename Value>
bool require_error(
    const std::expected<Value, apmesh::core::NumericError>& value,
    const apmesh::core::NumericError expected,
    const std::string_view message) {
    return require(!value.has_value() && value.error() == expected, message);
}

bool require_valid_policy(
    const apmesh::core::ProximityPolicy& policy,
    const std::string_view message) {
    return require(apmesh::core::validate_proximity_policy(policy).has_value(), message);
}

} // namespace

int main() {
    using apmesh::core::FloatingCategory;
    using apmesh::core::NumericError;
    using apmesh::core::ProximityPolicy;
    using apmesh::core::ProximityResult;

    static_assert(!std::is_convertible_v<ProximityResult, bool>);
    static_assert(!std::is_convertible_v<apmesh::core::ProximityEvidence, bool>);
    static_assert(!std::is_convertible_v<apmesh::core::ProximityEvidence, double>);

    constexpr ProximityPolicy absolute_policy{
        .absolute_tolerance = 0.25,
        .relative_tolerance = 0.0,
        .reference_scale = 1.0,
    };
    constexpr ProximityPolicy relative_policy{
        .absolute_tolerance = 0.0,
        .relative_tolerance = 0.25,
        .reference_scale = 4.0,
    };
    constexpr ProximityPolicy mixed_policy{
        .absolute_tolerance = 0.5,
        .relative_tolerance = 0.25,
        .reference_scale = 4.0,
    };

    bool passed = true;
    passed = require(apmesh::core::classify_floating(0.0) == FloatingCategory::zero,
                     "positive zero was not classified as zero") &&
             passed;
    passed = require(apmesh::core::classify_floating(-0.0) == FloatingCategory::zero,
                     "negative zero was not classified as zero") &&
             passed;
    passed = require(apmesh::core::classify_floating(std::numeric_limits<double>::denorm_min()) ==
                         FloatingCategory::subnormal,
                     "denormal minimum was not classified as subnormal") &&
             passed;
    passed = require(apmesh::core::classify_floating(1.0) == FloatingCategory::normal,
                     "finite normal was not classified as normal") &&
             passed;
    passed = require(apmesh::core::classify_floating(-1.0) == FloatingCategory::normal &&
                         apmesh::core::classify_floating(-std::numeric_limits<double>::denorm_min()) == FloatingCategory::subnormal &&
                         apmesh::core::classify_floating(std::numeric_limits<double>::max()) == FloatingCategory::normal,
                     "finite signed or extreme values were not classified") &&
             passed;
    passed = require(apmesh::core::classify_floating(std::numeric_limits<double>::infinity()) ==
                         FloatingCategory::infinite,
                     "infinity was not classified as infinite") &&
             passed;
    passed = require(apmesh::core::classify_floating(-std::numeric_limits<double>::infinity()) ==
                         FloatingCategory::infinite,
                     "negative infinity was not classified as infinite") &&
             passed;
    passed = require(apmesh::core::classify_floating(std::numeric_limits<double>::quiet_NaN()) ==
                         FloatingCategory::not_a_number,
                     "NaN was not classified as NaN") &&
             passed;
    passed = require(apmesh::core::is_finite(1.0) && !apmesh::core::is_finite(std::numeric_limits<double>::infinity()),
                     "finite classification disagrees with infinity") &&
             passed;

    passed = require_valid_policy(absolute_policy, "absolute policy was rejected") && passed;
    passed = require_valid_policy(relative_policy, "relative policy was rejected") && passed;
    passed = require_valid_policy(mixed_policy, "mixed policy was rejected") && passed;
    passed = require_error(
                 apmesh::core::validate_proximity_policy(ProximityPolicy{
                     .absolute_tolerance = -0.25,
                     .relative_tolerance = 0.0,
                     .reference_scale = 1.0,
                 }),
                 NumericError::negative_absolute_tolerance,
                 "negative absolute tolerance was accepted") &&
             passed;
    passed = require_error(
                 apmesh::core::validate_proximity_policy(ProximityPolicy{
                     .absolute_tolerance = 0.0,
                     .relative_tolerance = -0.25,
                     .reference_scale = 1.0,
                 }),
                 NumericError::negative_relative_tolerance,
                 "negative relative tolerance was accepted") &&
             passed;
    passed = require_error(
                 apmesh::core::validate_proximity_policy(ProximityPolicy{
                     .absolute_tolerance = 0.0,
                     .relative_tolerance = 0.0,
                     .reference_scale = 0.0,
                 }),
                 NumericError::non_positive_reference_scale,
                 "zero reference scale was accepted") &&
             passed;
    passed = require_error(
                 apmesh::core::validate_proximity_policy(ProximityPolicy{
                     .absolute_tolerance = 0.0,
                     .relative_tolerance = 0.0,
                     .reference_scale = std::numeric_limits<double>::infinity(),
                 }),
                 NumericError::non_finite_policy,
                 "infinite reference scale was accepted") &&
             passed;
    for (const auto invalid : {std::numeric_limits<double>::quiet_NaN(), std::numeric_limits<double>::infinity()}) {
        passed = require_error(
                     apmesh::core::validate_proximity_policy(ProximityPolicy{.absolute_tolerance = invalid, .relative_tolerance = 0.0, .reference_scale = 1.0}),
                     NumericError::non_finite_policy, "non-finite absolute tolerance was accepted") && passed;
        passed = require_error(
                     apmesh::core::validate_proximity_policy(ProximityPolicy{.absolute_tolerance = 0.0, .relative_tolerance = invalid, .reference_scale = 1.0}),
                     NumericError::non_finite_policy, "non-finite relative tolerance was accepted") && passed;
    }
    passed = require_error(
                 apmesh::core::validate_proximity_policy(ProximityPolicy{.absolute_tolerance = 0.0, .relative_tolerance = 0.0, .reference_scale = -1.0}),
                 NumericError::non_positive_reference_scale, "negative reference scale was accepted") && passed;
    passed = require_error(
                 apmesh::core::validate_proximity_policy(ProximityPolicy{.absolute_tolerance = 0.0, .relative_tolerance = 0.0, .reference_scale = std::numeric_limits<double>::quiet_NaN()}),
                 NumericError::non_finite_policy, "NaN reference scale was accepted") && passed;

    const auto exact_boundary = apmesh::core::compare_proximity(1.0, 1.25, absolute_policy);
    passed = require(exact_boundary.has_value() &&
                         exact_boundary->result == ProximityResult::within &&
                         exact_boundary->residual == 0.25 && exact_boundary->limit == 0.25,
                     "absolute exact boundary was not accepted") &&
             passed;
    const auto absolute_outside = apmesh::core::compare_proximity(1.0, 1.5, absolute_policy);
    passed = require(absolute_outside.has_value() && absolute_outside->result == ProximityResult::outside,
                     "absolute outside value was accepted") &&
             passed;
    const auto adjacent_inside = apmesh::core::compare_proximity(1.0, std::nextafter(1.25, 1.0), absolute_policy);
    const auto adjacent_outside = apmesh::core::compare_proximity(1.0, std::nextafter(1.25, 2.0), absolute_policy);
    passed = require(adjacent_inside.has_value() && adjacent_inside->result == ProximityResult::within &&
                         adjacent_outside.has_value() && adjacent_outside->result == ProximityResult::outside,
                     "adjacent boundary classification differs") && passed;
    const auto relative_boundary = apmesh::core::compare_proximity(4.0, 5.0, relative_policy);
    passed = require(relative_boundary.has_value() &&
                         relative_boundary->result == ProximityResult::within &&
                         relative_boundary->residual == 1.0 && relative_boundary->limit == 1.0,
                     "relative exact boundary was not accepted") &&
             passed;
    const auto mixed_boundary = apmesh::core::compare_proximity(10.0, 11.5, mixed_policy);
    passed = require(mixed_boundary.has_value() && mixed_boundary->result == ProximityResult::within,
                     "mixed exact boundary was not accepted") &&
             passed;

    const auto forward = apmesh::core::compare_proximity(3.0, 3.25, absolute_policy);
    const auto reverse = apmesh::core::compare_proximity(3.25, 3.0, absolute_policy);
    passed = require(forward.has_value() && reverse.has_value() &&
                         forward->result == reverse->result &&
                         forward->residual == reverse->residual && forward->limit == reverse->limit,
                     "proximity comparison is not symmetric") &&
             passed;
    const auto reflexive = apmesh::core::compare_proximity(3.0, 3.0, absolute_policy);
    passed = require(reflexive.has_value() && reflexive->result == ProximityResult::within && reflexive->residual == 0.0,
                     "finite reflexivity was not accepted") && passed;
    const auto near_zero = apmesh::core::compare_proximity(
        0.0,
        std::numeric_limits<double>::denorm_min(),
        ProximityPolicy{
            .absolute_tolerance = std::numeric_limits<double>::denorm_min(),
            .relative_tolerance = 0.0,
            .reference_scale = 1.0,
        });
    passed = require(near_zero.has_value() && near_zero->result == ProximityResult::within,
                     "near-zero exact boundary was not accepted") &&
             passed;
    const auto large_equal = apmesh::core::compare_proximity(
        std::numeric_limits<double>::max() / 4.0,
        std::numeric_limits<double>::max() / 4.0,
        ProximityPolicy{
            .absolute_tolerance = 0.0,
            .relative_tolerance = 0.0,
            .reference_scale = 1.0,
        });
    passed = require(large_equal.has_value() && large_equal->result == ProximityResult::within,
                     "large finite equality was not accepted") &&
             passed;
    const auto large_scale = apmesh::core::compare_proximity(
        0x1p+600, 0x1.8p+600,
        ProximityPolicy{.absolute_tolerance = 0.0, .relative_tolerance = 0.5, .reference_scale = 0x1p+600});
    passed = require(large_scale.has_value() && large_scale->result == ProximityResult::within &&
                         std::isfinite(large_scale->residual) && std::isfinite(large_scale->limit),
                     "large-scale finite comparison was not accepted") && passed;

    const auto base_scale = apmesh::core::compare_proximity(
        3.0,
        3.25,
        ProximityPolicy{
            .absolute_tolerance = 0.25,
            .relative_tolerance = 0.125,
            .reference_scale = 2.0,
        });
    const auto scaled = apmesh::core::compare_proximity(
        24.0,
        26.0,
        ProximityPolicy{
            .absolute_tolerance = 2.0,
            .relative_tolerance = 0.125,
            .reference_scale = 16.0,
        });
    passed = require(base_scale.has_value() && scaled.has_value() &&
                         base_scale->result == scaled->result,
                     "power-of-two scaling changed the decision") &&
             passed;

    passed = require_error(
                 apmesh::core::compare_proximity(
                     std::numeric_limits<double>::quiet_NaN(), 0.0, absolute_policy),
                 NumericError::non_finite_input,
                 "NaN input was accepted") &&
             passed;
    passed = require_error(
                 apmesh::core::compare_proximity(
                     std::numeric_limits<double>::max(),
                     -std::numeric_limits<double>::max(),
                     absolute_policy),
                 NumericError::non_finite_intermediate,
                 "overflowing residual was accepted") &&
             passed;
    passed = require_error(
                 apmesh::core::compare_proximity(0.0, 0.0,
                     ProximityPolicy{.absolute_tolerance = std::numeric_limits<double>::max(), .relative_tolerance = 1.0,
                                     .reference_scale = std::numeric_limits<double>::max()}),
                 NumericError::non_finite_intermediate, "overflowing limit was accepted") && passed;

    return passed ? 0 : 1;
}
