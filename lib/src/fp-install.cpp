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
            mError = InstallError(InstallError::FileMissing, fileInfo.filePath());
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

    // Ensure expected datapack source exists
    if(!mPreferences.gameDataSources.contains(MAIN_DATAPACK_SOURCE))
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
const MacroResolver* Install::macroResolver() const { return mMacroResolver; }
const Toolkit* Install::toolkit() const { return mToolkit; }

const Config& Install::config() const { return mConfig; }
const Preferences& Install::preferences() const { return mPreferences; }
const Services& Install::services() const { return mServices; }
const Execs& Install::execs() const { return mExecs; }
Daemon Install::outfittedDaemon() const { return mDaemon; }

QDir Install::dir() const { return mRootDirectory; }
QDir Install::entryLogosDirectory() const { return mEntryLogosDirectory; }
QDir Install::entryScreenshotsDirectory() const { return mEntryScreenshotsDirectory; }
QDir Install::extrasDirectory() const { return mExtrasDirectory; }
QDir Install::platformLogosDirectory() const { return mPlatformLogosDirectory; }

}
