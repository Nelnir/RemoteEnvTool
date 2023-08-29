#include "AppModel.hpp"
#include <iostream>

AppModel::AppModel()
{

}

std::string AppModel::currentHost() const
{
    return m_configuration.getHosts().front();
}