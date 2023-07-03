// Unit Includes
#include "fp/settings/fp-preferences.h"

// Qx Includes
#include <qx/core/qx-json.h>

using namespace Qt::Literals::StringLiterals;

namespace Fp
{

namespace Json
{
    namespace Object_AppPathOverride
    {
        const QString KEY_PATH = u"path"_s;
        const QString KEY_OVERRIDE = u"override"_s;
        const QString KEY_ENABLED = u"enabled"_s;
    };

    namespace Object_GameDataSource
    {
        const QString KEY_ARGUMENTS = u"arguments"_s;
        const QString KEY_NAME = u"name"_s;
        const QString KEY_TYPE = u"type"_s;
    }

    namespace Object_GameMetadataSource_GamesTags
    {
        const QString KEY_ACTUAL_UPDATE_TIME = u"actualUpdateTime"_s;
        const QString KEY_LAST_DELETE_TIME = u"lastDeleteTime"_s;
        const QString KEY_LAST_UPDATE_TIME = u"latestUpdateTime"_s;
    }

    namespace Object_GameMetadataSource
    {
        const QString KEY_BASE_URL = u"baseUrl"_s;
        const QString KEY_GAMES = u"games"_s;
        const QString KEY_NAME = u"name"_s;
        const QString KET_TAGS = u"tags"_s;
    }

