#include "apmesh/geometry/surface.hpp"

#include "apmesh/geometry/rational_bezier.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
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
    const double absolute = 1.0e-11,
    const double relative = 1.0e-11) {
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

long double bernstein2(const int index, const long double t) {
    const long double q = 1.0L - t;
    if (index == 0) {
        return q * q;
    }
    if (index == 1) {
        return 2.0L * t * q;
    }
    if (index == 2) {
        return t * t;
    }
    return 0.0L;
}

long double bernstein1(const int index, const long double t) {
    if (index == 0) {
        return 1.0L - t;
    }
    if (index == 1) {
        return t;
    }
    return 0.0L;
}

long double bernstein3(const int index, const long double t) {
    const long double q = 1.0L - t;
    if (index == 0) {
        return q * q * q;
    }
    if (index == 1) {
        return 3.0L * t * q * q;
    }
    if (index == 2) {
        return 3.0L * t * t * q;
    }
    if (index == 3) {
        return t * t * t;
    }
    return 0.0L;
}

long double bernstein3_first(const int index, const long double t) {
    return 3.0L *
           (bernstein2(index - 1, t) - bernstein2(index, t));
}

long double bernstein3_second(const int index, const long double t) {
    return 6.0L *
           (bernstein1(index - 2, t) -
            2.0L * bernstein1(index - 1, t) +
            bernstein1(index, t));
}

struct LVec3 {
    long double x{};
    long double y{};
    long double z{};
};

LVec3 add_scaled(
    LVec3 value,
    const apmesh::core::Point3& point,
    const long double scale) {
    value.x += scale * static_cast<long double>(point.x());
    value.y += scale * static_cast<long double>(point.y());
    value.z += scale * static_cast<long double>(point.z());
    return value;
}

LVec3 subtract_scaled(
    const LVec3& lhs,
    const LVec3& rhs,
    const long double scale) {
    return {
        lhs.x - scale * rhs.x,
        lhs.y - scale * rhs.y,
        lhs.z - scale * rhs.z,
    };
}

LVec3 divide(const LVec3& value, const long double divisor) {
    return {value.x / divisor, value.y / divisor, value.z / divisor};
}

struct ReferenceJet {
    LVec3 value{};
    LVec3 u{};
    LVec3 v{};
    LVec3 uu{};
    LVec3 uv{};
    LVec3 vv{};
};

ReferenceJet reference_jet(
    const apmesh::core::RationalBicubicBezierPatch3::ControlNet& controls,
    const apmesh::core::RationalBicubicBezierPatch3::WeightNet& weights,
    const double u,
    const double v) {
    const long double lu = static_cast<long double>(u);
    const long double lv = static_cast<long double>(v);

    LVec3 a{};
    LVec3 au{};
    LVec3 av{};
    LVec3 auu{};
    LVec3 auv{};
    LVec3 avv{};
    long double w = 0.0L;
    long double wu = 0.0L;
    long double wv = 0.0L;
    long double wuu = 0.0L;
    long double wuv = 0.0L;
    long double wvv = 0.0L;

    long double maximum_weight = 0.0L;
    for (const auto& row : weights) {
        for (const double weight : row) {
            maximum_weight =
                std::max(maximum_weight, static_cast<long double>(weight));
        }
    }

    for (int i = 0; i < 4; ++i) {
        const long double bu = bernstein3(i, lu);
        const long double du = bernstein3_first(i, lu);
        const long double ddu = bernstein3_second(i, lu);
        for (int j = 0; j < 4; ++j) {
            const long double bv = bernstein3(j, lv);
            const long double dv = bernstein3_first(j, lv);
            const long double ddv = bernstein3_second(j, lv);
            const long double weight =
                static_cast<long double>(
                    weights[static_cast<std::size_t>(i)]
                           [static_cast<std::size_t>(j)]) /
                maximum_weight;
            const auto& point =
                controls[static_cast<std::size_t>(i)]
                        [static_cast<std::size_t>(j)];

            const long double b = bu * bv * weight;
            const long double buv = du * bv * weight;
            const long double bvu = bu * dv * weight;
            const long double buu = ddu * bv * weight;
            const long double buv2 = du * dv * weight;
            const long double bvv = bu * ddv * weight;

            a = add_scaled(a, point, b);
            au = add_scaled(au, point, buv);
            av = add_scaled(av, point, bvu);
            auu = add_scaled(auu, point, buu);
            auv = add_scaled(auv, point, buv2);
            avv = add_scaled(avv, point, bvv);

            w += b;
            wu += buv;
            wv += bvu;
            wuu += buu;
            wuv += buv2;
            wvv += bvv;
        }
    }

    ReferenceJet result{};
    result.value = divide(a, w);
    result.u = divide(subtract_scaled(au, result.value, wu), w);
    result.v = divide(subtract_scaled(av, result.value, wv), w);

    LVec3 uu_numerator =
        subtract_scaled(auu, result.u, 2.0L * wu);
    uu_numerator =
        subtract_scaled(uu_numerator, result.value, wuu);
    result.uu = divide(uu_numerator, w);

    LVec3 vv_numerator =
        subtract_scaled(avv, result.v, 2.0L * wv);
    vv_numerator =
        subtract_scaled(vv_numerator, result.value, wvv);
    result.vv = divide(vv_numerator, w);

    LVec3 uv_numerator = subtract_scaled(auv, result.u, wv);
    uv_numerator = subtract_scaled(uv_numerator, result.v, wu);
    uv_numerator = subtract_scaled(uv_numerator, result.value, wuv);
    result.uv = divide(uv_numerator, w);
    return result;
}

apmesh::core::RationalBicubicBezierPatch3::ControlNet make_controls() {
    using apmesh::core::Point3;
    const auto seed = *Point3::make(0.0, 0.0, 0.0);
    apmesh::core::RationalBicubicBezierPatch3::ControlNet controls{{
        {{seed, seed, seed, seed}},
        {{seed, seed, seed, seed}},
        {{seed, seed, seed, seed}},
        {{seed, seed, seed, seed}},
    }};

    constexpr std::array<std::array<std::array<double, 3>, 4>, 4> data{{
        {{{-3.0, 0.0, 1.0}, {-2.0, 2.0, -1.0}, {-1.0, 4.0, 2.0}, {0.0, 6.0, 0.5}}},
        {{{0.0, -1.0, 2.0}, {1.0, 1.5, 4.0}, {2.0, 3.0, -2.0}, {3.0, 5.0, 1.0}}},
        {{{3.5, -2.0, -1.0}, {4.0, 0.5, 3.0}, {5.0, 2.5, 5.0}, {6.0, 4.5, -0.5}}},
        {{{7.0, -3.0, 0.0}, {8.0, -0.5, 2.5}, {9.0, 2.0, -1.5}, {10.0, 4.0, 3.0}}},
    }};

    for (std::size_t i = 0; i < 4U; ++i) {
        for (std::size_t j = 0; j < 4U; ++j) {
            controls[i][j] = *Point3::make(
                data[i][j][0], data[i][j][1], data[i][j][2]);
        }
    }
    return controls;
}

apmesh::core::RationalBicubicBezierPatch3::WeightNet make_weights() {
    return {{
        {{1.0, 2.0, 0.5, 4.0}},
        {{3.0, 0.75, 5.0, 1.25}},
        {{0.25, 6.0, 1.5, 2.5}},
        {{4.5, 0.625, 3.5, 1.0}},
    }};
}

apmesh::core::RationalBicubicBezierPatch3::ControlNet translate_controls(
    const apmesh::core::RationalBicubicBezierPatch3::ControlNet& source,
    const double dx,
    const double dy,
    const double dz) {
    auto result = source;
    for (auto& row : result) {
        for (auto& point : row) {
            point = *apmesh::core::Point3::make(
                point.x() + dx, point.y() + dy, point.z() + dz);
        }
    }
    return result;
}

apmesh::core::RationalBicubicBezierPatch3::ControlNet scale_controls(
    const apmesh::core::RationalBicubicBezierPatch3::ControlNet& source,
    const double factor) {
    auto result = source;
    for (auto& row : result) {
        for (auto& point : row) {
            point = *apmesh::core::Point3::make(
                factor * point.x(), factor * point.y(), factor * point.z());
        }
    }
    return result;
}

struct H3 {
    long double x{};
    long double y{};
    long double z{};
    long double w{};
};

H3 hpoint(const apmesh::core::Point3& point, const double weight) {
    const long double w = static_cast<long double>(weight);
    return {
        static_cast<long double>(point.x()) * w,
        static_cast<long double>(point.y()) * w,
        static_cast<long double>(point.z()) * w,
        w,
    };
}

H3 blend(const H3& lhs, const H3& rhs, const long double alpha) {
    return {
        (1.0L - alpha) * lhs.x + alpha * rhs.x,
        (1.0L - alpha) * lhs.y + alpha * rhs.y,
        (1.0L - alpha) * lhs.z + alpha * rhs.z,
        (1.0L - alpha) * lhs.w + alpha * rhs.w,
    };
}

struct ElevatedCubic {
    std::array<apmesh::core::Point3, 4> points;
    std::array<double, 4> weights;
};

ElevatedCubic elevate_quadratic(
    const std::array<apmesh::core::Point3, 3>& points,
    const std::array<double, 3>& weights) {
    const H3 h0 = hpoint(points[0], weights[0]);
    const H3 h1 = hpoint(points[1], weights[1]);
    const H3 h2 = hpoint(points[2], weights[2]);
    const std::array<H3, 4> elevated{
        h0,
        blend(h0, h1, 2.0L / 3.0L),
        blend(h1, h2, 1.0L / 3.0L),
        h2,
    };

    const auto seed = *apmesh::core::Point3::make(0.0, 0.0, 0.0);
    ElevatedCubic result{{seed, seed, seed, seed}, {1.0, 1.0, 1.0, 1.0}};
    for (std::size_t index = 0; index < 4U; ++index) {
        result.weights[index] = static_cast<double>(elevated[index].w);
        result.points[index] = *apmesh::core::Point3::make(
            static_cast<double>(elevated[index].x / elevated[index].w),
            static_cast<double>(elevated[index].y / elevated[index].w),
            static_cast<double>(elevated[index].z / elevated[index].w));
    }
    return result;
}

} // namespace

