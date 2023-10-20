#include "AppModel.hpp"
#include <boost/algorithm/string/replace.hpp>
#include <iostream>

AppModel::AppModel() : m_monitor(m_configuration.getValue(ConfigKey::LocalPath))
{

}

std::filesystem::path AppModel::getRemoteFileEquivalent(const std::filesystem::path& file)
{
    auto host = m_configuration.getCurrentHost();
    if(host.m_remotePath.empty()){
        return m_telnet.home() + '/' + file.string();
    }
    std::string remote = m_workingDir + host.m_remotePath + (host.m_remotePath.back() == '/' ? "" : "/") + file.string();
    return remote;
}

std::pair<bool, std::string> AppModel::uploadAddedFile(const std::filesystem::path& file, const bool& suppressOutput)
{
    auto local_file = m_configuration.getValue(ConfigKey::LocalPath) + file.string();
    std::pair<bool, std::string> ret;
    const auto& remote = getRemoteFileEquivalent(file.string());
    if(m_ftp.changeDirectory(remote.parent_path().string()).isOk()){
        ret.first = m_ftp.upload(local_file, "", sf::Ftp::TransferMode::Ascii).isOk();
        if(ret.first && !suppressOutput){
            notifyGood("Success: file uploaded " + local_file);
        } else if(!suppressOutput){
            notifyBad("Error: when uploading file " + local_file);
        }
    }
    ret.second = remote.string();
    return ret;
}

std::pair<bool, std::string> AppModel::updateRemoteFile(const std::filesystem::path& file, const bool& useDifftool, const bool& suppressOutput)
{
    auto local_file = m_configuration.getValue(ConfigKey::LocalPath) + file.string();
    auto remote = getRemoteFileEquivalent(file);
    auto& result = downloadRemoteFile(file, true);
    if(!result.first){
        return std::make_pair(false, result.second);
    }
    std::string fileToUpload;
    if(useDifftool){
        // force file change
        if(!difftool(result.second, local_file)){
            notifyBad("Error: left file must be edited");
            std::filesystem::remove(result.second);
            return std::make_pair(false, remote.string());
        }
        fileToUpload = result.second;
    } else{
        fileToUpload = local_file;
    }
    result.first = transferFile(fileToUpload, remote, true);
    std::filesystem::remove(result.second);
    if(result.first){
        if(!suppressOutput){
            notifyGood("Success: file updated " + remote.string());
        }
    } else if(!suppressOutput){
        notifyBad("Error: when updating file " + remote.string());
    }
    return std::make_pair(result.first, remote.string());
}

std::pair<bool, std::string> AppModel::deleteRemoteFile(const std::filesystem::path& file, const bool& suppressOutput)
{
    std::pair<bool, std::string> ret;
    const auto& remote = getRemoteFileEquivalent(file.string());
    if(m_ftp.changeDirectory(remote.parent_path().string()).isOk()){
        ret.first = m_ftp.deleteFile(remote).isOk();
        if(ret.first){
            if(!suppressOutput){
                notifyGood("Success: deleted file " + remote.string());
            }
        } else if(!suppressOutput){
            notifyBad("Error: unable to delete file " + remote.string());
        }
    }
    ret.second = remote.string();
    return ret;
}

bool AppModel::transferFile(const std::filesystem::path& file, const std::filesystem::path& to, const bool& suppressOutput)
{
    if(m_ftp.changeDirectory(to.parent_path().string()).isOk()){
        if(m_ftp.upload(file, "", sf::Ftp::TransferMode::Ascii).isOk()){
            if(!suppressOutput){
                notifyGood("Success: transfered file " + file.string());
            }
            return true;
        } else if(!suppressOutput){
            notifyBad("Error: when transfering file " + file.string());
        }
    }
    return false;
}

bool AppModel::connectToFtp(const HostData& host)
{
    auto ip = sf::IpAddress::resolve(host.m_hostname);
    if(!ip.has_value()){
        notifyBad("Error: unable to resolve ip address of ftp: " + host.m_hostname);
        return false;
    }
    auto response = m_ftp.connect(ip.value());
    if(response.isOk()){
        response = m_ftp.login(host.m_username, host.m_password);
        if(response.isOk()){
            m_workingDir = m_ftp.getWorkingDirectory().getDirectory().string() + "/";
            notifyGood("Success: connected via FTP to: " + host.m_hostname);
        } else{
            notifyBad("Error: wrong credentials when connecting via ftp to: " + host.m_hostname);
        }
    }
    return response.isOk();
}

bool AppModel::connectToTelnet(const HostData& host)
{
    notify("Connecting to telnet " + host.m_hostname + "...");
    auto ip = sf::IpAddress::resolve(host.m_hostname);
    if(!ip.has_value()){
        notifyBad("Error: unable to resolve ip address of telnet: " + host.m_hostname);
        return false;
    }
    if(!m_telnet.connect(ip.value())){
        notifyBad("Error: unable to connect via telnet to: " + host.m_hostname);
        return false;
    }
    if(!m_telnet.login(host.m_username, host.m_password)){
        notifyBad("Error: wrong credentials when connecting via telnet to: " + host.m_hostname);
        return false;
    }
    if(!m_telnet.executeInitialScript(host.m_script)){
        notifyBad("Error: when executing initial script: " + host.m_script);
        return false;
    }
    notifyGood("Success: connected via telnet to: " + host.m_hostname);
    return true;
}

