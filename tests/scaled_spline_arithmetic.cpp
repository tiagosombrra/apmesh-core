#include "scaled_arithmetic.hpp"
#include "apmesh/geometry/bspline.hpp"
#include "apmesh/geometry/nurbs.hpp"
#include "apmesh/geometry/nurbs_surface.hpp"

#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <vector>

namespace {
bool require(const bool condition, const char* message) {
    if (!condition) {
        std::cerr << message << '\n';
    }
    return condition;
}

template <typename Curve>
bool affine_curve(const Curve& curve, const double knot, const double scale) {
    for (const double fraction : {-1.0, -0.5, 0.0, 0.5, 1.0}) {
        const auto value = curve.evaluate(fraction * knot);
        const auto first = curve.first_derivative(fraction * knot);
        const auto second = curve.second_derivative(fraction * knot);
        if (!require(value && first && second,
                     "extreme affine curve jet failed")) {
            return false;
        }
        if (!require(value->x() == (3.0 + 3.0 * fraction) * scale &&
                         value->y() == 0.0 &&
                         first->x() == 0.375 && first->y() == 0.0 &&
                         second->x() == 0.0 && second->y() == 0.0,
                     "extreme affine curve analytic jet differs")) {
            return false;
        }
    }
    return true;
}

bool public_spline_contracts() {
    using namespace apmesh::core;
    const double knot = std::scalbn(1.0, 1023);
    const double scale = std::scalbn(1.0, 1020);
    const std::array<double, 5> coefficients{0.0, 1.0, 3.0, 5.0, 6.0};
    const std::array<Point2, 5> points2{
        *Point2::make(0.0, 0.0), *Point2::make(scale, 0.0),
        *Point2::make(3.0 * scale, 0.0), *Point2::make(5.0 * scale, 0.0),
        *Point2::make(6.0 * scale, 0.0)};
    const std::array<Point3, 5> points3{
        *Point3::make(0.0, 0.0, 0.0), *Point3::make(scale, 0.0, 0.0),
        *Point3::make(3.0 * scale, 0.0, 0.0), *Point3::make(5.0 * scale, 0.0, 0.0),
        *Point3::make(6.0 * scale, 0.0, 0.0)};
    const auto bspline2 = TwoSpanCubicBSpline2::make(points2, -knot, 0.0, knot);
    const auto bspline3 = TwoSpanCubicBSpline3::make(points3, -knot, 0.0, knot);
    bool passed = require(bspline2 && bspline3, "affine B-spline construction failed");
    if (!passed) { return false; }
    passed = affine_curve(*bspline2, knot, scale) && passed;
    passed = affine_curve(*bspline3, knot, scale) && passed;

    // Uniform weights at both ends of binary64 range must describe the same jet.
    for (const double weight : {std::numeric_limits<double>::denorm_min(),
                                std::numeric_limits<double>::max()}) {
        const std::array<double, 5> weights{weight, weight, weight, weight, weight};
        const auto nurbs2 = TwoSpanCubicNURBS2::make(points2, weights, -knot, 0.0, knot);
        const auto nurbs3 = TwoSpanCubicNURBS3::make(points3, weights, -knot, 0.0, knot);
        const auto multi2 = MultiSpanCubicNURBS2::make(
            std::vector<Point2>(points2.begin(), points2.end()),
            std::vector<double>(5U, weight), {0.0}, -knot, knot);
        const auto multi3 = MultiSpanCubicNURBS3::make(
            std::vector<Point3>(points3.begin(), points3.end()),
            std::vector<double>(5U, weight), {0.0}, -knot, knot);
        if (!require(nurbs2 && nurbs3 && multi2 && multi3,
                     "affine NURBS construction failed")) { return false; }
        passed = affine_curve(*nurbs2, knot, scale) && passed;
        passed = affine_curve(*nurbs3, knot, scale) && passed;
        passed = affine_curve(*multi2, knot, scale) && passed;
        passed = affine_curve(*multi3, knot, scale) && passed;

        std::vector<Point3> net;
        for (const double u : coefficients) {
            for (const double v : coefficients) {
                net.push_back(*Point3::make(u * scale, v * scale, 0.0));
            }
        }
        const auto surface = BicubicNURBSSurface3::make(
            net, std::vector<double>(25U, weight), 5U, 5U,
            {0.0}, {0.0}, -knot, knot, -knot, knot);
        if (!require(surface.has_value(), "affine NURBS surface construction failed")) {
            return false;
        }
        for (const double fraction : {-1.0, -0.5, 0.0, 0.5, 1.0}) {
            const auto value = surface->evaluate(fraction * knot, -fraction * knot);
            const auto first = surface->first_derivatives(fraction * knot, -fraction * knot);
            const auto second = surface->second_derivatives(fraction * knot, -fraction * knot);
            if (!require(value && first && second, "extreme affine surface jet failed")) {
                return false;
            }
            passed = require(
                value->x() == (3.0 + 3.0 * fraction) * scale &&
                value->y() == (3.0 - 3.0 * fraction) * scale && value->z() == 0.0 &&
                first->u.x() == 0.375 && first->u.y() == 0.0 && first->u.z() == 0.0 &&
                first->v.x() == 0.0 && first->v.y() == 0.375 && first->v.z() == 0.0 &&
                second->uu == *Vector3::make(0.0, 0.0, 0.0) &&
                second->uv == *Vector3::make(0.0, 0.0, 0.0) &&
                second->vv == *Vector3::make(0.0, 0.0, 0.0),
                "extreme affine surface analytic jet differs") && passed;
        }
    }
    return passed;
}
} // namespace

