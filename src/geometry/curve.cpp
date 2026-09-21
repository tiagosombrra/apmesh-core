#include "apmesh/geometry/curve.hpp"

#include "apmesh/core/numeric.hpp"
#include "detail/curve_length_interval.hpp"
#include "detail/curve_regularity_interval.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <expected>
#include <limits>
#include <optional>

namespace apmesh::core {
namespace {

[[nodiscard]] std::expected<void, CurveError> validate_parameter(
    const double parameter) noexcept {
    if (!is_finite(parameter)) {
        return std::unexpected{CurveError::non_finite_parameter};
    }
    if (parameter < 0.0 || parameter > 1.0) {
        return std::unexpected{CurveError::parameter_out_of_domain};
    }
    return {};
}

[[nodiscard]] std::expected<Point2, CurveError> interpolate(
    const Point2& lhs,
    const Point2& rhs,
    const double parameter) noexcept {
    const auto point = Point2::make(
        std::lerp(lhs.x(), rhs.x(), parameter),
        std::lerp(lhs.y(), rhs.y(), parameter));
    if (!point.has_value()) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return *point;
}

[[nodiscard]] std::expected<Point3, CurveError> interpolate(
    const Point3& lhs,
    const Point3& rhs,
    const double parameter) noexcept {
    const auto point = Point3::make(
        std::lerp(lhs.x(), rhs.x(), parameter),
        std::lerp(lhs.y(), rhs.y(), parameter),
        std::lerp(lhs.z(), rhs.z(), parameter));
    if (!point.has_value()) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return *point;
}

[[nodiscard]] std::expected<Vector2, CurveError> interpolate(
    const Vector2& lhs,
    const Vector2& rhs,
    const double parameter) noexcept {
    const auto vector = Vector2::make(
        std::lerp(lhs.x(), rhs.x(), parameter),
        std::lerp(lhs.y(), rhs.y(), parameter));
    if (!vector.has_value()) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return *vector;
}

[[nodiscard]] std::expected<Vector3, CurveError> interpolate(
    const Vector3& lhs,
    const Vector3& rhs,
    const double parameter) noexcept {
    const auto vector = Vector3::make(
        std::lerp(lhs.x(), rhs.x(), parameter),
        std::lerp(lhs.y(), rhs.y(), parameter),
        std::lerp(lhs.z(), rhs.z(), parameter));
    if (!vector.has_value()) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return *vector;
}

template <typename Point>
[[nodiscard]] std::expected<Point, CurveError> evaluate_de_casteljau(
    const std::array<Point, 4>& points,
    const double parameter) noexcept {
    const auto q0 = interpolate(points[0], points[1], parameter);
    const auto q1 = interpolate(points[1], points[2], parameter);
    const auto q2 = interpolate(points[2], points[3], parameter);
    if (!q0 || !q1 || !q2) {
        return std::unexpected{CurveError::non_finite_result};
    }

    const auto r0 = interpolate(*q0, *q1, parameter);
    const auto r1 = interpolate(*q1, *q2, parameter);
    if (!r0 || !r1) {
        return std::unexpected{CurveError::non_finite_result};
    }

    return interpolate(*r0, *r1, parameter);
}

template <typename Vector>
[[nodiscard]] std::expected<Vector, CurveError> scale_vector(
    const Vector& vector,
    const double scalar) noexcept {
    const auto scaled = vector * scalar;
    if (!scaled.has_value()) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return *scaled;
}

template <typename Point, typename Vector>
[[nodiscard]] std::expected<std::array<Vector, 3>, CurveError>
first_derivative_controls(const std::array<Point, 4>& points) noexcept {
    const auto delta0 = points[1] - points[0];
    const auto delta1 = points[2] - points[1];
    const auto delta2 = points[3] - points[2];
    if (!delta0 || !delta1 || !delta2) {
        return std::unexpected{CurveError::non_finite_result};
    }

    const auto control0 = scale_vector(*delta0, 3.0);
    const auto control1 = scale_vector(*delta1, 3.0);
    const auto control2 = scale_vector(*delta2, 3.0);
    if (!control0 || !control1 || !control2) {
        return std::unexpected{CurveError::non_finite_result};
    }

    return std::array<Vector, 3>{*control0, *control1, *control2};
}

template <typename Vector>
[[nodiscard]] std::expected<std::array<Vector, 2>, CurveError>
second_derivative_controls(const std::array<Vector, 3>& first_controls) noexcept {
    const auto delta0 = first_controls[1] - first_controls[0];
    const auto delta1 = first_controls[2] - first_controls[1];
    if (!delta0 || !delta1) {
        return std::unexpected{CurveError::non_finite_result};
    }

    const auto control0 = scale_vector(*delta0, 2.0);
    const auto control1 = scale_vector(*delta1, 2.0);
    if (!control0 || !control1) {
        return std::unexpected{CurveError::non_finite_result};
    }

    return std::array<Vector, 2>{*control0, *control1};
}

template <typename Vector>
[[nodiscard]] std::expected<Vector, CurveError> evaluate_quadratic(
    const std::array<Vector, 3>& controls,
    const double parameter) noexcept {
    const auto q0 = interpolate(controls[0], controls[1], parameter);
    const auto q1 = interpolate(controls[1], controls[2], parameter);
    if (!q0 || !q1) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return interpolate(*q0, *q1, parameter);
}

template <typename Vector>
[[nodiscard]] std::expected<Vector, CurveError> evaluate_linear(
    const std::array<Vector, 2>& controls,
    const double parameter) noexcept {
    return interpolate(controls[0], controls[1], parameter);
}

template <typename Point, typename Vector>
[[nodiscard]] std::expected<Vector, CurveError> evaluate_first_derivative(
    const std::array<Point, 4>& points,
    const double parameter) noexcept {
    const auto valid = validate_parameter(parameter);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }

