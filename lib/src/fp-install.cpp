// Unit Include
#include "fp/fp-install.h"

// Qx Includes
#include <qx/utility/qx-helpers.h>

namespace Fp
{
//===============================================================================================================
// InstallError
//===============================================================================================================

//-Constructor-------------------------------------------------------------
//Private:
InstallError::InstallError(Type t, const QString& s) :
    mType(t),
    mSpecific(s)
{}

//-Instance Functions-------------------------------------------------------------
//Public:
bool InstallError::isValid() const { return mType != NoError; }
QString InstallError::specific() const { return mSpecific; }
InstallError::Type InstallError::type() const { return mType; }

//Private:
Qx::Severity InstallError::deriveSeverity() const { return Qx::Critical; }
quint32 InstallError::deriveValue() const { return mType; }
QString InstallError::derivePrimary() const { return ERR_STRINGS.value(mType); }
QString InstallError::deriveSecondary() const { return mSpecific; }

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
    mExtConfigJsonFile = std::make_shared<QFile>(installPath + u"/"_s + EXT_CONFIG_JSON_PATH);
    mPreferencesJsonFile = std::make_shared<QFile>(installPath + u"/"_s + PREFERENCES_JSON_PATH);
    mVersionFile = std::make_unique<QFile>(installPath + u"/"_s + VER_TXT_PATH);
    mExtrasDirectory = QDir(installPath + u"/"_s + EXTRAS_PATH);


    //-Check install validity--------------------------------------------

    // Check for file existence
    const QList<const QFile*> filesToCheck{
        mDatabaseFile.get(),
        mConfigJsonFile.get(),
        // mExtConfigJsonFile.get(), OPTIONAL
        mPreferencesJsonFile.get(),
        mLauncherFile.get(),
        mVersionFile.get(),
    };

    for(const QFile* file : filesToCheck)
    {
        QFileInfo fileInfo(*file);
        if(!fileInfo.exists() || !fileInfo.isFile())
        {
            mError = InstallError(InstallError::FileMissing, fileInfo.filePath());
            return;
        }
    }

    // Get config
    ConfigReader configReader(&mConfig, mConfigJsonFile);
    if((mError = configReader.readInto()).isValid())
        return;

    /* Create macro resolver
     *
     * NOTE: Macro resolver essentially just needs the FP root folder, but technically it's suppsoed to source this from
     * the value in config.json, which can be different than the actual root. Sometimes this causes shenanigans as the
     * value in config.json will be relative and therefore specific to the Flashpoint launcher folder, but the FP team
     * designs the releases knowing this so trusting that value is generally best.
     *
     * FIXME: The comments in the Launcher source make it clear that the <fpPath> macro is supposed to resolve to the
     * fpPath value in config.json, resolved to an absolute path; however, currently it does not actually resolve it to
     * an absolute path and substitutes it verbatim. I'd prefer to not do this, but due to a quirk realted to FlashpointGameServer
     * not accepting absolute paths for it's command line arguments, we must respect that behavior for things to work.
     * As soon as absolute paths are possible again, switch back to them.
     */
    //mMacroResolver = new MacroResolver(mRootDirectory.absolutePath(), {});
    mMacroResolver = new MacroResolver(mConfig.flashpointPath, {});

    // Get other settings
    PreferencesReader prefReader(&mPreferences, mPreferencesJsonFile);
    if((mError = prefReader.readInto()).isValid())
        return;

    ExtConfigReader extConfigReader(&mExtConfig, mExtConfigJsonFile);
    if((mError = extConfigReader.readInto()).isValid())
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

    // Read version info (assumes file exists from earlier, only read first line in case there is a trailing newline)
    QString verTxtStr;
    Qx::readTextFromFile(verTxtStr, *mVersionFile, Qx::Start, Qx::TextPos(Qx::First, Qx::Last));
    mVersionInfo = std::make_shared<VersionInfo>(verTxtStr);

    // Ensure expected datapack source exists on Infinity
    bool onlineEdition = !mVersionInfo->isNull() && (mVersionInfo->edition() == VersionInfo::Infinity || mVersionInfo->edition() == VersionInfo::Linux);
    bool canDownload = mPreferences.gameDataSources && mPreferences.gameDataSources->contains(MAIN_DATAPACK_SOURCE);
    // Could use Toolkit::canDownloadDatapacks() here, but we need to check for the main datasource specifically since that's all we're setup to handle
    if(onlineEdition && !canDownload)
    {
        mError = InstallError(InstallError::DatapackSourceMissing, MAIN_DATAPACK_SOURCE);
        return;
    }

    // Note daemon
    if(mServices.daemon.size() != 1)
    {
        mError = InstallError(InstallError::DaemonCountMismatch, "1");
        return;
    }


    establishDaemon();

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

    // Add toolkit
    mToolkit = new Toolkit(*this, {});

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
    if(mToolkit)
        delete mToolkit;
}

