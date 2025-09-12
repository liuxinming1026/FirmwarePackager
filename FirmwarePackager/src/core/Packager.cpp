#include "Packager.h"

#include <archive.h>
#include <archive_entry.h>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <cstring>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace core {

Packager::Packager(Scanner& s, Hasher& h, IManifestWriter& m,
                   IScriptWriter& sc, IIdGenerator& id, ILogger& log,
                   std::filesystem::path tplRoot)
    : scanner(s), hasher(h), manifest(m), script(sc), idGen(id), logger(log),
      templateRoot(std::move(tplRoot)) {}

void Packager::package(const Project* project) {
    logger.info("Packaging project");
    std::string pkgId = project->pkgId.empty() ? idGen.generate() : project->pkgId;
    auto tempRoot = std::filesystem::temp_directory_path() / ("package-" + pkgId);
    std::filesystem::create_directories(tempRoot);

    std::filesystem::path packageDir = tempRoot / "package";
    std::filesystem::path payloadDir = packageDir / "payload";
    std::filesystem::path metaDir = packageDir / "META";
    std::filesystem::create_directories(payloadDir);
    std::filesystem::create_directories(metaDir);

    for (const auto& f : project->files) {
        auto src = project->rootDir / f.path;
        auto destRoot = payloadDir / f.path;
        if (f.recursive || std::filesystem::is_directory(src)) {
            if (!std::filesystem::exists(src)) continue;
            for (std::filesystem::recursive_directory_iterator it(src), end; it != end; ++it) {
                auto relToDir = std::filesystem::relative(it->path(), src);
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
                auto relToRoot = std::filesystem::relative(it->path(), project->rootDir);
                auto dst = payloadDir / relToRoot;
                std::filesystem::create_directories(dst.parent_path());
                std::filesystem::copy_file(it->path(), dst, std::filesystem::copy_options::overwrite_existing);
            }
        } else {
            if (!std::filesystem::exists(src)) continue;
            std::filesystem::create_directories(destRoot.parent_path());
            std::filesystem::copy_file(src, destRoot, std::filesystem::copy_options::overwrite_existing);
        }
    }

    manifest.write(*project, packageDir / "manifest.tsv");
    script.write(*project, packageDir, pkgId, templateRoot);

    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);

    std::ofstream pkgInfo(metaDir / "pkg.info");
    pkgInfo << "PKG_ID=" << pkgId << "\n";
    pkgInfo << "PKG_VERSION=" << project->version << "\n";
    pkgInfo << "PKG_NAME=" << project->name << "\n";
    pkgInfo << "CREATED_AT=" << std::put_time(std::gmtime(&t), "%Y-%m-%dT%H:%M:%SZ") << "\n";
    pkgInfo << "GENERATOR_VERSION=1.0\n";

    char hostname[256];
#ifdef _WIN32
    DWORD size = sizeof(hostname);
    if (!GetComputerNameA(hostname, &size)) std::strncpy(hostname, "unknown", sizeof(hostname));
#else
    if (gethostname(hostname, sizeof(hostname)) != 0) std::strncpy(hostname, "unknown", sizeof(hostname));
#endif
    hostname[sizeof(hostname)-1] = '\0';

    std::ofstream buildInfo(metaDir / "build.info");
    buildInfo << "built=" << std::put_time(std::gmtime(&t), "%Y-%m-%dT%H:%M:%SZ") << "\n";
    buildInfo << "host=" << hostname << "\n";

    std::filesystem::create_directories(project->outputDir);
    std::filesystem::path archivePath = project->outputDir / (project->name + ".tar.gz");
    createArchive(packageDir, archivePath);
    std::filesystem::remove_all(tempRoot);
}

void Packager::createArchive(const std::filesystem::path& dir, const std::filesystem::path& archive) {
    struct archive* a = archive_write_new();
    archive_write_add_filter_gzip(a);
    archive_write_set_format_pax_restricted(a);
    archive_write_open_filename(a, archive.string().c_str());

    auto base = dir.filename();
    {
        struct archive_entry* e = archive_entry_new();
        archive_entry_set_pathname(e, base.string().c_str());
        archive_entry_set_filetype(e, AE_IFDIR);
        auto mode = static_cast<int>(std::filesystem::status(dir).permissions()) & 0777;
        archive_entry_set_perm(e, mode);
        archive_write_header(a, e);
        archive_entry_free(e);
    }
    for (const auto& entry : std::filesystem::recursive_directory_iterator(dir)) {
        auto relInside = std::filesystem::relative(entry.path(), dir);
        auto rel = base / relInside;
        struct archive_entry* e = archive_entry_new();
        archive_entry_set_pathname(e, rel.string().c_str());
        auto mode = static_cast<int>(std::filesystem::status(entry.path()).permissions()) & 0777;
        archive_entry_set_perm(e, mode);
        if (entry.is_directory()) {
            archive_entry_set_filetype(e, AE_IFDIR);
            archive_write_header(a, e);
        } else if (entry.is_regular_file()) {
            archive_entry_set_size(e, std::filesystem::file_size(entry.path()));
            archive_entry_set_filetype(e, AE_IFREG);
            archive_write_header(a, e);
            std::ifstream in(entry.path(), std::ios::binary);
            char buf[8192];
            while (in) {
                in.read(buf, sizeof(buf));
                auto len = in.gcount();
                if (len > 0) archive_write_data(a, buf, len);
            }
        }
        archive_entry_free(e);
    }

    archive_write_close(a);
    archive_write_free(a);
}

} // namespace core
