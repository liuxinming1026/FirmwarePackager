#include "ProjectModel.h"

namespace core {

FileEntry::FileEntry()
    : recursive(false) {}

FileEntry::FileEntry(std::filesystem::path p, std::string i, std::string h)
    : path(std::move(p)), dest(path), id(std::move(i)), hash(std::move(h)),
      mode(""), owner(""), group(""), recursive(false) {}

Project::Project() = default;

Project::Project(std::string name)
    : name(std::move(name)) {}

} // namespace core

