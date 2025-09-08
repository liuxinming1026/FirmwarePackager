#include "Scanner.h"

#include <system_error>

namespace core {

Scanner::PathList Scanner::scan(const std::filesystem::path& root, const PathList& exclusions, bool recursive) const {
    PathList result;
    if (!std::filesystem::exists(root)) {
        return result;
    }

    PathList absEx;
    absEx.reserve(exclusions.size());
    for (const auto& ex : exclusions) {
        absEx.push_back(ex.is_absolute() ? ex : root / ex);
    }

    std::error_code ec;

    if (std::filesystem::is_regular_file(root)) {
        if (!isExcluded(root, absEx)) {
            result.push_back(root);
        }
        return result;
    }

    if (recursive) {
        std::filesystem::recursive_directory_iterator it(root, ec), end;
        for (; it != end; it.increment(ec)) {
            if (ec) break;
            const auto& p = it->path();
            if (isExcluded(p, absEx)) {
                if (it->is_directory()) {
                    it.disable_recursion_pending();
                }
                continue;
            }
            if (it->is_regular_file()) {
                result.push_back(p);
            }
        }
    } else {
        std::filesystem::directory_iterator it(root, ec), end;
        for (; it != end; it.increment(ec)) {
            if (ec) break;
            const auto& p = it->path();
            if (isExcluded(p, absEx)) {
                continue;
            }
            if (std::filesystem::is_regular_file(p)) {
                result.push_back(p);
            }
        }
    }
    return result;
}

bool Scanner::isExcluded(const std::filesystem::path& path, const PathList& exclusions) const {
    for (const auto& ex : exclusions) {
        auto exNorm = ex.lexically_normal();
        auto pathNorm = path.lexically_normal();
        if (pathNorm == exNorm) {
            return true;
        }
        if (pathNorm.native().rfind(exNorm.native(), 0) == 0) {
            return true;
        }
    }
    return false;
}

} // namespace core

