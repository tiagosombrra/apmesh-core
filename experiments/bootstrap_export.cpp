#include "apmesh/core/bootstrap.hpp"

#include <fstream>
#include <string_view>

namespace {

int write_certificate(const std::string_view output_path) {
    const apmesh::core::BootstrapCertificate certificate = apmesh::core::inspect_bootstrap();
    std::ofstream output{output_path.data(), std::ios::binary | std::ios::trunc};
    if (!output.is_open()) {
        return 2;
    }

    output << "{\"schema_version\":1,\"component\":\"" << certificate.component
           << "\",\"language\":\"" << certificate.language << "\"}\n";
    output.flush();
    return output.good() ? 0 : 3;
}

} // namespace

int main(const int argument_count, char* const arguments[]) {
    if (argument_count != 2) {
        return 1;
    }
    return write_certificate(arguments[1]);
}
