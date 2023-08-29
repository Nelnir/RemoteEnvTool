#ifndef APPMODEL_HPP
#define APPMODEL_HPP

#include "PathMonitor.hpp"
#include "Configuration.hpp"
#include <SFML/Network.hpp>

class Observer{
public:
    virtual void update() = 0;
};

class Subject{
public:
    void attach(Observer* obs) {m_observers.push_back(obs);}
    void detach(Observer* obs) {m_observers.remove(obs);}
    void notify(){
        std::for_each(m_observers.begin(), m_observers.end(), [&](Observer* obs) {obs->update();});
    }
private:
    std::list<Observer*> m_observers;
};

enum class FileChangeType{
    Added,
    Removed,
    Updated
};

class AppModel : public Subject {
public:
    AppModel();

    std::pair<std::string, HostData> currentHost() const { return m_configuration.getCurrentHost(); }

    bool changedFiles() {return m_filesChanged; }
    std::list<std::string> changedFiles(const FileChangeType& type) const;
    bool transferFile(const std::filesystem::path& file, const std::filesystem::path& remote);
    bool deleteRemoteFile(const std::string& file);
    void runPathMonitor(bool updateSnapshot = false);
    void resetPath(const std::string& path);
    bool connectToFtp();
    bool isConnectedToFtp();
    Configuration& config() {return m_configuration; }
    std::filesystem::path getRemoteFileEquivalent(const std::string& file);
    bool downloadRemoteFile(const std::filesystem::path& remote, std::filesystem::path& to);
    bool difftool(const std::string& first, const std::string& second);
private:
    Configuration m_configuration;
    PathMonitor m_monitor;
    sf::Ftp m_ftp;
    bool m_filesChanged;
    std::string m_workingDir;
};


#endif