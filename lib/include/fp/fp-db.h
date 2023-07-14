#ifndef FLASHPOINT_DB_H
#define FLASHPOINT_DB_H

// Shared Lib Support
#include "fp/fp_export.h"

// Qt Includes
#include <QStringList>
#include <QtSql>
#include <QColor>

// Qx Includes
#include <qx/core/qx-abstracterror.h>

// Project Includes
#include "fp/fp-items.h"

using namespace Qt::Literals::StringLiterals;

namespace Fp
{

class FP_FP_EXPORT QX_ERROR_TYPE(DbError, "Fp::DbError", 1101)
{
    friend class Db;
//-Class Enums-------------------------------------------------------------
public:
    enum Type
    {
        NoError = 0,
        SqlError = 1,
        InvalidSchema = 2,
        IdCollision = 3,
        IncompleteSearch = 4
    };

//-Class Variables-------------------------------------------------------------
private:
    static inline const QHash<Type, QString> ERR_STRINGS{
        {NoError, QSL("No error occurred.")},
        {SqlError, QSL("An unexpected SQL error occurred.")},
        {InvalidSchema, QSL("The schema of the database was different than expected.")},
        {IdCollision, QSL("A duplicate of a unique ID was found.")},
        {IncompleteSearch, QSL("A data search could not be completed.")}
    };

//-Instance Variables-------------------------------------------------------------
private:
    Type mType;
    QString mCause;
    QString mDetails;

//-Class Constructor-------------------------------------------------------------
private:
    DbError(Type t = NoError, const QString& c = {}, const QString& d = {});

//-Class Functions---------------------------------------------------------------
private:
    static DbError fromSqlError(const QSqlError& e);

//-Instance Functions-------------------------------------------------------------
private:
    Qx::Severity deriveSeverity() const override;
    quint32 deriveValue() const override;
    QString derivePrimary() const override;
    QString deriveSecondary() const override;
    QString deriveDetails() const override;

public:
    bool isValid() const;
    Type type() const;
    QString cause() const;
    QString details() const;
};

class FP_FP_EXPORT Db : public QObject
{
//-QObject Macro (Required for all QObject Derived Classes)-----------------------------------------------------------
    Q_OBJECT

//-Inner Classes-------------------------------------------------------------------------------------------------
public:
    class Table_Game
    {
    public:
        static inline const QString NAME = "game";

        static inline const QString COL_ID = "id";
        static inline const QString COL_PARENT_ID = "parentGameId";
        static inline const QString COL_TITLE = "title";
        static inline const QString COL_SERIES = "series";
        static inline const QString COL_DEVELOPER = "developer";
        static inline const QString COL_PUBLISHER = "publisher";
        static inline const QString COL_DATE_ADDED = "dateAdded";
        static inline const QString COL_DATE_MODIFIED = "dateModified";
        static inline const QString COL_BROKEN = "broken";
        static inline const QString COL_EXTREME = "extreme";
        static inline const QString COL_PLAY_MODE = "playMode";
        static inline const QString COL_STATUS = "status";
        static inline const QString COL_NOTES = "notes";
        static inline const QString COL_SOURCE = "source";
        static inline const QString COL_APP_PATH = "applicationPath";
        static inline const QString COL_LAUNCH_COMMAND = "launchCommand";
        static inline const QString COL_RELEASE_DATE = "releaseDate";
        static inline const QString COL_VERSION = "version";
        static inline const QString COL_ORIGINAL_DESC = "originalDescription";
        static inline const QString COL_LANGUAGE = "language";
        static inline const QString COL_LIBRARY = "library";
        static inline const QString COL_ORDER_TITLE = "orderTitle";
        static inline const QString COL_PLATFORM_NAME = "platformName";

        static inline const QStringList COLUMN_LIST = {COL_ID, COL_PARENT_ID, COL_TITLE, COL_SERIES, COL_DEVELOPER, COL_PUBLISHER, COL_DATE_ADDED, COL_DATE_MODIFIED,
                                               COL_BROKEN, COL_EXTREME, COL_PLAY_MODE, COL_STATUS, COL_NOTES, COL_SOURCE, COL_APP_PATH, COL_LAUNCH_COMMAND, COL_RELEASE_DATE,
                                               COL_VERSION, COL_ORIGINAL_DESC, COL_LANGUAGE, COL_LIBRARY, COL_ORDER_TITLE, COL_PLATFORM_NAME};

        static inline const QString ENTRY_GAME_LIBRARY = "arcade";
        static inline const QString ENTRY_ANIM_LIBRARY = "theatre";
        static inline const QString ENTRY_NOT_WORK = "Not Working";
    };

    class Table_Game_Data
    {
    public:
        static inline const QString NAME = "game_data";

