#pragma once

#include "apmesh/core/geometry.hpp"

#include <array>
#include <cstddef>
#include <expected>

namespace apmesh::core {

enum class CurveError {
    non_finite_parameter,
    parameter_out_of_domain,
    non_finite_result,
};

enum class CurveRegularityError {
    invalid_policy,
    non_finite_enclosure,
};

enum class CurveRegularityResult {
    regular,
    degenerate,
    indeterminate,
};

struct CurveRegularityPolicy {
    std::size_t max_subdivision_depth{};
    std::size_t max_processed_nodes{};
};

struct CurveRegularityEvidence {
    CurveRegularityResult result{};
    std::size_t processed_nodes{};
    std::size_t certified_leaves{};
    std::size_t max_depth_reached{};

    [[nodiscard]] bool operator==(const CurveRegularityEvidence&) const noexcept = default;
};


enum class CurveLengthError {
    invalid_policy,
    non_finite_parameter,
    parameter_out_of_domain,
    non_finite_enclosure,
};

enum class CurveLengthResult {
    converged,
    indeterminate,
};

struct CurveLengthPolicy {
    double absolute_tolerance{};
    double relative_tolerance{};
    std::size_t max_subdivision_depth{};
    std::size_t max_processed_nodes{};
};

struct CurveLengthEvidence {
    CurveLengthResult result{};
    double lower_length{};
    double upper_length{};
    std::size_t processed_nodes{};
    std::size_t accepted_leaves{};
    std::size_t max_depth_reached{};

    [[nodiscard]] bool operator==(const CurveLengthEvidence&) const noexcept = default;
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
    [[nodiscard]] std::expected<CurveRegularityEvidence, CurveRegularityError>
    certify_regularity(const CurveRegularityPolicy& policy) const noexcept;
    [[nodiscard]] std::expected<CurveLengthEvidence, CurveLengthError>
    arc_length_enclosure(const CurveLengthPolicy& policy) const noexcept;
    [[nodiscard]] std::expected<CurveLengthEvidence, CurveLengthError>
    cumulative_arc_length_enclosure(
        double parameter,
        const CurveLengthPolicy& policy) const noexcept;
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
    [[nodiscard]] std::expected<CurveRegularityEvidence, CurveRegularityError>
    certify_regularity(const CurveRegularityPolicy& policy) const noexcept;
    [[nodiscard]] std::expected<CurveLengthEvidence, CurveLengthError>
    arc_length_enclosure(const CurveLengthPolicy& policy) const noexcept;
    [[nodiscard]] std::expected<CurveLengthEvidence, CurveLengthError>
    cumulative_arc_length_enclosure(
        double parameter,
        const CurveLengthPolicy& policy) const noexcept;
    [[nodiscard]] CubicBezier3 reversed() const noexcept;

    [[nodiscard]] bool operator==(const CubicBezier3&) const noexcept = default;

private:
    std::array<Point3, 4> control_points_;
};

} // namespace apmesh::core
