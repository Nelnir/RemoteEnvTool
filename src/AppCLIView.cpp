#include "AppCLIView.hpp"
#include <iostream>
#include "Utils.hpp"

AppCLIView::AppCLIView(AppModel& model, const int& ar, char** av) : m_model(model), argc(ar), argv(av),
opt("Allowed options"), m_features(model, *this)
{
    opt.add_options()
    ("help", "produce help message")
    ("host", po::value<std::string>(), "set host which to connect (if no --host is passed, default value will be used)")
    ("interactive", "enable interactive mode")
    ("list-file", "lists files changed")
    ("transfer", po::value<std::string>(), "send files to remote host\narg values: added, deleted, updated, all")
    ("script", po::value<std::string>(), "execute telnet script\narg values: script name to be executed")
    ("restart", po::value<std::string>(), "restarts specified object\narg values: env (whole domain), retux (adapter), SERV-NAME (single server)")
    ("tlog", po::value<std::string>()->implicit_value(""), "starts writing log to a file\narg values: filename which to save (if no value is passed, current date will be used)")
    ;

#ifdef _WIN32
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
}

int AppCLIView::show(AppCLIController& controller)
{
    std::vector<std::string> simulated_args = {"--interactive"};
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
            return !m_model.transfer(vm["transfer"].as<std::string>());
        }

        if(vm.count("script")){
            return !m_model.script(". " + vm["script"].as<std::string>());
        }

        if(vm.count("restart")){
            return !m_model.restart(vm["restart"].as<std::string>());
        }

        if(vm.count("tlog")){
            const auto& filename = vm["tlog"].as<std::string>();
            return !m_model.tlog(filename.empty() ? Utils::getCurrentDateTime() : filename);
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

void AppCLIView::restart()
{
#ifdef _WIN32
    system("cls");
#endif
    drawMenu();
}

void AppCLIView::update(const std::string& str)
{
    writeWhite(str);
}

void AppCLIView::updateGood(const std::string& str)
{
    writeGreen(str);
}

void AppCLIView::updateBad(const std::string& str)
{
    writeRed(str);
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