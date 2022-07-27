#ifndef FLASHPOINT_ITEMS_H
#define FLASHPOINT_ITEMS_H

// Qt Includes
#include <QString>
#include <QDateTime>
#include <QUuid>

namespace Fp
{
// TODO: Change getters here to property style (no "get")

//-Enums----------------------------------------------------------------------------------------------------------
enum class ImageType {Logo, Screenshot};

//-Class Forward Declarations---------------------------------------------------------------------------------------
class GameBuilder;
class AddAppBuilder;
class PlaylistBuilder;
class PlaylistGameBuilder;

//-Namespace Global Classes-----------------------------------------------------------------------------------------
class Game
{
    friend class GameBuilder;

//-Instance Variables-----------------------------------------------------------------------------------------------
private:
    QUuid mId;
    QString mTitle;
    QString mSeries;
    QString mDeveloper;
    QString mPublisher;
    QDateTime mDateAdded;
    QDateTime mDateModified;
    QString mPlatform;
    bool mBroken;
    QString mPlayMode;
    QString mStatus;
    QString mNotes;
    QString mSource;
    QString mAppPath;
    QString mLaunchCommand;
    QDateTime mReleaseDate;
    QString mVersion;
    QString mOriginalDescription;
    QString mLanguage;
    QString mOrderTitle;
    QString mLibrary;

//-Constructor-------------------------------------------------------------------------------------------------
public:
    Game();

//-Instance Functions------------------------------------------------------------------------------------------
public:
    QUuid id() const;
    QString title() const;
    QString series() const;
    QString developer() const;
    QString publisher() const;
    QDateTime dateAdded() const;
    QDateTime dateModified() const;
    QString platform() const;
    bool isBroken() const;
    QString playMode() const;
    QString status() const;
    QString notes() const;
    QString source() const;
    QString appPath() const;
    QString launchCommand() const;
    QDateTime releaseDate() const;
    QString version() const;
    QString originalDescription() const;
    QString language() const;
    QString orderTitle() const;
    QString library() const;
};

class GameBuilder
{
//-Instance Variables------------------------------------------------------------------------------------------
private:
    Game mGameBlueprint;

//-Constructor-------------------------------------------------------------------------------------------------
public:
    GameBuilder();

//-Class Functions---------------------------------------------------------------------------------------------
private:
    static QString kosherizeRawDate(QString date);

//-Instance Functions------------------------------------------------------------------------------------------
public:
    GameBuilder& wId(QString rawId);
    GameBuilder& wTitle(QString title);
    GameBuilder& wSeries(QString series);
    GameBuilder& wDeveloper(QString developer);
    GameBuilder& wPublisher(QString publisher);
    GameBuilder& wDateAdded(QString rawDateAdded);
    GameBuilder& wDateModified(QString rawDateModified);
    GameBuilder& wPlatform(QString platform);
    GameBuilder& wBroken(QString rawBroken);
    GameBuilder& wPlayMode(QString playMode);
    GameBuilder& wStatus(QString status);
    GameBuilder& wNotes(QString notes);
    GameBuilder& wSource(QString source);
    GameBuilder& wAppPath(QString appPath);
    GameBuilder& wLaunchCommand(QString launchCommand);
    GameBuilder& wReleaseDate(QString rawReleaseDate);
    GameBuilder& wVersion(QString version);
    GameBuilder& wOriginalDescription(QString originalDescription);
    GameBuilder& wLanguage(QString language);
    GameBuilder& wOrderTitle(QString orderTitle);
    GameBuilder& wLibrary(QString library);

