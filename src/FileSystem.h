/**
 * @file FileSystem.h
 * @brief Portable filesystem abstraction for NexusDB.
 *
 * Since the system has GCC 6.3 (pre-C++17 <filesystem>), this header
 * provides directory listing, file existence checks, path manipulation,
 * and file deletion using platform-specific APIs (Windows) or POSIX.
 */

#ifndef NEXUSDB_FILESYSTEM_H
#define NEXUSDB_FILESYSTEM_H

#include <string>
#include <vector>
#include <fstream>

#ifdef _WIN32
#include <windows.h>
#include <direct.h>
#define PATH_SEP '\\'
#else
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#define PATH_SEP '/'
#endif

namespace nexusdb {
namespace fs {

/**
 * @brief Check if a file or directory exists.
 */
inline bool exists(const std::string& path) {
#ifdef _WIN32
    DWORD attr = GetFileAttributesA(path.c_str());
    return (attr != INVALID_FILE_ATTRIBUTES);
#else
    struct stat st;
    return (stat(path.c_str(), &st) == 0);
#endif
}

/**
 * @brief Check if the path is a directory.
 */
inline bool isDirectory(const std::string& path) {
#ifdef _WIN32
    DWORD attr = GetFileAttributesA(path.c_str());
    return (attr != INVALID_FILE_ATTRIBUTES) && (attr & FILE_ATTRIBUTE_DIRECTORY);
#else
    struct stat st;
    if (stat(path.c_str(), &st) != 0) return false;
    return S_ISDIR(st.st_mode);
#endif
}

/**
 * @brief Create a directory (and parents if needed).
 */
inline bool createDirectory(const std::string& path) {
#ifdef _WIN32
    return CreateDirectoryA(path.c_str(), NULL) || GetLastError() == ERROR_ALREADY_EXISTS;
#else
    return mkdir(path.c_str(), 0755) == 0 || errno == EEXIST;
#endif
}

/**
 * @brief List all files in a directory matching a given extension.
 * @param dir  Directory path.
 * @param ext  Extension filter (e.g. ".csv"). Empty means all files.
 * @return Vector of full file paths.
 */
inline std::vector<std::string> listFiles(const std::string& dir,
                                          const std::string& ext = "") {
    std::vector<std::string> result;
#ifdef _WIN32
    std::string pattern = dir + "\\*";
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(pattern.c_str(), &fd);
    if (hFind == INVALID_HANDLE_VALUE) return result;
    do {
        std::string name = fd.cFileName;
        if (name == "." || name == "..") continue;
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        if (!ext.empty()) {
            if (name.size() < ext.size()) continue;
            std::string suffix = name.substr(name.size() - ext.size());
            // Case-insensitive compare
            bool match = true;
            for (size_t i = 0; i < ext.size(); ++i) {
                if (tolower(suffix[i]) != tolower(ext[i])) { match = false; break; }
            }
            if (!match) continue;
        }
        result.push_back(dir + "\\" + name);
    } while (FindNextFileA(hFind, &fd));
    FindClose(hFind);
#else
    DIR* d = opendir(dir.c_str());
    if (!d) return result;
    struct dirent* entry;
    while ((entry = readdir(d)) != nullptr) {
        std::string name = entry->d_name;
        if (name == "." || name == "..") continue;
        std::string fullPath = dir + "/" + name;
        // Check if it's a regular file
        struct stat st;
        if (stat(fullPath.c_str(), &st) != 0 || !S_ISREG(st.st_mode)) continue;
        if (!ext.empty()) {
            if (name.size() < ext.size()) continue;
            if (name.substr(name.size() - ext.size()) != ext) continue;
        }
        result.push_back(fullPath);
    }
    closedir(d);
#endif
    return result;
}

/**
 * @brief Delete a file from disk.
 */
inline bool removeFile(const std::string& path) {
#ifdef _WIN32
    return DeleteFileA(path.c_str()) != 0;
#else
    return unlink(path.c_str()) == 0;
#endif
}

/**
 * @brief Extract filename (without path) from a full path.
 */
inline std::string filename(const std::string& path) {
    size_t pos = path.find_last_of("/\\");
    if (pos == std::string::npos) return path;
    return path.substr(pos + 1);
}

/**
 * @brief Extract the stem (filename without extension).
 */
inline std::string stem(const std::string& path) {
    std::string name = filename(path);
    size_t dot = name.find_last_of('.');
    if (dot == std::string::npos) return name;
    return name.substr(0, dot);
}

/**
 * @brief Get the current working directory.
 */
inline std::string currentPath() {
#ifdef _WIN32
    char buf[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, buf);
    return std::string(buf);
#else
    char buf[4096];
    if (getcwd(buf, sizeof(buf))) return std::string(buf);
    return ".";
#endif
}

/**
 * @brief Join two path components.
 */
inline std::string joinPath(const std::string& base, const std::string& child) {
    if (base.empty()) return child;
    char last = base.back();
    if (last == '/' || last == '\\') return base + child;
    return base + PATH_SEP + child;
}

} // namespace fs
} // namespace nexusdb

#endif // NEXUSDB_FILESYSTEM_H
