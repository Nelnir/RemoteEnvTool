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
    std::string m_accumulatedData;
public:
    TelnetClient();
    ~TelnetClient();
    bool connect(const sf::IpAddress& ip);
    bool login(const std::string& username, const std::string& password);
    std::future<std::string> executeCommand(const std::string& command);
    bool write(const uint8_t*, const size_t& size);
    bool write(const std::string& text);
    bool is_alive();
    void close();
    void registerCallback(const std::string& trigger, const std::function<void()>& func){
        m_callbacks[trigger] = func;
    }
private:
    void handleReadThread();
    void handleOption(const uint8_t& command, const uint8_t& option);
    std::unordered_map<std::string, std::function<void()>> m_callbacks;
};

#endif