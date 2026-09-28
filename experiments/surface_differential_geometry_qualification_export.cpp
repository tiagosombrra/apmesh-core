#include "apmesh/core/geometry.hpp"
#include "apmesh/geometry/coons_surface.hpp"
#include "apmesh/geometry/curve.hpp"
#include "apmesh/geometry/elementary_surface.hpp"
#include "apmesh/geometry/extrusion_surface.hpp"
#include "apmesh/geometry/nurbs_surface.hpp"
#include "apmesh/geometry/revolution_surface.hpp"
#include "apmesh/geometry/surface.hpp"
#include "apmesh/geometry/surface_differential.hpp"
#include "apmesh/geometry/trimmed_surface.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <expected>
#include <fstream>
#include <iomanip>
#include <limits>
#include <locale>
#include <numbers>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace {

using apmesh::core::AxisPlacement3;
using apmesh::core::BicubicBezierPatch3;
using apmesh::core::BicubicNURBSSurface3;
using apmesh::core::BoundedCylinderSurface3;
using apmesh::core::BoundedParametricSurface3;
using apmesh::core::BoundedPlaneSurface3;
using apmesh::core::BoundedSphereSurface3;
using apmesh::core::CurveParameterDomain;
using apmesh::core::CubicBezier3;
using apmesh::core::Point3;
using apmesh::core::RationalBicubicBezierPatch3;
using apmesh::core::SurfaceDifferentialError;
using apmesh::core::SurfaceError;
using apmesh::core::SurfaceFirstDerivatives3;
using apmesh::core::SurfaceMetricNormal3;
using apmesh::core::SurfaceSecondDerivatives3;
using apmesh::core::Vector3;

struct Fields {
    std::optional<double> e;
    std::optional<double> f;
    std::optional<double> g;
    std::optional<double> area_density;
    std::optional<double> nx;
    std::optional<double> ny;
    std::optional<double> nz;
    std::optional<double> condition_number;
    std::optional<double> l;
    std::optional<double> m;
    std::optional<double> n;
    std::optional<double> gaussian_curvature;
    std::optional<double> mean_curvature;
    std::optional<double> maximum_curvature;
    std::optional<double> minimum_curvature;
    std::optional<bool> is_umbilic;
};

struct ResultRecord {
    std::string outcome{"value"};
    std::optional<std::string> error;
    Fields fields;
};

struct CaseRecord {
    std::string id;
    std::string category;
    std::string comparison_rule;
    std::optional<std::string> policy;
    ResultRecord reference;
    ResultRecord observed;
    std::string observations{"{}"};
};

std::string hex_value(const double value) {
    std::ostringstream output;
    output.imbue(std::locale::classic());
    output << std::hexfloat << value;
    return output.str();
}

std::string error_name(const SurfaceDifferentialError error) {
    switch (error) {
    case SurfaceDifferentialError::non_finite_u_parameter:
        return "non_finite_u_parameter";
    case SurfaceDifferentialError::non_finite_v_parameter:
        return "non_finite_v_parameter";
    case SurfaceDifferentialError::u_parameter_out_of_domain:
        return "u_parameter_out_of_domain";
    case SurfaceDifferentialError::v_parameter_out_of_domain:
        return "v_parameter_out_of_domain";
    case SurfaceDifferentialError::insufficient_continuity:
        return "insufficient_continuity";
    case SurfaceDifferentialError::singular_parameterization:
        return "singular_parameterization";
    case SurfaceDifferentialError::non_representable_result:
        return "non_representable_result";
    }
    return "unknown";
}

std::string error_name(const SurfaceError error) {
    switch (error) {
    case SurfaceError::non_finite_u_parameter:
        return "non_finite_u_parameter";
    case SurfaceError::non_finite_v_parameter:
        return "non_finite_v_parameter";
    case SurfaceError::u_parameter_out_of_domain:
        return "u_parameter_out_of_domain";
    case SurfaceError::v_parameter_out_of_domain:
        return "v_parameter_out_of_domain";
    case SurfaceError::non_finite_result:
        return "non_representable_result";
    case SurfaceError::insufficient_continuity:
        return "insufficient_continuity";
    }
    return "unknown";
}

ResultRecord error_result(const SurfaceDifferentialError error) {
    return ResultRecord{
        .outcome = "error",
        .error = error_name(error),
        .fields = {},
    };
}

ResultRecord error_result(const std::string_view error) {
    return ResultRecord{
        .outcome = "error",
        .error = std::string{error},
        .fields = {},
    };
}

Fields metric_fields(
    const SurfaceMetricNormal3& metric,
    const std::optional<double> condition = std::nullopt) {
    return Fields{
        .e = metric.first_fundamental_form.e,
        .f = metric.first_fundamental_form.f,
        .g = metric.first_fundamental_form.g,
        .area_density = metric.area_density,
        .nx = metric.unit_normal.x(),
        .ny = metric.unit_normal.y(),
        .nz = metric.unit_normal.z(),
        .condition_number = condition,
    };
}

ResultRecord metric_result(
    const SurfaceFirstDerivatives3& first,
    const bool include_conditioning) {
    const auto metric = apmesh::core::surface_metric_normal(first);
    if (!metric) {
        return error_result(metric.error());
    }
    std::optional<double> condition;
    if (include_conditioning) {
        const auto conditioning =
            apmesh::core::surface_metric_conditioning(*metric);
        if (!conditioning) {
            return error_result(conditioning.error());
        }
        condition = conditioning->condition_number;
    }
    return ResultRecord{
        .outcome = "value",
        .error = std::nullopt,
        .fields = metric_fields(*metric, condition),
    };
}

