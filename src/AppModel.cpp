#include "AppModel.hpp"
#include <iostream>
#include <boost/algorithm/string/replace.hpp>

AppModel::AppModel() : m_monitor(m_configuration.getValue(ConfigKey::LocalPath)), m_filesChanged(false)
{

}

std::list<std::string> AppModel::changedFiles(const FileChangeType& type) const
{
    std::list<std::string> ret;
    switch(type){
        case FileChangeType::Added: ret = m_monitor.filesAddedd(); break;
        case FileChangeType::Removed: ret = m_monitor.filesDeleted(); break;
        case FileChangeType::Updated: ret = m_monitor.filesUpdated(); break;
    }
    return ret;
}

bool AppModel::transferFile(const std::filesystem::path& file, const std::filesystem::path& remote)
{
    if(m_ftp.changeDirectory(remote.parent_path().string()).isOk()){
        return m_ftp.upload(file, "", sf::Ftp::TransferMode::Ascii).isOk();
    }
    return false;
}

bool AppModel::deleteRemoteFile(const std::string& file)
{
    return m_ftp.deleteFile(file).isOk();
}

void AppModel::runPathMonitor(bool updateSnapshot) 
{
    m_filesChanged = m_monitor.check(updateSnapshot);
}

void AppModel::resetPath(const std::string& path)
{
    m_monitor.reset(path);
}

bool AppModel::connectToFtp()
{
    auto data = m_configuration.getCurrentHost();
    auto ip = sf::IpAddress::resolve(data.first);
    if(!ip.has_value())
        return false;
    auto response = m_ftp.connect(ip.value());
    if(response.isOk()){
        response = m_ftp.login(data.second.m_username, data.second.m_password);
        if(response.isOk()){
            m_workingDir = m_ftp.getWorkingDirectory().getDirectory().string() + "/";
        }
    }
    return response.isOk();
}

bool AppModel::isConnectedToFtp()
{
    return m_ftp.sendCommand("NOOP").isOk();
}

std::filesystem::path AppModel::getRemoteFileEquivalent(const std::string& file)
{
    std::string remote = m_workingDir + m_configuration.getCurrentHost().second.m_remotePath;
    std::string local = m_configuration.getValue(ConfigKey::LocalPath);
    std::string fileWithoutPath = file.substr(local.size());
    boost::replace_all(fileWithoutPath, "\\", "/");
    if(!fileWithoutPath.empty()){
        if(fileWithoutPath[0] == '/'){
            fileWithoutPath = fileWithoutPath.substr(1);
        }
    }
    remote += fileWithoutPath;
    return remote;
}

bool AppModel::downloadRemoteFile(const std::filesystem::path& remote, std::filesystem::path& to)
{
    if(m_ftp.download(remote, "", sf::Ftp::TransferMode::Ascii).isOk()){
        to = "REMOTE." + remote.extension().string();
        std::filesystem::rename(remote.filename(), to);
        return true;
    }
    return false;
}

bool AppModel::difftool(const std::string& first, const std::string& second)
{
    const auto& last_modified = std::filesystem::last_write_time(first);
    std::string diffCommand = m_configuration.getValue(ConfigKey::Difftool) + " " + first + " " + second;
#ifdef _WIN32
    system(diffCommand.c_str());
#endif
    return last_modified != std::filesystem::last_write_time(first);
}