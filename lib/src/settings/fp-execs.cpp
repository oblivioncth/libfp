// Unit Includes
#include "fp/settings/fp-execs.h"

// Json struct parsing implementation
QX_JSON_STRUCT_OUTSIDE(Fp::Exec,
    linux,
    win32,
    wine
);

QX_JSON_STRUCT_OUTSIDE(Fp::Execs,
    list
);

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