ResultRecord second_order_result(
    const SurfaceFirstDerivatives3& first,
    const SurfaceSecondDerivatives3& second,
    const bool include_principal) {
    const auto geometry =
        apmesh::core::surface_second_order_geometry(first, second);
    if (!geometry) {
        return error_result(geometry.error());
    }
    const auto conditioning =
        apmesh::core::surface_metric_conditioning(geometry->metric_normal);
    if (!conditioning) {
        return error_result(conditioning.error());
    }
    Fields fields =
        metric_fields(geometry->metric_normal, conditioning->condition_number);
    fields.l = geometry->second_fundamental_form.l;
    fields.m = geometry->second_fundamental_form.m;
    fields.n = geometry->second_fundamental_form.n;
    fields.gaussian_curvature = geometry->gaussian_curvature;
    fields.mean_curvature = geometry->mean_curvature;
    if (include_principal) {
        const auto principal =
            apmesh::core::surface_principal_curvatures(*geometry);
        if (!principal) {
            return error_result(principal.error());
        }
        fields.maximum_curvature = principal->maximum_curvature;
        fields.minimum_curvature = principal->minimum_curvature;
        fields.is_umbilic = principal->is_umbilic;
    }
    return ResultRecord{
        .outcome = "value",
        .error = std::nullopt,
        .fields = fields,
    };
}

template <BoundedParametricSurface3 Surface>
ResultRecord surface_result(
    const Surface& surface,
    const double u,
    const double v,
    const bool include_principal = true) {
    const auto geometry =
        apmesh::core::surface_second_order_geometry(surface, u, v);
    if (!geometry) {
        return error_result(geometry.error());
    }
    const auto conditioning =
        apmesh::core::surface_metric_conditioning(geometry->metric_normal);
    if (!conditioning) {
        return error_result(conditioning.error());
    }

    Fields fields =
        metric_fields(geometry->metric_normal, conditioning->condition_number);
    fields.l = geometry->second_fundamental_form.l;
    fields.m = geometry->second_fundamental_form.m;
    fields.n = geometry->second_fundamental_form.n;
    fields.gaussian_curvature = geometry->gaussian_curvature;
    fields.mean_curvature = geometry->mean_curvature;

    if (include_principal) {
        const auto principal =
            apmesh::core::surface_principal_curvatures(*geometry);
        if (!principal) {
            return error_result(principal.error());
        }
        fields.maximum_curvature = principal->maximum_curvature;
        fields.minimum_curvature = principal->minimum_curvature;
        fields.is_umbilic = principal->is_umbilic;
    }

    return ResultRecord{
        .outcome = "value",
        .error = std::nullopt,
        .fields = fields,
    };
}

template <BoundedParametricSurface3 Surface>
ResultRecord metric_surface_result(
    const Surface& surface,
    const double u,
    const double v,
    const bool include_conditioning = false) {
    const auto metric = apmesh::core::surface_metric_normal(surface, u, v);
    if (!metric) {
        return error_result(metric.error());
    }
    std::optional<double> condition;
    if (include_conditioning) {
        const auto conditioning =
            apmesh::core::surface_metric_conditioning(*metric);
        if (!conditioning) {
            return error_result(conditioning.error());
        }
        condition = conditioning->condition_number;
    }
    return ResultRecord{
        .outcome = "value",
        .error = std::nullopt,
        .fields = metric_fields(*metric, condition),
    };
}

Vector3 vector(const double x, const double y, const double z) {
    return *Vector3::make(x, y, z);
}

Point3 point(const double x, const double y, const double z) {
    return *Point3::make(x, y, z);
}

SurfaceFirstDerivatives3 first(
    const Vector3& u,
    const Vector3& v) {
    return SurfaceFirstDerivatives3{.u = u, .v = v};
}

SurfaceSecondDerivatives3 second(
    const Vector3& uu,
    const Vector3& uv,
    const Vector3& vv) {
    return SurfaceSecondDerivatives3{.uu = uu, .uv = uv, .vv = vv};
}

ResultRecord expected_metric(
    const double e,
    const double f,
    const double g,
    const double area,
    const Vector3& normal,
    const std::optional<double> condition = std::nullopt) {
    return ResultRecord{
        .outcome = "value",
        .error = std::nullopt,
        .fields = Fields{
            .e = e,
            .f = f,
            .g = g,
            .area_density = area,
            .nx = normal.x(),
            .ny = normal.y(),
            .nz = normal.z(),
            .condition_number = condition,
        },
    };
}

ResultRecord expected_second(
    const double e,
    const double f,
    const double g,
    const double area,
    const Vector3& normal,
    const double condition,
    const double l,
    const double m,
    const double n,
    const double gaussian,
    const double mean,
    const std::optional<double> maximum = std::nullopt,
    const std::optional<double> minimum = std::nullopt,
    const std::optional<bool> umbilic = std::nullopt) {
    return ResultRecord{
        .outcome = "value",
        .error = std::nullopt,
        .fields = Fields{
            .e = e,
            .f = f,
            .g = g,
            .area_density = area,
            .nx = normal.x(),
            .ny = normal.y(),
            .nz = normal.z(),
            .condition_number = condition,
            .l = l,
            .m = m,
            .n = n,
            .gaussian_curvature = gaussian,
            .mean_curvature = mean,
            .maximum_curvature = maximum,
            .minimum_curvature = minimum,
            .is_umbilic = umbilic,
        },
    };
}

struct ContinuityLimitedSurface {
    SurfaceFirstDerivatives3 first_value;

    SurfaceParameterDomain parameter_domain() const noexcept {
        return {
            .u = *CurveParameterDomain::make(0.0, 1.0),
            .v = *CurveParameterDomain::make(0.0, 1.0),
        };
    }

    std::expected<Point3, SurfaceError> evaluate(
        const double u,
        const double v) const noexcept {
        if (!std::isfinite(u)) {
            return std::unexpected{SurfaceError::non_finite_u_parameter};
        }
        if (!std::isfinite(v)) {
            return std::unexpected{SurfaceError::non_finite_v_parameter};
        }
        if (u < 0.0 || u > 1.0) {
            return std::unexpected{SurfaceError::u_parameter_out_of_domain};
        }
        if (v < 0.0 || v > 1.0) {
            return std::unexpected{SurfaceError::v_parameter_out_of_domain};
        }
        return point(0.0, 0.0, 0.0);
    }

