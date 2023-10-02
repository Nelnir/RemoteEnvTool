#include "AppCLIView.hpp"
#include <iostream>

AppCLIView::AppCLIView(AppModel& model, const int& ar, char** av) : m_model(model), argc(ar), argv(av),
opt("Allowed options"), m_features(model, *this)
{
    opt.add_options()
    ("help", "produce help message")
    ("host", po::value<std::string>(), "set host which to connect (if no --host is passed, default value will be used)")
    ("interactive", "enable interactive mode")
    ("list-file", "lists files changed")
    ("transfer", po::value<std::string>(), "send files to remote host\narg values: added, deleted, updated, all")
    ("script", po::value<std::string>(), "execute telnet script\narg values: build")
    ;

#ifdef _WIN32
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
}

int AppCLIView::show(AppCLIController& controller)
{
    std::vector<std::string> simulated_args = {"--script", "sprawy"};
    try{
        po::variables_map vm;
        po::store(po::command_line_parser(simulated_args).options(opt).run(), vm);
        //po::store(po::parse_command_line(argc, argv, opt), vm);
        po::notify(vm);    

        if (vm.count("help") /*|| argc == 1*/){
            writeHelp();
            return 0;
        }
        if(vm.count("interactive"))
            return interactive(controller);
        if(vm.count("list-file")){
            executeInteractiveFeature(controller, AppCLIFeatures::LISTS_FILE());
            return 0;
        }

        // if host specified, update default host
        if(vm.count("host")){
            const auto& host = vm["host"].as<std::string>();
            if(!m_model.config().setValue(ConfigKey::DefaultHost, host)){
                writeRed("Error: unable to change host to " + host);
                return 1;
            }
        }


        if(vm.count("transfer")){
            return executeTransferFeature(vm);
        }

        if(vm.count("script")){
            return executeScriptFeature(vm);
        }
        

    } catch (const po::error& e) {
        writeRed("Error: " + std::string(e.what()));
        writeHelp();
        return 1;
    }
    return 0;
}

int AppCLIView::interactive(AppCLIController& controller)
{
    writeGreen("INTERACTIVE MODE");
    drawMenu();
    while(true){
        int option = controller.readMainMenuStep();
        if(option == AppCLIFeatures::EXIT_OPTION()){
            break;
        }
        executeInteractiveFeature(controller, option);
    }
    return 0;
}

void AppCLIView::executeInteractiveFeature(AppCLIController& controller, const int& option)
{
    const auto& features = m_features.getFeatures();
    auto itr = features.find(option);

    if(itr == features.end()){
        writeRed("Unsupported option");
        return;
    }

    FeatureCallback callback = itr->second.second;
    callback(controller);
}

int AppCLIView::executeTransferFeature(const po::variables_map& vm)
{
    if(!m_model.connectToFtp(m_model.config().getCurrentHost())){
        writeRed("Error: unable to connect via FTP to " + m_model.config().getCurrentHost().m_hostname);
        return 1;
    }
    const auto& arg = vm["transfer"].as<std::string>();

    m_model.runPathMonitor();

    if(arg == "updated" || arg == "all"){
        for(const auto& file : m_model.monitor().filesUpdated()){
            const auto& result = m_model.updateRemoteFile(file, true);
            if(!result.first){
                writeRed("Error: unable to update (left file must be changed)" + result.second);
                return 1;
            } else{
                writeGreen("Updated: " + result.second);
            }
        }
    }

    if(arg == "added" || arg == "all"){
        for(const auto& file : m_model.monitor().filesAdded()){
            const auto& result = m_model.uploadAddedFile(file);
            if(!result.first){
                writeRed("Error: unable to upload " + result.second);
                return 1;
            } else{
                writeGreen("Uploaded: " + result.second);
            }
        }
    }
    if(arg == "deleted" || arg == "all"){
        for(const auto& file : m_model.monitor().filesDeleted()){
            const auto& result = m_model.deleteRemoteFile(file);
            if(!result.first){
                writeRed("Error: unable to delete: " + result.second);
                return 1;
            } else{
                writeGreen("Deleted: " + result.second);
            }
        }
    }
    return 0;
}


int AppCLIView::executeScriptFeature(const po::variables_map& vm)
{
    auto host = m_model.config().getCurrentHost();
    if(!m_model.connectToTelnet(host)){
        writeRed("Error: unable to connect via telnet to " + host.m_hostname);
        return 1;
    }
    if(!m_model.telnet().executeInitialScript(host.m_script)){
        writeRed("Error: when executing initial script: " + host.m_script);
        return 1;
    }

    writeGreen("Connected via telnet to: " + host.m_hostname);

    const auto& arg = vm["script"].as<std::string>();

    writeWhite("Executing script: " + arg);
    m_model.telnet().executeCommand(". " + arg, true);
    return 0;
}

void AppCLIView::update()
{
#ifdef _WIN32
    system("cls");
#endif
    drawMenu();
}

void AppCLIView::drawMenu()
{
    writeGreen("Current host: " + m_model.config().getCurrentHost().m_hostname);

    for(const auto& feature : m_features.getFeatures()){
        std::cout << '[' << feature.first << "] - " << feature.second.first << std::endl;
    }
}

void AppCLIView::writeGreen(const std::string& text)
{
#ifdef _WIN32
    SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN);
#endif
    std::cout << text << std::endl;
#ifdef _WIN32
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#endif
}

void AppCLIView::writeRed(const std::string& text)
{
#ifdef _WIN32
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
#endif
    std::cerr << text << std::endl;
#ifdef _WIN32
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#endif
}

void AppCLIView::writeWhite(const std::string& text)
{
    std::cout << text << std::endl;
}

void AppCLIView::writeHelp()
{
    std::cout << opt << std::endl;
}