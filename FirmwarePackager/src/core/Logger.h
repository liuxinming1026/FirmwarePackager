#pragma once

#include <iostream>
#include <string>

namespace core {

class ILogger {
public:
    virtual ~ILogger() = default;
    virtual void info(const std::string& msg) = 0;
    virtual void error(const std::string& msg) = 0;
};

class Logger : public ILogger {
public:
    explicit Logger(std::ostream& out = std::clog);
    void info(const std::string& msg) override;
    void error(const std::string& msg) override;
private:
    std::ostream* stream;
};

} // namespace core

