#include "apmesh/geometry/nurbs.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <expected>
#include <limits>
#include <span>
#include <utility>
#include <vector>

namespace apmesh::core {
namespace {

struct Homogeneous2 {
    long double x{};
    long double y{};
    long double w{};
};

struct Homogeneous3 {
    long double x{};
    long double y{};
    long double z{};
    long double w{};
};

[[nodiscard]] std::expected<void, CurveError> validate_parameter(
    const CurveParameterDomain& domain,
    const double parameter) noexcept {
    const auto contained = domain.contains(parameter);
    if (!contained.has_value()) {
        return std::unexpected{contained.error()};
    }
    if (!*contained) {
        return std::unexpected{CurveError::parameter_out_of_domain};
    }
    return {};
}

[[nodiscard]] std::expected<long double, CurveError> parameter_ratio(
    const double value,
    const double lower,
    const double upper) noexcept {
    long double wide_value = static_cast<long double>(value);
    long double wide_lower = static_cast<long double>(lower);
    long double wide_upper = static_cast<long double>(upper);

    long double numerator = wide_value - wide_lower;
    long double denominator = wide_upper - wide_lower;

    if (!std::isfinite(numerator) || !std::isfinite(denominator)) {
        const long double scale = std::max(
            {std::abs(wide_value), std::abs(wide_lower), std::abs(wide_upper)});
        if (!std::isfinite(scale) || scale == 0.0L) {
            return std::unexpected{CurveError::non_finite_result};
        }

        wide_value /= scale;
        wide_lower /= scale;
        wide_upper /= scale;
        numerator = wide_value - wide_lower;
        denominator = wide_upper - wide_lower;
    }

    if (!std::isfinite(numerator) || !std::isfinite(denominator) ||
        denominator <= 0.0L) {
        return std::unexpected{CurveError::non_finite_result};
    }

    const long double ratio = numerator / denominator;
    if (!std::isfinite(ratio) || ratio < 0.0L || ratio > 1.0L) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return ratio;
}

[[nodiscard]] double flat_knot(
    const std::span<const double> interior_knots,
    const double lower,
    const double upper,
    const std::size_t control_count,
    const std::size_t index) noexcept {
    if (index < 4U) {
        return lower;
    }
    if (index < control_count) {
        return interior_knots[index - 4U];
    }
    return upper;
}

[[nodiscard]] std::size_t locate_span(
    const std::span<const double> interior_knots,
    const double upper,
    const double parameter) noexcept {
    if (parameter == upper) {
        return interior_knots.size();
    }
    return static_cast<std::size_t>(
        std::upper_bound(
            interior_knots.begin(), interior_knots.end(), parameter) -
        interior_knots.begin());
}

[[nodiscard]] Homogeneous2 lerp_homogeneous(
    const Homogeneous2& lhs,
    const Homogeneous2& rhs,
    const long double parameter) noexcept {
    return {
        std::lerp(lhs.x, rhs.x, parameter),
        std::lerp(lhs.y, rhs.y, parameter),
        std::lerp(lhs.w, rhs.w, parameter),
    };
}

[[nodiscard]] Homogeneous3 lerp_homogeneous(
    const Homogeneous3& lhs,
    const Homogeneous3& rhs,
    const long double parameter) noexcept {
    return {
        std::lerp(lhs.x, rhs.x, parameter),
        std::lerp(lhs.y, rhs.y, parameter),
        std::lerp(lhs.z, rhs.z, parameter),
        std::lerp(lhs.w, rhs.w, parameter),
    };
}

template <typename Homogeneous, std::size_t Degree>
[[nodiscard]] std::expected<Homogeneous, CurveError> de_boor_local(
    std::array<Homogeneous, Degree + 1U> work,
    const std::size_t spline_span,
    const std::size_t knot_offset,
    const std::span<const double> interior_knots,
    const double lower,
    const double upper,
    const std::size_t control_count,
    const double parameter) noexcept {
    for (int level = 1; level <= static_cast<int>(Degree); ++level) {
        for (int local = static_cast<int>(Degree); local >= level; --local) {
            const std::size_t knot_index =
                spline_span - Degree + static_cast<std::size_t>(local);
            const std::size_t upper_index =
                knot_index + Degree - static_cast<std::size_t>(level) + 1U;
            const auto alpha = parameter_ratio(
                parameter,
                flat_knot(
                    interior_knots,
                    lower,
                    upper,
                    control_count,
                    knot_index + knot_offset),
                flat_knot(
                    interior_knots,
                    lower,
                    upper,
                    control_count,
                    upper_index + knot_offset));
            if (!alpha.has_value()) {
                return std::unexpected{alpha.error()};
            }
            work[static_cast<std::size_t>(local)] = lerp_homogeneous(
                work[static_cast<std::size_t>(local - 1)],
                work[static_cast<std::size_t>(local)],
                *alpha);
        }
    }

    return work[Degree];
}

[[nodiscard]] std::expected<long double, CurveError> scaled_difference(
    const long double next,
    const long double current,
    const int degree,
    const double lower_knot,
    const double upper_knot) noexcept {
    const long double denominator =
        static_cast<long double>(upper_knot) -
        static_cast<long double>(lower_knot);
    const long double numerator =
        static_cast<long double>(degree) * (next - current);

    if (!std::isfinite(denominator) || denominator <= 0.0L ||
        !std::isfinite(numerator)) {
        return std::unexpected{CurveError::non_finite_result};
    }

    const long double result = numerator / denominator;
    if (!std::isfinite(result)) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return result;
}

[[nodiscard]] std::expected<Homogeneous2, CurveError> derivative_control(
    const Homogeneous2& current,
    const Homogeneous2& next,
    const int degree,
    const double lower_knot,
    const double upper_knot) noexcept {
    const auto x = scaled_difference(
        next.x, current.x, degree, lower_knot, upper_knot);
    const auto y = scaled_difference(
        next.y, current.y, degree, lower_knot, upper_knot);
    const auto w = scaled_difference(
        next.w, current.w, degree, lower_knot, upper_knot);
    if (!x || !y || !w) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return Homogeneous2{*x, *y, *w};
}

[[nodiscard]] std::expected<Homogeneous3, CurveError> derivative_control(
    const Homogeneous3& current,
    const Homogeneous3& next,
    const int degree,
    const double lower_knot,
    const double upper_knot) noexcept {
    const auto x = scaled_difference(
        next.x, current.x, degree, lower_knot, upper_knot);
    const auto y = scaled_difference(
        next.y, current.y, degree, lower_knot, upper_knot);
    const auto z = scaled_difference(
        next.z, current.z, degree, lower_knot, upper_knot);
    const auto w = scaled_difference(
        next.w, current.w, degree, lower_knot, upper_knot);
    if (!x || !y || !z || !w) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return Homogeneous3{*x, *y, *z, *w};
}

[[nodiscard]] std::array<Homogeneous2, 4> local_homogeneous_controls(
    const std::span<const Point2> points,
    const std::span<const double> weights,
    const std::size_t start) noexcept {
    const double maximum_weight = std::max(
        {weights[start],
         weights[start + 1U],
         weights[start + 2U],
         weights[start + 3U]});

    std::array<Homogeneous2, 4> result{};
    for (std::size_t local = 0; local < result.size(); ++local) {
        const std::size_t index = start + local;
        const long double weight =
            static_cast<long double>(weights[index]) /
            static_cast<long double>(maximum_weight);
        result[local] = {
            static_cast<long double>(points[index].x()) * weight,
            static_cast<long double>(points[index].y()) * weight,
            weight,
        };
    }
    return result;
}

[[nodiscard]] std::array<Homogeneous3, 4> local_homogeneous_controls(
    const std::span<const Point3> points,
    const std::span<const double> weights,
    const std::size_t start) noexcept {
    const double maximum_weight = std::max(
        {weights[start],
         weights[start + 1U],
         weights[start + 2U],
         weights[start + 3U]});

    std::array<Homogeneous3, 4> result{};
    for (std::size_t local = 0; local < result.size(); ++local) {
        const std::size_t index = start + local;
        const long double weight =
            static_cast<long double>(weights[index]) /
            static_cast<long double>(maximum_weight);
        result[local] = {
            static_cast<long double>(points[index].x()) * weight,
            static_cast<long double>(points[index].y()) * weight,
            static_cast<long double>(points[index].z()) * weight,
            weight,
        };
    }
    return result;
}

template <typename Homogeneous>
[[nodiscard]] std::expected<std::array<Homogeneous, 3>, CurveError>
first_derivative_controls(
    const std::array<Homogeneous, 4>& controls,
    const std::span<const double> interior_knots,
    const double lower,
    const double upper,
    const std::size_t control_count,
    const std::size_t start) noexcept {
    std::array<Homogeneous, 3> result{};
    for (std::size_t local = 0; local < result.size(); ++local) {
        const std::size_t index = start + local;
        const auto value = derivative_control(
            controls[local],
            controls[local + 1U],
            3,
            flat_knot(
                interior_knots, lower, upper, control_count, index + 1U),
            flat_knot(
                interior_knots, lower, upper, control_count, index + 4U));
        if (!value.has_value()) {
            return std::unexpected{value.error()};
        }
        result[local] = *value;
    }
    return result;
}

template <typename Homogeneous>
[[nodiscard]] std::expected<std::array<Homogeneous, 2>, CurveError>
second_derivative_controls(
    const std::array<Homogeneous, 3>& controls,
    const std::span<const double> interior_knots,
    const double lower,
    const double upper,
    const std::size_t control_count,
    const std::size_t start) noexcept {
    std::array<Homogeneous, 2> result{};
    for (std::size_t local = 0; local < result.size(); ++local) {
        const std::size_t index = start + local;
        const auto value = derivative_control(
            controls[local],
            controls[local + 1U],
            2,
            flat_knot(
                interior_knots, lower, upper, control_count, index + 2U),
            flat_knot(
                interior_knots, lower, upper, control_count, index + 4U));
        if (!value.has_value()) {
            return std::unexpected{value.error()};
        }
        result[local] = *value;
    }
    return result;
}

[[nodiscard]] std::expected<Homogeneous2, CurveError> evaluate_homogeneous(
    const std::span<const Point2> points,
    const std::span<const double> weights,
    const std::span<const double> interior_knots,
    const double lower,
    const double upper,
    const double parameter) noexcept {
    const std::size_t start =
        locate_span(interior_knots, upper, parameter);
    const auto controls =
        local_homogeneous_controls(points, weights, start);
    return de_boor_local<Homogeneous2, 3>(
        controls,
        start + 3U,
        0U,
        interior_knots,
        lower,
        upper,
        points.size(),
        parameter);
}

[[nodiscard]] std::expected<Homogeneous3, CurveError> evaluate_homogeneous(
    const std::span<const Point3> points,
    const std::span<const double> weights,
    const std::span<const double> interior_knots,
    const double lower,
    const double upper,
    const double parameter) noexcept {
    const std::size_t start =
        locate_span(interior_knots, upper, parameter);
    const auto controls =
        local_homogeneous_controls(points, weights, start);
    return de_boor_local<Homogeneous3, 3>(
        controls,
        start + 3U,
        0U,
        interior_knots,
        lower,
        upper,
        points.size(),
        parameter);
}

[[nodiscard]] std::expected<std::array<Homogeneous2, 3>, CurveError>
evaluate_homogeneous_jet(
    const std::span<const Point2> points,
    const std::span<const double> weights,
    const std::span<const double> interior_knots,
    const double lower,
    const double upper,
    const double parameter) noexcept {
    const std::size_t start =
        locate_span(interior_knots, upper, parameter);
    const auto controls =
        local_homogeneous_controls(points, weights, start);
    const auto first = first_derivative_controls(
        controls,
        interior_knots,
        lower,
        upper,
        points.size(),
        start);
    if (!first.has_value()) {
        return std::unexpected{first.error()};
    }
    const auto second = second_derivative_controls(
        *first,
        interior_knots,
        lower,
        upper,
        points.size(),
        start);
    if (!second.has_value()) {
        return std::unexpected{second.error()};
    }

    const auto value = de_boor_local<Homogeneous2, 3>(
        controls,
        start + 3U,
        0U,
        interior_knots,
        lower,
        upper,
        points.size(),
        parameter);
    const auto d1 = de_boor_local<Homogeneous2, 2>(
        *first,
        start + 2U,
        1U,
        interior_knots,
        lower,
        upper,
        points.size(),
        parameter);
    const auto d2 = de_boor_local<Homogeneous2, 1>(
        *second,
        start + 1U,
        2U,
        interior_knots,
        lower,
        upper,
        points.size(),
        parameter);

    if (!value || !d1 || !d2) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return std::array<Homogeneous2, 3>{*value, *d1, *d2};
}

[[nodiscard]] std::expected<std::array<Homogeneous3, 3>, CurveError>
evaluate_homogeneous_jet(
    const std::span<const Point3> points,
    const std::span<const double> weights,
    const std::span<const double> interior_knots,
    const double lower,
    const double upper,
    const double parameter) noexcept {
    const std::size_t start =
        locate_span(interior_knots, upper, parameter);
    const auto controls =
        local_homogeneous_controls(points, weights, start);
    const auto first = first_derivative_controls(
        controls,
        interior_knots,
        lower,
        upper,
        points.size(),
        start);
    if (!first.has_value()) {
        return std::unexpected{first.error()};
    }
    const auto second = second_derivative_controls(
        *first,
        interior_knots,
        lower,
        upper,
        points.size(),
        start);
    if (!second.has_value()) {
        return std::unexpected{second.error()};
    }

    const auto value = de_boor_local<Homogeneous3, 3>(
        controls,
        start + 3U,
        0U,
        interior_knots,
        lower,
        upper,
        points.size(),
        parameter);
    const auto d1 = de_boor_local<Homogeneous3, 2>(
        *first,
        start + 2U,
        1U,
        interior_knots,
        lower,
        upper,
        points.size(),
        parameter);
    const auto d2 = de_boor_local<Homogeneous3, 1>(
        *second,
        start + 1U,
        2U,
        interior_knots,
        lower,
        upper,
        points.size(),
        parameter);

    if (!value || !d1 || !d2) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return std::array<Homogeneous3, 3>{*value, *d1, *d2};
}

[[nodiscard]] std::expected<double, CurveError> to_double(
    const long double value) noexcept {
    constexpr long double maximum =
        static_cast<long double>(std::numeric_limits<double>::max());

    if (!std::isfinite(value) || value > maximum || value < -maximum) {
        return std::unexpected{CurveError::non_finite_result};
    }

    const double converted = static_cast<double>(value);
    if (!std::isfinite(converted)) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return converted;
}

[[nodiscard]] std::expected<Point2, CurveError> dehomogenize_point(
    const Homogeneous2& value) noexcept {
    if (!std::isfinite(value.w) || value.w <= 0.0L) {
        return std::unexpected{CurveError::non_finite_result};
    }

    const auto x = to_double(value.x / value.w);
    const auto y = to_double(value.y / value.w);
    if (!x || !y) {
        return std::unexpected{CurveError::non_finite_result};
    }

    const auto point = Point2::make(*x, *y);
    if (!point.has_value()) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return *point;
}

[[nodiscard]] std::expected<Point3, CurveError> dehomogenize_point(
    const Homogeneous3& value) noexcept {
    if (!std::isfinite(value.w) || value.w <= 0.0L) {
        return std::unexpected{CurveError::non_finite_result};
    }

    const auto x = to_double(value.x / value.w);
    const auto y = to_double(value.y / value.w);
    const auto z = to_double(value.z / value.w);
    if (!x || !y || !z) {
        return std::unexpected{CurveError::non_finite_result};
    }

    const auto point = Point3::make(*x, *y, *z);
    if (!point.has_value()) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return *point;
}

[[nodiscard]] std::expected<Vector2, CurveError> dehomogenize_first(
    const Homogeneous2& value,
    const Homogeneous2& first) noexcept {
    if (!std::isfinite(value.w) || value.w <= 0.0L) {
        return std::unexpected{CurveError::non_finite_result};
    }

    const long double cx = value.x / value.w;
    const long double cy = value.y / value.w;
    const auto x = to_double((first.x - cx * first.w) / value.w);
    const auto y = to_double((first.y - cy * first.w) / value.w);
    if (!x || !y) {
        return std::unexpected{CurveError::non_finite_result};
    }

    const auto vector = Vector2::make(*x, *y);
    if (!vector.has_value()) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return *vector;
}

[[nodiscard]] std::expected<Vector3, CurveError> dehomogenize_first(
    const Homogeneous3& value,
    const Homogeneous3& first) noexcept {
    if (!std::isfinite(value.w) || value.w <= 0.0L) {
        return std::unexpected{CurveError::non_finite_result};
    }

    const long double cx = value.x / value.w;
    const long double cy = value.y / value.w;
    const long double cz = value.z / value.w;
    const auto x = to_double((first.x - cx * first.w) / value.w);
    const auto y = to_double((first.y - cy * first.w) / value.w);
    const auto z = to_double((first.z - cz * first.w) / value.w);
    if (!x || !y || !z) {
        return std::unexpected{CurveError::non_finite_result};
    }

    const auto vector = Vector3::make(*x, *y, *z);
    if (!vector.has_value()) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return *vector;
}

[[nodiscard]] std::expected<Vector2, CurveError> dehomogenize_second(
    const Homogeneous2& value,
    const Homogeneous2& first,
    const Homogeneous2& second) noexcept {
    if (!std::isfinite(value.w) || value.w <= 0.0L) {
        return std::unexpected{CurveError::non_finite_result};
    }

    const long double cx = value.x / value.w;
    const long double cy = value.y / value.w;
    const long double d1x = (first.x - cx * first.w) / value.w;
    const long double d1y = (first.y - cy * first.w) / value.w;
    if (!std::isfinite(d1x) || !std::isfinite(d1y)) {
        return std::unexpected{CurveError::non_finite_result};
    }

    const auto x = to_double(
        (second.x - 2.0L * d1x * first.w - cx * second.w) / value.w);
    const auto y = to_double(
        (second.y - 2.0L * d1y * first.w - cy * second.w) / value.w);
    if (!x || !y) {
        return std::unexpected{CurveError::non_finite_result};
    }

    const auto vector = Vector2::make(*x, *y);
    if (!vector.has_value()) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return *vector;
}

[[nodiscard]] std::expected<Vector3, CurveError> dehomogenize_second(
    const Homogeneous3& value,
    const Homogeneous3& first,
    const Homogeneous3& second) noexcept {
    if (!std::isfinite(value.w) || value.w <= 0.0L) {
        return std::unexpected{CurveError::non_finite_result};
    }

    const long double cx = value.x / value.w;
    const long double cy = value.y / value.w;
    const long double cz = value.z / value.w;
    const long double d1x = (first.x - cx * first.w) / value.w;
    const long double d1y = (first.y - cy * first.w) / value.w;
    const long double d1z = (first.z - cz * first.w) / value.w;
    if (!std::isfinite(d1x) || !std::isfinite(d1y) || !std::isfinite(d1z)) {
        return std::unexpected{CurveError::non_finite_result};
    }

    const auto x = to_double(
        (second.x - 2.0L * d1x * first.w - cx * second.w) / value.w);
    const auto y = to_double(
        (second.y - 2.0L * d1y * first.w - cy * second.w) / value.w);
    const auto z = to_double(
        (second.z - 2.0L * d1z * first.w - cz * second.w) / value.w);
    if (!x || !y || !z) {
        return std::unexpected{CurveError::non_finite_result};
    }

    const auto vector = Vector3::make(*x, *y, *z);
    if (!vector.has_value()) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return *vector;
}

[[nodiscard]] bool constant_points(
    const std::span<const Point2> points) noexcept {
    return std::all_of(
        points.begin() + 1,
        points.end(),
        [&points](const Point2& point) { return point == points.front(); });
}

[[nodiscard]] bool constant_points(
    const std::span<const Point3> points) noexcept {
    return std::all_of(
        points.begin() + 1,
        points.end(),
        [&points](const Point3& point) { return point == points.front(); });
}

[[nodiscard]] std::expected<void, MultiSpanNURBSConstructionError>
validate_layout(
    const std::size_t control_count,
    const std::size_t weight_count,
    const std::span<const double> interior_knots,
    const double lower,
    const double upper) noexcept {
    if (control_count < 5U) {
        return std::unexpected{
            MultiSpanNURBSConstructionError::insufficient_control_points};
    }
    if (weight_count != control_count) {
        return std::unexpected{
            MultiSpanNURBSConstructionError::control_weight_count_mismatch};
    }
    if (interior_knots.size() != control_count - 4U) {
        return std::unexpected{
            MultiSpanNURBSConstructionError::interior_knot_count_mismatch};
    }
    if (!std::isfinite(lower)) {
        return std::unexpected{
            MultiSpanNURBSConstructionError::non_finite_lower_knot};
    }
    if (!std::isfinite(upper)) {
        return std::unexpected{
            MultiSpanNURBSConstructionError::non_finite_upper_knot};
    }
    if (!(lower < upper)) {
        return std::unexpected{
            MultiSpanNURBSConstructionError::non_strict_knot_order};
    }

    double previous = lower;
    for (const double knot : interior_knots) {
        if (!std::isfinite(knot)) {
            return std::unexpected{
                MultiSpanNURBSConstructionError::non_finite_interior_knot};
        }
        if (!(previous < knot && knot < upper)) {
            return std::unexpected{
                MultiSpanNURBSConstructionError::non_strict_knot_order};
        }
        previous = knot;
    }
    return {};
}

[[nodiscard]] std::expected<void, MultiSpanNURBSConstructionError>
validate_weights(const std::span<const double> weights) noexcept {
    for (const double weight : weights) {
        if (!std::isfinite(weight)) {
            return std::unexpected{
                MultiSpanNURBSConstructionError::non_finite_weight};
        }
        if (weight <= 0.0) {
            return std::unexpected{
                MultiSpanNURBSConstructionError::non_positive_weight};
        }
    }
    return {};
}

} // namespace

MultiSpanCubicNURBS2::MultiSpanCubicNURBS2(
    std::vector<Point2> control_points,
    std::vector<double> weights,
    std::vector<double> interior_knots,
    const double lower_knot,
    const double upper_knot,
    const bool constant)
    : control_points_(std::move(control_points)),
      weights_(std::move(weights)),
      interior_knots_(std::move(interior_knots)),
      lower_knot_(lower_knot),
      upper_knot_(upper_knot),
      constant_(constant) {}

std::expected<MultiSpanCubicNURBS2, MultiSpanNURBSConstructionError>
MultiSpanCubicNURBS2::make(
    std::vector<Point2> control_points,
    std::vector<double> weights,
    std::vector<double> interior_knots,
    const double lower_knot,
    const double upper_knot) {
    const auto layout = validate_layout(
        control_points.size(),
        weights.size(),
        interior_knots,
        lower_knot,
        upper_knot);
    if (!layout.has_value()) {
        return std::unexpected{layout.error()};
    }
    const auto valid_weights = validate_weights(weights);
    if (!valid_weights.has_value()) {
        return std::unexpected{valid_weights.error()};
    }
    const bool constant =
        constant_points(std::span<const Point2>{control_points});
    return MultiSpanCubicNURBS2{
        std::move(control_points),
        std::move(weights),
        std::move(interior_knots),
        lower_knot,
        upper_knot,
        constant};
}

std::span<const Point2> MultiSpanCubicNURBS2::control_points() const noexcept {
    return std::span<const Point2>{control_points_};
}

std::span<const double> MultiSpanCubicNURBS2::weights() const noexcept {
    return std::span<const double>{weights_};
}

std::span<const double> MultiSpanCubicNURBS2::interior_knots() const noexcept {
    return std::span<const double>{interior_knots_};
}

std::size_t MultiSpanCubicNURBS2::span_count() const noexcept {
    return control_points_.size() - 3U;
}

double MultiSpanCubicNURBS2::lower_knot() const noexcept {
    return lower_knot_;
}

double MultiSpanCubicNURBS2::upper_knot() const noexcept {
    return upper_knot_;
}

CurveParameterDomain MultiSpanCubicNURBS2::parameter_domain() const noexcept {
    return *CurveParameterDomain::make(lower_knot_, upper_knot_);
}

std::expected<Point2, CurveError> MultiSpanCubicNURBS2::evaluate(
    const double parameter) const noexcept {
    const auto valid = validate_parameter(parameter_domain(), parameter);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }
    if (parameter == lower_knot_) {
        return control_points_.front();
    }
    if (parameter == upper_knot_) {
        return control_points_.back();
    }
    if (constant_) {
        return control_points_.front();
    }

