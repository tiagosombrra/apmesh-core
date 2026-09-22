#include "apmesh/geometry/line_segment.hpp"

#include "apmesh/core/numeric.hpp"

#include <cmath>
#include <cstdio>
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

} // namespace

int main() {
    using apmesh::core::BoundedParametricCurve2;
    using apmesh::core::BoundedParametricCurve3;
    using apmesh::core::CurveError;
    using apmesh::core::LineSegment2;
    using apmesh::core::LineSegment3;
    using apmesh::core::Point2;
    using apmesh::core::Point3;
    using apmesh::core::Vector2;
    using apmesh::core::Vector3;
    using apmesh::core::reversed_parameter;

    static_assert(BoundedParametricCurve2<LineSegment2>);
    static_assert(BoundedParametricCurve3<LineSegment3>);

    bool passed = true;

    const auto p20 = Point2::make(-2.0, 1.0);
    const auto p21 = Point2::make(6.0, 5.0);
    const auto p30 = Point3::make(-2.0, 1.0, -4.0);
    const auto p31 = Point3::make(6.0, 5.0, 4.0);
    if (!p20 || !p21 || !p30 || !p31) {
        return 1;
    }

    const LineSegment2 segment2{*p20, *p21};
    const LineSegment3 segment3{*p30, *p31};

    passed = require(segment2.source() == *p20 && segment2.target() == *p21,
                     "2D source/target identity differs") &&
             passed;
    passed = require(segment3.source() == *p30 && segment3.target() == *p31,
                     "3D source/target identity differs") &&
             passed;

    const auto domain2 = segment2.parameter_domain();
    const auto domain3 = segment3.parameter_domain();
    passed = require(
                 domain2.lower() == 0.0 && domain2.upper() == 1.0 &&
                     domain3.lower() == 0.0 && domain3.upper() == 1.0,
                 "line-segment domains are not exactly [0,1]") &&
             passed;

    const auto source_value2 = segment2.evaluate(0.0);
    const auto target_value2 = segment2.evaluate(1.0);
    const auto quarter_value2 = segment2.evaluate(0.25);
    const auto source_value3 = segment3.evaluate(0.0);
    const auto target_value3 = segment3.evaluate(1.0);
    const auto quarter_value3 = segment3.evaluate(0.25);
    const auto expected_quarter2 = Point2::make(0.0, 2.0);
    const auto expected_quarter3 = Point3::make(0.0, 2.0, -2.0);
    if (!expected_quarter2 || !expected_quarter3) {
        return 1;
    }

    passed = require(source_value2 && *source_value2 == *p20,
                     "2D evaluation did not preserve source endpoint") &&
             passed;
    passed = require(target_value2 && *target_value2 == *p21,
                     "2D evaluation did not preserve target endpoint") &&
             passed;
    passed = require(source_value3 && *source_value3 == *p30,
                     "3D evaluation did not preserve source endpoint") &&
             passed;
    passed = require(target_value3 && *target_value3 == *p31,
                     "3D evaluation did not preserve target endpoint") &&
             passed;
    passed = require(
                 quarter_value2 &&
                     close_point(*quarter_value2, *expected_quarter2, 8.0),
                 "2D interior interpolation differs") &&
             passed;
    passed = require(
                 quarter_value3 &&
                     close_point(*quarter_value3, *expected_quarter3, 8.0),
                 "3D interior interpolation differs") &&
             passed;

    const auto expected_d12 = Vector2::make(8.0, 4.0);
    const auto expected_d13 = Vector3::make(8.0, 4.0, 8.0);
    if (!expected_d12 || !expected_d13) {
        return 1;
    }

    for (const double parameter : {0.0, 0.25, 1.0}) {
        const auto d12 = segment2.first_derivative(parameter);
        const auto d13 = segment3.first_derivative(parameter);
        const auto d22 = segment2.second_derivative(parameter);
        const auto d23 = segment3.second_derivative(parameter);

        passed = require(
                     d12 && close_vector(*d12, *expected_d12, 16.0),
                     "2D first derivative is not constant") &&
                 passed;
        passed = require(
                     d13 && close_vector(*d13, *expected_d13, 16.0),
                     "3D first derivative is not constant") &&
                 passed;
        passed = require(
                     d22 && d22->x() == 0.0 && d22->y() == 0.0,
                     "2D second derivative is not exact zero") &&
                 passed;
        passed = require(
                     d23 && d23->x() == 0.0 && d23->y() == 0.0 &&
                         d23->z() == 0.0,
                     "3D second derivative is not exact zero") &&
                 passed;
    }

    const auto reversed2 = segment2.reversed();
    const auto reversed3 = segment3.reversed();
    passed = require(
                 reversed2.source() == segment2.target() &&
                     reversed2.target() == segment2.source() &&
                     reversed2.reversed() == segment2,
                 "2D reversal identity/involution differs") &&
             passed;
    passed = require(
                 reversed3.source() == segment3.target() &&
                     reversed3.target() == segment3.source() &&
                     reversed3.reversed() == segment3,
                 "3D reversal identity/involution differs") &&
             passed;

    constexpr double parameter = 0.25;
    const auto reversed_parameter_value =
        reversed_parameter(segment2.parameter_domain(), parameter);
    if (!reversed_parameter_value) {
        return 1;
    }

    const auto original_value2 = segment2.evaluate(parameter);
    const auto reverse_value2 = reversed2.evaluate(*reversed_parameter_value);
    const auto original_value3 = segment3.evaluate(parameter);
    const auto reverse_value3 = reversed3.evaluate(*reversed_parameter_value);
    const auto original_d12 = segment2.first_derivative(parameter);
    const auto reverse_d12 =
        reversed2.first_derivative(*reversed_parameter_value);
    const auto original_d13 = segment3.first_derivative(parameter);
    const auto reverse_d13 =
        reversed3.first_derivative(*reversed_parameter_value);
    const auto reverse_d22 =
        reversed2.second_derivative(*reversed_parameter_value);
    const auto reverse_d23 =
        reversed3.second_derivative(*reversed_parameter_value);

    passed = require(
                 original_value2 && reverse_value2 &&
                     close_point(*original_value2, *reverse_value2, 8.0),
                 "2D reversal evaluation covariance differs") &&
             passed;
    passed = require(
                 original_value3 && reverse_value3 &&
                     close_point(*original_value3, *reverse_value3, 8.0),
                 "3D reversal evaluation covariance differs") &&
             passed;
    passed = require(
                 original_d12 && reverse_d12 &&
                     close_scalar(reverse_d12->x(), -original_d12->x(), 16.0) &&
                     close_scalar(reverse_d12->y(), -original_d12->y(), 16.0),
                 "2D reversal first-derivative covariance differs") &&
             passed;
    passed = require(
                 original_d13 && reverse_d13 &&
                     close_scalar(reverse_d13->x(), -original_d13->x(), 16.0) &&
                     close_scalar(reverse_d13->y(), -original_d13->y(), 16.0) &&
                     close_scalar(reverse_d13->z(), -original_d13->z(), 16.0),
                 "3D reversal first-derivative covariance differs") &&
             passed;
    passed = require(
                 reverse_d22 && reverse_d22->x() == 0.0 &&
                     reverse_d22->y() == 0.0,
                 "2D reversal second-derivative covariance differs") &&
             passed;
    passed = require(
                 reverse_d23 && reverse_d23->x() == 0.0 &&
                     reverse_d23->y() == 0.0 && reverse_d23->z() == 0.0,
                 "3D reversal second-derivative covariance differs") &&
             passed;

    const auto degenerate2_point = Point2::make(3.0, -4.0);
    const auto degenerate3_point = Point3::make(3.0, -4.0, 5.0);
    if (!degenerate2_point || !degenerate3_point) {
        return 1;
    }
    const LineSegment2 degenerate2{*degenerate2_point, *degenerate2_point};
    const LineSegment3 degenerate3{*degenerate3_point, *degenerate3_point};

    const auto degenerate_value2 = degenerate2.evaluate(0.375);
    const auto degenerate_value3 = degenerate3.evaluate(0.375);
    const auto degenerate_d12 = degenerate2.first_derivative(0.375);
    const auto degenerate_d13 = degenerate3.first_derivative(0.375);
    const auto degenerate_d22 = degenerate2.second_derivative(0.375);
    const auto degenerate_d23 = degenerate3.second_derivative(0.375);

    passed = require(
                 degenerate_value2 && *degenerate_value2 == *degenerate2_point &&
                     degenerate_d12 && degenerate_d12->x() == 0.0 &&
                     degenerate_d12->y() == 0.0 && degenerate_d22 &&
                     degenerate_d22->x() == 0.0 && degenerate_d22->y() == 0.0 &&
                     degenerate2.reversed() == degenerate2,
                 "2D degenerate segment semantics differ") &&
             passed;
    passed = require(
                 degenerate_value3 && *degenerate_value3 == *degenerate3_point &&
                     degenerate_d13 && degenerate_d13->x() == 0.0 &&
                     degenerate_d13->y() == 0.0 && degenerate_d13->z() == 0.0 &&
                     degenerate_d23 && degenerate_d23->x() == 0.0 &&
                     degenerate_d23->y() == 0.0 && degenerate_d23->z() == 0.0 &&
                     degenerate3.reversed() == degenerate3,
                 "3D degenerate segment semantics differ") &&
             passed;

    const auto nan_value = segment2.evaluate(
        std::numeric_limits<double>::quiet_NaN());
    const auto below_d1 = segment2.first_derivative(-0.25);
    const auto above_d2 = segment3.second_derivative(1.25);
    passed = require(
                 !nan_value &&
                     nan_value.error() == CurveError::non_finite_parameter,
                 "non-finite parameter failure differs") &&
             passed;
    passed = require(
                 !below_d1 &&
                     below_d1.error() == CurveError::parameter_out_of_domain,
                 "below-domain derivative failure differs") &&
             passed;
    passed = require(
                 !above_d2 &&
                     above_d2.error() == CurveError::parameter_out_of_domain,
                 "above-domain second-derivative failure differs") &&
             passed;

    const double maximum = std::numeric_limits<double>::max();
    const auto extreme_source = Point2::make(-maximum, 0.0);
    const auto extreme_target = Point2::make(maximum, 0.0);
    if (!extreme_source || !extreme_target) {
        return 1;
    }
    const LineSegment2 extreme_segment{*extreme_source, *extreme_target};
    const auto extreme_midpoint = extreme_segment.evaluate(0.5);
    const auto extreme_derivative = extreme_segment.first_derivative(0.5);
    passed = require(
                 extreme_midpoint && std::isfinite(extreme_midpoint->x()) &&
                     extreme_midpoint->x() == 0.0,
                 "extreme finite interpolation introduced avoidable overflow") &&
             passed;
    passed = require(
                 !extreme_derivative &&
                     extreme_derivative.error() == CurveError::non_finite_result,
                 "unrepresentable segment derivative did not fail explicitly") &&
             passed;

    const auto translated_source2 = Point2::make(2.0, -1.0);
    const auto translated_target2 = Point2::make(10.0, 3.0);
    const auto expected_translated_value2 = Point2::make(4.0, 0.0);
    if (!translated_source2 || !translated_target2 ||
        !expected_translated_value2) {
        return 1;
    }
    const LineSegment2 translated2{*translated_source2, *translated_target2};
    const auto translated_value2 = translated2.evaluate(0.25);
    const auto translated_d12 = translated2.first_derivative(0.25);
    passed = require(
                 translated_value2 &&
                     close_point(
                         *translated_value2, *expected_translated_value2, 16.0) &&
                     translated_d12 &&
                     close_vector(*translated_d12, *expected_d12, 16.0),
                 "translation covariance differs") &&
             passed;

    const auto embedded_source3 = Point3::make(-2.0, 1.0, 0.0);
    const auto embedded_target3 = Point3::make(6.0, 5.0, 0.0);
    if (!embedded_source3 || !embedded_target3) {
        return 1;
    }
    const LineSegment3 embedded3{*embedded_source3, *embedded_target3};
    const auto embedded_value3 = embedded3.evaluate(0.25);
    const auto embedded_d13 = embedded3.first_derivative(0.25);
    passed = require(
                 quarter_value2 && embedded_value3 &&
                     close_scalar(quarter_value2->x(), embedded_value3->x(), 8.0) &&
                     close_scalar(quarter_value2->y(), embedded_value3->y(), 8.0) &&
                     embedded_value3->z() == 0.0,
                 "2D/3D embedded value parity differs") &&
             passed;
    passed = require(
                 embedded_d13 &&
                     close_scalar(expected_d12->x(), embedded_d13->x(), 16.0) &&
                     close_scalar(expected_d12->y(), embedded_d13->y(), 16.0) &&
                     embedded_d13->z() == 0.0,
                 "2D/3D embedded derivative parity differs") &&
             passed;

    const auto repeat_value_a = segment2.evaluate(0.625);
    const auto repeat_value_b = segment2.evaluate(0.625);
    const auto repeat_d1_a = segment3.first_derivative(0.625);
    const auto repeat_d1_b = segment3.first_derivative(0.625);
    passed = require(
                 repeat_value_a && repeat_value_b &&
                     *repeat_value_a == *repeat_value_b &&
                     repeat_d1_a && repeat_d1_b &&
                     *repeat_d1_a == *repeat_d1_b,
                 "repeated line-segment evidence is not deterministic") &&
             passed;

    return passed ? 0 : 1;
}
