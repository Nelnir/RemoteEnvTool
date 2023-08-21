#include "AppCLIView.hpp"
#include <boost/program_options.hpp>
#include <iostream>

namespace po = boost::program_options;

AppCLIView::AppCLIView(int argc, char** argv)
{

}

void AppCLIView::show(AppCLIController& controller)
{
    std::cout << "CLI-VIEW\n";
}