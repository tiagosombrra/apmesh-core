#include "apmesh/geometry/nurbs.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <expected>
#include <limits>

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

[[nodiscard]] std::array<double, 9> full_knots(
    const double lower,
    const double interior,
    const double upper) noexcept {
    return {
        lower,
        lower,
        lower,
        lower,
        interior,
        upper,
        upper,
        upper,
        upper,
    };
}

[[nodiscard]] std::array<double, 7> first_derivative_knots(
    const double lower,
    const double interior,
    const double upper) noexcept {
    return {
        lower,
        lower,
        lower,
        interior,
        upper,
        upper,
        upper,
    };
}

[[nodiscard]] std::array<double, 5> second_derivative_knots(
    const double lower,
    const double interior,
    const double upper) noexcept {
    return {lower, lower, interior, upper, upper};
}

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

template <typename Homogeneous, std::size_t ControlCount, std::size_t KnotCount>
[[nodiscard]] std::expected<Homogeneous, CurveError> de_boor(
    const std::array<Homogeneous, ControlCount>& controls,
    const std::array<double, KnotCount>& knots,
    const int degree,
    const int span,
    const double parameter) noexcept {
    std::array<Homogeneous, 4> work{};

    for (int local = 0; local <= degree; ++local) {
        const int control_index = span - degree + local;
        work[static_cast<std::size_t>(local)] =
            controls[static_cast<std::size_t>(control_index)];
    }

    for (int level = 1; level <= degree; ++level) {
        for (int local = degree; local >= level; --local) {
            const int knot_index = span - degree + local;
            const int upper_index = knot_index + degree - level + 1;
            const auto alpha = parameter_ratio(
                parameter,
                knots[static_cast<std::size_t>(knot_index)],
                knots[static_cast<std::size_t>(upper_index)]);
            if (!alpha.has_value()) {
                return std::unexpected{alpha.error()};
            }
            work[static_cast<std::size_t>(local)] = lerp_homogeneous(
                work[static_cast<std::size_t>(local - 1)],
                work[static_cast<std::size_t>(local)],
                *alpha);
        }
    }

    return work[static_cast<std::size_t>(degree)];
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

template <typename Homogeneous, std::size_t ControlCount, std::size_t KnotCount>
[[nodiscard]] std::expected<
    std::array<Homogeneous, ControlCount - 1>,
    CurveError>
derive_controls(
    const std::array<Homogeneous, ControlCount>& controls,
    const std::array<double, KnotCount>& knots,
    const int degree) noexcept {
    std::array<Homogeneous, ControlCount - 1> derived{};

    for (std::size_t index = 0; index + 1 < ControlCount; ++index) {
        const auto value = derivative_control(
            controls[index],
            controls[index + 1],
            degree,
            knots[index + 1],
            knots[index + static_cast<std::size_t>(degree) + 1]);
        if (!value.has_value()) {
            return std::unexpected{value.error()};
        }
        derived[index] = *value;
    }

    return derived;
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

[[nodiscard]] bool constant_points(
    const std::array<Point2, 5>& points) noexcept {
    return points[0] == points[1] &&
           points[1] == points[2] &&
           points[2] == points[3] &&
           points[3] == points[4];
}

[[nodiscard]] bool constant_points(
    const std::array<Point3, 5>& points) noexcept {
    return points[0] == points[1] &&
           points[1] == points[2] &&
           points[2] == points[3] &&
           points[3] == points[4];
}

[[nodiscard]] std::array<Homogeneous2, 5> homogeneous_controls(
    const std::array<Point2, 5>& points,
    const std::array<double, 5>& weights) noexcept {
    const double maximum_weight =
        *std::max_element(weights.begin(), weights.end());

    std::array<Homogeneous2, 5> result{};
    for (std::size_t index = 0; index < result.size(); ++index) {
        const long double weight =
            static_cast<long double>(weights[index]) /
            static_cast<long double>(maximum_weight);
        result[index] = {
            static_cast<long double>(points[index].x()) * weight,
            static_cast<long double>(points[index].y()) * weight,
            weight,
        };
    }
    return result;
}

[[nodiscard]] std::array<Homogeneous3, 5> homogeneous_controls(
    const std::array<Point3, 5>& points,
    const std::array<double, 5>& weights) noexcept {
    const double maximum_weight =
        *std::max_element(weights.begin(), weights.end());

    std::array<Homogeneous3, 5> result{};
    for (std::size_t index = 0; index < result.size(); ++index) {
        const long double weight =
            static_cast<long double>(weights[index]) /
            static_cast<long double>(maximum_weight);
        result[index] = {
            static_cast<long double>(points[index].x()) * weight,
            static_cast<long double>(points[index].y()) * weight,
            static_cast<long double>(points[index].z()) * weight,
            weight,
        };
    }
    return result;
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
    const auto d1 = dehomogenize_first(value, first);
    if (!d1.has_value() || !std::isfinite(value.w) || value.w <= 0.0L) {
        return std::unexpected{CurveError::non_finite_result};
    }

    const long double cx = value.x / value.w;
    const long double cy = value.y / value.w;
    const long double d1x = static_cast<long double>(d1->x());
    const long double d1y = static_cast<long double>(d1->y());

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
    const auto d1 = dehomogenize_first(value, first);
    if (!d1.has_value() || !std::isfinite(value.w) || value.w <= 0.0L) {
        return std::unexpected{CurveError::non_finite_result};
    }

    const long double cx = value.x / value.w;
    const long double cy = value.y / value.w;
    const long double cz = value.z / value.w;
    const long double d1x = static_cast<long double>(d1->x());
    const long double d1y = static_cast<long double>(d1->y());
    const long double d1z = static_cast<long double>(d1->z());

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

template <typename Point, typename Homogeneous>
[[nodiscard]] std::expected<Homogeneous, CurveError> evaluate_homogeneous(
    const std::array<Point, 5>& points,
    const std::array<double, 5>& weights,
    const double lower,
    const double interior,
    const double upper,
    const double parameter) noexcept {
    const auto domain = CurveParameterDomain::make(lower, upper);
    const auto valid = validate_parameter(*domain, parameter);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }

    const auto controls = homogeneous_controls(points, weights);
    const auto knots = full_knots(lower, interior, upper);
    const int span = parameter < interior ? 3 : 4;
    return de_boor(controls, knots, 3, span, parameter);
}

template <typename Point, typename Homogeneous>
[[nodiscard]] std::expected<
    std::array<Homogeneous, 3>,
    CurveError>
evaluate_homogeneous_jet(
    const std::array<Point, 5>& points,
    const std::array<double, 5>& weights,
    const double lower,
    const double interior,
    const double upper,
    const double parameter) noexcept {
    const auto domain = CurveParameterDomain::make(lower, upper);
    const auto valid = validate_parameter(*domain, parameter);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }

    const auto controls = homogeneous_controls(points, weights);
    const auto knots = full_knots(lower, interior, upper);
    const int span0 = parameter < interior ? 3 : 4;
    const auto value = de_boor(controls, knots, 3, span0, parameter);
    if (!value.has_value()) {
        return std::unexpected{value.error()};
    }

    const auto first_controls = derive_controls(controls, knots, 3);
    if (!first_controls.has_value()) {
        return std::unexpected{first_controls.error()};
    }
    const auto first_knots =
        first_derivative_knots(lower, interior, upper);
    const int span1 = parameter < interior ? 2 : 3;
    const auto first =
        de_boor(*first_controls, first_knots, 2, span1, parameter);
    if (!first.has_value()) {
        return std::unexpected{first.error()};
    }

    const auto second_controls =
        derive_controls(*first_controls, first_knots, 2);
    if (!second_controls.has_value()) {
        return std::unexpected{second_controls.error()};
    }
    const auto second_knots =
        second_derivative_knots(lower, interior, upper);
    const int span2 = parameter < interior ? 1 : 2;
    const auto second =
        de_boor(*second_controls, second_knots, 1, span2, parameter);
    if (!second.has_value()) {
        return std::unexpected{second.error()};
    }

    return std::array<Homogeneous, 3>{*value, *first, *second};
}

[[nodiscard]] bool valid_knot_order(
    const double lower,
    const double interior,
    const double upper) noexcept {
    return lower < interior && interior < upper;
}

[[nodiscard]] std::expected<void, NURBSConstructionError> validate_weights(
    const std::array<double, 5>& weights) noexcept {
    for (const double weight : weights) {
        if (!std::isfinite(weight)) {
            return std::unexpected{NURBSConstructionError::non_finite_weight};
        }
        if (weight <= 0.0) {
            return std::unexpected{NURBSConstructionError::non_positive_weight};
        }
    }
    return {};
}

} // namespace