    const auto value = evaluate_homogeneous(
        control_points(),
        weights(),
        interior_knots(),
        lower_knot_,
        upper_knot_,
        parameter);
    if (!value.has_value()) {
        return std::unexpected{value.error()};
    }
    return dehomogenize_point(*value);
}

std::expected<Vector2, CurveError>
MultiSpanCubicNURBS2::first_derivative(
    const double parameter) const noexcept {
    const auto valid = validate_parameter(parameter_domain(), parameter);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }
    if (constant_) {
        return *Vector2::make(0.0, 0.0);
    }

    const auto jet = evaluate_homogeneous_jet(
        control_points(),
        weights(),
        interior_knots(),
        lower_knot_,
        upper_knot_,
        parameter);
    if (!jet.has_value()) {
        return std::unexpected{jet.error()};
    }
    return dehomogenize_first((*jet)[0], (*jet)[1]);
}

std::expected<Vector2, CurveError>
MultiSpanCubicNURBS2::second_derivative(
    const double parameter) const noexcept {
    const auto valid = validate_parameter(parameter_domain(), parameter);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }
    if (constant_) {
        return *Vector2::make(0.0, 0.0);
    }

    const auto jet = evaluate_homogeneous_jet(
        control_points(),
        weights(),
        interior_knots(),
        lower_knot_,
        upper_knot_,
        parameter);
    if (!jet.has_value()) {
        return std::unexpected{jet.error()};
    }
    return dehomogenize_second((*jet)[0], (*jet)[1], (*jet)[2]);
}

