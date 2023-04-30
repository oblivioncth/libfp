// Unit Includes
#include "fp/fp-json.h"

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
// JSON::START_STOP
//===============================================================================================================

//-Operators----------------------------------------------------------------------------------------------------
//Public:
bool operator== (const Json::StartStop& lhs, const Json::StartStop& rhs) noexcept
{
    return lhs.path == rhs.path && lhs.filename == rhs.filename && lhs.arguments == rhs.arguments;
}

//-Hashing------------------------------------------------------------------------------------------------------
size_t qHash(const Json::StartStop& key, size_t seed) noexcept
{
    QtPrivate::QHashCombine hash;
    seed = hash(seed, key.path);
    seed = hash(seed, key.filename);
    seed = hash(seed, key.arguments);

    return seed;
}

//===============================================================================================================
// JSON::SETTINGS_READER
//===============================================================================================================

//-Constructor--------------------------------------------------------------------------------------------------------
//Public:
Json::SettingsReader::SettingsReader(Settings* targetSettings, std::shared_ptr<QFile> sourceJsonFile) :
    mTargetSettings(targetSettings),
    mSourceJsonFile(sourceJsonFile)
{}

//-Instance Functions-------------------------------------------------------------------------------------------------
//Public:
Qx::GenericError Json::SettingsReader::readInto()
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

//===============================================================================================================
// JSON::CONFIG_READER
//===============================================================================================================

//-Constructor--------------------------------------------------------------------------------------------------------
//Public:
Json::ConfigReader::ConfigReader(Config* targetConfig, std::shared_ptr<QFile> sourceJsonFile) :
    SettingsReader(targetConfig, sourceJsonFile)
{}

