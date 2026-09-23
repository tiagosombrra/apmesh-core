#include "apmesh/geometry/extrusion_surface.hpp"

#include "apmesh/core/geometry.hpp"
#include "apmesh/math/linear_algebra.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <cstdio>
#include <expected>
#include <limits>
#include <string_view>
#include <type_traits>
#include <utility>

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
    return std::abs(lhs - rhs) <=
           absolute * std::abs(scale) + relative * magnitude;
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

long double b0(const long double t) {
    const long double q = 1.0L - t;
    return q * q * q;
}

long double b1(const long double t) {
    const long double q = 1.0L - t;
    return 3.0L * t * q * q;
}

long double b2(const long double t) {
    const long double q = 1.0L - t;
    return 3.0L * t * t * q;
}

long double b3(const long double t) {
    return t * t * t;
}

struct CurveJet {
    long double x{};
    long double y{};
    long double z{};
    long double dx{};
    long double dy{};
    long double dz{};
    long double ddx{};
    long double ddy{};
    long double ddz{};
};

CurveJet reference_curve_jet(
    const std::array<apmesh::core::Point3, 4>& points,
    const double parameter) {
    const long double t = static_cast<long double>(parameter);
    const std::array<long double, 4> basis{b0(t), b1(t), b2(t), b3(t)};

    CurveJet result{};
    for (std::size_t index = 0; index < points.size(); ++index) {
        const auto& point = points[index];
        result.x += basis[index] * static_cast<long double>(point.x());
        result.y += basis[index] * static_cast<long double>(point.y());
        result.z += basis[index] * static_cast<long double>(point.z());
    }

    const std::array<std::array<long double, 3>, 3> delta{{
        {{
            static_cast<long double>(points[1].x()) -
                static_cast<long double>(points[0].x()),
            static_cast<long double>(points[1].y()) -
                static_cast<long double>(points[0].y()),
            static_cast<long double>(points[1].z()) -
                static_cast<long double>(points[0].z()),
        }},
        {{
            static_cast<long double>(points[2].x()) -
                static_cast<long double>(points[1].x()),
            static_cast<long double>(points[2].y()) -
                static_cast<long double>(points[1].y()),
            static_cast<long double>(points[2].z()) -
                static_cast<long double>(points[1].z()),
        }},
        {{
            static_cast<long double>(points[3].x()) -
                static_cast<long double>(points[2].x()),
            static_cast<long double>(points[3].y()) -
                static_cast<long double>(points[2].y()),
            static_cast<long double>(points[3].z()) -
                static_cast<long double>(points[2].z()),
        }},
    }};

    const long double q = 1.0L - t;
    const std::array<long double, 3> quadratic{
        q * q, 2.0L * t * q, t * t};
    for (std::size_t index = 0; index < 3U; ++index) {
        result.dx += 3.0L * quadratic[index] * delta[index][0];
        result.dy += 3.0L * quadratic[index] * delta[index][1];
        result.dz += 3.0L * quadratic[index] * delta[index][2];
    }

    const std::array<std::array<long double, 3>, 2> second{{
        {{
            delta[1][0] - delta[0][0],
            delta[1][1] - delta[0][1],
            delta[1][2] - delta[0][2],
        }},
        {{
            delta[2][0] - delta[1][0],
            delta[2][1] - delta[1][1],
            delta[2][2] - delta[1][2],
        }},
    }};
    result.ddx =
        6.0L * (q * second[0][0] + t * second[1][0]);
    result.ddy =
        6.0L * (q * second[0][1] + t * second[1][1]);
    result.ddz =
        6.0L * (q * second[0][2] + t * second[1][2]);

    return result;
}

std::array<apmesh::core::Point3, 4> translated_controls(
    const std::array<apmesh::core::Point3, 4>& controls,
    const double dx,
    const double dy,
    const double dz) {
    auto result = controls;
    for (auto& point : result) {
        point = *apmesh::core::Point3::make(
            point.x() + dx,
            point.y() + dy,
            point.z() + dz);
    }
    return result;
}

std::array<apmesh::core::Point3, 4> scaled_controls(
    const std::array<apmesh::core::Point3, 4>& controls,
    const double factor) {
    auto result = controls;
    for (auto& point : result) {
        point = *apmesh::core::Point3::make(
            factor * point.x(),
            factor * point.y(),
            factor * point.z());
    }
    return result;
}

