#include "apmesh/core/bootstrap.hpp"

#include <cstdio>

int main() {
    const apmesh::core::BootstrapCertificate certificate = apmesh::core::inspect_bootstrap();
    if (certificate.component != "apmesh::core" || certificate.language != "C++23") {
        std::fputs("consumer received an unexpected bootstrap certificate\n", stderr);
        return 1;
    }
    return 0;
}
