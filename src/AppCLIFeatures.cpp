#include "AppCLIFeatures.hpp"
#include "AppCLIView.hpp"
#include <thread>
#include <iostream>
#include "Utils.hpp"
#include "Windows.h"
#include <conio.h>

AppCLIFeatures::AppCLIFeatures(AppModel& model, AppCLIView& view) : m_model(model), m_view(view)
{
    registerFeature("Exit", [this](const AppCLIController& controller) {}, EXIT_OPTION());
    registerFeature("List changed files", std::bind(&AppCLIFeatures::listChangedFiles, this, std::placeholders::_1), LISTS_FILE());
    registerFeature("Transfer files", std::bind(&AppCLIFeatures::transferChangedFiles, this, std::placeholders::_1), TRANSFER_FILES());
    registerFeature("Change host", std::bind(&AppCLIFeatures::changeHost, this, std::placeholders::_1), CHANGE_HOST());
    registerFeature("Restart", std::bind(&AppCLIFeatures::restart, this, std::placeholders::_1), RESTART());
    registerFeature("Tlog", std::bind(&AppCLIFeatures::tlog, this, std::placeholders::_1), TLOG());
    registerFeature("Script", std::bind(&AppCLIFeatures::script, this, std::placeholders::_1), SCRIPT());
    registerFeature("Branch-changes", std::bind(&AppCLIFeatures::transferBranchFiles, this, std::placeholders::_1), BRANCH_CHANGES());
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
    m_model.m_monitor.setStrategy(std::make_unique<GitMonitoringStrategy>());
    m_model.listChangedFiles();
    pressEnter(controller);
}

void AppCLIFeatures::transferChangedFiles(AppCLIController& controller)
{
    m_model.m_monitor.setStrategy(std::make_unique<GitMonitoringStrategy>());
    transferFiles(controller);
    pressEnter(controller);
}

void AppCLIFeatures::changeHost(AppCLIController& controller)
{
    m_view.writeWhite("Available hosts:");
    const auto& hosts = m_model.config().getHosts();
    for(const auto& host : hosts){
        if(host == m_model.config().getCurrentHost().m_alias){
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
    m_view.writeWhite("Available options (empty input to exit):");
    m_view.writeWhite("env - restarts whole environment");
    m_view.writeWhite("retux - restarts retux adapter");
    m_view.writeWhite("S-[serv-name] - restarts single server");
    m_view.writeWhite("G-[group-name] - restarts group of servers");
    std::string arg = controller.read();
    if(arg.empty()){
        pressEnter(controller);
        return;
    }
    m_model.restart(arg);
    pressEnter(controller);
}

void AppCLIFeatures::tlog(AppCLIController& controller)
{
    auto host = m_model.config().getCurrentHost();
    if(!m_model.telnet().isConnected()){
        if(!m_model.connectToTelnet(host)){
            return;
        }
    }
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
    if(!m_model.m_telnet.isConnected()){
        if(!m_model.connectToTelnet(m_model.config().getCurrentHost())){
            return;
        }
    }

    m_view.writeWhite("Continuous script execution\n@in to go to base path\n@exit to exit to main menu");
    m_view.writeWhite(m_model.telnet().pwd() + ">", false);
    m_model.telnet().showThreadOutput(true);
    std::string command;
    while(true){
        if (_kbhit()){
            char ch = _getch();
            command += ch;
            if(ch == 13){ // enter
                if(command.find("@in") != std::string::npos){
                    m_model.telnet().showThreadOutput(false);
                    m_model.telnet().send("\b\b\b");
                    m_model.telnet().cdHome();
                    std::this_thread::sleep_for(std::chrono::milliseconds(250));
                    m_model.telnet().showThreadOutput(true);
                    m_view.writeWhite("\n" + m_model.telnet().home() + '>', false);
                    command.clear();
                    continue;
                } else if(command.find("@exit") != std::string::npos){
                    m_model.telnet().send("\b\b\b\b\b");
                    m_view.writeWhite("");
                    break;
                }
                command.clear();
            }
            m_model.telnet().send(std::string(1, ch));
        } else{
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    m_model.telnet().showThreadOutput(false);
    pressEnter(controller);
}

void AppCLIFeatures::transferBranchFiles(AppCLIController& controller)
{
    auto branchStrategy = std::make_unique<GitBranchChangesStrategy>();
    auto currentBranch = branchStrategy->getCurrentBranch(m_model.m_monitor.getPath());
    auto index = currentBranch.find("DEV");
    auto compareBranch = currentBranch.substr(0, index + 3);
    m_view.writeWhite("Compare files from " + currentBranch + " with branch (empty for " + compareBranch+ "): ");
    std::string branch;
    branch = controller.read();
    if(branch.empty()){
        branch = compareBranch;
    }
    branchStrategy->compareWith(branch);
    m_model.m_monitor.setStrategy(std::move(branchStrategy));
    if(m_model.listChangedFiles()){
        m_view.writeWhite("Proceed to transfer files via FTP? (y/n):");
        if(controller.yes()){
            transferFiles(controller);
        }
    }
    pressEnter(controller);
}

void AppCLIFeatures::transferFiles(AppCLIController& controller)
{
    if(!m_model.m_monitor.check()){
        m_view.writeWhite("No files changed.");
        return;
    }

    auto host = m_model.config().getCurrentHost();

    if(!m_model.isConnectedToFtp()){
        m_view.writeWhite("Connecting to ftp server...");
        if(!m_model.connectToFtp(host)){
            return;
        }
    }

    if(host.m_remotePath.empty() && m_model.m_telnet.source().empty()){
        m_view.writeWhite("[REMOTE_PATH] is not set, retrieving source path from telnet...");
        if(!m_model.m_telnet.isConnected()){
            if(!m_model.connectToTelnet(host)){
                return;
            }
        }
        if(m_model.m_telnet.source().empty()){
            m_view.writeRed("This host doesn't support transferring files, please set REMOTE_PATH in config file.");
            return;
        } else{
            m_view.writeGreen("Found path: " + m_model.m_telnet.source());
        }
    }

    for(const auto& file : m_model.m_monitor.filesUpdated()){
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

    for(const auto& file : m_model.m_monitor.filesAdded()){
        m_view.writeWhite("Upload file (y/n): " + file.string());
        if(controller.yes()){
            m_model.uploadAddedFile(file);
        }
    }

    for(const auto& file : m_model.m_monitor.filesRemoved()){
        const auto& remote = m_model.getRemoteFileEquivalent(file);
        m_view.writeWhite("Delete remote file (y/n): " + remote.string());
        if(controller.yes()){
            m_model.deleteRemoteFile(file);
        }
    }
}

void AppCLIFeatures::pressEnter(AppCLIController& controller)
{
    m_view.writeWhite("Press enter to continue...");
    controller.read();
    m_view.restart();
}

void AppCLIFeatures::help(AppCLIController& controller)
{
    std::string side;
    if(m_model.m_configuration.getValue(ConfigKey::DifftoolSide) == "RIGHT"){
        side = "right";
    } else{
        side = "left";
    }
    m_view.writeWhite("The " + side + " file must be modified in order to update the remote file via FTP.");
    m_view.writeWhite("To add new host, see file config.txt and update accordingly.");
    m_view.writeHelp();
    pressEnter(controller);
}