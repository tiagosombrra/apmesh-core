#pragma once

#include "apmesh/core/geometry.hpp"
#include "apmesh/geometry/parametric_surface.hpp"

#include <expected>

namespace apmesh::core {

class BoundedPlaneSurface3 {
public:
    constexpr BoundedPlaneSurface3(
        const AxisPlacement3& placement,
        const CurveParameterDomain& u_domain,
        const CurveParameterDomain& v_domain) noexcept
        : placement_(placement),
          u_domain_(u_domain),
          v_domain_(v_domain) {}

    [[nodiscard]] const AxisPlacement3& axis_placement() const noexcept;
    [[nodiscard]] const CurveParameterDomain& u_domain() const noexcept;
    [[nodiscard]] const CurveParameterDomain& v_domain() const noexcept;
    [[nodiscard]] bool u_is_reversed() const noexcept;
    [[nodiscard]] bool v_is_reversed() const noexcept;

    [[nodiscard]] SurfaceParameterDomain parameter_domain() const noexcept;
    [[nodiscard]] std::expected<Point3, SurfaceError>
    evaluate(double u, double v) const noexcept;
    [[nodiscard]] std::expected<SurfaceFirstDerivatives3, SurfaceError>
    first_derivatives(double u, double v) const noexcept;
    [[nodiscard]] std::expected<SurfaceSecondDerivatives3, SurfaceError>
    second_derivatives(double u, double v) const noexcept;

    [[nodiscard]] BoundedPlaneSurface3 u_reversed() const noexcept;
    [[nodiscard]] BoundedPlaneSurface3 v_reversed() const noexcept;

    [[nodiscard]] bool operator==(
        const BoundedPlaneSurface3&) const noexcept = default;

private:
    constexpr BoundedPlaneSurface3(
        const AxisPlacement3& placement,
        const CurveParameterDomain& u_domain,
        const CurveParameterDomain& v_domain,
        const bool u_reversed,
        const bool v_reversed) noexcept
        : placement_(placement),
          u_domain_(u_domain),
          v_domain_(v_domain),
          u_reversed_(u_reversed),
          v_reversed_(v_reversed) {}

    AxisPlacement3 placement_;
    CurveParameterDomain u_domain_;
    CurveParameterDomain v_domain_;
    bool u_reversed_{};
    bool v_reversed_{};
};

} // namespace apmesh::core
