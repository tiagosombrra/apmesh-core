#pragma once

#include "apmesh/geometry/curve.hpp"
#include "apmesh/geometry/parametric_surface.hpp"

#include <expected>

namespace apmesh::core {

enum class CoonsSurfaceConstructionError {
    lower_left_corner_mismatch,
    lower_right_corner_mismatch,
    upper_left_corner_mismatch,
    upper_right_corner_mismatch,
};

class CubicBezierCoonsPatch3 {
public:
    [[nodiscard]] static std::expected<
        CubicBezierCoonsPatch3,
        CoonsSurfaceConstructionError>
    make(
        const CubicBezier3& bottom,
        const CubicBezier3& top,
        const CubicBezier3& left,
        const CubicBezier3& right) noexcept;

    [[nodiscard]] const CubicBezier3& bottom() const noexcept;
    [[nodiscard]] const CubicBezier3& top() const noexcept;
    [[nodiscard]] const CubicBezier3& left() const noexcept;
    [[nodiscard]] const CubicBezier3& right() const noexcept;

    [[nodiscard]] SurfaceParameterDomain parameter_domain() const noexcept;
    [[nodiscard]] std::expected<Point3, SurfaceError>
    evaluate(double u, double v) const noexcept;
    [[nodiscard]] std::expected<SurfaceFirstDerivatives3, SurfaceError>
    first_derivatives(double u, double v) const noexcept;
    [[nodiscard]] std::expected<SurfaceSecondDerivatives3, SurfaceError>
    second_derivatives(double u, double v) const noexcept;

    [[nodiscard]] CubicBezierCoonsPatch3 u_reversed() const noexcept;
    [[nodiscard]] CubicBezierCoonsPatch3 v_reversed() const noexcept;

    [[nodiscard]] bool operator==(
        const CubicBezierCoonsPatch3&) const noexcept = default;

private:
    constexpr CubicBezierCoonsPatch3(
        const CubicBezier3& bottom,
        const CubicBezier3& top,
        const CubicBezier3& left,
        const CubicBezier3& right) noexcept
        : bottom_(bottom),
          top_(top),
          left_(left),
          right_(right) {}

    CubicBezier3 bottom_;
    CubicBezier3 top_;
    CubicBezier3 left_;
    CubicBezier3 right_;
};

} // namespace apmesh::core
