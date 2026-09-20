#include "apmesh/geometry/curve.hpp"

#include <array>
#include <concepts>
#include <expected>
#include <type_traits>

int main() {
    using apmesh::core::CubicBezier2;
    using apmesh::core::CubicBezier3;
    using apmesh::core::CurveError;
    using apmesh::core::Point2;
    using apmesh::core::Point3;

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
    return curve2.evaluate(0.5).has_value() &&
                   curve3.evaluate(0.5).has_value()
               ? 0
               : 1;
}
