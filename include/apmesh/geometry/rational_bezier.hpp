#pragma once

#include "apmesh/geometry/parametric_curve.hpp"

#include <array>
#include <expected>

namespace apmesh::core {

enum class RationalBezierConstructionError {
    non_finite_weight,
    non_positive_weight,
};

class RationalQuadraticBezier2 {
public:
    [[nodiscard]] static std::expected<
        RationalQuadraticBezier2,
        RationalBezierConstructionError>
    make(
        const Point2& p0,
        const Point2& p1,
        const Point2& p2,
        double w0,
        double w1,
        double w2) noexcept;

    [[nodiscard]] const std::array<Point2, 3>& control_points() const noexcept;
    [[nodiscard]] const std::array<double, 3>& weights() const noexcept;
    [[nodiscard]] CurveParameterDomain parameter_domain() const noexcept;
    [[nodiscard]] std::expected<Point2, CurveError>
    evaluate(double parameter) const noexcept;
    [[nodiscard]] std::expected<Vector2, CurveError>
    first_derivative(double parameter) const noexcept;
    [[nodiscard]] std::expected<Vector2, CurveError>
    second_derivative(double parameter) const noexcept;
    [[nodiscard]] RationalQuadraticBezier2 reversed() const noexcept;

    [[nodiscard]] bool operator==(
        const RationalQuadraticBezier2&) const noexcept = default;

private:
    constexpr RationalQuadraticBezier2(
        const std::array<Point2, 3>& control_points,
        const std::array<double, 3>& weights) noexcept
        : control_points_(control_points), weights_(weights) {}

    std::array<Point2, 3> control_points_;
    std::array<double, 3> weights_;
};

class RationalQuadraticBezier3 {
public:
    [[nodiscard]] static std::expected<
        RationalQuadraticBezier3,
        RationalBezierConstructionError>
    make(
        const Point3& p0,
        const Point3& p1,
        const Point3& p2,
        double w0,
        double w1,
        double w2) noexcept;

    [[nodiscard]] const std::array<Point3, 3>& control_points() const noexcept;
    [[nodiscard]] const std::array<double, 3>& weights() const noexcept;
    [[nodiscard]] CurveParameterDomain parameter_domain() const noexcept;
    [[nodiscard]] std::expected<Point3, CurveError>
    evaluate(double parameter) const noexcept;
    [[nodiscard]] std::expected<Vector3, CurveError>
    first_derivative(double parameter) const noexcept;
    [[nodiscard]] std::expected<Vector3, CurveError>
    second_derivative(double parameter) const noexcept;
    [[nodiscard]] RationalQuadraticBezier3 reversed() const noexcept;

    [[nodiscard]] bool operator==(
        const RationalQuadraticBezier3&) const noexcept = default;

private:
    constexpr RationalQuadraticBezier3(
        const std::array<Point3, 3>& control_points,
        const std::array<double, 3>& weights) noexcept
        : control_points_(control_points), weights_(weights) {}

    std::array<Point3, 3> control_points_;
    std::array<double, 3> weights_;
};

} // namespace apmesh::core
