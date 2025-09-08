#pragma once

#include <filesystem>
#include "ProjectModel.h"
#include "Scanner.h"
#include "Hasher.h"
#include "ManifestWriter.h"
#include "ScriptWriter.h"
#include "IdGenerator.h"
#include "Logger.h"

namespace core {

class IPackager {
public:
    virtual ~IPackager() = default;
    virtual Project buildProject(const std::filesystem::path& root, const Scanner::PathList& exclusions) = 0;
    virtual void package(const Project& project) = 0;
};

class Packager : public IPackager {
public:
    Packager(Scanner& scanner, Hasher& hasher, IManifestWriter& manifest,
             IScriptWriter& script, IIdGenerator& idGen, ILogger& logger);

    Project buildProject(const std::filesystem::path& root, const Scanner::PathList& exclusions) override;
    void package(const Project& project) override;

private:
    Scanner& scanner;
    Hasher& hasher;
    IManifestWriter& manifest;
    IScriptWriter& script;
    IIdGenerator& idGen;
    ILogger& logger;

    void createArchive(const std::filesystem::path& dir, const std::filesystem::path& archive);
};

} // namespace core

