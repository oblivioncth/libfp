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
Install::Install(QString installPath, bool preloadPlaylists) :
    mValid(false) // Install is invalid until proven otherwise
{
    QScopeGuard validityGuard([this](){ nullify(); }); // Automatically nullify on fail

    // Initialize static files and directories
    mRootDirectory = QDir(installPath);
    mLauncherFile = std::make_unique<QFile>(installPath + u"/"_s + LAUNCHER_PATH);
    mDatabaseFile = std::make_unique<QFile>(installPath + u"/"_s + DATABASE_PATH);
    mConfigJsonFile = std::make_shared<QFile>(installPath + u"/"_s + CONFIG_JSON_PATH);
    mPreferencesJsonFile = std::make_shared<QFile>(installPath + u"/"_s + PREFERENCES_JSON_PATH);
    mVersionFile = std::make_unique<QFile>(installPath + u"/"_s + VER_TXT_PATH);
    mExtrasDirectory = QDir(installPath + u"/"_s + EXTRAS_PATH);

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

    mServicesJsonFile = std::make_shared<QFile>(installPath + u"/"_s + mPreferences.jsonFolderPath + u"/"_s + SERVICES_JSON_NAME);
    mExecsJsonFile = std::make_shared<QFile>(installPath + u"/"_s + mPreferences.jsonFolderPath + u"/"_s + EXECS_JSON_NAME);
    mPlatformLogosDirectory = QDir(installPath + u"/"_s + mPreferences.logoFolderPath);
    mEntryLogosDirectory = QDir(installPath + u"/"_s + mPreferences.imageFolderPath + '/' + LOGOS_FOLDER_NAME);
    mEntryScreenshotsDirectory = QDir(installPath + u"/"_s + mPreferences.imageFolderPath + '/' + SCREENSHOTS_FOLDER_NAME);
    mPlaylistsDirectory = QDir(installPath + u"/"_s + mPreferences.playlistFolderPath);

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

    // Add playlists manager
    mPlaylistManager = new PlaylistManager(mPlaylistsDirectory, {});

    if(preloadPlaylists)
    {
        if(mError = mPlaylistManager->populate(); mError.isValid())
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
    if(mPlaylistManager)
        delete mPlaylistManager;
}

//-Class Functions------------------------------------------------------------------------------------------------
//Private:
QString Install::standardImageSubPath(QUuid gameId)
{
    QString gameIdString = gameId.toString(QUuid::WithoutBraces);
    return gameIdString.left(2) + '/' + gameIdString.mid(2, 2) + '/' + gameIdString;
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
    else if(appInfo.suffix().compare(u"bat"_s, Qt::CaseInsensitive) == 0)
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
    mPlatformLogosDirectory = QDir();
    mEntryLogosDirectory = QDir();
    mEntryScreenshotsDirectory = QDir();
    mExtrasDirectory = QDir();
    mPlaylistsDirectory = QDir();
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
    if(mPlaylistManager)
        qxDelete(mPlaylistManager);

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

    return nameVer.contains(u"ultimate"_s, Qt::CaseInsensitive) ? Edition::Ultimate :
           nameVer.contains(u"infinity"_s, Qt::CaseInsensitive) ? Edition::Infinity :
                                                               Edition::Core;
}

QString Install::nameVersionString() const
{
    // Check version file (only read first line in case there is a trailing newline character)
    QString readVersion = QString();
    if(mVersionFile->exists())
        Qx::readTextFromFile(readVersion, *mVersionFile, Qx::Start, Qx::TextPos(Qx::First, Qx::Last));

    return readVersion;
}

Qx::VersionNumber Install::version() const
{
    QString nameVer = nameVersionString();
    QRegularExpressionMatch versionMatch = VERSION_NUMBER_REGEX.match(nameVer);

    if(versionMatch.hasMatch())
    {
        Qx::VersionNumber fpVersion = Qx::VersionNumber::fromString(versionMatch.captured(u"version"_s));
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
PlaylistManager* Install::playlistManager() { return mPlaylistManager; }

const Config& Install::config() const { return mConfig; }
const Preferences& Install::preferences() const { return mPreferences; }
const Services& Install::services() const { return mServices; }
const Execs& Install::execs() const { return mExecs; }

QString Install::fullPath() const { return mRootDirectory.absolutePath(); }
QDir Install::entryLogosDirectory() const { return mEntryLogosDirectory; }
QDir Install::entryScreenshotsDirectory() const { return mEntryScreenshotsDirectory; }
QDir Install::extrasDirectory() const { return mExtrasDirectory; }

QString Install::platformLogoPath(const QString& platform)
{
    QString path = mPlatformLogosDirectory.absoluteFilePath(platform + IMAGE_UC_EXT);
    return QFile::exists(path) ? path : QString();
}

QString Install::entryImageLocalPath(ImageType imageType, const QUuid& gameId) const
{
    // Defaults to using compression if the setting isn't present
    const QDir& sourceDir = imageType == ImageType::Logo ? mEntryLogosDirectory : mEntryScreenshotsDirectory;
    bool compressed = !mPreferences.onDemandImagesCompressed.has_value() || mPreferences.onDemandImagesCompressed.value();
    QString localSubPath = standardImageSubPath(gameId) + (compressed ? IMAGE_C_EXT : IMAGE_UC_EXT);

    return sourceDir.absolutePath() + '/' + localSubPath;
}

QUrl Install::entryImageRemoteUrl(ImageType imageType, const QUuid& gameId) const
{
    // Defaults to using compression if the setting isn't present
    const QString typeFolder = (imageType == ImageType::Logo ? LOGOS_FOLDER_NAME : SCREENSHOTS_FOLDER_NAME);
    bool compressed = !mPreferences.onDemandImagesCompressed.has_value() || mPreferences.onDemandImagesCompressed.value();
    QString remoteSubPath = standardImageSubPath(gameId) + IMAGE_UC_EXT;

    if(compressed)
        remoteSubPath += IMAGE_C_URL_SUFFIX;

    return QUrl(mPreferences.onDemandBaseUrl + typeFolder + '/' + remoteSubPath);
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
    for(const Exec& swap : qAsConst(mExecs.execs))
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
