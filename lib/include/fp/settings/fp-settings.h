#ifndef FLASHPOINT_SETTINGS_H
#define FLASHPOINT_SETTINGS_H

// Shared Lib Support
#include "fp/fp_export.h"

// Qt Includes
#include <QString>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

// Qx Includes
#include <qx/core/qx-error.h>
#include <qx/core/qx-json.h>
#include <qx/utility/qx-concepts.h>

/* TODO: Make this a template to avoid need for static_cast for the derived type, and so traits can be used for "optional"
 * (the whole file) instead.
 */

namespace Fp
{

struct FP_FP_EXPORT Settings {};

class FP_FP_EXPORT SettingsReader
{
//-Instance Variables--------------------------------------------------------------------------------------------------
protected:
    Settings* mTargetSettings;
    std::shared_ptr<QFile> mSourceJsonFile;
    bool mOptional;

//-Constructor--------------------------------------------------------------------------------------------------------
public:
    SettingsReader(Settings* targetSettings, std::shared_ptr<QFile> sourceJsonFile, bool optional = false);

//-Instance Functions-------------------------------------------------------------------------------------------------
private:
    virtual Qx::JsonError parseDocument(const QJsonDocument& jsonDoc) = 0;

public:
    Qx::Error readInto();
};

}

#endif // FLASHPOINT_SETTINGS_H
