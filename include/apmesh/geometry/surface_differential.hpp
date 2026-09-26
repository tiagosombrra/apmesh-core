#pragma once

#include "apmesh/geometry/parametric_surface.hpp"

#include <expected>

namespace apmesh::core {

enum class SurfaceDifferentialError {
    non_finite_u_parameter,
    non_finite_v_parameter,
    u_parameter_out_of_domain,
    v_parameter_out_of_domain,
    insufficient_continuity,
    singular_parameterization,
    non_representable_result,
};

struct SurfaceFirstFundamentalForm {
    double e{};
    double f{};
    double g{};

    [[nodiscard]] bool operator==(
        const SurfaceFirstFundamentalForm&) const noexcept = default;
};

struct SurfaceMetricNormal3 {
    SurfaceFirstFundamentalForm first_fundamental_form{};
    double area_density{};
    Vector3 unit_normal;

    [[nodiscard]] bool operator==(
        const SurfaceMetricNormal3&) const noexcept = default;
};

[[nodiscard]] std::expected<SurfaceMetricNormal3, SurfaceDifferentialError>
surface_metric_normal(
    const SurfaceFirstDerivatives3& derivatives) noexcept;

struct SurfaceSecondFundamentalForm {
    double l{};
    double m{};
    double n{};

    [[nodiscard]] bool operator==(
        const SurfaceSecondFundamentalForm&) const noexcept = default;
};

struct SurfaceSecondOrderGeometry3 {
    SurfaceMetricNormal3 metric_normal;
    SurfaceSecondFundamentalForm second_fundamental_form{};
    double gaussian_curvature{};
    double mean_curvature{};

    [[nodiscard]] bool operator==(
        const SurfaceSecondOrderGeometry3&) const noexcept = default;
};

[[nodiscard]] std::expected<
    SurfaceSecondOrderGeometry3,
    SurfaceDifferentialError>
surface_second_order_geometry(
    const SurfaceFirstDerivatives3& first_derivatives,
    const SurfaceSecondDerivatives3& second_derivatives) noexcept;


struct SurfacePrincipalCurvatures {
    double maximum_curvature{};
    double minimum_curvature{};
    bool is_umbilic{};

    [[nodiscard]] bool operator==(
        const SurfacePrincipalCurvatures&) const noexcept = default;
};

[[nodiscard]] std::expected<
    SurfacePrincipalCurvatures,
    SurfaceDifferentialError>
surface_principal_curvatures(
    const SurfaceSecondOrderGeometry3& geometry) noexcept;

namespace detail {

[[nodiscard]] std::expected<
    SurfaceSecondOrderGeometry3,
    SurfaceDifferentialError>
surface_second_order_geometry(
    const SurfaceMetricNormal3& metric_normal,
    const SurfaceSecondDerivatives3& second_derivatives) noexcept;

[[nodiscard]] constexpr SurfaceDifferentialError
surface_differential_error(const SurfaceError error) noexcept {
    switch (error) {
    case SurfaceError::non_finite_u_parameter:
        return SurfaceDifferentialError::non_finite_u_parameter;
    case SurfaceError::non_finite_v_parameter:
        return SurfaceDifferentialError::non_finite_v_parameter;
    case SurfaceError::u_parameter_out_of_domain:
        return SurfaceDifferentialError::u_parameter_out_of_domain;
    case SurfaceError::v_parameter_out_of_domain:
        return SurfaceDifferentialError::v_parameter_out_of_domain;
    case SurfaceError::insufficient_continuity:
        return SurfaceDifferentialError::insufficient_continuity;
    case SurfaceError::non_finite_result:
        return SurfaceDifferentialError::non_representable_result;
    }
    return SurfaceDifferentialError::non_representable_result;
}

} // namespace detail

template <BoundedParametricSurface3 Surface>
[[nodiscard]] std::expected<SurfaceMetricNormal3, SurfaceDifferentialError>
surface_metric_normal(
    const Surface& surface,
    const double u,
    const double v) {
    const auto derivatives = surface.first_derivatives(u, v);
    if (!derivatives.has_value()) {
        return std::unexpected{
            detail::surface_differential_error(derivatives.error())};
    }
    return surface_metric_normal(*derivatives);
}

template <BoundedParametricSurface3 Surface>
[[nodiscard]] std::expected<
    SurfaceSecondOrderGeometry3,
    SurfaceDifferentialError>
surface_second_order_geometry(
    const Surface& surface,
    const double u,
    const double v) {
    const auto first_derivatives = surface.first_derivatives(u, v);
    if (!first_derivatives.has_value()) {
        return std::unexpected{
            detail::surface_differential_error(first_derivatives.error())};
    }

    const auto metric_normal = surface_metric_normal(*first_derivatives);
    if (!metric_normal.has_value()) {
        return std::unexpected{metric_normal.error()};
    }

    const auto second_derivatives = surface.second_derivatives(u, v);
    if (!second_derivatives.has_value()) {
        return std::unexpected{
            detail::surface_differential_error(second_derivatives.error())};
    }

    return detail::surface_second_order_geometry(
        *metric_normal, *second_derivatives);
}

template <BoundedParametricSurface3 Surface>
[[nodiscard]] std::expected<
    SurfacePrincipalCurvatures,
    SurfaceDifferentialError>
surface_principal_curvatures(
    const Surface& surface,
    const double u,
    const double v) {
    const auto geometry = surface_second_order_geometry(surface, u, v);
    if (!geometry.has_value()) {
        return std::unexpected{geometry.error()};
    }
    return surface_principal_curvatures(*geometry);
}

} // namespace apmesh::core
