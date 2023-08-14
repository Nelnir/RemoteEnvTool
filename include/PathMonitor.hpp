#ifndef PATH_MONITOR_HPP
#define PATH_MONITOR_HPP

#include <list>
#include <filesystem>
#include <unordered_map>

namespace fs = std::filesystem;

struct FileData {
    fs::file_time_type last_modified;
};

class PathMonitor {
public:
    PathMonitor(const std::string& path);
    bool check();
    std::list<std::string> filesAddedd() const { return m_filesAdded; }
    std::list<std::string> filesUpdated() const { return m_filesUpdated; }
    std::list<std::string> filesDeleted() const { return m_filesDeleted; }
private:
    std::unordered_map<std::string, FileData> getSnapshot(const std::string &path);
    std::unordered_map<std::string, FileData> m_data;
    const std::string m_path;

    void reset();
    std::list<std::string> m_filesAdded;
    std::list<std::string> m_filesUpdated;
    std::list<std::string> m_filesDeleted;
};

#endif