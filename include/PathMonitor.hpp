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
    PathMonitor(const std::string& path = "");
    ~PathMonitor();
    bool check(bool saveSnaphshot = true);
    std::list<std::string> filesAdded() const { return m_filesAdded; }
    std::list<std::string> filesUpdated() const { return m_filesUpdated; }
    std::list<std::string> filesDeleted() const { return m_filesDeleted; }
    void reset(const std::string& path);
private:
    std::unordered_map<std::string, FileData> getSnapshot(const std::string &path);
    std::unordered_map<std::string, FileData> m_data;
    const std::string m_path;

    const std::string m_file;
    bool saveSnapshot();
    bool readSnapshot();

    void reset();
    std::list<std::string> m_filesAdded;
    std::list<std::string> m_filesUpdated;
    std::list<std::string> m_filesDeleted;
};

#endif