#include "AppCLIView.hpp"
#include <iostream>

AppCLIView::AppCLIView(const int& ar, char** av) : argc(ar), argv(av),
opt("Allowed options")
{
    opt.add_options()
    ("help", "produce help message")
    ("host", po::value<std::string>(), "set host which to connect")
    ;

#ifdef _WIN32
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
}

void AppCLIView::show(AppCLIController& controller)
{
    std::vector<std::string> simulated_args = {"--host", "devcoo5"};

    po::variables_map vm;
    po::parsed_options parsed = po::command_line_parser(simulated_args).options(opt).allow_unregistered().run();
    po::store(parsed, vm);
    //po::store(po::parse_command_line(argc, argv, opt), vm);
    po::notify(vm);    

    checkForUnrecognizedOptions(parsed);

    if (vm.count("help")) {
        std::cout << opt << "\n";
    }

    if (vm.count("host")) {
        std::cout << "host was set to " << vm["host"].as<std::string>() << ".\n";
    } else {
        std::cout << "host was not set.\n";
    }
}

void AppCLIView::checkForUnrecognizedOptions(po::parsed_options& parsed)
{
    std::vector<std::string> unrecognized = po::collect_unrecognized(parsed.options, po::include_positional);
    
    if (!unrecognized.empty()) {
        setRedColor();
        std::cout << "Unrecognized options:\n";
        for (const std::string& option : unrecognized) {
            std::cout << option << "\n";
        }
        setWhiteColor();
    }
}

void AppCLIView::setGreenColor()
{
#ifdef _WIN32
    SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN);
#endif
}

void AppCLIView::setRedColor()
{
#ifdef _WIN32
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED);
#endif
}

void AppCLIView::setWhiteColor()
{
#ifdef _WIN32
    SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#endif
}