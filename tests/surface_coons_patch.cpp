#include "apmesh/geometry/coons_surface.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
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
    const double scale = 1.0,
    const double absolute = 1.0e-11,
    const double relative = 1.0e-11) {
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

apmesh::core::CubicBezier3 line_curve(
    const apmesh::core::Point3& first,
    const apmesh::core::Point3& last) {
    const auto at = [&](const double t) {
        return point(
            std::lerp(first.x(), last.x(), t),
            std::lerp(first.y(), last.y(), t),
            std::lerp(first.z(), last.z(), t));
    };
    return apmesh::core::CubicBezier3{
        first,
        at(1.0 / 3.0),
        at(2.0 / 3.0),
        last};
}

apmesh::core::CubicBezier3 transform_curve(
    const apmesh::core::CubicBezier3& curve,
    const double scale,
    const double dx,
    const double dy,
    const double dz) {
    const auto& controls = curve.control_points();
    const auto transform = [&](const apmesh::core::Point3& value) {
        return point(
            scale * value.x() + dx,
            scale * value.y() + dy,
            scale * value.z() + dz);
    };
    return apmesh::core::CubicBezier3{
        transform(controls[0]),
        transform(controls[1]),
        transform(controls[2]),
        transform(controls[3])};
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
    const apmesh::core::CubicBezier3& bottom,
    const apmesh::core::CubicBezier3& top,
    const apmesh::core::CubicBezier3& left,
    const apmesh::core::CubicBezier3& right,
    const double u,
    const double v) {
    const auto b = *bottom.evaluate(u);
    const auto t = *top.evaluate(u);
    const auto l = *left.evaluate(v);
    const auto r = *right.evaluate(v);
    const auto bd = *bottom.first_derivative(u);
    const auto td = *top.first_derivative(u);
    const auto ld = *left.first_derivative(v);
    const auto rd = *right.first_derivative(v);
    const auto bdd = *bottom.second_derivative(u);
    const auto tdd = *top.second_derivative(u);
    const auto ldd = *left.second_derivative(v);
    const auto rdd = *right.second_derivative(v);

    const auto& bc = bottom.control_points();
    const auto& tc = top.control_points();
    const auto& p00 = bc[0];
    const auto& p10 = bc[3];
    const auto& p01 = tc[0];
    const auto& p11 = tc[3];

    const long double lu = static_cast<long double>(u);
    const long double lv = static_cast<long double>(v);
    const long double qu = 1.0L - lu;
    const long double qv = 1.0L - lv;

    ReferenceJet result{};

    const auto fill = [&](const double lc,
                          const double rc,
                          const double bc,
                          const double tc,
                          const double ldc,
                          const double rdc,
                          const double bdc,
                          const double tdc,
                          const double lddc,
                          const double rddc,
                          const double bddc,
                          const double tddc,
                          const double c00,
                          const double c10,
                          const double c01,
                          const double c11,
                          long double& value,
                          long double& du,
                          long double& dv,
                          long double& duu,
                          long double& duv,
                          long double& dvv) {
        const long double bilinear =
            qu * qv * static_cast<long double>(c00) +
            lu * qv * static_cast<long double>(c10) +
            qu * lv * static_cast<long double>(c01) +
            lu * lv * static_cast<long double>(c11);
        const long double bilinear_u =
            qv * (static_cast<long double>(c10) -
                  static_cast<long double>(c00)) +
            lv * (static_cast<long double>(c11) -
                  static_cast<long double>(c01));
        const long double bilinear_v =
            qu * (static_cast<long double>(c01) -
                  static_cast<long double>(c00)) +
            lu * (static_cast<long double>(c11) -
                  static_cast<long double>(c10));
        const long double bilinear_uv =
            static_cast<long double>(c00) -
            static_cast<long double>(c10) -
            static_cast<long double>(c01) +
            static_cast<long double>(c11);

        value =
            qu * static_cast<long double>(lc) +
            lu * static_cast<long double>(rc) +
            qv * static_cast<long double>(bc) +
            lv * static_cast<long double>(tc) -
            bilinear;
        du =
            -static_cast<long double>(lc) +
            static_cast<long double>(rc) +
            qv * static_cast<long double>(bdc) +
            lv * static_cast<long double>(tdc) -
            bilinear_u;
        dv =
            qu * static_cast<long double>(ldc) +
            lu * static_cast<long double>(rdc) -
            static_cast<long double>(bc) +
            static_cast<long double>(tc) -
            bilinear_v;
        duu =
            qv * static_cast<long double>(bddc) +
            lv * static_cast<long double>(tddc);
        duv =
            -static_cast<long double>(ldc) +
            static_cast<long double>(rdc) -
            static_cast<long double>(bdc) +
            static_cast<long double>(tdc) -
            bilinear_uv;
        dvv =
            qu * static_cast<long double>(lddc) +
            lu * static_cast<long double>(rddc);
    };

    fill(
        l.x(), r.x(), b.x(), t.x(),
        ld.x(), rd.x(), bd.x(), td.x(),
        ldd.x(), rdd.x(), bdd.x(), tdd.x(),
        p00.x(), p10.x(), p01.x(), p11.x(),
        result.x, result.ux, result.vx,
        result.uux, result.uvx, result.vvx);
    fill(
        l.y(), r.y(), b.y(), t.y(),
        ld.y(), rd.y(), bd.y(), td.y(),
        ldd.y(), rdd.y(), bdd.y(), tdd.y(),
        p00.y(), p10.y(), p01.y(), p11.y(),
        result.y, result.uy, result.vy,
        result.uuy, result.uvy, result.vvy);
    fill(
        l.z(), r.z(), b.z(), t.z(),
        ld.z(), rd.z(), bd.z(), td.z(),
        ldd.z(), rdd.z(), bdd.z(), tdd.z(),
        p00.z(), p10.z(), p01.z(), p11.z(),
        result.z, result.uz, result.vz,
        result.uuz, result.uvz, result.vvz);

    return result;
}

} // namespace

