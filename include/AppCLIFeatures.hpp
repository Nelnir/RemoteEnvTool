#ifndef APP_CLI_FEATURES_HPP
#define APP_CLI_FEATURES_HPP

#include <map>
#include <utility>
#include <functional>
#include "AppCLIController.hpp"
#include "AppModel.hpp"

using FeatureCallback = std::function<void(AppCLIController&)>;
using Features = std::unordered_map<int, std::pair<std::string, FeatureCallback>>;

class AppCLIFeatures{
    const AppModel& m_model;
    Features m_features;
public:
    AppCLIFeatures(const AppModel& model);
    void registerFeature(const std::string& name, const FeatureCallback& callback, const int& index = -1);
    inline const Features& getFeatures() const { return m_features; }
    static int EXIT_OPTION() {return 0;}
private:
    void listChangedFiles(AppCLIController& controller);
};

#endif