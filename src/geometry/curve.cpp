#include "apmesh/geometry/curve.hpp"

#include "apmesh/core/numeric.hpp"
#include "detail/curve_inflection_interval.hpp"
#include "detail/curve_length_interval.hpp"
#include "detail/curve_regularity_interval.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <expected>
#include <limits>
#include <numeric>
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

[[nodiscard]] double max_abs_component(const Vector2& vector) noexcept {
    return std::max(std::abs(vector.x()), std::abs(vector.y()));
}

[[nodiscard]] double max_abs_component(const Vector3& vector) noexcept {
    return std::max(
        std::max(std::abs(vector.x()), std::abs(vector.y())),
        std::abs(vector.z()));
}

[[nodiscard]] std::expected<Vector2, CurveError> normalized_vector(
    const Vector2& vector,
    const double scale) noexcept {
    const auto normalized = Vector2::make(
        vector.x() / scale,
        vector.y() / scale);
    if (!normalized.has_value()) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return *normalized;
}

[[nodiscard]] std::expected<Vector3, CurveError> normalized_vector(
    const Vector3& vector,
    const double scale) noexcept {
    const auto normalized = Vector3::make(
        vector.x() / scale,
        vector.y() / scale,
        vector.z() / scale);
    if (!normalized.has_value()) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return *normalized;
}

[[nodiscard]] std::expected<double, CurveError> scaled_curvature_result(
    const double normalized_numerator,
    const double normalized_speed,
    const double velocity_scale,
    const double acceleration_scale) noexcept {
    if (normalized_numerator == 0.0) {
        return 0.0;
    }

    int numerator_exponent = 0;
    int velocity_exponent = 0;
    int acceleration_exponent = 0;
    const double numerator_mantissa =
        std::frexp(normalized_numerator, &numerator_exponent);
    const double velocity_mantissa =
        std::frexp(velocity_scale, &velocity_exponent);
    const double acceleration_mantissa =
        std::frexp(acceleration_scale, &acceleration_exponent);

    const double normalized_speed_squared =
        normalized_speed * normalized_speed;
    const double normalized_speed_cubed =
        normalized_speed_squared * normalized_speed;
    const double velocity_mantissa_squared =
        velocity_mantissa * velocity_mantissa;

    const double base =
        (numerator_mantissa * acceleration_mantissa) /
        (normalized_speed_cubed * velocity_mantissa_squared);
    if (!std::isfinite(base) || base <= 0.0) {
        return std::unexpected{CurveError::non_finite_result};
    }

    const int exponent =
        numerator_exponent + acceleration_exponent -
        2 * velocity_exponent;
    const double result = std::scalbn(base, exponent);
    if (!std::isfinite(result) || result == 0.0) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return result;
}

struct PlanarCurvatureResult {
    double magnitude{};
    int orientation{};
};

[[nodiscard]] std::expected<PlanarCurvatureResult, CurveError>
planar_curvature_impl(
    const Vector2& first,
    const Vector2& second) noexcept {
    const double velocity_scale = max_abs_component(first);
    if (velocity_scale == 0.0) {
        return std::unexpected{CurveError::singular_parameter};
    }

    const double acceleration_scale = max_abs_component(second);
    if (acceleration_scale == 0.0) {
        return PlanarCurvatureResult{0.0, 0};
    }

    const auto normalized_first = normalized_vector(first, velocity_scale);
    const auto normalized_second = normalized_vector(second, acceleration_scale);
    if (!normalized_first || !normalized_second) {
        return std::unexpected{CurveError::non_finite_result};
    }

    const auto normalized_speed = norm(*normalized_first);
    if (!normalized_speed || *normalized_speed == 0.0) {
        return std::unexpected{CurveError::non_finite_result};
    }

    const double determinant =
        normalized_first->x() * normalized_second->y() -
        normalized_first->y() * normalized_second->x();
    if (!std::isfinite(determinant)) {
        return std::unexpected{CurveError::non_finite_result};
    }
    if (determinant == 0.0) {
        return PlanarCurvatureResult{0.0, 0};
    }

    const auto magnitude = scaled_curvature_result(
        std::abs(determinant),
        *normalized_speed,
        velocity_scale,
        acceleration_scale);
    if (!magnitude) {
        return std::unexpected{magnitude.error()};
    }

    return PlanarCurvatureResult{
        *magnitude,
        determinant < 0.0 ? -1 : 1,
    };
}

[[nodiscard]] std::expected<double, CurveError> curvature_magnitude_impl(
    const Vector2& first,
    const Vector2& second) noexcept {
    const auto value = planar_curvature_impl(first, second);
    if (!value) {
        return std::unexpected{value.error()};
    }
    return value->magnitude;
}

