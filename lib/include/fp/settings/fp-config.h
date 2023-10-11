#ifndef FLASHPOINT_CONFIG_H
#define FLASHPOINT_CONFIG_H

// Project Includes
#include "fp/settings/fp-settings.h"

namespace Fp
{

struct FP_FP_EXPORT Config : public Settings
{
    QString flashpointPath;
    bool startServer;
};

class FP_FP_EXPORT ConfigReader : public SettingsReader
{
//-Constructor--------------------------------------------------------------------------------------------------------
public:
    ConfigReader(Config* targetConfig, std::shared_ptr<QFile> sourceJsonFile);

//-Instance Functions-------------------------------------------------------------------------------------------------
private:
    Qx::JsonError parseDocument(const QJsonDocument& configDoc);
};

}

#endif // FLASHPOINT_CONFIG_H