        static inline const QString COL_ID = "id";
        static inline const QString COL_GAME_ID = "gameId";
        static inline const QString COL_TITLE = "title";
        static inline const QString COL_DATE_ADDED = "dateAdded";
        static inline const QString COL_SHA256 = "sha256";
        static inline const QString COL_CRC32 = "crc32";
        static inline const QString COL_PRES_ON_DISK = "presentOnDisk";
        static inline const QString COL_PATH = "path";
        static inline const QString COL_SIZE = "size";
        static inline const QString COL_PARAM = "parameters";
        static inline const QString COL_APP_PATH = "applicationPath";
        static inline const QString COL_LAUNCH_COMMAND = "launchCommand";

        static inline const QStringList COLUMN_LIST = {COL_ID, COL_GAME_ID, COL_TITLE, COL_DATE_ADDED, COL_SHA256, COL_CRC32, COL_PRES_ON_DISK, COL_PATH, COL_SIZE, COL_PARAM,
                                                      COL_APP_PATH, COL_LAUNCH_COMMAND};
    };

    class Table_Add_App
    {
    public:
        static inline const QString NAME = "additional_app";

        static inline const QString COL_ID = "id";
        static inline const QString COL_APP_PATH = "applicationPath";
        static inline const QString COL_AUTORUN = "autoRunBefore";
        static inline const QString COL_LAUNCH_COMMAND = "launchCommand";
        static inline const QString COL_NAME = "name";
        static inline const QString COL_WAIT_EXIT = "waitForExit";
        static inline const QString COL_PARENT_ID = "parentGameId";

        static inline const QStringList COLUMN_LIST = {COL_ID, COL_APP_PATH, COL_AUTORUN, COL_LAUNCH_COMMAND, COL_NAME, COL_WAIT_EXIT, COL_PARENT_ID};

        static inline const QString ENTRY_EXTRAS = ":extras:";
        static inline const QString ENTRY_MESSAGE = ":message:";
    };


    class Table_Game_Tags_Tag
    {
    public:
        static inline const QString NAME = "game_tags_tag";

        static inline const QString COL_GAME_ID = "gameId";
        static inline const QString COL_TAG_ID = "tagId";

        static inline const QStringList COLUMN_LIST = {COL_GAME_ID, COL_TAG_ID};
    };

    class Table_Tag
    {
    public:
        static inline const QString NAME = "tag";

        static inline const QString COL_ID = "id";
        static inline const QString COL_PRIMARY_ALIAS_ID = "primaryAliasId";
        static inline const QString COL_CATEGORY_ID = "categoryId";

        static inline const QStringList COLUMN_LIST = {COL_ID, COL_PRIMARY_ALIAS_ID, COL_CATEGORY_ID};
    };

    class Table_Tag_Alias
    {
    public:
        static inline const QString NAME = "tag_alias";

        static inline const QString COL_ID = "id";
        static inline const QString COL_TAG_ID = "tagId";
        static inline const QString COL_NAME = "name";

        static inline const QStringList COLUMN_LIST = {COL_ID, COL_TAG_ID, COL_NAME};
    };

    class Table_Tag_Category
    {
    public:
        static inline const QString NAME = "tag_category";

        static inline const QString COL_ID = "id";
        static inline const QString COL_NAME = "name";
        static inline const QString COL_COLOR = "color";

        static inline const QStringList COLUMN_LIST = {COL_ID, COL_NAME, COL_COLOR};

    };

    class Key
    {
        friend class Install;
    private:
        Key() {};
        Key(const Key&) = default;
    };

//-Class Enums---------------------------------------------------------------------------------------------------
public:
    enum class LibraryFilter{ Game, Anim, Either };
    enum class EntryType{ Primary, AddApp, PrimaryThenAddApp };

//-Structs-----------------------------------------------------------------------------------------------------
private:
    struct TableSpecs
    {
        QString name;
        QStringList columns;
    };

public:
    /* TODO: Make this a proper class that abstracts away the direct interactions with the database.
     * Automatically advance to the first record, have a hasNext() like function, size() function,
     * direct value() function, etc.
     */
    struct QueryBuffer
    {
        QString source;
        QSqlQuery result;
        int size = 0;
    };

    struct Tag
    {
        int id;
        QString primaryAlias;
    };

    struct TagCategory
    {
        QString name;
        QColor color;
        QMap<int, Tag> tags;

        friend bool operator< (const TagCategory& lhs, const TagCategory& rhs) noexcept;
    };

    struct InclusionOptions
    {
        QSet<int> excludedTagIds;
        bool includeAnimations;
    };

    struct EntryFilter
    {
        EntryType type = EntryType::PrimaryThenAddApp;
        QUuid id;
        QUuid parent;
        QString name;
        bool playableOnly = false;
        bool exactName = true;
    };

//-Class Variables-----------------------------------------------------------------------------------------------
private:
    static inline const QString DATABASE_CONNECTION_NAME = "flashpoint_database";

