#include "apmesh/geometry/curve.hpp"

#include "apmesh/core/numeric.hpp"
#include "detail/curve_regularity_interval.hpp"

#include <array>
#include <cmath>
#include <expected>

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

CubicBezier3 CubicBezier3::reversed() const noexcept {
    return CubicBezier3{
        control_points_[3],
        control_points_[2],
        control_points_[1],
        control_points_[0],
    };
}

} // namespace apmesh::core
