#pragma once

#include "apmesh/geometry/parametric_surface.hpp"

#include <algorithm>
#include <cmath>
#include <expected>
#include <utility>

namespace apmesh::core {

enum class RectangularTrimmedSurfaceConstructionError {
    non_finite_u_source_parameter,
    non_finite_u_target_parameter,
    non_finite_v_source_parameter,
    non_finite_v_target_parameter,
    u_source_parameter_out_of_domain,
    u_target_parameter_out_of_domain,
    v_source_parameter_out_of_domain,
    v_target_parameter_out_of_domain,
    zero_width_u_trim,
    zero_width_v_trim,
};

template <BoundedParametricSurface3 Surface>
class RectangularTrimmedSurface3 {
public:
    [[nodiscard]] static std::expected<
        RectangularTrimmedSurface3,
        RectangularTrimmedSurfaceConstructionError>
    make(
        Surface basis,
        const double u_source,
        const double u_target,
        const double v_source,
        const double v_target) {
        const auto validation = validate_construction(
            basis, u_source, u_target, v_source, v_target);
        if (!validation.has_value()) {
            return std::unexpected{validation.error()};
        }
        return RectangularTrimmedSurface3{
            std::move(basis),
            u_source,
            u_target,
            v_source,
            v_target};
    }

    [[nodiscard]] const Surface& basis_surface() const noexcept {
        return basis_;
    }

    [[nodiscard]] double u_source_parameter() const noexcept {
        return u_source_;
    }

    [[nodiscard]] double u_target_parameter() const noexcept {
        return u_target_;
    }

    [[nodiscard]] double v_source_parameter() const noexcept {
        return v_source_;
    }

    [[nodiscard]] double v_target_parameter() const noexcept {
        return v_target_;
    }

    [[nodiscard]] SurfaceParameterDomain parameter_domain() const noexcept {
        return SurfaceParameterDomain{
            .u = *CurveParameterDomain::make(
                std::min(u_source_, u_target_),
                std::max(u_source_, u_target_)),
            .v = *CurveParameterDomain::make(
                std::min(v_source_, v_target_),
                std::max(v_source_, v_target_)),
        };
    }

    [[nodiscard]] std::expected<Point3, SurfaceError>
    evaluate(const double u, const double v) const {
        const auto mapped = map_parameters(u, v);
        if (!mapped.has_value()) {
            return std::unexpected{mapped.error()};
        }
        return basis_.evaluate(mapped->first, mapped->second);
    }

    [[nodiscard]] std::expected<SurfaceFirstDerivatives3, SurfaceError>
    first_derivatives(const double u, const double v) const {
        const auto mapped = map_parameters(u, v);
        if (!mapped.has_value()) {
            return std::unexpected{mapped.error()};
        }

        const auto derivatives =
            basis_.first_derivatives(mapped->first, mapped->second);
        if (!derivatives.has_value()) {
            return std::unexpected{derivatives.error()};
        }

        return SurfaceFirstDerivatives3{
            .u = u_forward() ? derivatives->u : -derivatives->u,
            .v = v_forward() ? derivatives->v : -derivatives->v,
        };
    }

    [[nodiscard]] std::expected<SurfaceSecondDerivatives3, SurfaceError>
    second_derivatives(const double u, const double v) const {
        const auto mapped = map_parameters(u, v);
        if (!mapped.has_value()) {
            return std::unexpected{mapped.error()};
        }

        const auto derivatives =
            basis_.second_derivatives(mapped->first, mapped->second);
        if (!derivatives.has_value()) {
            return std::unexpected{derivatives.error()};
        }

        return SurfaceSecondDerivatives3{
            .uu = derivatives->uu,
            .uv = u_forward() == v_forward()
                      ? derivatives->uv
                      : -derivatives->uv,
            .vv = derivatives->vv,
        };
    }

    [[nodiscard]] RectangularTrimmedSurface3 u_reversed() const {
        return RectangularTrimmedSurface3{
            basis_, u_target_, u_source_, v_source_, v_target_};
    }

    [[nodiscard]] RectangularTrimmedSurface3 v_reversed() const {
        return RectangularTrimmedSurface3{
            basis_, u_source_, u_target_, v_target_, v_source_};
    }

    [[nodiscard]] bool operator==(
        const RectangularTrimmedSurface3&) const = default;

private:
    RectangularTrimmedSurface3(
        Surface basis,
        const double u_source,
        const double u_target,
        const double v_source,
        const double v_target)
        : basis_(std::move(basis)),
          u_source_(u_source),
          u_target_(u_target),
          v_source_(v_source),
          v_target_(v_target) {}

    [[nodiscard]] bool u_forward() const noexcept {
        return u_source_ < u_target_;
    }

