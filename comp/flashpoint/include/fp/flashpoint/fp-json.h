#ifndef FLASHPOINT_JSON_H
#define FLASHPOINT_JSON_H

// Qt Includes
#include <QString>
#include <QSet>
#include <QFile>

// Qx Includes
#include <qx/core/qx-genericerror.h>

// Project Includes
#include "fp-macro.h"

namespace Fp
{

class Json
{
//-Inner Classes-------------------------------------------------------------------------------------------------
private:
    class Object_Config
    {
    public:
        static inline const QString KEY_FLASHPOINT_PATH = "flashpointPath"; // Reading this value is current redundant and unused, but this may change in the future
        static inline const QString KEY_START_SERVER = "startServer";
        static inline const QString KEY_SERVER = "server";
    };

    class Object_AppPathOverrides
    {
    public:
        static inline const QString KEY_PATH = "path";
        static inline const QString KEY_OVERRIDE = "override";
        static inline const QString KEY_ENABLED = "enabled";
    };

    class Object_Preferences
    {
    public:
        static inline const QString KEY_IMAGE_FOLDER_PATH = "imageFolderPath";
        static inline const QString KEY_JSON_FOLDER_PATH = "jsonFolderPath";
        static inline const QString KEY_HTDOCS_FOLDER_PATH = "htdocsFolderPath";
        static inline const QString KEY_DATA_PACKS_FOLDER_PATH = "dataPacksFolderPath";
        static inline const QString KEY_ON_DEMAND_IMAGES = "onDemandImages";
        static inline const QString KEY_ON_DEMAND_BASE_URL = "onDemandBaseUrl";
        static inline const QString KEY_APP_PATH_OVERRIDES = "appPathOverrides";
        static inline const QString KEY_NATIVE_PLATFORMS = "nativePlatforms";
    };

    class Object_Server
    {
    public:
        static inline const QString KEY_NAME = "name";
        static inline const QString KEY_PATH = "path";
        static inline const QString KEY_FILENAME = "filename";
        static inline const QString KEY_ARGUMENTS = "arguments";
        static inline const QString KEY_KILL = "kill";
    };

    class Object_StartStop
    {
    public:
        static inline const QString KEY_PATH = "path";
        static inline const QString KEY_FILENAME = "filename";
        static inline const QString KEY_ARGUMENTS = "arguments";
    };

    class Object_Daemon
    {
    public:
        static inline const QString KEY_NAME = "name";
        static inline const QString KEY_PATH = "path";
        static inline const QString KEY_FILENAME = "filename";
        static inline const QString KEY_ARGUMENTS = "arguments";
        static inline const QString KEY_KILL = "kill";
    };

    class Object_Services
    {
    public:
        static inline const QString KEY_WATCH = "watch";
        static inline const QString KEY_SERVER = "server";
        static inline const QString KEY_DAEMON = "daemon";
        static inline const QString KEY_START = "start";
        static inline const QString KEY_STOP = "stop";
    };

    class Object_Execs
    {
    public:
        static inline const QString KEY_EXECS = "execs";
    };

    class Object_Exec
    {
    public:
        static inline const QString KEY_WIN32 = "win32";
        static inline const QString KEY_LINUX = "linux";
        static inline const QString KEY_WINE = "wine";
    };

    struct Settings {};

public:
    struct ServerDaemon
    {
        QString name;
        QString path;
        QString filename;
        QStringList arguments;
        bool kill;
    };

    struct StartStop
    {
        QString path;
        QString filename;
        QStringList arguments;

        friend bool operator== (const StartStop& lhs, const StartStop& rhs) noexcept;
        friend size_t qHash(const StartStop& key, size_t seed) noexcept;
    };

    struct Config : public Settings
    {
        QString flashpointPath;
        bool startServer;
        QString server;
    };

    struct AppPathOverride
    {
        QString path;
        QString override;
        bool enabled;
    };

    struct Preferences : public Settings
    {
        QString imageFolderPath;
        QString jsonFolderPath;
        QString htdocsFolderPath;
        QString dataPacksFolderPath;
        bool onDemandImages;
        QString onDemandBaseUrl;
        QList<AppPathOverride> appPathOverrides;
        QSet<QString> nativePlatforms;
    };

    struct Services : public Settings
    {
        //QSet<Watch> watches;
        QHash<QString, ServerDaemon> servers;
        QHash<QString, ServerDaemon> daemons;
        QSet<StartStop> starts;
        QSet<StartStop> stops;
    };

    struct Exec
    {
        QString linux;
        QString win32;
        QString wine;
    };

    struct Execs : public Settings
    {
        QList<Exec> list;
    };

    class SettingsReader
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

    //-Instance Functions-------------------------------------------------------------------------------------------------
    private:
        virtual Qx::GenericError parseDocument(const QJsonDocument& jsonDoc) = 0;

    public:
        Qx::GenericError readInto();

    };

    class ConfigReader : public SettingsReader
    {
    //-Constructor--------------------------------------------------------------------------------------------------------
    public:
        ConfigReader(Config* targetConfig, std::shared_ptr<QFile> sourceJsonFile);

    //-Instance Functions-------------------------------------------------------------------------------------------------
    private:
        Qx::GenericError parseDocument(const QJsonDocument& configDoc);
    };

    class PreferencesReader : public SettingsReader
    {
    //-Constructor--------------------------------------------------------------------------------------------------------
    public:
        PreferencesReader(Preferences* targetPreferences, std::shared_ptr<QFile> sourceJsonFile);

    //-Instance Functions-------------------------------------------------------------------------------------------------
    private:
        Qx::GenericError parseDocument(const QJsonDocument& prefDoc);
        Qx::GenericError parseAppPathOverride(AppPathOverride& apoBuffer, const QJsonValue& jvApo);
        Qx::GenericError parseNativePlatform(QString& nativePlatformBuffer, const QJsonValue& jvNativePlatform);
    };

    class ServicesReader : public SettingsReader
    {
    //-Instance Variables--------------------------------------------------------------------------------------------------
    private:
        const MacroResolver* mHostMacroResolver;

    //-Constructor--------------------------------------------------------------------------------------------------------
    public:
        ServicesReader(Services* targetServices, std::shared_ptr<QFile> sourceJsonFile, const MacroResolver* macroResolver);

    //-Instance Functions-------------------------------------------------------------------------------------------------
    private:
        Qx::GenericError parseDocument(const QJsonDocument& servicesDoc);
        Qx::GenericError parseServerDaemon(ServerDaemon& serverBuffer, const QJsonValue& jvServer);
        Qx::GenericError parseStartStop(StartStop& startStopBuffer, const QJsonValue& jvStartStop);
    };

    class ExecsReader : public SettingsReader
    {
    //-Constructor--------------------------------------------------------------------------------------------------------
    public:
        ExecsReader(Execs* targetExecs, std::shared_ptr<QFile> sourceJsonFile);

    //-Instance Functions-------------------------------------------------------------------------------------------------
    private:
        Qx::GenericError parseDocument(const QJsonDocument& execsDoc);
        Qx::GenericError parseExec(Exec& execBuffer, const QJsonValue& jvExec);
    };
};
}

#endif // FLASHPOINT_JSON_H
