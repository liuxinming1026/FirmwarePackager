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
    // Write project manifest to output path.
    // The manifest is a tab-separated file with the following columns:
    //   relpath, dest, mode, owner, group and md5.
    // Directory entries are expanded to file-level records and receive a
    // virtual md5 calculated from their children.
    void write(const Project& project, const std::filesystem::path& output) const override;

private:
    static std::string md5File(const std::filesystem::path& p);
    static std::string md5String(const std::string& data);
};

} // namespace core

