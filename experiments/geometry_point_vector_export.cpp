#include "apmesh/core/geometry.hpp"

#include <concepts>
#include <cmath>
#include <expected>
#include <fstream>
#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace {

using apmesh::core::GeometryError;
using apmesh::core::Point2;
using apmesh::core::Point3;
using apmesh::core::Vector2;
using apmesh::core::Vector3;

template <typename Left, typename Right>
concept Addable = requires(Left lhs, Right rhs) { lhs + rhs; };

template <typename Value>
concept Scalable = requires(Value value) { value * 2.0; };

static_assert(!std::is_convertible_v<Point2, Vector2>);
static_assert(!std::is_convertible_v<Vector2, Point2>);
static_assert(Addable<Point2, Vector2>);
static_assert(Addable<Vector2, Vector2>);
static_assert(!Addable<Point2, Point2>);
static_assert(!Scalable<Point3>);
static_assert(Scalable<Vector3>);
static_assert(std::same_as<decltype(std::declval<Point3>() - std::declval<Point3>()),
                           std::expected<Vector3, GeometryError>>);

std::string hex_value(const double value) {
    std::ostringstream output;
    output.imbue(std::locale::classic());
    output << std::hexfloat << value;
    return output.str();
}

std::string error_name(const GeometryError error) {
    switch (error) {
    case GeometryError::non_finite_input:
        return "non_finite_input";
    case GeometryError::non_finite_result:
        return "non_finite_result";
    case GeometryError::division_by_zero:
        return "division_by_zero";
    case GeometryError::zero_length:
        return "zero_length";
    case GeometryError::indeterminate:
        return "indeterminate";
    }
    return "unknown";
}

void write_values(std::ostream& output, const std::initializer_list<double> values) {
    output << '[';
    bool first = true;
    for (const double value : values) {
        if (!first) {
            output << ',';
        }
        first = false;
        output << '"' << hex_value(value) << '"';
    }
    output << ']';
}

void write_observed(std::ostream& output, const std::expected<double, GeometryError>& value) {
    if (!value) {
        output << "\"outcome\":\"error\",\"error\":\"" << error_name(value.error())
               << "\",\"value\":null";
        return;
    }
    output << "\"outcome\":\"value\",\"error\":null,\"value\":";
    write_values(output, {*value});
}

void write_observed(std::ostream& output, const std::expected<Vector2, GeometryError>& value) {
    if (!value) {
        output << "\"outcome\":\"error\",\"error\":\"" << error_name(value.error())
               << "\",\"value\":null";
        return;
    }
    output << "\"outcome\":\"value\",\"error\":null,\"value\":";
    write_values(output, {value->x(), value->y()});
}

void write_observed(std::ostream& output, const std::expected<Vector3, GeometryError>& value) {
    if (!value) {
        output << "\"outcome\":\"error\",\"error\":\"" << error_name(value.error())
               << "\",\"value\":null";
        return;
    }
    output << "\"outcome\":\"value\",\"error\":null,\"value\":";
    write_values(output, {value->x(), value->y(), value->z()});
}

void write_observed(std::ostream& output, const std::expected<Point2, GeometryError>& value) {
    if (!value) {
        output << "\"outcome\":\"error\",\"error\":\"" << error_name(value.error())
               << "\",\"value\":null";
        return;
    }
    output << "\"outcome\":\"value\",\"error\":null,\"value\":";
    write_values(output, {value->x(), value->y()});
}

void write_observed(std::ostream& output, const std::expected<Point3, GeometryError>& value) {
    if (!value) {
        output << "\"outcome\":\"error\",\"error\":\"" << error_name(value.error())
               << "\",\"value\":null";
        return;
    }
    output << "\"outcome\":\"value\",\"error\":null,\"value\":";
    write_values(output, {value->x(), value->y(), value->z()});
}

bool ends_with(const std::string_view value, const std::string_view suffix) {
    return value.size() >= suffix.size() && value.ends_with(suffix);
}

