#include "apmesh/geometry/rational_bezier.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <expected>
#include <limits>

namespace apmesh::core {
namespace {

struct RationalBasis {
    std::array<long double, 3> weights{};
    std::array<long double, 3> basis{};
    std::array<long double, 3> first_basis{};
    std::array<long double, 3> second_basis{};
    long double denominator{};
    long double first_denominator{};
    long double second_denominator{};
};

[[nodiscard]] CurveParameterDomain unit_parameter_domain() noexcept {
    const auto domain = CurveParameterDomain::make(0.0, 1.0);
    return *domain;
}

[[nodiscard]] std::expected<void, CurveError> validate_parameter(
    const double parameter) noexcept {
    const auto contained = unit_parameter_domain().contains(parameter);
    if (!contained.has_value()) {
        return std::unexpected{contained.error()};
    }
    if (!*contained) {
        return std::unexpected{CurveError::parameter_out_of_domain};
    }
    return {};
}

[[nodiscard]] bool valid_weight(const double weight) noexcept {
    return std::isfinite(weight) && weight > 0.0;
}

[[nodiscard]] std::expected<RationalBasis, CurveError> make_basis(
    const std::array<double, 3>& weights,
    const double parameter) noexcept {
    const long double t = static_cast<long double>(parameter);
    const long double one_minus_t = 1.0L - t;

    const double maximum_weight =
        std::max(weights[0], std::max(weights[1], weights[2]));
    const long double scale = static_cast<long double>(maximum_weight);

    RationalBasis data{};
    for (std::size_t index = 0; index < 3; ++index) {
        data.weights[index] =
            static_cast<long double>(weights[index]) / scale;
    }

    data.basis = {
        one_minus_t * one_minus_t,
        2.0L * t * one_minus_t,
        t * t,
    };
    data.first_basis = {
        -2.0L * one_minus_t,
        2.0L * (1.0L - 2.0L * t),
        2.0L * t,
    };
    data.second_basis = {2.0L, -4.0L, 2.0L};

    for (std::size_t index = 0; index < 3; ++index) {
        data.denominator += data.weights[index] * data.basis[index];
        data.first_denominator +=
            data.weights[index] * data.first_basis[index];
        data.second_denominator +=
            data.weights[index] * data.second_basis[index];
    }

    if (!std::isfinite(data.denominator) ||
        !std::isfinite(data.first_denominator) ||
        !std::isfinite(data.second_denominator) ||
        data.denominator <= 0.0L) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return data;
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

[[nodiscard]] std::expected<long double, CurveError> rational_value_component(
    const RationalBasis& data,
    const std::array<double, 3>& coordinates) noexcept {
    long double numerator = 0.0L;
    for (std::size_t index = 0; index < 3; ++index) {
        numerator +=
            data.weights[index] * data.basis[index] *
            static_cast<long double>(coordinates[index]);
    }

    const long double value = numerator / data.denominator;
    if (!std::isfinite(value)) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return value;
}

[[nodiscard]] std::expected<long double, CurveError>
rational_first_derivative_component(
    const RationalBasis& data,
    const std::array<double, 3>& coordinates) noexcept {
    const auto value = rational_value_component(data, coordinates);
    if (!value.has_value()) {
        return std::unexpected{value.error()};
    }

    long double first_numerator = 0.0L;
    for (std::size_t index = 0; index < 3; ++index) {
        first_numerator +=
            data.weights[index] * data.first_basis[index] *
            static_cast<long double>(coordinates[index]);
    }

    const long double derivative =
        (first_numerator - *value * data.first_denominator) /
        data.denominator;
    if (!std::isfinite(derivative)) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return derivative;
}

[[nodiscard]] std::expected<long double, CurveError>
rational_second_derivative_component(
    const RationalBasis& data,
    const std::array<double, 3>& coordinates) noexcept {
    const auto value = rational_value_component(data, coordinates);
    if (!value.has_value()) {
        return std::unexpected{value.error()};
    }
    const auto first =
        rational_first_derivative_component(data, coordinates);
    if (!first.has_value()) {
        return std::unexpected{first.error()};
    }

    long double second_numerator = 0.0L;
    for (std::size_t index = 0; index < 3; ++index) {
        second_numerator +=
            data.weights[index] * data.second_basis[index] *
            static_cast<long double>(coordinates[index]);
    }

    const long double derivative =
        (second_numerator -
         2.0L * *first * data.first_denominator -
         *value * data.second_denominator) /
        data.denominator;
    if (!std::isfinite(derivative)) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return derivative;
}

[[nodiscard]] std::array<double, 3> x_coordinates(
    const std::array<Point2, 3>& points) noexcept {
    return {points[0].x(), points[1].x(), points[2].x()};
}

[[nodiscard]] std::array<double, 3> y_coordinates(
    const std::array<Point2, 3>& points) noexcept {
    return {points[0].y(), points[1].y(), points[2].y()};
}

[[nodiscard]] std::array<double, 3> x_coordinates(
    const std::array<Point3, 3>& points) noexcept {
    return {points[0].x(), points[1].x(), points[2].x()};
}

[[nodiscard]] std::array<double, 3> y_coordinates(
    const std::array<Point3, 3>& points) noexcept {
    return {points[0].y(), points[1].y(), points[2].y()};
}

[[nodiscard]] std::array<double, 3> z_coordinates(
    const std::array<Point3, 3>& points) noexcept {
    return {points[0].z(), points[1].z(), points[2].z()};
}

[[nodiscard]] std::expected<Point2, CurveError> evaluate(
    const std::array<Point2, 3>& points,
    const std::array<double, 3>& weights,
    const double parameter) noexcept {
    const auto valid = validate_parameter(parameter);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }
    if (parameter == 0.0) {
        return points[0];
    }
    if (parameter == 1.0) {
        return points[2];
    }
    if (points[0] == points[1] && points[1] == points[2]) {
        return points[0];
    }

    const auto data = make_basis(weights, parameter);
    if (!data.has_value()) {
        return std::unexpected{data.error()};
    }
    const auto x = rational_value_component(*data, x_coordinates(points));
    const auto y = rational_value_component(*data, y_coordinates(points));
    if (!x || !y) {
        return std::unexpected{CurveError::non_finite_result};
    }
    const auto dx = to_double(*x);
    const auto dy = to_double(*y);
    if (!dx || !dy) {
        return std::unexpected{CurveError::non_finite_result};
    }

    const auto point = Point2::make(*dx, *dy);
    if (!point.has_value()) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return *point;
}

[[nodiscard]] std::expected<Point3, CurveError> evaluate(
    const std::array<Point3, 3>& points,
    const std::array<double, 3>& weights,
    const double parameter) noexcept {
    const auto valid = validate_parameter(parameter);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }
    if (parameter == 0.0) {
        return points[0];
    }
    if (parameter == 1.0) {
        return points[2];
    }
    if (points[0] == points[1] && points[1] == points[2]) {
        return points[0];
    }

