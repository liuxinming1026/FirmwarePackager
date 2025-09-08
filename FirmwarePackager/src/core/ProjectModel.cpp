#include "ProjectModel.h"

namespace core {

FileEntry::FileEntry() = default;

FileEntry::FileEntry(std::filesystem::path p, std::string i, std::string h)
    : path(std::move(p)), id(std::move(i)), hash(std::move(h)) {}

Project::Project() = default;

Project::Project(std::string name) : name(std::move(name)) {}

} // namespace core

