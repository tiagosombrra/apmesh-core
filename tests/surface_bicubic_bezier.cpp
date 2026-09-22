#include "apmesh/geometry/surface.hpp"

#include "apmesh/geometry/curve.hpp"

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

long double bernstein1(const int index, const long double parameter) {
    if (index == 0) {
        return 1.0L - parameter;
    }
    if (index == 1) {
        return parameter;
    }
    return 0.0L;
}

long double bernstein2(const int index, const long double parameter) {
    const long double q = 1.0L - parameter;
    if (index == 0) {
        return q * q;
    }
    if (index == 1) {
        return 2.0L * parameter * q;
    }
    if (index == 2) {
        return parameter * parameter;
    }
    return 0.0L;
}

long double bernstein3(const int index, const long double parameter) {
    const long double q = 1.0L - parameter;
    if (index == 0) {
        return q * q * q;
    }
    if (index == 1) {
        return 3.0L * parameter * q * q;
    }
    if (index == 2) {
        return 3.0L * parameter * parameter * q;
    }
    if (index == 3) {
        return parameter * parameter * parameter;
    }
    return 0.0L;
}

long double bernstein3_first(
    const int index,
    const long double parameter) {
    return 3.0L *
           (bernstein2(index - 1, parameter) -
            bernstein2(index, parameter));
}

long double bernstein3_second(
    const int index,
    const long double parameter) {
    return 6.0L *
           (bernstein1(index - 2, parameter) -
            2.0L * bernstein1(index - 1, parameter) +
            bernstein1(index, parameter));
}

struct ReferenceJet {
    long double x{};
    long double y{};
    long double z{};
    long double ux{};
    long double uy{};
    long double uz{};
    long double vx{};
    long double vy{};
    long double vz{};
    long double uux{};
    long double uuy{};
    long double uuz{};
    long double uvx{};
    long double uvy{};
    long double uvz{};
    long double vvx{};
    long double vvy{};
    long double vvz{};
};

ReferenceJet reference_jet(
    const apmesh::core::BicubicBezierPatch3::ControlNet& controls,
    const double u,
    const double v) {
    ReferenceJet result{};
    const long double lu = static_cast<long double>(u);
    const long double lv = static_cast<long double>(v);

    for (int i = 0; i < 4; ++i) {
        const long double bu = bernstein3(i, lu);
        const long double du = bernstein3_first(i, lu);
        const long double ddu = bernstein3_second(i, lu);

        for (int j = 0; j < 4; ++j) {
            const long double bv = bernstein3(j, lv);
            const long double dv = bernstein3_first(j, lv);
            const long double ddv = bernstein3_second(j, lv);
            const auto& point =
                controls[static_cast<std::size_t>(i)]
                        [static_cast<std::size_t>(j)];
            const long double x = static_cast<long double>(point.x());
            const long double y = static_cast<long double>(point.y());
            const long double z = static_cast<long double>(point.z());

            const long double value_weight = bu * bv;
            const long double u_weight = du * bv;
            const long double v_weight = bu * dv;
            const long double uu_weight = ddu * bv;
            const long double uv_weight = du * dv;
            const long double vv_weight = bu * ddv;

            result.x += value_weight * x;
            result.y += value_weight * y;
            result.z += value_weight * z;
            result.ux += u_weight * x;
            result.uy += u_weight * y;
            result.uz += u_weight * z;
            result.vx += v_weight * x;
            result.vy += v_weight * y;
            result.vz += v_weight * z;
            result.uux += uu_weight * x;
            result.uuy += uu_weight * y;
            result.uuz += uu_weight * z;
            result.uvx += uv_weight * x;
            result.uvy += uv_weight * y;
            result.uvz += uv_weight * z;
            result.vvx += vv_weight * x;
            result.vvy += vv_weight * y;
            result.vvz += vv_weight * z;
        }
    }
    return result;
}

apmesh::core::BicubicBezierPatch3::ControlNet filled_net(
    const apmesh::core::Point3& point) {
    return {{
        {{point, point, point, point}},
        {{point, point, point, point}},
        {{point, point, point, point}},
        {{point, point, point, point}},
    }};
}