MultiSpanCubicNURBS2 MultiSpanCubicNURBS2::reversed() const {
    std::vector<Point2> reversed_points(
        control_points_.rbegin(), control_points_.rend());
    std::vector<double> reversed_weights(
        weights_.rbegin(), weights_.rend());
    std::vector<double> reversed_knots;
    reversed_knots.reserve(interior_knots_.size());

    const auto domain = parameter_domain();
    for (auto iterator = interior_knots_.rbegin();
         iterator != interior_knots_.rend();
         ++iterator) {
        const auto mapped = reversed_parameter(domain, *iterator);
        reversed_knots.push_back(*mapped);
    }

    return MultiSpanCubicNURBS2{
        std::move(reversed_points),
        std::move(reversed_weights),
        std::move(reversed_knots),
        lower_knot_,
        upper_knot_,
        constant_};
}

MultiSpanCubicNURBS3::MultiSpanCubicNURBS3(
    std::vector<Point3> control_points,
    std::vector<double> weights,
    std::vector<double> interior_knots,
    const double lower_knot,
    const double upper_knot,
    const bool constant)
    : control_points_(std::move(control_points)),
      weights_(std::move(weights)),
      interior_knots_(std::move(interior_knots)),
      lower_knot_(lower_knot),
      upper_knot_(upper_knot),
      constant_(constant) {}

