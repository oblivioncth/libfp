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

//-Instance Functions-------------------------------------------------------------------------------------------------
//Public:
Qx::Error SettingsReader::readInto()
{
    // Load original JSON file
    QByteArray settingsData;
    Qx::IoOpReport settingsLoadReport = Qx::readBytesFromFile(settingsData, *mSourceJsonFile);

    if(settingsLoadReport.isFailure())
        return settingsLoadReport;

    // Parse original JSON data
    QJsonParseError parseError;
    QJsonDocument settingsDocument = QJsonDocument::fromJson(settingsData, &parseError);

    if(settingsDocument.isNull())
        return Qx::Error(parseError).setSeverity(Qx::Critical);
    else
        return parseDocument(settingsDocument).withContext(QxJson::File(*mSourceJsonFile));
}

}
