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
        .absolute_tolerance = 3.0e-14 * reference_scale,
        .relative_tolerance = 3.0e-14,
        .reference_scale = reference_scale,
    };
    const auto comparison = apmesh::core::compare_proximity(
        actual,
        static_cast<double>(expected),
        policy);
    return comparison.has_value() &&
           comparison->result == apmesh::core::ProximityResult::within;
}

bool close_vector(
    const apmesh::core::Vector2& actual,
    const long double x,
    const long double y,
    const double reference_scale = 1.0) {
    return close_scalar(actual.x(), x, reference_scale) &&
           close_scalar(actual.y(), y, reference_scale);
}

bool close_vector(
    const apmesh::core::Vector3& actual,
    const long double x,
    const long double y,
    const long double z,
    const double reference_scale = 1.0) {
    return close_scalar(actual.x(), x, reference_scale) &&
           close_scalar(actual.y(), y, reference_scale) &&
           close_scalar(actual.z(), z, reference_scale);
}

bool close_vectors(
    const apmesh::core::Vector2& lhs,
    const apmesh::core::Vector2& rhs,
    const double reference_scale = 1.0) {
    return close_vector(
        lhs,
        static_cast<long double>(rhs.x()),
        static_cast<long double>(rhs.y()),
        reference_scale);
}

