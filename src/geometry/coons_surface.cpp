#include "apmesh/geometry/coons_surface.hpp"

#include <cmath>
#include <expected>

namespace apmesh::core {
namespace {

[[nodiscard]] SurfaceParameterDomain unit_square_domain() noexcept {
    return SurfaceParameterDomain{
        .u = *CurveParameterDomain::make(0.0, 1.0),
        .v = *CurveParameterDomain::make(0.0, 1.0),
    };
}

[[nodiscard]] std::expected<void, SurfaceError> validate_parameters(
    const double u,
    const double v) noexcept {
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
    return {};
}

template <typename Value>
[[nodiscard]] std::expected<Value, SurfaceError> map_curve_result(
    const std::expected<Value, CurveError>& value) noexcept {
    if (!value.has_value()) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    return *value;
}

[[nodiscard]] std::expected<Point3, SurfaceError> make_point(
    const long double x,
    const long double y,
    const long double z) noexcept {
    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z)) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    const double dx = static_cast<double>(x);
    const double dy = static_cast<double>(y);
    const double dz = static_cast<double>(z);
    const auto point = Point3::make(dx, dy, dz);
    if (!point.has_value()) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    return *point;
}

[[nodiscard]] std::expected<Vector3, SurfaceError> make_vector(
    const long double x,
    const long double y,
    const long double z) noexcept {
    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z)) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    const double dx = static_cast<double>(x);
    const double dy = static_cast<double>(y);
    const double dz = static_cast<double>(z);
    const auto vector = Vector3::make(dx, dy, dz);
    if (!vector.has_value()) {
        return std::unexpected{SurfaceError::non_finite_result};
    }
    return *vector;
}

[[nodiscard]] long double bilinear_component(
    const double p00,
    const double p10,
    const double p01,
    const double p11,
    const long double u,
    const long double v) noexcept {
    const long double one_minus_u = 1.0L - u;
    const long double one_minus_v = 1.0L - v;
    return one_minus_u * one_minus_v * static_cast<long double>(p00) +
           u * one_minus_v * static_cast<long double>(p10) +
           one_minus_u * v * static_cast<long double>(p01) +
           u * v * static_cast<long double>(p11);
}

[[nodiscard]] long double bilinear_u_component(
    const double p00,
    const double p10,
    const double p01,
    const double p11,
    const long double v) noexcept {
    return (1.0L - v) *
               (static_cast<long double>(p10) -
                static_cast<long double>(p00)) +
           v * (static_cast<long double>(p11) -
                static_cast<long double>(p01));
}

[[nodiscard]] long double bilinear_v_component(
    const double p00,
    const double p10,
    const double p01,
    const double p11,
    const long double u) noexcept {
    return (1.0L - u) *
               (static_cast<long double>(p01) -
                static_cast<long double>(p00)) +
           u * (static_cast<long double>(p11) -
                static_cast<long double>(p10));
}

[[nodiscard]] long double bilinear_uv_component(
    const double p00,
    const double p10,
    const double p01,
    const double p11) noexcept {
    return static_cast<long double>(p00) -
           static_cast<long double>(p10) -
           static_cast<long double>(p01) +
           static_cast<long double>(p11);
}

} // namespace

std::expected<CubicBezierCoonsPatch3, CoonsSurfaceConstructionError>
CubicBezierCoonsPatch3::make(
    const CubicBezier3& bottom,
    const CubicBezier3& top,
    const CubicBezier3& left,
    const CubicBezier3& right) noexcept {
    const auto& bottom_controls = bottom.control_points();
    const auto& top_controls = top.control_points();
    const auto& left_controls = left.control_points();
    const auto& right_controls = right.control_points();

    if (bottom_controls[0] != left_controls[0]) {
        return std::unexpected{
            CoonsSurfaceConstructionError::lower_left_corner_mismatch};
    }
    if (bottom_controls[3] != right_controls[0]) {
        return std::unexpected{
            CoonsSurfaceConstructionError::lower_right_corner_mismatch};
    }
    if (top_controls[0] != left_controls[3]) {
        return std::unexpected{
            CoonsSurfaceConstructionError::upper_left_corner_mismatch};
    }
    if (top_controls[3] != right_controls[3]) {
        return std::unexpected{
            CoonsSurfaceConstructionError::upper_right_corner_mismatch};
    }

    return CubicBezierCoonsPatch3{bottom, top, left, right};
}

