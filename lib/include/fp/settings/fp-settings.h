#ifndef FLASHPOINT_JSON_H
#define FLASHPOINT_JSON_H

// Shared Lib Support
#include "fp/fp_export.h"

// Qt Includes
#include <QString>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

// Qx Includes
#include <qx/core/qx-genericerror.h>
#include <qx/utility/qx-concepts.h>

namespace Fp
{

struct FP_FP_EXPORT Settings {};

using BasicJsonTypePtr = std::variant<bool*, double*, QString*>;

struct FP_FP_EXPORT KeyValuePtr
{
    QStringView key;
    BasicJsonTypePtr val;
};

class FP_FP_EXPORT SettingsReader
{
//-Class variables-----------------------------------------------------------------------------------------------------
public:
    static inline const QString ERR_PARSING_JSON_DOC = "Error parsing JSON Document: %1";
    static inline const QString ERR_JSON_UNEXP_FORMAT = "Unexpected document format";

//-Instance Variables--------------------------------------------------------------------------------------------------
protected:
    Settings* mTargetSettings;
    std::shared_ptr<QFile> mSourceJsonFile;

//-Constructor--------------------------------------------------------------------------------------------------------
public:
    SettingsReader(Settings* targetSettings, std::shared_ptr<QFile> sourceJsonFile);

//-Class Functions-------------------------------------------------------------------------------------------------
protected:
    static Qx::GenericError retrieveBasicKeys(QList<KeyValuePtr> keyValuePairs, const QJsonObject& obj); //TODO: Use this more, unless switching to improved Qx::Json

//-Instance Functions-------------------------------------------------------------------------------------------------
private:
    virtual Qx::GenericError parseDocument(const QJsonDocument& jsonDoc) = 0;

public:
    Qx::GenericError readInto();
};

}

#endif // FLASHPOINT_JSON_H
