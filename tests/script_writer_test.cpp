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
    core::IdGenerator idGen;
    core::ScriptWriter writer(idGen);
    writer.write(project, out);
    current_path(cwd);

    EXPECT_TRUE(exists(out/"scripts/install.sh"));
    EXPECT_TRUE(exists(out/"scripts/recover_boot.sh"));
    EXPECT_TRUE(exists(out/"scripts/init/sysv/S95-upgrade-recover"));

    std::ifstream in(out/"scripts/install.sh");
    std::stringstream buffer; buffer << in.rdbuf();
    std::string content = buffer.str();
    core::IdGenerator tmpGen;
    std::string pkgId = tmpGen.generate();
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
