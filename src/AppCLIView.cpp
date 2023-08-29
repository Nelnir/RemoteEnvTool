#include "AppCLIView.hpp"
#include <iostream>

AppCLIView::AppCLIView(AppModel& model, const int& ar, char** av) : m_model(model), argc(ar), argv(av),
opt("Allowed options"), m_features(model, *this)
{
    opt.add_options()
    ("help", "produce help message")
    ("host", po::value<std::string>(), "set host which to connect")
    ("interactive", "enable interactive mode")
    ("list-file", "lists files changed")
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
        po::parsed_options parsed = po::command_line_parser(simulated_args).options(opt).allow_unregistered().run();
        po::store(parsed, vm);
        //po::store(po::parse_command_line(argc, argv, opt), vm);

        po::notify(vm);    

        checkForUnrecognizedOptions(parsed);

        if (vm.count("help")) 
            std::cout << opt << "\n";
        if(vm.count("interactive"))
            return interactive(controller);
        if(vm.count("list-file"))
            executeFeature(controller, AppCLIFeatures::LISTS_FILE());
        

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
    writeGreen("Current host: " + m_model.currentHost().first);

    for(const auto& feature : m_features.getFeatures()){
        std::cout << '[' << feature.first << "] - " << feature.second.first << std::endl;
    }
}

void AppCLIView::checkForUnrecognizedOptions(po::parsed_options& parsed)
{
    std::vector<std::string> unrecognized = po::collect_unrecognized(parsed.options, po::include_positional);
    
    if (!unrecognized.empty()) {
        writeRed("Ignored options:");
        for (const std::string& option : unrecognized) {
            writeRed(option);
        }
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