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
SettingsReader::SettingsReader(Settings* targetSettings, std::shared_ptr<QFile> sourceJsonFile, bool optional) :
    mTargetSettings(targetSettings),
    mSourceJsonFile(sourceJsonFile),
    mOptional(optional)
{}

//-Instance Functions-------------------------------------------------------------------------------------------------
//Public:
Qx::Error SettingsReader::readInto()
{
    // Load original JSON file
    QByteArray settingsData;
    Qx::IoOpReport settingsLoadReport = Qx::readBytesFromFile(settingsData, *mSourceJsonFile);

    if(settingsLoadReport.isFailure())
    {
        if(settingsLoadReport.result() == Qx::IO_ERR_DNE && mOptional)
            return Qx::IoOpReport(Qx::IO_OP_INSPECT, Qx::IO_SUCCESS, *mSourceJsonFile);
        return settingsLoadReport;
    }

    // Parse original JSON data
    QJsonParseError parseError;
    QJsonDocument settingsDocument = QJsonDocument::fromJson(settingsData, &parseError);

    if(settingsDocument.isNull())
        return Qx::Error(parseError).setSeverity(Qx::Critical);
    else
        return parseDocument(settingsDocument).withContext(QxJson::File(*mSourceJsonFile));
}

}
