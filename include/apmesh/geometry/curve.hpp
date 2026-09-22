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
    singular_parameter,
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

enum class CurveInflectionError {
    invalid_policy,
    curve_not_regular,
    non_finite_enclosure,
};

enum class CurveInflectionIsolationResult {
    complete,
    indeterminate,
};

struct CurveInflectionBracket {
    double lower_parameter{};
    double upper_parameter{};

    [[nodiscard]] bool operator==(const CurveInflectionBracket&) const noexcept = default;
};

struct CurveInflectionIsolationPolicy {
    CurveRegularityPolicy regularity_policy{};
    double parameter_tolerance{};
    std::size_t max_subdivision_depth{};
    std::size_t max_processed_nodes{};
};

struct CurveInflectionIsolationEvidence {
    CurveInflectionIsolationResult result{};
    CurveRegularityEvidence regularity{};
    std::array<CurveInflectionBracket, 2> brackets{};
    std::size_t inflection_count{};
    std::size_t processed_nodes{};
    std::size_t root_free_leaves{};
    std::size_t isolated_root_leaves{};
    std::size_t max_depth_reached{};

    [[nodiscard]] bool operator==(
        const CurveInflectionIsolationEvidence&) const noexcept = default;
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


enum class CurveInverseLengthError {
    invalid_policy,
    non_finite_target,
    target_out_of_domain,
    target_domain_indeterminate,
    regularity_not_certified,
    non_finite_enclosure,
};

enum class CurveInverseLengthResult {
    converged,
    indeterminate,
};

struct CurveInverseLengthPolicy {
    CurveLengthPolicy length_policy{};
    CurveRegularityPolicy regularity_policy{};
    double parameter_tolerance{};
    std::size_t max_refinement_iterations{};
};

struct CurveInverseLengthEvidence {
    CurveInverseLengthResult result{};
    double target_lower_length{};
    double target_upper_length{};
    double lower_parameter{};
    double upper_parameter{};
    CurveLengthEvidence lower_cumulative{};
    CurveLengthEvidence upper_cumulative{};
    CurveLengthEvidence total_length{};
    CurveRegularityEvidence regularity{};
    std::size_t refinement_iterations{};

    [[nodiscard]] bool operator==(const CurveInverseLengthEvidence&) const noexcept = default;
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
    [[nodiscard]] std::expected<double, CurveError>
    curvature_magnitude(double parameter) const noexcept;
    [[nodiscard]] std::expected<double, CurveError>
    signed_curvature(double parameter) const noexcept;
    [[nodiscard]] std::expected<CurveRegularityEvidence, CurveRegularityError>
    certify_regularity(const CurveRegularityPolicy& policy) const noexcept;
    [[nodiscard]] std::expected<CurveInflectionIsolationEvidence, CurveInflectionError>
    isolate_simple_inflections(
        const CurveInflectionIsolationPolicy& policy) const noexcept;
    [[nodiscard]] std::expected<CurveLengthEvidence, CurveLengthError>
    arc_length_enclosure(const CurveLengthPolicy& policy) const noexcept;
    [[nodiscard]] std::expected<CurveLengthEvidence, CurveLengthError>
    cumulative_arc_length_enclosure(
        double parameter,
        const CurveLengthPolicy& policy) const noexcept;
    [[nodiscard]] std::expected<CurveInverseLengthEvidence, CurveInverseLengthError>
    inverse_arc_length_bracket(
        double target_length,
        const CurveInverseLengthPolicy& policy) const noexcept;
    [[nodiscard]] std::expected<CurveInverseLengthEvidence, CurveInverseLengthError>
    inverse_arc_length_fraction_bracket(
        double normalized_fraction,
        const CurveInverseLengthPolicy& policy) const noexcept;
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
    [[nodiscard]] std::expected<double, CurveError>
    curvature_magnitude(double parameter) const noexcept;
    [[nodiscard]] std::expected<CurveRegularityEvidence, CurveRegularityError>
    certify_regularity(const CurveRegularityPolicy& policy) const noexcept;
    [[nodiscard]] std::expected<CurveLengthEvidence, CurveLengthError>
    arc_length_enclosure(const CurveLengthPolicy& policy) const noexcept;
    [[nodiscard]] std::expected<CurveLengthEvidence, CurveLengthError>
    cumulative_arc_length_enclosure(
        double parameter,
        const CurveLengthPolicy& policy) const noexcept;
    [[nodiscard]] std::expected<CurveInverseLengthEvidence, CurveInverseLengthError>
    inverse_arc_length_bracket(
        double target_length,
        const CurveInverseLengthPolicy& policy) const noexcept;
    [[nodiscard]] std::expected<CurveInverseLengthEvidence, CurveInverseLengthError>
    inverse_arc_length_fraction_bracket(
        double normalized_fraction,
        const CurveInverseLengthPolicy& policy) const noexcept;
    [[nodiscard]] CubicBezier3 reversed() const noexcept;

    [[nodiscard]] bool operator==(const CubicBezier3&) const noexcept = default;

private:
    std::array<Point3, 4> control_points_;
};

} // namespace apmesh::core
