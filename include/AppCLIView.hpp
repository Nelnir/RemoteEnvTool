#ifndef APP_CLI_VIEW_HPP
#define APP_CLI_VIEW_HPP

#include "AppCLIController.hpp"
#include <boost/program_options.hpp>

#ifdef _WIN32
#include <windows.h>
#endif

namespace po = boost::program_options;

class AppCLIView{
public:
    AppCLIView(const int& argc, char** argv);
    void show(AppCLIController& controller);
private:
    void checkForUnrecognizedOptions(po::parsed_options& parsed);
    void setRedColor();
    void setGreenColor();
    void setWhiteColor();

    const int& argc;
    char** argv;

    po::options_description opt;

#ifdef _WIN32
HANDLE hConsole;
#endif
};

#endif