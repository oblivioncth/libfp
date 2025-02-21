// Unit Includes
#include "fp/settings/fp-extconfig.h"

// Json struct parsing implementation
QX_JSON_STRUCT_OUTSIDE_X(Fp::ExtConfig,
    QX_JSON_MEMBER_ALIASED(com_ruffle_enabled, "com.ruffle.enabled-all"),
    QX_JSON_MEMBER_ALIASED(com_ruffle_enabled_all, "com.ruffle.enabled")
)

namespace Fp
{

//===============================================================================================================
// ExtConfigReader
//===============================================================================================================

//-Constructor--------------------------------------------------------------------------------------------------------
//Public:
ExtConfigReader::ExtConfigReader(ExtConfig* targetConfig, std::shared_ptr<QFile> sourceJsonFile) :
    SettingsReader(targetConfig, sourceJsonFile, true)
{}

//-Instance Functions-------------------------------------------------------------------------------------------------
//Private:
Qx::JsonError ExtConfigReader::parseDocument(const QJsonDocument& configDoc)
{
    // Get derivation specific target
    ExtConfig* targetConfig = static_cast<ExtConfig*>(mTargetSettings);

    // Parse
    return Qx::parseJson(*targetConfig, configDoc);
}

}
