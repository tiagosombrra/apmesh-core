#include "apmesh/core/geometry.hpp"

#include <array>
#include <cmath>
#include <cstdio>
#include <expected>
#include <limits>
#include <string_view>
#include <type_traits>
#include <utility>

namespace {

using apmesh::core::CartesianFrame2;
using apmesh::core::CartesianFrame3;
using apmesh::core::GeometryError;
using apmesh::core::Mat2;
using apmesh::core::Mat3;
using apmesh::core::Point2;
using apmesh::core::Point3;
using apmesh::core::Vector2;
using apmesh::core::Vector3;

bool require(const bool condition, const std::string_view message) {
    if (!condition) {
        std::fputs(message.data(), stderr);
        std::fputc('\n', stderr);
    }
    return condition;
}

template <typename Value>
bool require_value(
    const std::expected<Value, GeometryError>& value,
    const std::string_view message) {
    return require(value.has_value(), message);
}

template <typename Value>
bool require_error(
    const std::expected<Value, GeometryError>& value,
    const GeometryError error,
    const std::string_view message) {
    return require(!value.has_value() && value.error() == error, message);
}

template <typename Frame, typename Point>
concept PointWorldMappable = requires(const Frame& frame, const Point& point) {
    frame.point_to_world(point);
};

template <typename Frame, typename Vector>
concept VectorWorldMappable = requires(const Frame& frame, const Vector& vector) {
    frame.vector_to_world(vector);
};

} // namespace

