#include "Logger.h"

#include <iostream>

namespace core {

Logger::Logger(std::ostream& out) : stream(&out) {}

void Logger::info(const std::string& msg) {
    (*stream) << "[INFO] " << msg << std::endl;
}

void Logger::error(const std::string& msg) {
    (*stream) << "[ERROR] " << msg << std::endl;
}

} // namespace core

