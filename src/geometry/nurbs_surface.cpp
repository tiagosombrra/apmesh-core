#include "apmesh/geometry/nurbs_surface.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <limits>
#include <span>
#include <utility>
#include <vector>

namespace apmesh::core {
namespace {

struct Homogeneous4 {
    long double x{};
    long double y{};
    long double z{};
    long double w{};
};

template <std::size_t UCount, std::size_t VCount>
using HomogeneousNet =
    std::array<std::array<Homogeneous4, VCount>, UCount>;

[[nodiscard]] bool finite(const Homogeneous4& value) noexcept {
    return std::isfinite(value.x) &&
           std::isfinite(value.y) &&
           std::isfinite(value.z) &&
           std::isfinite(value.w);
}

[[nodiscard]] Homogeneous4 interpolate(
    const Homogeneous4& lhs,
    const Homogeneous4& rhs,
    const long double parameter) noexcept {
    return Homogeneous4{
        .x = std::lerp(lhs.x, rhs.x, parameter),
        .y = std::lerp(lhs.y, rhs.y, parameter),
        .z = std::lerp(lhs.z, rhs.z, parameter),
        .w = std::lerp(lhs.w, rhs.w, parameter),
    };
}

[[nodiscard]] std::expected<void, SurfaceError> validate_parameters(
    const SurfaceParameterDomain& domain,
    const double u,
    const double v) noexcept {
    if (!std::isfinite(u)) {
        return std::unexpected{SurfaceError::non_finite_u_parameter};
    }
    if (!std::isfinite(v)) {
        return std::unexpected{SurfaceError::non_finite_v_parameter};
    }
    if (u < domain.u.lower() || u > domain.u.upper()) {
        return std::unexpected{SurfaceError::u_parameter_out_of_domain};
    }
    if (v < domain.v.lower() || v > domain.v.upper()) {
        return std::unexpected{SurfaceError::v_parameter_out_of_domain};
    }
    return {};
}

[[nodiscard]] std::expected<long double, SurfaceError> parameter_ratio(
    const double value,
    const double lower,
    const double upper) noexcept {
    long double wide_value = static_cast<long double>(value);
    long double wide_lower = static_cast<long double>(lower);
    long double wide_upper = static_cast<long double>(upper);

    long double numerator = wide_value - wide_lower;
    long double denominator = wide_upper - wide_lower;

    if (!std::isfinite(numerator) || !std::isfinite(denominator)) {
        const long double scale = std::max(
            {std::abs(wide_value), std::abs(wide_lower), std::abs(wide_upper)});
        if (!std::isfinite(scale) || scale == 0.0L) {
            return std::unexpected{SurfaceError::non_finite_result};
        }
        wide_value /= scale;
        wide_lower /= scale;
        wide_upper /= scale;
        numerator = wide_value - wide_lower;
        denominator = wide_upper - wide_lower;
    }

    if (!std::isfinite(numerator) || !std::isfinite(denominator) ||
        denominator <= 0.0L) {
        return std::unexpected{SurfaceError::non_finite_result};
    }

    const long double ratio = numerator / denominator;
    if (!std::isfinite(ratio) || ratio < 0.0L || ratio > 1.0L) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    return ratio;
}

[[nodiscard]] std::size_t locate_flat_span(
    const std::span<const double> flat_knots,
    const double parameter) noexcept {
    if (parameter == flat_knots.back()) {
        return flat_knots.size() - 5U;
    }
    const auto iterator =
        std::upper_bound(flat_knots.begin(), flat_knots.end(), parameter);
    return static_cast<std::size_t>(iterator - flat_knots.begin() - 1);
}

template <std::size_t Degree>
[[nodiscard]] std::expected<Homogeneous4, SurfaceError> de_boor_local(
    std::array<Homogeneous4, Degree + 1U> work,
    const std::size_t spline_span,
    const std::size_t knot_offset,
    const std::span<const double> flat_knots,
    const double parameter) noexcept {
    for (int level = 1; level <= static_cast<int>(Degree); ++level) {
        for (int local = static_cast<int>(Degree); local >= level; --local) {
            const std::size_t knot_index =
                spline_span - Degree + static_cast<std::size_t>(local);
            const std::size_t upper_index =
                knot_index + Degree - static_cast<std::size_t>(level) + 1U;
            const auto alpha = parameter_ratio(
                parameter,
                flat_knots[knot_index + knot_offset],
                flat_knots[upper_index + knot_offset]);
            if (!alpha.has_value()) {
                return std::unexpected{alpha.error()};
            }
            work[static_cast<std::size_t>(local)] = interpolate(
                work[static_cast<std::size_t>(local - 1)],
                work[static_cast<std::size_t>(local)],
                *alpha);
            if (!finite(work[static_cast<std::size_t>(local)])) {
                return std::unexpected{SurfaceError::non_finite_result};
            }
        }
    }
    return work[Degree];
}

[[nodiscard]] std::expected<long double, SurfaceError> scaled_difference(
    const long double next,
    const long double current,
    const int degree,
    const double lower_knot,
    const double upper_knot) noexcept {
    const long double denominator =
        static_cast<long double>(upper_knot) -
        static_cast<long double>(lower_knot);
    const long double numerator =
        static_cast<long double>(degree) * (next - current);
    if (!std::isfinite(denominator) || denominator <= 0.0L ||
        !std::isfinite(numerator)) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    const long double result = numerator / denominator;
    if (!std::isfinite(result)) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    return result;
}

[[nodiscard]] std::expected<Homogeneous4, SurfaceError> derivative_control(
    const Homogeneous4& current,
    const Homogeneous4& next,
    const int degree,
    const double lower_knot,
    const double upper_knot) noexcept {
    const auto x = scaled_difference(
        next.x, current.x, degree, lower_knot, upper_knot);
    const auto y = scaled_difference(
        next.y, current.y, degree, lower_knot, upper_knot);
    const auto z = scaled_difference(
        next.z, current.z, degree, lower_knot, upper_knot);
    const auto w = scaled_difference(
        next.w, current.w, degree, lower_knot, upper_knot);
    if (!x || !y || !z || !w) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    return Homogeneous4{*x, *y, *z, *w};
}

template <std::size_t UCount, std::size_t VCount>
[[nodiscard]] std::expected<
    HomogeneousNet<UCount - 1U, VCount>,
    SurfaceError>
differentiate_u(
    const HomogeneousNet<UCount, VCount>& controls,
    const std::span<const double> flat_knots,
    const std::size_t start,
    const int degree,
    const std::size_t lower_offset,
    const std::size_t upper_offset) noexcept {
    static_assert(UCount >= 2U);
    HomogeneousNet<UCount - 1U, VCount> result{};
    for (std::size_t u_index = 0; u_index + 1U < UCount; ++u_index) {
        const std::size_t global = start + u_index;
        for (std::size_t v_index = 0; v_index < VCount; ++v_index) {
            const auto value = derivative_control(
                controls[u_index][v_index],
                controls[u_index + 1U][v_index],
                degree,
                flat_knots[global + lower_offset],
                flat_knots[global + upper_offset]);
            if (!value.has_value()) {
                return std::unexpected{value.error()};
            }
            result[u_index][v_index] = *value;
        }
    }
    return result;
}

template <std::size_t UCount, std::size_t VCount>
[[nodiscard]] std::expected<
    HomogeneousNet<UCount, VCount - 1U>,
    SurfaceError>
differentiate_v(
    const HomogeneousNet<UCount, VCount>& controls,
    const std::span<const double> flat_knots,
    const std::size_t start,
    const int degree,
    const std::size_t lower_offset,
    const std::size_t upper_offset) noexcept {
    static_assert(VCount >= 2U);
    HomogeneousNet<UCount, VCount - 1U> result{};
    for (std::size_t u_index = 0; u_index < UCount; ++u_index) {
        for (std::size_t v_index = 0; v_index + 1U < VCount; ++v_index) {
            const std::size_t global = start + v_index;
            const auto value = derivative_control(
                controls[u_index][v_index],
                controls[u_index][v_index + 1U],
                degree,
                flat_knots[global + lower_offset],
                flat_knots[global + upper_offset]);
            if (!value.has_value()) {
                return std::unexpected{value.error()};
            }
            result[u_index][v_index] = *value;
        }
    }
    return result;
}

template <
    std::size_t UDegree,
    std::size_t VDegree,
    std::size_t UCount,
    std::size_t VCount>
[[nodiscard]] std::expected<Homogeneous4, SurfaceError> evaluate_tensor(
    const HomogeneousNet<UCount, VCount>& controls,
    const std::size_t u_span,
    const std::size_t v_span,
    const std::size_t u_offset,
    const std::size_t v_offset,
    const std::span<const double> u_flat_knots,
    const std::span<const double> v_flat_knots,
    const double u,
    const double v) noexcept {
    static_assert(UCount == UDegree + 1U);
    static_assert(VCount == VDegree + 1U);

    std::array<Homogeneous4, UCount> rows{};
    for (std::size_t u_index = 0; u_index < UCount; ++u_index) {
        const auto value = de_boor_local<VDegree>(
            controls[u_index],
            v_span,
            v_offset,
            v_flat_knots,
            v);
        if (!value.has_value()) {
            return std::unexpected{value.error()};
        }
        rows[u_index] = *value;
    }

    return de_boor_local<UDegree>(
        rows,
        u_span,
        u_offset,
        u_flat_knots,
        u);
}

[[nodiscard]] HomogeneousNet<4U, 4U> local_homogeneous_controls(
    const std::span<const Point3> points,
    const std::span<const double> weights,
    const std::size_t v_control_count,
    const std::size_t u_start,
    const std::size_t v_start) noexcept {
    double maximum_weight = weights[u_start * v_control_count + v_start];
    for (std::size_t u_local = 0; u_local < 4U; ++u_local) {
        for (std::size_t v_local = 0; v_local < 4U; ++v_local) {
            maximum_weight = std::max(
                maximum_weight,
                weights[
                    (u_start + u_local) * v_control_count +
                    (v_start + v_local)]);
        }
    }

    HomogeneousNet<4U, 4U> result{};
    for (std::size_t u_local = 0; u_local < 4U; ++u_local) {
        for (std::size_t v_local = 0; v_local < 4U; ++v_local) {
            const std::size_t index =
                (u_start + u_local) * v_control_count + (v_start + v_local);
            const long double weight =
                static_cast<long double>(weights[index]) /
                static_cast<long double>(maximum_weight);
            const Point3& point = points[index];
            result[u_local][v_local] = Homogeneous4{
                .x = static_cast<long double>(point.x()) * weight,
                .y = static_cast<long double>(point.y()) * weight,
                .z = static_cast<long double>(point.z()) * weight,
                .w = weight,
            };
        }
    }
    return result;
}

struct HomogeneousFirstJet {
    Homogeneous4 value{};
    Homogeneous4 u{};
    Homogeneous4 v{};
};

[[nodiscard]] std::expected<Homogeneous4, SurfaceError> evaluate_value(
    const std::span<const Point3> points,
    const std::span<const double> weights,
    const std::size_t v_control_count,
    const std::span<const double> u_flat_knots,
    const std::span<const double> v_flat_knots,
    const double u,
    const double v) noexcept {
    const std::size_t u_span = locate_flat_span(u_flat_knots, u);
    const std::size_t v_span = locate_flat_span(v_flat_knots, v);
    const auto controls = local_homogeneous_controls(
        points,
        weights,
        v_control_count,
        u_span - 3U,
        v_span - 3U);
    return evaluate_tensor<3U, 3U>(
        controls,
        u_span,
        v_span,
        0U,
        0U,
        u_flat_knots,
        v_flat_knots,
        u,
        v);
}

[[nodiscard]] std::expected<HomogeneousFirstJet, SurfaceError>
evaluate_first_jet(
    const std::span<const Point3> points,
    const std::span<const double> weights,
    const std::size_t v_control_count,
    const std::span<const double> u_flat_knots,
    const std::span<const double> v_flat_knots,
    const double u,
    const double v) noexcept {
    const std::size_t u_span = locate_flat_span(u_flat_knots, u);
    const std::size_t v_span = locate_flat_span(v_flat_knots, v);
    const std::size_t u_start = u_span - 3U;
    const std::size_t v_start = v_span - 3U;

    const auto controls = local_homogeneous_controls(
        points, weights, v_control_count, u_start, v_start);
    const auto du = differentiate_u(
        controls, u_flat_knots, u_start, 3, 1U, 4U);
    const auto dv = differentiate_v(
        controls, v_flat_knots, v_start, 3, 1U, 4U);
    if (!du || !dv) {
        return std::unexpected{SurfaceError::non_finite_result};
    }

    const auto value = evaluate_tensor<3U, 3U>(
        controls, u_span, v_span, 0U, 0U,
        u_flat_knots, v_flat_knots, u, v);
    const auto u_value = evaluate_tensor<2U, 3U>(
        *du, u_span - 1U, v_span, 1U, 0U,
        u_flat_knots, v_flat_knots, u, v);
    const auto v_value = evaluate_tensor<3U, 2U>(
        *dv, u_span, v_span - 1U, 0U, 1U,
        u_flat_knots, v_flat_knots, u, v);
    if (!value || !u_value || !v_value) {
        return std::unexpected{SurfaceError::non_finite_result};
    }

    return HomogeneousFirstJet{
        .value = *value,
        .u = *u_value,
        .v = *v_value,
    };
}

struct HomogeneousJet {
    Homogeneous4 value{};
    Homogeneous4 u{};
    Homogeneous4 v{};
    Homogeneous4 uu{};
    Homogeneous4 uv{};
    Homogeneous4 vv{};
};

[[nodiscard]] std::expected<HomogeneousJet, SurfaceError> evaluate_jet(
    const std::span<const Point3> points,
    const std::span<const double> weights,
    const std::size_t v_control_count,
    const std::span<const double> u_flat_knots,
    const std::span<const double> v_flat_knots,
    const double u,
    const double v) noexcept {
    const std::size_t u_span = locate_flat_span(u_flat_knots, u);
    const std::size_t v_span = locate_flat_span(v_flat_knots, v);
    const std::size_t u_start = u_span - 3U;
    const std::size_t v_start = v_span - 3U;

    const auto controls = local_homogeneous_controls(
        points, weights, v_control_count, u_start, v_start);
    const auto du = differentiate_u(
        controls, u_flat_knots, u_start, 3, 1U, 4U);
    const auto dv = differentiate_v(
        controls, v_flat_knots, v_start, 3, 1U, 4U);
    if (!du || !dv) {
        return std::unexpected{SurfaceError::non_finite_result};
    }

    const auto duu = differentiate_u(
        *du, u_flat_knots, u_start, 2, 2U, 4U);
    const auto duv = differentiate_v(
        *du, v_flat_knots, v_start, 3, 1U, 4U);
    const auto dvv = differentiate_v(
        *dv, v_flat_knots, v_start, 2, 2U, 4U);
    if (!duu || !duv || !dvv) {
        return std::unexpected{SurfaceError::non_finite_result};
    }

    const auto value = evaluate_tensor<3U, 3U>(
        controls, u_span, v_span, 0U, 0U,
        u_flat_knots, v_flat_knots, u, v);
    const auto u_value = evaluate_tensor<2U, 3U>(
        *du, u_span - 1U, v_span, 1U, 0U,
        u_flat_knots, v_flat_knots, u, v);
    const auto v_value = evaluate_tensor<3U, 2U>(
        *dv, u_span, v_span - 1U, 0U, 1U,
        u_flat_knots, v_flat_knots, u, v);
    const auto uu_value = evaluate_tensor<1U, 3U>(
        *duu, u_span - 2U, v_span, 2U, 0U,
        u_flat_knots, v_flat_knots, u, v);
    const auto uv_value = evaluate_tensor<2U, 2U>(
        *duv, u_span - 1U, v_span - 1U, 1U, 1U,
        u_flat_knots, v_flat_knots, u, v);
    const auto vv_value = evaluate_tensor<3U, 1U>(
        *dvv, u_span, v_span - 2U, 0U, 2U,
        u_flat_knots, v_flat_knots, u, v);

    if (!value || !u_value || !v_value ||
        !uu_value || !uv_value || !vv_value) {
        return std::unexpected{SurfaceError::non_finite_result};
    }

    return HomogeneousJet{
        .value = *value,
        .u = *u_value,
        .v = *v_value,
        .uu = *uu_value,
        .uv = *uv_value,
        .vv = *vv_value,
    };
}

[[nodiscard]] std::expected<double, SurfaceError> to_double(
    const long double value) noexcept {
    constexpr long double maximum =
        static_cast<long double>(std::numeric_limits<double>::max());
    if (!std::isfinite(value) || value > maximum || value < -maximum) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    const double converted = static_cast<double>(value);
    if (!std::isfinite(converted)) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    return converted;
}

struct LongVector3 {
    long double x{};
    long double y{};
    long double z{};
};

[[nodiscard]] std::expected<LongVector3, SurfaceError> dehomogenize_value(
    const Homogeneous4& value) noexcept {
    if (!finite(value) || value.w <= 0.0L) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    const LongVector3 result{
        .x = value.x / value.w,
        .y = value.y / value.w,
        .z = value.z / value.w,
    };
    if (!std::isfinite(result.x) ||
        !std::isfinite(result.y) ||
        !std::isfinite(result.z)) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    return result;
}

[[nodiscard]] std::expected<LongVector3, SurfaceError> dehomogenize_first(
    const Homogeneous4& value,
    const Homogeneous4& derivative) noexcept {
    const auto point = dehomogenize_value(value);
    if (!point || !finite(derivative)) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    const LongVector3 result{
        .x = (derivative.x - point->x * derivative.w) / value.w,
        .y = (derivative.y - point->y * derivative.w) / value.w,
        .z = (derivative.z - point->z * derivative.w) / value.w,
    };
    if (!std::isfinite(result.x) ||
        !std::isfinite(result.y) ||
        !std::isfinite(result.z)) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    return result;
}

[[nodiscard]] std::expected<LongVector3, SurfaceError> dehomogenize_second(
    const Homogeneous4& value,
    const Homogeneous4& first,
    const Homogeneous4& second) noexcept {
    const auto point = dehomogenize_value(value);
    const auto first_euclidean = dehomogenize_first(value, first);
    if (!point || !first_euclidean || !finite(second)) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    const LongVector3 result{
        .x = (second.x -
              2.0L * first_euclidean->x * first.w -
              point->x * second.w) /
             value.w,
        .y = (second.y -
              2.0L * first_euclidean->y * first.w -
              point->y * second.w) /
             value.w,
        .z = (second.z -
              2.0L * first_euclidean->z * first.w -
              point->z * second.w) /
             value.w,
    };
    if (!std::isfinite(result.x) ||
        !std::isfinite(result.y) ||
        !std::isfinite(result.z)) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    return result;
}

[[nodiscard]] std::expected<LongVector3, SurfaceError>
dehomogenize_mixed_second(
    const Homogeneous4& value,
    const Homogeneous4& first_u,
    const Homogeneous4& first_v,
    const Homogeneous4& mixed) noexcept {
    const auto point = dehomogenize_value(value);
    const auto u = dehomogenize_first(value, first_u);
    const auto v = dehomogenize_first(value, first_v);
    if (!point || !u || !v || !finite(mixed)) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    const LongVector3 result{
        .x = (mixed.x -
              u->x * first_v.w -
              v->x * first_u.w -
              point->x * mixed.w) /
             value.w,
        .y = (mixed.y -
              u->y * first_v.w -
              v->y * first_u.w -
              point->y * mixed.w) /
             value.w,
        .z = (mixed.z -
              u->z * first_v.w -
              v->z * first_u.w -
              point->z * mixed.w) /
             value.w,
    };
    if (!std::isfinite(result.x) ||
        !std::isfinite(result.y) ||
        !std::isfinite(result.z)) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    return result;
}

[[nodiscard]] std::expected<Point3, SurfaceError> point_from_long(
    const LongVector3& value) noexcept {
    const auto x = to_double(value.x);
    const auto y = to_double(value.y);
    const auto z = to_double(value.z);
    if (!x || !y || !z) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    const auto point = Point3::make(*x, *y, *z);
    if (!point.has_value()) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    return *point;
}

[[nodiscard]] std::expected<Vector3, SurfaceError> vector_from_long(
    const LongVector3& value) noexcept {
    const auto x = to_double(value.x);
    const auto y = to_double(value.y);
    const auto z = to_double(value.z);
    if (!x || !y || !z) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    const auto vector = Vector3::make(*x, *y, *z);
    if (!vector.has_value()) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    return *vector;
}

[[nodiscard]] bool constant_points(
    const std::span<const Point3> points) noexcept {
    if (points.empty()) {
        return false;
    }
    return std::all_of(
        points.begin() + 1,
        points.end(),
        [&points](const Point3& point) { return point == points.front(); });
}

[[nodiscard]] std::expected<void, BicubicNURBSSurfaceConstructionError>
validate_direction(
    const std::size_t control_count,
    const std::span<const double> interior_knots,
    const std::span<const std::uint8_t> interior_multiplicities,
    const double lower,
    const double upper,
    const bool u_direction) noexcept {
    if (control_count < 4U) {
        return std::unexpected{
            u_direction
                ? BicubicNURBSSurfaceConstructionError::
                      insufficient_u_control_points
                : BicubicNURBSSurfaceConstructionError::
                      insufficient_v_control_points};
    }
    if (interior_multiplicities.size() != interior_knots.size()) {
        return std::unexpected{
            u_direction
                ? BicubicNURBSSurfaceConstructionError::
                      u_interior_multiplicity_count_mismatch
                : BicubicNURBSSurfaceConstructionError::
                      v_interior_multiplicity_count_mismatch};
    }

    std::size_t flat_interior_count = 0U;
    for (const std::uint8_t multiplicity : interior_multiplicities) {
        if (multiplicity != 1U && multiplicity != 2U) {
            return std::unexpected{
                u_direction
                    ? BicubicNURBSSurfaceConstructionError::
                          unsupported_u_interior_multiplicity
                    : BicubicNURBSSurfaceConstructionError::
                          unsupported_v_interior_multiplicity};
        }
        flat_interior_count += static_cast<std::size_t>(multiplicity);
    }
    if (control_count != flat_interior_count + 4U) {
        return std::unexpected{
            u_direction
                ? BicubicNURBSSurfaceConstructionError::
                      u_interior_knot_count_mismatch
                : BicubicNURBSSurfaceConstructionError::
                      v_interior_knot_count_mismatch};
    }

    if (!std::isfinite(lower)) {
        return std::unexpected{
            u_direction
                ? BicubicNURBSSurfaceConstructionError::
                      non_finite_u_lower_knot
                : BicubicNURBSSurfaceConstructionError::
                      non_finite_v_lower_knot};
    }
    if (!std::isfinite(upper)) {
        return std::unexpected{
            u_direction
                ? BicubicNURBSSurfaceConstructionError::
                      non_finite_u_upper_knot
                : BicubicNURBSSurfaceConstructionError::
                      non_finite_v_upper_knot};
    }
    if (!(lower < upper)) {
        return std::unexpected{
            u_direction
                ? BicubicNURBSSurfaceConstructionError::
                      non_strict_u_knot_order
                : BicubicNURBSSurfaceConstructionError::
                      non_strict_v_knot_order};
    }

    double previous = lower;
    for (const double knot : interior_knots) {
        if (!std::isfinite(knot)) {
            return std::unexpected{
                u_direction
                    ? BicubicNURBSSurfaceConstructionError::
                          non_finite_u_interior_knot
                    : BicubicNURBSSurfaceConstructionError::
                          non_finite_v_interior_knot};
        }
        if (!(previous < knot && knot < upper)) {
            return std::unexpected{
                u_direction
                    ? BicubicNURBSSurfaceConstructionError::
                          non_strict_u_knot_order
                    : BicubicNURBSSurfaceConstructionError::
                          non_strict_v_knot_order};
        }
        previous = knot;
    }
    return {};
}

[[nodiscard]] std::vector<double> build_flat_knots(
    const std::span<const double> interior_knots,
    const std::span<const std::uint8_t> interior_multiplicities,
    const double lower,
    const double upper) {
    std::size_t flat_interior_count = 0U;
    for (const std::uint8_t multiplicity : interior_multiplicities) {
        flat_interior_count += static_cast<std::size_t>(multiplicity);
    }

    std::vector<double> result;
    result.reserve(flat_interior_count + 8U);
    for (int index = 0; index < 4; ++index) {
        result.push_back(lower);
    }
    for (std::size_t index = 0; index < interior_knots.size(); ++index) {
        for (std::uint8_t repeat = 0U;
             repeat < interior_multiplicities[index];
             ++repeat) {
            result.push_back(interior_knots[index]);
        }
    }
    for (int index = 0; index < 4; ++index) {
        result.push_back(upper);
    }
    return result;
}

[[nodiscard]] bool is_double_knot(
    const std::span<const double> interior_knots,
    const std::span<const std::uint8_t> interior_multiplicities,
    const double parameter) noexcept {
    const auto iterator =
        std::lower_bound(interior_knots.begin(), interior_knots.end(), parameter);
    if (iterator == interior_knots.end() || *iterator != parameter) {
        return false;
    }
    const std::size_t index =
        static_cast<std::size_t>(iterator - interior_knots.begin());
    return interior_multiplicities[index] == 2U;
}

[[nodiscard]] std::vector<double> reflected_knots(
    const std::span<const double> knots,
    const CurveParameterDomain& domain) {
    std::vector<double> result;
    result.reserve(knots.size());
    for (auto iterator = knots.rbegin(); iterator != knots.rend(); ++iterator) {
        const auto mapped = reversed_parameter(domain, *iterator);
        result.push_back(*mapped);
    }
    return result;
}

} // namespace

BicubicNURBSSurface3::BicubicNURBSSurface3(
    std::vector<Point3> control_points,
    std::vector<double> weights,
    const std::size_t u_control_count,
    const std::size_t v_control_count,
    std::vector<double> u_interior_knots,
    std::vector<double> v_interior_knots,
    std::vector<std::uint8_t> u_interior_multiplicities,
    std::vector<std::uint8_t> v_interior_multiplicities,
    std::vector<double> u_flat_knots,
    std::vector<double> v_flat_knots,
    const double u_lower_knot,
    const double u_upper_knot,
    const double v_lower_knot,
    const double v_upper_knot,
    const bool constant)
    : control_points_(std::move(control_points)),
      weights_(std::move(weights)),
      u_control_count_(u_control_count),
      v_control_count_(v_control_count),
      u_interior_knots_(std::move(u_interior_knots)),
      v_interior_knots_(std::move(v_interior_knots)),
      u_interior_multiplicities_(std::move(u_interior_multiplicities)),
      v_interior_multiplicities_(std::move(v_interior_multiplicities)),
      u_flat_knots_(std::move(u_flat_knots)),
      v_flat_knots_(std::move(v_flat_knots)),
      u_lower_knot_(u_lower_knot),
      u_upper_knot_(u_upper_knot),
      v_lower_knot_(v_lower_knot),
      v_upper_knot_(v_upper_knot),
      constant_(constant) {}

std::expected<BicubicNURBSSurface3, BicubicNURBSSurfaceConstructionError>
BicubicNURBSSurface3::make(
    std::vector<Point3> control_points,
    std::vector<double> weights,
    const std::size_t u_control_count,
    const std::size_t v_control_count,
    std::vector<double> u_interior_knots,
    std::vector<double> v_interior_knots,
    const double u_lower_knot,
    const double u_upper_knot,
    const double v_lower_knot,
    const double v_upper_knot) {
    std::vector<std::uint8_t> u_multiplicities(
        u_interior_knots.size(), 1U);
    std::vector<std::uint8_t> v_multiplicities(
        v_interior_knots.size(), 1U);
    return make(
        std::move(control_points),
        std::move(weights),
        u_control_count,
        v_control_count,
        std::move(u_interior_knots),
        std::move(v_interior_knots),
        std::move(u_multiplicities),
        std::move(v_multiplicities),
        u_lower_knot,
        u_upper_knot,
        v_lower_knot,
        v_upper_knot);
}

std::expected<BicubicNURBSSurface3, BicubicNURBSSurfaceConstructionError>
BicubicNURBSSurface3::make(
    std::vector<Point3> control_points,
    std::vector<double> weights,
    const std::size_t u_control_count,
    const std::size_t v_control_count,
    std::vector<double> u_interior_knots,
    std::vector<double> v_interior_knots,
    std::vector<std::uint8_t> u_interior_multiplicities,
    std::vector<std::uint8_t> v_interior_multiplicities,
    const double u_lower_knot,
    const double u_upper_knot,
    const double v_lower_knot,
    const double v_upper_knot) {
    const auto valid_u = validate_direction(
        u_control_count,
        u_interior_knots,
        u_interior_multiplicities,
        u_lower_knot,
        u_upper_knot,
        true);
    if (!valid_u.has_value()) {
        return std::unexpected{valid_u.error()};
    }
    const auto valid_v = validate_direction(
        v_control_count,
        v_interior_knots,
        v_interior_multiplicities,
        v_lower_knot,
        v_upper_knot,
        false);
    if (!valid_v.has_value()) {
        return std::unexpected{valid_v.error()};
    }

    if (u_control_count >
        std::numeric_limits<std::size_t>::max() / v_control_count) {
        return std::unexpected{
            BicubicNURBSSurfaceConstructionError::control_net_size_mismatch};
    }
    const std::size_t expected_size = u_control_count * v_control_count;
    if (control_points.size() != expected_size) {
        return std::unexpected{
            BicubicNURBSSurfaceConstructionError::control_net_size_mismatch};
    }
    if (weights.size() != control_points.size()) {
        return std::unexpected{
            BicubicNURBSSurfaceConstructionError::
                control_weight_count_mismatch};
    }

    for (const double weight : weights) {
        if (!std::isfinite(weight)) {
            return std::unexpected{
                BicubicNURBSSurfaceConstructionError::non_finite_weight};
        }
        if (weight <= 0.0) {
            return std::unexpected{
                BicubicNURBSSurfaceConstructionError::non_positive_weight};
        }
    }

    auto u_flat_knots = build_flat_knots(
        u_interior_knots,
        u_interior_multiplicities,
        u_lower_knot,
        u_upper_knot);
    auto v_flat_knots = build_flat_knots(
        v_interior_knots,
        v_interior_multiplicities,
        v_lower_knot,
        v_upper_knot);
    const bool constant = constant_points(control_points);

    return BicubicNURBSSurface3{
        std::move(control_points),
        std::move(weights),
        u_control_count,
        v_control_count,
        std::move(u_interior_knots),
        std::move(v_interior_knots),
        std::move(u_interior_multiplicities),
        std::move(v_interior_multiplicities),
        std::move(u_flat_knots),
        std::move(v_flat_knots),
        u_lower_knot,
        u_upper_knot,
        v_lower_knot,
        v_upper_knot,
        constant};
}

std::span<const Point3>
BicubicNURBSSurface3::control_points() const noexcept {
    return std::span<const Point3>{control_points_};
}

std::span<const double>
BicubicNURBSSurface3::weights() const noexcept {
    return std::span<const double>{weights_};
}

std::size_t BicubicNURBSSurface3::u_control_count() const noexcept {
    return u_control_count_;
}

std::size_t BicubicNURBSSurface3::v_control_count() const noexcept {
    return v_control_count_;
}

std::span<const double>
BicubicNURBSSurface3::u_interior_knots() const noexcept {
    return std::span<const double>{u_interior_knots_};
}

std::span<const double>
BicubicNURBSSurface3::v_interior_knots() const noexcept {
    return std::span<const double>{v_interior_knots_};
}

std::span<const std::uint8_t>
BicubicNURBSSurface3::u_interior_multiplicities() const noexcept {
    return std::span<const std::uint8_t>{u_interior_multiplicities_};
}

std::span<const std::uint8_t>
BicubicNURBSSurface3::v_interior_multiplicities() const noexcept {
    return std::span<const std::uint8_t>{v_interior_multiplicities_};
}

std::size_t BicubicNURBSSurface3::u_span_count() const noexcept {
    return u_interior_knots_.size() + 1U;
}

std::size_t BicubicNURBSSurface3::v_span_count() const noexcept {
    return v_interior_knots_.size() + 1U;
}

SurfaceParameterDomain
BicubicNURBSSurface3::parameter_domain() const noexcept {
    return SurfaceParameterDomain{
        .u = *CurveParameterDomain::make(u_lower_knot_, u_upper_knot_),
        .v = *CurveParameterDomain::make(v_lower_knot_, v_upper_knot_),
    };
}

std::expected<Point3, SurfaceError>
BicubicNURBSSurface3::evaluate(
    const double u,
    const double v) const noexcept {
    const auto valid = validate_parameters(parameter_domain(), u, v);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }

