// Unit Includes
#include "fp/settings/fp-config.h"

// Qx Includes
#include <qx/core/qx-json.h>

using namespace Qt::Literals::StringLiterals;

namespace Fp
{

namespace Json
{
    namespace Object_Config
    {
        const QString KEY_FLASHPOINT_PATH = u"flashpointPath"_s; // Reading this value is current redundant and unused, but this may change in the future
        const QString KEY_START_SERVER = u"startServer"_s;
        const QString KEY_SERVER = u"server"_s;
    };
}

//===============================================================================================================
// ConfigReader
//===============================================================================================================

//-Constructor--------------------------------------------------------------------------------------------------------
//Public:
ConfigReader::ConfigReader(Config* targetConfig, std::shared_ptr<QFile> sourceJsonFile) :
    SettingsReader(targetConfig, sourceJsonFile)
{}

//-Instance Functions-------------------------------------------------------------------------------------------------
//Private:
Qx::GenericError ConfigReader::parseDocument(const QJsonDocument& configDoc)
{
    // Get derivation specific target
    Config* targetConfig = static_cast<Config*>(mTargetSettings);

    // Get values
    const QList<KeyValuePtr> kvps{
        {Json::Object_Config::KEY_FLASHPOINT_PATH, &targetConfig->flashpointPath},
        {Json::Object_Config::KEY_START_SERVER, &targetConfig->startServer},
        {Json::Object_Config::KEY_SERVER, &targetConfig->server},
    };

    return retrieveBasicKeys(kvps, configDoc.object());
}

}