//-Instance Functions-------------------------------------------------------------------------------------------------
//Private:
Qx::GenericError Json::ConfigReader::parseDocument(const QJsonDocument& configDoc)
{
    // Get derivation specific target
    Config* targetConfig = static_cast<Config*>(mTargetSettings);

    // Get values
    Qx::GenericError valueError;

    if((valueError = Qx::Json::checkedKeyRetrieval(targetConfig->flashpointPath, configDoc.object(), Object_Config::KEY_FLASHPOINT_PATH)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    if((valueError = Qx::Json::checkedKeyRetrieval(targetConfig->startServer, configDoc.object(), Object_Config::KEY_START_SERVER)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    if((valueError = Qx::Json::checkedKeyRetrieval(targetConfig->server, configDoc.object(), Object_Config::KEY_SERVER)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    // Return invalid error on success
    return Qx::GenericError();

}

//===============================================================================================================
// JSON::PREFERENCES_READER
//===============================================================================================================

//-Constructor--------------------------------------------------------------------------------------------------------
//Public:
Json::PreferencesReader::PreferencesReader(Preferences* targetPreferences, std::shared_ptr<QFile> sourceJsonFile) :
    SettingsReader(targetPreferences, sourceJsonFile)
{}

//-Instance Functions-------------------------------------------------------------------------------------------------
//Private:
Qx::GenericError Json::PreferencesReader::parseDocument(const QJsonDocument& prefDoc)
{
    // Get derivation specific target
    Preferences* targetPreferences = static_cast<Preferences*>(mTargetSettings);

    // Get values
    Qx::GenericError valueError;

    // Single value
    if((valueError = Qx::Json::checkedKeyRetrieval(targetPreferences->imageFolderPath, prefDoc.object(), Object_Preferences::KEY_IMAGE_FOLDER_PATH)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    if((valueError = Qx::Json::checkedKeyRetrieval(targetPreferences->jsonFolderPath, prefDoc.object(), Object_Preferences::KEY_JSON_FOLDER_PATH)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    if((valueError = Qx::Json::checkedKeyRetrieval(targetPreferences->htdocsFolderPath, prefDoc.object(), Object_Preferences::KEY_HTDOCS_FOLDER_PATH)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    if((valueError = Qx::Json::checkedKeyRetrieval(targetPreferences->dataPacksFolderPath, prefDoc.object(), Object_Preferences::KEY_DATA_PACKS_FOLDER_PATH)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    if((valueError = Qx::Json::checkedKeyRetrieval(targetPreferences->onDemandImages, prefDoc.object(), Object_Preferences::KEY_ON_DEMAND_IMAGES)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    if((valueError = Qx::Json::checkedKeyRetrieval(targetPreferences->onDemandBaseUrl, prefDoc.object(), Object_Preferences::KEY_ON_DEMAND_BASE_URL)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    if((valueError = Qx::Json::checkedKeyRetrieval(targetPreferences->browserModeProxy, prefDoc.object(), Object_Preferences::KEY_BROWSER_MODE_PROXY)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    // Get app path overrides
    QJsonArray appPathOverrides;
    if((valueError = Qx::Json::checkedKeyRetrieval(appPathOverrides, prefDoc.object(), Object_Preferences::KEY_APP_PATH_OVERRIDES)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    // Parse app path overrides
    for(const QJsonValue& jvApo : qAsConst(appPathOverrides))
    {
        AppPathOverride apoBuffer;
        if((valueError = parseAppPathOverride(apoBuffer, jvApo)).isValid())
            return valueError;

        targetPreferences->appPathOverrides.append(apoBuffer);
    }

    // Get native platforms
    QJsonArray nativePlatforms;
    if((valueError = Qx::Json::checkedKeyRetrieval(nativePlatforms, prefDoc.object(), Object_Preferences::KEY_NATIVE_PLATFORMS)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    // Parse native platforms
    for(const QJsonValue& jvNativePlatform : qAsConst(nativePlatforms))
    {
        QString nativePlatformBuffer;
        if((valueError = parseNativePlatform(nativePlatformBuffer, jvNativePlatform)).isValid())
            return valueError;

        targetPreferences->nativePlatforms.insert(nativePlatformBuffer);
    }

    // Return invalid error on success
    return Qx::GenericError();
}

Qx::GenericError Json::PreferencesReader::parseAppPathOverride(AppPathOverride& apoBuffer, const QJsonValue& jvApo)
{
    // Ensure array element is Object
    if(!jvApo.isObject())
        return Qx::GenericError(Qx::GenericError::Critical, ERR_PARSING_JSON_DOC.arg(mSourceJsonFile->fileName()), ERR_JSON_UNEXP_FORMAT);

    // Get app path override Object
    QJsonObject joApo = jvApo.toObject();

    // Value error checking buffer
    Qx::GenericError valueError;

    // Get direct values
    if((valueError = Qx::Json::checkedKeyRetrieval(apoBuffer.path, joApo, Object_AppPathOverrides::KEY_PATH)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    if((valueError = Qx::Json::checkedKeyRetrieval(apoBuffer.override, joApo, Object_AppPathOverrides::KEY_OVERRIDE)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    if((valueError = Qx::Json::checkedKeyRetrieval(apoBuffer.enabled, joApo, Object_AppPathOverrides::KEY_ENABLED)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    // Return invalid error on success
    return Qx::GenericError();
}

Qx::GenericError Json::PreferencesReader::parseNativePlatform(QString& nativePlatformBuffer, const QJsonValue& jvNativePlatform)
{
    // Ensure array element is String
    if(!jvNativePlatform.isString())
        return Qx::GenericError(Qx::GenericError::Critical, ERR_PARSING_JSON_DOC.arg(mSourceJsonFile->fileName()), ERR_JSON_UNEXP_FORMAT);

    // Get native platform string
    nativePlatformBuffer = jvNativePlatform.toString();

    // Return invalid error on success
    return Qx::GenericError();
}

//===============================================================================================================
// JSON::SERVICES_READER
//===============================================================================================================

//-Constructor--------------------------------------------------------------------------------------------------------
//Public:
Json::ServicesReader::ServicesReader(Services* targetServices, std::shared_ptr<QFile> sourceJsonFile, const MacroResolver* macroResolver) :
    SettingsReader(targetServices, sourceJsonFile),
    mHostMacroResolver(macroResolver)
{}

//-Instance Functions-------------------------------------------------------------------------------------------------
//Private:
Qx::GenericError Json::ServicesReader::parseDocument(const QJsonDocument& servicesDoc)
{
    // Get derivation specific target
    Services* targetServices = static_cast<Services*>(mTargetSettings);

    // Value error checking buffer
    Qx::GenericError valueError;

    // Get watches
    // TODO: include logs

    // Get servers
    QJsonArray jaServers;
    if((valueError = Qx::Json::checkedKeyRetrieval(jaServers, servicesDoc.object(), Object_Services::KEY_SERVER)).isValid())
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
    if((valueError = Qx::Json::checkedKeyRetrieval(jaDaemons, servicesDoc.object(), Object_Services::KEY_DAEMON)).isValid())
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
    if((valueError = Qx::Json::checkedKeyRetrieval(jaStarts, servicesDoc.object(), Object_Services::KEY_START)).isValid())
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
    if((valueError = Qx::Json::checkedKeyRetrieval(jaStops, servicesDoc.object(), Object_Services::KEY_STOP)).isValid())
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

Qx::GenericError Json::ServicesReader::parseServerDaemon(ServerDaemon& serverBuffer, const QJsonValue& jvServer)
{
    // Ensure array element is Object
    if(!jvServer.isObject())
        return Qx::GenericError(Qx::GenericError::Critical, ERR_PARSING_JSON_DOC.arg(mSourceJsonFile->fileName()), ERR_JSON_UNEXP_FORMAT);

    // Get server Object
    QJsonObject joServer = jvServer.toObject();

    // Value error checking buffer
    Qx::GenericError valueError;

    // Get direct values
    if((valueError = Qx::Json::checkedKeyRetrieval(serverBuffer.name, joServer, Object_Server::KEY_NAME)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    if((valueError = Qx::Json::checkedKeyRetrieval(serverBuffer.path, joServer, Object_Server::KEY_PATH)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    if((valueError = Qx::Json::checkedKeyRetrieval(serverBuffer.filename, joServer, Object_Server::KEY_FILENAME)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    if((valueError = Qx::Json::checkedKeyRetrieval(serverBuffer.kill, joServer, Object_Server::KEY_KILL)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    // Get arguments
    QJsonArray jaArgs;
    if((valueError = Qx::Json::checkedKeyRetrieval(jaArgs, joServer, Object_Server::KEY_ARGUMENTS)).isValid())
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

Qx::GenericError Json::ServicesReader::parseStartStop(StartStop& startStopBuffer, const QJsonValue& jvStartStop)
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
    if((valueError = Qx::Json::checkedKeyRetrieval(startStopBuffer.path, joStartStop , Object_StartStop::KEY_PATH)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    if((valueError = Qx::Json::checkedKeyRetrieval(startStopBuffer.filename, joStartStop, Object_StartStop::KEY_FILENAME)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    // Get arguments
    QJsonArray jaArgs;
    if((valueError = Qx::Json::checkedKeyRetrieval(jaArgs, joStartStop, Object_StartStop::KEY_ARGUMENTS)).isValid())
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

//===============================================================================================================
// JSON::EXECS_READER
//===============================================================================================================

//-Constructor--------------------------------------------------------------------------------------------------------
//Public:
Json::ExecsReader::ExecsReader(Execs* targetExecs, std::shared_ptr<QFile> sourceJsonFile) :
    SettingsReader(targetExecs, sourceJsonFile)
{}

//-Instance Functions-------------------------------------------------------------------------------------------------
//Private:
Qx::GenericError Json::ExecsReader::parseDocument(const QJsonDocument& execsDoc)
{
    // Get derivation specific target
    Execs* targetExecs = static_cast<Execs*>(mTargetSettings);

    // Value error checking buffer
    Qx::GenericError valueError;

    // Get exec entries
    QJsonArray jaExecs;
    if((valueError = Qx::Json::checkedKeyRetrieval(jaExecs, execsDoc.object(), Object_Execs::KEY_EXECS)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    // Parse starts
    for(const QJsonValue& jvExec : qAsConst(jaExecs))
    {
        Exec execBuffer;
        if((valueError = parseExec(execBuffer, jvExec)).isValid())
            return valueError;

        targetExecs->list.append(execBuffer);
    }

    // Return invalid error on success
    return Qx::GenericError();
}

Qx::GenericError Json::ExecsReader::parseExec(Exec& execBuffer, const QJsonValue& jvExec)
{
    // Ensure array element is Object
    if(!jvExec.isObject())
        return Qx::GenericError(Qx::GenericError::Critical, ERR_PARSING_JSON_DOC.arg(mSourceJsonFile->fileName()), ERR_JSON_UNEXP_FORMAT);

    // Get exec Object
    QJsonObject joExec = jvExec.toObject();

    // Value error checking buffer
    Qx::GenericError valueError;

    // Get direct values (ignore errors for optional values)
    if((valueError = Qx::Json::checkedKeyRetrieval(execBuffer.win32, joExec , Object_Exec::KEY_WIN32)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    Qx::Json::checkedKeyRetrieval(execBuffer.linux, joExec , Object_Exec::KEY_LINUX);
    Qx::Json::checkedKeyRetrieval(execBuffer.wine, joExec , Object_Exec::KEY_WINE);


    // Return invalid error on success
    return Qx::GenericError();
}

}
