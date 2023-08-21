#ifndef APP_CLI_CONTROLLER_HPP
#define APP_CLI_CONTROLLER_HPP

#include "AppModel.hpp"

class AppCLIController{
public:
    AppCLIController(AppModel& model);
private:
    AppModel& m_model;
};

#endif