    const auto controls = first_derivative_controls<Point, Vector>(points);
    if (!controls.has_value()) {
        return std::unexpected{controls.error()};
    }
    return evaluate_quadratic(*controls, parameter);
}

template <typename Point, typename Vector>
[[nodiscard]] std::expected<Vector, CurveError> evaluate_second_derivative(
    const std::array<Point, 4>& points,
    const double parameter) noexcept {
    const auto valid = validate_parameter(parameter);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }

    const auto first_controls = first_derivative_controls<Point, Vector>(points);
    if (!first_controls.has_value()) {
        return std::unexpected{first_controls.error()};
    }
    const auto second_controls = second_derivative_controls(*first_controls);
    if (!second_controls.has_value()) {
        return std::unexpected{second_controls.error()};
    }
    return evaluate_linear(*second_controls, parameter);
}

template <typename Vector>
[[nodiscard]] std::expected<double, CurveError> vector_speed(
    const Vector& vector) noexcept {
    const auto value = norm(vector);
    if (!value.has_value()) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return *value;
}

constexpr std::size_t maximum_supported_length_depth = 64;

[[nodiscard]] std::expected<void, CurveLengthError> validate_length_policy(
    const CurveLengthPolicy& policy) noexcept {
    if (!std::isfinite(policy.absolute_tolerance) ||
        !std::isfinite(policy.relative_tolerance) ||
        policy.absolute_tolerance < 0.0 ||
        policy.relative_tolerance < 0.0 ||
        policy.max_processed_nodes == 0 ||
        policy.max_subdivision_depth > maximum_supported_length_depth) {
        return std::unexpected{CurveLengthError::invalid_policy};
    }
    return {};
}

[[nodiscard]] std::optional<double> exact_difference(
    const double lhs,
    const double rhs) noexcept {
    const double negated_rhs = -rhs;
    const double sum = lhs + negated_rhs;
    if (!std::isfinite(sum)) {
        return std::nullopt;
    }

    const double virtual_rhs = sum - lhs;
    const double error =
        (lhs - (sum - virtual_rhs)) + (negated_rhs - virtual_rhs);
    if (error != 0.0) {
        return std::nullopt;
    }
    return sum;
}

template <std::size_t Count>
[[nodiscard]] bool nondecreasing(
    const std::array<double, Count>& values) noexcept {
    for (std::size_t index = 1; index < Count; ++index) {
        if (values[index] < values[index - 1]) {
            return false;
        }
    }
    return true;
}

