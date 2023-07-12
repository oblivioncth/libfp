// Unit Include
#include "fp/fp-install.h"

// Qx Includes
#include <qx/utility/qx-helpers.h>
#include <qx/core/qx-genericerror.h>

namespace Fp
{
//===============================================================================================================
// INSTALL
//===============================================================================================================

//-Constructor------------------------------------------------------------------------------------------------
//Public:
Install::Install(QString installPath) :
    mValid(false) // Install is invalid until proven otherwise
{
    QScopeGuard validityGuard([this](){ nullify(); }); // Automatically nullify on fail

    // Initialize static files and directories
    mRootDirectory = QDir(installPath);
    mLauncherFile = std::make_unique<QFile>(installPath + "/" + LAUNCHER_PATH);
    mDatabaseFile = std::make_unique<QFile>(installPath + "/" + DATABASE_PATH);
    mConfigJsonFile = std::make_shared<QFile>(installPath + "/" + CONFIG_JSON_PATH);
    mPreferencesJsonFile = std::make_shared<QFile>(installPath + "/" + PREFERENCES_JSON_PATH);
    mVersionFile = std::make_unique<QFile>(installPath + "/" + VER_TXT_PATH);
    mExtrasDirectory = QDir(installPath + "/" + EXTRAS_PATH);

    // Create macro resolver
    mMacroResolver = new MacroResolver(mRootDirectory.absolutePath(), {});

    //-Check install validity--------------------------------------------

    // Check for file existence
    const QList<const QFile*> filesToCheck{
        mDatabaseFile.get(),
        mConfigJsonFile.get(),
        mPreferencesJsonFile.get(),
        mLauncherFile.get(),
        mVersionFile.get(),
    };

    for(const QFile* file : filesToCheck)
    {
        QFileInfo fileInfo(*file);
        if(!fileInfo.exists() || !fileInfo.isFile())
        {
            mError = Qx::GenericError(Qx::Critical, 1, ERR_FILE_MISSING, fileInfo.filePath());
            return;
        }
    }

    // Get settings
    ConfigReader configReader(&mConfig, mConfigJsonFile);
    if((mError = configReader.readInto()).isValid())
        return;

    PreferencesReader prefReader(&mPreferences, mPreferencesJsonFile);
    if((mError = prefReader.readInto()).isValid())
        return;

    mServicesJsonFile = std::make_shared<QFile>(installPath + "/" + mPreferences.jsonFolderPath + "/" + SERVICES_JSON_NAME);
    mExecsJsonFile = std::make_shared<QFile>(installPath + "/" + mPreferences.jsonFolderPath + "/" + EXECS_JSON_NAME);
    mLogosDirectory = QDir(installPath + "/" + mPreferences.imageFolderPath + '/' + LOGOS_FOLDER_NAME);
    mScreenshotsDirectory = QDir(installPath + "/" + mPreferences.imageFolderPath + '/' + SCREENSHOTS_FOLDER_NAME);

    ServicesReader servicesReader(&mServices, mServicesJsonFile, mMacroResolver);
    if((mError = servicesReader.readInto()).isValid())
        return;

    if(mExecsJsonFile->exists()) // Optional
    {
        ExecsReader execsReader(&mExecs, mExecsJsonFile);
        if((mError = execsReader.readInto()).isValid())
            return;
    }

    // Add database
    mDatabase = new Db(mDatabaseFile->fileName(), {});

    if(!mDatabase->isValid())
    {
        mError = mDatabase->error();
        return;
    }

    // Give the OK
    mValid = true;
    validityGuard.dismiss();
}

//-Destructor------------------------------------------------------------------------------------------------
//Public:
Install::~Install()
{
    if(mMacroResolver)
        delete mMacroResolver;
    if(mDatabase)
        delete mDatabase;
}

//-Class Functions------------------------------------------------------------------------------------------------
//Private:
QString Install::standardImageSubPath(QUuid gameId)
{
    QString gameIdString = gameId.toString(QUuid::WithoutBraces);
    return gameIdString.left(2) + '/' + gameIdString.mid(2, 2) + '/' + gameIdString + IMAGE_EXT;
}

//Public:
Qx::Error Install::appInvolvesSecurePlayer(bool& involvesBuffer, QFileInfo appInfo)
{
    // Reset buffer
    involvesBuffer = false;

    if(appInfo.fileName().contains(SECURE_PLAYER_INFO.baseName()))
    {
        involvesBuffer = true;
        return Qx::Error();
    }
    else if(appInfo.suffix().compare("bat", Qt::CaseInsensitive) == 0)
    {
        // Check if bat uses secure player
        QFile batFile(appInfo.absoluteFilePath());
        Qx::IoOpReport readReport = Qx::fileContainsString(involvesBuffer, batFile, SECURE_PLAYER_INFO.baseName());

        // Check for read errors
        if(readReport.isFailure())
            return Qx::Error(readReport).setSeverity(Qx::Critical);
        else
            return Qx::Error();
    }
    else
        return Qx::Error();
}

//-Instance Functions------------------------------------------------------------------------------------------------
//Private:
void Install::nullify()
{
    // Files and directories
    mRootDirectory = QDir();
    mLogosDirectory = QDir();
    mScreenshotsDirectory = QDir();
    mExtrasDirectory = QDir();
    mLauncherFile.reset();
    mDatabaseFile.reset();
    mConfigJsonFile.reset();
    mPreferencesJsonFile.reset();
    mServicesJsonFile.reset();
    mVersionFile.reset();
    if(mMacroResolver)
        qxDelete(mMacroResolver);
    if(mDatabase)
        qxDelete(mDatabase);

    // Settings
    mConfig = {};
    mPreferences = {};
    mServices = {};
}

//Public:
bool Install::isValid() const { return mValid; }
Qx::Error Install::error() const { return mError; }

Install::Edition Install::edition() const
{
    QString nameVer = nameVersionString();

    return nameVer.contains("ultimate", Qt::CaseInsensitive) ? Edition::Ultimate :
           nameVer.contains("infinity", Qt::CaseInsensitive) ? Edition::Infinity :
                                                               Edition::Core;
}

QString Install::nameVersionString() const
{
    // Check version file (only read first line in case there is a trailing newline character)
    QString readVersion = QString();
    if(mVersionFile->exists())
        Qx::readTextFromFile(readVersion, *mVersionFile, Qx::TextPos::START, Qx::TextPos(Qx::Index32::FIRST, Qx::Index32::LAST));

    return readVersion;
}

Qx::VersionNumber Install::version() const
{
    QString nameVer = nameVersionString();
    QRegularExpressionMatch versionMatch = VERSION_NUMBER_REGEX.match(nameVer);

    if(versionMatch.hasMatch())
    {
        Qx::VersionNumber fpVersion = Qx::VersionNumber::fromString(versionMatch.captured("version"));
        if(!fpVersion.isNull())
            return fpVersion;
    }

    qWarning("Could not determine flashpoint version number!");
    return Qx::VersionNumber();
}

QString Install::launcherChecksum() const
{
    QString launcherHash;
    Qx::calculateFileChecksum(launcherHash, *mLauncherFile, QCryptographicHash::Sha256);

    return launcherHash;
}

Db* Install::database() { return mDatabase; }

const Config& Install::config() const { return mConfig; }
const Preferences& Install::preferences() const { return mPreferences; }
const Services& Install::services() const { return mServices; }
const Execs& Install::execs() const { return mExecs; }

QString Install::fullPath() const { return mRootDirectory.absolutePath(); }
QDir Install::logosDirectory() const { return mLogosDirectory; }
QDir Install::screenshotsDirectory() const { return mScreenshotsDirectory; }
QDir Install::extrasDirectory() const { return mExtrasDirectory; }

QString Install::imageLocalPath(ImageType imageType, QUuid gameId) const
{
    const QDir& sourceDir = imageType == ImageType::Logo ? mLogosDirectory : mScreenshotsDirectory;
    return sourceDir.absolutePath() + '/' + standardImageSubPath(gameId);
}

QUrl Install::imageRemoteUrl(ImageType imageType, QUuid gameId) const
{
    const QString typeFolder = (imageType == ImageType::Logo ? LOGOS_FOLDER_NAME : SCREENSHOTS_FOLDER_NAME);
    return QUrl(mPreferences.onDemandBaseUrl + typeFolder + '/' + standardImageSubPath(gameId));
}

const MacroResolver* Install::macroResolver() const { return mMacroResolver; }

QString Install::resolveAppPathOverrides(const QString& appPath) const
{
    // Check if path has an associated override
    for(const AppPathOverride& override : qAsConst(mPreferences.appPathOverrides))
    {
        if(override.path == appPath && override.enabled)
            return override.override;
    }

    return appPath;
}

QString Install::resolveExecSwaps(const QString& appPath, const QString& platform) const
{
    // Get swap preference
    bool preferNative = mPreferences.nativePlatforms.contains(platform);

    // Check if path has an associated swap
    for(const Exec& swap : qAsConst(mExecs.list))
    {
        if(swap.win32 == appPath)
        {
            if(preferNative && !swap.linux.isEmpty())
                return swap.linux;
            else if(!swap.wine.isEmpty())
                return swap.wine;
        }
    }

    return appPath;
}

}
