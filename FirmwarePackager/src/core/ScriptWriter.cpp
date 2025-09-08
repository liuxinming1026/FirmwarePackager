#include "ScriptWriter.h"

#include <fstream>
#include <sstream>

namespace core {

namespace {
std::string replaceAll(std::string str, const std::string& from, const std::string& to) {
    size_t pos = 0;
    while ((pos = str.find(from, pos)) != std::string::npos) {
        str.replace(pos, from.length(), to);
        pos += to.length();
    }
    return str;
}
}

void ScriptWriter::write(const Project& project, const std::filesystem::path& output) const {
    std::filesystem::create_directories(output);

    // Build replacement values
    std::string pkgName = project.name;
    std::string pkgId = std::to_string(std::hash<std::string>{}(project.name));
    std::string pkgVersion = project.version;

    std::string files;
    for (const auto& f : project.files) {
        if (!files.empty()) files += ' ';
        files += f.path.string();
    }

    std::filesystem::path tplDir = std::filesystem::path("templates") / "scripts";
    for (const auto& entry : std::filesystem::recursive_directory_iterator(tplDir)) {
        if (!entry.is_regular_file()) continue;
        std::ifstream in(entry.path());
        std::stringstream buffer;
        buffer << in.rdbuf();
        std::string content = buffer.str();

        content = replaceAll(content, "@PKG_ID@", pkgId);
        content = replaceAll(content, "@PKG_NAME@", pkgName);
        content = replaceAll(content, "@PKG_VERSION@", pkgVersion);
        content = replaceAll(content, "@FILES@", files);

        std::filesystem::path rel = std::filesystem::relative(entry.path(), tplDir);
        std::filesystem::path outFile = output / rel;
        if (outFile.extension() == ".in") {
            outFile.replace_extension("");
        }
        std::filesystem::create_directories(outFile.parent_path());
        std::ofstream out(outFile);
        out << content;
        out.close();
        std::filesystem::permissions(
            outFile,
            std::filesystem::perms::owner_read | std::filesystem::perms::owner_write |
                std::filesystem::perms::owner_exec | std::filesystem::perms::group_read |
                std::filesystem::perms::group_exec | std::filesystem::perms::others_read |
                std::filesystem::perms::others_exec);
    }
}

} // namespace core