template <std::size_t Count>
[[nodiscard]] bool nonincreasing(
    const std::array<double, Count>& values) noexcept {
    for (std::size_t index = 1; index < Count; ++index) {
        if (values[index] > values[index - 1]) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] std::optional<double> exact_axis_monotone_length(
    const std::array<Point2, 4>& points) noexcept {
    const std::array<double, 4> x{
        points[0].x(), points[1].x(), points[2].x(), points[3].x()};
    const std::array<double, 4> y{
        points[0].y(), points[1].y(), points[2].y(), points[3].y()};

    const bool x_constant =
        x[0] == x[1] && x[1] == x[2] && x[2] == x[3];
    const bool y_constant =
        y[0] == y[1] && y[1] == y[2] && y[2] == y[3];

    if (x_constant && y_constant) {
        return 0.0;
    }

    const std::array<double, 4>* varying = nullptr;
    if (x_constant && !y_constant) {
        varying = &y;
    } else if (y_constant && !x_constant) {
        varying = &x;
    } else {
        return std::nullopt;
    }

    if (!nondecreasing(*varying) && !nonincreasing(*varying)) {
        return std::nullopt;
    }

    const auto difference = exact_difference((*varying)[3], (*varying)[0]);
    if (!difference) {
        return std::nullopt;
    }
    return std::abs(*difference);
}

[[nodiscard]] std::optional<double> exact_axis_monotone_length(
    const std::array<Point3, 4>& points) noexcept {
    const std::array<double, 4> x{
        points[0].x(), points[1].x(), points[2].x(), points[3].x()};
    const std::array<double, 4> y{
        points[0].y(), points[1].y(), points[2].y(), points[3].y()};
    const std::array<double, 4> z{
        points[0].z(), points[1].z(), points[2].z(), points[3].z()};

    const bool x_constant =
        x[0] == x[1] && x[1] == x[2] && x[2] == x[3];
    const bool y_constant =
        y[0] == y[1] && y[1] == y[2] && y[2] == y[3];
    const bool z_constant =
        z[0] == z[1] && z[1] == z[2] && z[2] == z[3];

    const std::size_t varying_count =
        static_cast<std::size_t>(!x_constant) +
        static_cast<std::size_t>(!y_constant) +
        static_cast<std::size_t>(!z_constant);

    if (varying_count == 0) {
        return 0.0;
    }
    if (varying_count != 1) {
        return std::nullopt;
    }

    const std::array<double, 4>* varying =
        !x_constant ? &x : (!y_constant ? &y : &z);
    if (!nondecreasing(*varying) && !nonincreasing(*varying)) {
        return std::nullopt;
    }

    const auto difference = exact_difference((*varying)[3], (*varying)[0]);
    if (!difference) {
        return std::nullopt;
    }
    return std::abs(*difference);
}

template <std::size_t Dimension>
using LengthEdges = std::array<detail::IntervalVector<Dimension>, 3>;

[[nodiscard]] std::expected<detail::ClosedInterval, CurveLengthError>
length_component_interval(
    const double next,
    const double current) noexcept {
    if (const auto exact = exact_difference(next, current)) {
        return detail::ClosedInterval{*exact, *exact};
    }

    const auto next_interval = detail::point_interval(next);
    const auto current_interval = detail::point_interval(current);
    if (!next_interval || !current_interval) {
        return std::unexpected{CurveLengthError::non_finite_enclosure};
    }
    const auto difference = detail::subtract(*next_interval, *current_interval);
    if (!difference) {
        return std::unexpected{CurveLengthError::non_finite_enclosure};
    }
    return *difference;
}

[[nodiscard]] std::expected<LengthEdges<2>, CurveLengthError>
length_edges(const std::array<Point2, 4>& points) noexcept {
    LengthEdges<2> result{};
    for (std::size_t edge = 0; edge < 3; ++edge) {
        const auto x = length_component_interval(
            points[edge + 1].x(),
            points[edge].x());
        const auto y = length_component_interval(
            points[edge + 1].y(),
            points[edge].y());
        if (!x || !y) {
            return std::unexpected{CurveLengthError::non_finite_enclosure};
        }
        result[edge] = detail::IntervalVector<2>{*x, *y};
    }
    return result;
}

[[nodiscard]] std::expected<LengthEdges<3>, CurveLengthError>
length_edges(const std::array<Point3, 4>& points) noexcept {
    LengthEdges<3> result{};
    for (std::size_t edge = 0; edge < 3; ++edge) {
        const auto x = length_component_interval(
            points[edge + 1].x(),
            points[edge].x());
        const auto y = length_component_interval(
            points[edge + 1].y(),
            points[edge].y());
        const auto z = length_component_interval(
            points[edge + 1].z(),
            points[edge].z());
        if (!x || !y || !z) {
            return std::unexpected{CurveLengthError::non_finite_enclosure};
        }
        result[edge] = detail::IntervalVector<3>{*x, *y, *z};
    }
    return result;
}

struct LengthBounds {
    double lower{};
    double upper{};
};

template <std::size_t Dimension>
[[nodiscard]] std::expected<LengthBounds, CurveLengthError>
length_bounds(const LengthEdges<Dimension>& edges) noexcept {
    const auto first_sum = detail::add_vectors(edges[0], edges[1]);
    if (!first_sum) {
        return std::unexpected{CurveLengthError::non_finite_enclosure};
    }
    const auto chord = detail::add_vectors(*first_sum, edges[2]);
    if (!chord) {
        return std::unexpected{CurveLengthError::non_finite_enclosure};
    }

    const auto chord_norm = detail::euclidean_norm_bounds(*chord);
    const auto edge0_norm = detail::euclidean_norm_bounds(edges[0]);
    const auto edge1_norm = detail::euclidean_norm_bounds(edges[1]);
    const auto edge2_norm = detail::euclidean_norm_bounds(edges[2]);
    if (!chord_norm || !edge0_norm || !edge1_norm || !edge2_norm) {
        return std::unexpected{CurveLengthError::non_finite_enclosure};
    }

    double upper = edge0_norm->upper;
    for (const double value : {edge1_norm->upper, edge2_norm->upper}) {
        if (value == 0.0) {
            continue;
        }
        if (upper == 0.0) {
            upper = value;
            continue;
        }
        const double raw = upper + value;
        if (!std::isfinite(raw)) {
            return std::unexpected{CurveLengthError::non_finite_enclosure};
        }
        const auto widened = detail::widen_up(raw);
        if (!widened) {
            return std::unexpected{CurveLengthError::non_finite_enclosure};
        }
        upper = *widened;
    }

    if (chord_norm->lower > upper) {
        return std::unexpected{CurveLengthError::non_finite_enclosure};
    }
    return LengthBounds{
        .lower = chord_norm->lower,
        .upper = upper,
    };
}

[[nodiscard]] std::expected<double, CurveLengthError> accumulate_lower(
    const double total,
    const double value) noexcept {
    if (value == 0.0) {
        return total;
    }
    if (total == 0.0) {
        return value;
    }
    const double raw = total + value;
    if (!std::isfinite(raw)) {
        return std::unexpected{CurveLengthError::non_finite_enclosure};
    }
    const auto widened = detail::widen_down(raw);
    if (!widened) {
        return std::unexpected{CurveLengthError::non_finite_enclosure};
    }
    return std::max(0.0, *widened);
}

[[nodiscard]] std::expected<double, CurveLengthError> accumulate_upper(
    const double total,
    const double value) noexcept {
    if (value == 0.0) {
        return total;
    }
    if (total == 0.0) {
        return value;
    }
    const double raw = total + value;
    if (!std::isfinite(raw)) {
        return std::unexpected{CurveLengthError::non_finite_enclosure};
    }
    const auto widened = detail::widen_up(raw);
    if (!widened) {
        return std::unexpected{CurveLengthError::non_finite_enclosure};
    }
    return *widened;
}

[[nodiscard]] std::expected<double, CurveLengthError> width_upper(
    const LengthBounds& bounds) noexcept {
    if (bounds.lower > bounds.upper ||
        !std::isfinite(bounds.lower) ||
        !std::isfinite(bounds.upper)) {
        return std::unexpected{CurveLengthError::non_finite_enclosure};
    }
    if (bounds.lower == bounds.upper) {
        return 0.0;
    }
    const double raw = bounds.upper - bounds.lower;
    if (!std::isfinite(raw) || raw < 0.0) {
        return std::unexpected{CurveLengthError::non_finite_enclosure};
    }
    const auto widened = detail::widen_up(raw);
    if (!widened) {
        return std::unexpected{CurveLengthError::non_finite_enclosure};
    }
    return *widened;
}

[[nodiscard]] double nonnegative_product_lower(
    const double lhs,
    const double rhs) noexcept {
    if (lhs == 0.0 || rhs == 0.0) {
        return 0.0;
    }
    const double product = lhs * rhs;
    if (!std::isfinite(product) || product <= 0.0) {
        return 0.0;
    }
    return std::nextafter(product, 0.0);
}

[[nodiscard]] bool length_width_satisfies(
    const double width,
    const double root_upper,
    const CurveLengthPolicy& policy,
    const double weight) noexcept {
    if (width == 0.0) {
        return true;
    }

    const double absolute_limit =
        nonnegative_product_lower(policy.absolute_tolerance, weight);
    if (width <= absolute_limit) {
        return true;
    }

    if (root_upper <= 0.0 || policy.relative_tolerance == 0.0) {
        return false;
    }

    const double raw_ratio = width / root_upper;
    if (!std::isfinite(raw_ratio)) {
        return false;
    }
    const double ratio_upper = std::nextafter(
        raw_ratio,
        std::numeric_limits<double>::infinity());
    const double relative_limit =
        nonnegative_product_lower(policy.relative_tolerance, weight);
    return ratio_upper <= relative_limit;
}

template <std::size_t Dimension>
struct LengthNode {
    LengthEdges<Dimension> edges{};
    std::size_t depth{};
};

struct LengthAccumulator {
    double lower{};
    double upper{};
};

[[nodiscard]] std::expected<void, CurveLengthError> append_bounds(
    LengthAccumulator& accumulator,
    const LengthBounds& bounds) noexcept {
    const auto lower = accumulate_lower(accumulator.lower, bounds.lower);
    const auto upper = accumulate_upper(accumulator.upper, bounds.upper);
    if (!lower || !upper) {
        return std::unexpected{CurveLengthError::non_finite_enclosure};
    }
    accumulator.lower = *lower;
    accumulator.upper = *upper;
    return {};
}

template <std::size_t Dimension>
[[nodiscard]] std::expected<LengthAccumulator, CurveLengthError>
complete_enclosure(
    LengthAccumulator accepted,
    const std::array<LengthNode<Dimension>, maximum_supported_length_depth + 1>& stack,
    const std::size_t stack_size,
    const LengthNode<Dimension>* current = nullptr) noexcept {
    if (current != nullptr) {
        const auto bounds = length_bounds(current->edges);
        if (!bounds || !append_bounds(accepted, *bounds)) {
            return std::unexpected{CurveLengthError::non_finite_enclosure};
        }
    }

    for (std::size_t index = 0; index < stack_size; ++index) {
        const auto bounds = length_bounds(stack[index].edges);
        if (!bounds || !append_bounds(accepted, *bounds)) {
            return std::unexpected{CurveLengthError::non_finite_enclosure};
        }
    }
    return accepted;
}

template <typename Point, std::size_t Dimension>
[[nodiscard]] std::expected<CurveLengthEvidence, CurveLengthError>
arc_length_enclosure_impl(
    const std::array<Point, 4>& points,
    const CurveLengthPolicy& policy) noexcept {
    const auto valid_policy = validate_length_policy(policy);
    if (!valid_policy) {
        return std::unexpected{valid_policy.error()};
    }

    if (const auto exact = exact_axis_monotone_length(points)) {
        return CurveLengthEvidence{
            .result = CurveLengthResult::converged,
            .lower_length = *exact,
            .upper_length = *exact,
            .processed_nodes = 1,
            .accepted_leaves = 1,
            .max_depth_reached = 0,
        };
    }

    const auto root_edges = length_edges(points);
    if (!root_edges) {
        return std::unexpected{root_edges.error()};
    }
    const auto root_bounds = length_bounds(*root_edges);
    if (!root_bounds) {
        return std::unexpected{root_bounds.error()};
    }

    std::array<LengthNode<Dimension>, maximum_supported_length_depth + 1> stack{};
    std::size_t stack_size = 1;
    stack[0] = LengthNode<Dimension>{
        .edges = *root_edges,
        .depth = 0,
    };

    LengthAccumulator accepted{};
    std::size_t processed_nodes = 0;
    std::size_t accepted_leaves = 0;
    std::size_t max_depth_reached = 0;

    while (stack_size > 0) {
        if (processed_nodes >= policy.max_processed_nodes) {
            const auto full = complete_enclosure(
                accepted,
                stack,
                stack_size);
            if (!full) {
                return std::unexpected{full.error()};
            }
            return CurveLengthEvidence{
                .result = CurveLengthResult::indeterminate,
                .lower_length = full->lower,
                .upper_length = full->upper,
                .processed_nodes = processed_nodes,
                .accepted_leaves = accepted_leaves,
                .max_depth_reached = max_depth_reached,
            };
        }

        const LengthNode<Dimension> node = stack[--stack_size];
        ++processed_nodes;
        max_depth_reached = std::max(max_depth_reached, node.depth);

        const auto bounds = length_bounds(node.edges);
        if (!bounds) {
            return std::unexpected{bounds.error()};
        }
        const auto width = width_upper(*bounds);
        if (!width) {
            return std::unexpected{width.error()};
        }

        const double weight = std::ldexp(1.0, -static_cast<int>(node.depth));
        if (length_width_satisfies(
                *width,
                root_bounds->upper,
                policy,
                weight)) {
            if (!append_bounds(accepted, *bounds)) {
                return std::unexpected{CurveLengthError::non_finite_enclosure};
            }
            ++accepted_leaves;
            continue;
        }

        if (node.depth >= policy.max_subdivision_depth) {
            const auto full = complete_enclosure(
                accepted,
                stack,
                stack_size,
                &node);
            if (!full) {
                return std::unexpected{full.error()};
            }
            return CurveLengthEvidence{
                .result = CurveLengthResult::indeterminate,
                .lower_length = full->lower,
                .upper_length = full->upper,
                .processed_nodes = processed_nodes,
                .accepted_leaves = accepted_leaves,
                .max_depth_reached = max_depth_reached,
            };
        }

        const auto children = detail::subdivide_cubic_edges_midpoint(node.edges);
        if (!children) {
            return std::unexpected{CurveLengthError::non_finite_enclosure};
        }

        stack[stack_size++] = LengthNode<Dimension>{
            .edges = children->second,
            .depth = node.depth + 1,
        };
        stack[stack_size++] = LengthNode<Dimension>{
            .edges = children->first,
            .depth = node.depth + 1,
        };
    }

    const LengthBounds final_bounds{
        .lower = accepted.lower,
        .upper = accepted.upper,
    };
    const auto final_width = width_upper(final_bounds);
    if (!final_width) {
        return std::unexpected{final_width.error()};
    }

    const CurveLengthResult result =
        length_width_satisfies(
            *final_width,
            root_bounds->upper,
            policy,
            1.0)
        ? CurveLengthResult::converged
        : CurveLengthResult::indeterminate;

    return CurveLengthEvidence{
        .result = result,
        .lower_length = accepted.lower,
        .upper_length = accepted.upper,
        .processed_nodes = processed_nodes,
        .accepted_leaves = accepted_leaves,
        .max_depth_reached = max_depth_reached,
    };
}

constexpr std::size_t maximum_supported_regularity_depth = 64;


[[nodiscard]] std::expected<void, CurveRegularityError> validate_regularity_policy(
    const CurveRegularityPolicy& policy) noexcept {
    if (policy.max_processed_nodes == 0 ||
        policy.max_subdivision_depth > maximum_supported_regularity_depth) {
        return std::unexpected{CurveRegularityError::invalid_policy};
    }
    return {};
}

template <std::size_t Dimension>
using DerivativeIntervalControls =
    std::array<detail::IntervalVector<Dimension>, 3>;

[[nodiscard]] std::expected<detail::ClosedInterval, CurveRegularityError>
derivative_component_interval(
    const double next,
    const double current) noexcept {
    const auto next_interval = detail::point_interval(next);
    const auto current_interval = detail::point_interval(current);
    if (!next_interval || !current_interval) {
        return std::unexpected{CurveRegularityError::non_finite_enclosure};
    }
    const auto delta = detail::subtract(*next_interval, *current_interval);
    if (!delta) {
        return std::unexpected{CurveRegularityError::non_finite_enclosure};
    }
    const auto scaled = detail::multiply(*delta, 3.0);
    if (!scaled) {
        return std::unexpected{CurveRegularityError::non_finite_enclosure};
    }
    return *scaled;
}

[[nodiscard]] std::expected<DerivativeIntervalControls<2>, CurveRegularityError>
derivative_interval_controls(
    const std::array<Point2, 4>& points) noexcept {
    DerivativeIntervalControls<2> controls{};
    for (std::size_t index = 0; index < 3; ++index) {
        const auto x = derivative_component_interval(
            points[index + 1].x(),
            points[index].x());
        const auto y = derivative_component_interval(
            points[index + 1].y(),
            points[index].y());
        if (!x || !y) {
            return std::unexpected{CurveRegularityError::non_finite_enclosure};
        }
        controls[index] = detail::IntervalVector<2>{*x, *y};
    }
    return controls;
}

[[nodiscard]] std::expected<DerivativeIntervalControls<3>, CurveRegularityError>
derivative_interval_controls(
    const std::array<Point3, 4>& points) noexcept {
    DerivativeIntervalControls<3> controls{};
    for (std::size_t index = 0; index < 3; ++index) {
        const auto x = derivative_component_interval(
            points[index + 1].x(),
            points[index].x());
        const auto y = derivative_component_interval(
            points[index + 1].y(),
            points[index].y());
        const auto z = derivative_component_interval(
            points[index + 1].z(),
            points[index].z());
        if (!x || !y || !z) {
            return std::unexpected{CurveRegularityError::non_finite_enclosure};
        }
        controls[index] = detail::IntervalVector<3>{*x, *y, *z};
    }
    return controls;
}

template <std::size_t Dimension>
[[nodiscard]] std::expected<std::array<detail::ClosedInterval, 5>, CurveRegularityError>
squared_speed_coefficients(
    const DerivativeIntervalControls<Dimension>& controls) noexcept {
    const auto s0 = detail::dot(controls[0], controls[0]);
    const auto s1 = detail::dot(controls[0], controls[1]);
    const auto d0d2 = detail::dot(controls[0], controls[2]);
    const auto d1d1 = detail::dot(controls[1], controls[1]);
    const auto s3 = detail::dot(controls[1], controls[2]);
    const auto s4 = detail::dot(controls[2], controls[2]);
    if (!s0 || !s1 || !d0d2 || !d1d1 || !s3 || !s4) {
        return std::unexpected{CurveRegularityError::non_finite_enclosure};
    }

    const auto twice_d1d1 = detail::multiply(*d1d1, 2.0);
    if (!twice_d1d1) {
        return std::unexpected{CurveRegularityError::non_finite_enclosure};
    }
    const auto middle_sum = detail::add(*d0d2, *twice_d1d1);
    if (!middle_sum) {
        return std::unexpected{CurveRegularityError::non_finite_enclosure};
    }
    const auto s2 = detail::divide_positive(*middle_sum, 3.0);
    if (!s2) {
        return std::unexpected{CurveRegularityError::non_finite_enclosure};
    }

    return std::array<detail::ClosedInterval, 5>{
        *s0,
        *s1,
        *s2,
        *s3,
        *s4,
    };
}

struct RegularityNode {
    std::array<detail::ClosedInterval, 5> coefficients{};
    std::size_t depth{};
};

template <typename Point, std::size_t Dimension>
[[nodiscard]] std::expected<CurveRegularityEvidence, CurveRegularityError>
certify_regularity_impl(
    const std::array<Point, 4>& points,
    const DerivativeIntervalControls<Dimension>& derivative_controls,
    const CurveRegularityPolicy& policy) noexcept {
    if (points[1] == points[0] || points[3] == points[2]) {
        return CurveRegularityEvidence{
            .result = CurveRegularityResult::degenerate,
            .processed_nodes = 0,
            .certified_leaves = 0,
            .max_depth_reached = 0,
        };
    }

    const auto root_coefficients = squared_speed_coefficients(derivative_controls);
    if (!root_coefficients) {
        return std::unexpected{root_coefficients.error()};
    }

    std::array<RegularityNode, maximum_supported_regularity_depth + 1> stack{};
    std::size_t stack_size = 1;
    stack[0] = RegularityNode{*root_coefficients, 0};

    std::size_t processed_nodes = 0;
    std::size_t certified_leaves = 0;
    std::size_t max_depth_reached = 0;

    while (stack_size > 0) {
        if (processed_nodes >= policy.max_processed_nodes) {
            return CurveRegularityEvidence{
                .result = CurveRegularityResult::indeterminate,
                .processed_nodes = processed_nodes,
                .certified_leaves = certified_leaves,
                .max_depth_reached = max_depth_reached,
            };
        }

        const RegularityNode node = stack[--stack_size];
        ++processed_nodes;
        max_depth_reached = std::max(max_depth_reached, node.depth);

        if (detail::strictly_positive(node.coefficients)) {
            ++certified_leaves;
            continue;
        }

        if (node.depth >= policy.max_subdivision_depth) {
            return CurveRegularityEvidence{
                .result = CurveRegularityResult::indeterminate,
                .processed_nodes = processed_nodes,
                .certified_leaves = certified_leaves,
                .max_depth_reached = max_depth_reached,
            };
        }

        const auto children = detail::subdivide_quartic_midpoint(node.coefficients);
        if (!children) {
            return std::unexpected{CurveRegularityError::non_finite_enclosure};
        }

        stack[stack_size++] = RegularityNode{
            .coefficients = children->second,
            .depth = node.depth + 1,
        };
        stack[stack_size++] = RegularityNode{
            .coefficients = children->first,
            .depth = node.depth + 1,
        };
    }

    return CurveRegularityEvidence{
        .result = CurveRegularityResult::regular,
        .processed_nodes = processed_nodes,
        .certified_leaves = certified_leaves,
        .max_depth_reached = max_depth_reached,
    };
}

} // namespace

