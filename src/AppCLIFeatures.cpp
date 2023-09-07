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
    auto& path = m_model.monitor();
    const auto& updated = path.filesUpdated();
    if(!updated.empty())
        m_view.writeGreen("UPDATED:");
    for(const auto& file : updated){
        m_view.writeGreen(file.string());
    }

    const auto& added = path.filesAdded();
    if(!added.empty())
        m_view.writeGreen("ADDED:");
    for(const auto& file : added){
        m_view.writeGreen(file.string());
    }

    const auto& removed = path.filesDeleted();
    if(!removed.empty())
        m_view.writeRed("DELETED:");
    for(const auto& file : removed){
        m_view.writeRed(file.string());
    }
    if(updated.empty() && added.empty() && removed.empty()){
        m_view.writeWhite("No files changed.");
    }
}

void AppCLIFeatures::transferFiles(AppCLIController& controller)
{
    if(!m_model.runPathMonitor()){
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

    for(const auto& file : m_model.monitor().filesUpdated()){
        m_view.writeWhite("Update file (y/n): " + file.string());
        if(controller.yes()){
            m_view.writeWhite("Use difftool (y/n):");
            const auto& difftool = controller.yes();
            if(!difftool){
                m_view.writeWhite("Then simply overwrite remote with local (y/n)?");
                if(!controller.yes())
                    continue;
            }
            const auto& result = m_model.updateRemoteFile(file, difftool);
            if(!result.first){
                if(difftool){
                    m_view.writeRed("Unable to update file (left file must be changed): " + result.second);
                } else{
                    m_view.writeRed("Unable to update file: " + result.second);
                }
            } else{
                m_view.writeGreen("File updated: " + result.second);
            }
        }
    }

    for(const auto& file : m_model.monitor().filesAdded()){
        m_view.writeWhite("Upload file (y/n): " + file.string());
        if(controller.yes()){
            const auto& result = m_model.uploadAddedFile(file);
            if(result.first){
                m_view.writeGreen("File uploaded: " + result.second);
            } else{
                m_view.writeRed("Error: unable to upload " + result.second);
            }
        }
    }

    for(const auto& file : m_model.monitor().filesDeleted()){
        const auto& remote = m_model.config().getRemoteFileEquivalent(file);
        m_view.writeWhite("Delete remote file (y/n): " + remote.string());
        if(controller.yes()){
            if(m_model.deleteRemoteFile(file).first){
                m_view.writeGreen("File removed: " + remote.string());
            } else{
                m_view.writeRed("Error: unable to delete " + remote.string());
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
        if(host == m_model.config().getCurrentHost().first){
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
    m_view.writeWhite("The left file must be modified in order to update the remote file via FTP.");
    m_view.writeWhite("To add new host, see file config.txt and update accordingly.");
    m_view.writeHelp();
}