    std::expected<SurfaceFirstDerivatives3, SurfaceError> first_derivatives(
        const double u,
        const double v) const noexcept {
        const auto value = evaluate(u, v);
        if (!value) {
            return std::unexpected{value.error()};
        }
        return first_value;
    }

    std::expected<SurfaceSecondDerivatives3, SurfaceError> second_derivatives(
        const double u,
        const double v) const noexcept {
        const auto value = evaluate(u, v);
        if (!value) {
            return std::unexpected{value.error()};
        }
        return std::unexpected{SurfaceError::insufficient_continuity};
    }
};

static_assert(BoundedParametricSurface3<ContinuityLimitedSurface>);

BicubicBezierPatch3::ControlNet planar_control_net() {
    return {{
        {{point(0.0, 0.0, 0.0), point(0.0, 1.0 / 3.0, 0.0),
          point(0.0, 2.0 / 3.0, 0.0), point(0.0, 1.0, 0.0)}},
        {{point(1.0 / 3.0, 0.0, 0.0), point(1.0 / 3.0, 1.0 / 3.0, 0.0),
          point(1.0 / 3.0, 2.0 / 3.0, 0.0), point(1.0 / 3.0, 1.0, 0.0)}},
        {{point(2.0 / 3.0, 0.0, 0.0), point(2.0 / 3.0, 1.0 / 3.0, 0.0),
          point(2.0 / 3.0, 2.0 / 3.0, 0.0), point(2.0 / 3.0, 1.0, 0.0)}},
        {{point(1.0, 0.0, 0.0), point(1.0, 1.0 / 3.0, 0.0),
          point(1.0, 2.0 / 3.0, 0.0), point(1.0, 1.0, 0.0)}},
    }};
}

RationalBicubicBezierPatch3::WeightNet unit_weights() {
    return {{
        {{1.0, 1.0, 1.0, 1.0}},
        {{1.0, 1.0, 1.0, 1.0}},
        {{1.0, 1.0, 1.0, 1.0}},
        {{1.0, 1.0, 1.0, 1.0}},
    }};
}

std::vector<Point3> double_knot_points(
    const std::size_t u_count,
    const std::size_t v_count) {
    std::vector<Point3> points;
    points.reserve(u_count * v_count);
    for (std::size_t i = 0; i < u_count; ++i) {
        for (std::size_t j = 0; j < v_count; ++j) {
            const double x =
                -2.5 + 0.95 * static_cast<double>(i) +
                0.17 * static_cast<double>(j);
            const double y =
                -1.5 + 0.72 * static_cast<double>(j) -
                0.11 * static_cast<double>(i);
            const double z =
                0.41 * static_cast<double>(i * i) -
                0.33 * static_cast<double>(j * j) +
                0.19 * static_cast<double>(i * j) +
                0.07 * static_cast<double>(i * i * j);
            points.push_back(point(x, y, z));
        }
    }
    return points;
}

std::vector<double> double_knot_weights(
    const std::size_t u_count,
    const std::size_t v_count) {
    std::vector<double> weights;
    weights.reserve(u_count * v_count);
    for (std::size_t i = 0; i < u_count; ++i) {
        for (std::size_t j = 0; j < v_count; ++j) {
            weights.push_back(
                0.65 +
                0.2 * static_cast<double>((5U * i + 3U * j) % 9U));
        }
    }
    return weights;
}

std::string quoted(const std::string_view value) {
    return "\"" + std::string{value} + "\"";
}

void write_optional_double(
    std::ostream& output,
    const std::optional<double>& value) {
    if (!value) {
        output << "null";
        return;
    }
    output << quoted(hex_value(*value));
}

void write_optional_bool(
    std::ostream& output,
    const std::optional<bool>& value) {
    if (!value) {
        output << "null";
        return;
    }
    output << (*value ? "true" : "false");
}

void write_fields(std::ostream& output, const Fields& fields) {
    output << "{";
    output << "\"e\":";
    write_optional_double(output, fields.e);
    output << ",\"f\":";
    write_optional_double(output, fields.f);
    output << ",\"g\":";
    write_optional_double(output, fields.g);
    output << ",\"area_density\":";
    write_optional_double(output, fields.area_density);
    output << ",\"normal\":";
    if (fields.nx && fields.ny && fields.nz) {
        output << "[" << quoted(hex_value(*fields.nx)) << ","
               << quoted(hex_value(*fields.ny)) << ","
               << quoted(hex_value(*fields.nz)) << "]";
    } else {
        output << "null";
    }
    output << ",\"condition_number\":";
    write_optional_double(output, fields.condition_number);
    output << ",\"l\":";
    write_optional_double(output, fields.l);
    output << ",\"m\":";
    write_optional_double(output, fields.m);
    output << ",\"n\":";
    write_optional_double(output, fields.n);
    output << ",\"gaussian_curvature\":";
    write_optional_double(output, fields.gaussian_curvature);
    output << ",\"mean_curvature\":";
    write_optional_double(output, fields.mean_curvature);
    output << ",\"maximum_curvature\":";
    write_optional_double(output, fields.maximum_curvature);
    output << ",\"minimum_curvature\":";
    write_optional_double(output, fields.minimum_curvature);
    output << ",\"is_umbilic\":";
    write_optional_bool(output, fields.is_umbilic);
    output << "}";
}

void write_result(std::ostream& output, const ResultRecord& result) {
    output << "{\"outcome\":" << quoted(result.outcome) << ",\"error\":";
    if (result.error) {
        output << quoted(*result.error);
    } else {
        output << "null";
    }
    output << ",\"fields\":";
    write_fields(output, result.fields);
    output << "}";
}