const std::array<Point2, 4>& CubicBezier2::control_points() const noexcept {
    return control_points_;
}

std::expected<Point2, CurveError> CubicBezier2::evaluate(
    const double parameter) const noexcept {
    const auto valid = validate_parameter(parameter);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }
    return evaluate_de_casteljau(control_points_, parameter);
}

std::expected<Vector2, CurveError> CubicBezier2::first_derivative(
    const double parameter) const noexcept {
    return evaluate_first_derivative<Point2, Vector2>(control_points_, parameter);
}

std::expected<Vector2, CurveError> CubicBezier2::second_derivative(
    const double parameter) const noexcept {
    return evaluate_second_derivative<Point2, Vector2>(control_points_, parameter);
}

std::expected<double, CurveError> CubicBezier2::speed(
    const double parameter) const noexcept {
    const auto derivative = first_derivative(parameter);
    if (!derivative.has_value()) {
        return std::unexpected{derivative.error()};
    }
    return vector_speed(*derivative);
}

std::expected<CurveRegularityEvidence, CurveRegularityError>
CubicBezier2::certify_regularity(
    const CurveRegularityPolicy& policy) const noexcept {
    const auto valid_policy = validate_regularity_policy(policy);
    if (!valid_policy) {
        return std::unexpected{valid_policy.error()};
    }
    const auto controls = derivative_interval_controls(control_points_);
    if (!controls) {
        return std::unexpected{controls.error()};
    }
    return certify_regularity_impl(control_points_, *controls, policy);
}

