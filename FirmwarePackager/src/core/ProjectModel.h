#pragma once

#include <string>
#include <vector>
#include <filesystem>

namespace core {

/**
 * Represents a single file included in the project.  Directories are not
 * stored explicitly; they are inferred from the parent directories of
 * `dest` when packaging.
 */
struct FileEntry {
    std::filesystem::path path;   // location on disk
    std::filesystem::path dest;   // destination inside package
    std::string id;               // generated identifier
    std::string hash;             // md5 hash
    std::string mode;             // file mode
    std::string owner;            // file owner
    std::string group;            // file group
    bool recursive;               // legacy flag used when expanding directories
    std::vector<std::filesystem::path> excludes; // paths to exclude during expansion

    FileEntry();
    FileEntry(std::filesystem::path p, std::string i, std::string h);
};

/**
 * Simple container managing FileEntry objects.  Only files are tracked; any
 * required directories are implied by FileEntry::dest paths.
 */
struct FileStore {
    explicit FileStore(std::vector<FileEntry>& files) : files(&files) {}

    /// Adds a file entry to the store.
    void add(const FileEntry& entry) { files->push_back(entry); }

    /// Returns the files currently in the store.
    std::vector<FileEntry>& entries() { return *files; }
    /// Returns the files currently in the store (const overload).
    const std::vector<FileEntry>& entries() const { return *files; }

private:
    std::vector<FileEntry>* files;
};

struct Project {
    std::string name;                 // project/package name
    std::string version;              // package version
    std::string pkgId;                // unique package identifier
    std::filesystem::path rootDir;    // project root directory
    std::filesystem::path outputDir;  // output directory
    std::vector<FileEntry> files;     // files included in project
    FileStore store;                  // file store interface

    Project();
    explicit Project(std::string name);
};

} // namespace core

