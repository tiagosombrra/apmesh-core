#include "apmesh/geometry/surface_differential.hpp"

#include <algorithm>
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

[[nodiscard]] std::expected<double, SurfaceDifferentialError>
representable_double(const long double value) noexcept {
    if (!std::isfinite(value)) {
        return std::unexpected{
            SurfaceDifferentialError::non_representable_result};
    }

    constexpr long double maximum =
        static_cast<long double>(std::numeric_limits<double>::max());
    if (value > maximum || value < -maximum) {
        return std::unexpected{
            SurfaceDifferentialError::non_representable_result};
    }

    const double converted = static_cast<double>(value);
    if (!std::isfinite(converted)) {
        return std::unexpected{
            SurfaceDifferentialError::non_representable_result};
    }
    if (value != 0.0L && converted == 0.0) {
        return std::unexpected{
            SurfaceDifferentialError::non_representable_result};
    }
    return converted;
}

[[nodiscard]] std::expected<double, SurfaceDifferentialError>
project_onto_unit_normal(
    const Vector3& vector,
    const Vector3& unit_normal) noexcept {
    return representable_double(
        static_cast<long double>(vector.x()) *
                static_cast<long double>(unit_normal.x()) +
        static_cast<long double>(vector.y()) *
                static_cast<long double>(unit_normal.y()) +
        static_cast<long double>(vector.z()) *
                static_cast<long double>(unit_normal.z()));
}


struct SymmetricPrincipalValues {
    long double maximum{};
    long double minimum{};
    bool is_umbilic{};
};

[[nodiscard]] std::expected<
    SymmetricPrincipalValues,
    SurfaceDifferentialError>