int main() {
    using apmesh::core::BoundedParametricSurface3;
    using apmesh::core::CoonsSurfaceConstructionError;
    using apmesh::core::CubicBezier3;
    using apmesh::core::CubicBezierCoonsPatch3;
    using apmesh::core::SurfaceError;

    static_assert(BoundedParametricSurface3<CubicBezierCoonsPatch3>);

    bool passed = true;

    const auto p00 = point(-2.0, -1.0, 0.5);
    const auto p10 = point(3.0, -0.5, -1.0);
    const auto p01 = point(-1.5, 4.0, 2.0);
    const auto p11 = point(4.0, 3.0, 1.25);

    const CubicBezier3 bottom{
        p00,
        point(-0.5, -2.0, 2.0),
        point(1.5, 1.0, -2.0),
        p10};
    const CubicBezier3 top{
        p01,
        point(0.0, 5.0, -1.0),
        point(2.5, 2.0, 4.0),
        p11};
    const CubicBezier3 left{
        p00,
        point(-3.0, 0.5, 3.0),
        point(0.0, 2.5, -1.5),
        p01};
    const CubicBezier3 right{
        p10,
        point(5.0, 0.0, 2.5),
        point(2.0, 2.5, -2.5),
        p11};

    const auto patch =
        CubicBezierCoonsPatch3::make(bottom, top, left, right);
    if (!patch) {
        return 1;
    }

    const CubicBezier3 bad_left_lower{
        point(-2.0, -1.0, 0.75),
        left.control_points()[1],
        left.control_points()[2],
        left.control_points()[3]};
    const CubicBezier3 bad_right_lower{
        point(3.0, -0.5, -0.75),
        right.control_points()[1],
        right.control_points()[2],
        right.control_points()[3]};
    const CubicBezier3 bad_left_upper{
        left.control_points()[0],
        left.control_points()[1],
        left.control_points()[2],
        point(-1.5, 4.0, 2.25)};
    const CubicBezier3 bad_right_upper{
        right.control_points()[0],
        right.control_points()[1],
        right.control_points()[2],
        point(4.0, 3.0, 1.5)};

    const auto lower_left_failure =
        CubicBezierCoonsPatch3::make(bottom, top, bad_left_lower, right);
    const auto lower_right_failure =
        CubicBezierCoonsPatch3::make(bottom, top, left, bad_right_lower);
    const auto upper_left_failure =
        CubicBezierCoonsPatch3::make(bottom, top, bad_left_upper, right);
    const auto upper_right_failure =
        CubicBezierCoonsPatch3::make(bottom, top, left, bad_right_upper);

    passed = require(
                 !lower_left_failure &&
                     lower_left_failure.error() ==
                         CoonsSurfaceConstructionError::
                             lower_left_corner_mismatch &&
                     !lower_right_failure &&
                     lower_right_failure.error() ==
                         CoonsSurfaceConstructionError::
                             lower_right_corner_mismatch &&
                     !upper_left_failure &&
                     upper_left_failure.error() ==
                         CoonsSurfaceConstructionError::
                             upper_left_corner_mismatch &&
                     !upper_right_failure &&
                     upper_right_failure.error() ==
                         CoonsSurfaceConstructionError::
                             upper_right_corner_mismatch,
                 "Coons corner validation differs") &&
             passed;

    const auto domain = patch->parameter_domain();
    passed = require(
                 domain.u.lower() == 0.0 &&
                     domain.u.upper() == 1.0 &&
                     domain.v.lower() == 0.0 &&
                     domain.v.upper() == 1.0,
                 "Coons parameter domain differs") &&
             passed;

    const std::array<double, 4> boundary_samples{0.0, 0.25, 0.7, 1.0};
    for (const double parameter : boundary_samples) {
        const auto bottom_value = bottom.evaluate(parameter);
        const auto top_value = top.evaluate(parameter);
        const auto left_value = left.evaluate(parameter);
        const auto right_value = right.evaluate(parameter);
        const auto bottom_patch = patch->evaluate(parameter, 0.0);
        const auto top_patch = patch->evaluate(parameter, 1.0);
        const auto left_patch = patch->evaluate(0.0, parameter);
        const auto right_patch = patch->evaluate(1.0, parameter);

        const auto bottom_tangent = bottom.first_derivative(parameter);
        const auto top_tangent = top.first_derivative(parameter);
        const auto left_tangent = left.first_derivative(parameter);
        const auto right_tangent = right.first_derivative(parameter);
        const auto bottom_first = patch->first_derivatives(parameter, 0.0);
        const auto top_first = patch->first_derivatives(parameter, 1.0);
        const auto left_first = patch->first_derivatives(0.0, parameter);
        const auto right_first = patch->first_derivatives(1.0, parameter);

        passed = require(
                     bottom_value && top_value && left_value && right_value &&
                         bottom_patch && top_patch && left_patch && right_patch &&
                         *bottom_patch == *bottom_value &&
                         *top_patch == *top_value &&
                         *left_patch == *left_value &&
                         *right_patch == *right_value &&
                         bottom_tangent && top_tangent &&
                         left_tangent && right_tangent &&
                         bottom_first && top_first && left_first && right_first &&
                         bottom_first->u == *bottom_tangent &&
                         top_first->u == *top_tangent &&
                         left_first->v == *left_tangent &&
                         right_first->v == *right_tangent,
                     "Coons boundary identity/tangent parity differs") &&
                 passed;
    }

    constexpr std::array<std::array<double, 2>, 5> samples{{
        {0.15, 0.2},
        {0.35, 0.65},
        {0.5, 0.5},
        {0.8, 0.3},
        {0.9, 0.85},
    }};
    for (const auto& sample : samples) {
        const double u = sample[0];
        const double v = sample[1];
        const auto reference =
            reference_jet(bottom, top, left, right, u, v);
        const auto value = patch->evaluate(u, v);
        const auto first = patch->first_derivatives(u, v);
        const auto second = patch->second_derivatives(u, v);
        passed = require(
                     value && first && second &&
                         close_scalar(
                             value->x(), static_cast<double>(reference.x), 256.0) &&
                         close_scalar(
                             value->y(), static_cast<double>(reference.y), 256.0) &&
                         close_scalar(
                             value->z(), static_cast<double>(reference.z), 256.0) &&
                         close_scalar(
                             first->u.x(), static_cast<double>(reference.ux), 1024.0) &&
                         close_scalar(
                             first->u.y(), static_cast<double>(reference.uy), 1024.0) &&
                         close_scalar(
                             first->u.z(), static_cast<double>(reference.uz), 1024.0) &&
                         close_scalar(
                             first->v.x(), static_cast<double>(reference.vx), 1024.0) &&
                         close_scalar(
                             first->v.y(), static_cast<double>(reference.vy), 1024.0) &&
                         close_scalar(
                             first->v.z(), static_cast<double>(reference.vz), 1024.0) &&
                         close_scalar(
                             second->uu.x(), static_cast<double>(reference.uux), 4096.0) &&
                         close_scalar(
                             second->uu.y(), static_cast<double>(reference.uuy), 4096.0) &&
                         close_scalar(
                             second->uu.z(), static_cast<double>(reference.uuz), 4096.0) &&
                         close_scalar(
                             second->uv.x(), static_cast<double>(reference.uvx), 4096.0) &&
                         close_scalar(
                             second->uv.y(), static_cast<double>(reference.uvy), 4096.0) &&
                         close_scalar(
                             second->uv.z(), static_cast<double>(reference.uvz), 4096.0) &&
                         close_scalar(
                             second->vv.x(), static_cast<double>(reference.vvx), 4096.0) &&
                         close_scalar(
                             second->vv.y(), static_cast<double>(reference.vvy), 4096.0) &&
                         close_scalar(
                             second->vv.z(), static_cast<double>(reference.vvz), 4096.0),
                     "independent Coons oracle differs") &&
                 passed;
    }

    const auto q00 = point(0.0, 0.0, 0.0);
    const auto q10 = point(1.0, 0.0, 0.0);
    const auto q01 = point(0.0, 1.0, 0.0);
    const auto q11 = point(1.0, 1.0, 1.0);
    const auto bilinear = CubicBezierCoonsPatch3::make(
        line_curve(q00, q10),
        line_curve(q01, q11),
        line_curve(q00, q01),
        line_curve(q10, q11));
    if (!bilinear) {
        return 1;
    }

    for (const auto& sample : samples) {
        const double u = sample[0];
        const double v = sample[1];
        const auto value = bilinear->evaluate(u, v);
        const auto first = bilinear->first_derivatives(u, v);
        const auto second = bilinear->second_derivatives(u, v);
        passed = require(
                     value && first && second &&
                         close_scalar(value->x(), u, 64.0) &&
                         close_scalar(value->y(), v, 64.0) &&
                         close_scalar(value->z(), u * v, 64.0) &&
                         close_scalar(first->u.x(), 1.0, 64.0) &&
                         close_scalar(first->u.y(), 0.0, 64.0) &&
                         close_scalar(first->u.z(), v, 64.0) &&
                         close_scalar(first->v.x(), 0.0, 64.0) &&
                         close_scalar(first->v.y(), 1.0, 64.0) &&
                         close_scalar(first->v.z(), u, 64.0) &&
                         close_vector(
                             second->uu,
                             *apmesh::core::Vector3::make(0.0, 0.0, 0.0),
                             64.0) &&
                         close_scalar(second->uv.x(), 0.0, 64.0) &&
                         close_scalar(second->uv.y(), 0.0, 64.0) &&
                         close_scalar(second->uv.z(), 1.0, 64.0) &&
                         close_vector(
                             second->vv,
                             *apmesh::core::Vector3::make(0.0, 0.0, 0.0),
                             64.0),
                     "analytic bilinear Coons fixture differs") &&
                 passed;
    }

    const auto plane = CubicBezierCoonsPatch3::make(
        line_curve(q00, q10),
        line_curve(q01, point(1.0, 1.0, 0.0)),
        line_curve(q00, q01),
        line_curve(q10, point(1.0, 1.0, 0.0)));
    if (!plane) {
        return 1;
    }
    const auto plane_value = plane->evaluate(0.4, 0.6);
    const auto plane_first = plane->first_derivatives(0.4, 0.6);
    const auto plane_second = plane->second_derivatives(0.4, 0.6);
    passed = require(
                 plane_value && plane_first && plane_second &&
                     close_scalar(plane_value->x(), 0.4, 64.0) &&
                     close_scalar(plane_value->y(), 0.6, 64.0) &&
                     close_scalar(plane_value->z(), 0.0, 64.0) &&
                     close_vector(
                         plane_first->u,
                         *apmesh::core::Vector3::make(1.0, 0.0, 0.0),
                         64.0) &&
                     close_vector(
                         plane_first->v,
                         *apmesh::core::Vector3::make(0.0, 1.0, 0.0),
                         64.0) &&
                     close_vector(
                         plane_second->uu,
                         *apmesh::core::Vector3::make(0.0, 0.0, 0.0),
                         64.0) &&
                     close_vector(
                         plane_second->uv,
                         *apmesh::core::Vector3::make(0.0, 0.0, 0.0),
                         64.0) &&
                     close_vector(
                         plane_second->vv,
                         *apmesh::core::Vector3::make(0.0, 0.0, 0.0),
                         64.0),
                 "analytic plane Coons fixture differs") &&
             passed;

    const auto u_reversed = patch->u_reversed();
    const auto v_reversed = patch->v_reversed();
    passed = require(
                 u_reversed.u_reversed() == *patch &&
                     v_reversed.v_reversed() == *patch,
                 "Coons reversal is not an involution") &&
             passed;

    constexpr double ru = 0.27;
    constexpr double rv = 0.68;
    const auto original_value = patch->evaluate(ru, rv);
    const auto original_first = patch->first_derivatives(ru, rv);
    const auto original_second = patch->second_derivatives(ru, rv);
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
                     close_point(*original_value, *u_value, 512.0) &&
                     close_scalar(u_first->u.x(), -original_first->u.x(), 2048.0) &&
                     close_scalar(u_first->u.y(), -original_first->u.y(), 2048.0) &&
                     close_scalar(u_first->u.z(), -original_first->u.z(), 2048.0) &&
                     close_vector(u_first->v, original_first->v, 2048.0) &&
                     close_vector(u_second->uu, original_second->uu, 8192.0) &&
                     close_scalar(u_second->uv.x(), -original_second->uv.x(), 8192.0) &&
                     close_scalar(u_second->uv.y(), -original_second->uv.y(), 8192.0) &&
                     close_scalar(u_second->uv.z(), -original_second->uv.z(), 8192.0) &&
                     close_vector(u_second->vv, original_second->vv, 8192.0) &&
                     close_point(*original_value, *v_value, 512.0) &&
                     close_vector(v_first->u, original_first->u, 2048.0) &&
                     close_scalar(v_first->v.x(), -original_first->v.x(), 2048.0) &&
                     close_scalar(v_first->v.y(), -original_first->v.y(), 2048.0) &&
                     close_scalar(v_first->v.z(), -original_first->v.z(), 2048.0) &&
                     close_vector(v_second->uu, original_second->uu, 8192.0) &&
                     close_scalar(v_second->uv.x(), -original_second->uv.x(), 8192.0) &&
                     close_scalar(v_second->uv.y(), -original_second->uv.y(), 8192.0) &&
                     close_scalar(v_second->uv.z(), -original_second->uv.z(), 8192.0) &&
                     close_vector(v_second->vv, original_second->vv, 8192.0),
                 "Coons reversal covariance differs") &&
             passed;

    const auto constant_point = point(3.0, -4.0, 2.0);
    const CubicBezier3 constant_curve{
        constant_point, constant_point, constant_point, constant_point};
    const auto constant = CubicBezierCoonsPatch3::make(
        constant_curve, constant_curve, constant_curve, constant_curve);
    if (!constant) {
        return 1;
    }
    const auto constant_value = constant->evaluate(0.3, 0.7);
    const auto constant_first = constant->first_derivatives(0.3, 0.7);
    const auto constant_second = constant->second_derivatives(0.3, 0.7);
    const auto zero = *apmesh::core::Vector3::make(0.0, 0.0, 0.0);
    passed = require(
                 constant_value && *constant_value == constant_point &&
                     constant_first && constant_second &&
                     constant_first->u == zero &&
                     constant_first->v == zero &&
                     constant_second->uu == zero &&
                     constant_second->uv == zero &&
                     constant_second->vv == zero,
                 "constant Coons patch semantics differ") &&
             passed;

    const auto translated = CubicBezierCoonsPatch3::make(
        transform_curve(bottom, 1.0, 8.0, -6.0, 3.0),
        transform_curve(top, 1.0, 8.0, -6.0, 3.0),
        transform_curve(left, 1.0, 8.0, -6.0, 3.0),
        transform_curve(right, 1.0, 8.0, -6.0, 3.0));
    const auto scaled = CubicBezierCoonsPatch3::make(
        transform_curve(bottom, 2.0, 0.0, 0.0, 0.0),
        transform_curve(top, 2.0, 0.0, 0.0, 0.0),
        transform_curve(left, 2.0, 0.0, 0.0, 0.0),
        transform_curve(right, 2.0, 0.0, 0.0, 0.0));
    if (!translated || !scaled) {
        return 1;
    }

    const auto base_value = patch->evaluate(0.41, 0.59);
    const auto translated_value = translated->evaluate(0.41, 0.59);
    const auto scaled_value = scaled->evaluate(0.41, 0.59);
    const auto base_first = patch->first_derivatives(0.41, 0.59);
    const auto translated_first = translated->first_derivatives(0.41, 0.59);
    const auto scaled_first = scaled->first_derivatives(0.41, 0.59);
    const auto base_second = patch->second_derivatives(0.41, 0.59);
    const auto translated_second = translated->second_derivatives(0.41, 0.59);
    const auto scaled_second = scaled->second_derivatives(0.41, 0.59);

    passed = require(
                 base_value && translated_value && scaled_value &&
                     close_scalar(translated_value->x() - base_value->x(), 8.0, 512.0) &&
                     close_scalar(translated_value->y() - base_value->y(), -6.0, 512.0) &&
                     close_scalar(translated_value->z() - base_value->z(), 3.0, 512.0) &&
                     close_scalar(scaled_value->x(), 2.0 * base_value->x(), 512.0) &&
                     close_scalar(scaled_value->y(), 2.0 * base_value->y(), 512.0) &&
                     close_scalar(scaled_value->z(), 2.0 * base_value->z(), 512.0) &&
                     base_first && translated_first && scaled_first &&
                     close_vector(translated_first->u, base_first->u, 2048.0) &&
                     close_vector(translated_first->v, base_first->v, 2048.0) &&
                     close_scalar(scaled_first->u.x(), 2.0 * base_first->u.x(), 2048.0) &&
                     close_scalar(scaled_first->v.z(), 2.0 * base_first->v.z(), 2048.0) &&
                     base_second && translated_second && scaled_second &&
                     close_vector(translated_second->uu, base_second->uu, 8192.0) &&
                     close_vector(translated_second->uv, base_second->uv, 8192.0) &&
                     close_vector(translated_second->vv, base_second->vv, 8192.0) &&
                     close_scalar(scaled_second->uu.x(), 2.0 * base_second->uu.x(), 8192.0) &&
                     close_scalar(scaled_second->uv.y(), 2.0 * base_second->uv.y(), 8192.0) &&
                     close_scalar(scaled_second->vv.z(), 2.0 * base_second->vv.z(), 8192.0),
                 "Coons affine covariance differs") &&
             passed;

    const double extreme_scale = std::ldexp(1.0, 500);
    const auto extreme = CubicBezierCoonsPatch3::make(
        transform_curve(bottom, extreme_scale, 0.0, 0.0, 0.0),
        transform_curve(top, extreme_scale, 0.0, 0.0, 0.0),
        transform_curve(left, extreme_scale, 0.0, 0.0, 0.0),
        transform_curve(right, extreme_scale, 0.0, 0.0, 0.0));
    if (!extreme) {
        return 1;
    }
    const auto extreme_value = extreme->evaluate(0.37, 0.62);
    const auto extreme_first = extreme->first_derivatives(0.37, 0.62);
    const auto extreme_second = extreme->second_derivatives(0.37, 0.62);
    passed = require(
                 extreme_value && extreme_first && extreme_second &&
                     std::isfinite(extreme_value->x()) &&
                     std::isfinite(extreme_value->y()) &&
                     std::isfinite(extreme_value->z()) &&
                     std::isfinite(extreme_first->u.x()) &&
                     std::isfinite(extreme_first->v.y()) &&
                     std::isfinite(extreme_second->uv.z()),
                 "Coons extreme finite jet introduced avoidable overflow") &&
             passed;

    const double maximum = std::numeric_limits<double>::max();
    const auto huge_left = point(-maximum, 0.0, 0.0);
    const auto huge_right = point(maximum, 0.0, 0.0);
    const CubicBezier3 overflow_bottom{
        huge_left, huge_right, huge_right, huge_right};
    const CubicBezier3 overflow_top{
        huge_left, huge_right, huge_right, huge_right};
    const CubicBezier3 overflow_left{
        huge_left, huge_left, huge_left, huge_left};
    const CubicBezier3 overflow_right{
        huge_right, huge_right, huge_right, huge_right};
    const auto overflow_patch = CubicBezierCoonsPatch3::make(
        overflow_bottom, overflow_top, overflow_left, overflow_right);
    if (!overflow_patch) {
        return 1;
    }
    const auto overflow_first =
        overflow_patch->first_derivatives(0.0, 0.5);
    passed = require(
                 !overflow_first &&
                     overflow_first.error() == SurfaceError::non_finite_result,
                 "unrepresentable Coons derivative did not fail explicitly") &&
             passed;

    const auto nan_u =
        patch->evaluate(std::numeric_limits<double>::quiet_NaN(), 0.5);
    const auto nan_v =
        patch->evaluate(0.5, std::numeric_limits<double>::quiet_NaN());
    const auto below_u = patch->first_derivatives(-0.1, 0.5);
    const auto above_v = patch->second_derivatives(0.5, 1.1);
    passed = require(
                 !nan_u &&
                     nan_u.error() == SurfaceError::non_finite_u_parameter &&
                     !nan_v &&
                     nan_v.error() == SurfaceError::non_finite_v_parameter &&
                     !below_u &&
                     below_u.error() == SurfaceError::u_parameter_out_of_domain &&
                     !above_v &&
                     above_v.error() == SurfaceError::v_parameter_out_of_domain,
                 "Coons parameter failure semantics differ") &&
             passed;

    const auto repeat_value_a = patch->evaluate(0.35, 0.65);
    const auto repeat_value_b = patch->evaluate(0.35, 0.65);
    const auto repeat_first_a = patch->first_derivatives(0.35, 0.65);
    const auto repeat_first_b = patch->first_derivatives(0.35, 0.65);
    const auto repeat_second_a = patch->second_derivatives(0.35, 0.65);
    const auto repeat_second_b = patch->second_derivatives(0.35, 0.65);
    const auto repeat_failure_a = patch->evaluate(1.1, 0.5);
    const auto repeat_failure_b = patch->evaluate(1.1, 0.5);
    const auto repeat_construct_a =
        CubicBezierCoonsPatch3::make(bottom, top, bad_left_lower, right);
    const auto repeat_construct_b =
        CubicBezierCoonsPatch3::make(bottom, top, bad_left_lower, right);
    passed = require(
                 repeat_value_a && repeat_value_b &&
                     *repeat_value_a == *repeat_value_b &&
                     repeat_first_a && repeat_first_b &&
                     *repeat_first_a == *repeat_first_b &&
                     repeat_second_a && repeat_second_b &&
                     *repeat_second_a == *repeat_second_b &&
                     !repeat_failure_a && !repeat_failure_b &&
                     repeat_failure_a.error() == repeat_failure_b.error() &&
                     !repeat_construct_a && !repeat_construct_b &&
                     repeat_construct_a.error() == repeat_construct_b.error(),
                 "Coons repeated evidence is not deterministic") &&
             passed;

    return passed ? 0 : 1;
}