//-Instance Functions------------------------------------------------------------------------------------------------
//Private:
void Install::establishDaemon()
{
    Q_ASSERT(mServices.daemon.size() == 1);

    // NOTE: This assumes only one daemon is present!
    const ServerDaemon& d = *mServices.daemon.constBegin();

    if(d.name.contains(u"qemu"_s, Qt::CaseInsensitive) ||
        d.filename.contains(u"qemu"_s, Qt::CaseInsensitive))
        mDaemon = Qemu;
    else if(d.name.contains(u"docker"_s, Qt::CaseInsensitive) ||
             d.filename.contains(u"docker"_s, Qt::CaseInsensitive))
        mDaemon = Docker;
    else if(d.name.contains(u"proxy"_s, Qt::CaseInsensitive) ||
             d.filename.contains(u"proxy"_s, Qt::CaseInsensitive))
        mDaemon = FpProxy;
    else if(d.name.contains(u"game server"_s, Qt::CaseInsensitive) ||
             d.filename.contains(u"game server"_s, Qt::CaseInsensitive))
        mDaemon = FpGameServer;
    else
        mDaemon = Unknown;
}

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
    mVersionInfo.reset();
    if(mMacroResolver)
        qxDelete(mMacroResolver);
    if(mDatabase)
        qxDelete(mDatabase);
    if(mPlaylistManager)
        qxDelete(mPlaylistManager);
    if(mToolkit)
        qxDelete(mToolkit);

    // Settings
    mConfig = {};
    mPreferences = {};
    mServices = {};
}

//Public:
bool Install::isValid() const { return mValid; }
Qx::Error Install::error() const { return mError; }

std::shared_ptr<Install::VersionInfo> Install::versionInfo() const { return mVersionInfo; }

QString Install::launcherChecksum() const
{
    QString launcherHash;
    Qx::calculateFileChecksum(launcherHash, *mLauncherFile, QCryptographicHash::Sha256);

    return launcherHash;
}

Db* Install::database() { return mDatabase; }
PlaylistManager* Install::playlistManager() { return mPlaylistManager; }
const MacroResolver* Install::macroResolver() const { return mMacroResolver; }
const Toolkit* Install::toolkit() const { return mToolkit; }

const Config& Install::config() const { return mConfig; }
const ExtConfig& Install::extConfig() const { return mExtConfig; }
const Preferences& Install::preferences() const { return mPreferences; }
const Services& Install::services() const { return mServices; }
const Execs& Install::execs() const { return mExecs; }
Daemon Install::outfittedDaemon() const { return mDaemon; }

QDir Install::dir() const { return mRootDirectory; }
QDir Install::entryLogosDirectory() const { return mEntryLogosDirectory; }
QDir Install::entryScreenshotsDirectory() const { return mEntryScreenshotsDirectory; }
QDir Install::extrasDirectory() const { return mExtrasDirectory; }
QDir Install::platformLogosDirectory() const { return mPlatformLogosDirectory; }

//===============================================================================================================
// Install::VersionInfo
//===============================================================================================================


//-Constructor------------------------------------------------------------------------------------------------
//Public:
Install::VersionInfo::VersionInfo(const QString& verTxtStr) :
    mEdition(Edition::Unknown)
{
    QRegularExpressionMatch rem = VER_TXT_REGEX.match(verTxtStr);
    if(!rem.hasMatch() ||
       !(rem.hasCaptured(VER_TXT_GRP_EDITIONA) || rem.hasCaptured(VER_TXT_GRP_EDITIONB)) ||
       !rem.hasCaptured(VER_TXT_GRP_VERSION) ||
       !rem.hasCaptured(VER_TXT_GRP_NICK))
    {
        qWarning("Unknown version.txt format!");
        return;
    }

    mFullString = verTxtStr;
    QString edStr = rem.hasCaptured(VER_TXT_GRP_EDITIONA) ? rem.captured(VER_TXT_GRP_EDITIONA) : rem.captured(VER_TXT_GRP_EDITIONB);
    mEdition = edStr.contains(u"ultimate"_s, Qt::CaseInsensitive) ? Edition::Ultimate :
               edStr.contains(u"infinity"_s, Qt::CaseInsensitive) ? Edition::Infinity :
               edStr.contains(u"core"_s, Qt::CaseInsensitive) ? Edition::Core :
               edStr.contains(u"linux"_s, Qt::CaseInsensitive) ? Edition::Linux :
                                                                 Edition::Unknown;
    mVersion = Qx::VersionNumber::fromString(rem.captured(VER_TXT_GRP_VERSION));
    Q_ASSERT(!mVersion.isNull()); // Regex should fail before this
    mNickname = rem.captured(VER_TXT_GRP_NICK);
}

//-Instance Functions------------------------------------------------------------------------------------------------------
//Public:
bool Install::VersionInfo::isNull() const { return mFullString.isEmpty(); }
QString Install::VersionInfo::fullString() const { return mFullString; }
Install::VersionInfo::Edition Install::VersionInfo::edition() const { return mEdition; }
Qx::VersionNumber Install::VersionInfo::version() const { return mVersion; }
QString Install::VersionInfo::nickname() const { return mNickname; }

}
