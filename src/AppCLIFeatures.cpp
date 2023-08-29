#include "AppCLIFeatures.hpp"
#include "AppCLIView.hpp"
#include <thread>
#include <iostream>

AppCLIFeatures::AppCLIFeatures(AppModel& model, AppCLIView& view) : m_model(model), m_view(view)
{
    registerFeature("Exit", [this](const AppCLIController& controller) {}, EXIT_OPTION());
    registerFeature("List changed files", std::bind(&AppCLIFeatures::listChangedFiles, this, std::placeholders::_1), LISTS_FILE());
    registerFeature("Transfer files", std::bind(&AppCLIFeatures::transferFiles, this, std::placeholders::_1), TRANSFER_FILES());
    registerFeature("Reset file snapshot", std::bind(&AppCLIFeatures::resetSnapshot, this, std::placeholders::_1), RESET_SNAPSHOT());
    registerFeature("Change host", std::bind(&AppCLIFeatures::changeHost, this, std::placeholders::_1), CHANGE_HOST());
    registerFeature("Help", std::bind(&AppCLIFeatures::help, this, std::placeholders::_1), HELP());
}

void AppCLIFeatures::registerFeature(const std::string& name, const FeatureCallback& callback, const int& index){
    if(index == -1){
        m_features.try_emplace(m_features.size(), std::make_pair(name, callback));
    } else{
        m_features.emplace(index, std::make_pair(name, callback));
    }
}

void AppCLIFeatures::listChangedFiles(AppCLIController& controller)
{
    m_model.runPathMonitor();
    const auto& updated = m_model.changedFiles(FileChangeType::Updated);
    if(!updated.empty())
        m_view.writeGreen("UPDATED:");
    for(const auto& file : updated){
        m_view.writeGreen(file);
    }

    const auto& added = m_model.changedFiles(FileChangeType::Added);
    if(!added.empty())
        m_view.writeGreen("ADDED:");
    for(const auto& file : added){
        m_view.writeGreen(file);
    }

    const auto& removed = m_model.changedFiles(FileChangeType::Removed);
    if(!removed.empty())
        m_view.writeRed("DELETED:");
    for(const auto& file : removed){
        m_view.writeRed(file);
    }
    if(updated.empty() && added.empty() && removed.empty()){
        m_view.writeWhite("No files changed.");
    }
}

void AppCLIFeatures::transferFiles(AppCLIController& controller)
{
    m_model.runPathMonitor();
    if(!m_model.changedFiles()){
        m_view.writeWhite("No files changed.");
        return;
    }

    if(!m_model.isConnectedToFtp()){
        m_view.writeWhite("Connecting to ftp server...");
        if(m_model.connectToFtp()){
            m_view.writeGreen("Success");
        } else{
            m_view.writeRed("Error when connecting to ftp.");
            return;
        }
    }

    for(const auto& file : m_model.changedFiles(FileChangeType::Added)){
        const auto& remote = m_model.getRemoteFileEquivalent(file);
        m_view.writeWhite("Upload file (y/n): " + file + "?");
        if(controller.yes()){
            if(m_model.transferFile(file, remote)){
                m_view.writeGreen("File uploaded: " + remote.string());
                m_model.resetPath(file);
            } else{
                m_view.writeRed("Unable to upload.");
            }
        }
    }

    for(const auto& file : m_model.changedFiles(FileChangeType::Updated)){
        const auto& remote = m_model.getRemoteFileEquivalent(file);
        std::filesystem::path downloadedFilepath;
        if(!m_model.downloadRemoteFile(remote, downloadedFilepath)){
            m_view.writeRed("Unable to download remote file: " + remote.string());
            continue;
        }
        m_view.writeWhite("Use difftool for (y/n): " + remote.string() + "?");
        bool sendModifiedFile = false;
        if(controller.yes()){
            sendModifiedFile = m_model.difftool(downloadedFilepath.string(), file);
            if(sendModifiedFile){
                auto newPath = downloadedFilepath.parent_path() / remote.filename();
                std::filesystem::rename(downloadedFilepath, newPath);
                downloadedFilepath = newPath;
            }
        }
        if(!sendModifiedFile){
            m_view.writeWhite("Then simply overwrite remote with local (y/n)?");
            if(!controller.yes()){
                continue;
            }
        }
        
        const std::string uploadFile = sendModifiedFile ? downloadedFilepath.string() : file;
        if(m_model.transferFile(uploadFile, remote)){
            m_view.writeGreen("File updated: " + remote.string());
            m_model.resetPath(file);
        } else{
            m_view.writeRed("Unable to update file: " + remote.string());
        }
    }

    for(const auto& file : m_model.changedFiles(FileChangeType::Removed)){
        const auto& remote = m_model.getRemoteFileEquivalent(file);
        m_view.writeWhite("Delete remote file (y/n): " + remote.string());
        if(controller.yes()){
            if(m_model.deleteRemoteFile(remote.string())){
                m_view.writeGreen("File removed: " + remote.string());
                m_model.resetPath(file);
            } else{
                m_view.writeRed("Couldn't remove file: " + remote.string());
            }
        }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(750));
    m_model.notify();
}

void AppCLIFeatures::resetSnapshot(AppCLIController& controller)
{
    m_model.runPathMonitor(true);
    m_view.writeWhite("Snapshot resetted.");
}

void AppCLIFeatures::changeHost(AppCLIController& controller)
{
    m_view.writeWhite("Available hosts:");
    const auto& hosts = m_model.config().getHosts();
    for(const auto& host : hosts){
        if(host == m_model.currentHost().first){
            m_view.writeGreen(host);
        } else{
            m_view.writeWhite(host);
        }
    }
    const auto& selectedHost = controller.read();
    if(m_model.config().setValue(ConfigKey::DefaultHost, selectedHost)){
        m_view.writeGreen("Host was changed to: " + selectedHost);
    } else{
        m_view.writeRed("Unable to choose: " + selectedHost);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(750));
    m_model.notify();
}

void AppCLIFeatures::help(AppCLIController& controller)
{
    m_view.writeWhite("The left file will be send to remote FTP, if modified during difftool display.");
    m_view.writeWhite("In order to add new host, see file config.txt and update accordingly.");
}