std::array<std::pair<std::string_view, std::optional<double>>, 15>
numeric_fields(const Fields& fields) {
    return {{
        {"e", fields.e},
        {"f", fields.f},
        {"g", fields.g},
        {"area_density", fields.area_density},
        {"normal_x", fields.nx},
        {"normal_y", fields.ny},
        {"normal_z", fields.nz},
        {"condition_number", fields.condition_number},
        {"l", fields.l},
        {"m", fields.m},
        {"n", fields.n},
        {"gaussian_curvature", fields.gaussian_curvature},
        {"mean_curvature", fields.mean_curvature},
        {"maximum_curvature", fields.maximum_curvature},
        {"minimum_curvature", fields.minimum_curvature},
    }};
}

double policy_tolerance(const std::string_view policy) {
    return policy == "binary_strict" ? 0x1p-42 : 0x1p-38;
}

void write_comparisons(std::ostream& output, const CaseRecord& value) {
    output << "[";
    if (value.policy &&
        value.reference.outcome == "value" &&
        value.observed.outcome == "value") {
        const auto expected = numeric_fields(value.reference.fields);
        const auto observed = numeric_fields(value.observed.fields);
        bool first_item = true;
        for (std::size_t index = 0; index < expected.size(); ++index) {
            if (!expected[index].second || !observed[index].second) {
                continue;
            }
            const double reference = *expected[index].second;
            const double actual = *observed[index].second;
            const double tolerance = policy_tolerance(*value.policy);
            const double reference_scale = std::max(1.0, std::abs(reference));
            const double residual = std::abs(actual - reference);
            const double limit =
                tolerance + tolerance * reference_scale;
            if (!first_item) {
                output << ",";
            }
            first_item = false;
            output << "{\"field\":" << quoted(expected[index].first)
                   << ",\"policy\":" << quoted(*value.policy)
                   << ",\"reference\":" << quoted(hex_value(reference))
                   << ",\"observed\":" << quoted(hex_value(actual))
                   << ",\"reference_scale\":"
                   << quoted(hex_value(reference_scale))
                   << ",\"residual\":" << quoted(hex_value(residual))
                   << ",\"limit\":" << quoted(hex_value(limit))
                   << "}";
        }
    }
    output << "]";
}

void write_case(
    std::ostream& output,
    const CaseRecord& value,
    const bool prepend_comma) {
    if (prepend_comma) {
        output << ",";
    }
    output << "{\"id\":" << quoted(value.id)
           << ",\"category\":" << quoted(value.category)
           << ",\"comparison_rule\":" << quoted(value.comparison_rule)
           << ",\"policy\":";
    if (value.policy) {
        output << quoted(*value.policy);
    } else {
        output << "null";
    }
    output << ",\"reference\":";
    write_result(output, value.reference);
    output << ",\"observed\":";
    write_result(output, value.observed);
    output << ",\"comparisons\":";
    write_comparisons(output, value);
    output << ",\"observations\":" << value.observations << "}";
}

CaseRecord exact_case(
    std::string id,
    std::string category,
    ResultRecord reference,
    ResultRecord observed,
    std::string observations = "{}") {
    return CaseRecord{
        .id = std::move(id),
        .category = std::move(category),
        .comparison_rule = "exact",
        .policy = std::nullopt,
        .reference = std::move(reference),
        .observed = std::move(observed),
        .observations = std::move(observations),
    };
}

CaseRecord mixed_case(
    std::string id,
    std::string category,
    ResultRecord reference,
    ResultRecord observed,
    std::string observations = "{}") {
    return CaseRecord{
        .id = std::move(id),
        .category = std::move(category),
        .comparison_rule = "mixed",
        .policy = std::nullopt,
        .reference = std::move(reference),
        .observed = std::move(observed),
        .observations = std::move(observations),
    };
}

CaseRecord proximity_case(
    std::string id,
    std::string category,
    std::string policy,
    ResultRecord reference,
    ResultRecord observed,
    std::string observations = "{}") {
    return CaseRecord{
        .id = std::move(id),
        .category = std::move(category),
        .comparison_rule = "proximity",
        .policy = std::move(policy),
        .reference = std::move(reference),
        .observed = std::move(observed),
        .observations = std::move(observations),
    };
}

std::string conditioning_series_json() {
    const auto u = vector(1.0, 0.0, 0.0);
    std::ostringstream output;
    output << "{\"law\":\"near_singular_power_of_two_series\","
           << "\"series\":[";
    bool first_item = true;
    for (const int exponent : {20, 100, 500}) {
        const double tiny = std::ldexp(1.0, -exponent);
        const auto result = metric_result(
            first(u, vector(1.0, tiny, 0.0)), true);
        if (!first_item) {
            output << ",";
        }
        first_item = false;
        output << "{\"separation_exponent\":" << exponent
               << ",\"condition_number\":";
        if (result.fields.condition_number) {
            output << quoted(hex_value(*result.fields.condition_number));
        } else {
            output << "null";
        }
        output << "}";
    }
    output << "]}";
    return output.str();
}

