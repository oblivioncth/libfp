// Unit Includes
#include "fp/settings/fp-execs.h"

// Qx Includes
#include <qx/core/qx-json.h>

using namespace Qt::Literals::StringLiterals;

namespace Fp
{

namespace Json
{
    namespace Object_Execs
    {
        const QString KEY_EXECS = u"execs"_s;
    };

    namespace Object_Exec
    {
        const QString KEY_WIN32 = u"win32"_s;
        const QString KEY_LINUX = u"linux"_s;
        const QString KEY_WINE = u"wine"_s;
    };
}

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
Qx::GenericError ExecsReader::parseDocument(const QJsonDocument& execsDoc)
{
    // Get derivation specific target
    Execs* targetExecs = static_cast<Execs*>(mTargetSettings);

    // Value error checking buffer
    QJsonObject rootObj = execsDoc.object();
    Qx::GenericError valueError;

    // Get exec entries
    QJsonArray jaExecs;
    if((valueError = Qx::Json::checkedKeyRetrieval(jaExecs, rootObj, Json::Object_Execs::KEY_EXECS)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    // Parse execs
    for(const QJsonValue& jvExec : qAsConst(jaExecs))
    {
        Exec execBuffer;
        if((valueError = parseExec(execBuffer, jvExec)).isValid())
            return valueError;

        targetExecs->list.append(execBuffer);
    }

    // Return invalid error on success
    return Qx::GenericError();
}

Qx::GenericError ExecsReader::parseExec(Exec& execBuffer, const QJsonValue& jvExec)
{
    // Ensure array element is Object
    if(!jvExec.isObject())
        return Qx::GenericError(Qx::GenericError::Critical, ERR_PARSING_JSON_DOC.arg(mSourceJsonFile->fileName()), ERR_JSON_UNEXP_FORMAT);

    // Get exec Object
    QJsonObject joExec = jvExec.toObject();

    // Value error checking buffer
    Qx::GenericError valueError;

    // Get direct values (ignore errors for optional values)
    if((valueError = Qx::Json::checkedKeyRetrieval(execBuffer.win32, joExec , Json::Object_Exec::KEY_WIN32)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    Qx::Json::checkedKeyRetrieval(execBuffer.linux, joExec, Json::Object_Exec::KEY_LINUX);
    Qx::Json::checkedKeyRetrieval(execBuffer.wine, joExec, Json::Object_Exec::KEY_WINE);


    // Return invalid error on success
    return Qx::GenericError();
}

}
