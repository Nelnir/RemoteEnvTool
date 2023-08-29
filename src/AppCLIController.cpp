#include "AppCLIController.hpp"
#include <iostream>
#include <cctype>

AppCLIController::AppCLIController(AppModel& model) : m_model(model)
{

}

int AppCLIController::readMainMenuStep()
{
    return asNumber(read());
}

bool AppCLIController::yes()
{
    const auto& ret = read();
    if(ret.empty())
        return false;
    return tolower(ret.front()) == 'y';
}

std::string AppCLIController::read()
{
    std::string input;
    std::cout << '>';
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