#include <iostream>
#include "telnetpp/telnetpp.hpp"
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/connect.hpp>
#include <boost/beast/core.hpp>
#include <boost/program_options.hpp>
#include "AppModel.hpp"

int main(int argc, char** argv)
{
    AppModel model;
    model.test();
    return 0;
}