// Unit Include
#include "fp/fp-items.h"

// Qt Includes
#include <QCommandLineParser>
#include <QProcess>

// Qx Includes
#include <qx/core/qx-string.h>
#include <qx/core/qx-algorithm.h>

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
QString Game::ruffleSupport() const { return mRuffleSupport; }

//===============================================================================================================
// Game::Builder
//===============================================================================================================

//-Constructor-------------------------------------------------------------------------------------------------
//Public:
Game::Builder::Builder() {}

//-Class Functions---------------------------------------------------------------------------------------------
//Private:
QString Game::Builder::kosherizeRawDate(const QString& date)
{
    static const QString DEFAULT_MONTH = u"-01"_s;
    static const QString DEFAULT_DAY = u"-01"_s;

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
Game::Builder& Game::Builder::wId(QStringView rawId) { mGameBlueprint.mId = QUuid(rawId); return *this; }
Game::Builder& Game::Builder::wTitle(const QString& title) { mGameBlueprint.mTitle = title; return *this; }
Game::Builder& Game::Builder::wSeries(const QString& series) { mGameBlueprint.mSeries = series; return *this; }
Game::Builder& Game::Builder::wDeveloper(const QString& developer) { mGameBlueprint.mDeveloper = developer; return *this; }
Game::Builder& Game::Builder::wPublisher(const QString& publisher) { mGameBlueprint.mPublisher = publisher; return *this; }
Game::Builder& Game::Builder::wDateAdded(QStringView rawDateAdded) { mGameBlueprint.mDateAdded = QDateTime::fromString(rawDateAdded, Qt::ISODateWithMs); return *this; }
Game::Builder& Game::Builder::wDateModified(QStringView rawDateModified) { mGameBlueprint.mDateModified = QDateTime::fromString(rawDateModified, Qt::ISODateWithMs); return *this; }
Game::Builder& Game::Builder::wBroken(QStringView rawBroken)  { mGameBlueprint.mBroken = rawBroken.toInt() != 0; return *this; }
Game::Builder& Game::Builder::wPlayMode(const QString& playMode) { mGameBlueprint.mPlayMode = playMode; return *this; }
Game::Builder& Game::Builder::wStatus(const QString& status) { mGameBlueprint.mStatus = status; return *this; }
Game::Builder& Game::Builder::wNotes(const QString& notes)  { mGameBlueprint.mNotes = notes; return *this; }
Game::Builder& Game::Builder::wSource(const QString& source)  { mGameBlueprint.mSource = source; return *this; }
Game::Builder& Game::Builder::wAppPath(const QString& appPath)  { mGameBlueprint.mAppPath = appPath; return *this; }
Game::Builder& Game::Builder::wLaunchCommand(const QString& launchCommand) { mGameBlueprint.mLaunchCommand = launchCommand; return *this; }
Game::Builder& Game::Builder::wReleaseDate(QStringView rawReleaseDate)  { mGameBlueprint.mReleaseDate = QDateTime::fromString(kosherizeRawDate(rawReleaseDate.toString()), Qt::ISODate); return *this; }
Game::Builder& Game::Builder::wVersion(const QString& version)  { mGameBlueprint.mVersion = version; return *this; }
Game::Builder& Game::Builder::wOriginalDescription(const QString& originalDescription)  { mGameBlueprint.mOriginalDescription = originalDescription; return *this; }
Game::Builder& Game::Builder::wLanguage(const QString& language)  { mGameBlueprint.mLanguage = language; return *this; }
Game::Builder& Game::Builder::wOrderTitle(const QString& orderTitle)  { mGameBlueprint.mOrderTitle = orderTitle; return *this; }
Game::Builder& Game::Builder::wLibrary(const QString& library) { mGameBlueprint.mLibrary = library; return *this; }
Game::Builder& Game::Builder::wPlatformName(const QString& platformName) { mGameBlueprint.mPlatformName = platformName; return *this; }
Game::Builder& Game::Builder::wRuffleSupport(const QString& ruffleSupport) { mGameBlueprint.mRuffleSupport = ruffleSupport; return *this; }

Game Game::Builder::build() { return mGameBlueprint; }

//===============================================================================================================
// GameDataParameters
//===============================================================================================================

//-Constructor------------------------------------------------------------------------------------------------
//Public:
GameDataParameters::GameDataParameters(const QString& rawParameters)
{
    static const auto OPT_EXTRACT = QCommandLineOption(u"extract"_s);
    static const auto OPT_EXTRACTED = QCommandLineOption(u"extracted"_s, {}, u"extracted"_s); // Takes value
    static const auto OPT_SERVER = QCommandLineOption(u"server"_s, {}, u"server"_s); // Takes value

    QCommandLineParser parser;
    parser.setSingleDashWordOptionMode(QCommandLineParser::ParseAsLongOptions);
    bool optAdd = parser.addOptions({
        OPT_EXTRACT,
        OPT_EXTRACTED,
        OPT_SERVER
    });
    Q_ASSERT(optAdd);

    // Determine params
    QStringList param{u"GDP"_s}; // Need to add dummy "executable name" for QCommandLineParser, it's ignored
    param.append(QProcess::splitCommand(rawParameters));

    // Parse
    if(!parser.parse(param))
        mErrorStr = parser.errorText();
    QStringList posArgs = parser.positionalArguments();
    if(!posArgs.isEmpty())
    {
        if(!mErrorStr.isEmpty())
            mErrorStr += ' ';
        mErrorStr += u"Unexpected positional arguments: {"_s + posArgs.join(',') + u"}."_s;
    }

    // Set values
    mExtract = parser.isSet(OPT_EXTRACT);
    mExtractedMarkerFile = parser.value(OPT_EXTRACTED);
    mServer = parser.value(OPT_SERVER);
}

//-Instance Functions------------------------------------------------------------------------------------------------
//Public:
bool GameDataParameters::isExtract() const { return mExtract; }
QString GameDataParameters::extractedMarkerFile() const { return mExtractedMarkerFile; }
QString GameDataParameters::server() const { return mServer; }

bool GameDataParameters::hasError() const { return !mErrorStr.isEmpty(); }
QString GameDataParameters::errorString() const { return mErrorStr; }

//===============================================================================================================
// GameData
//===============================================================================================================

//-Constructor------------------------------------------------------------------------------------------------
//Public:
GameData::GameData() :
    mNull(true)
{}

//-Instance Functions------------------------------------------------------------------------------------------------
//Public:
bool GameData::isNull() const { return mNull; }

quint32 GameData::id() const { return mId; }
QUuid GameData::gameId() const { return mGameId; }
QString GameData::title() const { return mTitle; }
QDateTime GameData::dateAdded() const { return mDateAdded; }
QString GameData::sha256() const { return mSha256; }
quint32 GameData::crc32() const { return mCrc32; }
bool GameData::presentOnDisk() const { return mPresentOnDisk; }
QString GameData::path() const { return mPath; }
quint32 GameData::size() const { return mSize; }
QString GameData::rawParameters() const { return mRawParameters; }
GameDataParameters GameData::parameters() const { return GameDataParameters(mRawParameters); }
QString GameData::appPath() const { return mAppPath; }
QString GameData::launchCommand() const { return mLaunchCommand; }

//===============================================================================================================
// GameData::Builder
//===============================================================================================================

//-Constructor-------------------------------------------------------------------------------------------------
//Public:
GameData::Builder::Builder() {}

//-Instance Functions------------------------------------------------------------------------------------------
//Public:
GameData::Builder& GameData::Builder::wId(QStringView rawId) { mGameDataBlueprint.mId = rawId.toInt(); return *this; }
GameData::Builder& GameData::Builder::wGameId(QStringView rawId) { mGameDataBlueprint.mGameId = QUuid(rawId); return *this; }
GameData::Builder& GameData::Builder::wTitle(const QString& title) { mGameDataBlueprint.mTitle = title; return *this; }

GameData::Builder& GameData::Builder::wDateAdded(const QString& rawDateAdded)
{
    /* Have to bodge fractional time because of: https://github.com/qt/qtbase/blob/4e7f5c43a3be609502ccc15861319503dc2c842b/src/corelib/time/qdatetime.cpp#L2515
     * It seems that the converter was supposed to floor to the previous MS (which is what JS's Date() does), but qRound() was accidentally used,
     * and then aftewards additional functionality/tests were built around the fact that the conversion can round up, so a change being accepted
     * is potentially unlikely
     */

    QString cleanDate = rawDateAdded;
    if(!cleanDate.endsWith('z', Qt::CaseInsensitive))
        cleanDate.append('Z');// Times should always been in UTC
    int dotPos = cleanDate.indexOf('.'); // Fractional time (could be comma, but seems to be unused in FP)
    if(dotPos != -1)
    {
        // Ignore fractional precision past whole MS (how JS's Date() handles it
        constexpr int MAX_DECIMALS = 3;
        int decimalStart = dotPos + 1;
        int decimalEnd = cleanDate.size() - 2; // Extra -1 because of 'Z'
        int decimalPlaces = Qx::length(decimalStart, decimalEnd);
        if(decimalPlaces > MAX_DECIMALS)
        {
            int extra = decimalPlaces - MAX_DECIMALS;
            cleanDate.remove(decimalStart + MAX_DECIMALS, extra);
        }

    }
    mGameDataBlueprint.mDateAdded = QDateTime::fromString(cleanDate, Qt::ISODateWithMs);

    return *this;
}

GameData::Builder& GameData::Builder::wSha256(const QString& sha256) { mGameDataBlueprint.mSha256 = sha256; return *this; }
GameData::Builder& GameData::Builder::wCrc32(QStringView rawCrc32) { mGameDataBlueprint.mCrc32 = rawCrc32.toInt(); return *this; }
GameData::Builder& GameData::Builder::wPresentOnDisk(QStringView rawBroken) { mGameDataBlueprint.mPresentOnDisk = rawBroken.toInt() != 0; return *this; }
GameData::Builder& GameData::Builder::wPath(const QString& path) { mGameDataBlueprint.mPath = path; return *this; }
GameData::Builder& GameData::Builder::wSize(QStringView rawSize) { mGameDataBlueprint.mSize = rawSize.toInt(); return *this; }
GameData::Builder& GameData::Builder::wRawParameters(const QString& parameters) { mGameDataBlueprint.mRawParameters = parameters; return *this; }
GameData::Builder& GameData::Builder::wAppPath(const QString& appPath) { mGameDataBlueprint.mAppPath = appPath; return *this; }
GameData::Builder& GameData::Builder::wLaunchCommand(const QString& launchCommand) { mGameDataBlueprint.mLaunchCommand = launchCommand; return *this; }

GameData GameData::Builder::build() { mGameDataBlueprint.mNull = false; return mGameDataBlueprint; }

//===============================================================================================================
// GameTags
//===============================================================================================================

//-Constructor------------------------------------------------------------------------------------------------
//Public:
GameTags::GameTags() {}

//-Instance Functions------------------------------------------------------------------------------------------------
//Public:
QStringList GameTags::tags() const
{
    QStringList all;
    for(const QStringList& tl : mTags)
        all.append(tl);

    return all;
}

QStringList GameTags::tags(const QString& category) const { return mTags.value(category); }

//===============================================================================================================
// GameTags::Builder
//===============================================================================================================

//-Constructor-------------------------------------------------------------------------------------------------
//Public:
GameTags::Builder::Builder() {}

//-Instance Functions------------------------------------------------------------------------------------------
//Public:
GameTags::Builder& GameTags::Builder::wTag(const QString& genre, const QString& tag) { mGameTagsBlueprint.mTags[genre].append(tag); return *this; }

GameTags GameTags::Builder::build() { return mGameTagsBlueprint; }

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
size_t qHash(const AddApp& key, size_t seed) noexcept
{
    return qHashMulti(seed,
        key.mId,
        key.mAppPath,
        key.mAutorunBefore,
        key.mLaunchCommand,
        key.mName,
        key.mWaitExit,
        key.mParentId
    );
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
AddApp::Builder& AddApp::Builder::wId(QStringView rawId) { mAddAppBlueprint.mId = QUuid(rawId); return *this; }
AddApp::Builder& AddApp::Builder::wAppPath(const QString& appPath) { mAddAppBlueprint.mAppPath = appPath; return *this; }
AddApp::Builder& AddApp::Builder::wAutorunBefore(QStringView rawAutorunBefore)  { mAddAppBlueprint.mAutorunBefore = rawAutorunBefore.toInt() != 0; return *this; }
AddApp::Builder& AddApp::Builder::wLaunchCommand(const QString& launchCommand) { mAddAppBlueprint.mLaunchCommand = launchCommand; return *this; }
AddApp::Builder& AddApp::Builder::wName(const QString& name) { mAddAppBlueprint.mName = name; return *this; }
AddApp::Builder& AddApp::Builder::wWaitExit(QStringView rawWaitExit)  { mAddAppBlueprint.mWaitExit = rawWaitExit.toInt() != 0; return *this; }
AddApp::Builder& AddApp::Builder::wParentId(QStringView rawParentId) { mAddAppBlueprint.mParentId = QUuid(rawParentId); return *this; }

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
const GameTags& Set::tags() const { return mTags; }
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
Set::Builder& Set::Builder::wTags(const GameTags& tags) { mSetBlueprint.mTags = tags; return *this; }
Set::Builder& Set::Builder::wAddApp(const AddApp& addApp) { mSetBlueprint.mAddApps.append(addApp); return *this; }
Set::Builder& Set::Builder::wAddApps(const QList<AddApp>& addApps) { mSetBlueprint.mAddApps.append(addApps); return *this; }

Set Set::Builder::build() { return mSetBlueprint; }

//===============================================================================================================
// PlaylistGame
//===============================================================================================================

//-Constructor------------------------------------------------------------------------------------------------
//Public:
PlaylistGame::PlaylistGame() {}

//-Instance Functions------------------------------------------------------------------------------------------------
//Public:

std::optional<int> PlaylistGame::id() const { return mId; }
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
PlaylistGame::Builder& PlaylistGame::Builder::wId(std::optional<int> id) { mPlaylistGameBlueprint.mId = id; return *this; }
PlaylistGame::Builder& PlaylistGame::Builder::wPlaylistId(QStringView rawPlaylistId) { mPlaylistGameBlueprint.mPlaylistId = QUuid(rawPlaylistId); return *this; }
PlaylistGame::Builder& PlaylistGame::Builder::wOrder(int order) { mPlaylistGameBlueprint.mOrder = order; return *this; }
PlaylistGame::Builder& PlaylistGame::Builder::wGameId(QStringView rawGameId) { mPlaylistGameBlueprint.mGameId = QUuid(rawGameId); return *this; }

PlaylistGame PlaylistGame::Builder::build() { return mPlaylistGameBlueprint; }

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
QString Playlist::library() const { return mLibrary; }
QImage Playlist::icon() const { return mIcon; }
const QList<PlaylistGame>& Playlist::playlistGames() const { return mPlaylistGames; }
QList<PlaylistGame>& Playlist::playlistGames() { return mPlaylistGames; }

//===============================================================================================================
// Playlist::Builder
//===============================================================================================================

//-Constructor-------------------------------------------------------------------------------------------------
//Public:
Playlist::Builder::Builder() {}

//-Instance Functions------------------------------------------------------------------------------------------
//Public:
Playlist::Builder& Playlist::Builder::wId(QStringView rawId) { mPlaylistBlueprint.mId = QUuid(rawId); return *this; }
Playlist::Builder& Playlist::Builder::wTitle(const QString& title) { mPlaylistBlueprint.mTitle = title; return *this; }
Playlist::Builder& Playlist::Builder::wDescription(const QString& description) { mPlaylistBlueprint.mDescription = description; return *this; }
Playlist::Builder& Playlist::Builder::wAuthor(const QString& author) { mPlaylistBlueprint.mAuthor = author; return *this; }
Playlist::Builder& Playlist::Builder::wLibrary(const QString& library) { mPlaylistBlueprint.mLibrary = library; return *this; }
Playlist::Builder& Playlist::Builder::wIcon(const QImage& icon) { mPlaylistBlueprint.mIcon = icon; return *this; }
Playlist::Builder& Playlist::Builder::wPlaylistGame(const PlaylistGame& playlistGame) { mPlaylistBlueprint.mPlaylistGames.append(playlistGame); return *this; }

Playlist Playlist::Builder::build() { return mPlaylistBlueprint; }

}
