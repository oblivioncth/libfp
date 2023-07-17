#ifndef FLASHPOINT_ITEMS_H
#define FLASHPOINT_ITEMS_H

// Shared Lib Support
#include "fp/fp_export.h"

// Qt Includes
#include <QString>
#include <QDateTime>
#include <QUuid>

namespace Fp
{
//-Enums----------------------------------------------------------------------------------------------------------
enum class ImageType {Logo, Screenshot};

//-Namespace Global Classes-----------------------------------------------------------------------------------------
class FP_FP_EXPORT Game
{
//-Inner Classes----------------------------------------------------------------------------------------------------
public:
    class Builder;

//-Instance Variables-----------------------------------------------------------------------------------------------
private:
    QUuid mId;
    QString mTitle;
    QString mSeries;
    QString mDeveloper;
    QString mPublisher;
    QDateTime mDateAdded;
    QDateTime mDateModified;
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
    QString mPlatformName;

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
    QString platformName() const;
};

class FP_FP_EXPORT Game::Builder
{
//-Instance Variables------------------------------------------------------------------------------------------
private:
    Game mGameBlueprint;

//-Constructor-------------------------------------------------------------------------------------------------
public:
    Builder();

//-Class Functions---------------------------------------------------------------------------------------------
private:
    static QString kosherizeRawDate(QString date);

//-Instance Functions------------------------------------------------------------------------------------------
public:
    Builder& wId(QString rawId);
    Builder& wTitle(QString title);
    Builder& wSeries(QString series);
    Builder& wDeveloper(QString developer);
    Builder& wPublisher(QString publisher);
    Builder& wDateAdded(QString rawDateAdded);
    Builder& wDateModified(QString rawDateModified);
    Builder& wBroken(QString rawBroken);
    Builder& wPlayMode(QString playMode);
    Builder& wStatus(QString status);
    Builder& wNotes(QString notes);
    Builder& wSource(QString source);
    Builder& wAppPath(QString appPath);
    Builder& wLaunchCommand(QString launchCommand);
    Builder& wReleaseDate(QString rawReleaseDate);
    Builder& wVersion(QString version);
    Builder& wOriginalDescription(QString originalDescription);
    Builder& wLanguage(QString language);
    Builder& wOrderTitle(QString orderTitle);
    Builder& wLibrary(QString library);
    Builder& wPlatformName(QString platformName);

    Game build();
};

class FP_FP_EXPORT GameData
{
//-Inner Classes----------------------------------------------------------------------------------------------------
public:
    class Builder;

//-Instance Variables-----------------------------------------------------------------------------------------------
private:
    bool mNull;

    quint32 mId;
    QUuid mGameId;
    QString mTitle;
    QDateTime mDateAdded;
    QString mSha256;
    quint32 mCrc32;
    bool mPresentOnDisk;
    QString mPath;
    quint32 mSize;
    QString mParameters;
    QString mAppPath;
    QString mLaunchCommand;

//-Constructor-------------------------------------------------------------------------------------------------
public:
    GameData();

//-Instance Functions------------------------------------------------------------------------------------------
public:
    bool isNull() const;

    quint32 id() const;
    QUuid gameId() const;
    QString title() const;
    QDateTime dateAdded() const;
    QString sha256() const;
    quint32 crc32() const;
    bool presentOnDisk() const;
    QString path() const;
    quint32 size() const;
    QString parameters() const;
    QString appPath() const;
    QString launchCommand() const;
};

class FP_FP_EXPORT GameData::Builder
{
//-Instance Variables------------------------------------------------------------------------------------------
private:
    GameData mGameDataBlueprint;

//-Constructor-------------------------------------------------------------------------------------------------
public:
    Builder();

//-Instance Functions------------------------------------------------------------------------------------------
public:
    Builder& wId(QStringView rawId);
    Builder& wGameId(QStringView rawId);
    Builder& wTitle(const QString& title);
    Builder& wDateAdded(QStringView rawDateAdded);
    Builder& wSha256(const QString& sha256);
    Builder& wCrc32(QStringView rawCrc32);
    Builder& wPresentOnDisk(QStringView rawBroken);
    Builder& wPath(const QString& path);
    Builder& wSize(QStringView rawSize);
    Builder& wParameters(const QString& parameters);
    Builder& wAppPath(const QString& appPath);
    Builder& wLaunchCommand(const QString& launchCommand);