apmesh::core::BicubicBezierPatch3::ControlNet make_generic_net() {
    using apmesh::core::Point3;
    const auto seed = *Point3::make(0.0, 0.0, 0.0);
    auto controls = filled_net(seed);

    constexpr std::array<std::array<std::array<double, 3>, 4>, 4> data{{
        {{{-3.0, 0.0, 1.0}, {-2.0, 2.0, -1.0}, {-1.0, 4.0, 2.0}, {0.0, 6.0, 0.5}}},
        {{{0.0, -1.0, 2.0}, {1.0, 1.5, 4.0}, {2.0, 3.0, -2.0}, {3.0, 5.0, 1.0}}},
        {{{3.5, -2.0, -1.0}, {4.0, 0.5, 3.0}, {5.0, 2.5, 5.0}, {6.0, 4.5, -0.5}}},
        {{{7.0, -3.0, 0.0}, {8.0, -0.5, 2.5}, {9.0, 2.0, -1.5}, {10.0, 4.0, 3.0}}},
    }};

    for (std::size_t i = 0; i < 4U; ++i) {
        for (std::size_t j = 0; j < 4U; ++j) {
            const auto point = Point3::make(
                data[i][j][0],
                data[i][j][1],
                data[i][j][2]);
            controls[i][j] = *point;
        }
    }
    return controls;
}

apmesh::core::BicubicBezierPatch3::ControlNet make_saddle_net() {
    using apmesh::core::Point3;
    const auto seed = *Point3::make(0.0, 0.0, 0.0);
    auto controls = filled_net(seed);
    for (std::size_t i = 0; i < 4U; ++i) {
        const double u = static_cast<double>(i) / 3.0;
        for (std::size_t j = 0; j < 4U; ++j) {
            const double v = static_cast<double>(j) / 3.0;
            controls[i][j] = *Point3::make(u, v, u * v);
        }
    }
    return controls;
}

apmesh::core::BicubicBezierPatch3::ControlNet translated_net(
    const apmesh::core::BicubicBezierPatch3::ControlNet& source,
    const double dx,
    const double dy,
    const double dz) {
    auto result = source;
    for (auto& row : result) {
        for (auto& point : row) {
            point = *apmesh::core::Point3::make(
                point.x() + dx,
                point.y() + dy,
                point.z() + dz);
        }
    }
    return result;
}

apmesh::core::BicubicBezierPatch3::ControlNet scaled_net(
    const apmesh::core::BicubicBezierPatch3::ControlNet& source,
    const double factor) {
    auto result = source;
    for (auto& row : result) {
        for (auto& point : row) {
            point = *apmesh::core::Point3::make(
                factor * point.x(),
                factor * point.y(),
                factor * point.z());
        }
    }
    return result;
}

} // namespace