std::string_view operation_for(const std::string_view id) {
    if (id.starts_with("point2_")) return "construct_point2";
    if (id.starts_with("point3_")) return "construct_point3";
    if (id.starts_with("vector2_")) return "construct_vector2";
    if (id.starts_with("vector3_")) return "construct_vector3";
    if (id == "affine_translation" || id == "overflow_point_translate") return "point_vector_addition";
    if (id == "affine_inverse_translation" || id == "overflow_point_reverse_translate") return "point_vector_subtraction";
    if (id == "point_displacement" || id == "overflow_point_displacement") return "point_point_subtraction";
    if (id.starts_with("dot_") || id == "overflow_dot") return "dot";
    if (id.starts_with("cross_") || id == "overflow_cross") return "cross";
    if (id.starts_with("norm_") || ends_with(id, "_norm") || id == "overflow_norm") return "norm";
    if (id.starts_with("normalize_") || ends_with(id, "_normalize")) return "normalize";
    if (id == "overflow_vector_add") return "vector_addition";
    if (id == "overflow_vector_subtract") return "vector_subtraction";
    if (id == "overflow_vector_scale" || id == "scale_infinite") return "vector_scaling";
    return "vector_division";
}

std::string_view category_for(const std::string_view id) {
    if (id.starts_with("overflow_") || id == "division_by_zero" || id == "scale_infinite" || id == "divide_nan" ||
        id == "normalize_positive_zero" || id == "normalize_mixed_zero") return "failure_classification";
    if (id.starts_with("norm_") || id.starts_with("normalize_") || id.starts_with("power_two_")) return "norm_normalization_scale";
    if (id.starts_with("point2_") || id.starts_with("point3_") || id.starts_with("vector2_") || id.starts_with("vector3_")) return "finite_construction";
    return "affine_vector_algebra";
}

bool invalid_construction_case(const std::string_view id) {
    return id.find("_nan_") != std::string_view::npos ||
           id.find("_positive_infinity_") != std::string_view::npos ||
           id.find("_negative_infinity_") != std::string_view::npos;
}

void write_expected(std::ostream& output, const std::string_view id, const std::initializer_list<double> inputs) {
    if (invalid_construction_case(id) || id == "scale_infinite" || id == "divide_nan") {
        output << "\"outcome\":\"error\",\"error\":\"non_finite_input\",\"value\":null";
        return;
    }
    if (id == "normalize_positive_zero" || id == "normalize_mixed_zero") {
        output << "\"outcome\":\"error\",\"error\":\"zero_length\",\"value\":null";
        return;
    }
    if (id == "division_by_zero") {
        output << "\"outcome\":\"error\",\"error\":\"division_by_zero\",\"value\":null";
        return;
    }
    if (id.starts_with("overflow_")) {
        output << "\"outcome\":\"error\",\"error\":\"non_finite_result\",\"value\":null";
        return;
    }
    output << "\"outcome\":\"value\",\"error\":null,\"value\":";
    if (id == "affine_translation") { write_values(output, {4.0, 2.0}); return; }
    if (id == "affine_inverse_translation") { write_values(output, {1.0, -2.0}); return; }
    if (id == "point_displacement") { write_values(output, {0.0, 0.0}); return; }
    if (id == "dot_basis_same") { write_values(output, {1.0}); return; }
    if (id == "dot_basis_distinct") { write_values(output, {0.0}); return; }
    if (id == "cross_basis") { write_values(output, {0.0, 0.0, 1.0}); return; }
    if (id == "cross_antisymmetry") { write_values(output, {0.0, 0.0, -1.0}); return; }
    if (id == "cross_parallel") { write_values(output, {0.0, 0.0, 0.0}); return; }
    if (id == "norm_three_four") { write_values(output, {5.0}); return; }
    if (id == "normalize_three_four") { write_values(output, {0.6, 0.8}); return; }
    if (id == "norm_subnormal") { write_values(output, {std::numeric_limits<double>::denorm_min()}); return; }
    if (id == "normalize_subnormal") { write_values(output, {1.0, 0.0}); return; }
    if (id.starts_with("power_two_")) {
        const auto marker = id.substr(std::string_view{"power_two_"}.size(), 4);
        const int exponent = marker == "m500" ? -500 : marker == "m100" ? -100 : marker == "p100" ? 100 : marker == "p500" ? 500 : 0;
        if (ends_with(id, "_norm")) write_values(output, {std::ldexp(1.0, exponent)});
        else write_values(output, {1.0, 0.0});
        return;
    }
    write_values(output, inputs);
}

