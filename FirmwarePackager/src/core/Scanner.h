#pragma once

#include <filesystem>
#include <vector>

namespace core {

// Recursively scans a directory expanding all files while honoring exclusions.
class Scanner {
public:
    using PathList = std::vector<std::filesystem::path>;

    // Returns a list of files under 'root' honoring exclusions. If 'recursive' is false
    // only the top level is scanned.
    PathList scan(const std::filesystem::path& root, const PathList& exclusions, bool recursive = true) const;

private:
    bool isExcluded(const std::filesystem::path& path, const PathList& exclusions) const;
};

} // namespace core

