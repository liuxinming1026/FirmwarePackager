#include "ProjectSerializer.h"

#include <fstream>
#include <nlohmann/json.hpp>

namespace core {

using nlohmann::json;

Project ProjectSerializer::load(const std::string& filePath) const {
    std::ifstream in(filePath);
    if (!in)
        throw std::runtime_error("Unable to open project file");
    json j;
    in >> j;
    Project project;
    project.name = j.value("name", "");
    project.version = j.value("version", "");
    project.pkgId = j.value("pkgId", "");
    project.rootDir = j.value("rootDir", "");
    project.outputDir = j.value("outputDir", "");
    if (j.contains("files")) {
        for (const auto& item : j["files"]) {
            FileEntry entry;
            entry.path = item.value("path", "");
            entry.dest = item.value("dest", "");
            entry.id = item.value("id", "");
            entry.hash = item.value("hash", "");
            entry.mode = item.value("mode", "");
            entry.owner = item.value("owner", "");
            entry.group = item.value("group", "");
            entry.recursive = item.value("recursive", false);
            if (item.contains("excludes")) {
                for (const auto& ex : item["excludes"])
                    entry.excludes.push_back(ex.get<std::string>());
            }
            project.files.push_back(entry);
        }
    }
    return project;
}

void ProjectSerializer::save(const Project& project, const std::string& filePath) const {
    json j;
    j["name"] = project.name;
    j["version"] = project.version;
    j["pkgId"] = project.pkgId;
    j["rootDir"] = project.rootDir.string();
    j["outputDir"] = project.outputDir.string();
    j["files"] = json::array();
    for (const auto& file : project.files) {
        json item;
        item["path"] = file.path.string();
        item["dest"] = file.dest.string();
        item["id"] = file.id;
        item["hash"] = file.hash;
        item["mode"] = file.mode;
        item["owner"] = file.owner;
        item["group"] = file.group;
        item["recursive"] = file.recursive;
        json ex = json::array();
        for (const auto& e : file.excludes)
            ex.push_back(e.string());
        item["excludes"] = ex;
        j["files"].push_back(item);
    }
    std::ofstream out(filePath);
    out << j.dump(4);
}

} // namespace core

