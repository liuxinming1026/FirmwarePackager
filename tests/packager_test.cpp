#include <gtest/gtest.h>
#include "core/Packager.h"
#include "core/Scanner.h"
#include "core/Hasher.h"
#include "core/ManifestWriter.h"
#include "core/ScriptWriter.h"
#include "core/IdGenerator.h"
#include "core/Logger.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <string>

using namespace std::filesystem;

class SilentLogger : public core::ILogger {
public:
    void info(const std::string&) override {}
    void error(const std::string&) override {}
};

TEST(PackagerTest, GeneratesArchiveWithExpectedContents) {
    path root = temp_directory_path() / "pkg_root";
    remove_all(root);
    create_directories(root / "dir" / "sub");
    { std::ofstream(root / "dir" / "a.txt") << "data"; }
    { std::ofstream(root / "dir" / "sub" / "b.txt") << "skip"; }
    { std::ofstream(root / "dir" / "exclude.txt") << "skip"; }

    path out = temp_directory_path() / "pkg_out";
    remove_all(out);

    core::Scanner scanner;
    core::Hasher hasher;
    core::ManifestWriter mw;
    core::IdGenerator idgen;
    core::ScriptWriter sw;
    SilentLogger logger;
    core::Packager pack(scanner, hasher, mw, sw, idgen, logger);

    core::Project project("pkg_root");
    project.rootDir = root;
    project.outputDir = out;
    project.version = "1.0";
    core::FileEntry fe; fe.path="dir"; fe.dest="destdir"; fe.recursive=true; fe.excludes={"sub","exclude.txt"}; project.files.push_back(fe);

    auto cwd = current_path();
    current_path("FirmwarePackager/FirmwarePackager");
    pack.package(project);
    current_path(cwd);

    path archive = out / (project.name + ".tar.gz");
    ASSERT_TRUE(exists(archive));

    path extractDir = temp_directory_path() / "pkg_extract";
    remove_all(extractDir);
    create_directories(extractDir);
    std::string cmd = "tar -xzf " + archive.string() + " -C " + extractDir.string();
    ASSERT_EQ(std::system(cmd.c_str()), 0);

    path packageDir = extractDir / "package";
    ASSERT_TRUE(exists(packageDir));
    EXPECT_FALSE(exists(extractDir / "payload"));
    EXPECT_FALSE(exists(extractDir / "manifest.tsv"));

    path payloadFile = packageDir / "payload" / "destdir" / "a.txt";
    ASSERT_TRUE(exists(payloadFile));
    EXPECT_FALSE(exists(packageDir / "payload" / "destdir" / "sub" / "b.txt"));
    EXPECT_FALSE(exists(packageDir / "payload" / "destdir" / "exclude.txt"));
    std::ifstream in(payloadFile); std::string data; std::getline(in, data); EXPECT_EQ(data, "data");

    path manifestPath = packageDir / "manifest.tsv";
    ASSERT_TRUE(exists(manifestPath));
    std::ifstream mf(manifestPath);
    std::string line; std::getline(mf, line); // header
    ASSERT_TRUE(std::getline(mf, line));
    std::vector<std::string> cols; std::stringstream ss(line); std::string c;
    while (std::getline(ss, c, '\t')) cols.push_back(c);
    ASSERT_EQ(cols.size(), 6u);
    EXPECT_EQ(cols[0], "dir");
    EXPECT_EQ(cols[1], "destdir");
    ASSERT_TRUE(std::getline(mf, line));
    cols.clear(); ss.clear(); ss.str(line);
    while (std::getline(ss, c, '\t')) cols.push_back(c);
    EXPECT_EQ(cols[0], "dir/a.txt");
    EXPECT_EQ(cols[1], "destdir/a.txt");
    EXPECT_EQ(cols[5], hasher.md5File(root/"dir"/"a.txt"));

    path scriptPath = packageDir / "scripts" / "install.sh";
    ASSERT_TRUE(exists(scriptPath));
    std::ifstream sf(scriptPath); std::stringstream buf; buf << sf.rdbuf(); std::string script = buf.str();
    EXPECT_NE(script.find(project.name), std::string::npos);
    EXPECT_NE(script.find(project.version), std::string::npos);
    EXPECT_EQ(script.find("@PKG_NAME@"), std::string::npos);

    remove_all(root);
    remove_all(out);
    remove_all(extractDir);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
