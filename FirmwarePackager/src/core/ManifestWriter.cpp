#include "ManifestWriter.h"

#include <fstream>

namespace core {

void ManifestWriter::write(const Project& project, const std::filesystem::path& output) const {
    std::ofstream out(output);
    if (!out.is_open()) {
        return;
    }
    out << "path\tid\thash\n";
    for (const auto& f : project.files) {
        out << f.path.string() << '\t' << f.id << '\t' << f.hash << '\n';
    }
}

} // namespace core

