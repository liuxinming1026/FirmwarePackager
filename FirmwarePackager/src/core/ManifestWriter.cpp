#include "ManifestWriter.h"

#include <algorithm>
#include <fstream>
#include <iterator>
#include <openssl/md5.h>
#include <vector>

namespace core {

namespace {
std::string toHex(const unsigned char* digest) {
    static const char* hex = "0123456789abcdef";
    std::string out;
    out.reserve(32);
    for (int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
        unsigned char b = digest[i];
        out.push_back(hex[b >> 4]);
        out.push_back(hex[b & 0xF]);
    }
    return out;
}
}

std::string ManifestWriter::md5String(const std::string& data) {
    unsigned char digest[MD5_DIGEST_LENGTH];
    MD5(reinterpret_cast<const unsigned char*>(data.data()), data.size(), digest);
    return toHex(digest);
}

std::string ManifestWriter::md5File(const std::filesystem::path& p) {
    std::ifstream in(p, std::ios::binary);
    if (!in.is_open()) return {};
    MD5_CTX ctx; MD5_Init(&ctx);
    char buf[4096];
    while (in) {
        in.read(buf, sizeof(buf));
        std::streamsize s = in.gcount();
        if (s > 0) MD5_Update(&ctx, buf, static_cast<size_t>(s));
    }
    unsigned char digest[MD5_DIGEST_LENGTH];
    MD5_Final(digest, &ctx);
    return toHex(digest);
}

void ManifestWriter::write(const Project& project, const std::filesystem::path& output) const {
    std::ofstream out(output);
    if (!out.is_open()) {
        return;
    }
    out << "relpath\tdest\tmode\towner\tgroup\tmd5\n";

    for (const auto& f : project.files) {
        auto abs = project.rootDir / f.path;
        if (f.recursive || std::filesystem::is_directory(abs)) {
            struct Rec { std::filesystem::path rel; std::filesystem::path dest; std::string hash; };
            std::vector<Rec> files;
            if (std::filesystem::exists(abs)) {
                for (std::filesystem::recursive_directory_iterator it(abs), end; it != end; ++it) {
                    auto relToDir = std::filesystem::relative(it->path(), abs);
                    std::string relStr = relToDir.generic_string();
                    bool skip = false;
                    for (const auto& ex : f.excludes) {
                        auto exStr = ex.generic_string();
                        if (relStr == exStr || relStr.rfind(exStr + '/', 0) == 0) {
                            skip = true;
                            break;
                        }
                    }
                    if (skip) {
                        if (it->is_directory()) it.disable_recursion_pending();
                        continue;
                    }
                    if (!it->is_regular_file()) continue;
                    auto rel = std::filesystem::relative(it->path(), project.rootDir);
                    auto dest = f.dest / relToDir;
                    std::string h = md5File(it->path());
                    files.push_back({rel, dest, h});
                }
            }
            std::sort(files.begin(), files.end(), [](const Rec& a, const Rec& b){ return a.rel < b.rel; });
            std::string concat;
            concat.reserve(files.size() * 32);
            for (const auto& r : files) concat += r.hash;
            std::string dirHash = md5String(concat);
            out << f.path.generic_string() << '\t'
                << f.dest.generic_string() << '\t'
                << f.mode << '\t'
                << f.owner << '\t'
                << f.group << '\t'
                << dirHash << '\n';
            for (const auto& r : files) {
                out << r.rel.generic_string() << '\t'
                    << r.dest.generic_string() << '\t'
                    << f.mode << '\t'
                    << f.owner << '\t'
                    << f.group << '\t'
                    << r.hash << '\n';
            }
        } else {
            std::string hash = f.hash;
            if (hash.empty()) hash = md5File(abs);
            out << f.path.generic_string() << '\t'
                << f.dest.generic_string() << '\t'
                << f.mode << '\t'
                << f.owner << '\t'
                << f.group << '\t'
                << hash << '\n';
        }
    }
}

} // namespace core

