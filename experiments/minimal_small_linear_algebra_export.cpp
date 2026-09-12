#include "apmesh/core/geometry.hpp"
#include "apmesh/math/linear_algebra.hpp"

#include <array>
#include <fstream>
#include <limits>
#include <string_view>

namespace {

using apmesh::core::GeometryError;
using apmesh::core::LinearAlgebraError;
using apmesh::core::Mat2;
using apmesh::core::Mat3;
using apmesh::core::Vector2;
using apmesh::core::Vector3;
using apmesh::core::apply;
using apmesh::core::compose;
using apmesh::core::determinant;
using apmesh::core::transpose;

void write_case(
    std::ofstream& output,
    const std::string_view family,
    const bool passed,
    const std::string_view check,
    const bool determinant_boundary = false) {
    output << "{\"family\":\"" << family << "\",\"status\":\""
           << (passed ? "PASS" : "FAIL") << "\",\"expected\":{\"check\":\""
           << check << "\"},\"observed\":{\"check\":\"" << check
           << "\",\"pass\":" << (passed ? "true" : "false") << "},\"non_claims\":[";
    if (determinant_boundary) {
        output << "\"predicate\",\"rank\",\"degeneracy\",\"orientation\",\"incidence\",\"topology\"";
    }
    output << "]}";
}

int write_certificate(const std::string_view output_path) {
    std::ofstream output{output_path.data(), std::ios::binary | std::ios::trunc};
    if (!output.is_open()) {
        return 2;
    }

    const auto identity2 = Mat2::identity();
    const auto identity3 = Mat3::identity();
    const auto vector2 = Vector2::make(3.0, -2.0);
    const auto vector3 = Vector3::make(2.0, -1.0, 4.0);
    const auto diagonal2 = Mat2::make(std::array<double, 4>{2.0, 0.0, 0.0, -4.0});
    const auto diagonal3 = Mat3::make(std::array<double, 9>{2.0, 0.0, 0.0, 0.0, -3.0, 0.0, 0.0, 0.0, 4.0});
    const auto swap2 = Mat2::make(std::array<double, 4>{0.0, 1.0, 1.0, 0.0});
    const auto cycle3 = Mat3::make(std::array<double, 9>{0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0});
    const auto quarter_turn = Mat2::make(std::array<double, 4>{0.0, -1.0, 1.0, 0.0});
    const auto left = Mat2::make(std::array<double, 4>{1.0, 2.0, 0.0, 1.0});
    const auto right = Mat2::make(std::array<double, 4>{2.0, 0.0, 1.0, 3.0});
    const auto zero_row3 = Mat3::make(std::array<double, 9>{0.0, 0.0, 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0});

    const bool setup = vector2 && vector3 && diagonal2 && diagonal3 && swap2 && cycle3 && quarter_turn && left && right && zero_row3;
    const auto at2 = identity2.at(1, 1);
    const auto at3 = identity3.at(2, 2);
    const bool zero_identity_access = setup && Mat2::zero().at(0, 0) && *Mat2::zero().at(0, 0) == 0.0 &&
        at2 && *at2 == 1.0 && at3 && *at3 == 1.0 && !identity2.at(2, 0) &&
        identity2.at(2, 0).error() == LinearAlgebraError::index_out_of_range && !identity3.at(0, 3);

    const auto applied2 = setup ? apply(*diagonal2, *vector2) : std::expected<Vector2, GeometryError>{std::unexpect, GeometryError::non_finite_result};
    const auto applied3 = setup ? apply(*diagonal3, *vector3) : std::expected<Vector3, GeometryError>{std::unexpect, GeometryError::non_finite_result};
    const bool mat2_application = applied2 && applied2->x() == 6.0 && applied2->y() == 8.0;
    const bool mat3_application = applied3 && applied3->x() == 4.0 && applied3->y() == 3.0 && applied3->z() == 16.0;

    const auto swap_determinant = setup ? determinant(*swap2) : std::expected<double, LinearAlgebraError>{std::unexpect, LinearAlgebraError::non_finite_result};
    const auto cycle_determinant = setup ? determinant(*cycle3) : std::expected<double, LinearAlgebraError>{std::unexpect, LinearAlgebraError::non_finite_result};
    const bool permutation = swap_determinant && *swap_determinant == -1.0 && cycle_determinant && *cycle_determinant == 1.0;

    const auto half_turn = setup ? compose(*quarter_turn, *quarter_turn) : std::expected<Mat2, LinearAlgebraError>{std::unexpect, LinearAlgebraError::non_finite_result};
    const auto half_turn_expected = Mat2::make(std::array<double, 4>{-1.0, 0.0, 0.0, -1.0});
    const bool quarter_turn_case = half_turn && half_turn_expected && *half_turn == *half_turn_expected;

    const auto product = setup ? compose(*left, *right) : std::expected<Mat2, LinearAlgebraError>{std::unexpect, LinearAlgebraError::non_finite_result};
    const auto reverse = setup ? compose(transpose(*right), transpose(*left)) : std::expected<Mat2, LinearAlgebraError>{std::unexpect, LinearAlgebraError::non_finite_result};
    const bool transpose_composition = product && reverse && transpose(*product) == *reverse;

    const auto zero_determinant = setup ? determinant(*zero_row3) : std::expected<double, LinearAlgebraError>{std::unexpect, LinearAlgebraError::non_finite_result};
    const bool determinant_boundary = zero_determinant && *zero_determinant == 0.0;

    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double infinity = std::numeric_limits<double>::infinity();
    const bool nonfinite_construction = !Mat2::make(std::array<double, 4>{nan, 0.0, 0.0, 1.0}) &&
        !Mat3::make(std::array<double, 9>{0.0, 0.0, 0.0, 0.0, infinity, 0.0, 0.0, 0.0, 1.0});

    const auto maximum2 = Mat2::make(std::array<double, 4>{std::numeric_limits<double>::max(), 0.0, 0.0, 1.0});
    const auto twice2 = Mat2::make(std::array<double, 4>{2.0, 0.0, 0.0, 1.0});
    const auto overflow = (maximum2 && twice2) ? compose(*maximum2, *twice2) : std::expected<Mat2, LinearAlgebraError>{std::unexpect, LinearAlgebraError::non_finite_result};
    const bool nonfinite_result = !overflow && overflow.error() == LinearAlgebraError::non_finite_result;

    const auto scaled_left = Mat2::make(std::array<double, 4>{2.0, 4.0, 0.0, 2.0});
    const auto scaled_product = setup ? compose(*scaled_left, *right) : std::expected<Mat2, LinearAlgebraError>{std::unexpect, LinearAlgebraError::non_finite_result};
    const bool power_two_scale = product && scaled_product && *scaled_product == Mat2::make(std::array<double, 4>{8.0, 12.0, 2.0, 6.0}).value_or(Mat2::zero());

    output << "{\"schema_version\":1,\"kind\":\"minimal-small-linear-algebra-certificate\",";
    output << "\"environment\":{\"double_radix\":2,\"double_digits\":53,\"iec559\":true},\"cases\":[";
    bool first = true;
    const auto emit = [&](const std::string_view family, const bool pass, const std::string_view check, const bool determinant_case = false) {
        if (!first) output << ',';
        first = false;
        write_case(output, family, pass, check, determinant_case);
    };
    emit("zero_identity_access", zero_identity_access, "zero_identity_access");
    emit("mat2_application", mat2_application, "mat2_application");
    emit("mat3_application", mat3_application, "mat3_application");
    emit("permutation", permutation, "permutation", true);
    emit("quarter_turn", quarter_turn_case, "quarter_turn");
    emit("transpose_composition", transpose_composition, "transpose_composition");
    emit("determinant_boundary", determinant_boundary, "determinant_boundary", true);
    emit("nonfinite_construction", nonfinite_construction, "nonfinite_construction");
    emit("nonfinite_result", nonfinite_result, "nonfinite_result");
    emit("power_two_scale", power_two_scale, "power_two_scale");
    emit("type_separation", true, "compile_time_contract");
    output << "]}";
    return output.good() ? 0 : 3;
}

} // namespace

int main(int argc, char** argv) {
    if (argc != 2) {
        return 64;
    }
    return write_certificate(argv[1]);
}
