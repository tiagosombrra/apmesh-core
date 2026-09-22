#include "apmesh/geometry/line_segment.hpp"

#include <cmath>
#include <expected>

namespace apmesh::core {
namespace {

[[nodiscard]] CurveParameterDomain unit_parameter_domain() noexcept {
    const auto domain = CurveParameterDomain::make(0.0, 1.0);
    return *domain;
}

[[nodiscard]] std::expected<void, CurveError> validate_parameter(
    const double parameter) noexcept {
    const auto contained = unit_parameter_domain().contains(parameter);
    if (!contained.has_value()) {
        return std::unexpected{contained.error()};
    }
    if (!*contained) {
        return std::unexpected{CurveError::parameter_out_of_domain};
    }
    return {};
}

[[nodiscard]] std::expected<Point2, CurveError> interpolate(
    const Point2& source,
    const Point2& target,
    const double parameter) noexcept {
    const auto point = Point2::make(
        std::lerp(source.x(), target.x(), parameter),
        std::lerp(source.y(), target.y(), parameter));
    if (!point.has_value()) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return *point;
}

[[nodiscard]] std::expected<Point3, CurveError> interpolate(
    const Point3& source,
    const Point3& target,
    const double parameter) noexcept {
    const auto point = Point3::make(
        std::lerp(source.x(), target.x(), parameter),
        std::lerp(source.y(), target.y(), parameter),
        std::lerp(source.z(), target.z(), parameter));
    if (!point.has_value()) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return *point;
}

[[nodiscard]] std::expected<Vector2, CurveError> derivative(
    const Point2& source,
    const Point2& target) noexcept {
    const auto value = target - source;
    if (!value.has_value()) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return *value;
}

[[nodiscard]] std::expected<Vector3, CurveError> derivative(
    const Point3& source,
    const Point3& target) noexcept {
    const auto value = target - source;
    if (!value.has_value()) {
        return std::unexpected{CurveError::non_finite_result};
    }
    return *value;
}

[[nodiscard]] Vector2 zero_vector2() noexcept {
    return *Vector2::make(0.0, 0.0);
}

[[nodiscard]] Vector3 zero_vector3() noexcept {
    return *Vector3::make(0.0, 0.0, 0.0);
}

} // namespace

const Point2& LineSegment2::source() const noexcept {
    return source_;
}

const Point2& LineSegment2::target() const noexcept {
    return target_;
}

CurveParameterDomain LineSegment2::parameter_domain() const noexcept {
    return unit_parameter_domain();
}

std::expected<Point2, CurveError> LineSegment2::evaluate(
    const double parameter) const noexcept {
    const auto valid = validate_parameter(parameter);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }
    return interpolate(source_, target_, parameter);
}

std::expected<Vector2, CurveError> LineSegment2::first_derivative(
    const double parameter) const noexcept {
    const auto valid = validate_parameter(parameter);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }
    return derivative(source_, target_);
}

std::expected<Vector2, CurveError> LineSegment2::second_derivative(
    const double parameter) const noexcept {
    const auto valid = validate_parameter(parameter);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }
    return zero_vector2();
}

LineSegment2 LineSegment2::reversed() const noexcept {
    return LineSegment2{target_, source_};
}

const Point3& LineSegment3::source() const noexcept {
    return source_;
}

const Point3& LineSegment3::target() const noexcept {
    return target_;
}

CurveParameterDomain LineSegment3::parameter_domain() const noexcept {
    return unit_parameter_domain();
}

std::expected<Point3, CurveError> LineSegment3::evaluate(
    const double parameter) const noexcept {
    const auto valid = validate_parameter(parameter);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }
    return interpolate(source_, target_, parameter);
}

std::expected<Vector3, CurveError> LineSegment3::first_derivative(
    const double parameter) const noexcept {
    const auto valid = validate_parameter(parameter);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }
    return derivative(source_, target_);
}

std::expected<Vector3, CurveError> LineSegment3::second_derivative(
    const double parameter) const noexcept {
    const auto valid = validate_parameter(parameter);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }
    return zero_vector3();
}

LineSegment3 LineSegment3::reversed() const noexcept {
    return LineSegment3{target_, source_};
}

} // namespace apmesh::core
