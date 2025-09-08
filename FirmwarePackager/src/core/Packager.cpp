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
                   IScriptWriter& sc, IIdGenerator& id, ILogger& log)
    : scanner(s), hasher(h), manifest(m), script(sc), idGen(id), logger(log) {}

Project Packager::buildProject(const std::filesystem::path& root, const Scanner::PathList& exclusions) {
    Project project(root.filename().string());
    project.rootDir = root;
    auto paths = scanner.scan(root, exclusions);
    for (const auto& p : paths) {
        std::ifstream in(p, std::ios::binary);
        std::vector<uint8_t> data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        std::string hash = hasher.md5(data);
        std::string id = idGen.generate();
        auto rel = std::filesystem::relative(p, root);
        project.files.emplace_back(rel, id, hash);
    }
    return project;
}

void Packager::package(const Project& project) {
    logger.info("Packaging project");
    auto tempRoot = std::filesystem::temp_directory_path() / ("package-" + idGen.generate());
    std::filesystem::create_directories(tempRoot);

    std::filesystem::path payloadDir = tempRoot / "payload";
    std::filesystem::path scriptsDir = tempRoot / "scripts";
    std::filesystem::path metaDir = tempRoot / "META";
    std::filesystem::create_directories(payloadDir);
    std::filesystem::create_directories(scriptsDir);
    std::filesystem::create_directories(metaDir);

    for (const auto& f : project.files) {
        auto src = project.rootDir / f.path;
        auto dst = payloadDir / f.path;
        std::filesystem::create_directories(dst.parent_path());
        std::filesystem::copy_file(src, dst, std::filesystem::copy_options::overwrite_existing);
    }

    manifest.write(project, tempRoot / "manifest.tsv");
    script.write(project, scriptsDir);

    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::string pkgId = idGen.generate();

    std::ofstream pkgInfo(metaDir / "pkg.info");
    pkgInfo << "PKG_ID=" << pkgId << "\n";
    pkgInfo << "PKG_VERSION=" << project.version << "\n";
    pkgInfo << "PKG_NAME=" << project.name << "\n";
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

    std::filesystem::create_directories(project.outputDir);
    std::filesystem::path archivePath = project.outputDir / (project.name + ".tar.gz");
    createArchive(tempRoot, archivePath);
    std::filesystem::remove_all(tempRoot);
}

void Packager::createArchive(const std::filesystem::path& dir, const std::filesystem::path& archive) {
    struct archive* a = archive_write_new();
    archive_write_add_filter_gzip(a);
    archive_write_set_format_pax_restricted(a);
    archive_write_open_filename(a, archive.string().c_str());

    for (const auto& entry : std::filesystem::recursive_directory_iterator(dir)) {
        if (!entry.is_regular_file()) continue;
        auto rel = std::filesystem::relative(entry.path(), dir);
        struct archive_entry* e = archive_entry_new();
        archive_entry_set_pathname(e, rel.string().c_str());
        archive_entry_set_size(e, std::filesystem::file_size(entry.path()));
        archive_entry_set_filetype(e, AE_IFREG);
        archive_entry_set_perm(e, 0644);
        archive_write_header(a, e);
        std::ifstream in(entry.path(), std::ios::binary);
        char buf[8192];
        while (in) {
            in.read(buf, sizeof(buf));
            auto len = in.gcount();
            if (len > 0) archive_write_data(a, buf, len);
        }
        archive_entry_free(e);
    }

    archive_write_close(a);
    archive_write_free(a);
}

} // namespace core