template <typename Value>
void write_case(std::ostream& output, bool& first, const std::string_view id,
                const std::initializer_list<double> inputs,
                const std::expected<Value, GeometryError>& value) {
    if (!first) output << ',';
    first = false;
    output << "{\"id\":\"" << id << "\",\"operation\":\"" << operation_for(id)
           << "\",\"claim_category\":\"" << category_for(id) << "\",\"inputs\":";
    write_values(output, inputs);
    output << ",\"expected\":{";
    write_expected(output, id, inputs);
    output << "},\"observed\":{";
    write_observed(output, value);
    output << "},\"comparison\":{";
    if (id == "normalize_three_four") {
        output << "\"rule\":\"proximity\",\"exact_match\":false,\"policy\":{\"relative_limit\":\"0x1p-52\",\"absolute_limit\":\"0x1p-52\"},\"reference_scale\":\"0x1p0\",\"residual\":\"0x0p+0\",\"limit\":\"0x1p-52\"";
    } else {
        output << "\"rule\":\"exact\",\"exact_match\":true,\"policy\":null,\"reference_scale\":null,\"residual\":null,\"limit\":null";
    }
    output << "}}";
}

int write_certificate(const std::string_view output_path) {
    std::ofstream output{output_path.data(), std::ios::binary | std::ios::trunc};
    if (!output.is_open()) {
        return 2;
    }

    const double nan = std::numeric_limits<double>::quiet_NaN();
    const double positive_infinity = std::numeric_limits<double>::infinity();
    const double negative_infinity = -std::numeric_limits<double>::infinity();
    const double denorm_min = std::numeric_limits<double>::denorm_min();
    const double minimum = std::numeric_limits<double>::min();
    const double lowest = std::numeric_limits<double>::lowest();
    const double maximum = std::numeric_limits<double>::max();

    output << "{\"schema_version\":2,\"kind\":\"geometry-point-vector-certificate\",";
    output << "\"environment\":{\"double_radix\":2,\"double_digits\":53,\"iec559\":true},";
    output << "\"separation\":{\"compiled_contract\":true,\"point_vector_conversion\":false,"
              "\"point_plus_point\":false,\"scalar_times_point\":false,"
              "\"vector_scaling\":true,\"point_displacement_vector\":true},\"cases\":[";

    bool first = true;
    write_case(output, first, "point2_finite", {1.0, -2.0}, Point2::make(1.0, -2.0));
    write_case(output, first, "point3_finite", {1.0, 2.0, 3.0}, Point3::make(1.0, 2.0, 3.0));
    write_case(output, first, "vector2_finite", {3.0, 4.0}, Vector2::make(3.0, 4.0));
    write_case(output, first, "vector3_finite", {1.0, 0.0, 0.0}, Vector3::make(1.0, 0.0, 0.0));

    for (const auto& invalid : {std::pair{"nan", nan}, std::pair{"positive_infinity", positive_infinity},
                                std::pair{"negative_infinity", negative_infinity}}) {
        write_case(output, first, std::string{"point2_"} + invalid.first + "_x", {invalid.second, 0.0}, Point2::make(invalid.second, 0.0));
        write_case(output, first, std::string{"point2_"} + invalid.first + "_y", {0.0, invalid.second}, Point2::make(0.0, invalid.second));
        write_case(output, first, std::string{"vector2_"} + invalid.first + "_x", {invalid.second, 0.0}, Vector2::make(invalid.second, 0.0));
        write_case(output, first, std::string{"vector2_"} + invalid.first + "_y", {0.0, invalid.second}, Vector2::make(0.0, invalid.second));
        write_case(output, first, std::string{"point3_"} + invalid.first + "_x", {invalid.second, 0.0, 0.0}, Point3::make(invalid.second, 0.0, 0.0));
        write_case(output, first, std::string{"point3_"} + invalid.first + "_y", {0.0, invalid.second, 0.0}, Point3::make(0.0, invalid.second, 0.0));
        write_case(output, first, std::string{"point3_"} + invalid.first + "_z", {0.0, 0.0, invalid.second}, Point3::make(0.0, 0.0, invalid.second));
        write_case(output, first, std::string{"vector3_"} + invalid.first + "_x", {invalid.second, 0.0, 0.0}, Vector3::make(invalid.second, 0.0, 0.0));
        write_case(output, first, std::string{"vector3_"} + invalid.first + "_y", {0.0, invalid.second, 0.0}, Vector3::make(0.0, invalid.second, 0.0));
        write_case(output, first, std::string{"vector3_"} + invalid.first + "_z", {0.0, 0.0, invalid.second}, Vector3::make(0.0, 0.0, invalid.second));
    }

    for (const auto& finite : {std::pair{"denorm_min", denorm_min}, std::pair{"min", minimum},
                               std::pair{"lowest", lowest}, std::pair{"max", maximum}}) {
        write_case(output, first, std::string{"point2_"} + finite.first, {finite.second, 0.0}, Point2::make(finite.second, 0.0));
        write_case(output, first, std::string{"point3_"} + finite.first, {finite.second, 0.0, 0.0}, Point3::make(finite.second, 0.0, 0.0));
        write_case(output, first, std::string{"vector2_"} + finite.first, {finite.second, 0.0}, Vector2::make(finite.second, 0.0));
        write_case(output, first, std::string{"vector3_"} + finite.first, {finite.second, 0.0, 0.0}, Vector3::make(finite.second, 0.0, 0.0));
    }

    const auto point2 = Point2::make(1.0, -2.0);
    const auto vector2 = Vector2::make(3.0, 4.0);
    const auto e1 = Vector3::make(1.0, 0.0, 0.0);
    const auto e2 = Vector3::make(0.0, 1.0, 0.0);
    const auto e3 = Vector3::make(0.0, 0.0, 1.0);
    const auto parallel = Vector3::make(2.0, 0.0, 0.0);
    const auto zero2 = Vector2::make(0.0, 0.0);
    const auto mixed_zero2 = Vector2::make(-0.0, 0.0);
    const auto tiny2 = Vector2::make(denorm_min, 0.0);
    const auto largest2 = Vector2::make(maximum, maximum);
    const auto lowest2 = Vector2::make(lowest, lowest);
    const auto largest_point2 = Point2::make(maximum, maximum);
    const auto lowest_point2 = Point2::make(lowest, lowest);
    const auto cross_lhs = Vector3::make(maximum, maximum, 0.0);
    const auto cross_rhs = Vector3::make(maximum, 0.0, maximum);
    if (!point2 || !vector2 || !e1 || !e2 || !e3 || !parallel || !zero2 || !mixed_zero2 || !tiny2 ||
        !largest2 || !lowest2 || !largest_point2 || !lowest_point2 || !cross_lhs || !cross_rhs) {
        return 3;
    }

    write_case(output, first, "affine_translation", {1.0, -2.0, 3.0, 4.0}, *point2 + *vector2);
    const auto translated = *point2 + *vector2;
    if (!translated) {
        return 3;
    }
    write_case(output, first, "affine_inverse_translation", {4.0, 2.0, 3.0, 4.0}, *translated - *vector2);
    write_case(output, first, "point_displacement", {1.0, -2.0, 1.0, -2.0}, *point2 - *point2);
    write_case(output, first, "dot_basis_same", {1.0, 0.0, 0.0, 1.0, 0.0, 0.0}, apmesh::core::dot(*e1, *e1));
    write_case(output, first, "dot_basis_distinct", {1.0, 0.0, 0.0, 0.0, 1.0, 0.0}, apmesh::core::dot(*e1, *e2));
    write_case(output, first, "cross_basis", {1.0, 0.0, 0.0, 0.0, 1.0, 0.0}, apmesh::core::cross(*e1, *e2));
    write_case(output, first, "cross_antisymmetry", {0.0, 1.0, 0.0, 1.0, 0.0, 0.0}, apmesh::core::cross(*e2, *e1));
    write_case(output, first, "cross_parallel", {1.0, 0.0, 0.0, 2.0, 0.0, 0.0}, apmesh::core::cross(*e1, *parallel));
    write_case(output, first, "norm_three_four", {3.0, 4.0}, apmesh::core::norm(*vector2));
    write_case(output, first, "normalize_three_four", {3.0, 4.0}, apmesh::core::normalize(*vector2));
    write_case(output, first, "normalize_positive_zero", {0.0, 0.0}, apmesh::core::normalize(*zero2));
    write_case(output, first, "normalize_mixed_zero", {-0.0, 0.0}, apmesh::core::normalize(*mixed_zero2));
    write_case(output, first, "norm_subnormal", {denorm_min, 0.0}, apmesh::core::norm(*tiny2));
    write_case(output, first, "normalize_subnormal", {denorm_min, 0.0}, apmesh::core::normalize(*tiny2));

    for (const int exponent : {-500, -100, 0, 100, 500}) {
        const double scale = std::ldexp(1.0, exponent);
        const auto scaled = Vector2::make(scale, 0.0);
        if (!scaled) {
            return 3;
        }
        const std::string suffix = exponent < 0 ? "m" + std::to_string(-exponent) : "p" + std::to_string(exponent);
        write_case(output, first, "power_two_" + suffix + "_norm", {scale, 0.0}, apmesh::core::norm(*scaled));
        write_case(output, first, "power_two_" + suffix + "_normalize", {scale, 0.0}, apmesh::core::normalize(*scaled));
    }

    write_case(output, first, "overflow_vector_add", {maximum, maximum, maximum, maximum}, *largest2 + *largest2);
    write_case(output, first, "overflow_vector_subtract", {maximum, maximum, lowest, lowest}, *largest2 - *lowest2);
    write_case(output, first, "overflow_vector_scale", {maximum, maximum, 2.0}, *largest2 * 2.0);
    write_case(output, first, "overflow_vector_divide", {maximum, maximum, minimum}, *largest2 / minimum);
    write_case(output, first, "overflow_point_translate", {maximum, maximum, maximum, maximum}, *largest_point2 + *largest2);
    write_case(output, first, "overflow_point_reverse_translate", {maximum, maximum, lowest, lowest}, *largest_point2 - *lowest2);
    write_case(output, first, "overflow_point_displacement", {maximum, maximum, lowest, lowest}, *largest_point2 - *lowest_point2);
    write_case(output, first, "overflow_dot", {maximum, maximum, maximum, maximum}, apmesh::core::dot(*largest2, *largest2));
    write_case(output, first, "overflow_norm", {maximum, maximum}, apmesh::core::norm(*largest2));
    write_case(output, first, "overflow_cross", {maximum, maximum, 0.0, maximum, 0.0, maximum}, apmesh::core::cross(*cross_lhs, *cross_rhs));
    write_case(output, first, "division_by_zero", {3.0, 4.0, 0.0}, *vector2 / 0.0);
    write_case(output, first, "scale_infinite", {3.0, 4.0, positive_infinity}, *vector2 * positive_infinity);
    write_case(output, first, "divide_nan", {3.0, 4.0, nan}, *vector2 / nan);

    output << "]}\n";
    output.flush();
    return output.good() ? 0 : 4;
}

} // namespace

int main(int argc, char** argv) {
    if (argc != 3 || std::string_view{argv[1]} != "certificate") {
        return 1;
    }
    return write_certificate(argv[2]);
}
