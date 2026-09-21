#include "apmesh/geometry/curve.hpp"

#include <array>
#include <concepts>
#include <expected>
#include <type_traits>

int main() {
    using apmesh::core::CubicBezier2;
    using apmesh::core::CubicBezier3;
    using apmesh::core::CurveError;
    using apmesh::core::CurveLengthError;
    using apmesh::core::CurveLengthEvidence;
    using apmesh::core::CurveLengthPolicy;
    using apmesh::core::CurveRegularityError;
    using apmesh::core::CurveRegularityEvidence;
    using apmesh::core::CurveRegularityPolicy;
    using apmesh::core::Point2;
    using apmesh::core::Point3;
    using apmesh::core::Vector2;
    using apmesh::core::Vector3;

    static_assert(std::is_copy_constructible_v<CubicBezier2>);
    static_assert(std::is_copy_assignable_v<CubicBezier2>);
    static_assert(std::is_copy_constructible_v<CubicBezier3>);
    static_assert(std::same_as<
        decltype(std::declval<const CubicBezier2&>().evaluate(0.5)),
        std::expected<Point2, CurveError>>);
    static_assert(std::same_as<
        decltype(std::declval<const CubicBezier3&>().evaluate(0.5)),
        std::expected<Point3, CurveError>>);
    static_assert(std::same_as<
        decltype(std::declval<const CubicBezier2&>().first_derivative(0.5)),
        std::expected<Vector2, CurveError>>);
    static_assert(std::same_as<
        decltype(std::declval<const CubicBezier3&>().first_derivative(0.5)),
        std::expected<Vector3, CurveError>>);
    static_assert(std::same_as<
        decltype(std::declval<const CubicBezier2&>().second_derivative(0.5)),
        std::expected<Vector2, CurveError>>);
    static_assert(std::same_as<
        decltype(std::declval<const CubicBezier3&>().second_derivative(0.5)),
        std::expected<Vector3, CurveError>>);
    static_assert(std::same_as<
        decltype(std::declval<const CubicBezier2&>().speed(0.5)),
        std::expected<double, CurveError>>);
    static_assert(std::same_as<
        decltype(std::declval<const CubicBezier3&>().speed(0.5)),
        std::expected<double, CurveError>>);
    static_assert(std::same_as<
        decltype(std::declval<const CubicBezier2&>().certify_regularity(
            std::declval<const CurveRegularityPolicy&>())),
        std::expected<CurveRegularityEvidence, CurveRegularityError>>);
    static_assert(std::same_as<
        decltype(std::declval<const CubicBezier3&>().certify_regularity(
            std::declval<const CurveRegularityPolicy&>())),
        std::expected<CurveRegularityEvidence, CurveRegularityError>>);
    static_assert(std::same_as<
        decltype(std::declval<const CubicBezier2&>().arc_length_enclosure(
            std::declval<const CurveLengthPolicy&>())),
        std::expected<CurveLengthEvidence, CurveLengthError>>);
    static_assert(std::same_as<
        decltype(std::declval<const CubicBezier3&>().arc_length_enclosure(
            std::declval<const CurveLengthPolicy&>())),
        std::expected<CurveLengthEvidence, CurveLengthError>>);
    static_assert(std::same_as<
        decltype(std::declval<const CubicBezier2&>().control_points()),
        const std::array<Point2, 4>&>);
    static_assert(std::same_as<
        decltype(std::declval<const CubicBezier3&>().control_points()),
        const std::array<Point3, 4>&>);

    const auto point2 = Point2::make(0.0, 0.0);
    const auto point3 = Point3::make(0.0, 0.0, 0.0);
    if (!point2 || !point3) {
        return 1;
    }

    const CubicBezier2 curve2{*point2, *point2, *point2, *point2};
    const CubicBezier3 curve3{*point3, *point3, *point3, *point3};
    const CurveRegularityPolicy policy{
        .max_subdivision_depth = 8,
        .max_processed_nodes = 128,
    };
    const CurveLengthPolicy length_policy{
        .absolute_tolerance = 0.0,
        .relative_tolerance = 0.0,
        .max_subdivision_depth = 8,
        .max_processed_nodes = 128,
    };
    return curve2.evaluate(0.5).has_value() &&
                   curve3.evaluate(0.5).has_value() &&
                   curve2.first_derivative(0.5).has_value() &&
                   curve3.first_derivative(0.5).has_value() &&
                   curve2.second_derivative(0.5).has_value() &&
                   curve3.second_derivative(0.5).has_value() &&
                   curve2.speed(0.5).has_value() &&
                   curve3.speed(0.5).has_value() &&
                   curve2.certify_regularity(policy).has_value() &&
                   curve3.certify_regularity(policy).has_value() &&
                   curve2.arc_length_enclosure(length_policy).has_value() &&
                   curve3.arc_length_enclosure(length_policy).has_value()
               ? 0
               : 1;
}
