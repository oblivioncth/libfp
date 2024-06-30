#ifndef FLASHPOINT_INSTALL_H
#define FLASHPOINT_INSTALL_H

// Shared Lib Support
#include "fp/fp_export.h"

// Qt Includes
#include <QString>
#include <QDir>
#include <QFile>
#include <QtSql>

// Qx Includes
#include <qx/core/qx-versionnumber.h>
#include <qx/io/qx-common-io.h>

// Project Includes
#include "fp/settings/fp-config.h"
#include "fp/settings/fp-execs.h"
#include "fp/settings/fp-preferences.h"
#include "fp/settings/fp-services.h"
#include "fp/fp-macro.h"
#include "fp/fp-db.h"
#include "fp/fp-playlistmanager.h"
#include "fp/fp-daemon.h"
#include "fp/fp-toolkit.h"

namespace Fp
{

inline const QString NAME = u"Flashpoint"_s;

class QX_ERROR_TYPE(InstallError, "Fp::InstallError", 1100)
{
    friend class Install;
//-Class Enums-------------------------------------------------------------
public:
    enum Type
    {
        NoError,
        FileMissing,
        DaemonCountMismatch,
        DatapackSourceMissing
    };

//-Class Variables-------------------------------------------------------------
private:
    static inline const QHash<Type, QString> ERR_STRINGS{
        {NoError, u""_s},
        {FileMissing, u"A required flashpoint install file is missing."_s},
        {DaemonCountMismatch, u"The number of configured daemons differs from the expected amount."_s},
        {DatapackSourceMissing, u"Expected datapack source missing."_s}
    };

//-Instance Variables-------------------------------------------------------------
private:
    Type mType;
    QString mSpecific;

//-Constructor-------------------------------------------------------------
private:
    InstallError(Type t = NoError, const QString& s = {});

//-Instance Functions-------------------------------------------------------------
public:
    bool isValid() const;
    Type type() const;
    QString specific() const;

private:
    Qx::Severity deriveSeverity() const override;
    quint32 deriveValue() const override;
    QString derivePrimary() const override;
    QString deriveSecondary() const override;
};

class FP_FP_EXPORT Install
{
    friend class Toolkit;

//-Inner Class---------------------------------------------------------------------------------------------------
public:
    class FP_FP_EXPORT VersionInfo;

//-Class Variables-----------------------------------------------------------------------------------------------
public: // Ugh
#if defined _WIN32
    static inline const QString LAUNCHER_NAME =  u"Flashpoint.exe"_s;
#elif defined __linux__
    static inline const QString LAUNCHER_NAME =  u"flashpoint-launcher"_s;
#endif

private:
    // Static paths
    static inline const QString LAUNCHER_PATH =  u"Launcher/"_s + LAUNCHER_NAME;
    static inline const QString DATABASE_PATH = u"Data/flashpoint.sqlite"_s;
    static inline const QString CONFIG_JSON_PATH = u"Launcher/config.json"_s;
    static inline const QString PREFERENCES_JSON_PATH = u"preferences.json"_s;
    static inline const QString VER_TXT_PATH = u"version.txt"_s;

    // Dynamic path file names
    static inline const QString SERVICES_JSON_NAME = u"services.json"_s;
    static inline const QString EXECS_JSON_NAME = u"execs.json"_s;

    // Static Folders
    static inline const QString EXTRAS_PATH = u"Extras"_s;

    // Dynamic path folder names
    static inline const QString LOGOS_FOLDER_NAME = u"Logos"_s;
    static inline const QString SCREENSHOTS_FOLDER_NAME = u"Screenshots"_s;

    // Main datapack source
    static inline const QString MAIN_DATAPACK_SOURCE = u"Flashpoint Project"_s;

//-Instance Variables-----------------------------------------------------------------------------------------------
private:
    // Validity
    bool mValid;
    Qx::Error mError;