[[nodiscard]] std::expected<double, CurveError> signed_curvature_impl(
    const Vector2& first,
    const Vector2& second) noexcept {
    const auto value = planar_curvature_impl(first, second);
    if (!value) {
        return std::unexpected{value.error()};
    }
    if (value->orientation == 0) {
        return 0.0;
    }
    return value->orientation < 0 ? -value->magnitude : value->magnitude;
}

[[nodiscard]] std::expected<double, CurveError> curvature_magnitude_impl(
    const Vector3& first,
    const Vector3& second) noexcept {
    const double velocity_scale = max_abs_component(first);
    if (velocity_scale == 0.0) {
        return std::unexpected{CurveError::singular_parameter};
    }

    const double acceleration_scale = max_abs_component(second);
    if (acceleration_scale == 0.0) {
        return 0.0;
    }

    const auto normalized_first = normalized_vector(first, velocity_scale);
    const auto normalized_second = normalized_vector(second, acceleration_scale);
    if (!normalized_first || !normalized_second) {
        return std::unexpected{CurveError::non_finite_result};
    }

    const auto normalized_speed = norm(*normalized_first);
    const auto normalized_cross = cross(*normalized_first, *normalized_second);
    if (!normalized_speed || *normalized_speed == 0.0 || !normalized_cross) {
        return std::unexpected{CurveError::non_finite_result};
    }
    const auto normalized_numerator = norm(*normalized_cross);
    if (!normalized_numerator) {
        return std::unexpected{CurveError::non_finite_result};
    }

    return scaled_curvature_result(
        *normalized_numerator,
        *normalized_speed,
        velocity_scale,
        acceleration_scale);
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

[[nodiscard]] std::expected<void, CurveLengthError> validate_length_parameter(
    const double parameter) noexcept {
    if (!std::isfinite(parameter)) {
        return std::unexpected{CurveLengthError::non_finite_parameter};
    }
    if (parameter < 0.0 || parameter > 1.0) {
        return std::unexpected{CurveLengthError::parameter_out_of_domain};
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

template <std::size_t Dimension>
[[nodiscard]] std::expected<detail::IntervalVector<Dimension>, CurveLengthError>
scale_interval_vector(
    const detail::IntervalVector<Dimension>& value,
    const detail::ClosedInterval& scalar) noexcept {
    detail::IntervalVector<Dimension> result{};
    for (std::size_t index = 0; index < Dimension; ++index) {
        if ((scalar.lower == 0.0 && scalar.upper == 0.0) ||
            (value[index].lower == 0.0 && value[index].upper == 0.0)) {
            result[index] = detail::ClosedInterval{0.0, 0.0};
            continue;
        }
        const auto component = detail::multiply(value[index], scalar);
        if (!component) {
            return std::unexpected{CurveLengthError::non_finite_enclosure};
        }
        result[index] = *component;
    }
    return result;
}

template <std::size_t Dimension>
[[nodiscard]] std::expected<detail::IntervalVector<Dimension>, CurveLengthError>
lerp_interval_vector(
    const detail::IntervalVector<Dimension>& lhs,
    const detail::IntervalVector<Dimension>& rhs,
    const detail::ClosedInterval& one_minus_parameter,
    const detail::ClosedInterval& parameter) noexcept {
    const auto lhs_scaled =
        scale_interval_vector(lhs, one_minus_parameter);
    const auto rhs_scaled =
        scale_interval_vector(rhs, parameter);
    if (!lhs_scaled || !rhs_scaled) {
        return std::unexpected{CurveLengthError::non_finite_enclosure};
    }
    const auto sum = detail::add_vectors(*lhs_scaled, *rhs_scaled);
    if (!sum) {
        return std::unexpected{CurveLengthError::non_finite_enclosure};
    }
    return *sum;
}

template <std::size_t Dimension>
[[nodiscard]] std::expected<LengthEdges<Dimension>, CurveLengthError>
prefix_length_edges(
    const LengthEdges<Dimension>& root_edges,
    const double parameter) noexcept {
    const detail::ClosedInterval parameter_interval{
        parameter,
        parameter,
    };

    detail::ClosedInterval one_minus_parameter{};
    if (const auto exact = exact_difference(1.0, parameter)) {
        one_minus_parameter = detail::ClosedInterval{*exact, *exact};
    } else {
        const auto one = detail::point_interval(1.0);
        const auto parameter_point = detail::point_interval(parameter);
        if (!one || !parameter_point) {
            return std::unexpected{CurveLengthError::non_finite_enclosure};
        }
        const auto difference = detail::subtract(*one, *parameter_point);
        if (!difference) {
            return std::unexpected{CurveLengthError::non_finite_enclosure};
        }
        one_minus_parameter = *difference;
    }

    const auto first_blend = lerp_interval_vector(
        root_edges[0],
        root_edges[1],
        one_minus_parameter,
        parameter_interval);
    const auto second_blend = lerp_interval_vector(
        root_edges[1],
        root_edges[2],
        one_minus_parameter,
        parameter_interval);
    if (!first_blend || !second_blend) {
        return std::unexpected{CurveLengthError::non_finite_enclosure};
    }

    const auto final_blend = lerp_interval_vector(
        *first_blend,
        *second_blend,
        one_minus_parameter,
        parameter_interval);
    if (!final_blend) {
        return std::unexpected{CurveLengthError::non_finite_enclosure};
    }

    const auto prefix0 =
        scale_interval_vector(root_edges[0], parameter_interval);
    const auto prefix1 =
        scale_interval_vector(*first_blend, parameter_interval);
    const auto prefix2 =
        scale_interval_vector(*final_blend, parameter_interval);
    if (!prefix0 || !prefix1 || !prefix2) {
        return std::unexpected{CurveLengthError::non_finite_enclosure};
    }

    return LengthEdges<Dimension>{*prefix0, *prefix1, *prefix2};
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

template <std::size_t Dimension>
[[nodiscard]] std::expected<CurveLengthEvidence, CurveLengthError>
arc_length_enclosure_from_edges(
    const LengthEdges<Dimension>& root_edges,
    const CurveLengthPolicy& policy) noexcept {
    const auto root_bounds = length_bounds(root_edges);
    if (!root_bounds) {
        return std::unexpected{root_bounds.error()};
    }

    std::array<LengthNode<Dimension>, maximum_supported_length_depth + 1> stack{};
    std::size_t stack_size = 1;
    stack[0] = LengthNode<Dimension>{
        .edges = root_edges,
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
    return arc_length_enclosure_from_edges(*root_edges, policy);
}

template <typename Point, std::size_t Dimension>
[[nodiscard]] std::expected<CurveLengthEvidence, CurveLengthError>
cumulative_arc_length_enclosure_impl(
    const std::array<Point, 4>& points,
    const double parameter,
    const CurveLengthPolicy& policy) noexcept {
    const auto valid_parameter = validate_length_parameter(parameter);
    if (!valid_parameter) {
        return std::unexpected{valid_parameter.error()};
    }
    const auto valid_policy = validate_length_policy(policy);
    if (!valid_policy) {
        return std::unexpected{valid_policy.error()};
    }

    if (parameter == 0.0) {
        return CurveLengthEvidence{
            .result = CurveLengthResult::converged,
            .lower_length = 0.0,
            .upper_length = 0.0,
            .processed_nodes = 1,
            .accepted_leaves = 1,
            .max_depth_reached = 0,
        };
    }
    if (parameter == 1.0) {
        return arc_length_enclosure_impl<Point, Dimension>(points, policy);
    }

    const auto root_edges = length_edges(points);
    if (!root_edges) {
        return std::unexpected{root_edges.error()};
    }
    const auto prefix_edges = prefix_length_edges(*root_edges, parameter);
    if (!prefix_edges) {
        return std::unexpected{prefix_edges.error()};
    }
    return arc_length_enclosure_from_edges(*prefix_edges, policy);
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


constexpr std::size_t maximum_supported_inflection_depth = 64;

[[nodiscard]] CurveInflectionError inflection_error(
    const CurveRegularityError error) noexcept {
    switch (error) {
    case CurveRegularityError::invalid_policy:
        return CurveInflectionError::invalid_policy;
    case CurveRegularityError::non_finite_enclosure:
        return CurveInflectionError::non_finite_enclosure;
    }
    return CurveInflectionError::non_finite_enclosure;
}

[[nodiscard]] std::expected<void, CurveInflectionError>
validate_inflection_policy(
    const CurveInflectionIsolationPolicy& policy) noexcept {
    const auto regularity = validate_regularity_policy(policy.regularity_policy);
    if (!regularity) {
        return std::unexpected{CurveInflectionError::invalid_policy};
    }
    if (!std::isfinite(policy.parameter_tolerance) ||
        policy.parameter_tolerance <= 0.0 ||
        policy.max_processed_nodes == 0 ||
        policy.max_subdivision_depth > maximum_supported_inflection_depth) {
        return std::unexpected{CurveInflectionError::invalid_policy};
    }
    return {};
}

[[nodiscard]] std::expected<DerivativeIntervalControls<2>, CurveInflectionError>
inflection_derivative_interval_controls(
    const std::array<Point2, 4>& points) noexcept {
    DerivativeIntervalControls<2> controls{};
    for (std::size_t index = 0; index < 3; ++index) {
        const auto x = detail::inflection_difference_scaled(
            points[index + 1].x(),
            points[index].x(),
            3.0);
        const auto y = detail::inflection_difference_scaled(
            points[index + 1].y(),
            points[index].y(),
            3.0);
        if (!x || !y) {
            return std::unexpected{CurveInflectionError::non_finite_enclosure};
        }
        controls[index] = detail::IntervalVector<2>{*x, *y};
    }
    return controls;
}

[[nodiscard]] std::expected<detail::QuadraticIntervalCoefficients, CurveInflectionError>
inflection_numerator_coefficients(
    const DerivativeIntervalControls<2>& controls) noexcept {
    const auto d01 = detail::inflection_determinant(controls[0], controls[1]);
    const auto d02 = detail::inflection_determinant(controls[0], controls[2]);
    const auto d12 = detail::inflection_determinant(controls[1], controls[2]);
    if (!d01 || !d02 || !d12) {
        return std::unexpected{CurveInflectionError::non_finite_enclosure};
    }

    const auto n0 = detail::inflection_multiply(*d01, 2.0);
    const auto n2 = detail::inflection_multiply(*d12, 2.0);
    if (!n0 || !n2) {
        return std::unexpected{CurveInflectionError::non_finite_enclosure};
    }

    return detail::QuadraticIntervalCoefficients{
        *n0,
        *d02,
        *n2,
    };
}

struct InflectionNode {
    detail::QuadraticIntervalCoefficients coefficients{};
    double lower_parameter{};
    double upper_parameter{};
    std::size_t depth{};
};

[[nodiscard]] bool inflection_bracket_within_tolerance(
    const double lower,
    const double upper,
    const double tolerance) noexcept {
    const double raw_width = upper - lower;
    if (!std::isfinite(raw_width) || raw_width < 0.0) {
        return false;
    }
    if (raw_width == 0.0) {
        return true;
    }

    const double conservative_width = std::nextafter(
        raw_width,
        std::numeric_limits<double>::infinity());
    return std::isfinite(conservative_width) &&
           conservative_width <= tolerance;
}

[[nodiscard]] CurveInflectionIsolationEvidence inflection_evidence(
    const CurveInflectionIsolationResult result,
    const CurveRegularityEvidence& regularity,
    std::array<CurveInflectionBracket, 2> brackets,
    const std::size_t inflection_count,
    const std::size_t processed_nodes,
    const std::size_t root_free_leaves,
    const std::size_t isolated_root_leaves,
    const std::size_t max_depth_reached) noexcept {
    std::sort(
        brackets.begin(),
        brackets.begin() + inflection_count,
        [](const CurveInflectionBracket& lhs, const CurveInflectionBracket& rhs) {
            if (lhs.lower_parameter != rhs.lower_parameter) {
                return lhs.lower_parameter < rhs.lower_parameter;
            }
            return lhs.upper_parameter < rhs.upper_parameter;
        });

    return CurveInflectionIsolationEvidence{
        .result = result,
        .regularity = regularity,
        .brackets = brackets,
        .inflection_count = inflection_count,
        .processed_nodes = processed_nodes,
        .root_free_leaves = root_free_leaves,
        .isolated_root_leaves = isolated_root_leaves,
        .max_depth_reached = max_depth_reached,
    };
}

[[nodiscard]] std::expected<CurveInflectionIsolationEvidence, CurveInflectionError>
isolate_simple_inflections_impl(
    const DerivativeIntervalControls<2>& derivative_controls,
    const CurveRegularityEvidence& regularity,
    const CurveInflectionIsolationPolicy& policy) noexcept {
    const auto root_coefficients =
        inflection_numerator_coefficients(derivative_controls);
    if (!root_coefficients) {
        return std::unexpected{root_coefficients.error()};
    }

    const auto root_variation =
        detail::quadratic_sign_variation(*root_coefficients);
    if (root_variation.resolved && root_variation.all_zero) {
        return inflection_evidence(
            CurveInflectionIsolationResult::complete,
            regularity,
            {},
            0,
            1,
            0,
            0,
            0);
    }

    std::array<InflectionNode, maximum_supported_inflection_depth + 1> stack{};
    std::size_t stack_size = 1;
    stack[0] = InflectionNode{
        .coefficients = *root_coefficients,
        .lower_parameter = 0.0,
        .upper_parameter = 1.0,
        .depth = 0,
    };

    std::array<CurveInflectionBracket, 2> brackets{};
    std::size_t inflection_count = 0;
    std::size_t processed_nodes = 0;
    std::size_t root_free_leaves = 0;
    std::size_t isolated_root_leaves = 0;
    std::size_t max_depth_reached = 0;

    const auto indeterminate = [&]() noexcept {
        return inflection_evidence(
            CurveInflectionIsolationResult::indeterminate,
            regularity,
            brackets,
            inflection_count,
            processed_nodes,
            root_free_leaves,
            isolated_root_leaves,
            max_depth_reached);
    };

    while (stack_size > 0) {
        if (processed_nodes >= policy.max_processed_nodes) {
            return indeterminate();
        }

        const InflectionNode node = stack[--stack_size];
        ++processed_nodes;
        max_depth_reached = std::max(max_depth_reached, node.depth);

        const auto variation =
            detail::quadratic_sign_variation(node.coefficients);

        if (variation.resolved && variation.all_zero) {
            return inflection_evidence(
                CurveInflectionIsolationResult::complete,
                regularity,
                {},
                0,
                processed_nodes,
                root_free_leaves,
                isolated_root_leaves,
                max_depth_reached);
        }

        if (variation.resolved && variation.variations == 0) {
            ++root_free_leaves;
            continue;
        }

        if (variation.resolved &&
            variation.variations == 1 &&
            inflection_bracket_within_tolerance(
                node.lower_parameter,
                node.upper_parameter,
                policy.parameter_tolerance)) {
            if (inflection_count >= brackets.size()) {
                return indeterminate();
            }
            brackets[inflection_count++] = CurveInflectionBracket{
                .lower_parameter = node.lower_parameter,
                .upper_parameter = node.upper_parameter,
            };
            ++isolated_root_leaves;
            continue;
        }

        if (node.depth >= policy.max_subdivision_depth) {
            return indeterminate();
        }

        const auto subdivision =
            detail::subdivide_quadratic_midpoint(node.coefficients);
        if (!subdivision) {
            return std::unexpected{CurveInflectionError::non_finite_enclosure};
        }

        if (detail::interval_contains_zero(
                subdivision->shared_boundary_value)) {
            return indeterminate();
        }

        const double midpoint =
            std::midpoint(node.lower_parameter, node.upper_parameter);
        if (!(midpoint > node.lower_parameter) ||
            !(midpoint < node.upper_parameter)) {
            return indeterminate();
        }

        stack[stack_size++] = InflectionNode{
            .coefficients = subdivision->right,
            .lower_parameter = midpoint,
            .upper_parameter = node.upper_parameter,
            .depth = node.depth + 1,
        };
        stack[stack_size++] = InflectionNode{
            .coefficients = subdivision->left,
            .lower_parameter = node.lower_parameter,
            .upper_parameter = midpoint,
            .depth = node.depth + 1,
        };
    }

    return inflection_evidence(
        CurveInflectionIsolationResult::complete,
        regularity,
        brackets,
        inflection_count,
        processed_nodes,
        root_free_leaves,
        isolated_root_leaves,
        max_depth_reached);
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

[[nodiscard]] CurveInverseLengthError inverse_error(
    const CurveLengthError error) noexcept {
    switch (error) {
    case CurveLengthError::invalid_policy:
        return CurveInverseLengthError::invalid_policy;
    case CurveLengthError::non_finite_enclosure:
        return CurveInverseLengthError::non_finite_enclosure;
    case CurveLengthError::non_finite_parameter:
    case CurveLengthError::parameter_out_of_domain:
        return CurveInverseLengthError::non_finite_enclosure;
    }
    return CurveInverseLengthError::non_finite_enclosure;
}

[[nodiscard]] CurveInverseLengthError inverse_error(
    const CurveRegularityError error) noexcept {
    switch (error) {
    case CurveRegularityError::invalid_policy:
        return CurveInverseLengthError::invalid_policy;
    case CurveRegularityError::non_finite_enclosure:
        return CurveInverseLengthError::non_finite_enclosure;
    }
    return CurveInverseLengthError::non_finite_enclosure;
}

[[nodiscard]] std::expected<void, CurveInverseLengthError>
validate_inverse_length_policy(
    const CurveInverseLengthPolicy& policy) noexcept {
    const auto length_valid = validate_length_policy(policy.length_policy);
    if (!length_valid) {
        return std::unexpected{CurveInverseLengthError::invalid_policy};
    }
    const auto regularity_valid =
        validate_regularity_policy(policy.regularity_policy);
    if (!regularity_valid) {
        return std::unexpected{CurveInverseLengthError::invalid_policy};
    }
    if (!std::isfinite(policy.parameter_tolerance) ||
        policy.parameter_tolerance < 0.0 ||
        policy.max_refinement_iterations == 0) {
        return std::unexpected{CurveInverseLengthError::invalid_policy};
    }
    return {};
}

[[nodiscard]] bool valid_length_evidence(
    const CurveLengthEvidence& evidence) noexcept {
    return std::isfinite(evidence.lower_length) &&
           std::isfinite(evidence.upper_length) &&
           evidence.lower_length >= 0.0 &&
           evidence.upper_length >= evidence.lower_length;
}

[[nodiscard]] std::expected<std::pair<double, double>, CurveInverseLengthError>
fraction_target_interval(
    const double fraction,
    const CurveLengthEvidence& total) noexcept {
    const double raw_lower = fraction * total.lower_length;
    const double raw_upper = fraction * total.upper_length;
    if (!std::isfinite(raw_lower) || !std::isfinite(raw_upper)) {
        return std::unexpected{CurveInverseLengthError::non_finite_enclosure};
    }

    const double lower =
        raw_lower == 0.0 ? 0.0 : std::nextafter(raw_lower, 0.0);
    const double upper =
        raw_upper == 0.0
            ? 0.0
            : std::nextafter(
                  raw_upper,
                  std::numeric_limits<double>::infinity());
    if (!std::isfinite(lower) || !std::isfinite(upper) ||
        lower < 0.0 || upper < lower) {
        return std::unexpected{CurveInverseLengthError::non_finite_enclosure};
    }
    return std::pair{lower, upper};
}

[[nodiscard]] CurveInverseLengthEvidence inverse_evidence(
    const CurveInverseLengthResult result,
    const double target_lower,
    const double target_upper,
    const double lower_parameter,
    const double upper_parameter,
    const CurveLengthEvidence& lower_cumulative,
    const CurveLengthEvidence& upper_cumulative,
    const CurveLengthEvidence& total,
    const CurveRegularityEvidence& regularity,
    const std::size_t iterations) noexcept {
    return CurveInverseLengthEvidence{
        .result = result,
        .target_lower_length = target_lower,
        .target_upper_length = target_upper,
        .lower_parameter = lower_parameter,
        .upper_parameter = upper_parameter,
        .lower_cumulative = lower_cumulative,
        .upper_cumulative = upper_cumulative,
        .total_length = total,
        .regularity = regularity,
        .refinement_iterations = iterations,
    };
}

enum class InverseTargetMode {
    absolute_length,
    normalized_fraction,
};

template <typename Curve>
[[nodiscard]] std::expected<CurveInverseLengthEvidence, CurveInverseLengthError>
inverse_arc_length_bracket_impl(
    const Curve& curve,
    const double target,
    const CurveInverseLengthPolicy& policy,
    const InverseTargetMode mode) noexcept {
    const auto valid_policy = validate_inverse_length_policy(policy);
    if (!valid_policy) {
        return std::unexpected{valid_policy.error()};
    }

    if (!std::isfinite(target)) {
        return std::unexpected{CurveInverseLengthError::non_finite_target};
    }
    if (target < 0.0 ||
        (mode == InverseTargetMode::normalized_fraction && target > 1.0)) {
        return std::unexpected{CurveInverseLengthError::target_out_of_domain};
    }

    const auto regularity =
        curve.certify_regularity(policy.regularity_policy);
    if (!regularity) {
        return std::unexpected{inverse_error(regularity.error())};
    }
    if (regularity->result != CurveRegularityResult::regular) {
        return std::unexpected{
            CurveInverseLengthError::regularity_not_certified};
    }

    const auto total = curve.arc_length_enclosure(policy.length_policy);
    if (!total) {
        return std::unexpected{inverse_error(total.error())};
    }
    if (!valid_length_evidence(*total)) {
        return std::unexpected{CurveInverseLengthError::non_finite_enclosure};
    }

    const auto zero =
        curve.cumulative_arc_length_enclosure(0.0, policy.length_policy);
    if (!zero) {
        return std::unexpected{inverse_error(zero.error())};
    }
    if (!valid_length_evidence(*zero) ||
        zero->lower_length != 0.0 || zero->upper_length != 0.0) {
        return std::unexpected{CurveInverseLengthError::non_finite_enclosure};
    }

    double target_lower = 0.0;
    double target_upper = 0.0;

    if (mode == InverseTargetMode::absolute_length) {
        if (target > total->upper_length) {
            return std::unexpected{
                CurveInverseLengthError::target_out_of_domain};
        }

        target_lower = target;
        target_upper = target;

        if (target == 0.0) {
            return inverse_evidence(
                CurveInverseLengthResult::converged,
                0.0,
                0.0,
                0.0,
                0.0,
                *zero,
                *zero,
                *total,
                *regularity,
                0);
        }

        if (total->lower_length == total->upper_length &&
            target == total->lower_length) {
            return inverse_evidence(
                CurveInverseLengthResult::converged,
                target,
                target,
                1.0,
                1.0,
                *total,
                *total,
                *total,
                *regularity,
                0);
        }

        if (target > total->lower_length) {
            return std::unexpected{
                CurveInverseLengthError::target_domain_indeterminate};
        }
    } else {
        if (target == 0.0) {
            return inverse_evidence(
                CurveInverseLengthResult::converged,
                0.0,
                0.0,
                0.0,
                0.0,
                *zero,
                *zero,
                *total,
                *regularity,
                0);
        }
        if (target == 1.0) {
            return inverse_evidence(
                CurveInverseLengthResult::converged,
                total->lower_length,
                total->upper_length,
                1.0,
                1.0,
                *total,
                *total,
                *total,
                *regularity,
                0);
        }

        const auto target_interval =
            fraction_target_interval(target, *total);
        if (!target_interval) {
            return std::unexpected{target_interval.error()};
        }
        target_lower = target_interval->first;
        target_upper = target_interval->second;
        if (target_upper > total->lower_length) {
            return std::unexpected{
                CurveInverseLengthError::target_domain_indeterminate};
        }
    }

    double lower_parameter = 0.0;
    double upper_parameter = 1.0;
    CurveLengthEvidence lower_cumulative = *zero;
    CurveLengthEvidence upper_cumulative = *total;

    if (!(lower_cumulative.upper_length <= target_lower &&
          target_upper <= upper_cumulative.lower_length)) {
        return std::unexpected{
            CurveInverseLengthError::target_domain_indeterminate};
    }

    if (upper_parameter - lower_parameter <=
        policy.parameter_tolerance) {
        return inverse_evidence(
            CurveInverseLengthResult::converged,
            target_lower,
            target_upper,
            lower_parameter,
            upper_parameter,
            lower_cumulative,
            upper_cumulative,
            *total,
            *regularity,
            0);
    }

    for (std::size_t iteration = 0;
         iteration < policy.max_refinement_iterations;
         ++iteration) {
        const double midpoint =
            std::midpoint(lower_parameter, upper_parameter);
        if (midpoint == lower_parameter ||
            midpoint == upper_parameter) {
            return inverse_evidence(
                CurveInverseLengthResult::indeterminate,
                target_lower,
                target_upper,
                lower_parameter,
                upper_parameter,
                lower_cumulative,
                upper_cumulative,
                *total,
                *regularity,
                iteration);
        }

        const auto middle_cumulative =
            curve.cumulative_arc_length_enclosure(
                midpoint,
                policy.length_policy);
        if (!middle_cumulative) {
            return std::unexpected{
                inverse_error(middle_cumulative.error())};
        }
        if (!valid_length_evidence(*middle_cumulative)) {
            return std::unexpected{
                CurveInverseLengthError::non_finite_enclosure};
        }

        const std::size_t completed_iterations = iteration + 1;
        if (target_lower == target_upper &&
            middle_cumulative->lower_length ==
                middle_cumulative->upper_length &&
            middle_cumulative->lower_length == target_lower) {
            return inverse_evidence(
                CurveInverseLengthResult::converged,
                target_lower,
                target_upper,
                midpoint,
                midpoint,
                *middle_cumulative,
                *middle_cumulative,
                *total,
                *regularity,
                completed_iterations);
        }

        if (middle_cumulative->upper_length <= target_lower) {
            lower_parameter = midpoint;
            lower_cumulative = *middle_cumulative;
        } else if (
            target_upper <= middle_cumulative->lower_length) {
            upper_parameter = midpoint;
            upper_cumulative = *middle_cumulative;
        } else {
            return inverse_evidence(
                CurveInverseLengthResult::indeterminate,
                target_lower,
                target_upper,
                lower_parameter,
                upper_parameter,
                lower_cumulative,
                upper_cumulative,
                *total,
                *regularity,
                completed_iterations);
        }

        if (upper_parameter - lower_parameter <=
            policy.parameter_tolerance) {
            return inverse_evidence(
                CurveInverseLengthResult::converged,
                target_lower,
                target_upper,
                lower_parameter,
                upper_parameter,
                lower_cumulative,
                upper_cumulative,
                *total,
                *regularity,
                completed_iterations);
        }
    }

    return inverse_evidence(
        CurveInverseLengthResult::indeterminate,
        target_lower,
        target_upper,
        lower_parameter,
        upper_parameter,
        lower_cumulative,
        upper_cumulative,
        *total,
        *regularity,
        policy.max_refinement_iterations);
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

std::expected<double, CurveError> CubicBezier2::curvature_magnitude(
    const double parameter) const noexcept {
    const auto first = first_derivative(parameter);
    if (!first.has_value()) {
        return std::unexpected{first.error()};
    }
    if (max_abs_component(*first) == 0.0) {
        return std::unexpected{CurveError::singular_parameter};
    }

    const auto second = second_derivative(parameter);
    if (!second.has_value()) {
        return std::unexpected{second.error()};
    }
    return curvature_magnitude_impl(*first, *second);
}

std::expected<double, CurveError> CubicBezier2::signed_curvature(
    const double parameter) const noexcept {
    const auto first = first_derivative(parameter);
    if (!first.has_value()) {
        return std::unexpected{first.error()};
    }
    if (max_abs_component(*first) == 0.0) {
        return std::unexpected{CurveError::singular_parameter};
    }

    const auto second = second_derivative(parameter);
    if (!second.has_value()) {
        return std::unexpected{second.error()};
    }
    return signed_curvature_impl(*first, *second);
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


std::expected<CurveInflectionIsolationEvidence, CurveInflectionError>
CubicBezier2::isolate_simple_inflections(
    const CurveInflectionIsolationPolicy& policy) const noexcept {
    const auto valid_policy = validate_inflection_policy(policy);
    if (!valid_policy) {
        return std::unexpected{valid_policy.error()};
    }

    const auto regularity = certify_regularity(policy.regularity_policy);
    if (!regularity) {
        return std::unexpected{inflection_error(regularity.error())};
    }
    if (regularity->result == CurveRegularityResult::degenerate) {
        return std::unexpected{CurveInflectionError::curve_not_regular};
    }
    if (regularity->result == CurveRegularityResult::indeterminate) {
        return inflection_evidence(
            CurveInflectionIsolationResult::indeterminate,
            *regularity,
            {},
            0,
            0,
            0,
            0,
            0);
    }

    const auto controls = inflection_derivative_interval_controls(control_points_);
    if (!controls) {
        return std::unexpected{controls.error()};
    }

    return isolate_simple_inflections_impl(*controls, *regularity, policy);
}

std::expected<CurveLengthEvidence, CurveLengthError>
CubicBezier2::arc_length_enclosure(
    const CurveLengthPolicy& policy) const noexcept {
    return arc_length_enclosure_impl<Point2, 2>(control_points_, policy);
}

std::expected<CurveLengthEvidence, CurveLengthError>
CubicBezier2::cumulative_arc_length_enclosure(
    const double parameter,
    const CurveLengthPolicy& policy) const noexcept {
    return cumulative_arc_length_enclosure_impl<Point2, 2>(
        control_points_,
        parameter,
        policy);
}

std::expected<CurveInverseLengthEvidence, CurveInverseLengthError>
CubicBezier2::inverse_arc_length_bracket(
    const double target_length,
    const CurveInverseLengthPolicy& policy) const noexcept {
    return inverse_arc_length_bracket_impl(
        *this,
        target_length,
        policy,
        InverseTargetMode::absolute_length);
}

std::expected<CurveInverseLengthEvidence, CurveInverseLengthError>
CubicBezier2::inverse_arc_length_fraction_bracket(
    const double normalized_fraction,
    const CurveInverseLengthPolicy& policy) const noexcept {
    return inverse_arc_length_bracket_impl(
        *this,
        normalized_fraction,
        policy,
        InverseTargetMode::normalized_fraction);
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

std::expected<double, CurveError> CubicBezier3::curvature_magnitude(
    const double parameter) const noexcept {
    const auto first = first_derivative(parameter);
    if (!first.has_value()) {
        return std::unexpected{first.error()};
    }
    if (max_abs_component(*first) == 0.0) {
        return std::unexpected{CurveError::singular_parameter};
    }

    const auto second = second_derivative(parameter);
    if (!second.has_value()) {
        return std::unexpected{second.error()};
    }
    return curvature_magnitude_impl(*first, *second);
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

std::expected<CurveLengthEvidence, CurveLengthError>
CubicBezier3::cumulative_arc_length_enclosure(
    const double parameter,
    const CurveLengthPolicy& policy) const noexcept {
    return cumulative_arc_length_enclosure_impl<Point3, 3>(
        control_points_,
        parameter,
        policy);
}

std::expected<CurveInverseLengthEvidence, CurveInverseLengthError>
CubicBezier3::inverse_arc_length_bracket(
    const double target_length,
    const CurveInverseLengthPolicy& policy) const noexcept {
    return inverse_arc_length_bracket_impl(
        *this,
        target_length,
        policy,
        InverseTargetMode::absolute_length);
}

std::expected<CurveInverseLengthEvidence, CurveInverseLengthError>
CubicBezier3::inverse_arc_length_fraction_bracket(
    const double normalized_fraction,
    const CurveInverseLengthPolicy& policy) const noexcept {
    return inverse_arc_length_bracket_impl(
        *this,
        normalized_fraction,
        policy,
        InverseTargetMode::normalized_fraction);
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
