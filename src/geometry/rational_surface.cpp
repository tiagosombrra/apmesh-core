#include "apmesh/geometry/surface.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <expected>
#include <limits>
#include <utility>

namespace apmesh::core {
namespace {

struct Homogeneous4 {
    long double x{};
    long double y{};
    long double z{};
    long double w{};
};

struct LongVector3 {
    long double x{};
    long double y{};
    long double z{};
};

template <std::size_t UCount, std::size_t VCount>
using HomogeneousNet =
    std::array<std::array<Homogeneous4, VCount>, UCount>;

[[nodiscard]] SurfaceParameterDomain unit_square_domain() noexcept {
    return SurfaceParameterDomain{
        .u = *CurveParameterDomain::make(0.0, 1.0),
        .v = *CurveParameterDomain::make(0.0, 1.0),
    };
}

[[nodiscard]] std::expected<void, SurfaceError> validate_parameters(
    const double u,
    const double v) noexcept {
    if (!std::isfinite(u)) {
        return std::unexpected{SurfaceError::non_finite_u_parameter};
    }
    if (!std::isfinite(v)) {
        return std::unexpected{SurfaceError::non_finite_v_parameter};
    }
    if (u < 0.0 || u > 1.0) {
        return std::unexpected{SurfaceError::u_parameter_out_of_domain};
    }
    if (v < 0.0 || v > 1.0) {
        return std::unexpected{SurfaceError::v_parameter_out_of_domain};
    }
    return {};
}

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

[[nodiscard]] Homogeneous4 scaled_difference(
    const Homogeneous4& lhs,
    const Homogeneous4& rhs,
    const long double scale) noexcept {
    return Homogeneous4{
        .x = (lhs.x - rhs.x) * scale,
        .y = (lhs.y - rhs.y) * scale,
        .z = (lhs.z - rhs.z) * scale,
        .w = (lhs.w - rhs.w) * scale,
    };
}

template <std::size_t Count>
[[nodiscard]] std::expected<Homogeneous4, SurfaceError> evaluate_bezier(
    std::array<Homogeneous4, Count> work,
    const double parameter) noexcept {
    static_assert(Count >= 1U);
    const long double t = static_cast<long double>(parameter);
    for (std::size_t active = Count; active > 1U; --active) {
        for (std::size_t index = 0; index + 1U < active; ++index) {
            work[index] = interpolate(work[index], work[index + 1U], t);
            if (!finite(work[index])) {
                return std::unexpected{SurfaceError::non_finite_result};
            }
        }
    }
    return work[0];
}

template <std::size_t UCount, std::size_t VCount>
[[nodiscard]] std::expected<Homogeneous4, SurfaceError> evaluate_tensor(
    const HomogeneousNet<UCount, VCount>& controls,
    const double u,
    const double v) noexcept {
    std::array<Homogeneous4, UCount> rows{};
    for (std::size_t u_index = 0; u_index < UCount; ++u_index) {
        const auto value = evaluate_bezier<VCount>(controls[u_index], v);
        if (!value.has_value()) {
            return std::unexpected{value.error()};
        }
        rows[u_index] = *value;
    }
    return evaluate_bezier<UCount>(rows, u);
}

template <std::size_t UCount, std::size_t VCount>
[[nodiscard]] HomogeneousNet<UCount - 1U, VCount> differentiate_u(
    const HomogeneousNet<UCount, VCount>& controls) noexcept {
    static_assert(UCount >= 2U);
    HomogeneousNet<UCount - 1U, VCount> result{};
    constexpr long double degree =
        static_cast<long double>(UCount - 1U);
    for (std::size_t u_index = 0; u_index + 1U < UCount; ++u_index) {
        for (std::size_t v_index = 0; v_index < VCount; ++v_index) {
            result[u_index][v_index] = scaled_difference(
                controls[u_index + 1U][v_index],
                controls[u_index][v_index],
                degree);
        }
    }
    return result;
}

template <std::size_t UCount, std::size_t VCount>
[[nodiscard]] HomogeneousNet<UCount, VCount - 1U> differentiate_v(
    const HomogeneousNet<UCount, VCount>& controls) noexcept {
    static_assert(VCount >= 2U);
    HomogeneousNet<UCount, VCount - 1U> result{};
    constexpr long double degree =
        static_cast<long double>(VCount - 1U);
    for (std::size_t u_index = 0; u_index < UCount; ++u_index) {
        for (std::size_t v_index = 0; v_index + 1U < VCount; ++v_index) {
            result[u_index][v_index] = scaled_difference(
                controls[u_index][v_index + 1U],
                controls[u_index][v_index],
                degree);
        }
    }
    return result;
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
    if (!point.has_value() || !finite(derivative)) {
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

[[nodiscard]] bool constant_controls(
    const RationalBicubicBezierPatch3::ControlNet& controls) noexcept {
    const Point3& first = controls[0][0];
    for (const auto& row : controls) {
        for (const Point3& point : row) {
            if (!(point == first)) {
                return false;
            }
        }
    }
    return true;
}

[[nodiscard]] HomogeneousNet<4U, 4U> homogeneous_controls(
    const RationalBicubicBezierPatch3::ControlNet& controls,
    const RationalBicubicBezierPatch3::WeightNet& weights) noexcept {
    double maximum_weight = weights[0][0];
    for (const auto& row : weights) {
        for (const double weight : row) {
            maximum_weight = std::max(maximum_weight, weight);
        }
    }
    const long double scale = static_cast<long double>(maximum_weight);

    HomogeneousNet<4U, 4U> result{};
    for (std::size_t u_index = 0; u_index < 4U; ++u_index) {
        for (std::size_t v_index = 0; v_index < 4U; ++v_index) {
            const long double weight =
                static_cast<long double>(weights[u_index][v_index]) / scale;
            const Point3& point = controls[u_index][v_index];
            result[u_index][v_index] = Homogeneous4{
                .x = static_cast<long double>(point.x()) * weight,
                .y = static_cast<long double>(point.y()) * weight,
                .z = static_cast<long double>(point.z()) * weight,
                .w = weight,
            };
        }
    }
    return result;
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
    const RationalBicubicBezierPatch3::ControlNet& controls,
    const RationalBicubicBezierPatch3::WeightNet& weights,
    const double u,
    const double v) noexcept {
    const auto homogeneous = homogeneous_controls(controls, weights);
    const auto u_controls = differentiate_u(homogeneous);
    const auto v_controls = differentiate_v(homogeneous);
    const auto uu_controls = differentiate_u(u_controls);
    const auto uv_controls = differentiate_v(u_controls);
    const auto vv_controls = differentiate_v(v_controls);

    const auto value = evaluate_tensor(homogeneous, u, v);
    const auto du = evaluate_tensor(u_controls, u, v);
    const auto dv = evaluate_tensor(v_controls, u, v);
    const auto duu = evaluate_tensor(uu_controls, u, v);
    const auto duv = evaluate_tensor(uv_controls, u, v);
    const auto dvv = evaluate_tensor(vv_controls, u, v);
    if (!value || !du || !dv || !duu || !duv || !dvv) {
        return std::unexpected{SurfaceError::non_finite_result};
    }

    return HomogeneousJet{
        .value = *value,
        .u = *du,
        .v = *dv,
        .uu = *duu,
        .uv = *duv,
        .vv = *dvv,
    };
}

} // namespace

std::expected<RationalBicubicBezierPatch3, RationalSurfaceConstructionError>
RationalBicubicBezierPatch3::make(
    const ControlNet& control_points,
    const WeightNet& weights) noexcept {
    for (const auto& row : weights) {
        for (const double weight : row) {
            if (!std::isfinite(weight)) {
                return std::unexpected{
                    RationalSurfaceConstructionError::non_finite_weight};
            }
            if (weight <= 0.0) {
                return std::unexpected{
                    RationalSurfaceConstructionError::non_positive_weight};
            }
        }
    }
    return RationalBicubicBezierPatch3{
        control_points,
        weights,
        constant_controls(control_points)};
}

const RationalBicubicBezierPatch3::ControlNet&
RationalBicubicBezierPatch3::control_points() const noexcept {
    return control_points_;
}

const RationalBicubicBezierPatch3::WeightNet&
RationalBicubicBezierPatch3::weights() const noexcept {
    return weights_;
}

SurfaceParameterDomain
RationalBicubicBezierPatch3::parameter_domain() const noexcept {
    return unit_square_domain();
}

std::expected<Point3, SurfaceError>
RationalBicubicBezierPatch3::evaluate(
    const double u,
    const double v) const noexcept {
    const auto valid = validate_parameters(u, v);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }

    if (u == 0.0 && v == 0.0) {
        return control_points_[0][0];
    }
    if (u == 0.0 && v == 1.0) {
        return control_points_[0][3];
    }
    if (u == 1.0 && v == 0.0) {
        return control_points_[3][0];
    }
    if (u == 1.0 && v == 1.0) {
        return control_points_[3][3];
    }
    if (constant_) {
        return control_points_[0][0];
    }

    const auto jet = evaluate_jet(control_points_, weights_, u, v);
    if (!jet.has_value()) {
        return std::unexpected{jet.error()};
    }
    const auto value = dehomogenize_value(jet->value);
    if (!value.has_value()) {
        return std::unexpected{value.error()};
    }
    return point_from_long(*value);
}

std::expected<SurfaceFirstDerivatives3, SurfaceError>
RationalBicubicBezierPatch3::first_derivatives(
    const double u,
    const double v) const noexcept {
    const auto valid = validate_parameters(u, v);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }
    if (constant_) {
        const auto zero = Vector3::make(0.0, 0.0, 0.0);
        return SurfaceFirstDerivatives3{.u = *zero, .v = *zero};
    }

