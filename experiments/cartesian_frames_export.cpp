#include "apmesh/core/geometry.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <concepts>
#include <cstdint>
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
concept VectorLocalMappable = requires(const Frame& frame, const Vector& vector) {
    frame.vector_to_local(vector);
};

static_assert(PointWorldMappable<CartesianFrame2, Point2>);
static_assert(!PointWorldMappable<CartesianFrame2, Point3>);
static_assert(PointWorldMappable<CartesianFrame3, Point3>);
static_assert(!PointWorldMappable<CartesianFrame3, Point2>);
static_assert(VectorLocalMappable<CartesianFrame2, Vector2>);
static_assert(!VectorLocalMappable<CartesianFrame2, Vector3>);
static_assert(VectorLocalMappable<CartesianFrame3, Vector3>);
static_assert(!VectorLocalMappable<CartesianFrame3, Vector2>);

using Values = std::vector<double>;

struct Result {
    std::string outcome;
    std::string error;
    Values value;
};

std::string hex_value(const double value) {
    std::ostringstream output;
    output.imbue(std::locale::classic());
    output << std::hexfloat << value;
    return output.str();
}

void write_values(std::ostream& output, const Values& values) {
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
    output << ",\"value\":";
    if (result.outcome == "value") write_values(output, result.value);
    else output << "null";
}

Result value(Values values) { return {"value", {}, std::move(values)}; }
Result error(const std::string_view name) { return {"error", std::string{name}, {}}; }

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

Values values(const Point2& point) { return {point.x(), point.y()}; }
Values values(const Point3& point) { return {point.x(), point.y(), point.z()}; }
Values values(const Vector2& vector) { return {vector.x(), vector.y()}; }
Values values(const Vector3& vector) { return {vector.x(), vector.y(), vector.z()}; }

template <typename Value>
Result observed(const std::expected<Value, GeometryError>& input) {
    if (!input) return error(error_name(input.error()));
    return value(values(*input));
}

template <typename Frame>
Result observed_frame_construction(const std::expected<Frame, GeometryError>& input) {
    if (!input) return error(error_name(input.error()));
    return error("unexpected_frame_value");
}

template <typename Frame>
Result observed_frame_admission(const std::expected<Frame, GeometryError>& input) {
    if (!input) return error(error_name(input.error()));
    return value({});
}

template <typename Value>
Result observed_linear(const std::expected<Value, LinearAlgebraError>& input) {
    if (!input) return error(error_name(input.error()));
    return value({});
}

bool canonical_match(const Result& left, const Result& right) {
    if (left.outcome != right.outcome || left.error != right.error || left.value.size() != right.value.size()) return false;
    for (std::size_t index = 0; index < left.value.size(); ++index) {
        if (left.value[index] == 0.0 && right.value[index] == 0.0) continue;
        if (std::bit_cast<std::uint64_t>(left.value[index]) != std::bit_cast<std::uint64_t>(right.value[index])) return false;
    }
    return true;
}

template <std::size_t Dimension>
std::array<double, Dimension> direct_world(
    const std::array<double, Dimension>& origin,
    const std::array<int, Dimension>& permutation,
    const std::array<int, Dimension>& signs,
    const int exponent,
    const std::array<double, Dimension>& local,
    const bool point) {
    std::array<double, Dimension> result{};
    const double scale = std::scalbn(1.0, exponent);
    for (std::size_t row = 0; row < Dimension; ++row) {
        result[row] = (point ? origin[row] : 0.0) + scale * static_cast<double>(signs[row]) * local[permutation[row]];
    }
    return result;
}

template <std::size_t Dimension>
std::array<double, Dimension> direct_local(
    const std::array<double, Dimension>& origin,
    const std::array<int, Dimension>& permutation,
    const std::array<int, Dimension>& signs,
    const int exponent,
    const std::array<double, Dimension>& world,
    const bool point) {
    std::array<double, Dimension> result{};
    const double inverse_scale = std::scalbn(1.0, -exponent);
    for (std::size_t row = 0; row < Dimension; ++row) {
        result[permutation[row]] = inverse_scale * static_cast<double>(signs[row]) * (world[row] - (point ? origin[row] : 0.0));
    }
    return result;
}