std::expected<MultiSpanCubicNURBS3, MultiSpanNURBSConstructionError>
MultiSpanCubicNURBS3::make(
    std::vector<Point3> control_points,
    std::vector<double> weights,
    std::vector<double> interior_knots,
    const double lower_knot,
    const double upper_knot) {
    const auto layout = validate_layout(
        control_points.size(),
        weights.size(),
        interior_knots,
        lower_knot,
        upper_knot);
    if (!layout.has_value()) {
        return std::unexpected{layout.error()};
    }
    const auto valid_weights = validate_weights(weights);
    if (!valid_weights.has_value()) {
        return std::unexpected{valid_weights.error()};
    }
    const bool constant =
        constant_points(std::span<const Point3>{control_points});
    return MultiSpanCubicNURBS3{
        std::move(control_points),
        std::move(weights),
        std::move(interior_knots),
        lower_knot,
        upper_knot,
        constant};
}

std::span<const Point3> MultiSpanCubicNURBS3::control_points() const noexcept {
    return std::span<const Point3>{control_points_};
}

std::span<const double> MultiSpanCubicNURBS3::weights() const noexcept {
    return std::span<const double>{weights_};
}

std::span<const double> MultiSpanCubicNURBS3::interior_knots() const noexcept {
    return std::span<const double>{interior_knots_};
}

