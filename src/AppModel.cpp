#include "AppModel.hpp"
#include <iostream>

AppModel::AppModel()
{

}

std::string AppModel::currentHost() const
{
    return m_configuration.getHosts().front();
}

std::list<std::string> AppModel::changedFiles(const FileChangeType& type) const
{
    std::list<std::string> ret;
    switch(type){
        case FileChangeType::Added: ret = m_monitor.filesAddedd(); break;
        case FileChangeType::Removed: ret = m_monitor.filesDeleted(); break;
        case FileChangeType::Updated: ret = m_monitor.filesUpdated(); break;
    }
    return ret;
}