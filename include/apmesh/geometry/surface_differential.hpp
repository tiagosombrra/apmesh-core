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

namespace detail {

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

} // namespace apmesh::core
