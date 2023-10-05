#ifndef PATH_MONITOR_HPP
#define PATH_MONITOR_HPP

#include <list>
#include <filesystem>
#include <unordered_map>

class MonitoringStrategy{
public:
    virtual ~MonitoringStrategy() {}
    virtual std::vector<std::filesystem::path> getAddedFiles() const {return m_added;}
    virtual std::vector<std::filesystem::path> getRemovedFiles() const {return m_removed;}
    virtual std::vector<std::filesystem::path> getUpdatedFiles() const {return m_updated;}
    virtual bool check(const std::filesystem::path& path) = 0;
protected:
    std::vector<std::filesystem::path> m_added;
    std::vector<std::filesystem::path> m_removed;
    std::vector<std::filesystem::path> m_updated;
};

class GitMonitoringStrategy : public MonitoringStrategy{
public:
    bool check(const std::filesystem::path& path);
};

class GitBranchChangesStrategy : public MonitoringStrategy{
private:
    std::string m_compareWith;
public:
    bool check(const std::filesystem::path& path);
    std::string getCurrentBranch(const std::filesystem::path& path);
    void compareWith(const std::string& str) {m_compareWith = str;}
};

class PathMonitor {
public:
    PathMonitor(const std::filesystem::path& path = "", std::unique_ptr<MonitoringStrategy> strategy = std::make_unique<GitMonitoringStrategy>());
    std::vector<std::filesystem::path> filesAdded() const{
        return m_strategy->getAddedFiles();
    }
    std::vector<std::filesystem::path> filesRemoved() const {
        return m_strategy->getRemovedFiles();
    }
    std::vector<std::filesystem::path> filesUpdated() const {
        return m_strategy->getUpdatedFiles();
    }
    bool check(){
        return m_strategy->check(m_path);
    }
    void setStrategy(std::unique_ptr<MonitoringStrategy> strategy){
        m_strategy = std::move(strategy);
    }
    std::filesystem::path getPath() const {return m_path;}
private:
    const std::filesystem::path m_path;
    std::unique_ptr<MonitoringStrategy> m_strategy;
};

#endif