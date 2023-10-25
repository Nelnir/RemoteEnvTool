#include "Utils.hpp"
#include <iostream>
namespace Utils{

std::string getCurrentDateTime()
{
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_time), "%Y-%m-%d_%H-%M-%S");  // Format: YYYY-MM-DD_HH-MM-SS
    return ss.str();
}

std::string getPwd(const std::string& data)
{
    auto index = data.find_last_of('\n', data.find_last_of('\n'));
    if(index == std::string::npos) return "";
    auto beginning = data.find_first_of('/', index);
    if(beginning == std::string::npos) return "";
    auto str = data.substr(beginning, index - beginning);
    str.erase(std::remove_if(str.begin(), str.end(), ::isspace), str.end());
    str.erase(std::remove(str.begin(), str.end(), '>'), str.end());
    return str;
}

std::string getSource(const std::string& data)
{
    const std::string key = "zrodla: ";
    auto key_index = data.find(key);
    if(key_index == std::string::npos) return "";
    auto end_index = data.find('\r', key_index);
    if(end_index == std::string::npos) return "";
    return data.substr(key_index + key.length(), end_index - key_index - key.length());
}

}