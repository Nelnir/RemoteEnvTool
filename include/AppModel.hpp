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

class AppModel : public Subject {
public:
    AppModel();

    std::pair<bool, std::string> uploadAddedFile(const std::filesystem::path& file);
    std::pair<bool, std::string> updateRemoteFile(const std::filesystem::path& file, const bool& useDifftool);
    std::pair<bool, std::string> deleteRemoteFile(const std::filesystem::path& file);

    void runPathMonitor(const bool& updateSnapshot = false);
    bool connectToFtp();
    bool isConnectedToFtp();
    bool changedFiles() {return m_filesChanged; }
    Configuration& config() {return m_configuration; }
    PathMonitor& monitor() {return m_monitor; }
    std::filesystem::path getRemoteFileEquivalent(const std::string& file);
    std::pair<bool, std::string> downloadRemoteFile(const std::filesystem::path& file);
    bool difftool(const std::string& first, const std::string& second);
private:
    void resetPath(const std::filesystem::path& path);
    bool transferFile(const std::filesystem::path& file, const std::filesystem::path& to);
    Configuration m_configuration;
    PathMonitor m_monitor;
    sf::Ftp m_ftp;
    bool m_filesChanged;
    std::string m_workingDir;
};


#endif