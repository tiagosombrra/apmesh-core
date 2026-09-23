#include "apmesh/core/geometry.hpp"
#include "apmesh/core/numeric.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <expected>
#include <limits>
#include <string_view>

namespace {

using apmesh::core::AxisPlacement3;
using apmesh::core::CartesianFrame3;
using apmesh::core::GeometryError;
using apmesh::core::Mat3;
using apmesh::core::Point3;
using apmesh::core::ProximityPolicy;
using apmesh::core::ProximityResult;
using apmesh::core::Vector3;

bool require(const bool condition, const std::string_view message) {
    if (!condition) {
        std::fputs(message.data(), stderr);
        std::fputc('\n', stderr);
    }
    return condition;
}

bool close_scalar(
    const double lhs,
    const double rhs,
    const double scale = 1.0,
    const double absolute = 2.0e-13,
    const double relative = 2.0e-13) {
    const auto comparison = apmesh::core::compare_proximity(
        lhs,
        rhs,
        ProximityPolicy{
            .absolute_tolerance = absolute * std::max(1.0, std::abs(scale)),
            .relative_tolerance = relative,
            .reference_scale = std::max(1.0, std::abs(scale)),
        });
    return comparison.has_value() &&
           comparison->result == ProximityResult::within;
}

bool close_vector(
    const Vector3& lhs,
    const Vector3& rhs,
    const double scale = 1.0) {
    return close_scalar(lhs.x(), rhs.x(), scale) &&
           close_scalar(lhs.y(), rhs.y(), scale) &&
           close_scalar(lhs.z(), rhs.z(), scale);
}

bool close_point(
    const Point3& lhs,
    const Point3& rhs,
    const double scale = 1.0) {
    return close_scalar(lhs.x(), rhs.x(), scale) &&
           close_scalar(lhs.y(), rhs.y(), scale) &&
           close_scalar(lhs.z(), rhs.z(), scale);
}

template <typename Value>
bool require_error(
    const std::expected<Value, GeometryError>& value,
    const GeometryError expected,
    const std::string_view message) {
    return require(
        !value.has_value() && value.error() == expected,
        message);
}

struct LongVector3 {
    long double x{};
    long double y{};
    long double z{};
};

LongVector3 normalize_long(const LongVector3 value) {
    const long double scale =
        std::max({std::abs(value.x), std::abs(value.y), std::abs(value.z)});
    const LongVector3 scaled{
        value.x / scale,
        value.y / scale,
        value.z / scale,
    };
    const long double length =
        std::hypot(scaled.x, scaled.y, scaled.z);
    return {
        scaled.x / length,
        scaled.y / length,
        scaled.z / length,
    };
}

LongVector3 cross_long(
    const LongVector3 lhs,
    const LongVector3 rhs) {
    return {
        lhs.y * rhs.z - lhs.z * rhs.y,
        lhs.z * rhs.x - lhs.x * rhs.z,
        lhs.x * rhs.y - lhs.y * rhs.x,
    };
}

bool close_vector_to_long(
    const Vector3& value,
    const LongVector3 reference,
    const double scale = 1.0) {
    return close_scalar(value.x(), static_cast<double>(reference.x), scale) &&
           close_scalar(value.y(), static_cast<double>(reference.y), scale) &&
           close_scalar(value.z(), static_cast<double>(reference.z), scale);
}

int permutation_sign(const std::array<int, 3>& permutation) {
    int inversions = 0;
    for (std::size_t i = 0; i < permutation.size(); ++i) {
        for (std::size_t j = i + 1; j < permutation.size(); ++j) {
            if (permutation[i] > permutation[j]) {
                ++inversions;
            }
        }
    }
    return inversions % 2 == 0 ? 1 : -1;
}

Vector3 signed_axis(const int axis, const int sign) {
    const double sx = axis == 0 ? static_cast<double>(sign) : 0.0;
    const double sy = axis == 1 ? static_cast<double>(sign) : 0.0;
    const double sz = axis == 2 ? static_cast<double>(sign) : 0.0;
    return Vector3::make(sx, sy, sz).value();
}

std::array<double, 9> basis_entries(
    const Vector3& x_axis,
    const Vector3& y_axis,
    const Vector3& z_axis) {
    return {
        x_axis.x(), y_axis.x(), z_axis.x(),
        x_axis.y(), y_axis.y(), z_axis.y(),
        x_axis.z(), y_axis.z(), z_axis.z(),
    };
}

} // namespace