apmesh::core::CubicBezier3 make_curve(
    const std::array<apmesh::core::Point3, 4>& controls) {
    return apmesh::core::CubicBezier3{
        controls[0], controls[1], controls[2], controls[3]};
}

} // namespace

int main() {
    using apmesh::core::BoundedParametricSurface3;
    using apmesh::core::CartesianFrame3;
    using apmesh::core::CubicBezier3;
    using apmesh::core::CubicBezierLinearExtrusionSurface3;
    using apmesh::core::LinearExtrusionSurfaceConstructionError;
    using apmesh::core::Mat3;
    using apmesh::core::Point3;
    using apmesh::core::SurfaceError;
    using apmesh::core::SurfaceFirstDerivatives3;
    using apmesh::core::SurfaceParameterDomain;
    using apmesh::core::SurfaceSecondDerivatives3;
    using apmesh::core::Vector3;

    static_assert(
        BoundedParametricSurface3<CubicBezierLinearExtrusionSurface3>);
    static_assert(std::is_copy_constructible_v<
                  CubicBezierLinearExtrusionSurface3>);
    static_assert(std::same_as<
        decltype(std::declval<
                 const CubicBezierLinearExtrusionSurface3&>().
                     parameter_domain()),
        SurfaceParameterDomain>);
    static_assert(std::same_as<
        decltype(std::declval<
                 const CubicBezierLinearExtrusionSurface3&>().
                     evaluate(0.5, 0.5)),
        std::expected<Point3, SurfaceError>>);
    static_assert(std::same_as<
        decltype(std::declval<
                 const CubicBezierLinearExtrusionSurface3&>().
                     first_derivatives(0.5, 0.5)),
        std::expected<SurfaceFirstDerivatives3, SurfaceError>>);
    static_assert(std::same_as<
        decltype(std::declval<
                 const CubicBezierLinearExtrusionSurface3&>().
                     second_derivatives(0.5, 0.5)),
        std::expected<SurfaceSecondDerivatives3, SurfaceError>>);

    bool passed = true;

    const std::array<Point3, 4> controls{
        *Point3::make(-3.0, 1.0, 0.5),
        *Point3::make(-1.0, 4.0, 2.0),
        *Point3::make(2.0, -2.0, 5.0),
        *Point3::make(6.0, 3.0, -1.0),
    };
    const CubicBezier3 basis = make_curve(controls);
    const auto displacement = Vector3::make(2.5, -1.5, 4.0);
    if (!displacement) {
        return 1;
    }

    const auto surface =
        CubicBezierLinearExtrusionSurface3::make(basis, *displacement);
    if (!surface) {
        return 1;
    }

    passed = require(
                 surface->basis_curve() == basis &&
                     surface->extrusion_displacement() == *displacement,
                 "linear extrusion stored representation differs") &&
             passed;

    const auto domain = surface->parameter_domain();
    passed = require(
                 domain.u.lower() == 0.0 &&
                     domain.u.upper() == 1.0 &&
                     domain.v.lower() == 0.0 &&
                     domain.v.upper() == 1.0,
                 "linear extrusion domain differs") &&
             passed;

    const double nan = std::numeric_limits<double>::quiet_NaN();
    const auto u_nan = surface->evaluate(nan, nan);
    const auto v_nan = surface->evaluate(0.5, nan);
    const auto v_nan_before_u_domain = surface->evaluate(-0.1, nan);
    const auto u_low = surface->evaluate(-0.1, 0.5);
    const auto v_high = surface->evaluate(0.5, 1.1);
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
                 "linear extrusion parameter validation order differs") &&
             passed;

    constexpr std::array<std::array<double, 2>, 7> samples{{
        {0.0, 0.0},
        {0.125, 0.75},
        {0.25, 0.5},
        {0.5, 0.25},
        {0.75, 0.875},
        {1.0, 0.0},
        {1.0, 1.0},
    }};

    for (const auto& sample : samples) {
        const double u = sample[0];
        const double v = sample[1];
        const auto reference = reference_curve_jet(controls, u);
        const double ex = displacement->x();
        const double ey = displacement->y();
        const double ez = displacement->z();

        const auto value = surface->evaluate(u, v);
        const auto first = surface->first_derivatives(u, v);
        const auto second = surface->second_derivatives(u, v);

        passed = require(
                     value && first && second &&
                         close_scalar(
                             value->x(),
                             static_cast<double>(reference.x) + v * ex,
                             256.0) &&
                         close_scalar(
                             value->y(),
                             static_cast<double>(reference.y) + v * ey,
                             256.0) &&
                         close_scalar(
                             value->z(),
                             static_cast<double>(reference.z) + v * ez,
                             256.0) &&
                         close_scalar(
                             first->u.x(),
                             static_cast<double>(reference.dx),
                             1024.0) &&
                         close_scalar(
                             first->u.y(),
                             static_cast<double>(reference.dy),
                             1024.0) &&
                         close_scalar(
                             first->u.z(),
                             static_cast<double>(reference.dz),
                             1024.0) &&
                         first->v == *displacement &&
                         close_scalar(
                             second->uu.x(),
                             static_cast<double>(reference.ddx),
                             4096.0) &&
                         close_scalar(
                             second->uu.y(),
                             static_cast<double>(reference.ddy),
                             4096.0) &&
                         close_scalar(
                             second->uu.z(),
                             static_cast<double>(reference.ddz),
                             4096.0) &&
                         second->uv == *Vector3::make(0.0, 0.0, 0.0) &&
                         second->vv == *Vector3::make(0.0, 0.0, 0.0),
                     "linear extrusion independent analytic oracle differs") &&
                 passed;
    }

    constexpr std::array<double, 5> boundary_parameters{
        0.0, 0.2, 0.5, 0.8, 1.0};
    for (const double u : boundary_parameters) {
        const auto start_value = surface->evaluate(u, 0.0);
        const auto basis_value = basis.evaluate(u);
        const auto end_value = surface->evaluate(u, 1.0);
        const auto stored_end_value = surface->end_curve().evaluate(u);
        passed = require(
                     start_value && basis_value &&
                         *start_value == *basis_value &&
                         end_value && stored_end_value &&
                         *end_value == *stored_end_value,
                     "linear extrusion V-boundary parity differs") &&
                 passed;
    }

    for (const double v : boundary_parameters) {
        const auto start_edge = surface->evaluate(0.0, v);
        const auto end_edge = surface->evaluate(1.0, v);
        const auto p0_offset = *displacement * v;
        const auto p3_offset = *displacement * v;
        const auto expected_start =
            p0_offset ? controls[0] + *p0_offset
                      : std::expected<Point3, apmesh::core::GeometryError>{
                            std::unexpected{
                                apmesh::core::GeometryError::
                                    non_finite_result}};
        const auto expected_end =
            p3_offset ? controls[3] + *p3_offset
                      : std::expected<Point3, apmesh::core::GeometryError>{
                            std::unexpected{
                                apmesh::core::GeometryError::
                                    non_finite_result}};
        passed = require(
                     start_edge && expected_start &&
                         close_point(*start_edge, *expected_start, 256.0) &&
                         end_edge && expected_end &&
                         close_point(*end_edge, *expected_end, 256.0),
                     "linear extrusion U-boundary parity differs") &&
                 passed;
    }

    const auto u_reversed = surface->u_reversed();
    const auto v_reversed = surface->v_reversed();
    passed = require(
                 u_reversed.u_reversed() == *surface &&
                     v_reversed.v_reversed() == *surface,
                 "linear extrusion reversal is not an involution") &&
             passed;

    constexpr double ru = 0.3;
    constexpr double rv = 0.65;
    const auto original_value = surface->evaluate(ru, rv);
    const auto original_first = surface->first_derivatives(ru, rv);
    const auto original_second = surface->second_derivatives(ru, rv);
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
                     close_point(*u_value, *original_value, 256.0) &&
                     close_scalar(
                         u_first->u.x(), -original_first->u.x(), 1024.0) &&
                     close_scalar(
                         u_first->u.y(), -original_first->u.y(), 1024.0) &&
                     close_scalar(
                         u_first->u.z(), -original_first->u.z(), 1024.0) &&
                     close_vector(u_first->v, original_first->v, 1024.0) &&
                     close_vector(u_second->uu, original_second->uu, 4096.0) &&
                     u_second->uv == *Vector3::make(0.0, 0.0, 0.0) &&
                     u_second->vv == *Vector3::make(0.0, 0.0, 0.0) &&
                     close_point(*v_value, *original_value, 256.0) &&
                     close_vector(v_first->u, original_first->u, 1024.0) &&
                     close_scalar(
                         v_first->v.x(), -original_first->v.x(), 1024.0) &&
                     close_scalar(
                         v_first->v.y(), -original_first->v.y(), 1024.0) &&
                     close_scalar(
                         v_first->v.z(), -original_first->v.z(), 1024.0) &&
                     close_vector(v_second->uu, original_second->uu, 4096.0) &&
                     v_second->uv == *Vector3::make(0.0, 0.0, 0.0) &&
                     v_second->vv == *Vector3::make(0.0, 0.0, 0.0),
                 "linear extrusion reversal covariance differs") &&
             passed;

    const auto original_cross =
        original_first
            ? apmesh::core::cross(original_first->u, original_first->v)
            : std::expected<Vector3, apmesh::core::GeometryError>{
                  std::unexpected{
                      apmesh::core::GeometryError::non_finite_result}};
    const auto u_cross =
        u_first ? apmesh::core::cross(u_first->u, u_first->v)
                : std::expected<Vector3, apmesh::core::GeometryError>{
                      std::unexpected{
                          apmesh::core::GeometryError::non_finite_result}};
    const auto v_cross =
        v_first ? apmesh::core::cross(v_first->u, v_first->v)
                : std::expected<Vector3, apmesh::core::GeometryError>{
                      std::unexpected{
                          apmesh::core::GeometryError::non_finite_result}};
    passed = require(
                 original_cross && u_cross && v_cross &&
                     close_scalar(
                         u_cross->x(), -original_cross->x(), 2048.0) &&
                     close_scalar(
                         u_cross->y(), -original_cross->y(), 2048.0) &&
                     close_scalar(
                         u_cross->z(), -original_cross->z(), 2048.0) &&
                     close_scalar(
                         v_cross->x(), -original_cross->x(), 2048.0) &&
                     close_scalar(
                         v_cross->y(), -original_cross->y(), 2048.0) &&
                     close_scalar(
                         v_cross->z(), -original_cross->z(), 2048.0),
                 "single reversal did not flip surface orientation") &&
             passed;

    const auto zero = Vector3::make(0.0, 0.0, 0.0);
    const auto zero_surface =
        zero ? CubicBezierLinearExtrusionSurface3::make(basis, *zero)
             : std::expected<
                   CubicBezierLinearExtrusionSurface3,
                   LinearExtrusionSurfaceConstructionError>{
                   std::unexpected{
                       LinearExtrusionSurfaceConstructionError::
                           non_finite_extruded_control}};
    passed = require(
                 zero_surface &&
                     zero_surface->evaluate(0.4, 0.0) ==
                         zero_surface->evaluate(0.4, 1.0) &&
                     zero_surface->first_derivatives(0.4, 0.6) &&
                     zero_surface->first_derivatives(0.4, 0.6)->v == *zero,
                 "zero-extrusion representation differs") &&
             passed;

    const auto constant = Point3::make(2.0, -1.0, 3.0);
    if (!constant) {
        return 1;
    }
    const CubicBezier3 constant_curve{
        *constant, *constant, *constant, *constant};
    const auto constant_surface =
        CubicBezierLinearExtrusionSurface3::make(
            constant_curve, *displacement);
    passed = require(
                 constant_surface &&
                     constant_surface->first_derivatives(0.4, 0.6) &&
                     constant_surface->first_derivatives(0.4, 0.6)->u ==
                         *zero &&
                     constant_surface->second_derivatives(0.4, 0.6) &&
                     constant_surface->second_derivatives(0.4, 0.6)->uu ==
                         *zero,
                 "constant-basis extrusion was rejected or altered") &&
             passed;

    const auto translated =
        make_curve(translated_controls(controls, 8.0, -6.0, 4.0));
    const auto translated_surface =
        CubicBezierLinearExtrusionSurface3::make(
            translated, *displacement);
    const auto scaled =
        make_curve(scaled_controls(controls, 2.0));
    const auto scaled_displacement =
        *displacement * 2.0;
    const auto scaled_surface =
        scaled_displacement
            ? CubicBezierLinearExtrusionSurface3::make(
                  scaled, *scaled_displacement)
            : std::expected<
                  CubicBezierLinearExtrusionSurface3,
                  LinearExtrusionSurfaceConstructionError>{
                  std::unexpected{
                      LinearExtrusionSurfaceConstructionError::
                          non_finite_extruded_control}};

    const auto base_affine_value = surface->evaluate(0.35, 0.7);
    const auto translated_value =
        translated_surface
            ? translated_surface->evaluate(0.35, 0.7)
            : std::expected<Point3, SurfaceError>{
                  std::unexpected{SurfaceError::non_finite_result}};
    const auto scaled_value =
        scaled_surface
            ? scaled_surface->evaluate(0.35, 0.7)
            : std::expected<Point3, SurfaceError>{
                  std::unexpected{SurfaceError::non_finite_result}};
    passed = require(
                 base_affine_value && translated_value && scaled_value &&
                     close_scalar(
                         translated_value->x() - base_affine_value->x(),
                         8.0,
                         256.0) &&
                     close_scalar(
                         translated_value->y() - base_affine_value->y(),
                         -6.0,
                         256.0) &&
                     close_scalar(
                         translated_value->z() - base_affine_value->z(),
                         4.0,
                         256.0) &&
                     close_scalar(
                         scaled_value->x(),
                         2.0 * base_affine_value->x(),
                         512.0) &&
                     close_scalar(
                         scaled_value->y(),
                         2.0 * base_affine_value->y(),
                         512.0) &&
                     close_scalar(
                         scaled_value->z(),
                         2.0 * base_affine_value->z(),
                         512.0),
                 "linear extrusion translation/scale covariance differs") &&
             passed;

    const auto frame_origin = Point3::make(5.0, -2.0, 1.0);
    const auto frame_basis = Mat3::make({
        0.0, 1.0, 0.0,
        0.0, 0.0, 1.0,
        1.0, 0.0, 0.0});
    const auto frame =
        frame_origin && frame_basis
            ? CartesianFrame3::make(*frame_origin, *frame_basis, 1)
            : std::expected<CartesianFrame3, apmesh::core::GeometryError>{
                  std::unexpected{
                      apmesh::core::GeometryError::invalid_frame}};
    if (!frame) {
        return 1;
    }

    std::array<Point3, 4> mapped_controls = controls;
    for (std::size_t index = 0; index < controls.size(); ++index) {
        const auto mapped = frame->point_to_world(controls[index]);
        if (!mapped) {
            return 1;
        }
        mapped_controls[index] = *mapped;
    }
    const auto mapped_displacement =
        frame->vector_to_world(*displacement);
    if (!mapped_displacement) {
        return 1;
    }
    const auto framed_surface =
        CubicBezierLinearExtrusionSurface3::make(
            make_curve(mapped_controls), *mapped_displacement);
    if (!framed_surface) {
        return 1;
    }

    const auto frame_value = surface->evaluate(0.45, 0.55);
    const auto framed_value = framed_surface->evaluate(0.45, 0.55);
    const auto expected_frame_value =
        frame_value
            ? frame->point_to_world(*frame_value)
            : std::expected<Point3, apmesh::core::GeometryError>{
                  std::unexpected{
                      apmesh::core::GeometryError::non_finite_result}};
    const auto frame_first = surface->first_derivatives(0.45, 0.55);
    const auto framed_first =
        framed_surface->first_derivatives(0.45, 0.55);
    const auto expected_frame_u =
        frame_first
            ? frame->vector_to_world(frame_first->u)
            : std::expected<Vector3, apmesh::core::GeometryError>{
                  std::unexpected{
                      apmesh::core::GeometryError::non_finite_result}};
    const auto expected_frame_v =
        frame_first
            ? frame->vector_to_world(frame_first->v)
            : std::expected<Vector3, apmesh::core::GeometryError>{
                  std::unexpected{
                      apmesh::core::GeometryError::non_finite_result}};
    passed = require(
                 framed_value && expected_frame_value &&
                     close_point(
                         *framed_value, *expected_frame_value, 1024.0) &&
                     framed_first && expected_frame_u && expected_frame_v &&
                     close_vector(
                         framed_first->u, *expected_frame_u, 4096.0) &&
                     close_vector(
                         framed_first->v, *expected_frame_v, 4096.0),
                 "admitted Cartesian-frame covariance differs") &&
             passed;

    const double maximum = std::numeric_limits<double>::max();
    const std::array<Point3, 4> extreme_controls{
        *Point3::make(maximum / 32.0, 0.0, 0.0),
        *Point3::make(maximum / 32.0, maximum / 64.0, 0.0),
        *Point3::make(maximum / 32.0, 0.0, maximum / 64.0),
        *Point3::make(maximum / 32.0, maximum / 64.0, maximum / 64.0),
    };
    const auto extreme_displacement =
        Vector3::make(maximum / 64.0, 0.0, 0.0);
    const auto extreme_surface =
        extreme_displacement
            ? CubicBezierLinearExtrusionSurface3::make(
                  make_curve(extreme_controls),
                  *extreme_displacement)
            : std::expected<
                  CubicBezierLinearExtrusionSurface3,
                  LinearExtrusionSurfaceConstructionError>{
                  std::unexpected{
                      LinearExtrusionSurfaceConstructionError::
                          non_finite_extruded_control}};
    const auto extreme_value =
        extreme_surface
            ? extreme_surface->evaluate(0.5, 0.5)
            : std::expected<Point3, SurfaceError>{
                  std::unexpected{SurfaceError::non_finite_result}};
    const auto extreme_first =
        extreme_surface
            ? extreme_surface->first_derivatives(0.5, 0.5)
            : std::expected<SurfaceFirstDerivatives3, SurfaceError>{
                  std::unexpected{SurfaceError::non_finite_result}};
    passed = require(
                 extreme_surface && extreme_value && extreme_first &&
                     std::isfinite(extreme_value->x()) &&
                     std::isfinite(extreme_value->y()) &&
                     std::isfinite(extreme_value->z()) &&
                     std::isfinite(extreme_first->u.x()) &&
                     std::isfinite(extreme_first->v.x()),
                 "extreme finite extrusion introduced avoidable overflow") &&
             passed;

    const std::array<Point3, 4> overflow_controls{
        *Point3::make(maximum, 0.0, 0.0),
        *Point3::make(maximum, 1.0, 0.0),
        *Point3::make(maximum, 2.0, 0.0),
        *Point3::make(maximum, 3.0, 0.0),
    };
    const auto overflow_displacement =
        Vector3::make(maximum, 0.0, 0.0);
    const auto overflow_surface =
        overflow_displacement
            ? CubicBezierLinearExtrusionSurface3::make(
                  make_curve(overflow_controls),
                  *overflow_displacement)
            : std::expected<
                  CubicBezierLinearExtrusionSurface3,
                  LinearExtrusionSurfaceConstructionError>{
                  std::unexpected{
                      LinearExtrusionSurfaceConstructionError::
                          non_finite_extruded_control}};
    passed = require(
                 !overflow_surface &&
                     overflow_surface.error() ==
                         LinearExtrusionSurfaceConstructionError::
                             non_finite_extruded_control,
                 "non-finite extruded controls were accepted") &&
             passed;

    const auto repeat_value_a = surface->evaluate(0.375, 0.625);
    const auto repeat_value_b = surface->evaluate(0.375, 0.625);
    const auto repeat_first_a =
        surface->first_derivatives(0.375, 0.625);
    const auto repeat_first_b =
        surface->first_derivatives(0.375, 0.625);
    const auto repeat_failure_a = surface->evaluate(-1.0, 0.5);
    const auto repeat_failure_b = surface->evaluate(-1.0, 0.5);
    passed = require(
                 repeat_value_a && repeat_value_b &&
                     *repeat_value_a == *repeat_value_b &&
                     repeat_first_a && repeat_first_b &&
                     *repeat_first_a == *repeat_first_b &&
                     !repeat_failure_a && !repeat_failure_b &&
                     repeat_failure_a.error() == repeat_failure_b.error(),
                 "linear extrusion repeated evidence is not deterministic") &&
             passed;

    return passed ? 0 : 1;
}