bool AppModel::isConnectedToFtp()
{
    return m_ftp.sendCommand("NOOP").isOk();
}

std::pair<bool, std::string> AppModel::downloadRemoteFile(const std::filesystem::path& file, const bool& suppressOutput)
{
    std::pair<bool, std::string> ret;
    const auto& remote = getRemoteFileEquivalent(file.string());
    if(m_ftp.changeDirectory(remote.parent_path().string()).isOk()){
        if(!std::filesystem::exists("temp/")){
            std::filesystem::create_directory("temp/");
        }
        ret.first = m_ftp.download(remote, "temp/", sf::Ftp::TransferMode::Ascii).isOk();
        if(ret.first){
            ret.second = "temp/" + remote.filename().string();
            if(!suppressOutput){
                notifyGood("Success: downloaded file to " + ret.second);
            }
        } else if(!suppressOutput){
            notifyBad("Error: when downloading file " + file.string());
        }
    } else{
        notifyBad("Error: when changing directory to " + remote.parent_path().string());
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

bool AppModel::listChangedFiles()
{
    if(!m_monitor.check()){
        notify("No files changed.");
        return false;
    }

    const auto& updated = m_monitor.filesUpdated();
    if(!updated.empty())
        notifyGood("UPDATED:");
    for(const auto& file : updated){
        notifyGood(file.string());
    }

    const auto& added = m_monitor.filesAdded();
    if(!added.empty()){
        if(!updated.empty())
            notify("");
        notifyGood("ADDED:");
    }
    for(const auto& file : added){
        notifyGood(file.string());
    }

    const auto& removed = m_monitor.filesRemoved();
    if(!removed.empty()){
        if(!added.empty())
            notify("");
        notifyBad("DELETED:");
    }
    for(const auto& file : removed){
        notifyBad(file.string());
    }
    return true;
}

std::pair<bool, std::string> AppModel::tlog(const std::string& filename)
{
    auto host = m_configuration.getCurrentHost();
    if(!m_telnet.isConnected()){
        if(!connectToTelnet(host)){
            return std::make_pair(false, "");
        }
    }

    notify("Starts writing to file, press enter to stop...");
    
    auto command = "tlog > " + filename;
    notify(command);
    m_telnet.executeCommand(command, false, true);
    std::string input;
    std::getline(std::cin, input);
    m_telnet.executeCommand("\x03", false, true);

    if(!isConnectedToFtp()){
        notify("Connecting via FTP in order to download file...");
        if(!connectToFtp(host)){
            return std::make_pair(false, "");
        }
    }

    auto result = downloadRemoteFile(filename);
    if(result.first){
        notify("Removing file from remote host...");
        deleteRemoteFile(filename);
    }
    return result;
}

bool AppModel::restart(const std::string& arg)
{
    auto host = m_configuration.getCurrentHost();
    if(!m_telnet.isConnected()){
        if(!connectToTelnet(host)){
            return false;
        }
    }

    notify("Executing restart: " + arg);
    if(arg == "env"){
        m_telnet.executeCommand("tmshutdown -y", true);
        m_telnet.executeCommand("tmboot -y", true);
    } else if(arg == "retux"){
        m_telnet.executeCommand("cd $APPDIR", true);
        m_telnet.executeCommand("./RetuxAdapter.sh stop", true);
        m_telnet.executeCommand("./RetuxAdapter.sh start", true);
    } else{
        m_telnet.executeCommand("tmshutdown -s " + arg, true);
        m_telnet.executeCommand("tmboot -s " + arg, true);
    }
    return true;
}

bool AppModel::script(const std::string& script)
{
    auto host = m_configuration.getCurrentHost();
    if(!m_telnet.isConnected()){
        if(!connectToTelnet(host)){
            return false;
        }
    }

    notify("Executing script: " + script);
    m_telnet.executeCommand(script, true);
    return 0;
}

bool AppModel::transfer(const std::string& arg, const bool& useDifftool)
{
    if(!isConnectedToFtp()){
        if(!connectToFtp(m_configuration.getCurrentHost())){
            return false;
        }
    }

    if(m_configuration.getCurrentHost().m_remotePath.empty()){
        notifyBad("This host doesn't support transferring files, please set REMOTE_PATH in config file.");
        return false;
    }

    if(!m_monitor.check()){
        notify("No files changed.");
        return true;
    }

    if(arg == "updated" || arg == "all"){
        for(const auto& file : m_monitor.filesUpdated()){
            notify("Updating file: " + file.string());
            const auto& result = updateRemoteFile(file, useDifftool);
            if(!result.first){
                return false;
            }
        }
    }

    if(arg == "added" || arg == "all"){
        for(const auto& file : m_monitor.filesAdded()){
            notify("Uploading file: " + file.string());
            const auto& result = uploadAddedFile(file, useDifftool);
            if(!result.first){
                return false;
            }
        }
    }
    if(arg == "deleted" || arg == "all"){
        for(const auto& file : m_monitor.filesRemoved()){
            notify("Deleting file: " + file.string());
            const auto& result = deleteRemoteFile(file, useDifftool);
            if(!result.first){
                return false;
            }
        }
    }
    return true;
}
