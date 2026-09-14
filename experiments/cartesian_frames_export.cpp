#include "apmesh/core/geometry.hpp"

#include <array>
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
using apmesh::core::Mat2;
using apmesh::core::Mat3;
using apmesh::core::Point2;
using apmesh::core::Point3;
using apmesh::core::Vector2;
using apmesh::core::Vector3;

template <typename Frame, typename Point>
concept PointMappable = requires(const Frame& frame, const Point& point) { frame.point_to_world(point); };
static_assert(PointMappable<CartesianFrame2, Point2>);
static_assert(!PointMappable<CartesianFrame2, Point3>);
static_assert(PointMappable<CartesianFrame3, Point3>);
static_assert(!PointMappable<CartesianFrame3, Point2>);

std::string hex(const double value) {
    std::ostringstream stream; stream.imbue(std::locale::classic()); stream << std::hexfloat << value; return stream.str();
}
std::string error_name(const GeometryError error) {
    switch (error) {
    case GeometryError::non_finite_input: return "non_finite_input";
    case GeometryError::non_finite_result: return "non_finite_result";
    case GeometryError::invalid_frame: return "invalid_frame";
    case GeometryError::scale_out_of_range: return "scale_out_of_range";
    default: return "other";
    }
}
void values(std::ostream& out, const std::vector<double>& input) {
    out << '['; for (std::size_t i = 0; i < input.size(); ++i) { if (i) out << ','; out << '"' << hex(input[i]) << '"'; } out << ']';
}
template <typename Value>
std::vector<double> flat(const Value& value);
template <> std::vector<double> flat(const Point2& v) { return {v.x(), v.y()}; }
template <> std::vector<double> flat(const Vector2& v) { return {v.x(), v.y()}; }
template <> std::vector<double> flat(const Point3& v) { return {v.x(), v.y(), v.z()}; }
template <> std::vector<double> flat(const Vector3& v) { return {v.x(), v.y(), v.z()}; }
template <> std::vector<double> flat(const CartesianFrame2&) { return {}; }
template <> std::vector<double> flat(const CartesianFrame3&) { return {}; }
void write_case(std::ostream& out, bool& first, const std::string_view id, const std::vector<double>& value) {
    if (!first) out << ',';
    first = false;
    out << "{\"id\":\"" << id << "\",\"outcome\":\"value\",\"error\":null,\"value\":";
    values(out, value);
    out << '}';
}
void write_error(std::ostream& out, bool& first, const std::string_view id, const GeometryError error) {
    if (!first) out << ',';
    first = false;
    out << "{\"id\":\"" << id << "\",\"outcome\":\"error\",\"error\":\"" << error_name(error) << "\",\"value\":null}";
}
template <typename Value>
void write_result(std::ostream& out, bool& first, const std::string_view id, const std::expected<Value, GeometryError>& result) {
    if (result) write_case(out, first, id, flat(*result)); else write_error(out, first, id, result.error());
}

