#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <filesystem>

namespace core {

class Hasher {
public:
    std::string md5(const std::vector<uint8_t>& data) const;
    std::string md5File(const std::filesystem::path& path) const;
    std::string md5Directory(const std::filesystem::path& path) const;
};

} // namespace core