bool close_vectors(
    const apmesh::core::Vector3& lhs,
    const apmesh::core::Vector3& rhs,
    const double reference_scale = 1.0) {
    return close_vector(
        lhs,
        static_cast<long double>(rhs.x()),
        static_cast<long double>(rhs.y()),
        static_cast<long double>(rhs.z()),
        reference_scale);
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

    const auto d20 = curve2.first_derivative(0.0);
    const auto d21 = curve2.first_derivative(1.0);
    const auto d30 = curve3.first_derivative(0.0);
    const auto d31 = curve3.first_derivative(1.0);
    const auto expected_d20 = Vector2::make(0.0, 6.0);
    const auto expected_d21 = Vector2::make(0.0, -6.0);
    const auto expected_d30 = Vector3::make(3.0, 6.0, 12.0);
    const auto expected_d31 = Vector3::make(3.0, 3.0, 18.0);
    passed = require(d20 && d21 && expected_d20 && expected_d21 &&
                         *d20 == *expected_d20 && *d21 == *expected_d21,
                     "2D endpoint first derivatives differ") && passed;
    passed = require(d30 && d31 && expected_d30 && expected_d31 &&
                         *d30 == *expected_d30 && *d31 == *expected_d31,
                     "3D endpoint first derivatives differ") && passed;

    const auto dd20 = curve2.second_derivative(0.0);
    const auto dd21 = curve2.second_derivative(1.0);
    const auto dd30 = curve3.second_derivative(0.0);
    const auto dd31 = curve3.second_derivative(1.0);
    const auto expected_dd20 = Vector2::make(12.0, -12.0);
    const auto expected_dd21 = Vector2::make(-12.0, -12.0);
    const auto expected_dd30 = Vector3::make(6.0, -30.0, -36.0);
    const auto expected_dd31 = Vector3::make(-6.0, 24.0, 48.0);
    passed = require(dd20 && dd21 && expected_dd20 && expected_dd21 &&
                         *dd20 == *expected_dd20 && *dd21 == *expected_dd21,
                     "2D endpoint second derivatives differ") && passed;
    passed = require(dd30 && dd31 && expected_dd30 && expected_dd31 &&
                         *dd30 == *expected_dd30 && *dd31 == *expected_dd31,
                     "3D endpoint second derivatives differ") && passed;

    const auto constant_point2 = Point2::make(-3.0, 5.0);
    const auto constant_point3 = Point3::make(2.0, -4.0, 8.0);
    if (!constant_point2 || !constant_point3) {
        return 1;
    }
    const CubicBezier2 constant2{
        *constant_point2, *constant_point2, *constant_point2, *constant_point2};
    const CubicBezier3 constant3{
        *constant_point3, *constant_point3, *constant_point3, *constant_point3};
    for (const double parameter : {0.0, 0.25, 0.5, 0.75, 1.0}) {
        const auto first2 = constant2.first_derivative(parameter);
        const auto second2 = constant2.second_derivative(parameter);
        const auto speed2 = constant2.speed(parameter);
        const auto first3 = constant3.first_derivative(parameter);
        const auto second3 = constant3.second_derivative(parameter);
        const auto speed3 = constant3.speed(parameter);
        passed = require(first2 && second2 && speed2 &&
                             first2->x() == 0.0 && first2->y() == 0.0 &&
                             second2->x() == 0.0 && second2->y() == 0.0 &&
                             *speed2 == 0.0,
                         "constant 2D cubic differential semantics differ") && passed;
        passed = require(first3 && second3 && speed3 &&
                             first3->x() == 0.0 && first3->y() == 0.0 &&
                             first3->z() == 0.0 &&
                             second3->x() == 0.0 && second3->y() == 0.0 &&
                             second3->z() == 0.0 && *speed3 == 0.0,
                         "constant 3D cubic differential semantics differ") && passed;
    }

    const auto l0 = Point2::make(0.0, 0.0);
    const auto l1 = Point2::make(1.0, 0.0);
    const auto l2 = Point2::make(2.0, 0.0);
    const auto l3 = Point2::make(3.0, 0.0);
    if (!l0 || !l1 || !l2 || !l3) {
        return 1;
    }
    const CubicBezier2 linear{*l0, *l1, *l2, *l3};
    for (const double parameter : {0.0, 0.125, 0.5, 0.875, 1.0}) {
        const auto first = linear.first_derivative(parameter);
        const auto second = linear.second_derivative(parameter);
        const auto speed = linear.speed(parameter);
        passed = require(first && second && speed &&
                             first->x() == 3.0 && first->y() == 0.0 &&
                             second->x() == 0.0 && second->y() == 0.0 &&
                             *speed == 3.0,
                         "linear-equivalent cubic differential semantics differ") && passed;
    }

    const auto q0 = Point2::make(0.0, 0.0);
    const auto q1 = Point2::make(1.0, 0.0);
    const auto q2 = Point2::make(4.0, 0.0);
    const auto q3 = Point2::make(9.0, 0.0);
    if (!q0 || !q1 || !q2 || !q3) {
        return 1;
    }
    const CubicBezier2 quadratic_equivalent{*q0, *q1, *q2, *q3};
    const auto expected_quadratic_second = Vector2::make(12.0, 0.0);
    if (!expected_quadratic_second) {
        return 1;
    }
    for (const double parameter : {0.0, 0.25, 0.5, 0.75, 1.0}) {
        const auto first = quadratic_equivalent.first_derivative(parameter);
        const auto second = quadratic_equivalent.second_derivative(parameter);
        const long double expected_first =
            3.0L + 12.0L * static_cast<long double>(parameter);
        passed = require(first && second &&
                             close_vector(*first, expected_first, 0.0L, 16.0) &&
                             *second == *expected_quadratic_second,
                         "quadratic-equivalent differential reference differs") && passed;
    }

    for (const double parameter : {0.125, 0.25, 0.5, 0.75, 0.875}) {
        const auto first2 = curve2.first_derivative(parameter);
        const auto second2 = curve2.second_derivative(parameter);
        const auto speed2 = curve2.speed(parameter);
        const long double t = static_cast<long double>(parameter);
        const auto& controls2 = curve2.control_points();
        const long double first2_x =
            first_reference(controls2, t, &Point2::x);
        const long double first2_y =
            first_reference(controls2, t, &Point2::y);
        passed = require(
                     first2 && second2 && speed2 &&
                         close_vector(*first2, first2_x, first2_y, 16.0) &&
                         close_vector(
                             *second2,
                             second_reference(controls2, t, &Point2::x),
                             second_reference(controls2, t, &Point2::y),
                             32.0) &&
                         close_scalar(
                             *speed2,
                             std::sqrt(first2_x * first2_x + first2_y * first2_y),
                             16.0),
                     "2D genuine cubic differential reference differs") &&
                 passed;

        const auto first3 = curve3.first_derivative(parameter);
        const auto second3 = curve3.second_derivative(parameter);
        const auto speed3 = curve3.speed(parameter);
        const auto& controls3 = curve3.control_points();
        const long double first3_x =
            first_reference(controls3, t, &Point3::x);
        const long double first3_y =
            first_reference(controls3, t, &Point3::y);
        const long double first3_z =
            first_reference(controls3, t, &Point3::z);
        passed = require(
                     first3 && second3 && speed3 &&
                         close_vector(
                             *first3,
                             first3_x,
                             first3_y,
                             first3_z,
                             32.0) &&
                         close_vector(
                             *second3,
                             second_reference(controls3, t, &Point3::x),
                             second_reference(controls3, t, &Point3::y),
                             second_reference(controls3, t, &Point3::z),
                             64.0) &&
                         close_scalar(
                             *speed3,
                             std::sqrt(
                                 first3_x * first3_x +
                                 first3_y * first3_y +
                                 first3_z * first3_z),
                             32.0),
                     "3D genuine cubic differential reference differs") &&
                 passed;
    }

    const auto s0 = Point2::make(0.0, 0.0);
    const auto s1 = Point2::make(1.0, 0.0);
    const auto s2 = Point2::make(1.0, 0.0);
    const auto s3 = Point2::make(0.0, 0.0);
    if (!s0 || !s1 || !s2 || !s3) {
        return 1;
    }
    const CubicBezier2 stationary{*s0, *s1, *s2, *s3};
    const auto stationary_first = stationary.first_derivative(0.5);
    const auto stationary_speed = stationary.speed(0.5);
    passed = require(stationary_first && stationary_speed &&
                         stationary_first->x() == 0.0 &&
                         stationary_first->y() == 0.0 &&
                         *stationary_speed == 0.0,
                     "interior stationary point was not preserved") && passed;

    const auto s30 = Point3::make(0.0, 0.0, 0.0);
    const auto s31 = Point3::make(1.0, 0.0, 0.0);
    const auto s32 = Point3::make(1.0, 0.0, 0.0);
    const auto s33 = Point3::make(0.0, 0.0, 0.0);
    if (!s30 || !s31 || !s32 || !s33) {
        return 1;
    }
    const CubicBezier3 stationary3{*s30, *s31, *s32, *s33};
    const auto stationary_first3 = stationary3.first_derivative(0.5);
    const auto stationary_speed3 = stationary3.speed(0.5);
    passed = require(stationary_first3 && stationary_speed3 &&
                         stationary_first3->x() == 0.0 &&
                         stationary_first3->y() == 0.0 &&
                         stationary_first3->z() == 0.0 &&
                         *stationary_speed3 == 0.0,
                     "3D interior stationary point was not preserved") && passed;

    const CubicBezier2 endpoint_stationary{*p20, *p20, *p22, *p23};
    const auto endpoint_stationary_first =
        endpoint_stationary.first_derivative(0.0);
    passed = require(endpoint_stationary_first &&
                         endpoint_stationary_first->x() == 0.0 &&
                         endpoint_stationary_first->y() == 0.0,
                     "endpoint stationary derivative differs") && passed;

    const auto reversed2 = curve2.reversed();
    const auto reversed3 = curve3.reversed();
    for (const double parameter : {0.0, 0.125, 0.5, 0.875, 1.0}) {
        const auto reverse_first2 = reversed2.first_derivative(parameter);
        const auto forward_first2 = curve2.first_derivative(1.0 - parameter);
        const auto reverse_second2 = reversed2.second_derivative(parameter);
        const auto forward_second2 = curve2.second_derivative(1.0 - parameter);
        const auto reverse_speed2 = reversed2.speed(parameter);
        const auto forward_speed2 = curve2.speed(1.0 - parameter);
        passed = require(
                     reverse_first2 && forward_first2 &&
                         reverse_second2 && forward_second2 &&
                         reverse_speed2 && forward_speed2 &&
                         close_vectors(*reverse_first2, -*forward_first2, 16.0) &&
                         close_vectors(*reverse_second2, *forward_second2, 32.0) &&
                         close_scalar(*reverse_speed2, *forward_speed2, 16.0),
                     "2D reversal differential identities differ") &&
                 passed;

        const auto reverse_first3 = reversed3.first_derivative(parameter);
        const auto forward_first3 = curve3.first_derivative(1.0 - parameter);
        const auto reverse_second3 = reversed3.second_derivative(parameter);
        const auto forward_second3 = curve3.second_derivative(1.0 - parameter);
        const auto reverse_speed3 = reversed3.speed(parameter);
        const auto forward_speed3 = curve3.speed(1.0 - parameter);
        passed = require(
                     reverse_first3 && forward_first3 &&
                         reverse_second3 && forward_second3 &&
                         reverse_speed3 && forward_speed3 &&
                         close_vectors(*reverse_first3, -*forward_first3, 32.0) &&
                         close_vectors(*reverse_second3, *forward_second3, 64.0) &&
                         close_scalar(*reverse_speed3, *forward_speed3, 32.0),
                     "3D reversal differential identities differ") &&
                 passed;
    }

    const auto translation2 = Vector2::make(8.0, -4.0);
    const auto translation3 = Vector3::make(-2.0, 4.0, 6.0);
    if (!translation2 || !translation3) {
        return 1;
    }
    auto translated2 = curve2.control_points();
    auto translated3 = curve3.control_points();
    for (std::size_t index = 0; index < 4; ++index) {
        const auto point2 = translated2[index] + *translation2;
        const auto point3 = translated3[index] + *translation3;
        if (!point2 || !point3) {
            return 1;
        }
        translated2[index] = *point2;
        translated3[index] = *point3;
    }
    const CubicBezier2 translated_curve2{
        translated2[0], translated2[1], translated2[2], translated2[3]};
    const CubicBezier3 translated_curve3{
        translated3[0], translated3[1], translated3[2], translated3[3]};
    for (const double parameter : {0.25, 0.5, 0.75}) {
        const auto first2 = curve2.first_derivative(parameter);
        const auto translated_first2 =
            translated_curve2.first_derivative(parameter);
        const auto second2 = curve2.second_derivative(parameter);
        const auto translated_second2 =
            translated_curve2.second_derivative(parameter);
        const auto speed2 = curve2.speed(parameter);
        const auto translated_speed2 = translated_curve2.speed(parameter);
        passed = require(first2 && translated_first2 && second2 &&
                             translated_second2 && speed2 && translated_speed2 &&
                             close_vectors(*first2, *translated_first2, 16.0) &&
                             close_vectors(*second2, *translated_second2, 32.0) &&
                             close_scalar(*speed2, *translated_speed2, 16.0),
                         "2D translation invariance differs") && passed;

        const auto first3 = curve3.first_derivative(parameter);
        const auto translated_first3 =
            translated_curve3.first_derivative(parameter);
        const auto second3 = curve3.second_derivative(parameter);
        const auto translated_second3 =
            translated_curve3.second_derivative(parameter);
        const auto speed3 = curve3.speed(parameter);
        const auto translated_speed3 = translated_curve3.speed(parameter);
        passed = require(first3 && translated_first3 && second3 &&
                             translated_second3 && speed3 && translated_speed3 &&
                             close_vectors(*first3, *translated_first3, 32.0) &&
                             close_vectors(*second3, *translated_second3, 64.0) &&
                             close_scalar(*speed3, *translated_speed3, 32.0),
                         "3D translation invariance differs") && passed;
    }

    const auto frame_origin2 = Point2::make(2.0, -1.0);
    const auto frame_basis2 = Mat2::make({0.0, -1.0, 1.0, 0.0});
    const auto frame2 = frame_origin2 && frame_basis2
        ? CartesianFrame2::make(*frame_origin2, *frame_basis2, 1)
        : std::expected<CartesianFrame2, apmesh::core::GeometryError>{
              std::unexpected{apmesh::core::GeometryError::invalid_frame}};
    const auto frame_origin3 = Point3::make(-1.0, 2.0, 4.0);
    const auto frame_basis3 = Mat3::make({
        0.0, 1.0, 0.0,
        0.0, 0.0, -1.0,
        -1.0, 0.0, 0.0});
    const auto frame3 = frame_origin3 && frame_basis3
        ? CartesianFrame3::make(*frame_origin3, *frame_basis3, -1)
        : std::expected<CartesianFrame3, apmesh::core::GeometryError>{
              std::unexpected{apmesh::core::GeometryError::invalid_frame}};
    if (!frame2 || !frame3) {
        return 1;
    }

    auto framed2 = curve2.control_points();
    auto framed3 = curve3.control_points();
    for (std::size_t index = 0; index < 4; ++index) {
        const auto mapped2 = frame2->point_to_world(framed2[index]);
        const auto mapped3 = frame3->point_to_world(framed3[index]);
        if (!mapped2 || !mapped3) {
            return 1;
        }
        framed2[index] = *mapped2;
        framed3[index] = *mapped3;
    }
    const CubicBezier2 framed_curve2{
        framed2[0], framed2[1], framed2[2], framed2[3]};
    const CubicBezier3 framed_curve3{
        framed3[0], framed3[1], framed3[2], framed3[3]};

    for (const double parameter : {0.25, 0.5, 0.75}) {
        const auto local_first2 = curve2.first_derivative(parameter);
        const auto local_second2 = curve2.second_derivative(parameter);
        const auto mapped_first2 =
            local_first2 ? frame2->vector_to_world(*local_first2)
                         : std::expected<Vector2, apmesh::core::GeometryError>{
                               std::unexpected{apmesh::core::GeometryError::non_finite_result}};
        const auto mapped_second2 =
            local_second2 ? frame2->vector_to_world(*local_second2)
                          : std::expected<Vector2, apmesh::core::GeometryError>{
                                std::unexpected{apmesh::core::GeometryError::non_finite_result}};
        const auto direct_first2 = framed_curve2.first_derivative(parameter);
        const auto direct_second2 = framed_curve2.second_derivative(parameter);
        passed = require(mapped_first2 && mapped_second2 && direct_first2 &&
                             direct_second2 &&
                             close_vectors(*mapped_first2, *direct_first2, 32.0) &&
                             close_vectors(*mapped_second2, *direct_second2, 64.0),
                         "2D admitted-frame differential covariance differs") && passed;

        const auto local_first3 = curve3.first_derivative(parameter);
        const auto local_second3 = curve3.second_derivative(parameter);
        const auto mapped_first3 =
            local_first3 ? frame3->vector_to_world(*local_first3)
                         : std::expected<Vector3, apmesh::core::GeometryError>{
                               std::unexpected{apmesh::core::GeometryError::non_finite_result}};
        const auto mapped_second3 =
            local_second3 ? frame3->vector_to_world(*local_second3)
                          : std::expected<Vector3, apmesh::core::GeometryError>{
                                std::unexpected{apmesh::core::GeometryError::non_finite_result}};
        const auto direct_first3 = framed_curve3.first_derivative(parameter);
        const auto direct_second3 = framed_curve3.second_derivative(parameter);
        passed = require(mapped_first3 && mapped_second3 && direct_first3 &&
                             direct_second3 &&
                             close_vectors(*mapped_first3, *direct_first3, 64.0) &&
                             close_vectors(*mapped_second3, *direct_second3, 128.0),
                         "3D admitted-frame differential covariance differs") && passed;
    }

    const double maximum = std::numeric_limits<double>::max();
    const auto extreme0 = Point2::make(-maximum, 0.0);
    const auto extreme1 = Point2::make(maximum, 0.0);
    if (!extreme0 || !extreme1) {
        return 1;
    }
    const CubicBezier2 extreme{
        *extreme0, *extreme1, *extreme1, *extreme0};
    passed = require_error(
                 extreme.first_derivative(0.5),
                 CurveError::non_finite_result,
                 "first-derivative overflow was not reported") &&
             passed;
    passed = require_error(
                 extreme.second_derivative(0.5),
                 CurveError::non_finite_result,
                 "second-derivative overflow was not reported") &&
             passed;
    passed = require_error(
                 extreme.speed(0.5),
                 CurveError::non_finite_result,
                 "speed overflow was not reported") &&
             passed;

    const double tiny = std::numeric_limits<double>::min();
    const auto tiny0 = Point2::make(0.0, 0.0);
    const auto tiny1 = Point2::make(tiny, 0.0);
    const auto tiny2 = Point2::make(2.0 * tiny, 0.0);
    const auto tiny3 = Point2::make(3.0 * tiny, 0.0);
    if (!tiny0 || !tiny1 || !tiny2 || !tiny3) {
        return 1;
    }
    const CubicBezier2 tiny_curve{*tiny0, *tiny1, *tiny2, *tiny3};
    const auto tiny_first = tiny_curve.first_derivative(0.5);
    const auto tiny_speed = tiny_curve.speed(0.5);
    passed = require(tiny_first && tiny_speed &&
                         tiny_first->x() > 0.0 &&
                         *tiny_speed > 0.0,
                     "small nonzero derivative was silently classified as zero") && passed;

    for (const double bad :
         {std::numeric_limits<double>::quiet_NaN(),
          std::numeric_limits<double>::infinity(),
          -std::numeric_limits<double>::infinity()}) {
        passed = require_error(
                     curve2.first_derivative(bad),
                     CurveError::non_finite_parameter,
                     "non-finite first-derivative parameter was accepted") &&
                 passed;
        passed = require_error(
                     curve2.second_derivative(bad),
                     CurveError::non_finite_parameter,
                     "non-finite second-derivative parameter was accepted") &&
                 passed;
        passed = require_error(
                     curve2.speed(bad),
                     CurveError::non_finite_parameter,
                     "non-finite speed parameter was accepted") &&
                 passed;
    }

    for (const double bad : {-0.25, 1.25}) {
        passed = require_error(
                     curve2.first_derivative(bad),
                     CurveError::parameter_out_of_domain,
                     "out-of-domain first-derivative parameter was accepted") &&
                 passed;
        passed = require_error(
                     curve2.second_derivative(bad),
                     CurveError::parameter_out_of_domain,
                     "out-of-domain second-derivative parameter was accepted") &&
                 passed;
        passed = require_error(
                     curve2.speed(bad),
                     CurveError::parameter_out_of_domain,
                     "out-of-domain speed parameter was accepted") &&
                 passed;
    }

    const auto signed_first = curve2.first_derivative(-0.0);
    const auto positive_first = curve2.first_derivative(0.0);
    const auto signed_second = curve2.second_derivative(-0.0);
    const auto positive_second = curve2.second_derivative(0.0);
    const auto signed_speed = curve2.speed(-0.0);
    const auto positive_speed = curve2.speed(0.0);
    passed = require(signed_first && positive_first &&
                         signed_second && positive_second &&
                         signed_speed && positive_speed &&
                         *signed_first == *positive_first &&
                         *signed_second == *positive_second &&
                         *signed_speed == *positive_speed,
                     "signed-zero differential parameter changed semantics") && passed;

    const auto repeated_first_a = curve3.first_derivative(0.375);
    const auto repeated_first_b = curve3.first_derivative(0.375);
    const auto repeated_second_a = curve3.second_derivative(0.375);
    const auto repeated_second_b = curve3.second_derivative(0.375);
    const auto repeated_speed_a = curve3.speed(0.375);
    const auto repeated_speed_b = curve3.speed(0.375);
    passed = require(repeated_first_a && repeated_first_b &&
                         repeated_second_a && repeated_second_b &&
                         repeated_speed_a && repeated_speed_b &&
                         *repeated_first_a == *repeated_first_b &&
                         *repeated_second_a == *repeated_second_b &&
                         *repeated_speed_a == *repeated_speed_b,
                     "differential queries were not deterministic") && passed;

    return passed ? 0 : 1;
}
