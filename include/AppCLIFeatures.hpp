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
    AppModel& m_model;
    AppCLIView& m_view;
    Features m_features;
public:
    AppCLIFeatures(AppModel& model, AppCLIView& view);
    void registerFeature(const std::string& name, const FeatureCallback& callback, const int& index = -1);
    inline const Features& getFeatures() const { return m_features; }

    static int EXIT_OPTION()                {return 0;}
    static int LISTS_FILE()                 {return 1;}
    static int TRANSFER_FILES()             {return 2;}
    static int CHANGE_HOST()                {return 3;}
    static int RESTART()                    {return 4;}
    static int TLOG()                       {return 5;}
    static int SCRIPT()                     {return 6;}
    static int BRANCH_CHANGES()             {return 7;}
    static int HELP()                       {return 8;}
private:
    void listChangedFiles(AppCLIController& controller);
    void transferChangedFiles(AppCLIController& controller);
    void changeHost(AppCLIController& controller);
    void help(AppCLIController& controller);
    void restart(AppCLIController& controller);
    void tlog(AppCLIController& controller);
    void script(AppCLIController& controller);
    void transferBranchFiles(AppCLIController& controller);
    void pressEnter(AppCLIController& controller);

    // HELPER
    void transferFiles(AppCLIController& controller);
};

#endif