template <std::size_t Dimension>
Values as_values(const std::array<double, Dimension>& input) {
    return Values(input.begin(), input.end());
}

template <std::size_t Dimension>
std::string basis_id(const std::array<int, Dimension>& permutation, const std::array<int, Dimension>& signs) {
    std::string result{"p"};
    for (const int entry : permutation) result += std::to_string(entry);
    result += "_s";
    for (const int entry : signs) result += entry > 0 ? "p" : "m";
    return result;
}

void write_case(
    std::ostream& output,
    bool& first,
    const std::string& id,
    const int dimension,
    const std::string_view operation,
    const Values& origin,
    const Values& basis,
    const int exponent,
    const Values& input,
    const Result& expected,
    const Result& actual) {
    if (!first) output << ',';
    first = false;
    output << "{\"schema_version\":1,\"id\":\"" << id << "\",\"dimension\":" << dimension
           << ",\"operation\":\"" << operation << "\",\"claim_category\":\"cartesian_similarity\",\"inputs\":{\"origin\":";
    write_values(output, origin);
    output << ",\"basis\":";
    write_values(output, basis);
    output << ",\"scale_exponent\":" << exponent << ",\"value\":";
    write_values(output, input);
    output << "},\"expected\":{";
    write_result(output, expected);
    output << "},\"observed\":{";
    write_result(output, actual);
    output << "},\"comparison\":{\"rule\":\"exact_hex\",\"signed_zero_policy\":\"normalize_to_positive\",\"exact_match\":"
           << (canonical_match(expected, actual) ? "true" : "false")
           << ",\"proximity_policy\":null},\"non_claims\":[\"orientation\",\"handedness\",\"rank\",\"incidence\",\"topology\",\"general_invertibility\"]}";
}

template <std::size_t Dimension>
void enumerate_bases(const auto& callback) {
    std::array<int, Dimension> permutation{};
    for (std::size_t index = 0; index < Dimension; ++index) permutation[index] = static_cast<int>(index);
    do {
        const int masks = 1 << static_cast<int>(Dimension);
        for (int mask = 0; mask < masks; ++mask) {
            std::array<int, Dimension> signs{};
            for (std::size_t index = 0; index < Dimension; ++index) signs[index] = (mask & (1 << static_cast<int>(index))) == 0 ? 1 : -1;
            callback(permutation, signs);
        }
    } while (std::next_permutation(permutation.begin(), permutation.end()));
}

