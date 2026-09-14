#include "apmesh/core/geometry.hpp"

#include "apmesh/core/numeric.hpp"

#include <cmath>
#include <expected>
#include <limits>

namespace apmesh::core {
namespace {

[[nodiscard]] bool finite(const double value) noexcept {
    return is_finite(value);
}

[[nodiscard]] bool finite(const double x, const double y) noexcept {
    return finite(x) && finite(y);
}

[[nodiscard]] bool finite(const double x, const double y, const double z) noexcept {
    return finite(x) && finite(y) && finite(z);
}

[[nodiscard]] std::expected<double, GeometryError> finite_result(const double value) noexcept {
    if (!finite(value)) {
        return std::unexpected{GeometryError::non_finite_result};
    }
    return value;
}

template <typename Value>
[[nodiscard]] std::expected<Value, GeometryError> finite_geometry_result(
    const std::expected<Value, GeometryError>& value) noexcept {
    if (!value.has_value()) {
        return std::unexpected{GeometryError::non_finite_result};
    }
    return value;
}

[[nodiscard]] bool is_signed_unit(const double value) noexcept {
    return value == 1.0 || value == -1.0;
}

template <std::size_t Dimension, typename Matrix>
[[nodiscard]] bool is_signed_permutation_basis(const Matrix& matrix) noexcept {
    for (std::size_t row = 0; row < Dimension; ++row) {
        std::size_t signed_unit_count = 0;
        for (std::size_t column = 0; column < Dimension; ++column) {
            const auto entry = matrix.at(row, column);
            if (!entry.has_value()) {
                return false;
            }
            if (*entry == 0.0) {
                continue;
            }
            if (!is_signed_unit(*entry)) {
                return false;
            }
            ++signed_unit_count;
        }
        if (signed_unit_count != 1) {
            return false;
        }
    }

    for (std::size_t column = 0; column < Dimension; ++column) {
        std::size_t signed_unit_count = 0;
        for (std::size_t row = 0; row < Dimension; ++row) {
            const auto entry = matrix.at(row, column);
            if (!entry.has_value()) {
                return false;
            }
            if (*entry != 0.0) {
                ++signed_unit_count;
            }
        }
        if (signed_unit_count != 1) {
            return false;
        }
    }
    return true;
}

[[nodiscard]] bool scale_is_reciprocal_safe(
    const int exponent,
    double& scale,
    double& inverse_scale) noexcept {
    if (exponent == std::numeric_limits<int>::min()) {
        return false;
    }
    scale = std::scalbn(1.0, exponent);
    inverse_scale = std::scalbn(1.0, -exponent);
    return finite(scale) && scale != 0.0 && finite(inverse_scale) && inverse_scale != 0.0;
}

[[nodiscard]] Point2 local_origin2() noexcept {
    return Point2::make(0.0, 0.0).value();
}

[[nodiscard]] Point3 local_origin3() noexcept {
    return Point3::make(0.0, 0.0, 0.0).value();
}

} // namespace

std::expected<Vector2, GeometryError> Vector2::make(const double x, const double y) noexcept {
    if (!finite(x, y)) {
        return std::unexpected{GeometryError::non_finite_input};
    }
    return Vector2{x, y};
}

double Vector2::x() const noexcept { return x_; }
double Vector2::y() const noexcept { return y_; }

std::expected<Vector3, GeometryError> Vector3::make(const double x, const double y, const double z) noexcept {
    if (!finite(x, y, z)) {
        return std::unexpected{GeometryError::non_finite_input};
    }
    return Vector3{x, y, z};
}

double Vector3::x() const noexcept { return x_; }
double Vector3::y() const noexcept { return y_; }
double Vector3::z() const noexcept { return z_; }

std::expected<Point2, GeometryError> Point2::make(const double x, const double y) noexcept {
    if (!finite(x, y)) {
        return std::unexpected{GeometryError::non_finite_input};
    }
    return Point2{x, y};
}

double Point2::x() const noexcept { return x_; }
double Point2::y() const noexcept { return y_; }

std::expected<Point3, GeometryError> Point3::make(const double x, const double y, const double z) noexcept {
    if (!finite(x, y, z)) {
        return std::unexpected{GeometryError::non_finite_input};
    }
    return Point3{x, y, z};
}

double Point3::x() const noexcept { return x_; }
double Point3::y() const noexcept { return y_; }
double Point3::z() const noexcept { return z_; }

Vector2 operator-(const Vector2& vector) noexcept {
    return Vector2::make(-vector.x(), -vector.y()).value();
}

Vector3 operator-(const Vector3& vector) noexcept {
    return Vector3::make(-vector.x(), -vector.y(), -vector.z()).value();
}

std::expected<Vector2, GeometryError> operator+(const Vector2& lhs, const Vector2& rhs) noexcept {
    return finite_geometry_result(Vector2::make(lhs.x() + rhs.x(), lhs.y() + rhs.y()));
}

std::expected<Vector3, GeometryError> operator+(const Vector3& lhs, const Vector3& rhs) noexcept {
    return finite_geometry_result(Vector3::make(lhs.x() + rhs.x(), lhs.y() + rhs.y(), lhs.z() + rhs.z()));
}

std::expected<Vector2, GeometryError> operator-(const Vector2& lhs, const Vector2& rhs) noexcept {
    return finite_geometry_result(Vector2::make(lhs.x() - rhs.x(), lhs.y() - rhs.y()));
}

std::expected<Vector3, GeometryError> operator-(const Vector3& lhs, const Vector3& rhs) noexcept {
    return finite_geometry_result(Vector3::make(lhs.x() - rhs.x(), lhs.y() - rhs.y(), lhs.z() - rhs.z()));
}

std::expected<Vector2, GeometryError> operator*(const Vector2& vector, const double scalar) noexcept {
    if (!finite(scalar)) {
        return std::unexpected{GeometryError::non_finite_input};
    }
    return finite_geometry_result(Vector2::make(vector.x() * scalar, vector.y() * scalar));
}

std::expected<Vector3, GeometryError> operator*(const Vector3& vector, const double scalar) noexcept {
    if (!finite(scalar)) {
        return std::unexpected{GeometryError::non_finite_input};
    }
    return finite_geometry_result(Vector3::make(vector.x() * scalar, vector.y() * scalar, vector.z() * scalar));
}

std::expected<Vector2, GeometryError> operator*(const double scalar, const Vector2& vector) noexcept {
    return vector * scalar;
}

std::expected<Vector3, GeometryError> operator*(const double scalar, const Vector3& vector) noexcept {
    return vector * scalar;
}

std::expected<Vector2, GeometryError> operator/(const Vector2& vector, const double scalar) noexcept {
    if (!finite(scalar)) {
        return std::unexpected{GeometryError::non_finite_input};
    }
    if (scalar == 0.0) {
        return std::unexpected{GeometryError::division_by_zero};
    }
    return finite_geometry_result(Vector2::make(vector.x() / scalar, vector.y() / scalar));
}

std::expected<Vector3, GeometryError> operator/(const Vector3& vector, const double scalar) noexcept {
    if (!finite(scalar)) {
        return std::unexpected{GeometryError::non_finite_input};
    }
    if (scalar == 0.0) {
        return std::unexpected{GeometryError::division_by_zero};
    }
    return finite_geometry_result(Vector3::make(vector.x() / scalar, vector.y() / scalar, vector.z() / scalar));
}

std::expected<Point2, GeometryError> operator+(const Point2& point, const Vector2& vector) noexcept {
    return finite_geometry_result(Point2::make(point.x() + vector.x(), point.y() + vector.y()));
}

std::expected<Point3, GeometryError> operator+(const Point3& point, const Vector3& vector) noexcept {
    return finite_geometry_result(Point3::make(point.x() + vector.x(), point.y() + vector.y(), point.z() + vector.z()));
}

std::expected<Point2, GeometryError> operator-(const Point2& point, const Vector2& vector) noexcept {
    return finite_geometry_result(Point2::make(point.x() - vector.x(), point.y() - vector.y()));
}

std::expected<Point3, GeometryError> operator-(const Point3& point, const Vector3& vector) noexcept {
    return finite_geometry_result(Point3::make(point.x() - vector.x(), point.y() - vector.y(), point.z() - vector.z()));
}

std::expected<Vector2, GeometryError> operator-(const Point2& lhs, const Point2& rhs) noexcept {
    return finite_geometry_result(Vector2::make(lhs.x() - rhs.x(), lhs.y() - rhs.y()));
}

std::expected<Vector3, GeometryError> operator-(const Point3& lhs, const Point3& rhs) noexcept {
    return finite_geometry_result(Vector3::make(lhs.x() - rhs.x(), lhs.y() - rhs.y(), lhs.z() - rhs.z()));
}

std::expected<double, GeometryError> dot(const Vector2& lhs, const Vector2& rhs) noexcept {
    return finite_result(lhs.x() * rhs.x() + lhs.y() * rhs.y());
}

std::expected<double, GeometryError> dot(const Vector3& lhs, const Vector3& rhs) noexcept {
    return finite_result(lhs.x() * rhs.x() + lhs.y() * rhs.y() + lhs.z() * rhs.z());
}

std::expected<Vector3, GeometryError> cross(const Vector3& lhs, const Vector3& rhs) noexcept {
    return finite_geometry_result(Vector3::make(
        lhs.y() * rhs.z() - lhs.z() * rhs.y(),
        lhs.z() * rhs.x() - lhs.x() * rhs.z(),
        lhs.x() * rhs.y() - lhs.y() * rhs.x()));
}

std::expected<double, GeometryError> norm(const Vector2& vector) noexcept {
    return finite_result(std::hypot(vector.x(), vector.y()));
}

std::expected<double, GeometryError> norm(const Vector3& vector) noexcept {
    return finite_result(std::hypot(vector.x(), vector.y(), vector.z()));
}

std::expected<Vector2, GeometryError> normalize(const Vector2& vector) noexcept {
    const auto length = norm(vector);
    if (!length.has_value()) {
        return std::unexpected{length.error()};
    }
    if (*length == 0.0) {
        return std::unexpected{GeometryError::zero_length};
    }
    return vector / *length;
}

std::expected<Vector3, GeometryError> normalize(const Vector3& vector) noexcept {
    const auto length = norm(vector);
    if (!length.has_value()) {
        return std::unexpected{length.error()};
    }
    if (*length == 0.0) {
        return std::unexpected{GeometryError::zero_length};
    }
    return vector / *length;
}

std::expected<Vector2, GeometryError> apply(
    const Mat2& matrix,
    const Vector2& vector) noexcept {
    const auto m00 = matrix.at(0, 0);
    const auto m01 = matrix.at(0, 1);
    const auto m10 = matrix.at(1, 0);
    const auto m11 = matrix.at(1, 1);
    if (!m00 || !m01 || !m10 || !m11) {
        return std::unexpected{GeometryError::non_finite_result};
    }
    return finite_geometry_result(Vector2::make(
        *m00 * vector.x() + *m01 * vector.y(),
        *m10 * vector.x() + *m11 * vector.y()));
}

std::expected<Vector3, GeometryError> apply(
    const Mat3& matrix,
    const Vector3& vector) noexcept {
    const auto m00 = matrix.at(0, 0);
    const auto m01 = matrix.at(0, 1);
    const auto m02 = matrix.at(0, 2);
    const auto m10 = matrix.at(1, 0);
    const auto m11 = matrix.at(1, 1);
    const auto m12 = matrix.at(1, 2);
    const auto m20 = matrix.at(2, 0);
    const auto m21 = matrix.at(2, 1);
    const auto m22 = matrix.at(2, 2);
    if (!m00 || !m01 || !m02 || !m10 || !m11 || !m12 || !m20 || !m21 || !m22) {
        return std::unexpected{GeometryError::non_finite_result};
    }
    return finite_geometry_result(Vector3::make(
        *m00 * vector.x() + *m01 * vector.y() + *m02 * vector.z(),
        *m10 * vector.x() + *m11 * vector.y() + *m12 * vector.z(),
        *m20 * vector.x() + *m21 * vector.y() + *m22 * vector.z()));
}

std::expected<CartesianFrame2, GeometryError> CartesianFrame2::make(
    const Point2& origin,
    const Mat2& basis,
    const int scale_exponent) noexcept {
    if (!is_signed_permutation_basis<2>(basis)) {
        return std::unexpected{GeometryError::invalid_frame};
    }
    double scale = 0.0;
    double inverse_scale = 0.0;
    if (!scale_is_reciprocal_safe(scale_exponent, scale, inverse_scale)) {
        return std::unexpected{GeometryError::scale_out_of_range};
    }
    return CartesianFrame2{origin, basis, scale_exponent, scale, inverse_scale};
}

CartesianFrame2 CartesianFrame2::identity() noexcept {
    return CartesianFrame2{local_origin2(), Mat2::identity(), 0, 1.0, 1.0};
}

const Point2& CartesianFrame2::origin() const noexcept { return origin_; }
const Mat2& CartesianFrame2::basis() const noexcept { return basis_; }
int CartesianFrame2::scale_exponent() const noexcept { return scale_exponent_; }

std::expected<Vector2, GeometryError> CartesianFrame2::vector_to_world(
    const Vector2& vector) const noexcept {
    const auto mapped = apply(basis_, vector);
    if (!mapped) {
        return std::unexpected{mapped.error()};
    }
    return *mapped * scale_;
}

std::expected<Vector2, GeometryError> CartesianFrame2::vector_to_local(
    const Vector2& vector) const noexcept {
    const auto mapped = apply(transpose(basis_), vector);
    if (!mapped) {
        return std::unexpected{mapped.error()};
    }
    return *mapped * inverse_scale_;
}

std::expected<Point2, GeometryError> CartesianFrame2::point_to_world(
    const Point2& point) const noexcept {
    const auto local_vector = point - local_origin2();
    if (!local_vector) {
        return std::unexpected{local_vector.error()};
    }
    const auto mapped = vector_to_world(*local_vector);
    if (!mapped) {
        return std::unexpected{mapped.error()};
    }
    return origin_ + *mapped;
}

std::expected<Point2, GeometryError> CartesianFrame2::point_to_local(
    const Point2& point) const noexcept {
    const auto world_vector = point - origin_;
    if (!world_vector) {
        return std::unexpected{world_vector.error()};
    }
    const auto mapped = vector_to_local(*world_vector);
    if (!mapped) {
        return std::unexpected{mapped.error()};
    }
    return local_origin2() + *mapped;
}

std::expected<CartesianFrame3, GeometryError> CartesianFrame3::make(
    const Point3& origin,
    const Mat3& basis,
    const int scale_exponent) noexcept {
    if (!is_signed_permutation_basis<3>(basis)) {
        return std::unexpected{GeometryError::invalid_frame};
    }
    double scale = 0.0;
    double inverse_scale = 0.0;
    if (!scale_is_reciprocal_safe(scale_exponent, scale, inverse_scale)) {
        return std::unexpected{GeometryError::scale_out_of_range};
    }
    return CartesianFrame3{origin, basis, scale_exponent, scale, inverse_scale};
}

CartesianFrame3 CartesianFrame3::identity() noexcept {
    return CartesianFrame3{local_origin3(), Mat3::identity(), 0, 1.0, 1.0};
}

const Point3& CartesianFrame3::origin() const noexcept { return origin_; }
const Mat3& CartesianFrame3::basis() const noexcept { return basis_; }
int CartesianFrame3::scale_exponent() const noexcept { return scale_exponent_; }

std::expected<Vector3, GeometryError> CartesianFrame3::vector_to_world(
    const Vector3& vector) const noexcept {
    const auto mapped = apply(basis_, vector);
    if (!mapped) {
        return std::unexpected{mapped.error()};
    }
    return *mapped * scale_;
}

std::expected<Vector3, GeometryError> CartesianFrame3::vector_to_local(
    const Vector3& vector) const noexcept {
    const auto mapped = apply(transpose(basis_), vector);
    if (!mapped) {
        return std::unexpected{mapped.error()};
    }
    return *mapped * inverse_scale_;
}

std::expected<Point3, GeometryError> CartesianFrame3::point_to_world(
    const Point3& point) const noexcept {
    const auto local_vector = point - local_origin3();
    if (!local_vector) {
        return std::unexpected{local_vector.error()};
    }
    const auto mapped = vector_to_world(*local_vector);
    if (!mapped) {
        return std::unexpected{mapped.error()};
    }
    return origin_ + *mapped;
}

std::expected<Point3, GeometryError> CartesianFrame3::point_to_local(
    const Point3& point) const noexcept {
    const auto world_vector = point - origin_;
    if (!world_vector) {
        return std::unexpected{world_vector.error()};
    }
    const auto mapped = vector_to_local(*world_vector);
    if (!mapped) {
        return std::unexpected{mapped.error()};
    }
    return local_origin3() + *mapped;
}

} // namespace apmesh::core