    if (u == u_lower_knot_ && v == v_lower_knot_) {
        return control_points_.front();
    }
    if (u == u_lower_knot_ && v == v_upper_knot_) {
        return control_points_[v_control_count_ - 1U];
    }
    if (u == u_upper_knot_ && v == v_lower_knot_) {
        return control_points_[(u_control_count_ - 1U) * v_control_count_];
    }
    if (u == u_upper_knot_ && v == v_upper_knot_) {
        return control_points_.back();
    }
    if (constant_) {
        return control_points_.front();
    }

    const auto homogeneous = evaluate_value(
        control_points(),
        weights(),
        v_control_count_,
        u_flat_knots_,
        v_flat_knots_,
        u,
        v);
    if (!homogeneous.has_value()) {
        return std::unexpected{homogeneous.error()};
    }
    const auto point = dehomogenize_value(*homogeneous);
    if (!point.has_value()) {
        return std::unexpected{point.error()};
    }
    return point_from_long(*point);
}

std::expected<SurfaceFirstDerivatives3, SurfaceError>
BicubicNURBSSurface3::first_derivatives(
    const double u,
    const double v) const noexcept {
    const auto valid = validate_parameters(parameter_domain(), u, v);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }
    if (constant_) {
        const auto zero = Vector3::make(0.0, 0.0, 0.0);
        return SurfaceFirstDerivatives3{.u = *zero, .v = *zero};
    }

    const auto jet = evaluate_first_jet(
        control_points(),
        weights(),
        v_control_count_,
        u_flat_knots_,
        v_flat_knots_,
        u,
        v);
    if (!jet.has_value()) {
        return std::unexpected{jet.error()};
    }
    const auto du = dehomogenize_first(jet->value, jet->u);
    const auto dv = dehomogenize_first(jet->value, jet->v);
    if (!du || !dv) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    const auto u_vector = vector_from_long(*du);
    const auto v_vector = vector_from_long(*dv);
    if (!u_vector || !v_vector) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    return SurfaceFirstDerivatives3{.u = *u_vector, .v = *v_vector};
}