std::expected<TwoSpanCubicNURBS2, NURBSConstructionError>
TwoSpanCubicNURBS2::make(
    const std::array<Point2, 5>& control_points,
    const std::array<double, 5>& weights,
    const double lower_knot,
    const double interior_knot,
    const double upper_knot) noexcept {
    if (!std::isfinite(lower_knot)) {
        return std::unexpected{
            NURBSConstructionError::non_finite_lower_knot};
    }
    if (!std::isfinite(interior_knot)) {
        return std::unexpected{
            NURBSConstructionError::non_finite_interior_knot};
    }
    if (!std::isfinite(upper_knot)) {
        return std::unexpected{
            NURBSConstructionError::non_finite_upper_knot};
    }
    if (!valid_knot_order(lower_knot, interior_knot, upper_knot)) {
        return std::unexpected{
            NURBSConstructionError::non_strict_knot_order};
    }
    const auto valid = validate_weights(weights);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }
    return TwoSpanCubicNURBS2{
        control_points, weights, lower_knot, interior_knot, upper_knot};
}

const std::array<Point2, 5>&
TwoSpanCubicNURBS2::control_points() const noexcept {
    return control_points_;
}

const std::array<double, 5>& TwoSpanCubicNURBS2::weights() const noexcept {
    return weights_;
}

