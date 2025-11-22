#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <memory>
#include <functional>

namespace VortexEngine {

namespace fs = std::filesystem;

// File operation result
enum class FileResult {
    Success,
    Error,
    FileNotFound,
    PermissionDenied,
    InvalidPath,
    AlreadyExists
};

// File information structure
struct FileInfo {
    std::string path;
    std::string filename;
    std::string extension;
    size_t size;
    bool isDirectory;
    bool isFile;
    bool isReadable;
    bool isWritable;
    bool isExecutable;
    std::string lastModified;
    std::string lastAccessed;
    std::string created;
};

// Directory information structure
struct DirectoryInfo {
    std::string path;
    std::string name;
    size_t fileCount;
    size_t directoryCount;
    std::vector<FileInfo> files;
    std::vector<DirectoryInfo> subdirectories;
};

// File utility class
class FileUtils {
public:
    // File existence and properties
    static bool exists(const std::string& path);
    static bool isFile(const std::string& path);
    static bool isDirectory(const std::string& path);
    static bool isReadable(const std::string& path);
    static bool isWritable(const std::string& path);
    static bool isExecutable(const std::string& path);

    // File size and information
    static size_t getFileSize(const std::string& path);
    static FileInfo getFileInfo(const std::string& path);
    static std::string getFilename(const std::string& path);
    static std::string getExtension(const std::string& path);
    static std::string getDirectory(const std::string& path);
    static std::string getAbsolutePath(const std::string& path);
    static std::string getRelativePath(const std::string& path, const std::string& base);

    // File operations
    static FileResult createFile(const std::string& path);
    static FileResult createDirectory(const std::string& path);
    static FileResult createDirectories(const std::string& path);
    static FileResult deleteFile(const std::string& path);
    static FileResult deleteDirectory(const std::string& path, bool recursive = false);
    static FileResult copyFile(const std::string& source, const std::string& destination);
    static FileResult moveFile(const std::string& source, const std::string& destination);
    static FileResult copyDirectory(const std::string& source, const std::string& destination);
    static FileResult moveDirectory(const std::string& source, const std::string& destination);

    // File reading and writing
    static FileResult readFile(const std::string& path, std::vector<uint8_t>& data);
    static FileResult readFile(const std::string& path, std::string& content);
    static FileResult writeFile(const std::string& path, const std::vector<uint8_t>& data);
    static FileResult writeFile(const std::string& path, const std::string& content);
    static FileResult appendFile(const std::string& path, const std::string& content);

    // Directory operations
    static std::vector<FileInfo> listFiles(const std::string& path, bool recursive = false);
    static std::vector<DirectoryInfo> listDirectories(const std::string& path, bool recursive = false);
    static std::vector<std::string> listFilesByExtension(const std::string& path, const std::string& extension, bool recursive = false);
    static std::vector<std::string> findFiles(const std::string& path, const std::string& pattern, bool recursive = false);
    static std::vector<std::string> findDirectories(const std::string& path, const std::string& pattern, bool recursive = false);

    // File system operations
    static FileResult changeWorkingDirectory(const std::string& path);
    static std::string getWorkingDirectory();
    static FileResult setCurrentDirectory(const std::string& path);
    static std::string getCurrentDirectory();
    static FileResult setHomeDirectory(const std::string& path);
    static std::string getHomeDirectory();
    static FileResult setTempDirectory(const std::string& path);
    static std::string getTempDirectory();

    // Path operations
    static std::string normalizePath(const std::string& path);
    static std::string joinPaths(const std::vector<std::string>& paths);
    static std::string joinPaths(const std::string& path1, const std::string& path2);
    static std::string joinPaths(const std::string& path1, const std::string& path2, const std::string& path3);
    static std::string joinPaths(const std::string& path1, const std::string& path2, const std::string& path3, const std::string& path4);
    static std::string makePathAbsolute(const std::string& path);
    static std::string makePathRelative(const std::string& path, const std::string& base);
    static std::string removeTrailingSlash(const std::string& path);
    static std::string addTrailingSlash(const std::string& path);
    static std::string replaceExtension(const std::string& path, const std::string& newExtension);
    static std::string changeExtension(const std::string& path, const std::string& newExtension);
    static std::string removeExtension(const std::string& path);

