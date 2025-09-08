#pragma once

#include <filesystem>
#include "ProjectModel.h"
#include "IdGenerator.h"

namespace core {

class IScriptWriter {
public:
    virtual ~IScriptWriter() = default;
    virtual void write(const Project& project, const std::filesystem::path& output) const = 0;
};

class ScriptWriter : public IScriptWriter {
public:
    explicit ScriptWriter(IIdGenerator& idGen);
    void write(const Project& project, const std::filesystem::path& output) const override;

private:
    IIdGenerator& idGen;
};

} // namespace core