int export_certificate(const std::string_view destination) {
    std::ofstream out(destination.data(), std::ios::binary | std::ios::trunc);
    if (!out) return 2;
    const auto zero2 = Point2::make(0.0, 0.0); const auto zero3 = Point3::make(0.0, 0.0, 0.0);
    const auto p2 = Point2::make(1.0, 2.0); const auto v2 = Vector2::make(1.0, 2.0);
    const auto p3 = Point3::make(1.0, 2.0, 3.0); const auto v3 = Vector3::make(1.0, 2.0, 3.0);
    const auto quarter = Mat2::make(std::array<double, 4>{0.0, -1.0, 1.0, 0.0});
    const auto cycle = Mat3::make(std::array<double, 9>{0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0});
    const auto reflection = Mat2::make(std::array<double, 4>{-1.0, 0.0, 0.0, 1.0});
    const auto origin2 = Point2::make(10.0, 20.0); const auto origin3 = Point3::make(5.0, -2.0, 1.0);
    if (!zero2 || !zero3 || !p2 || !v2 || !p3 || !v3 || !quarter || !cycle || !reflection || !origin2 || !origin3) return 3;
    const auto frame2 = CartesianFrame2::make(*origin2, *quarter, 1); const auto frame3 = CartesianFrame3::make(*origin3, *cycle, -1);
    if (!frame2 || !frame3) return 4;
    out << "{\"schema_version\":1,\"kind\":\"cartesian-frames-certificate\",\"signed_zero_policy\":\"normalize_to_positive\",\"cases\":[";
    bool first = true;
    write_result(out, first, "identity2_point", CartesianFrame2::identity().point_to_world(*p2));
    write_result(out, first, "identity2_vector", CartesianFrame2::identity().vector_to_world(*v2));
    write_result(out, first, "identity3_point", CartesianFrame3::identity().point_to_world(*p3));
    write_result(out, first, "identity3_vector", CartesianFrame3::identity().vector_to_world(*v3));
    write_case(out, first, "accessor2_origin", flat(frame2->origin()));
    write_case(out, first, "accessor2_exponent", {static_cast<double>(frame2->scale_exponent())});
    write_case(out, first, "accessor3_origin", flat(frame3->origin()));
    write_case(out, first, "accessor3_exponent", {static_cast<double>(frame3->scale_exponent())});
    write_result(out, first, "quarter2_axis_x", frame2->vector_to_world(*Vector2::make(1.0, 0.0)));
    write_result(out, first, "quarter2_axis_y", frame2->vector_to_world(*Vector2::make(0.0, 1.0)));
    write_result(out, first, "quarter2_point_x", frame2->point_to_world(*Point2::make(1.0, 0.0)));
    write_result(out, first, "quarter2_point_y", frame2->point_to_world(*Point2::make(0.0, 1.0)));
    write_result(out, first, "cycle3_axis_x", frame3->vector_to_world(*Vector3::make(1.0, 0.0, 0.0)));
    write_result(out, first, "cycle3_axis_y", frame3->vector_to_world(*Vector3::make(0.0, 1.0, 0.0)));
    write_result(out, first, "cycle3_axis_z", frame3->vector_to_world(*Vector3::make(0.0, 0.0, 1.0)));
    write_result(out, first, "cycle3_point_x", frame3->point_to_world(*Point3::make(1.0, 0.0, 0.0)));
    write_result(out, first, "cycle3_point_y", frame3->point_to_world(*Point3::make(0.0, 1.0, 0.0)));
    write_result(out, first, "cycle3_point_z", frame3->point_to_world(*Point3::make(0.0, 0.0, 1.0)));
    const auto reflected = CartesianFrame2::make(*zero2, *reflection, 0);
    write_result(out, first, "reflection2_vector", reflected->vector_to_world(*v2));
    for (const int exponent : {-8, -1, 0, 1, 8}) {
        const auto scaled = CartesianFrame2::make(*zero2, Mat2::identity(), exponent);
        const std::string id = exponent < 0 ? "scale_m" + std::to_string(-exponent) : "scale_" + std::to_string(exponent);
        write_result(out, first, id, scaled->vector_to_world(*Vector2::make(1.0, 0.0)));
        write_result(out, first, id + "_point", scaled->point_to_world(*Point2::make(1.0, 0.0)));
    }
    const auto world_p2 = frame2->point_to_world(*p2); const auto world_v2 = frame2->vector_to_world(*v2);
    const auto world_p3 = frame3->point_to_world(*p3); const auto world_v3 = frame3->vector_to_world(*v3);
    write_result(out, first, "roundtrip2_point", frame2->point_to_local(*world_p2));
    write_result(out, first, "roundtrip2_vector", frame2->vector_to_local(*world_v2));
    write_result(out, first, "roundtrip3_point", frame3->point_to_local(*world_p3));
    write_result(out, first, "roundtrip3_vector", frame3->vector_to_local(*world_v3));
    write_result(out, first, "roundtrip2_reverse_point", frame2->point_to_world(*frame2->point_to_local(*world_p2)));
    write_result(out, first, "roundtrip2_reverse_vector", frame2->vector_to_world(*frame2->vector_to_local(*world_v2)));
    write_result(out, first, "roundtrip3_reverse_point", frame3->point_to_world(*frame3->point_to_local(*world_p3)));
    write_result(out, first, "roundtrip3_reverse_vector", frame3->vector_to_world(*frame3->vector_to_local(*world_v3)));
    const auto local_sum = *p2 + *v2; const auto world_sum = *world_p2 + *world_v2;
    const auto affine = local_sum && world_sum ? frame2->point_to_world(*local_sum) : std::expected<Point2, GeometryError>{std::unexpected{GeometryError::non_finite_result}};
    write_result(out, first, "affine2", affine && world_sum ? *affine - *world_sum : std::expected<Vector2, GeometryError>{std::unexpected{GeometryError::non_finite_result}});
    const auto local_difference = *p2 - *zero2; const auto mapped_difference = local_difference ? frame2->vector_to_world(*local_difference) : std::expected<Vector2, GeometryError>{std::unexpected{GeometryError::non_finite_result}};
    write_result(out, first, "difference2", mapped_difference);
    const auto scale_two = CartesianFrame2::make(*zero2, Mat2::identity(), 1); const auto unit = Vector2::make(1.0, 0.0);
    const auto metric_vector = scale_two->vector_to_world(*unit); const auto metric = metric_vector ? apmesh::core::dot(*metric_vector, *metric_vector) : std::expected<double, GeometryError>{std::unexpected{GeometryError::non_finite_result}};
    if (metric) write_case(out, first, "metric_scale2", {*metric}); else write_error(out, first, "metric_scale2", metric.error());
    const auto norm = metric_vector ? apmesh::core::norm(*metric_vector) : std::expected<double, GeometryError>{std::unexpected{GeometryError::non_finite_result}};
    if (norm) write_case(out, first, "norm_scale2", {*norm}); else write_error(out, first, "norm_scale2", norm.error());
    if (!first) out << ',';
    first = false;
    out << "{\"id\":\"compile_time_dimension_separation\",\"outcome\":\"compile_time_rejection\",\"error\":null,\"value\":[]}";
    const auto duplicate = Mat2::make(std::array<double, 4>{1.0, 0.0, 1.0, 0.0}); const auto nonunit = Mat2::make(std::array<double, 4>{2.0, 0.0, 0.0, 1.0}); const auto missing = Mat2::make(std::array<double, 4>{0.0, 0.0, 0.0, 1.0});
    const auto signed_zero = Mat2::make(std::array<double, 4>{-0.0, -1.0, 1.0, 0.0});
    write_result(out, first, "reject_duplicate_basis2", CartesianFrame2::make(*zero2, *duplicate, 0));
    write_result(out, first, "reject_nonunit_basis2", CartesianFrame2::make(*zero2, *nonunit, 0));
    write_result(out, first, "reject_missing_basis2", CartesianFrame2::make(*zero2, *missing, 0));
    write_result(out, first, "accept_signed_zero_basis2", CartesianFrame2::make(*zero2, *signed_zero, 0));
    const auto duplicate3 = Mat3::make(std::array<double, 9>{1.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0});
    const auto nonunit3 = Mat3::make(std::array<double, 9>{2.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0});
    const auto missing3 = Mat3::make(std::array<double, 9>{0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0});
    write_result(out, first, "reject_duplicate_basis3", CartesianFrame3::make(*zero3, *duplicate3, 0));
    write_result(out, first, "reject_nonunit_basis3", CartesianFrame3::make(*zero3, *nonunit3, 0));
    write_result(out, first, "reject_missing_basis3", CartesianFrame3::make(*zero3, *missing3, 0));
    if (!first) out << ',';
    first = false;
    out << "{\"id\":\"mat2_nonfinite_rejection\",\"outcome\":\"error\",\"error\":\"non_finite_input\",\"value\":null}";
    write_result(out, first, "reject_scale_high", CartesianFrame2::make(*zero2, Mat2::identity(), 1024));
    write_result(out, first, "reject_scale_low", CartesianFrame2::make(*zero2, Mat2::identity(), -1075));
    write_result(out, first, "reject_scale_nonfinite_reciprocal", CartesianFrame2::make(*zero2, Mat2::identity(), -1024));
    write_result(out, first, "accept_scale_high_boundary", CartesianFrame2::make(*zero2, Mat2::identity(), 1023));
    write_result(out, first, "accept_scale_low_boundary", CartesianFrame2::make(*zero2, Mat2::identity(), -1023));
    const auto maximum = Vector2::make(std::numeric_limits<double>::max(), 0.0);
    write_result(out, first, "overflow_vector2", scale_two->vector_to_world(*maximum));
    const auto maximum_point = Point2::make(std::numeric_limits<double>::max(), 0.0);
    write_result(out, first, "overflow_point2", scale_two->point_to_world(*maximum_point));
    out << "]}";
    return out.good() ? 0 : 5;
}
} // namespace
int main(int argc, char** argv) { return argc == 2 ? export_certificate(argv[1]) : 64; }
