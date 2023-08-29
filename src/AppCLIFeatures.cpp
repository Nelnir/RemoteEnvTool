#include "AppCLIFeatures.hpp"
#include "AppCLIView.hpp"
#include <iostream>

AppCLIFeatures::AppCLIFeatures(const AppModel& model, AppCLIView& view) : m_model(model), m_view(view)
{
    registerFeature("Exit", [this](const AppCLIController& controller) {}, EXIT_OPTION());
    registerFeature("List changed files", std::bind(&AppCLIFeatures::listChangedFiles, this, std::placeholders::_1), LISTS_FILE());
}

void AppCLIFeatures::registerFeature(const std::string& name, const FeatureCallback& callback, const int& index){
    if(index == -1){
        m_features.try_emplace(m_features.size(), std::make_pair(name, callback));
    } else{
        m_features.emplace(index, std::make_pair(name, callback));
    }
}

void AppCLIFeatures::listChangedFiles(AppCLIController& controller)
{
    const auto& updated = m_model.changedFiles(FileChangeType::Updated);
    if(!updated.empty())
        m_view.writeGreen("UPDATED:");
    for(const auto& file : updated){
        m_view.writeGreen(file);
    }

    const auto& added = m_model.changedFiles(FileChangeType::Added);
    if(!added.empty())
        m_view.writeGreen("ADDED:");
    for(const auto& file : added){
        m_view.writeGreen(file);
    }

    const auto& removed = m_model.changedFiles(FileChangeType::Removed);
    if(!removed.empty())
        m_view.writeRed("DELETED:");
    for(const auto& file : removed){
        m_view.writeRed(file);
    }
    if(updated.empty() && added.empty() && removed.empty()){
        m_view.writeWhite("No files changed.");
    }
}