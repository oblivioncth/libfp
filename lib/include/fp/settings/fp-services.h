#ifndef FLASHPOINT_SERVICES_H
#define FLASHPOINT_SERVICES_H

// Qt Includes
#include <QSet>

// Project Includes
#include "fp/settings/fp-settings.h"
#include "fp/fp-macro.h"

namespace Fp
{

enum KnownDaemon{
    None = 0x0,
    Docker = 0x1,
    Qemu = 0x2
};
Q_DECLARE_FLAGS(KnownDaemons, KnownDaemon);
Q_DECLARE_OPERATORS_FOR_FLAGS(KnownDaemons);

struct FP_FP_EXPORT ServerDaemon
{
    QString name;
    QString path;
    QString filename;
    QStringList arguments;
    bool kill;

    QX_JSON_STRUCT(
        name,
        path,
        filename,
        arguments,
        kill
    );
};

struct FP_FP_EXPORT StartStop
{
    QString path;
    QString filename;
    QStringList arguments;

    friend bool operator== (const StartStop& lhs, const StartStop& rhs) noexcept;
    friend size_t qHash(const StartStop& key, size_t seed) noexcept;

    QX_JSON_STRUCT(
        path,
        filename,
        arguments
    );
};

struct FP_FP_EXPORT Services : public Settings
{
    //QSet<Watch> watches;
    QHash<QString, ServerDaemon> servers;
    QHash<QString, ServerDaemon> daemons;
    QSet<StartStop> starts;
    QSet<StartStop> stops;
    KnownDaemons recognizedDaemons; // Non-standard
    // TODO: ^If Settings container obj is made (see other todo), move this there

    QX_JSON_STRUCT(
        servers,
        daemons,
        starts,
        stops,
    );
};

class FP_FP_EXPORT ServicesReader : public SettingsReader
{
//-Instance Variables--------------------------------------------------------------------------------------------------
private:
    const MacroResolver* mHostMacroResolver;

//-Constructor--------------------------------------------------------------------------------------------------------
public:
    ServicesReader(Services* targetServices, std::shared_ptr<QFile> sourceJsonFile, const MacroResolver* macroResolver);

//-Instance Functions-------------------------------------------------------------------------------------------------
private:
    Qx::JsonError parseDocument(const QJsonDocument& servicesDoc);
    Qx::JsonError parseServerDaemon(ServerDaemon& serverBuffer, const QJsonValue& jvServer);
    Qx::JsonError parseStartStop(StartStop& startStopBuffer, const QJsonValue& jvStartStop);
};

}

#endif // FLASHPOINT_SERVICES_H
