#pragma once

#include "apmesh/geometry/parametric_curve.hpp"

#include <array>
#include <expected>

namespace apmesh::core {

enum class BSplineConstructionError {
    non_finite_lower_knot,
    non_finite_interior_knot,
    non_finite_upper_knot,
    non_strict_knot_order,
};

class TwoSpanCubicBSpline2 {
public:
    [[nodiscard]] static std::expected<
        TwoSpanCubicBSpline2,
        BSplineConstructionError>
    make(
        const std::array<Point2, 5>& control_points,
        double lower_knot,
        double interior_knot,
        double upper_knot) noexcept;

    [[nodiscard]] const std::array<Point2, 5>&
    control_points() const noexcept;
    [[nodiscard]] std::array<double, 9> knots() const noexcept;
    [[nodiscard]] double lower_knot() const noexcept;
    [[nodiscard]] double interior_knot() const noexcept;
    [[nodiscard]] double upper_knot() const noexcept;

    [[nodiscard]] CurveParameterDomain parameter_domain() const noexcept;
    [[nodiscard]] std::expected<Point2, CurveError>
    evaluate(double parameter) const noexcept;
    [[nodiscard]] std::expected<Vector2, CurveError>
    first_derivative(double parameter) const noexcept;
    [[nodiscard]] std::expected<Vector2, CurveError>
    second_derivative(double parameter) const noexcept;
    [[nodiscard]] TwoSpanCubicBSpline2 reversed() const noexcept;

    [[nodiscard]] bool operator==(
        const TwoSpanCubicBSpline2&) const noexcept = default;

private:
    constexpr TwoSpanCubicBSpline2(
        const std::array<Point2, 5>& control_points,
        const double lower_knot,
        const double interior_knot,
        const double upper_knot) noexcept
        : control_points_(control_points),
          lower_knot_(lower_knot),
          interior_knot_(interior_knot),
          upper_knot_(upper_knot) {}

    std::array<Point2, 5> control_points_;
    double lower_knot_{};
    double interior_knot_{};
    double upper_knot_{};
};

class TwoSpanCubicBSpline3 {
public:
    [[nodiscard]] static std::expected<
        TwoSpanCubicBSpline3,
        BSplineConstructionError>
    make(
        const std::array<Point3, 5>& control_points,
        double lower_knot,
        double interior_knot,
        double upper_knot) noexcept;

    [[nodiscard]] const std::array<Point3, 5>&
    control_points() const noexcept;
    [[nodiscard]] std::array<double, 9> knots() const noexcept;
    [[nodiscard]] double lower_knot() const noexcept;
    [[nodiscard]] double interior_knot() const noexcept;
    [[nodiscard]] double upper_knot() const noexcept;

    [[nodiscard]] CurveParameterDomain parameter_domain() const noexcept;
    [[nodiscard]] std::expected<Point3, CurveError>
    evaluate(double parameter) const noexcept;
    [[nodiscard]] std::expected<Vector3, CurveError>
    first_derivative(double parameter) const noexcept;
    [[nodiscard]] std::expected<Vector3, CurveError>
    second_derivative(double parameter) const noexcept;
    [[nodiscard]] TwoSpanCubicBSpline3 reversed() const noexcept;

    [[nodiscard]] bool operator==(
        const TwoSpanCubicBSpline3&) const noexcept = default;

private:
    constexpr TwoSpanCubicBSpline3(
        const std::array<Point3, 5>& control_points,
        const double lower_knot,
        const double interior_knot,
        const double upper_knot) noexcept
        : control_points_(control_points),
          lower_knot_(lower_knot),
          interior_knot_(interior_knot),
          upper_knot_(upper_knot) {}

    std::array<Point3, 5> control_points_;
    double lower_knot_{};
    double interior_knot_{};
    double upper_knot_{};
};

} // namespace apmesh::core
