#include "apmesh/geometry/surface_differential.hpp"

#include <cmath>
#include <limits>

namespace apmesh::core {
namespace {

[[nodiscard]] std::expected<double, SurfaceDifferentialError>
finite_product(const double lhs, const double rhs) noexcept {
    if (lhs == 0.0 || rhs == 0.0) {
        return 0.0;
    }

    int lhs_exponent = 0;
    int rhs_exponent = 0;
    const double lhs_fraction = std::frexp(lhs, &lhs_exponent);
    const double rhs_fraction = std::frexp(rhs, &rhs_exponent);
    const long long exponent =
        static_cast<long long>(lhs_exponent) +
        static_cast<long long>(rhs_exponent);
    if (exponent > std::numeric_limits<int>::max() ||
        exponent < std::numeric_limits<int>::min()) {
        return std::unexpected{
            SurfaceDifferentialError::non_representable_result};
    }

    const double value = std::ldexp(
        lhs_fraction * rhs_fraction,
        static_cast<int>(exponent));
    if (!std::isfinite(value) || value == 0.0) {
        return std::unexpected{
            SurfaceDifferentialError::non_representable_result};
    }
    return value;
}

[[nodiscard]] std::expected<Vector3, SurfaceDifferentialError>
scaled_unit_vector(
    const Vector3& vector,
    const double length) noexcept {
    const auto value = vector / length;
    if (!value.has_value()) {
        return std::unexpected{
            SurfaceDifferentialError::non_representable_result};
    }
    return *value;
}

} // namespace

} // namespace apmesh::core
