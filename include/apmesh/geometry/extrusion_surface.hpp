#pragma once

#include "apmesh/geometry/curve.hpp"
#include "apmesh/geometry/parametric_surface.hpp"

#include <expected>

namespace apmesh::core {

enum class LinearExtrusionSurfaceConstructionError {
    non_finite_extruded_control,
};

class CubicBezierLinearExtrusionSurface3 {
public:
    [[nodiscard]] static std::expected<
        CubicBezierLinearExtrusionSurface3,
        LinearExtrusionSurfaceConstructionError>
    make(
        const CubicBezier3& basis_curve,
        const Vector3& extrusion_displacement) noexcept;

    [[nodiscard]] const CubicBezier3& basis_curve() const noexcept;
    [[nodiscard]] const CubicBezier3& end_curve() const noexcept;
    [[nodiscard]] const Vector3& extrusion_displacement() const noexcept;

    [[nodiscard]] SurfaceParameterDomain parameter_domain() const noexcept;
    [[nodiscard]] std::expected<Point3, SurfaceError>
    evaluate(double u, double v) const noexcept;
    [[nodiscard]] std::expected<SurfaceFirstDerivatives3, SurfaceError>
    first_derivatives(double u, double v) const noexcept;
    [[nodiscard]] std::expected<SurfaceSecondDerivatives3, SurfaceError>
    second_derivatives(double u, double v) const noexcept;

    [[nodiscard]] CubicBezierLinearExtrusionSurface3
    u_reversed() const noexcept;
    [[nodiscard]] CubicBezierLinearExtrusionSurface3
    v_reversed() const noexcept;

    [[nodiscard]] bool operator==(
        const CubicBezierLinearExtrusionSurface3&) const noexcept = default;

private:
    constexpr CubicBezierLinearExtrusionSurface3(
        const CubicBezier3& basis_curve,
        const CubicBezier3& end_curve,
        const Vector3& extrusion_displacement) noexcept
        : basis_curve_(basis_curve),
          end_curve_(end_curve),
          extrusion_displacement_(extrusion_displacement) {}

    CubicBezier3 basis_curve_;
    CubicBezier3 end_curve_;
    Vector3 extrusion_displacement_;
};

} // namespace apmesh::core
