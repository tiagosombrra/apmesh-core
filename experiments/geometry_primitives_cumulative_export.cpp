#include "apmesh/core/geometry.hpp"

#include <array>
#include <bit>
#include <cmath>
#include <concepts>
#include <expected>
#include <fstream>
#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

using apmesh::core::CartesianFrame2;
using apmesh::core::CartesianFrame3;
using apmesh::core::GeometryError;
using apmesh::core::LinearAlgebraError;
using apmesh::core::Mat2;
using apmesh::core::Mat3;
using apmesh::core::Point2;
using apmesh::core::Point3;
using apmesh::core::Vector2;
using apmesh::core::Vector3;

template <typename Frame, typename Point>
concept PointWorldMappable = requires(const Frame& frame, const Point& point) {
    frame.point_to_world(point);
};

template <typename Frame, typename Vector>
concept VectorWorldMappable = requires(const Frame& frame, const Vector& vector) {
    frame.vector_to_world(vector);
};

static_assert(PointWorldMappable<CartesianFrame2, Point2>);
static_assert(!PointWorldMappable<CartesianFrame2, Point3>);
static_assert(PointWorldMappable<CartesianFrame3, Point3>);
static_assert(!PointWorldMappable<CartesianFrame3, Point2>);
static_assert(VectorWorldMappable<CartesianFrame2, Vector2>);
static_assert(!VectorWorldMappable<CartesianFrame2, Vector3>);
static_assert(VectorWorldMappable<CartesianFrame3, Vector3>);
static_assert(!VectorWorldMappable<CartesianFrame3, Vector2>);

struct Result {
    std::string outcome;
    std::string error;
    std::vector<double> values;
};

std::string hex_value(const double value) {
    std::ostringstream output;
    output.imbue(std::locale::classic());
    output << std::hexfloat << value;
    return output.str();
}

void write_values(std::ostream& output, const std::vector<double>& values) {
    output << '[';
    for (std::size_t index = 0; index < values.size(); ++index) {
        if (index != 0U) output << ',';
        output << '"' << hex_value(values[index]) << '"';
    }
    output << ']';
}

void write_result(std::ostream& output, const Result& result) {
    output << "\"outcome\":\"" << result.outcome << "\",\"error\":";
    if (result.error.empty()) output << "null";
    else output << '"' << result.error << '"';
    output << ",\"values\":";
    if (result.outcome == "value") write_values(output, result.values);
    else output << "null";
}

std::string error_name(const GeometryError value) {
    switch (value) {
    case GeometryError::non_finite_input: return "non_finite_input";
    case GeometryError::non_finite_result: return "non_finite_result";
    case GeometryError::division_by_zero: return "division_by_zero";
    case GeometryError::zero_length: return "zero_length";
    case GeometryError::indeterminate: return "indeterminate";
    case GeometryError::invalid_frame: return "invalid_frame";
    case GeometryError::scale_out_of_range: return "scale_out_of_range";
    }
    return "unknown";
}

std::string error_name(const LinearAlgebraError value) {
    switch (value) {
    case LinearAlgebraError::non_finite_input: return "non_finite_input";
    case LinearAlgebraError::non_finite_result: return "non_finite_result";
    case LinearAlgebraError::index_out_of_range: return "index_out_of_range";
    }
    return "unknown";
}

std::vector<double> values(const Vector2& value) { return {value.x(), value.y()}; }
std::vector<double> values(const Vector3& value) { return {value.x(), value.y(), value.z()}; }
std::vector<double> values(const Point2& value) { return {value.x(), value.y()}; }
std::vector<double> values(const Point3& value) { return {value.x(), value.y(), value.z()}; }

template <typename Matrix, std::size_t Size>
std::vector<double> matrix_values(const Matrix& matrix) {
    std::vector<double> result;
    result.reserve(Size * Size);
    for (std::size_t row = 0; row < Size; ++row) {
        for (std::size_t column = 0; column < Size; ++column) {
            const auto entry = matrix.at(row, column);
            if (!entry) return {};
            result.push_back(*entry);
        }
    }
    return result;
}

