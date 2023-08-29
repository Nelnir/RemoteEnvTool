#ifndef APP_CLI_CONTROLLER_HPP
#define APP_CLI_CONTROLLER_HPP

#include "AppModel.hpp"

class AppCLIController{
public:
    AppCLIController(AppModel& model);
    int readMainMenuStep();
    bool yes();
private:
    std::string read();
    int asNumber(const std::string& input);
    AppModel& m_model;
};

#endif