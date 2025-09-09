#include "ProjectModel.h"
#include <system_error>

namespace core {

FileEntry::FileEntry()
    : owner("root"), group("root"), recursive(false) {}

FileEntry::FileEntry(std::filesystem::path p, std::string i, std::string h)
    : path(std::move(p)), dest(path), id(std::move(i)), hash(std::move(h)),
      mode(""), owner("root"), group("root"), recursive(false) {
    std::error_code ec;
    if (std::filesystem::is_directory(path, ec)) {
        mode = "0755";
    } else {
        mode = "0644";
    }
}

Project::Project()
    : pkgId("") {}

Project::Project(std::string name)
    : name(std::move(name)), pkgId("") {}

} // namespace core

