#ifndef APP_CLI_VIEW_HPP
#define APP_CLI_VIEW_HPP

#include "AppCLIController.hpp"

class AppCLIView{
public:
    AppCLIView(int argc, char** argv);
    void show(AppCLIController& controller);
};

#endif