const CubicBezier3& CubicBezierCoonsPatch3::bottom() const noexcept {
    return bottom_;
}

const CubicBezier3& CubicBezierCoonsPatch3::top() const noexcept {
    return top_;
}

const CubicBezier3& CubicBezierCoonsPatch3::left() const noexcept {
    return left_;
}

const CubicBezier3& CubicBezierCoonsPatch3::right() const noexcept {
    return right_;
}

SurfaceParameterDomain
CubicBezierCoonsPatch3::parameter_domain() const noexcept {
    return unit_square_domain();
}

std::expected<Point3, SurfaceError> CubicBezierCoonsPatch3::evaluate(
    const double u,
    const double v) const noexcept {
    const auto valid = validate_parameters(u, v);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }

    if (v == 0.0) {
        return map_curve_result(bottom_.evaluate(u));
    }
    if (v == 1.0) {
        return map_curve_result(top_.evaluate(u));
    }
    if (u == 0.0) {
        return map_curve_result(left_.evaluate(v));
    }
    if (u == 1.0) {
        return map_curve_result(right_.evaluate(v));
    }

    const auto left_value = left_.evaluate(v);
    const auto right_value = right_.evaluate(v);
    const auto bottom_value = bottom_.evaluate(u);
    const auto top_value = top_.evaluate(u);
    if (!left_value || !right_value || !bottom_value || !top_value) {
        return std::unexpected{SurfaceError::non_finite_result};
    }

    const auto& bottom_controls = bottom_.control_points();
    const auto& top_controls = top_.control_points();
    const Point3& p00 = bottom_controls[0];
    const Point3& p10 = bottom_controls[3];
    const Point3& p01 = top_controls[0];
    const Point3& p11 = top_controls[3];

    const long double lu = static_cast<long double>(u);
    const long double lv = static_cast<long double>(v);
    const long double one_minus_u = 1.0L - lu;
    const long double one_minus_v = 1.0L - lv;

    const auto component = [&](const double l,
                               const double r,
                               const double b,
                               const double t,
                               const double c00,
                               const double c10,
                               const double c01,
                               const double c11) noexcept {
        const long double ruled_u =
            one_minus_u * static_cast<long double>(l) +
            lu * static_cast<long double>(r);
        const long double ruled_v =
            one_minus_v * static_cast<long double>(b) +
            lv * static_cast<long double>(t);
        return ruled_u + ruled_v -
               bilinear_component(c00, c10, c01, c11, lu, lv);
    };

    return make_point(
        component(
            left_value->x(),
            right_value->x(),
            bottom_value->x(),
            top_value->x(),
            p00.x(),
            p10.x(),
            p01.x(),
            p11.x()),
        component(
            left_value->y(),
            right_value->y(),
            bottom_value->y(),
            top_value->y(),
            p00.y(),
            p10.y(),
            p01.y(),
            p11.y()),
        component(
            left_value->z(),
            right_value->z(),
            bottom_value->z(),
            top_value->z(),
            p00.z(),
            p10.z(),
            p01.z(),
            p11.z()));
}

