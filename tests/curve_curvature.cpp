#include "apmesh/geometry/curve.hpp"

#include "apmesh/core/numeric.hpp"

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

template <typename Point>
long double first_reference(
    const std::array<Point, 4>& points,
    const long double parameter,
    double (Point::*component)() const noexcept) {
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

template <typename Point>
long double second_reference(
    const std::array<Point, 4>& points,
    const long double parameter,
    double (Point::*component)() const noexcept) {
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

long double reference_curvature(
    const apmesh::core::CubicBezier2& curve,
    const long double parameter) {
    const auto& points = curve.control_points();
    const long double vx = first_reference(points, parameter, &apmesh::core::Point2::x);
    const long double vy = first_reference(points, parameter, &apmesh::core::Point2::y);
    const long double ax = second_reference(points, parameter, &apmesh::core::Point2::x);
    const long double ay = second_reference(points, parameter, &apmesh::core::Point2::y);
    const long double speed = std::sqrt(vx * vx + vy * vy);
    return std::abs(vx * ay - vy * ax) / (speed * speed * speed);
}

long double reference_curvature(
    const apmesh::core::CubicBezier3& curve,
    const long double parameter) {
    const auto& points = curve.control_points();
    const long double vx = first_reference(points, parameter, &apmesh::core::Point3::x);
    const long double vy = first_reference(points, parameter, &apmesh::core::Point3::y);
    const long double vz = first_reference(points, parameter, &apmesh::core::Point3::z);
    const long double ax = second_reference(points, parameter, &apmesh::core::Point3::x);
    const long double ay = second_reference(points, parameter, &apmesh::core::Point3::y);
    const long double az = second_reference(points, parameter, &apmesh::core::Point3::z);
    const long double cx = vy * az - vz * ay;
    const long double cy = vz * ax - vx * az;
    const long double cz = vx * ay - vy * ax;
    const long double numerator = std::sqrt(cx * cx + cy * cy + cz * cz);
    const long double speed = std::sqrt(vx * vx + vy * vy + vz * vz);
    return numerator / (speed * speed * speed);
}

template <typename Point, typename Vector>
bool translate_controls(
    std::array<Point, 4>& controls,
    const Vector& translation) {
    for (auto& point : controls) {
        const auto translated = point + translation;
        if (!translated.has_value()) {
            return false;
        }
        point = *translated;
    }
    return true;
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
    using apmesh::core::Vector3;

    bool passed = true;

    const auto l20 = Point2::make(0.0, 0.0);
    const auto l21 = Point2::make(1.0, 0.0);
    const auto l22 = Point2::make(2.0, 0.0);
    const auto l23 = Point2::make(3.0, 0.0);
    const auto l30 = Point3::make(0.0, 0.0, 0.0);
    const auto l31 = Point3::make(1.0, 2.0, 3.0);
    const auto l32 = Point3::make(2.0, 4.0, 6.0);
    const auto l33 = Point3::make(3.0, 6.0, 9.0);
    if (!l20 || !l21 || !l22 || !l23 || !l30 || !l31 || !l32 || !l33) {
        return 1;
    }
    const CubicBezier2 line2{*l20, *l21, *l22, *l23};
    const CubicBezier3 line3{*l30, *l31, *l32, *l33};
    for (const double parameter : {0.0, 0.25, 0.5, 0.75, 1.0}) {
        const auto curvature2 = line2.curvature_magnitude(parameter);
        const auto curvature3 = line3.curvature_magnitude(parameter);
        passed = require(
                     curvature2 && curvature3 &&
                         *curvature2 == 0.0 && *curvature3 == 0.0,
                     "regular line curvature is not exactly zero") &&
                 passed;
    }

    const auto p20 = Point2::make(0.0, 0.0);
    const auto p21 = Point2::make(1.0 / 3.0, 0.0);
    const auto p22 = Point2::make(2.0 / 3.0, 1.0 / 3.0);
    const auto p23 = Point2::make(1.0, 1.0);
    const auto p30 = Point3::make(0.0, 0.0, 0.0);
    const auto p31 = Point3::make(1.0 / 3.0, 0.0, 0.0);
    const auto p32 = Point3::make(2.0 / 3.0, 1.0 / 3.0, 0.0);
    const auto p33 = Point3::make(1.0, 1.0, 1.0);
    if (!p20 || !p21 || !p22 || !p23 || !p30 || !p31 || !p32 || !p33) {
        return 1;
    }
    const CubicBezier2 parabola2{*p20, *p21, *p22, *p23};
    const CubicBezier3 polynomial3{*p30, *p31, *p32, *p33};

    const auto parabola_start = parabola2.curvature_magnitude(0.0);
    const auto spatial_start = polynomial3.curvature_magnitude(0.0);
    passed = require(
                 parabola_start && *parabola_start == 2.0,
                 "degree-elevated parabola start curvature differs") &&
             passed;
    passed = require(
                 spatial_start && *spatial_start == 2.0,
                 "spatial polynomial start curvature differs") &&
             passed;

    for (const double parameter : {0.125, 0.25, 0.5, 0.75, 0.875}) {
        const auto curvature2 = parabola2.curvature_magnitude(parameter);
        const auto curvature3 = polynomial3.curvature_magnitude(parameter);
        passed = require(
                     curvature2 &&
                         close_scalar(
                             *curvature2,
                             reference_curvature(
                                 parabola2,
                                 static_cast<long double>(parameter))),
                     "2D curvature reference differs") &&
                 passed;
        passed = require(
                     curvature3 &&
                         close_scalar(
                             *curvature3,
                             reference_curvature(
                                 polynomial3,
                                 static_cast<long double>(parameter))),
                     "3D curvature reference differs") &&
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
    const auto inflection_curvature = inflection.curvature_magnitude(0.5);
    const auto inflection_speed = inflection.speed(0.5);
    passed = require(
                 inflection_curvature && inflection_speed &&
                     *inflection_speed != 0.0 &&
                     *inflection_curvature == 0.0,
                 "regular inflection was not a successful zero-curvature result") &&
             passed;

    const auto s0 = Point2::make(0.0, 0.0);
    const auto s1 = Point2::make(0.0, 0.0);
    const auto s2 = Point2::make(1.0 / 3.0, 0.0);
    const auto s3 = Point2::make(1.0, 1.0);
    const auto constant_point = Point3::make(2.0, -3.0, 5.0);
    if (!s0 || !s1 || !s2 || !s3 || !constant_point) {
        return 1;
    }
    const CubicBezier2 singular_endpoint{*s0, *s1, *s2, *s3};
    const CubicBezier3 constant3{
        *constant_point, *constant_point, *constant_point, *constant_point};
    passed = require_error(
                 singular_endpoint.curvature_magnitude(0.0),
                 CurveError::singular_parameter,
                 "singular endpoint was accepted for curvature") &&
             passed;
    for (const double parameter : {0.0, 0.5, 1.0}) {
        passed = require_error(
                     constant3.curvature_magnitude(parameter),
                     CurveError::singular_parameter,
                     "constant curve was accepted for curvature") &&
                 passed;
    }

    const auto reversed2 = parabola2.reversed();
    const auto reversed3 = polynomial3.reversed();
    for (const double parameter : {0.0, 0.125, 0.5, 0.875, 1.0}) {
        const auto forward2 = parabola2.curvature_magnitude(1.0 - parameter);
        const auto reverse2 = reversed2.curvature_magnitude(parameter);
        const auto forward3 = polynomial3.curvature_magnitude(1.0 - parameter);
        const auto reverse3 = reversed3.curvature_magnitude(parameter);
        passed = require(
                     forward2 && reverse2 &&
                         close_scalar(*reverse2, *forward2),
                     "2D reversal changed curvature magnitude") &&
                 passed;
        passed = require(
                     forward3 && reverse3 &&
                         close_scalar(*reverse3, *forward3),
                     "3D reversal changed curvature magnitude") &&
                 passed;
    }

    const auto translation2 = Vector2::make(8.0, -4.0);
    const auto translation3 = Vector3::make(-2.0, 4.0, 6.0);
    if (!translation2 || !translation3) {
        return 1;
    }
    auto translated2 = parabola2.control_points();
    auto translated3 = polynomial3.control_points();
    if (!translate_controls(translated2, *translation2) ||
        !translate_controls(translated3, *translation3)) {
        return 1;
    }
    const CubicBezier2 translated_curve2{
        translated2[0], translated2[1], translated2[2], translated2[3]};
    const CubicBezier3 translated_curve3{
        translated3[0], translated3[1], translated3[2], translated3[3]};
    for (const double parameter : {0.25, 0.5, 0.75}) {
        const auto original2 = parabola2.curvature_magnitude(parameter);
        const auto moved2 = translated_curve2.curvature_magnitude(parameter);
        const auto original3 = polynomial3.curvature_magnitude(parameter);
        const auto moved3 = translated_curve3.curvature_magnitude(parameter);
        passed = require(
                     original2 && moved2 && close_scalar(*moved2, *original2),
                     "2D translation changed curvature") &&
                 passed;
        passed = require(
                     original3 && moved3 && close_scalar(*moved3, *original3),
                     "3D translation changed curvature") &&
                 passed;
    }

    const auto origin2 = Point2::make(4.0, -2.0);
    const auto basis2 = Mat2::make({0.0, -1.0, 1.0, 0.0});
    const auto frame2 = origin2 && basis2
        ? CartesianFrame2::make(*origin2, *basis2, 0)
        : std::expected<CartesianFrame2, apmesh::core::GeometryError>{
              std::unexpected{apmesh::core::GeometryError::invalid_frame}};
    if (!frame2) {
        return 1;
    }
    auto framed2 = parabola2.control_points();
    for (std::size_t index = 0; index < framed2.size(); ++index) {
        const auto mapped = frame2->point_to_world(framed2[index]);
        if (!mapped) {
            return 1;
        }
        framed2[index] = *mapped;
    }
    const CubicBezier2 framed_curve2{
        framed2[0], framed2[1], framed2[2], framed2[3]};
    for (const double parameter : {0.125, 0.5, 0.875}) {
        const auto local = parabola2.curvature_magnitude(parameter);
        const auto framed = framed_curve2.curvature_magnitude(parameter);
        passed = require(
                     local && framed && close_scalar(*framed, *local),
                     "orthogonal 2D frame changed curvature magnitude") &&
                 passed;
    }

    const auto origin3 = Point3::make(-1.0, 2.0, 4.0);
    const auto basis3 = Mat3::make({
        0.0, 1.0, 0.0,
        0.0, 0.0, -1.0,
        -1.0, 0.0, 0.0});
    const auto frame3 = origin3 && basis3
        ? CartesianFrame3::make(*origin3, *basis3, 0)
        : std::expected<CartesianFrame3, apmesh::core::GeometryError>{
              std::unexpected{apmesh::core::GeometryError::invalid_frame}};
    if (!frame3) {
        return 1;
    }
    auto framed3 = polynomial3.control_points();
    for (std::size_t index = 0; index < framed3.size(); ++index) {
        const auto mapped = frame3->point_to_world(framed3[index]);
        if (!mapped) {
            return 1;
        }
        framed3[index] = *mapped;
    }
    const CubicBezier3 framed_curve3{
        framed3[0], framed3[1], framed3[2], framed3[3]};
    for (const double parameter : {0.125, 0.5, 0.875}) {
        const auto local = polynomial3.curvature_magnitude(parameter);
        const auto framed = framed_curve3.curvature_magnitude(parameter);
        passed = require(
                     local && framed && close_scalar(*framed, *local),
                     "orthogonal 3D frame changed curvature magnitude") &&
                 passed;
    }

    const auto scale_origin2 = Point2::make(0.0, 0.0);
    const auto identity2 = Mat2::identity();
    const auto scale_frame2 = scale_origin2
        ? CartesianFrame2::make(*scale_origin2, identity2, 5)
        : std::expected<CartesianFrame2, apmesh::core::GeometryError>{
              std::unexpected{apmesh::core::GeometryError::invalid_frame}};
    if (!scale_frame2) {
        return 1;
    }
    auto scaled2 = parabola2.control_points();
    for (std::size_t index = 0; index < scaled2.size(); ++index) {
        const auto mapped = scale_frame2->point_to_world(scaled2[index]);
        if (!mapped) {
            return 1;
        }
        scaled2[index] = *mapped;
    }
    const CubicBezier2 scaled_curve2{
        scaled2[0], scaled2[1], scaled2[2], scaled2[3]};
    for (const double parameter : {0.0, 0.25, 0.75, 1.0}) {
        const auto original = parabola2.curvature_magnitude(parameter);
        const auto scaled = scaled_curve2.curvature_magnitude(parameter);
        passed = require(
                     original && scaled &&
                         close_scalar(*scaled, *original / 32.0),
                     "power-of-two scale covariance differs") &&
                 passed;
    }

    const auto ep30 = Point3::make(p20->x(), p20->y(), 0.0);
    const auto ep31 = Point3::make(p21->x(), p21->y(), 0.0);
    const auto ep32 = Point3::make(p22->x(), p22->y(), 0.0);
    const auto ep33 = Point3::make(p23->x(), p23->y(), 0.0);
    if (!ep30 || !ep31 || !ep32 || !ep33) {
        return 1;
    }
    const CubicBezier3 embedded{
        *ep30, *ep31, *ep32, *ep33};
    for (const double parameter : {0.0, 0.125, 0.5, 0.875, 1.0}) {
        const auto planar = parabola2.curvature_magnitude(parameter);
        const auto spatial = embedded.curvature_magnitude(parameter);
        passed = require(
                     planar && spatial && close_scalar(*spatial, *planar),
                     "2D/3D planar embedding curvature differs") &&
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
        const auto curvature = extreme.curvature_magnitude(0.0);
        const long double expected =
            2.0L / (3.0L * static_cast<long double>(scale));
        passed = require(
                     curvature && *curvature > 0.0 &&
                         close_scalar(*curvature, expected, 2.0e-12),
                     exponent < 0
                         ? "near-singular nonzero derivative was misclassified"
                         : "scale-aware extreme curvature evaluation differs") &&
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
                 unrepresentable.curvature_magnitude(0.0),
                 CurveError::non_finite_result,
                 "unrepresentable positive curvature did not fail explicitly") &&
             passed;

    passed = require_error(
                 parabola2.curvature_magnitude(
                     std::numeric_limits<double>::quiet_NaN()),
                 CurveError::non_finite_parameter,
                 "NaN curvature parameter was accepted") &&
             passed;
    passed = require_error(
                 parabola2.curvature_magnitude(-0.25),
                 CurveError::parameter_out_of_domain,
                 "below-domain curvature parameter was accepted") &&
             passed;
    passed = require_error(
                 polynomial3.curvature_magnitude(1.25),
                 CurveError::parameter_out_of_domain,
                 "above-domain curvature parameter was accepted") &&
             passed;

    const auto signed_zero = parabola2.curvature_magnitude(-0.0);
    const auto positive_zero = parabola2.curvature_magnitude(0.0);
    passed = require(
                 signed_zero && positive_zero && *signed_zero == *positive_zero,
                 "signed-zero curvature parameter changed semantics") &&
             passed;

    const auto repeat_a = polynomial3.curvature_magnitude(0.375);
    const auto repeat_b = polynomial3.curvature_magnitude(0.375);
    passed = require(
                 repeat_a && repeat_b && *repeat_a == *repeat_b,
                 "curvature query was not deterministic") &&
             passed;

    return passed ? 0 : 1;
}
