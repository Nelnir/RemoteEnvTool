#ifndef APP_CLI_VIEW_HPP
#define APP_CLI_VIEW_HPP

#include "AppCLIController.hpp"
#include "AppCLIFeatures.hpp"
#include <boost/program_options.hpp>

#ifdef _WIN32
#include <windows.h>
#endif

namespace po = boost::program_options;

class AppCLIView : public Observer {
    const AppModel& m_model;
    const int& argc;
    char** argv;
    po::options_description opt;
    AppCLIFeatures m_features;
#ifdef _WIN32
    HANDLE hConsole;
#endif

public:
    AppCLIView(const AppModel& model, const int& argc, char** argv);
    int show(AppCLIController& controller);
    int interactive(AppCLIController& controller);
    void executeFeature(AppCLIController& controller, const int& option);

    void update();

    void writeRed(const std::string& text);
    void writeGreen(const std::string& text);
    void writeWhite(const std::string& text);
private:
    void drawMenu();
    void checkForUnrecognizedOptions(po::parsed_options& parsed);
};

#endif