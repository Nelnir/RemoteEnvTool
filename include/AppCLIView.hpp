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
    AppModel& m_model;
    const int& argc;
    char** argv;
    po::options_description opt;
    AppCLIFeatures m_features;
#ifdef _WIN32
    HANDLE hConsole;
#endif

public:
    AppCLIView(AppModel& model, const int& argc, char** argv);
    int show(AppCLIController& controller);
    int interactive(AppCLIController& controller);

    void update();

    void writeRed(const std::string& text);
    void writeGreen(const std::string& text);
    void writeWhite(const std::string& text);
    void writeHelp();
private:
    void drawMenu();
    void executeInteractiveFeature(AppCLIController& controller, const int& option);
    int executeTransferFeature(const po::variables_map& vm);
    int executeScriptFeature(const po::variables_map& vm);
    int executeRestartFeature(const po::variables_map& vm);
    int executeTlogFeature(const po::variables_map& vm);

    bool connectToTelnet(const HostData& host);
    bool connectToFTP(const HostData& host);
};

#endif