    const auto data = make_basis(weights, parameter);
    if (!data.has_value()) {
        return std::unexpected{data.error()};
    }
    const auto x = rational_value_component(*data, x_coordinates(points));
    const auto y = rational_value_component(*data, y_coordinates(points));
    const auto z = rational_value_component(*data, z_coordinates(points));
    if (!x || !y || !z) {
        return std::unexpected{CurveError::non_finite_result};
    }
    const auto dx = to_double(*x);
    const auto dy = to_double(*y);
    const auto dz = to_double(*z);
    if (!dx || !dy || !dz) {
        return std::unexpected{CurveError::non_finite_result};
    }

    const auto point = Point3::make(*dx, *dy, *dz);
    if (!point.has_value()) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return *point;
}

[[nodiscard]] bool constant(
    const std::array<Point2, 3>& points) noexcept {
    return points[0] == points[1] && points[1] == points[2];
}

[[nodiscard]] bool constant(
    const std::array<Point3, 3>& points) noexcept {
    return points[0] == points[1] && points[1] == points[2];
}

[[nodiscard]] std::expected<Vector2, CurveError> first_derivative(
    const std::array<Point2, 3>& points,
    const std::array<double, 3>& weights,
    const double parameter) noexcept {
    const auto valid = validate_parameter(parameter);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }
    if (constant(points)) {
        return *Vector2::make(0.0, 0.0);
    }

