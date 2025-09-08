#include <gtest/gtest.h>
#include "core/ScriptWriter.h"
#include "core/ProjectModel.h"
#include <filesystem>
#include <fstream>
#include <sstream>

using namespace std::filesystem;

TEST(ScriptWriterTest, GeneratesScriptsWithReplacements){
    core::Project project;
    project.name = "demo";
    project.version = "1.0";
    core::FileEntry fe; fe.path = "file1"; project.files.push_back(fe);

    path out = temp_directory_path()/"sw_out";
    remove_all(out);

    // ScriptWriter expects templates/ under current working directory
    auto cwd = current_path();
    current_path("FirmwarePackager");
    core::ScriptWriter writer;
    writer.write(project, out);
    current_path(cwd);

    EXPECT_TRUE(exists(out/"install.sh"));
    EXPECT_TRUE(exists(out/"recover_boot.sh"));
    EXPECT_TRUE(exists(out/"init/sysv/S95-upgrade-recover"));

    std::ifstream in(out/"install.sh");
    std::stringstream buffer; buffer << in.rdbuf();
    std::string content = buffer.str();
    std::string pkgId = std::to_string(std::hash<std::string>{}(project.name));
    EXPECT_NE(content.find(project.name), std::string::npos);
    EXPECT_NE(content.find(project.version), std::string::npos);
    EXPECT_NE(content.find(pkgId), std::string::npos);
    EXPECT_NE(content.find("/opt/upgrade/packages/" + pkgId), std::string::npos);
    EXPECT_EQ(content.find("@PKG_NAME@"), std::string::npos);
    EXPECT_EQ(content.find("@PKG_VERSION@"), std::string::npos);

    auto perms = status(out/"install.sh").permissions();
    EXPECT_NE(perms & perms::owner_exec, perms::none);
}

int main(int argc, char** argv){
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