int main() {
    using apmesh::core::detail::Scaled;
    using apmesh::core::detail::is_finite;
    using apmesh::core::detail::lerp;
    bool passed = true;
    const Scaled maximum{std::numeric_limits<double>::max()};
    const Scaled minimum{std::numeric_limits<double>::denorm_min()};
    const Scaled one{1.0L};
    const Scaled two{2.0L};

    // Expected results are binary identities, independent of spline formulas.
    const Scaled twice_maximum = maximum - (-maximum);
    passed = require(is_finite(twice_maximum) &&
                         twice_maximum > maximum &&
                         static_cast<double>(twice_maximum / maximum) == 2.0,
                     "overflowing knot difference was materialized") && passed;
    const Scaled tiny_weight = minimum / maximum;
    passed = require(is_finite(tiny_weight) && tiny_weight > 0.0L &&
                         static_cast<double>(tiny_weight * maximum) ==
                             std::numeric_limits<double>::denorm_min(),
                     "positive weight ratio underflowed before recovery") && passed;

    const Scaled huge_derivative = maximum / minimum;
    passed = require(is_finite(huge_derivative) &&
                         static_cast<double>(huge_derivative * minimum) ==
                             std::numeric_limits<double>::max(),
                     "derivative-control scale could not be recovered") && passed;
    passed = require(huge_derivative - huge_derivative == 0.0L &&
                         static_cast<double>(lerp(-huge_derivative,
                                                  huge_derivative, 0.5L)) == 0.0,
                     "cancellation outside binary64 range differs") && passed;
    passed = require(lerp(minimum, maximum, 0.0L) == minimum &&
                         lerp(minimum, maximum, 1.0L) == maximum &&
                         lerp(maximum, maximum, 0.375L) == maximum,
                     "exact interpolation endpoints/constant differ") && passed;

    // Exhaust the binary64 power-of-two exponent envelope, including subnormals.
    for (int exponent = -1074; exponent <= 1023; ++exponent) {
        const double power = std::scalbn(1.0, exponent);
        const Scaled value{power};
        passed = require(static_cast<double>(value) == power &&
                             static_cast<double>((value * maximum) / maximum) == power &&
                             static_cast<double>((value / minimum) * minimum) == power &&
                             static_cast<double>(value + (-value)) == 0.0,
                         "power-of-two scale identity differs") && passed;
    }

    passed = require(-maximum < -minimum && -minimum < 0.0L &&
                         minimum < one && one < maximum &&
                         maximum < twice_maximum,
                     "signed scaled ordering differs") && passed;
    const Scaled invalid{std::numeric_limits<double>::infinity()};
    passed = require(!is_finite(invalid) && !is_finite(one / 0.0L) &&
                         !is_finite(invalid * 0.0L) &&
                         !is_finite(invalid + one) &&
                         static_cast<double>(minimum / two) == 0.0 &&
                         static_cast<double>((minimum * 3.0L) / two) ==
                             std::numeric_limits<double>::denorm_min() * 2.0,
                     "invalid propagation or final subnormal rounding differs") && passed;
    passed = public_spline_contracts() && passed;
    return passed ? 0 : 1;
}
