#pragma once

#include "apmesh/geometry/parametric_surface.hpp"

#include <array>
#include <expected>

namespace apmesh::core {

class BicubicBezierPatch3 {
public:
    using ControlRow = std::array<Point3, 4>;
    using ControlNet = std::array<ControlRow, 4>;

    constexpr explicit BicubicBezierPatch3(
        const ControlNet& control_points) noexcept
        : control_points_(control_points) {}

    [[nodiscard]] const ControlNet& control_points() const noexcept;
    [[nodiscard]] SurfaceParameterDomain parameter_domain() const noexcept;
    [[nodiscard]] std::expected<Point3, SurfaceError>
    evaluate(double u, double v) const noexcept;
    [[nodiscard]] std::expected<SurfaceFirstDerivatives3, SurfaceError>
    first_derivatives(double u, double v) const noexcept;
    [[nodiscard]] std::expected<SurfaceSecondDerivatives3, SurfaceError>
    second_derivatives(double u, double v) const noexcept;
    [[nodiscard]] BicubicBezierPatch3 u_reversed() const noexcept;
    [[nodiscard]] BicubicBezierPatch3 v_reversed() const noexcept;

    [[nodiscard]] bool operator==(
        const BicubicBezierPatch3&) const noexcept = default;

private:
    ControlNet control_points_;
};

} // namespace apmesh::core
