#include "apmesh/geometry/trimmed_surface.hpp"

#include "apmesh/geometry/coons_surface.hpp"
#include "apmesh/geometry/curve.hpp"
#include "apmesh/geometry/nurbs_surface.hpp"
#include "apmesh/geometry/surface.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdio>
#include <expected>
#include <limits>
#include <string_view>
#include <type_traits>
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
    const double absolute = 8.0e-12,
    const double relative = 8.0e-12) {
    const double magnitude =
        std::max({1.0, std::abs(lhs), std::abs(rhs), std::abs(scale)});
    return std::abs(lhs - rhs) <= absolute * std::abs(scale) +
                                      relative * magnitude;
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
    const apmesh::core::Vector3& lhs,
    const apmesh::core::Vector3& rhs,
    const double scale = 1.0) {
    return close_scalar(lhs.x(), rhs.x(), scale) &&
           close_scalar(lhs.y(), rhs.y(), scale) &&
           close_scalar(lhs.z(), rhs.z(), scale);
}

apmesh::core::Point3 point(
    const double x,
    const double y,
    const double z) {
    return *apmesh::core::Point3::make(x, y, z);
}

apmesh::core::BicubicBezierPatch3::ControlNet make_controls() {
    const auto seed = point(0.0, 0.0, 0.0);
    apmesh::core::BicubicBezierPatch3::ControlNet controls{{
        {{seed, seed, seed, seed}},
        {{seed, seed, seed, seed}},
        {{seed, seed, seed, seed}},
        {{seed, seed, seed, seed}},
    }};
    for (std::size_t i = 0; i < 4U; ++i) {
        for (std::size_t j = 0; j < 4U; ++j) {
            const double u = static_cast<double>(i) / 3.0;
            const double v = static_cast<double>(j) / 3.0;
            controls[i][j] = point(
                -2.0 + 4.0 * u + 0.5 * v,
                1.0 - u + 3.0 * v,
                0.75 + 2.0 * u - v + u * v);
        }
    }
    return controls;
}

apmesh::core::RationalBicubicBezierPatch3::WeightNet make_weights() {
    return {{
        {{1.0, 2.0, 0.75, 1.5}},
        {{1.25, 0.5, 2.5, 1.0}},
        {{0.8, 1.8, 0.65, 2.2}},
        {{1.4, 0.9, 1.6, 0.7}},
    }};
}

std::vector<apmesh::core::Point3> flatten(
    const apmesh::core::BicubicBezierPatch3::ControlNet& controls) {
    std::vector<apmesh::core::Point3> result;
    result.reserve(16U);
    for (const auto& row : controls) {
        result.insert(result.end(), row.begin(), row.end());
    }
    return result;
}

std::vector<double> flatten(
    const apmesh::core::RationalBicubicBezierPatch3::WeightNet& weights) {
    std::vector<double> result;
    result.reserve(16U);
    for (const auto& row : weights) {
        result.insert(result.end(), row.begin(), row.end());
    }
    return result;
}

apmesh::core::CubicBezier3 line_curve(
    const apmesh::core::Point3& a,
    const apmesh::core::Point3& b) {
    const auto p1 = point(
        a.x() + (b.x() - a.x()) / 3.0,
        a.y() + (b.y() - a.y()) / 3.0,
        a.z() + (b.z() - a.z()) / 3.0);
    const auto p2 = point(
        a.x() + 2.0 * (b.x() - a.x()) / 3.0,
        a.y() + 2.0 * (b.y() - a.y()) / 3.0,
        a.z() + 2.0 * (b.z() - a.z()) / 3.0);
    return apmesh::core::CubicBezier3{a, p1, p2, b};
}

std::expected<double, apmesh::core::CurveError> mapped_parameter(
    const double source,
    const double target,
    const double parameter) {
    if (source < target) {
        return parameter;
    }
    const auto domain = apmesh::core::CurveParameterDomain::make(
        std::min(source, target), std::max(source, target));
    if (!domain) {
        return std::unexpected{
            apmesh::core::CurveError::non_finite_result};
    }
    return apmesh::core::reversed_parameter(*domain, parameter);
}

