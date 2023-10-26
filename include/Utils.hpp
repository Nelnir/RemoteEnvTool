#ifndef UTILS_HPP
#define UTILS_HPP

#include <chrono>
#include <sstream>
#include <iomanip>
#include <string>

namespace Utils{
std::string getCurrentDateTime();
std::string getPwd(const std::string& str);
std::string getSource(const std::string& str);
std::string getExecutablePath();
}

#endif