#pragma once

#include "apmesh/core/geometry.hpp"
#include "apmesh/geometry/curve.hpp"
#include "apmesh/geometry/parametric_surface.hpp"

#include <expected>

namespace apmesh::core {

enum class RevolutionSurfaceConstructionError {
    non_finite_sweep_angle,
    zero_sweep_angle,
    full_or_multiple_revolution_not_admitted,
    non_finite_rotated_control,
};

class CubicBezierRevolutionSurface3 {
public:
    [[nodiscard]] static std::expected<
        CubicBezierRevolutionSurface3,
        RevolutionSurfaceConstructionError>
    make(
        const CubicBezier3& generatrix,
        const AxisPlacement3& axis,
        double sweep_angle) noexcept;

    [[nodiscard]] const CubicBezier3& start_curve() const noexcept;
    [[nodiscard]] const CubicBezier3& end_curve() const noexcept;
    [[nodiscard]] const AxisPlacement3& axis_placement() const noexcept;
    [[nodiscard]] double sweep_angle() const noexcept;

    [[nodiscard]] SurfaceParameterDomain parameter_domain() const noexcept;
    [[nodiscard]] std::expected<Point3, SurfaceError>
    evaluate(double u, double v) const noexcept;
    [[nodiscard]] std::expected<SurfaceFirstDerivatives3, SurfaceError>
    first_derivatives(double u, double v) const noexcept;
    [[nodiscard]] std::expected<SurfaceSecondDerivatives3, SurfaceError>
    second_derivatives(double u, double v) const noexcept;

    [[nodiscard]] CubicBezierRevolutionSurface3
    u_reversed() const noexcept;
    [[nodiscard]] CubicBezierRevolutionSurface3
    v_reversed() const noexcept;

    [[nodiscard]] bool operator==(
        const CubicBezierRevolutionSurface3&) const noexcept = default;

private:
    constexpr CubicBezierRevolutionSurface3(
        const CubicBezier3& start_curve,
        const CubicBezier3& end_curve,
        const AxisPlacement3& axis,
        const double sweep_angle) noexcept
        : start_curve_(start_curve),
          end_curve_(end_curve),
          axis_(axis),
          sweep_angle_(sweep_angle) {}

    CubicBezier3 start_curve_;
    CubicBezier3 end_curve_;
    AxisPlacement3 axis_;
    double sweep_angle_{};
};

} // namespace apmesh::core