    GameData build();
};

class FP_FP_EXPORT AddApp
{
//-Inner Classes----------------------------------------------------------------------------------------------------
public:
    class Builder;

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

class FP_FP_EXPORT AddApp::Builder
{
//-Instance Variables------------------------------------------------------------------------------------------
private:
    AddApp mAddAppBlueprint;

//-Constructor-------------------------------------------------------------------------------------------------
public:
    Builder();

//-Instance Functions------------------------------------------------------------------------------------------
public:
    Builder& wId(QString rawId);
    Builder& wAppPath(QString appPath);
    Builder& wAutorunBefore(QString rawAutorunBefore);
    Builder& wLaunchCommand(QString launchCommand);
    Builder& wName(QString name);
    Builder& wWaitExit(QString rawWaitExit);
    Builder& wParentId(QString rawParentId);

    AddApp build();
};

class FP_FP_EXPORT Set
{
//-Inner Classes----------------------------------------------------------------------------------------------------
public:
    class Builder;

//-Instance Variables-----------------------------------------------------------------------------------------------
private:
    Game mGame;
    QList<AddApp> mAddApps;

//-Constructor-------------------------------------------------------------------------------------------------
public:
    Set();

//-Instance Functions------------------------------------------------------------------------------------------------------
public:
    const Game& game() const;
    const QList<AddApp>& addApps() const;
};

class FP_FP_EXPORT Set::Builder
{
//-Instance Variables------------------------------------------------------------------------------------------
private:
    Set mSetBlueprint;

//-Constructor-------------------------------------------------------------------------------------------------
public:
    Builder();

//-Instance Functions------------------------------------------------------------------------------------------
public:
    Builder& wGame(const Game& game);
    Builder& wAddApp(const AddApp& addApp);
    Builder& wAddApps(const QList<AddApp>& addApps);

    Set build();
};

class FP_FP_EXPORT PlaylistGame
{
    //-Inner Classes----------------------------------------------------------------------------------------------------
public:
    class Builder;

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

class FP_FP_EXPORT PlaylistGame::Builder
{
    //-Instance Variables------------------------------------------------------------------------------------------
private:
    PlaylistGame mPlaylistGameBlueprint;

    //-Constructor-------------------------------------------------------------------------------------------------
public:
    Builder();

    //-Instance Functions------------------------------------------------------------------------------------------
public:
    Builder& wId(int id);
    Builder& wPlaylistId(QString rawPlaylistId);
    Builder& wOrder(int order);
    Builder& wGameId(QString rawGameId);

    PlaylistGame build();
};

class FP_FP_EXPORT Playlist
{
//-Inner Classes----------------------------------------------------------------------------------------------------
public:
    class Builder;

//-Instance Variables-----------------------------------------------------------------------------------------------
private:
    QUuid mId;
    QString mTitle;
    QString mDescription;
    QString mAuthor;
    QList<PlaylistGame> mPlaylistGames;

//-Constructor-------------------------------------------------------------------------------------------------
public:
    Playlist();

//-Instance Functions------------------------------------------------------------------------------------------------------
public:
    QUuid id() const;
    QString title() const;
    QString description() const;
    QString author() const;
    const QList<PlaylistGame>& playlistGames() const;

};

class FP_FP_EXPORT Playlist::Builder
{
//-Instance Variables------------------------------------------------------------------------------------------
private:
    Playlist mPlaylistBlueprint;

//-Constructor-------------------------------------------------------------------------------------------------
public:
    Builder();

//-Instance Functions------------------------------------------------------------------------------------------
public:
    Builder& wId(QString rawId);
    Builder& wTitle(QString title);
    Builder& wDescription(QString description);
    Builder& wAuthor(QString author);
    Builder& wPlaylistGame(const PlaylistGame& playlistGame);

    Playlist build();
};

}

#endif // FLASHPOINT_ITEMS_H