    const auto data = make_basis(weights, parameter);
    if (!data.has_value()) {
        return std::unexpected{data.error()};
    }
    const auto x =
        rational_first_derivative_component(*data, x_coordinates(points));
    const auto y =
        rational_first_derivative_component(*data, y_coordinates(points));
    if (!x || !y) {
        return std::unexpected{CurveError::non_finite_result};
    }
    const auto dx = to_double(*x);
    const auto dy = to_double(*y);
    if (!dx || !dy) {
        return std::unexpected{CurveError::non_finite_result};
    }

    const auto vector = Vector2::make(*dx, *dy);
    if (!vector.has_value()) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return *vector;
}

[[nodiscard]] std::expected<Vector3, CurveError> first_derivative(
    const std::array<Point3, 3>& points,
    const std::array<double, 3>& weights,
    const double parameter) noexcept {
    const auto valid = validate_parameter(parameter);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }
    if (constant(points)) {
        return *Vector3::make(0.0, 0.0, 0.0);
    }

    const auto data = make_basis(weights, parameter);
    if (!data.has_value()) {
        return std::unexpected{data.error()};
    }
    const auto x =
        rational_first_derivative_component(*data, x_coordinates(points));
    const auto y =
        rational_first_derivative_component(*data, y_coordinates(points));
    const auto z =
        rational_first_derivative_component(*data, z_coordinates(points));
    if (!x || !y || !z) {
        return std::unexpected{CurveError::non_finite_result};
    }
    const auto dx = to_double(*x);
    const auto dy = to_double(*y);
    const auto dz = to_double(*z);
    if (!dx || !dy || !dz) {
        return std::unexpected{CurveError::non_finite_result};
    }

    const auto vector = Vector3::make(*dx, *dy, *dz);
    if (!vector.has_value()) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return *vector;
}

[[nodiscard]] std::expected<Vector2, CurveError> second_derivative(
    const std::array<Point2, 3>& points,
    const std::array<double, 3>& weights,
    const double parameter) noexcept {
    const auto valid = validate_parameter(parameter);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }
    if (constant(points)) {
        return *Vector2::make(0.0, 0.0);
    }

    const auto data = make_basis(weights, parameter);
    if (!data.has_value()) {
        return std::unexpected{data.error()};
    }
    const auto x =
        rational_second_derivative_component(*data, x_coordinates(points));
    const auto y =
        rational_second_derivative_component(*data, y_coordinates(points));
    if (!x || !y) {
        return std::unexpected{CurveError::non_finite_result};
    }
    const auto dx = to_double(*x);
    const auto dy = to_double(*y);
    if (!dx || !dy) {
        return std::unexpected{CurveError::non_finite_result};
    }

    const auto vector = Vector2::make(*dx, *dy);
    if (!vector.has_value()) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return *vector;
}

[[nodiscard]] std::expected<Vector3, CurveError> second_derivative(
    const std::array<Point3, 3>& points,
    const std::array<double, 3>& weights,
    const double parameter) noexcept {
    const auto valid = validate_parameter(parameter);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }
    if (constant(points)) {
        return *Vector3::make(0.0, 0.0, 0.0);
    }

    const auto data = make_basis(weights, parameter);
    if (!data.has_value()) {
        return std::unexpected{data.error()};
    }
    const auto x =
        rational_second_derivative_component(*data, x_coordinates(points));
    const auto y =
        rational_second_derivative_component(*data, y_coordinates(points));
    const auto z =
        rational_second_derivative_component(*data, z_coordinates(points));
    if (!x || !y || !z) {
        return std::unexpected{CurveError::non_finite_result};
    }
    const auto dx = to_double(*x);
    const auto dy = to_double(*y);
    const auto dz = to_double(*z);
    if (!dx || !dy || !dz) {
        return std::unexpected{CurveError::non_finite_result};
    }

    const auto vector = Vector3::make(*dx, *dy, *dz);
    if (!vector.has_value()) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return *vector;
}

} // namespace

