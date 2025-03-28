#ifndef FLASHPOINT_ITEMS_H
#define FLASHPOINT_ITEMS_H

// Shared Lib Support
#include "fp/fp_export.h"

// Qt Includes
#include <QString>
#include <QDateTime>
#include <QUuid>
#include <QImage>

using namespace Qt::Literals::StringLiterals;

namespace Fp
{
//-Enums----------------------------------------------------------------------------------------------------------
enum class ImageType {Logo, Screenshot};

//-Namespace Classes---------------------------------------------------------------------------------------------
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
    QString mRuffleSupport; // Could be an enum

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
    QString ruffleSupport() const;
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
    static QString kosherizeRawDate(const QString& date);

//-Instance Functions------------------------------------------------------------------------------------------
public:
    Builder& wId(QStringView rawId);
    Builder& wTitle(const QString& title);
    Builder& wSeries(const QString& series);
    Builder& wDeveloper(const QString& developer);
    Builder& wPublisher(const QString& publisher);
    Builder& wDateAdded(QStringView rawDateAdded);
    Builder& wDateModified(QStringView rawDateModified);
    Builder& wBroken(QStringView rawBroken);
    Builder& wPlayMode(const QString& playMode);
    Builder& wStatus(const QString& status);
    Builder& wNotes(const QString& notes);
    Builder& wSource(const QString& source);
    Builder& wAppPath(const QString& appPath);
    Builder& wLaunchCommand(const QString& launchCommand);
    Builder& wReleaseDate(QStringView rawReleaseDate);
    Builder& wVersion(const QString& version);
    Builder& wOriginalDescription(const QString& originalDescription);
    Builder& wLanguage(const QString& language);
    Builder& wOrderTitle(const QString& orderTitle);
    Builder& wLibrary(const QString& library);
    Builder& wPlatformName(const QString& platformName);
    Builder& wRuffleSupport(const QString& ruffleSupport);

    Game build();
};

class FP_FP_EXPORT GameDataParameters
{
//-Instance Variables-----------------------------------------------------------------------------------------------
private:
    bool mExtract;
    QString mExtractedMarkerFile;
    QString mServer;

    QString mErrorStr;

//-Constructor-------------------------------------------------------------------------------------------------
public:
    GameDataParameters(const QString& rawParameters);

//-Instance Functions------------------------------------------------------------------------------------------
public:
    bool isExtract() const;
    QString extractedMarkerFile() const;
    QString server() const;

    bool hasError() const;
    QString errorString() const;
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
    QString mRawParameters;
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
    QString rawParameters() const;
    GameDataParameters parameters() const;
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
    Builder& wDateAdded(const QString& rawDateAdded);
    Builder& wSha256(const QString& sha256);
    Builder& wCrc32(QStringView rawCrc32);
    Builder& wPresentOnDisk(QStringView rawBroken);
    Builder& wPath(const QString& path);
    Builder& wSize(QStringView rawSize);
    Builder& wRawParameters(const QString& parameters);
    Builder& wAppPath(const QString& appPath);
    Builder& wLaunchCommand(const QString& launchCommand);

    GameData build();
};

class FP_FP_EXPORT GameTags
{
//-Inner Classes----------------------------------------------------------------------------------------------------
public:
    class Builder;

//-Instance Variables-----------------------------------------------------------------------------------------------
private:
    QHash<QString, QStringList> mTags;

//-Constructor-------------------------------------------------------------------------------------------------
public:
    GameTags();

//-Instance Functions------------------------------------------------------------------------------------------
public:
    QStringList tags() const;
    QStringList tags(const QString& category) const;
};

class FP_FP_EXPORT GameTags::Builder
{
//-Instance Variables------------------------------------------------------------------------------------------
private:
    GameTags mGameTagsBlueprint;

//-Constructor-------------------------------------------------------------------------------------------------
public:
    Builder();

//-Instance Functions------------------------------------------------------------------------------------------
public:
    Builder& wTag(const QString& genre, const QString& tag);

    GameTags build();
};

class FP_FP_EXPORT AddApp
{
//-Inner Classes----------------------------------------------------------------------------------------------------
public:
    class Builder;

//-Class Variables-----------------------------------------------------------------------------------------------
private:
    static inline const QString SPEC_PATH_MSG = u":message:"_s;
    static inline const QString SPEC_PATH_EXTRA = u":extras:"_s;

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
    friend size_t qHash(const AddApp& key, size_t seed) noexcept;

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
    Builder& wId(QStringView rawId);
    Builder& wAppPath(const QString& appPath);
    Builder& wAutorunBefore(QStringView rawAutorunBefore);
    Builder& wLaunchCommand(const QString& launchCommand);
    Builder& wName(const QString& name);
    Builder& wWaitExit(QStringView rawWaitExit);
    Builder& wParentId(QStringView rawParentId);

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
    GameTags mTags;
    QList<AddApp> mAddApps;

//-Constructor-------------------------------------------------------------------------------------------------
public:
    Set();

//-Instance Functions------------------------------------------------------------------------------------------------------
public:
    const Game& game() const;
    const GameTags& tags() const;
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
    Builder& wTags(const GameTags& tags);
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
    std::optional<int> mId;
    QUuid mPlaylistId;
    int mOrder;
    QUuid mGameId;

    //-Constructor-------------------------------------------------------------------------------------------------
public:
    PlaylistGame();

    //-Instance Functions------------------------------------------------------------------------------------------------------
public:
    std::optional<int> id() const;
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
    Builder& wId(std::optional<int> id);
    Builder& wPlaylistId(QStringView rawPlaylistId);
    Builder& wOrder(int order);
    Builder& wGameId(QStringView rawGameId);

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
    QString mLibrary;
    QImage mIcon;

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
    QString library() const;
    QImage icon() const;

    const QList<PlaylistGame>& playlistGames() const;
    QList<PlaylistGame>& playlistGames();

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
    Builder& wId(QStringView rawId);
    Builder& wTitle(const QString& title);
    Builder& wDescription(const QString& description);
    Builder& wAuthor(const QString& author);
    Builder& wLibrary(const QString& library);
    Builder& wIcon(const QImage& icon);
    Builder& wPlaylistGame(const PlaylistGame& playlistGame);

    Playlist build();
};

//-Namespace Types---------------------------------------------------------------------------------------------
using Entry = std::variant<Game, AddApp>;

}

#endif // FLASHPOINT_ITEMS_H
