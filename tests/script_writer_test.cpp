#include <gtest/gtest.h>
#include "core/ScriptWriter.h"
#include "core/ProjectModel.h"
#include "core/IdGenerator.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

using namespace std::filesystem;

TEST(ScriptWriterTest, GeneratesScriptsWithReplacements){
    core::Project project;
    project.name = "demo";
    project.version = "1.0";
    core::FileEntry fe; fe.path = "file1"; project.files.push_back(fe);

    path out = temp_directory_path()/"sw_out";
    remove_all(out);

    // determine template root before changing working directory
    auto tplRoot = current_path()/"FirmwarePackager/templates";
    auto cwd = current_path();
    path tmpCwd = temp_directory_path()/"sw_cwd";
    remove_all(tmpCwd);
    create_directories(tmpCwd);
    current_path(tmpCwd);

    core::IdGenerator idGen;
    core::ScriptWriter writer;
    std::string pkgId = idGen.generate();
    writer.write(project, out, pkgId, tplRoot);
    current_path(cwd);

    EXPECT_TRUE(exists(out/"scripts/install.sh"));
    EXPECT_TRUE(exists(out/"scripts/recover_boot.sh"));
    EXPECT_TRUE(exists(out/"scripts/init/sysv/S95-upgrade-recover"));

    std::ifstream in(out/"scripts/install.sh", std::ios::binary);
    std::stringstream buffer; buffer << in.rdbuf();
    std::string content = buffer.str();
    EXPECT_EQ(content.find('\r'), std::string::npos);
    EXPECT_NE(content.find(project.name), std::string::npos);
    EXPECT_NE(content.find(project.version), std::string::npos);
    EXPECT_NE(content.find(pkgId), std::string::npos);
    EXPECT_EQ(content.find("@PKG_NAME@"), std::string::npos);
    EXPECT_EQ(content.find("@PKG_VERSION@"), std::string::npos);
    EXPECT_EQ(content.find("@FILES@"), std::string::npos);

    auto perms = status(out/"scripts/install.sh").permissions();
    EXPECT_NE(perms & perms::owner_exec, perms::none);
}

int main(int argc, char** argv){
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