std::size_t MultiSpanCubicNURBS3::span_count() const noexcept {
    return control_points_.size() - 3U;
}

double MultiSpanCubicNURBS3::lower_knot() const noexcept {
    return lower_knot_;
}

double MultiSpanCubicNURBS3::upper_knot() const noexcept {
    return upper_knot_;
}

CurveParameterDomain MultiSpanCubicNURBS3::parameter_domain() const noexcept {
    return *CurveParameterDomain::make(lower_knot_, upper_knot_);
}

std::expected<Point3, CurveError> MultiSpanCubicNURBS3::evaluate(
    const double parameter) const noexcept {
    const auto valid = validate_parameter(parameter_domain(), parameter);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }
    if (parameter == lower_knot_) {
        return control_points_.front();
    }
    if (parameter == upper_knot_) {
        return control_points_.back();
    }
    if (constant_) {
        return control_points_.front();
    }

    const auto value = evaluate_homogeneous(
        control_points(),
        weights(),
        interior_knots(),
        lower_knot_,
        upper_knot_,
        parameter);
    if (!value.has_value()) {
        return std::unexpected{value.error()};
    }
    return dehomogenize_point(*value);
}

std::expected<Vector3, CurveError>
MultiSpanCubicNURBS3::first_derivative(
    const double parameter) const noexcept {
    const auto valid = validate_parameter(parameter_domain(), parameter);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }
    if (constant_) {
        return *Vector3::make(0.0, 0.0, 0.0);
    }

    const auto jet = evaluate_homogeneous_jet(
        control_points(),
        weights(),
        interior_knots(),
        lower_knot_,
        upper_knot_,
        parameter);
    if (!jet.has_value()) {
        return std::unexpected{jet.error()};
    }
    return dehomogenize_first((*jet)[0], (*jet)[1]);
}

