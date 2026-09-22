#include "apmesh/geometry/curve.hpp"
#include "apmesh/geometry/frame.hpp"

#include <algorithm>
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
    const double relative_tolerance = 8.0e-13) {
    const double reference = static_cast<double>(expected);
    const double scale = std::max(
        std::abs(reference),
        std::numeric_limits<double>::min());
    const apmesh::core::ProximityPolicy policy{
        .absolute_tolerance = 0.0,
        .relative_tolerance = relative_tolerance,
        .reference_scale = scale,
    };
    const auto comparison =
        apmesh::core::compare_proximity(actual, reference, policy);
    return comparison.has_value() &&
           comparison->result == apmesh::core::ProximityResult::within;
}

long double first_reference(
    const std::array<apmesh::core::Point2, 4>& points,
    const long double parameter,
    double (apmesh::core::Point2::*component)() const noexcept) {
    const long double d0 =
        3.0L * (static_cast<long double>((points[1].*component)()) -
                static_cast<long double>((points[0].*component)()));
    const long double d1 =
        3.0L * (static_cast<long double>((points[2].*component)()) -
                static_cast<long double>((points[1].*component)()));
    const long double d2 =
        3.0L * (static_cast<long double>((points[3].*component)()) -
                static_cast<long double>((points[2].*component)()));
    const long double one_minus = 1.0L - parameter;
    return one_minus * one_minus * d0 +
           2.0L * one_minus * parameter * d1 +
           parameter * parameter * d2;
}

long double second_reference(
    const std::array<apmesh::core::Point2, 4>& points,
    const long double parameter,
    double (apmesh::core::Point2::*component)() const noexcept) {
    const long double d0 =
        3.0L * (static_cast<long double>((points[1].*component)()) -
                static_cast<long double>((points[0].*component)()));
    const long double d1 =
        3.0L * (static_cast<long double>((points[2].*component)()) -
                static_cast<long double>((points[1].*component)()));
    const long double d2 =
        3.0L * (static_cast<long double>((points[3].*component)()) -
                static_cast<long double>((points[2].*component)()));
    const long double e0 = 2.0L * (d1 - d0);
    const long double e1 = 2.0L * (d2 - d1);
    return (1.0L - parameter) * e0 + parameter * e1;
}

long double signed_reference(
    const apmesh::core::CubicBezier2& curve,
    const long double parameter) {
    const auto& points = curve.control_points();
    const long double vx =
        first_reference(points, parameter, &apmesh::core::Point2::x);
    const long double vy =
        first_reference(points, parameter, &apmesh::core::Point2::y);
    const long double ax =
        second_reference(points, parameter, &apmesh::core::Point2::x);
    const long double ay =
        second_reference(points, parameter, &apmesh::core::Point2::y);
    const long double speed = std::sqrt(vx * vx + vy * vy);
    return (vx * ay - vy * ax) / (speed * speed * speed);
}

bool transform_controls(
    std::array<apmesh::core::Point2, 4>& controls,
    const apmesh::core::CartesianFrame2& frame) {
    for (auto& point : controls) {
        const auto mapped = frame.point_to_world(point);
        if (!mapped) {
            return false;
        }
        point = *mapped;
    }
    return true;
}

bool translate_controls(
    std::array<apmesh::core::Point2, 4>& controls,
    const apmesh::core::Vector2& translation) {
    for (auto& point : controls) {
        const auto translated = point + translation;
        if (!translated) {
            return false;
        }
        point = *translated;
    }
    return true;
}

} // namespace