std::expected<SurfaceSecondDerivatives3, SurfaceError>
BicubicNURBSSurface3::second_derivatives(
    const double u,
    const double v) const noexcept {
    const auto valid = validate_parameters(parameter_domain(), u, v);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }
    if (is_double_knot(
            u_interior_knots_, u_interior_multiplicities_, u) ||
        is_double_knot(
            v_interior_knots_, v_interior_multiplicities_, v)) {
        return std::unexpected{SurfaceError::insufficient_continuity};
    }
    if (constant_) {
        const auto zero = Vector3::make(0.0, 0.0, 0.0);
        return SurfaceSecondDerivatives3{
            .uu = *zero,
            .uv = *zero,
            .vv = *zero,
        };
    }

    const auto jet = evaluate_jet(
        control_points(),
        weights(),
        v_control_count_,
        u_flat_knots_,
        v_flat_knots_,
        u,
        v);
    if (!jet.has_value()) {
        return std::unexpected{jet.error()};
    }

    const auto uu = dehomogenize_second(jet->value, jet->u, jet->uu);
    const auto uv = dehomogenize_mixed_second(
        jet->value, jet->u, jet->v, jet->uv);
    const auto vv = dehomogenize_second(jet->value, jet->v, jet->vv);
    if (!uu || !uv || !vv) {
        return std::unexpected{SurfaceError::non_finite_result};
    }

    const auto uu_vector = vector_from_long(*uu);
    const auto uv_vector = vector_from_long(*uv);
    const auto vv_vector = vector_from_long(*vv);
    if (!uu_vector || !uv_vector || !vv_vector) {
        return std::unexpected{SurfaceError::non_finite_result};
    }

    return SurfaceSecondDerivatives3{
        .uu = *uu_vector,
        .uv = *uv_vector,
        .vv = *vv_vector,
    };
}

