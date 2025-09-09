#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <openssl/md5.h>

using namespace std::filesystem;

namespace {
std::string toHex(const unsigned char* d){
    static const char* hex="0123456789abcdef";
    std::string out; out.reserve(32);
    for(int i=0;i<MD5_DIGEST_LENGTH;++i){ unsigned char b=d[i]; out.push_back(hex[b>>4]); out.push_back(hex[b&0xF]); }
    return out;
}

std::string md5File(const path& p){
    std::ifstream in(p, std::ios::binary); if(!in) return {};
    MD5_CTX ctx; MD5_Init(&ctx); char buf[4096];
    while(in){ in.read(buf,sizeof(buf)); std::streamsize s=in.gcount(); if(s>0) MD5_Update(&ctx,buf,static_cast<size_t>(s)); }
    unsigned char d[MD5_DIGEST_LENGTH]; MD5_Final(d,&ctx); return toHex(d);
}

std::string replaceAll(std::string s, const std::string& from, const std::string& to){
    size_t pos=0; while((pos=s.find(from,pos))!=std::string::npos){ s.replace(pos,from.size(),to); pos+=to.size(); } return s;
}

std::string loadScript(){
    std::ifstream in("FirmwarePackager/templates/scripts/install.sh.in");
    std::stringstream buf; buf<<in.rdbuf();
    std::string script = buf.str();
    script = replaceAll(script, "@PKG_ID@", "TESTPKG");
    script = replaceAll(script, "@PKG_NAME@", "test");
    script = replaceAll(script, "@PKG_VERSION@", "1.0");
    return script;
}

void writeScript(const path& dir){
    create_directories(dir/"scripts");
    std::ofstream out(dir/"scripts"/"install.sh");
    out<<loadScript();
    out.close();
    permissions(dir/"scripts"/"install.sh", perms::owner_read|perms::owner_exec|perms::owner_write);
}

void cleanupState(){
    remove_all("/opt/upgrade/backup/TESTPKG");
    remove("/opt/upgrade/state/TESTPKG.state");
    remove("/opt/upgrade/state/TESTPKG.journal");
    remove("/opt/upgrade/packages/TESTPKG.tar.gz");
}
}

TEST(InstallScript, DetectsMd5Mismatch){
    cleanupState();
    path pkg = temp_directory_path()/"pkg_md5"; remove_all(pkg);
    writeScript(pkg);
    { std::ofstream(pkg/"file.txt")<<"data"; }
    std::ofstream manifest(pkg/"manifest.tsv");
    manifest<<"relpath\tdest\tmode\towner\tgroup\tmd5\n";
    manifest<<"file.txt\t"<<(pkg/"out.txt").string()<<"\t0644\troot\troot\tdeadbeefdeadbeefdeadbeefdeadbeef\n";
    manifest.close();

    path archive = temp_directory_path()/"pkg_md5.tar.gz";
    std::string cmd = "tar -czf " + archive.string() + " -C " + pkg.string() + " .";
    ASSERT_EQ(std::system(cmd.c_str()),0);
    int rc = std::system(("sh "+(pkg/"scripts"/"install.sh").string()+" --store "+archive.string()).c_str());
    EXPECT_NE(rc,0);
    EXPECT_FALSE(exists(pkg/"out.txt"));
    std::ifstream st("/opt/upgrade/state/TESTPKG.state");
    std::stringstream s; s<<st.rdbuf();
    EXPECT_NE(s.str().find("STATUS=FAIL"), std::string::npos);
    cleanupState();
    remove(archive);
    remove_all(pkg);
}

TEST(InstallScript, RollsBackOnFailure){
    cleanupState();
    path pkg = temp_directory_path()/"pkg_rb"; remove_all(pkg);
    writeScript(pkg);
    { std::ofstream(pkg/"a.txt")<<"newA"; }
    { std::ofstream(pkg/"b.txt")<<"newB"; }
    path destA = pkg/"destA.txt"; { std::ofstream(destA)<<"oldA"; }
    path destB = pkg/"destB.txt"; // does not exist
    std::string hashA = md5File(pkg/"a.txt");
    std::ofstream manifest(pkg/"manifest.tsv");
    manifest<<"relpath\tdest\tmode\towner\tgroup\tmd5\n";
    manifest<<"a.txt\t"<<destA.string()<<"\t0644\troot\troot\t"<<hashA<<"\n";
    manifest<<"b.txt\t"<<destB.string()<<"\t0644\troot\troot\t00000000000000000000000000000000\n";
    manifest.close();

    path archive = temp_directory_path()/"pkg_rb.tar.gz";
    std::string cmd = "tar -czf " + archive.string() + " -C " + pkg.string() + " .";
    ASSERT_EQ(std::system(cmd.c_str()),0);
    int rc = std::system(("sh "+(pkg/"scripts"/"install.sh").string()+" --store "+archive.string()).c_str());
    EXPECT_NE(rc,0);
    std::ifstream in(destA); std::string data; std::getline(in,data); EXPECT_EQ(data,"oldA");
    EXPECT_FALSE(exists(destB));
    cleanupState();
    remove(archive);
    remove_all(pkg);
}

int main(int argc, char** argv){
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
