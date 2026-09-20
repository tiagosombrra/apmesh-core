#pragma once

#include "apmesh/core/geometry.hpp"

#include <array>
#include <expected>

namespace apmesh::core {

enum class CurveError {
    non_finite_parameter,
    parameter_out_of_domain,
    non_finite_result,
};

class CubicBezier2 {
public:
    constexpr CubicBezier2(
        const Point2& p0,
        const Point2& p1,
        const Point2& p2,
        const Point2& p3) noexcept
        : control_points_{p0, p1, p2, p3} {}

    [[nodiscard]] const std::array<Point2, 4>& control_points() const noexcept;
    [[nodiscard]] std::expected<Point2, CurveError> evaluate(double parameter) const noexcept;
    [[nodiscard]] std::expected<Vector2, CurveError> first_derivative(double parameter) const noexcept;
    [[nodiscard]] std::expected<Vector2, CurveError> second_derivative(double parameter) const noexcept;
    [[nodiscard]] std::expected<double, CurveError> speed(double parameter) const noexcept;
    [[nodiscard]] CubicBezier2 reversed() const noexcept;

    [[nodiscard]] bool operator==(const CubicBezier2&) const noexcept = default;

private:
    std::array<Point2, 4> control_points_;
};

class CubicBezier3 {
public:
    constexpr CubicBezier3(
        const Point3& p0,
        const Point3& p1,
        const Point3& p2,
        const Point3& p3) noexcept
        : control_points_{p0, p1, p2, p3} {}

    [[nodiscard]] const std::array<Point3, 4>& control_points() const noexcept;
    [[nodiscard]] std::expected<Point3, CurveError> evaluate(double parameter) const noexcept;
    [[nodiscard]] std::expected<Vector3, CurveError> first_derivative(double parameter) const noexcept;
    [[nodiscard]] std::expected<Vector3, CurveError> second_derivative(double parameter) const noexcept;
    [[nodiscard]] std::expected<double, CurveError> speed(double parameter) const noexcept;
    [[nodiscard]] CubicBezier3 reversed() const noexcept;

    [[nodiscard]] bool operator==(const CubicBezier3&) const noexcept = default;

private:
    std::array<Point3, 4> control_points_;
};

} // namespace apmesh::core
