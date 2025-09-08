#include <gtest/gtest.h>
#include "core/ManifestWriter.h"
#include "core/ProjectModel.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include <openssl/md5.h>

using namespace std::filesystem;

namespace {
std::string toHex(const unsigned char* digest) {
    static const char* hex = "0123456789abcdef";
    std::string out; out.reserve(32);
    for (int i=0;i<MD5_DIGEST_LENGTH;++i){ unsigned char b=digest[i]; out.push_back(hex[b>>4]); out.push_back(hex[b&0xF]); }
    return out;
}
std::string md5File(const path& p){
    std::ifstream in(p, std::ios::binary); if(!in) return {};
    MD5_CTX ctx; MD5_Init(&ctx); char buf[4096];
    while(in){ in.read(buf,sizeof(buf)); std::streamsize s=in.gcount(); if(s>0) MD5_Update(&ctx,buf,static_cast<size_t>(s)); }
    unsigned char d[MD5_DIGEST_LENGTH]; MD5_Final(d,&ctx); return toHex(d);
}
std::string md5String(const std::string& s){ unsigned char d[MD5_DIGEST_LENGTH]; MD5((const unsigned char*)s.data(), s.size(), d); return toHex(d);} 
}

TEST(ManifestWriterTest, ExpandsDirectoriesAndWritesColumns){
    auto tmp = temp_directory_path()/"fp_test";
    remove_all(tmp);
    create_directories(tmp/"dir"/"sub");
    { std::ofstream(tmp/"dir"/"a.txt")<<"hello"; }
    { std::ofstream(tmp/"dir"/"sub"/"b.txt")<<"world"; }

    core::Project project; project.rootDir = tmp;
    core::FileEntry fe; fe.path="dir"; fe.dest="destdir"; fe.mode="0644"; fe.owner="root"; fe.group="root"; fe.recursive=true; project.files.push_back(fe);

    core::ManifestWriter writer;
    auto manifestPath = tmp/"manifest.tsv";
    writer.write(project, manifestPath);

    std::ifstream in(manifestPath);
    ASSERT_TRUE(in.is_open());
    std::vector<std::vector<std::string>> lines; std::string line;
    while(std::getline(in,line)){
        std::vector<std::string> cols; std::stringstream ss(line); std::string c; while(std::getline(ss,c,'\t')) cols.push_back(c); lines.push_back(cols); }

    ASSERT_EQ(lines.size(), 4u);
    EXPECT_EQ(lines[0], (std::vector<std::string>{"relpath","dest","mode","owner","group","md5"}));

    std::string hashA = md5File(tmp/"dir"/"a.txt");
    std::string hashB = md5File(tmp/"dir"/"sub"/"b.txt");
    std::string dirHash = md5String(hashA + hashB);

    auto dirLine = lines[1];
    EXPECT_EQ(dirLine[0], "dir");
    EXPECT_EQ(dirLine[1], "destdir");
    EXPECT_EQ(dirLine[2], "0644");
    EXPECT_EQ(dirLine[3], "root");
    EXPECT_EQ(dirLine[4], "root");
    EXPECT_EQ(dirLine[5], dirHash);

    EXPECT_EQ(lines[2][0], "dir/a.txt");
    EXPECT_EQ(lines[2][1], "destdir/a.txt");
    EXPECT_EQ(lines[2][5], hashA);

    EXPECT_EQ(lines[3][0], "dir/sub/b.txt");
    EXPECT_EQ(lines[3][1], "destdir/sub/b.txt");
    EXPECT_EQ(lines[3][5], hashB);

    remove_all(tmp);
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
