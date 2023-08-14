#include "PathMonitor.hpp"

PathMonitor::PathMonitor(const std::string& path) : m_path(path)
{
    m_data = getSnapshot(path);
}

bool PathMonitor::check()
{
    reset();
    auto currentSnapshot = getSnapshot(m_path);

    // Check for deletions and changes
    for (const auto &[file, fileData] : m_data) {
        if (currentSnapshot.find(file) == currentSnapshot.end()) {
            m_filesDeleted.emplace_back(file);
        } else if (fileData.last_modified != currentSnapshot[file].last_modified) {
            m_filesUpdated.emplace_back(file);
        }
    }

    // Check for additions
    for (const auto &[file, fileData] : currentSnapshot) {
        if (m_data.find(file) == m_data.end()) {
            m_filesAdded.emplace_back(file);
        }
    }

    m_data = currentSnapshot;
    return !m_filesAdded.empty() || !m_filesDeleted.empty() || !m_filesUpdated.empty();
}

std::unordered_map<std::string, FileData> PathMonitor::getSnapshot(const std::string &path)
{
    std::unordered_map<std::string, FileData> snapshot;
    if(!std::filesystem::exists(path))
        return snapshot;
        
    for (const auto &entry : fs::recursive_directory_iterator(path)) {
        if (entry.is_regular_file()) {
            snapshot[entry.path().string()] = {entry.last_write_time()};
        }
    }
    return snapshot;
}

void PathMonitor::reset()
{
    m_filesAdded.clear();
    m_filesDeleted.clear();
    m_filesUpdated.clear();
}