#pragma once

#include <expected>

namespace apmesh::core {

enum class GeometryError {
    non_finite_input,
    non_finite_result,
    division_by_zero,
    zero_length,
    indeterminate,
};

class Vector2 {
public:
    [[nodiscard]] static std::expected<Vector2, GeometryError> make(double x, double y) noexcept;

    [[nodiscard]] double x() const noexcept;
    [[nodiscard]] double y() const noexcept;

    [[nodiscard]] bool operator==(const Vector2&) const noexcept = default;

private:
    constexpr Vector2(const double x, const double y) noexcept : x_(x), y_(y) {}

    double x_;
    double y_;
};

class Vector3 {
public:
    [[nodiscard]] static std::expected<Vector3, GeometryError> make(double x, double y, double z) noexcept;

    [[nodiscard]] double x() const noexcept;
    [[nodiscard]] double y() const noexcept;
    [[nodiscard]] double z() const noexcept;

    [[nodiscard]] bool operator==(const Vector3&) const noexcept = default;

private:
    constexpr Vector3(const double x, const double y, const double z) noexcept : x_(x), y_(y), z_(z) {}

    double x_;
    double y_;
    double z_;
};

class Point2 {
public:
    [[nodiscard]] static std::expected<Point2, GeometryError> make(double x, double y) noexcept;

    [[nodiscard]] double x() const noexcept;
    [[nodiscard]] double y() const noexcept;

    [[nodiscard]] bool operator==(const Point2&) const noexcept = default;

private:
    constexpr Point2(const double x, const double y) noexcept : x_(x), y_(y) {}

    double x_;
    double y_;
};

class Point3 {
public:
    [[nodiscard]] static std::expected<Point3, GeometryError> make(double x, double y, double z) noexcept;

    [[nodiscard]] double x() const noexcept;
    [[nodiscard]] double y() const noexcept;
    [[nodiscard]] double z() const noexcept;

    [[nodiscard]] bool operator==(const Point3&) const noexcept = default;

private:
    constexpr Point3(const double x, const double y, const double z) noexcept : x_(x), y_(y), z_(z) {}

    double x_;
    double y_;
    double z_;
};

[[nodiscard]] Vector2 operator-(const Vector2& vector) noexcept;
[[nodiscard]] Vector3 operator-(const Vector3& vector) noexcept;

[[nodiscard]] std::expected<Vector2, GeometryError> operator+(const Vector2& lhs, const Vector2& rhs) noexcept;
[[nodiscard]] std::expected<Vector3, GeometryError> operator+(const Vector3& lhs, const Vector3& rhs) noexcept;
[[nodiscard]] std::expected<Vector2, GeometryError> operator-(const Vector2& lhs, const Vector2& rhs) noexcept;
[[nodiscard]] std::expected<Vector3, GeometryError> operator-(const Vector3& lhs, const Vector3& rhs) noexcept;
[[nodiscard]] std::expected<Vector2, GeometryError> operator*(const Vector2& vector, double scalar) noexcept;
[[nodiscard]] std::expected<Vector3, GeometryError> operator*(const Vector3& vector, double scalar) noexcept;
[[nodiscard]] std::expected<Vector2, GeometryError> operator*(double scalar, const Vector2& vector) noexcept;
[[nodiscard]] std::expected<Vector3, GeometryError> operator*(double scalar, const Vector3& vector) noexcept;
[[nodiscard]] std::expected<Vector2, GeometryError> operator/(const Vector2& vector, double scalar) noexcept;
[[nodiscard]] std::expected<Vector3, GeometryError> operator/(const Vector3& vector, double scalar) noexcept;

[[nodiscard]] std::expected<Point2, GeometryError> operator+(const Point2& point, const Vector2& vector) noexcept;
[[nodiscard]] std::expected<Point3, GeometryError> operator+(const Point3& point, const Vector3& vector) noexcept;
[[nodiscard]] std::expected<Point2, GeometryError> operator-(const Point2& point, const Vector2& vector) noexcept;
[[nodiscard]] std::expected<Point3, GeometryError> operator-(const Point3& point, const Vector3& vector) noexcept;
[[nodiscard]] std::expected<Vector2, GeometryError> operator-(const Point2& lhs, const Point2& rhs) noexcept;
[[nodiscard]] std::expected<Vector3, GeometryError> operator-(const Point3& lhs, const Point3& rhs) noexcept;

[[nodiscard]] std::expected<double, GeometryError> dot(const Vector2& lhs, const Vector2& rhs) noexcept;
[[nodiscard]] std::expected<double, GeometryError> dot(const Vector3& lhs, const Vector3& rhs) noexcept;
[[nodiscard]] std::expected<Vector3, GeometryError> cross(const Vector3& lhs, const Vector3& rhs) noexcept;
[[nodiscard]] std::expected<double, GeometryError> norm(const Vector2& vector) noexcept;
[[nodiscard]] std::expected<double, GeometryError> norm(const Vector3& vector) noexcept;
[[nodiscard]] std::expected<Vector2, GeometryError> normalize(const Vector2& vector) noexcept;
[[nodiscard]] std::expected<Vector3, GeometryError> normalize(const Vector3& vector) noexcept;

} // namespace apmesh::core
