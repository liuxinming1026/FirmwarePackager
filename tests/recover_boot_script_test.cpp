#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <cstdlib>

using namespace std::filesystem;

TEST(RecoverBootScript, RunsInstallWithResumeAndKeepsState) {
    path stateDir = "/opt/upgrade/state";
    path pkgDir = "/opt/upgrade/packages";
    remove_all(stateDir);
    remove_all(pkgDir);
    create_directories(stateDir);
    create_directories(pkgDir);

    std::string id = "testpkg";
    path stateFile = stateDir / (id + ".state");
    { std::ofstream(stateFile) << "STATUS=RUNNING\n"; }

    path temp = temp_directory_path() / "recover_pkg";
    remove_all(temp);
    create_directories(temp / "package/scripts");

    path install = temp / "package/scripts/install.sh";
    {
        std::ofstream out(install);
        out << "#!/bin/sh\n";
        out << "echo \"$1\" > /tmp/install_arg\n";
    }
    permissions(install, perms::owner_read | perms::owner_write | perms::owner_exec);

    path archive = pkgDir / (id + ".tar.gz");
    std::string cmd = "tar -czf " + archive.string() + " -C " + temp.string() + " package";
    ASSERT_EQ(std::system(cmd.c_str()), 0);

    path script = temp_directory_path() / "recover_boot.sh";
    {
        std::ifstream in("FirmwarePackager/templates/scripts/recover_boot.sh.in");
        std::ofstream out(script);
        out << in.rdbuf();
    }
    permissions(script, perms::owner_read | perms::owner_write | perms::owner_exec);

    ASSERT_EQ(std::system(script.string().c_str()), 0);

    std::ifstream arg("/tmp/install_arg");
    std::string argVal;
    std::getline(arg, argVal);
    EXPECT_EQ(argVal, "--resume");

    EXPECT_TRUE(exists(stateFile));

    remove("/tmp/install_arg");
    remove_all(temp);
    remove_all(pkgDir);
    remove_all(stateDir);
}

int main(int argc, char** argv){
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

