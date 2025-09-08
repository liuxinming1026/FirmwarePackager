#pragma once

#include <filesystem>
#include "ProjectModel.h"

namespace core {

class IScriptWriter {
public:
    virtual ~IScriptWriter() = default;
    virtual void write(const Project& project, const std::filesystem::path& output) const = 0;
};

class ScriptWriter : public IScriptWriter {
public:
    void write(const Project& project, const std::filesystem::path& output) const override;
};

} // namespace core

