#include "apmesh/geometry/nurbs.hpp"

#include "apmesh/core/numeric.hpp"
#include "apmesh/geometry/bspline.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <limits>
#include <span>
#include <string_view>
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
    const double absolute = 4.0e-12,
    const double relative = 4.0e-12) {
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

bool close_point(
    const apmesh::core::Point3& lhs,
    const apmesh::core::Point3& rhs,
    const double scale = 1.0) {
    return close_scalar(lhs.x(), rhs.x(), scale) &&
           close_scalar(lhs.y(), rhs.y(), scale) &&
           close_scalar(lhs.z(), rhs.z(), scale);
}

bool close_vector(
    const apmesh::core::Vector2& lhs,
    const apmesh::core::Vector2& rhs,
    const double scale = 1.0) {
    return close_scalar(lhs.x(), rhs.x(), scale) &&
           close_scalar(lhs.y(), rhs.y(), scale);
}

bool close_vector(
    const apmesh::core::Vector3& lhs,
    const apmesh::core::Vector3& rhs,
    const double scale = 1.0) {
    return close_scalar(lhs.x(), rhs.x(), scale) &&
           close_scalar(lhs.y(), rhs.y(), scale) &&
           close_scalar(lhs.z(), rhs.z(), scale);
}

std::vector<double> full_knots(
    const std::span<const double> interior,
    const double lower,
    const double upper) {
    std::vector<double> knots;
    knots.reserve(interior.size() + 8U);
    for (int index = 0; index < 4; ++index) {
        knots.push_back(lower);
    }
    knots.insert(knots.end(), interior.begin(), interior.end());
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
        const long double upper =
            static_cast<long double>(
                knots[static_cast<std::size_t>(index + 1)]);
        return lower <= parameter && parameter < upper ? 1.0L : 0.0L;
    }

    long double value = 0.0L;
    const long double left_denominator =
        static_cast<long double>(
            knots[static_cast<std::size_t>(index + degree)]) -
        static_cast<long double>(knots[static_cast<std::size_t>(index)]);
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
        static_cast<long double>(knots[static_cast<std::size_t>(index)]);
    if (left_denominator != 0.0L) {
        value +=
            static_cast<long double>(degree) /
            left_denominator *
            basis(knots, index, degree - 1, parameter);
    }

    const long double right_denominator =
        static_cast<long double>(
            knots[static_cast<std::size_t>(index + degree + 1)]) -
        static_cast<long double>(
            knots[static_cast<std::size_t>(index + 1)]);
    if (right_denominator != 0.0L) {
        value -=
            static_cast<long double>(degree) /
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
        static_cast<long double>(knots[static_cast<std::size_t>(index)]);
    if (left_denominator != 0.0L) {
        value +=
            static_cast<long double>(degree) /
            left_denominator *
            basis_first(knots, index, degree - 1, parameter);
    }

    const long double right_denominator =
        static_cast<long double>(
            knots[static_cast<std::size_t>(index + degree + 1)]) -
        static_cast<long double>(
            knots[static_cast<std::size_t>(index + 1)]);
    if (right_denominator != 0.0L) {
        value -=
            static_cast<long double>(degree) /
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
    const std::span<const double> interior_knots,
    const double lower,
    const double upper,
    const double parameter) {
    const auto knots = full_knots(interior_knots, lower, upper);
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
        const int basis_index = static_cast<int>(index);
        const long double n = basis(knots, basis_index, 3, u);
        const long double dn = basis_first(knots, basis_index, 3, u);
        const long double ddn = basis_second(knots, basis_index, 3, u);
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
    const long double ddx = (ddax - 2.0L * dx * dw - x * ddw) / w;
    const long double ddy = (dday - 2.0L * dy * dw - y * ddw) / w;

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

H2 mix(
    const H2& lhs,
    const H2& rhs,
    const long double alpha) {
    return {
        std::lerp(lhs.x, rhs.x, alpha),
        std::lerp(lhs.y, rhs.y, alpha),
        std::lerp(lhs.w, rhs.w, alpha),
    };
}

struct InsertedNURBS2 {
    std::vector<apmesh::core::Point2> points;
    std::vector<double> weights;
    std::vector<double> interior_knots;
};

std::expected<InsertedNURBS2, apmesh::core::GeometryError>
insert_simple_knot(
    const std::span<const apmesh::core::Point2> points,
    const std::span<const double> weights,
    const std::span<const double> interior_knots,
    const double lower,
    const double upper,
    const double inserted_knot) {
    const auto knots = full_knots(interior_knots, lower, upper);
    const int degree = 3;
    const int last_control = static_cast<int>(points.size()) - 1;
    const auto upper_iterator =
        std::upper_bound(knots.begin(), knots.end(), inserted_knot);
    const int span =
        static_cast<int>(upper_iterator - knots.begin()) - 1;

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
    for (int index = span; index <= last_control; ++index) {
        result[static_cast<std::size_t>(index + 1)] =
            source[static_cast<std::size_t>(index)];
    }
    for (int index = span - degree + 1; index <= span; ++index) {
        const long double numerator =
            static_cast<long double>(inserted_knot) -
            static_cast<long double>(knots[static_cast<std::size_t>(index)]);
        const long double denominator =
            static_cast<long double>(
                knots[static_cast<std::size_t>(index + degree)]) -
            static_cast<long double>(knots[static_cast<std::size_t>(index)]);
        const long double alpha = numerator / denominator;
        result[static_cast<std::size_t>(index)] = mix(
            source[static_cast<std::size_t>(index - 1)],
            source[static_cast<std::size_t>(index)],
            alpha);
    }

    InsertedNURBS2 inserted;
    inserted.points.reserve(result.size());
    inserted.weights.reserve(result.size());
    for (const auto& value : result) {
        const auto point = apmesh::core::Point2::make(
            static_cast<double>(value.x / value.w),
            static_cast<double>(value.y / value.w));
        if (!point.has_value()) {
            return std::unexpected{point.error()};
        }
        inserted.points.push_back(*point);
        inserted.weights.push_back(static_cast<double>(value.w));
    }

    inserted.interior_knots.assign(
        interior_knots.begin(), interior_knots.end());
    inserted.interior_knots.insert(
        std::upper_bound(
            inserted.interior_knots.begin(),
            inserted.interior_knots.end(),
            inserted_knot),
        inserted_knot);
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
    using apmesh::core::TwoSpanCubicBSpline2;
    using apmesh::core::TwoSpanCubicBSpline3;
    using apmesh::core::TwoSpanCubicNURBS2;
    using apmesh::core::TwoSpanCubicNURBS3;
    using apmesh::core::reversed_parameter;

    static_assert(BoundedParametricCurve2<MultiSpanCubicNURBS2>);
    static_assert(BoundedParametricCurve3<MultiSpanCubicNURBS3>);
    static_assert(std::same_as<
        decltype(
            std::declval<const MultiSpanCubicNURBS2&>().control_points()),
        std::span<const Point2>>);
    static_assert(std::same_as<
        decltype(std::declval<const MultiSpanCubicNURBS3&>().weights()),
        std::span<const double>>);

    bool passed = true;

    const auto p0 = Point2::make(-2.0, 1.0);
    const auto p1 = Point2::make(-0.5, 4.0);
    const auto p2 = Point2::make(2.0, -1.0);
    const auto p3 = Point2::make(5.0, 3.0);
    const auto p4 = Point2::make(7.0, 0.5);
    if (!p0 || !p1 || !p2 || !p3 || !p4) {
        return 1;
    }

    const std::array<Point2, 5> fixed_points{
        *p0, *p1, *p2, *p3, *p4};
    const std::array<double, 5> fixed_weights{
        1.0, 2.0, 0.75, 4.0, 1.5};

    const auto fixed2 = TwoSpanCubicNURBS2::make(
        fixed_points, fixed_weights, -2.0, 0.5, 4.0);
    const auto general2 = MultiSpanCubicNURBS2::make(
        std::vector<Point2>(fixed_points.begin(), fixed_points.end()),
        std::vector<double>(fixed_weights.begin(), fixed_weights.end()),
        {0.5},
        -2.0,
        4.0);
    if (!fixed2 || !general2) {
        return 1;
    }

    const auto q0 = Point3::make(-2.0, 1.0, 0.0);
    const auto q1 = Point3::make(-0.5, 4.0, 1.0);
    const auto q2 = Point3::make(2.0, -1.0, 2.0);
    const auto q3 = Point3::make(5.0, 3.0, 3.0);
    const auto q4 = Point3::make(7.0, 0.5, 4.0);
    if (!q0 || !q1 || !q2 || !q3 || !q4) {
        return 1;
    }
    const std::array<Point3, 5> fixed_points3{
        *q0, *q1, *q2, *q3, *q4};
    const auto fixed3 = TwoSpanCubicNURBS3::make(
        fixed_points3, fixed_weights, -2.0, 0.5, 4.0);
    const auto general3 = MultiSpanCubicNURBS3::make(
        std::vector<Point3>(fixed_points3.begin(), fixed_points3.end()),
        std::vector<double>(fixed_weights.begin(), fixed_weights.end()),
        {0.5},
        -2.0,
        4.0);
    if (!fixed3 || !general3) {
        return 1;
    }

    passed = require(
                 general2->span_count() == 2U &&
                     general2->control_points().size() == 5U &&
                     general2->weights().size() == 5U &&
                     general2->interior_knots().size() == 1U &&
                     general2->interior_knots()[0] == 0.5 &&
                     general2->parameter_domain().lower() == -2.0 &&
                     general2->parameter_domain().upper() == 4.0 &&
                     general2->evaluate(-2.0) &&
                     *general2->evaluate(-2.0) == fixed_points.front() &&
                     general2->evaluate(4.0) &&
                     *general2->evaluate(4.0) == fixed_points.back(),
                 "multi-span storage/domain/endpoint semantics differ") &&
             passed;

    const std::vector<Point2> too_few{
        fixed_points[0], fixed_points[1], fixed_points[2], fixed_points[3]};
    const auto insufficient = MultiSpanCubicNURBS2::make(
        too_few, {1.0, 1.0, 1.0, 1.0}, {}, 0.0, 1.0);
    const auto weight_mismatch = MultiSpanCubicNURBS2::make(
        std::vector<Point2>(fixed_points.begin(), fixed_points.end()),
        {1.0, 1.0, 1.0, 1.0},
        {0.5},
        0.0,
        1.0);
    const auto knot_count_mismatch = MultiSpanCubicNURBS2::make(
        std::vector<Point2>(fixed_points.begin(), fixed_points.end()),
        std::vector<double>(fixed_weights.begin(), fixed_weights.end()),
        {},
        0.0,
        1.0);
    const auto repeated_knots = MultiSpanCubicNURBS2::make(
        {fixed_points[0],
         fixed_points[1],
         fixed_points[2],
         fixed_points[3],
         fixed_points[4],
         fixed_points[4]},
        {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
        {0.5, 0.5},
        0.0,
        1.0);
    const auto non_finite_knot = MultiSpanCubicNURBS2::make(
        std::vector<Point2>(fixed_points.begin(), fixed_points.end()),
        std::vector<double>(fixed_weights.begin(), fixed_weights.end()),
        {std::numeric_limits<double>::quiet_NaN()},
        0.0,
        1.0);
    const auto zero_weight = MultiSpanCubicNURBS2::make(
        std::vector<Point2>(fixed_points.begin(), fixed_points.end()),
        {1.0, 1.0, 0.0, 1.0, 1.0},
        {0.5},
        0.0,
        1.0);

    passed = require(
                 !insufficient &&
                     insufficient.error() ==
                         MultiSpanNURBSConstructionError::
                             insufficient_control_points &&
                     !weight_mismatch &&
                     weight_mismatch.error() ==
                         MultiSpanNURBSConstructionError::
                             control_weight_count_mismatch &&
                     !knot_count_mismatch &&
                     knot_count_mismatch.error() ==
                         MultiSpanNURBSConstructionError::
                             interior_knot_count_mismatch &&
                     !repeated_knots &&
                     repeated_knots.error() ==
                         MultiSpanNURBSConstructionError::
                             non_strict_knot_order &&
                     !non_finite_knot &&
                     non_finite_knot.error() ==
                         MultiSpanNURBSConstructionError::
                             non_finite_interior_knot &&
                     !zero_weight &&
                     zero_weight.error() ==
                         MultiSpanNURBSConstructionError::non_positive_weight,
                 "multi-span construction failures differ") &&
             passed;

    const auto nan_query =
        general2->evaluate(std::numeric_limits<double>::quiet_NaN());
    const auto below = general2->first_derivative(-2.1);
    const auto above = general3->second_derivative(4.1);
    passed = require(
                 !nan_query &&
                     nan_query.error() == CurveError::non_finite_parameter &&
                     !below &&
                     below.error() == CurveError::parameter_out_of_domain &&
                     !above &&
                     above.error() == CurveError::parameter_out_of_domain,
                 "multi-span query failure semantics differ") &&
             passed;

    constexpr std::array<double, 5> fixed_parameters{
        -2.0, -0.75, 0.5, 2.25, 4.0};
    for (const double parameter : fixed_parameters) {
        const auto gv = general2->evaluate(parameter);
        const auto fv = fixed2->evaluate(parameter);
        const auto gd1 = general2->first_derivative(parameter);
        const auto fd1 = fixed2->first_derivative(parameter);
        const auto gd2 = general2->second_derivative(parameter);
        const auto fd2 = fixed2->second_derivative(parameter);

        passed = require(
                     gv && fv && close_point(*gv, *fv, 64.0) &&
                         gd1 && fd1 && close_vector(*gd1, *fd1, 256.0) &&
                         gd2 && fd2 && close_vector(*gd2, *fd2, 1024.0),
                     "fixed two-span 2D NURBS parity differs") &&
                 passed;

        const auto gv3 = general3->evaluate(parameter);
        const auto fv3 = fixed3->evaluate(parameter);
        const auto gd13 = general3->first_derivative(parameter);
        const auto fd13 = fixed3->first_derivative(parameter);
        const auto gd23 = general3->second_derivative(parameter);
        const auto fd23 = fixed3->second_derivative(parameter);

        passed = require(
                     gv3 && fv3 && close_point(*gv3, *fv3, 64.0) &&
                         gd13 && fd13 &&
                         close_vector(*gd13, *fd13, 256.0) &&
                         gd23 && fd23 &&
                         close_vector(*gd23, *fd23, 1024.0),
                     "fixed two-span 3D NURBS parity differs") &&
                 passed;
    }

    const std::array<double, 5> equal_weights{4.0, 4.0, 4.0, 4.0, 4.0};
    const auto equal_general2 = MultiSpanCubicNURBS2::make(
        std::vector<Point2>(fixed_points.begin(), fixed_points.end()),
        std::vector<double>(equal_weights.begin(), equal_weights.end()),
        {0.5},
        -2.0,
        4.0);
    const auto polynomial2 = TwoSpanCubicBSpline2::make(
        fixed_points, -2.0, 0.5, 4.0);
    const auto equal_general3 = MultiSpanCubicNURBS3::make(
        std::vector<Point3>(fixed_points3.begin(), fixed_points3.end()),
        std::vector<double>(equal_weights.begin(), equal_weights.end()),
        {0.5},
        -2.0,
        4.0);
    const auto polynomial3 = TwoSpanCubicBSpline3::make(
        fixed_points3, -2.0, 0.5, 4.0);
    if (!equal_general2 || !polynomial2 || !equal_general3 || !polynomial3) {
        return 1;
    }
    for (const double parameter : fixed_parameters) {
        const auto nv = equal_general2->evaluate(parameter);
        const auto bv = polynomial2->evaluate(parameter);
        const auto nd1 = equal_general2->first_derivative(parameter);
        const auto bd1 = polynomial2->first_derivative(parameter);
        const auto nd2 = equal_general2->second_derivative(parameter);
        const auto bd2 = polynomial2->second_derivative(parameter);
        passed = require(
                     nv && bv && close_point(*nv, *bv, 64.0) &&
                         nd1 && bd1 && close_vector(*nd1, *bd1, 256.0) &&
                         nd2 && bd2 && close_vector(*nd2, *bd2, 1024.0),
                     "two-span polynomial subset parity differs") &&
                 passed;

        const auto nv3 = equal_general3->evaluate(parameter);
        const auto bv3 = polynomial3->evaluate(parameter);
        const auto nd13 = equal_general3->first_derivative(parameter);
        const auto bd13 = polynomial3->first_derivative(parameter);
        const auto nd23 = equal_general3->second_derivative(parameter);
        const auto bd23 = polynomial3->second_derivative(parameter);
        passed = require(
                     nv3 && bv3 && close_point(*nv3, *bv3, 64.0) &&
                         nd13 && bd13 &&
                         close_vector(*nd13, *bd13, 256.0) &&
                         nd23 && bd23 &&
                         close_vector(*nd23, *bd23, 1024.0),
                     "two-span 3D polynomial subset parity differs") &&
                 passed;
    }

    const auto m0 = Point2::make(-3.0, 0.5);
    const auto m1 = Point2::make(-1.5, 4.0);
    const auto m2 = Point2::make(0.0, -2.0);
    const auto m3 = Point2::make(2.0, 3.5);
    const auto m4 = Point2::make(4.0, -1.0);
    const auto m5 = Point2::make(6.0, 2.5);
    const auto m6 = Point2::make(8.0, 0.0);
    if (!m0 || !m1 || !m2 || !m3 || !m4 || !m5 || !m6) {
        return 1;
    }
    const std::vector<Point2> multi_points{
        *m0, *m1, *m2, *m3, *m4, *m5, *m6};
    const std::vector<double> multi_weights{
        1.0, 2.0, 0.5, 4.0, 1.25, 3.0, 0.75};
    const std::vector<double> multi_knots{-1.0, 0.75, 3.0};
    const auto multi = MultiSpanCubicNURBS2::make(
        multi_points, multi_weights, multi_knots, -3.0, 5.0);
    if (!multi || multi->span_count() != 4U) {
        return 1;
    }

    constexpr std::array<double, 7> oracle_parameters{
        -2.0, -1.0, 0.0, 0.75, 2.0, 3.0, 4.0};
    for (const double parameter : oracle_parameters) {
        const auto reference = reference_jet(
            multi_points,
            multi_weights,
            multi_knots,
            -3.0,
            5.0,
            parameter);
        const auto value = multi->evaluate(parameter);
        const auto d1 = multi->first_derivative(parameter);
        const auto d2 = multi->second_derivative(parameter);

        passed = require(
                     value && d1 && d2 &&
                         close_scalar(
                             value->x(),
                             static_cast<double>(reference.x),
                             128.0) &&
                         close_scalar(
                             value->y(),
                             static_cast<double>(reference.y),
                             128.0) &&
                         close_scalar(
                             d1->x(),
                             static_cast<double>(reference.dx),
                             512.0) &&
                         close_scalar(
                             d1->y(),
                             static_cast<double>(reference.dy),
                             512.0) &&
                         close_scalar(
                             d2->x(),
                             static_cast<double>(reference.ddx),
                             2048.0) &&
                         close_scalar(
                             d2->y(),
                             static_cast<double>(reference.ddy),
                             2048.0),
                     "independent multi-span rational-basis jet differs") &&
                 passed;
    }

    auto outside_points = multi_points;
    auto outside_weights = multi_weights;
    outside_points[0] = *Point2::make(-100.0, 90.0);
    outside_points[1] = *Point2::make(110.0, -80.0);
    outside_points[6] = *Point2::make(-120.0, 70.0);
    outside_weights[0] = 32.0;
    outside_weights[1] = 64.0;
    outside_weights[6] = 128.0;
    const auto outside_changed = MultiSpanCubicNURBS2::make(
        outside_points, outside_weights, multi_knots, -3.0, 5.0);
    if (!outside_changed) {
        return 1;
    }
    const auto local_value = multi->evaluate(2.0);
    const auto changed_value = outside_changed->evaluate(2.0);
    const auto local_d1 = multi->first_derivative(2.0);
    const auto changed_d1 = outside_changed->first_derivative(2.0);
    const auto local_d2 = multi->second_derivative(2.0);
    const auto changed_d2 = outside_changed->second_derivative(2.0);
    passed = require(
                 local_value && changed_value &&
                     *local_value == *changed_value &&
                     local_d1 && changed_d1 && *local_d1 == *changed_d1 &&
                     local_d2 && changed_d2 && *local_d2 == *changed_d2,
                 "multi-span two-sided local support differs") &&
             passed;

    auto right_knot_points = multi_points;
    auto right_knot_weights = multi_weights;
    right_knot_points[0] = *Point2::make(250.0, -250.0);
    right_knot_weights[0] = 256.0;
    const auto right_knot_changed = MultiSpanCubicNURBS2::make(
        right_knot_points, right_knot_weights, multi_knots, -3.0, 5.0);
    if (!right_knot_changed) {
        return 1;
    }
    const auto knot_value = multi->evaluate(-1.0);
    const auto changed_knot_value = right_knot_changed->evaluate(-1.0);
    const auto knot_d1 = multi->first_derivative(-1.0);
    const auto changed_knot_d1 =
        right_knot_changed->first_derivative(-1.0);
    const auto knot_d2 = multi->second_derivative(-1.0);
    const auto changed_knot_d2 =
        right_knot_changed->second_derivative(-1.0);
    passed = require(
                 knot_value && changed_knot_value &&
                     *knot_value == *changed_knot_value &&
                     knot_d1 && changed_knot_d1 &&
                     *knot_d1 == *changed_knot_d1 &&
                     knot_d2 && changed_knot_d2 &&
                     *knot_d2 == *changed_knot_d2,
                 "right-span interior-knot policy differs") &&
             passed;

    const auto inserted = insert_simple_knot(
        fixed_points, fixed_weights, std::array<double, 1>{0.5},
        -2.0, 4.0, -0.25);
    if (!inserted) {
        return 1;
    }
    const auto inserted_curve = MultiSpanCubicNURBS2::make(
        inserted->points,
        inserted->weights,
        inserted->interior_knots,
        -2.0,
        4.0);
    if (!inserted_curve || inserted_curve->span_count() != 3U) {
        return 1;
    }
    constexpr std::array<double, 7> insertion_parameters{
        -1.5, -0.25, 0.0, 0.5, 1.5, 3.0, 3.75};
    for (const double parameter : insertion_parameters) {
        const auto base_value2 = fixed2->evaluate(parameter);
        const auto inserted_value = inserted_curve->evaluate(parameter);
        const auto base_d1 = fixed2->first_derivative(parameter);
        const auto inserted_d1 = inserted_curve->first_derivative(parameter);
        const auto base_d2 = fixed2->second_derivative(parameter);
        const auto inserted_d2 = inserted_curve->second_derivative(parameter);
        passed = require(
                     base_value2 && inserted_value &&
                         close_point(*base_value2, *inserted_value, 128.0) &&
                         base_d1 && inserted_d1 &&
                         close_vector(*base_d1, *inserted_d1, 512.0) &&
                         base_d2 && inserted_d2 &&
                         close_vector(*base_d2, *inserted_d2, 2048.0),
                     "test-only knot-insertion parity differs") &&
                 passed;
    }

    std::vector<double> scaled_weights = multi_weights;
    for (double& weight : scaled_weights) {
        weight *= 8.0;
    }
    const auto weight_scaled = MultiSpanCubicNURBS2::make(
        multi_points, scaled_weights, multi_knots, -3.0, 5.0);
    if (!weight_scaled) {
        return 1;
    }
    for (const double parameter : oracle_parameters) {
        const auto av = multi->evaluate(parameter);
        const auto bv = weight_scaled->evaluate(parameter);
        const auto ad1 = multi->first_derivative(parameter);
        const auto bd1 = weight_scaled->first_derivative(parameter);
        const auto ad2 = multi->second_derivative(parameter);
        const auto bd2 = weight_scaled->second_derivative(parameter);
        passed = require(
                     av && bv && *av == *bv &&
                         ad1 && bd1 && *ad1 == *bd1 &&
                         ad2 && bd2 && *ad2 == *bd2,
                     "multi-span common weight-scale invariance differs") &&
                 passed;
    }

    const auto reversed = multi->reversed();
    passed = require(
                 reversed.reversed() == *multi &&
                     reversed.control_points().front() ==
                         multi->control_points().back() &&
                     reversed.weights().front() ==
                         multi->weights().back() &&
                     reversed.interior_knots().size() ==
                         multi->interior_knots().size(),
                 "multi-span reversal storage/involution differs") &&
             passed;

    constexpr double reversal_parameter_value = 2.0;
    const auto mapped = reversed_parameter(
        multi->parameter_domain(), reversal_parameter_value);
    if (!mapped) {
        return 1;
    }
    const auto original_value = multi->evaluate(reversal_parameter_value);
    const auto reversed_value = reversed.evaluate(*mapped);
    const auto original_d1 =
        multi->first_derivative(reversal_parameter_value);
    const auto reversed_d1 = reversed.first_derivative(*mapped);
    const auto original_d2 =
        multi->second_derivative(reversal_parameter_value);
    const auto reversed_d2 = reversed.second_derivative(*mapped);
    passed = require(
                 original_value && reversed_value &&
                     close_point(*original_value, *reversed_value, 128.0) &&
                     original_d1 && reversed_d1 &&
                     close_scalar(
                         reversed_d1->x(), -original_d1->x(), 512.0) &&
                     close_scalar(
                         reversed_d1->y(), -original_d1->y(), 512.0) &&
                     original_d2 && reversed_d2 &&
                     close_vector(*original_d2, *reversed_d2, 2048.0),
                 "multi-span reversal covariance differs") &&
             passed;

    const auto constant_point = Point2::make(3.0, -4.0);
    if (!constant_point) {
        return 1;
    }
    const std::vector<Point2> constant_points(7U, *constant_point);
    const auto constant = MultiSpanCubicNURBS2::make(
        constant_points,
        {1.0, 2.0, 8.0, 0.5, 4.0, 3.0, 0.75},
        multi_knots,
        -3.0,
        5.0);
    if (!constant) {
        return 1;
    }
    const auto constant_value = constant->evaluate(1.5);
    const auto constant_d1 = constant->first_derivative(1.5);
    const auto constant_d2 = constant->second_derivative(1.5);
    passed = require(
                 constant_value && *constant_value == *constant_point &&
                     constant_d1 && constant_d1->x() == 0.0 &&
                     constant_d1->y() == 0.0 &&
                     constant_d2 && constant_d2->x() == 0.0 &&
                     constant_d2->y() == 0.0,
                 "multi-span constant-curve semantics differ") &&
             passed;

    std::vector<Point2> translated_points;
    std::vector<Point2> coordinate_scaled_points;
    translated_points.reserve(multi_points.size());
    coordinate_scaled_points.reserve(multi_points.size());
    for (const auto& point : multi_points) {
        const auto translated =
            Point2::make(point.x() + 8.0, point.y() - 6.0);
        const auto scaled =
            Point2::make(2.0 * point.x(), 2.0 * point.y());
        if (!translated || !scaled) {
            return 1;
        }
        translated_points.push_back(*translated);
        coordinate_scaled_points.push_back(*scaled);
    }
    const auto translated = MultiSpanCubicNURBS2::make(
        translated_points, multi_weights, multi_knots, -3.0, 5.0);
    const auto coordinate_scaled = MultiSpanCubicNURBS2::make(
        coordinate_scaled_points, multi_weights, multi_knots, -3.0, 5.0);
    if (!translated || !coordinate_scaled) {
        return 1;
    }

    const auto affine_value = multi->evaluate(1.5);
    const auto translated_value = translated->evaluate(1.5);
    const auto scaled_value = coordinate_scaled->evaluate(1.5);
    const auto affine_d1 = multi->first_derivative(1.5);
    const auto translated_d1 = translated->first_derivative(1.5);
    const auto scaled_d1 = coordinate_scaled->first_derivative(1.5);
    const auto affine_d2 = multi->second_derivative(1.5);
    const auto translated_d2 = translated->second_derivative(1.5);
    const auto scaled_d2 = coordinate_scaled->second_derivative(1.5);
    passed = require(
                 affine_value && translated_value && scaled_value &&
                     close_scalar(
                         translated_value->x() - affine_value->x(),
                         8.0,
                         64.0) &&
                     close_scalar(
                         translated_value->y() - affine_value->y(),
                         -6.0,
                         64.0) &&
                     close_scalar(
                         scaled_value->x(), 2.0 * affine_value->x(), 128.0) &&
                     close_scalar(
                         scaled_value->y(), 2.0 * affine_value->y(), 128.0) &&
                     affine_d1 && translated_d1 && scaled_d1 &&
                     close_vector(*affine_d1, *translated_d1, 512.0) &&
                     close_scalar(
                         scaled_d1->x(), 2.0 * affine_d1->x(), 1024.0) &&
                     close_scalar(
                         scaled_d1->y(), 2.0 * affine_d1->y(), 1024.0) &&
                     affine_d2 && translated_d2 && scaled_d2 &&
                     close_vector(*affine_d2, *translated_d2, 2048.0) &&
                     close_scalar(
                         scaled_d2->x(), 2.0 * affine_d2->x(), 4096.0) &&
                     close_scalar(
                         scaled_d2->y(), 2.0 * affine_d2->y(), 4096.0),
                 "multi-span affine covariance differs") &&
             passed;

    std::vector<Point3> embedded_points;
    embedded_points.reserve(multi_points.size());
    for (const auto& point : multi_points) {
        const auto embedded = Point3::make(point.x(), point.y(), 0.0);
        if (!embedded) {
            return 1;
        }
        embedded_points.push_back(*embedded);
    }
    const auto embedded = MultiSpanCubicNURBS3::make(
        embedded_points, multi_weights, multi_knots, -3.0, 5.0);
    if (!embedded) {
        return 1;
    }
    const auto embed_value2 = multi->evaluate(1.25);
    const auto embed_value3 = embedded->evaluate(1.25);
    const auto embed_d12 = multi->first_derivative(1.25);
    const auto embed_d13 = embedded->first_derivative(1.25);
    const auto embed_d22 = multi->second_derivative(1.25);
    const auto embed_d23 = embedded->second_derivative(1.25);
    passed = require(
                 embed_value2 && embed_value3 &&
                     close_scalar(embed_value2->x(), embed_value3->x(), 128.0) &&
                     close_scalar(embed_value2->y(), embed_value3->y(), 128.0) &&
                     embed_value3->z() == 0.0 &&
                     embed_d12 && embed_d13 &&
                     close_scalar(embed_d12->x(), embed_d13->x(), 512.0) &&
                     close_scalar(embed_d12->y(), embed_d13->y(), 512.0) &&
                     embed_d13->z() == 0.0 &&
                     embed_d22 && embed_d23 &&
                     close_scalar(embed_d22->x(), embed_d23->x(), 2048.0) &&
                     close_scalar(embed_d22->y(), embed_d23->y(), 2048.0) &&
                     embed_d23->z() == 0.0,
                 "multi-span 2D/3D embedding parity differs") &&
             passed;

    const double maximum = std::numeric_limits<double>::max();
    const double minimum = std::numeric_limits<double>::denorm_min();
    const std::vector<double> extreme_knots{
        -maximum / 2.0, 0.0, maximum / 2.0};
    const auto extreme = MultiSpanCubicNURBS2::make(
        multi_points,
        {minimum,
         maximum,
         maximum / 2.0,
         maximum / 4.0,
         maximum / 8.0,
         maximum / 16.0,
         minimum},
        extreme_knots,
        -maximum,
        maximum);
    if (!extreme) {
        return 1;
    }
    const auto extreme_value = extreme->evaluate(maximum * 0.75);
    const auto extreme_d1 = extreme->first_derivative(maximum * 0.75);
    const auto extreme_d2 = extreme->second_derivative(maximum * 0.75);
    passed = require(
                 extreme_value && std::isfinite(extreme_value->x()) &&
                     std::isfinite(extreme_value->y()) &&
                     extreme_d1 && std::isfinite(extreme_d1->x()) &&
                     std::isfinite(extreme_d1->y()) &&
                     extreme_d2 && std::isfinite(extreme_d2->x()) &&
                     std::isfinite(extreme_d2->y()),
                 "multi-span extreme finite jet introduced avoidable overflow") &&
             passed;

    const auto huge0 = Point2::make(-maximum, 0.0);
    const auto huge1 = Point2::make(maximum, 0.0);
    if (!huge0 || !huge1) {
        return 1;
    }
    const std::vector<Point2> unrepresentable_points{
        *huge0,
        *huge1,
        multi_points[2],
        multi_points[3],
        multi_points[4],
        multi_points[5]};
    const auto unrepresentable = MultiSpanCubicNURBS2::make(
        unrepresentable_points,
        {1.0, 1.0, 1.0, 1.0, 1.0, 1.0},
        {minimum, 0.5},
        0.0,
        1.0);
    if (!unrepresentable) {
        return 1;
    }
    const auto unrepresentable_d1 =
        unrepresentable->first_derivative(0.0);
    passed = require(
                 !unrepresentable_d1 &&
                     unrepresentable_d1.error() ==
                         CurveError::non_finite_result,
                 "multi-span unrepresentable derivative did not fail") &&
             passed;

    const auto repeat_value_a = multi->evaluate(0.75);
    const auto repeat_value_b = multi->evaluate(0.75);
    const auto repeat_d1_a = multi->first_derivative(2.0);
    const auto repeat_d1_b = multi->first_derivative(2.0);
    const auto repeat_d2_a = multi->second_derivative(-1.0);
    const auto repeat_d2_b = multi->second_derivative(-1.0);
    const auto repeat_failure_a =
        multi->evaluate(std::numeric_limits<double>::infinity());
    const auto repeat_failure_b =
        multi->evaluate(std::numeric_limits<double>::infinity());
    passed = require(
                 repeat_value_a && repeat_value_b &&
                     *repeat_value_a == *repeat_value_b &&
                     repeat_d1_a && repeat_d1_b &&
                     *repeat_d1_a == *repeat_d1_b &&
                     repeat_d2_a && repeat_d2_b &&
                     *repeat_d2_a == *repeat_d2_b &&
                     !repeat_failure_a && !repeat_failure_b &&
                     repeat_failure_a.error() == repeat_failure_b.error(),
                 "multi-span repeated evidence is not deterministic") &&
             passed;

    return passed ? 0 : 1;
}
