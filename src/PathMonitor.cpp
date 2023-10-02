#include "PathMonitor.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <thread>
#include <set>

PathMonitor::PathMonitor(const std::filesystem::path& path) : m_path(path),
m_file("snapshot.txt")
{
    // if no snapshot to read,
    if(!readSnapshot()){
        m_data = getSnapshot(path); // get current snapshot
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(3));
}

PathMonitor::~PathMonitor()
{
    if(!saveSnapshot()){
        std::cerr << "Unable to save file snapshot." << std::endl;
    }
}

bool PathMonitor::check(const bool& saveSnapshot)
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

    if(saveSnapshot)
        m_data = currentSnapshot;
    return !m_filesAdded.empty() || !m_filesDeleted.empty() || !m_filesUpdated.empty();
}

void PathMonitor::reset(const std::filesystem::path& file)
{
    auto itr = m_data.find(file);
    if(itr == m_data.end()){
        //m_data[file] = {std::filesystem::last_write_time(file).time_since_epoch().count()};
    } else if(std::filesystem::exists(file)){
        itr->second.last_modified = std::filesystem::last_write_time(file).time_since_epoch().count();
    } else{ // file was deleted
        m_data.erase(itr);
    }
}

std::unordered_map<std::filesystem::path, FileData> PathMonitor::getSnapshot(const std::filesystem::path &path)
{
    std::unordered_map<std::filesystem::path, FileData> snapshot;
    if(!std::filesystem::exists(path))
        return snapshot;

    const std::set<std::string> extensions = {".cpp", ".h", ".hpp"};
        
    for (const auto &entry : fs::recursive_directory_iterator(path)) {
        const auto& path = entry.path().string();
        if (entry.is_regular_file() && extensions.count(entry.path().extension().string())) {
            snapshot[path] = {entry.last_write_time().time_since_epoch().count()};
        }
    }
    return snapshot;
}

bool PathMonitor::saveSnapshot()
{
    if(m_data.empty()) return true;
    std::ofstream file(m_file);
    if(!file) return false;
    for(const auto& pair : m_data){
        file << pair.first.string() << " " << pair.second.last_modified << '\n';
    }
    file.close();
    return true;
}

bool PathMonitor::readSnapshot()
{
    std::ifstream file(m_file);
    if(!file) return false;
    std::string line;
    while(std::getline(file, line)){
        std::istringstream iss(line);
        std::string key;
        int64_t nano;
        if(!(iss >> key >> nano)){
            std::cerr << "Malformed line in config: " << line << std::endl;
            continue;
        }
        auto itr = m_data.find(key);
        if(itr == m_data.end()){
            FileData data;
            data.last_modified = nano;
            m_data.insert(std::make_pair(key, data));
        }
    }
    return true;
}

void PathMonitor::reset()
{
    m_filesAdded.clear();
    m_filesDeleted.clear();
    m_filesUpdated.clear();
}