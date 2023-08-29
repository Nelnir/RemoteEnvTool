#ifndef APPMODEL_HPP
#define APPMODEL_HPP

#include "PathMonitor.hpp"
#include "Configuration.hpp"
#include <SFML/Network.hpp>

class Observer{
public:
    virtual void update() = 0;
};

class Subject{
public:
    void attach(Observer* obs) {m_observers.push_back(obs);}
    void detach(Observer* obs) {m_observers.remove(obs);}
    void notify(){
        std::for_each(m_observers.begin(), m_observers.end(), [&](Observer* obs) {obs->update();});
    }
private:
    std::list<Observer*> m_observers;
};

class AppModel : public Subject {
public:
    AppModel();
    std::string currentHost() const;
private:
    PathMonitor m_monitor;
    Configuration m_configuration;
    sf::Ftp m_ftp;
};


#endif