#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace core {

class Hasher {
public:
    std::string md5(const std::vector<uint8_t>& data) const;
};

} // namespace core

