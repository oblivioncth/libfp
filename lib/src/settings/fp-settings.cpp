// Unit Includes
#include "fp/settings/fp-settings.h"

// Qt Includes
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

// Qx Includes
#include <qx/core/qx-json.h>
#include <qx/io/qx-common-io.h>

namespace Fp
{

//===============================================================================================================
// SettingsReader
//===============================================================================================================

//-Constructor--------------------------------------------------------------------------------------------------------
//Public:
SettingsReader::SettingsReader(Settings* targetSettings, std::shared_ptr<QFile> sourceJsonFile) :
    mTargetSettings(targetSettings),
    mSourceJsonFile(sourceJsonFile)
{}

//-Class Functions-------------------------------------------------------------------------------------------------
//Protected:
Qx::GenericError SettingsReader::retrieveBasicKeys(QList<KeyValuePtr> keyValuePairs, const QJsonObject& obj)
{
    for(const KeyValuePtr& kvp : keyValuePairs)
    {       
        QStringView key = kvp.key;
        auto fn = [&obj, &key](const auto& vPtr)->Qx::GenericError{
            Q_ASSERT(vPtr);
            return Qx::Json::checkedKeyRetrieval(*vPtr, obj, key);
        };
        if(auto e = std::visit(fn, kvp.val); e.isValid())
            return e;
    }

    return Qx::GenericError();
}

//-Instance Functions-------------------------------------------------------------------------------------------------
//Public:
Qx::GenericError SettingsReader::readInto()
{
    // Load original JSON file
    QByteArray settingsData;
    Qx::IoOpReport settingsLoadReport = Qx::readBytesFromFile(settingsData, *mSourceJsonFile);

    if(settingsLoadReport.isFailure())
        return Qx::GenericError(Qx::GenericError::Critical, ERR_PARSING_JSON_DOC.arg(mSourceJsonFile->fileName()), settingsLoadReport.outcomeInfo());

    // Parse original JSON data
    QJsonParseError parseError;
    QJsonDocument settingsDocument = QJsonDocument::fromJson(settingsData, &parseError);

    if(settingsDocument.isNull())
        return Qx::GenericError(Qx::GenericError::Critical, ERR_PARSING_JSON_DOC.arg(mSourceJsonFile->fileName()), parseError.errorString());
    else
    {
        // Ensure top level container is object
        if(!settingsDocument.isObject())
            return Qx::GenericError(Qx::GenericError::Critical, ERR_PARSING_JSON_DOC.arg(mSourceJsonFile->fileName()), ERR_JSON_UNEXP_FORMAT);

        return parseDocument(settingsDocument);
    }
}

}
