#include "apmesh/core/geometry.hpp"
#include "apmesh/core/numeric.hpp"

#include <cmath>
#include <concepts>
#include <cstdio>
#include <expected>
#include <limits>
#include <string_view>
#include <type_traits>
#include <utility>

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
    const std::expected<Value, apmesh::core::GeometryError>& value,
    const apmesh::core::GeometryError expected,
    const std::string_view message) {
    return require(!value.has_value() && value.error() == expected, message);
}

template <typename Value>
bool require_value(const std::expected<Value, apmesh::core::GeometryError>& value,
                   const std::string_view message) {
    return require(value.has_value(), message);
}

template <typename Left, typename Right>
concept Addable = requires(Left lhs, Right rhs) { lhs + rhs; };

template <typename Left, typename Right>
concept Subtractable = requires(Left lhs, Right rhs) { lhs - rhs; };

template <typename Value>
concept Scalable = requires(Value value) { value * 2.0; };

bool close(const double lhs, const double rhs) {
    constexpr apmesh::core::ProximityPolicy normalized_three_four_policy{
        .absolute_tolerance = 1.0e-15,
        .relative_tolerance = 1.0e-15,
        .reference_scale = 1.0,
    };
    const auto comparison = apmesh::core::compare_proximity(
        lhs,
        rhs,
        normalized_three_four_policy);
    return comparison.has_value() && comparison->result == apmesh::core::ProximityResult::within;
}

} // namespace