void write_2d_cases(std::ostream& output, bool& first) {
    const std::array<double, 2> origin{8.0, -16.0};
    const std::array<double, 2> point{1.0, -2.0};
    const std::array<double, 2> vector{2.0, -4.0};
    const Values origin_values = as_values(origin);
    enumerate_bases<2>([&](const auto& permutation, const auto& signs) {
        std::array<double, 4> entries{};
        for (std::size_t row = 0; row < 2; ++row) entries[row * 2 + static_cast<std::size_t>(permutation[row])] = static_cast<double>(signs[row]);
        const auto basis = Mat2::make(entries);
        if (!basis) return;
        const std::string name = basis_id(permutation, signs);
        for (const int exponent : {-8, -1, 0, 1, 8}) {
            const auto frame = CartesianFrame2::make(*Point2::make(origin[0], origin[1]), *basis, exponent);
            if (!frame) return;
            const auto local_point = Point2::make(point[0], point[1]);
            const auto local_vector = Vector2::make(vector[0], vector[1]);
            const auto world_point = direct_world(origin, permutation, signs, exponent, point, true);
            const auto world_vector = direct_world(origin, permutation, signs, exponent, vector, false);
            const auto world_point_value = Point2::make(world_point[0], world_point[1]);
            const auto world_vector_value = Vector2::make(world_vector[0], world_vector[1]);
            const std::string prefix = "d2_" + name + "_k" + std::to_string(exponent) + "_";
            write_case(output, first, prefix + "point_to_world", 2, "point_to_world", origin_values, Values(entries.begin(), entries.end()), exponent, as_values(point), value(as_values(world_point)), observed(frame->point_to_world(*local_point)));
            write_case(output, first, prefix + "vector_to_world", 2, "vector_to_world", origin_values, Values(entries.begin(), entries.end()), exponent, as_values(vector), value(as_values(world_vector)), observed(frame->vector_to_world(*local_vector)));
            const auto point_l2w2l = frame->point_to_world(*local_point).and_then([&](const Point2& item) { return frame->point_to_local(item); });
            const auto vector_l2w2l = frame->vector_to_world(*local_vector).and_then([&](const Vector2& item) { return frame->vector_to_local(item); });
            const auto point_w2l2w = frame->point_to_local(*world_point_value).and_then([&](const Point2& item) { return frame->point_to_world(item); });
            const auto vector_w2l2w = frame->vector_to_local(*world_vector_value).and_then([&](const Vector2& item) { return frame->vector_to_world(item); });
            write_case(output, first, prefix + "point_local_world_round_trip", 2, "point_local_world_round_trip", origin_values, Values(entries.begin(), entries.end()), exponent, as_values(point), value(as_values(point)), observed(point_l2w2l));
            write_case(output, first, prefix + "vector_local_world_round_trip", 2, "vector_local_world_round_trip", origin_values, Values(entries.begin(), entries.end()), exponent, as_values(vector), value(as_values(vector)), observed(vector_l2w2l));
            write_case(output, first, prefix + "point_world_local_round_trip", 2, "point_world_local_round_trip", origin_values, Values(entries.begin(), entries.end()), exponent, as_values(world_point), value(as_values(world_point)), observed(point_w2l2w));
            write_case(output, first, prefix + "vector_world_local_round_trip", 2, "vector_world_local_round_trip", origin_values, Values(entries.begin(), entries.end()), exponent, as_values(world_vector), value(as_values(world_vector)), observed(vector_w2l2w));
        }
    });
}

void write_3d_cases(std::ostream& output, bool& first) {
    const std::array<double, 3> origin{8.0, -16.0, 32.0};
    const std::array<double, 3> point{1.0, -2.0, 4.0};
    const std::array<double, 3> vector{2.0, -4.0, 8.0};
    const Values origin_values = as_values(origin);
    enumerate_bases<3>([&](const auto& permutation, const auto& signs) {
        std::array<double, 9> entries{};
        for (std::size_t row = 0; row < 3; ++row) entries[row * 3 + static_cast<std::size_t>(permutation[row])] = static_cast<double>(signs[row]);
        const auto basis = Mat3::make(entries);
        if (!basis) return;
        const std::string name = basis_id(permutation, signs);
        for (const int exponent : {-8, -1, 0, 1, 8}) {
            const auto frame = CartesianFrame3::make(*Point3::make(origin[0], origin[1], origin[2]), *basis, exponent);
            if (!frame) return;
            const auto local_point = Point3::make(point[0], point[1], point[2]);
            const auto local_vector = Vector3::make(vector[0], vector[1], vector[2]);
            const auto world_point = direct_world(origin, permutation, signs, exponent, point, true);
            const auto world_vector = direct_world(origin, permutation, signs, exponent, vector, false);
            const auto world_point_value = Point3::make(world_point[0], world_point[1], world_point[2]);
            const auto world_vector_value = Vector3::make(world_vector[0], world_vector[1], world_vector[2]);
            const std::string prefix = "d3_" + name + "_k" + std::to_string(exponent) + "_";
            write_case(output, first, prefix + "point_to_world", 3, "point_to_world", origin_values, Values(entries.begin(), entries.end()), exponent, as_values(point), value(as_values(world_point)), observed(frame->point_to_world(*local_point)));
            write_case(output, first, prefix + "vector_to_world", 3, "vector_to_world", origin_values, Values(entries.begin(), entries.end()), exponent, as_values(vector), value(as_values(world_vector)), observed(frame->vector_to_world(*local_vector)));
            const auto point_l2w2l = frame->point_to_world(*local_point).and_then([&](const Point3& item) { return frame->point_to_local(item); });
            const auto vector_l2w2l = frame->vector_to_world(*local_vector).and_then([&](const Vector3& item) { return frame->vector_to_local(item); });
            const auto point_w2l2w = frame->point_to_local(*world_point_value).and_then([&](const Point3& item) { return frame->point_to_world(item); });
            const auto vector_w2l2w = frame->vector_to_local(*world_vector_value).and_then([&](const Vector3& item) { return frame->vector_to_world(item); });
            write_case(output, first, prefix + "point_local_world_round_trip", 3, "point_local_world_round_trip", origin_values, Values(entries.begin(), entries.end()), exponent, as_values(point), value(as_values(point)), observed(point_l2w2l));
            write_case(output, first, prefix + "vector_local_world_round_trip", 3, "vector_local_world_round_trip", origin_values, Values(entries.begin(), entries.end()), exponent, as_values(vector), value(as_values(vector)), observed(vector_l2w2l));
            write_case(output, first, prefix + "point_world_local_round_trip", 3, "point_world_local_round_trip", origin_values, Values(entries.begin(), entries.end()), exponent, as_values(world_point), value(as_values(world_point)), observed(point_w2l2w));
            write_case(output, first, prefix + "vector_world_local_round_trip", 3, "vector_world_local_round_trip", origin_values, Values(entries.begin(), entries.end()), exponent, as_values(world_vector), value(as_values(world_vector)), observed(vector_w2l2w));
        }
    });
}

