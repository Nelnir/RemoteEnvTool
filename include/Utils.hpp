#ifndef UTILS_HPP
#define UTILS_HPP

#include <chrono>
#include <sstream>
#include <iomanip>
#include <string>

namespace Utils{

std::string getCurrentDateTime() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_time), "%Y-%m-%d_%H-%M-%S");  // Format: YYYY-MM-DD_HH-MM-SS
    return ss.str();
}

}

#endif