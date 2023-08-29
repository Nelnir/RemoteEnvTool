#include "AppCLIController.hpp"
#include <iostream>

AppCLIController::AppCLIController(AppModel& model) : m_model(model)
{

}

int AppCLIController::readMainMenuStep()
{
    return asNumber(read());
}

std::string AppCLIController::read()
{
    std::string input;
    std::getline(std::cin, input);
    return input;
}

int AppCLIController::asNumber(const std::string& input)
{
    int number;
    try {
        number = std::stoi(input);
    } catch (const std::exception& e) {
        number = -1;
    } 
    return number;
}