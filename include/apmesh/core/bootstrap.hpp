#pragma once

#include <string_view>

namespace apmesh::core {

struct BootstrapCertificate {
    std::string_view component;
    std::string_view language;
};

[[nodiscard]] BootstrapCertificate inspect_bootstrap() noexcept;

} // namespace apmesh::core