    namespace Object_Preferences
    {
        const QString KEY_APP_PATH_OVERRIDES = u"appPathOverrides"_s;
        const QString KEY_BROWSER_MODE_PROXY = u"browserModeProxy"_s;
        const QString KEY_DATA_PACKS_FOLDER_PATH = u"dataPacksFolderPath"_s;
        const QString KEY_FPFSS_BASE_URL = u"fpfssBaseUrl"_s;
        const QString KEY_GAME_DATA_SOURCES = u"gameDataSources"_s;
        const QString KEY_GAME_METADATA_SOURCES = u"gameMetadataSources"_s;
        const QString KEY_HTDOCS_FOLDER_PATH = u"htdocsFolderPath"_s;
        const QString KEY_IMAGE_FOLDER_PATH = u"imageFolderPath"_s;
        const QString KEY_JSON_FOLDER_PATH = u"jsonFolderPath"_s;
        const QString KEY_NATIVE_PLATFORMS = u"nativePlatforms"_s;
        const QString KEY_ON_DEMAND_BASE_URL = u"onDemandBaseUrl"_s;
        const QString KEY_ON_DEMAND_IMAGES = u"onDemandImages"_s;
        const QString KEY_SERVER = u"server"_s;
    };
}

//===============================================================================================================
// PreferencesReader
//===============================================================================================================

//-Constructor--------------------------------------------------------------------------------------------------------
//Public:
PreferencesReader::PreferencesReader(Preferences* targetPreferences, std::shared_ptr<QFile> sourceJsonFile) :
    SettingsReader(targetPreferences, sourceJsonFile)
{}

//-Instance Functions-------------------------------------------------------------------------------------------------
//Private:
Qx::GenericError PreferencesReader::parseDocument(const QJsonDocument& prefDoc)
{
    // Get derivation specific target
    Preferences* targetPreferences = static_cast<Preferences*>(mTargetSettings);

    // Get values
    QJsonObject rootObj = prefDoc.object();
    Qx::GenericError valueError;

    // Single value
    if((valueError = Qx::Json::checkedKeyRetrieval(targetPreferences->fpfssBaseUrl, rootObj, Json::Object_Preferences::KEY_FPFSS_BASE_URL)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    if((valueError = Qx::Json::checkedKeyRetrieval(targetPreferences->imageFolderPath, rootObj, Json::Object_Preferences::KEY_IMAGE_FOLDER_PATH)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    if((valueError = Qx::Json::checkedKeyRetrieval(targetPreferences->jsonFolderPath, rootObj, Json::Object_Preferences::KEY_JSON_FOLDER_PATH)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    if((valueError = Qx::Json::checkedKeyRetrieval(targetPreferences->htdocsFolderPath, rootObj, Json::Object_Preferences::KEY_HTDOCS_FOLDER_PATH)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    if((valueError = Qx::Json::checkedKeyRetrieval(targetPreferences->dataPacksFolderPath, rootObj, Json::Object_Preferences::KEY_DATA_PACKS_FOLDER_PATH)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    if((valueError = Qx::Json::checkedKeyRetrieval(targetPreferences->onDemandImages, rootObj, Json::Object_Preferences::KEY_ON_DEMAND_IMAGES)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    if((valueError = Qx::Json::checkedKeyRetrieval(targetPreferences->onDemandBaseUrl, rootObj, Json::Object_Preferences::KEY_ON_DEMAND_BASE_URL)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    if((valueError = Qx::Json::checkedKeyRetrieval(targetPreferences->browserModeProxy, rootObj, Json::Object_Preferences::KEY_BROWSER_MODE_PROXY)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    if((valueError = Qx::Json::checkedKeyRetrieval(targetPreferences->server, rootObj, Json::Object_Preferences::KEY_SERVER)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    // Get app path overrides
    QJsonArray appPathOverrides;
    if((valueError = Qx::Json::checkedKeyRetrieval(appPathOverrides, rootObj, Json::Object_Preferences::KEY_APP_PATH_OVERRIDES)).isValid())
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
    if((valueError = Qx::Json::checkedKeyRetrieval(nativePlatforms, rootObj, Json::Object_Preferences::KEY_NATIVE_PLATFORMS)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    // Parse native platforms
    for(const QJsonValue& jvNativePlatform : qAsConst(nativePlatforms))
    {
        QString nativePlatformBuffer;
        if((valueError = parseNativePlatform(nativePlatformBuffer, jvNativePlatform)).isValid())
            return valueError;

        targetPreferences->nativePlatforms.insert(nativePlatformBuffer);
    }

    //TODO: PARSE NEW STRUCTUTRES

    // Get game data sources
    QJsonArray gameDataSources;
    if((valueError = Qx::Json::checkedKeyRetrieval(gameDataSources, rootObj, Json::Object_Preferences::KEY_GAME_DATA_SOURCES)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    // Parse game data sources
    for(const QJsonValue& jvGameDataSource : qAsConst(gameDataSources))
    {
        GameDataSource gameDataSourceBuffer;
        if((valueError = parseGameDataSource(gameDataSourceBuffer, jvGameDataSource)).isValid())
            return valueError;

        targetPreferences->gameDataSources.insert(gameDataSourceBuffer.name, gameDataSourceBuffer);
    }

    // Get game data sources
    QJsonArray gameMetadataSources;
    if((valueError = Qx::Json::checkedKeyRetrieval(gameMetadataSources, rootObj, Json::Object_Preferences::KEY_GAME_METADATA_SOURCES)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    // Parse game data sources
    for(const QJsonValue& jvGameMetadataSource : qAsConst(gameMetadataSources))
    {
        GameMetadataSource gameMetadataSourceBuffer;
        if((valueError = parseGameMetadataSource(gameMetadataSourceBuffer, jvGameMetadataSource)).isValid())
            return valueError;

        targetPreferences->gameMetadataSources.insert(gameMetadataSourceBuffer.name, gameMetadataSourceBuffer);
    }

    // Return invalid error on success
    return Qx::GenericError();
}

Qx::GenericError PreferencesReader::parseAppPathOverride(AppPathOverride& apoBuffer, const QJsonValue& jvApo)
{
    // Ensure array element is Object
    if(!jvApo.isObject())
        return Qx::GenericError(Qx::GenericError::Critical, ERR_PARSING_JSON_DOC.arg(mSourceJsonFile->fileName()), ERR_JSON_UNEXP_FORMAT);

    // Get app path override Object
    QJsonObject joApo = jvApo.toObject();

    // Value error checking buffer
    Qx::GenericError valueError;

    // Get direct values
    if((valueError = Qx::Json::checkedKeyRetrieval(apoBuffer.path, joApo, Json::Object_AppPathOverride::KEY_PATH)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    if((valueError = Qx::Json::checkedKeyRetrieval(apoBuffer.override, joApo, Json::Object_AppPathOverride::KEY_OVERRIDE)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    if((valueError = Qx::Json::checkedKeyRetrieval(apoBuffer.enabled, joApo, Json::Object_AppPathOverride::KEY_ENABLED)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    // Return invalid error on success
    return Qx::GenericError();
}

Qx::GenericError PreferencesReader::parseNativePlatform(QString& nativePlatformBuffer, const QJsonValue& jvNativePlatform)
{
    // Ensure array element is String
    if(!jvNativePlatform.isString())
        return Qx::GenericError(Qx::GenericError::Critical, ERR_PARSING_JSON_DOC.arg(mSourceJsonFile->fileName()), ERR_JSON_UNEXP_FORMAT);

    // Get native platform string
    nativePlatformBuffer = jvNativePlatform.toString();

    // Return invalid error on success
    return Qx::GenericError();
}

Qx::GenericError PreferencesReader::parseGameDataSource(GameDataSource& gdsBuffer, const QJsonValue& jvGds)
{
    // Ensure array element is Object
    if(!jvGds.isObject())
        return Qx::GenericError(Qx::GenericError::Critical, ERR_PARSING_JSON_DOC.arg(mSourceJsonFile->fileName()), ERR_JSON_UNEXP_FORMAT);

    // Get game data source Object
    QJsonObject joGds = jvGds.toObject();

    // Value error checking buffer
    Qx::GenericError valueError;

    // Get direct values
    if((valueError = Qx::Json::checkedKeyRetrieval(gdsBuffer.name, joGds, Json::Object_GameDataSource::KEY_NAME)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    if((valueError = Qx::Json::checkedKeyRetrieval(gdsBuffer.type, joGds, Json::Object_GameDataSource::KEY_TYPE)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    // Get arguments
    QJsonArray arguments;
    if((valueError = Qx::Json::checkedKeyRetrieval(arguments, joGds, Json::Object_GameDataSource::KEY_ARGUMENTS)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    // Parse arguments
    for(const QJsonValue& jvArgument : qAsConst(arguments))
    {
        if(!jvArgument.isString())
            return Qx::GenericError(Qx::GenericError::Critical, ERR_PARSING_JSON_DOC.arg(mSourceJsonFile->fileName()), ERR_JSON_UNEXP_FORMAT);

        gdsBuffer.arguments.append(jvArgument.toString());
    }

    // Return invalid error on success
    return Qx::GenericError();
}

Qx::GenericError PreferencesReader::parseGameMetadataSource(GameMetadataSource& gmsBuffer, const QJsonValue& jvGms)
{
    // Ensure array element is Object
    if(!jvGms.isObject())
        return Qx::GenericError(Qx::GenericError::Critical, ERR_PARSING_JSON_DOC.arg(mSourceJsonFile->fileName()), ERR_JSON_UNEXP_FORMAT);

    // Get game data source Object
    QJsonObject joGms = jvGms.toObject();

    // Value error checking buffer
    Qx::GenericError valueError;

    // Get direct values
    if((valueError = Qx::Json::checkedKeyRetrieval(gmsBuffer.baseUrl, joGms, Json::Object_GameMetadataSource::KEY_BASE_URL)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    if((valueError = Qx::Json::checkedKeyRetrieval(gmsBuffer.name, joGms, Json::Object_GameMetadataSource::KEY_NAME)).isValid())
        return valueError.setErrorLevel(Qx::GenericError::Critical);

    // Return invalid error on success
    return Qx::GenericError();
}

}
