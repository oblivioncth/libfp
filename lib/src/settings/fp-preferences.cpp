// Unit Includes
#include "fp/settings/fp-preferences.h"

// Qx Includes
#include <qx/core/qx-json.h>

// Configure key generator for mappable types
namespace QxJson
{

template<>
QString keygen<QString, Fp::GameDataSource>(const Fp::GameDataSource& value)
{
    return value.name;
};

template<>
QString keygen<QString, Fp::GameMetadataSource>(const Fp::GameMetadataSource& value)
{
    return value.name;
};

}

// Json struct parsing implementation
QX_JSON_STRUCT_OUTSIDE(Fp::AppPathOverride,
    path,
    override,
    enabled
);

QX_JSON_STRUCT_OUTSIDE(Fp::GameDataSource,
    arguments,
    name,
    type
);

QX_JSON_STRUCT_OUTSIDE(Fp::GameMetadataSource,
    baseUrl,
    name
);

QX_JSON_STRUCT_OUTSIDE(Fp::Preferences,
    fpfssBaseUrl,
    gameDataSources,
    gameMetadataSources,
    imageFolderPath,
    logoFolderPath,
    playlistFolderPath,
    jsonFolderPath,
    htdocsFolderPath,
    dataPacksFolderPath,
    onDemandImages,
    onDemandImagesCompressed,
    onDemandBaseUrl,
    appPathOverrides,
    nativePlatforms,
    browserModeProxy,
    server
);

namespace Fp
{

//===============================================================================================================
// PreferencesReader
//===============================================================================================================

//-Constructor--------------------------------------------------------------------------------------------------------
//Public:
PreferencesReader::PreferencesReader(Preferences* targetPreferences, std::shared_ptr<QFile> sourceJsonFile) :
    SettingsReader(targetPreferences, sourceJsonFile)
{}

//-Instance Functions-------------------------------------------------------------------------------------------------
//Private:
Qx::JsonError PreferencesReader::parseDocument(const QJsonDocument& prefDoc)
{
    // Get derivation specific target
    Preferences* targetPreferences = static_cast<Preferences*>(mTargetSettings);

    // Parse
    return Qx::parseJson(*targetPreferences, prefDoc);
}

}
