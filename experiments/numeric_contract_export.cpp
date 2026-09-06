#include "apmesh/core/numeric.hpp"

#include <cfenv>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>
#include <string_view>

namespace {

using apmesh::core::FloatingCategory;
using apmesh::core::NumericError;
using apmesh::core::ProximityPolicy;
using apmesh::core::ProximityResult;

std::string_view category_name(const FloatingCategory category) {
    switch (category) {
    case FloatingCategory::zero:
        return "zero";
    case FloatingCategory::subnormal:
        return "subnormal";
    case FloatingCategory::normal:
        return "normal";
    case FloatingCategory::infinite:
        return "infinite";
    case FloatingCategory::not_a_number:
        return "not_a_number";
    }
    return "unknown";
}

std::string_view error_name(const NumericError error) {
    switch (error) {
    case NumericError::non_finite_input:
        return "non_finite_input";
    case NumericError::non_finite_policy:
        return "non_finite_policy";
    case NumericError::negative_absolute_tolerance:
        return "negative_absolute_tolerance";
    case NumericError::negative_relative_tolerance:
        return "negative_relative_tolerance";
    case NumericError::non_positive_reference_scale:
        return "non_positive_reference_scale";
    case NumericError::non_finite_intermediate:
        return "non_finite_intermediate";
    }
    return "unknown";
}

std::string_view result_name(const ProximityResult result) {
    return result == ProximityResult::within ? "within" : "outside";
}

std::string hex_value(const double value) {
    std::ostringstream output;
    output.imbue(std::locale::classic());
    output << std::hexfloat << value;
    return output.str();
}

void write_classification(
    std::ostream& output,
    const std::string_view case_name,
    const double value,
    const bool prefix_comma) {
    if (prefix_comma) {
        output << ',';
    }
    output << "{\"case\":\"" << case_name << "\",\"category\":\""
           << category_name(apmesh::core::classify_floating(value)) << "\"}";
}

void write_policy(
    std::ostream& output,
    const std::string_view case_name,
    const ProximityPolicy& policy,
    const bool prefix_comma) {
    if (prefix_comma) {
        output << ',';
    }
    const auto validation = apmesh::core::validate_proximity_policy(policy);
    output << "{\"case\":\"" << case_name << "\",\"outcome\":\"";
    if (validation.has_value()) {
        output << "valid\",\"error\":null}";
        return;
    }
    output << "error\",\"error\":\"" << error_name(validation.error()) << "\"}";
}

void write_proximity(
    std::ostream& output,
    const std::string_view case_name,
    const double lhs,
    const double rhs,
    const ProximityPolicy& policy,
    const bool prefix_comma) {
    if (prefix_comma) {
        output << ',';
    }
    const auto comparison = apmesh::core::compare_proximity(lhs, rhs, policy);
    output << "{\"case\":\"" << case_name << "\",\"outcome\":\"";
    if (!comparison.has_value()) {
        output << "error\",\"error\":\"" << error_name(comparison.error())
               << "\",\"residual_hex\":null,\"limit_hex\":null}";
        return;
    }
    output << result_name(comparison->result) << "\",\"error\":null,\"residual_hex\":\""
           << hex_value(comparison->residual) << "\",\"limit_hex\":\""
           << hex_value(comparison->limit) << "\"}";
}

int write_certificate(const std::string_view output_path) {
    std::ofstream output{output_path.data(), std::ios::binary | std::ios::trunc};
    if (!output.is_open()) {
        return 2;
    }

    constexpr ProximityPolicy absolute_policy{.absolute_tolerance = 0.25, .relative_tolerance = 0.0, .reference_scale = 1.0};
    constexpr ProximityPolicy relative_policy{.absolute_tolerance = 0.0, .relative_tolerance = 0.25, .reference_scale = 4.0};
    constexpr ProximityPolicy mixed_policy{.absolute_tolerance = 0.5, .relative_tolerance = 0.25, .reference_scale = 4.0};

    output << "{\"schema_version\":1,\"kind\":\"numeric-contract-certificate\",\"classification\":[";
    write_classification(output, "positive_zero", 0.0, false);
    write_classification(output, "negative_zero", -0.0, true);
    write_classification(output, "denorm_min", std::numeric_limits<double>::denorm_min(), true);
    write_classification(output, "negative_denorm_min", -std::numeric_limits<double>::denorm_min(), true);
    write_classification(output, "one", 1.0, true);
    write_classification(output, "negative_one", -1.0, true);
    write_classification(output, "max_finite", std::numeric_limits<double>::max(), true);
    write_classification(output, "positive_infinity", std::numeric_limits<double>::infinity(), true);
    write_classification(output, "negative_infinity", -std::numeric_limits<double>::infinity(), true);
    write_classification(output, "quiet_nan", std::numeric_limits<double>::quiet_NaN(), true);
    output << "],\"policy\":[";
    write_policy(output, "zero_valid", ProximityPolicy{.absolute_tolerance = 0.0, .relative_tolerance = 0.0, .reference_scale = 1.0}, false);
    write_policy(output, "absolute_valid", absolute_policy, true);
    write_policy(output, "relative_valid", relative_policy, true);
    write_policy(output, "mixed_valid", mixed_policy, true);
    write_policy(output, "negative_absolute", ProximityPolicy{.absolute_tolerance = -0.25, .relative_tolerance = 0.0, .reference_scale = 1.0}, true);
    write_policy(output, "negative_relative", ProximityPolicy{.absolute_tolerance = 0.0, .relative_tolerance = -0.25, .reference_scale = 1.0}, true);
    write_policy(output, "zero_scale", ProximityPolicy{.absolute_tolerance = 0.0, .relative_tolerance = 0.0, .reference_scale = 0.0}, true);
    write_policy(output, "infinite_scale", ProximityPolicy{.absolute_tolerance = 0.0, .relative_tolerance = 0.0, .reference_scale = std::numeric_limits<double>::infinity()}, true);
    write_policy(output, "negative_scale", ProximityPolicy{.absolute_tolerance = 0.0, .relative_tolerance = 0.0, .reference_scale = -1.0}, true);
    write_policy(output, "nan_absolute", ProximityPolicy{.absolute_tolerance = std::numeric_limits<double>::quiet_NaN(), .relative_tolerance = 0.0, .reference_scale = 1.0}, true);
    write_policy(output, "infinite_absolute", ProximityPolicy{.absolute_tolerance = std::numeric_limits<double>::infinity(), .relative_tolerance = 0.0, .reference_scale = 1.0}, true);
    write_policy(output, "nan_relative", ProximityPolicy{.absolute_tolerance = 0.0, .relative_tolerance = std::numeric_limits<double>::quiet_NaN(), .reference_scale = 1.0}, true);
    write_policy(output, "infinite_relative", ProximityPolicy{.absolute_tolerance = 0.0, .relative_tolerance = std::numeric_limits<double>::infinity(), .reference_scale = 1.0}, true);
    write_policy(output, "nan_scale", ProximityPolicy{.absolute_tolerance = 0.0, .relative_tolerance = 0.0, .reference_scale = std::numeric_limits<double>::quiet_NaN()}, true);
    output << "],\"proximity\":[";
    write_proximity(output, "absolute_boundary", 1.0, 1.25, absolute_policy, false);
    write_proximity(output, "absolute_outside", 1.0, 1.5, absolute_policy, true);
    write_proximity(output, "adjacent_inside", 1.0, std::nextafter(1.25, 1.0), absolute_policy, true);
    write_proximity(output, "adjacent_outside", 1.0, std::nextafter(1.25, 2.0), absolute_policy, true);
    write_proximity(output, "relative_boundary", 4.0, 5.0, relative_policy, true);
    write_proximity(output, "mixed_boundary", 10.0, 11.5, mixed_policy, true);
    write_proximity(output, "symmetric_forward", 3.0, 3.25, absolute_policy, true);
    write_proximity(output, "symmetric_reverse", 3.25, 3.0, absolute_policy, true);
    write_proximity(output, "reflexive", 3.0, 3.0, absolute_policy, true);
    write_proximity(output, "near_zero", 0.0, std::numeric_limits<double>::denorm_min(), ProximityPolicy{.absolute_tolerance = std::numeric_limits<double>::denorm_min(), .relative_tolerance = 0.0, .reference_scale = 1.0}, true);
    write_proximity(output, "large_equal", std::numeric_limits<double>::max() / 4.0, std::numeric_limits<double>::max() / 4.0, ProximityPolicy{.absolute_tolerance = 0.0, .relative_tolerance = 0.0, .reference_scale = 1.0}, true);
    write_proximity(output, "large_scale", 0x1p+600, 0x1.8p+600, ProximityPolicy{.absolute_tolerance = 0.0, .relative_tolerance = 0.5, .reference_scale = 0x1p+600}, true);
    write_proximity(output, "power_two_base", 3.0, 3.25, ProximityPolicy{.absolute_tolerance = 0.25, .relative_tolerance = 0.125, .reference_scale = 2.0}, true);
    write_proximity(output, "power_two_scaled", 24.0, 26.0, ProximityPolicy{.absolute_tolerance = 2.0, .relative_tolerance = 0.125, .reference_scale = 16.0}, true);
    write_proximity(output, "nan_input", std::numeric_limits<double>::quiet_NaN(), 0.0, absolute_policy, true);
    write_proximity(output, "overflowing_residual", std::numeric_limits<double>::max(), -std::numeric_limits<double>::max(), absolute_policy, true);
    write_proximity(output, "overflowing_limit", 0.0, 0.0, ProximityPolicy{.absolute_tolerance = std::numeric_limits<double>::max(), .relative_tolerance = 1.0, .reference_scale = std::numeric_limits<double>::max()}, true);
    output << "],\"separation\":{\"proximity_result_values\":[\"within\",\"outside\"],\"identity_conversion\":false,\"predicate_sign\":false}}\n";
    output.flush();
    return output.good() ? 0 : 3;
}

std::string_view round_style_name() {
    switch (std::numeric_limits<double>::round_style) {
    case std::round_toward_zero:
        return "toward_zero";
    case std::round_to_nearest:
        return "to_nearest";
    case std::round_toward_infinity:
        return "toward_infinity";
    case std::round_toward_neg_infinity:
        return "toward_negative_infinity";
    case std::round_indeterminate:
        return "indeterminate";
    }
    return "unknown";
}

std::string_view active_rounding_name() {
    switch (std::fegetround()) {
    case FE_TONEAREST:
        return "to_nearest";
    case FE_TOWARDZERO:
        return "toward_zero";
    case FE_UPWARD:
        return "toward_infinity";
    case FE_DOWNWARD:
        return "toward_negative_infinity";
    default:
        return "indeterminate";
    }
}

int write_environment(const std::string_view output_path) {
    std::ofstream output{output_path.data(), std::ios::binary | std::ios::trunc};
    if (!output.is_open()) {
        return 2;
    }

    output << "{\"schema_version\":1,\"kind\":\"numeric-contract-environment\",\"double\":{"
           << "\"radix\":" << std::numeric_limits<double>::radix
           << ",\"digits\":" << std::numeric_limits<double>::digits
           << ",\"is_iec559\":" << (std::numeric_limits<double>::is_iec559 ? "true" : "false")
           << ",\"subnormal_supported\":"
           << (std::numeric_limits<double>::denorm_min() != 0.0 ? "true" : "false")
           << ",\"round_style\":\"" << round_style_name()
           << "\"},\"active_rounding\":\"" << active_rounding_name() << "\"}\n";
    output.flush();
    return output.good() ? 0 : 3;
}

} // namespace

int main(const int argument_count, char* const arguments[]) {
    if (argument_count != 3) {
        return 1;
    }
    const std::string_view mode{arguments[1]};
    if (mode == "certificate") {
        return write_certificate(arguments[2]);
    }
    if (mode == "environment") {
        return write_environment(arguments[2]);
    }
    return 1;
}
