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
