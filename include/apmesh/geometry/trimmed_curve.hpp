#pragma once

#include "apmesh/geometry/parametric_curve.hpp"

#include <algorithm>
#include <cmath>
#include <expected>
#include <utility>

namespace apmesh::core {

enum class TrimmedCurveConstructionError {
    non_finite_source_parameter,
    non_finite_target_parameter,
    source_parameter_out_of_domain,
    target_parameter_out_of_domain,
    zero_width_trim,
};

template <BoundedParametricCurve2 Curve>
class TrimmedCurve2 {
public:
    [[nodiscard]] static std::expected<
        TrimmedCurve2,
        TrimmedCurveConstructionError>
    make(
        Curve basis,
        const double source_parameter,
        const double target_parameter) {
        const auto validation =
            validate_construction(basis, source_parameter, target_parameter);
        if (!validation.has_value()) {
            return std::unexpected{validation.error()};
        }
        return TrimmedCurve2{
            std::move(basis), source_parameter, target_parameter};
    }

    [[nodiscard]] const Curve& basis_curve() const noexcept {
        return basis_;
    }

    [[nodiscard]] double source_parameter() const noexcept {
        return source_parameter_;
    }

    [[nodiscard]] double target_parameter() const noexcept {
        return target_parameter_;
    }

    [[nodiscard]] CurveParameterDomain parameter_domain() const noexcept {
        const auto domain = CurveParameterDomain::make(
            std::min(source_parameter_, target_parameter_),
            std::max(source_parameter_, target_parameter_));
        return *domain;
    }

    [[nodiscard]] std::expected<Point2, CurveError>
    evaluate(const double parameter) const {
        const auto mapped = map_parameter(parameter);
        if (!mapped.has_value()) {
            return std::unexpected{mapped.error()};
        }
        return basis_.evaluate(*mapped);
    }

    [[nodiscard]] std::expected<Vector2, CurveError>
    first_derivative(const double parameter) const {
        const auto mapped = map_parameter(parameter);
        if (!mapped.has_value()) {
            return std::unexpected{mapped.error()};
        }

        const auto derivative = basis_.first_derivative(*mapped);
        if (!derivative.has_value()) {
            return std::unexpected{derivative.error()};
        }

        if (source_parameter_ < target_parameter_) {
            return *derivative;
        }
        return -*derivative;
    }

    [[nodiscard]] std::expected<Vector2, CurveError>
    second_derivative(const double parameter) const {
        const auto mapped = map_parameter(parameter);
        if (!mapped.has_value()) {
            return std::unexpected{mapped.error()};
        }
        return basis_.second_derivative(*mapped);
    }

    [[nodiscard]] TrimmedCurve2 reversed() const {
        return TrimmedCurve2{basis_, target_parameter_, source_parameter_};
    }

    [[nodiscard]] bool operator==(const TrimmedCurve2&) const = default;

private:
    TrimmedCurve2(
        Curve basis,
        const double source_parameter,
        const double target_parameter)
        : basis_(std::move(basis)),
          source_parameter_(source_parameter),
          target_parameter_(target_parameter) {}

    [[nodiscard]] static std::expected<void, TrimmedCurveConstructionError>
    validate_construction(
        const Curve& basis,
        const double source_parameter,
        const double target_parameter) {
        if (!std::isfinite(source_parameter)) {
            return std::unexpected{
                TrimmedCurveConstructionError::non_finite_source_parameter};
        }
        if (!std::isfinite(target_parameter)) {
            return std::unexpected{
                TrimmedCurveConstructionError::non_finite_target_parameter};
        }

        const auto basis_domain = basis.parameter_domain();
        const auto source_contained = basis_domain.contains(source_parameter);
        if (!source_contained.has_value() || !*source_contained) {
            return std::unexpected{
                TrimmedCurveConstructionError::source_parameter_out_of_domain};
        }

        const auto target_contained = basis_domain.contains(target_parameter);
        if (!target_contained.has_value() || !*target_contained) {
            return std::unexpected{
                TrimmedCurveConstructionError::target_parameter_out_of_domain};
        }

        if (source_parameter == target_parameter) {
            return std::unexpected{
                TrimmedCurveConstructionError::zero_width_trim};
        }
        return {};
    }

    [[nodiscard]] std::expected<double, CurveError>
    map_parameter(const double parameter) const noexcept {
        const auto domain = parameter_domain();
        const auto contained = domain.contains(parameter);
        if (!contained.has_value()) {
            return std::unexpected{contained.error()};
        }
        if (!*contained) {
            return std::unexpected{CurveError::parameter_out_of_domain};
        }

        if (source_parameter_ < target_parameter_) {
            return parameter;
        }
        return reversed_parameter(domain, parameter);
    }

