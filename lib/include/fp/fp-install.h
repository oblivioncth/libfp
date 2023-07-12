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

namespace Fp
{

inline const QString NAME = QStringLiteral("Flashpoint");

class FP_FP_EXPORT Install
{
//-Class Enums---------------------------------------------------------------------------------------------------
enum class Edition {Ultimate, Infinity, Core};

//-Class Variables-----------------------------------------------------------------------------------------------
public: // Ugh
#if defined _WIN32
    static inline const QString LAUNCHER_NAME =  "Flashpoint.exe";
#elif defined __linux__
    static inline const QString LAUNCHER_NAME =  "flashpoint-launcher";
#endif

private:
    // Static paths
    static inline const QString LAUNCHER_PATH =  "Launcher/" + LAUNCHER_NAME;
    static inline const QString DATABASE_PATH = "Data/flashpoint.sqlite";
    static inline const QString CONFIG_JSON_PATH = "Launcher/config.json";
    static inline const QString PREFERENCES_JSON_PATH = "preferences.json";
    static inline const QString VER_TXT_PATH = "version.txt";

    // File Info
    static inline const QString IMAGE_EXT = ".png";


    // Dynamic path file names
    static inline const QString SERVICES_JSON_NAME = "services.json";
    static inline const QString EXECS_JSON_NAME = "execs.json";

    // Static Folders
    static inline const QString EXTRAS_PATH = "Extras";

    // Dynamic path folder names
    static inline const QString LOGOS_FOLDER_NAME = "Logos";
    static inline const QString SCREENSHOTS_FOLDER_NAME = "Screenshots";

    // Error
    static inline const QString ERR_FILE_MISSING = QSL("A required flashpoint install file is missing.");

    // Settings
    static inline const QString MACRO_FP_PATH = "<fpPath>";

    // Regex
    static inline const QRegularExpression VERSION_NUMBER_REGEX = QRegularExpression("[fF]lashpoint (?<version>.*?) ");

public:
    static inline const QFileInfo SECURE_PLAYER_INFO = QFileInfo("FlashpointSecurePlayer.exe");

//-Instance Variables-----------------------------------------------------------------------------------------------
private:
    // Validity
    bool mValid;
    Qx::Error mError;

    // Files and directories
    QDir mRootDirectory;
    QDir mLogosDirectory;
    QDir mScreenshotsDirectory;
    QDir mExtrasDirectory;
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

    // Utilities
    MacroResolver* mMacroResolver = nullptr;

//-Constructor-------------------------------------------------------------------------------------------------
public:
    Install(QString installPath);

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

    // Support Application Checks
    // TODO: At some point create a "Settings" object that wraps all of these, would need to rename existing Fp::Settings
    const Config& config() const;
    const Preferences& preferences() const;
    const Services& services() const;
    const Execs& execs() const;

    // Data access
    QString fullPath() const;
    QDir logosDirectory() const;
    QDir screenshotsDirectory() const;
    QDir extrasDirectory() const;
    QString imageLocalPath(ImageType imageType, QUuid gameId) const;
    QUrl imageRemoteUrl(ImageType imageType, QUuid gameId) const;
    const MacroResolver* macroResolver() const;

    // Helper
    QString resolveAppPathOverrides(const QString& appPath) const;
    QString resolveExecSwaps(const QString& appPath, const QString& platform) const;
};

}



#endif // FLASHPOINT_INSTALL_H
