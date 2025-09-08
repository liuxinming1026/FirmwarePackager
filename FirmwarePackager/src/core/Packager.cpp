#include "Packager.h"

#include <archive.h>
#include <archive_entry.h>
#include <chrono>
#include <fstream>
#include <iomanip>

namespace core {

Packager::Packager(Scanner& s, Hasher& h, IManifestWriter& m,
                   IScriptWriter& sc, IIdGenerator& id, ILogger& log)
    : scanner(s), hasher(h), manifest(m), script(sc), idGen(id), logger(log) {}

Project Packager::buildProject(const std::filesystem::path& root, const Scanner::PathList& exclusions) {
    Project project(root.filename().string());
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

void Packager::package(const std::filesystem::path& root, const std::filesystem::path& outdir, const Scanner::PathList& exclusions) {
    logger.info("Packaging project");
    Project project = buildProject(root, exclusions);
    std::filesystem::create_directories(outdir);
    for (const auto& f : project.files) {
        auto src = root / f.path;
        auto dst = outdir / f.path;
        std::filesystem::create_directories(dst.parent_path());
        std::filesystem::copy_file(src, dst, std::filesystem::copy_options::overwrite_existing);
    }
    manifest.write(project, outdir / "manifest.tsv");
    script.write(project, outdir / "scripts");

    std::ofstream pkgInfo(outdir / "pkg.info");
    pkgInfo << "name=" << project.name << "\n";

    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::ofstream buildInfo(outdir / "build.info");
    buildInfo << "built=" << std::put_time(std::gmtime(&t), "%Y-%m-%dT%H:%M:%SZ") << "\n";

    std::filesystem::path archivePath = outdir.parent_path() / (project.name + ".tar.gz");
    createArchive(outdir, archivePath);
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

