#ifndef APP_CLI_CONTROLLER_HPP
#define APP_CLI_CONTROLLER_HPP

#include "AppModel.hpp"

class AppCLIController{
public:
    AppCLIController(AppModel& model);
    int readMainMenuStep();
    bool yes();
    std::string read(const bool& showMark = false);
private:
    int asNumber(const std::string& input);
    AppModel& m_model;
};

#endif