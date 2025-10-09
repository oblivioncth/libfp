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
enum class ImageType { Logo, Screenshot };

//-Namespace Classes---------------------------------------------------------------------------------------------
class FP_FP_EXPORT Game
{
    friend class Db;
    friend class Entry;
//-Inner Struct----------------------------------------------------------------------------------------------------
private:
    struct Sql
    {
        static inline const QString ENTRY_GAME_LIBRARY = u"arcade"_s;
        static inline const QString ENTRY_ANIM_LIBRARY = u"theatre"_s;
        static inline const QString ENTRY_NOT_WORK = u"Not Working"_s;

        QUuid id;
        QString title;
        QString series;
        QString developer;
        QString publisher;
        QDateTime dateAdded;
        QDateTime dateModified;
        bool broken;
        QString playMode;
        QString status;
        QString notes;
        QString source;
        QString applicationPath;
        QString launchCommand;
        QDateTime releaseDate;
        QString version;
        QString originalDescription;
        QString language;
        QString orderTitle;
        QString library;
        QString platformName;
        QString ruffleSupport; // Could be an enum
    };

//-Instance Variables-----------------------------------------------------------------------------------------------
private:
    Sql mData;

//-Constructor-------------------------------------------------------------------------------------------------
private:
    Game(Sql&& sql);

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
    QString applicationPath() const;
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
    friend class Db;
//-Inner Structs----------------------------------------------------------------------------------------------------
private:
    struct Sql
    {
        quint32 id;
        QUuid gameId;
        QString title;
        QDateTime dateAdded;
        QString sha256;
        quint32 crc32;
        bool presentOnDisk;
        QString path;
        quint32 size;
        QString parameters;
        QString applicationPath;
        QString launchCommand;
    };

//-Instance Variables-----------------------------------------------------------------------------------------------
private:
    Sql mData;
    bool mNull;

//-Constructor-------------------------------------------------------------------------------------------------
private:
    GameData(Sql&& sql);

public:
    GameData();

//-Instance Functions------------------------------------------------------------------------------------------
public:
    bool isNull() const; // TODO: IS THIS NEEDED???

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
    GameDataParameters parsedParameters() const;
    QString applicationPath() const;
    QString launchCommand() const;
};

class FP_FP_EXPORT GameTags
{
    friend class Db;
//-Instance Variables-----------------------------------------------------------------------------------------------
private:
    QHash<QString, QStringList> mTags;

//-Constructor-------------------------------------------------------------------------------------------------
public:
    GameTags();

//-Instance Functions------------------------------------------------------------------------------------------
private:
    void addTag(const QString& genre, const QString& tag);

public:
    QStringList tags() const;
    QStringList tags(const QString& category) const;
};

class FP_FP_EXPORT AddApp
{
    friend class Db;
    friend class Entry;
//-Inner Structs----------------------------------------------------------------------------------------------------
private:
    struct Sql
    {
        static inline const QString ENTRY_EXTRAS = u":extras:"_s;
        static inline const QString ENTRY_MESSAGE = u":message:"_s;

        QUuid id;
        QString applicationPath;
        bool autorunBefore;
        QString launchCommand;
        QString name;
        bool waitForExit;
        QUuid parentGameId;

        bool operator==(const Sql& other) const noexcept = default;
    };

//-Instance Variables-----------------------------------------------------------------------------------------------
private:
    Sql mData;

//-Constructor-------------------------------------------------------------------------------------------------
private:
    AddApp(Sql&& sql);

public:
    AddApp();

//-Operators-----------------------------------------------------------------------------------------------------------
public:
    friend bool operator==(const AddApp& lhs, const AddApp& rhs) noexcept;

//-Hashing-------------------------------------------------------------------------------------------------------------
public:
    friend size_t qHash(const AddApp& key, size_t seed) noexcept;

//-Instance Functions------------------------------------------------------------------------------------------------------
public:
    QUuid id() const;
    QString applicationPath() const;
    bool isAutorunBefore() const;
    QString launchCommand() const;
    QString name() const;
    bool isWaitForExit() const;
    QUuid parentGameId() const;

    bool isPlayable() const;
    bool isMessage() const;
    bool isExtra() const;
};

class FP_FP_EXPORT Entry
{
    friend class Db;
//-Class Aliases---------------------------------------------------------------------------------------------------
public:
    using variant_t = std::variant<Game, AddApp>;

//-Instance Variables-----------------------------------------------------------------------------------------------
private:
    variant_t mData;

//-Constructor-------------------------------------------------------------------------------------------------
private:
    Entry(Game::Sql&& sql);
    Entry(AddApp::Sql&& sql);

public:
    Entry();
    Entry(const Game& game);
    Entry(Game&& game);
    Entry(const AddApp& addApp);
    Entry(AddApp&& addApp);

//-Instance Functions------------------------------------------------------------------------------------------------------
public:
    QUuid id() const;
    QString name() const;
    QString title() const;
    bool holdsGame() const;
    bool holdsAddApp() const;

    template <typename Visitor>
    decltype(auto) visit(Visitor&& v)
    {
        return std::visit(std::forward<Visitor>(v), mData);
    }

    template <typename Visitor>
    decltype(auto) visit(Visitor&& v) const
    {
        return std::visit(std::forward<Visitor>(v), mData);
    }

    Game& getGame();
    const Game& getGame() const;
    AddApp& getAddApp();
    const AddApp& getAddApp() const;
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

}

#endif // FLASHPOINT_ITEMS_H