    // File system monitoring
    using FileChangeCallback = std::function<void(const std::string& path, bool created, bool modified, bool deleted)>;
    static void watchFile(const std::string& path, FileChangeCallback callback);
    static void watchDirectory(const std::string& path, FileChangeCallback callback);
    static void stopWatching(const std::string& path);
    static void stopAllWatching();

    // File system statistics
    static size_t getDirectorySize(const std::string& path, bool recursive = false);
    static size_t getFreeSpace(const std::string& path);
    static size_t getTotalSpace(const std::string& path);
    static size_t getUsedSpace(const std::string& path);
    static uint64_t getLastModifiedTime(const std::string& path);
    static uint64_t getLastAccessedTime(const std::string& path);
    static uint64_t getCreationTime(const std::string& path);

    // File system utilities
    static bool createTempFile(std::string& path);
    static bool createTempDirectory(std::string& path);
    static std::string generateTempFilename(const std::string& prefix = "", const std::string& extension = "");
    static std::string generateTempDirectoryname(const std::string& prefix = "");
    static FileResult cleanTempDirectory(const std::string& path);

    // Archive operations
    static FileResult createArchive(const std::string& archivePath, const std::vector<std::string>& files);
    static FileResult extractArchive(const std::string& archivePath, const std::string& destination);
    static FileResult listArchive(const std::string& archivePath, std::vector<std::string>& files);

    // File system information
    static std::string getFileSystemType(const std::string& path);
    static uint32_t getBlockSize(const std::string& path);
    static uint32_t getAllocationUnitSize(const std::string& path);

    // Error handling
    static std::string getLastError();
    static void clearLastError();

private:
    // File system monitoring
    static class FileSystemWatcher;
    static std::unique_ptr<FileSystemWatcher> m_watcher;

    // Internal methods
    static void initializeWatcher();
    static void shutdownWatcher();
    static uint64_t fileTimeToUnixTime(const fs::file_time_type& ft);
    static std::string fileTimeToString(const fs::file_time_type& ft);
    static FileResult convertFilesystemError(const std::error_code& ec);
};

// File stream wrapper
class FileStream {
public:
    FileStream();
    ~FileStream();

    // Stream operations
    bool open(const std::string& path, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out);
    void close();
    bool isOpen() const;
    bool isGood() const;
    bool isEof() const;
    bool fail() const;
    bool bad() const;

    // Stream positioning
    std::streampos tellg();
    std::streampos tellp();
    std::streampos seekg(std::streampos pos);
    std::streampos seekg(std::streamoff off, std::ios_base::seekdir dir);
    std::streampos seekp(std::streampos pos);
    std::streampos seekp(std::streamoff off, std::ios_base::seekdir dir);

    // Stream reading
    std::streamsize read(char* s, std::streamsize n);
    std::streamsize read(uint8_t* s, std::streamsize n);
    std::string readLine();
    std::string readAll();
    std::vector<uint8_t> readAllBytes();

    // Stream writing
    std::streamsize write(const char* s, std::streamsize n);
    std::streamsize write(const uint8_t* s, std::streamsize n);
    std::streamsize write(const std::string& s);
    std::streamsize write(const std::vector<uint8_t>& data);

    // Stream formatting
    FileStream& operator<<(const std::string& s);
    FileStream& operator<<(const char* s);
    FileStream& operator<<(int i);
    FileStream& operator<<(float f);
    FileStream& operator<<(double d);
    FileStream& operator<<(bool b);

    // Stream state
    void clear();
    void setstate(std::ios_base::iostate state);
    std::ios_base::iostate rdstate() const;
    bool operator!() const;
    explicit operator bool() const;

    // Stream buffer access
    std::streambuf* rdbuf() const;
    void setbuf(char* s, std::streamsize n);

