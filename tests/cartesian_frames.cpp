#include "apmesh/core/geometry.hpp"

#include <cmath>
#include <concepts>
#include <cstdio>
#include <expected>
#include <limits>
#include <string_view>
#include <utility>

namespace {

using apmesh::core::CartesianFrame2;
using apmesh::core::CartesianFrame3;
using apmesh::core::GeometryError;
using apmesh::core::LinearAlgebraError;
using apmesh::core::Mat2;
using apmesh::core::Mat3;
using apmesh::core::Point2;
using apmesh::core::Point3;
using apmesh::core::Vector2;
using apmesh::core::Vector3;

[[nodiscard]] bool require(const bool condition, const std::string_view message) {
    if (!condition) {
        std::fputs(message.data(), stderr);
        std::fputc('\n', stderr);
    }
    return condition;
}

template <typename Value>
[[nodiscard]] bool has_value(
    const std::expected<Value, GeometryError>& result,
    const std::string_view message) {
    return require(result.has_value(), message);
}

template <typename Value>
[[nodiscard]] bool has_error(
    const std::expected<Value, GeometryError>& result,
    const GeometryError error,
    const std::string_view message) {
    return require(!result && result.error() == error, message);
}

template <typename Frame, typename Point>
concept PointWorldMappable = requires(const Frame& frame, const Point& point) {
    frame.point_to_world(point);
};

template <typename Frame, typename Vector>
concept VectorWorldMappable = requires(const Frame& frame, const Vector& vector) {
    frame.vector_to_world(vector);
};

template <typename Frame, typename Point>
concept PointLocalMappable = requires(const Frame& frame, const Point& point) {
    frame.point_to_local(point);
};

template <typename Frame, typename Vector>
concept VectorLocalMappable = requires(const Frame& frame, const Vector& vector) {
    frame.vector_to_local(vector);
};

template <typename Matrix, typename Point>
concept MatrixAppliesToPoint = requires(const Matrix& matrix, const Point& point) {
    apmesh::core::apply(matrix, point);
};

} // namespace

