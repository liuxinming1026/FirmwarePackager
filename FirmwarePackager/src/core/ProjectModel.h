#pragma once

#include <string>
#include <vector>
#include <filesystem>

namespace core {

struct FileEntry {
    std::filesystem::path path;   // location on disk
    std::filesystem::path dest;   // destination inside package
    std::string id;               // generated identifier
    std::string hash;             // md5 hash
    std::string mode;             // file mode
    std::string owner;            // file owner
    std::string group;            // file group
    bool recursive;               // whether to traverse directories
    std::vector<std::filesystem::path> excludes; // paths to exclude

    FileEntry();
    FileEntry(std::filesystem::path p, std::string i, std::string h);
};

struct Project {
    std::string name;                 // project/package name
    std::string version;              // package version
    std::filesystem::path rootDir;    // project root directory
    std::filesystem::path outputDir;  // output directory
    std::vector<FileEntry> files;     // files included in project

    Project();
    explicit Project(std::string name);
};

} // namespace core

