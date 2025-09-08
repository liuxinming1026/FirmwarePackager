#pragma once

#include <filesystem>
#include <vector>

namespace core {

// Recursively scans a directory expanding all files while honoring exclusions.
class Scanner {
public:
    using PathList = std::vector<std::filesystem::path>;

    // Returns a list of all files under 'root' that are not excluded.
    PathList scan(const std::filesystem::path& root, const PathList& exclusions) const;

private:
    bool isExcluded(const std::filesystem::path& path, const PathList& exclusions) const;
};

} // namespace core

