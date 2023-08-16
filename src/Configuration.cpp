#include "Configuration.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

Configuration::Configuration(const std::string& path) : m_configFile(path)
{
    setDefaultValues();
    readFile();
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

std::string Configuration::getValue(const ConfigKey& key) const
{
    auto itr = m_configData.find(key);
    if(itr != m_configData.end())
        return itr->second;
    return "";
}

bool Configuration::readFile()
{
    std::ifstream file(m_configFile);
    if(!file) return false;
    std::string line;
    while(std::getline(file, line)){
        std::istringstream iss(line);
        std::string key, value;
        if (!(iss >> key >> value)){
            std::cerr << "Malformed line in config: " + line << std::endl;
            continue;
        }
        auto itr = m_configData.find(stringToKey(key));
        if(itr == m_configData.end()){
            std::cerr << "No such key as: " << key << std::endl;
            continue;
        }
        itr->second = value;
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
    file.close();
    return true;
}

void Configuration::setDefaultValues()
{
    m_configData.insert({ConfigKey::Host, "devcoo5"});
    m_configData.insert({ConfigKey::Port, "23"});
    m_configData.insert({ConfigKey::Path, "local path to sources which will be monitored"});
    m_configData.insert({ConfigKey::RemotePath, "remote path to sources which will be updated"});
    m_configData.insert({ConfigKey::Difftool, "meld"});
}

std::string Configuration::keyToString(const ConfigKey& key) const
{
const std::map<ConfigKey, std::string> map = {
    {ConfigKey::Host, "Host"},
    {ConfigKey::Port, "Port"},
    {ConfigKey::Path, "Path"},
    {ConfigKey::RemotePath, "RemotePath"},
    {ConfigKey::Difftool, "Difftool"}};
    auto itr = map.find(key);
    return itr->second;
}

ConfigKey Configuration::stringToKey(const std::string& key)
{
    static const std::map<std::string, ConfigKey> map = {
    {"Host", ConfigKey::Host},
    {"Port", ConfigKey::Port},
    {"Path", ConfigKey::Path},
    {"RemotePath", ConfigKey::RemotePath},
    {"Difftool", ConfigKey::Difftool}};
    auto itr = map.find(key);
    if(itr != map.end())
        return itr->second;
    return ConfigKey::None;
}