int main() {
    static_assert(PointWorldMappable<CartesianFrame2, Point2>);
    static_assert(!PointWorldMappable<CartesianFrame2, Point3>);
    static_assert(VectorWorldMappable<CartesianFrame3, Vector3>);
    static_assert(!VectorWorldMappable<CartesianFrame3, Vector2>);
    static_assert(std::same_as<decltype(std::declval<const CartesianFrame2&>().basis()), const Mat2&>);
    static_assert(std::same_as<decltype(std::declval<const CartesianFrame3&>().origin()), const Point3&>);

    bool passed = true;
    const auto zero2 = Point2::make(0.0, 0.0);
    const auto zero3 = Point3::make(0.0, 0.0, 0.0);
    const auto local2 = Point2::make(1.0, 2.0);
    const auto local3 = Point3::make(1.0, 2.0, 3.0);
    const auto vector2 = Vector2::make(1.0, 2.0);
    const auto vector3 = Vector3::make(1.0, 2.0, 3.0);
    if (!zero2 || !zero3 || !local2 || !local3 || !vector2 || !vector3) {
        return 1;
    }

    const auto identity2 = CartesianFrame2::identity();
    const auto identity3 = CartesianFrame3::identity();
    passed = require(identity2.point_to_world(*local2) == local2,
                     "2D identity did not preserve point") && passed;
    passed = require(identity2.vector_to_world(*vector2) == vector2,
                     "2D identity did not preserve vector") && passed;
    passed = require(identity3.point_to_local(*local3) == local3,
                     "3D identity did not preserve point") && passed;
    passed = require(identity3.vector_to_local(*vector3) == vector3,
                     "3D identity did not preserve vector") && passed;

    const auto quarter_turn = Mat2::make({0.0, -1.0, 1.0, 0.0});
    const auto origin2 = Point2::make(10.0, 20.0);
    if (!quarter_turn || !origin2) {
        return 1;
    }
    const auto frame2 = CartesianFrame2::make(*origin2, *quarter_turn, 1);
    passed = require_value(frame2, "quarter-turn frame construction failed") && passed;
    if (frame2) {
        const auto mapped_point = frame2->point_to_world(*local2);
        const auto mapped_vector = frame2->vector_to_world(*vector2);
        const auto expected_point = Point2::make(6.0, 22.0);
        const auto expected_vector = Vector2::make(-4.0, 2.0);
        passed = require(mapped_point && expected_point && *mapped_point == *expected_point,
                         "2D quarter-turn/scaled point differs") && passed;
        passed = require(mapped_vector && expected_vector && *mapped_vector == *expected_vector,
                         "2D quarter-turn/scaled vector differs") && passed;
        passed = require(mapped_point && frame2->point_to_local(*mapped_point) == local2,
                         "2D point round trip differs") && passed;
        passed = require(mapped_vector && frame2->vector_to_local(*mapped_vector) == vector2,
                         "2D vector round trip differs") && passed;

        const auto translated_origin = Point2::make(-5.0, 7.0);
        const auto translated_frame = translated_origin
            ? CartesianFrame2::make(*translated_origin, *quarter_turn, 1)
            : std::expected<CartesianFrame2, GeometryError>{std::unexpected{GeometryError::non_finite_input}};
        passed = require(translated_frame && translated_frame->vector_to_world(*vector2) == mapped_vector,
                         "frame origin changed vector mapping") && passed;

        const auto displaced = *local2 + *vector2;
        const auto mapped_displaced = displaced ? frame2->point_to_world(*displaced)
                                                : std::expected<Point2, GeometryError>{std::unexpected{GeometryError::non_finite_result}};
        const auto point_plus_vector = mapped_point && mapped_vector ? *mapped_point + *mapped_vector
                                                                       : std::expected<Point2, GeometryError>{std::unexpected{GeometryError::non_finite_result}};
        passed = require(mapped_displaced && point_plus_vector && *mapped_displaced == *point_plus_vector,
                         "2D affine compatibility differs") && passed;
        const auto difference = *local2 - *zero2;
        const auto mapped_difference = difference ? frame2->vector_to_world(*difference)
                                                  : std::expected<Vector2, GeometryError>{std::unexpected{GeometryError::non_finite_result}};
        const auto mapped_zero = frame2->point_to_world(*zero2);
        const auto world_difference = mapped_point && mapped_zero ? *mapped_point - *mapped_zero
                                                                    : std::expected<Vector2, GeometryError>{std::unexpected{GeometryError::non_finite_result}};
        passed = require(mapped_difference && world_difference && *mapped_difference == *world_difference,
                         "2D difference compatibility differs") && passed;
    }

    const auto axis_cycle = Mat3::make({0.0, 1.0, 0.0,
                                        0.0, 0.0, 1.0,
                                        1.0, 0.0, 0.0});
    const auto origin3 = Point3::make(5.0, -2.0, 1.0);
    if (!axis_cycle || !origin3) {
        return 1;
    }
    const auto frame3 = CartesianFrame3::make(*origin3, *axis_cycle, -1);
    passed = require_value(frame3, "3D axis-cycle frame construction failed") && passed;
    if (frame3) {
        const auto mapped_vector = frame3->vector_to_world(*vector3);
        const auto expected_vector = Vector3::make(1.0, 1.5, 0.5);
        passed = require(mapped_vector && expected_vector && *mapped_vector == *expected_vector,
                         "3D axis-cycle/scaled vector differs") && passed;
        const auto mapped_point = frame3->point_to_world(*local3);
        const auto expected_point = Point3::make(6.0, -0.5, 1.5);
        passed = require(mapped_point && expected_point && *mapped_point == *expected_point,
                         "3D axis-cycle/scaled point differs") && passed;
        passed = require(mapped_point && frame3->point_to_local(*mapped_point) == local3,
                         "3D point round trip differs") && passed;
    }

    const auto reflection = Mat2::make({-1.0, 0.0, 0.0, 1.0});
    const auto reflected = reflection ? CartesianFrame2::make(*zero2, *reflection, 0)
                                      : std::expected<CartesianFrame2, GeometryError>{std::unexpected{GeometryError::invalid_frame}};
    const auto reflection_expected = Vector2::make(-1.0, 2.0);
    passed = require(reflected && reflection_expected && reflected->vector_to_world(*vector2) == reflection_expected,
                     "reflecting signed permutation was rejected or mapped incorrectly") && passed;

    for (const int exponent : {-8, -1, 0, 1, 8}) {
        const auto scaled = CartesianFrame2::make(*zero2, Mat2::identity(), exponent);
        const auto unit = Vector2::make(1.0, 0.0);
        const auto expected = Vector2::make(std::scalbn(1.0, exponent), 0.0);
        passed = require(scaled && unit && expected && scaled->vector_to_world(*unit) == expected,
                         "power-of-two vector scale differs") && passed;
        const auto dot_mapped = scaled && unit ? scaled->vector_to_world(*unit) : std::expected<Vector2, GeometryError>{std::unexpected{GeometryError::non_finite_result}};
        const auto dot_value = dot_mapped ? apmesh::core::dot(*dot_mapped, *dot_mapped)
                                          : std::expected<double, GeometryError>{std::unexpected{GeometryError::non_finite_result}};
        passed = require(dot_value && *dot_value == std::scalbn(1.0, 2 * exponent),
                         "metric power-of-two scale differs") && passed;
    }

    const auto signed_zero_basis = Mat2::make({-0.0, -1.0, 1.0, 0.0});
    passed = require(signed_zero_basis && CartesianFrame2::make(*zero2, *signed_zero_basis, 0),
                     "signed zero basis entry was not treated as zero") && passed;
    const auto duplicate_basis = Mat2::make({1.0, 0.0, 1.0, 0.0});
    const auto non_unit_basis = Mat2::make({2.0, 0.0, 0.0, 1.0});
    passed = duplicate_basis && require_error(CartesianFrame2::make(*zero2, *duplicate_basis, 0),
                                              GeometryError::invalid_frame,
                                              "duplicate signed-permutation column accepted") && passed;
    passed = non_unit_basis && require_error(CartesianFrame2::make(*zero2, *non_unit_basis, 0),
                                             GeometryError::invalid_frame,
                                             "non-unit basis entry accepted") && passed;
    passed = require_error(CartesianFrame2::make(*zero2, Mat2::identity(), 1024),
                           GeometryError::scale_out_of_range,
                           "infinite scale exponent accepted") && passed;
    passed = require_error(CartesianFrame2::make(*zero2, Mat2::identity(), -1075),
                           GeometryError::scale_out_of_range,
                           "zero scale exponent accepted") && passed;

    const auto scale_two = CartesianFrame2::make(*zero2, Mat2::identity(), 1);
    const auto maximum_point = Point2::make(std::numeric_limits<double>::max(), 0.0);
    passed = scale_two && maximum_point &&
             require_error(scale_two->point_to_world(*maximum_point),
                           GeometryError::non_finite_result,
                           "overflowing frame map was accepted") && passed;

    return passed ? 0 : 1;
}