    // Files and directories
    QDir mRootDirectory;
    QDir mPlatformLogosDirectory;
    QDir mEntryLogosDirectory;
    QDir mEntryScreenshotsDirectory;
    QDir mExtrasDirectory;
    QDir mPlaylistsDirectory;
    std::unique_ptr<QFile> mLauncherFile;
    std::unique_ptr<QFile> mDatabaseFile;
    std::shared_ptr<QFile> mConfigJsonFile;
    std::shared_ptr<QFile> mPreferencesJsonFile;
    std::shared_ptr<QFile> mServicesJsonFile;
    std::shared_ptr<QFile> mExecsJsonFile;
    std::unique_ptr<QFile> mVersionFile;

    // Info
    std::shared_ptr<VersionInfo> mVersionInfo;

    // Settings
    Config mConfig;
    Preferences mPreferences;
    Services mServices;
    Execs mExecs;
    Daemon mDaemon;

    // Facilities
    Db* mDatabase = nullptr;
    PlaylistManager* mPlaylistManager = nullptr;
    MacroResolver* mMacroResolver = nullptr;
    Toolkit* mToolkit = nullptr;

//-Constructor-------------------------------------------------------------------------------------------------
public:
    Install(QString installPath, bool preloadPlaylists = false);

//-Destructor-------------------------------------------------------------------------------------------------
public:
    ~Install();

//-Instance Functions------------------------------------------------------------------------------------------------------
private:
    void establishDaemon();
    void nullify();

public:
    // Validity
    bool isValid() const;
    Qx::Error error() const;

    // General information
    std::shared_ptr<VersionInfo> versionInfo() const;
    QString launcherChecksum() const;

    // Facilities
    Db* database();
    PlaylistManager* playlistManager();
    const MacroResolver* macroResolver() const;
    const Toolkit* toolkit() const;

    // Settings
    // TODO: At some point create a "Settings" object that wraps all of these, would need to rename existing Fp::Settings
    const Config& config() const;
    const Preferences& preferences() const;
    const Services& services() const;
    const Execs& execs() const;
    Daemon outfittedDaemon() const;

    // Data access
    QDir dir() const;
    QDir entryLogosDirectory() const;
    QDir entryScreenshotsDirectory() const;
    QDir extrasDirectory() const;
    QDir platformLogosDirectory() const;
};

class Install::VersionInfo
{
//-Class Enums---------------------------------------------------------------------------------------------------
public:
    enum Edition {Ultimate, Infinity, Core};

//-Class Variables
private:
    // Regex
    static inline const QString VER_TXT_GRP_EDITIONA = u"e1"_s;
    static inline const QString VER_TXT_GRP_EDITIONB = u"e2"_s;
    static inline const QString VER_TXT_GRP_VERSION = u"v"_s;
    static inline const QString VER_TXT_GRP_NICK = u"n"_s;
    static inline const QRegularExpression VER_TXT_REGEX = QRegularExpression(
        uR"([fF]lashpoint\s+(?<)"_s + VER_TXT_GRP_EDITIONA +
        uR"(>[a-zA-Z]+)?\s*(?<)"_s + VER_TXT_GRP_VERSION +
        uR"(>[0-9]+(?:\.[0-9]+)?)?\s*(?<)"_s + VER_TXT_GRP_EDITIONB +
        uR"(>[a-zA-Z]+)?\s+-\s+(?<)"_s + VER_TXT_GRP_NICK +
        uR"(>.*))"_s
    );

//-Instance Variables-----------------------------------------------------------------------------------------------
private:
    QString mFullString;
    Edition mEdition;
    Qx::VersionNumber mVersion;
    QString mNickname;

//-Constructor-------------------------------------------------------------------------------------------------
public:
    VersionInfo(const QString& verTxtStr);

//-Instance Functions------------------------------------------------------------------------------------------------------
public:
    bool isNull() const;
    QString fullString() const;
    Edition edition() const;
    Qx::VersionNumber version() const;
    QString nickname() const;
};

}



#endif // FLASHPOINT_INSTALL_H
