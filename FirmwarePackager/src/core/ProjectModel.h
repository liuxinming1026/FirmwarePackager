#pragma once

#include <string>
#include <vector>
#include <filesystem>

namespace core {

struct FileEntry {
    std::filesystem::path path;   // location on disk
    std::string id;               // generated identifier
    std::string hash;             // md5 hash

    FileEntry();
    FileEntry(std::filesystem::path p, std::string i, std::string h);
};

struct Project {
    std::string name;                 // project name
    std::vector<FileEntry> files;     // files included in project

    Project();
    explicit Project(std::string name);
};

} // namespace core

