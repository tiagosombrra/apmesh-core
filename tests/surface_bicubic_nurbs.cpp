#include "apmesh/geometry/nurbs_surface.hpp"

#include "apmesh/geometry/nurbs.hpp"
#include "apmesh/geometry/surface.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdio>
#include <expected>
#include <limits>
#include <span>
#include <string_view>
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
    const double absolute = 2.0e-10,
    const double relative = 2.0e-10) {
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

std::vector<double> flat_knots(
    const std::span<const double> interior,
    const double lower,
    const double upper) {
    std::vector<double> result;
    result.reserve(interior.size() + 8U);
    for (int index = 0; index < 4; ++index) {
        result.push_back(lower);
    }
    result.insert(result.end(), interior.begin(), interior.end());
    for (int index = 0; index < 4; ++index) {
        result.push_back(upper);
    }
    return result;
}

long double basis(
    const std::span<const double> knots,
    const int index,
    const int degree,
    const long double parameter) {
    if (degree == 0) {
        const long double lower =
            static_cast<long double>(knots[static_cast<std::size_t>(index)]);
        const long double upper = static_cast<long double>(
            knots[static_cast<std::size_t>(index + 1)]);
        return lower <= parameter && parameter < upper ? 1.0L : 0.0L;
    }

    long double value = 0.0L;
    const long double left_denominator =
        static_cast<long double>(
            knots[static_cast<std::size_t>(index + degree)]) -
        static_cast<long double>(
            knots[static_cast<std::size_t>(index)]);
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
        static_cast<long double>(
            knots[static_cast<std::size_t>(index)]);
    if (left_denominator != 0.0L) {
        value += static_cast<long double>(degree) /
                 left_denominator *
                 basis(knots, index, degree - 1, parameter);
    }

    const long double right_denominator =
        static_cast<long double>(
            knots[static_cast<std::size_t>(index + degree + 1)]) -
        static_cast<long double>(
            knots[static_cast<std::size_t>(index + 1)]);
    if (right_denominator != 0.0L) {
        value -= static_cast<long double>(degree) /
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
        static_cast<long double>(
            knots[static_cast<std::size_t>(index)]);
    if (left_denominator != 0.0L) {
        value += static_cast<long double>(degree) /
                 left_denominator *
                 basis_first(knots, index, degree - 1, parameter);
    }

    const long double right_denominator =
        static_cast<long double>(
            knots[static_cast<std::size_t>(index + degree + 1)]) -
        static_cast<long double>(
            knots[static_cast<std::size_t>(index + 1)]);
    if (right_denominator != 0.0L) {
        value -= static_cast<long double>(degree) /
                 right_denominator *
                 basis_first(knots, index + 1, degree - 1, parameter);
    }
    return value;
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
    const std::span<const apmesh::core::Point3> points,
    const std::span<const double> weights,
    const std::size_t u_count,
    const std::size_t v_count,
    const std::span<const double> u_flat,
    const std::span<const double> v_flat,
    const double u,
    const double v) {
    const long double lu = static_cast<long double>(u);
    const long double lv = static_cast<long double>(v);
    const double maximum_weight =
        *std::max_element(weights.begin(), weights.end());

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

    for (std::size_t i = 0; i < u_count; ++i) {
        const long double bu =
            basis(u_flat, static_cast<int>(i), 3, lu);
        const long double du =
            basis_first(u_flat, static_cast<int>(i), 3, lu);
        const long double ddu =
            basis_second(u_flat, static_cast<int>(i), 3, lu);

        for (std::size_t j = 0; j < v_count; ++j) {
            const long double bv =
                basis(v_flat, static_cast<int>(j), 3, lv);
            const long double dv =
                basis_first(v_flat, static_cast<int>(j), 3, lv);
            const long double ddv =
                basis_second(v_flat, static_cast<int>(j), 3, lv);
            const std::size_t index = i * v_count + j;
            const long double weight =
                static_cast<long double>(weights[index]) /
                static_cast<long double>(maximum_weight);
            const auto& point = points[index];

            const long double b = bu * bv * weight;
            const long double bu1 = du * bv * weight;
            const long double bv1 = bu * dv * weight;
            const long double bu2 = ddu * bv * weight;
            const long double buv = du * dv * weight;
            const long double bv2 = bu * ddv * weight;

            a = add_scaled(a, point, b);
            au = add_scaled(au, point, bu1);
            av = add_scaled(av, point, bv1);
            auu = add_scaled(auu, point, bu2);
            auv = add_scaled(auv, point, buv);
            avv = add_scaled(avv, point, bv2);

            w += b;
            wu += bu1;
            wv += bv1;
            wuu += bu2;
            wuv += buv;
            wvv += bv2;
        }
    }

    ReferenceJet result{};
    result.value = divide(a, w);
    result.u = divide(subtract_scaled(au, result.value, wu), w);
    result.v = divide(subtract_scaled(av, result.value, wv), w);

    auto uu_numerator =
        subtract_scaled(auu, result.u, 2.0L * wu);
    uu_numerator =
        subtract_scaled(uu_numerator, result.value, wuu);
    result.uu = divide(uu_numerator, w);

    auto vv_numerator =
        subtract_scaled(avv, result.v, 2.0L * wv);
    vv_numerator =
        subtract_scaled(vv_numerator, result.value, wvv);
    result.vv = divide(vv_numerator, w);

    auto uv_numerator =
        subtract_scaled(auv, result.u, wv);
    uv_numerator =
        subtract_scaled(uv_numerator, result.v, wu);
    uv_numerator =
        subtract_scaled(uv_numerator, result.value, wuv);
    result.uv = divide(uv_numerator, w);

    return result;
}

bool close_reference(
    const apmesh::core::Point3& value,
    const apmesh::core::SurfaceFirstDerivatives3& first,
    const apmesh::core::SurfaceSecondDerivatives3& second,
    const ReferenceJet& reference) {
    return close_scalar(value.x(), static_cast<double>(reference.value.x), 256.0) &&
           close_scalar(value.y(), static_cast<double>(reference.value.y), 256.0) &&
           close_scalar(value.z(), static_cast<double>(reference.value.z), 256.0) &&
           close_scalar(first.u.x(), static_cast<double>(reference.u.x), 1024.0) &&
           close_scalar(first.u.y(), static_cast<double>(reference.u.y), 1024.0) &&
           close_scalar(first.u.z(), static_cast<double>(reference.u.z), 1024.0) &&
           close_scalar(first.v.x(), static_cast<double>(reference.v.x), 1024.0) &&
           close_scalar(first.v.y(), static_cast<double>(reference.v.y), 1024.0) &&
           close_scalar(first.v.z(), static_cast<double>(reference.v.z), 1024.0) &&
           close_scalar(second.uu.x(), static_cast<double>(reference.uu.x), 4096.0) &&
           close_scalar(second.uu.y(), static_cast<double>(reference.uu.y), 4096.0) &&
           close_scalar(second.uu.z(), static_cast<double>(reference.uu.z), 4096.0) &&
           close_scalar(second.uv.x(), static_cast<double>(reference.uv.x), 4096.0) &&
           close_scalar(second.uv.y(), static_cast<double>(reference.uv.y), 4096.0) &&
           close_scalar(second.uv.z(), static_cast<double>(reference.uv.z), 4096.0) &&
           close_scalar(second.vv.x(), static_cast<double>(reference.vv.x), 4096.0) &&
           close_scalar(second.vv.y(), static_cast<double>(reference.vv.y), 4096.0) &&
           close_scalar(second.vv.z(), static_cast<double>(reference.vv.z), 4096.0);
}

std::vector<apmesh::core::Point3> make_points(
    const std::size_t u_count,
    const std::size_t v_count) {
    std::vector<apmesh::core::Point3> points;
    points.reserve(u_count * v_count);
    for (std::size_t i = 0; i < u_count; ++i) {
        for (std::size_t j = 0; j < v_count; ++j) {
            const double x =
                -3.0 + 1.25 * static_cast<double>(i) +
                0.2 * static_cast<double>(j);
            const double y =
                -2.0 + 0.8 * static_cast<double>(j) -
                0.15 * static_cast<double>(i);
            const double z =
                0.35 * static_cast<double>(i * i) -
                0.25 * static_cast<double>(j) +
                0.12 * static_cast<double>(i * j);
            points.push_back(*apmesh::core::Point3::make(x, y, z));
        }
    }
    return points;
}

std::vector<double> make_weights(
    const std::size_t u_count,
    const std::size_t v_count) {
    std::vector<double> weights;
    weights.reserve(u_count * v_count);
    for (std::size_t i = 0; i < u_count; ++i) {
        for (std::size_t j = 0; j < v_count; ++j) {
            weights.push_back(
                0.75 + 0.35 * static_cast<double>((3U * i + 2U * j) % 7U));
        }
    }
    return weights;
}

apmesh::core::RationalBicubicBezierPatch3::ControlNet make_bicubic_controls() {
    using apmesh::core::Point3;
    const auto seed = *Point3::make(0.0, 0.0, 0.0);
    apmesh::core::RationalBicubicBezierPatch3::ControlNet controls{{
        {{seed, seed, seed, seed}},
        {{seed, seed, seed, seed}},
        {{seed, seed, seed, seed}},
        {{seed, seed, seed, seed}},
    }};
    for (std::size_t i = 0; i < 4U; ++i) {
        for (std::size_t j = 0; j < 4U; ++j) {
            controls[i][j] = *Point3::make(
                static_cast<double>(i) + 0.25 * static_cast<double>(j),
                0.5 * static_cast<double>(j) - 0.1 * static_cast<double>(i),
                0.3 * static_cast<double>(i * j) -
                    0.2 * static_cast<double>(j));
        }
    }
    return controls;
}

apmesh::core::RationalBicubicBezierPatch3::WeightNet make_bicubic_weights() {
    return {{
        {{1.0, 2.0, 0.75, 1.5}},
        {{1.25, 0.5, 2.5, 1.0}},
        {{0.8, 1.8, 0.65, 2.2}},
        {{1.4, 0.9, 1.6, 0.7}},
    }};
}

std::vector<apmesh::core::Point3> flatten(
    const apmesh::core::RationalBicubicBezierPatch3::ControlNet& controls) {
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

struct H4 {
    long double x{};
    long double y{};
    long double z{};
    long double w{};
};

H4 homogeneous(
    const apmesh::core::Point3& point,
    const double weight) {
    const long double w = static_cast<long double>(weight);
    return {
        static_cast<long double>(point.x()) * w,
        static_cast<long double>(point.y()) * w,
        static_cast<long double>(point.z()) * w,
        w,
    };
}

H4 blend(const H4& lhs, const H4& rhs, const long double alpha) {
    return {
        (1.0L - alpha) * lhs.x + alpha * rhs.x,
        (1.0L - alpha) * lhs.y + alpha * rhs.y,
        (1.0L - alpha) * lhs.z + alpha * rhs.z,
        (1.0L - alpha) * lhs.w + alpha * rhs.w,
    };
}

std::vector<H4> insert_simple_knot(
    const std::span<const H4> controls,
    const std::span<const double> knots,
    const double knot) {
    constexpr int degree = 3;
    const int n = static_cast<int>(controls.size()) - 1;
    const auto upper = std::upper_bound(knots.begin(), knots.end(), knot);
    const int span = static_cast<int>(upper - knots.begin()) - 1;
    int multiplicity = 0;
    for (const double value : knots) {
        if (value == knot) {
            ++multiplicity;
        }
    }

    std::vector<H4> result(controls.size() + 1U);
    for (int index = 0; index <= span - degree; ++index) {
        result[static_cast<std::size_t>(index)] =
            controls[static_cast<std::size_t>(index)];
    }
    for (int index = span - multiplicity; index <= n; ++index) {
        result[static_cast<std::size_t>(index + 1)] =
            controls[static_cast<std::size_t>(index)];
    }
    for (int index = span - degree + 1;
         index <= span - multiplicity;
         ++index) {
        const long double numerator =
            static_cast<long double>(knot) -
            static_cast<long double>(knots[static_cast<std::size_t>(index)]);
        const long double denominator =
            static_cast<long double>(
                knots[static_cast<std::size_t>(index + degree)]) -
            static_cast<long double>(knots[static_cast<std::size_t>(index)]);
        const long double alpha = numerator / denominator;
        result[static_cast<std::size_t>(index)] = blend(
            controls[static_cast<std::size_t>(index - 1)],
            controls[static_cast<std::size_t>(index)],
            alpha);
    }
    return result;
}

struct InsertedSurface {
    std::vector<apmesh::core::Point3> points;
    std::vector<double> weights;
};

InsertedSurface insert_u_then_v(
    const apmesh::core::RationalBicubicBezierPatch3::ControlNet& controls,
    const apmesh::core::RationalBicubicBezierPatch3::WeightNet& weights,
    const double u_knot,
    const double v_knot) {
    const std::array<double, 8> bezier_knots{
        0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0};

    std::array<std::array<H4, 4>, 4> source{};
    for (std::size_t i = 0; i < 4U; ++i) {
        for (std::size_t j = 0; j < 4U; ++j) {
            source[i][j] = homogeneous(controls[i][j], weights[i][j]);
        }
    }

    std::array<std::array<H4, 4>, 5> u_inserted{};
    for (std::size_t j = 0; j < 4U; ++j) {
        std::array<H4, 4> column{};
        for (std::size_t i = 0; i < 4U; ++i) {
            column[i] = source[i][j];
        }
        const auto inserted = insert_simple_knot(
            column, bezier_knots, u_knot);
        for (std::size_t i = 0; i < 5U; ++i) {
            u_inserted[i][j] = inserted[i];
        }
    }

    std::array<std::array<H4, 5>, 5> both_inserted{};
    for (std::size_t i = 0; i < 5U; ++i) {
        const auto inserted = insert_simple_knot(
            u_inserted[i], bezier_knots, v_knot);
        for (std::size_t j = 0; j < 5U; ++j) {
            both_inserted[i][j] = inserted[j];
        }
    }

    InsertedSurface result;
    result.points.reserve(25U);
    result.weights.reserve(25U);
    for (const auto& row : both_inserted) {
        for (const H4& value : row) {
            result.points.push_back(*apmesh::core::Point3::make(
                static_cast<double>(value.x / value.w),
                static_cast<double>(value.y / value.w),
                static_cast<double>(value.z / value.w)));
            result.weights.push_back(static_cast<double>(value.w));
        }
    }
    return result;
}

std::vector<apmesh::core::Point3> translated(
    const std::span<const apmesh::core::Point3> points,
    const double dx,
    const double dy,
    const double dz) {
    std::vector<apmesh::core::Point3> result;
    result.reserve(points.size());
    for (const auto& point : points) {
        result.push_back(*apmesh::core::Point3::make(
            point.x() + dx, point.y() + dy, point.z() + dz));
    }
    return result;
}

std::vector<apmesh::core::Point3> scaled(
    const std::span<const apmesh::core::Point3> points,
    const double factor) {
    std::vector<apmesh::core::Point3> result;
    result.reserve(points.size());
    for (const auto& point : points) {
        result.push_back(*apmesh::core::Point3::make(
            factor * point.x(), factor * point.y(), factor * point.z()));
    }
    return result;
}

} // namespace

int main() {
    using apmesh::core::BicubicBezierPatch3;
    using apmesh::core::BicubicNURBSSurface3;
    using apmesh::core::BicubicNURBSSurfaceConstructionError;
    using apmesh::core::BoundedParametricSurface3;
    using apmesh::core::MultiSpanCubicNURBS3;
    using apmesh::core::Point3;
    using apmesh::core::RationalBicubicBezierPatch3;
    using apmesh::core::SurfaceError;
    using apmesh::core::reversed_parameter;

    static_assert(BoundedParametricSurface3<BicubicNURBSSurface3>);
    static_assert(std::same_as<
        decltype(std::declval<const BicubicNURBSSurface3&>().control_points()),
        std::span<const Point3>>);
    static_assert(std::same_as<
        decltype(std::declval<const BicubicNURBSSurface3&>().weights()),
        std::span<const double>>);

    bool passed = true;

    constexpr std::size_t u_count = 6U;
    constexpr std::size_t v_count = 7U;
    const std::vector<double> u_knots{-0.5, 1.25};
    const std::vector<double> v_knots{-1.0, 0.5, 2.25};
    const auto points = make_points(u_count, v_count);
    const auto weights = make_weights(u_count, v_count);

    const auto surface = BicubicNURBSSurface3::make(
        points,
        weights,
        u_count,
        v_count,
        u_knots,
        v_knots,
        -2.0,
        3.0,
        -3.0,
        4.0);
    if (!surface) {
        return 1;
    }

    passed = require(
                 surface->u_control_count() == u_count &&
                     surface->v_control_count() == v_count &&
                     surface->u_span_count() == 3U &&
                     surface->v_span_count() == 4U &&
                     surface->control_points().size() == u_count * v_count &&
                     surface->weights().size() == u_count * v_count &&
                     surface->parameter_domain().u.lower() == -2.0 &&
                     surface->parameter_domain().u.upper() == 3.0 &&
                     surface->parameter_domain().v.lower() == -3.0 &&
                     surface->parameter_domain().v.upper() == 4.0,
                 "bicubic NURBS dynamic layout/domain differs") &&
             passed;

    const auto u_flat = flat_knots(u_knots, -2.0, 3.0);
    const auto v_flat = flat_knots(v_knots, -3.0, 4.0);
    const std::array<std::pair<double, double>, 10> oracle_parameters{{
        {-1.5, -2.0},
        {-0.5, -0.5},
        {0.0, 0.5},
        {0.75, 1.0},
        {1.25, 2.25},
        {2.0, 3.0},
        {-0.5, 1.75},
        {0.25, -1.0},
        {1.75, 0.5},
        {2.5, 2.25},
    }};
    for (const auto& [u, v] : oracle_parameters) {
        const auto reference = reference_jet(
            points, weights, u_count, v_count, u_flat, v_flat, u, v);
        const auto value = surface->evaluate(u, v);
        const auto first = surface->first_derivatives(u, v);
        const auto second = surface->second_derivatives(u, v);
        passed = require(
                     value && first && second &&
                         close_reference(*value, *first, *second, reference),
                     "bicubic NURBS independent tensor oracle differs") &&
                 passed;
    }

    const auto controls = make_bicubic_controls();
    const auto rational_weights = make_bicubic_weights();
    const auto rational =
        RationalBicubicBezierPatch3::make(controls, rational_weights);
    const auto rational_nurbs = BicubicNURBSSurface3::make(
        flatten(controls),
        flatten(rational_weights),
        4U,
        4U,
        {},
        {},
        0.0,
        1.0,
        0.0,
        1.0);
    if (!rational || !rational_nurbs) {
        return 1;
    }

    const std::array<std::pair<double, double>, 4> subset_parameters{{
        {0.0, 0.0},
        {0.2, 0.7},
        {0.65, 0.35},
        {1.0, 1.0},
    }};
    for (const auto& [u, v] : subset_parameters) {
        const auto rv = rational->evaluate(u, v);
        const auto nv = rational_nurbs->evaluate(u, v);
        const auto rf = rational->first_derivatives(u, v);
        const auto nf = rational_nurbs->first_derivatives(u, v);
        const auto rs = rational->second_derivatives(u, v);
        const auto ns = rational_nurbs->second_derivatives(u, v);
        passed = require(
                     rv && nv && close_point(*rv, *nv, 256.0) &&
                         rf && nf &&
                         close_vector(rf->u, nf->u, 1024.0) &&
                         close_vector(rf->v, nf->v, 1024.0) &&
                         rs && ns &&
                         close_vector(rs->uu, ns->uu, 4096.0) &&
                         close_vector(rs->uv, ns->uv, 4096.0) &&
                         close_vector(rs->vv, ns->vv, 4096.0),
                     "bicubic NURBS rational-Bezier subset differs") &&
                 passed;
    }

    RationalBicubicBezierPatch3::WeightNet ones{{
        {{1.0, 1.0, 1.0, 1.0}},
        {{1.0, 1.0, 1.0, 1.0}},
        {{1.0, 1.0, 1.0, 1.0}},
        {{1.0, 1.0, 1.0, 1.0}},
    }};
    const BicubicBezierPatch3 polynomial{controls};
    const auto polynomial_nurbs = BicubicNURBSSurface3::make(
        flatten(controls),
        flatten(ones),
        4U,
        4U,
        {},
        {},
        0.0,
        1.0,
        0.0,
        1.0);
    if (!polynomial_nurbs) {
        return 1;
    }
    for (const auto& [u, v] : subset_parameters) {
        const auto pv = polynomial.evaluate(u, v);
        const auto nv = polynomial_nurbs->evaluate(u, v);
        const auto pf = polynomial.first_derivatives(u, v);
        const auto nf = polynomial_nurbs->first_derivatives(u, v);
        const auto ps = polynomial.second_derivatives(u, v);
        const auto ns = polynomial_nurbs->second_derivatives(u, v);
        passed = require(
                     pv && nv && close_point(*pv, *nv, 256.0) &&
                         pf && nf &&
                         close_vector(pf->u, nf->u, 1024.0) &&
                         close_vector(pf->v, nf->v, 1024.0) &&
                         ps && ns &&
                         close_vector(ps->uu, ns->uu, 4096.0) &&
                         close_vector(ps->uv, ns->uv, 4096.0) &&
                         close_vector(ps->vv, ns->vv, 4096.0),
                     "bicubic NURBS polynomial subset differs") &&
                 passed;
    }

    auto boundary_curve = [&](const bool u_boundary, const bool upper) {
        std::vector<Point3> boundary_points;
        std::vector<double> boundary_weights;
        if (u_boundary) {
            const std::size_t u_index = upper ? u_count - 1U : 0U;
            for (std::size_t j = 0; j < v_count; ++j) {
                const std::size_t index = u_index * v_count + j;
                boundary_points.push_back(points[index]);
                boundary_weights.push_back(weights[index]);
            }
            return MultiSpanCubicNURBS3::make(
                boundary_points,
                boundary_weights,
                v_knots,
                -3.0,
                4.0);
        }

        const std::size_t v_index = upper ? v_count - 1U : 0U;
        for (std::size_t i = 0; i < u_count; ++i) {
            const std::size_t index = i * v_count + v_index;
            boundary_points.push_back(points[index]);
            boundary_weights.push_back(weights[index]);
        }
        return MultiSpanCubicNURBS3::make(
            boundary_points,
            boundary_weights,
            u_knots,
            -2.0,
            3.0);
    };

    const auto u_lower_curve = boundary_curve(true, false);
    const auto u_upper_curve = boundary_curve(true, true);
    const auto v_lower_curve = boundary_curve(false, false);
    const auto v_upper_curve = boundary_curve(false, true);
    if (!u_lower_curve || !u_upper_curve || !v_lower_curve || !v_upper_curve) {
        return 1;
    }

    constexpr double edge_v = 1.0;
    for (const auto& pair :
         std::array<std::pair<double, const MultiSpanCubicNURBS3*>, 2>{
             {{-2.0, &*u_lower_curve}, {3.0, &*u_upper_curve}}}) {
        const auto sv = surface->evaluate(pair.first, edge_v);
        const auto sf = surface->first_derivatives(pair.first, edge_v);
        const auto ss = surface->second_derivatives(pair.first, edge_v);
        const auto cv = pair.second->evaluate(edge_v);
        const auto cd1 = pair.second->first_derivative(edge_v);
        const auto cd2 = pair.second->second_derivative(edge_v);
        passed = require(
                     sv && sf && ss && cv && cd1 && cd2 &&
                         close_point(*sv, *cv, 256.0) &&
                         close_vector(sf->v, *cd1, 1024.0) &&
                         close_vector(ss->vv, *cd2, 4096.0),
                     "U-boundary NURBS curve parity differs") &&
                 passed;
    }

    constexpr double edge_u = 0.25;
    for (const auto& pair :
         std::array<std::pair<double, const MultiSpanCubicNURBS3*>, 2>{
             {{-3.0, &*v_lower_curve}, {4.0, &*v_upper_curve}}}) {
        const auto sv = surface->evaluate(edge_u, pair.first);
        const auto sf = surface->first_derivatives(edge_u, pair.first);
        const auto ss = surface->second_derivatives(edge_u, pair.first);
        const auto cv = pair.second->evaluate(edge_u);
        const auto cd1 = pair.second->first_derivative(edge_u);
        const auto cd2 = pair.second->second_derivative(edge_u);
        passed = require(
                     sv && sf && ss && cv && cd1 && cd2 &&
                         close_point(*sv, *cv, 256.0) &&
                         close_vector(sf->u, *cd1, 1024.0) &&
                         close_vector(ss->uu, *cd2, 4096.0),
                     "V-boundary NURBS curve parity differs") &&
                 passed;
    }

    const auto inserted = insert_u_then_v(
        controls, rational_weights, 0.35, 0.65);
    const auto inserted_surface = BicubicNURBSSurface3::make(
        inserted.points,
        inserted.weights,
        5U,
        5U,
        std::vector<double>{0.35},
        std::vector<double>{0.65},
        0.0,
        1.0,
        0.0,
        1.0);
    if (!inserted_surface) {
        return 1;
    }
    const std::array<std::pair<double, double>, 6> insertion_parameters{{
        {0.1, 0.2},
        {0.35, 0.2},
        {0.5, 0.65},
        {0.35, 0.65},
        {0.8, 0.9},
        {0.9, 0.4},
    }};
    for (const auto& [u, v] : insertion_parameters) {
        const auto bv = rational->evaluate(u, v);
        const auto iv = inserted_surface->evaluate(u, v);
        const auto bf = rational->first_derivatives(u, v);
        const auto inf = inserted_surface->first_derivatives(u, v);
        const auto bs = rational->second_derivatives(u, v);
        const auto ins = inserted_surface->second_derivatives(u, v);
        passed = require(
                     bv && iv && close_point(*bv, *iv, 512.0) &&
                         bf && inf &&
                         close_vector(bf->u, inf->u, 2048.0) &&
                         close_vector(bf->v, inf->v, 2048.0) &&
                         bs && ins &&
                         close_vector(bs->uu, ins->uu, 8192.0) &&
                         close_vector(bs->uv, ins->uv, 8192.0) &&
                         close_vector(bs->vv, ins->vv, 8192.0),
                     "test-only U/V knot insertion changed surface") &&
                 passed;
    }

    auto local_points = points;
    auto local_weights = weights;
    const auto extreme_point =
        *Point3::make(1.0e200, -2.0e200, 3.0e200);
    for (std::size_t i = 0; i < u_count; ++i) {
        for (std::size_t j = 0; j < v_count; ++j) {
            if (i == 0U || i == u_count - 1U ||
                j < 2U || j == v_count - 1U) {
                const std::size_t index = i * v_count + j;
                local_points[index] = extreme_point;
                local_weights[index] =
                    (index % 2U == 0U)
                        ? std::numeric_limits<double>::max()
                        : std::numeric_limits<double>::denorm_min();
            }
        }
    }
    const auto local_surface = BicubicNURBSSurface3::make(
        local_points,
        local_weights,
        u_count,
        v_count,
        u_knots,
        v_knots,
        -2.0,
        3.0,
        -3.0,
        4.0);
    if (!local_surface) {
        return 1;
    }
    constexpr double local_u = 0.0;
    constexpr double local_v = 1.0;
    const auto local_base_value = surface->evaluate(local_u, local_v);
    const auto local_new_value = local_surface->evaluate(local_u, local_v);
    const auto local_base_first =
        surface->first_derivatives(local_u, local_v);
    const auto local_new_first =
        local_surface->first_derivatives(local_u, local_v);
    const auto local_base_second =
        surface->second_derivatives(local_u, local_v);
    const auto local_new_second =
        local_surface->second_derivatives(local_u, local_v);
    passed = require(
                 local_base_value && local_new_value &&
                     *local_base_value == *local_new_value &&
                     local_base_first && local_new_first &&
                     *local_base_first == *local_new_first &&
                     local_base_second && local_new_second &&
                     *local_base_second == *local_new_second,
                 "bicubic NURBS local support leaked distant data") &&
             passed;

    auto scaled_weights = weights;
    for (double& weight : scaled_weights) {
        weight *= 8.0;
    }
    const auto weight_scaled = BicubicNURBSSurface3::make(
        points,
        scaled_weights,
        u_count,
        v_count,
        u_knots,
        v_knots,
        -2.0,
        3.0,
        -3.0,
        4.0);
    if (!weight_scaled) {
        return 1;
    }
    const auto base_value = surface->evaluate(0.75, 1.0);
    const auto scaled_value = weight_scaled->evaluate(0.75, 1.0);
    const auto base_first = surface->first_derivatives(0.75, 1.0);
    const auto scaled_first = weight_scaled->first_derivatives(0.75, 1.0);
    const auto base_second = surface->second_derivatives(0.75, 1.0);
    const auto scaled_second = weight_scaled->second_derivatives(0.75, 1.0);
    passed = require(
                 base_value && scaled_value &&
                     close_point(*base_value, *scaled_value, 256.0) &&
                     base_first && scaled_first &&
                     close_vector(base_first->u, scaled_first->u, 1024.0) &&
                     close_vector(base_first->v, scaled_first->v, 1024.0) &&
                     base_second && scaled_second &&
                     close_vector(base_second->uu, scaled_second->uu, 4096.0) &&
                     close_vector(base_second->uv, scaled_second->uv, 4096.0) &&
                     close_vector(base_second->vv, scaled_second->vv, 4096.0),
                 "bicubic NURBS common-weight scale invariance differs") &&
             passed;

    const auto u_reversed = surface->u_reversed();
    const auto v_reversed = surface->v_reversed();
    passed = require(
                 u_reversed.u_reversed() == *surface &&
                     v_reversed.v_reversed() == *surface,
                 "bicubic NURBS reversal is not an involution") &&
             passed;

    constexpr double reversal_u = 0.2;
    constexpr double reversal_v = 1.1;
    const auto mapped_u =
        reversed_parameter(surface->parameter_domain().u, reversal_u);
    const auto mapped_v =
        reversed_parameter(surface->parameter_domain().v, reversal_v);
    if (!mapped_u || !mapped_v) {
        return 1;
    }

    const auto ov = surface->evaluate(reversal_u, reversal_v);
    const auto of = surface->first_derivatives(reversal_u, reversal_v);
    const auto os = surface->second_derivatives(reversal_u, reversal_v);
    const auto urv = u_reversed.evaluate(*mapped_u, reversal_v);
    const auto urf = u_reversed.first_derivatives(*mapped_u, reversal_v);
    const auto urs = u_reversed.second_derivatives(*mapped_u, reversal_v);
    const auto vrv = v_reversed.evaluate(reversal_u, *mapped_v);
    const auto vrf = v_reversed.first_derivatives(reversal_u, *mapped_v);
    const auto vrs = v_reversed.second_derivatives(reversal_u, *mapped_v);
    passed = require(
                 ov && of && os && urv && urf && urs && vrv && vrf && vrs &&
                     close_point(*ov, *urv, 512.0) &&
                     close_scalar(urf->u.x(), -of->u.x(), 2048.0) &&
                     close_scalar(urf->u.y(), -of->u.y(), 2048.0) &&
                     close_scalar(urf->u.z(), -of->u.z(), 2048.0) &&
                     close_vector(urf->v, of->v, 2048.0) &&
                     close_vector(urs->uu, os->uu, 8192.0) &&
                     close_scalar(urs->uv.x(), -os->uv.x(), 8192.0) &&
                     close_scalar(urs->uv.y(), -os->uv.y(), 8192.0) &&
                     close_scalar(urs->uv.z(), -os->uv.z(), 8192.0) &&
                     close_vector(urs->vv, os->vv, 8192.0) &&
                     close_point(*ov, *vrv, 512.0) &&
                     close_vector(vrf->u, of->u, 2048.0) &&
                     close_scalar(vrf->v.x(), -of->v.x(), 2048.0) &&
                     close_scalar(vrf->v.y(), -of->v.y(), 2048.0) &&
                     close_scalar(vrf->v.z(), -of->v.z(), 2048.0) &&
                     close_vector(vrs->uu, os->uu, 8192.0) &&
                     close_scalar(vrs->uv.x(), -os->uv.x(), 8192.0) &&
                     close_scalar(vrs->uv.y(), -os->uv.y(), 8192.0) &&
                     close_scalar(vrs->uv.z(), -os->uv.z(), 8192.0) &&
                     close_vector(vrs->vv, os->vv, 8192.0),
                 "bicubic NURBS U/V reversal covariance differs") &&
             passed;

    const auto constant_point = *Point3::make(3.0, -4.0, 2.0);
    const std::vector<Point3> constant_points(
        u_count * v_count, constant_point);
    const auto constant_surface = BicubicNURBSSurface3::make(
        constant_points,
        weights,
        u_count,
        v_count,
        u_knots,
        v_knots,
        -2.0,
        3.0,
        -3.0,
        4.0);
    if (!constant_surface) {
        return 1;
    }
    const auto constant_value = constant_surface->evaluate(0.5, 1.0);
    const auto constant_first =
        constant_surface->first_derivatives(0.5, 1.0);
    const auto constant_second =
        constant_surface->second_derivatives(0.5, 1.0);
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
                 "bicubic NURBS constant-patch semantics differ") &&
             passed;

    const auto translated_points = translated(points, 8.0, -6.0, 3.0);
    const auto scaled_points = scaled(points, 2.0);
    const auto translated_surface = BicubicNURBSSurface3::make(
        translated_points, weights, u_count, v_count, u_knots, v_knots,
        -2.0, 3.0, -3.0, 4.0);
    const auto coordinate_scaled_surface = BicubicNURBSSurface3::make(
        scaled_points, weights, u_count, v_count, u_knots, v_knots,
        -2.0, 3.0, -3.0, 4.0);
    if (!translated_surface || !coordinate_scaled_surface) {
        return 1;
    }

    const auto tv = translated_surface->evaluate(0.75, 1.0);
    const auto tf = translated_surface->first_derivatives(0.75, 1.0);
    const auto ts = translated_surface->second_derivatives(0.75, 1.0);
    const auto csv = coordinate_scaled_surface->evaluate(0.75, 1.0);
    const auto csf =
        coordinate_scaled_surface->first_derivatives(0.75, 1.0);
    const auto css =
        coordinate_scaled_surface->second_derivatives(0.75, 1.0);
    passed = require(
                 base_value && base_first && base_second &&
                     tv && tf && ts && csv && csf && css &&
                     close_scalar(tv->x() - base_value->x(), 8.0, 128.0) &&
                     close_scalar(tv->y() - base_value->y(), -6.0, 128.0) &&
                     close_scalar(tv->z() - base_value->z(), 3.0, 128.0) &&
                     close_vector(tf->u, base_first->u, 1024.0) &&
                     close_vector(tf->v, base_first->v, 1024.0) &&
                     close_vector(ts->uu, base_second->uu, 4096.0) &&
                     close_vector(ts->uv, base_second->uv, 4096.0) &&
                     close_vector(ts->vv, base_second->vv, 4096.0) &&
                     close_scalar(csv->x(), 2.0 * base_value->x(), 512.0) &&
                     close_scalar(csv->y(), 2.0 * base_value->y(), 512.0) &&
                     close_scalar(csv->z(), 2.0 * base_value->z(), 512.0) &&
                     close_scalar(csf->u.x(), 2.0 * base_first->u.x(), 2048.0) &&
                     close_scalar(csf->v.y(), 2.0 * base_first->v.y(), 2048.0) &&
                     close_scalar(css->uv.z(), 2.0 * base_second->uv.z(), 8192.0),
                 "bicubic NURBS affine covariance differs") &&
             passed;

    const double maximum = std::numeric_limits<double>::max();
    const double minimum = std::numeric_limits<double>::denorm_min();
    constexpr std::size_t extreme_count = 6U;
    const auto extreme_points = make_points(extreme_count, extreme_count);
    std::vector<double> extreme_weights(extreme_count * extreme_count);
    for (std::size_t index = 0; index < extreme_weights.size(); ++index) {
        extreme_weights[index] =
            index % 3U == 0U
                ? minimum
                : (index % 3U == 1U ? maximum : maximum / 4.0);
    }
    const std::vector<double> extreme_u_knots{-maximum / 2.0, 0.0};
    const std::vector<double> extreme_v_knots{-maximum / 3.0, maximum / 3.0};
    const auto extreme_surface = BicubicNURBSSurface3::make(
        extreme_points,
        extreme_weights,
        extreme_count,
        extreme_count,
        extreme_u_knots,
        extreme_v_knots,
        -maximum,
        maximum,
        -maximum,
        maximum);
    if (!extreme_surface) {
        return 1;
    }
    const auto extreme_value =
        extreme_surface->evaluate(maximum / 4.0, 0.0);
    const auto extreme_first =
        extreme_surface->first_derivatives(maximum / 4.0, 0.0);
    const auto extreme_second =
        extreme_surface->second_derivatives(maximum / 4.0, 0.0);
    passed = require(
                 extreme_value && extreme_first && extreme_second,
                 "bicubic NURBS extreme-finite jet failed avoidably") &&
             passed;

    auto huge_controls = flatten(controls);
    for (std::size_t j = 0; j < 4U; ++j) {
        huge_controls[j] =
            *Point3::make(-maximum, 0.0, 0.0);
        huge_controls[4U + j] =
            *Point3::make(maximum, 0.0, 0.0);
    }
    const auto unrepresentable = BicubicNURBSSurface3::make(
        huge_controls,
        std::vector<double>(16U, 1.0),
        4U,
        4U,
        {},
        {},
        0.0,
        minimum,
        0.0,
        1.0);
    if (!unrepresentable) {
        return 1;
    }
    const auto representable_value =
        unrepresentable->evaluate(0.0, 0.5);
    const auto unrepresentable_first =
        unrepresentable->first_derivatives(0.0, 0.5);
    passed = require(
                 representable_value &&
                     !unrepresentable_first &&
                     unrepresentable_first.error() ==
                         SurfaceError::non_finite_result,
                 "bicubic NURBS derivative-order failure isolation differs") &&
             passed;

    const auto insufficient_u = BicubicNURBSSurface3::make(
        std::vector<Point3>(12U, constant_point),
        std::vector<double>(12U, 1.0),
        3U,
        4U,
        {},
        {},
        0.0,
        1.0,
        0.0,
        1.0);
    const auto bad_size = BicubicNURBSSurface3::make(
        std::vector<Point3>(15U, constant_point),
        std::vector<double>(15U, 1.0),
        4U,
        4U,
        {},
        {},
        0.0,
        1.0,
        0.0,
        1.0);
    const auto bad_knot_count = BicubicNURBSSurface3::make(
        points,
        weights,
        u_count,
        v_count,
        std::vector<double>{-0.5},
        v_knots,
        -2.0,
        3.0,
        -3.0,
        4.0);
    auto bad_weights = weights;
    bad_weights[0] = 0.0;
    const auto bad_weight = BicubicNURBSSurface3::make(
        points,
        bad_weights,
        u_count,
        v_count,
        u_knots,
        v_knots,
        -2.0,
        3.0,
        -3.0,
        4.0);
    passed = require(
                 !insufficient_u &&
                     insufficient_u.error() ==
                         BicubicNURBSSurfaceConstructionError::
                             insufficient_u_control_points &&
                     !bad_size &&
                     bad_size.error() ==
                         BicubicNURBSSurfaceConstructionError::
                             control_net_size_mismatch &&
                     !bad_knot_count &&
                     bad_knot_count.error() ==
                         BicubicNURBSSurfaceConstructionError::
                             u_interior_knot_count_mismatch &&
                     !bad_weight &&
                     bad_weight.error() ==
                         BicubicNURBSSurfaceConstructionError::
                             non_positive_weight,
                 "bicubic NURBS construction failures differ") &&
             passed;

    const auto nan_u = surface->evaluate(
        std::numeric_limits<double>::quiet_NaN(), 0.0);
    const auto nan_v = surface->evaluate(
        0.0, std::numeric_limits<double>::quiet_NaN());
    const auto below_u = surface->first_derivatives(-2.1, 0.0);
    const auto above_v = surface->second_derivatives(0.0, 4.1);
    passed = require(
                 !nan_u &&
                     nan_u.error() == SurfaceError::non_finite_u_parameter &&
                     !nan_v &&
                     nan_v.error() == SurfaceError::non_finite_v_parameter &&
                     !below_u &&
                     below_u.error() ==
                         SurfaceError::u_parameter_out_of_domain &&
                     !above_v &&
                     above_v.error() ==
                         SurfaceError::v_parameter_out_of_domain,
                 "bicubic NURBS query failure semantics differ") &&
             passed;

    const auto repeat_value_a = surface->evaluate(0.75, 1.0);
    const auto repeat_value_b = surface->evaluate(0.75, 1.0);
    const auto repeat_first_a = surface->first_derivatives(0.75, 1.0);
    const auto repeat_first_b = surface->first_derivatives(0.75, 1.0);
    const auto repeat_second_a = surface->second_derivatives(0.75, 1.0);
    const auto repeat_second_b = surface->second_derivatives(0.75, 1.0);
    const auto repeat_failure_a = surface->evaluate(4.0, 1.0);
    const auto repeat_failure_b = surface->evaluate(4.0, 1.0);
    passed = require(
                 repeat_value_a && repeat_value_b &&
                     *repeat_value_a == *repeat_value_b &&
                     repeat_first_a && repeat_first_b &&
                     *repeat_first_a == *repeat_first_b &&
                     repeat_second_a && repeat_second_b &&
                     *repeat_second_a == *repeat_second_b &&
                     !repeat_failure_a && !repeat_failure_b &&
                     repeat_failure_a.error() == repeat_failure_b.error(),
                 "bicubic NURBS repeated evidence is not deterministic") &&
             passed;

    return passed ? 0 : 1;
}
