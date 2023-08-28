#include "Configuration.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

Configuration::Configuration(const std::string& path) : m_configFile(path)
{
    if(!readFile()){
        setDefaultValues();
    }
}

Configuration::~Configuration()
{
    saveFile();
}

bool Configuration::setValue(const ConfigKey& key, const std::string& value)
{
    auto itr = m_configData.find(key);
    bool ret = itr != m_configData.end();
    if(ret) itr->second = value;
    return ret;
}

bool Configuration::setHostValue(const std::string& host, const HostConfig& key, const std::string& value)
{
    auto itr = m_hosts.find(host);
    if(itr == m_hosts.end())
        return false;
    switch(key)
    {
        case HostConfig::Password: itr->second.m_password = value; break;
        case HostConfig::Username: itr->second.m_username = value; break;
        case HostConfig::RemotePath: itr->second.m_remotePath = value; break;
        case HostConfig::HostName:
            auto copy = itr->second;
            m_hosts.erase(itr);
            m_hosts.emplace(value, copy);
        break;
    }
    return true;
}

std::string Configuration::getValue(const ConfigKey& key) const
{
    auto itr = m_configData.find(key);
    if(itr != m_configData.end())
        return itr->second;
    return "";
}

std::string Configuration::getHostValue(const std::string& host, const HostConfig& key) const
{
    auto itr = m_hosts.find(host);
    if(itr != m_hosts.end()){
        switch(key){
            case HostConfig::Password: return itr->second.m_password;
            case HostConfig::Username: return itr->second.m_username;
            case HostConfig::RemotePath: return itr->second.m_remotePath;
            case HostConfig::HostName: return itr->first;
        }
    }
    return "";
}

std::list<std::string> Configuration::getHosts() const
{
    std::list<std::string> keys;
    for (const auto& pair : m_hosts) {
        keys.push_back(pair.first);
    }
    return keys;
}

bool Configuration::deleteHost(const std::string& host)
{
    auto itr = m_hosts.find(host);
    if(itr == m_hosts.end())
        return false;
    m_hosts.erase(itr);
    return true;
}

bool Configuration::addHost(const std::string& host, const HostData& data)
{
    return m_hosts.emplace(host, data).second;
}

bool Configuration::readFile()
{
    std::ifstream file(m_configFile);
    if(!file) return false;
    auto current_host_itr = m_hosts.end();
    std::string line;
    while(std::getline(file, line)){
        if(line.empty()) continue;
        std::istringstream iss(line);
        std::string string_key, value;
        if (!(iss >> string_key >> value)){
            std::cerr << "Malformed line in config: " << line << std::endl;
            continue;
        }
        auto key = stringToKey(string_key);
        if(key != ConfigKey::None){
            // Handling config data
            auto itr = m_configData.find(key);
            if(itr != m_configData.end()){
                itr->second = value;
            } else{
                m_configData.insert({key, value});
            }
        } else{ 
            // maybe its host config key?
            auto key = stringToHostKey(string_key);
            if(key != HostConfig::None){
                if(key == HostConfig::HostName){
                    current_host_itr = m_hosts.find(value);
                    if(current_host_itr == m_hosts.end()){
                        current_host_itr = m_hosts.insert({value, {}}).first;
                    }
                } else if(current_host_itr == m_hosts.end()){
                    std::cerr << "[HOST] field must come before any other host field!" << std::endl;
                } else{
                    switch(key){
                        case HostConfig::Password: current_host_itr->second.m_password = value; break;
                        case HostConfig::RemotePath: current_host_itr->second.m_remotePath = value; break;
                        case HostConfig::Username: current_host_itr->second.m_username = value; break;
                    }
                }
            } else{ // its garbage
                std::cerr << "No such key as: " << string_key << std::endl;
                continue;
            }
        }
    }
    file.close();
    return true;
}

bool Configuration::saveFile() const
{
    std::ofstream file(m_configFile);
    if (!file) return false;
    for (const auto& pair : m_configData)
        file << keyToString(pair.first) << " " << pair.second << '\n';
    file << '\n';
    for(const auto& pair : m_hosts)
        file << keyToString(HostConfig::HostName) << " " << pair.first << '\n'
             << keyToString(HostConfig::Username) << " " << pair.second.m_username << '\n'
             << keyToString(HostConfig::Password) << " " << pair.second.m_password << '\n'
             << keyToString(HostConfig::RemotePath) << " " << pair.second.m_remotePath << "\n\n";
    file.close();
    return true;
}

void Configuration::setDefaultValues()
{
    m_configData.insert({ConfigKey::DefaultHost, "devcoo5"});
    m_configData.insert({ConfigKey::Port, "23"});
    m_configData.insert({ConfigKey::LocalPath, "C:\\example\\path"});
    m_configData.insert({ConfigKey::Difftool, "C:\\example\\path\\difftool.exe"});

    HostData example;
    example.m_username = "username";
    example.m_password = "password";
    example.m_remotePath = "appksi\\zrodla_zer\\...";
    m_hosts.insert({"devcoo5", example});
}

std::string Configuration::keyToString(const ConfigKey& key)
{
    static const std::map<ConfigKey, std::string> map = {
    {ConfigKey::DefaultHost, "DEFAULT_HOST:"},
    {ConfigKey::Port, "PORT:"},
    {ConfigKey::LocalPath, "LOCAL_PATH:"},
    {ConfigKey::Difftool, "DIFFTOOL:"}};
    auto itr = map.find(key);
    return itr->second;
}

ConfigKey Configuration::stringToKey(const std::string& key)
{
    static const std::map<std::string, ConfigKey> map = {
    {"DEFAULT_HOST:", ConfigKey::DefaultHost},
    {"PORT:", ConfigKey::Port},
    {"LOCAL_PATH:", ConfigKey::LocalPath},
    {"DIFFTOOL:", ConfigKey::Difftool}};
    auto itr = map.find(key);
    if(itr != map.end())
        return itr->second;
    return ConfigKey::None;
}

std::string Configuration::keyToString(const HostConfig& key)
{
    static const std::map<HostConfig, std::string> map = {
    {HostConfig::HostName, "HOST:"},
    {HostConfig::Password, "PASSWORD:"},
    {HostConfig::Username, "USERNAME:"},
    {HostConfig::RemotePath, "REMOTE_PATH:"}};
    auto itr = map.find(key);
    return itr->second;
}

HostConfig Configuration::stringToHostKey(const std::string& key)
{
    static const std::map<std::string, HostConfig> map = {
    {"HOST:", HostConfig::HostName},
    {"PASSWORD:", HostConfig::Password},
    {"USERNAME:", HostConfig::Username},
    {"REMOTE_PATH:", HostConfig::RemotePath}};
    auto itr = map.find(key);
    if(itr != map.end())
        return itr->second;
    return HostConfig::None;
}