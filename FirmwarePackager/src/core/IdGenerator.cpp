#include "IdGenerator.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace core {

IdGenerator::IdGenerator() : rng(std::random_device{}()), dist(0, 0xFFFFFFFF) {}

std::string IdGenerator::generate() {
    using namespace std::chrono;
    auto now = system_clock::now();
    std::time_t t = system_clock::to_time_t(now);
    std::tm tm;
#ifdef _WIN32
    gmtime_s(&tm, &t);
#else
    gmtime_r(&t, &tm);
#endif

    char buf[16];
    std::strftime(buf, sizeof(buf), "%Y%m%d-%H%M%S", &tm);

    uint32_t r;
    {
        std::lock_guard<std::mutex> lock(mtx);
        r = dist(rng);
    }

    std::ostringstream oss;
    oss << buf << '-' << std::hex << std::setw(8) << std::setfill('0') << r;
    return oss.str();
}

} // namespace core