void write_adversarial_cases(std::ostream& output, bool& first) {
    const auto zero2 = Point2::make(0.0, 0.0);
    const auto zero3 = Point3::make(0.0, 0.0, 0.0);
    const Values empty{};
    const Values zero_values2{0.0, 0.0};
    const Values zero_values3{0.0, 0.0, 0.0};
    const Values identity_values2{1.0, 0.0, 0.0, 1.0};
    const Values identity_values3{1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0};
    const auto duplicate2 = Mat2::make({1.0, 0.0, 1.0, 0.0});
    const auto duplicate3 = Mat3::make({1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0});
    const auto nonunit2 = Mat2::make({2.0, 0.0, 0.0, 1.0});
    const auto missing3 = Mat3::make({0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0});
    write_case(output, first, "frame2_duplicate_basis", 2, "frame_construction", zero_values2, Values{1.0, 0.0, 1.0, 0.0}, 0, empty, error("invalid_frame"), observed_frame_construction(CartesianFrame2::make(*zero2, *duplicate2, 0)));
    write_case(output, first, "frame3_duplicate_basis", 3, "frame_construction", zero_values3, Values{1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0}, 0, empty, error("invalid_frame"), observed_frame_construction(CartesianFrame3::make(*zero3, *duplicate3, 0)));
    write_case(output, first, "frame2_non_unit_basis", 2, "frame_construction", zero_values2, Values{2.0, 0.0, 0.0, 1.0}, 0, empty, error("invalid_frame"), observed_frame_construction(CartesianFrame2::make(*zero2, *nonunit2, 0)));
    write_case(output, first, "frame3_missing_basis", 3, "frame_construction", zero_values3, Values{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0}, 0, empty, error("invalid_frame"), observed_frame_construction(CartesianFrame3::make(*zero3, *missing3, 0)));
    write_case(output, first, "point2_nan", 2, "point_construction", zero_values2, identity_values2, 0, Values{std::numeric_limits<double>::quiet_NaN(), 0.0}, error("non_finite_input"), observed(Point2::make(std::numeric_limits<double>::quiet_NaN(), 0.0)));
    write_case(output, first, "point3_nan", 3, "point_construction", zero_values3, identity_values3, 0, Values{std::numeric_limits<double>::quiet_NaN(), 0.0, 0.0}, error("non_finite_input"), observed(Point3::make(std::numeric_limits<double>::quiet_NaN(), 0.0, 0.0)));
    const auto mat2_nan = Mat2::make({std::numeric_limits<double>::quiet_NaN(), 0.0, 0.0, 1.0});
    const auto mat3_nan = Mat3::make({std::numeric_limits<double>::quiet_NaN(), 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0});
    write_case(output, first, "mat2_nan", 2, "basis_construction", zero_values2, Values{std::numeric_limits<double>::quiet_NaN(), 0.0, 0.0, 1.0}, 0, empty, error("non_finite_input"), observed_linear(mat2_nan));
    write_case(output, first, "mat3_nan", 3, "basis_construction", zero_values3, Values{std::numeric_limits<double>::quiet_NaN(), 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0}, 0, empty, error("non_finite_input"), observed_linear(mat3_nan));
    const Mat2 identity2 = Mat2::identity(); const Mat3 identity3 = Mat3::identity();
    write_case(output, first, "frame2_scale_m1023", 2, "frame_construction", zero_values2, identity_values2, -1023, empty, value({}), observed_frame_admission(CartesianFrame2::make(*zero2, identity2, -1023)));
    write_case(output, first, "frame3_scale_p1023", 3, "frame_construction", zero_values3, identity_values3, 1023, empty, value({}), observed_frame_admission(CartesianFrame3::make(*zero3, identity3, 1023)));
    write_case(output, first, "frame2_scale_m1075", 2, "frame_construction", zero_values2, identity_values2, -1075, empty, error("scale_out_of_range"), observed_frame_construction(CartesianFrame2::make(*zero2, identity2, -1075)));
    write_case(output, first, "frame2_scale_m1024", 2, "frame_construction", zero_values2, identity_values2, -1024, empty, error("scale_out_of_range"), observed_frame_construction(CartesianFrame2::make(*zero2, identity2, -1024)));
    write_case(output, first, "frame2_scale_p1024", 2, "frame_construction", zero_values2, identity_values2, 1024, empty, error("scale_out_of_range"), observed_frame_construction(CartesianFrame2::make(*zero2, identity2, 1024)));
    write_case(output, first, "frame3_scale_int_min", 3, "frame_construction", zero_values3, identity_values3, std::numeric_limits<int>::min(), empty, error("scale_out_of_range"), observed_frame_construction(CartesianFrame3::make(*zero3, identity3, std::numeric_limits<int>::min())));
    const auto twice2 = CartesianFrame2::make(*zero2, identity2, 1);
    const auto twice3 = CartesianFrame3::make(*zero3, identity3, 1);
    const auto maximum_point = Point2::make(std::numeric_limits<double>::max(), 0.0);
    const auto maximum_vector = Vector3::make(std::numeric_limits<double>::max(), 0.0, 0.0);
    write_case(output, first, "frame2_overflow_point", 2, "point_to_world", zero_values2, identity_values2, 1, values(*maximum_point), error("non_finite_result"), observed(twice2->point_to_world(*maximum_point)));
    write_case(output, first, "frame3_overflow_vector", 3, "vector_to_world", zero_values3, identity_values3, 1, values(*maximum_vector), error("non_finite_result"), observed(twice3->vector_to_world(*maximum_vector)));
}

int write_certificate(const std::string_view output_path) {
    std::ofstream output{output_path.data(), std::ios::binary | std::ios::trunc};
    if (!output.is_open()) return 2;
    output << "{\"schema_version\":1,\"kind\":\"cartesian-frames-certificate\",\"environment\":{\"double_radix\":" << std::numeric_limits<double>::radix
           << ",\"double_digits\":" << std::numeric_limits<double>::digits << ",\"iec559\":" << (std::numeric_limits<double>::is_iec559 ? "true" : "false") << "},\"cases\":[";
    bool first = true;
    write_2d_cases(output, first);
    write_3d_cases(output, first);
    write_adversarial_cases(output, first);
    output << "]}";
    return output.good() ? 0 : 3;
}

} // namespace

int main(int argc, char** argv) {
    if (argc != 3 || std::string_view{argv[1]} != "certificate") return 64;
    return write_certificate(argv[2]);
}
