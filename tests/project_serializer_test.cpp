#include <gtest/gtest.h>
#include "core/ProjectSerializer.h"
#include <filesystem>

TEST(ProjectSerializerTest, PkgIdPersists) {
    core::Project project("demo");
    project.pkgId = "pkg123";
    project.rootDir = std::filesystem::temp_directory_path();
    project.outputDir = std::filesystem::temp_directory_path();
    core::ProjectSerializer serializer;
    std::filesystem::path file = std::filesystem::temp_directory_path() / "project.json";
    serializer.save(project, file.string());
    core::Project loaded = serializer.load(file.string());
    EXPECT_EQ(loaded.pkgId, project.pkgId);
    std::filesystem::remove(file);
}

TEST(ProjectSerializerTest, DefaultModesApplied) {
    core::Project project("demo");
    project.rootDir = std::filesystem::temp_directory_path();
    project.outputDir = std::filesystem::temp_directory_path();

    core::FileEntry fileEntry; fileEntry.path = "file"; fileEntry.dest = "file";
    project.store.entries().push_back(fileEntry);

    core::ProjectSerializer serializer;
    std::filesystem::path file = std::filesystem::temp_directory_path() / "project_modes.json";
    serializer.save(project, file.string());
    core::Project loaded = serializer.load(file.string());
    ASSERT_EQ(loaded.store.entries().size(), 1u);
    EXPECT_EQ(loaded.store.entries()[0].mode, "0644");
    std::filesystem::remove(file);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
