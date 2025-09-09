#include <gtest/gtest.h>
#include "core/Hasher.h"
#include <filesystem>
#include <fstream>
#include <vector>
#include <algorithm>
#include <openssl/md5.h>

using namespace std::filesystem;

namespace {
std::string toHex(const unsigned char* digest) {
    static const char* hex = "0123456789abcdef";
    std::string out; out.reserve(32);
    for (int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
        unsigned char b = digest[i];
        out.push_back(hex[b >> 4]);
        out.push_back(hex[b & 0xF]);
    }
    return out;
}

std::string md5FileRef(const path& p) {
    std::ifstream in(p, std::ios::binary); if (!in) return {};
    MD5_CTX ctx; MD5_Init(&ctx); char buf[4096];
    while (in) { in.read(buf, sizeof(buf)); std::streamsize s = in.gcount(); if (s > 0) MD5_Update(&ctx, buf, static_cast<size_t>(s)); }
    unsigned char d[MD5_DIGEST_LENGTH]; MD5_Final(d, &ctx); return toHex(d);
}

std::string md5StringRef(const std::string& s) {
    unsigned char d[MD5_DIGEST_LENGTH];
    MD5(reinterpret_cast<const unsigned char*>(s.data()), s.size(), d);
    return toHex(d);
}
}

TEST(HasherTest, Md5FileMatchesReference) {
    path tmp = temp_directory_path() / "hasher_file.txt";
    { std::ofstream(tmp) << "hello"; }
    core::Hasher h;
    EXPECT_EQ(h.md5File(tmp), md5FileRef(tmp));
    remove(tmp);
}

TEST(HasherTest, Md5FileHandlesLargeFiles) {
    path tmp = temp_directory_path() / "hasher_large.bin";
    {
        std::ofstream out(tmp, std::ios::binary);
        for (size_t i = 0; i < (1 << 20); ++i)
            out.put(static_cast<char>(i & 0xFF));
    }
    core::Hasher h;
    EXPECT_EQ(h.md5File(tmp), md5FileRef(tmp));
    remove(tmp);
}

TEST(HasherTest, Md5DirectoryAggregatesChildren) {
    path dir = temp_directory_path() / "hasher_dir";
    remove_all(dir);
    create_directories(dir / "sub");
    { std::ofstream(dir / "a.txt") << "alpha"; }
    { std::ofstream(dir / "sub" / "b.txt") << "beta"; }

    core::Hasher h;
    std::string hashA = md5FileRef(dir / "a.txt");
    std::string hashB = md5FileRef(dir / "sub" / "b.txt");
    std::vector<std::pair<path, std::string>> entries = {{"a.txt", hashA}, {"sub/b.txt", hashB}};
    std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b){ return a.first < b.first; });
    std::string concat;
    for (const auto& e : entries) concat += e.second;
    std::string expected = md5StringRef(concat);
    EXPECT_EQ(h.md5Directory(dir), expected);

    remove_all(dir);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