    const auto jet = evaluate_jet(control_points_, weights_, u, v);
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
RationalBicubicBezierPatch3::second_derivatives(
    const double u,
    const double v) const noexcept {
    const auto valid = validate_parameters(u, v);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }
    if (constant_) {
        const auto zero = Vector3::make(0.0, 0.0, 0.0);
        return SurfaceSecondDerivatives3{
            .uu = *zero,
            .uv = *zero,
            .vv = *zero,
        };
    }

    const auto jet = evaluate_jet(control_points_, weights_, u, v);
    if (!jet.has_value()) {
        return std::unexpected{jet.error()};
    }

    const auto uu =
        dehomogenize_second(jet->value, jet->u, jet->uu);
    const auto vv =
        dehomogenize_second(jet->value, jet->v, jet->vv);
    const auto uv = dehomogenize_mixed_second(
        jet->value, jet->u, jet->v, jet->uv);
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

RationalBicubicBezierPatch3
RationalBicubicBezierPatch3::u_reversed() const noexcept {
    ControlNet reversed_controls = control_points_;
    WeightNet reversed_weights = weights_;
    for (std::size_t u_index = 0; u_index < 4U; ++u_index) {
        reversed_controls[u_index] = control_points_[3U - u_index];
        reversed_weights[u_index] = weights_[3U - u_index];
    }
    return RationalBicubicBezierPatch3{
        reversed_controls, reversed_weights, constant_};
}

RationalBicubicBezierPatch3
RationalBicubicBezierPatch3::v_reversed() const noexcept {
    ControlNet reversed_controls = control_points_;
    WeightNet reversed_weights = weights_;
    for (std::size_t u_index = 0; u_index < 4U; ++u_index) {
        for (std::size_t v_index = 0; v_index < 4U; ++v_index) {
            reversed_controls[u_index][v_index] =
                control_points_[u_index][3U - v_index];
            reversed_weights[u_index][v_index] =
                weights_[u_index][3U - v_index];
        }
    }
    return RationalBicubicBezierPatch3{
        reversed_controls, reversed_weights, constant_};
}

} // namespace apmesh::core