int main() {
    using apmesh::core::GeometryError;
    using apmesh::core::Point2;
    using apmesh::core::Point3;
    using apmesh::core::Vector2;
    using apmesh::core::Vector3;

    static_assert(!std::is_convertible_v<Point2, Vector2>);
    static_assert(!std::is_convertible_v<Vector2, Point2>);
    static_assert(Addable<Point2, Vector2>);
    static_assert(Addable<Vector2, Vector2>);
    static_assert(!Addable<Point2, Point2>);
    static_assert(Subtractable<Point3, Point3>);
    static_assert(!Scalable<Point3>);
    static_assert(Scalable<Vector3>);
    static_assert(std::same_as<decltype(std::declval<Point3>() - std::declval<Point3>()),
                               std::expected<Vector3, GeometryError>>);

    bool passed = true;
    const auto point2 = Point2::make(1.0, -2.0);
    const auto point3 = Point3::make(1.0, 2.0, 3.0);
    const auto vector2 = Vector2::make(3.0, 4.0);
    const auto vector3 = Vector3::make(1.0, 0.0, 0.0);
    passed = require_value(point2, "finite Point2 construction failed") && passed;
    passed = require_value(point3, "finite Point3 construction failed") && passed;
    passed = require_value(vector2, "finite Vector2 construction failed") && passed;
    passed = require_value(vector3, "finite Vector3 construction failed") && passed;
    if (!point2 || !point3 || !vector2 || !vector3) {
        return 1;
    }

    const auto translated = *point2 + *vector2;
    passed = require_value(translated, "point translation failed") && passed;
    if (translated) {
        passed = require(translated->x() == 4.0 && translated->y() == 2.0,
                         "point translation has incorrect coordinates") && passed;
        const auto recovered = *translated - *vector2;
        passed = require_value(recovered, "inverse point translation failed") && passed;
        passed = require(recovered && *recovered == *point2,
                         "inverse point translation did not recover point") && passed;
    }

    const auto displacement = *point2 - *point2;
    passed = require_value(displacement, "point displacement failed") && passed;
    passed = require(displacement && displacement->x() == 0.0 && displacement->y() == 0.0,
                     "self displacement is not zero") && passed;

    const auto e1 = Vector3::make(1.0, 0.0, 0.0);
    const auto e2 = Vector3::make(0.0, 1.0, 0.0);
    const auto e3 = Vector3::make(0.0, 0.0, 1.0);
    if (!e1 || !e2 || !e3) {
        return 1;
    }
    const auto dot_same = apmesh::core::dot(*e1, *e1);
    const auto dot_distinct = apmesh::core::dot(*e1, *e2);
    passed = require(dot_same && *dot_same == 1.0 && dot_distinct && *dot_distinct == 0.0,
                     "basis dot products differ") && passed;
    const auto cross = apmesh::core::cross(*e1, *e2);
    const auto reverse_cross = apmesh::core::cross(*e2, *e1);
    passed = require(cross && *cross == *e3, "basis cross product differs") && passed;
    passed = require(reverse_cross && reverse_cross->x() == 0.0 && reverse_cross->y() == 0.0 && reverse_cross->z() == -1.0,
                     "cross product antisymmetry differs") && passed;

    const auto length = apmesh::core::norm(*vector2);
    const auto normalized = apmesh::core::normalize(*vector2);
    passed = require(length && *length == 5.0, "3-4 norm differs") && passed;
    passed = require(normalized && close(normalized->x(), 0.6) && close(normalized->y(), 0.8),
                     "normalized 3-4 vector differs") && passed;

    for (const double invalid : {std::numeric_limits<double>::quiet_NaN(),
                                 std::numeric_limits<double>::infinity(),
                                 -std::numeric_limits<double>::infinity()}) {
        passed = require_error(Point2::make(invalid, 0.0), GeometryError::non_finite_input,
                               "Point2 x accepted invalid numeric input") && passed;
        passed = require_error(Point2::make(0.0, invalid), GeometryError::non_finite_input,
                               "Point2 y accepted invalid numeric input") && passed;
        passed = require_error(Vector2::make(invalid, 0.0), GeometryError::non_finite_input,
                               "Vector2 x accepted invalid numeric input") && passed;
        passed = require_error(Vector2::make(0.0, invalid), GeometryError::non_finite_input,
                               "Vector2 y accepted invalid numeric input") && passed;
        passed = require_error(Point3::make(invalid, 0.0, 0.0), GeometryError::non_finite_input,
                               "Point3 x accepted invalid numeric input") && passed;
        passed = require_error(Point3::make(0.0, invalid, 0.0), GeometryError::non_finite_input,
                               "Point3 y accepted invalid numeric input") && passed;
        passed = require_error(Point3::make(0.0, 0.0, invalid), GeometryError::non_finite_input,
                               "Point3 z accepted invalid numeric input") && passed;
        passed = require_error(Vector3::make(invalid, 0.0, 0.0), GeometryError::non_finite_input,
                               "Vector3 x accepted invalid numeric input") && passed;
        passed = require_error(Vector3::make(0.0, invalid, 0.0), GeometryError::non_finite_input,
                               "Vector3 y accepted invalid numeric input") && passed;
        passed = require_error(Vector3::make(0.0, 0.0, invalid), GeometryError::non_finite_input,
                               "Vector3 z accepted invalid numeric input") && passed;
    }

    const auto positive_zero = Vector2::make(0.0, 0.0);
    const auto mixed_zero = Vector2::make(-0.0, 0.0);
    passed = require(positive_zero && mixed_zero && *positive_zero == *mixed_zero,
                     "signed zero vectors differ numerically") && passed;
    passed = require_error(apmesh::core::normalize(*positive_zero), GeometryError::zero_length,
                           "positive zero normalization was accepted") && passed;
    passed = require_error(apmesh::core::normalize(*mixed_zero), GeometryError::zero_length,
                           "mixed signed-zero normalization was accepted") && passed;

    const double subnormal = std::numeric_limits<double>::denorm_min();
    for (const double finite_extreme : {subnormal, std::numeric_limits<double>::min(),
                                        std::numeric_limits<double>::lowest(),
                                        std::numeric_limits<double>::max()}) {
        passed = require(Point2::make(finite_extreme, 0.0).has_value(),
                         "finite Point2 envelope value was rejected") && passed;
        passed = require(Vector3::make(0.0, finite_extreme, 0.0).has_value(),
                         "finite Vector3 envelope value was rejected") && passed;
    }
    const auto tiny = Vector2::make(subnormal, 0.0);
    const auto tiny_length = tiny ? apmesh::core::norm(*tiny) : std::expected<double, GeometryError>{std::unexpected{GeometryError::non_finite_input}};
    const auto tiny_normalized = tiny ? apmesh::core::normalize(*tiny) : std::expected<Vector2, GeometryError>{std::unexpected{GeometryError::non_finite_input}};
    passed = require(tiny_length && *tiny_length == subnormal,
                     "subnormal vector was not measured as nonzero") && passed;
    passed = require(tiny_normalized && tiny_normalized->x() == 1.0 && tiny_normalized->y() == 0.0,
                     "subnormal vector normalization differs") && passed;

    for (const int exponent : {-500, -100, 0, 100, 500}) {
        const double scale = std::ldexp(1.0, exponent);
        const auto scaled = Vector2::make(scale, 0.0);
        passed = require_value(scaled, "power-of-two vector construction failed") && passed;
        if (!scaled) {
            continue;
        }
        const auto scaled_length = apmesh::core::norm(*scaled);
        const auto scaled_normalized = apmesh::core::normalize(*scaled);
        passed = require(scaled_length && *scaled_length == scale,
                         "power-of-two norm differs") && passed;
        passed = require(scaled_normalized && scaled_normalized->x() == 1.0 && scaled_normalized->y() == 0.0,
                         "power-of-two normalization differs") && passed;
    }

    const double maximum = std::numeric_limits<double>::max();
    const double lowest = std::numeric_limits<double>::lowest();
    const auto largest_vector = Vector2::make(maximum, maximum);
    const auto lowest_vector = Vector2::make(lowest, lowest);
    const auto largest_point = Point2::make(maximum, maximum);
    const auto lowest_point = Point2::make(lowest, lowest);
    passed = require(largest_vector && lowest_vector && largest_point && lowest_point,
                     "finite extreme values were rejected") && passed;
    if (largest_vector && lowest_vector && largest_point && lowest_point) {
        passed = require_error(*largest_vector + *largest_vector, GeometryError::non_finite_result,
                               "overflowing vector addition was accepted") && passed;
        passed = require_error(*largest_vector - *lowest_vector, GeometryError::non_finite_result,
                               "overflowing vector subtraction was accepted") && passed;
        passed = require_error(*largest_vector * 2.0, GeometryError::non_finite_result,
                               "overflowing vector scaling was accepted") && passed;
        passed = require_error(*largest_vector / std::numeric_limits<double>::min(), GeometryError::non_finite_result,
                               "overflowing vector division was accepted") && passed;
        passed = require_error(*largest_point + *largest_vector, GeometryError::non_finite_result,
                               "overflowing point translation was accepted") && passed;
        passed = require_error(*largest_point - *lowest_vector, GeometryError::non_finite_result,
                               "overflowing inverse point translation was accepted") && passed;
        passed = require_error(*largest_point - *lowest_point, GeometryError::non_finite_result,
                               "overflowing point displacement was accepted") && passed;
        passed = require_error(apmesh::core::dot(*largest_vector, *largest_vector), GeometryError::non_finite_result,
                               "overflowing dot product was accepted") && passed;
        passed = require_error(apmesh::core::norm(*largest_vector), GeometryError::non_finite_result,
                               "overflowing norm was accepted") && passed;
    }

    const auto cross_maximum = apmesh::core::cross(*Vector3::make(maximum, maximum, 0.0), *Vector3::make(maximum, 0.0, maximum));
    passed = require_error(cross_maximum, GeometryError::non_finite_result,
                           "overflowing cross product was accepted") && passed;
    passed = require_error(*vector2 / 0.0, GeometryError::division_by_zero,
                           "zero vector divisor was accepted") && passed;
    passed = require_error(*vector2 * std::numeric_limits<double>::infinity(), GeometryError::non_finite_input,
                           "infinite vector scalar was accepted") && passed;
    passed = require_error(*vector2 / std::numeric_limits<double>::quiet_NaN(), GeometryError::non_finite_input,
                           "NaN vector divisor was accepted") && passed;

    return passed ? 0 : 1;
}
