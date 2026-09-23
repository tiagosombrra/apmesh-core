#pragma once

#include "apmesh/math/linear_algebra.hpp"

#include <expected>

namespace apmesh::core {

enum class GeometryError {
    non_finite_input,
    non_finite_result,
    division_by_zero,
    zero_length,
    indeterminate,
    invalid_frame,
    scale_out_of_range,
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

class CartesianFrame2 {
public:
    [[nodiscard]] static std::expected<CartesianFrame2, GeometryError> make(
        const Point2& origin,
        const Mat2& basis,
        int scale_exponent) noexcept;
    [[nodiscard]] static CartesianFrame2 identity() noexcept;

    [[nodiscard]] const Point2& origin() const noexcept;
    [[nodiscard]] const Mat2& basis() const noexcept;
    [[nodiscard]] int scale_exponent() const noexcept;

    [[nodiscard]] std::expected<Point2, GeometryError> point_to_world(
        const Point2& point) const noexcept;
    [[nodiscard]] std::expected<Vector2, GeometryError> vector_to_world(
        const Vector2& vector) const noexcept;
    [[nodiscard]] std::expected<Point2, GeometryError> point_to_local(
        const Point2& point) const noexcept;
    [[nodiscard]] std::expected<Vector2, GeometryError> vector_to_local(
        const Vector2& vector) const noexcept;

private:
    constexpr CartesianFrame2(
        const Point2& origin,
        const Mat2& basis,
        const int scale_exponent,
        const double scale,
        const double inverse_scale) noexcept
        : origin_(origin),
          basis_(basis),
          scale_exponent_(scale_exponent),
          scale_(scale),
          inverse_scale_(inverse_scale) {}

    Point2 origin_;
    Mat2 basis_;
    int scale_exponent_;
    double scale_;
    double inverse_scale_;
};

class CartesianFrame3 {
public:
    [[nodiscard]] static std::expected<CartesianFrame3, GeometryError> make(
        const Point3& origin,
        const Mat3& basis,
        int scale_exponent) noexcept;
    [[nodiscard]] static CartesianFrame3 identity() noexcept;

    [[nodiscard]] const Point3& origin() const noexcept;
    [[nodiscard]] const Mat3& basis() const noexcept;
    [[nodiscard]] int scale_exponent() const noexcept;

    [[nodiscard]] std::expected<Point3, GeometryError> point_to_world(
        const Point3& point) const noexcept;
    [[nodiscard]] std::expected<Vector3, GeometryError> vector_to_world(
        const Vector3& vector) const noexcept;
    [[nodiscard]] std::expected<Point3, GeometryError> point_to_local(
        const Point3& point) const noexcept;
    [[nodiscard]] std::expected<Vector3, GeometryError> vector_to_local(
        const Vector3& vector) const noexcept;

private:
    constexpr CartesianFrame3(
        const Point3& origin,
        const Mat3& basis,
        const int scale_exponent,
        const double scale,
        const double inverse_scale) noexcept
        : origin_(origin),
          basis_(basis),
          scale_exponent_(scale_exponent),
          scale_(scale),
          inverse_scale_(inverse_scale) {}

    Point3 origin_;
    Mat3 basis_;
    int scale_exponent_;
    double scale_;
    double inverse_scale_;
};

class AxisPlacement3 {
public:
    [[nodiscard]] static std::expected<AxisPlacement3, GeometryError> make(
        const Point3& origin,
        const Vector3& main_direction,
        const Vector3& x_reference) noexcept;
    [[nodiscard]] static AxisPlacement3 identity() noexcept;

    [[nodiscard]] const Point3& origin() const noexcept;
    [[nodiscard]] const Vector3& x_direction() const noexcept;
    [[nodiscard]] const Vector3& y_direction() const noexcept;
    [[nodiscard]] const Vector3& z_direction() const noexcept;

    [[nodiscard]] std::expected<Point3, GeometryError> point_to_world(
        const Point3& point) const noexcept;
    [[nodiscard]] std::expected<Vector3, GeometryError> vector_to_world(
        const Vector3& vector) const noexcept;
    [[nodiscard]] std::expected<Point3, GeometryError> point_to_local(
        const Point3& point) const noexcept;
    [[nodiscard]] std::expected<Vector3, GeometryError> vector_to_local(
        const Vector3& vector) const noexcept;

    [[nodiscard]] bool operator==(const AxisPlacement3&) const noexcept = default;

private:
    constexpr AxisPlacement3(
        const Point3& origin,
        const Vector3& x_direction,
        const Vector3& y_direction,
        const Vector3& z_direction) noexcept
        : origin_(origin),
          x_direction_(x_direction),
          y_direction_(y_direction),
          z_direction_(z_direction) {}

    Point3 origin_;
    Vector3 x_direction_;
    Vector3 y_direction_;
    Vector3 z_direction_;
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

[[nodiscard]] std::expected<Vector2, GeometryError> apply(
    const Mat2& matrix,
    const Vector2& vector) noexcept;
[[nodiscard]] std::expected<Vector3, GeometryError> apply(
    const Mat3& matrix,
    const Vector3& vector) noexcept;

} // namespace apmesh::core
