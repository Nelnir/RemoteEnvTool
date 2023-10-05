#include "PathMonitor.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <cstdio>

bool GitMonitoringStrategy::check(const std::filesystem::path& path) 
{
    m_added.clear();
    m_removed.clear();
    m_updated.clear();

    // Redirect the output of the git command to a temporary file
    std::string tmpFile = "temp_output.txt";
    std::string cmd = "git -C " + path.string() + " status --porcelain > " + tmpFile;
    
    if (system(cmd.c_str()) != 0) {
        std::cerr << "Failed to execute git command." << std::endl;
        return false;
    }

    std::ifstream file(tmpFile);
    if (!file.is_open()) {
        std::cerr << "Failed to open temporary file." << std::endl;
        return false;
    }

    std::string line, status;
    std::filesystem::path filePath;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        ss >> status >> filePath;

        if (status == "A")
            m_added.push_back(filePath);
        else if (status == "D")
            m_removed.push_back(filePath);
        else if (status == "M")
            m_updated.push_back(filePath);
    }

    file.close();
    std::remove(tmpFile.c_str());  // Delete the temporary file

    if (m_added.empty() && m_removed.empty() && m_updated.empty()) 
        return false;
    return true;
}

bool GitBranchChangesStrategy::check(const std::filesystem::path& path) 
{
    m_added.clear();
    m_removed.clear();
    m_updated.clear();

    // Redirect the output of the git command to a temporary file
    std::string tmpFile = "temp_output.txt";
    std::string cmd = "git -C " + path.string() + " diff --name-status " + m_compareWith + "..HEAD > " + tmpFile;
    
    if (system(cmd.c_str()) != 0) {
        std::cerr << "Failed to execute git command." << std::endl;
        return false;
    }

    std::ifstream file(tmpFile);
    if (!file.is_open()) {
        std::cerr << "Failed to open temporary file." << std::endl;
        return false;
    }

    std::string line, status;
    std::filesystem::path filePath;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        ss >> status >> filePath;

        if (status == "A")
            m_added.push_back(filePath);
        else if (status == "D")
            m_removed.push_back(filePath);
        else if (status == "M")
            m_updated.push_back(filePath);
    }

    file.close();
    std::remove(tmpFile.c_str());  // Delete the temporary file

    if (m_added.empty() && m_removed.empty() && m_updated.empty()) 
        return false;
    return true;
}

std::string GitBranchChangesStrategy::getCurrentBranch(const std::filesystem::path& path)
{
    // Redirect the output of the git command to a temporary file
    std::string tmpFile = "temp_output.txt";
    std::string cmd = "git -C " + path.string() + " rev-parse --abbrev-ref HEAD > " + tmpFile;
    
    if (system(cmd.c_str()) != 0) {
        std::cerr << "Failed to execute git command." << std::endl;
        return false;
    }

    std::ifstream file(tmpFile);
    if (!file.is_open()) {
        std::cerr << "Failed to open temporary file." << std::endl;
        return false;
    }

    std::string line;
    std::getline(file, line);
    file.close();
    std::remove(tmpFile.c_str());  // Delete the temporary file
    return line;
}


PathMonitor::PathMonitor(const std::filesystem::path& path, std::unique_ptr<MonitoringStrategy> strategy) :
m_path(path),
m_strategy(std::move(strategy))
{

}