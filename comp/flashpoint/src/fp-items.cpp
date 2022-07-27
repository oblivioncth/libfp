// Unit Include
#include "fp/flashpoint/fp-items.h"

// Qx Includes
#include <qx/core/qx-string.h>

namespace Fp
{

//===============================================================================================================
// GAME
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
QString Game::platform() const { return mPlatform; }
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

//===============================================================================================================
// GAME BUILDER
//===============================================================================================================

//-Constructor-------------------------------------------------------------------------------------------------
//Public:
GameBuilder::GameBuilder() {}

//-Class Functions---------------------------------------------------------------------------------------------
//Private:
QString GameBuilder::kosherizeRawDate(QString date)
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
GameBuilder& GameBuilder::wId(QString rawId) { mGameBlueprint.mId = QUuid(rawId); return *this; }
GameBuilder& GameBuilder::wTitle(QString title) { mGameBlueprint.mTitle = title; return *this; }
GameBuilder& GameBuilder::wSeries(QString series) { mGameBlueprint.mSeries = series; return *this; }
GameBuilder& GameBuilder::wDeveloper(QString developer) { mGameBlueprint.mDeveloper = developer; return *this; }
GameBuilder& GameBuilder::wPublisher(QString publisher) { mGameBlueprint.mPublisher = publisher; return *this; }
GameBuilder& GameBuilder::wDateAdded(QString rawDateAdded) { mGameBlueprint.mDateAdded = QDateTime::fromString(rawDateAdded, Qt::ISODateWithMs); return *this; }
GameBuilder& GameBuilder::wDateModified(QString rawDateModified) { mGameBlueprint.mDateModified = QDateTime::fromString(rawDateModified, Qt::ISODateWithMs); return *this; }
GameBuilder& GameBuilder::wPlatform(QString platform) { mGameBlueprint.mPlatform = platform; return *this; }
GameBuilder& GameBuilder::wBroken(QString rawBroken)  { mGameBlueprint.mBroken = rawBroken.toInt() != 0; return *this; }
GameBuilder& GameBuilder::wPlayMode(QString playMode) { mGameBlueprint.mPlayMode = playMode; return *this; }
GameBuilder& GameBuilder::wStatus(QString status) { mGameBlueprint.mStatus = status; return *this; }
GameBuilder& GameBuilder::wNotes(QString notes)  { mGameBlueprint.mNotes = notes; return *this; }
GameBuilder& GameBuilder::wSource(QString source)  { mGameBlueprint.mSource = source; return *this; }
GameBuilder& GameBuilder::wAppPath(QString appPath)  { mGameBlueprint.mAppPath = appPath; return *this; }
GameBuilder& GameBuilder::wLaunchCommand(QString launchCommand) { mGameBlueprint.mLaunchCommand = launchCommand; return *this; }
GameBuilder& GameBuilder::wReleaseDate(QString rawReleaseDate)  { mGameBlueprint.mReleaseDate = QDateTime::fromString(kosherizeRawDate(rawReleaseDate), Qt::ISODate); return *this; }
GameBuilder& GameBuilder::wVersion(QString version)  { mGameBlueprint.mVersion = version; return *this; }
GameBuilder& GameBuilder::wOriginalDescription(QString originalDescription)  { mGameBlueprint.mOriginalDescription = originalDescription; return *this; }
GameBuilder& GameBuilder::wLanguage(QString language)  { mGameBlueprint.mLanguage = language; return *this; }
GameBuilder& GameBuilder::wOrderTitle(QString orderTitle)  { mGameBlueprint.mOrderTitle = orderTitle; return *this; }
GameBuilder& GameBuilder::wLibrary(QString library) { mGameBlueprint.mLibrary = library; return *this; }

Game GameBuilder::build() { return mGameBlueprint; }

//===============================================================================================================
// ADD APP
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
// ADD APP BUILDER
//===============================================================================================================

//-Constructor-------------------------------------------------------------------------------------------------
//Public:
AddAppBuilder::AddAppBuilder() {}

//-Instance Functions------------------------------------------------------------------------------------------
//Public:
AddAppBuilder& AddAppBuilder::wId(QString rawId) { mAddAppBlueprint.mId = QUuid(rawId); return *this; }
AddAppBuilder& AddAppBuilder::wAppPath(QString appPath) { mAddAppBlueprint.mAppPath = appPath; return *this; }
AddAppBuilder& AddAppBuilder::wAutorunBefore(QString rawAutorunBefore)  { mAddAppBlueprint.mAutorunBefore = rawAutorunBefore.toInt() != 0; return *this; }
AddAppBuilder& AddAppBuilder::wLaunchCommand(QString launchCommand) { mAddAppBlueprint.mLaunchCommand = launchCommand; return *this; }
AddAppBuilder& AddAppBuilder::wName(QString name) { mAddAppBlueprint.mName = name; return *this; }
AddAppBuilder& AddAppBuilder::wWaitExit(QString rawWaitExit)  { mAddAppBlueprint.mWaitExit = rawWaitExit.toInt() != 0; return *this; }
AddAppBuilder& AddAppBuilder::wParentId(QString rawParentId) { mAddAppBlueprint.mParentId = QUuid(rawParentId); return *this; }

AddApp AddAppBuilder::build() { return mAddAppBlueprint; }

//===============================================================================================================
// Set
//===============================================================================================================

//-Constructor------------------------------------------------------------------------------------------------
//Public:
Set::Set() {}

//-Instance Functions------------------------------------------------------------------------------------------------
//Public:
Game Set::game() { return mGame; }
QList<AddApp> Set::addApps() { return mAddApps; }

//===============================================================================================================
// SetBuilder
//===============================================================================================================

//-Constructor-------------------------------------------------------------------------------------------------
//Public:
SetBuilder::SetBuilder() {}

//-Instance Functions------------------------------------------------------------------------------------------
//Public:
SetBuilder& SetBuilder::wGame(const Game& game) { mSetBlueprint.mGame = game; return *this; }
SetBuilder& SetBuilder::wAddApp(const AddApp& addApp) { mSetBlueprint.mAddApps.append(addApp); return *this; }
SetBuilder& SetBuilder::wAddApps(const QList<AddApp>& addApps) { mSetBlueprint.mAddApps.append(addApps); return *this; }

Set SetBuilder::build() { return mSetBlueprint; }

//===============================================================================================================
// PLAYLIST
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
// PLAYLIST BUILDER
//===============================================================================================================

//-Constructor-------------------------------------------------------------------------------------------------
//Public:
PlaylistBuilder::PlaylistBuilder() {}

//-Instance Functions------------------------------------------------------------------------------------------
//Public:
PlaylistBuilder& PlaylistBuilder::wId(QString rawId) { mPlaylistBlueprint.mId = QUuid(rawId); return *this; }
PlaylistBuilder& PlaylistBuilder::wTitle(QString title) { mPlaylistBlueprint.mTitle = title; return *this; }
PlaylistBuilder& PlaylistBuilder::wDescription(QString description) { mPlaylistBlueprint.mDescription = description; return *this; }
PlaylistBuilder& PlaylistBuilder::wAuthor(QString author) { mPlaylistBlueprint.mAuthor = author; return *this; }

Playlist PlaylistBuilder::build() { return mPlaylistBlueprint; }

//===============================================================================================================
// PLAYLIST GAME
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
// PLAYLIST GAME BUILDER
//===============================================================================================================

//-Constructor-------------------------------------------------------------------------------------------------
//Public:
   PlaylistGameBuilder::PlaylistGameBuilder() {}

//-Instance Functions------------------------------------------------------------------------------------------
//Public:
    PlaylistGameBuilder& PlaylistGameBuilder::wId(QString rawId) { mPlaylistGameBlueprint.mId = rawId.toInt(); return *this; }
    PlaylistGameBuilder& PlaylistGameBuilder::wPlaylistId(QString rawPlaylistId) { mPlaylistGameBlueprint.mPlaylistId = QUuid(rawPlaylistId); return *this; }

    PlaylistGameBuilder& PlaylistGameBuilder::wOrder(QString rawOrder)
    {
        bool validInt = false;
        mPlaylistGameBlueprint.mOrder = rawOrder.toInt(&validInt);
        if(!validInt)
            mPlaylistGameBlueprint.mOrder = -1;

        return *this;
    }

    PlaylistGameBuilder& PlaylistGameBuilder::wGameId(QString rawGameId) { mPlaylistGameBlueprint.mGameId = QUuid(rawGameId); return *this; }

    PlaylistGame PlaylistGameBuilder::build() { return mPlaylistGameBlueprint; }
};
