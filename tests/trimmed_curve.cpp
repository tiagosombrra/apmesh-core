#include "apmesh/geometry/trimmed_curve.hpp"

#include "apmesh/core/numeric.hpp"
#include "apmesh/geometry/curve.hpp"
#include "apmesh/geometry/line_segment.hpp"
#include "apmesh/geometry/rational_bezier.hpp"

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
    const double reference_scale = 1.0) {
    const apmesh::core::ProximityPolicy policy{
        .absolute_tolerance = 3.0e-13 * reference_scale,
        .relative_tolerance = 3.0e-13,
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

class ExtremeDomainCurve2 {
public:
    [[nodiscard]] apmesh::core::CurveParameterDomain parameter_domain() const noexcept {
        const double maximum = std::numeric_limits<double>::max();
        const auto domain =
            apmesh::core::CurveParameterDomain::make(maximum / 2.0, maximum);
        return *domain;
    }

    [[nodiscard]] std::expected<apmesh::core::Point2, apmesh::core::CurveError>
    evaluate(const double parameter) const noexcept {
        const auto contained = parameter_domain().contains(parameter);
        if (!contained.has_value()) {
            return std::unexpected{contained.error()};
        }
        if (!*contained) {
            return std::unexpected{
                apmesh::core::CurveError::parameter_out_of_domain};
        }

        const double maximum = std::numeric_limits<double>::max();
        const auto point =
            apmesh::core::Point2::make(parameter / maximum, 0.0);
        if (!point.has_value()) {
            return std::unexpected{apmesh::core::CurveError::non_finite_result};
        }
        return *point;
    }

    [[nodiscard]] std::expected<apmesh::core::Vector2, apmesh::core::CurveError>
    first_derivative(const double parameter) const noexcept {
        const auto contained = parameter_domain().contains(parameter);
        if (!contained.has_value()) {
            return std::unexpected{contained.error()};
        }
        if (!*contained) {
            return std::unexpected{
                apmesh::core::CurveError::parameter_out_of_domain};
        }

        const auto vector = apmesh::core::Vector2::make(
            1.0 / std::numeric_limits<double>::max(), 0.0);
        if (!vector.has_value()) {
            return std::unexpected{apmesh::core::CurveError::non_finite_result};
        }
        return *vector;
    }

    [[nodiscard]] std::expected<apmesh::core::Vector2, apmesh::core::CurveError>
    second_derivative(const double parameter) const noexcept {
        const auto contained = parameter_domain().contains(parameter);
        if (!contained.has_value()) {
            return std::unexpected{contained.error()};
        }
        if (!*contained) {
            return std::unexpected{
                apmesh::core::CurveError::parameter_out_of_domain};
        }

        return *apmesh::core::Vector2::make(0.0, 0.0);
    }

    [[nodiscard]] ExtremeDomainCurve2 reversed() const noexcept {
        return *this;
    }

    [[nodiscard]] bool operator==(
        const ExtremeDomainCurve2&) const noexcept = default;
};

} // namespace

int main() {
    using apmesh::core::BoundedParametricCurve2;
    using apmesh::core::BoundedParametricCurve3;
    using apmesh::core::CubicBezier2;
    using apmesh::core::CubicBezier3;
    using apmesh::core::CurveError;
    using apmesh::core::LineSegment2;
    using apmesh::core::LineSegment3;
    using apmesh::core::Point2;
    using apmesh::core::Point3;
    using apmesh::core::RationalQuadraticBezier2;
    using apmesh::core::RationalQuadraticBezier3;
    using apmesh::core::TrimmedCurve2;
    using apmesh::core::TrimmedCurve3;
    using apmesh::core::TrimmedCurveConstructionError;
    using apmesh::core::reversed_parameter;

    static_assert(BoundedParametricCurve2<TrimmedCurve2<LineSegment2>>);
    static_assert(BoundedParametricCurve3<TrimmedCurve3<LineSegment3>>);
    static_assert(BoundedParametricCurve2<TrimmedCurve2<CubicBezier2>>);
    static_assert(BoundedParametricCurve3<TrimmedCurve3<CubicBezier3>>);
    static_assert(
        BoundedParametricCurve2<TrimmedCurve2<RationalQuadraticBezier2>>);
    static_assert(
        BoundedParametricCurve3<TrimmedCurve3<RationalQuadraticBezier3>>);
    static_assert(BoundedParametricCurve2<TrimmedCurve2<ExtremeDomainCurve2>>);

    bool passed = true;

    const auto line20 = Point2::make(0.0, 0.0);
    const auto line21 = Point2::make(10.0, 4.0);
    const auto line30 = Point3::make(0.0, 0.0, 0.0);
    const auto line31 = Point3::make(10.0, 4.0, 0.0);
    if (!line20 || !line21 || !line30 || !line31) {
        return 1;
    }

    const LineSegment2 line2{*line20, *line21};
    const LineSegment3 line3{*line30, *line31};

    const auto nan_source = TrimmedCurve2<LineSegment2>::make(
        line2,
        std::numeric_limits<double>::quiet_NaN(),
        0.8);
    const auto inf_target = TrimmedCurve2<LineSegment2>::make(
        line2,
        0.2,
        std::numeric_limits<double>::infinity());
    const auto source_out =
        TrimmedCurve2<LineSegment2>::make(line2, -0.1, 0.8);
    const auto target_out =
        TrimmedCurve2<LineSegment2>::make(line2, 0.2, 1.1);
    const auto zero_width =
        TrimmedCurve2<LineSegment2>::make(line2, 0.5, 0.5);

    passed = require(
                 !nan_source &&
                     nan_source.error() ==
                         TrimmedCurveConstructionError::
                             non_finite_source_parameter,
                 "non-finite trim source error differs") &&
             passed;
    passed = require(
                 !inf_target &&
                     inf_target.error() ==
                         TrimmedCurveConstructionError::
                             non_finite_target_parameter,
                 "non-finite trim target error differs") &&
             passed;
    passed = require(
                 !source_out &&
                     source_out.error() ==
                         TrimmedCurveConstructionError::
                             source_parameter_out_of_domain &&
                     !target_out &&
                     target_out.error() ==
                         TrimmedCurveConstructionError::
                             target_parameter_out_of_domain,
                 "trim basis-domain construction errors differ") &&
             passed;
    passed = require(
                 !zero_width &&
                     zero_width.error() ==
                         TrimmedCurveConstructionError::zero_width_trim,
                 "zero-width trim error differs") &&
             passed;

    const auto forward_line =
        TrimmedCurve2<LineSegment2>::make(line2, 0.2, 0.8);
    const auto reverse_line =
        TrimmedCurve2<LineSegment2>::make(line2, 0.8, 0.2);
    const auto forward_line3 =
        TrimmedCurve3<LineSegment3>::make(line3, 0.2, 0.8);
    const auto reverse_line3 =
        TrimmedCurve3<LineSegment3>::make(line3, 0.8, 0.2);
    if (!forward_line || !reverse_line || !forward_line3 || !reverse_line3) {
        return 1;
    }

    const auto forward_domain = forward_line->parameter_domain();
    const auto reverse_domain = reverse_line->parameter_domain();
    passed = require(
                 forward_domain.lower() == 0.2 &&
                     forward_domain.upper() == 0.8 &&
                     reverse_domain == forward_domain,
                 "trimmed line domain differs") &&
             passed;
    passed = require(
                 forward_line->source_parameter() == 0.2 &&
                     forward_line->target_parameter() == 0.8 &&
                     reverse_line->source_parameter() == 0.8 &&
                     reverse_line->target_parameter() == 0.2,
                 "trimmed line orientation storage differs") &&
             passed;

    constexpr std::array<double, 3> forward_parameters{0.2, 0.5, 0.8};
    for (const double parameter : forward_parameters) {
        const auto trim_value = forward_line->evaluate(parameter);
        const auto basis_value = line2.evaluate(parameter);
        const auto trim_d1 = forward_line->first_derivative(parameter);
        const auto basis_d1 = line2.first_derivative(parameter);
        const auto trim_d2 = forward_line->second_derivative(parameter);
        const auto basis_d2 = line2.second_derivative(parameter);

        passed = require(
                     trim_value && basis_value &&
                         close_point(*trim_value, *basis_value, 16.0) &&
                         trim_d1 && basis_d1 &&
                         close_vector(*trim_d1, *basis_d1, 16.0) &&
                         trim_d2 && basis_d2 &&
                         close_vector(*trim_d2, *basis_d2, 16.0),
                     "forward line trim does not match basis") &&
                 passed;
    }

    const auto reverse_lower = reverse_line->evaluate(0.2);
    const auto basis_at_source = line2.evaluate(0.8);
    const auto reverse_upper = reverse_line->evaluate(0.8);
    const auto basis_at_target = line2.evaluate(0.2);
    passed = require(
                 reverse_lower && basis_at_source &&
                     close_point(*reverse_lower, *basis_at_source, 16.0) &&
                     reverse_upper && basis_at_target &&
                     close_point(*reverse_upper, *basis_at_target, 16.0),
                 "reverse trim oriented endpoints differ") &&
             passed;

    constexpr double reverse_query = 0.35;
    const auto mapped_reverse =
        reversed_parameter(reverse_line->parameter_domain(), reverse_query);
    if (!mapped_reverse) {
        return 1;
    }
    const auto reverse_value = reverse_line->evaluate(reverse_query);
    const auto mapped_basis_value = line2.evaluate(*mapped_reverse);
    const auto reverse_d1 = reverse_line->first_derivative(reverse_query);
    const auto mapped_basis_d1 = line2.first_derivative(*mapped_reverse);
    const auto reverse_d2 = reverse_line->second_derivative(reverse_query);
    const auto mapped_basis_d2 = line2.second_derivative(*mapped_reverse);
    passed = require(
                 reverse_value && mapped_basis_value &&
                     close_point(*reverse_value, *mapped_basis_value, 16.0),
                 "reverse trim value mapping differs") &&
             passed;
    passed = require(
                 reverse_d1 && mapped_basis_d1 &&
                     close_scalar(
                         reverse_d1->x(), -mapped_basis_d1->x(), 16.0) &&
                     close_scalar(
                         reverse_d1->y(), -mapped_basis_d1->y(), 16.0),
                 "reverse trim D1 covariance differs") &&
             passed;
    passed = require(
                 reverse_d2 && mapped_basis_d2 &&
                     close_vector(*reverse_d2, *mapped_basis_d2, 16.0),
                 "reverse trim D2 covariance differs") &&
             passed;

    const auto mapped_reverse3 =
        reversed_parameter(reverse_line3->parameter_domain(), reverse_query);
    if (!mapped_reverse3) {
        return 1;
    }
    const auto reverse_value3 = reverse_line3->evaluate(reverse_query);
    const auto mapped_basis_value3 = line3.evaluate(*mapped_reverse3);
    const auto reverse_d13 = reverse_line3->first_derivative(reverse_query);
    const auto mapped_basis_d13 = line3.first_derivative(*mapped_reverse3);
    const auto reverse_d23 = reverse_line3->second_derivative(reverse_query);
    const auto mapped_basis_d23 = line3.second_derivative(*mapped_reverse3);
    passed = require(
                 reverse_value3 && mapped_basis_value3 &&
                     close_point(*reverse_value3, *mapped_basis_value3, 16.0) &&
                     reverse_d13 && mapped_basis_d13 &&
                     close_scalar(
                         reverse_d13->x(), -mapped_basis_d13->x(), 16.0) &&
                     close_scalar(
                         reverse_d13->y(), -mapped_basis_d13->y(), 16.0) &&
                     close_scalar(
                         reverse_d13->z(), -mapped_basis_d13->z(), 16.0) &&
                     reverse_d23 && mapped_basis_d23 &&
                     close_vector(*reverse_d23, *mapped_basis_d23, 16.0),
                 "reverse 3D line trim covariance differs") &&
             passed;

    const auto reversed_forward = forward_line->reversed();
    passed = require(
                 reversed_forward == *reverse_line &&
                     reversed_forward.reversed() == *forward_line,
                 "trim reversal representation/involution differs") &&
             passed;

    constexpr double covariance_parameter = 0.3;
    const auto covariance_mapped =
        reversed_parameter(forward_line->parameter_domain(), covariance_parameter);
    if (!covariance_mapped) {
        return 1;
    }
    const auto original_value =
        forward_line->evaluate(covariance_parameter);
    const auto reversed_value =
        reversed_forward.evaluate(*covariance_mapped);
    const auto original_d1 =
        forward_line->first_derivative(covariance_parameter);
    const auto reversed_d1 =
        reversed_forward.first_derivative(*covariance_mapped);
    const auto original_d2 =
        forward_line->second_derivative(covariance_parameter);
    const auto reversed_d2 =
        reversed_forward.second_derivative(*covariance_mapped);
    passed = require(
                 original_value && reversed_value &&
                     close_point(*original_value, *reversed_value, 16.0) &&
                     original_d1 && reversed_d1 &&
                     close_scalar(reversed_d1->x(), -original_d1->x(), 16.0) &&
                     close_scalar(reversed_d1->y(), -original_d1->y(), 16.0) &&
                     original_d2 && reversed_d2 &&
                     close_vector(*original_d2, *reversed_d2, 16.0),
                 "trim common reversal covariance differs") &&
             passed;

    const auto cubic20 = Point2::make(0.0, 0.0);
    const auto cubic21 = Point2::make(1.0, 4.0);
    const auto cubic22 = Point2::make(4.0, 4.0);
    const auto cubic23 = Point2::make(6.0, 0.0);
    const auto cubic30 = Point3::make(0.0, 0.0, 0.0);
    const auto cubic31 = Point3::make(1.0, 4.0, 0.0);
    const auto cubic32 = Point3::make(4.0, 4.0, 0.0);
    const auto cubic33 = Point3::make(6.0, 0.0, 0.0);
    if (!cubic20 || !cubic21 || !cubic22 || !cubic23 ||
        !cubic30 || !cubic31 || !cubic32 || !cubic33) {
        return 1;
    }
    const CubicBezier2 cubic2{*cubic20, *cubic21, *cubic22, *cubic23};
    const CubicBezier3 cubic3{*cubic30, *cubic31, *cubic32, *cubic33};

    const auto cubic_forward =
        TrimmedCurve2<CubicBezier2>::make(cubic2, 0.1, 0.9);
    const auto cubic_reverse =
        TrimmedCurve2<CubicBezier2>::make(cubic2, 0.9, 0.1);
    const auto cubic_forward3 =
        TrimmedCurve3<CubicBezier3>::make(cubic3, 0.1, 0.9);
    const auto cubic_reverse3 =
        TrimmedCurve3<CubicBezier3>::make(cubic3, 0.9, 0.1);
    if (!cubic_forward || !cubic_reverse || !cubic_forward3 ||
        !cubic_reverse3) {
        return 1;
    }

    constexpr double cubic_query = 0.3;
    const auto cubic_forward_value = cubic_forward->evaluate(cubic_query);
    const auto cubic_basis_value = cubic2.evaluate(cubic_query);
    const auto cubic_forward_d1 =
        cubic_forward->first_derivative(cubic_query);
    const auto cubic_basis_d1 = cubic2.first_derivative(cubic_query);
    const auto cubic_forward_d2 =
        cubic_forward->second_derivative(cubic_query);
    const auto cubic_basis_d2 = cubic2.second_derivative(cubic_query);
    passed = require(
                 cubic_forward_value && cubic_basis_value &&
                     close_point(*cubic_forward_value, *cubic_basis_value, 16.0) &&
                     cubic_forward_d1 && cubic_basis_d1 &&
                     close_vector(*cubic_forward_d1, *cubic_basis_d1, 32.0) &&
                     cubic_forward_d2 && cubic_basis_d2 &&
                     close_vector(*cubic_forward_d2, *cubic_basis_d2, 64.0),
                 "forward cubic trim parity differs") &&
             passed;

    const auto cubic_reverse_mapped =
        reversed_parameter(cubic_reverse->parameter_domain(), cubic_query);
    if (!cubic_reverse_mapped) {
        return 1;
    }
    const auto cubic_reverse_value = cubic_reverse->evaluate(cubic_query);
    const auto cubic_mapped_value = cubic2.evaluate(*cubic_reverse_mapped);
    const auto cubic_reverse_d1 =
        cubic_reverse->first_derivative(cubic_query);
    const auto cubic_mapped_d1 =
        cubic2.first_derivative(*cubic_reverse_mapped);
    const auto cubic_reverse_d2 =
        cubic_reverse->second_derivative(cubic_query);
    const auto cubic_mapped_d2 =
        cubic2.second_derivative(*cubic_reverse_mapped);
    passed = require(
                 cubic_reverse_value && cubic_mapped_value &&
                     close_point(*cubic_reverse_value, *cubic_mapped_value, 16.0) &&
                     cubic_reverse_d1 && cubic_mapped_d1 &&
                     close_scalar(
                         cubic_reverse_d1->x(), -cubic_mapped_d1->x(), 32.0) &&
                     close_scalar(
                         cubic_reverse_d1->y(), -cubic_mapped_d1->y(), 32.0) &&
                     cubic_reverse_d2 && cubic_mapped_d2 &&
                     close_vector(*cubic_reverse_d2, *cubic_mapped_d2, 64.0),
                 "reverse cubic trim parity differs") &&
             passed;

    const auto cubic_reverse_mapped3 =
        reversed_parameter(cubic_reverse3->parameter_domain(), cubic_query);
    if (!cubic_reverse_mapped3) {
        return 1;
    }
    const auto cubic_reverse_value3 = cubic_reverse3->evaluate(cubic_query);
    const auto cubic_mapped_value3 = cubic3.evaluate(*cubic_reverse_mapped3);
    const auto cubic_reverse_d13 =
        cubic_reverse3->first_derivative(cubic_query);
    const auto cubic_mapped_d13 =
        cubic3.first_derivative(*cubic_reverse_mapped3);
    const auto cubic_reverse_d23 =
        cubic_reverse3->second_derivative(cubic_query);
    const auto cubic_mapped_d23 =
        cubic3.second_derivative(*cubic_reverse_mapped3);
    passed = require(
                 cubic_reverse_value3 && cubic_mapped_value3 &&
                     close_point(
                         *cubic_reverse_value3, *cubic_mapped_value3, 16.0) &&
                     cubic_reverse_d13 && cubic_mapped_d13 &&
                     close_scalar(
                         cubic_reverse_d13->x(), -cubic_mapped_d13->x(), 32.0) &&
                     close_scalar(
                         cubic_reverse_d13->y(), -cubic_mapped_d13->y(), 32.0) &&
                     close_scalar(
                         cubic_reverse_d13->z(), -cubic_mapped_d13->z(), 32.0) &&
                     cubic_reverse_d23 && cubic_mapped_d23 &&
                     close_vector(
                         *cubic_reverse_d23, *cubic_mapped_d23, 64.0),
                 "reverse 3D cubic trim parity differs") &&
             passed;

    const auto rational20 = Point2::make(1.0, 0.0);
    const auto rational21 = Point2::make(1.0, 1.0);
    const auto rational22 = Point2::make(0.0, 1.0);
    const auto rational30 = Point3::make(1.0, 0.0, 0.0);
    const auto rational31 = Point3::make(1.0, 1.0, 0.0);
    const auto rational32 = Point3::make(0.0, 1.0, 0.0);
    if (!rational20 || !rational21 || !rational22 ||
        !rational30 || !rational31 || !rational32) {
        return 1;
    }

    const double middle_weight = std::sqrt(0.5);
    const auto rational2 = RationalQuadraticBezier2::make(
        *rational20, *rational21, *rational22, 1.0, middle_weight, 1.0);
    const auto rational3 = RationalQuadraticBezier3::make(
        *rational30, *rational31, *rational32, 1.0, middle_weight, 1.0);
    if (!rational2 || !rational3) {
        return 1;
    }

    const auto rational_forward =
        TrimmedCurve2<RationalQuadraticBezier2>::make(*rational2, 0.15, 0.85);
    const auto rational_reverse =
        TrimmedCurve2<RationalQuadraticBezier2>::make(*rational2, 0.85, 0.15);
    const auto rational_forward3 =
        TrimmedCurve3<RationalQuadraticBezier3>::make(*rational3, 0.15, 0.85);
    const auto rational_reverse3 =
        TrimmedCurve3<RationalQuadraticBezier3>::make(*rational3, 0.85, 0.15);
    if (!rational_forward || !rational_reverse || !rational_forward3 ||
        !rational_reverse3) {
        return 1;
    }

    constexpr double rational_query = 0.4;
    const auto rational_forward_value =
        rational_forward->evaluate(rational_query);
    const auto rational_basis_value = rational2->evaluate(rational_query);
    const auto rational_forward_d1 =
        rational_forward->first_derivative(rational_query);
    const auto rational_basis_d1 =
        rational2->first_derivative(rational_query);
    const auto rational_forward_d2 =
        rational_forward->second_derivative(rational_query);
    const auto rational_basis_d2 =
        rational2->second_derivative(rational_query);
    passed = require(
                 rational_forward_value && rational_basis_value &&
                     close_point(
                         *rational_forward_value, *rational_basis_value, 4.0) &&
                     rational_forward_d1 && rational_basis_d1 &&
                     close_vector(
                         *rational_forward_d1, *rational_basis_d1, 8.0) &&
                     rational_forward_d2 && rational_basis_d2 &&
                     close_vector(
                         *rational_forward_d2, *rational_basis_d2, 16.0),
                 "forward rational trim parity differs") &&
             passed;

    const auto rational_reverse_mapped =
        reversed_parameter(rational_reverse->parameter_domain(), rational_query);
    if (!rational_reverse_mapped) {
        return 1;
    }
    const auto rational_reverse_value =
        rational_reverse->evaluate(rational_query);
    const auto rational_mapped_value =
        rational2->evaluate(*rational_reverse_mapped);
    const auto rational_reverse_d1 =
        rational_reverse->first_derivative(rational_query);
    const auto rational_mapped_d1 =
        rational2->first_derivative(*rational_reverse_mapped);
    const auto rational_reverse_d2 =
        rational_reverse->second_derivative(rational_query);
    const auto rational_mapped_d2 =
        rational2->second_derivative(*rational_reverse_mapped);
    passed = require(
                 rational_reverse_value && rational_mapped_value &&
                     close_point(
                         *rational_reverse_value, *rational_mapped_value, 4.0) &&
                     rational_reverse_d1 && rational_mapped_d1 &&
                     close_scalar(
                         rational_reverse_d1->x(),
                         -rational_mapped_d1->x(),
                         8.0) &&
                     close_scalar(
                         rational_reverse_d1->y(),
                         -rational_mapped_d1->y(),
                         8.0) &&
                     rational_reverse_d2 && rational_mapped_d2 &&
                     close_vector(
                         *rational_reverse_d2, *rational_mapped_d2, 16.0),
                 "reverse rational trim parity differs") &&
             passed;

    const auto rational_reverse_mapped3 =
        reversed_parameter(rational_reverse3->parameter_domain(), rational_query);
    if (!rational_reverse_mapped3) {
        return 1;
    }
    const auto rational_reverse_value3 =
        rational_reverse3->evaluate(rational_query);
    const auto rational_mapped_value3 =
        rational3->evaluate(*rational_reverse_mapped3);
    const auto rational_reverse_d13 =
        rational_reverse3->first_derivative(rational_query);
    const auto rational_mapped_d13 =
        rational3->first_derivative(*rational_reverse_mapped3);
    const auto rational_reverse_d23 =
        rational_reverse3->second_derivative(rational_query);
    const auto rational_mapped_d23 =
        rational3->second_derivative(*rational_reverse_mapped3);
    passed = require(
                 rational_reverse_value3 && rational_mapped_value3 &&
                     close_point(
                         *rational_reverse_value3, *rational_mapped_value3, 4.0) &&
                     rational_reverse_d13 && rational_mapped_d13 &&
                     close_scalar(
                         rational_reverse_d13->x(),
                         -rational_mapped_d13->x(),
                         8.0) &&
                     close_scalar(
                         rational_reverse_d13->y(),
                         -rational_mapped_d13->y(),
                         8.0) &&
                     close_scalar(
                         rational_reverse_d13->z(),
                         -rational_mapped_d13->z(),
                         8.0) &&
                     rational_reverse_d23 && rational_mapped_d23 &&
                     close_vector(
                         *rational_reverse_d23, *rational_mapped_d23, 16.0),
                 "reverse 3D rational trim parity differs") &&
             passed;

    const auto full_line =
        TrimmedCurve2<LineSegment2>::make(line2, 0.0, 1.0);
    const auto full_cubic =
        TrimmedCurve2<CubicBezier2>::make(cubic2, 0.0, 1.0);
    const auto full_rational =
        TrimmedCurve2<RationalQuadraticBezier2>::make(*rational2, 0.0, 1.0);
    if (!full_line || !full_cubic || !full_rational) {
        return 1;
    }

    constexpr double full_query = 0.37;
    const auto full_line_value = full_line->evaluate(full_query);
    const auto line_basis_value = line2.evaluate(full_query);
    const auto full_cubic_value = full_cubic->evaluate(full_query);
    const auto cubic_full_basis = cubic2.evaluate(full_query);
    const auto full_rational_value = full_rational->evaluate(full_query);
    const auto rational_full_basis = rational2->evaluate(full_query);
    passed = require(
                 full_line_value && line_basis_value &&
                     close_point(*full_line_value, *line_basis_value, 16.0) &&
                     full_cubic_value && cubic_full_basis &&
                     close_point(*full_cubic_value, *cubic_full_basis, 16.0) &&
                     full_rational_value && rational_full_basis &&
                     close_point(
                         *full_rational_value, *rational_full_basis, 4.0),
                 "full-domain forward trim identity differs") &&
             passed;

    const auto full_reverse_cubic =
        TrimmedCurve2<CubicBezier2>::make(cubic2, 1.0, 0.0);
    const auto reversed_cubic_basis = cubic2.reversed();
    if (!full_reverse_cubic) {
        return 1;
    }
    const auto full_reverse_value =
        full_reverse_cubic->evaluate(full_query);
    const auto reversed_basis_value =
        reversed_cubic_basis.evaluate(full_query);
    const auto full_reverse_d1 =
        full_reverse_cubic->first_derivative(full_query);
    const auto reversed_basis_d1 =
        reversed_cubic_basis.first_derivative(full_query);
    const auto full_reverse_d2 =
        full_reverse_cubic->second_derivative(full_query);
    const auto reversed_basis_d2 =
        reversed_cubic_basis.second_derivative(full_query);
    passed = require(
                 full_reverse_value && reversed_basis_value &&
                     close_point(
                         *full_reverse_value, *reversed_basis_value, 16.0) &&
                     full_reverse_d1 && reversed_basis_d1 &&
                     close_vector(
                         *full_reverse_d1, *reversed_basis_d1, 32.0) &&
                     full_reverse_d2 && reversed_basis_d2 &&
                     close_vector(
                         *full_reverse_d2, *reversed_basis_d2, 64.0),
                 "full-domain reverse trim/basis reversal parity differs") &&
             passed;

    const auto bad_query = forward_line->evaluate(
        std::numeric_limits<double>::quiet_NaN());
    const auto below_trim = forward_line->first_derivative(0.1);
    const auto above_trim = forward_line->second_derivative(0.9);
    passed = require(
                 !bad_query &&
                     bad_query.error() == CurveError::non_finite_parameter &&
                     !below_trim &&
                     below_trim.error() == CurveError::parameter_out_of_domain &&
                     !above_trim &&
                     above_trim.error() == CurveError::parameter_out_of_domain,
                 "trim query failure semantics differ") &&
             passed;

    const double maximum = std::numeric_limits<double>::max();
    const ExtremeDomainCurve2 extreme_basis{};
    const auto extreme_trim =
        TrimmedCurve2<ExtremeDomainCurve2>::make(
            extreme_basis, maximum, maximum / 2.0);
    if (!extreme_trim) {
        return 1;
    }
    const auto extreme_lower = extreme_trim->evaluate(maximum / 2.0);
    const auto extreme_upper = extreme_trim->evaluate(maximum);
    passed = require(
                 extreme_lower && extreme_lower->x() == 1.0 &&
                     extreme_upper && extreme_upper->x() == 0.5,
                 "extreme reverse-domain mapping overflowed or differed") &&
             passed;

    constexpr double embedding_parameter = 0.45;
    const auto line2_value = forward_line->evaluate(embedding_parameter);
    const auto line3_value = forward_line3->evaluate(embedding_parameter);
    const auto line2_d1 = forward_line->first_derivative(embedding_parameter);
    const auto line3_d1 = forward_line3->first_derivative(embedding_parameter);
    const auto line2_d2 = forward_line->second_derivative(embedding_parameter);
    const auto line3_d2 = forward_line3->second_derivative(embedding_parameter);
    passed = require(
                 line2_value && line3_value &&
                     close_scalar(line2_value->x(), line3_value->x(), 16.0) &&
                     close_scalar(line2_value->y(), line3_value->y(), 16.0) &&
                     line3_value->z() == 0.0 &&
                     line2_d1 && line3_d1 &&
                     close_scalar(line2_d1->x(), line3_d1->x(), 16.0) &&
                     close_scalar(line2_d1->y(), line3_d1->y(), 16.0) &&
                     line3_d1->z() == 0.0 &&
                     line2_d2 && line3_d2 &&
                     line3_d2->x() == 0.0 && line3_d2->y() == 0.0 &&
                     line3_d2->z() == 0.0,
                 "line trim 2D/3D embedding parity differs") &&
             passed;

    const auto cubic2_value = cubic_forward->evaluate(embedding_parameter);
    const auto cubic3_value = cubic_forward3->evaluate(embedding_parameter);
    const auto cubic2_d1 = cubic_forward->first_derivative(embedding_parameter);
    const auto cubic3_d1 = cubic_forward3->first_derivative(embedding_parameter);
    const auto cubic2_d2 = cubic_forward->second_derivative(embedding_parameter);
    const auto cubic3_d2 = cubic_forward3->second_derivative(embedding_parameter);
    passed = require(
                 cubic2_value && cubic3_value &&
                     close_scalar(cubic2_value->x(), cubic3_value->x(), 16.0) &&
                     close_scalar(cubic2_value->y(), cubic3_value->y(), 16.0) &&
                     cubic3_value->z() == 0.0 &&
                     cubic2_d1 && cubic3_d1 &&
                     close_scalar(cubic2_d1->x(), cubic3_d1->x(), 32.0) &&
                     close_scalar(cubic2_d1->y(), cubic3_d1->y(), 32.0) &&
                     cubic3_d1->z() == 0.0 &&
                     cubic2_d2 && cubic3_d2 &&
                     close_scalar(cubic2_d2->x(), cubic3_d2->x(), 64.0) &&
                     close_scalar(cubic2_d2->y(), cubic3_d2->y(), 64.0) &&
                     cubic3_d2->z() == 0.0,
                 "cubic trim 2D/3D embedding parity differs") &&
             passed;

    const auto rational2_value =
        rational_forward->evaluate(embedding_parameter);
    const auto rational3_value =
        rational_forward3->evaluate(embedding_parameter);
    const auto rational2_d1 =
        rational_forward->first_derivative(embedding_parameter);
    const auto rational3_d1 =
        rational_forward3->first_derivative(embedding_parameter);
    const auto rational2_d2 =
        rational_forward->second_derivative(embedding_parameter);
    const auto rational3_d2 =
        rational_forward3->second_derivative(embedding_parameter);
    passed = require(
                 rational2_value && rational3_value &&
                     close_scalar(
                         rational2_value->x(), rational3_value->x(), 4.0) &&
                     close_scalar(
                         rational2_value->y(), rational3_value->y(), 4.0) &&
                     rational3_value->z() == 0.0 &&
                     rational2_d1 && rational3_d1 &&
                     close_scalar(
                         rational2_d1->x(), rational3_d1->x(), 8.0) &&
                     close_scalar(
                         rational2_d1->y(), rational3_d1->y(), 8.0) &&
                     rational3_d1->z() == 0.0 &&
                     rational2_d2 && rational3_d2 &&
                     close_scalar(
                         rational2_d2->x(), rational3_d2->x(), 16.0) &&
                     close_scalar(
                         rational2_d2->y(), rational3_d2->y(), 16.0) &&
                     rational3_d2->z() == 0.0,
                 "rational trim 2D/3D embedding parity differs") &&
             passed;

    const auto repeat_value_a = rational_reverse->evaluate(0.55);
    const auto repeat_value_b = rational_reverse->evaluate(0.55);
    const auto repeat_d1_a = cubic_reverse->first_derivative(0.55);
    const auto repeat_d1_b = cubic_reverse->first_derivative(0.55);
    passed = require(
                 repeat_value_a && repeat_value_b &&
                     *repeat_value_a == *repeat_value_b &&
                     repeat_d1_a && repeat_d1_b &&
                     *repeat_d1_a == *repeat_d1_b,
                 "trim repeated evidence is not deterministic") &&
             passed;

    return passed ? 0 : 1;
}
