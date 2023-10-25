#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP

#include <string>
#include <map>
#include <unordered_map>
#include <filesystem>

/**
 * @enum ConfigKey
 * @brief Represents the keys used for configuration settings.
 * 
 * This enumeration lists all the possible configuration keys
 * used throughout the application. These keys are used to retrieve
 * or set specific configuration values within the Configuration class.
 */
enum class ConfigKey {
    DefaultHost,     ///< Default Host
    LocalPath,       ///< Path to local sources which will be monitored for changes
    Difftool,        ///< Path to difftool used for comparing file differences
    DifftoolSide,    ///< Specify file which needs to be edited, LEFT or RIGHT (default is LEFT)
    None
};

/**
 * @enum HostConfig
 * @brief Represents the keys used for host settings.
 * 
 * This enumeration lists all the possible configuration keys
 * used throughout the application for one host configuration.
 */
enum class HostConfig {
    Alias,          ///< Alias of the host
    HostName,       ///< Name of host which to connect
    RemotePath,     ///< Path to remote sources which will be updated
    Username,       ///< Username to log in to FTP server
    Password,       ///< Password to log in to FTP server
    Port,           ///< Port which will be used for script execution (telnet or ssh)
    Script,         ///< Initial script to be executed in order to initialize environment
    None
};

/**
 * @struct HostData
 * @brief Holds all information regarding connecting to specific host
 */
struct HostData{
    std::string m_alias;
    std::string m_hostname;
    std::string m_remotePath;
    std::string m_username;
    std::string m_password;
    std::string m_port;
    std::string m_script;
    HostData(const std::string& alias = "", const std::string& hostname = "", const std::string& remotePath = "", const std::string& username = "",
             const std::string& password = "", const std::string& port = "", const std::string& script = "") :
             m_alias(alias), m_hostname(hostname), m_remotePath(remotePath), m_username(username), m_password(password),
             m_port(port), m_script(script) {}
};

/**
 * @class Configuration
 * 
 * @brief Manages the application's configuration settings.
 * 
 * This class is responsible for reading, writing, and managing
 * the application's various configuration settings. Configuration 
 * settings are identified by keys defined in the ConfigKey enum.
 * 
 * @see ConfigKey for a list of available configuration keys.
 */
class Configuration{
public:
    /**
     * @brief Constructor that initializes the m_configFile attribute and calls the readFile method.
     * 
     * @param path Filepath to settings file, if no path is specified the file 'config.txt' with default
     * values will be created on destruction of object.
     */
    Configuration(const std::string& path = "config.txt");
    ~Configuration();

    /**
     * @brief Reads configuration data from a specified file.
     * 
     * This method processes the file line-by-line, attempting to extract a key 
     * followed by a value from each line. If a key is found in the map `m_configData`,
     * its corresponding value is updated with the value from the file.
     * 
     * @note The file should be formatted such that each line consists of a key 
     * followed by a value, separated by whitespace. Lines that don't adhere to this 
     * format will be considered malformed and result in an outout to std::cerr.
     * 
     * @return true if the data was readed successfully; false otherwise.
     */
    bool readFile();

    /**
     * @brief Saves the configuration data to a file.
     * 
     * This method writes each key-value pair from m_configData to the specified file,
     * with one key-value pair per line. If the file cannot be opened for writing, the
     * method will return false.
     * 
     * @return true if the data was saved successfully; false otherwise.
     */
    bool saveFile();

    bool setValue(const ConfigKey& key, const std::string& value);
    bool setHostValue(const std::string& host, const HostConfig& key, const std::string& value);
    std::string getValue(const ConfigKey& key) const;
    std::string getHostValue(const std::string& host, const HostConfig& key) const;
    std::list<std::string> getHosts() const;
    bool deleteHost(const std::string& host);
    bool addHost(const HostData& data);
    const HostData& getCurrentHost() const;
private:
    void setDefaultValues();
    static std::string keyToString(const ConfigKey& key);
    static ConfigKey stringToKey(const std::string& key);
    static std::string keyToString(const HostConfig& key);
    static HostConfig stringToHostKey(const std::string& key);

    std::map<ConfigKey, std::string> m_configData;
    std::unordered_map<std::string, HostData> m_hosts;
    const std::string m_configFile;
    bool m_save;
};

#endif