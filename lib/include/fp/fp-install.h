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
#include "fp/fp-items.h"
#include "fp/fp-playlistmanager.h"

namespace Fp
{

inline const QString NAME = u"Flashpoint"_s;

class FP_FP_EXPORT Install
{
//-Class Enums---------------------------------------------------------------------------------------------------
enum class Edition {Ultimate, Infinity, Core};

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

    // File Info
    static inline const QString IMAGE_UC_EXT = u".png"_s;
    static inline const QString IMAGE_C_EXT = u".jpg"_s;
    static inline const QString IMAGE_C_URL_SUFFIX = u"?type=jpg"_s;

    // Dynamic path file names
    static inline const QString SERVICES_JSON_NAME = u"services.json"_s;
    static inline const QString EXECS_JSON_NAME = u"execs.json"_s;

    // Static Folders
    static inline const QString EXTRAS_PATH = u"Extras"_s;

    // Dynamic path folder names
    static inline const QString LOGOS_FOLDER_NAME = u"Logos"_s;
    static inline const QString SCREENSHOTS_FOLDER_NAME = u"Screenshots"_s;

    // Error
    static inline const QString ERR_FILE_MISSING = u"A required flashpoint install file is missing."_s;

    // Settings
    static inline const QString MACRO_FP_PATH = u"<fpPath>"_s;

    // Regex
    static inline const QRegularExpression VERSION_NUMBER_REGEX = QRegularExpression(u"[fF]lashpoint (?<version>.*?) "_s);

public:
    static inline const QFileInfo SECURE_PLAYER_INFO = QFileInfo(u"FlashpointSecurePlayer.exe"_s);

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

    // Settings
    Config mConfig;
    Preferences mPreferences;
    Services mServices;
    Execs mExecs;

    // Database
    Db* mDatabase = nullptr;

    // Playlist Manager
    PlaylistManager* mPlaylistManager = nullptr;

    // Utilities
    MacroResolver* mMacroResolver = nullptr;

//-Constructor-------------------------------------------------------------------------------------------------
public:
    Install(QString installPath, bool preloadPlaylists = false);

//-Destructor-------------------------------------------------------------------------------------------------
public:
    ~Install();

//-Class Functions------------------------------------------------------------------------------------------------------
private:
    static QString standardImageSubPath(QUuid gameId);

public:
    static Qx::Error appInvolvesSecurePlayer(bool& involvesBuffer, QFileInfo appInfo);

//-Instance Functions------------------------------------------------------------------------------------------------------
private:
    void nullify();

public:
    // Validity
    bool isValid() const;
    Qx::Error error() const;

    // General information
    Edition edition() const;
    QString nameVersionString() const;
    Qx::VersionNumber version() const;
    QString launcherChecksum() const;

    // Database
    Db* database();

    // Playlist Manager
    PlaylistManager* playlistManager();

    // Support Application Checks
    // TODO: At some point create a "Settings" object that wraps all of these, would need to rename existing Fp::Settings
    const Config& config() const;
    const Preferences& preferences() const;
    const Services& services() const;
    const Execs& execs() const;

    // Data access
    QString fullPath() const;
    QDir entryLogosDirectory() const;
    QDir entryScreenshotsDirectory() const;
    QDir extrasDirectory() const;
    QString platformLogoPath(const QString& platform);
    QString entryImageLocalPath(ImageType imageType, const QUuid& gameId) const;
    QUrl entryImageRemoteUrl(ImageType imageType, const QUuid& gameId) const;
    const MacroResolver* macroResolver() const;

    // Helper
    QString resolveAppPathOverrides(const QString& appPath) const;
    QString resolveExecSwaps(const QString& appPath, const QString& platform) const;
};

}



#endif // FLASHPOINT_INSTALL_H
