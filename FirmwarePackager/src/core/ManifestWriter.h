#pragma once

#include <filesystem>
#include "ProjectModel.h"

namespace core {

class IManifestWriter {
public:
    virtual ~IManifestWriter() = default;
    virtual void write(const Project& project, const std::filesystem::path& output) const = 0;
};

class ManifestWriter : public IManifestWriter {
public:
    void write(const Project& project, const std::filesystem::path& output) const override;
};

} // namespace core

