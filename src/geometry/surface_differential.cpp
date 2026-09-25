#include "apmesh/geometry/surface_differential.hpp"

#include <cmath>
#include <limits>

namespace apmesh::core {
namespace {

[[nodiscard]] std::expected<double, SurfaceDifferentialError>
finite_product(const double lhs, const double rhs) noexcept {
    if (lhs == 0.0 || rhs == 0.0) {
        return 0.0;
    }

    int lhs_exponent = 0;
    int rhs_exponent = 0;
    const double lhs_fraction = std::frexp(lhs, &lhs_exponent);
    const double rhs_fraction = std::frexp(rhs, &rhs_exponent);
    const long long exponent =
        static_cast<long long>(lhs_exponent) +
        static_cast<long long>(rhs_exponent);
    if (exponent > std::numeric_limits<int>::max() ||
        exponent < std::numeric_limits<int>::min()) {
        return std::unexpected{
            SurfaceDifferentialError::non_representable_result};
    }

    const double value = std::ldexp(
        lhs_fraction * rhs_fraction,
        static_cast<int>(exponent));
    if (!std::isfinite(value) || value == 0.0) {
        return std::unexpected{
            SurfaceDifferentialError::non_representable_result};
    }
    return value;
}

[[nodiscard]] std::expected<double, SurfaceDifferentialError>
finite_product(
    const double first,
    const double second,
    const double third) noexcept {
    if (first == 0.0 || second == 0.0 || third == 0.0) {
        return 0.0;
    }

    int first_exponent = 0;
    int second_exponent = 0;
    int third_exponent = 0;
    const double first_fraction = std::frexp(first, &first_exponent);
    const double second_fraction = std::frexp(second, &second_exponent);
    const double third_fraction = std::frexp(third, &third_exponent);
    const long long exponent =
        static_cast<long long>(first_exponent) +
        static_cast<long long>(second_exponent) +
        static_cast<long long>(third_exponent);
    if (exponent > std::numeric_limits<int>::max() ||
        exponent < std::numeric_limits<int>::min()) {
        return std::unexpected{
            SurfaceDifferentialError::non_representable_result};
    }

    const double value = std::ldexp(
        first_fraction * second_fraction * third_fraction,
        static_cast<int>(exponent));
    if (!std::isfinite(value) || value == 0.0) {
        return std::unexpected{
            SurfaceDifferentialError::non_representable_result};
    }
    return value;
}

[[nodiscard]] std::expected<Vector3, SurfaceDifferentialError>
scaled_unit_vector(
    const Vector3& vector,
    const double length) noexcept {
    const auto value = vector / length;
    if (!value.has_value()) {
        return std::unexpected{
            SurfaceDifferentialError::non_representable_result};
    }
    return *value;
}

} // namespace

std::expected<SurfaceMetricNormal3, SurfaceDifferentialError>
surface_metric_normal(
    const SurfaceFirstDerivatives3& derivatives) noexcept {
    const auto u_length = norm(derivatives.u);
    const auto v_length = norm(derivatives.v);
    if (!u_length.has_value() || !v_length.has_value() ||
        !std::isfinite(*u_length) || !std::isfinite(*v_length)) {
        return std::unexpected{
            SurfaceDifferentialError::non_representable_result};
    }
    if (*u_length == 0.0 || *v_length == 0.0) {
        return std::unexpected{
            SurfaceDifferentialError::singular_parameterization};
    }

    const auto unit_u = scaled_unit_vector(derivatives.u, *u_length);
    const auto unit_v = scaled_unit_vector(derivatives.v, *v_length);
    if (!unit_u.has_value() || !unit_v.has_value()) {
        return std::unexpected{
            SurfaceDifferentialError::non_representable_result};
    }

    const auto scaled_cross = cross(*unit_u, *unit_v);
    if (!scaled_cross.has_value()) {
        return std::unexpected{
            SurfaceDifferentialError::non_representable_result};
    }
    const auto sine = norm(*scaled_cross);
    if (!sine.has_value() || !std::isfinite(*sine)) {
        return std::unexpected{
            SurfaceDifferentialError::non_representable_result};
    }
    if (*sine == 0.0) {
        return std::unexpected{
            SurfaceDifferentialError::singular_parameterization};
    }

    const auto unit_normal = scaled_unit_vector(*scaled_cross, *sine);
    const auto cosine = dot(*unit_u, *unit_v);
    if (!unit_normal.has_value() || !cosine.has_value() ||
        !std::isfinite(*cosine)) {
        return std::unexpected{
            SurfaceDifferentialError::non_representable_result};
    }

    const auto e = finite_product(*u_length, *u_length);
    const auto f = finite_product(*u_length, *v_length, *cosine);
    const auto g = finite_product(*v_length, *v_length);
    const auto area_density =
        finite_product(*u_length, *v_length, *sine);
    if (!e.has_value() || !f.has_value() || !g.has_value() ||
        !area_density.has_value()) {
        return std::unexpected{
            SurfaceDifferentialError::non_representable_result};
    }

    return SurfaceMetricNormal3{
        .first_fundamental_form = SurfaceFirstFundamentalForm{
            .e = *e,
            .f = *f,
            .g = *g,
        },
        .area_density = *area_density,
        .unit_normal = *unit_normal,
    };
}

} // namespace apmesh::core