int main() {
    using apmesh::core::BicubicBezierPatch3;
    using apmesh::core::BoundedParametricSurface3;
    using apmesh::core::CubicBezier3;
    using apmesh::core::Point3;
    using apmesh::core::SurfaceError;
    using apmesh::core::SurfaceFirstDerivatives3;
    using apmesh::core::SurfaceParameterDomain;
    using apmesh::core::SurfaceSecondDerivatives3;

    static_assert(BoundedParametricSurface3<BicubicBezierPatch3>);
    static_assert(std::is_copy_constructible_v<BicubicBezierPatch3>);
    static_assert(std::same_as<
        decltype(std::declval<const BicubicBezierPatch3&>().parameter_domain()),
        SurfaceParameterDomain>);
    static_assert(std::same_as<
        decltype(std::declval<const BicubicBezierPatch3&>().evaluate(0.5, 0.5)),
        std::expected<Point3, SurfaceError>>);
    static_assert(std::same_as<
        decltype(
            std::declval<const BicubicBezierPatch3&>().
                first_derivatives(0.5, 0.5)),
        std::expected<SurfaceFirstDerivatives3, SurfaceError>>);
    static_assert(std::same_as<
        decltype(
            std::declval<const BicubicBezierPatch3&>().
                second_derivatives(0.5, 0.5)),
        std::expected<SurfaceSecondDerivatives3, SurfaceError>>);

    bool passed = true;

    const auto controls = make_generic_net();
    const BicubicBezierPatch3 patch{controls};

    const auto domain = patch.parameter_domain();
    passed = require(
                 domain.u.lower() == 0.0 &&
                     domain.u.upper() == 1.0 &&
                     domain.v.lower() == 0.0 &&
                     domain.v.upper() == 1.0,
                 "bicubic surface domain differs") &&
             passed;

    const auto nan = std::numeric_limits<double>::quiet_NaN();
    const auto u_nan = patch.evaluate(nan, nan);
    const auto v_nan = patch.evaluate(0.5, nan);
    const auto v_nan_before_u_domain = patch.evaluate(-0.1, nan);
    const auto u_low = patch.evaluate(-0.1, 0.5);
    const auto v_high = patch.evaluate(0.5, 1.1);
    passed = require(
                 !u_nan &&
                     u_nan.error() == SurfaceError::non_finite_u_parameter &&
                     !v_nan &&
                     v_nan.error() == SurfaceError::non_finite_v_parameter &&
                     !v_nan_before_u_domain &&
                     v_nan_before_u_domain.error() ==
                         SurfaceError::non_finite_v_parameter &&
                     !u_low &&
                     u_low.error() ==
                         SurfaceError::u_parameter_out_of_domain &&
                     !v_high &&
                     v_high.error() ==
                         SurfaceError::v_parameter_out_of_domain,
                 "surface parameter validation order differs") &&
             passed;

    passed = require(
                 patch.evaluate(0.0, 0.0) &&
                     *patch.evaluate(0.0, 0.0) == controls[0][0] &&
                     patch.evaluate(0.0, 1.0) &&
                     *patch.evaluate(0.0, 1.0) == controls[0][3] &&
                     patch.evaluate(1.0, 0.0) &&
                     *patch.evaluate(1.0, 0.0) == controls[3][0] &&
                     patch.evaluate(1.0, 1.0) &&
                     *patch.evaluate(1.0, 1.0) == controls[3][3],
                 "bicubic exact corners differ") &&
             passed;

    constexpr std::array<std::array<double, 2>, 5> samples{{
        {0.125, 0.25},
        {0.25, 0.75},
        {0.5, 0.5},
        {0.75, 0.125},
        {0.875, 0.875},
    }};
    for (const auto& sample : samples) {
        const double u = sample[0];
        const double v = sample[1];
        const auto reference = reference_jet(controls, u, v);
        const auto value = patch.evaluate(u, v);
        const auto first = patch.first_derivatives(u, v);
        const auto second = patch.second_derivatives(u, v);

        passed = require(
                     value && first && second &&
                         close_scalar(
                             value->x(),
                             static_cast<double>(reference.x),
                             128.0) &&
                         close_scalar(
                             value->y(),
                             static_cast<double>(reference.y),
                             128.0) &&
                         close_scalar(
                             value->z(),
                             static_cast<double>(reference.z),
                             128.0) &&
                         close_scalar(
                             first->u.x(),
                             static_cast<double>(reference.ux),
                             512.0) &&
                         close_scalar(
                             first->u.y(),
                             static_cast<double>(reference.uy),
                             512.0) &&
                         close_scalar(
                             first->u.z(),
                             static_cast<double>(reference.uz),
                             512.0) &&
                         close_scalar(
                             first->v.x(),
                             static_cast<double>(reference.vx),
                             512.0) &&
                         close_scalar(
                             first->v.y(),
                             static_cast<double>(reference.vy),
                             512.0) &&
                         close_scalar(
                             first->v.z(),
                             static_cast<double>(reference.vz),
                             512.0) &&
                         close_scalar(
                             second->uu.x(),
                             static_cast<double>(reference.uux),
                             2048.0) &&
                         close_scalar(
                             second->uu.y(),
                             static_cast<double>(reference.uuy),
                             2048.0) &&
                         close_scalar(
                             second->uu.z(),
                             static_cast<double>(reference.uuz),
                             2048.0) &&
                         close_scalar(
                             second->uv.x(),
                             static_cast<double>(reference.uvx),
                             2048.0) &&
                         close_scalar(
                             second->uv.y(),
                             static_cast<double>(reference.uvy),
                             2048.0) &&
                         close_scalar(
                             second->uv.z(),
                             static_cast<double>(reference.uvz),
                             2048.0) &&
                         close_scalar(
                             second->vv.x(),
                             static_cast<double>(reference.vvx),
                             2048.0) &&
                         close_scalar(
                             second->vv.y(),
                             static_cast<double>(reference.vvy),
                             2048.0) &&
                         close_scalar(
                             second->vv.z(),
                             static_cast<double>(reference.vvz),
                             2048.0),
                     "independent bicubic Bernstein oracle differs") &&
                 passed;
    }

    const CubicBezier3 u0_curve{
        controls[0][0], controls[0][1], controls[0][2], controls[0][3]};
    const CubicBezier3 u1_curve{
        controls[3][0], controls[3][1], controls[3][2], controls[3][3]};
    const CubicBezier3 v0_curve{
        controls[0][0], controls[1][0], controls[2][0], controls[3][0]};
    const CubicBezier3 v1_curve{
        controls[0][3], controls[1][3], controls[2][3], controls[3][3]};

    constexpr std::array<double, 5> edge_parameters{
        0.0, 0.2, 0.5, 0.8, 1.0};
    for (const double parameter : edge_parameters) {
        const auto p_u0 = patch.evaluate(0.0, parameter);
        const auto p_u1 = patch.evaluate(1.0, parameter);
        const auto p_v0 = patch.evaluate(parameter, 0.0);
        const auto p_v1 = patch.evaluate(parameter, 1.0);
        const auto d_u0 = patch.first_derivatives(0.0, parameter);
        const auto d_u1 = patch.first_derivatives(1.0, parameter);
        const auto d_v0 = patch.first_derivatives(parameter, 0.0);
        const auto d_v1 = patch.first_derivatives(parameter, 1.0);

        passed = require(
                     p_u0 && u0_curve.evaluate(parameter) &&
                         close_point(*p_u0, *u0_curve.evaluate(parameter), 128.0) &&
                         p_u1 && u1_curve.evaluate(parameter) &&
                         close_point(*p_u1, *u1_curve.evaluate(parameter), 128.0) &&
                         p_v0 && v0_curve.evaluate(parameter) &&
                         close_point(*p_v0, *v0_curve.evaluate(parameter), 128.0) &&
                         p_v1 && v1_curve.evaluate(parameter) &&
                         close_point(*p_v1, *v1_curve.evaluate(parameter), 128.0) &&
                         d_u0 && u0_curve.first_derivative(parameter) &&
                         close_vector(
                             d_u0->v,
                             *u0_curve.first_derivative(parameter),
                             512.0) &&
                         d_u1 && u1_curve.first_derivative(parameter) &&
                         close_vector(
                             d_u1->v,
                             *u1_curve.first_derivative(parameter),
                             512.0) &&
                         d_v0 && v0_curve.first_derivative(parameter) &&
                         close_vector(
                             d_v0->u,
                             *v0_curve.first_derivative(parameter),
                             512.0) &&
                         d_v1 && v1_curve.first_derivative(parameter) &&
                         close_vector(
                             d_v1->u,
                             *v1_curve.first_derivative(parameter),
                             512.0),
                     "bicubic boundary curve parity differs") &&
                 passed;
    }

    const BicubicBezierPatch3 saddle{make_saddle_net()};
    constexpr std::array<std::array<double, 2>, 3> saddle_samples{{
        {0.2, 0.3},
        {0.5, 0.75},
        {0.9, 0.1},
    }};
    for (const auto& sample : saddle_samples) {
        const double u = sample[0];
        const double v = sample[1];
        const auto value = saddle.evaluate(u, v);
        const auto first = saddle.first_derivatives(u, v);
        const auto second = saddle.second_derivatives(u, v);
        passed = require(
                     value && first && second &&
                         close_scalar(value->x(), u, 16.0) &&
                         close_scalar(value->y(), v, 16.0) &&
                         close_scalar(value->z(), u * v, 16.0) &&
                         close_scalar(first->u.x(), 1.0, 16.0) &&
                         close_scalar(first->u.y(), 0.0, 16.0) &&
                         close_scalar(first->u.z(), v, 16.0) &&
                         close_scalar(first->v.x(), 0.0, 16.0) &&
                         close_scalar(first->v.y(), 1.0, 16.0) &&
                         close_scalar(first->v.z(), u, 16.0) &&
                         close_vector(
                             second->uu,
                             *apmesh::core::Vector3::make(0.0, 0.0, 0.0),
                             16.0) &&
                         close_scalar(second->uv.x(), 0.0, 16.0) &&
                         close_scalar(second->uv.y(), 0.0, 16.0) &&
                         close_scalar(second->uv.z(), 1.0, 16.0) &&
                         close_vector(
                             second->vv,
                             *apmesh::core::Vector3::make(0.0, 0.0, 0.0),
                             16.0),
                     "analytic saddle fixture differs") &&
                 passed;
    }

    const auto u_reversed = patch.u_reversed();
    const auto v_reversed = patch.v_reversed();
    passed = require(
                 u_reversed.u_reversed() == patch &&
                     v_reversed.v_reversed() == patch,
                 "surface reversal is not an involution") &&
             passed;

    constexpr double ru = 0.3;
    constexpr double rv = 0.65;
    const auto original_value = patch.evaluate(ru, rv);
    const auto original_first = patch.first_derivatives(ru, rv);
    const auto original_second = patch.second_derivatives(ru, rv);
    const auto u_value = u_reversed.evaluate(1.0 - ru, rv);
    const auto u_first = u_reversed.first_derivatives(1.0 - ru, rv);
    const auto u_second = u_reversed.second_derivatives(1.0 - ru, rv);
    const auto v_value = v_reversed.evaluate(ru, 1.0 - rv);
    const auto v_first = v_reversed.first_derivatives(ru, 1.0 - rv);
    const auto v_second = v_reversed.second_derivatives(ru, 1.0 - rv);

    passed = require(
                 original_value && original_first && original_second &&
                     u_value && u_first && u_second &&
                     v_value && v_first && v_second &&
                     close_point(*original_value, *u_value, 256.0) &&
                     close_scalar(u_first->u.x(), -original_first->u.x(), 1024.0) &&
                     close_scalar(u_first->u.y(), -original_first->u.y(), 1024.0) &&
                     close_scalar(u_first->u.z(), -original_first->u.z(), 1024.0) &&
                     close_vector(u_first->v, original_first->v, 1024.0) &&
                     close_vector(u_second->uu, original_second->uu, 4096.0) &&
                     close_scalar(u_second->uv.x(), -original_second->uv.x(), 4096.0) &&
                     close_scalar(u_second->uv.y(), -original_second->uv.y(), 4096.0) &&
                     close_scalar(u_second->uv.z(), -original_second->uv.z(), 4096.0) &&
                     close_vector(u_second->vv, original_second->vv, 4096.0) &&
                     close_point(*original_value, *v_value, 256.0) &&
                     close_vector(v_first->u, original_first->u, 1024.0) &&
                     close_scalar(v_first->v.x(), -original_first->v.x(), 1024.0) &&
                     close_scalar(v_first->v.y(), -original_first->v.y(), 1024.0) &&
                     close_scalar(v_first->v.z(), -original_first->v.z(), 1024.0) &&
                     close_vector(v_second->uu, original_second->uu, 4096.0) &&
                     close_scalar(v_second->uv.x(), -original_second->uv.x(), 4096.0) &&
                     close_scalar(v_second->uv.y(), -original_second->uv.y(), 4096.0) &&
                     close_scalar(v_second->uv.z(), -original_second->uv.z(), 4096.0) &&
                     close_vector(v_second->vv, original_second->vv, 4096.0),
                 "surface reversal covariance differs") &&
             passed;

    const auto original_cross =
        apmesh::core::cross(original_first->u, original_first->v);
    const auto u_cross =
        apmesh::core::cross(u_first->u, u_first->v);
    const auto double_reversed =
        u_reversed.v_reversed();
    const auto double_first =
        double_reversed.first_derivatives(1.0 - ru, 1.0 - rv);
    const auto double_cross =
        double_first
            ? apmesh::core::cross(double_first->u, double_first->v)
            : std::expected<apmesh::core::Vector3, apmesh::core::GeometryError>{
                  std::unexpected{apmesh::core::GeometryError::indeterminate}};
    passed = require(
                 original_cross && u_cross && double_cross &&
                     close_scalar(u_cross->x(), -original_cross->x(), 2048.0) &&
                     close_scalar(u_cross->y(), -original_cross->y(), 2048.0) &&
                     close_scalar(u_cross->z(), -original_cross->z(), 2048.0) &&
                     close_vector(*double_cross, *original_cross, 2048.0),
                 "surface reversal orientation relation differs") &&
             passed;

    const auto constant_point = *Point3::make(2.0, -3.0, 5.0);
    auto constant_controls = filled_net(constant_point);
    const BicubicBezierPatch3 constant_patch{constant_controls};
    const auto constant_value = constant_patch.evaluate(0.37, 0.61);
    const auto constant_first =
        constant_patch.first_derivatives(0.37, 0.61);
    const auto constant_second =
        constant_patch.second_derivatives(0.37, 0.61);
    const auto zero = *apmesh::core::Vector3::make(0.0, 0.0, 0.0);
    passed = require(
                 constant_value && *constant_value == constant_point &&
                     constant_first &&
                     constant_first->u == zero &&
                     constant_first->v == zero &&
                     constant_second &&
                     constant_second->uu == zero &&
                     constant_second->uv == zero &&
                     constant_second->vv == zero,
                 "constant surface semantics differ") &&
             passed;

    auto degenerate_controls = filled_net(*Point3::make(0.0, 0.0, 0.0));
    for (std::size_t i = 0; i < 4U; ++i) {
        for (std::size_t j = 0; j < 4U; ++j) {
            degenerate_controls[i][j] = *Point3::make(
                static_cast<double>(i + j), 0.0, 0.0);
        }
    }
    const BicubicBezierPatch3 degenerate{degenerate_controls};
    passed = require(
                 degenerate.evaluate(0.4, 0.6).has_value() &&
                     degenerate.first_derivatives(0.4, 0.6).has_value() &&
                     degenerate.second_derivatives(0.4, 0.6).has_value(),
                 "degenerate surface was rejected at representation layer") &&
             passed;

    const BicubicBezierPatch3 translated{
        translated_net(controls, 8.0, -4.0, 2.0)};
    const BicubicBezierPatch3 scaled{scaled_net(controls, 2.0)};
    const auto base_value = patch.evaluate(0.4, 0.6);
    const auto base_first = patch.first_derivatives(0.4, 0.6);
    const auto base_second = patch.second_derivatives(0.4, 0.6);
    const auto translated_value = translated.evaluate(0.4, 0.6);
    const auto translated_first = translated.first_derivatives(0.4, 0.6);
    const auto translated_second = translated.second_derivatives(0.4, 0.6);
    const auto scaled_value = scaled.evaluate(0.4, 0.6);
    const auto scaled_first = scaled.first_derivatives(0.4, 0.6);
    const auto scaled_second = scaled.second_derivatives(0.4, 0.6);

    passed = require(
                 base_value && base_first && base_second &&
                     translated_value && translated_first && translated_second &&
                     scaled_value && scaled_first && scaled_second &&
                     close_scalar(translated_value->x() - base_value->x(), 8.0, 64.0) &&
                     close_scalar(translated_value->y() - base_value->y(), -4.0, 64.0) &&
                     close_scalar(translated_value->z() - base_value->z(), 2.0, 64.0) &&
                     close_vector(translated_first->u, base_first->u, 1024.0) &&
                     close_vector(translated_first->v, base_first->v, 1024.0) &&
                     close_vector(translated_second->uu, base_second->uu, 4096.0) &&
                     close_vector(translated_second->uv, base_second->uv, 4096.0) &&
                     close_vector(translated_second->vv, base_second->vv, 4096.0) &&
                     close_scalar(scaled_value->x(), 2.0 * base_value->x(), 256.0) &&
                     close_scalar(scaled_value->y(), 2.0 * base_value->y(), 256.0) &&
                     close_scalar(scaled_value->z(), 2.0 * base_value->z(), 256.0) &&
                     close_scalar(scaled_first->u.x(), 2.0 * base_first->u.x(), 2048.0) &&
                     close_scalar(scaled_first->u.y(), 2.0 * base_first->u.y(), 2048.0) &&
                     close_scalar(scaled_first->u.z(), 2.0 * base_first->u.z(), 2048.0) &&
                     close_scalar(scaled_first->v.x(), 2.0 * base_first->v.x(), 2048.0) &&
                     close_scalar(scaled_first->v.y(), 2.0 * base_first->v.y(), 2048.0) &&
                     close_scalar(scaled_first->v.z(), 2.0 * base_first->v.z(), 2048.0) &&
                     close_scalar(scaled_second->uv.x(), 2.0 * base_second->uv.x(), 8192.0) &&
                     close_scalar(scaled_second->uv.y(), 2.0 * base_second->uv.y(), 8192.0) &&
                     close_scalar(scaled_second->uv.z(), 2.0 * base_second->uv.z(), 8192.0),
                 "surface affine covariance differs") &&
             passed;

    const double magnitude = std::numeric_limits<double>::max() / 64.0;
    auto extreme_controls = filled_net(*Point3::make(0.0, 0.0, 0.0));
    for (std::size_t i = 0; i < 4U; ++i) {
        for (std::size_t j = 0; j < 4U; ++j) {
            extreme_controls[i][j] = *Point3::make(
                magnitude * static_cast<double>(i + 1U),
                -magnitude * static_cast<double>(j + 1U),
                magnitude * static_cast<double>(i + j + 1U));
        }
    }
    const BicubicBezierPatch3 extreme{extreme_controls};
    const auto extreme_value =
        extreme.evaluate(std::nextafter(0.0, 1.0), std::nextafter(1.0, 0.0));
    const auto extreme_first =
        extreme.first_derivatives(0.25, 0.75);
    const auto extreme_second =
        extreme.second_derivatives(0.25, 0.75);
    passed = require(
                 extreme_value && extreme_first && extreme_second &&
                     std::isfinite(extreme_value->x()) &&
                     std::isfinite(extreme_value->y()) &&
                     std::isfinite(extreme_value->z()) &&
                     std::isfinite(extreme_first->u.x()) &&
                     std::isfinite(extreme_first->v.y()) &&
                     std::isfinite(extreme_second->uv.z()),
                 "extreme finite surface evidence failed") &&
             passed;

    const auto repeat_value_a = patch.evaluate(0.375, 0.625);
    const auto repeat_value_b = patch.evaluate(0.375, 0.625);
    const auto repeat_first_a = patch.first_derivatives(0.375, 0.625);
    const auto repeat_first_b = patch.first_derivatives(0.375, 0.625);
    const auto repeat_second_a = patch.second_derivatives(0.375, 0.625);
    const auto repeat_second_b = patch.second_derivatives(0.375, 0.625);
    const auto repeat_failure_a = patch.evaluate(1.25, 0.5);
    const auto repeat_failure_b = patch.evaluate(1.25, 0.5);
    passed = require(
                 repeat_value_a && repeat_value_b &&
                     *repeat_value_a == *repeat_value_b &&
                     repeat_first_a && repeat_first_b &&
                     *repeat_first_a == *repeat_first_b &&
                     repeat_second_a && repeat_second_b &&
                     *repeat_second_a == *repeat_second_b &&
                     !repeat_failure_a && !repeat_failure_b &&
                     repeat_failure_a.error() == repeat_failure_b.error(),
                 "surface deterministic repeat evidence differs") &&
             passed;

    return passed ? 0 : 1;
}