std::string family_conformance_json() {
    const auto net = planar_control_net();
    const BicubicBezierPatch3 bezier{net};
    const auto rational =
        RationalBicubicBezierPatch3::make(net, unit_weights());

    std::vector<Point3> flat_controls;
    flat_controls.reserve(16U);
    for (const auto& row : net) {
        flat_controls.insert(
            flat_controls.end(), row.begin(), row.end());
    }
    const auto nurbs = BicubicNURBSSurface3::make(
        flat_controls,
        std::vector<double>(16U, 1.0),
        4U,
        4U,
        {},
        {},
        0.0,
        1.0,
        0.0,
        1.0);

    const CubicBezier3 bottom{
        net[0][0], net[1][0], net[2][0], net[3][0]};
    const CubicBezier3 top{
        net[0][3], net[1][3], net[2][3], net[3][3]};
    const CubicBezier3 left{
        net[0][0], net[0][1], net[0][2], net[0][3]};
    const CubicBezier3 right{
        net[3][0], net[3][1], net[3][2], net[3][3]};
    const auto coons =
        apmesh::core::CubicBezierCoonsPatch3::make(
            bottom, top, left, right);
    const auto trimmed =
        apmesh::core::RectangularTrimmedSurface3<
            BicubicBezierPatch3>::make(
                bezier, 0.1, 0.9, 0.2, 0.8);
    const auto extrusion =
        apmesh::core::CubicBezierLinearExtrusionSurface3::make(
            bottom, vector(0.0, 1.0, 0.0));
    const CubicBezier3 generatrix{
        point(2.0, 0.0, 0.0),
        point(2.0, 0.0, 1.0 / 3.0),
        point(2.0, 0.0, 2.0 / 3.0),
        point(2.0, 0.0, 1.0)};
    const auto revolution =
        apmesh::core::CubicBezierRevolutionSurface3::make(
            generatrix, AxisPlacement3::identity(), 1.0);

    const auto unit_domain = *CurveParameterDomain::make(0.0, 1.0);
    const BoundedPlaneSurface3 plane{
        AxisPlacement3::identity(), unit_domain, unit_domain};
    const auto cylinder = BoundedCylinderSurface3::make(
        AxisPlacement3::identity(),
        2.0,
        *CurveParameterDomain::make(-0.5, 0.5),
        *CurveParameterDomain::make(-1.0, 1.0));
    const auto sphere = BoundedSphereSurface3::make(
        AxisPlacement3::identity(),
        2.0,
        *CurveParameterDomain::make(-0.5, 0.5),
        *CurveParameterDomain::make(-0.6, 0.6));

    const std::vector<double> u_knots{0.0, 1.5};
    const std::vector<double> v_knots{-0.5, 1.0};
    const std::vector<std::uint8_t> u_mult{2U, 1U};
    const std::vector<std::uint8_t> v_mult{1U, 2U};
    const auto double_knot = BicubicNURBSSurface3::make(
        double_knot_points(7U, 7U),
        double_knot_weights(7U, 7U),
        7U,
        7U,
        u_knots,
        v_knots,
        u_mult,
        v_mult,
        -2.0,
        3.0,
        -1.5,
        2.5);

    if (!rational || !nurbs || !coons || !trimmed ||
        !extrusion || !revolution || !cylinder || !sphere ||
        !double_knot) {
        return "{\"construction\":\"failed\"}";
    }

    const auto regular = [](
        const auto& surface,
        const double u,
        const double v) {
        const auto metric =
            apmesh::core::surface_metric_normal(surface, u, v);
        const auto second =
            apmesh::core::surface_second_order_geometry(surface, u, v);
        const auto principal =
            apmesh::core::surface_principal_curvatures(surface, u, v);
        return metric && second && principal;
    };

    std::ostringstream output;
    output << "{\"families\":[";
    bool first_family = true;
    const auto append = [&](const std::string_view id, const bool ok) mutable {
        if (!first_family) {
            output << ",";
        }
        first_family = false;
        output << "{\"id\":" << quoted(id)
               << ",\"regular_differential\":\""
               << (ok ? "value" : "failure") << "\"}";
    };
    append("bicubic_bezier", regular(bezier, 0.4, 0.6));
    append("rational_bicubic_bezier", regular(*rational, 0.4, 0.6));
    append("bicubic_nurbs", regular(*nurbs, 0.4, 0.6));
    append("coons_patch", regular(*coons, 0.4, 0.6));
    append("rectangular_trim", regular(*trimmed, 0.4, 0.5));
    append("linear_extrusion", regular(*extrusion, 0.4, 0.6));
    append("revolution", regular(*revolution, 0.4, 0.5));
    append("plane", regular(plane, 0.4, 0.6));
    append("cylinder", regular(*cylinder, 0.2, 0.1));
    append("sphere", regular(*sphere, 0.2, 0.3));

    const auto double_first =
        double_knot->first_derivatives(0.0, 0.25);
    const auto double_second =
        double_knot->second_derivatives(0.0, 0.25);
    output << "],\"double_knot_c1\":{"
           << "\"id\":\"bicubic_nurbs_double_knot\","
           << "\"first_order\":\""
           << (double_first ? "value" : error_name(double_first.error()))
           << "\",\"second_order\":\""
           << (double_second ? "value" : error_name(double_second.error()))
           << "\"}}";
    return output.str();
}

