#include "apmesh/geometry/curve.hpp"

#include <array>
#include <charconv>
#include <cmath>
#include <cstddef>
#include <expected>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <locale>
#include <sstream>
#include <string>
#include <string_view>

namespace {

using apmesh::core::CubicBezier2;
using apmesh::core::CubicBezier3;
using apmesh::core::CurveError;
using apmesh::core::CurveInverseLengthError;
using apmesh::core::CurveInverseLengthEvidence;
using apmesh::core::CurveInverseLengthPolicy;
using apmesh::core::CurveInverseLengthResult;
using apmesh::core::CurveLengthError;
using apmesh::core::CurveLengthEvidence;
using apmesh::core::CurveLengthPolicy;
using apmesh::core::CurveLengthResult;
using apmesh::core::CurveRegularityError;
using apmesh::core::CurveRegularityEvidence;
using apmesh::core::CurveRegularityPolicy;
using apmesh::core::CurveRegularityResult;
using apmesh::core::Point2;
using apmesh::core::Point3;
using apmesh::core::Vector2;
using apmesh::core::Vector3;

std::string json_string(const std::string_view value) {
    std::string result{"\""};
    for (const unsigned char character : value) {
        switch (character) {
        case '"': result += "\\\""; break;
        case '\\': result += "\\\\"; break;
        case '\n': result += "\\n"; break;
        case '\r': result += "\\r"; break;
        case '\t': result += "\\t"; break;
        default:
            if (character < 0x20U) {
                constexpr char digits[] = "0123456789abcdef";
                result += "\\u00";
                result += digits[(character >> 4U) & 0x0FU];
                result += digits[character & 0x0FU];
            } else {
                result += static_cast<char>(character);
            }
            break;
        }
    }
    result += '"';
    return result;
}

std::string number(const double value) {
    if (!std::isfinite(value)) {
        return "null";
    }
    std::ostringstream output;
    output.imbue(std::locale::classic());
    output << std::setprecision(std::numeric_limits<double>::max_digits10)
           << value;
    return output.str();
}

std::string point_json(const Point2& value) {
    return "[" + number(value.x()) + "," + number(value.y()) + "]";
}

std::string vector_json(const Vector2& value) {
    return "[" + number(value.x()) + "," + number(value.y()) + "]";
}

std::string regularity_name(const CurveRegularityResult value) {
    switch (value) {
    case CurveRegularityResult::regular: return "regular";
    case CurveRegularityResult::degenerate: return "degenerate";
    case CurveRegularityResult::indeterminate: return "indeterminate";
    }
    return "unknown";
}

std::string length_result_name(const CurveLengthResult value) {
    switch (value) {
    case CurveLengthResult::converged: return "converged";
    case CurveLengthResult::indeterminate: return "indeterminate";
    }
    return "unknown";
}

std::string inverse_result_name(const CurveInverseLengthResult value) {
    switch (value) {
    case CurveInverseLengthResult::converged: return "converged";
    case CurveInverseLengthResult::indeterminate: return "indeterminate";
    }
    return "unknown";
}

std::string curve_error_name(const CurveError value) {
    switch (value) {
    case CurveError::non_finite_parameter: return "non_finite_parameter";
    case CurveError::parameter_out_of_domain: return "parameter_out_of_domain";
    case CurveError::non_finite_result: return "non_finite_result";
    }
    return "unknown";
}

std::string regularity_error_name(const CurveRegularityError value) {
    switch (value) {
    case CurveRegularityError::invalid_policy: return "invalid_policy";
    case CurveRegularityError::non_finite_enclosure: return "non_finite_enclosure";
    }
    return "unknown";
}

std::string length_error_name(const CurveLengthError value) {
    switch (value) {
    case CurveLengthError::invalid_policy: return "invalid_policy";
    case CurveLengthError::non_finite_parameter: return "non_finite_parameter";
    case CurveLengthError::parameter_out_of_domain: return "parameter_out_of_domain";
    case CurveLengthError::non_finite_enclosure: return "non_finite_enclosure";
    }
    return "unknown";
}

std::string inverse_error_name(const CurveInverseLengthError value) {
    switch (value) {
    case CurveInverseLengthError::invalid_policy: return "invalid_policy";
    case CurveInverseLengthError::non_finite_target: return "non_finite_target";
    case CurveInverseLengthError::target_out_of_domain: return "target_out_of_domain";
    case CurveInverseLengthError::target_domain_indeterminate:
        return "target_domain_indeterminate";
    case CurveInverseLengthError::regularity_not_certified:
        return "regularity_not_certified";
    case CurveInverseLengthError::non_finite_enclosure:
        return "non_finite_enclosure";
    }
    return "unknown";
}

std::string length_json(const CurveLengthEvidence& value) {
    return std::string{"{"} +
        "\"result\":" + json_string(length_result_name(value.result)) + "," +
        "\"lower\":" + number(value.lower_length) + "," +
        "\"upper\":" + number(value.upper_length) + "," +
        "\"processed_nodes\":" + std::to_string(value.processed_nodes) + "," +
        "\"accepted_leaves\":" + std::to_string(value.accepted_leaves) + "," +
        "\"max_depth_reached\":" + std::to_string(value.max_depth_reached) +
        "}";
}

std::string regularity_json(const CurveRegularityEvidence& value) {
    return std::string{"{"} +
        "\"result\":" + json_string(regularity_name(value.result)) + "," +
        "\"processed_nodes\":" + std::to_string(value.processed_nodes) + "," +
        "\"certified_leaves\":" + std::to_string(value.certified_leaves) + "," +
        "\"max_depth_reached\":" + std::to_string(value.max_depth_reached) +
        "}";
}

std::string inverse_json(const CurveInverseLengthEvidence& value) {
    return std::string{"{"} +
        "\"result\":" + json_string(inverse_result_name(value.result)) + "," +
        "\"target_lower\":" + number(value.target_lower_length) + "," +
        "\"target_upper\":" + number(value.target_upper_length) + "," +
        "\"lower_parameter\":" + number(value.lower_parameter) + "," +
        "\"upper_parameter\":" + number(value.upper_parameter) + "," +
        "\"lower_cumulative\":" + length_json(value.lower_cumulative) + "," +
        "\"upper_cumulative\":" + length_json(value.upper_cumulative) + "," +
        "\"total_length\":" + length_json(value.total_length) + "," +
        "\"regularity\":" + regularity_json(value.regularity) + "," +
        "\"refinement_iterations\":" + std::to_string(value.refinement_iterations) +
        "}";
}

double distance(const Point2& lhs, const Point2& rhs) {
    return std::hypot(lhs.x() - rhs.x(), lhs.y() - rhs.y());
}

template <typename Point>
double point_residual(const Point& lhs, const Point& rhs) {
    return distance(lhs, rhs);
}

template <typename Vector>
double vector_residual(const Vector& lhs, const Vector& rhs);

template <>
double vector_residual(const Vector2& lhs, const Vector2& rhs) {
    return std::hypot(lhs.x() - rhs.x(), lhs.y() - rhs.y());
}

long double parabola_prefix_length(const long double parameter) {
    return 0.5L * parameter *
               std::sqrt(1.0L + 4.0L * parameter * parameter) +
           0.25L * std::asinh(2.0L * parameter);
}

std::string certificate(const std::string_view cell, const int repetition) {
    const auto p20 = Point2::make(0.0, 0.0);
    const auto p21 = Point2::make(0.0, 2.0);
    const auto p22 = Point2::make(2.0, 2.0);
    const auto p23 = Point2::make(2.0, 0.0);
    const auto l20 = Point2::make(0.0, 0.0);
    const auto l21 = Point2::make(1.0, 0.0);
    const auto l22 = Point2::make(2.0, 0.0);
    const auto l23 = Point2::make(3.0, 0.0);
    const auto q0 = Point2::make(0.0, 0.0);
    const auto q1 = Point2::make(1.0 / 3.0, 0.0);
    const auto q2 = Point2::make(2.0 / 3.0, 1.0 / 3.0);
    const auto q3 = Point2::make(1.0, 1.0);
    const auto q30 = Point3::make(0.0, 0.0, 0.0);
    const auto q31 = Point3::make(1.0 / 3.0, 0.0, 0.0);
    const auto q32 = Point3::make(2.0 / 3.0, 1.0 / 3.0, 0.0);
    const auto q33 = Point3::make(1.0, 1.0, 0.0);

    if (!p20 || !p21 || !p22 || !p23 ||
        !l20 || !l21 || !l22 || !l23 ||
        !q0 || !q1 || !q2 || !q3 || !q30 || !q31 || !q32 || !q33) {
        return {};
    }

    const CubicBezier2 curve2{*p20, *p21, *p22, *p23};
    const CubicBezier2 line2{*l20, *l21, *l22, *l23};
    const CubicBezier2 parabola2{*q0, *q1, *q2, *q3};
    const CubicBezier3 parabola3{*q30, *q31, *q32, *q33};

    const CurveLengthPolicy tight_length{
        .absolute_tolerance = 0x1p-40,
        .relative_tolerance = 0x1p-40,
        .max_subdivision_depth = 32,
        .max_processed_nodes = 8192,
    };
    const CurveRegularityPolicy deep_regularity{
        .max_subdivision_depth = 32,
        .max_processed_nodes = 4096,
    };
    const CurveRegularityPolicy coarse_regularity{
        .max_subdivision_depth = 0,
        .max_processed_nodes = 1,
    };
    const CurveInverseLengthPolicy inverse_policy{
        .length_policy = tight_length,
        .regularity_policy = deep_regularity,
        .parameter_tolerance = 0x1p-5,
        .max_refinement_iterations = 64,
    };

    const auto value0 = curve2.evaluate(0.0);
    const auto value1 = curve2.evaluate(1.0);
    const auto midpoint = curve2.evaluate(0.5);
    const auto d0 = curve2.first_derivative(0.0);
    const auto dd0 = curve2.second_derivative(0.0);
    const auto speed0 = curve2.speed(0.0);
    const auto line_reg = line2.certify_regularity(deep_regularity);

    const auto constant_point = Point2::make(2.0, 3.0);
    if (!constant_point) {
        return {};
    }
    const CubicBezier2 constant{
        *constant_point, *constant_point, *constant_point, *constant_point};
    const auto constant_reg = constant.certify_regularity(deep_regularity);

    constexpr double epsilon = 0x1p-10;
    const auto n0 = Point2::make(0.0, 0.0);
    const auto n1 = Point2::make(1.0, epsilon);
    const auto n2 = Point2::make(1.0, 2.0 * epsilon);
    const auto n3 = Point2::make(0.0, 3.0 * epsilon);
    if (!n0 || !n1 || !n2 || !n3) {
        return {};
    }
    const CubicBezier2 near_stationary{*n0, *n1, *n2, *n3};
    const auto coarse_reg = near_stationary.certify_regularity(coarse_regularity);

    const auto line_total = line2.arc_length_enclosure(tight_length);
    const auto parabola_total = parabola2.arc_length_enclosure(tight_length);
    const auto line_half =
        line2.cumulative_arc_length_enclosure(0.5, tight_length);
    const auto parabola_quarter =
        parabola2.cumulative_arc_length_enclosure(0.25, tight_length);
    const auto inverse_line =
        line2.inverse_arc_length_bracket(0.6, inverse_policy);
    const double parabola_target =
        static_cast<double>(parabola_prefix_length(0.25L));
    const auto inverse_parabola =
        parabola2.inverse_arc_length_bracket(parabola_target, inverse_policy);

    if (!value0 || !value1 || !midpoint || !d0 || !dd0 || !speed0 ||
        !line_reg || !constant_reg || !coarse_reg || !line_total ||
        !parabola_total || !line_half || !parabola_quarter ||
        !inverse_line || !inverse_parabola) {
        return {};
    }

    const auto reverse_value = curve2.reversed().evaluate(0.25);
    const auto forward_value = curve2.evaluate(0.75);
    const auto reverse_d = curve2.reversed().first_derivative(0.25);
    const auto forward_d = curve2.first_derivative(0.75);
    const auto reverse_speed = curve2.reversed().speed(0.25);
    const auto forward_speed = curve2.speed(0.75);
    if (!reverse_value || !forward_value || !reverse_d || !forward_d ||
        !reverse_speed || !forward_speed) {
        return {};
    }
    const auto neg_forward_d = Vector2::make(-forward_d->x(), -forward_d->y());
    if (!neg_forward_d) {
        return {};
    }

    const auto translated0 = Point2::make(8.0, -4.0);
    const auto translated1 = Point2::make(8.0, -2.0);
    const auto translated2 = Point2::make(10.0, -2.0);
    const auto translated3 = Point2::make(10.0, -4.0);
    if (!translated0 || !translated1 || !translated2 || !translated3) {
        return {};
    }
    const CubicBezier2 translated{
        *translated0, *translated1, *translated2, *translated3};
    const auto translated_mid = translated.evaluate(0.5);
    const auto translated_expected = Point2::make(midpoint->x() + 8.0, midpoint->y() - 4.0);
    if (!translated_mid || !translated_expected) {
        return {};
    }

    const auto scaled0 = Point2::make(0.0, 0.0);
    const auto scaled1 = Point2::make(0.0, 4.0);
    const auto scaled2 = Point2::make(4.0, 4.0);
    const auto scaled3 = Point2::make(4.0, 0.0);
    if (!scaled0 || !scaled1 || !scaled2 || !scaled3) {
        return {};
    }
    const CubicBezier2 scaled{
        *scaled0, *scaled1, *scaled2, *scaled3};
    const auto scaled_mid = scaled.evaluate(0.5);
    if (!scaled_mid) {
        return {};
    }
    const double scale_residual =
        std::hypot(scaled_mid->x() - 2.0 * midpoint->x(),
                   scaled_mid->y() - 2.0 * midpoint->y());

    const auto planar2 = parabola2.evaluate(0.25);
    const auto planar3 = parabola3.evaluate(0.25);
    const auto planar_d2 = parabola2.first_derivative(0.25);
    const auto planar_d3 = parabola3.first_derivative(0.25);
    const auto planar_speed2 = parabola2.speed(0.25);
    const auto planar_speed3 = parabola3.speed(0.25);
    const auto planar_total2 = parabola2.arc_length_enclosure(tight_length);
    const auto planar_total3 = parabola3.arc_length_enclosure(tight_length);
    if (!planar2 || !planar3 || !planar_d2 || !planar_d3 ||
        !planar_speed2 || !planar_speed3 || !planar_total2 || !planar_total3) {
        return {};
    }

    const auto nan_eval =
        curve2.evaluate(std::numeric_limits<double>::quiet_NaN());
    const auto below_eval =
        curve2.evaluate(std::nextafter(0.0, -std::numeric_limits<double>::infinity()));
    CurveRegularityPolicy invalid_reg{
        .max_subdivision_depth = 1,
        .max_processed_nodes = 0,
    };
    const auto invalid_reg_result = line2.certify_regularity(invalid_reg);
    CurveLengthPolicy invalid_length = tight_length;
    invalid_length.absolute_tolerance = -1.0;
    const auto invalid_length_result = line2.arc_length_enclosure(invalid_length);
    CurveInverseLengthPolicy invalid_inverse = inverse_policy;
    invalid_inverse.max_refinement_iterations = 0;
    const auto invalid_inverse_result =
        line2.inverse_arc_length_bracket(0.6, invalid_inverse);
    const auto nonfinite_target = line2.inverse_arc_length_bracket(
        std::numeric_limits<double>::quiet_NaN(), inverse_policy);
    const auto outside_target =
        line2.inverse_arc_length_bracket(4.0, inverse_policy);
    CurveInverseLengthPolicy coarse_length = inverse_policy;
    coarse_length.length_policy = CurveLengthPolicy{
        .absolute_tolerance = 0.0,
        .relative_tolerance = 0.0,
        .max_subdivision_depth = 0,
        .max_processed_nodes = 1,
    };
    const auto coarse_total = parabola2.arc_length_enclosure(coarse_length.length_policy);
    if (!coarse_total) {
        return {};
    }
    const double uncertain_target =
        std::midpoint(coarse_total->lower_length, coarse_total->upper_length);
    const auto uncertain_inverse =
        parabola2.inverse_arc_length_bracket(uncertain_target, coarse_length);
    const auto degenerate_inverse =
        constant.inverse_arc_length_bracket(0.0, inverse_policy);
    CurveInverseLengthPolicy coarse_regular_inverse = inverse_policy;
    coarse_regular_inverse.regularity_policy = coarse_regularity;
    const auto uncertified_regular_inverse =
        near_stationary.inverse_arc_length_fraction_bracket(0.5, coarse_regular_inverse);

    std::ostringstream out;
    out.imbue(std::locale::classic());
    out << "{";
    out << "\"schema_version\":1,";
    out << "\"kind\":\"continuous-curve-geometry-certificate\",";
    out << "\"provenance\":{\"cell\":" << json_string(cell)
        << ",\"repetition\":" << repetition << "},";
    out << "\"scientific_projection\":{";
    out << "\"case_inventory\":["
        << "\"representation_value\","
        << "\"differential_speed\","
        << "\"regularity_regular\","
        << "\"regularity_degenerate\","
        << "\"regularity_indeterminate\","
        << "\"total_length_line\","
        << "\"total_length_parabola\","
        << "\"cumulative_length_line\","
        << "\"cumulative_length_parabola\","
        << "\"inverse_line\","
        << "\"inverse_parabola\","
        << "\"reversal_relations\","
        << "\"translation_scale_relations\","
        << "\"planar_embedding_parity\","
        << "\"failure_semantics\"],";
    out << "\"type_contract\":{\"dimensions\":[2,3],\"degree\":3,"
        << "\"control_count\":4,\"parameter_domain\":\"[0,1]\","
        << "\"polynomial\":true,\"rational\":false},";
    out << "\"cases\":{";
    out << "\"representation_value\":{"
        << "\"endpoint0\":" << point_json(*value0) << ","
        << "\"endpoint1\":" << point_json(*value1) << ","
        << "\"midpoint\":" << point_json(*midpoint) << "},";
    out << "\"differential_speed\":{"
        << "\"first_at_0\":" << vector_json(*d0) << ","
        << "\"second_at_0\":" << vector_json(*dd0) << ","
        << "\"speed_at_0\":" << number(*speed0) << "},";
    out << "\"regularity_regular\":" << regularity_json(*line_reg) << ",";
    out << "\"regularity_degenerate\":" << regularity_json(*constant_reg) << ",";
    out << "\"regularity_indeterminate\":" << regularity_json(*coarse_reg) << ",";
    out << "\"total_length_line\":" << length_json(*line_total) << ",";
    out << "\"total_length_parabola\":" << length_json(*parabola_total) << ",";
    out << "\"cumulative_length_line\":" << length_json(*line_half) << ",";
    out << "\"cumulative_length_parabola\":" << length_json(*parabola_quarter) << ",";
    out << "\"inverse_line\":" << inverse_json(*inverse_line) << ",";
    out << "\"inverse_parabola\":{"
        << "\"analytic_parameter\":0.25,"
        << "\"analytic_target\":" << number(parabola_target) << ","
        << "\"evidence\":" << inverse_json(*inverse_parabola) << "},";
    out << "\"reversal_relations\":{"
        << "\"value_residual\":" << number(point_residual(*reverse_value, *forward_value)) << ","
        << "\"first_derivative_residual\":" << number(vector_residual(*reverse_d, *neg_forward_d)) << ","
        << "\"speed_residual\":" << number(std::abs(*reverse_speed - *forward_speed)) << "},";
    out << "\"translation_scale_relations\":{"
        << "\"translation_residual\":" << number(point_residual(*translated_mid, *translated_expected)) << ","
        << "\"scale_residual\":" << number(scale_residual) << "},";
    out << "\"planar_embedding_parity\":{"
        << "\"value_residual\":" << number(std::hypot(
               planar2->x() - planar3->x(), planar2->y() - planar3->y())) << ","
        << "\"z\":" << number(planar3->z()) << ","
        << "\"first_derivative_residual\":" << number(std::hypot(
               planar_d2->x() - planar_d3->x(), planar_d2->y() - planar_d3->y())) << ","
        << "\"derivative_z\":" << number(planar_d3->z()) << ","
        << "\"speed_residual\":" << number(std::abs(*planar_speed2 - *planar_speed3)) << ","
        << "\"total_2d\":" << length_json(*planar_total2) << ","
        << "\"total_3d\":" << length_json(*planar_total3) << "},";
    out << "\"failure_semantics\":{"
        << "\"non_finite_parameter\":" << json_string(
               nan_eval ? "unexpected_success" : curve_error_name(nan_eval.error())) << ","
        << "\"out_of_domain_parameter\":" << json_string(
               below_eval ? "unexpected_success" : curve_error_name(below_eval.error())) << ","
        << "\"invalid_regularity_policy\":" << json_string(
               invalid_reg_result ? "unexpected_success" : regularity_error_name(invalid_reg_result.error())) << ","
        << "\"invalid_length_policy\":" << json_string(
               invalid_length_result ? "unexpected_success" : length_error_name(invalid_length_result.error())) << ","
        << "\"invalid_inverse_policy\":" << json_string(
               invalid_inverse_result ? "unexpected_success" : inverse_error_name(invalid_inverse_result.error())) << ","
        << "\"non_finite_inverse_target\":" << json_string(
               nonfinite_target ? "unexpected_success" : inverse_error_name(nonfinite_target.error())) << ","
        << "\"inverse_target_outside_domain\":" << json_string(
               outside_target ? "unexpected_success" : inverse_error_name(outside_target.error())) << ","
        << "\"inverse_target_domain_indeterminate\":" << json_string(
               uncertain_inverse ? "unexpected_success" : inverse_error_name(uncertain_inverse.error())) << ","
        << "\"inverse_regularity_not_certified\":" << json_string(
               degenerate_inverse ? "unexpected_success" : inverse_error_name(degenerate_inverse.error())) << ","
        << "\"inverse_indeterminate_regularity\":" << json_string(
               uncertified_regular_inverse ? "unexpected_success" : inverse_error_name(uncertified_regular_inverse.error()))
        << "}";
    out << "},";

    out << "\"reference_inputs\":{";
    out << "\"value_curve_controls\":["
        << point_json(*p20) << "," << point_json(*p21) << ","
        << point_json(*p22) << "," << point_json(*p23) << "],";
    out << "\"line_controls\":["
        << point_json(*l20) << "," << point_json(*l21) << ","
        << point_json(*l22) << "," << point_json(*l23) << "],";
    out << "\"parabola_controls\":["
        << point_json(*q0) << "," << point_json(*q1) << ","
        << point_json(*q2) << "," << point_json(*q3) << "],";
    out << "\"length_policy\":{\"absolute_tolerance\":" << number(tight_length.absolute_tolerance)
        << ",\"relative_tolerance\":" << number(tight_length.relative_tolerance)
        << ",\"max_subdivision_depth\":" << tight_length.max_subdivision_depth
        << ",\"max_processed_nodes\":" << tight_length.max_processed_nodes << "},";
    out << "\"regularity_policy\":{\"max_subdivision_depth\":"
        << deep_regularity.max_subdivision_depth
        << ",\"max_processed_nodes\":" << deep_regularity.max_processed_nodes << "},";
    out << "\"inverse_policy\":{\"parameter_tolerance\":"
        << number(inverse_policy.parameter_tolerance)
        << ",\"max_refinement_iterations\":"
        << inverse_policy.max_refinement_iterations << "}";
    out << "},";

    out << "\"series\":{";

    out << "\"value_reference\":[";
    bool first = true;
    for (const double t : {0.0, 0.25, 0.5, 0.75, 1.0}) {
        const auto value = curve2.evaluate(t);
        if (!value) return {};
        if (!first) out << ",";
        first = false;
        out << "{\"t\":" << number(t) << ",\"value\":" << point_json(*value) << "}";
    }
    out << "],";

    out << "\"differential_speed\":[";
    first = true;
    for (const double t : {0.0, 0.25, 0.5, 0.75, 1.0}) {
        const auto first_d = curve2.first_derivative(t);
        const auto second_d = curve2.second_derivative(t);
        const auto speed = curve2.speed(t);
        if (!first_d || !second_d || !speed) return {};
        if (!first) out << ",";
        first = false;
        out << "{\"t\":" << number(t)
            << ",\"first\":" << vector_json(*first_d)
            << ",\"second\":" << vector_json(*second_d)
            << ",\"speed\":" << number(*speed) << "}";
    }
    out << "],";

    out << "\"arc_length_enclosure\":[";
    first = true;
    const std::array<std::size_t, 4> depths{0U, 2U, 4U, 8U};
    const std::array<std::size_t, 4> nodes{1U, 16U, 64U, 1024U};
    for (std::size_t i = 0; i < depths.size(); ++i) {
        const CurveLengthPolicy policy{
            .absolute_tolerance = 0.0,
            .relative_tolerance = 0.0,
            .max_subdivision_depth = depths[i],
            .max_processed_nodes = nodes[i],
        };
        const auto evidence = parabola2.arc_length_enclosure(policy);
        if (!evidence) return {};
        if (!first) out << ",";
        first = false;
        out << "{\"depth\":" << depths[i]
            << ",\"nodes\":" << nodes[i]
            << ",\"lower\":" << number(evidence->lower_length)
            << ",\"upper\":" << number(evidence->upper_length)
            << ",\"width\":" << number(evidence->upper_length - evidence->lower_length)
            << "}";
    }
    out << "],";

    out << "\"inverse_bracket\":[";
    first = true;
    for (const std::size_t iterations : {1U, 2U, 4U, 8U}) {
        CurveInverseLengthPolicy p = inverse_policy;
        p.parameter_tolerance = 0.0;
        p.max_refinement_iterations = iterations;
        const auto evidence = line2.inverse_arc_length_bracket(0.6, p);
        if (!evidence) return {};
        if (!first) out << ",";
        first = false;
        out << "{\"iterations\":" << iterations
            << ",\"lower\":" << number(evidence->lower_parameter)
            << ",\"upper\":" << number(evidence->upper_parameter)
            << ",\"width\":" << number(evidence->upper_parameter - evidence->lower_parameter)
            << "}";
    }
    out << "]";

    out << "},";

    out << "\"nonclaims\":["
        << "\"curvature\",\"torsion\",\"frenet_frames\",\"arbitrary_degree\","
        << "\"rational_curves\",\"bsplines\",\"nurbs\",\"physical_sampling\","
        << "\"boundary_discretization\",\"surface_ownership\",\"meshing\","
        << "\"quad_dominant\",\"parallel_equivalence\",\"wsl_cloud_equivalence\","
        << "\"native_windows_qualification\"]";

    out << "}";
    out << "}";
    return out.str();
}

} // namespace

int main(int argc, char** argv) {
    if (argc < 3 || std::string_view{argv[1]} != "certificate") {
        std::cerr << "usage: exporter certificate OUTPUT [CELL] [REPETITION]\n";
        return 2;
    }
    const std::string_view cell = argc >= 4 ? std::string_view{argv[3]} : "focused";
    int repetition = 1;
    if (argc >= 5) {
        const std::string_view text{argv[4]};
        const auto result = std::from_chars(text.data(), text.data() + text.size(), repetition);
        if (result.ec != std::errc{} || result.ptr != text.data() + text.size() ||
            repetition <= 0) {
            std::cerr << "invalid repetition\n";
            return 2;
        }
    }

    const std::string value = certificate(cell, repetition);
    if (value.empty()) {
        std::cerr << "certificate construction failed\n";
        return 1;
    }

    std::ofstream output{argv[2], std::ios::binary | std::ios::trunc};
    output.imbue(std::locale::classic());
    output << value << '\n';
    if (!output) {
        std::cerr << "certificate write failed\n";
        return 1;
    }
    return 0;
}
