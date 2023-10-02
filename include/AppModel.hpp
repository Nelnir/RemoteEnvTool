#ifndef APPMODEL_HPP
#define APPMODEL_HPP

#include "PathMonitor.hpp"
#include "Configuration.hpp"
#include "TelnetClient.hpp"
#include <SFML/Network.hpp>

class Observer{
public:
    virtual void update(const std::string&) = 0;
    virtual void updateGood(const std::string&) = 0;
    virtual void updateBad(const std::string&) = 0;
};

class Subject{
public:
    void attach(Observer* obs) {m_observers.push_back(obs);}
    void detach(Observer* obs) {m_observers.remove(obs);}
    void notify(const std::string& str){
        std::for_each(m_observers.begin(), m_observers.end(), [&](Observer* obs) {obs->update(str);});
    }
    void notifyGood(const std::string& str){
        std::for_each(m_observers.begin(), m_observers.end(), [&](Observer* obs) {obs->updateGood(str);});
    }
    void notifyBad(const std::string& str){
        std::for_each(m_observers.begin(), m_observers.end(), [&](Observer* obs) {obs->updateBad(str);});
    }
private:
    std::list<Observer*> m_observers;
};

class AppModel : public Subject {
    friend class AppCLIFeatures;
public:
    AppModel();

    bool connectToFtp(const HostData& host);
    bool connectToTelnet(const HostData& host);
    bool isConnectedToFtp();
    Configuration& config() { return m_configuration; }
    PathMonitor& monitor() { return m_monitor; }
    TelnetClient& telnet() { return m_telnet; }

    bool listChangedFiles();
    bool tlog(const std::string& filename);
    bool restart(const std::string& arg);
    bool script(const std::string& script);
    bool transfer(const std::string& arg);
private:
    bool transferFile(const std::filesystem::path& file, const std::filesystem::path& to, const bool& suppressOutput = false);
    std::filesystem::path getRemoteFileEquivalent(const std::string& file);
    std::pair<bool, std::string> uploadAddedFile(const std::filesystem::path& file, const bool& suppressOutput = false);
    std::pair<bool, std::string> updateRemoteFile(const std::filesystem::path& file, const bool& useDifftool, const bool& suppressOutput = false);
    std::pair<bool, std::string> deleteRemoteFile(const std::filesystem::path& file, const bool& suppressOutput = false);
    std::pair<bool, std::string> downloadRemoteFile(const std::filesystem::path& file, const bool& suppressOutput = false);

    bool difftool(const std::string& first, const std::string& second);

    Configuration m_configuration;
    PathMonitor m_monitor;
    TelnetClient m_telnet;
    sf::Ftp m_ftp;
    std::string m_workingDir;
};


#endif