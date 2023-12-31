#ifndef TELNET_CLIENT_HPP
#define TELNET_CLIENT_HPP

#include <SFML/Network.hpp>
#include <future>
#include <thread>
#include <atomic>
#include <unordered_map>
#include <functional>
#include "Configuration.hpp"

class TelnetClient{
    sf::TcpSocket m_socket;
    unsigned char m_buffer[4096];
    std::thread m_readThread;
    std::atomic<bool> m_keepReading;
    std::atomic<bool> m_blockReading;
    std::string m_accumulatedData;
    std::string m_home;
    std::string m_pwd;
    std::string m_source;
    sf::Clock keepAliveClock;
    bool m_showThreadOutput;
public:
    TelnetClient();
    ~TelnetClient();
    bool connect(const sf::IpAddress& ip, const uint16_t& port = 23);
    bool login(const std::string& username, const std::string& password);
    std::future<std::string> executeCommand(const std::string& command, const bool& showResult = false, const bool& exitImmediately = false, const bool& outputNewLine = true);
    void showThreadOutput(const bool& val);
    bool send(const std::string& str);
    bool write(const uint8_t*, const size_t& size);
    bool write(const std::string& text);
    bool isConnected() const;
    void close();
    void registerCallback(const std::string& trigger, const std::function<void()>& func){
        m_callbacks[trigger] = func;
    }
    bool executeInitialScript(const std::string& script);
    const std::string& home() const {return m_home;}
    const std::string& pwd() const {return m_pwd;}
    const std::string& source() const {return m_source;}
    void cdHome();
private:
    void handleReadThread();
    void handleOption(const uint8_t& command, const uint8_t& option);
    std::unordered_map<std::string, std::function<void()>> m_callbacks;
};

#endif