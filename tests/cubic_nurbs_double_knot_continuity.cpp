#include "apmesh/geometry/nurbs.hpp"

#include "apmesh/core/numeric.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <expected>
#include <limits>
#include <span>
#include <string_view>
#include <utility>
#include <vector>

namespace {

bool require(const bool condition, const std::string_view message) {
    if (!condition) {
        std::fputs(message.data(), stderr);
        std::fputc('\n', stderr);
    }
    return condition;
}

bool close_scalar(
    const double lhs,
    const double rhs,
    const double scale = 1.0,
    const double absolute = 6.0e-12,
    const double relative = 6.0e-12) {
    const apmesh::core::ProximityPolicy policy{
        .absolute_tolerance = absolute * scale,
        .relative_tolerance = relative,
        .reference_scale = scale,
    };
    const auto comparison =
        apmesh::core::compare_proximity(lhs, rhs, policy);
    return comparison.has_value() &&
           comparison->result == apmesh::core::ProximityResult::within;
}

bool close_point(
    const apmesh::core::Point2& lhs,
    const apmesh::core::Point2& rhs,
    const double scale = 1.0) {
    return close_scalar(lhs.x(), rhs.x(), scale) &&
           close_scalar(lhs.y(), rhs.y(), scale);
}

bool close_vector(
    const apmesh::core::Vector2& lhs,
    const apmesh::core::Vector2& rhs,
    const double scale = 1.0) {
    return close_scalar(lhs.x(), rhs.x(), scale) &&
           close_scalar(lhs.y(), rhs.y(), scale);
}

std::vector<double> full_knots(
    const std::span<const double> interior_knots,
    const std::span<const std::uint8_t> multiplicities,
    const double lower,
    const double upper) {
    std::size_t interior_count = 0U;
    for (const std::uint8_t multiplicity : multiplicities) {
        interior_count += static_cast<std::size_t>(multiplicity);
    }

    std::vector<double> knots;
    knots.reserve(interior_count + 8U);
    for (int index = 0; index < 4; ++index) {
        knots.push_back(lower);
    }
    for (std::size_t index = 0; index < interior_knots.size(); ++index) {
        for (std::uint8_t repeat = 0U;
             repeat < multiplicities[index];
             ++repeat) {
            knots.push_back(interior_knots[index]);
        }
    }
    for (int index = 0; index < 4; ++index) {
        knots.push_back(upper);
    }
    return knots;
}

long double basis(
    const std::span<const double> knots,
    const int index,
    const int degree,
    const long double parameter) {
    if (degree == 0) {
        const long double lower =
            static_cast<long double>(knots[static_cast<std::size_t>(index)]);
        const long double upper = static_cast<long double>(
            knots[static_cast<std::size_t>(index + 1)]);
        return lower <= parameter && parameter < upper ? 1.0L : 0.0L;
    }

    long double value = 0.0L;
    const long double left_denominator =
        static_cast<long double>(
            knots[static_cast<std::size_t>(index + degree)]) -
        static_cast<long double>(
            knots[static_cast<std::size_t>(index)]);
    if (left_denominator != 0.0L) {
        value +=
            (parameter -
             static_cast<long double>(
                 knots[static_cast<std::size_t>(index)])) /
            left_denominator *
            basis(knots, index, degree - 1, parameter);
    }

    const long double right_denominator =
        static_cast<long double>(
            knots[static_cast<std::size_t>(index + degree + 1)]) -
        static_cast<long double>(
            knots[static_cast<std::size_t>(index + 1)]);
    if (right_denominator != 0.0L) {
        value +=
            (static_cast<long double>(
                 knots[static_cast<std::size_t>(index + degree + 1)]) -
             parameter) /
            right_denominator *
            basis(knots, index + 1, degree - 1, parameter);
    }
    return value;
}

long double basis_first(
    const std::span<const double> knots,
    const int index,
    const int degree,
    const long double parameter) {
    if (degree == 0) {
        return 0.0L;
    }

    long double value = 0.0L;
    const long double left_denominator =
        static_cast<long double>(
            knots[static_cast<std::size_t>(index + degree)]) -
        static_cast<long double>(
            knots[static_cast<std::size_t>(index)]);
    if (left_denominator != 0.0L) {
        value += static_cast<long double>(degree) /
                 left_denominator *
                 basis(knots, index, degree - 1, parameter);
    }

    const long double right_denominator =
        static_cast<long double>(
            knots[static_cast<std::size_t>(index + degree + 1)]) -
        static_cast<long double>(
            knots[static_cast<std::size_t>(index + 1)]);
    if (right_denominator != 0.0L) {
        value -= static_cast<long double>(degree) /
                 right_denominator *
                 basis(knots, index + 1, degree - 1, parameter);
    }
    return value;
}

long double basis_second(
    const std::span<const double> knots,
    const int index,
    const int degree,
    const long double parameter) {
    if (degree <= 1) {
        return 0.0L;
    }

    long double value = 0.0L;
    const long double left_denominator =
        static_cast<long double>(
            knots[static_cast<std::size_t>(index + degree)]) -
        static_cast<long double>(
            knots[static_cast<std::size_t>(index)]);
    if (left_denominator != 0.0L) {
        value += static_cast<long double>(degree) /
                 left_denominator *
                 basis_first(knots, index, degree - 1, parameter);
    }

    const long double right_denominator =
        static_cast<long double>(
            knots[static_cast<std::size_t>(index + degree + 1)]) -
        static_cast<long double>(
            knots[static_cast<std::size_t>(index + 1)]);
    if (right_denominator != 0.0L) {
        value -= static_cast<long double>(degree) /
                 right_denominator *
                 basis_first(knots, index + 1, degree - 1, parameter);
    }
    return value;
}

struct ReferenceJet2 {
    long double x{};
    long double y{};
    long double dx{};
    long double dy{};
    long double ddx{};
    long double ddy{};
};

ReferenceJet2 reference_jet(
    const std::span<const apmesh::core::Point2> points,
    const std::span<const double> weights,
    const std::span<const double> flat_knots,
    const double parameter) {
    const long double u = static_cast<long double>(parameter);

    long double ax = 0.0L;
    long double ay = 0.0L;
    long double w = 0.0L;
    long double dax = 0.0L;
    long double day = 0.0L;
    long double dw = 0.0L;
    long double ddax = 0.0L;
    long double dday = 0.0L;
    long double ddw = 0.0L;

    for (std::size_t index = 0; index < points.size(); ++index) {
        const int i = static_cast<int>(index);
        const long double n = basis(flat_knots, i, 3, u);
        const long double dn = basis_first(flat_knots, i, 3, u);
        const long double ddn = basis_second(flat_knots, i, 3, u);
        const long double weight =
            static_cast<long double>(weights[index]);

        ax += n * weight * static_cast<long double>(points[index].x());
        ay += n * weight * static_cast<long double>(points[index].y());
        w += n * weight;

        dax += dn * weight * static_cast<long double>(points[index].x());
        day += dn * weight * static_cast<long double>(points[index].y());
        dw += dn * weight;

        ddax += ddn * weight * static_cast<long double>(points[index].x());
        dday += ddn * weight * static_cast<long double>(points[index].y());
        ddw += ddn * weight;
    }

    const long double x = ax / w;
    const long double y = ay / w;
    const long double dx = (dax - x * dw) / w;
    const long double dy = (day - y * dw) / w;
    const long double ddx =
        (ddax - 2.0L * dx * dw - x * ddw) / w;
    const long double ddy =
        (dday - 2.0L * dy * dw - y * ddw) / w;
    return {x, y, dx, dy, ddx, ddy};
}

struct H2 {
    long double x{};
    long double y{};
    long double w{};
};

H2 homogeneous(
    const apmesh::core::Point2& point,
    const double weight) {
    const long double w = static_cast<long double>(weight);
    return {
        static_cast<long double>(point.x()) * w,
        static_cast<long double>(point.y()) * w,
        w,
    };
}

H2 blend(const H2& lhs, const H2& rhs, const long double alpha) {
    return {
        (1.0L - alpha) * lhs.x + alpha * rhs.x,
        (1.0L - alpha) * lhs.y + alpha * rhs.y,
        (1.0L - alpha) * lhs.w + alpha * rhs.w,
    };
}

struct InsertedNURBS2 {
    std::vector<apmesh::core::Point2> points;
    std::vector<double> weights;
};

std::expected<InsertedNURBS2, apmesh::core::GeometryError>
insert_existing_simple_knot(
    const std::span<const apmesh::core::Point2> points,
    const std::span<const double> weights,
    const std::span<const double> flat_knots,
    const double knot) {
    constexpr int degree = 3;
    constexpr int multiplicity = 1;

    const int last_control = static_cast<int>(points.size()) - 1;
    const auto upper =
        std::upper_bound(flat_knots.begin(), flat_knots.end(), knot);
    const int span = static_cast<int>(upper - flat_knots.begin()) - 1;

    std::vector<H2> source;
    source.reserve(points.size());
    for (std::size_t index = 0; index < points.size(); ++index) {
        source.push_back(homogeneous(points[index], weights[index]));
    }

    std::vector<H2> result(points.size() + 1U);

    for (int index = 0; index <= span - degree; ++index) {
        result[static_cast<std::size_t>(index)] =
            source[static_cast<std::size_t>(index)];
    }
    for (int index = span - multiplicity; index <= last_control; ++index) {
        result[static_cast<std::size_t>(index + 1)] =
            source[static_cast<std::size_t>(index)];
    }
    for (int index = span - degree + 1;
         index <= span - multiplicity;
         ++index) {
        const long double numerator =
            static_cast<long double>(knot) -
            static_cast<long double>(
                flat_knots[static_cast<std::size_t>(index)]);
        const long double denominator =
            static_cast<long double>(
                flat_knots[static_cast<std::size_t>(index + degree)]) -
            static_cast<long double>(
                flat_knots[static_cast<std::size_t>(index)]);
        const long double alpha = numerator / denominator;
        result[static_cast<std::size_t>(index)] = blend(
            source[static_cast<std::size_t>(index - 1)],
            source[static_cast<std::size_t>(index)],
            alpha);
    }

    InsertedNURBS2 inserted;
    inserted.points.reserve(result.size());
    inserted.weights.reserve(result.size());
    for (const H2& value : result) {
        const auto point = apmesh::core::Point2::make(
            static_cast<double>(value.x / value.w),
            static_cast<double>(value.y / value.w));
        if (!point.has_value()) {
            return std::unexpected{point.error()};
        }
        inserted.points.push_back(*point);
        inserted.weights.push_back(static_cast<double>(value.w));
    }
    return inserted;
}

} // namespace

