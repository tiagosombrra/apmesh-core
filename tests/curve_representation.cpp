#include "apmesh/geometry/curve.hpp"

#include "apmesh/core/numeric.hpp"

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

template <typename Value>
bool require_error(
    const std::expected<Value, apmesh::core::CurveError>& value,
    const apmesh::core::CurveError expected,
    const std::string_view message) {
    return require(!value.has_value() && value.error() == expected, message);
}

bool close_scalar(
    const double actual,
    const long double expected,
    const double reference_scale = 1.0) {
    const apmesh::core::ProximityPolicy policy{
        .absolute_tolerance = 2.0e-14 * reference_scale,
        .relative_tolerance = 2.0e-14,
        .reference_scale = reference_scale,
    };
    const auto comparison = apmesh::core::compare_proximity(
        actual,
        static_cast<double>(expected),
        policy);
    return comparison.has_value() &&
           comparison->result == apmesh::core::ProximityResult::within;
}

bool close_point(
    const apmesh::core::Point2& actual,
    const long double expected_x,
    const long double expected_y,
    const double reference_scale = 1.0) {
    return close_scalar(actual.x(), expected_x, reference_scale) &&
           close_scalar(actual.y(), expected_y, reference_scale);
}

bool close_point(
    const apmesh::core::Point3& actual,
    const long double expected_x,
    const long double expected_y,
    const long double expected_z,
    const double reference_scale = 1.0) {
    return close_scalar(actual.x(), expected_x, reference_scale) &&
           close_scalar(actual.y(), expected_y, reference_scale) &&
           close_scalar(actual.z(), expected_z, reference_scale);
}

template <typename Point>
long double bernstein_component(
    const std::array<Point, 4>& points,
    const long double parameter,
    double (Point::*component)() const noexcept) {
    const long double one_minus = 1.0L - parameter;
    const long double b0 = one_minus * one_minus * one_minus;
    const long double b1 = 3.0L * one_minus * one_minus * parameter;
    const long double b2 = 3.0L * one_minus * parameter * parameter;
    const long double b3 = parameter * parameter * parameter;
    return b0 * static_cast<long double>((points[0].*component)()) +
           b1 * static_cast<long double>((points[1].*component)()) +
           b2 * static_cast<long double>((points[2].*component)()) +
           b3 * static_cast<long double>((points[3].*component)());
}

bool close_points(
    const apmesh::core::Point2& lhs,
    const apmesh::core::Point2& rhs,
    const double reference_scale = 1.0) {
    return close_scalar(lhs.x(), rhs.x(), reference_scale) &&
           close_scalar(lhs.y(), rhs.y(), reference_scale);
}

bool close_points(
    const apmesh::core::Point3& lhs,
    const apmesh::core::Point3& rhs,
    const double reference_scale = 1.0) {
    return close_scalar(lhs.x(), rhs.x(), reference_scale) &&
           close_scalar(lhs.y(), rhs.y(), reference_scale) &&
           close_scalar(lhs.z(), rhs.z(), reference_scale);
}

} // namespace