std::expected<CurveLengthEvidence, CurveLengthError>
CubicBezier2::arc_length_enclosure(
    const CurveLengthPolicy& policy) const noexcept {
    return arc_length_enclosure_impl<Point2, 2>(control_points_, policy);
}

CubicBezier2 CubicBezier2::reversed() const noexcept {
    return CubicBezier2{
        control_points_[3],
        control_points_[2],
        control_points_[1],
        control_points_[0],
    };
}

const std::array<Point3, 4>& CubicBezier3::control_points() const noexcept {
    return control_points_;
}

std::expected<Point3, CurveError> CubicBezier3::evaluate(
    const double parameter) const noexcept {
    const auto valid = validate_parameter(parameter);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }
    return evaluate_de_casteljau(control_points_, parameter);
}

std::expected<Vector3, CurveError> CubicBezier3::first_derivative(
    const double parameter) const noexcept {
    return evaluate_first_derivative<Point3, Vector3>(control_points_, parameter);
}

std::expected<Vector3, CurveError> CubicBezier3::second_derivative(
    const double parameter) const noexcept {
    return evaluate_second_derivative<Point3, Vector3>(control_points_, parameter);
}

std::expected<double, CurveError> CubicBezier3::speed(
    const double parameter) const noexcept {
    const auto derivative = first_derivative(parameter);
    if (!derivative.has_value()) {
        return std::unexpected{derivative.error()};
    }
    return vector_speed(*derivative);
}

std::expected<CurveRegularityEvidence, CurveRegularityError>
CubicBezier3::certify_regularity(
    const CurveRegularityPolicy& policy) const noexcept {
    const auto valid_policy = validate_regularity_policy(policy);
    if (!valid_policy) {
        return std::unexpected{valid_policy.error()};
    }
    const auto controls = derivative_interval_controls(control_points_);
    if (!controls) {
        return std::unexpected{controls.error()};
    }
    return certify_regularity_impl(control_points_, *controls, policy);
}

std::expected<CurveLengthEvidence, CurveLengthError>
CubicBezier3::arc_length_enclosure(
    const CurveLengthPolicy& policy) const noexcept {
    return arc_length_enclosure_impl<Point3, 3>(control_points_, policy);
}

CubicBezier3 CubicBezier3::reversed() const noexcept {
    return CubicBezier3{
        control_points_[3],
        control_points_[2],
        control_points_[1],
        control_points_[0],
    };
}

} // namespace apmesh::core