std::vector<CaseRecord> build_cases() {
    const auto x = vector(1.0, 0.0, 0.0);
    const auto y = vector(0.0, 1.0, 0.0);
    const auto z = vector(0.0, 0.0, 1.0);
    const auto zero = vector(0.0, 0.0, 0.0);
    const auto oblique_u = vector(2.0, 0.0, 0.0);
    const auto oblique_v = vector(1.0, 3.0, 0.0);
    const auto condition_v = vector(1.0, 1.0, 0.0);
    const double condition_nonorthogonal =
        0.5 * (3.0 + std::sqrt(5.0));

    std::vector<CaseRecord> cases;
    cases.reserve(35U);

    cases.push_back(exact_case(
        "metric_plane_identity",
        "first_order",
        expected_metric(1.0, 0.0, 1.0, 1.0, z, 1.0),
        metric_result(first(x, y), true)));

    cases.push_back(mixed_case(
        "metric_oblique",
        "first_order",
        expected_metric(4.0, 2.0, 10.0, 6.0, z),
        metric_result(first(oblique_u, oblique_v), false)));

    const auto unit_domain = *CurveParameterDomain::make(0.0, 1.0);
    const BoundedPlaneSurface3 plane{
        AxisPlacement3::identity(), unit_domain, unit_domain};
    const double nan = std::numeric_limits<double>::quiet_NaN();

    const auto add_parameter_error = [&](
        const std::string& id,
        const double u,
        const double v,
        const SurfaceDifferentialError expected_error) {
        cases.push_back(exact_case(
            id,
            "error",
            error_result(expected_error),
            metric_surface_result(plane, u, v)));
    };
    add_parameter_error(
        "parameter_non_finite_u",
        nan,
        nan,
        SurfaceDifferentialError::non_finite_u_parameter);
    add_parameter_error(
        "parameter_non_finite_v",
        0.5,
        nan,
        SurfaceDifferentialError::non_finite_v_parameter);
    add_parameter_error(
        "parameter_u_out_of_domain",
        -0.1,
        0.5,
        SurfaceDifferentialError::u_parameter_out_of_domain);
    add_parameter_error(
        "parameter_v_out_of_domain",
        0.5,
        1.1,
        SurfaceDifferentialError::v_parameter_out_of_domain);

    const SurfaceMetricNormal3 nonrepresentable_metric{
        .first_fundamental_form = {.e = 1.0, .f = 1.0, .g = 1.0},
        .area_density = std::numeric_limits<double>::denorm_min(),
        .unit_normal = z,
    };
    const auto nonrepresentable =
        apmesh::core::surface_metric_conditioning(nonrepresentable_metric);
    cases.push_back(exact_case(
        "non_representable_metric",
        "error",
        error_result(SurfaceDifferentialError::non_representable_result),
        nonrepresentable
            ? ResultRecord{}
            : error_result(nonrepresentable.error())));

    cases.push_back(exact_case(
        "metric_exact_singular",
        "error",
        error_result(SurfaceDifferentialError::singular_parameterization),
        metric_result(first(x, vector(2.0, 0.0, 0.0)), true)));

    const double tiny = std::ldexp(1.0, -500);
    cases.push_back(proximity_case(
        "metric_near_singular",
        "first_order",
        "binary_strict",
        expected_metric(
            1.0, 1.0, 1.0, tiny, z, std::ldexp(1.0, 501)),
        metric_result(first(x, vector(1.0, tiny, 0.0)), true),
        conditioning_series_json()));

    const double huge = std::ldexp(1.0, 450);
    const double small = std::ldexp(1.0, -450);
    cases.push_back(proximity_case(
        "metric_extreme_finite",
        "first_order",
        "binary_strict",
        expected_metric(
            std::ldexp(1.0, 900),
            0.0,
            std::ldexp(1.0, -900),
            1.0,
            z,
            std::ldexp(1.0, 900)),
        metric_result(
            first(vector(huge, 0.0, 0.0), vector(0.0, small, 0.0)),
            true)));

    cases.push_back(proximity_case(
        "conditioning_nonorthogonal",
        "conditioning",
        "binary_strict",
        expected_metric(
            4.0, 2.0, 2.0, 2.0, z, condition_nonorthogonal),
        metric_result(first(oblique_u, condition_v), true)));

    const auto stretched = vector(4.0, 0.0, 0.0);
    cases.push_back(proximity_case(
        "conditioning_parameter_scale",
        "invariance",
        "binary_strict",
        expected_metric(16.0, 0.0, 1.0, 4.0, z, 4.0),
        metric_result(first(stretched, y), true),
        "{\"law\":\"non_uniform_parameter_scaling_changes_conditioning\","
        "\"baseline_condition_number\":\"0x1p+0\","
        "\"transformed_condition_number\":\"0x1p+2\","
        "\"law_satisfied\":true}"));

    const auto scaled_u = *oblique_u * 8.0;
    const auto scaled_v = *condition_v * 8.0;
    cases.push_back(proximity_case(
        "conditioning_spatial_scale",
        "invariance",
        "binary_strict",
        expected_metric(
            256.0,
            128.0,
            128.0,
            128.0,
            z,
            condition_nonorthogonal),
        metric_result(first(*scaled_u, *scaled_v), true),
        "{\"law\":\"uniform_spatial_scale_preserves_conditioning\","
        "\"scale\":\"0x1p+3\",\"law_satisfied\":true}"));

    cases.push_back(proximity_case(
        "conditioning_axis_swap",
        "invariance",
        "binary_strict",
        expected_metric(
            2.0, 2.0, 4.0, 2.0, -z, condition_nonorthogonal),
        metric_result(first(condition_v, oblique_u), true),
        "{\"law\":\"parameter_axis_swap_preserves_conditioning\","
        "\"law_satisfied\":true}"));

    cases.push_back(proximity_case(
        "conditioning_u_reversal",
        "invariance",
        "binary_strict",
        expected_metric(
            4.0, -2.0, 2.0, 2.0, -z, condition_nonorthogonal),
        metric_result(first(-oblique_u, condition_v), true),
        "{\"law\":\"u_reversal_preserves_conditioning\","
        "\"normal_orientation\":\"flipped\",\"law_satisfied\":true}"));

    cases.push_back(proximity_case(
        "conditioning_v_reversal",
        "invariance",
        "binary_strict",
        expected_metric(
            4.0, -2.0, 2.0, 2.0, -z, condition_nonorthogonal),
        metric_result(first(oblique_u, -condition_v), true),
        "{\"law\":\"v_reversal_preserves_conditioning\","
        "\"normal_orientation\":\"flipped\",\"law_satisfied\":true}"));

    const auto second_oblique = second(
        vector(0.0, 0.0, 4.0),
        vector(0.0, 0.0, -2.0),
        vector(0.0, 0.0, 6.0));
    cases.push_back(mixed_case(
        "second_order_oblique",
        "second_order",
        expected_second(
            4.0, 2.0, 10.0, 6.0, z,
            (7.0 + std::sqrt(13.0)) / 6.0,
            4.0, -2.0, 6.0, 5.0 / 9.0, 1.0),
        second_order_result(
            first(oblique_u, oblique_v), second_oblique, false)));

    cases.push_back(exact_case(
        "second_order_hyperbolic",
        "second_order",
        expected_second(
            1.0, 0.0, 1.0, 1.0, z, 1.0,
            2.0, 0.0, -2.0, -4.0, 0.0),
        second_order_result(
            first(x, y),
            second(
                vector(0.0, 0.0, 2.0),
                zero,
                vector(0.0, 0.0, -2.0)),
            false)));

    const auto principal_case = [&](
        const std::string& id,
        const std::string& rule,
        const std::optional<std::string>& policy,
        const double l,
        const double n,
        const double gaussian,
        const double mean,
        const double maximum,
        const double minimum,
        const bool umbilic) {
        const ResultRecord expected = expected_second(
            1.0, 0.0, 1.0, 1.0, z, 1.0,
            l, 0.0, n, gaussian, mean,
            maximum, minimum, umbilic);
        const ResultRecord observed = second_order_result(
            first(x, y),
            second(vector(0.0, 0.0, l), zero, vector(0.0, 0.0, n)),
            true);
        if (rule == "proximity") {
            cases.push_back(proximity_case(
                id, "principal", *policy, expected, observed));
        } else {
            cases.push_back(exact_case(
                id, "principal", expected, observed));
        }
    };

    principal_case(
        "principal_elliptic", "exact", std::nullopt,
        3.0, 1.0, 3.0, 2.0, 3.0, 1.0, false);
    principal_case(
        "principal_hyperbolic", "exact", std::nullopt,
        2.0, -4.0, -8.0, -1.0, 2.0, -4.0, false);
    principal_case(
        "principal_parabolic", "exact", std::nullopt,
        2.0, 0.0, 0.0, 1.0, 2.0, 0.0, false);
    principal_case(
        "principal_umbilic", "exact", std::nullopt,
        2.0, 2.0, 4.0, 2.0, 2.0, 2.0, true);

    const double near = std::nextafter(2.0, 3.0);
    principal_case(
        "principal_near_umbilic", "exact", std::nullopt,
        near, 2.0, near * 2.0, 0.5 * (near + 2.0),
        near, 2.0, false);

    cases.push_back(proximity_case(
        "principal_nonorthogonal",
        "principal",
        "binary_strict",
        expected_second(
            4.0, 2.0, 10.0, 6.0, z,
            (7.0 + std::sqrt(13.0)) / 6.0,
            4.0, -2.0, 6.0, 5.0 / 9.0, 1.0,
            5.0 / 3.0, 1.0 / 3.0, false),
        second_order_result(
            first(oblique_u, oblique_v), second_oblique, true)));

    cases.push_back(exact_case(
        "second_order_singular",
        "error",
        error_result(SurfaceDifferentialError::singular_parameterization),
        second_order_result(
            first(x, vector(2.0, 0.0, 0.0)),
            second(zero, zero, zero),
            false)));

    const ContinuityLimitedSurface limited{
        .first_value = first(x, y),
    };
    const auto limited_result =
        apmesh::core::surface_second_order_geometry(
            limited, 0.5, 0.5);
    cases.push_back(exact_case(
        "second_order_insufficient_continuity",
        "error",
        error_result(SurfaceDifferentialError::insufficient_continuity),
        limited_result
            ? ResultRecord{}
            : error_result(limited_result.error())));

    const auto plane_observed = surface_result(plane, 0.25, 0.75);
    const auto translated_origin = point(8.0, -6.0, 4.0);
    const auto translated_placement = AxisPlacement3::make(
        translated_origin, z, x);
    const BoundedPlaneSurface3 translated_plane{
        *translated_placement, unit_domain, unit_domain};
    const auto translated_plane_result =
        surface_result(translated_plane, 0.25, 0.75);
    const auto plane_u_reversed =
        surface_result(
            plane.u_reversed(),
            *apmesh::core::reversed_parameter(unit_domain, 0.25),
            0.75);
    const bool plane_translation_equal =
        plane_observed.fields.e == translated_plane_result.fields.e &&
        plane_observed.fields.gaussian_curvature ==
            translated_plane_result.fields.gaussian_curvature;
    const bool plane_u_normal_flipped =
        plane_observed.fields.nz &&
        plane_u_reversed.fields.nz &&
        *plane_observed.fields.nz == -*plane_u_reversed.fields.nz;
    cases.push_back(proximity_case(
        "plane_surface",
        "analytic_surface",
        "surface_strict",
        expected_second(
            1.0, 0.0, 1.0, 1.0, z, 1.0,
            0.0, 0.0, 0.0, 0.0, 0.0,
            0.0, 0.0, true),
        plane_observed,
        std::string{"{\"translation_invariant\":"} +
            (plane_translation_equal ? "true" : "false") +
            ",\"u_reversal_normal_flipped\":" +
            (plane_u_normal_flipped ? "true" : "false") + "}"));

    const auto cylinder_domain_u = *CurveParameterDomain::make(-0.5, 0.5);
    const auto cylinder_domain_v = *CurveParameterDomain::make(-1.0, 1.0);
    const auto cylinder_case = [&](
        const std::string& id,
        const double radius) {
        const auto cylinder = BoundedCylinderSurface3::make(
            AxisPlacement3::identity(),
            radius,
            cylinder_domain_u,
            cylinder_domain_v);
        const double inverse = 1.0 / radius;
        const double condition =
            std::max(radius, inverse);
        const ResultRecord expected = expected_second(
            radius * radius, 0.0, 1.0, radius, x, condition,
            -radius, 0.0, 0.0, 0.0, -0.5 * inverse,
            0.0, -inverse, false);
        const ResultRecord observed =
            surface_result(*cylinder, 0.0, 0.25);
        std::string observations{"{}"};
        if (radius == 2.0) {
            const auto mapped =
                *apmesh::core::reversed_parameter(
                    cylinder_domain_u, 0.0);
            const auto reversed =
                surface_result(cylinder->u_reversed(), mapped, 0.25);
            const bool gaussian_same =
                observed.fields.gaussian_curvature ==
                    reversed.fields.gaussian_curvature;
            const bool mean_flipped =
                observed.fields.mean_curvature &&
                reversed.fields.mean_curvature &&
                *observed.fields.mean_curvature ==
                    -*reversed.fields.mean_curvature;
            observations =
                std::string{"{\"u_reversal_gaussian_invariant\":"} +
                (gaussian_same ? "true" : "false") +
                ",\"u_reversal_mean_flipped\":" +
                (mean_flipped ? "true" : "false") + "}";
        }
        cases.push_back(proximity_case(
            id, "analytic_surface", "surface_strict",
            expected, observed, observations));
    };
    cylinder_case("cylinder_radius_1", 1.0);
    cylinder_case("cylinder_radius_2", 2.0);
    cylinder_case("cylinder_radius_4", 4.0);

    const double half_pi = 0.5 * std::numbers::pi_v<double>;
    const auto sphere_domain_u = *CurveParameterDomain::make(-0.5, 0.5);
    const auto sphere_domain_v =
        *CurveParameterDomain::make(-half_pi, half_pi);
    const auto sphere = *BoundedSphereSurface3::make(
        AxisPlacement3::identity(),
        2.0,
        sphere_domain_u,
        sphere_domain_v);
    const double sphere_radius = 2.0;
    const double sphere_inverse = 0.5;

    cases.push_back(proximity_case(
        "sphere_equator",
        "analytic_surface",
        "surface_strict",
        expected_second(
            4.0, 0.0, 4.0, 4.0, x, 1.0,
            -2.0, 0.0, -2.0, 0.25, -0.5,
            -0.5, -0.5, true),
        surface_result(sphere, 0.0, 0.0)));

    const double latitude = 0.5;
    const double cosine = std::cos(latitude);
    const double sine = std::sin(latitude);
    const auto sphere_latitude_observed =
        surface_result(sphere, 0.0, latitude);
    const auto reversed_u = sphere.u_reversed();
    const double mapped_u =
        *apmesh::core::reversed_parameter(sphere_domain_u, 0.0);
    const auto reversed_latitude =
        surface_result(reversed_u, mapped_u, latitude);
    const bool sphere_gaussian_same =
        sphere_latitude_observed.fields.gaussian_curvature ==
            reversed_latitude.fields.gaussian_curvature;
    const bool sphere_mean_flipped =
        sphere_latitude_observed.fields.mean_curvature &&
        reversed_latitude.fields.mean_curvature &&
        *sphere_latitude_observed.fields.mean_curvature ==
            -*reversed_latitude.fields.mean_curvature;
    cases.push_back(proximity_case(
        "sphere_latitude",
        "analytic_surface",
        "surface_strict",
        expected_second(
            4.0 * cosine * cosine,
            0.0,
            4.0,
            4.0 * cosine,
            vector(cosine, 0.0, sine),
            1.0 / cosine,
            -2.0 * cosine * cosine,
            0.0,
            -2.0,
            0.25,
            -0.5,
            -0.5,
            -0.5,
            true),
        sphere_latitude_observed,
        std::string{"{\"u_reversal_gaussian_invariant\":"} +
            (sphere_gaussian_same ? "true" : "false") +
            ",\"u_reversal_mean_flipped\":" +
            (sphere_mean_flipped ? "true" : "false") + "}"));

    cases.push_back(exact_case(
        "sphere_pole",
        "error",
        error_result(SurfaceDifferentialError::singular_parameterization),
        surface_result(sphere, 0.0, half_pi)));

    const double near_pole = half_pi - std::ldexp(1.0, -20);
    const double near_cosine = std::cos(near_pole);
    const double near_sine = std::sin(near_pole);
    cases.push_back(proximity_case(
        "sphere_near_pole",
        "analytic_surface",
        "surface_strict",
        expected_second(
            4.0 * near_cosine * near_cosine,
            0.0,
            4.0,
            4.0 * near_cosine,
            vector(near_cosine, 0.0, near_sine),
            1.0 / near_cosine,
            -2.0 * near_cosine * near_cosine,
            0.0,
            -2.0,
            0.25,
            -0.5,
            -0.5,
            -0.5,
            true),
        surface_result(sphere, 0.0, near_pole)));

    const ResultRecord family_reference{
        .outcome = "value",
        .error = std::nullopt,
        .fields = {},
    };
    cases.push_back(exact_case(
        "family_conformance",
        "conformance",
        family_reference,
        family_reference,
        family_conformance_json()));

    return cases;
}