int main() {
    using apmesh::core::CartesianFrame2;
    using apmesh::core::CartesianFrame3;
    using apmesh::core::CubicBezier2;
    using apmesh::core::CubicBezier3;
    using apmesh::core::CurveError;
    using apmesh::core::Mat2;
    using apmesh::core::Mat3;
    using apmesh::core::Point2;
    using apmesh::core::Point3;
    using apmesh::core::Vector2;

    bool passed = true;

    const auto p20 = Point2::make(0.0, 0.0);
    const auto p21 = Point2::make(0.0, 2.0);
    const auto p22 = Point2::make(2.0, 2.0);
    const auto p23 = Point2::make(2.0, 0.0);
    const auto p30 = Point3::make(0.0, 0.0, 0.0);
    const auto p31 = Point3::make(1.0, 2.0, 4.0);
    const auto p32 = Point3::make(3.0, -1.0, 2.0);
    const auto p33 = Point3::make(4.0, 0.0, 8.0);
    if (!p20 || !p21 || !p22 || !p23 || !p30 || !p31 || !p32 || !p33) {
        return 1;
    }

    const CubicBezier2 curve2{*p20, *p21, *p22, *p23};
    const CubicBezier3 curve3{*p30, *p31, *p32, *p33};

    const std::array<Point2, 4> expected_controls2{*p20, *p21, *p22, *p23};
    const std::array<Point3, 4> expected_controls3{*p30, *p31, *p32, *p33};
    passed = require(curve2.control_points() == expected_controls2,
                     "2D control-point order differs") && passed;
    passed = require(curve3.control_points() == expected_controls3,
                     "3D control-point order differs") && passed;

    const auto endpoint20 = curve2.evaluate(0.0);
    const auto endpoint21 = curve2.evaluate(1.0);
    const auto endpoint30 = curve3.evaluate(0.0);
    const auto endpoint31 = curve3.evaluate(1.0);
    passed = require(endpoint20 && *endpoint20 == *p20 &&
                         endpoint21 && *endpoint21 == *p23,
                     "2D endpoint interpolation differs") && passed;
    passed = require(endpoint30 && *endpoint30 == *p30 &&
                         endpoint31 && *endpoint31 == *p33,
                     "3D endpoint interpolation differs") && passed;

    const auto signed_zero = curve2.evaluate(-0.0);
    passed = require(signed_zero && *signed_zero == *p20,
                     "signed-zero parameter changed endpoint semantics") && passed;

    const auto midpoint2 = curve2.evaluate(0.5);
    const auto expected_midpoint2 = Point2::make(1.0, 1.5);
    passed = require(midpoint2 && expected_midpoint2 &&
                         *midpoint2 == *expected_midpoint2,
                     "exact 2D midpoint differs") && passed;

    const auto line0 = Point2::make(-4.0, 3.0);
    const auto line1 = Point2::make(-1.0, 3.0);
    const auto line2 = Point2::make(5.0, 3.0);
    const auto line3 = Point2::make(8.0, 3.0);
    if (!line0 || !line1 || !line2 || !line3) {
        return 1;
    }
    const CubicBezier2 collinear_curve{*line0, *line1, *line2, *line3};
    for (const double parameter : {0.25, 0.5, 0.75}) {
        const auto value = collinear_curve.evaluate(parameter);
        const long double t = static_cast<long double>(parameter);
        const auto& controls = collinear_curve.control_points();
        const long double expected_x =
            bernstein_component(controls, t, &Point2::x);
        const long double expected_y =
            bernstein_component(controls, t, &Point2::y);
        passed = require(
                     value &&
                         close_point(*value, expected_x, expected_y, 8.0) &&
                         value->y() == 3.0,
                     "collinear cubic Bernstein reference differs") &&
                 passed;
    }

    for (const double parameter : {0.25, 0.5, 0.75}) {
        const auto value = curve2.evaluate(parameter);
        const long double t = static_cast<long double>(parameter);
        const auto& controls = curve2.control_points();
        const long double expected_x =
            bernstein_component(controls, t, &Point2::x);
        const long double expected_y =
            bernstein_component(controls, t, &Point2::y);
        passed = require(value && close_point(*value, expected_x, expected_y, 2.0),
                         "2D independent Bernstein reference differs") && passed;
    }

    for (const double parameter : {0.125, 0.5, 0.875}) {
        const auto value = curve3.evaluate(parameter);
        const long double t = static_cast<long double>(parameter);
        const auto& controls = curve3.control_points();
        const long double expected_x =
            bernstein_component(controls, t, &Point3::x);
        const long double expected_y =
            bernstein_component(controls, t, &Point3::y);
        const long double expected_z =
            bernstein_component(controls, t, &Point3::z);
        passed = require(value &&
                             close_point(*value, expected_x, expected_y, expected_z, 8.0),
                         "3D independent Bernstein reference differs") && passed;
    }

    const auto constant = Point2::make(-3.5, 7.25);
    if (!constant) {
        return 1;
    }
    const CubicBezier2 constant_curve{*constant, *constant, *constant, *constant};
    for (const double parameter : {0.0, 0.125, 0.5, 0.875, 1.0}) {
        const auto value = constant_curve.evaluate(parameter);
        passed = require(value && *value == *constant,
                         "constant cubic did not remain constant") && passed;
    }

    const CubicBezier2 repeated_controls{*p20, *p20, *p22, *p23};
    passed = require(repeated_controls.evaluate(0.5).has_value(),
                     "repeated control points were rejected") && passed;

    const auto reversed2 = curve2.reversed();
    const std::array<Point2, 4> expected_reversed2{*p23, *p22, *p21, *p20};
    passed = require(reversed2.control_points() == expected_reversed2,
                     "2D reversal did not reverse control order") && passed;
    const auto reversed_endpoint0 = reversed2.evaluate(0.0);
    const auto reversed_endpoint1 = reversed2.evaluate(1.0);
    passed = require(reversed_endpoint0 && *reversed_endpoint0 == *p23 &&
                         reversed_endpoint1 && *reversed_endpoint1 == *p20,
                     "2D reversal did not swap exact endpoints") && passed;
    passed = require(reversed2.reversed() == curve2,
                     "2D double reversal did not recover the curve") && passed;
    passed = require(curve3.reversed().reversed() == curve3,
                     "3D double reversal did not recover the curve") && passed;

    for (const double parameter : {0.0, 0.125, 0.25, 0.5, 0.75, 0.875, 1.0}) {
        const auto forward2 = curve2.evaluate(1.0 - parameter);
        const auto reverse2 = reversed2.evaluate(parameter);
        passed = require(forward2 && reverse2 &&
                             close_points(*forward2, *reverse2, 2.0),
                         "2D reversal evaluation relation differs") && passed;

        const auto forward3 = curve3.evaluate(1.0 - parameter);
        const auto reverse3 = curve3.reversed().evaluate(parameter);
        passed = require(forward3 && reverse3 &&
                             close_points(*forward3, *reverse3, 8.0),
                         "3D reversal evaluation relation differs") && passed;
    }

    const auto translation = Vector2::make(8.0, -4.0);
    if (!translation) {
        return 1;
    }
    auto translated_controls = curve2.control_points();
    for (std::size_t index = 0; index < translated_controls.size(); ++index) {
        const auto translated = curve2.control_points()[index] + *translation;
        if (!translated) {
            return 1;
        }
        translated_controls[index] = *translated;
    }
    const CubicBezier2 translated_curve{
        translated_controls[0],
        translated_controls[1],
        translated_controls[2],
        translated_controls[3]};
    for (const double parameter : {0.25, 0.5, 0.75}) {
        const auto original = curve2.evaluate(parameter);
        const auto translated_value = translated_curve.evaluate(parameter);
        const auto translated_original =
            original ? *original + *translation
                     : std::expected<Point2, apmesh::core::GeometryError>{
                           std::unexpected{apmesh::core::GeometryError::non_finite_result}};
        passed = require(original && translated_value && translated_original &&
                             close_points(*translated_value, *translated_original, 10.0),
                         "translation covariance differs") && passed;
    }

    const auto origin2 = Point2::make(2.0, -1.0);
    const auto basis2 = Mat2::make({0.0, -1.0, 1.0, 0.0});
    const auto frame2 = origin2 && basis2
        ? CartesianFrame2::make(*origin2, *basis2, 1)
        : std::expected<CartesianFrame2, apmesh::core::GeometryError>{
              std::unexpected{apmesh::core::GeometryError::invalid_frame}};
    if (!frame2) {
        return 1;
    }
    auto framed_controls = curve2.control_points();
    for (std::size_t index = 0; index < framed_controls.size(); ++index) {
        const auto mapped = frame2->point_to_world(curve2.control_points()[index]);
        if (!mapped) {
            return 1;
        }
        framed_controls[index] = *mapped;
    }
    const CubicBezier2 framed_curve{
        framed_controls[0],
        framed_controls[1],
        framed_controls[2],
        framed_controls[3]};
    for (const double parameter : {0.25, 0.5, 0.75}) {
        const auto local = curve2.evaluate(parameter);
        const auto mapped_local =
            local ? frame2->point_to_world(*local)
                  : std::expected<Point2, apmesh::core::GeometryError>{
                        std::unexpected{apmesh::core::GeometryError::non_finite_result}};
        const auto direct = framed_curve.evaluate(parameter);
        passed = require(local && mapped_local && direct &&
                             close_points(*mapped_local, *direct, 10.0),
                         "admitted 2D frame covariance differs") && passed;
    }

    const auto origin3 = Point3::make(-1.0, 2.0, 4.0);
    const auto basis3 = Mat3::make({0.0, 1.0, 0.0,
                                    0.0, 0.0, -1.0,
                                    -1.0, 0.0, 0.0});
    const auto frame3 = origin3 && basis3
        ? CartesianFrame3::make(*origin3, *basis3, -1)
        : std::expected<CartesianFrame3, apmesh::core::GeometryError>{
              std::unexpected{apmesh::core::GeometryError::invalid_frame}};
    if (!frame3) {
        return 1;
    }
    auto framed_controls3 = curve3.control_points();
    for (std::size_t index = 0; index < framed_controls3.size(); ++index) {
        const auto mapped = frame3->point_to_world(curve3.control_points()[index]);
        if (!mapped) {
            return 1;
        }
        framed_controls3[index] = *mapped;
    }
    const CubicBezier3 framed_curve3{
        framed_controls3[0],
        framed_controls3[1],
        framed_controls3[2],
        framed_controls3[3]};
    for (const double parameter : {0.25, 0.5, 0.75}) {
        const auto local = curve3.evaluate(parameter);
        const auto mapped_local =
            local ? frame3->point_to_world(*local)
                  : std::expected<Point3, apmesh::core::GeometryError>{
                        std::unexpected{apmesh::core::GeometryError::non_finite_result}};
        const auto direct = framed_curve3.evaluate(parameter);
        passed = require(local && mapped_local && direct &&
                             close_points(*mapped_local, *direct, 10.0),
                         "admitted 3D frame covariance differs") && passed;
    }

    const double maximum = std::numeric_limits<double>::max();
    const auto extreme0 = Point2::make(maximum, 0.0);
    const auto extreme1 = Point2::make(-maximum, 0.0);
    if (!extreme0 || !extreme1) {
        return 1;
    }
    const CubicBezier2 extreme_curve{
        *extreme0,
        *extreme1,
        *extreme0,
        *extreme1};
    const auto extreme_midpoint = extreme_curve.evaluate(0.5);
    passed = require(extreme_midpoint &&
                         apmesh::core::is_finite(extreme_midpoint->x()) &&
                         apmesh::core::is_finite(extreme_midpoint->y()),
                     "finite extreme interpolation produced non-finite output") && passed;

    passed = require_error(
                 curve2.evaluate(std::numeric_limits<double>::quiet_NaN()),
                 CurveError::non_finite_parameter,
                 "NaN parameter was accepted") &&
             passed;
    passed = require_error(
                 curve2.evaluate(std::numeric_limits<double>::infinity()),
                 CurveError::non_finite_parameter,
                 "positive infinity parameter was accepted") &&
             passed;
    passed = require_error(
                 curve2.evaluate(-std::numeric_limits<double>::infinity()),
                 CurveError::non_finite_parameter,
                 "negative infinity parameter was accepted") &&
             passed;
    passed = require_error(
                 curve2.evaluate(-0.25),
                 CurveError::parameter_out_of_domain,
                 "below-domain parameter was accepted") &&
             passed;
    passed = require_error(
                 curve2.evaluate(1.25),
                 CurveError::parameter_out_of_domain,
                 "above-domain parameter was accepted") &&
             passed;

    const auto repeated_a = curve3.evaluate(0.375);
    const auto repeated_b = curve3.evaluate(0.375);
    passed = require(repeated_a && repeated_b && *repeated_a == *repeated_b,
                     "repeated evaluation was not deterministic") && passed;

    return passed ? 0 : 1;
}