int main() {
    static_assert(PointWorldMappable<CartesianFrame2, Point2>);
    static_assert(!PointWorldMappable<CartesianFrame2, Point3>);
    static_assert(PointWorldMappable<CartesianFrame3, Point3>);
    static_assert(!PointWorldMappable<CartesianFrame3, Point2>);
    static_assert(VectorWorldMappable<CartesianFrame2, Vector2>);
    static_assert(!VectorWorldMappable<CartesianFrame2, Vector3>);
    static_assert(VectorWorldMappable<CartesianFrame3, Vector3>);
    static_assert(!VectorWorldMappable<CartesianFrame3, Vector2>);
    static_assert(PointLocalMappable<CartesianFrame2, Point2>);
    static_assert(!PointLocalMappable<CartesianFrame2, Point3>);
    static_assert(PointLocalMappable<CartesianFrame3, Point3>);
    static_assert(!PointLocalMappable<CartesianFrame3, Point2>);
    static_assert(VectorLocalMappable<CartesianFrame2, Vector2>);
    static_assert(!VectorLocalMappable<CartesianFrame2, Vector3>);
    static_assert(VectorLocalMappable<CartesianFrame3, Vector3>);
    static_assert(!VectorLocalMappable<CartesianFrame3, Vector2>);
    static_assert(!MatrixAppliesToPoint<Mat2, Point2>);
    static_assert(!MatrixAppliesToPoint<Mat3, Point3>);

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
    passed = require(identity2.point_to_world(*local2) == local2 &&
                         identity2.point_to_local(*local2) == local2 &&
                         identity2.vector_to_world(*vector2) == vector2 &&
                         identity2.vector_to_local(*vector2) == vector2,
                     "2D identity map differs") && passed;
    passed = require(identity3.point_to_world(*local3) == local3 &&
                         identity3.point_to_local(*local3) == local3 &&
                         identity3.vector_to_world(*vector3) == vector3 &&
                         identity3.vector_to_local(*vector3) == vector3,
                     "3D identity map differs") && passed;

    const auto quarter_turn = Mat2::make({0.0, -1.0, 1.0, 0.0});
    const auto origin2 = Point2::make(10.0, 20.0);
    if (!quarter_turn || !origin2) {
        return 1;
    }
    const auto frame2 = CartesianFrame2::make(*origin2, *quarter_turn, 1);
    passed = has_value(frame2, "2D frame construction failed") && passed;
    if (frame2) {
        const auto mapped_point = frame2->point_to_world(*local2);
        const auto mapped_vector = frame2->vector_to_world(*vector2);
        const auto expected_point = Point2::make(6.0, 22.0);
        const auto expected_vector = Vector2::make(-4.0, 2.0);
        passed = require(frame2->origin() == *origin2 && frame2->basis() == *quarter_turn &&
                             frame2->scale_exponent() == 1,
                         "2D frame accessors differ") && passed;
        passed = require(mapped_point && expected_point && *mapped_point == *expected_point &&
                             mapped_vector && expected_vector && *mapped_vector == *expected_vector,
                         "2D quarter-turn map differs") && passed;
        passed = require(mapped_point && frame2->point_to_local(*mapped_point) == local2 &&
                             mapped_vector && frame2->vector_to_local(*mapped_vector) == vector2,
                         "2D local-to-world round trip differs") && passed;
        const auto world_point = Point2::make(17.0, -4.0);
        const auto world_vector = Vector2::make(6.0, -10.0);
        const auto local_point = world_point ? frame2->point_to_local(*world_point)
                                             : std::expected<Point2, GeometryError>{std::unexpected{GeometryError::non_finite_input}};
        const auto local_vector = world_vector ? frame2->vector_to_local(*world_vector)
                                               : std::expected<Vector2, GeometryError>{std::unexpected{GeometryError::non_finite_input}};
        passed = require(world_point && world_vector && local_point && local_vector &&
                             frame2->point_to_world(*local_point) == world_point &&
                             frame2->vector_to_world(*local_vector) == world_vector,
                         "2D world-to-local round trip differs") && passed;

        const auto alternate_origin = Point2::make(-5.0, 7.0);
        const auto translated = alternate_origin
            ? CartesianFrame2::make(*alternate_origin, *quarter_turn, 1)
            : std::expected<CartesianFrame2, GeometryError>{std::unexpected{GeometryError::non_finite_input}};
        passed = require(translated && mapped_vector && translated->vector_to_world(*vector2) == mapped_vector &&
                             translated->point_to_world(*zero2) == alternate_origin,
                         "translation was applied to a vector or omitted from a point") && passed;

        const auto displaced = *local2 + *vector2;
        const auto mapped_displaced = displaced
            ? frame2->point_to_world(*displaced)
            : std::expected<Point2, GeometryError>{std::unexpected{GeometryError::non_finite_result}};
        const auto point_plus_vector = mapped_point && mapped_vector
            ? *mapped_point + *mapped_vector
            : std::expected<Point2, GeometryError>{std::unexpected{GeometryError::non_finite_result}};
        const auto local_difference = *local2 - *zero2;
        const auto mapped_difference = local_difference
            ? frame2->vector_to_world(*local_difference)
            : std::expected<Vector2, GeometryError>{std::unexpected{GeometryError::non_finite_result}};
        const auto mapped_zero = frame2->point_to_world(*zero2);
        const auto world_difference = mapped_point && mapped_zero
            ? *mapped_point - *mapped_zero
            : std::expected<Vector2, GeometryError>{std::unexpected{GeometryError::non_finite_result}};
        passed = require(mapped_displaced && point_plus_vector && *mapped_displaced == *point_plus_vector &&
                             mapped_difference && world_difference && *mapped_difference == *world_difference,
                         "2D affine or difference compatibility differs") && passed;
    }

    const auto axis_cycle = Mat3::make({0.0, 1.0, 0.0,
                                        0.0, 0.0, 1.0,
                                        1.0, 0.0, 0.0});
    const auto origin3 = Point3::make(5.0, -2.0, 1.0);
    if (!axis_cycle || !origin3) {
        return 1;
    }
    const auto frame3 = CartesianFrame3::make(*origin3, *axis_cycle, -1);
    passed = has_value(frame3, "3D frame construction failed") && passed;
    if (frame3) {
        const auto mapped_point = frame3->point_to_world(*local3);
        const auto mapped_vector = frame3->vector_to_world(*vector3);
        const auto expected_point = Point3::make(6.0, -0.5, 1.5);
        const auto expected_vector = Vector3::make(1.0, 1.5, 0.5);
        passed = require(mapped_point && expected_point && *mapped_point == *expected_point &&
                             mapped_vector && expected_vector && *mapped_vector == *expected_vector,
                         "3D axis-cycle map differs") && passed;
        passed = require(mapped_point && frame3->point_to_local(*mapped_point) == local3 &&
                             mapped_vector && frame3->vector_to_local(*mapped_vector) == vector3,
                         "3D local-to-world round trip differs") && passed;
        const auto world_point = Point3::make(7.0, 3.0, -5.0);
        const auto world_vector = Vector3::make(2.0, -4.0, 8.0);
        const auto local_point = world_point ? frame3->point_to_local(*world_point)
                                             : std::expected<Point3, GeometryError>{std::unexpected{GeometryError::non_finite_input}};
        const auto local_vector = world_vector ? frame3->vector_to_local(*world_vector)
                                               : std::expected<Vector3, GeometryError>{std::unexpected{GeometryError::non_finite_input}};
        passed = require(world_point && world_vector && local_point && local_vector &&
                             frame3->point_to_world(*local_point) == world_point &&
                             frame3->vector_to_world(*local_vector) == world_vector,
                         "3D world-to-local round trip differs") && passed;
    }

    const auto reflection = Mat2::make({-1.0, 0.0, 0.0, 1.0});
    const auto reflected = reflection ? CartesianFrame2::make(*zero2, *reflection, 0)
                                      : std::expected<CartesianFrame2, GeometryError>{std::unexpected{GeometryError::invalid_frame}};
    const auto reflected_value = Vector2::make(-1.0, 2.0);
    passed = require(reflected && reflected_value && reflected->vector_to_world(*vector2) == reflected_value,
                     "reflecting signed permutation differs") && passed;

    for (const int exponent : {-8, -1, 0, 1, 8}) {
        const auto scaled = CartesianFrame2::make(*zero2, Mat2::identity(), exponent);
        const auto unit_vector = Vector2::make(1.0, 0.0);
        const auto unit_point = Point2::make(1.0, 0.0);
        const auto expected_vector = Vector2::make(std::scalbn(1.0, exponent), 0.0);
        const auto expected_point = Point2::make(std::scalbn(1.0, exponent), 0.0);
        passed = require(scaled && unit_vector && unit_point && expected_vector && expected_point &&
                             scaled->vector_to_world(*unit_vector) == expected_vector &&
                             scaled->point_to_world(*unit_point) == expected_point &&
                             scaled->vector_to_local(*expected_vector) == unit_vector &&
                             scaled->point_to_local(*expected_point) == unit_point,
                         "power-of-two scale map differs") && passed;
    }

    const auto signed_zero_basis = Mat2::make({-0.0, -1.0, 1.0, 0.0});
    const auto duplicate_basis = Mat2::make({1.0, 0.0, 1.0, 0.0});
    const auto non_unit_basis = Mat2::make({2.0, 0.0, 0.0, 1.0});
    const auto missing_basis = Mat2::make({0.0, 0.0, 0.0, 1.0});
    const auto duplicate_basis3 = Mat3::make({1.0, 0.0, 0.0,
                                               1.0, 0.0, 0.0,
                                               0.0, 0.0, 1.0});
    const auto non_finite_basis = Mat2::make({std::numeric_limits<double>::quiet_NaN(), 0.0,
                                               0.0, 1.0});
    passed = require(signed_zero_basis && CartesianFrame2::make(*zero2, *signed_zero_basis, 0) &&
                         duplicate_basis && has_error(CartesianFrame2::make(*zero2, *duplicate_basis, 0), GeometryError::invalid_frame, "duplicate basis accepted") &&
                         non_unit_basis && has_error(CartesianFrame2::make(*zero2, *non_unit_basis, 0), GeometryError::invalid_frame, "non-unit basis accepted") &&
                         missing_basis && has_error(CartesianFrame2::make(*zero2, *missing_basis, 0), GeometryError::invalid_frame, "missing basis accepted") &&
                         duplicate_basis3 && has_error(CartesianFrame3::make(*zero3, *duplicate_basis3, 0), GeometryError::invalid_frame, "3D duplicate basis accepted") &&
                         !non_finite_basis && non_finite_basis.error() == LinearAlgebraError::non_finite_input,
                     "basis validation differs") && passed;

    passed = has_error(CartesianFrame2::make(*zero2, Mat2::identity(), 1024), GeometryError::scale_out_of_range, "infinite scale accepted") && passed;
    passed = has_error(CartesianFrame2::make(*zero2, Mat2::identity(), -1075), GeometryError::scale_out_of_range, "zero scale accepted") && passed;
    passed = has_error(CartesianFrame2::make(*zero2, Mat2::identity(), -1024), GeometryError::scale_out_of_range, "non-finite reciprocal accepted") && passed;
    passed = require(CartesianFrame2::make(*zero2, Mat2::identity(), 1023) &&
                         CartesianFrame2::make(*zero2, Mat2::identity(), -1023),
                     "finite reciprocal-safe scale boundary rejected") && passed;
    passed = has_error(CartesianFrame3::make(*zero3, Mat3::identity(), std::numeric_limits<int>::min()),
                       GeometryError::scale_out_of_range,
                       "minimum exponent accepted") && passed;

    const auto scale_two = CartesianFrame2::make(*zero2, Mat2::identity(), 1);
    const auto maximum_point = Point2::make(std::numeric_limits<double>::max(), 0.0);
    const auto maximum_vector = Vector2::make(std::numeric_limits<double>::max(), 0.0);
    passed = require(scale_two && maximum_point && maximum_vector &&
                         has_error(scale_two->point_to_world(*maximum_point), GeometryError::non_finite_result, "point overflow accepted") &&
                         has_error(scale_two->vector_to_world(*maximum_vector), GeometryError::non_finite_result, "vector overflow accepted"),
                     "non-finite mapped result differs") && passed;

    return passed ? 0 : 1;
}