std::vector<double> values(const Mat2& matrix) { return matrix_values<Mat2, 2>(matrix); }
std::vector<double> values(const Mat3& matrix) { return matrix_values<Mat3, 3>(matrix); }

template <typename Value>
Result observed(const std::expected<Value, GeometryError>& result) {
    if (!result) return {"error", error_name(result.error()), {}};
    return {"value", {}, values(*result)};
}

template <typename Value>
Result observed_linear(const std::expected<Value, LinearAlgebraError>& result) {
    if (!result) return {"error", error_name(result.error()), {}};
    return {"value", {}, values(*result)};
}

template <typename Frame>
Result observed_frame(const std::expected<Frame, GeometryError>& result) {
    if (!result) return {"error", error_name(result.error()), {}};
    return {"value", {}, {}};
}

void write_case(
    std::ostream& output,
    bool& first,
    const std::string_view identifier,
    const int dimension,
    const std::string_view operation,
    const Result& result) {
    if (!first) output << ',';
    first = false;
    output << "{\"id\":\"" << identifier << "\",\"dimension\":" << dimension
           << ",\"operation\":\"" << operation << "\",\"observed\":{";
    write_result(output, result);
    output << "}}";
}

int write_certificate(const std::string_view output_path) {
    std::ofstream output{output_path.data(), std::ios::binary | std::ios::trunc};
    if (!output.is_open()) return 2;

    const auto point2 = Point2::make(1.0, -2.0);
    const auto point3 = Point3::make(1.0, -2.0, 4.0);
    const auto vector2 = Vector2::make(2.0, -4.0);
    const auto vector3 = Vector3::make(2.0, -4.0, 8.0);
    const auto origin2 = Point2::make(8.0, -16.0);
    const auto origin3 = Point3::make(8.0, -16.0, 32.0);
    const auto basis2 = Mat2::make({0.0, 1.0, -1.0, 0.0});
    const auto basis3 = Mat3::make({0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0});
    const auto left2 = Mat2::make({0.0, -1.0, 1.0, 0.0});
    const auto right2 = Mat2::make({2.0, 0.0, 0.0, -1.0});
    const auto left3 = Mat3::make({0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0});
    const auto right3 = Mat3::make({2.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, 1.0});
    const auto compose2 = left2.and_then([&](const Mat2& left) { return right2.and_then([&](const Mat2& right) { return apmesh::core::compose(left, right); }); });
    const auto compose3 = left3.and_then([&](const Mat3& left) { return right3.and_then([&](const Mat3& right) { return apmesh::core::compose(left, right); }); });
    const auto zero2 = Vector2::make(-0.0, 0.0);
    const auto zero_normal_vector = Vector2::make(0.0, -0.0);
    const auto identity_frame2 = CartesianFrame2::identity();
    if (!point2 || !point3 || !vector2 || !vector3 || !origin2 || !origin3 || !basis2 || !basis3 || !left2 || !right2 || !left3 || !right3 || !compose2 || !compose3 || !zero2 || !zero_normal_vector) return 3;
    const auto frame2 = CartesianFrame2::make(*origin2, *basis2, 1);
    const auto frame3 = CartesianFrame3::make(*origin3, *basis3, 1);
    if (!frame2 || !frame3) return 3;

    const auto local2_translation = *point2 + *vector2;
    const auto local3_translation = *point3 + *vector3;
    const auto frame2_difference = local2_translation
        .and_then([&](const Point2& translated_local) { return frame2->point_to_world(translated_local); })
        .and_then([&](const Point2& translated_world) { return frame2->point_to_world(*point2).and_then([&](const Point2& world) { return translated_world - world; }); });
    const auto frame3_difference = local3_translation
        .and_then([&](const Point3& translated_local) { return frame3->point_to_world(translated_local); })
        .and_then([&](const Point3& translated_world) { return frame3->point_to_world(*point3).and_then([&](const Point3& world) { return translated_world - world; }); });
    const auto frame2_round_trip = frame2->point_to_world(*point2).and_then([&](const Point2& world) { return frame2->point_to_local(world); });
    const auto frame3_round_trip = frame3->point_to_world(*point3).and_then([&](const Point3& world) { return frame3->point_to_local(world); });
    const auto composed_apply2 = apmesh::core::apply(*compose2, *vector2);
    const auto composed_apply3 = apmesh::core::apply(*compose3, *vector3);
    const auto transpose2 = apmesh::core::compose(apmesh::core::transpose(*right2), apmesh::core::transpose(*left2));
    const auto transpose3 = apmesh::core::compose(apmesh::core::transpose(*right3), apmesh::core::transpose(*left3));
    const auto identity_point = identity_frame2.point_to_world(*point2);
    const auto signed_zero = identity_frame2.vector_to_world(*zero2);
    const auto duplicate_basis = Mat2::make({1.0, 0.0, 1.0, 0.0});
    if (!duplicate_basis) return 3;
    const auto invalid_frame = CartesianFrame2::make(*origin2, *duplicate_basis, 0);
    const auto zero_normalization = apmesh::core::normalize(*zero_normal_vector);
    const auto out_of_range = CartesianFrame2::make(*origin2, Mat2::identity(), -1075);
    const auto invalid_vector = Vector2::make(std::numeric_limits<double>::quiet_NaN(), 0.0);
    const auto overflow_matrix = Mat2::make({2.0, 0.0, 0.0, 1.0});
    const auto maximum_vector = Vector2::make(std::numeric_limits<double>::max(), 0.0);
    if (!duplicate_basis || !overflow_matrix || !maximum_vector) return 3;
    const auto overflow_apply = apmesh::core::apply(*overflow_matrix, *maximum_vector);

    output << "{\"schema_version\":1,\"kind\":\"geometry-primitives-cumulative-certificate\",\"environment\":{\"double_radix\":"
           << std::numeric_limits<double>::radix << ",\"double_digits\":" << std::numeric_limits<double>::digits
           << ",\"iec559\":" << (std::numeric_limits<double>::is_iec559 ? "true" : "false")
           << "},\"type_separation\":{\"frame2_accepts_point2\":true,\"frame2_rejects_point3\":true,\"frame3_accepts_point3\":true,\"frame3_rejects_point2\":true,\"frame2_accepts_vector2\":true,\"frame2_rejects_vector3\":true,\"frame3_accepts_vector3\":true,\"frame3_rejects_vector2\":true},\"non_claims\":[\"orientation\",\"predicate\",\"coincidence\",\"incidence\",\"topology\"],\"cases\":[";
    bool first = true;
    write_case(output, first, "frame2_affine_difference", 2, "frame_affine_difference", observed(frame2_difference));
    write_case(output, first, "frame3_affine_difference", 3, "frame_affine_difference", observed(frame3_difference));
    write_case(output, first, "frame2_point_round_trip", 2, "point_round_trip", observed(frame2_round_trip));
    write_case(output, first, "frame3_point_round_trip", 3, "point_round_trip", observed(frame3_round_trip));
    write_case(output, first, "mat2_compose_apply", 2, "compose_apply", observed(composed_apply2));
    write_case(output, first, "mat3_compose_apply", 3, "compose_apply", observed(composed_apply3));
    write_case(output, first, "mat2_transpose_composition", 2, "transpose_composition", observed_linear(transpose2));
    write_case(output, first, "mat3_transpose_composition", 3, "transpose_composition", observed_linear(transpose3));
    write_case(output, first, "frame2_identity_point", 2, "identity_point", observed(identity_point));
    write_case(output, first, "frame2_signed_zero_vector", 2, "signed_zero_vector", observed(signed_zero));
    write_case(output, first, "frame2_invalid_basis", 2, "frame_construction", observed_frame(invalid_frame));
    write_case(output, first, "vector2_zero_normalize", 2, "normalize", observed(zero_normalization));
    write_case(output, first, "frame2_scale_out_of_range", 2, "frame_construction", observed_frame(out_of_range));
    write_case(output, first, "vector2_nonfinite_construction", 2, "vector_construction", observed(invalid_vector));
    write_case(output, first, "mat2_vector_overflow", 2, "matrix_vector_application", observed(overflow_apply));
    output << "]}";
    return output.good() ? 0 : 4;
}

} // namespace

int main(int argc, char** argv) {
    if (argc != 3 || std::string_view{argv[1]} != "certificate") return 64;
    return write_certificate(argv[2]);
}