BicubicNURBSSurface3 BicubicNURBSSurface3::u_reversed() const {
    std::vector<Point3> points(control_points_.size(), control_points_.front());
    std::vector<double> weights(weights_.size());

    for (std::size_t u_index = 0; u_index < u_control_count_; ++u_index) {
        for (std::size_t v_index = 0; v_index < v_control_count_; ++v_index) {
            const std::size_t destination =
                u_index * v_control_count_ + v_index;
            const std::size_t source =
                (u_control_count_ - 1U - u_index) * v_control_count_ + v_index;
            points[destination] = control_points_[source];
            weights[destination] = weights_[source];
        }
    }

    const auto domain = parameter_domain();
    auto u_knots = reflected_knots(u_interior_knots_, domain.u);
    std::vector<std::uint8_t> u_multiplicities(
        u_interior_multiplicities_.rbegin(),
        u_interior_multiplicities_.rend());
    auto u_flat = build_flat_knots(
        u_knots, u_multiplicities, u_lower_knot_, u_upper_knot_);

    return BicubicNURBSSurface3{
        std::move(points),
        std::move(weights),
        u_control_count_,
        v_control_count_,
        std::move(u_knots),
        v_interior_knots_,
        std::move(u_multiplicities),
        v_interior_multiplicities_,
        std::move(u_flat),
        v_flat_knots_,
        u_lower_knot_,
        u_upper_knot_,
        v_lower_knot_,
        v_upper_knot_,
        constant_};
}

