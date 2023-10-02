#include "AppCLIFeatures.hpp"
#include "AppCLIView.hpp"
#include <thread>
#include <iostream>
#include "Utils.hpp"

AppCLIFeatures::AppCLIFeatures(AppModel& model, AppCLIView& view) : m_model(model), m_view(view)
{
    registerFeature("Exit", [this](const AppCLIController& controller) {}, EXIT_OPTION());
    registerFeature("List changed files", std::bind(&AppCLIFeatures::listChangedFiles, this, std::placeholders::_1), LISTS_FILE());
    registerFeature("Transfer files", std::bind(&AppCLIFeatures::transferFiles, this, std::placeholders::_1), TRANSFER_FILES());
    registerFeature("Reset file snapshot", std::bind(&AppCLIFeatures::resetSnapshot, this, std::placeholders::_1), RESET_SNAPSHOT());
    registerFeature("Change host", std::bind(&AppCLIFeatures::changeHost, this, std::placeholders::_1), CHANGE_HOST());
    registerFeature("Restart", std::bind(&AppCLIFeatures::restart, this, std::placeholders::_1), RESTART());
    registerFeature("Tlog", std::bind(&AppCLIFeatures::tlog, this, std::placeholders::_1), TLOG());
    registerFeature("Script", std::bind(&AppCLIFeatures::script, this, std::placeholders::_1), SCRIPT());
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
    m_model.listChangedFiles();
    pressEnter(controller);
}

void AppCLIFeatures::transferFiles(AppCLIController& controller)
{
    if(!m_model.m_monitor.check(false)){
        m_view.writeWhite("No files changed.");
        return;
    }

    if(!m_model.isConnectedToFtp()){
        m_view.writeWhite("Connecting to ftp server...");
        if(m_model.connectToFtp(m_model.config().getCurrentHost())){
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
    pressEnter(controller);
}

void AppCLIFeatures::resetSnapshot(AppCLIController& controller)
{
    m_model.m_monitor.check();
    m_view.writeWhite("Snapshot resetted.");
    pressEnter(controller);
}

void AppCLIFeatures::changeHost(AppCLIController& controller)
{
    m_view.writeWhite("Available hosts:");
    const auto& hosts = m_model.config().getHosts();
    for(const auto& host : hosts){
        if(host == m_model.config().getCurrentHost().m_hostname){
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
    pressEnter(controller);
}

void AppCLIFeatures::restart(AppCLIController& controller)
{
    m_view.writeWhite("Available options:");
    m_view.writeWhite("env - restarts whole environment");
    m_view.writeWhite("retux - restarts retux adapter");
    m_view.writeWhite("[serv-name] - restarts single server");
    std::string arg = controller.read();
    m_model.restart(arg);
    pressEnter(controller);
}

void AppCLIFeatures::tlog(AppCLIController& controller)
{
    m_view.writeWhite("Filename which to save to (empty for default): ");
    std::string filename = controller.read();
    if(filename.empty()){
        filename = Utils::getCurrentDateTime();
    }
    m_model.tlog(filename);
    pressEnter(controller);
}

void AppCLIFeatures::script(AppCLIController& controller)
{
    m_view.writeWhite("Enter script to execute: ");
    m_model.script(controller.read());
    pressEnter(controller);
}

void AppCLIFeatures::pressEnter(AppCLIController& controller)
{
    m_view.writeWhite("Press enter to continue...");
    controller.read();
    m_view.restart();
}

void AppCLIFeatures::help(AppCLIController& controller)
{
    m_view.writeWhite("The left file must be modified in order to update the remote file via FTP.");
    m_view.writeWhite("To add new host, see file config.txt and update accordingly.");
    m_view.writeHelp();
}