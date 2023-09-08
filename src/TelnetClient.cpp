#include "TelnetClient.hpp"
#include <sstream>
#include <iostream>

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

bool TelnetClient::is_alive()
{
    std::size_t received;
    return m_socket.receive(m_buffer, sizeof(m_buffer), received) != sf::Socket::Status::Disconnected;
}

void TelnetClient::close()
{
    m_keepReading = false;
    if (m_readThread.joinable()) {
        m_readThread.join();
    }
    m_socket.disconnect();
}

bool TelnetClient::connect(const sf::IpAddress& ip)
{
    m_accumulatedData.clear();
    if(m_socket.connect(ip, 23, sf::milliseconds(250)) != sf::Socket::Status::Done){
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
    registerCallback("success", [&authPromise](){
        authPromise.set_value(true);
    });

    if(authFuture.wait_for(std::chrono::seconds(3)) == std::future_status::ready) {
        return authFuture.get();
    }
    return false;
}

std::future<std::string> TelnetClient::executeCommand(const std::string& command)
{
    auto promise = std::make_shared<std::promise<std::string>>();
    std::thread([this, command, promise]() {
        std::string fullCommand = command + "\n";
        if (m_socket.send(fullCommand.c_str(), fullCommand.size()) != sf::Socket::Status::Done) {
            promise->set_exception(std::make_exception_ptr(std::runtime_error("Send failed")));
            return;
        }

        std::size_t received;
        if (m_socket.receive(m_buffer, sizeof(m_buffer), received) != sf::Socket::Status::Done) {
            promise->set_exception(std::make_exception_ptr(std::runtime_error("Receive failed")));
            return;
        }

        promise->set_value(std::string((char)m_buffer, received));
    }).detach();

    return promise->get_future();
}

void TelnetClient::handleReadThread()
{
    sf::SocketSelector selector;
    selector.add(m_socket);

    while (m_keepReading) {
        if (selector.wait(sf::seconds(1))) {
            if (selector.isReady(m_socket)) {
                std::size_t received;
                if (m_socket.receive(m_buffer, sizeof(m_buffer), received) == sf::Socket::Status::Done) {
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
                        std::cout << "RECEIVED TEXT: " << filteredStr << std::endl;
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
                }
            }
        }
    }
}

void TelnetClient::handleOption(const uint8_t& command, const uint8_t& option)
{
    uint8_t response[] = {255, 252, option};
    write(response, 3);
}