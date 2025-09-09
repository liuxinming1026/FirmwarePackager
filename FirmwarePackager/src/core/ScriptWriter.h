#pragma once

#include <filesystem>
#include <string>
#include "ProjectModel.h"

namespace core {

class IScriptWriter {
public:
    virtual ~IScriptWriter() = default;
    virtual void write(const Project& project, const std::filesystem::path& output,
                      const std::string& pkgId,
                      const std::filesystem::path& templateRoot) const = 0;
};

class ScriptWriter : public IScriptWriter {
public:
    ScriptWriter() = default;
    void write(const Project& project, const std::filesystem::path& output,
               const std::string& pkgId,
               const std::filesystem::path& templateRoot) const override;
};

} // namespace core

