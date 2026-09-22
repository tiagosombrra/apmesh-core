#pragma once

#include "apmesh/geometry/parametric_curve.hpp"

#include <expected>

namespace apmesh::core {

class LineSegment2 {
public:
    constexpr LineSegment2(const Point2& source, const Point2& target) noexcept
        : source_(source), target_(target) {}

    [[nodiscard]] const Point2& source() const noexcept;
    [[nodiscard]] const Point2& target() const noexcept;
    [[nodiscard]] CurveParameterDomain parameter_domain() const noexcept;
    [[nodiscard]] std::expected<Point2, CurveError>
    evaluate(double parameter) const noexcept;
    [[nodiscard]] std::expected<Vector2, CurveError>
    first_derivative(double parameter) const noexcept;
    [[nodiscard]] std::expected<Vector2, CurveError>
    second_derivative(double parameter) const noexcept;
    [[nodiscard]] LineSegment2 reversed() const noexcept;

    [[nodiscard]] bool operator==(const LineSegment2&) const noexcept = default;

private:
    Point2 source_;
    Point2 target_;
};

class LineSegment3 {
public:
    constexpr LineSegment3(const Point3& source, const Point3& target) noexcept
        : source_(source), target_(target) {}

    [[nodiscard]] const Point3& source() const noexcept;
    [[nodiscard]] const Point3& target() const noexcept;
    [[nodiscard]] CurveParameterDomain parameter_domain() const noexcept;
    [[nodiscard]] std::expected<Point3, CurveError>
    evaluate(double parameter) const noexcept;
    [[nodiscard]] std::expected<Vector3, CurveError>
    first_derivative(double parameter) const noexcept;
    [[nodiscard]] std::expected<Vector3, CurveError>
    second_derivative(double parameter) const noexcept;
    [[nodiscard]] LineSegment3 reversed() const noexcept;

    [[nodiscard]] bool operator==(const LineSegment3&) const noexcept = default;

private:
    Point3 source_;
    Point3 target_;
};

} // namespace apmesh::core
