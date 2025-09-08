#include "Hasher.h"

#include <array>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <vector>
#include <openssl/md5.h>

#ifdef _WIN32
#include <windows.h>
#include <bcrypt.h>
#pragma comment(lib, "bcrypt")
#endif

namespace core {

namespace {
std::string toHex(const std::array<uint8_t,16>& digest) {
    static const char* hex = "0123456789abcdef";
    std::string out;
    out.reserve(32);
    for (auto b : digest) {
        out.push_back(hex[b >> 4]);
        out.push_back(hex[b & 0xF]);
    }
    return out;
}
} // anonymous namespace

std::string Hasher::md5(const std::vector<uint8_t>& data) const {
#ifdef _WIN32
    BCRYPT_ALG_HANDLE hAlg = nullptr;
    BCRYPT_HASH_HANDLE hHash = nullptr;
    DWORD hashLen = 0, cbData = 0;
    std::array<uint8_t,16> hash{};

    if (BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_MD5_ALGORITHM, nullptr, 0) != 0)
        return {};
    if (BCryptCreateHash(hAlg, &hHash, nullptr, 0, nullptr, 0, 0) != 0) {
        BCryptCloseAlgorithmProvider(hAlg,0);
        return {};
    }
    if (BCryptHashData(hHash, const_cast<PUCHAR>(data.data()), (ULONG)data.size(), 0) != 0) {
        BCryptDestroyHash(hHash);
        BCryptCloseAlgorithmProvider(hAlg,0);
        return {};
    }
    if (BCryptGetProperty(hHash, BCRYPT_HASH_LENGTH, (PUCHAR)&hashLen, sizeof(hashLen), &cbData, 0) != 0 || hashLen != hash.size()) {
        BCryptDestroyHash(hHash);
        BCryptCloseAlgorithmProvider(hAlg,0);
        return {};
    }
    if (BCryptFinishHash(hHash, hash.data(), hashLen, 0) != 0) {
        BCryptDestroyHash(hHash);
        BCryptCloseAlgorithmProvider(hAlg,0);
        return {};
    }
    BCryptDestroyHash(hHash);
    BCryptCloseAlgorithmProvider(hAlg,0);
    return toHex(hash);
#else
    std::array<uint8_t,16> hash{};
    MD5(data.data(), data.size(), hash.data());
    return toHex(hash);
#endif
}

std::string Hasher::md5File(const std::filesystem::path& path) const {
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) return {};
    std::vector<uint8_t> data(std::istreambuf_iterator<char>(in), {});
    return md5(data);
}

std::string Hasher::md5Directory(const std::filesystem::path& path) const {
    if (!std::filesystem::exists(path)) return {};
    struct Item { std::filesystem::path rel; std::string hash; };
    std::vector<Item> items;
    for (auto& entry : std::filesystem::recursive_directory_iterator(path)) {
        if (!entry.is_regular_file()) continue;
        auto rel = std::filesystem::relative(entry.path(), path);
        items.push_back({rel, md5File(entry.path())});
    }
    std::sort(items.begin(), items.end(), [](const Item& a, const Item& b){ return a.rel < b.rel; });
    std::string concat;
    concat.reserve(items.size() * 32);
    for (const auto& it : items) concat += it.hash;
    std::vector<uint8_t> bytes(concat.begin(), concat.end());
    return md5(bytes);
}

} // namespace core