    // File information
    std::string getFilename() const;
    std::string getPath() const;
    size_t getSize() const;
    bool isReadable() const;
    bool isWritable() const;

private:
    std::fstream m_stream;
    std::string m_path;
    bool m_open;
};

// File path utilities
class PathUtils {
public:
    // Path manipulation
    static std::string combine(const std::string& path1, const std::string& path2);
    static std::string combine(const std::string& path1, const std::string& path2, const std::string& path3);
    static std::string combine(const std::string& path1, const std::string& path2, const std::string& path3, const std::string& path4);
    static std::string getDirectoryName(const std::string& path);
    static std::string getFileName(const std::string& path);
    static std::string getFileNameWithoutExtension(const std::string& path);
    static std::string getExtension(const std::string& path);
    static std::string changeExtension(const std::string& path, const std::string& extension);
    static std::string removeExtension(const std::string& path);

    // Path normalization
    static std::string normalize(const std::string& path);
    static std::string makeAbsolute(const std::string& path);
    static std::string makeRelative(const std::string& path, const std::string& basePath);
    static std::string getCanonical(const std::string& path);

    // Path comparison
    static bool isEqual(const std::string& path1, const std::string& path2);
    static bool isEquivalent(const std::string& path1, const std::string& path2);

    // Path validation
    static bool isValid(const std::string& path);
    static bool isRoot(const std::string& path);
    static bool isAbsolute(const std::string& path);
    static bool isRelative(const std::string& path);

    // Path operations
    static std::string getParentPath(const std::string& path);
    static std::string getCommonPath(const std::string& path1, const std::string& path2);
    static std::string getRelativePath(const std::string& path, const std::string& basePath);
    static std::string getAbsolutePath(const std::string& path);

    // Path splitting
    static std::vector<std::string> split(const std::string& path);
    static std::vector<std::string> splitExtension(const std::string& path);
    static std::vector<std::string> splitDrive(const std::string& path);
    static std::vector<std::string> splitRoot(const std::string& path);

    // Path joining
    static std::string join(const std::vector<std::string>& paths);
    static std::string join(const std::initializer_list<std::string>& paths);

    // Path utilities
    static std::string getCurrentDirectory();
    static std::string getHomeDirectory();
    static std::string getTempDirectory();
    static std::string getAppDataDirectory();
    static std::string getDocumentsDirectory();
    static std::string getDownloadsDirectory();
    static std::string getPicturesDirectory();
    static std::string getMusicDirectory();
    static std::string getVideosDirectory();

    // Path conversion
    static std::string toUnix(const std::string& path);
    static std::string toWindows(const std::string& path);
    static std::string toNative(const std::string& path);
    static std::string fromNative(const std::string& path);

    // Path encoding
    static std::string encode(const std::string& path);
    static std::string decode(const std::string& path);
    static std::string urlEncode(const std::string& path);
    static std::string urlDecode(const std::string& path);

    // Path filtering
    static bool hasExtension(const std::string& path, const std::string& extension);
    static bool hasExtension(const std::string& path, const std::vector<std::string>& extensions);
    static bool matchesExtension(const std::string& path, const std::string& extension);
    static bool matchesExtension(const std::string& path, const std::vector<std::string>& extensions);

    // Path patterns
    static bool matchesPattern(const std::string& path, const std::string& pattern);
    static bool matchesPattern(const std::string& path, const std::vector<std::string>& patterns);
    static std::vector<std::string> filterByPattern(const std::vector<std::string>& paths, const std::string& pattern);
    static std::vector<std::string> filterByPattern(const std::vector<std::string>& paths, const std::vector<std::string>& patterns);

private:
    // Path separator detection
    static char getPathSeparator();
    static char getAltPathSeparator();
    static bool isPathSeparator(char c);
    static bool isAltPathSeparator(char c);

    // Path normalization helpers
    static std::string removeRedundantSeparators(const std::string& path);
    static std::string resolveParentReferences(const std::string& path);
    static std::string resolveCurrentReferences(const std::string& path);
};

} // namespace VortexEngine