std::expected<RationalQuadraticBezier2, RationalBezierConstructionError>
RationalQuadraticBezier2::make(
    const Point2& p0,
    const Point2& p1,
    const Point2& p2,
    const double w0,
    const double w1,
    const double w2) noexcept {
    const std::array<double, 3> weights{w0, w1, w2};
    for (const double weight : weights) {
        if (!std::isfinite(weight)) {
            return std::unexpected{
                RationalBezierConstructionError::non_finite_weight};
        }
        if (!valid_weight(weight)) {
            return std::unexpected{
                RationalBezierConstructionError::non_positive_weight};
        }
    }
    return RationalQuadraticBezier2{{p0, p1, p2}, weights};
}

const std::array<Point2, 3>&
RationalQuadraticBezier2::control_points() const noexcept {
    return control_points_;
}

const std::array<double, 3>&
RationalQuadraticBezier2::weights() const noexcept {
    return weights_;
}

CurveParameterDomain
RationalQuadraticBezier2::parameter_domain() const noexcept {
    return unit_parameter_domain();
}

std::expected<Point2, CurveError> RationalQuadraticBezier2::evaluate(
    const double parameter) const noexcept {
    return apmesh::core::evaluate(control_points_, weights_, parameter);
}

std::expected<Vector2, CurveError>
RationalQuadraticBezier2::first_derivative(
    const double parameter) const noexcept {
    return apmesh::core::first_derivative(
        control_points_, weights_, parameter);
}

std::expected<Vector2, CurveError>
RationalQuadraticBezier2::second_derivative(
    const double parameter) const noexcept {
    return apmesh::core::second_derivative(
        control_points_, weights_, parameter);
}

RationalQuadraticBezier2 RationalQuadraticBezier2::reversed() const noexcept {
    return RationalQuadraticBezier2{
        {control_points_[2], control_points_[1], control_points_[0]},
        {weights_[2], weights_[1], weights_[0]}};
}

std::expected<RationalQuadraticBezier3, RationalBezierConstructionError>
RationalQuadraticBezier3::make(
    const Point3& p0,
    const Point3& p1,
    const Point3& p2,
    const double w0,
    const double w1,
    const double w2) noexcept {
    const std::array<double, 3> weights{w0, w1, w2};
    for (const double weight : weights) {
        if (!std::isfinite(weight)) {
            return std::unexpected{
                RationalBezierConstructionError::non_finite_weight};
        }
        if (!valid_weight(weight)) {
            return std::unexpected{
                RationalBezierConstructionError::non_positive_weight};
        }
    }
    return RationalQuadraticBezier3{{p0, p1, p2}, weights};
}

const std::array<Point3, 3>&
RationalQuadraticBezier3::control_points() const noexcept {
    return control_points_;
}

const std::array<double, 3>&
RationalQuadraticBezier3::weights() const noexcept {
    return weights_;
}

CurveParameterDomain
RationalQuadraticBezier3::parameter_domain() const noexcept {
    return unit_parameter_domain();
}

std::expected<Point3, CurveError> RationalQuadraticBezier3::evaluate(
    const double parameter) const noexcept {
    return apmesh::core::evaluate(control_points_, weights_, parameter);
}

std::expected<Vector3, CurveError>
RationalQuadraticBezier3::first_derivative(
    const double parameter) const noexcept {
    return apmesh::core::first_derivative(
        control_points_, weights_, parameter);
}

std::expected<Vector3, CurveError>
RationalQuadraticBezier3::second_derivative(
    const double parameter) const noexcept {
    return apmesh::core::second_derivative(
        control_points_, weights_, parameter);
}

RationalQuadraticBezier3 RationalQuadraticBezier3::reversed() const noexcept {
    return RationalQuadraticBezier3{
        {control_points_[2], control_points_[1], control_points_[0]},
        {weights_[2], weights_[1], weights_[0]}};
}

} // namespace apmesh::core