int main() {
    using apmesh::core::BoundedParametricCurve2;
    using apmesh::core::BoundedParametricCurve3;
    using apmesh::core::CurveError;
    using apmesh::core::MultiSpanCubicNURBS2;
    using apmesh::core::MultiSpanCubicNURBS3;
    using apmesh::core::MultiSpanNURBSConstructionError;
    using apmesh::core::Point2;
    using apmesh::core::Point3;
    using apmesh::core::reversed_parameter;

    static_assert(BoundedParametricCurve2<MultiSpanCubicNURBS2>);
    static_assert(BoundedParametricCurve3<MultiSpanCubicNURBS3>);
    static_assert(std::same_as<
        decltype(
            std::declval<const MultiSpanCubicNURBS2&>().
                interior_multiplicities()),
        std::span<const std::uint8_t>>);

    bool passed = true;

    const auto p0 = Point2::make(-3.0, 0.0);
    const auto p1 = Point2::make(-2.0, 4.0);
    const auto p2 = Point2::make(-0.5, -3.0);
    const auto p3 = Point2::make(1.0, 5.0);
    const auto p4 = Point2::make(3.0, -2.0);
    const auto p5 = Point2::make(5.0, 4.0);
    const auto p6 = Point2::make(7.0, 0.5);
    if (!p0 || !p1 || !p2 || !p3 || !p4 || !p5 || !p6) {
        return 1;
    }

    const std::vector<Point2> points{
        *p0, *p1, *p2, *p3, *p4, *p5, *p6};
    const std::vector<double> weights{
        1.0, 2.0, 0.5, 3.0, 1.25, 2.5, 0.75};
    const std::vector<double> knots{-0.5, 1.75};
    const std::vector<std::uint8_t> multiplicities{2U, 1U};

    const auto curve = MultiSpanCubicNURBS2::make(
        points, weights, knots, multiplicities, -3.0, 4.0);
    if (!curve) {
        return 1;
    }

    passed = require(
                 curve->span_count() == 3U &&
                     curve->interior_knots().size() == 2U &&
                     curve->interior_multiplicities().size() == 2U &&
                     curve->interior_multiplicities()[0] == 2U &&
                     curve->interior_multiplicities()[1] == 1U,
                 "double-knot storage/span semantics differ") &&
             passed;

    const auto multiplicity_count_mismatch =
        MultiSpanCubicNURBS2::make(
            points, weights, knots, std::vector<std::uint8_t>{2U}, -3.0, 4.0);
    const auto unsupported_multiplicity =
        MultiSpanCubicNURBS2::make(
            points,
            weights,
            knots,
            std::vector<std::uint8_t>{3U, 1U},
            -3.0,
            4.0);
    const auto control_count_mismatch =
        MultiSpanCubicNURBS2::make(
            std::vector<Point2>(points.begin(), points.end() - 1),
            std::vector<double>(weights.begin(), weights.end() - 1),
            knots,
            multiplicities,
            -3.0,
            4.0);

    passed = require(
                 !multiplicity_count_mismatch &&
                     multiplicity_count_mismatch.error() ==
                         MultiSpanNURBSConstructionError::
                             interior_multiplicity_count_mismatch &&
                     !unsupported_multiplicity &&
                     unsupported_multiplicity.error() ==
                         MultiSpanNURBSConstructionError::
                             unsupported_interior_multiplicity &&
                     !control_count_mismatch &&
                     control_count_mismatch.error() ==
                         MultiSpanNURBSConstructionError::
                             interior_knot_count_mismatch,
                 "double-knot construction validation differs") &&
             passed;

    const auto flat = full_knots(
        knots, multiplicities, -3.0, 4.0);
    constexpr std::array<double, 5> interior_samples{
        -2.0, -1.0, 0.5, 1.75, 3.0};
    for (const double parameter : interior_samples) {
        const auto reference =
            reference_jet(points, weights, flat, parameter);
        const auto value = curve->evaluate(parameter);
        const auto d1 = curve->first_derivative(parameter);
        const auto d2 = curve->second_derivative(parameter);

        const bool is_double = parameter == knots[0];
        passed = require(
                     value &&
                         close_scalar(
                             value->x(),
                             static_cast<double>(reference.x),
                             128.0) &&
                         close_scalar(
                             value->y(),
                             static_cast<double>(reference.y),
                             128.0) &&
                         d1 &&
                         close_scalar(
                             d1->x(),
                             static_cast<double>(reference.dx),
                             512.0) &&
                         close_scalar(
                             d1->y(),
                             static_cast<double>(reference.dy),
                             512.0) &&
                         (is_double
                              ? (!d2 &&
                                 d2.error() ==
                                     CurveError::insufficient_continuity)
                              : (d2 &&
                                 close_scalar(
                                     d2->x(),
                                     static_cast<double>(reference.ddx),
                                     2048.0) &&
                                 close_scalar(
                                     d2->y(),
                                     static_cast<double>(reference.ddy),
                                     2048.0))),
                     "double-knot independent rational-basis contract differs") &&
                 passed;
    }

    const double double_knot = knots[0];
    const double left_parameter =
        std::nextafter(
            double_knot, -std::numeric_limits<double>::infinity());
    const double right_parameter =
        std::nextafter(
            double_knot, std::numeric_limits<double>::infinity());

    const auto left_reference =
        reference_jet(points, weights, flat, left_parameter);
    const auto right_reference =
        reference_jet(points, weights, flat, right_parameter);

    const double second_jump = std::hypot(
        static_cast<double>(left_reference.ddx - right_reference.ddx),
        static_cast<double>(left_reference.ddy - right_reference.ddy));

    const auto left_value = curve->evaluate(left_parameter);
    const auto right_value = curve->evaluate(right_parameter);
    const auto left_d1 = curve->first_derivative(left_parameter);
    const auto right_d1 = curve->first_derivative(right_parameter);
    const auto left_d2 = curve->second_derivative(left_parameter);
    const auto right_d2 = curve->second_derivative(right_parameter);

    passed = require(
                 left_value && right_value &&
                     close_point(*left_value, *right_value, 512.0) &&
                     left_d1 && right_d1 &&
                     close_vector(*left_d1, *right_d1, 2048.0) &&
                     left_d2 && right_d2 &&
                     second_jump > 1.0e-6 &&
                     close_scalar(
                         left_d2->x(),
                         static_cast<double>(left_reference.ddx),
                         4096.0) &&
                     close_scalar(
                         left_d2->y(),
                         static_cast<double>(left_reference.ddy),
                         4096.0) &&
                     close_scalar(
                         right_d2->x(),
                         static_cast<double>(right_reference.ddx),
                         4096.0) &&
                     close_scalar(
                         right_d2->y(),
                         static_cast<double>(right_reference.ddy),
                         4096.0),
                 "C1 fixture does not expose distinct one-sided D2") &&
             passed;

    const auto exact_value = curve->evaluate(double_knot);
    const auto exact_d1 = curve->first_derivative(double_knot);
    const auto exact_d2 = curve->second_derivative(double_knot);
    passed = require(
                 exact_value && exact_d1 && !exact_d2 &&
                     exact_d2.error() ==
                         CurveError::insufficient_continuity,
                 "ordinary D2 did not fail exactly at double knot") &&
             passed;

    const auto simple_d2 = curve->second_derivative(knots[1]);
    passed = require(
                 simple_d2.has_value(),
                 "ordinary D2 failed at simple knot") &&
             passed;

    const std::vector<Point2> simple_points{
        points[0], points[1], points[2], points[3], points[4], points[5]};
    const std::vector<double> simple_weights{
        weights[0], weights[1], weights[2], weights[3], weights[4], weights[5]};
    const std::vector<std::uint8_t> simple_multiplicities{1U, 1U};

    const auto legacy = MultiSpanCubicNURBS2::make(
        simple_points, simple_weights, knots, -3.0, 4.0);
    const auto explicit_simple = MultiSpanCubicNURBS2::make(
        simple_points,
        simple_weights,
        knots,
        simple_multiplicities,
        -3.0,
        4.0);
    if (!legacy || !explicit_simple) {
        return 1;
    }

    passed = require(
                 *legacy == *explicit_simple &&
                     legacy->interior_multiplicities()[0] == 1U &&
                     legacy->interior_multiplicities()[1] == 1U,
                 "legacy simple-knot construction changed") &&
             passed;

    const auto simple_flat = full_knots(
        knots, simple_multiplicities, -3.0, 4.0);
    const auto inserted = insert_existing_simple_knot(
        simple_points,
        simple_weights,
        simple_flat,
        knots[0]);
    if (!inserted) {
        return 1;
    }

    const auto inserted_curve = MultiSpanCubicNURBS2::make(
        inserted->points,
        inserted->weights,
        knots,
        multiplicities,
        -3.0,
        4.0);
    if (!inserted_curve) {
        return 1;
    }

    constexpr std::array<double, 7> insertion_samples{
        -2.0, -1.0, -0.5, 0.0, 1.0, 1.75, 3.0};
    for (const double parameter : insertion_samples) {
        const auto base_value = legacy->evaluate(parameter);
        const auto inserted_value = inserted_curve->evaluate(parameter);
        const auto base_d1 = legacy->first_derivative(parameter);
        const auto inserted_d1 = inserted_curve->first_derivative(parameter);
        passed = require(
                     base_value && inserted_value &&
                         close_point(*base_value, *inserted_value, 256.0) &&
                         base_d1 && inserted_d1 &&
                         close_vector(*base_d1, *inserted_d1, 1024.0),
                     "repeated-knot insertion changed value/D1 geometry") &&
                 passed;

        const auto base_d2 = legacy->second_derivative(parameter);
        const auto inserted_d2 = inserted_curve->second_derivative(parameter);
        if (parameter == knots[0]) {
            passed = require(
                         base_d2 && !inserted_d2 &&
                             inserted_d2.error() ==
                                 CurveError::insufficient_continuity,
                         "inserted double knot did not enforce representation D2 failure") &&
                     passed;
        } else {
            passed = require(
                         base_d2 && inserted_d2 &&
                             close_vector(*base_d2, *inserted_d2, 4096.0),
                         "repeated-knot insertion changed off-knot D2 geometry") &&
                     passed;
        }
    }

    const auto reversed = curve->reversed();
    const auto double_reversed = reversed.reversed();
    passed = require(
                 double_reversed == *curve &&
                     reversed.interior_multiplicities().size() == 2U &&
                     reversed.interior_multiplicities()[0] == 1U &&
                     reversed.interior_multiplicities()[1] == 2U,
                 "double-knot reversal storage/involution differs") &&
             passed;

    const auto reflected_double =
        reversed_parameter(curve->parameter_domain(), double_knot);
    if (!reflected_double) {
        return 1;
    }
    const auto reversed_d2 =
        reversed.second_derivative(*reflected_double);
    passed = require(
                 !reversed_d2 &&
                     reversed_d2.error() ==
                         CurveError::insufficient_continuity,
                 "reversal did not preserve double-knot D2 failure") &&
             passed;

    constexpr double reversal_parameter = 0.5;
    const auto mapped =
        reversed_parameter(curve->parameter_domain(), reversal_parameter);
    if (!mapped) {
        return 1;
    }
    const auto original_value = curve->evaluate(reversal_parameter);
    const auto reverse_value = reversed.evaluate(*mapped);
    const auto original_d1 = curve->first_derivative(reversal_parameter);
    const auto reverse_d1 = reversed.first_derivative(*mapped);
    const auto original_d2 = curve->second_derivative(reversal_parameter);
    const auto reverse_d2 = reversed.second_derivative(*mapped);
    passed = require(
                 original_value && reverse_value &&
                     close_point(*original_value, *reverse_value, 256.0) &&
                     original_d1 && reverse_d1 &&
                     close_scalar(
                         reverse_d1->x(), -original_d1->x(), 1024.0) &&
                     close_scalar(
                         reverse_d1->y(), -original_d1->y(), 1024.0) &&
                     original_d2 && reverse_d2 &&
                     close_vector(*original_d2, *reverse_d2, 4096.0),
                 "double-knot reversal covariance differs") &&
             passed;

    std::vector<Point3> points3;
    points3.reserve(points.size());
    for (const auto& point : points) {
        const auto embedded = Point3::make(point.x(), point.y(), 0.0);
        if (!embedded) {
            return 1;
        }
        points3.push_back(*embedded);
    }
    const auto curve3 = MultiSpanCubicNURBS3::make(
        points3, weights, knots, multiplicities, -3.0, 4.0);
    if (!curve3) {
        return 1;
    }

    const auto value2 = curve->evaluate(0.5);
    const auto value3 = curve3->evaluate(0.5);
    const auto d12 = curve->first_derivative(0.5);
    const auto d13 = curve3->first_derivative(0.5);
    const auto d22 = curve->second_derivative(0.5);
    const auto d23 = curve3->second_derivative(0.5);
    const auto d23_double = curve3->second_derivative(double_knot);
    passed = require(
                 value2 && value3 &&
                     close_scalar(value2->x(), value3->x(), 256.0) &&
                     close_scalar(value2->y(), value3->y(), 256.0) &&
                     value3->z() == 0.0 &&
                     d12 && d13 &&
                     close_scalar(d12->x(), d13->x(), 1024.0) &&
                     close_scalar(d12->y(), d13->y(), 1024.0) &&
                     d13->z() == 0.0 &&
                     d22 && d23 &&
                     close_scalar(d22->x(), d23->x(), 4096.0) &&
                     close_scalar(d22->y(), d23->y(), 4096.0) &&
                     d23->z() == 0.0 &&
                     !d23_double &&
                     d23_double.error() ==
                         CurveError::insufficient_continuity,
                 "double-knot 2D/3D embedding semantics differ") &&
             passed;

    const auto constant_point = Point2::make(3.0, -4.0);
    if (!constant_point) {
        return 1;
    }
    const std::vector<Point2> constant_points(points.size(), *constant_point);
    const auto constant_curve = MultiSpanCubicNURBS2::make(
        constant_points,
        weights,
        knots,
        multiplicities,
        -3.0,
        4.0);
    if (!constant_curve) {
        return 1;
    }
    const auto constant_value = constant_curve->evaluate(double_knot);
    const auto constant_d1 = constant_curve->first_derivative(double_knot);
    const auto constant_d2 = constant_curve->second_derivative(double_knot);
    passed = require(
                 constant_value && *constant_value == *constant_point &&
                     constant_d1 && constant_d1->x() == 0.0 &&
                     constant_d1->y() == 0.0 &&
                     !constant_d2 &&
                     constant_d2.error() ==
                         CurveError::insufficient_continuity,
                 "representation-guaranteed C1 policy was bypassed by constant geometry") &&
             passed;

    const auto repeat_a = curve->second_derivative(double_knot);
    const auto repeat_b = curve->second_derivative(double_knot);
    const auto repeat_value_a = curve->evaluate(double_knot);
    const auto repeat_value_b = curve->evaluate(double_knot);
    passed = require(
                 !repeat_a && !repeat_b &&
                     repeat_a.error() == repeat_b.error() &&
                     repeat_value_a && repeat_value_b &&
                     *repeat_value_a == *repeat_value_b,
                 "double-knot success/failure evidence is not deterministic") &&
             passed;

    return passed ? 0 : 1;
}
