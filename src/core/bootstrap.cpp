#include "apmesh/core/bootstrap.hpp"

namespace apmesh::core {

BootstrapCertificate inspect_bootstrap() noexcept {
    return BootstrapCertificate{
        .component = "apmesh::core",
        .language = "C++23",
    };
}

} // namespace apmesh::core
