#include <gtest/gtest.h>
#include "core/Scanner.h"
#include <filesystem>
#include <fstream>
#include <algorithm>

using namespace std::filesystem;

TEST(ScannerTest, HonorsExclusions) {
    path root = temp_directory_path() / "scanner_root";
    remove_all(root);
    create_directories(root / "sub");
    create_directories(root / "skipdir");
    {
        std::ofstream(root / "keep.txt") << "keep";
        std::ofstream(root / "skip.txt") << "skip";
        std::ofstream(root / "sub" / "keep2.txt") << "keep2";
        std::ofstream(root / "sub" / "skipme.txt") << "skipme";
        std::ofstream(root / "skipdir" / "ignored.txt") << "ignore";
    }

    core::Scanner scanner;
    core::Scanner::PathList exclusions = {"skip.txt", "sub/skipme.txt", "skipdir"};
    auto files = scanner.scan(root, exclusions);
    std::sort(files.begin(), files.end());

    core::Scanner::PathList expected = {root / "keep.txt", root / "sub" / "keep2.txt"};
    std::sort(expected.begin(), expected.end());

    EXPECT_EQ(files, expected);

    remove_all(root);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
