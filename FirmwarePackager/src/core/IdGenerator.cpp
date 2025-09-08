#include "IdGenerator.h"

#include <sstream>
#include <iomanip>

namespace core {

IdGenerator::IdGenerator() : counter(0) {}

std::string IdGenerator::generate() {
    uint64_t value = counter.fetch_add(1, std::memory_order_relaxed);
    std::ostringstream oss;
    oss << std::hex << std::setw(16) << std::setfill('0') << value;
    return oss.str();
}

} // namespace core

