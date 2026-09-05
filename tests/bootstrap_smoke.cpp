#include "apmesh/core/bootstrap.hpp"

#include <cstdio>
#include <expected>
#include <string_view>

namespace {

enum class LocalProbeError {
    unavailable,
};

bool require(const bool condition, const std::string_view message) {
    if (!condition) {
        std::fputs(message.data(), stderr);
        std::fputc('\n', stderr);
    }
    return condition;
}

} // namespace

int main() {
    const apmesh::core::BootstrapCertificate certificate = apmesh::core::inspect_bootstrap();
    const std::expected<int, LocalProbeError> success{42};
    const std::expected<int, LocalProbeError> failure{
        std::unexpected{LocalProbeError::unavailable},
    };

    return require(certificate.component == "apmesh::core", "unexpected component") &&
                   require(certificate.language == "C++23", "unexpected language") &&
                   require(success.has_value() && success.value() == 42, "expected success probe failed") &&
                   require(!failure.has_value() &&
                               failure.error() == LocalProbeError::unavailable,
                           "expected error probe failed")
               ? 0
               : 1;
}