metric_whitened_principal_values(
    const SurfaceFirstFundamentalForm& first,
    const double area_density,
    const SurfaceSecondFundamentalForm& second) noexcept {
    const long double e0 = static_cast<long double>(first.e);
    const long double f0 = static_cast<long double>(first.f);
    const long double g0 = static_cast<long double>(first.g);
    const long double l0 = static_cast<long double>(second.l);
    const long double m0 = static_cast<long double>(second.m);
    const long double n0 = static_cast<long double>(second.n);
    const long double j0 = static_cast<long double>(area_density);

    if (!std::isfinite(e0) || !std::isfinite(f0) || !std::isfinite(g0) ||
        !std::isfinite(l0) || !std::isfinite(m0) || !std::isfinite(n0) ||
        !std::isfinite(j0)) {
        return std::unexpected{
            SurfaceDifferentialError::non_representable_result};
    }

    const long double metric_scale =
        std::max({std::abs(e0), std::abs(f0), std::abs(g0)});
    if (metric_scale == 0.0L || j0 <= 0.0L || e0 <= 0.0L || g0 <= 0.0L) {
        return std::unexpected{
            SurfaceDifferentialError::singular_parameterization};
    }

    const long double second_scale =
        std::max({std::abs(l0), std::abs(m0), std::abs(n0)});
    if (second_scale == 0.0L) {
        return SymmetricPrincipalValues{
            .maximum = 0.0L,
            .minimum = 0.0L,
            .is_umbilic = true,
        };
    }

    const long double e = e0 / metric_scale;
    const long double f = f0 / metric_scale;
    const long double g = g0 / metric_scale;
    const long double j = j0 / metric_scale;
    const long double l = l0 / second_scale;
    const long double m = m0 / second_scale;
    const long double n = n0 / second_scale;

    if (!std::isfinite(e) || !std::isfinite(f) || !std::isfinite(g) ||
        !std::isfinite(j) || e <= 0.0L || g <= 0.0L || j <= 0.0L) {
        return std::unexpected{
            SurfaceDifferentialError::non_representable_result};
    }

    const long double a = std::sqrt(e);
    const long double c_cholesky = j / a;
    if (!std::isfinite(a) || !std::isfinite(c_cholesky) ||
        a == 0.0L || c_cholesky == 0.0L) {
        return std::unexpected{
            SurfaceDifferentialError::non_representable_result};
    }

    const long double ratio = f / e;
    const long double whitened_00 = l / e;
    const long double whitened_01 =
        (m - ratio * l) / j;
    const long double whitened_11_numerator =
        std::fma(ratio, std::fma(ratio, l, -2.0L * m), n);
    const long double whitened_11 =
        whitened_11_numerator / (c_cholesky * c_cholesky);

    if (!std::isfinite(whitened_00) ||
        !std::isfinite(whitened_01) ||
        !std::isfinite(whitened_11)) {
        return std::unexpected{
            SurfaceDifferentialError::non_representable_result};
    }

    const bool is_umbilic =
        whitened_01 == 0.0L && whitened_00 == whitened_11;

    const long double operator_scale =
        std::max({
            std::abs(whitened_00),
            std::abs(whitened_01),
            std::abs(whitened_11),
        });
    if (operator_scale == 0.0L) {
        return SymmetricPrincipalValues{
            .maximum = 0.0L,
            .minimum = 0.0L,
            .is_umbilic = true,
        };
    }

    const long double a00 = whitened_00 / operator_scale;
    const long double a01 = whitened_01 / operator_scale;
    const long double a11 = whitened_11 / operator_scale;

    long double lambda_max = 0.0L;
    long double lambda_min = 0.0L;
    if (is_umbilic) {
        lambda_max = a00;
        lambda_min = a00;
    } else {
        const long double trace = a00 + a11;
        const long double gap = std::hypot(a00 - a11, 2.0L * a01);
        const long double determinant =
            std::fma(a00, a11, -(a01 * a01));

        if (trace >= 0.0L) {
            lambda_max = 0.5L * (trace + gap);
            lambda_min =
                lambda_max != 0.0L
                    ? determinant / lambda_max
                    : 0.5L * (trace - gap);
        } else {
            lambda_min = 0.5L * (trace - gap);
            lambda_max =
                lambda_min != 0.0L
                    ? determinant / lambda_min
                    : 0.5L * (trace + gap);
        }

        if (lambda_max < lambda_min) {
            std::swap(lambda_max, lambda_min);
        }
    }

    const long double curvature_scale =
        operator_scale * second_scale / metric_scale;
    const long double maximum = lambda_max * curvature_scale;
    const long double minimum = lambda_min * curvature_scale;
    if (!std::isfinite(maximum) || !std::isfinite(minimum)) {
        return std::unexpected{
            SurfaceDifferentialError::non_representable_result};
    }

    return SymmetricPrincipalValues{
        .maximum = maximum,
        .minimum = minimum,
        .is_umbilic = is_umbilic,
    };
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

namespace detail {

std::expected<SurfaceSecondOrderGeometry3, SurfaceDifferentialError>
surface_second_order_geometry(
    const SurfaceMetricNormal3& metric_normal,
    const SurfaceSecondDerivatives3& second_derivatives) noexcept {
    const auto l = project_onto_unit_normal(
        second_derivatives.uu, metric_normal.unit_normal);
    const auto m = project_onto_unit_normal(
        second_derivatives.uv, metric_normal.unit_normal);
    const auto n = project_onto_unit_normal(
        second_derivatives.vv, metric_normal.unit_normal);
    if (!l.has_value() || !m.has_value() || !n.has_value()) {
        return std::unexpected{
            SurfaceDifferentialError::non_representable_result};
    }

    const long double e = static_cast<long double>(
        metric_normal.first_fundamental_form.e);
    const long double f = static_cast<long double>(
        metric_normal.first_fundamental_form.f);
    const long double g = static_cast<long double>(
        metric_normal.first_fundamental_form.g);
    const long double ll = static_cast<long double>(*l);
    const long double mm = static_cast<long double>(*m);
    const long double nn = static_cast<long double>(*n);
    const long double area =
        static_cast<long double>(metric_normal.area_density);
    const long double denominator = area * area;

    if (!std::isfinite(denominator) || denominator == 0.0L) {
        return std::unexpected{
            SurfaceDifferentialError::non_representable_result};
    }

    const long double gaussian_numerator = ll * nn - mm * mm;
    const long double mean_numerator =
        e * nn - 2.0L * f * mm + g * ll;

    const auto gaussian = representable_double(
        gaussian_numerator / denominator);
    const auto mean = representable_double(
        mean_numerator / (2.0L * denominator));
    if (!gaussian.has_value() || !mean.has_value()) {
        return std::unexpected{
            SurfaceDifferentialError::non_representable_result};
    }

    return SurfaceSecondOrderGeometry3{
        .metric_normal = metric_normal,
        .second_fundamental_form = SurfaceSecondFundamentalForm{
            .l = *l,
            .m = *m,
            .n = *n,
        },
        .gaussian_curvature = *gaussian,
        .mean_curvature = *mean,
    };
}

} // namespace detail

std::expected<SurfaceSecondOrderGeometry3, SurfaceDifferentialError>
surface_second_order_geometry(
    const SurfaceFirstDerivatives3& first_derivatives,
    const SurfaceSecondDerivatives3& second_derivatives) noexcept {
    const auto metric_normal = surface_metric_normal(first_derivatives);
    if (!metric_normal.has_value()) {
        return std::unexpected{metric_normal.error()};
    }
    return detail::surface_second_order_geometry(
        *metric_normal, second_derivatives);
}

std::expected<SurfacePrincipalCurvatures, SurfaceDifferentialError>
surface_principal_curvatures(
    const SurfaceSecondOrderGeometry3& geometry) noexcept {
    const auto principal = metric_whitened_principal_values(
        geometry.metric_normal.first_fundamental_form,
        geometry.metric_normal.area_density,
        geometry.second_fundamental_form);
    if (!principal.has_value()) {
        return std::unexpected{principal.error()};
    }

    const auto maximum = representable_double(principal->maximum);
    const auto minimum = representable_double(principal->minimum);
    if (!maximum.has_value() || !minimum.has_value()) {
        return std::unexpected{
            SurfaceDifferentialError::non_representable_result};
    }

    return SurfacePrincipalCurvatures{
        .maximum_curvature = *maximum,
        .minimum_curvature = *minimum,
        .is_umbilic = principal->is_umbilic,
    };
}

} // namespace apmesh::core