    Curve basis_;
    double source_parameter_{};
    double target_parameter_{};
};

template <BoundedParametricCurve3 Curve>
class TrimmedCurve3 {
public:
    [[nodiscard]] static std::expected<
        TrimmedCurve3,
        TrimmedCurveConstructionError>
    make(
        Curve basis,
        const double source_parameter,
        const double target_parameter) {
        const auto validation =
            validate_construction(basis, source_parameter, target_parameter);
        if (!validation.has_value()) {
            return std::unexpected{validation.error()};
        }
        return TrimmedCurve3{
            std::move(basis), source_parameter, target_parameter};
    }

    [[nodiscard]] const Curve& basis_curve() const noexcept {
        return basis_;
    }

    [[nodiscard]] double source_parameter() const noexcept {
        return source_parameter_;
    }

    [[nodiscard]] double target_parameter() const noexcept {
        return target_parameter_;
    }

    [[nodiscard]] CurveParameterDomain parameter_domain() const noexcept {
        const auto domain = CurveParameterDomain::make(
            std::min(source_parameter_, target_parameter_),
            std::max(source_parameter_, target_parameter_));
        return *domain;
    }

    [[nodiscard]] std::expected<Point3, CurveError>
    evaluate(const double parameter) const {
        const auto mapped = map_parameter(parameter);
        if (!mapped.has_value()) {
            return std::unexpected{mapped.error()};
        }
        return basis_.evaluate(*mapped);
    }

    [[nodiscard]] std::expected<Vector3, CurveError>
    first_derivative(const double parameter) const {
        const auto mapped = map_parameter(parameter);
        if (!mapped.has_value()) {
            return std::unexpected{mapped.error()};
        }

        const auto derivative = basis_.first_derivative(*mapped);
        if (!derivative.has_value()) {
            return std::unexpected{derivative.error()};
        }

        if (source_parameter_ < target_parameter_) {
            return *derivative;
        }
        return -*derivative;
    }

    [[nodiscard]] std::expected<Vector3, CurveError>
    second_derivative(const double parameter) const {
        const auto mapped = map_parameter(parameter);
        if (!mapped.has_value()) {
            return std::unexpected{mapped.error()};
        }
        return basis_.second_derivative(*mapped);
    }

    [[nodiscard]] TrimmedCurve3 reversed() const {
        return TrimmedCurve3{basis_, target_parameter_, source_parameter_};
    }

    [[nodiscard]] bool operator==(const TrimmedCurve3&) const = default;

private:
    TrimmedCurve3(
        Curve basis,
        const double source_parameter,
        const double target_parameter)
        : basis_(std::move(basis)),
          source_parameter_(source_parameter),
          target_parameter_(target_parameter) {}

    [[nodiscard]] static std::expected<void, TrimmedCurveConstructionError>
    validate_construction(
        const Curve& basis,
        const double source_parameter,
        const double target_parameter) {
        if (!std::isfinite(source_parameter)) {
            return std::unexpected{
                TrimmedCurveConstructionError::non_finite_source_parameter};
        }
        if (!std::isfinite(target_parameter)) {
            return std::unexpected{
                TrimmedCurveConstructionError::non_finite_target_parameter};
        }

        const auto basis_domain = basis.parameter_domain();
        const auto source_contained = basis_domain.contains(source_parameter);
        if (!source_contained.has_value() || !*source_contained) {
            return std::unexpected{
                TrimmedCurveConstructionError::source_parameter_out_of_domain};
        }

        const auto target_contained = basis_domain.contains(target_parameter);
        if (!target_contained.has_value() || !*target_contained) {
            return std::unexpected{
                TrimmedCurveConstructionError::target_parameter_out_of_domain};
        }

        if (source_parameter == target_parameter) {
            return std::unexpected{
                TrimmedCurveConstructionError::zero_width_trim};
        }
        return {};
    }

    [[nodiscard]] std::expected<double, CurveError>
    map_parameter(const double parameter) const noexcept {
        const auto domain = parameter_domain();
        const auto contained = domain.contains(parameter);
        if (!contained.has_value()) {
            return std::unexpected{contained.error()};
        }
        if (!*contained) {
            return std::unexpected{CurveError::parameter_out_of_domain};
        }

        if (source_parameter_ < target_parameter_) {
            return parameter;
        }
        return reversed_parameter(domain, parameter);
    }

    Curve basis_;
    double source_parameter_{};
    double target_parameter_{};
};

} // namespace apmesh::core
