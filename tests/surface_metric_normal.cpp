#include "apmesh/core/numeric.hpp"
#include "apmesh/geometry/coons_surface.hpp"
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
#include <cstdio>
#include <limits>
#include <string_view>
#include <vector>

namespace {

bool require(const bool condition, const std::string_view message) {
    if (!condition) {
        std::fputs(message.data(), stderr);
        std::fputc('\n', stderr);
    }
    return condition;
}

} // namespace

int main() {
    return 0;
}
