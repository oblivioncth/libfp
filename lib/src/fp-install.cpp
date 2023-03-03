// Unit Include
#include "fp/flashpoint/fp-install.h"

// Qx Includes
#include <qx/io/qx-common-io.h>
#include <qx/utility/qx-helpers.h>

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
            mError = Qx::GenericError(Qx::GenericError::Critical, ERR_INVALID, FILE_DNE.arg(fileInfo.filePath()));
            return;
        }
    }

    // Get settings
    Qx::GenericError readReport;

    Json::ConfigReader configReader(&mConfig, mConfigJsonFile);
    if((readReport = configReader.readInto()).isValid())
    {
        mError = Qx::GenericError(Qx::GenericError::Critical, ERR_INVALID, readReport.primaryInfo() + " [" + readReport.secondaryInfo() + "]");
        return;
    }

    Json::PreferencesReader prefReader(&mPreferences, mPreferencesJsonFile);
    if((readReport = prefReader.readInto()).isValid())
    {
        mError = Qx::GenericError(Qx::GenericError::Critical, ERR_INVALID, readReport.primaryInfo() + " [" + readReport.secondaryInfo() + "]");
        return;
    }
    mServicesJsonFile = std::make_shared<QFile>(installPath + "/" + mPreferences.jsonFolderPath + "/" + SERVICES_JSON_NAME);
    mExecsJsonFile = std::make_shared<QFile>(installPath + "/" + mPreferences.jsonFolderPath + "/" + EXECS_JSON_NAME);
    mLogosDirectory = QDir(installPath + "/" + mPreferences.imageFolderPath + '/' + LOGOS_FOLDER_NAME);
    mScreenshotsDirectory = QDir(installPath + "/" + mPreferences.imageFolderPath + '/' + SCREENSHOTS_FOLDER_NAME);

    Json::ServicesReader servicesReader(&mServices, mServicesJsonFile, mMacroResolver);
    if((readReport = servicesReader.readInto()).isValid())
    {
        mError = Qx::GenericError(Qx::GenericError::Critical, ERR_INVALID, readReport.primaryInfo() + " [" + readReport.secondaryInfo() + "]");
        return;
    }

    if(mExecsJsonFile->exists()) // Optional
    {
        Json::ExecsReader execsReader(&mExecs, mExecsJsonFile);
        if((readReport = execsReader.readInto()).isValid())
        {
            mError = Qx::GenericError(Qx::GenericError::Critical, ERR_INVALID, readReport.primaryInfo() + " [" + readReport.secondaryInfo() + "]");
            return;
        }
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
QString Install::standardImageSubPath(ImageType imageType, QUuid gameId)
{
    QString gameIdString = gameId.toString(QUuid::WithoutBraces);
    return gameIdString.left(2) + '/' + gameIdString.mid(2, 2) + '/' + gameIdString + IMAGE_EXT;
}

//Public:
Qx::GenericError Install::appInvolvesSecurePlayer(bool& involvesBuffer, QFileInfo appInfo)
{
    // Reset buffer
    involvesBuffer = false;

    if(appInfo.fileName().contains(SECURE_PLAYER_INFO.baseName()))
    {
        involvesBuffer = true;
        return Qx::GenericError();
    }
    else if(appInfo.suffix().compare("bat", Qt::CaseInsensitive) == 0)
    {
        // Check if bat uses secure player
        QFile batFile(appInfo.absoluteFilePath());
        Qx::IoOpReport readReport = Qx::fileContainsString(involvesBuffer, batFile, SECURE_PLAYER_INFO.baseName());

        // Check for read errors
        if(readReport.isFailure())
            return Qx::GenericError(Qx::GenericError::Critical, readReport.outcome(), readReport.outcomeInfo());
        else
            return Qx::GenericError();
    }
    else
        return Qx::GenericError();
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
Qx::GenericError Install::error() const { return mError; }

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

const Json::Config& Install::config() const { return mConfig; }
const Json::Preferences& Install::preferences() const { return mPreferences; }
const Json::Services& Install::services() const { return mServices; }
const Json::Execs& Install::execs() const { return mExecs; }

QString Install::fullPath() const { return mRootDirectory.absolutePath(); }
QDir Install::logosDirectory() const { return mLogosDirectory; }
QDir Install::screenshotsDirectory() const { return mScreenshotsDirectory; }
QDir Install::extrasDirectory() const { return mExtrasDirectory; }

QString Install::imageLocalPath(ImageType imageType, QUuid gameId) const
{
    const QDir& sourceDir = imageType == ImageType::Logo ? mLogosDirectory : mScreenshotsDirectory;
    return sourceDir.absolutePath() + '/' + standardImageSubPath(imageType, gameId);
}

QUrl Install::imageRemoteUrl(ImageType imageType, QUuid gameId) const
{
    const QString typeFolder = (imageType == ImageType::Logo ? LOGOS_FOLDER_NAME : SCREENSHOTS_FOLDER_NAME);
    return QUrl(mPreferences.onDemandBaseUrl + typeFolder + '/' + standardImageSubPath(imageType, gameId));
}

const MacroResolver* Install::macroResolver() const { return mMacroResolver; }

QString Install::resolveAppPathOverrides(const QString& appPath) const
{
    // Check if path has an associated override
    for(const Json::AppPathOverride& override : qAsConst(mPreferences.appPathOverrides))
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
    for(const Json::Exec& swap : qAsConst(mExecs.list))
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
