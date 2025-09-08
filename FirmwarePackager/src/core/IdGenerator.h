#pragma once

#include <atomic>
#include <string>

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
    std::atomic<uint64_t> counter;
};

} // namespace core

