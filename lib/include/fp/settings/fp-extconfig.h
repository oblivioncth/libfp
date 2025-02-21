#ifndef FLASHPOINT_EXTCONFIG_H
#define FLASHPOINT_EXTCONFIG_H

// Project Includes
#include "fp/settings/fp-settings.h"

namespace Fp
{

struct FP_FP_EXPORT ExtConfig : public Settings
{
    /* The defaults here matter because this file may not be present. Ideally, we'd take the defaults
     * from the launcher defaults themselves, but I believe they're hard coded.
     */
    bool com_ruffle_enabled = false;
    bool com_ruffle_enabled_all = false;
};

class FP_FP_EXPORT ExtConfigReader : public SettingsReader
{
//-Constructor--------------------------------------------------------------------------------------------------------
public:
    ExtConfigReader(ExtConfig* targetConfig, std::shared_ptr<QFile> sourceJsonFile);

//-Instance Functions-------------------------------------------------------------------------------------------------
private:
    Qx::JsonError parseDocument(const QJsonDocument& configDoc);
};

}

#endif // FLASHPOINT_EXTCONFIG_H