template <apmesh::core::BoundedParametricSurface3 Surface>
bool verify_mapping(
    const Surface& basis,
    const apmesh::core::RectangularTrimmedSurface3<Surface>& trim,
    const double u,
    const double v) {
    const auto mapped_u = mapped_parameter(
        trim.u_source_parameter(), trim.u_target_parameter(), u);
    const auto mapped_v = mapped_parameter(
        trim.v_source_parameter(), trim.v_target_parameter(), v);
    if (!mapped_u || !mapped_v) {
        return false;
    }

    const auto basis_value = basis.evaluate(*mapped_u, *mapped_v);
    const auto trim_value = trim.evaluate(u, v);
    const auto basis_first =
        basis.first_derivatives(*mapped_u, *mapped_v);
    const auto trim_first = trim.first_derivatives(u, v);
    const auto basis_second =
        basis.second_derivatives(*mapped_u, *mapped_v);
    const auto trim_second = trim.second_derivatives(u, v);

    if (!basis_value || !trim_value || !basis_first || !trim_first ||
        !basis_second || !trim_second) {
        return false;
    }

    const bool u_forward =
        trim.u_source_parameter() < trim.u_target_parameter();
    const bool v_forward =
        trim.v_source_parameter() < trim.v_target_parameter();

    return *trim_value == *basis_value &&
           trim_first->u ==
               (u_forward ? basis_first->u : -basis_first->u) &&
           trim_first->v ==
               (v_forward ? basis_first->v : -basis_first->v) &&
           trim_second->uu == basis_second->uu &&
           trim_second->vv == basis_second->vv &&
           trim_second->uv ==
               (u_forward == v_forward
                    ? basis_second->uv
                    : -basis_second->uv);
}

} // namespace

