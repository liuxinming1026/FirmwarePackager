#include "Hasher.h"

#include <array>
#include <sstream>
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
}

} // namespace core

