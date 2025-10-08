// Unit Include
#include "fp/fp-items.h"

// Qt Includes
#include <QCommandLineParser>
#include <QProcess>

// Qx Includes
#include <qx/core/qx-string.h>
#include <qx/utility/qx-helpers.h>


namespace Fp
{

//===============================================================================================================
// Game
//===============================================================================================================

//-Constructor------------------------------------------------------------------------------------------------
//Private:
Game::Game(Sql&& sql) :
    mData(std::move(sql))
{}

//Public:
Game::Game() {}

//-Instance Functions------------------------------------------------------------------------------------------------
//Public:
QUuid Game::id() const { return mData.id; }
QString Game::title() const { return mData.title; }
QString Game::series() const { return mData.series; }
QString Game::developer() const { return mData.developer; }
QString Game::publisher() const { return mData.publisher; }
QDateTime Game::dateAdded() const { return mData.dateAdded; }
QDateTime Game::dateModified() const { return mData.dateModified; }
QString Game::playMode() const { return mData.playMode; }
bool Game::isBroken() const { return mData.broken; }
QString Game::status() const { return mData.status; }
QString Game::notes() const{ return mData.notes; }
QString Game::source() const { return mData.source; }
QString Game::appPath() const { return mData.appPath; }
QString Game::launchCommand() const { return mData.launchCommand; }
QDateTime Game::releaseDate() const { return mData.releaseDate; }
QString Game::version() const { return mData.version; }
QString Game::originalDescription() const { return mData.originalDescription; }
QString Game::language() const { return mData.language; }
QString Game::orderTitle() const { return mData.orderTitle; }
QString Game::library() const { return mData.library; }
QString Game::platformName() const { return mData.platformName; }
QString Game::ruffleSupport() const { return mData.ruffleSupport; }

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
//Private:
GameData::GameData(Sql&& sql) :
    mData(sql),
    mNull(false)
{}

//Public:
GameData::GameData() :
    mNull(true)
{}

//-Instance Functions------------------------------------------------------------------------------------------------
//Public:
bool GameData::isNull() const { return mNull; }

quint32 GameData::id() const { return mData.id; }
QUuid GameData::gameId() const { return mData.gameId; }
QString GameData::title() const { return mData.title; }
QDateTime GameData::dateAdded() const { return mData.dateAdded; }
QString GameData::sha256() const { return mData.sha256; }
quint32 GameData::crc32() const { return mData.crc32; }
bool GameData::presentOnDisk() const { return mData.presentOnDisk; }
QString GameData::path() const { return mData.path; }
quint32 GameData::size() const { return mData.size; }
QString GameData::rawParameters() const { return mData.rawParameters; }
GameDataParameters GameData::parameters() const { return GameDataParameters(mData.rawParameters); }
QString GameData::appPath() const { return mData.appPath; }
QString GameData::launchCommand() const { return mData.launchCommand; }

//===============================================================================================================
// GameTags
//===============================================================================================================

//-Constructor------------------------------------------------------------------------------------------------
//Public:
GameTags::GameTags() {}

//-Instance Functions------------------------------------------------------------------------------------------------
//Private:
void GameTags::addTag(const QString& genre, const QString& tag) { mTags[genre].append(tag); }

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
// AddApp
//===============================================================================================================

//-Constructor------------------------------------------------------------------------------------------------
//Private:
AddApp::AddApp(Sql&& sql) :
    mData(std::move(sql))
{}

//Public:
AddApp::AddApp() {}

//-Operators----------------------------------------------------------------------------------------------------
//Public:
bool operator==(const AddApp& lhs, const AddApp& rhs) noexcept { return lhs.mData == rhs.mData; }

//-Hashing------------------------------------------------------------------------------------------------------
size_t qHash(const AddApp& key, size_t seed) noexcept
{
    return qHashMulti(seed,
        key.mData.id,
        key.mData.appPath,
        key.mData.autorunBefore,
        key.mData.launchCommand,
        key.mData.name,
        key.mData.waitExit,
        key.mData.parentId
    );
}

//-Instance Functions------------------------------------------------------------------------------------------------
//Public:
QUuid AddApp::id() const { return mData.id; }
QString AddApp::appPath() const { return mData.appPath; }
bool AddApp::isAutorunBefore() const { return  mData.autorunBefore; }
QString AddApp::launchCommand() const { return mData.launchCommand; }
QString AddApp::name() const { return mData.name; }
bool AddApp::isWaitExit() const { return mData.waitExit; }
QUuid AddApp::parentId() const { return mData.parentId; }

bool AddApp::isPlayable() const { return !isMessage() && !isExtra() && !mData.autorunBefore; }
bool AddApp::isMessage() const { return mData.appPath == Sql::ENTRY_MESSAGE; }
bool AddApp::isExtra() const { return mData.appPath == Sql::ENTRY_EXTRAS; }

//===============================================================================================================
// Entry
//===============================================================================================================

//-Constructor------------------------------------------------------------------------------------------------
//Private:
Entry::Entry(Game::Sql&& sql) : mData(Game(std::move(sql))) {}
Entry::Entry(AddApp::Sql&& sql) : mData(AddApp(std::move(sql))) {}

//Public:
Entry::Entry() {}
Entry::Entry(const Game& game) : mData(game) {}
Entry::Entry(Game&& game) : mData(std::move(game)) {}
Entry::Entry(const AddApp& addApp) : mData(addApp) {}
Entry::Entry(AddApp&& addApp) : mData(std::move(addApp)) {}

//-Instance Functions------------------------------------------------------------------------------------------------
//Public:
QUuid Entry::id() const
{
    return std::visit([](auto& d) { return d.id(); }, mData);
}

QString Entry::name() const
{
    return std::visit(qxFuncAggregate{
        [](const Game& g) { return g.title(); },
        [](const AddApp& aa) { return aa.name(); }
    }, mData);
}

QString Entry::title() const { return name(); }

bool Entry::holdsGame() const { return std::holds_alternative<Game>(mData); }
bool Entry::holdsAddApp() const { return std::holds_alternative<AddApp>(mData); }

Game& Entry::getGame() { return std::get<Game>(mData); }
const Game& Entry::getGame() const { return std::get<Game>(mData); }
AddApp& Entry::getAddApp() { return std::get<AddApp>(mData); }
const AddApp& Entry::getAddApp() const { return std::get<AddApp>(mData); }

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
