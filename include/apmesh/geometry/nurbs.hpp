#pragma once

#include "apmesh/geometry/parametric_curve.hpp"

#include <array>
#include <cstddef>
#include <expected>
#include <span>
#include <vector>

namespace apmesh::core {

enum class NURBSConstructionError {
    non_finite_lower_knot,
    non_finite_interior_knot,
    non_finite_upper_knot,
    non_strict_knot_order,
    non_finite_weight,
    non_positive_weight,
};

class TwoSpanCubicNURBS2 {
public:
    [[nodiscard]] static std::expected<
        TwoSpanCubicNURBS2,
        NURBSConstructionError>
    make(
        const std::array<Point2, 5>& control_points,
        const std::array<double, 5>& weights,
        double lower_knot,
        double interior_knot,
        double upper_knot) noexcept;

    [[nodiscard]] const std::array<Point2, 5>&
    control_points() const noexcept;
    [[nodiscard]] const std::array<double, 5>& weights() const noexcept;
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
    [[nodiscard]] TwoSpanCubicNURBS2 reversed() const noexcept;

    [[nodiscard]] bool operator==(
        const TwoSpanCubicNURBS2&) const noexcept = default;

private:
    constexpr TwoSpanCubicNURBS2(
        const std::array<Point2, 5>& control_points,
        const std::array<double, 5>& weights,
        const double lower_knot,
        const double interior_knot,
        const double upper_knot) noexcept
        : control_points_(control_points),
          weights_(weights),
          lower_knot_(lower_knot),
          interior_knot_(interior_knot),
          upper_knot_(upper_knot) {}

    std::array<Point2, 5> control_points_;
    std::array<double, 5> weights_;
    double lower_knot_{};
    double interior_knot_{};
    double upper_knot_{};
};

class TwoSpanCubicNURBS3 {
public:
    [[nodiscard]] static std::expected<
        TwoSpanCubicNURBS3,
        NURBSConstructionError>
    make(
        const std::array<Point3, 5>& control_points,
        const std::array<double, 5>& weights,
        double lower_knot,
        double interior_knot,
        double upper_knot) noexcept;

    [[nodiscard]] const std::array<Point3, 5>&
    control_points() const noexcept;
    [[nodiscard]] const std::array<double, 5>& weights() const noexcept;
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
    [[nodiscard]] TwoSpanCubicNURBS3 reversed() const noexcept;

    [[nodiscard]] bool operator==(
        const TwoSpanCubicNURBS3&) const noexcept = default;

private:
    constexpr TwoSpanCubicNURBS3(
        const std::array<Point3, 5>& control_points,
        const std::array<double, 5>& weights,
        const double lower_knot,
        const double interior_knot,
        const double upper_knot) noexcept
        : control_points_(control_points),
          weights_(weights),
          lower_knot_(lower_knot),
          interior_knot_(interior_knot),
          upper_knot_(upper_knot) {}

    std::array<Point3, 5> control_points_;
    std::array<double, 5> weights_;
    double lower_knot_{};
    double interior_knot_{};
    double upper_knot_{};
};

enum class MultiSpanNURBSConstructionError {
    insufficient_control_points,
    control_weight_count_mismatch,
    interior_knot_count_mismatch,
    non_finite_lower_knot,
    non_finite_interior_knot,
    non_finite_upper_knot,
    non_strict_knot_order,
    non_finite_weight,
    non_positive_weight,
};

class MultiSpanCubicNURBS2 {
public:
    [[nodiscard]] static std::expected<
        MultiSpanCubicNURBS2,
        MultiSpanNURBSConstructionError>
    make(
        std::vector<Point2> control_points,
        std::vector<double> weights,
        std::vector<double> interior_knots,
        double lower_knot,
        double upper_knot);

    [[nodiscard]] std::span<const Point2> control_points() const noexcept;
    [[nodiscard]] std::span<const double> weights() const noexcept;
    [[nodiscard]] std::span<const double> interior_knots() const noexcept;
    [[nodiscard]] std::size_t span_count() const noexcept;
    [[nodiscard]] double lower_knot() const noexcept;
    [[nodiscard]] double upper_knot() const noexcept;

    [[nodiscard]] CurveParameterDomain parameter_domain() const noexcept;
    [[nodiscard]] std::expected<Point2, CurveError>
    evaluate(double parameter) const noexcept;
    [[nodiscard]] std::expected<Vector2, CurveError>
    first_derivative(double parameter) const noexcept;
    [[nodiscard]] std::expected<Vector2, CurveError>
    second_derivative(double parameter) const noexcept;
    [[nodiscard]] MultiSpanCubicNURBS2 reversed() const;

    [[nodiscard]] bool operator==(
        const MultiSpanCubicNURBS2&) const noexcept = default;

private:
    MultiSpanCubicNURBS2(
        std::vector<Point2> control_points,
        std::vector<double> weights,
        std::vector<double> interior_knots,
        double lower_knot,
        double upper_knot,
        bool constant);

    std::vector<Point2> control_points_;
    std::vector<double> weights_;
    std::vector<double> interior_knots_;
    double lower_knot_{};
    double upper_knot_{};
    bool constant_{};
};

class MultiSpanCubicNURBS3 {
public:
    [[nodiscard]] static std::expected<
        MultiSpanCubicNURBS3,
        MultiSpanNURBSConstructionError>
    make(
        std::vector<Point3> control_points,
        std::vector<double> weights,
        std::vector<double> interior_knots,
        double lower_knot,
        double upper_knot);

    [[nodiscard]] std::span<const Point3> control_points() const noexcept;
    [[nodiscard]] std::span<const double> weights() const noexcept;
    [[nodiscard]] std::span<const double> interior_knots() const noexcept;
    [[nodiscard]] std::size_t span_count() const noexcept;
    [[nodiscard]] double lower_knot() const noexcept;
    [[nodiscard]] double upper_knot() const noexcept;

    [[nodiscard]] CurveParameterDomain parameter_domain() const noexcept;
    [[nodiscard]] std::expected<Point3, CurveError>
    evaluate(double parameter) const noexcept;
    [[nodiscard]] std::expected<Vector3, CurveError>
    first_derivative(double parameter) const noexcept;
    [[nodiscard]] std::expected<Vector3, CurveError>
    second_derivative(double parameter) const noexcept;
    [[nodiscard]] MultiSpanCubicNURBS3 reversed() const;

    [[nodiscard]] bool operator==(
        const MultiSpanCubicNURBS3&) const noexcept = default;

private:
    MultiSpanCubicNURBS3(
        std::vector<Point3> control_points,
        std::vector<double> weights,
        std::vector<double> interior_knots,
        double lower_knot,
        double upper_knot,
        bool constant);

    std::vector<Point3> control_points_;
    std::vector<double> weights_;
    std::vector<double> interior_knots_;
    double lower_knot_{};
    double upper_knot_{};
    bool constant_{};
};

} // namespace apmesh::core
