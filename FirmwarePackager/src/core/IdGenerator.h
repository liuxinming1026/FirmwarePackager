#pragma once

#include <string>
#include <random>
#include <mutex>

namespace core {

class IIdGenerator {
public:
    virtual ~IIdGenerator() = default;
    virtual std::string generate() = 0;
};

class IdGenerator : public IIdGenerator {
public:
    IdGenerator();
    std::string generate() override;
private:
    std::mt19937 rng;
    std::mutex mtx;
    std::uniform_int_distribution<uint32_t> dist;
};

} // namespace core

