#include "apmesh/math/linear_algebra.hpp"

#include <array>
#include <type_traits>

int main() {
    using apmesh::core::Mat2;
    using apmesh::core::Mat3;

    static_assert(!std::is_same_v<Mat2, Mat3>);
    static_assert(!std::is_default_constructible_v<Mat2>);
    static_assert(!std::is_default_constructible_v<Mat3>);

    const auto matrix = Mat2::make(std::array<double, 4>{1.0, 0.0, 0.0, 1.0});
    return matrix.has_value() && matrix->at(1, 1).has_value() ? 0 : 1;
}
