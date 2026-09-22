#include "apmesh/geometry/bspline.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <expected>
#include <limits>

namespace apmesh::core {
namespace {

struct Wide2 {
    long double x{};
    long double y{};
};

struct Wide3 {
    long double x{};
    long double y{};
    long double z{};
};

[[nodiscard]] Wide2 to_wide(const Point2& point) noexcept {
    return {
        static_cast<long double>(point.x()),
        static_cast<long double>(point.y()),
    };
}

[[nodiscard]] Wide3 to_wide(const Point3& point) noexcept {
    return {
        static_cast<long double>(point.x()),
        static_cast<long double>(point.y()),
        static_cast<long double>(point.z()),
    };
}

[[nodiscard]] Wide2 lerp_wide(
    const Wide2& lhs,
    const Wide2& rhs,
    const long double parameter) noexcept {
    return {
        std::lerp(lhs.x, rhs.x, parameter),
        std::lerp(lhs.y, rhs.y, parameter),
    };
}

[[nodiscard]] Wide3 lerp_wide(
    const Wide3& lhs,
    const Wide3& rhs,
    const long double parameter) noexcept {
    return {
        std::lerp(lhs.x, rhs.x, parameter),
        std::lerp(lhs.y, rhs.y, parameter),
        std::lerp(lhs.z, rhs.z, parameter),
    };
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

[[nodiscard]] std::expected<Point2, CurveError> to_point(
    const Wide2& point) noexcept {
    const auto x = to_double(point.x);
    const auto y = to_double(point.y);
    if (!x || !y) {
        return std::unexpected{CurveError::non_finite_result};
    }

    const auto result = Point2::make(*x, *y);
    if (!result) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return *result;
}

[[nodiscard]] std::expected<Point3, CurveError> to_point(
    const Wide3& point) noexcept {
    const auto x = to_double(point.x);
    const auto y = to_double(point.y);
    const auto z = to_double(point.z);
    if (!x || !y || !z) {
        return std::unexpected{CurveError::non_finite_result};
    }

    const auto result = Point3::make(*x, *y, *z);
    if (!result) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return *result;
}

[[nodiscard]] std::expected<Vector2, CurveError> to_vector(
    const Wide2& vector) noexcept {
    const auto x = to_double(vector.x);
    const auto y = to_double(vector.y);
    if (!x || !y) {
        return std::unexpected{CurveError::non_finite_result};
    }

    const auto result = Vector2::make(*x, *y);
    if (!result) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return *result;
}

[[nodiscard]] std::expected<Vector3, CurveError> to_vector(
    const Wide3& vector) noexcept {
    const auto x = to_double(vector.x);
    const auto y = to_double(vector.y);
    const auto z = to_double(vector.z);
    if (!x || !y || !z) {
        return std::unexpected{CurveError::non_finite_result};
    }

    const auto result = Vector3::make(*x, *y, *z);
    if (!result) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return *result;
}

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

template <typename Wide, std::size_t ControlCount, std::size_t KnotCount>
[[nodiscard]] std::expected<Wide, CurveError> de_boor(
    const std::array<Wide, ControlCount>& controls,
    const std::array<double, KnotCount>& knots,
    const int degree,
    const int span,
    const double parameter) noexcept {
    std::array<Wide, 4> work{};

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
            if (!alpha) {
                return std::unexpected{alpha.error()};
            }

            work[static_cast<std::size_t>(local)] = lerp_wide(
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

[[nodiscard]] std::expected<Wide2, CurveError> derivative_control(
    const Wide2& current,
    const Wide2& next,
    const int degree,
    const double lower_knot,
    const double upper_knot) noexcept {
    const auto x = scaled_difference(
        next.x, current.x, degree, lower_knot, upper_knot);
    const auto y = scaled_difference(
        next.y, current.y, degree, lower_knot, upper_knot);
    if (!x || !y) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return Wide2{*x, *y};
}

[[nodiscard]] std::expected<Wide3, CurveError> derivative_control(
    const Wide3& current,
    const Wide3& next,
    const int degree,
    const double lower_knot,
    const double upper_knot) noexcept {
    const auto x = scaled_difference(
        next.x, current.x, degree, lower_knot, upper_knot);
    const auto y = scaled_difference(
        next.y, current.y, degree, lower_knot, upper_knot);
    const auto z = scaled_difference(
        next.z, current.z, degree, lower_knot, upper_knot);
    if (!x || !y || !z) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return Wide3{*x, *y, *z};
}

template <typename Wide, std::size_t ControlCount, std::size_t KnotCount>
[[nodiscard]] std::expected<std::array<Wide, ControlCount - 1>, CurveError>
derive_controls(
    const std::array<Wide, ControlCount>& controls,
    const std::array<double, KnotCount>& knots,
    const int degree) noexcept {
    std::array<Wide, ControlCount - 1> derived{};

    for (std::size_t index = 0; index + 1 < ControlCount; ++index) {
        const auto value = derivative_control(
            controls[index],
            controls[index + 1],
            degree,
            knots[index + 1],
            knots[index + static_cast<std::size_t>(degree) + 1]);
        if (!value) {
            return std::unexpected{value.error()};
        }
        derived[index] = *value;
    }

    return derived;
}

template <typename Point, typename Wide, std::size_t ControlCount>
[[nodiscard]] std::array<Wide, ControlCount> widen(
    const std::array<Point, ControlCount>& controls) noexcept {
    std::array<Wide, ControlCount> result{};
    for (std::size_t index = 0; index < ControlCount; ++index) {
        result[index] = to_wide(controls[index]);
    }
    return result;
}

[[nodiscard]] std::expected<void, CurveError> validate_parameter(
    const CurveParameterDomain& domain,
    const double parameter) noexcept {
    const auto contained = domain.contains(parameter);
    if (!contained) {
        return std::unexpected{contained.error()};
    }
    if (!*contained) {
        return std::unexpected{CurveError::parameter_out_of_domain};
    }
    return {};
}

template <typename Point, typename Wide>
[[nodiscard]] std::expected<Point, CurveError> evaluate_impl(
    const std::array<Point, 5>& controls,
    const double lower,
    const double interior,
    const double upper,
    const double parameter) noexcept {
    const auto domain = CurveParameterDomain::make(lower, upper);
    const auto valid = validate_parameter(*domain, parameter);
    if (!valid) {
        return std::unexpected{valid.error()};
    }

    if (parameter == lower) {
        return controls[0];
    }
    if (parameter == upper) {
        return controls[4];
    }

    const auto wide_controls = widen<Point, Wide>(controls);
    const auto knots = full_knots(lower, interior, upper);
    const int span = parameter < interior ? 3 : 4;
    const auto wide_value =
        de_boor(wide_controls, knots, 3, span, parameter);
    if (!wide_value) {
        return std::unexpected{wide_value.error()};
    }
    return to_point(*wide_value);
}

template <typename Point, typename Wide, typename Vector>
[[nodiscard]] std::expected<Vector, CurveError> first_derivative_impl(
    const std::array<Point, 5>& controls,
    const double lower,
    const double interior,
    const double upper,
    const double parameter) noexcept {
    const auto domain = CurveParameterDomain::make(lower, upper);
    const auto valid = validate_parameter(*domain, parameter);
    if (!valid) {
        return std::unexpected{valid.error()};
    }

    const auto wide_controls = widen<Point, Wide>(controls);
    const auto knots = full_knots(lower, interior, upper);
    const auto first_controls = derive_controls(wide_controls, knots, 3);
    if (!first_controls) {
        return std::unexpected{first_controls.error()};
    }

    if (parameter == lower) {
        return to_vector((*first_controls)[0]);
    }
    if (parameter == upper) {
        return to_vector((*first_controls)[3]);
    }

    const auto derivative_knots =
        first_derivative_knots(lower, interior, upper);
    const int span = parameter < interior ? 2 : 3;
    const auto wide_value =
        de_boor(*first_controls, derivative_knots, 2, span, parameter);
    if (!wide_value) {
        return std::unexpected{wide_value.error()};
    }
    return to_vector(*wide_value);
}

template <typename Point, typename Wide, typename Vector>
[[nodiscard]] std::expected<Vector, CurveError> second_derivative_impl(
    const std::array<Point, 5>& controls,
    const double lower,
    const double interior,
    const double upper,
    const double parameter) noexcept {
    const auto domain = CurveParameterDomain::make(lower, upper);
    const auto valid = validate_parameter(*domain, parameter);
    if (!valid) {
        return std::unexpected{valid.error()};
    }

    const auto wide_controls = widen<Point, Wide>(controls);
    const auto knots = full_knots(lower, interior, upper);
    const auto first_controls = derive_controls(wide_controls, knots, 3);
    if (!first_controls) {
        return std::unexpected{first_controls.error()};
    }

    const auto derivative_knots =
        first_derivative_knots(lower, interior, upper);
    const auto second_controls =
        derive_controls(*first_controls, derivative_knots, 2);
    if (!second_controls) {
        return std::unexpected{second_controls.error()};
    }

    if (parameter == lower) {
        return to_vector((*second_controls)[0]);
    }
    if (parameter == upper) {
        return to_vector((*second_controls)[2]);
    }

    const auto second_knots =
        second_derivative_knots(lower, interior, upper);
    const int span = parameter < interior ? 1 : 2;
    const auto wide_value =
        de_boor(*second_controls, second_knots, 1, span, parameter);
    if (!wide_value) {
        return std::unexpected{wide_value.error()};
    }
    return to_vector(*wide_value);
}

[[nodiscard]] bool valid_knot_order(
    const double lower,
    const double interior,
    const double upper) noexcept {
    return lower < interior && interior < upper;
}

} // namespace

std::expected<TwoSpanCubicBSpline2, BSplineConstructionError>
TwoSpanCubicBSpline2::make(
    const std::array<Point2, 5>& control_points,
    const double lower_knot,
    const double interior_knot,
    const double upper_knot) noexcept {
    if (!std::isfinite(lower_knot)) {
        return std::unexpected{
            BSplineConstructionError::non_finite_lower_knot};
    }
    if (!std::isfinite(interior_knot)) {
        return std::unexpected{
            BSplineConstructionError::non_finite_interior_knot};
    }
    if (!std::isfinite(upper_knot)) {
        return std::unexpected{
            BSplineConstructionError::non_finite_upper_knot};
    }
    if (!valid_knot_order(lower_knot, interior_knot, upper_knot)) {
        return std::unexpected{
            BSplineConstructionError::non_strict_knot_order};
    }

    return TwoSpanCubicBSpline2{
        control_points, lower_knot, interior_knot, upper_knot};
}

const std::array<Point2, 5>&
TwoSpanCubicBSpline2::control_points() const noexcept {
    return control_points_;
}

std::array<double, 9> TwoSpanCubicBSpline2::knots() const noexcept {
    return full_knots(lower_knot_, interior_knot_, upper_knot_);
}

double TwoSpanCubicBSpline2::lower_knot() const noexcept {
    return lower_knot_;
}

double TwoSpanCubicBSpline2::interior_knot() const noexcept {
    return interior_knot_;
}

double TwoSpanCubicBSpline2::upper_knot() const noexcept {
    return upper_knot_;
}

CurveParameterDomain
TwoSpanCubicBSpline2::parameter_domain() const noexcept {
    return *CurveParameterDomain::make(lower_knot_, upper_knot_);
}

std::expected<Point2, CurveError> TwoSpanCubicBSpline2::evaluate(
    const double parameter) const noexcept {
    return evaluate_impl<Point2, Wide2>(
        control_points_,
        lower_knot_,
        interior_knot_,
        upper_knot_,
        parameter);
}

std::expected<Vector2, CurveError>
TwoSpanCubicBSpline2::first_derivative(
    const double parameter) const noexcept {
    return first_derivative_impl<Point2, Wide2, Vector2>(
        control_points_,
        lower_knot_,
        interior_knot_,
        upper_knot_,
        parameter);
}

std::expected<Vector2, CurveError>
TwoSpanCubicBSpline2::second_derivative(
    const double parameter) const noexcept {
    return second_derivative_impl<Point2, Wide2, Vector2>(
        control_points_,
        lower_knot_,
        interior_knot_,
        upper_knot_,
        parameter);
}

TwoSpanCubicBSpline2 TwoSpanCubicBSpline2::reversed() const noexcept {
    const auto reversed_knot =
        reversed_parameter(parameter_domain(), interior_knot_);
    return TwoSpanCubicBSpline2{
        {control_points_[4],
         control_points_[3],
         control_points_[2],
         control_points_[1],
         control_points_[0]},
        lower_knot_,
        *reversed_knot,
        upper_knot_};
}

std::expected<TwoSpanCubicBSpline3, BSplineConstructionError>
TwoSpanCubicBSpline3::make(
    const std::array<Point3, 5>& control_points,
    const double lower_knot,
    const double interior_knot,
    const double upper_knot) noexcept {
    if (!std::isfinite(lower_knot)) {
        return std::unexpected{
            BSplineConstructionError::non_finite_lower_knot};
    }
    if (!std::isfinite(interior_knot)) {
        return std::unexpected{
            BSplineConstructionError::non_finite_interior_knot};
    }
    if (!std::isfinite(upper_knot)) {
        return std::unexpected{
            BSplineConstructionError::non_finite_upper_knot};
    }
    if (!valid_knot_order(lower_knot, interior_knot, upper_knot)) {
        return std::unexpected{
            BSplineConstructionError::non_strict_knot_order};
    }

    return TwoSpanCubicBSpline3{
        control_points, lower_knot, interior_knot, upper_knot};
}

const std::array<Point3, 5>&
TwoSpanCubicBSpline3::control_points() const noexcept {
    return control_points_;
}

std::array<double, 9> TwoSpanCubicBSpline3::knots() const noexcept {
    return full_knots(lower_knot_, interior_knot_, upper_knot_);
}

double TwoSpanCubicBSpline3::lower_knot() const noexcept {
    return lower_knot_;
}

double TwoSpanCubicBSpline3::interior_knot() const noexcept {
    return interior_knot_;
}

double TwoSpanCubicBSpline3::upper_knot() const noexcept {
    return upper_knot_;
}

CurveParameterDomain
TwoSpanCubicBSpline3::parameter_domain() const noexcept {
    return *CurveParameterDomain::make(lower_knot_, upper_knot_);
}

std::expected<Point3, CurveError> TwoSpanCubicBSpline3::evaluate(
    const double parameter) const noexcept {
    return evaluate_impl<Point3, Wide3>(
        control_points_,
        lower_knot_,
        interior_knot_,
        upper_knot_,
        parameter);
}

std::expected<Vector3, CurveError>
TwoSpanCubicBSpline3::first_derivative(
    const double parameter) const noexcept {
    return first_derivative_impl<Point3, Wide3, Vector3>(
        control_points_,
        lower_knot_,
        interior_knot_,
        upper_knot_,
        parameter);
}

std::expected<Vector3, CurveError>
TwoSpanCubicBSpline3::second_derivative(
    const double parameter) const noexcept {
    return second_derivative_impl<Point3, Wide3, Vector3>(
        control_points_,
        lower_knot_,
        interior_knot_,
        upper_knot_,
        parameter);
}

TwoSpanCubicBSpline3 TwoSpanCubicBSpline3::reversed() const noexcept {
    const auto reversed_knot =
        reversed_parameter(parameter_domain(), interior_knot_);
    return TwoSpanCubicBSpline3{
        {control_points_[4],
         control_points_[3],
         control_points_[2],
         control_points_[1],
         control_points_[0]},
        lower_knot_,
        *reversed_knot,
        upper_knot_};
}

} // namespace apmesh::core