std::array<double, 9> TwoSpanCubicNURBS2::knots() const noexcept {
    return full_knots(lower_knot_, interior_knot_, upper_knot_);
}

double TwoSpanCubicNURBS2::lower_knot() const noexcept {
    return lower_knot_;
}

double TwoSpanCubicNURBS2::interior_knot() const noexcept {
    return interior_knot_;
}

double TwoSpanCubicNURBS2::upper_knot() const noexcept {
    return upper_knot_;
}

CurveParameterDomain TwoSpanCubicNURBS2::parameter_domain() const noexcept {
    return *CurveParameterDomain::make(lower_knot_, upper_knot_);
}

std::expected<Point2, CurveError> TwoSpanCubicNURBS2::evaluate(
    const double parameter) const noexcept {
    const auto valid = validate_parameter(parameter_domain(), parameter);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }
    if (parameter == lower_knot_) {
        return control_points_[0];
    }
    if (parameter == upper_knot_) {
        return control_points_[4];
    }
    if (constant_points(control_points_)) {
        return control_points_[0];
    }

    const auto value = evaluate_homogeneous<Point2, Homogeneous2>(
        control_points_,
        weights_,
        lower_knot_,
        interior_knot_,
        upper_knot_,
        parameter);
    if (!value.has_value()) {
        return std::unexpected{value.error()};
    }
    return dehomogenize_point(*value);
}

std::expected<Vector2, CurveError>
TwoSpanCubicNURBS2::first_derivative(
    const double parameter) const noexcept {
    const auto valid = validate_parameter(parameter_domain(), parameter);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }
    if (constant_points(control_points_)) {
        return *Vector2::make(0.0, 0.0);
    }

    const auto jet = evaluate_homogeneous_jet<Point2, Homogeneous2>(
        control_points_,
        weights_,
        lower_knot_,
        interior_knot_,
        upper_knot_,
        parameter);
    if (!jet.has_value()) {
        return std::unexpected{jet.error()};
    }
    return dehomogenize_first((*jet)[0], (*jet)[1]);
}

std::expected<Vector2, CurveError>
TwoSpanCubicNURBS2::second_derivative(
    const double parameter) const noexcept {
    const auto valid = validate_parameter(parameter_domain(), parameter);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }
    if (constant_points(control_points_)) {
        return *Vector2::make(0.0, 0.0);
    }

    const auto jet = evaluate_homogeneous_jet<Point2, Homogeneous2>(
        control_points_,
        weights_,
        lower_knot_,
        interior_knot_,
        upper_knot_,
        parameter);
    if (!jet.has_value()) {
        return std::unexpected{jet.error()};
    }
    return dehomogenize_second((*jet)[0], (*jet)[1], (*jet)[2]);
}

TwoSpanCubicNURBS2 TwoSpanCubicNURBS2::reversed() const noexcept {
    const auto reversed_knot =
        reversed_parameter(parameter_domain(), interior_knot_);
    return TwoSpanCubicNURBS2{
        {control_points_[4],
         control_points_[3],
         control_points_[2],
         control_points_[1],
         control_points_[0]},
        {weights_[4], weights_[3], weights_[2], weights_[1], weights_[0]},
        lower_knot_,
        *reversed_knot,
        upper_knot_};
}