int write_certificate(const std::string_view output_path) {
    const auto cases = build_cases();
    if (cases.size() != 35U) {
        return 2;
    }

    std::ofstream output{
        std::string{output_path},
        std::ios::binary | std::ios::trunc};
    if (!output.is_open()) {
        return 3;
    }
    output.imbue(std::locale::classic());

    output
        << "{\"schema_version\":1,"
        << "\"kind\":\"surface-differential-geometry-qualification-certificate\","
        << "\"candidate_identity\":\"REPORT_ONLY_TOOLING_UNPREPARED\","
        << "\"claim\":\"pointwise_local_surface_differential_geometry\","
        << "\"signed_zero_policy\":{"
        << "\"raw_hex_retained\":true,"
        << "\"semantic_projection\":\"canonicalize_positive_and_negative_zero\","
        << "\"signed_zero_scientific_claim\":false},"
        << "\"non_claims\":["
        << "\"surface_representation_qualification\","
        << "\"principal_directions\","
        << "\"curvature_line_continuity\","
        << "\"near_umbilic_threshold\","
        << "\"meshing_conditioning_threshold\","
        << "\"cone\","
        << "\"torus\","
        << "\"periodic_seam_topology\","
        << "\"general_pcurves\","
        << "\"topological_face_binding\","
        << "\"boundary_curve_discretization\","
        << "\"physical_sizing\","
        << "\"shared_boundary_certification\","
        << "\"triangular_meshing\","
        << "\"quadrilateral_meshing\","
        << "\"adaptive_meshing\","
        << "\"anisotropic_tensor_metrics\","
        << "\"quad_dominant\","
        << "\"parallel_gpu_simd_equivalence\","
        << "\"native_windows_qualification\","
        << "\"arbitrary_precision\","
        << "\"third_party_runtime_numerics\"],"
        << "\"figure_sources\":["
        << "\"sphere-equator-latitude-pole\","
        << "\"cylinder-radius-1-2-4\","
        << "\"metric-near-singular-series\","
        << "\"elliptic-hyperbolic-parabolic\"],"
        << "\"cases\":[";

    for (std::size_t index = 0; index < cases.size(); ++index) {
        write_case(output, cases[index], index != 0U);
    }
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