    Game build();
};

class AddApp
{
    friend class AddAppBuilder;

//-Class Variables-----------------------------------------------------------------------------------------------
private:
    QString SPEC_PATH_MSG = ":message:";
    QString SPEC_PATH_EXTRA = ":extras:";

//-Instance Variables-----------------------------------------------------------------------------------------------
private:
    QUuid mId;
    QString mAppPath;
    bool mAutorunBefore;
    QString mLaunchCommand;
    QString mName;
    bool mWaitExit;
    QUuid mParentId;

//-Constructor-------------------------------------------------------------------------------------------------
public:
    AddApp();

//-Operators-----------------------------------------------------------------------------------------------------------
public:
    friend bool operator== (const AddApp& lhs, const AddApp& rhs) noexcept;

//-Hashing-------------------------------------------------------------------------------------------------------------
public:
    friend uint qHash(const AddApp& key, uint seed) noexcept;

//-Instance Functions------------------------------------------------------------------------------------------------------
public:
    QUuid id() const;
    QString appPath() const;
    bool isAutorunBefore() const;
    QString launchCommand() const;
    QString name() const;
    bool isWaitExit() const;
    QUuid parentId() const;
    bool isPlayable() const;
};

class AddAppBuilder
{
//-Instance Variables------------------------------------------------------------------------------------------
private:
    AddApp mAddAppBlueprint;

//-Constructor-------------------------------------------------------------------------------------------------
public:
    AddAppBuilder();

//-Instance Functions------------------------------------------------------------------------------------------
public:
    AddAppBuilder& wId(QString rawId);
    AddAppBuilder& wAppPath(QString appPath);
    AddAppBuilder& wAutorunBefore(QString rawAutorunBefore);
    AddAppBuilder& wLaunchCommand(QString launchCommand);
    AddAppBuilder& wName(QString name);
    AddAppBuilder& wWaitExit(QString rawWaitExit);
    AddAppBuilder& wParentId(QString rawParentId);

    AddApp build();
};

class Set
{
    friend class SetBuilder;

//-Instance Variables-----------------------------------------------------------------------------------------------
private:
    Game mGame;
    QList<AddApp> mAddApps;

//-Constructor-------------------------------------------------------------------------------------------------
public:
    Set();

//-Instance Functions------------------------------------------------------------------------------------------------------
public:
    Game game() const;
    QList<AddApp> addApps() const;
};

class SetBuilder
{
//-Instance Variables------------------------------------------------------------------------------------------
private:
    Set mSetBlueprint;

//-Constructor-------------------------------------------------------------------------------------------------
public:
    SetBuilder();

//-Instance Functions------------------------------------------------------------------------------------------
public:
    SetBuilder& wGame(const Game& game);
    SetBuilder& wAddApp(const AddApp& addApp);
    SetBuilder& wAddApps(const QList<AddApp>& addApps);

    Set build();
};

class Playlist
{
    friend class PlaylistBuilder;

//-Instance Variables-----------------------------------------------------------------------------------------------
private:
    QUuid mId;
    QString mTitle;
    QString mDescription;
    QString mAuthor;

//-Constructor-------------------------------------------------------------------------------------------------
public:
    Playlist();

//-Instance Functions------------------------------------------------------------------------------------------------------
public:
    QUuid id() const;
    QString title() const;
    QString description() const;
    QString author() const;

};

class PlaylistBuilder
{
//-Instance Variables------------------------------------------------------------------------------------------
private:
    Playlist mPlaylistBlueprint;

//-Constructor-------------------------------------------------------------------------------------------------
public:
    PlaylistBuilder();

//-Instance Functions------------------------------------------------------------------------------------------
public:
    PlaylistBuilder& wId(QString rawId);
    PlaylistBuilder& wTitle(QString title);
    PlaylistBuilder& wDescription(QString description);
    PlaylistBuilder& wAuthor(QString author);

    Playlist build();
};

class PlaylistGame
{
    friend class PlaylistGameBuilder;

//-Instance Variables-----------------------------------------------------------------------------------------------
private:
    int mId;
    QUuid mPlaylistId;
    int mOrder;
    QUuid mGameId;

//-Constructor-------------------------------------------------------------------------------------------------
public:
    PlaylistGame();

//-Instance Functions------------------------------------------------------------------------------------------------------
public:
    int id() const;
    QUuid playlistId() const;
    int order() const;
    QUuid gameId() const;
};

class PlaylistGameBuilder
{
//-Instance Variables------------------------------------------------------------------------------------------
private:
    PlaylistGame mPlaylistGameBlueprint;

//-Constructor-------------------------------------------------------------------------------------------------
public:
    PlaylistGameBuilder();

//-Instance Functions------------------------------------------------------------------------------------------
public:
    PlaylistGameBuilder& wId(QString rawId);
    PlaylistGameBuilder& wPlaylistId(QString rawPlaylistId);
    PlaylistGameBuilder& wOrder(QString rawOrder);
    PlaylistGameBuilder& wGameId(QString rawGameId);

    PlaylistGame build();
};

}

#endif // FLASHPOINT_ITEMS_H