int main() {
    using apmesh::core::BicubicBezierPatch3;
    using apmesh::core::BoundedParametricSurface3;
    using apmesh::core::Point3;
    using apmesh::core::RationalBicubicBezierPatch3;
    using apmesh::core::RationalQuadraticBezier3;
    using apmesh::core::RationalSurfaceConstructionError;
    using apmesh::core::SurfaceError;

    static_assert(BoundedParametricSurface3<RationalBicubicBezierPatch3>);

    bool passed = true;
    const auto controls = make_controls();
    const auto weights = make_weights();

    const auto patch = RationalBicubicBezierPatch3::make(controls, weights);
    if (!patch) {
        return 1;
    }

    auto bad_weights = weights;
    bad_weights[1][2] = std::numeric_limits<double>::quiet_NaN();
    const auto nan_weight =
        RationalBicubicBezierPatch3::make(controls, bad_weights);
    bad_weights = weights;
    bad_weights[2][0] = std::numeric_limits<double>::infinity();
    const auto inf_weight =
        RationalBicubicBezierPatch3::make(controls, bad_weights);
    bad_weights = weights;
    bad_weights[0][3] = 0.0;
    const auto zero_weight =
        RationalBicubicBezierPatch3::make(controls, bad_weights);
    bad_weights = weights;
    bad_weights[3][1] = -1.0;
    const auto negative_weight =
        RationalBicubicBezierPatch3::make(controls, bad_weights);

    passed = require(
                 !nan_weight &&
                     nan_weight.error() ==
                         RationalSurfaceConstructionError::non_finite_weight &&
                     !inf_weight &&
                     inf_weight.error() ==
                         RationalSurfaceConstructionError::non_finite_weight &&
                     !zero_weight &&
                     zero_weight.error() ==
                         RationalSurfaceConstructionError::non_positive_weight &&
                     !negative_weight &&
                     negative_weight.error() ==
                         RationalSurfaceConstructionError::non_positive_weight,
                 "rational surface weight validation differs") &&
             passed;

    const auto domain = patch->parameter_domain();
    passed = require(
                 domain.u.lower() == 0.0 && domain.u.upper() == 1.0 &&
                     domain.v.lower() == 0.0 && domain.v.upper() == 1.0,
                 "rational surface domain is not exactly [0,1]^2") &&
             passed;

    const auto c00 = patch->evaluate(0.0, 0.0);
    const auto c01 = patch->evaluate(0.0, 1.0);
    const auto c10 = patch->evaluate(1.0, 0.0);
    const auto c11 = patch->evaluate(1.0, 1.0);
    passed = require(
                 c00 && *c00 == controls[0][0] &&
                     c01 && *c01 == controls[0][3] &&
                     c10 && *c10 == controls[3][0] &&
                     c11 && *c11 == controls[3][3],
                 "rational surface corners are not exact") &&
             passed;

    constexpr std::array<std::array<double, 2>, 8> samples{{
        {{0.125, 0.2}},
        {{0.35, 0.7}},
        {{0.5, 0.5}},
        {{0.8, 0.15}},
        {{0.0, 0.4}},
        {{1.0, 0.6}},
        {{0.3, 0.0}},
        {{0.65, 1.0}},
    }};

    for (const auto& sample : samples) {
        const double u = sample[0];
        const double v = sample[1];
        const auto reference = reference_jet(controls, weights, u, v);
        const auto value = patch->evaluate(u, v);
        const auto first = patch->first_derivatives(u, v);
        const auto second = patch->second_derivatives(u, v);

        passed = require(
                     value &&
                         close_scalar(value->x(), static_cast<double>(reference.value.x), 128.0) &&
                         close_scalar(value->y(), static_cast<double>(reference.value.y), 128.0) &&
                         close_scalar(value->z(), static_cast<double>(reference.value.z), 128.0) &&
                         first &&
                         close_scalar(first->u.x(), static_cast<double>(reference.u.x), 512.0) &&
                         close_scalar(first->u.y(), static_cast<double>(reference.u.y), 512.0) &&
                         close_scalar(first->u.z(), static_cast<double>(reference.u.z), 512.0) &&
                         close_scalar(first->v.x(), static_cast<double>(reference.v.x), 512.0) &&
                         close_scalar(first->v.y(), static_cast<double>(reference.v.y), 512.0) &&
                         close_scalar(first->v.z(), static_cast<double>(reference.v.z), 512.0) &&
                         second &&
                         close_scalar(second->uu.x(), static_cast<double>(reference.uu.x), 2048.0) &&
                         close_scalar(second->uu.y(), static_cast<double>(reference.uu.y), 2048.0) &&
                         close_scalar(second->uu.z(), static_cast<double>(reference.uu.z), 2048.0) &&
                         close_scalar(second->uv.x(), static_cast<double>(reference.uv.x), 2048.0) &&
                         close_scalar(second->uv.y(), static_cast<double>(reference.uv.y), 2048.0) &&
                         close_scalar(second->uv.z(), static_cast<double>(reference.uv.z), 2048.0) &&
                         close_scalar(second->vv.x(), static_cast<double>(reference.vv.x), 2048.0) &&
                         close_scalar(second->vv.y(), static_cast<double>(reference.vv.y), 2048.0) &&
                         close_scalar(second->vv.z(), static_cast<double>(reference.vv.z), 2048.0),
                     "rational surface independent Bernstein oracle differs") &&
                 passed;
    }

    RationalBicubicBezierPatch3::WeightNet equal_weights{};
    for (auto& row : equal_weights) {
        row.fill(4.0);
    }
    const auto equal_patch =
        RationalBicubicBezierPatch3::make(controls, equal_weights);
    const BicubicBezierPatch3 polynomial{controls};
    if (!equal_patch) {
        return 1;
    }

    constexpr std::array<std::array<double, 2>, 4> parity_samples{{
        {{0.1, 0.2}}, {{0.4, 0.8}}, {{0.7, 0.35}}, {{0.9, 0.9}},
    }};
    for (const auto& sample : parity_samples) {
        const auto rv = equal_patch->evaluate(sample[0], sample[1]);
        const auto pv = polynomial.evaluate(sample[0], sample[1]);
        const auto rf = equal_patch->first_derivatives(sample[0], sample[1]);
        const auto pf = polynomial.first_derivatives(sample[0], sample[1]);
        const auto rs = equal_patch->second_derivatives(sample[0], sample[1]);
        const auto ps = polynomial.second_derivatives(sample[0], sample[1]);
        passed = require(
                     rv && pv && close_point(*rv, *pv, 128.0) &&
                         rf && pf &&
                         close_vector(rf->u, pf->u, 512.0) &&
                         close_vector(rf->v, pf->v, 512.0) &&
                         rs && ps &&
                         close_vector(rs->uu, ps->uu, 2048.0) &&
                         close_vector(rs->uv, ps->uv, 2048.0) &&
                         close_vector(rs->vv, ps->vv, 2048.0),
                     "equal-weight polynomial surface parity differs") &&
                 passed;
    }

    auto scaled_weights = weights;
    for (auto& row : scaled_weights) {
        for (double& weight : row) {
            weight *= 8.0;
        }
    }
    const auto weight_scaled =
        RationalBicubicBezierPatch3::make(controls, scaled_weights);
    if (!weight_scaled) {
        return 1;
    }
    for (const auto& sample : parity_samples) {
        const auto av = patch->evaluate(sample[0], sample[1]);
        const auto bv = weight_scaled->evaluate(sample[0], sample[1]);
        const auto af = patch->first_derivatives(sample[0], sample[1]);
        const auto bf = weight_scaled->first_derivatives(sample[0], sample[1]);
        const auto as = patch->second_derivatives(sample[0], sample[1]);
        const auto bs = weight_scaled->second_derivatives(sample[0], sample[1]);
        passed = require(
                     av && bv && close_point(*av, *bv, 128.0) &&
                         af && bf &&
                         close_vector(af->u, bf->u, 512.0) &&
                         close_vector(af->v, bf->v, 512.0) &&
                         as && bs &&
                         close_vector(as->uu, bs->uu, 2048.0) &&
                         close_vector(as->uv, bs->uv, 2048.0) &&
                         close_vector(as->vv, bs->vv, 2048.0),
                     "rational surface common-weight scale differs") &&
                 passed;
    }

    const auto u_reversed = patch->u_reversed();
    const auto v_reversed = patch->v_reversed();
    passed = require(
                 u_reversed.u_reversed() == *patch &&
                     v_reversed.v_reversed() == *patch,
                 "rational surface reversal is not an involution") &&
             passed;

    constexpr double ru = 0.27;
    constexpr double rv = 0.61;
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
                     v_value && v_first && v_second &&
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
                 "rational surface reversal covariance differs") &&
             passed;

    const auto constant_point = *Point3::make(3.0, -4.0, 2.0);
    RationalBicubicBezierPatch3::ControlNet constant_controls{{
        {{constant_point, constant_point, constant_point, constant_point}},
        {{constant_point, constant_point, constant_point, constant_point}},
        {{constant_point, constant_point, constant_point, constant_point}},
        {{constant_point, constant_point, constant_point, constant_point}},
    }};
    const auto constant_patch =
        RationalBicubicBezierPatch3::make(constant_controls, weights);
    if (!constant_patch) {
        return 1;
    }
    const auto constant_value = constant_patch->evaluate(0.37, 0.73);
    const auto constant_first = constant_patch->first_derivatives(0.37, 0.73);
    const auto constant_second = constant_patch->second_derivatives(0.37, 0.73);
    passed = require(
                 constant_value && *constant_value == constant_point &&
                     constant_first &&
                     constant_first->u.x() == 0.0 &&
                     constant_first->u.y() == 0.0 &&
                     constant_first->u.z() == 0.0 &&
                     constant_first->v.x() == 0.0 &&
                     constant_first->v.y() == 0.0 &&
                     constant_first->v.z() == 0.0 &&
                     constant_second &&
                     constant_second->uu.x() == 0.0 &&
                     constant_second->uu.y() == 0.0 &&
                     constant_second->uu.z() == 0.0 &&
                     constant_second->uv.x() == 0.0 &&
                     constant_second->uv.y() == 0.0 &&
                     constant_second->uv.z() == 0.0 &&
                     constant_second->vv.x() == 0.0 &&
                     constant_second->vv.y() == 0.0 &&
                     constant_second->vv.z() == 0.0,
                 "rational constant surface semantics differ") &&
             passed;

    const auto translated_controls =
        translate_controls(controls, 8.0, -6.0, 3.0);
    const auto scaled_controls = scale_controls(controls, 2.0);
    const auto translated_patch =
        RationalBicubicBezierPatch3::make(translated_controls, weights);
    const auto coordinate_scaled_patch =
        RationalBicubicBezierPatch3::make(scaled_controls, weights);
    if (!translated_patch || !coordinate_scaled_patch) {
        return 1;
    }

    const auto base_value = patch->evaluate(0.42, 0.58);
    const auto translated_value = translated_patch->evaluate(0.42, 0.58);
    const auto scaled_value = coordinate_scaled_patch->evaluate(0.42, 0.58);
    const auto base_first = patch->first_derivatives(0.42, 0.58);
    const auto translated_first =
        translated_patch->first_derivatives(0.42, 0.58);
    const auto scaled_first =
        coordinate_scaled_patch->first_derivatives(0.42, 0.58);
    const auto base_second = patch->second_derivatives(0.42, 0.58);
    const auto translated_second =
        translated_patch->second_derivatives(0.42, 0.58);
    const auto scaled_second =
        coordinate_scaled_patch->second_derivatives(0.42, 0.58);

    passed = require(
                 base_value && translated_value && scaled_value &&
                     close_scalar(translated_value->x() - base_value->x(), 8.0, 64.0) &&
                     close_scalar(translated_value->y() - base_value->y(), -6.0, 64.0) &&
                     close_scalar(translated_value->z() - base_value->z(), 3.0, 64.0) &&
                     close_scalar(scaled_value->x(), 2.0 * base_value->x(), 128.0) &&
                     close_scalar(scaled_value->y(), 2.0 * base_value->y(), 128.0) &&
                     close_scalar(scaled_value->z(), 2.0 * base_value->z(), 128.0) &&
                     base_first && translated_first && scaled_first &&
                     close_vector(base_first->u, translated_first->u, 512.0) &&
                     close_vector(base_first->v, translated_first->v, 512.0) &&
                     close_scalar(scaled_first->u.x(), 2.0 * base_first->u.x(), 1024.0) &&
                     close_scalar(scaled_first->v.y(), 2.0 * base_first->v.y(), 1024.0) &&
                     base_second && translated_second && scaled_second &&
                     close_vector(base_second->uu, translated_second->uu, 2048.0) &&
                     close_vector(base_second->uv, translated_second->uv, 2048.0) &&
                     close_vector(base_second->vv, translated_second->vv, 2048.0) &&
                     close_scalar(scaled_second->uu.x(), 2.0 * base_second->uu.x(), 4096.0) &&
                     close_scalar(scaled_second->uv.y(), 2.0 * base_second->uv.y(), 4096.0) &&
                     close_scalar(scaled_second->vv.z(), 2.0 * base_second->vv.z(), 4096.0),
                 "rational surface affine covariance differs") &&
             passed;

    const auto q0 = *Point3::make(1.0, 0.0, 0.0);
    const auto q1 = *Point3::make(1.0, 1.0, 0.5);
    const auto q2 = *Point3::make(0.0, 1.0, 1.0);
    const std::array<Point3, 3> qpoints{q0, q1, q2};
    const std::array<double, 3> qweights{1.0, std::sqrt(0.5), 1.0};
    const auto rational_curve = RationalQuadraticBezier3::make(
        q0, q1, q2, qweights[0], qweights[1], qweights[2]);
    if (!rational_curve) {
        return 1;
    }
    const ElevatedCubic elevated = elevate_quadratic(qpoints, qweights);

    RationalBicubicBezierPatch3::ControlNet extrusion_controls{{
        elevated.points,
        elevated.points,
        elevated.points,
        elevated.points,
    }};
    RationalBicubicBezierPatch3::WeightNet extrusion_weights{{
        elevated.weights,
        elevated.weights,
        elevated.weights,
        elevated.weights,
    }};
    const auto extrusion_patch =
        RationalBicubicBezierPatch3::make(
            extrusion_controls, extrusion_weights);
    if (!extrusion_patch) {
        return 1;
    }

    for (const double v : std::array<double, 4>{0.0, 0.25, 0.6, 1.0}) {
        const auto curve_value = rational_curve->evaluate(v);
        const auto curve_d1 = rational_curve->first_derivative(v);
        const auto curve_d2 = rational_curve->second_derivative(v);
        const auto surface_value = extrusion_patch->evaluate(0.37, v);
        const auto surface_first =
            extrusion_patch->first_derivatives(0.37, v);
        const auto surface_second =
            extrusion_patch->second_derivatives(0.37, v);
        passed = require(
                     curve_value && surface_value &&
                         close_point(*curve_value, *surface_value, 128.0) &&
                         curve_d1 && surface_first &&
                         close_vector(*curve_d1, surface_first->v, 512.0) &&
                         surface_first->u.x() == 0.0 &&
                         surface_first->u.y() == 0.0 &&
                         surface_first->u.z() == 0.0 &&
                         curve_d2 && surface_second &&
                         close_vector(*curve_d2, surface_second->vv, 2048.0) &&
                         surface_second->uu.x() == 0.0 &&
                         surface_second->uu.y() == 0.0 &&
                         surface_second->uu.z() == 0.0 &&
                         surface_second->uv.x() == 0.0 &&
                         surface_second->uv.y() == 0.0 &&
                         surface_second->uv.z() == 0.0,
                     "rational quadratic degree-elevation surface relation differs") &&
                 passed;
    }

    const double maximum = std::numeric_limits<double>::max();
    const double minimum = std::numeric_limits<double>::denorm_min();
    auto extreme_weights = weights;
    extreme_weights[0][0] = minimum;
    extreme_weights[0][1] = maximum;
    extreme_weights[1][2] = maximum / 2.0;
    extreme_weights[2][3] = minimum;
    const auto extreme_patch =
        RationalBicubicBezierPatch3::make(controls, extreme_weights);
    if (!extreme_patch) {
        return 1;
    }
    const auto extreme_value = extreme_patch->evaluate(0.4, 0.6);
    const auto extreme_first =
        extreme_patch->first_derivatives(0.4, 0.6);
    const auto extreme_second =
        extreme_patch->second_derivatives(0.4, 0.6);
    passed = require(
                 extreme_value && extreme_first && extreme_second,
                 "scale-aware rational surface failed representable extreme weights") &&
             passed;

    auto huge_controls = controls;
    const auto huge_negative = *Point3::make(-maximum, 0.0, 0.0);
    const auto huge_positive = *Point3::make(maximum, 0.0, 0.0);
    for (std::size_t v = 0; v < 4U; ++v) {
        huge_controls[0][v] = huge_negative;
        huge_controls[1][v] = huge_positive;
        huge_controls[2][v] = huge_positive;
        huge_controls[3][v] = huge_positive;
    }
    RationalBicubicBezierPatch3::WeightNet one_weights{};
    for (auto& row : one_weights) {
        row.fill(1.0);
    }
    const auto huge_patch =
        RationalBicubicBezierPatch3::make(huge_controls, one_weights);
    if (!huge_patch) {
        return 1;
    }
    const auto unrepresentable =
        huge_patch->first_derivatives(0.0, 0.5);
    passed = require(
                 !unrepresentable &&
                     unrepresentable.error() ==
                         SurfaceError::non_finite_result,
                 "unrepresentable rational surface derivative did not fail explicitly") &&
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
                 "rational surface query failure semantics differ") &&
             passed;

    const auto repeat_value_a = patch->evaluate(0.35, 0.65);
    const auto repeat_value_b = patch->evaluate(0.35, 0.65);
    const auto repeat_first_a = patch->first_derivatives(0.35, 0.65);
    const auto repeat_first_b = patch->first_derivatives(0.35, 0.65);
    const auto repeat_second_a = patch->second_derivatives(0.35, 0.65);
    const auto repeat_second_b = patch->second_derivatives(0.35, 0.65);
    const auto repeat_failure_a = patch->evaluate(1.2, 0.5);
    const auto repeat_failure_b = patch->evaluate(1.2, 0.5);
    passed = require(
                 repeat_value_a && repeat_value_b &&
                     *repeat_value_a == *repeat_value_b &&
                     repeat_first_a && repeat_first_b &&
                     *repeat_first_a == *repeat_first_b &&
                     repeat_second_a && repeat_second_b &&
                     *repeat_second_a == *repeat_second_b &&
                     !repeat_failure_a && !repeat_failure_b &&
                     repeat_failure_a.error() == repeat_failure_b.error(),
                 "rational surface repeated evidence is not deterministic") &&
             passed;

    return passed ? 0 : 1;
}
