#include "apmesh/geometry/nurbs.hpp"

#include "apmesh/core/numeric.hpp"
#include "apmesh/geometry/bspline.hpp"
#include "apmesh/geometry/rational_bezier.hpp"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <limits>
#include <string_view>

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
    const double absolute = 3.0e-12,
    const double relative = 3.0e-12) {
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

long double basis(
    const std::array<double, 9>& knots,
    const int index,
    const int degree,
    const long double parameter) {
    if (degree == 0) {
        const long double lower =
            static_cast<long double>(knots[static_cast<std::size_t>(index)]);
        const long double upper =
            static_cast<long double>(knots[static_cast<std::size_t>(index + 1)]);
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
    const std::array<double, 9>& knots,
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
    const std::array<double, 9>& knots,
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
    const std::array<apmesh::core::Point2, 5>& points,
    const std::array<double, 5>& weights,
    const std::array<double, 9>& knots,
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

    for (int index = 0; index < 5; ++index) {
        const long double n = basis(knots, index, 3, u);
        const long double dn = basis_first(knots, index, 3, u);
        const long double ddn = basis_second(knots, index, 3, u);
        const long double weight =
            static_cast<long double>(weights[static_cast<std::size_t>(index)]);
        const auto& point = points[static_cast<std::size_t>(index)];

        ax += n * weight * static_cast<long double>(point.x());
        ay += n * weight * static_cast<long double>(point.y());
        w += n * weight;

        dax += dn * weight * static_cast<long double>(point.x());
        day += dn * weight * static_cast<long double>(point.y());
        dw += dn * weight;

        ddax += ddn * weight * static_cast<long double>(point.x());
        dday += ddn * weight * static_cast<long double>(point.y());
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

H2 mix(const H2& lhs, const H2& rhs, const long double t) {
    return {
        std::lerp(lhs.x, rhs.x, t),
        std::lerp(lhs.y, rhs.y, t),
        std::lerp(lhs.w, rhs.w, t),
    };
}

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

std::expected<apmesh::core::Point2, apmesh::core::GeometryError>
dehomogenize(const H2& value) {
    return apmesh::core::Point2::make(
        static_cast<double>(value.x / value.w),
        static_cast<double>(value.y / value.w));
}

} // namespace

int main() {
    using apmesh::core::BoundedParametricCurve2;
    using apmesh::core::BoundedParametricCurve3;
    using apmesh::core::CurveError;
    using apmesh::core::NURBSConstructionError;
    using apmesh::core::Point2;
    using apmesh::core::Point3;
    using apmesh::core::RationalQuadraticBezier2;
    using apmesh::core::TwoSpanCubicBSpline2;
    using apmesh::core::TwoSpanCubicBSpline3;
    using apmesh::core::TwoSpanCubicNURBS2;
    using apmesh::core::TwoSpanCubicNURBS3;
    using apmesh::core::reversed_parameter;

    static_assert(BoundedParametricCurve2<TwoSpanCubicNURBS2>);
    static_assert(BoundedParametricCurve3<TwoSpanCubicNURBS3>);

    bool passed = true;

    const auto p0 = Point2::make(-2.0, 1.0);
    const auto p1 = Point2::make(-0.5, 4.0);
    const auto p2 = Point2::make(2.0, -1.0);
    const auto p3 = Point2::make(5.0, 3.0);
    const auto p4 = Point2::make(7.0, 0.5);
    if (!p0 || !p1 || !p2 || !p3 || !p4) {
        return 1;
    }
    const std::array<Point2, 5> controls2{*p0, *p1, *p2, *p3, *p4};
    const std::array<double, 5> weights{1.0, 2.0, 0.75, 4.0, 1.5};

    const auto curve2 =
        TwoSpanCubicNURBS2::make(controls2, weights, -2.0, 0.5, 4.0);
    if (!curve2) {
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
    const std::array<Point3, 5> controls3{*q0, *q1, *q2, *q3, *q4};
    const auto curve3 =
        TwoSpanCubicNURBS3::make(controls3, weights, -2.0, 0.5, 4.0);
    if (!curve3) {
        return 1;
    }

    const auto nan_weight = TwoSpanCubicNURBS2::make(
        controls2,
        {1.0,
         1.0,
         std::numeric_limits<double>::quiet_NaN(),
         1.0,
         1.0},
        -2.0,
        0.5,
        4.0);
    const auto zero_weight = TwoSpanCubicNURBS2::make(
        controls2, {1.0, 1.0, 0.0, 1.0, 1.0}, -2.0, 0.5, 4.0);
    const auto negative_weight = TwoSpanCubicNURBS2::make(
        controls2, {1.0, 1.0, -1.0, 1.0, 1.0}, -2.0, 0.5, 4.0);
    const auto bad_knots =
        TwoSpanCubicNURBS2::make(controls2, weights, -2.0, -2.0, 4.0);

    passed = require(
                 !nan_weight &&
                     nan_weight.error() ==
                         NURBSConstructionError::non_finite_weight &&
                     !zero_weight &&
                     zero_weight.error() ==
                         NURBSConstructionError::non_positive_weight &&
                     !negative_weight &&
                     negative_weight.error() ==
                         NURBSConstructionError::non_positive_weight &&
                     !bad_knots &&
                     bad_knots.error() ==
                         NURBSConstructionError::non_strict_knot_order,
                 "NURBS construction validation differs") &&
             passed;

    passed = require(
                 curve2->parameter_domain().lower() == -2.0 &&
                     curve2->parameter_domain().upper() == 4.0 &&
                     curve2->evaluate(-2.0) &&
                     *curve2->evaluate(-2.0) == controls2[0] &&
                     curve2->evaluate(4.0) &&
                     *curve2->evaluate(4.0) == controls2[4],
                 "NURBS exact domain/endpoints differ") &&
             passed;

    const auto knots = curve2->knots();
    constexpr std::array<double, 3> reference_parameters{-1.25, 0.5, 2.75};
    for (const double parameter : reference_parameters) {
        const auto reference =
            reference_jet(controls2, weights, knots, parameter);
        const auto value = curve2->evaluate(parameter);
        const auto d1 = curve2->first_derivative(parameter);
        const auto d2 = curve2->second_derivative(parameter);

        passed = require(
                     value && d1 && d2 &&
                         close_scalar(
                             value->x(),
                             static_cast<double>(reference.x),
                             16.0) &&
                         close_scalar(
                             value->y(),
                             static_cast<double>(reference.y),
                             16.0) &&
                         close_scalar(
                             d1->x(),
                             static_cast<double>(reference.dx),
                             64.0) &&
                         close_scalar(
                             d1->y(),
                             static_cast<double>(reference.dy),
                             64.0) &&
                         close_scalar(
                             d2->x(),
                             static_cast<double>(reference.ddx),
                             256.0) &&
                         close_scalar(
                             d2->y(),
                             static_cast<double>(reference.ddy),
                             256.0),
                     "independent rational-basis NURBS jet differs") &&
                 passed;
    }

    const std::array<double, 5> equal_weights{4.0, 4.0, 4.0, 4.0, 4.0};
    const auto equal_nurbs =
        TwoSpanCubicNURBS2::make(
            controls2, equal_weights, -2.0, 0.5, 4.0);
    const auto polynomial =
        TwoSpanCubicBSpline2::make(controls2, -2.0, 0.5, 4.0);
    if (!equal_nurbs || !polynomial) {
        return 1;
    }

    constexpr std::array<double, 5> parity_parameters{
        -2.0, -0.75, 0.5, 2.25, 4.0};
    for (const double parameter : parity_parameters) {
        const auto nv = equal_nurbs->evaluate(parameter);
        const auto bv = polynomial->evaluate(parameter);
        const auto nd1 = equal_nurbs->first_derivative(parameter);
        const auto bd1 = polynomial->first_derivative(parameter);
        const auto nd2 = equal_nurbs->second_derivative(parameter);
        const auto bd2 = polynomial->second_derivative(parameter);
        passed = require(
                     nv && bv && close_point(*nv, *bv, 32.0) &&
                         nd1 && bd1 && close_vector(*nd1, *bd1, 128.0) &&
                         nd2 && bd2 && close_vector(*nd2, *bd2, 512.0),
                     "equal-weight polynomial B-spline parity differs") &&
                 passed;
    }

    const auto equal_nurbs3 =
        TwoSpanCubicNURBS3::make(
            controls3, equal_weights, -2.0, 0.5, 4.0);
    const auto polynomial3 =
        TwoSpanCubicBSpline3::make(controls3, -2.0, 0.5, 4.0);
    if (!equal_nurbs3 || !polynomial3) {
        return 1;
    }
    for (const double parameter : parity_parameters) {
        const auto nv = equal_nurbs3->evaluate(parameter);
        const auto bv = polynomial3->evaluate(parameter);
        const auto nd1 = equal_nurbs3->first_derivative(parameter);
        const auto bd1 = polynomial3->first_derivative(parameter);
        const auto nd2 = equal_nurbs3->second_derivative(parameter);
        const auto bd2 = polynomial3->second_derivative(parameter);
        passed = require(
                     nv && bv && close_point(*nv, *bv, 32.0) &&
                         nd1 && bd1 && close_vector(*nd1, *bd1, 128.0) &&
                         nd2 && bd2 && close_vector(*nd2, *bd2, 512.0),
                     "3D equal-weight polynomial B-spline parity differs") &&
                 passed;
    }

    const std::array<double, 5> scaled_weights{
        8.0, 16.0, 6.0, 32.0, 12.0};
    const auto scaled =
        TwoSpanCubicNURBS2::make(
            controls2, scaled_weights, -2.0, 0.5, 4.0);
    if (!scaled) {
        return 1;
    }
    for (const double parameter : reference_parameters) {
        const auto av = curve2->evaluate(parameter);
        const auto bv = scaled->evaluate(parameter);
        const auto ad1 = curve2->first_derivative(parameter);
        const auto bd1 = scaled->first_derivative(parameter);
        const auto ad2 = curve2->second_derivative(parameter);
        const auto bd2 = scaled->second_derivative(parameter);
        passed = require(
                     av && bv && *av == *bv &&
                         ad1 && bd1 && *ad1 == *bd1 &&
                         ad2 && bd2 && *ad2 == *bd2,
                     "common power-of-two weight-scale invariance differs") &&
                 passed;
    }

    auto left_controls = controls2;
    left_controls[4] = *Point2::make(100.0, -80.0);
    auto left_weights = weights;
    left_weights[4] = 64.0;
    const auto left_changed =
        TwoSpanCubicNURBS2::make(
            left_controls, left_weights, -2.0, 0.5, 4.0);
    auto right_controls = controls2;
    right_controls[0] = *Point2::make(-100.0, 90.0);
    auto right_weights = weights;
    right_weights[0] = 32.0;
    const auto right_changed =
        TwoSpanCubicNURBS2::make(
            right_controls, right_weights, -2.0, 0.5, 4.0);
    if (!left_changed || !right_changed) {
        return 1;
    }

    const auto left_base = curve2->evaluate(-1.0);
    const auto left_other = left_changed->evaluate(-1.0);
    const auto left_d1 = curve2->first_derivative(-1.0);
    const auto left_other_d1 = left_changed->first_derivative(-1.0);
    const auto left_d2 = curve2->second_derivative(-1.0);
    const auto left_other_d2 = left_changed->second_derivative(-1.0);
    const auto right_base = curve2->evaluate(2.0);
    const auto right_other = right_changed->evaluate(2.0);
    const auto right_d1 = curve2->first_derivative(2.0);
    const auto right_other_d1 = right_changed->first_derivative(2.0);
    const auto right_d2 = curve2->second_derivative(2.0);
    const auto right_other_d2 = right_changed->second_derivative(2.0);
    passed = require(
                 left_base && left_other && *left_base == *left_other &&
                     left_d1 && left_other_d1 &&
                     *left_d1 == *left_other_d1 &&
                     left_d2 && left_other_d2 &&
                     *left_d2 == *left_other_d2 &&
                     right_base && right_other && *right_base == *right_other &&
                     right_d1 && right_other_d1 &&
                     *right_d1 == *right_other_d1 &&
                     right_d2 && right_other_d2 &&
                     *right_d2 == *right_other_d2,
                 "NURBS local-support value/D1/D2 isolation differs") &&
             passed;

    const auto reversed = curve2->reversed();
    passed = require(
                 reversed.reversed() == *curve2 &&
                     reversed.control_points()[0] == controls2[4] &&
                     reversed.weights()[0] == weights[4],
                 "NURBS reversal storage/involution differs") &&
             passed;

    constexpr double reversal_parameter_value = -0.75;
    const auto mapped =
        reversed_parameter(
            curve2->parameter_domain(), reversal_parameter_value);
    if (!mapped) {
        return 1;
    }
    const auto original_value =
        curve2->evaluate(reversal_parameter_value);
    const auto reversed_value = reversed.evaluate(*mapped);
    const auto original_d1 =
        curve2->first_derivative(reversal_parameter_value);
    const auto reversed_d1 = reversed.first_derivative(*mapped);
    const auto original_d2 =
        curve2->second_derivative(reversal_parameter_value);
    const auto reversed_d2 = reversed.second_derivative(*mapped);
    passed = require(
                 original_value && reversed_value &&
                     close_point(*original_value, *reversed_value, 32.0) &&
                     original_d1 && reversed_d1 &&
                     close_scalar(
                         reversed_d1->x(), -original_d1->x(), 128.0) &&
                     close_scalar(
                         reversed_d1->y(), -original_d1->y(), 128.0) &&
                     original_d2 && reversed_d2 &&
                     close_vector(*original_d2, *reversed_d2, 512.0),
                 "NURBS reversal covariance differs") &&
             passed;

    const auto c0 = Point2::make(1.0, 0.0);
    const auto c1 = Point2::make(1.0, 1.0);
    const auto c2 = Point2::make(0.0, 1.0);
    if (!c0 || !c1 || !c2) {
        return 1;
    }
    const double rq_weight = std::sqrt(0.5);
    const auto rational_quadratic =
        RationalQuadraticBezier2::make(
            *c0, *c1, *c2, 1.0, rq_weight, 1.0);
    if (!rational_quadratic) {
        return 1;
    }

    const H2 h0 = homogeneous(*c0, 1.0);
    const H2 h1 = homogeneous(*c1, rq_weight);
    const H2 h2 = homogeneous(*c2, 1.0);
    const H2 g0 = h0;
    const H2 g1{
        (h0.x + 2.0L * h1.x) / 3.0L,
        (h0.y + 2.0L * h1.y) / 3.0L,
        (h0.w + 2.0L * h1.w) / 3.0L};
    const H2 g2{
        (2.0L * h1.x + h2.x) / 3.0L,
        (2.0L * h1.y + h2.y) / 3.0L,
        (2.0L * h1.w + h2.w) / 3.0L};
    const H2 g3 = h2;

    constexpr long double insertion = 0.37L;
    const std::array<H2, 5> inserted{
        g0,
        mix(g0, g1, insertion),
        mix(g1, g2, insertion),
        mix(g2, g3, insertion),
        g3};

    std::array<Point2, 5> conic_controls{
        *c0, *c0, *c0, *c0, *c2};
    std::array<double, 5> conic_weights{};
    for (std::size_t index = 0; index < inserted.size(); ++index) {
        const auto point = dehomogenize(inserted[index]);
        if (!point) {
            return 1;
        }
        conic_controls[index] = *point;
        conic_weights[index] = static_cast<double>(inserted[index].w);
    }

    const auto conic_nurbs =
        TwoSpanCubicNURBS2::make(
            conic_controls, conic_weights, 0.0, 0.37, 1.0);
    if (!conic_nurbs) {
        return 1;
    }

    constexpr std::array<double, 3> conic_parameters{0.15, 0.5, 0.85};
    for (const double parameter : conic_parameters) {
        const auto rv = rational_quadratic->evaluate(parameter);
        const auto nv = conic_nurbs->evaluate(parameter);
        const auto rd1 = rational_quadratic->first_derivative(parameter);
        const auto nd1 = conic_nurbs->first_derivative(parameter);
        const auto rd2 = rational_quadratic->second_derivative(parameter);
        const auto nd2 = conic_nurbs->second_derivative(parameter);

        passed = require(
                     rv && nv && close_point(*rv, *nv, 4.0) &&
                         rd1 && nd1 && close_vector(*rd1, *nd1, 16.0) &&
                         rd2 && nd2 && close_vector(*rd2, *nd2, 64.0),
                     "rational-quadratic degree-elevation/knot parity differs") &&
                 passed;
    }

    const auto constant_point = Point2::make(3.0, -4.0);
    if (!constant_point) {
        return 1;
    }
    const std::array<Point2, 5> constant_controls{
        *constant_point,
        *constant_point,
        *constant_point,
        *constant_point,
        *constant_point};
    const auto constant = TwoSpanCubicNURBS2::make(
        constant_controls,
        {1.0, 2.0, 8.0, 0.5, 4.0},
        -2.0,
        0.5,
        4.0);
    if (!constant) {
        return 1;
    }
    const auto constant_value = constant->evaluate(1.25);
    const auto constant_d1 = constant->first_derivative(1.25);
    const auto constant_d2 = constant->second_derivative(1.25);
    passed = require(
                 constant_value && *constant_value == *constant_point &&
                     constant_d1 && constant_d1->x() == 0.0 &&
                     constant_d1->y() == 0.0 &&
                     constant_d2 && constant_d2->x() == 0.0 &&
                     constant_d2->y() == 0.0,
                 "constant NURBS semantics differ") &&
             passed;

    const auto nan_query =
        curve2->evaluate(std::numeric_limits<double>::quiet_NaN());
    const auto below = curve2->first_derivative(-2.1);
    const auto above = curve3->second_derivative(4.1);
    passed = require(
                 !nan_query &&
                     nan_query.error() == CurveError::non_finite_parameter &&
                     !below &&
                     below.error() == CurveError::parameter_out_of_domain &&
                     !above &&
                     above.error() == CurveError::parameter_out_of_domain,
                 "NURBS query failure semantics differ") &&
             passed;

    const auto embedded0 = Point3::make(p0->x(), p0->y(), 0.0);
    const auto embedded1 = Point3::make(p1->x(), p1->y(), 0.0);
    const auto embedded2 = Point3::make(p2->x(), p2->y(), 0.0);
    const auto embedded3 = Point3::make(p3->x(), p3->y(), 0.0);
    const auto embedded4 = Point3::make(p4->x(), p4->y(), 0.0);
    if (!embedded0 || !embedded1 || !embedded2 ||
        !embedded3 || !embedded4) {
        return 1;
    }
    const std::array<Point3, 5> embedded_controls{
        *embedded0, *embedded1, *embedded2, *embedded3, *embedded4};
    const auto embedded = TwoSpanCubicNURBS3::make(
        embedded_controls, weights, -2.0, 0.5, 4.0);
    if (!embedded) {
        return 1;
    }
    const auto embed_value2 = curve2->evaluate(1.1);
    const auto embed_value3 = embedded->evaluate(1.1);
    const auto embed_d12 = curve2->first_derivative(1.1);
    const auto embed_d13 = embedded->first_derivative(1.1);
    const auto embed_d22 = curve2->second_derivative(1.1);
    const auto embed_d23 = embedded->second_derivative(1.1);
    passed = require(
                 embed_value2 && embed_value3 &&
                     close_scalar(embed_value2->x(), embed_value3->x(), 32.0) &&
                     close_scalar(embed_value2->y(), embed_value3->y(), 32.0) &&
                     embed_value3->z() == 0.0 &&
                     embed_d12 && embed_d13 &&
                     close_scalar(embed_d12->x(), embed_d13->x(), 128.0) &&
                     close_scalar(embed_d12->y(), embed_d13->y(), 128.0) &&
                     embed_d13->z() == 0.0 &&
                     embed_d22 && embed_d23 &&
                     close_scalar(embed_d22->x(), embed_d23->x(), 512.0) &&
                     close_scalar(embed_d22->y(), embed_d23->y(), 512.0) &&
                     embed_d23->z() == 0.0,
                 "NURBS 2D/3D embedding parity differs") &&
             passed;

    const auto translated0 = Point2::make(p0->x() + 8.0, p0->y() - 6.0);
    const auto translated1 = Point2::make(p1->x() + 8.0, p1->y() - 6.0);
    const auto translated2 = Point2::make(p2->x() + 8.0, p2->y() - 6.0);
    const auto translated3 = Point2::make(p3->x() + 8.0, p3->y() - 6.0);
    const auto translated4 = Point2::make(p4->x() + 8.0, p4->y() - 6.0);
    if (!translated0 || !translated1 || !translated2 ||
        !translated3 || !translated4) {
        return 1;
    }
    const std::array<Point2, 5> translated_controls{
        *translated0,
        *translated1,
        *translated2,
        *translated3,
        *translated4};
    const auto translated = TwoSpanCubicNURBS2::make(
        translated_controls, weights, -2.0, 0.5, 4.0);
    if (!translated) {
        return 1;
    }
    const auto base_value = curve2->evaluate(1.25);
    const auto translated_value = translated->evaluate(1.25);
    const auto base_d1 = curve2->first_derivative(1.25);
    const auto translated_d1 = translated->first_derivative(1.25);
    const auto base_d2 = curve2->second_derivative(1.25);
    const auto translated_d2 = translated->second_derivative(1.25);
    passed = require(
                 base_value && translated_value &&
                     close_scalar(
                         translated_value->x() - base_value->x(),
                         8.0,
                         16.0) &&
                     close_scalar(
                         translated_value->y() - base_value->y(),
                         -6.0,
                         16.0) &&
                     base_d1 && translated_d1 &&
                     close_vector(*base_d1, *translated_d1, 128.0) &&
                     base_d2 && translated_d2 &&
                     close_vector(*base_d2, *translated_d2, 512.0),
                 "NURBS translation covariance differs") &&
             passed;

    const auto scaled0 = Point2::make(2.0 * p0->x(), 2.0 * p0->y());
    const auto scaled1 = Point2::make(2.0 * p1->x(), 2.0 * p1->y());
    const auto scaled2 = Point2::make(2.0 * p2->x(), 2.0 * p2->y());
    const auto scaled3 = Point2::make(2.0 * p3->x(), 2.0 * p3->y());
    const auto scaled4 = Point2::make(2.0 * p4->x(), 2.0 * p4->y());
    if (!scaled0 || !scaled1 || !scaled2 || !scaled3 || !scaled4) {
        return 1;
    }
    const std::array<Point2, 5> scaled_controls{
        *scaled0, *scaled1, *scaled2, *scaled3, *scaled4};
    const auto coordinate_scaled = TwoSpanCubicNURBS2::make(
        scaled_controls, weights, -2.0, 0.5, 4.0);
    if (!coordinate_scaled) {
        return 1;
    }
    const auto scaled_value = coordinate_scaled->evaluate(1.25);
    const auto scaled_d1 = coordinate_scaled->first_derivative(1.25);
    const auto scaled_d2 = coordinate_scaled->second_derivative(1.25);
    passed = require(
                 base_value && scaled_value &&
                     close_scalar(
                         scaled_value->x(), 2.0 * base_value->x(), 64.0) &&
                     close_scalar(
                         scaled_value->y(), 2.0 * base_value->y(), 64.0) &&
                     base_d1 && scaled_d1 &&
                     close_scalar(
                         scaled_d1->x(), 2.0 * base_d1->x(), 256.0) &&
                     close_scalar(
                         scaled_d1->y(), 2.0 * base_d1->y(), 256.0) &&
                     base_d2 && scaled_d2 &&
                     close_scalar(
                         scaled_d2->x(), 2.0 * base_d2->x(), 1024.0) &&
                     close_scalar(
                         scaled_d2->y(), 2.0 * base_d2->y(), 1024.0),
                 "NURBS exact power-of-two coordinate scale covariance differs") &&
             passed;

    const double maximum = std::numeric_limits<double>::max();
    const double minimum = std::numeric_limits<double>::denorm_min();
    const auto extreme = TwoSpanCubicNURBS2::make(
        controls2,
        {minimum, maximum, maximum / 2.0, maximum / 4.0, minimum},
        -maximum,
        0.0,
        maximum);
    if (!extreme) {
        return 1;
    }
    const auto extreme_value = extreme->evaluate(maximum / 2.0);
    const auto extreme_d1 = extreme->first_derivative(maximum / 2.0);
    const auto extreme_d2 = extreme->second_derivative(maximum / 2.0);
    passed = require(
                 extreme_value && std::isfinite(extreme_value->x()) &&
                     std::isfinite(extreme_value->y()) &&
                     extreme_d1 && std::isfinite(extreme_d1->x()) &&
                     std::isfinite(extreme_d1->y()) &&
                     extreme_d2 && std::isfinite(extreme_d2->x()) &&
                     std::isfinite(extreme_d2->y()),
                 "NURBS extreme finite jet introduced avoidable overflow") &&
             passed;

    const auto huge0 = Point2::make(-maximum, 0.0);
    const auto huge1 = Point2::make(maximum, 0.0);
    if (!huge0 || !huge1) {
        return 1;
    }
    const std::array<Point2, 5> unrepresentable_controls{
        *huge0, *huge1, controls2[2], controls2[3], controls2[4]};
    const auto unrepresentable = TwoSpanCubicNURBS2::make(
        unrepresentable_controls,
        {1.0, 1.0, 1.0, 1.0, 1.0},
        0.0,
        minimum,
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
                 "unrepresentable NURBS derivative did not fail explicitly") &&
             passed;

    const auto repeat_value_a = curve2->evaluate(0.5);
    const auto repeat_value_b = curve2->evaluate(0.5);
    const auto repeat_d1_a = curve3->first_derivative(2.0);
    const auto repeat_d1_b = curve3->first_derivative(2.0);
    const auto repeat_d2_a = curve3->second_derivative(-1.0);
    const auto repeat_d2_b = curve3->second_derivative(-1.0);
    passed = require(
                 repeat_value_a && repeat_value_b &&
                     *repeat_value_a == *repeat_value_b &&
                     repeat_d1_a && repeat_d1_b &&
                     *repeat_d1_a == *repeat_d1_b &&
                     repeat_d2_a && repeat_d2_b &&
                     *repeat_d2_a == *repeat_d2_b,
                 "NURBS repeated evidence is not deterministic") &&
             passed;

    return passed ? 0 : 1;
}