std::expected<SurfaceFirstDerivatives3, SurfaceError>
CubicBezierCoonsPatch3::first_derivatives(
    const double u,
    const double v) const noexcept {
    const auto valid = validate_parameters(u, v);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }

    const auto left_value = left_.evaluate(v);
    const auto right_value = right_.evaluate(v);
    const auto bottom_value = bottom_.evaluate(u);
    const auto top_value = top_.evaluate(u);
    const auto left_d1 = left_.first_derivative(v);
    const auto right_d1 = right_.first_derivative(v);
    const auto bottom_d1 = bottom_.first_derivative(u);
    const auto top_d1 = top_.first_derivative(u);
    if (!left_value || !right_value || !bottom_value || !top_value ||
        !left_d1 || !right_d1 || !bottom_d1 || !top_d1) {
        return std::unexpected{SurfaceError::non_finite_result};
    }

    const auto& bottom_controls = bottom_.control_points();
    const auto& top_controls = top_.control_points();
    const Point3& p00 = bottom_controls[0];
    const Point3& p10 = bottom_controls[3];
    const Point3& p01 = top_controls[0];
    const Point3& p11 = top_controls[3];

    const long double lu = static_cast<long double>(u);
    const long double lv = static_cast<long double>(v);
    const long double one_minus_u = 1.0L - lu;
    const long double one_minus_v = 1.0L - lv;

    const auto u_component = [&](const double l,
                                 const double r,
                                 const double bd,
                                 const double td,
                                 const double c00,
                                 const double c10,
                                 const double c01,
                                 const double c11) noexcept {
        return -static_cast<long double>(l) +
               static_cast<long double>(r) +
               one_minus_v * static_cast<long double>(bd) +
               lv * static_cast<long double>(td) -
               bilinear_u_component(c00, c10, c01, c11, lv);
    };

    const auto v_component = [&](const double ld,
                                 const double rd,
                                 const double b,
                                 const double t,
                                 const double c00,
                                 const double c10,
                                 const double c01,
                                 const double c11) noexcept {
        return one_minus_u * static_cast<long double>(ld) +
               lu * static_cast<long double>(rd) -
               static_cast<long double>(b) +
               static_cast<long double>(t) -
               bilinear_v_component(c00, c10, c01, c11, lu);
    };

    auto su = make_vector(
        u_component(
            left_value->x(),
            right_value->x(),
            bottom_d1->x(),
            top_d1->x(),
            p00.x(),
            p10.x(),
            p01.x(),
            p11.x()),
        u_component(
            left_value->y(),
            right_value->y(),
            bottom_d1->y(),
            top_d1->y(),
            p00.y(),
            p10.y(),
            p01.y(),
            p11.y()),
        u_component(
            left_value->z(),
            right_value->z(),
            bottom_d1->z(),
            top_d1->z(),
            p00.z(),
            p10.z(),
            p01.z(),
            p11.z()));

    auto sv = make_vector(
        v_component(
            left_d1->x(),
            right_d1->x(),
            bottom_value->x(),
            top_value->x(),
            p00.x(),
            p10.x(),
            p01.x(),
            p11.x()),
        v_component(
            left_d1->y(),
            right_d1->y(),
            bottom_value->y(),
            top_value->y(),
            p00.y(),
            p10.y(),
            p01.y(),
            p11.y()),
        v_component(
            left_d1->z(),
            right_d1->z(),
            bottom_value->z(),
            top_value->z(),
            p00.z(),
            p10.z(),
            p01.z(),
            p11.z()));

    if (!su || !sv) {
        return std::unexpected{SurfaceError::non_finite_result};
    }

    if (v == 0.0) {
        su = *bottom_d1;
    } else if (v == 1.0) {
        su = *top_d1;
    }

    if (u == 0.0) {
        sv = *left_d1;
    } else if (u == 1.0) {
        sv = *right_d1;
    }

    return SurfaceFirstDerivatives3{.u = *su, .v = *sv};
}

