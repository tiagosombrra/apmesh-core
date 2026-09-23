#include "apmesh/geometry/nurbs.hpp"
#include "apmesh/geometry/nurbs_surface.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdio>
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

std::vector<double> flat_knots(
    const std::span<const double> interior_knots,
    const std::span<const std::uint8_t> multiplicities,
    const double lower,
    const double upper) {
    std::size_t interior_count = 0U;
    for (const std::uint8_t multiplicity : multiplicities) {
        interior_count += static_cast<std::size_t>(multiplicity);
    }

    std::vector<double> knots;
    knots.reserve(interior_count + 8U);
    for (int index = 0; index < 4; ++index) {
        knots.push_back(lower);
    }
    for (std::size_t index = 0; index < interior_knots.size(); ++index) {
        for (std::uint8_t repeat = 0U;
             repeat < multiplicities[index];
             ++repeat) {
            knots.push_back(interior_knots[index]);
        }
    }
    for (int index = 0; index < 4; ++index) {
        knots.push_back(upper);
    }
    return knots;
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
        static_cast<long double>(knots[static_cast<std::size_t>(index)]);
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
        static_cast<long double>(knots[static_cast<std::size_t>(index)]);
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

struct Long3 {
    long double x{};
    long double y{};
    long double z{};
};

Long3 add_scaled(
    const Long3& value,
    const apmesh::core::Point3& point,
    const long double scale) {
    return {
        value.x + scale * static_cast<long double>(point.x()),
        value.y + scale * static_cast<long double>(point.y()),
        value.z + scale * static_cast<long double>(point.z()),
    };
}

Long3 subtract_scaled(
    const Long3& lhs,
    const Long3& rhs,
    const long double scale) {
    return {
        lhs.x - scale * rhs.x,
        lhs.y - scale * rhs.y,
        lhs.z - scale * rhs.z,
    };
}

Long3 divide(const Long3& value, const long double denominator) {
    return {
        value.x / denominator,
        value.y / denominator,
        value.z / denominator,
    };
}

struct ReferenceJet {
    Long3 value{};
    Long3 u{};
    Long3 v{};
    Long3 uu{};
    Long3 uv{};
    Long3 vv{};
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

    Long3 a{};
    Long3 au{};
    Long3 av{};
    Long3 auu{};
    Long3 auv{};
    Long3 avv{};
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

    auto uu_numerator = subtract_scaled(auu, result.u, 2.0L * wu);
    uu_numerator = subtract_scaled(uu_numerator, result.value, wuu);
    result.uu = divide(uu_numerator, w);

    auto vv_numerator = subtract_scaled(avv, result.v, 2.0L * wv);
    vv_numerator = subtract_scaled(vv_numerator, result.value, wvv);
    result.vv = divide(vv_numerator, w);

    auto uv_numerator = subtract_scaled(auv, result.u, wv);
    uv_numerator = subtract_scaled(uv_numerator, result.v, wu);
    uv_numerator = subtract_scaled(uv_numerator, result.value, wuv);
    result.uv = divide(uv_numerator, w);

    return result;
}

std::vector<apmesh::core::Point3> make_points(
    const std::size_t u_count,
    const std::size_t v_count) {
    std::vector<apmesh::core::Point3> points;
    points.reserve(u_count * v_count);
    for (std::size_t i = 0; i < u_count; ++i) {
        for (std::size_t j = 0; j < v_count; ++j) {
            const double x =
                -2.5 + 0.95 * static_cast<double>(i) +
                0.17 * static_cast<double>(j);
            const double y =
                -1.5 + 0.72 * static_cast<double>(j) -
                0.11 * static_cast<double>(i);
            const double z =
                0.41 * static_cast<double>(i * i) -
                0.33 * static_cast<double>(j * j) +
                0.19 * static_cast<double>(i * j) +
                0.07 * static_cast<double>(i * i * j);
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
                0.65 + 0.2 * static_cast<double>((5U * i + 3U * j) % 9U));
        }
    }
    return weights;
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

std::vector<H4> insert_existing_knot(
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

struct SurfaceData {
    std::vector<apmesh::core::Point3> points;
    std::vector<double> weights;
    std::size_t u_count{};
    std::size_t v_count{};
};

SurfaceData insert_u(
    const SurfaceData& source,
    const std::span<const double> u_flat,
    const double knot) {
    std::vector<std::vector<H4>> columns(source.v_count);
    for (std::size_t v_index = 0; v_index < source.v_count; ++v_index) {
        std::vector<H4> column;
        column.reserve(source.u_count);
        for (std::size_t u_index = 0; u_index < source.u_count; ++u_index) {
            const std::size_t index =
                u_index * source.v_count + v_index;
            column.push_back(
                homogeneous(source.points[index], source.weights[index]));
        }
        columns[v_index] = insert_existing_knot(column, u_flat, knot);
    }

    SurfaceData result;
    result.u_count = source.u_count + 1U;
    result.v_count = source.v_count;
    result.points.reserve(result.u_count * result.v_count);
    result.weights.reserve(result.u_count * result.v_count);
    for (std::size_t u_index = 0; u_index < result.u_count; ++u_index) {
        for (std::size_t v_index = 0; v_index < result.v_count; ++v_index) {
            const H4& value = columns[v_index][u_index];
            result.points.push_back(*apmesh::core::Point3::make(
                static_cast<double>(value.x / value.w),
                static_cast<double>(value.y / value.w),
                static_cast<double>(value.z / value.w)));
            result.weights.push_back(static_cast<double>(value.w));
        }
    }
    return result;
}

SurfaceData insert_v(
    const SurfaceData& source,
    const std::span<const double> v_flat,
    const double knot) {
    SurfaceData result;
    result.u_count = source.u_count;
    result.v_count = source.v_count + 1U;
    result.points.reserve(result.u_count * result.v_count);
    result.weights.reserve(result.u_count * result.v_count);

    for (std::size_t u_index = 0; u_index < source.u_count; ++u_index) {
        std::vector<H4> row;
        row.reserve(source.v_count);
        for (std::size_t v_index = 0; v_index < source.v_count; ++v_index) {
            const std::size_t index =
                u_index * source.v_count + v_index;
            row.push_back(
                homogeneous(source.points[index], source.weights[index]));
        }
        const auto inserted = insert_existing_knot(row, v_flat, knot);
        for (const H4& value : inserted) {
            result.points.push_back(*apmesh::core::Point3::make(
                static_cast<double>(value.x / value.w),
                static_cast<double>(value.y / value.w),
                static_cast<double>(value.z / value.w)));
            result.weights.push_back(static_cast<double>(value.w));
        }
    }
    return result;
}

double second_jump_norm(
    const ReferenceJet& lhs,
    const ReferenceJet& rhs,
    const bool u_direction) {
    const Long3& a = u_direction ? lhs.uu : lhs.vv;
    const Long3& b = u_direction ? rhs.uu : rhs.vv;
    const long double dx = a.x - b.x;
    const long double dy = a.y - b.y;
    const long double dz = a.z - b.z;
    return static_cast<double>(std::sqrt(dx * dx + dy * dy + dz * dz));
}

} // namespace

int main() {
    using apmesh::core::BicubicNURBSSurface3;
    using apmesh::core::BicubicNURBSSurfaceConstructionError;
    using apmesh::core::BoundedParametricSurface3;
    using apmesh::core::MultiSpanCubicNURBS3;
    using apmesh::core::Point3;
    using apmesh::core::SurfaceError;

    static_assert(BoundedParametricSurface3<BicubicNURBSSurface3>);
    static_assert(std::same_as<
        decltype(
            std::declval<const BicubicNURBSSurface3&>().
                u_interior_multiplicities()),
        std::span<const std::uint8_t>>);
    static_assert(std::same_as<
        decltype(
            std::declval<const BicubicNURBSSurface3&>().
                v_interior_multiplicities()),
        std::span<const std::uint8_t>>);

    bool passed = true;

    const std::vector<double> u_knots{0.0, 1.5};
    const std::vector<double> v_knots{-0.5, 1.0};
    const std::vector<std::uint8_t> u_mult{2U, 1U};
    const std::vector<std::uint8_t> v_mult{1U, 2U};
    constexpr std::size_t u_count = 7U;
    constexpr std::size_t v_count = 7U;
    const auto points = make_points(u_count, v_count);
    const auto weights = make_weights(u_count, v_count);

    const auto surface = BicubicNURBSSurface3::make(
        points,
        weights,
        u_count,
        v_count,
        u_knots,
        v_knots,
        u_mult,
        v_mult,
        -2.0,
        3.0,
        -1.5,
        2.5);
    if (!surface) {
        return 1;
    }

    passed = require(
                 surface->u_span_count() == 3U &&
                     surface->v_span_count() == 3U &&
                     surface->u_interior_multiplicities().size() == 2U &&
                     surface->v_interior_multiplicities().size() == 2U &&
                     surface->u_interior_multiplicities()[0] == 2U &&
                     surface->u_interior_multiplicities()[1] == 1U &&
                     surface->v_interior_multiplicities()[0] == 1U &&
                     surface->v_interior_multiplicities()[1] == 2U,
                 "surface multiplicity storage/span semantics differ") &&
             passed;

    const auto u_count_mismatch = BicubicNURBSSurface3::make(
        points,
        weights,
        u_count,
        v_count,
        u_knots,
        v_knots,
        std::vector<std::uint8_t>{2U},
        v_mult,
        -2.0,
        3.0,
        -1.5,
        2.5);
    const auto v_unsupported = BicubicNURBSSurface3::make(
        points,
        weights,
        u_count,
        v_count,
        u_knots,
        v_knots,
        u_mult,
        std::vector<std::uint8_t>{1U, 3U},
        -2.0,
        3.0,
        -1.5,
        2.5);
    const auto size_mismatch = BicubicNURBSSurface3::make(
        points,
        weights,
        6U,
        v_count,
        u_knots,
        v_knots,
        u_mult,
        v_mult,
        -2.0,
        3.0,
        -1.5,
        2.5);

    passed = require(
                 !u_count_mismatch &&
                     u_count_mismatch.error() ==
                         BicubicNURBSSurfaceConstructionError::
                             u_interior_multiplicity_count_mismatch &&
                     !v_unsupported &&
                     v_unsupported.error() ==
                         BicubicNURBSSurfaceConstructionError::
                             unsupported_v_interior_multiplicity &&
                     !size_mismatch &&
                     size_mismatch.error() ==
                         BicubicNURBSSurfaceConstructionError::
                             u_interior_knot_count_mismatch,
                 "surface multiplicity construction failures differ") &&
             passed;

    const auto u_flat = flat_knots(u_knots, u_mult, -2.0, 3.0);
    const auto v_flat = flat_knots(v_knots, v_mult, -1.5, 2.5);

    constexpr double u_double = 0.0;
    constexpr double v_double = 1.0;
    const std::array<std::pair<double, double>, 5> samples{{
        {-1.0, 0.0},
        {0.75, 0.0},
        {1.5, 0.25},
        {2.25, -0.5},
        {2.5, 1.75},
    }};

    for (const auto& [u, v] : samples) {
        const auto reference =
            reference_jet(points, weights, u_count, v_count, u_flat, v_flat, u, v);
        const auto value = surface->evaluate(u, v);
        const auto first = surface->first_derivatives(u, v);
        const auto second = surface->second_derivatives(u, v);
        passed = require(
                     value && first && second &&
                         close_scalar(value->x(), static_cast<double>(reference.value.x), 512.0) &&
                         close_scalar(value->y(), static_cast<double>(reference.value.y), 512.0) &&
                         close_scalar(value->z(), static_cast<double>(reference.value.z), 512.0) &&
                         close_scalar(first->u.x(), static_cast<double>(reference.u.x), 2048.0) &&
                         close_scalar(first->u.y(), static_cast<double>(reference.u.y), 2048.0) &&
                         close_scalar(first->u.z(), static_cast<double>(reference.u.z), 2048.0) &&
                         close_scalar(first->v.x(), static_cast<double>(reference.v.x), 2048.0) &&
                         close_scalar(first->v.y(), static_cast<double>(reference.v.y), 2048.0) &&
                         close_scalar(first->v.z(), static_cast<double>(reference.v.z), 2048.0) &&
                         close_scalar(second->uu.x(), static_cast<double>(reference.uu.x), 8192.0) &&
                         close_scalar(second->uu.y(), static_cast<double>(reference.uu.y), 8192.0) &&
                         close_scalar(second->uu.z(), static_cast<double>(reference.uu.z), 8192.0) &&
                         close_scalar(second->uv.x(), static_cast<double>(reference.uv.x), 8192.0) &&
                         close_scalar(second->uv.y(), static_cast<double>(reference.uv.y), 8192.0) &&
                         close_scalar(second->uv.z(), static_cast<double>(reference.uv.z), 8192.0) &&
                         close_scalar(second->vv.x(), static_cast<double>(reference.vv.x), 8192.0) &&
                         close_scalar(second->vv.y(), static_cast<double>(reference.vv.y), 8192.0) &&
                         close_scalar(second->vv.z(), static_cast<double>(reference.vv.z), 8192.0),
                     "surface repeated-knot rational tensor oracle differs") &&
                 passed;
    }

    const auto u_line_reference =
        reference_jet(
            points, weights, u_count, v_count, u_flat, v_flat, u_double, 0.25);
    const auto u_line_value = surface->evaluate(u_double, 0.25);
    const auto u_line_first = surface->first_derivatives(u_double, 0.25);
    const auto u_line_second = surface->second_derivatives(u_double, 0.25);
    passed = require(
                 u_line_value && u_line_first && !u_line_second &&
                     close_scalar(
                         u_line_value->x(),
                         static_cast<double>(u_line_reference.value.x),
                         512.0) &&
                     close_scalar(
                         u_line_first->u.z(),
                         static_cast<double>(u_line_reference.u.z),
                         2048.0) &&
                     close_scalar(
                         u_line_first->v.z(),
                         static_cast<double>(u_line_reference.v.z),
                         2048.0) &&
                     u_line_second.error() ==
                         SurfaceError::insufficient_continuity,
                 "U double-knot line semantics differ") &&
             passed;

    const auto v_line_reference =
        reference_jet(
            points, weights, u_count, v_count, u_flat, v_flat, 0.75, v_double);
    const auto v_line_value = surface->evaluate(0.75, v_double);
    const auto v_line_first = surface->first_derivatives(0.75, v_double);
    const auto v_line_second = surface->second_derivatives(0.75, v_double);
    passed = require(
                 v_line_value && v_line_first && !v_line_second &&
                     close_scalar(
                         v_line_value->y(),
                         static_cast<double>(v_line_reference.value.y),
                         512.0) &&
                     close_scalar(
                         v_line_first->u.x(),
                         static_cast<double>(v_line_reference.u.x),
                         2048.0) &&
                     close_scalar(
                         v_line_first->v.x(),
                         static_cast<double>(v_line_reference.v.x),
                         2048.0) &&
                     v_line_second.error() ==
                         SurfaceError::insufficient_continuity,
                 "V double-knot line semantics differ") &&
             passed;

    const auto intersection_value = surface->evaluate(u_double, v_double);
    const auto intersection_first =
        surface->first_derivatives(u_double, v_double);
    const auto intersection_second =
        surface->second_derivatives(u_double, v_double);
    passed = require(
                 intersection_value && intersection_first &&
                     !intersection_second &&
                     intersection_second.error() ==
                         SurfaceError::insufficient_continuity,
                 "U/V double-line intersection semantics differ") &&
             passed;

    const auto simple_u_second = surface->second_derivatives(1.5, 0.25);
    const auto simple_v_second = surface->second_derivatives(0.75, -0.5);
    passed = require(
                 simple_u_second && simple_v_second,
                 "second derivatives failed at simple knot line") &&
             passed;

    const double u_left =
        std::nextafter(u_double, -std::numeric_limits<double>::infinity());
    const double u_right =
        std::nextafter(u_double, std::numeric_limits<double>::infinity());
    const auto u_left_reference =
        reference_jet(
            points, weights, u_count, v_count, u_flat, v_flat, u_left, 0.25);
    const auto u_right_reference =
        reference_jet(
            points, weights, u_count, v_count, u_flat, v_flat, u_right, 0.25);
    const double u_jump =
        second_jump_norm(u_left_reference, u_right_reference, true);
    const auto u_left_first = surface->first_derivatives(u_left, 0.25);
    const auto u_right_first = surface->first_derivatives(u_right, 0.25);
    const auto u_left_second = surface->second_derivatives(u_left, 0.25);
    const auto u_right_second = surface->second_derivatives(u_right, 0.25);
    passed = require(
                 u_jump > 1.0e-7 &&
                     u_left_first && u_right_first &&
                     close_vector(u_left_first->u, u_right_first->u, 8192.0) &&
                     close_vector(u_left_first->v, u_right_first->v, 8192.0) &&
                     u_left_second && u_right_second,
                 "U C1 fixture does not expose continuous first / distinct second jet") &&
             passed;

    const double v_left =
        std::nextafter(v_double, -std::numeric_limits<double>::infinity());
    const double v_right =
        std::nextafter(v_double, std::numeric_limits<double>::infinity());
    const auto v_left_reference =
        reference_jet(
            points, weights, u_count, v_count, u_flat, v_flat, 0.75, v_left);
    const auto v_right_reference =
        reference_jet(
            points, weights, u_count, v_count, u_flat, v_flat, 0.75, v_right);
    const double v_jump =
        second_jump_norm(v_left_reference, v_right_reference, false);
    const auto v_left_first = surface->first_derivatives(0.75, v_left);
    const auto v_right_first = surface->first_derivatives(0.75, v_right);
    const auto v_left_second = surface->second_derivatives(0.75, v_left);
    const auto v_right_second = surface->second_derivatives(0.75, v_right);
    passed = require(
                 v_jump > 1.0e-7 &&
                     v_left_first && v_right_first &&
                     close_vector(v_left_first->u, v_right_first->u, 8192.0) &&
                     close_vector(v_left_first->v, v_right_first->v, 8192.0) &&
                     v_left_second && v_right_second,
                 "V C1 fixture does not expose continuous first / distinct second jet") &&
             passed;

    constexpr std::size_t simple_count = 6U;
    const std::vector<double> simple_u_knots{0.0, 1.5};
    const std::vector<double> simple_v_knots{-0.5, 1.0};
    const std::vector<std::uint8_t> simple_mult{1U, 1U};
    const auto simple_points = make_points(simple_count, simple_count);
    const auto simple_weights = make_weights(simple_count, simple_count);

    const auto legacy = BicubicNURBSSurface3::make(
        simple_points,
        simple_weights,
        simple_count,
        simple_count,
        simple_u_knots,
        simple_v_knots,
        -2.0,
        3.0,
        -1.5,
        2.5);
    const auto explicit_simple = BicubicNURBSSurface3::make(
        simple_points,
        simple_weights,
        simple_count,
        simple_count,
        simple_u_knots,
        simple_v_knots,
        simple_mult,
        simple_mult,
        -2.0,
        3.0,
        -1.5,
        2.5);
    if (!legacy || !explicit_simple) {
        return 1;
    }
    passed = require(
                 *legacy == *explicit_simple,
                 "legacy simple-knot surface construction changed") &&
             passed;

    SurfaceData source{
        .points = simple_points,
        .weights = simple_weights,
        .u_count = simple_count,
        .v_count = simple_count,
    };
    const auto simple_u_flat =
        flat_knots(simple_u_knots, simple_mult, -2.0, 3.0);
    const auto simple_v_flat =
        flat_knots(simple_v_knots, simple_mult, -1.5, 2.5);
    const SurfaceData u_inserted =
        insert_u(source, simple_u_flat, simple_u_knots[0]);
    const SurfaceData both_inserted =
        insert_v(u_inserted, simple_v_flat, simple_v_knots[1]);

    const auto inserted_surface = BicubicNURBSSurface3::make(
        both_inserted.points,
        both_inserted.weights,
        both_inserted.u_count,
        both_inserted.v_count,
        simple_u_knots,
        simple_v_knots,
        std::vector<std::uint8_t>{2U, 1U},
        std::vector<std::uint8_t>{1U, 2U},
        -2.0,
        3.0,
        -1.5,
        2.5);
    if (!inserted_surface) {
        return 1;
    }

    const std::array<std::pair<double, double>, 4> insertion_samples{{
        {-1.0, 0.0},
        {0.75, 0.0},
        {2.25, -0.5},
        {2.5, 1.75},
    }};
    for (const auto& [u, v] : insertion_samples) {
        const auto base_value = legacy->evaluate(u, v);
        const auto inserted_value = inserted_surface->evaluate(u, v);
        const auto base_first = legacy->first_derivatives(u, v);
        const auto inserted_first = inserted_surface->first_derivatives(u, v);
        const auto base_second = legacy->second_derivatives(u, v);
        const auto inserted_second =
            inserted_surface->second_derivatives(u, v);
        passed = require(
                     base_value && inserted_value &&
                         close_point(*base_value, *inserted_value, 1024.0) &&
                         base_first && inserted_first &&
                         close_vector(base_first->u, inserted_first->u, 4096.0) &&
                         close_vector(base_first->v, inserted_first->v, 4096.0) &&
                         base_second && inserted_second &&
                         close_vector(base_second->uu, inserted_second->uu, 16384.0) &&
                         close_vector(base_second->uv, inserted_second->uv, 16384.0) &&
                         close_vector(base_second->vv, inserted_second->vv, 16384.0),
                     "test-only repeated-knot insertion changed off-line geometry") &&
                 passed;
    }

    const auto inserted_line_value =
        inserted_surface->evaluate(simple_u_knots[0], 0.0);
    const auto inserted_line_first =
        inserted_surface->first_derivatives(simple_u_knots[0], 0.0);
    const auto inserted_line_second =
        inserted_surface->second_derivatives(simple_u_knots[0], 0.0);
    const auto base_line_value = legacy->evaluate(simple_u_knots[0], 0.0);
    const auto base_line_first =
        legacy->first_derivatives(simple_u_knots[0], 0.0);
    const auto base_line_second =
        legacy->second_derivatives(simple_u_knots[0], 0.0);
    passed = require(
                 inserted_line_value && base_line_value &&
                     close_point(*inserted_line_value, *base_line_value, 1024.0) &&
                     inserted_line_first && base_line_first &&
                     close_vector(inserted_line_first->u, base_line_first->u, 4096.0) &&
                     close_vector(inserted_line_first->v, base_line_first->v, 4096.0) &&
                     !inserted_line_second &&
                     inserted_line_second.error() ==
                         SurfaceError::insufficient_continuity &&
                     base_line_second,
                 "repeated-knot insertion did not preserve physical surface / C1 policy") &&
             passed;

    std::vector<Point3> u_boundary_points;
    std::vector<double> u_boundary_weights;
    u_boundary_points.reserve(u_count);
    u_boundary_weights.reserve(u_count);
    for (std::size_t i = 0; i < u_count; ++i) {
        const std::size_t index = i * v_count;
        u_boundary_points.push_back(points[index]);
        u_boundary_weights.push_back(weights[index]);
    }
    const auto u_boundary = MultiSpanCubicNURBS3::make(
        u_boundary_points,
        u_boundary_weights,
        u_knots,
        u_mult,
        -2.0,
        3.0);
    if (!u_boundary) {
        return 1;
    }
    const auto surface_u_value = surface->evaluate(u_double, -1.5);
    const auto surface_u_first = surface->first_derivatives(u_double, -1.5);
    const auto surface_u_second = surface->second_derivatives(u_double, -1.5);
    const auto curve_u_value = u_boundary->evaluate(u_double);
    const auto curve_u_first = u_boundary->first_derivative(u_double);
    const auto curve_u_second = u_boundary->second_derivative(u_double);
    passed = require(
                 surface_u_value && curve_u_value &&
                     close_point(*surface_u_value, *curve_u_value, 1024.0) &&
                     surface_u_first && curve_u_first &&
                     close_vector(surface_u_first->u, *curve_u_first, 4096.0) &&
                     !surface_u_second && !curve_u_second &&
                     surface_u_second.error() ==
                         SurfaceError::insufficient_continuity &&
                     curve_u_second.error() ==
                         apmesh::core::CurveError::insufficient_continuity,
                 "U boundary curve C1 parity differs") &&
             passed;

    std::vector<Point3> v_boundary_points;
    std::vector<double> v_boundary_weights;
    v_boundary_points.reserve(v_count);
    v_boundary_weights.reserve(v_count);
    for (std::size_t j = 0; j < v_count; ++j) {
        v_boundary_points.push_back(points[j]);
        v_boundary_weights.push_back(weights[j]);
    }
    const auto v_boundary = MultiSpanCubicNURBS3::make(
        v_boundary_points,
        v_boundary_weights,
        v_knots,
        v_mult,
        -1.5,
        2.5);
    if (!v_boundary) {
        return 1;
    }
    const auto surface_v_value = surface->evaluate(-2.0, v_double);
    const auto surface_v_first = surface->first_derivatives(-2.0, v_double);
    const auto surface_v_second = surface->second_derivatives(-2.0, v_double);
    const auto curve_v_value = v_boundary->evaluate(v_double);
    const auto curve_v_first = v_boundary->first_derivative(v_double);
    const auto curve_v_second = v_boundary->second_derivative(v_double);
    passed = require(
                 surface_v_value && curve_v_value &&
                     close_point(*surface_v_value, *curve_v_value, 1024.0) &&
                     surface_v_first && curve_v_first &&
                     close_vector(surface_v_first->v, *curve_v_first, 4096.0) &&
                     !surface_v_second && !curve_v_second &&
                     surface_v_second.error() ==
                         SurfaceError::insufficient_continuity &&
                     curve_v_second.error() ==
                         apmesh::core::CurveError::insufficient_continuity,
                 "V boundary curve C1 parity differs") &&
             passed;

    const auto u_reversed = surface->u_reversed();
    const auto v_reversed = surface->v_reversed();
    const auto uu = u_reversed.u_reversed();
    const auto vv = v_reversed.v_reversed();
    passed = require(
                 uu == *surface && vv == *surface &&
                     u_reversed.u_interior_multiplicities()[0] == 1U &&
                     u_reversed.u_interior_multiplicities()[1] == 2U &&
                     v_reversed.v_interior_multiplicities()[0] == 2U &&
                     v_reversed.v_interior_multiplicities()[1] == 1U,
                 "surface multiplicity reversal/involution differs") &&
             passed;

    const double mapped_u = 1.0;
    const double mapped_v = 0.0;
    const auto u_reversed_second =
        u_reversed.second_derivatives(mapped_u, 0.25);
    const auto v_reversed_second =
        v_reversed.second_derivatives(0.75, mapped_v);
    passed = require(
                 !u_reversed_second && !v_reversed_second &&
                     u_reversed_second.error() ==
                         SurfaceError::insufficient_continuity &&
                     v_reversed_second.error() ==
                         SurfaceError::insufficient_continuity,
                 "surface reversal did not preserve C1 failure lines") &&
             passed;

    const auto constant_point = Point3::make(3.0, -4.0, 2.0);
    if (!constant_point) {
        return 1;
    }
    const std::vector<Point3> constant_points(
        points.size(), *constant_point);
    const auto constant_surface = BicubicNURBSSurface3::make(
        constant_points,
        weights,
        u_count,
        v_count,
        u_knots,
        v_knots,
        u_mult,
        v_mult,
        -2.0,
        3.0,
        -1.5,
        2.5);
    if (!constant_surface) {
        return 1;
    }
    const auto constant_value =
        constant_surface->evaluate(u_double, 0.25);
    const auto constant_first =
        constant_surface->first_derivatives(u_double, 0.25);
    const auto constant_second =
        constant_surface->second_derivatives(u_double, 0.25);
    passed = require(
                 constant_value && *constant_value == *constant_point &&
                     constant_first &&
                     constant_first->u.x() == 0.0 &&
                     constant_first->u.y() == 0.0 &&
                     constant_first->u.z() == 0.0 &&
                     constant_first->v.x() == 0.0 &&
                     constant_first->v.y() == 0.0 &&
                     constant_first->v.z() == 0.0 &&
                     !constant_second &&
                     constant_second.error() ==
                         SurfaceError::insufficient_continuity,
                 "constant surface bypassed representation-level C1 policy") &&
             passed;

    const auto nan_second = surface->second_derivatives(
        std::numeric_limits<double>::quiet_NaN(), v_double);
    const auto outside_second = surface->second_derivatives(
        u_double, 3.0);
    passed = require(
                 !nan_second &&
                     nan_second.error() ==
                         SurfaceError::non_finite_u_parameter &&
                     !outside_second &&
                     outside_second.error() ==
                         SurfaceError::v_parameter_out_of_domain,
                 "surface second-jet validation order changed") &&
             passed;

    const auto repeat_a = surface->second_derivatives(u_double, 0.25);
    const auto repeat_b = surface->second_derivatives(u_double, 0.25);
    const auto repeat_value_a = surface->evaluate(u_double, 0.25);
    const auto repeat_value_b = surface->evaluate(u_double, 0.25);
    passed = require(
                 !repeat_a && !repeat_b &&
                     repeat_a.error() == repeat_b.error() &&
                     repeat_value_a && repeat_value_b &&
                     *repeat_value_a == *repeat_value_b,
                 "surface C1 success/failure evidence is not deterministic") &&
             passed;

    return passed ? 0 : 1;
}
