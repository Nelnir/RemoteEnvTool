#ifndef PATH_MONITOR_HPP
#define PATH_MONITOR_HPP

#include <list>
#include <filesystem>
#include <unordered_map>

namespace fs = std::filesystem;

struct FileData {
    int64_t last_modified;
};

class PathMonitor {
public:
    PathMonitor(const std::filesystem::path& path = "");
    ~PathMonitor();
    bool check(const bool& saveSnaphshot = true);
    std::list<std::filesystem::path> filesAdded() const { return m_filesAdded; }
    std::list<std::filesystem::path> filesUpdated() const { return m_filesUpdated; }
    std::list<std::filesystem::path> filesDeleted() const { return m_filesDeleted; }
    void reset(const std::filesystem::path& path);
private:
    std::unordered_map<std::filesystem::path, FileData> getSnapshot(const std::filesystem::path &path);
    std::unordered_map<std::filesystem::path, FileData> m_data;
    const std::filesystem::path m_path;

    const std::filesystem::path m_file;
    bool saveSnapshot();
    bool readSnapshot();

    void reset();
    std::list<std::filesystem::path> m_filesAdded;
    std::list<std::filesystem::path> m_filesUpdated;
    std::list<std::filesystem::path> m_filesDeleted;
};

#endif