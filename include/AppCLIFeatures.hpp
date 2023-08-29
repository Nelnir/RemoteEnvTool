#ifndef APP_CLI_FEATURES_HPP
#define APP_CLI_FEATURES_HPP

#include <map>
#include <utility>
#include <functional>
#include "AppCLIController.hpp"
#include "AppModel.hpp"

using FeatureCallback = std::function<void(AppCLIController&)>;
using Features = std::unordered_map<int, std::pair<std::string, FeatureCallback>>;
using WriteCallback = std::function<void(const std::string&)>;

class AppCLIView;

class AppCLIFeatures{
    const AppModel& m_model;
    AppCLIView& m_view;
    Features m_features;
public:
    AppCLIFeatures(const AppModel& model, AppCLIView& view);
    void registerFeature(const std::string& name, const FeatureCallback& callback, const int& index = -1);
    inline const Features& getFeatures() const { return m_features; }
    static int EXIT_OPTION() {return 0;}
    static int LISTS_FILE() {return 1;}
private:
    void listChangedFiles(AppCLIController& controller);
};

#endif