std::expected<Vector3, CurveError>
MultiSpanCubicNURBS3::second_derivative(
    const double parameter) const noexcept {
    const auto valid = validate_parameter(parameter_domain(), parameter);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }
    if (constant_) {
        return *Vector3::make(0.0, 0.0, 0.0);
    }

    const auto jet = evaluate_homogeneous_jet(
        control_points(),
        weights(),
        interior_knots(),
        lower_knot_,
        upper_knot_,
        parameter);
    if (!jet.has_value()) {
        return std::unexpected{jet.error()};
    }
    return dehomogenize_second((*jet)[0], (*jet)[1], (*jet)[2]);
}

MultiSpanCubicNURBS3 MultiSpanCubicNURBS3::reversed() const {
    std::vector<Point3> reversed_points(
        control_points_.rbegin(), control_points_.rend());
    std::vector<double> reversed_weights(
        weights_.rbegin(), weights_.rend());
    std::vector<double> reversed_knots;
    reversed_knots.reserve(interior_knots_.size());

    const auto domain = parameter_domain();
    for (auto iterator = interior_knots_.rbegin();
         iterator != interior_knots_.rend();
         ++iterator) {
        const auto mapped = reversed_parameter(domain, *iterator);
        reversed_knots.push_back(*mapped);
    }

    return MultiSpanCubicNURBS3{
        std::move(reversed_points),
        std::move(reversed_weights),
        std::move(reversed_knots),
        lower_knot_,
        upper_knot_,
        constant_};
}

} // namespace apmesh::core
