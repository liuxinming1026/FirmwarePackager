#include "ProjectModel.h"

namespace core {

FileEntry::FileEntry()
    : owner("root"), group("root"), recursive(false) {}

FileEntry::FileEntry(std::filesystem::path p, std::string i, std::string h)
    : path(std::move(p)), dest(path), id(std::move(i)), hash(std::move(h)),
      mode(""), owner("root"), group("root"), recursive(false) {}

Project::Project() = default;

Project::Project(std::string name)
    : name(std::move(name)) {}

} // namespace core

