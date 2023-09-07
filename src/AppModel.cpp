#include "AppModel.hpp"
#include <iostream>

AppModel::AppModel() : m_monitor(m_configuration.getValue(ConfigKey::LocalPath))
{

}

std::pair<bool, std::string> AppModel::uploadAddedFile(const std::filesystem::path& file)
{
    std::pair<bool, std::string> ret;
    const auto& remote = config().getRemoteFileEquivalent(file.string());
    if(m_ftp.changeDirectory(remote.parent_path().string()).isOk()){
        ret.first = m_ftp.upload(file, "", sf::Ftp::TransferMode::Ascii).isOk();
        resetPath(file);
    }
    ret.second = remote.string();
    return ret;
}

std::pair<bool, std::string> AppModel::updateRemoteFile(const std::filesystem::path& file, const bool& useDifftool)
{
    const auto& remote = config().getRemoteFileEquivalent(file.string());
    auto& result = downloadRemoteFile(file);
    if(!result.first){
        return std::make_pair(false, remote.string());
    }
    std::string fileToUpload;
    if(useDifftool){
        // force file change
        if(!difftool(result.second, file.string())){
            std::filesystem::remove(result.second);
            return std::make_pair(false, remote.string());
        }
        fileToUpload = result.second;
    } else{
        fileToUpload = file.string();
    }
    result.first = transferFile(fileToUpload, remote);
    std::filesystem::remove(result.second);
    if(result.first){
        resetPath(file);
    }
    return std::make_pair(result.first, remote.string());
}

std::pair<bool, std::string> AppModel::deleteRemoteFile(const std::filesystem::path& file)
{
    std::pair<bool, std::string> ret;
    const auto& remote = config().getRemoteFileEquivalent(file.string());
    if(m_ftp.changeDirectory(remote.parent_path().string()).isOk()){
        ret.first = m_ftp.deleteFile(remote).isOk();
        if(ret.first){
            resetPath(file);
        }
    }
    ret.second = remote.string();
    return ret;
}

bool AppModel::transferFile(const std::filesystem::path& file, const std::filesystem::path& to)
{
    if(m_ftp.changeDirectory(to.parent_path().string()).isOk()){
        return m_ftp.upload(file, "", sf::Ftp::TransferMode::Ascii).isOk();
    }
    return false;
}

bool AppModel::runPathMonitor(const bool& updateSnapshot) 
{
    return m_monitor.check(updateSnapshot);
}

void AppModel::resetPath(const std::filesystem::path& path)
{
    m_monitor.reset(path.string());
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

std::pair<bool, std::string> AppModel::downloadRemoteFile(const std::filesystem::path& file)
{
    std::pair<bool, std::string> ret;
    const auto& remote = config().getRemoteFileEquivalent(file.string());
    if(m_ftp.changeDirectory(remote.parent_path().string()).isOk()){
        if(!std::filesystem::exists("temp/")){
            std::filesystem::create_directory("temp/");
        }
        ret.first = m_ftp.download(remote, "temp/", sf::Ftp::TransferMode::Ascii).isOk();
        if(ret.first){
            ret.second = "temp/" + remote.filename().string();
        }
    }
    return ret;
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