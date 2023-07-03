// Unit Includes
#include "fp/settings/fp-services.h"

// Qx Includes
#include <qx/core/qx-json.h>

using namespace Qt::Literals::StringLiterals;

namespace Fp
{

namespace Json
{
    namespace Object_ServerDaemon
    {
        const QString KEY_NAME = u"name"_s;
        const QString KEY_PATH = u"path"_s;
        const QString KEY_FILENAME = u"filename"_s;
        const QString KEY_ARGUMENTS = u"arguments"_s;
        const QString KEY_KILL = u"kill"_s;
    };
    namespace Object_StartStop
    {
        const QString KEY_PATH = u"path"_s;
        const QString KEY_FILENAME = u"filename"_s;
        const QString KEY_ARGUMENTS = u"arguments"_s;
    };

    namespace Object_Services
    {
        const QString KEY_WATCH = u"watch"_s;
        const QString KEY_SERVER = u"server"_s;
        const QString KEY_DAEMON = u"daemon"_s;
        const QString KEY_START = u"start"_s;
        const QString KEY_STOP = u"stop"_s;
    };
}

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
Qx::GenericError ServicesReader::parseDocument(const QJsonDocument& servicesDoc)
{
    // Get derivation specific target
    Services* targetServices = static_cast<Services*>(mTargetSettings);

    // Value error checking buffer
    QJsonObject rootObj = servicesDoc.object();
    Qx::GenericError valueError;

    // Get watches
    // TODO: include logs

    // Get servers
    QJsonArray jaServers;
    if((valueError = Qx::Json::checkedKeyRetrieval(jaServers, rootObj, Json::Object_Services::KEY_SERVER)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    // Parse servers
    for(const QJsonValue& jvServer : qAsConst(jaServers))
    {
        ServerDaemon serverBuffer;
        if((valueError = parseServerDaemon(serverBuffer, jvServer)).isValid())
            return valueError;

        targetServices->servers.insert(serverBuffer.name, serverBuffer);
    }

    // Get daemons
    QJsonArray jaDaemons;
    if((valueError = Qx::Json::checkedKeyRetrieval(jaDaemons, rootObj, Json::Object_Services::KEY_DAEMON)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    // Parse daemons
    for(const QJsonValue& jvDaemon : qAsConst(jaDaemons))
    {
        ServerDaemon daemonBuffer;
        if((valueError = parseServerDaemon(daemonBuffer, jvDaemon)).isValid())
            return valueError;

        targetServices->daemons.insert(daemonBuffer.name, daemonBuffer);

        /* NOTE: If for some reason this list becomes large, use a hash instead
         * (e.g. if(hash.contains("NAME")){ recognizedDaemons.setFlag(hash["NAME]); } )
         */
        if(daemonBuffer.name.contains("qemu", Qt::CaseInsensitive) ||
           daemonBuffer.filename.contains("qemu", Qt::CaseInsensitive))
            targetServices->recognizedDaemons.setFlag(KnownDaemon::Qemu);
        else if(daemonBuffer.name.contains("docker", Qt::CaseInsensitive) ||
                daemonBuffer.filename.contains("docker", Qt::CaseInsensitive))
            targetServices->recognizedDaemons.setFlag(KnownDaemon::Docker);
    }

    // Get starts
    QJsonArray jaStarts;
    if((valueError = Qx::Json::checkedKeyRetrieval(jaStarts, rootObj, Json::Object_Services::KEY_START)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    // Parse starts
    for(const QJsonValue& jvStart : qAsConst(jaStarts))
    {
        StartStop startStopBuffer;
        if((valueError = parseStartStop(startStopBuffer, jvStart)).isValid())
            return valueError;

        targetServices->starts.insert(startStopBuffer);
    }

    // Get stops
    QJsonArray jaStops;
    if((valueError = Qx::Json::checkedKeyRetrieval(jaStops, rootObj, Json::Object_Services::KEY_STOP)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    // Parse starts
    for(const QJsonValue& jvStop : qAsConst(jaStops))
    {
        StartStop startStopBuffer;
        if((valueError = parseStartStop(startStopBuffer, jvStop)).isValid())
            return valueError;

        targetServices->stops.insert(startStopBuffer);
    }

    // Return invalid error on success
    return Qx::GenericError();
}

Qx::GenericError ServicesReader::parseServerDaemon(ServerDaemon& serverBuffer, const QJsonValue& jvServer)
{
    // Ensure array element is Object
    if(!jvServer.isObject())
        return Qx::GenericError(Qx::GenericError::Critical, ERR_PARSING_JSON_DOC.arg(mSourceJsonFile->fileName()), ERR_JSON_UNEXP_FORMAT);

    // Get server Object
    QJsonObject joServer = jvServer.toObject();

    // Value error checking buffer
    Qx::GenericError valueError;

    // Get direct values
    if((valueError = Qx::Json::checkedKeyRetrieval(serverBuffer.name, joServer, Json::Object_ServerDaemon::KEY_NAME)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    if((valueError = Qx::Json::checkedKeyRetrieval(serverBuffer.path, joServer, Json::Object_ServerDaemon::KEY_PATH)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    if((valueError = Qx::Json::checkedKeyRetrieval(serverBuffer.filename, joServer, Json::Object_ServerDaemon::KEY_FILENAME)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    if((valueError = Qx::Json::checkedKeyRetrieval(serverBuffer.kill, joServer, Json::Object_ServerDaemon::KEY_KILL)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    // Get arguments
    QJsonArray jaArgs;
    if((valueError = Qx::Json::checkedKeyRetrieval(jaArgs, joServer, Json::Object_ServerDaemon::KEY_ARGUMENTS)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    for(const QJsonValue& jvArg : qAsConst(jaArgs))
    {
        // Ensure array element is String
        if(!jvArg.isString())
            return Qx::GenericError(Qx::GenericError::Critical, ERR_PARSING_JSON_DOC.arg(mSourceJsonFile->fileName()), ERR_JSON_UNEXP_FORMAT);

        serverBuffer.arguments.append(jvArg.toString());
    }

    // Resolve macros for relevant variables
    serverBuffer.path = mHostMacroResolver->resolve(serverBuffer.path);
    for(QString& arg : serverBuffer.arguments)
        arg = mHostMacroResolver->resolve(arg);

    // Return invalid error on success
    return Qx::GenericError();
}

Qx::GenericError ServicesReader::parseStartStop(StartStop& startStopBuffer, const QJsonValue& jvStartStop)
{
    // Ensure return buffer is null
    startStopBuffer = StartStop();

    // Ensure array element is Object
    if(!jvStartStop.isObject())
        return Qx::GenericError(Qx::GenericError::Critical, ERR_PARSING_JSON_DOC.arg(mSourceJsonFile->fileName()), ERR_JSON_UNEXP_FORMAT);

    // Get server Object
    QJsonObject joStartStop = jvStartStop.toObject();

    // Value error checking buffer
    Qx::GenericError valueError;

    // Get direct values
    if((valueError = Qx::Json::checkedKeyRetrieval(startStopBuffer.path, joStartStop, Json::Object_StartStop::KEY_PATH)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    if((valueError = Qx::Json::checkedKeyRetrieval(startStopBuffer.filename, joStartStop, Json::Object_StartStop::KEY_FILENAME)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    // Get arguments
    QJsonArray jaArgs;
    if((valueError = Qx::Json::checkedKeyRetrieval(jaArgs, joStartStop, Json::Object_StartStop::KEY_ARGUMENTS)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    for(const QJsonValue& jvArg : qAsConst(jaArgs))
    {
        // Ensure array element is String
        if(!jvArg.isString())
            return Qx::GenericError(Qx::GenericError::Critical, ERR_PARSING_JSON_DOC.arg(mSourceJsonFile->fileName()), ERR_JSON_UNEXP_FORMAT);

        startStopBuffer.arguments.append(jvArg.toString());
    }

    // Resolve macros for relevant variables
    startStopBuffer.path = mHostMacroResolver->resolve(startStopBuffer.path);
    for(QString& arg : startStopBuffer.arguments)
        arg = mHostMacroResolver->resolve(arg);

    // Return invalid error on success
    return Qx::GenericError();
}

}
