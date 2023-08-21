#ifndef APPMODEL_HPP
#define APPMODEL_HPP

#include "PathMonitor.hpp"
#include "Configuration.hpp"
#include <SFML/Network.hpp>

class AppModel{
public:
    AppModel();
private:
    PathMonitor m_monitor;
    Configuration m_configuration;
    sf::Ftp m_ftp;
};


#endif