    [[nodiscard]] bool v_forward() const noexcept {
        return v_source_ < v_target_;
    }

    [[nodiscard]] static std::expected<
        void,
        RectangularTrimmedSurfaceConstructionError>
    validate_construction(
        const Surface& basis,
        const double u_source,
        const double u_target,
        const double v_source,
        const double v_target) {
        if (!std::isfinite(u_source)) {
            return std::unexpected{
                RectangularTrimmedSurfaceConstructionError::
                    non_finite_u_source_parameter};
        }
        if (!std::isfinite(u_target)) {
            return std::unexpected{
                RectangularTrimmedSurfaceConstructionError::
                    non_finite_u_target_parameter};
        }
        if (!std::isfinite(v_source)) {
            return std::unexpected{
                RectangularTrimmedSurfaceConstructionError::
                    non_finite_v_source_parameter};
        }
        if (!std::isfinite(v_target)) {
            return std::unexpected{
                RectangularTrimmedSurfaceConstructionError::
                    non_finite_v_target_parameter};
        }

        const auto domain = basis.parameter_domain();

        const auto u_source_contained = domain.u.contains(u_source);
        if (!u_source_contained.has_value() || !*u_source_contained) {
            return std::unexpected{
                RectangularTrimmedSurfaceConstructionError::
                    u_source_parameter_out_of_domain};
        }

        const auto u_target_contained = domain.u.contains(u_target);
        if (!u_target_contained.has_value() || !*u_target_contained) {
            return std::unexpected{
                RectangularTrimmedSurfaceConstructionError::
                    u_target_parameter_out_of_domain};
        }

        const auto v_source_contained = domain.v.contains(v_source);
        if (!v_source_contained.has_value() || !*v_source_contained) {
            return std::unexpected{
                RectangularTrimmedSurfaceConstructionError::
                    v_source_parameter_out_of_domain};
        }

        const auto v_target_contained = domain.v.contains(v_target);
        if (!v_target_contained.has_value() || !*v_target_contained) {
            return std::unexpected{
                RectangularTrimmedSurfaceConstructionError::
                    v_target_parameter_out_of_domain};
        }

        if (u_source == u_target) {
            return std::unexpected{
                RectangularTrimmedSurfaceConstructionError::
                    zero_width_u_trim};
        }
        if (v_source == v_target) {
            return std::unexpected{
                RectangularTrimmedSurfaceConstructionError::
                    zero_width_v_trim};
        }
        return {};
    }

    [[nodiscard]] std::expected<double, SurfaceError>
    map_u(const double u) const noexcept {
        if (!std::isfinite(u)) {
            return std::unexpected{SurfaceError::non_finite_u_parameter};
        }
        const auto domain = parameter_domain().u;
        const auto contained = domain.contains(u);
        if (!contained.has_value() || !*contained) {
            return std::unexpected{SurfaceError::u_parameter_out_of_domain};
        }
        if (u_forward()) {
            return u;
        }
        const auto mapped = reversed_parameter(domain, u);
        if (!mapped.has_value()) {
            return std::unexpected{SurfaceError::non_finite_result};
        }
        return *mapped;
    }

    [[nodiscard]] std::expected<double, SurfaceError>
    map_v(const double v) const noexcept {
        if (!std::isfinite(v)) {
            return std::unexpected{SurfaceError::non_finite_v_parameter};
        }
        const auto domain = parameter_domain().v;
        const auto contained = domain.contains(v);
        if (!contained.has_value() || !*contained) {
            return std::unexpected{SurfaceError::v_parameter_out_of_domain};
        }
        if (v_forward()) {
            return v;
        }
        const auto mapped = reversed_parameter(domain, v);
        if (!mapped.has_value()) {
            return std::unexpected{SurfaceError::non_finite_result};
        }
        return *mapped;
    }

    [[nodiscard]] std::expected<std::pair<double, double>, SurfaceError>
    map_parameters(const double u, const double v) const noexcept {
        if (!std::isfinite(u)) {
            return std::unexpected{SurfaceError::non_finite_u_parameter};
        }
        if (!std::isfinite(v)) {
            return std::unexpected{SurfaceError::non_finite_v_parameter};
        }

        const auto mapped_u = map_u(u);
        if (!mapped_u.has_value()) {
            return std::unexpected{mapped_u.error()};
        }
        const auto mapped_v = map_v(v);
        if (!mapped_v.has_value()) {
            return std::unexpected{mapped_v.error()};
        }
        return std::pair<double, double>{*mapped_u, *mapped_v};
    }

    Surface basis_;
    double u_source_{};
    double u_target_{};
    double v_source_{};
    double v_target_{};
};

} // namespace apmesh::core