    // TODO: Self register this somehow so that it doesnt need to be modified when tables are added or removed.
    // Might require wrapping tables in actual classes
    static inline const QList<Db::TableSpecs> DATABASE_SPECS_LIST = {
        {Db::Table_Game::NAME, Db::Table_Game::COLUMN_LIST},
        {Db::Table_Add_App::NAME, Db::Table_Add_App::COLUMN_LIST},
        {Db::Table_Game_Data::NAME, Db::Table_Game_Data::COLUMN_LIST},
        {Db::Table_Game_Tags_Tag::NAME, Db::Table_Game_Tags_Tag::COLUMN_LIST},
        {Db::Table_Tag::NAME, Db::Table_Tag::COLUMN_LIST},
        {Db::Table_Tag_Alias::NAME, Db::Table_Tag_Alias::COLUMN_LIST},
        {Db::Table_Tag_Category::NAME, Db::Table_Tag_Category::COLUMN_LIST},
    };
    static inline const QString GENERAL_QUERY_SIZE_COMMAND = "COUNT(1)";

    static inline const QString GAME_ONLY_FILTER = Db::Table_Game::COL_LIBRARY + " = '" + Db::Table_Game::ENTRY_GAME_LIBRARY + "'";
    static inline const QString ANIM_ONLY_FILTER = Db::Table_Game::COL_LIBRARY + " = '" + Db::Table_Game::ENTRY_ANIM_LIBRARY + "'";
    static inline const QString GAME_AND_ANIM_FILTER = "(" + GAME_ONLY_FILTER + " OR " + ANIM_ONLY_FILTER + ")";

    // Error
    static inline const QString ERR_MISSING_TABLE = "The Flashpoint database is missing expected tables.";
    static inline const QString ERR_TABLE_MISSING_COLUMN = "The Flashpoint database tables are missing expected columns.";
    static inline const QString ERR_ID_NOT_FOUND = u"An entry matching the specified ID could not be found in the Flashpoint database."_s;
    static inline const QString ERR_ID_DUPLICATE_ENTRY = u"This should not be possible and may indicate an error within the Flashpoint database"_s;

//-Instance Variables-----------------------------------------------------------------------------------------------
private:
    bool mValid;
    DbError mError;

    // Database information
    QSet<const QThread*> mConnectedThreads;
    const QString mDatabaseName;
    QStringList mPlatformList;
    QStringList mPlaylistList;
    QMap<int, TagCategory> mTagMap; // Order matters for display in tag selector

//-Constructor-------------------------------------------------------------------------------------------------
public:
    explicit Db(const QString& databaseName, const Key&);

//-Destructor-------------------------------------------------------------------------------------------------
public:
    ~Db();

//-Class Functions--------------------------------------------------------------------------------------------
private:
    QString threadConnectionName(const QThread* thread);

//-Instance Functions------------------------------------------------------------------------------------------------------
private:
    // Validity
    void nullify();

    // Connection
    void closeConnection(const QThread* thread);
    void closeAllConnections();
    QSqlError getThreadConnection(QSqlDatabase& connection);
    QSqlError makeNonBindQuery(QueryBuffer& resultBuffer, QSqlDatabase* database, QString queryCommand, QString sizeQueryCommand) const;

    // Init
    QSqlError checkDatabaseForRequiredTables(QSet<QString>& missingTablesBuffer);
    QSqlError checkDatabaseForRequiredColumns(QSet<QString>& missingColumsBuffer);
    QSqlError populateAvailableItems();
    QSqlError populateTags();

public:
    // Validity
    bool isValid();
    DbError error();

    // TODO: See if these query functions can be consolidated via by better filtration arguments

    // Queries - OFLIb
    DbError queryGamesByPlatform(QList<Db::QueryBuffer>& resultBuffer, QStringList platforms, InclusionOptions inclusionOptions,
                                   std::optional<const QList<QUuid>*> idInclusionFilter = std::nullopt);
    DbError queryAllAddApps(QueryBuffer& resultBuffer);
    DbError queryAllEntryTags(QueryBuffer& resultBuffer);

    // Queries - CLIFp
    DbError queryEntrys(QueryBuffer& resultBuffer, EntryFilter filter);
    DbError queryEntryDataById(QueryBuffer& resultBuffer, const QUuid& appId);
    DbError queryAllGameIds(QueryBuffer& resultBuffer, LibraryFilter filter);

    // Info
    QStringList platformList() const;
    QMap<int, TagCategory> tags() const;

    // Checks
    DbError entryUsesDataPack(bool& resultBuffer, QUuid gameId);

    // Helper
    DbError getEntry(std::variant<Game, AddApp>& entry, const QUuid& entryId);
    DbError getGameData(GameData& data, const QUuid& gameId);

//-Slots ------------------------------------------------------------------------------------------------------
private:
    void connectedThreadDestroyed(QObject* thread);
};

}



#endif // FLASHPOINT_DB_H