BicubicNURBSSurface3 BicubicNURBSSurface3::v_reversed() const {
    std::vector<Point3> points(control_points_.size(), control_points_.front());
    std::vector<double> weights(weights_.size());

    for (std::size_t u_index = 0; u_index < u_control_count_; ++u_index) {
        for (std::size_t v_index = 0; v_index < v_control_count_; ++v_index) {
            const std::size_t destination =
                u_index * v_control_count_ + v_index;
            const std::size_t source =
                u_index * v_control_count_ +
                (v_control_count_ - 1U - v_index);
            points[destination] = control_points_[source];
            weights[destination] = weights_[source];
        }
    }

    const auto domain = parameter_domain();
    auto v_knots = reflected_knots(v_interior_knots_, domain.v);
    std::vector<std::uint8_t> v_multiplicities(
        v_interior_multiplicities_.rbegin(),
        v_interior_multiplicities_.rend());
    auto v_flat = build_flat_knots(
        v_knots, v_multiplicities, v_lower_knot_, v_upper_knot_);

    return BicubicNURBSSurface3{
        std::move(points),
        std::move(weights),
        u_control_count_,
        v_control_count_,
        u_interior_knots_,
        std::move(v_knots),
        u_interior_multiplicities_,
        std::move(v_multiplicities),
        u_flat_knots_,
        std::move(v_flat),
        u_lower_knot_,
        u_upper_knot_,
        v_lower_knot_,
        v_upper_knot_,
        constant_};
}

} // namespace apmesh::core
