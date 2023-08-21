#ifndef APPMODEL_HPP
#define APPMODEL_HPP

#include "PathMonitor.hpp"
#include "Configuration.hpp"

class AppModel{
public:
    AppModel();
    void test();
private:
    PathMonitor m_monitor;
    Configuration m_configuration;
};


#endif