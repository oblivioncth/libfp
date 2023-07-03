// Unit Include
#include "fp/fp-items.h"

// Qx Includes
#include <qx/core/qx-string.h>

namespace Fp
{

//===============================================================================================================
// Game
//===============================================================================================================

//-Constructor------------------------------------------------------------------------------------------------
//Public:
Game::Game() {}

//-Instance Functions------------------------------------------------------------------------------------------------
//Public:
QUuid Game::id() const { return mId; }
QString Game::title() const { return mTitle; }
QString Game::series() const { return mSeries; }
QString Game::developer() const { return mDeveloper; }
QString Game::publisher() const { return mPublisher; }
QDateTime Game::dateAdded() const { return mDateAdded; }
QDateTime Game::dateModified() const { return mDateModified; }
QString Game::playMode() const { return mPlayMode; }
bool Game::isBroken() const { return mBroken; }
QString Game::status() const { return mStatus; }
QString Game::notes() const{ return mNotes; }
QString Game::source() const { return mSource; }
QString Game::appPath() const { return mAppPath; }
QString Game::launchCommand() const { return mLaunchCommand; }
QDateTime Game::releaseDate() const { return mReleaseDate; }
QString Game::version() const { return mVersion; }
QString Game::originalDescription() const { return mOriginalDescription; }
QString Game::language() const { return mLanguage; }
QString Game::orderTitle() const { return mOrderTitle; }
QString Game::library() const { return mLibrary; }
QString Game::platformName() const { return mPlatformName; }

//===============================================================================================================
// Game::Builder
//===============================================================================================================

//-Constructor-------------------------------------------------------------------------------------------------
//Public:
Game::Builder::Builder() {}

//-Class Functions---------------------------------------------------------------------------------------------
//Private:
QString Game::Builder::kosherizeRawDate(QString date)
{
    static const QString DEFAULT_MONTH = "-01";
    static const QString DEFAULT_DAY = "-01";

    if(Qx::String::isOnlyNumbers(date) && date.length() == 4) // Year only
        return date + DEFAULT_MONTH + DEFAULT_DAY;
    else if(Qx::String::isOnlyNumbers(date.left(4)) &&
            Qx::String::isOnlyNumbers(date.mid(5,2)) &&
            date.at(4) == '-' && date.length() == 7) // Year and month only
        return  date + DEFAULT_DAY;
    else if(Qx::String::isOnlyNumbers(date.left(4)) &&
            Qx::String::isOnlyNumbers(date.mid(5,2)) &&
            Qx::String::isOnlyNumbers(date.mid(8,2)) &&
            date.at(4) == '-' && date.at(7) == '-' && date.length() == 10) // Year month and day
        return  date;
    else
        return QString(); // Invalid date provided
}

//-Instance Functions------------------------------------------------------------------------------------------
//Public:
Game::Builder& Game::Builder::wId(QString rawId) { mGameBlueprint.mId = QUuid(rawId); return *this; }
Game::Builder& Game::Builder::wTitle(QString title) { mGameBlueprint.mTitle = title; return *this; }
Game::Builder& Game::Builder::wSeries(QString series) { mGameBlueprint.mSeries = series; return *this; }
Game::Builder& Game::Builder::wDeveloper(QString developer) { mGameBlueprint.mDeveloper = developer; return *this; }
Game::Builder& Game::Builder::wPublisher(QString publisher) { mGameBlueprint.mPublisher = publisher; return *this; }
Game::Builder& Game::Builder::wDateAdded(QString rawDateAdded) { mGameBlueprint.mDateAdded = QDateTime::fromString(rawDateAdded, Qt::ISODateWithMs); return *this; }
Game::Builder& Game::Builder::wDateModified(QString rawDateModified) { mGameBlueprint.mDateModified = QDateTime::fromString(rawDateModified, Qt::ISODateWithMs); return *this; }
Game::Builder& Game::Builder::wBroken(QString rawBroken)  { mGameBlueprint.mBroken = rawBroken.toInt() != 0; return *this; }
Game::Builder& Game::Builder::wPlayMode(QString playMode) { mGameBlueprint.mPlayMode = playMode; return *this; }
Game::Builder& Game::Builder::wStatus(QString status) { mGameBlueprint.mStatus = status; return *this; }
Game::Builder& Game::Builder::wNotes(QString notes)  { mGameBlueprint.mNotes = notes; return *this; }
Game::Builder& Game::Builder::wSource(QString source)  { mGameBlueprint.mSource = source; return *this; }
Game::Builder& Game::Builder::wAppPath(QString appPath)  { mGameBlueprint.mAppPath = appPath; return *this; }
Game::Builder& Game::Builder::wLaunchCommand(QString launchCommand) { mGameBlueprint.mLaunchCommand = launchCommand; return *this; }
Game::Builder& Game::Builder::wReleaseDate(QString rawReleaseDate)  { mGameBlueprint.mReleaseDate = QDateTime::fromString(kosherizeRawDate(rawReleaseDate), Qt::ISODate); return *this; }
Game::Builder& Game::Builder::wVersion(QString version)  { mGameBlueprint.mVersion = version; return *this; }
Game::Builder& Game::Builder::wOriginalDescription(QString originalDescription)  { mGameBlueprint.mOriginalDescription = originalDescription; return *this; }
Game::Builder& Game::Builder::wLanguage(QString language)  { mGameBlueprint.mLanguage = language; return *this; }
Game::Builder& Game::Builder::wOrderTitle(QString orderTitle)  { mGameBlueprint.mOrderTitle = orderTitle; return *this; }
Game::Builder& Game::Builder::wLibrary(QString library) { mGameBlueprint.mLibrary = library; return *this; }
Game::Builder& Game::Builder::wPlatformName(QString platformName) { mGameBlueprint.mPlatformName = platformName; return *this; }

Game Game::Builder::build() { return mGameBlueprint; }

//===============================================================================================================
// AddApp
//===============================================================================================================

//-Constructor------------------------------------------------------------------------------------------------
//Public:
AddApp::AddApp() {}

//-Operators----------------------------------------------------------------------------------------------------
//Public:
bool operator== (const AddApp& lhs, const AddApp& rhs) noexcept
{
    return lhs.mId == rhs.mId &&
           lhs.mAppPath == rhs.mAppPath &&
           lhs.mAutorunBefore == rhs.mAutorunBefore &&
           lhs.mLaunchCommand == rhs.mLaunchCommand &&
           lhs.mName == rhs.mName &&
           lhs.mWaitExit == rhs.mWaitExit &&
           lhs.mParentId == rhs.mParentId;
}

//-Hashing------------------------------------------------------------------------------------------------------
uint qHash(const AddApp& key, uint seed) noexcept
{
    QtPrivate::QHashCombine hash;
    seed = hash(seed, key.mId);
    seed = hash(seed, key.mAppPath);
    seed = hash(seed, key.mAutorunBefore);
    seed = hash(seed, key.mLaunchCommand);
    seed = hash(seed, key.mName);
    seed = hash(seed, key.mWaitExit);
    seed = hash(seed, key.mParentId);

    return seed;
}

//-Instance Functions------------------------------------------------------------------------------------------------
//Public:
QUuid AddApp::id() const { return mId; }
QString AddApp::appPath() const { return mAppPath; }
bool AddApp::isAutorunBefore() const { return  mAutorunBefore; }
QString AddApp::launchCommand() const { return mLaunchCommand; }
QString AddApp::name() const { return mName; }
bool AddApp::isWaitExit() const { return mWaitExit; }
QUuid AddApp::parentId() const { return mParentId; }
bool AddApp::isPlayable() const { return mAppPath != SPEC_PATH_EXTRA && mAppPath != SPEC_PATH_MSG && !mAutorunBefore; }

//===============================================================================================================
// AddApp::Builder
//===============================================================================================================

//-Constructor-------------------------------------------------------------------------------------------------
//Public:
AddApp::Builder::Builder() {}

//-Instance Functions------------------------------------------------------------------------------------------
//Public:
AddApp::Builder& AddApp::Builder::wId(QString rawId) { mAddAppBlueprint.mId = QUuid(rawId); return *this; }
AddApp::Builder& AddApp::Builder::wAppPath(QString appPath) { mAddAppBlueprint.mAppPath = appPath; return *this; }
AddApp::Builder& AddApp::Builder::wAutorunBefore(QString rawAutorunBefore)  { mAddAppBlueprint.mAutorunBefore = rawAutorunBefore.toInt() != 0; return *this; }
AddApp::Builder& AddApp::Builder::wLaunchCommand(QString launchCommand) { mAddAppBlueprint.mLaunchCommand = launchCommand; return *this; }
AddApp::Builder& AddApp::Builder::wName(QString name) { mAddAppBlueprint.mName = name; return *this; }
AddApp::Builder& AddApp::Builder::wWaitExit(QString rawWaitExit)  { mAddAppBlueprint.mWaitExit = rawWaitExit.toInt() != 0; return *this; }
AddApp::Builder& AddApp::Builder::wParentId(QString rawParentId) { mAddAppBlueprint.mParentId = QUuid(rawParentId); return *this; }

AddApp AddApp::Builder::build() { return mAddAppBlueprint; }

//===============================================================================================================
// Set
//===============================================================================================================

//-Constructor------------------------------------------------------------------------------------------------
//Public:
Set::Set() {}

//-Instance Functions------------------------------------------------------------------------------------------------
//Public:
const Game& Set::game() const { return mGame; }
const QList<AddApp>& Set::addApps() const { return mAddApps; }

//===============================================================================================================
// Set::Builder
//===============================================================================================================

//-Constructor-------------------------------------------------------------------------------------------------
//Public:
Set::Builder::Builder() {}

//-Instance Functions------------------------------------------------------------------------------------------
//Public:
Set::Builder& Set::Builder::wGame(const Game& game) { mSetBlueprint.mGame = game; return *this; }
Set::Builder& Set::Builder::wAddApp(const AddApp& addApp) { mSetBlueprint.mAddApps.append(addApp); return *this; }
Set::Builder& Set::Builder::wAddApps(const QList<AddApp>& addApps) { mSetBlueprint.mAddApps.append(addApps); return *this; }

Set Set::Builder::build() { return mSetBlueprint; }

//===============================================================================================================
// Playlist
//===============================================================================================================

//-Constructor-------------------------------------------------------------------------------------------------
//Public:
Playlist::Playlist() {}

//-Instance Functions------------------------------------------------------------------------------------------------------
//Public:
QUuid Playlist::id() const { return mId; }
QString Playlist::title() const { return mTitle; }
QString Playlist::description() const { return mDescription; }
QString Playlist::author() const { return mAuthor; }

//===============================================================================================================
// Playlist::Builder
//===============================================================================================================

//-Constructor-------------------------------------------------------------------------------------------------
//Public:
Playlist::Builder::Builder() {}

//-Instance Functions------------------------------------------------------------------------------------------
//Public:
Playlist::Builder& Playlist::Builder::wId(QString rawId) { mPlaylistBlueprint.mId = QUuid(rawId); return *this; }
Playlist::Builder& Playlist::Builder::wTitle(QString title) { mPlaylistBlueprint.mTitle = title; return *this; }
Playlist::Builder& Playlist::Builder::wDescription(QString description) { mPlaylistBlueprint.mDescription = description; return *this; }
Playlist::Builder& Playlist::Builder::wAuthor(QString author) { mPlaylistBlueprint.mAuthor = author; return *this; }

Playlist Playlist::Builder::build() { return mPlaylistBlueprint; }

//===============================================================================================================
// PlaylistGame
//===============================================================================================================

//-Constructor------------------------------------------------------------------------------------------------
//Public:
PlaylistGame::PlaylistGame() {}

//-Instance Functions------------------------------------------------------------------------------------------------
//Public:

int PlaylistGame::id() const { return mId; }
QUuid PlaylistGame::playlistId() const { return mPlaylistId; }
int PlaylistGame::order() const { return mOrder; }
QUuid PlaylistGame::gameId() const { return mGameId; }

//===============================================================================================================
// PlaylistGame::Builder
//===============================================================================================================

//-Constructor-------------------------------------------------------------------------------------------------
//Public:
   PlaylistGame::Builder::Builder() {}

//-Instance Functions------------------------------------------------------------------------------------------
//Public:
    PlaylistGame::Builder& PlaylistGame::Builder::wId(QString rawId) { mPlaylistGameBlueprint.mId = rawId.toInt(); return *this; }
    PlaylistGame::Builder& PlaylistGame::Builder::wPlaylistId(QString rawPlaylistId) { mPlaylistGameBlueprint.mPlaylistId = QUuid(rawPlaylistId); return *this; }

    PlaylistGame::Builder& PlaylistGame::Builder::wOrder(QString rawOrder)
    {
        bool validInt = false;
        mPlaylistGameBlueprint.mOrder = rawOrder.toInt(&validInt);
        if(!validInt)
            mPlaylistGameBlueprint.mOrder = -1;

        return *this;
    }

    PlaylistGame::Builder& PlaylistGame::Builder::wGameId(QString rawGameId) { mPlaylistGameBlueprint.mGameId = QUuid(rawGameId); return *this; }

    PlaylistGame PlaylistGame::Builder::build() { return mPlaylistGameBlueprint; }
};
