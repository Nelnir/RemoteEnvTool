#include "AppCLIView.hpp"
#include <iostream>

AppCLIView::AppCLIView(AppModel& model, const int& ar, char** av) : m_model(model), argc(ar), argv(av),
opt("Allowed options"), m_features(model, *this)
{
    opt.add_options()
    ("help", "produce help message")
    ("host", po::value<std::string>(), "set host which to connect (if no value is passed, default will be used)")
    ("interactive", "enable interactive mode")
    ("list-file", "lists files changed")
    ("transfer", po::value<std::string>(), "send files to remote host\narg values: added, deleted, updated, all")
    ;

#ifdef _WIN32
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
}

int AppCLIView::show(AppCLIController& controller)
{
    std::vector<std::string> simulated_args = {"--transfer", "all"};
    try{
        po::variables_map vm;
        po::parsed_options parsed = po::command_line_parser(simulated_args).options(opt).run();
        po::store(parsed, vm);
        //po::store(po::parse_command_line(argc, argv, opt), vm);
        po::notify(vm);    

        if (vm.count("help")){
            std::cout << opt << "\n";
            return 0;
        }
        if(vm.count("interactive"))
            return interactive(controller);
        if(vm.count("list-file")){
            executeFeature(controller, AppCLIFeatures::LISTS_FILE());
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
            if(!m_model.connectToFtp()){
                writeRed("Error: unable to connect via FTP to " + m_model.config().getCurrentHost().first);
                return 1;
            }
            const auto& arg = vm["transfer"].as<std::string>();

            m_model.runPathMonitor();

            if(arg == "updated" || arg == "all"){
                for(const auto& file : m_model.monitor().filesUpdated()){
                    const auto& result = m_model.updateRemoteFile(file, true);
                    if(!result.first){
                        writeRed("Error: unable to update " + result.second);
                        return 1;
                    }
                }
            }

            if(arg == "added" || arg == "all"){
                for(const auto& file : m_model.monitor().filesAdded()){
                    const auto& result = m_model.uploadAddedFile(file);
                    if(!result.first){
                        writeRed("Error: unable to upload " + result.second);
                        return 1;
                    }
                }
            }
            if(arg == "deleted" || arg == "all"){
                for(const auto& file : m_model.monitor().filesDeleted()){
                    const auto& result = m_model.deleteRemoteFile(file);
                    if(!result.first){
                        writeRed("Error: unable to delete: " + result.second);
                        return 1;
                    }
                }
            }
            return 0;
        }
        

    } catch (const po::error& e) {
        writeRed("Error: " + std::string(e.what()));
        std::cout << opt << std::endl;
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
        executeFeature(controller, option);
    }
    return 0;
}

void AppCLIView::executeFeature(AppCLIController& controller, const int& option)
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

void AppCLIView::update()
{
#ifdef _WIN32
    system("cls");
#endif
    drawMenu();
}

void AppCLIView::drawMenu()
{
    writeGreen("Current host: " + m_model.config().getCurrentHost().first);

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