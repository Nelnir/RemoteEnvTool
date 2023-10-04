#include "AppCLIFeatures.hpp"
#include "AppCLIView.hpp"
#include <thread>
#include <iostream>
#include "Utils.hpp"
#include "Windows.h"

AppCLIFeatures::AppCLIFeatures(AppModel& model, AppCLIView& view) : m_model(model), m_view(view)
{
    registerFeature("Exit", [this](const AppCLIController& controller) {}, EXIT_OPTION());
    registerFeature("List changed files", std::bind(&AppCLIFeatures::listChangedFiles, this, std::placeholders::_1), LISTS_FILE());
    registerFeature("Transfer files", std::bind(&AppCLIFeatures::transferFiles, this, std::placeholders::_1), TRANSFER_FILES());
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
    if(!m_model.m_monitor.check()){
        m_view.writeWhite("No files changed.");
        return;
    }

    if(!m_model.isConnectedToFtp()){
        m_view.writeWhite("Connecting to ftp server...");
        if(!m_model.connectToFtp(m_model.config().getCurrentHost())){
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
            m_model.updateRemoteFile(file, difftool);
        }
    }

    for(const auto& file : m_model.monitor().filesAdded()){
        m_view.writeWhite("Upload file (y/n): " + file.string());
        if(controller.yes()){
            m_model.uploadAddedFile(file);
        }
    }

    for(const auto& file : m_model.monitor().filesRemoved()){
        const auto& remote = m_model.getRemoteFileEquivalent(file);
        m_view.writeWhite("Delete remote file (y/n): " + remote.string());
        if(controller.yes()){
            m_model.deleteRemoteFile(file);
        }
    }
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
        if(m_model.isConnectedToFtp()){
            void(m_model.m_ftp.disconnect());
        }
        if(m_model.telnet().isConnected()){
            m_model.telnet().close();
        }
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
    if(arg.empty()) return;
    m_model.restart(arg);
    pressEnter(controller);
}

void AppCLIFeatures::tlog(AppCLIController& controller)
{
    m_view.writeWhite("Filename which to save to (empty for current date): ");
    std::string filename = controller.read();
    if(filename.empty()){
        filename = Utils::getCurrentDateTime() + ".txt";
    }
    auto result = m_model.tlog(filename);
    if(result.first){
        m_view.writeWhite("Open downloaded file with notepad++? (y/n)");
        if(controller.yes()){
            std::thread([result](){system(std::string("notepad++ " + result.second).c_str());}).detach();
        }
    }
    pressEnter(controller);
}

void AppCLIFeatures::script(AppCLIController& controller)
{
    m_view.writeWhite("Executing script ($in to go to base path)");
    auto text = m_model.telnet().pwd();
    m_view.writeWhite(text, false);
    std::string arg = controller.read();
    if(arg.empty()){
        pressEnter(controller);
        return;
    }
    if(arg == "$in"){
        m_model.telnet().executeCommand("cd " + m_model.telnet().home(), false, true);
        m_view.writeWhite(m_model.telnet().home(), false);
        arg = controller.read();
        if(arg.empty()) return;
    }
    m_model.script(arg);
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
    pressEnter(controller);
}