int main() {
    using apmesh::core::CartesianFrame2;
    using apmesh::core::CubicBezier2;
    using apmesh::core::CurveError;
    using apmesh::core::Mat2;
    using apmesh::core::Point2;
    using apmesh::core::Vector2;

    bool passed = true;

    const auto p0 = Point2::make(0.0, 0.0);
    const auto p1 = Point2::make(1.0 / 3.0, 0.0);
    const auto p2 = Point2::make(2.0 / 3.0, 1.0 / 3.0);
    const auto p3 = Point2::make(1.0, 1.0);
    if (!p0 || !p1 || !p2 || !p3) {
        return 1;
    }
    const CubicBezier2 parabola{*p0, *p1, *p2, *p3};

    const auto start = parabola.signed_curvature(0.0);
    passed = require(
                 start && *start == 2.0 && !std::signbit(*start),
                 "parabola signed curvature start differs") &&
             passed;

    for (const double parameter : {0.0, 0.125, 0.25, 0.5, 0.75, 1.0}) {
        const auto signed_value = parabola.signed_curvature(parameter);
        const auto magnitude = parabola.curvature_magnitude(parameter);
        passed = require(
                     signed_value &&
                         close_scalar(
                             *signed_value,
                             signed_reference(
                                 parabola,
                                 static_cast<long double>(parameter))),
                     "signed curvature analytic reference differs") &&
                 passed;
        passed = require(
                     signed_value && magnitude &&
                         std::abs(*signed_value) == *magnitude,
                     "signed curvature magnitude parity differs") &&
                 passed;
    }

    const auto origin = Point2::make(0.0, 0.0);
    const auto reflection_basis = Mat2::make({1.0, 0.0, 0.0, -1.0});
    const auto reflection = origin && reflection_basis
        ? CartesianFrame2::make(*origin, *reflection_basis, 0)
        : std::expected<CartesianFrame2, apmesh::core::GeometryError>{
              std::unexpected{apmesh::core::GeometryError::invalid_frame}};
    if (!reflection) {
        return 1;
    }
    auto reflected_controls = parabola.control_points();
    if (!transform_controls(reflected_controls, *reflection)) {
        return 1;
    }
    const CubicBezier2 reflected{
        reflected_controls[0],
        reflected_controls[1],
        reflected_controls[2],
        reflected_controls[3]};
    for (const double parameter : {0.0, 0.25, 0.5, 0.75, 1.0}) {
        const auto original = parabola.signed_curvature(parameter);
        const auto transformed = reflected.signed_curvature(parameter);
        const auto original_magnitude = parabola.curvature_magnitude(parameter);
        const auto reflected_magnitude = reflected.curvature_magnitude(parameter);
        passed = require(
                     original && transformed &&
                         close_scalar(*transformed, -*original),
                     "orientation-reversing frame did not flip curvature sign") &&
                 passed;
        passed = require(
                     original_magnitude && reflected_magnitude &&
                         *original_magnitude == *reflected_magnitude,
                     "reflection changed curvature magnitude") &&
                 passed;
    }

    const auto reversed = parabola.reversed();
    for (const double parameter : {0.0, 0.125, 0.5, 0.875, 1.0}) {
        const auto forward = parabola.signed_curvature(1.0 - parameter);
        const auto reverse = reversed.signed_curvature(parameter);
        passed = require(
                     forward && reverse && close_scalar(*reverse, -*forward),
                     "reversal did not flip signed curvature") &&
                 passed;
    }

    const auto rotation_basis = Mat2::make({0.0, -1.0, 1.0, 0.0});
    const auto rotation_origin = Point2::make(4.0, -2.0);
    const auto rotation = rotation_origin && rotation_basis
        ? CartesianFrame2::make(*rotation_origin, *rotation_basis, 0)
        : std::expected<CartesianFrame2, apmesh::core::GeometryError>{
              std::unexpected{apmesh::core::GeometryError::invalid_frame}};
    if (!rotation) {
        return 1;
    }
    auto rotated_controls = parabola.control_points();
    if (!transform_controls(rotated_controls, *rotation)) {
        return 1;
    }
    const CubicBezier2 rotated{
        rotated_controls[0],
        rotated_controls[1],
        rotated_controls[2],
        rotated_controls[3]};
    for (const double parameter : {0.125, 0.5, 0.875}) {
        const auto original = parabola.signed_curvature(parameter);
        const auto transformed = rotated.signed_curvature(parameter);
        passed = require(
                     original && transformed &&
                         close_scalar(*transformed, *original),
                     "orientation-preserving frame changed signed curvature") &&
                 passed;
    }

    const auto translation = Vector2::make(8.0, -4.0);
    if (!translation) {
        return 1;
    }
    auto translated_controls = parabola.control_points();
    if (!translate_controls(translated_controls, *translation)) {
        return 1;
    }
    const CubicBezier2 translated{
        translated_controls[0],
        translated_controls[1],
        translated_controls[2],
        translated_controls[3]};
    for (const double parameter : {0.25, 0.5, 0.75}) {
        const auto original = parabola.signed_curvature(parameter);
        const auto moved = translated.signed_curvature(parameter);
        passed = require(
                     original && moved && close_scalar(*moved, *original),
                     "translation changed signed curvature") &&
                 passed;
    }

    const auto scale_basis = Mat2::identity();
    const auto scale_frame = origin
        ? CartesianFrame2::make(*origin, scale_basis, 5)
        : std::expected<CartesianFrame2, apmesh::core::GeometryError>{
              std::unexpected{apmesh::core::GeometryError::invalid_frame}};
    if (!scale_frame) {
        return 1;
    }
    auto scaled_controls = parabola.control_points();
    if (!transform_controls(scaled_controls, *scale_frame)) {
        return 1;
    }
    const CubicBezier2 scaled{
        scaled_controls[0],
        scaled_controls[1],
        scaled_controls[2],
        scaled_controls[3]};
    for (const double parameter : {0.0, 0.25, 0.75, 1.0}) {
        const auto original = parabola.signed_curvature(parameter);
        const auto scaled_value = scaled.signed_curvature(parameter);
        passed = require(
                     original && scaled_value &&
                         close_scalar(*scaled_value, *original / 32.0),
                     "power-of-two scale covariance differs") &&
                 passed;
    }

    const auto l0 = Point2::make(0.0, 0.0);
    const auto l1 = Point2::make(1.0, 0.0);
    const auto l2 = Point2::make(2.0, 0.0);
    const auto l3 = Point2::make(3.0, 0.0);
    if (!l0 || !l1 || !l2 || !l3) {
        return 1;
    }
    const CubicBezier2 line{*l0, *l1, *l2, *l3};
    for (const double parameter : {0.0, 0.5, 1.0}) {
        const auto value = line.signed_curvature(parameter);
        passed = require(
                     value && *value == 0.0 && !std::signbit(*value),
                     "straight-line signed curvature is not canonical positive zero") &&
                 passed;
    }

    const auto i0 = Point2::make(0.0, -0.125);
    const auto i1 = Point2::make(1.0 / 3.0, 0.125);
    const auto i2 = Point2::make(2.0 / 3.0, -0.125);
    const auto i3 = Point2::make(1.0, 0.125);
    if (!i0 || !i1 || !i2 || !i3) {
        return 1;
    }
    const CubicBezier2 inflection{*i0, *i1, *i2, *i3};
    const auto inflection_value = inflection.signed_curvature(0.5);
    const auto inflection_speed = inflection.speed(0.5);
    passed = require(
                 inflection_value && inflection_speed &&
                     *inflection_speed != 0.0 &&
                     *inflection_value == 0.0 &&
                     !std::signbit(*inflection_value),
                 "regular inflection was not canonical positive zero") &&
             passed;

    const auto s0 = Point2::make(0.0, 0.0);
    const auto s1 = Point2::make(0.0, 0.0);
    const auto s2 = Point2::make(1.0 / 3.0, 0.0);
    const auto s3 = Point2::make(1.0, 1.0);
    if (!s0 || !s1 || !s2 || !s3) {
        return 1;
    }
    const CubicBezier2 singular_endpoint{*s0, *s1, *s2, *s3};
    passed = require_error(
                 singular_endpoint.signed_curvature(0.0),
                 CurveError::singular_parameter,
                 "singular endpoint was accepted for signed curvature") &&
             passed;

    const CubicBezier2 constant{*s0, *s0, *s0, *s0};
    for (const double parameter : {0.0, 0.5, 1.0}) {
        passed = require_error(
                     constant.signed_curvature(parameter),
                     CurveError::singular_parameter,
                     "constant curve was accepted for signed curvature") &&
                 passed;
    }

    for (const int exponent : {-500, 600}) {
        const double scale = std::ldexp(1.0, exponent);
        const auto e0 = Point2::make(0.0, 0.0);
        const auto e1 = Point2::make(scale, 0.0);
        const auto e2 = Point2::make(2.0 * scale, scale);
        const auto e3 = Point2::make(3.0 * scale, 3.0 * scale);
        if (!e0 || !e1 || !e2 || !e3) {
            return 1;
        }
        const CubicBezier2 extreme{*e0, *e1, *e2, *e3};
        const auto value = extreme.signed_curvature(0.0);
        const long double expected =
            2.0L / (3.0L * static_cast<long double>(scale));
        passed = require(
                     value && *value > 0.0 &&
                         close_scalar(*value, expected, 2.0e-12),
                     exponent < 0
                         ? "tiny nonzero derivative was misclassified"
                         : "scale-aware signed curvature differs") &&
                 passed;
    }

    const double tiny = std::ldexp(1.0, -500);
    const double huge = std::ldexp(1.0, 500);
    const auto u0 = Point2::make(0.0, 0.0);
    const auto u1 = Point2::make(tiny, 0.0);
    const auto u2 = Point2::make(2.0 * tiny, huge);
    const auto u3 = Point2::make(3.0 * tiny, 0.0);
    if (!u0 || !u1 || !u2 || !u3) {
        return 1;
    }
    const CubicBezier2 unrepresentable{*u0, *u1, *u2, *u3};
    passed = require_error(
                 unrepresentable.signed_curvature(0.0),
                 CurveError::non_finite_result,
                 "unrepresentable signed curvature did not fail explicitly") &&
             passed;

    passed = require_error(
                 parabola.signed_curvature(
                     std::numeric_limits<double>::quiet_NaN()),
                 CurveError::non_finite_parameter,
                 "NaN signed-curvature parameter was accepted") &&
             passed;
    passed = require_error(
                 parabola.signed_curvature(-0.25),
                 CurveError::parameter_out_of_domain,
                 "below-domain signed-curvature parameter was accepted") &&
             passed;
    passed = require_error(
                 parabola.signed_curvature(1.25),
                 CurveError::parameter_out_of_domain,
                 "above-domain signed-curvature parameter was accepted") &&
             passed;

    const auto negative_zero = parabola.signed_curvature(-0.0);
    const auto positive_zero = parabola.signed_curvature(0.0);
    passed = require(
                 negative_zero && positive_zero &&
                     *negative_zero == *positive_zero,
                 "signed-zero parameter changed signed-curvature semantics") &&
             passed;

    const auto repeat_a = reflected.signed_curvature(0.375);
    const auto repeat_b = reflected.signed_curvature(0.375);
    passed = require(
                 repeat_a && repeat_b && *repeat_a == *repeat_b,
                 "signed curvature query was not deterministic") &&
             passed;

    return passed ? 0 : 1;
}
