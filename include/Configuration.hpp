#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP

#include <string>
#include <map>

/**
 * @enum ConfigKey
 * @brief Represents the keys used for configuration settings.
 * 
 * This enumeration lists all the possible configuration keys
 * used throughout the application. These keys are used to retrieve
 * or set specific configuration values within the Configuration class.
 */
enum class ConfigKey {
    Host,            ///< Host which to connect to
    Port,            ///< Currently supported only telnet (23)
    Path,            ///< Path to sources which will be monitored for changes
    RemotePath,      ///< Path to remote sources which will be updated
    Difftool,        ///< Path to difftool used for comparing file differences
    None
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
    bool saveFile() const;

    bool setValue(const ConfigKey& key, const std::string& value);
    std::string getValue(const ConfigKey& key) const;
private:
    void setDefaultValues();
    std::string keyToString(const ConfigKey& key) const;
    ConfigKey stringToKey(const std::string& key);

    std::map<ConfigKey, std::string> m_configData;
    const std::string m_configFile;
};

#endif