#pragma once

#include "apmesh/geometry/parametric_curve.hpp"

#include <concepts>
#include <expected>

namespace apmesh::core {

enum class SurfaceError {
    non_finite_u_parameter,
    non_finite_v_parameter,
    u_parameter_out_of_domain,
    v_parameter_out_of_domain,
    non_finite_result,
};

struct SurfaceParameterDomain {
    CurveParameterDomain u;
    CurveParameterDomain v;

    [[nodiscard]] bool operator==(
        const SurfaceParameterDomain&) const noexcept = default;
};

struct SurfaceFirstDerivatives3 {
    Vector3 u;
    Vector3 v;

    [[nodiscard]] bool operator==(
        const SurfaceFirstDerivatives3&) const noexcept = default;
};

struct SurfaceSecondDerivatives3 {
    Vector3 uu;
    Vector3 uv;
    Vector3 vv;

    [[nodiscard]] bool operator==(
        const SurfaceSecondDerivatives3&) const noexcept = default;
};

template <typename Surface>
concept BoundedParametricSurface3 =
    requires(
        const Surface& surface,
        const double u,
        const double v) {
        {
            surface.parameter_domain()
        } -> std::same_as<SurfaceParameterDomain>;
        {
            surface.evaluate(u, v)
        } -> std::same_as<std::expected<Point3, SurfaceError>>;
        {
            surface.first_derivatives(u, v)
        } -> std::same_as<
            std::expected<SurfaceFirstDerivatives3, SurfaceError>>;
        {
            surface.second_derivatives(u, v)
        } -> std::same_as<
            std::expected<SurfaceSecondDerivatives3, SurfaceError>>;
    };

} // namespace apmesh::core
