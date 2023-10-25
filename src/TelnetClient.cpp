#include "TelnetClient.hpp"
#include <sstream>
#include <iostream>
#include "Utils.hpp"

class BlockReadingGuard {
public:
    BlockReadingGuard(std::atomic<bool>& blockReadingFlag) : m_blockReadingFlag(blockReadingFlag) {m_blockReadingFlag = true;}
    ~BlockReadingGuard() {m_blockReadingFlag = false;}
private:
    std::atomic<bool>& m_blockReadingFlag;
};

TelnetClient::TelnetClient()
{

}

TelnetClient::~TelnetClient()
{
    close();
}

bool TelnetClient::write(const uint8_t* data, const size_t& size)
{
    return m_socket.send(data, size) == sf::Socket::Status::Done;
}

bool TelnetClient::write(const std::string& text)
{
    return m_socket.send(text.c_str(), text.size()) == sf::Socket::Status::Done;
}

bool TelnetClient::isConnected() const
{
    return m_keepReading;
}

void TelnetClient::close()
{
    m_pwd = m_home = "";
    m_keepReading = false;
    m_blockReading = false;
    if (m_readThread.joinable()) {
        m_readThread.join();
    }
    m_socket.disconnect();
}

bool TelnetClient::connect(const sf::IpAddress& ip, const uint16_t& port)
{
    if(m_keepReading){
        return false;
    }
    m_accumulatedData.clear();
    if(m_socket.connect(ip, port, sf::milliseconds(250)) != sf::Socket::Status::Done){
        return false;
    }
    m_keepReading = true;
    m_readThread = std::thread(&TelnetClient::handleReadThread, this);
    return true;
}

bool TelnetClient::login(const std::string& username, const std::string& password)
{
    std::promise<bool> authPromise;
    std::future<bool> authFuture = authPromise.get_future();
    registerCallback("login:", [this, &username](){
        write(username);
    });
    registerCallback("Password:", [this, &password](){
        write(password);
    });
    registerCallback(">", [&authPromise](){
        authPromise.set_value(true);
    });

    if(authFuture.wait_for(std::chrono::seconds(10)) == std::future_status::ready) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        return authFuture.get();
    }
    return false;
}

std::future<std::string> TelnetClient::executeCommand(const std::string& command, const bool& showResult, const bool& exitImmediately)
{
    keepAliveClock.restart();
    return std::async(std::launch::async, [this, command, showResult, exitImmediately]() {
        BlockReadingGuard guard(m_blockReading);
        std::string fullCommand = command + "\n";
        std::string data;

        if(!isConnected()){
            return data;
        }
        
        if (m_socket.send(fullCommand.c_str(), fullCommand.size()) != sf::Socket::Status::Done) {
            throw std::runtime_error("Send failed");
        }

        if(exitImmediately){
            return data;
        }

        m_socket.setBlocking(false);
        auto startTime = std::chrono::steady_clock::now();
        bool building = false;
        while (true) {
            std::size_t received;
            auto status = m_socket.receive(m_buffer, sizeof(m_buffer), received);
            if (status == sf::Socket::Status::Done) {
                std::string chunk(reinterpret_cast<char*>(m_buffer), received);
                data += chunk;

                if (showResult) {
                    std::cout << chunk;
                }

                // sometimes after some script executing we get results with current path enclosed in < path >,
                // idk why is that, but the latter condition prevents early leaving
                if (chunk.find('>') != std::string::npos && chunk.find('<') == std::string::npos) {
                    m_pwd = Utils::getPwd(data);
                    if(m_source.empty()){
                        m_source = Utils::getSource(data);
                    }
                    break;
                } // if we find string making target then we are building, increase exit time to 1 minute (set building to true)
                else if(chunk.find("making target")){
                    building = true;
                }

                if (received == 0) {
                    break;  // if no data is received, we can also exit
                }
                startTime = std::chrono::steady_clock::now();
            } else if(status == sf::Socket::Status::NotReady){
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                if ((std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime).count() >= 20000 && !building) ||
                    (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - startTime).count() >= 60000 && building)) {
                    break;
                }   
            } else {
                m_socket.setBlocking(true);
                throw std::runtime_error("Receive failed");
            }
        }
        if(showResult){
            std::cout << std::endl;
        }
        m_socket.setBlocking(true);
        return data;
    });
}

bool TelnetClient::executeInitialScript(const std::string& script)
{
    auto promise = executeCommand(". " + script);
    if(promise.wait_for(std::chrono::seconds(5)) == std::future_status::ready){
        m_home = m_pwd = Utils::getPwd(promise.get());
        return true;
    }
    return false;
}

void TelnetClient::handleReadThread()
{
    sf::SocketSelector selector;
    selector.add(m_socket);
    
    keepAliveClock.restart();
    while (m_keepReading) {
        if (selector.wait(sf::milliseconds(250))) {
            if (selector.isReady(m_socket) && !m_blockReading) {
                std::size_t received;
                auto status = m_socket.receive(m_buffer, sizeof(m_buffer), received);
                if (status == sf::Socket::Status::Done) {
                    std::stringstream textStream;

                    // handle commands
                    for (std::size_t i = 0; i < received; ++i) {
                        if (m_buffer[i] == 255) {
                            if (i + 1 < received && m_buffer[i + 1] == 255) {
                                // This is an escaped 255 byte, so treat it as regular data.
                                textStream << m_buffer[i];
                                i += 1; // Skip the next 255.
                            } else if (i + 2 < received) {
                                handleOption(m_buffer[i + 1], m_buffer[i + 2]);
                                i += 2; // Skip the two command bytes.
                            }
                        } else {
                            textStream << m_buffer[i];
                        }
                    }


                    std::string filteredStr = textStream.str();
                    if (!filteredStr.empty()) {
                        //std::cout << filteredStr << std::endl;
                    }

                    m_accumulatedData += filteredStr;

                    for(auto itr = m_callbacks.begin(); itr != m_callbacks.end(); ++itr){
                        auto callback = *itr;
                        if(m_accumulatedData.find(callback.first) != std::string::npos){
                            callback.second();
                            write(reinterpret_cast<const uint8_t*>("\n"), 1);
                            m_callbacks.erase(itr);
                            m_accumulatedData.clear();
                            break;
                        }
                    }
                } else if (status == sf::Socket::Status::Disconnected){
                    close();
                    return;
                }
            }
        }

        if(keepAliveClock.getElapsedTime().asSeconds() >= 300 && m_socket.isBlocking()){
            void(m_socket.send(" ", 1));
            m_accumulatedData.clear();
            keepAliveClock.restart();
        }
    }
}

void TelnetClient::cdHome()
{
    if(pwd() != home()){
        executeCommand("cd " + home(), false, true);
        m_pwd = m_home;
    }
}

void TelnetClient::handleOption(const uint8_t& command, const uint8_t& option)
{
    uint8_t response[] = {255, 252, option};
    write(response, 3);
}
