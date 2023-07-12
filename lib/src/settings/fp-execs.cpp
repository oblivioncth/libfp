// Unit Includes
#include "fp/settings/fp-execs.h"

// Qx Includes
#include <qx/core/qx-json.h>

namespace Fp
{
//===============================================================================================================
// ExecsReader
//===============================================================================================================

//-Constructor--------------------------------------------------------------------------------------------------------
//Public:
ExecsReader::ExecsReader(Execs* targetExecs, std::shared_ptr<QFile> sourceJsonFile) :
    SettingsReader(targetExecs, sourceJsonFile)
{}

//-Instance Functions-------------------------------------------------------------------------------------------------
//Private:
Qx::JsonError ExecsReader::parseDocument(const QJsonDocument& execsDoc)
{
    // Get derivation specific target
    Execs* targetExecs = static_cast<Execs*>(mTargetSettings);

    // Parse
    return Qx::parseJson(*targetExecs, execsDoc);
}

}