std::expected<SurfaceSecondDerivatives3, SurfaceError>
CubicBezierCoonsPatch3::second_derivatives(
    const double u,
    const double v) const noexcept {
    const auto valid = validate_parameters(u, v);
    if (!valid.has_value()) {
        return std::unexpected{valid.error()};
    }

    const auto left_d1 = left_.first_derivative(v);
    const auto right_d1 = right_.first_derivative(v);
    const auto bottom_d1 = bottom_.first_derivative(u);
    const auto top_d1 = top_.first_derivative(u);
    const auto left_d2 = left_.second_derivative(v);
    const auto right_d2 = right_.second_derivative(v);
    const auto bottom_d2 = bottom_.second_derivative(u);
    const auto top_d2 = top_.second_derivative(u);
    if (!left_d1 || !right_d1 || !bottom_d1 || !top_d1 ||
        !left_d2 || !right_d2 || !bottom_d2 || !top_d2) {
        return std::unexpected{SurfaceError::non_finite_result};
    }

    const auto& bottom_controls = bottom_.control_points();
    const auto& top_controls = top_.control_points();
    const Point3& p00 = bottom_controls[0];
    const Point3& p10 = bottom_controls[3];
    const Point3& p01 = top_controls[0];
    const Point3& p11 = top_controls[3];

    const long double lu = static_cast<long double>(u);
    const long double lv = static_cast<long double>(v);
    const long double one_minus_u = 1.0L - lu;
    const long double one_minus_v = 1.0L - lv;

    const auto pure_u_component =
        [&](const double bdd, const double tdd) noexcept {
            return one_minus_v * static_cast<long double>(bdd) +
                   lv * static_cast<long double>(tdd);
        };
    const auto pure_v_component =
        [&](const double ldd, const double rdd) noexcept {
            return one_minus_u * static_cast<long double>(ldd) +
                   lu * static_cast<long double>(rdd);
        };
    const auto mixed_component = [&](const double ld,
                                     const double rd,
                                     const double bd,
                                     const double td,
                                     const double c00,
                                     const double c10,
                                     const double c01,
                                     const double c11) noexcept {
        return -static_cast<long double>(ld) +
               static_cast<long double>(rd) -
               static_cast<long double>(bd) +
               static_cast<long double>(td) -
               bilinear_uv_component(c00, c10, c01, c11);
    };

    auto suu = make_vector(
        pure_u_component(bottom_d2->x(), top_d2->x()),
        pure_u_component(bottom_d2->y(), top_d2->y()),
        pure_u_component(bottom_d2->z(), top_d2->z()));
    auto svv = make_vector(
        pure_v_component(left_d2->x(), right_d2->x()),
        pure_v_component(left_d2->y(), right_d2->y()),
        pure_v_component(left_d2->z(), right_d2->z()));
    const auto suv = make_vector(
        mixed_component(
            left_d1->x(),
            right_d1->x(),
            bottom_d1->x(),
            top_d1->x(),
            p00.x(),
            p10.x(),
            p01.x(),
            p11.x()),
        mixed_component(
            left_d1->y(),
            right_d1->y(),
            bottom_d1->y(),
            top_d1->y(),
            p00.y(),
            p10.y(),
            p01.y(),
            p11.y()),
        mixed_component(
            left_d1->z(),
            right_d1->z(),
            bottom_d1->z(),
            top_d1->z(),
            p00.z(),
            p10.z(),
            p01.z(),
            p11.z()));

    if (!suu || !suv || !svv) {
        return std::unexpected{SurfaceError::non_finite_result};
    }

    if (v == 0.0) {
        suu = *bottom_d2;
    } else if (v == 1.0) {
        suu = *top_d2;
    }

    if (u == 0.0) {
        svv = *left_d2;
    } else if (u == 1.0) {
        svv = *right_d2;
    }

    return SurfaceSecondDerivatives3{
        .uu = *suu,
        .uv = *suv,
        .vv = *svv,
    };
}

CubicBezierCoonsPatch3 CubicBezierCoonsPatch3::u_reversed() const noexcept {
    return CubicBezierCoonsPatch3{
        bottom_.reversed(),
        top_.reversed(),
        right_,
        left_};
}

CubicBezierCoonsPatch3 CubicBezierCoonsPatch3::v_reversed() const noexcept {
    return CubicBezierCoonsPatch3{
        top_,
        bottom_,
        left_.reversed(),
        right_.reversed()};
}

} // namespace apmesh::core
