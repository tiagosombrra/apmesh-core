#include "apmesh/geometry/rational_bezier.hpp"

#include "apmesh/core/numeric.hpp"
#include "apmesh/geometry/curve.hpp"

#include <array>
#include <cmath>
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
    const double reference_scale = 1.0,
    const double absolute_tolerance = 2.0e-13,
    const double relative_tolerance = 2.0e-13) {
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

std::expected<apmesh::core::Point2, apmesh::core::GeometryError>
lerp_point(
    const apmesh::core::Point2& lhs,
    const apmesh::core::Point2& rhs,
    const double parameter) {
    return apmesh::core::Point2::make(
        std::lerp(lhs.x(), rhs.x(), parameter),
        std::lerp(lhs.y(), rhs.y(), parameter));
}

std::expected<apmesh::core::Point3, apmesh::core::GeometryError>
lerp_point(
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
    using apmesh::core::BoundedParametricCurve2;
    using apmesh::core::BoundedParametricCurve3;
    using apmesh::core::CubicBezier2;
    using apmesh::core::CubicBezier3;
    using apmesh::core::CurveError;
    using apmesh::core::Point2;
    using apmesh::core::Point3;
    using apmesh::core::RationalBezierConstructionError;
    using apmesh::core::RationalQuadraticBezier2;
    using apmesh::core::RationalQuadraticBezier3;
    using apmesh::core::Vector2;
    using apmesh::core::Vector3;
    using apmesh::core::reversed_parameter;

    static_assert(BoundedParametricCurve2<RationalQuadraticBezier2>);
    static_assert(BoundedParametricCurve3<RationalQuadraticBezier3>);

    bool passed = true;

    const auto p20 = Point2::make(0.0, 0.0);
    const auto p21 = Point2::make(2.0, 4.0);
    const auto p22 = Point2::make(6.0, 0.0);
    const auto p30 = Point3::make(0.0, 0.0, 0.0);
    const auto p31 = Point3::make(2.0, 4.0, 2.0);
    const auto p32 = Point3::make(6.0, 0.0, 4.0);
    if (!p20 || !p21 || !p22 || !p30 || !p31 || !p32) {
        return 1;
    }

    const auto curve2 =
        RationalQuadraticBezier2::make(*p20, *p21, *p22, 1.0, 2.0, 1.0);
    const auto curve3 =
        RationalQuadraticBezier3::make(*p30, *p31, *p32, 1.0, 2.0, 1.0);
    if (!curve2 || !curve3) {
        return 1;
    }

    const auto nan_weight = RationalQuadraticBezier2::make(
        *p20,
        *p21,
        *p22,
        1.0,
        std::numeric_limits<double>::quiet_NaN(),
        1.0);
    const auto inf_weight = RationalQuadraticBezier2::make(
        *p20,
        *p21,
        *p22,
        1.0,
        std::numeric_limits<double>::infinity(),
        1.0);
    const auto zero_weight =
        RationalQuadraticBezier2::make(*p20, *p21, *p22, 1.0, 0.0, 1.0);
    const auto negative_weight =
        RationalQuadraticBezier2::make(*p20, *p21, *p22, 1.0, -1.0, 1.0);

    passed = require(
                 !nan_weight &&
                     nan_weight.error() ==
                         RationalBezierConstructionError::non_finite_weight &&
                     !inf_weight &&
                     inf_weight.error() ==
                         RationalBezierConstructionError::non_finite_weight,
                 "non-finite weight validation differs") &&
             passed;
    passed = require(
                 !zero_weight &&
                     zero_weight.error() ==
                         RationalBezierConstructionError::non_positive_weight &&
                     !negative_weight &&
                     negative_weight.error() ==
                         RationalBezierConstructionError::non_positive_weight,
                 "non-positive weight validation differs") &&
             passed;

    const auto domain2 = curve2->parameter_domain();
    const auto domain3 = curve3->parameter_domain();
    passed = require(
                 domain2.lower() == 0.0 && domain2.upper() == 1.0 &&
                     domain3.lower() == 0.0 && domain3.upper() == 1.0,
                 "rational quadratic domains are not exactly [0,1]") &&
             passed;

    const auto value20 = curve2->evaluate(0.0);
    const auto value21 = curve2->evaluate(1.0);
    const auto value30 = curve3->evaluate(0.0);
    const auto value31 = curve3->evaluate(1.0);
    passed = require(
                 value20 && *value20 == *p20 &&
                     value21 && *value21 == *p22,
                 "2D rational endpoints are not exact") &&
             passed;
    passed = require(
                 value30 && *value30 == *p30 &&
                     value31 && *value31 == *p32,
                 "3D rational endpoints are not exact") &&
             passed;

    const auto midpoint2 = curve2->evaluate(0.5);
    const auto midpoint3 = curve3->evaluate(0.5);
    const auto expected_midpoint2 = Point2::make(7.0 / 3.0, 8.0 / 3.0);
    const auto expected_midpoint3 =
        Point3::make(7.0 / 3.0, 8.0 / 3.0, 2.0);
    const auto expected_d12 = Vector2::make(4.0, 0.0);
    const auto expected_d13 = Vector3::make(4.0, 0.0, 8.0 / 3.0);
    const auto expected_d22 = Vector2::make(32.0 / 9.0, -128.0 / 9.0);
    const auto expected_d23 =
        Vector3::make(32.0 / 9.0, -128.0 / 9.0, 0.0);
    if (!expected_midpoint2 || !expected_midpoint3 ||
        !expected_d12 || !expected_d13 || !expected_d22 || !expected_d23) {
        return 1;
    }
    const auto d12 = curve2->first_derivative(0.5);
    const auto d13 = curve3->first_derivative(0.5);
    const auto d22 = curve2->second_derivative(0.5);
    const auto d23 = curve3->second_derivative(0.5);
    passed = require(
                 midpoint2 &&
                     close_point(*midpoint2, *expected_midpoint2, 8.0) &&
                     d12 && close_vector(*d12, *expected_d12, 16.0) &&
                     d22 && close_vector(*d22, *expected_d22, 32.0),
                 "2D independent rational jet fixture differs") &&
             passed;
    passed = require(
                 midpoint3 &&
                     close_point(*midpoint3, *expected_midpoint3, 8.0) &&
                     d13 && close_vector(*d13, *expected_d13, 16.0) &&
                     d23 && close_vector(*d23, *expected_d23, 32.0),
                 "3D independent rational jet fixture differs") &&
             passed;

    const auto q20 = Point2::make(-1.0, 2.0);
    const auto q21 = Point2::make(1.0, 5.0);
    const auto q22 = Point2::make(4.0, -2.0);
    const auto q30 = Point3::make(-1.0, 2.0, 0.0);
    const auto q31 = Point3::make(1.0, 5.0, 2.0);
    const auto q32 = Point3::make(4.0, -2.0, 4.0);
    if (!q20 || !q21 || !q22 || !q30 || !q31 || !q32) {
        return 1;
    }

    const auto equal2 =
        RationalQuadraticBezier2::make(*q20, *q21, *q22, 4.0, 4.0, 4.0);
    const auto equal3 =
        RationalQuadraticBezier3::make(*q30, *q31, *q32, 4.0, 4.0, 4.0);
    const auto c21 = lerp_point(*q20, *q21, 2.0 / 3.0);
    const auto c22 = lerp_point(*q21, *q22, 1.0 / 3.0);
    const auto c31 = lerp_point(*q30, *q31, 2.0 / 3.0);
    const auto c32 = lerp_point(*q31, *q32, 1.0 / 3.0);
    if (!equal2 || !equal3 || !c21 || !c22 || !c31 || !c32) {
        return 1;
    }
    const CubicBezier2 cubic2{*q20, *c21, *c22, *q22};
    const CubicBezier3 cubic3{*q30, *c31, *c32, *q32};

    constexpr std::array<double, 3> parity_parameters{0.125, 0.5, 0.875};
    for (const double parameter : parity_parameters) {
        const auto rv2 = equal2->evaluate(parameter);
        const auto cv2 = cubic2.evaluate(parameter);
        const auto rd12 = equal2->first_derivative(parameter);
        const auto cd12 = cubic2.first_derivative(parameter);
        const auto rd22 = equal2->second_derivative(parameter);
        const auto cd22 = cubic2.second_derivative(parameter);

        const auto rv3 = equal3->evaluate(parameter);
        const auto cv3 = cubic3.evaluate(parameter);
        const auto rd13 = equal3->first_derivative(parameter);
        const auto cd13 = cubic3.first_derivative(parameter);
        const auto rd23 = equal3->second_derivative(parameter);
        const auto cd23 = cubic3.second_derivative(parameter);

        passed = require(
                     rv2 && cv2 && close_point(*rv2, *cv2, 16.0) &&
                         rd12 && cd12 &&
                         close_vector(*rd12, *cd12, 32.0) &&
                         rd22 && cd22 &&
                         close_vector(*rd22, *cd22, 64.0),
                     "2D equal-weight degree-elevation parity differs") &&
                 passed;
        passed = require(
                     rv3 && cv3 && close_point(*rv3, *cv3, 16.0) &&
                         rd13 && cd13 &&
                         close_vector(*rd13, *cd13, 32.0) &&
                         rd23 && cd23 &&
                         close_vector(*rd23, *cd23, 64.0),
                     "3D equal-weight degree-elevation parity differs") &&
                 passed;
    }

    const auto reversed2 = curve2->reversed();
    const auto reversed3 = curve3->reversed();
    passed = require(
                 reversed2.reversed() == *curve2 &&
                     reversed3.reversed() == *curve3,
                 "rational reversal involution differs") &&
             passed;
    passed = require(
                 reversed2.control_points()[0] == curve2->control_points()[2] &&
                     reversed2.weights()[0] == curve2->weights()[2] &&
                     reversed3.control_points()[2] ==
                         curve3->control_points()[0] &&
                     reversed3.weights()[2] == curve3->weights()[0],
                 "rational reversal representation differs") &&
             passed;

    constexpr double reversal_parameter_value = 0.3;
    const auto mapped =
        reversed_parameter(curve2->parameter_domain(), reversal_parameter_value);
    if (!mapped) {
        return 1;
    }
    const auto original_value2 = curve2->evaluate(reversal_parameter_value);
    const auto reversed_value2 = reversed2.evaluate(*mapped);
    const auto original_d12 =
        curve2->first_derivative(reversal_parameter_value);
    const auto reversed_d12 = reversed2.first_derivative(*mapped);
    const auto original_d22 =
        curve2->second_derivative(reversal_parameter_value);
    const auto reversed_d22 = reversed2.second_derivative(*mapped);
    passed = require(
                 original_value2 && reversed_value2 &&
                     close_point(*original_value2, *reversed_value2, 16.0) &&
                     original_d12 && reversed_d12 &&
                     close_scalar(reversed_d12->x(), -original_d12->x(), 32.0) &&
                     close_scalar(reversed_d12->y(), -original_d12->y(), 32.0) &&
                     original_d22 && reversed_d22 &&
                     close_vector(*original_d22, *reversed_d22, 64.0),
                 "2D rational reversal covariance differs") &&
             passed;

    const auto scaled2 =
        RationalQuadraticBezier2::make(*p20, *p21, *p22, 8.0, 16.0, 8.0);
    const auto scaled3 =
        RationalQuadraticBezier3::make(*p30, *p31, *p32, 8.0, 16.0, 8.0);
    if (!scaled2 || !scaled3) {
        return 1;
    }
    for (const double parameter : parity_parameters) {
        const auto a2 = curve2->evaluate(parameter);
        const auto b2 = scaled2->evaluate(parameter);
        const auto ad12 = curve2->first_derivative(parameter);
        const auto bd12 = scaled2->first_derivative(parameter);
        const auto ad22 = curve2->second_derivative(parameter);
        const auto bd22 = scaled2->second_derivative(parameter);

        const auto a3 = curve3->evaluate(parameter);
        const auto b3 = scaled3->evaluate(parameter);
        const auto ad13 = curve3->first_derivative(parameter);
        const auto bd13 = scaled3->first_derivative(parameter);
        const auto ad23 = curve3->second_derivative(parameter);
        const auto bd23 = scaled3->second_derivative(parameter);

        passed = require(
                     a2 && b2 && *a2 == *b2 &&
                         ad12 && bd12 && *ad12 == *bd12 &&
                         ad22 && bd22 && *ad22 == *bd22,
                     "2D common power-of-two weight scale invariance differs") &&
                 passed;
        passed = require(
                     a3 && b3 && *a3 == *b3 &&
                         ad13 && bd13 && *ad13 == *bd13 &&
                         ad23 && bd23 && *ad23 == *bd23,
                     "3D common power-of-two weight scale invariance differs") &&
                 passed;
    }

    const auto constant_point2 = Point2::make(3.0, -2.0);
    const auto constant_point3 = Point3::make(3.0, -2.0, 7.0);
    if (!constant_point2 || !constant_point3) {
        return 1;
    }
    const auto constant2 = RationalQuadraticBezier2::make(
        *constant_point2, *constant_point2, *constant_point2, 1.0, 5.0, 2.0);
    const auto constant3 = RationalQuadraticBezier3::make(
        *constant_point3, *constant_point3, *constant_point3, 1.0, 5.0, 2.0);
    if (!constant2 || !constant3) {
        return 1;
    }
    const auto constant_value2 = constant2->evaluate(0.37);
    const auto constant_d12 = constant2->first_derivative(0.37);
    const auto constant_d22 = constant2->second_derivative(0.37);
    const auto constant_value3 = constant3->evaluate(0.37);
    const auto constant_d13 = constant3->first_derivative(0.37);
    const auto constant_d23 = constant3->second_derivative(0.37);
    passed = require(
                 constant_value2 && *constant_value2 == *constant_point2 &&
                     constant_d12 && constant_d12->x() == 0.0 &&
                     constant_d12->y() == 0.0 &&
                     constant_d22 && constant_d22->x() == 0.0 &&
                     constant_d22->y() == 0.0,
                 "2D constant rational curve semantics differ") &&
             passed;
    passed = require(
                 constant_value3 && *constant_value3 == *constant_point3 &&
                     constant_d13 && constant_d13->x() == 0.0 &&
                     constant_d13->y() == 0.0 && constant_d13->z() == 0.0 &&
                     constant_d23 && constant_d23->x() == 0.0 &&
                     constant_d23->y() == 0.0 && constant_d23->z() == 0.0,
                 "3D constant rational curve semantics differ") &&
             passed;

    const auto parabola0 = Point2::make(-1.0, 1.0);
    const auto parabola1 = Point2::make(0.0, -1.0);
    const auto parabola2 = Point2::make(1.0, 1.0);
    if (!parabola0 || !parabola1 || !parabola2) {
        return 1;
    }
    const auto parabola = RationalQuadraticBezier2::make(
        *parabola0, *parabola1, *parabola2, 1.0, 1.0, 1.0);
    if (!parabola) {
        return 1;
    }
    for (const double parameter : parity_parameters) {
        const auto value = parabola->evaluate(parameter);
        passed = require(
                     value &&
                         close_scalar(
                             value->y(), value->x() * value->x(), 2.0),
                     "independent parabolic conic residual differs") &&
                 passed;
    }

    const double quarter_weight = std::sqrt(0.5);
    const auto circle0 = Point2::make(1.0, 0.0);
    const auto circle1 = Point2::make(1.0, 1.0);
    const auto circle2 = Point2::make(0.0, 1.0);
    if (!circle0 || !circle1 || !circle2) {
        return 1;
    }
    const auto quarter_circle = RationalQuadraticBezier2::make(
        *circle0, *circle1, *circle2, 1.0, quarter_weight, 1.0);
    if (!quarter_circle) {
        return 1;
    }
    constexpr std::array<double, 3> circle_parameters{0.2, 0.5, 0.8};
    for (const double parameter : circle_parameters) {
        const auto value = quarter_circle->evaluate(parameter);
        if (!value) {
            passed = false;
            continue;
        }
        const double residual =
            value->x() * value->x() + value->y() * value->y();
        passed = require(
                     close_scalar(
                         residual,
                         1.0,
                         1.0,
                         8.0e-13,
                         8.0e-13),
                     "stored-double quarter-circle residual differs") &&
                 passed;
    }

    const auto bad_nan = curve2->evaluate(
        std::numeric_limits<double>::quiet_NaN());
    const auto bad_below = curve2->first_derivative(-0.1);
    const auto bad_above = curve3->second_derivative(1.1);
    passed = require(
                 !bad_nan &&
                     bad_nan.error() == CurveError::non_finite_parameter &&
                     !bad_below &&
                     bad_below.error() == CurveError::parameter_out_of_domain &&
                     !bad_above &&
                     bad_above.error() == CurveError::parameter_out_of_domain,
                 "rational parameter failure semantics differ") &&
             passed;

    const double maximum = std::numeric_limits<double>::max();
    const double minimum = std::numeric_limits<double>::denorm_min();
    const auto extreme0 = Point2::make(-maximum, 0.0);
    const auto extreme1 = Point2::make(0.0, maximum / 4.0);
    const auto extreme2 = Point2::make(maximum, 0.0);
    if (!extreme0 || !extreme1 || !extreme2) {
        return 1;
    }
    const auto extreme = RationalQuadraticBezier2::make(
        *extreme0, *extreme1, *extreme2, minimum, maximum, minimum);
    const auto derivative_extreme = RationalQuadraticBezier2::make(
        *extreme0, *extreme2, *extreme2, 1.0, 1.0, 1.0);
    if (!extreme || !derivative_extreme) {
        return 1;
    }
    const auto extreme_value = extreme->evaluate(0.5);
    const auto unrepresentable_d1 =
        derivative_extreme->first_derivative(0.0);
    passed = require(
                 extreme_value && std::isfinite(extreme_value->x()) &&
                     std::isfinite(extreme_value->y()),
                 "extreme rational value introduced avoidable overflow") &&
             passed;
    passed = require(
                 !unrepresentable_d1 &&
                     unrepresentable_d1.error() ==
                         CurveError::non_finite_result,
                 "unrepresentable rational derivative did not fail explicitly") &&
             passed;

    const auto translated0 = Point2::make(10.0, -7.0);
    const auto translated1 = Point2::make(12.0, -3.0);
    const auto translated2 = Point2::make(16.0, -7.0);
    if (!translated0 || !translated1 || !translated2) {
        return 1;
    }
    const auto translated = RationalQuadraticBezier2::make(
        *translated0, *translated1, *translated2, 1.0, 2.0, 1.0);
    if (!translated) {
        return 1;
    }
    const auto original_translation_value = curve2->evaluate(0.5);
    const auto translated_value = translated->evaluate(0.5);
    const auto original_translation_d1 = curve2->first_derivative(0.5);
    const auto translated_d1 = translated->first_derivative(0.5);
    const auto original_translation_d2 = curve2->second_derivative(0.5);
    const auto translated_d2 = translated->second_derivative(0.5);
    passed = require(
                 original_translation_value && translated_value &&
                     close_scalar(
                         translated_value->x() - original_translation_value->x(),
                         10.0,
                         32.0) &&
                     close_scalar(
                         translated_value->y() - original_translation_value->y(),
                         -7.0,
                         32.0) &&
                     original_translation_d1 && translated_d1 &&
                     close_vector(
                         *original_translation_d1, *translated_d1, 32.0) &&
                     original_translation_d2 && translated_d2 &&
                     close_vector(
                         *original_translation_d2, *translated_d2, 64.0),
                 "rational translation covariance differs") &&
             passed;

    const auto embedded0 = Point3::make(0.0, 0.0, 0.0);
    const auto embedded1 = Point3::make(2.0, 4.0, 0.0);
    const auto embedded2 = Point3::make(6.0, 0.0, 0.0);
    if (!embedded0 || !embedded1 || !embedded2) {
        return 1;
    }
    const auto embedded = RationalQuadraticBezier3::make(
        *embedded0, *embedded1, *embedded2, 1.0, 2.0, 1.0);
    if (!embedded) {
        return 1;
    }
    const auto embed_value2 = curve2->evaluate(0.37);
    const auto embed_value3 = embedded->evaluate(0.37);
    const auto embed_d12 = curve2->first_derivative(0.37);
    const auto embed_d13 = embedded->first_derivative(0.37);
    const auto embed_d22 = curve2->second_derivative(0.37);
    const auto embed_d23 = embedded->second_derivative(0.37);
    passed = require(
                 embed_value2 && embed_value3 &&
                     close_scalar(embed_value2->x(), embed_value3->x(), 16.0) &&
                     close_scalar(embed_value2->y(), embed_value3->y(), 16.0) &&
                     embed_value3->z() == 0.0 &&
                     embed_d12 && embed_d13 &&
                     close_scalar(embed_d12->x(), embed_d13->x(), 32.0) &&
                     close_scalar(embed_d12->y(), embed_d13->y(), 32.0) &&
                     embed_d13->z() == 0.0 &&
                     embed_d22 && embed_d23 &&
                     close_scalar(embed_d22->x(), embed_d23->x(), 64.0) &&
                     close_scalar(embed_d22->y(), embed_d23->y(), 64.0) &&
                     embed_d23->z() == 0.0,
                 "rational 2D/3D embedding parity differs") &&
             passed;

    const auto repeat_value_a = curve2->evaluate(0.625);
    const auto repeat_value_b = curve2->evaluate(0.625);
    const auto repeat_d1_a = curve3->first_derivative(0.625);
    const auto repeat_d1_b = curve3->first_derivative(0.625);
    const auto repeat_d2_a = curve3->second_derivative(0.625);
    const auto repeat_d2_b = curve3->second_derivative(0.625);
    passed = require(
                 repeat_value_a && repeat_value_b &&
                     *repeat_value_a == *repeat_value_b &&
                     repeat_d1_a && repeat_d1_b &&
                     *repeat_d1_a == *repeat_d1_b &&
                     repeat_d2_a && repeat_d2_b &&
                     *repeat_d2_a == *repeat_d2_b,
                 "rational repeated evidence is not deterministic") &&
             passed;

    return passed ? 0 : 1;
}
