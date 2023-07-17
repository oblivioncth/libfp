// Unit Includes
#include "fp/settings/fp-services.h"

// Qx Includes
#include <qx/core/qx-json.h>

// Json struct parsing implementation

// Configure key generator for mappable types
namespace QxJson
{

template<>
QString keygen<QString, Fp::ServerDaemon>(const Fp::ServerDaemon& value)
{
    return value.name;
};

}

// Json struct parsing implementation
QX_JSON_STRUCT_OUTSIDE(Fp::ServerDaemon,
    name,
    path,
    filename,
    arguments,
    kill
);

QX_JSON_STRUCT_OUTSIDE(Fp::StartStop,
    path,
    filename,
    arguments
);

QX_JSON_STRUCT_OUTSIDE(Fp::Services,
    server,
    daemon,
    start,
    stop,
);

namespace Fp
{
//===============================================================================================================
// StartStop
//===============================================================================================================

//-Operators----------------------------------------------------------------------------------------------------
//Public:
bool operator== (const StartStop& lhs, const StartStop& rhs) noexcept
{
    return lhs.path == rhs.path && lhs.filename == rhs.filename && lhs.arguments == rhs.arguments;
}

//-Hashing------------------------------------------------------------------------------------------------------
size_t qHash(const StartStop& key, size_t seed) noexcept
{
    QtPrivate::QHashCombine hash;
    seed = hash(seed, key.path);
    seed = hash(seed, key.filename);
    seed = hash(seed, key.arguments);

    return seed;
}

//===============================================================================================================
// ServicesReader
//===============================================================================================================

//-Constructor--------------------------------------------------------------------------------------------------------
//Public:
ServicesReader::ServicesReader(Services* targetServices, std::shared_ptr<QFile> sourceJsonFile, const MacroResolver* macroResolver) :
    SettingsReader(targetServices, sourceJsonFile),
    mHostMacroResolver(macroResolver)
{}

//-Instance Functions-------------------------------------------------------------------------------------------------
//Private:
Qx::JsonError ServicesReader::parseDocument(const QJsonDocument& servicesDoc)
{
    // Get derivation specific target
    Services* targetServices = static_cast<Services*>(mTargetSettings);

    // Parse
    Qx::JsonError err = Qx::parseJson(*targetServices, servicesDoc);
    if(err.isValid())
        return err;

    // Check for known daemons
    for(const ServerDaemon& d : qAsConst(targetServices->daemon))
    {
        /* NOTE: If for some reason this list becomes large, use a hash instead
         * (e.g. if(hash.contains("NAME")){ recognizedDaemons.setFlag(hash["NAME]); } )
         */
        if(d.name.contains("qemu", Qt::CaseInsensitive) ||
            d.filename.contains("qemu", Qt::CaseInsensitive))
            targetServices->recognizedDaemons.setFlag(KnownDaemon::Qemu);
        else if(d.name.contains("docker", Qt::CaseInsensitive) ||
                 d.filename.contains("docker", Qt::CaseInsensitive))
            targetServices->recognizedDaemons.setFlag(KnownDaemon::Docker);
    }

    return Qx::JsonError();
}

}
