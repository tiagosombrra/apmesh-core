#include "apmesh/geometry/bspline.hpp"

#include "apmesh/core/numeric.hpp"
#include "apmesh/geometry/curve.hpp"

#include <array>
#include <cmath>
#include <cstdio>
#include <expected>
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
    const double reference_scale = 1.0,
    const double absolute_tolerance = 5.0e-12,
    const double relative_tolerance = 5.0e-12) {
    const apmesh::core::ProximityPolicy policy{
        .absolute_tolerance = absolute_tolerance * reference_scale,
        .relative_tolerance = relative_tolerance,
        .reference_scale = reference_scale,
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
    const std::array<long double, 9>& knots,
    const int index,
    const int degree,
    const long double parameter) {
    if (degree == 0) {
        return knots[static_cast<std::size_t>(index)] <= parameter &&
                       parameter <
                           knots[static_cast<std::size_t>(index + 1)]
                   ? 1.0L
                   : 0.0L;
    }

    long double value = 0.0L;
    const long double left_denominator =
        knots[static_cast<std::size_t>(index + degree)] -
        knots[static_cast<std::size_t>(index)];
    if (left_denominator != 0.0L) {
        value +=
            (parameter - knots[static_cast<std::size_t>(index)]) /
            left_denominator *
            basis(knots, index, degree - 1, parameter);
    }

    const long double right_denominator =
        knots[static_cast<std::size_t>(index + degree + 1)] -
        knots[static_cast<std::size_t>(index + 1)];
    if (right_denominator != 0.0L) {
        value +=
            (knots[static_cast<std::size_t>(index + degree + 1)] -
             parameter) /
            right_denominator *
            basis(knots, index + 1, degree - 1, parameter);
    }

    return value;
}

long double basis_derivative(
    const std::array<long double, 9>& knots,
    const int index,
    const int degree,
    const int order,
    const long double parameter) {
    if (order == 0) {
        return basis(knots, index, degree, parameter);
    }
    if (degree == 0 || order > degree) {
        return 0.0L;
    }

    long double value = 0.0L;
    const long double left_denominator =
        knots[static_cast<std::size_t>(index + degree)] -
        knots[static_cast<std::size_t>(index)];
    if (left_denominator != 0.0L) {
        value +=
            static_cast<long double>(degree) / left_denominator *
            basis_derivative(
                knots, index, degree - 1, order - 1, parameter);
    }

    const long double right_denominator =
        knots[static_cast<std::size_t>(index + degree + 1)] -
        knots[static_cast<std::size_t>(index + 1)];
    if (right_denominator != 0.0L) {
        value -=
            static_cast<long double>(degree) / right_denominator *
            basis_derivative(
                knots, index + 1, degree - 1, order - 1, parameter);
    }

    return value;
}

std::array<long double, 9> wide_knots(
    const double lower,
    const double interior,
    const double upper) {
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

std::array<long double, 2> reference2(
    const std::array<apmesh::core::Point2, 5>& controls,
    const double lower,
    const double interior,
    const double upper,
    const double parameter,
    const int derivative_order) {
    const auto knots = wide_knots(lower, interior, upper);
    std::array<long double, 2> result{0.0L, 0.0L};

    for (int index = 0; index < 5; ++index) {
        const long double coefficient = basis_derivative(
            knots,
            index,
            3,
            derivative_order,
            static_cast<long double>(parameter));
        result[0] +=
            coefficient *
            static_cast<long double>(
                controls[static_cast<std::size_t>(index)].x());
        result[1] +=
            coefficient *
            static_cast<long double>(
                controls[static_cast<std::size_t>(index)].y());
    }
    return result;
}

std::array<long double, 3> reference3(
    const std::array<apmesh::core::Point3, 5>& controls,
    const double lower,
    const double interior,
    const double upper,
    const double parameter,
    const int derivative_order) {
    const auto knots = wide_knots(lower, interior, upper);
    std::array<long double, 3> result{0.0L, 0.0L, 0.0L};

    for (int index = 0; index < 5; ++index) {
        const long double coefficient = basis_derivative(
            knots,
            index,
            3,
            derivative_order,
            static_cast<long double>(parameter));
        const auto& point = controls[static_cast<std::size_t>(index)];
        result[0] += coefficient * static_cast<long double>(point.x());
        result[1] += coefficient * static_cast<long double>(point.y());
        result[2] += coefficient * static_cast<long double>(point.z());
    }
    return result;
}

std::expected<apmesh::core::Point2, apmesh::core::GeometryError> lerp_point(
    const apmesh::core::Point2& lhs,
    const apmesh::core::Point2& rhs,
    const double parameter) {
    return apmesh::core::Point2::make(
        std::lerp(lhs.x(), rhs.x(), parameter),
        std::lerp(lhs.y(), rhs.y(), parameter));
}

std::expected<apmesh::core::Point3, apmesh::core::GeometryError> lerp_point(
    const apmesh::core::Point3& lhs,
    const apmesh::core::Point3& rhs,
    const double parameter) {
    return apmesh::core::Point3::make(
        std::lerp(lhs.x(), rhs.x(), parameter),
        std::lerp(lhs.y(), rhs.y(), parameter),
        std::lerp(lhs.z(), rhs.z(), parameter));
}

} // namespace

int main() {
    using apmesh::core::BSplineConstructionError;
    using apmesh::core::BoundedParametricCurve2;
    using apmesh::core::BoundedParametricCurve3;
    using apmesh::core::CubicBezier2;
    using apmesh::core::CubicBezier3;
    using apmesh::core::CurveError;
    using apmesh::core::Point2;
    using apmesh::core::Point3;
    using apmesh::core::TwoSpanCubicBSpline2;
    using apmesh::core::TwoSpanCubicBSpline3;
    using apmesh::core::Vector2;
    using apmesh::core::Vector3;
    using apmesh::core::reversed_parameter;

    static_assert(BoundedParametricCurve2<TwoSpanCubicBSpline2>);
    static_assert(BoundedParametricCurve3<TwoSpanCubicBSpline3>);

    bool passed = true;

    const auto p20 = Point2::make(-2.0, 1.0);
    const auto p21 = Point2::make(-0.5, 4.0);
    const auto p22 = Point2::make(2.0, -1.0);
    const auto p23 = Point2::make(5.0, 3.0);
    const auto p24 = Point2::make(7.0, 0.5);
    const auto p30 = Point3::make(-2.0, 1.0, 0.0);
    const auto p31 = Point3::make(-0.5, 4.0, 1.0);
    const auto p32 = Point3::make(2.0, -1.0, 2.0);
    const auto p33 = Point3::make(5.0, 3.0, 3.0);
    const auto p34 = Point3::make(7.0, 0.5, 4.0);
    if (!p20 || !p21 || !p22 || !p23 || !p24 ||
        !p30 || !p31 || !p32 || !p33 || !p34) {
        return 1;
    }

    const std::array<Point2, 5> controls2{*p20, *p21, *p22, *p23, *p24};
    const std::array<Point3, 5> controls3{*p30, *p31, *p32, *p33, *p34};

    const auto curve2 =
        TwoSpanCubicBSpline2::make(controls2, -2.0, 0.5, 4.0);
    const auto curve3 =
        TwoSpanCubicBSpline3::make(controls3, -2.0, 0.5, 4.0);
    if (!curve2 || !curve3) {
        return 1;
    }

    const auto nan_lower = TwoSpanCubicBSpline2::make(
        controls2,
        std::numeric_limits<double>::quiet_NaN(),
        0.5,
        4.0);
    const auto inf_interior = TwoSpanCubicBSpline2::make(
        controls2,
        -2.0,
        std::numeric_limits<double>::infinity(),
        4.0);
    const auto inf_upper = TwoSpanCubicBSpline2::make(
        controls2,
        -2.0,
        0.5,
        std::numeric_limits<double>::infinity());
    const auto bad_order_a =
        TwoSpanCubicBSpline2::make(controls2, 0.5, 0.5, 4.0);
    const auto bad_order_b =
        TwoSpanCubicBSpline2::make(controls2, -2.0, 4.0, 4.0);

    passed = require(
                 !nan_lower &&
                     nan_lower.error() ==
                         BSplineConstructionError::non_finite_lower_knot &&
                     !inf_interior &&
                     inf_interior.error() ==
                         BSplineConstructionError::non_finite_interior_knot &&
                     !inf_upper &&
                     inf_upper.error() ==
                         BSplineConstructionError::non_finite_upper_knot,
                 "B-spline non-finite knot errors differ") &&
             passed;
    passed = require(
                 !bad_order_a &&
                     bad_order_a.error() ==
                         BSplineConstructionError::non_strict_knot_order &&
                     !bad_order_b &&
                     bad_order_b.error() ==
                         BSplineConstructionError::non_strict_knot_order,
                 "B-spline strict knot-order errors differ") &&
             passed;

    const auto domain = curve2->parameter_domain();
    const auto knots = curve2->knots();
    passed = require(
                 domain.lower() == -2.0 && domain.upper() == 4.0 &&
                     curve2->lower_knot() == -2.0 &&
                     curve2->interior_knot() == 0.5 &&
                     curve2->upper_knot() == 4.0,
                 "B-spline domain/distinct-knot access differs") &&
             passed;
    passed = require(
                 knots ==
                     std::array<double, 9>{
                         -2.0,
                         -2.0,
                         -2.0,
                         -2.0,
                         0.5,
                         4.0,
                         4.0,
                         4.0,
                         4.0},
                 "B-spline full knot vector differs") &&
             passed;

    const auto endpoint0 = curve2->evaluate(-2.0);
    const auto endpoint1 = curve2->evaluate(4.0);
    passed = require(
                 endpoint0 && *endpoint0 == controls2[0] &&
                     endpoint1 && *endpoint1 == controls2[4],
                 "B-spline exact endpoint values differ") &&
             passed;

    const auto expected_left_d1 = Vector2::make(
        3.0 * (controls2[1].x() - controls2[0].x()) / 2.5,
        3.0 * (controls2[1].y() - controls2[0].y()) / 2.5);
    const auto expected_right_d1 = Vector2::make(
        3.0 * (controls2[4].x() - controls2[3].x()) / 3.5,
        3.0 * (controls2[4].y() - controls2[3].y()) / 3.5);
    const auto left_d1 = curve2->first_derivative(-2.0);
    const auto right_d1 = curve2->first_derivative(4.0);
    if (!expected_left_d1 || !expected_right_d1) {
        return 1;
    }
    passed = require(
                 left_d1 && close_vector(*left_d1, *expected_left_d1, 16.0) &&
                     right_d1 &&
                     close_vector(*right_d1, *expected_right_d1, 16.0),
                 "B-spline endpoint tangent relations differ") &&
             passed;

    constexpr std::array<double, 3> reference_parameters{-1.25, 0.5, 2.25};
    for (const double parameter : reference_parameters) {
        const auto value2 = curve2->evaluate(parameter);
        const auto d12 = curve2->first_derivative(parameter);
        const auto d22 = curve2->second_derivative(parameter);
        const auto ref_value2 =
            reference2(controls2, -2.0, 0.5, 4.0, parameter, 0);
        const auto ref_d12 =
            reference2(controls2, -2.0, 0.5, 4.0, parameter, 1);
        const auto ref_d22 =
            reference2(controls2, -2.0, 0.5, 4.0, parameter, 2);

        passed = require(
                     value2 &&
                         close_scalar(
                             value2->x(),
                             static_cast<double>(ref_value2[0]),
                             16.0) &&
                         close_scalar(
                             value2->y(),
                             static_cast<double>(ref_value2[1]),
                             16.0) &&
                         d12 &&
                         close_scalar(
                             d12->x(), static_cast<double>(ref_d12[0]), 32.0) &&
                         close_scalar(
                             d12->y(), static_cast<double>(ref_d12[1]), 32.0) &&
                         d22 &&
                         close_scalar(
                             d22->x(), static_cast<double>(ref_d22[0]), 64.0) &&
                         close_scalar(
                             d22->y(), static_cast<double>(ref_d22[1]), 64.0),
                     "2D independent B-spline basis oracle differs") &&
                 passed;

        const auto value3 = curve3->evaluate(parameter);
        const auto d13 = curve3->first_derivative(parameter);
        const auto d23 = curve3->second_derivative(parameter);
        const auto ref_value3 =
            reference3(controls3, -2.0, 0.5, 4.0, parameter, 0);
        const auto ref_d13 =
            reference3(controls3, -2.0, 0.5, 4.0, parameter, 1);
        const auto ref_d23 =
            reference3(controls3, -2.0, 0.5, 4.0, parameter, 2);

        passed = require(
                     value3 &&
                         close_scalar(
                             value3->x(),
                             static_cast<double>(ref_value3[0]),
                             16.0) &&
                         close_scalar(
                             value3->y(),
                             static_cast<double>(ref_value3[1]),
                             16.0) &&
                         close_scalar(
                             value3->z(),
                             static_cast<double>(ref_value3[2]),
                             16.0) &&
                         d13 &&
                         close_scalar(
                             d13->x(), static_cast<double>(ref_d13[0]), 32.0) &&
                         close_scalar(
                             d13->y(), static_cast<double>(ref_d13[1]), 32.0) &&
                         close_scalar(
                             d13->z(), static_cast<double>(ref_d13[2]), 32.0) &&
                         d23 &&
                         close_scalar(
                             d23->x(), static_cast<double>(ref_d23[0]), 64.0) &&
                         close_scalar(
                             d23->y(), static_cast<double>(ref_d23[1]), 64.0) &&
                         close_scalar(
                             d23->z(), static_cast<double>(ref_d23[2]), 64.0),
                     "3D independent B-spline basis oracle differs") &&
                 passed;
    }

    auto left_modified_controls = controls2;
    const auto left_far = Point2::make(1.0e8, -1.0e8);
    auto right_modified_controls = controls2;
    const auto right_far = Point2::make(-1.0e8, 1.0e8);
    if (!left_far || !right_far) {
        return 1;
    }
    left_modified_controls[4] = *left_far;
    right_modified_controls[0] = *right_far;
    const auto left_modified =
        TwoSpanCubicBSpline2::make(left_modified_controls, -2.0, 0.5, 4.0);
    const auto right_modified =
        TwoSpanCubicBSpline2::make(right_modified_controls, -2.0, 0.5, 4.0);
    if (!left_modified || !right_modified) {
        return 1;
    }
    const auto left_base = curve2->evaluate(-1.0);
    const auto left_other = left_modified->evaluate(-1.0);
    const auto right_base = curve2->evaluate(2.0);
    const auto right_other = right_modified->evaluate(2.0);
    passed = require(
                 left_base && left_other && *left_base == *left_other &&
                     right_base && right_other && *right_base == *right_other,
                 "B-spline local-support isolation differs") &&
             passed;

    const auto b0 = Point2::make(-3.0, 1.0);
    const auto b1 = Point2::make(-1.0, 5.0);
    const auto b2 = Point2::make(3.0, -2.0);
    const auto b3 = Point2::make(8.0, 2.0);
    const auto b03 = Point3::make(-3.0, 1.0, 0.0);
    const auto b13 = Point3::make(-1.0, 5.0, 2.0);
    const auto b23 = Point3::make(3.0, -2.0, 4.0);
    const auto b33 = Point3::make(8.0, 2.0, 6.0);
    if (!b0 || !b1 || !b2 || !b3 || !b03 || !b13 || !b23 || !b33) {
        return 1;
    }

    constexpr double bezier_lower = -1.0;
    constexpr double bezier_knot = 0.2;
    constexpr double bezier_upper = 3.0;
    constexpr double insertion_t =
        (bezier_knot - bezier_lower) / (bezier_upper - bezier_lower);

    const auto bp1 = lerp_point(*b0, *b1, insertion_t);
    const auto bp2 = lerp_point(*b1, *b2, insertion_t);
    const auto bp3 = lerp_point(*b2, *b3, insertion_t);
    const auto bp13 = lerp_point(*b03, *b13, insertion_t);
    const auto bp23 = lerp_point(*b13, *b23, insertion_t);
    const auto bp33 = lerp_point(*b23, *b33, insertion_t);
    if (!bp1 || !bp2 || !bp3 || !bp13 || !bp23 || !bp33) {
        return 1;
    }

    const CubicBezier2 cubic2{*b0, *b1, *b2, *b3};
    const CubicBezier3 cubic3{*b03, *b13, *b23, *b33};
    const std::array<Point2, 5> inserted2{*b0, *bp1, *bp2, *bp3, *b3};
    const std::array<Point3, 5> inserted3{*b03, *bp13, *bp23, *bp33, *b33};
    const auto parity2 = TwoSpanCubicBSpline2::make(
        inserted2, bezier_lower, bezier_knot, bezier_upper);
    const auto parity3 = TwoSpanCubicBSpline3::make(
        inserted3, bezier_lower, bezier_knot, bezier_upper);
    if (!parity2 || !parity3) {
        return 1;
    }

    constexpr std::array<double, 3> parity_parameters{-0.5, 0.2, 2.25};
    constexpr double parameter_width = bezier_upper - bezier_lower;
    for (const double parameter : parity_parameters) {
        const double t = (parameter - bezier_lower) / parameter_width;

        const auto pv2 = parity2->evaluate(parameter);
        const auto cv2 = cubic2.evaluate(t);
        const auto pd12 = parity2->first_derivative(parameter);
        const auto cd12 = cubic2.first_derivative(t);
        const auto pd22 = parity2->second_derivative(parameter);
        const auto cd22 = cubic2.second_derivative(t);

        const auto pv3 = parity3->evaluate(parameter);
        const auto cv3 = cubic3.evaluate(t);
        const auto pd13 = parity3->first_derivative(parameter);
        const auto cd13 = cubic3.first_derivative(t);
        const auto pd23 = parity3->second_derivative(parameter);
        const auto cd23 = cubic3.second_derivative(t);

        passed = require(
                     pv2 && cv2 && close_point(*pv2, *cv2, 32.0) &&
                         pd12 && cd12 &&
                         close_scalar(
                             pd12->x(), cd12->x() / parameter_width, 32.0) &&
                         close_scalar(
                             pd12->y(), cd12->y() / parameter_width, 32.0) &&
                         pd22 && cd22 &&
                         close_scalar(
                             pd22->x(),
                             cd22->x() / (parameter_width * parameter_width),
                             64.0) &&
                         close_scalar(
                             pd22->y(),
                             cd22->y() / (parameter_width * parameter_width),
                             64.0),
                     "2D B-spline/Bezier knot-insertion parity differs") &&
                 passed;

        passed = require(
                     pv3 && cv3 && close_point(*pv3, *cv3, 32.0) &&
                         pd13 && cd13 &&
                         close_scalar(
                             pd13->x(), cd13->x() / parameter_width, 32.0) &&
                         close_scalar(
                             pd13->y(), cd13->y() / parameter_width, 32.0) &&
                         close_scalar(
                             pd13->z(), cd13->z() / parameter_width, 32.0) &&
                         pd23 && cd23 &&
                         close_scalar(
                             pd23->x(),
                             cd23->x() / (parameter_width * parameter_width),
                             64.0) &&
                         close_scalar(
                             pd23->y(),
                             cd23->y() / (parameter_width * parameter_width),
                             64.0) &&
                         close_scalar(
                             pd23->z(),
                             cd23->z() / (parameter_width * parameter_width),
                             64.0),
                     "3D B-spline/Bezier knot-insertion parity differs") &&
                 passed;
    }

    const auto reversed2 = curve2->reversed();
    const auto reversed3 = curve3->reversed();
    const auto expected_reversed_knot =
        reversed_parameter(curve2->parameter_domain(), 0.5);
    if (!expected_reversed_knot) {
        return 1;
    }
    passed = require(
                 reversed2.control_points()[0] == controls2[4] &&
                     reversed2.control_points()[4] == controls2[0] &&
                     reversed2.interior_knot() == *expected_reversed_knot &&
                     reversed2.reversed() == *curve2 &&
                     reversed3.reversed() == *curve3,
                 "B-spline reversal representation/involution differs") &&
             passed;

    constexpr double reverse_query = -0.75;
    const auto mapped =
        reversed_parameter(curve2->parameter_domain(), reverse_query);
    if (!mapped) {
        return 1;
    }
    const auto original_value = curve2->evaluate(reverse_query);
    const auto reverse_value = reversed2.evaluate(*mapped);
    const auto original_d1 = curve2->first_derivative(reverse_query);
    const auto reverse_d1 = reversed2.first_derivative(*mapped);
    const auto original_d2 = curve2->second_derivative(reverse_query);
    const auto reverse_d2 = reversed2.second_derivative(*mapped);
    passed = require(
                 original_value && reverse_value &&
                     close_point(*original_value, *reverse_value, 32.0) &&
                     original_d1 && reverse_d1 &&
                     close_scalar(reverse_d1->x(), -original_d1->x(), 64.0) &&
                     close_scalar(reverse_d1->y(), -original_d1->y(), 64.0) &&
                     original_d2 && reverse_d2 &&
                     close_vector(*original_d2, *reverse_d2, 128.0),
                 "B-spline reversal covariance differs") &&
             passed;

    const auto translated0 = Point2::make(8.0, -6.0);
    const auto translated1 = Point2::make(9.5, -3.0);
    const auto translated2 = Point2::make(12.0, -8.0);
    const auto translated3 = Point2::make(15.0, -4.0);
    const auto translated4 = Point2::make(17.0, -6.5);
    if (!translated0 || !translated1 || !translated2 ||
        !translated3 || !translated4) {
        return 1;
    }
    const std::array<Point2, 5> translated_controls{
        *translated0, *translated1, *translated2, *translated3, *translated4};
    const auto translated =
        TwoSpanCubicBSpline2::make(translated_controls, -2.0, 0.5, 4.0);
    if (!translated) {
        return 1;
    }
    const auto base_translation_value = curve2->evaluate(1.75);
    const auto translated_value = translated->evaluate(1.75);
    const auto base_translation_d1 = curve2->first_derivative(1.75);
    const auto translated_d1 = translated->first_derivative(1.75);
    const auto base_translation_d2 = curve2->second_derivative(1.75);
    const auto translated_d2 = translated->second_derivative(1.75);
    passed = require(
                 base_translation_value && translated_value &&
                     close_scalar(
                         translated_value->x() - base_translation_value->x(),
                         10.0,
                         32.0) &&
                     close_scalar(
                         translated_value->y() - base_translation_value->y(),
                         -7.0,
                         32.0) &&
                     base_translation_d1 && translated_d1 &&
                     close_vector(*base_translation_d1, *translated_d1, 64.0) &&
                     base_translation_d2 && translated_d2 &&
                     close_vector(*base_translation_d2, *translated_d2, 128.0),
                 "B-spline translation covariance differs") &&
             passed;

    std::array<Point2, 5> scaled_controls2{};
    for (std::size_t index = 0; index < controls2.size(); ++index) {
        const auto point = Point2::make(
            controls2[index].x() * 2.0,
            controls2[index].y() * 2.0);
        if (!point) {
            return 1;
        }
        scaled_controls2[index] = *point;
    }
    const auto scaled =
        TwoSpanCubicBSpline2::make(scaled_controls2, -2.0, 0.5, 4.0);
    if (!scaled) {
        return 1;
    }
    const auto scale_value = scaled->evaluate(1.25);
    const auto base_scale_value = curve2->evaluate(1.25);
    const auto scale_d1 = scaled->first_derivative(1.25);
    const auto base_scale_d1 = curve2->first_derivative(1.25);
    const auto scale_d2 = scaled->second_derivative(1.25);
    const auto base_scale_d2 = curve2->second_derivative(1.25);
    passed = require(
                 scale_value && base_scale_value &&
                     close_scalar(
                         scale_value->x(), 2.0 * base_scale_value->x(), 32.0) &&
                     close_scalar(
                         scale_value->y(), 2.0 * base_scale_value->y(), 32.0) &&
                     scale_d1 && base_scale_d1 &&
                     close_scalar(
                         scale_d1->x(), 2.0 * base_scale_d1->x(), 64.0) &&
                     close_scalar(
                         scale_d1->y(), 2.0 * base_scale_d1->y(), 64.0) &&
                     scale_d2 && base_scale_d2 &&
                     close_scalar(
                         scale_d2->x(), 2.0 * base_scale_d2->x(), 128.0) &&
                     close_scalar(
                         scale_d2->y(), 2.0 * base_scale_d2->y(), 128.0),
                 "B-spline exact power-of-two scale covariance differs") &&
             passed;

    const std::array<Point3, 5> embedded_controls{
        *Point3::make(p20->x(), p20->y(), 0.0),
        *Point3::make(p21->x(), p21->y(), 0.0),
        *Point3::make(p22->x(), p22->y(), 0.0),
        *Point3::make(p23->x(), p23->y(), 0.0),
        *Point3::make(p24->x(), p24->y(), 0.0),
    };
    const auto embedded =
        TwoSpanCubicBSpline3::make(embedded_controls, -2.0, 0.5, 4.0);
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
                     close_scalar(embed_d12->x(), embed_d13->x(), 64.0) &&
                     close_scalar(embed_d12->y(), embed_d13->y(), 64.0) &&
                     embed_d13->z() == 0.0 &&
                     embed_d22 && embed_d23 &&
                     close_scalar(embed_d22->x(), embed_d23->x(), 128.0) &&
                     close_scalar(embed_d22->y(), embed_d23->y(), 128.0) &&
                     embed_d23->z() == 0.0,
                 "B-spline 2D/3D embedding parity differs") &&
             passed;

    const double maximum = std::numeric_limits<double>::max();
    const auto extreme =
        TwoSpanCubicBSpline2::make(controls2, -maximum, 0.0, maximum);
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
                 "B-spline extreme finite knot evaluation differs") &&
             passed;

    const double tiny = std::numeric_limits<double>::denorm_min();
    const auto huge0 = Point2::make(-maximum, 0.0);
    const auto huge1 = Point2::make(maximum, 0.0);
    if (!huge0 || !huge1) {
        return 1;
    }
    std::array<Point2, 5> huge_controls{
        *huge0, *huge1, controls2[2], controls2[3], controls2[4]};
    const auto unrepresentable =
        TwoSpanCubicBSpline2::make(huge_controls, 0.0, tiny, 1.0);
    if (!unrepresentable) {
        return 1;
    }
    const auto bad_d1 = unrepresentable->first_derivative(0.0);
    passed = require(
                 !bad_d1 &&
                     bad_d1.error() == CurveError::non_finite_result,
                 "B-spline unrepresentable derivative did not fail explicitly") &&
             passed;

    const auto nan_query = curve2->evaluate(
        std::numeric_limits<double>::quiet_NaN());
    const auto below = curve2->first_derivative(-2.1);
    const auto above = curve3->second_derivative(4.1);
    passed = require(
                 !nan_query &&
                     nan_query.error() == CurveError::non_finite_parameter &&
                     !below &&
                     below.error() == CurveError::parameter_out_of_domain &&
                     !above &&
                     above.error() == CurveError::parameter_out_of_domain,
                 "B-spline query failure semantics differ") &&
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
                 "B-spline repeated evidence is not deterministic") &&
             passed;

    return passed ? 0 : 1;
}