std::expected<TwoSpanCubicNURBS3, NURBSConstructionError>
TwoSpanCubicNURBS3::make(
    const std::array<Point3, 5>& control_points,
    const std::array<double, 5>& weights,
    const double lower_knot,
    const double interior_knot,
    const double upper_knot) noexcept {
    if (!std::isfinite(lower_knot)) {
        return std::unexpected{
            NURBSConstructionError::non_finite_lower_knot};
    }
    if (!std::isfinite(interior_knot)) {
        return std::unexpected{
            NURBSConstructionError::non_finite_interior_knot};
    }
    if (!std::isfinite(upper_knot)) {
        return std::unexpected{
            NURBSConstructionError::non_finite_upper_knot};
    }
    if (!valid_knot_order(lower_knot, interior_knot, upper_knot)) {
        return std::unexpected{
            NURBSConstructionError::non_strict_knot_order};
    }
    const auto valid = validate_weights(weights);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }
    return TwoSpanCubicNURBS3{
        control_points, weights, lower_knot, interior_knot, upper_knot};
}

const std::array<Point3, 5>&
TwoSpanCubicNURBS3::control_points() const noexcept {
    return control_points_;
}

const std::array<double, 5>& TwoSpanCubicNURBS3::weights() const noexcept {
    return weights_;
}

std::array<double, 9> TwoSpanCubicNURBS3::knots() const noexcept {
    return full_knots(lower_knot_, interior_knot_, upper_knot_);
}

double TwoSpanCubicNURBS3::lower_knot() const noexcept {
    return lower_knot_;
}

double TwoSpanCubicNURBS3::interior_knot() const noexcept {
    return interior_knot_;
}

double TwoSpanCubicNURBS3::upper_knot() const noexcept {
    return upper_knot_;
}

CurveParameterDomain TwoSpanCubicNURBS3::parameter_domain() const noexcept {
    return *CurveParameterDomain::make(lower_knot_, upper_knot_);
}

std::expected<Point3, CurveError> TwoSpanCubicNURBS3::evaluate(
    const double parameter) const noexcept {
    const auto valid = validate_parameter(parameter_domain(), parameter);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }
    if (parameter == lower_knot_) {
        return control_points_[0];
    }
    if (parameter == upper_knot_) {
        return control_points_[4];
    }
    if (constant_points(control_points_)) {
        return control_points_[0];
    }

    const auto value = evaluate_homogeneous<Point3, Homogeneous3>(
        control_points_,
        weights_,
        lower_knot_,
        interior_knot_,
        upper_knot_,
        parameter);
    if (!value.has_value()) {
        return std::unexpected{value.error()};
    }
    return dehomogenize_point(*value);
}

std::expected<Vector3, CurveError>
TwoSpanCubicNURBS3::first_derivative(
    const double parameter) const noexcept {
    const auto valid = validate_parameter(parameter_domain(), parameter);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }
    if (constant_points(control_points_)) {
        return *Vector3::make(0.0, 0.0, 0.0);
    }

    const auto jet = evaluate_homogeneous_jet<Point3, Homogeneous3>(
        control_points_,
        weights_,
        lower_knot_,
        interior_knot_,
        upper_knot_,
        parameter);
    if (!jet.has_value()) {
        return std::unexpected{jet.error()};
    }
    return dehomogenize_first((*jet)[0], (*jet)[1]);
}

std::expected<Vector3, CurveError>
TwoSpanCubicNURBS3::second_derivative(
    const double parameter) const noexcept {
    const auto valid = validate_parameter(parameter_domain(), parameter);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }
    if (constant_points(control_points_)) {
        return *Vector3::make(0.0, 0.0, 0.0);
    }

    const auto jet = evaluate_homogeneous_jet<Point3, Homogeneous3>(
        control_points_,
        weights_,
        lower_knot_,
        interior_knot_,
        upper_knot_,
        parameter);
    if (!jet.has_value()) {
        return std::unexpected{jet.error()};
    }
    return dehomogenize_second((*jet)[0], (*jet)[1], (*jet)[2]);
}

TwoSpanCubicNURBS3 TwoSpanCubicNURBS3::reversed() const noexcept {
    const auto reversed_knot =
        reversed_parameter(parameter_domain(), interior_knot_);
    return TwoSpanCubicNURBS3{
        {control_points_[4],
         control_points_[3],
         control_points_[2],
         control_points_[1],
         control_points_[0]},
        {weights_[4], weights_[3], weights_[2], weights_[1], weights_[0]},
        lower_knot_,
        *reversed_knot,
        upper_knot_};
}

} // namespace apmesh::core
