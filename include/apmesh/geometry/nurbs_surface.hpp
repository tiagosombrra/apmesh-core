#pragma once

#include "apmesh/geometry/parametric_surface.hpp"

#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <vector>

namespace apmesh::core {

enum class BicubicNURBSSurfaceConstructionError {
    insufficient_u_control_points,
    insufficient_v_control_points,
    control_net_size_mismatch,
    control_weight_count_mismatch,
    u_interior_knot_count_mismatch,
    v_interior_knot_count_mismatch,
    u_interior_multiplicity_count_mismatch,
    v_interior_multiplicity_count_mismatch,
    unsupported_u_interior_multiplicity,
    unsupported_v_interior_multiplicity,
    non_finite_u_lower_knot,
    non_finite_u_interior_knot,
    non_finite_u_upper_knot,
    non_strict_u_knot_order,
    non_finite_v_lower_knot,
    non_finite_v_interior_knot,
    non_finite_v_upper_knot,
    non_strict_v_knot_order,
    non_finite_weight,
    non_positive_weight,
};

class BicubicNURBSSurface3 {
public:
    [[nodiscard]] static std::expected<
        BicubicNURBSSurface3,
        BicubicNURBSSurfaceConstructionError>
    make(
        std::vector<Point3> control_points,
        std::vector<double> weights,
        std::size_t u_control_count,
        std::size_t v_control_count,
        std::vector<double> u_interior_knots,
        std::vector<double> v_interior_knots,
        double u_lower_knot,
        double u_upper_knot,
        double v_lower_knot,
        double v_upper_knot);

    [[nodiscard]] static std::expected<
        BicubicNURBSSurface3,
        BicubicNURBSSurfaceConstructionError>
    make(
        std::vector<Point3> control_points,
        std::vector<double> weights,
        std::size_t u_control_count,
        std::size_t v_control_count,
        std::vector<double> u_interior_knots,
        std::vector<double> v_interior_knots,
        std::vector<std::uint8_t> u_interior_multiplicities,
        std::vector<std::uint8_t> v_interior_multiplicities,
        double u_lower_knot,
        double u_upper_knot,
        double v_lower_knot,
        double v_upper_knot);

    [[nodiscard]] std::span<const Point3> control_points() const noexcept;
    [[nodiscard]] std::span<const double> weights() const noexcept;
    [[nodiscard]] std::size_t u_control_count() const noexcept;
    [[nodiscard]] std::size_t v_control_count() const noexcept;
    [[nodiscard]] std::span<const double> u_interior_knots() const noexcept;
    [[nodiscard]] std::span<const double> v_interior_knots() const noexcept;
    [[nodiscard]] std::span<const std::uint8_t>
    u_interior_multiplicities() const noexcept;
    [[nodiscard]] std::span<const std::uint8_t>
    v_interior_multiplicities() const noexcept;
    [[nodiscard]] std::size_t u_span_count() const noexcept;
    [[nodiscard]] std::size_t v_span_count() const noexcept;

    [[nodiscard]] SurfaceParameterDomain parameter_domain() const noexcept;
    [[nodiscard]] std::expected<Point3, SurfaceError>
    evaluate(double u, double v) const noexcept;
    [[nodiscard]] std::expected<SurfaceFirstDerivatives3, SurfaceError>
    first_derivatives(double u, double v) const noexcept;
    [[nodiscard]] std::expected<SurfaceSecondDerivatives3, SurfaceError>
    second_derivatives(double u, double v) const noexcept;

    [[nodiscard]] BicubicNURBSSurface3 u_reversed() const;
    [[nodiscard]] BicubicNURBSSurface3 v_reversed() const;

    [[nodiscard]] bool operator==(
        const BicubicNURBSSurface3&) const = default;

private:
    BicubicNURBSSurface3(
        std::vector<Point3> control_points,
        std::vector<double> weights,
        std::size_t u_control_count,
        std::size_t v_control_count,
        std::vector<double> u_interior_knots,
        std::vector<double> v_interior_knots,
        std::vector<std::uint8_t> u_interior_multiplicities,
        std::vector<std::uint8_t> v_interior_multiplicities,
        std::vector<double> u_flat_knots,
        std::vector<double> v_flat_knots,
        double u_lower_knot,
        double u_upper_knot,
        double v_lower_knot,
        double v_upper_knot,
        bool constant);

    std::vector<Point3> control_points_;
    std::vector<double> weights_;
    std::size_t u_control_count_{};
    std::size_t v_control_count_{};
    std::vector<double> u_interior_knots_;
    std::vector<double> v_interior_knots_;
    std::vector<std::uint8_t> u_interior_multiplicities_;
    std::vector<std::uint8_t> v_interior_multiplicities_;
    std::vector<double> u_flat_knots_;
    std::vector<double> v_flat_knots_;
    double u_lower_knot_{};
    double u_upper_knot_{};
    double v_lower_knot_{};
    double v_upper_knot_{};
    bool constant_{};
};

} // namespace apmesh::core
