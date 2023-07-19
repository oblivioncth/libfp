#ifndef FLASHPOINT_PREFERENCES_H
#define FLASHPOINT_PREFERENCES_H

// Qt Includes
#include <QSet>

// Project Includes
#include "fp/settings/fp-settings.h"

namespace Fp
{

struct FP_FP_EXPORT AppPathOverride
{
    QString path;
    QString override;
    bool enabled;
};

struct FP_FP_EXPORT GameDataSource
{
    QList<QString> arguments;
    QString name;
    QString type;
};

//struct FP_FP_EXPORT GameMetadataSource_GamesTags
//{
//    QDateTime actualUpdateTime;
//    QDateTime latestDeleteTime;
//    QDateTime latestUpdateTime;
//};

struct FP_FP_EXPORT GameMetadataSource
{
    QString baseUrl;
    //GameMetadataSource_GamesTags games;
    QString name;
    //GameMetadataSource_GamesTags tags;
};

struct FP_FP_EXPORT Preferences : public Settings
{
    QString fpfssBaseUrl;
    QHash<QString, GameDataSource> gameDataSources;
    QHash<QString, GameMetadataSource> gameMetadataSources;
    QString imageFolderPath;
    QString playlistFolderPath;
    QString jsonFolderPath;
    QString htdocsFolderPath;
    QString dataPacksFolderPath;
    bool onDemandImages;
    bool onDemandImagesCompressed;
    QString onDemandBaseUrl;
    QList<AppPathOverride> appPathOverrides;
    QSet<QString> nativePlatforms;
    QString browserModeProxy;
    QString server;
};

class FP_FP_EXPORT PreferencesReader : public SettingsReader
{
//-Constructor--------------------------------------------------------------------------------------------------------
public:
    PreferencesReader(Preferences* targetPreferences, std::shared_ptr<QFile> sourceJsonFile);

//-Instance Functions-------------------------------------------------------------------------------------------------
private:
    Qx::JsonError parseDocument(const QJsonDocument& prefDoc);
};

}

#endif // FLASHPOINT_PREFERENCES_H
