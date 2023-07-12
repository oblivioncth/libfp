// Unit Includes
#include "fp/settings/fp-config.h"

namespace Fp
{

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
Qx::JsonError ConfigReader::parseDocument(const QJsonDocument& configDoc)
{
    // Get derivation specific target
    Config* targetConfig = static_cast<Config*>(mTargetSettings);

    // Parse
    return Qx::parseJson(*targetConfig, configDoc);
}

}