int main() {
    using apmesh::core::BicubicBezierPatch3;
    using apmesh::core::BicubicNURBSSurface3;
    using apmesh::core::BoundedParametricSurface3;
    using apmesh::core::CubicBezierCoonsPatch3;
    using apmesh::core::RationalBicubicBezierPatch3;
    using apmesh::core::RectangularTrimmedSurface3;
    using apmesh::core::RectangularTrimmedSurfaceConstructionError;
    using apmesh::core::SurfaceError;

    static_assert(
        BoundedParametricSurface3<
            RectangularTrimmedSurface3<BicubicBezierPatch3>>);
    static_assert(
        BoundedParametricSurface3<
            RectangularTrimmedSurface3<RationalBicubicBezierPatch3>>);
    static_assert(
        BoundedParametricSurface3<
            RectangularTrimmedSurface3<BicubicNURBSSurface3>>);
    static_assert(
        BoundedParametricSurface3<
            RectangularTrimmedSurface3<CubicBezierCoonsPatch3>>);

    bool passed = true;

    const auto controls = make_controls();
    const BicubicBezierPatch3 polynomial{controls};

    const auto trim = RectangularTrimmedSurface3<BicubicBezierPatch3>::make(
        polynomial, 0.2, 0.85, 0.9, 0.1);
    if (!trim) {
        return 1;
    }

    passed = require(
                 trim->basis_surface() == polynomial &&
                     trim->u_source_parameter() == 0.2 &&
                     trim->u_target_parameter() == 0.85 &&
                     trim->v_source_parameter() == 0.9 &&
                     trim->v_target_parameter() == 0.1 &&
                     trim->parameter_domain().u.lower() == 0.2 &&
                     trim->parameter_domain().u.upper() == 0.85 &&
                     trim->parameter_domain().v.lower() == 0.1 &&
                     trim->parameter_domain().v.upper() == 0.9,
                 "rectangular trim storage/domain differs") &&
             passed;

    passed = require(
                 verify_mapping(polynomial, *trim, 0.35, 0.25) &&
                     verify_mapping(polynomial, *trim, 0.8, 0.75),
                 "polynomial rectangular trim mapping differs") &&
             passed;

    const double nan = std::numeric_limits<double>::quiet_NaN();
    const auto non_finite_u_source =
        RectangularTrimmedSurface3<BicubicBezierPatch3>::make(
            polynomial, nan, 0.8, 0.1, 0.9);
    const auto non_finite_u_target =
        RectangularTrimmedSurface3<BicubicBezierPatch3>::make(
            polynomial, 0.2, nan, 0.1, 0.9);
    const auto non_finite_v_source =
        RectangularTrimmedSurface3<BicubicBezierPatch3>::make(
            polynomial, 0.2, 0.8, nan, 0.9);
    const auto non_finite_v_target =
        RectangularTrimmedSurface3<BicubicBezierPatch3>::make(
            polynomial, 0.2, 0.8, 0.1, nan);
    const auto u_source_out =
        RectangularTrimmedSurface3<BicubicBezierPatch3>::make(
            polynomial, -0.1, 0.8, 0.1, 0.9);
    const auto u_target_out =
        RectangularTrimmedSurface3<BicubicBezierPatch3>::make(
            polynomial, 0.2, 1.1, 0.1, 0.9);
    const auto v_source_out =
        RectangularTrimmedSurface3<BicubicBezierPatch3>::make(
            polynomial, 0.2, 0.8, -0.1, 0.9);
    const auto v_target_out =
        RectangularTrimmedSurface3<BicubicBezierPatch3>::make(
            polynomial, 0.2, 0.8, 0.1, 1.1);
    const auto zero_u =
        RectangularTrimmedSurface3<BicubicBezierPatch3>::make(
            polynomial, 0.4, 0.4, 0.1, 0.9);
    const auto zero_v =
        RectangularTrimmedSurface3<BicubicBezierPatch3>::make(
            polynomial, 0.2, 0.8, 0.6, 0.6);

    passed = require(
                 !non_finite_u_source &&
                     non_finite_u_source.error() ==
                         RectangularTrimmedSurfaceConstructionError::
                             non_finite_u_source_parameter &&
                     !non_finite_u_target &&
                     non_finite_u_target.error() ==
                         RectangularTrimmedSurfaceConstructionError::
                             non_finite_u_target_parameter &&
                     !non_finite_v_source &&
                     non_finite_v_source.error() ==
                         RectangularTrimmedSurfaceConstructionError::
                             non_finite_v_source_parameter &&
                     !non_finite_v_target &&
                     non_finite_v_target.error() ==
                         RectangularTrimmedSurfaceConstructionError::
                             non_finite_v_target_parameter &&
                     !u_source_out &&
                     u_source_out.error() ==
                         RectangularTrimmedSurfaceConstructionError::
                             u_source_parameter_out_of_domain &&
                     !u_target_out &&
                     u_target_out.error() ==
                         RectangularTrimmedSurfaceConstructionError::
                             u_target_parameter_out_of_domain &&
                     !v_source_out &&
                     v_source_out.error() ==
                         RectangularTrimmedSurfaceConstructionError::
                             v_source_parameter_out_of_domain &&
                     !v_target_out &&
                     v_target_out.error() ==
                         RectangularTrimmedSurfaceConstructionError::
                             v_target_parameter_out_of_domain &&
                     !zero_u &&
                     zero_u.error() ==
                         RectangularTrimmedSurfaceConstructionError::
                             zero_width_u_trim &&
                     !zero_v &&
                     zero_v.error() ==
                         RectangularTrimmedSurfaceConstructionError::
                             zero_width_v_trim,
                 "rectangular trim construction validation differs") &&
             passed;

    const auto bad_u_nan = trim->evaluate(nan, 0.5);
    const auto bad_v_inf = trim->evaluate(
        0.5, std::numeric_limits<double>::infinity());
    const auto bad_u_domain = trim->first_derivatives(0.1, 0.5);
    const auto bad_v_domain = trim->second_derivatives(0.5, 0.95);
    passed = require(
                 !bad_u_nan &&
                     bad_u_nan.error() ==
                         SurfaceError::non_finite_u_parameter &&
                     !bad_v_inf &&
                     bad_v_inf.error() ==
                         SurfaceError::non_finite_v_parameter &&
                     !bad_u_domain &&
                     bad_u_domain.error() ==
                         SurfaceError::u_parameter_out_of_domain &&
                     !bad_v_domain &&
                     bad_v_domain.error() ==
                         SurfaceError::v_parameter_out_of_domain,
                 "rectangular trim query failures differ") &&
             passed;

    const auto u_reversed = trim->u_reversed();
    const auto v_reversed = trim->v_reversed();
    passed = require(
                 u_reversed.parameter_domain() == trim->parameter_domain() &&
                     v_reversed.parameter_domain() == trim->parameter_domain() &&
                     u_reversed.u_reversed() == *trim &&
                     v_reversed.v_reversed() == *trim &&
                     trim->u_reversed().v_reversed() ==
                         trim->v_reversed().u_reversed() &&
                     verify_mapping(polynomial, u_reversed, 0.35, 0.25) &&
                     verify_mapping(polynomial, v_reversed, 0.35, 0.25),
                 "rectangular trim reversal laws differ") &&
             passed;

    const auto rational =
        RationalBicubicBezierPatch3::make(controls, make_weights());
    if (!rational) {
        return 1;
    }
    const auto rational_trim =
        RectangularTrimmedSurface3<RationalBicubicBezierPatch3>::make(
            *rational, 0.75, 0.15, 0.2, 0.85);
    if (!rational_trim) {
        return 1;
    }
    passed = require(
                 verify_mapping(*rational, *rational_trim, 0.3, 0.7),
                 "rational rectangular trim mapping differs") &&
             passed;

    const auto nurbs = BicubicNURBSSurface3::make(
        flatten(controls),
        flatten(make_weights()),
        4U,
        4U,
        {},
        {},
        -4.0,
        5.0,
        -3.0,
        7.0);
    if (!nurbs) {
        return 1;
    }
    const auto nurbs_trim =
        RectangularTrimmedSurface3<BicubicNURBSSurface3>::make(
            *nurbs, 3.5, -2.5, -1.0, 6.0);
    if (!nurbs_trim) {
        return 1;
    }
    passed = require(
                 nurbs_trim->parameter_domain().u.lower() == -2.5 &&
                     nurbs_trim->parameter_domain().u.upper() == 3.5 &&
                     nurbs_trim->parameter_domain().v.lower() == -1.0 &&
                     nurbs_trim->parameter_domain().v.upper() == 6.0 &&
                     verify_mapping(*nurbs, *nurbs_trim, -1.0, 4.0),
                 "non-normalized NURBS trim differs") &&
             passed;

    const auto p00 = point(0.0, 0.0, 0.0);
    const auto p10 = point(1.0, 0.0, 0.25);
    const auto p01 = point(0.0, 1.0, -0.5);
    const auto p11 = point(1.0, 1.0, 1.0);
    const auto coons = CubicBezierCoonsPatch3::make(
        line_curve(p00, p10),
        line_curve(p01, p11),
        line_curve(p00, p01),
        line_curve(p10, p11));
    if (!coons) {
        return 1;
    }
    const auto coons_trim =
        RectangularTrimmedSurface3<CubicBezierCoonsPatch3>::make(
            *coons, 0.1, 0.9, 0.8, 0.2);
    if (!coons_trim) {
        return 1;
    }
    passed = require(
                 verify_mapping(*coons, *coons_trim, 0.4, 0.35),
                 "Coons rectangular trim mapping differs") &&
             passed;

    const auto nested =
        RectangularTrimmedSurface3<
            RectangularTrimmedSurface3<BicubicBezierPatch3>>::make(
            *trim, 0.7, 0.3, 0.2, 0.8);
    const auto direct =
        RectangularTrimmedSurface3<BicubicBezierPatch3>::make(
            polynomial, 0.7, 0.3, 0.8, 0.2);
    const auto nested_escape =
        RectangularTrimmedSurface3<
            RectangularTrimmedSurface3<BicubicBezierPatch3>>::make(
            *trim, 0.05, 0.3, 0.2, 0.8);
    if (!nested || !direct) {
        return 1;
    }
    const auto nested_value = nested->evaluate(0.45, 0.55);
    const auto direct_value = direct->evaluate(0.45, 0.55);
    const auto nested_first = nested->first_derivatives(0.45, 0.55);
    const auto direct_first = direct->first_derivatives(0.45, 0.55);
    const auto nested_second = nested->second_derivatives(0.45, 0.55);
    const auto direct_second = direct->second_derivatives(0.45, 0.55);
    passed = require(
                 nested_value && direct_value &&
                     close_point(*nested_value, *direct_value, 256.0) &&
                     nested_first && direct_first &&
                     close_vector(
                         nested_first->u, direct_first->u, 1024.0) &&
                     close_vector(
                         nested_first->v, direct_first->v, 1024.0) &&
                     nested_second && direct_second &&
                     close_vector(
                         nested_second->uu, direct_second->uu, 4096.0) &&
                     close_vector(
                         nested_second->uv, direct_second->uv, 4096.0) &&
                     close_vector(
                         nested_second->vv, direct_second->vv, 4096.0) &&
                     !nested_escape &&
                     nested_escape.error() ==
                         RectangularTrimmedSurfaceConstructionError::
                             u_source_parameter_out_of_domain,
                 "nested rectangular trim semantics differ") &&
             passed;

    std::vector<apmesh::core::Point3> c1_points;
    c1_points.reserve(24U);
    for (std::size_t i = 0; i < 6U; ++i) {
        for (std::size_t j = 0; j < 4U; ++j) {
            c1_points.push_back(point(
                static_cast<double>(i),
                static_cast<double>(j),
                0.2 * static_cast<double>(i * i) +
                    0.15 * static_cast<double>(i * j) -
                    0.1 * static_cast<double>(j)));
        }
    }
    const std::vector<double> c1_weights(24U, 1.0);
    const auto c1_nurbs = BicubicNURBSSurface3::make(
        c1_points,
        c1_weights,
        6U,
        4U,
        std::vector<double>{0.0},
        std::vector<double>{},
        std::vector<std::uint8_t>{2U},
        std::vector<std::uint8_t>{},
        -2.0,
        2.0,
        -1.0,
        1.0);
    if (!c1_nurbs) {
        return 1;
    }
    const auto c1_trim =
        RectangularTrimmedSurface3<BicubicNURBSSurface3>::make(
            *c1_nurbs, 1.5, -1.5, -0.8, 0.8);
    if (!c1_trim) {
        return 1;
    }
    const auto c1_value = c1_trim->evaluate(0.0, 0.25);
    const auto c1_first = c1_trim->first_derivatives(0.0, 0.25);
    const auto c1_second = c1_trim->second_derivatives(0.0, 0.25);
    passed = require(
                 c1_value && c1_first && !c1_second &&
                     c1_second.error() ==
                         SurfaceError::insufficient_continuity,
                 "rectangular trim hid NURBS continuity failure") &&
             passed;

    const double huge = 1.0e300;
    const auto constant_point = point(1.0, -2.0, 3.0);
    const std::vector<apmesh::core::Point3> constant_points(
        16U, constant_point);
    const std::vector<double> constant_weights(16U, 1.0);
    const auto extreme_basis = BicubicNURBSSurface3::make(
        constant_points,
        constant_weights,
        4U,
        4U,
        {},
        {},
        -huge,
        huge,
        -huge,
        huge);
    if (!extreme_basis) {
        return 1;
    }
    const auto extreme_trim =
        RectangularTrimmedSurface3<BicubicNURBSSurface3>::make(
            *extreme_basis,
            8.0e299,
            -8.0e299,
            -7.0e299,
            7.0e299);
    if (!extreme_trim) {
        return 1;
    }
    const auto extreme_value_a =
        extreme_trim->evaluate(6.0e299, 5.0e299);
    const auto extreme_value_b =
        extreme_trim->evaluate(6.0e299, 5.0e299);
    const auto extreme_first_a =
        extreme_trim->first_derivatives(6.0e299, 5.0e299);
    const auto extreme_first_b =
        extreme_trim->first_derivatives(6.0e299, 5.0e299);
    passed = require(
                 extreme_value_a && extreme_value_b &&
                     *extreme_value_a == constant_point &&
                     *extreme_value_a == *extreme_value_b &&
                     extreme_first_a && extreme_first_b &&
                     *extreme_first_a == *extreme_first_b,
                 "extreme finite rectangular trim mapping is not deterministic") &&
             passed;

    const auto repeated_failure_a = trim->evaluate(0.05, 0.5);
    const auto repeated_failure_b = trim->evaluate(0.05, 0.5);
    passed = require(
                 !repeated_failure_a && !repeated_failure_b &&
                     repeated_failure_a.error() ==
                         repeated_failure_b.error(),
                 "rectangular trim typed failure is not deterministic") &&
             passed;

    return passed ? 0 : 1;
}