int main() {
    bool passed = true;

    const auto zero_point = Point3::make(0.0, 0.0, 0.0);
    const auto x_axis = Vector3::make(1.0, 0.0, 0.0);
    const auto y_axis = Vector3::make(0.0, 1.0, 0.0);
    const auto z_axis = Vector3::make(0.0, 0.0, 1.0);
    if (!zero_point || !x_axis || !y_axis || !z_axis) {
        return 1;
    }

    const AxisPlacement3 identity = AxisPlacement3::identity();
    passed = require(
                 identity.origin() == *zero_point &&
                     identity.x_direction() == *x_axis &&
                     identity.y_direction() == *y_axis &&
                     identity.z_direction() == *z_axis,
                 "AxisPlacement3 identity storage differs") &&
             passed;

    const auto local_point = Point3::make(1.0, -2.0, 3.0);
    const auto local_vector = Vector3::make(-4.0, 5.0, 6.0);
    if (!local_point || !local_vector) {
        return 1;
    }
    passed = require(
                 identity.point_to_world(*local_point) == local_point &&
                     identity.point_to_local(*local_point) == local_point &&
                     identity.vector_to_world(*local_vector) == local_vector &&
                     identity.vector_to_local(*local_vector) == local_vector,
                 "AxisPlacement3 identity transform differs") &&
             passed;

    const auto arbitrary_origin = Point3::make(7.0, -11.0, 5.0);
    const auto main_direction = Vector3::make(1.0, 2.0, 3.0);
    const auto x_reference = Vector3::make(4.0, -2.0, 1.0);
    if (!arbitrary_origin || !main_direction || !x_reference) {
        return 1;
    }

    const auto arbitrary =
        AxisPlacement3::make(
            *arbitrary_origin, *main_direction, *x_reference);
    if (!arbitrary) {
        return 1;
    }

    const LongVector3 reference_z =
        normalize_long({1.0L, 2.0L, 3.0L});
    const LongVector3 reference_r =
        normalize_long({4.0L, -2.0L, 1.0L});
    const LongVector3 reference_y =
        normalize_long(cross_long(reference_z, reference_r));
    const LongVector3 reference_x =
        normalize_long(cross_long(reference_y, reference_z));

    passed = require(
                 arbitrary->origin() == *arbitrary_origin &&
                     close_vector_to_long(
                         arbitrary->x_direction(), reference_x) &&
                     close_vector_to_long(
                         arbitrary->y_direction(), reference_y) &&
                     close_vector_to_long(
                         arbitrary->z_direction(), reference_z),
                 "arbitrary placement independent orientation differs") &&
             passed;

    const auto x_norm = apmesh::core::norm(arbitrary->x_direction());
    const auto y_norm = apmesh::core::norm(arbitrary->y_direction());
    const auto z_norm = apmesh::core::norm(arbitrary->z_direction());
    const auto xy_dot =
        apmesh::core::dot(
            arbitrary->x_direction(), arbitrary->y_direction());
    const auto xz_dot =
        apmesh::core::dot(
            arbitrary->x_direction(), arbitrary->z_direction());
    const auto yz_dot =
        apmesh::core::dot(
            arbitrary->y_direction(), arbitrary->z_direction());
    const auto handed =
        apmesh::core::cross(
            arbitrary->x_direction(), arbitrary->y_direction());

    passed = require(
                 x_norm && y_norm && z_norm &&
                     close_scalar(*x_norm, 1.0) &&
                     close_scalar(*y_norm, 1.0) &&
                     close_scalar(*z_norm, 1.0) &&
                     xy_dot && xz_dot && yz_dot &&
                     close_scalar(*xy_dot, 0.0) &&
                     close_scalar(*xz_dot, 0.0) &&
                     close_scalar(*yz_dot, 0.0) &&
                     handed &&
                     close_vector(*handed, arbitrary->z_direction()),
                 "arbitrary placement orthonormal/right-handed contract differs") &&
             passed;

    const auto round_trip_point =
        arbitrary->point_to_world(*local_point);
    const auto round_trip_vector =
        arbitrary->vector_to_world(*local_vector);
    const auto recovered_point =
        round_trip_point
            ? arbitrary->point_to_local(*round_trip_point)
            : std::expected<Point3, GeometryError>{
                  std::unexpected{GeometryError::non_finite_result}};
    const auto recovered_vector =
        round_trip_vector
            ? arbitrary->vector_to_local(*round_trip_vector)
            : std::expected<Vector3, GeometryError>{
                  std::unexpected{GeometryError::non_finite_result}};

    passed = require(
                 round_trip_point && recovered_point &&
                     close_point(*recovered_point, *local_point, 8.0) &&
                     round_trip_vector && recovered_vector &&
                     close_vector(*recovered_vector, *local_vector, 8.0),
                 "arbitrary placement local/world round trip differs") &&
             passed;

    std::array<int, 3> permutation{0, 1, 2};
    std::size_t right_handed_basis_count = 0U;
    do {
        const int parity = permutation_sign(permutation);
        for (const int sx : {-1, 1}) {
            for (const int sy : {-1, 1}) {
                for (const int sz : {-1, 1}) {
                    if (parity * sx * sy * sz != 1) {
                        continue;
                    }
                    ++right_handed_basis_count;

                    const Vector3 bx =
                        signed_axis(permutation[0], sx);
                    const Vector3 by =
                        signed_axis(permutation[1], sy);
                    const Vector3 bz =
                        signed_axis(permutation[2], sz);
                    const auto basis =
                        Mat3::make(basis_entries(bx, by, bz));
                    const auto origin =
                        Point3::make(3.0, -5.0, 7.0);
                    if (!basis || !origin) {
                        return 1;
                    }
                    const auto frame =
                        CartesianFrame3::make(*origin, *basis, 0);
                    const auto placement =
                        AxisPlacement3::make(*origin, bz, bx);
                    if (!frame || !placement) {
                        return 1;
                    }

                    const auto frame_point =
                        frame->point_to_world(*local_point);
                    const auto placement_point =
                        placement->point_to_world(*local_point);
                    const auto frame_vector =
                        frame->vector_to_world(*local_vector);
                    const auto placement_vector =
                        placement->vector_to_world(*local_vector);
                    passed = require(
                                 placement->x_direction() == bx &&
                                     placement->y_direction() == by &&
                                     placement->z_direction() == bz &&
                                     frame_point && placement_point &&
                                     *frame_point == *placement_point &&
                                     frame_vector && placement_vector &&
                                     *frame_vector == *placement_vector,
                                 "qualified signed-permutation parity differs") &&
                             passed;
                }
            }
        }
    } while (std::next_permutation(
        permutation.begin(), permutation.end()));

    passed = require(
                 right_handed_basis_count == 24U,
                 "signed-permutation parity did not cover all 24 right-handed bases") &&
             passed;

    const double high_scale = std::scalbn(1.0, 500);
    const double low_scale = std::scalbn(1.0, -500);
    const auto scaled_main =
        Vector3::make(
            main_direction->x() * high_scale,
            main_direction->y() * high_scale,
            main_direction->z() * high_scale);
    const auto scaled_reference =
        Vector3::make(
            x_reference->x() * low_scale,
            x_reference->y() * low_scale,
            x_reference->z() * low_scale);
    if (!scaled_main || !scaled_reference) {
        return 1;
    }

    const auto scale_main_only =
        AxisPlacement3::make(
            *arbitrary_origin, *scaled_main, *x_reference);
    const auto scale_reference_only =
        AxisPlacement3::make(
            *arbitrary_origin, *main_direction, *scaled_reference);
    const auto scale_both =
        AxisPlacement3::make(
            *arbitrary_origin, *scaled_main, *scaled_reference);
    if (!scale_main_only || !scale_reference_only || !scale_both) {
        return 1;
    }

    for (const AxisPlacement3* candidate :
         {&*scale_main_only, &*scale_reference_only, &*scale_both}) {
        passed = require(
                     close_vector(
                         candidate->x_direction(),
                         arbitrary->x_direction()) &&
                         close_vector(
                             candidate->y_direction(),
                             arbitrary->y_direction()) &&
                         close_vector(
                             candidate->z_direction(),
                             arbitrary->z_direction()),
                     "power-of-two direction scaling changed placement") &&
                 passed;
    }

    const auto zero_vector = Vector3::make(0.0, 0.0, 0.0);
    const auto parallel = Vector3::make(2.0, 4.0, 6.0);
    const auto antiparallel = Vector3::make(-2.0, -4.0, -6.0);
    if (!zero_vector || !parallel || !antiparallel) {
        return 1;
    }
    passed = require_error(
                 AxisPlacement3::make(
                     *zero_point, *zero_vector, *x_reference),
                 GeometryError::invalid_frame,
                 "zero main direction was accepted") &&
             passed;
    passed = require_error(
                 AxisPlacement3::make(
                     *zero_point, *main_direction, *zero_vector),
                 GeometryError::invalid_frame,
                 "zero X reference was accepted") &&
             passed;
    passed = require_error(
                 AxisPlacement3::make(
                     *zero_point, *main_direction, *parallel),
                 GeometryError::invalid_frame,
                 "parallel directions were accepted") &&
             passed;
    passed = require_error(
                 AxisPlacement3::make(
                     *zero_point, *main_direction, *antiparallel),
                 GeometryError::invalid_frame,
                 "antiparallel directions were accepted") &&
             passed;

    const double subnormal = std::numeric_limits<double>::denorm_min();
    const auto near_main = Vector3::make(1.0, 0.0, 0.0);
    const auto near_reference = Vector3::make(1.0, subnormal, 0.0);
    if (!near_main || !near_reference) {
        return 1;
    }
    const auto near_parallel =
        AxisPlacement3::make(
            *zero_point, *near_main, *near_reference);
    passed = require(
                 near_parallel.has_value() &&
                     near_parallel->z_direction() == *near_main &&
                     near_parallel->x_direction() == *y_axis &&
                     near_parallel->y_direction() == *z_axis,
                 "near-parallel finite directions were threshold-rejected") &&
             passed;

    const double maximum = std::numeric_limits<double>::max();
    const auto extreme_main =
        Vector3::make(maximum, maximum / 2.0, maximum / 4.0);
    const auto extreme_reference =
        Vector3::make(maximum / 8.0, -maximum / 4.0, maximum / 16.0);
    const auto extreme_origin =
        Point3::make(1.0e100, -1.0e100, 5.0e99);
    if (!extreme_main || !extreme_reference || !extreme_origin) {
        return 1;
    }
    const auto extreme =
        AxisPlacement3::make(
            *extreme_origin, *extreme_main, *extreme_reference);
    passed = require(
                 extreme.has_value(),
                 "extreme finite direction magnitudes were rejected") &&
             passed;
    if (extreme) {
        const auto extreme_local =
            Vector3::make(1.0e90, -2.0e90, 3.0e90);
        if (!extreme_local) {
            return 1;
        }
        const auto extreme_world =
            extreme->vector_to_world(*extreme_local);
        const auto extreme_recovered =
            extreme_world
                ? extreme->vector_to_local(*extreme_world)
                : std::expected<Vector3, GeometryError>{
                      std::unexpected{GeometryError::non_finite_result}};
        passed = require(
                     extreme_world && extreme_recovered &&
                         close_vector(
                             *extreme_recovered,
                             *extreme_local,
                             3.0e90),
                     "extreme finite placement round trip differs") &&
                 passed;
    }

    const auto maximum_origin =
        Point3::make(maximum, 0.0, 0.0);
    const auto maximum_local =
        Point3::make(maximum, 0.0, 0.0);
    if (!maximum_origin || !maximum_local) {
        return 1;
    }
    const auto translated_identity =
        AxisPlacement3::make(
            *maximum_origin, *z_axis, *x_axis);
    if (!translated_identity) {
        return 1;
    }
    passed = require_error(
                 translated_identity->point_to_world(*maximum_local),
                 GeometryError::non_finite_result,
                 "non-representable translated point was accepted") &&
             passed;

    const auto repeated_a =
        AxisPlacement3::make(
            *arbitrary_origin, *main_direction, *x_reference);
    const auto repeated_b =
        AxisPlacement3::make(
            *arbitrary_origin, *main_direction, *x_reference);
    passed = require(
                 repeated_a && repeated_b &&
                     *repeated_a == *repeated_b &&
                     repeated_a->point_to_world(*local_point) ==
                         repeated_b->point_to_world(*local_point) &&
                     repeated_a->vector_to_local(*local_vector) ==
                         repeated_b->vector_to_local(*local_vector),
                 "AxisPlacement3 repeated evidence is not deterministic") &&
             passed;

    return passed ? 0 : 1;
}
