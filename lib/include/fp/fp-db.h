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
        IncompleteSearch = 4,
        UpdateRowMismatch = 5
    };

//-Class Variables-------------------------------------------------------------
private:
    static inline const QHash<Type, QString> ERR_STRINGS{
        {NoError, u"No error occurred."_s},
        {SqlError, u"An unexpected SQL error occurred."_s},
        {InvalidSchema, u"The schema of the database was different than expected."_s},
        {IdCollision, u"A duplicate of a unique ID was found."_s},
        {IncompleteSearch, u"A data search could not be completed."_s},
        {UpdateRowMismatch, u"An update statement affected a different number of rows than expected."_s},
    };

//-Instance Variables-------------------------------------------------------------
private:
    Type mType;
    QString mCause;
    QString mDetails;

//-Class Constructor-------------------------------------------------------------
private:
    DbError(Type t, const QString& c, const QString& d = {});

public:
    DbError();

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
        static inline const QString NAME = u"game"_s;

        static inline const QString COL_ID = u"id"_s;
        static inline const QString COL_PARENT_ID = u"parentGameId"_s;
        static inline const QString COL_TITLE = u"title"_s;
        static inline const QString COL_SERIES = u"series"_s;
        static inline const QString COL_DEVELOPER = u"developer"_s;
        static inline const QString COL_PUBLISHER = u"publisher"_s;
        static inline const QString COL_DATE_ADDED = u"dateAdded"_s;
        static inline const QString COL_DATE_MODIFIED = u"dateModified"_s;
        static inline const QString COL_BROKEN = u"broken"_s;
        static inline const QString COL_EXTREME = u"extreme"_s;
        static inline const QString COL_PLAY_MODE = u"playMode"_s;
        static inline const QString COL_STATUS = u"status"_s;
        static inline const QString COL_NOTES = u"notes"_s;
        static inline const QString COL_SOURCE = u"source"_s;
        static inline const QString COL_APP_PATH = u"applicationPath"_s;
        static inline const QString COL_LAUNCH_COMMAND = u"launchCommand"_s;
        static inline const QString COL_RELEASE_DATE = u"releaseDate"_s;
        static inline const QString COL_VERSION = u"version"_s;
        static inline const QString COL_ORIGINAL_DESC = u"originalDescription"_s;
        static inline const QString COL_LANGUAGE = u"language"_s;
        static inline const QString COL_LIBRARY = u"library"_s;
        static inline const QString COL_ORDER_TITLE = u"orderTitle"_s;
        static inline const QString COL_PLATFORM_NAME = u"platformName"_s;
        static inline const QString COL_RUFFLE_SUPPORT = u"ruffleSupport"_s;

        static inline const QStringList COLUMN_LIST = {COL_ID, COL_PARENT_ID, COL_TITLE, COL_SERIES, COL_DEVELOPER, COL_PUBLISHER, COL_DATE_ADDED, COL_DATE_MODIFIED,
                                               COL_BROKEN, COL_EXTREME, COL_PLAY_MODE, COL_STATUS, COL_NOTES, COL_SOURCE, COL_APP_PATH, COL_LAUNCH_COMMAND, COL_RELEASE_DATE,
                                               COL_VERSION, COL_ORIGINAL_DESC, COL_LANGUAGE, COL_LIBRARY, COL_ORDER_TITLE, COL_PLATFORM_NAME, COL_RUFFLE_SUPPORT};

        static inline const QString ENTRY_GAME_LIBRARY = u"arcade"_s;
        static inline const QString ENTRY_ANIM_LIBRARY = u"theatre"_s;
        static inline const QString ENTRY_NOT_WORK = u"Not Working"_s;
    };

    class Table_Game_Data
    {
    public:
        static inline const QString NAME = u"game_data"_s;

        static inline const QString COL_ID = u"id"_s;
        static inline const QString COL_GAME_ID = u"gameId"_s;
        static inline const QString COL_TITLE = u"title"_s;
        static inline const QString COL_DATE_ADDED = u"dateAdded"_s;
        static inline const QString COL_SHA256 = u"sha256"_s;
        static inline const QString COL_CRC32 = u"crc32"_s;
        static inline const QString COL_PRES_ON_DISK = u"presentOnDisk"_s;
        static inline const QString COL_PATH = u"path"_s;
        static inline const QString COL_SIZE = u"size"_s;
        static inline const QString COL_PARAM = u"parameters"_s;
        static inline const QString COL_APP_PATH = u"applicationPath"_s;
        static inline const QString COL_LAUNCH_COMMAND = u"launchCommand"_s;

        static inline const QStringList COLUMN_LIST = {COL_ID, COL_GAME_ID, COL_TITLE, COL_DATE_ADDED, COL_SHA256, COL_CRC32, COL_PRES_ON_DISK, COL_PATH, COL_SIZE, COL_PARAM,
                                                      COL_APP_PATH, COL_LAUNCH_COMMAND};
    };

    class Table_Game_Redirect
    {
    public:
        static inline const QString NAME = u"game_redirect"_s;

        static inline const QString COL_ID = u"id"_s;
        static inline const QString COL_SOURCE_ID = u"sourceId"_s;

        static inline const QStringList COLUMN_LIST = {COL_ID, COL_SOURCE_ID};
    };


    class Table_Add_App
    {
    public:
        static inline const QString NAME = u"additional_app"_s;

        static inline const QString COL_ID = u"id"_s;
        static inline const QString COL_APP_PATH = u"applicationPath"_s;
        static inline const QString COL_AUTORUN = u"autoRunBefore"_s;
        static inline const QString COL_LAUNCH_COMMAND = u"launchCommand"_s;
        static inline const QString COL_NAME = u"name"_s;
        static inline const QString COL_WAIT_EXIT = u"waitForExit"_s;
        static inline const QString COL_PARENT_ID = u"parentGameId"_s;

        static inline const QStringList COLUMN_LIST = {COL_ID, COL_APP_PATH, COL_AUTORUN, COL_LAUNCH_COMMAND, COL_NAME, COL_WAIT_EXIT, COL_PARENT_ID};

        static inline const QString ENTRY_EXTRAS = u":extras:"_s;
        static inline const QString ENTRY_MESSAGE = u":message:"_s;
    };


    class Table_Game_Tags_Tag
    {
    public:
        static inline const QString NAME = u"game_tags_tag"_s;

        static inline const QString COL_GAME_ID = u"gameId"_s;
        static inline const QString COL_TAG_ID = u"tagId"_s;

        static inline const QStringList COLUMN_LIST = {COL_GAME_ID, COL_TAG_ID};
    };

    class Table_Tag
    {
    public:
        static inline const QString NAME = u"tag"_s;

        static inline const QString COL_ID = u"id"_s;
        static inline const QString COL_PRIMARY_ALIAS_ID = u"primaryAliasId"_s;
        static inline const QString COL_CATEGORY_ID = u"categoryId"_s;

        static inline const QStringList COLUMN_LIST = {COL_ID, COL_PRIMARY_ALIAS_ID, COL_CATEGORY_ID};
    };

    class Table_Tag_Alias
    {
    public:
        static inline const QString NAME = u"tag_alias"_s;

        static inline const QString COL_ID = u"id"_s;
        static inline const QString COL_TAG_ID = u"tagId"_s;
        static inline const QString COL_NAME = u"name"_s;

        static inline const QStringList COLUMN_LIST = {COL_ID, COL_TAG_ID, COL_NAME};
    };

    class Table_Tag_Category
    {
    public:
        static inline const QString NAME = u"tag_category"_s;

        static inline const QString COL_ID = u"id"_s;
        static inline const QString COL_NAME = u"name"_s;
        static inline const QString COL_COLOR = u"color"_s;

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
        QString category;
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
        QSet<int> excludedTagIds = {};
        bool includeAnimations = {};
    };

    struct EntryFilter
    {
        EntryType type = EntryType::PrimaryThenAddApp;
        QUuid id = {};
        QUuid parent = {};
        QString name = {};
        bool playableOnly = false;
        bool exactName = true;
    };

//-Class Variables-----------------------------------------------------------------------------------------------
private:
    static inline const QString DATABASE_CONNECTION_NAME = u"flashpoint_database"_s;

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
        {Db::Table_Game_Redirect::NAME, Db::Table_Game_Redirect::COLUMN_LIST},
    };
    static inline const QString GENERAL_QUERY_SIZE_COMMAND = u"COUNT(1)"_s;

    static inline const QString GAME_ONLY_FILTER = Db::Table_Game::COL_LIBRARY + u" = '"_s + Db::Table_Game::ENTRY_GAME_LIBRARY + u"'"_s;
    static inline const QString ANIM_ONLY_FILTER = Db::Table_Game::COL_LIBRARY + u" = '"_s + Db::Table_Game::ENTRY_ANIM_LIBRARY + u"'"_s;
    static inline const QString GAME_AND_ANIM_FILTER = u"("_s + GAME_ONLY_FILTER + u" OR "_s + ANIM_ONLY_FILTER + u")"_s;

    // Error
    static inline const QString ERR_MISSING_TABLE = u"The Flashpoint database is missing expected tables."_s;
    static inline const QString ERR_TABLE_MISSING_COLUMN = u"The Flashpoint database tables are missing expected columns."_s;
    static inline const QString ERR_ID_NOT_FOUND = u"An entry matching the specified ID could not be found in the Flashpoint database."_s;
    static inline const QString ERR_ID_DUPLICATE_ENTRY = u"This should not be possible and may indicate an error within the Flashpoint database"_s;

//-Instance Variables-----------------------------------------------------------------------------------------------
private:
    bool mValid;
    DbError mError;

    // Database information
    QSet<const QThread*> mConnectedThreads;
    const QString mDatabaseName;
    QStringList mPlatformNames;
    QStringList mPlaylistList;
    QMap<int, TagCategory> mTagDirectory; // Tag category id -> Tag category
    QHash<int, const Tag*> mTagMap; // Tag id -> Tag
    QHash<QUuid, QUuid> mGameRedirects;

//-Constructor-------------------------------------------------------------------------------------------------
public:
    explicit Db(const QString& databaseName, const Key&);

//-Destructor-------------------------------------------------------------------------------------------------
public:
    ~Db();

//-Instance Functions------------------------------------------------------------------------------------------------------
private:
    // Validity
    void nullify();

    // Connection
    void closeConnection(const QThread* thread);
    void closeAllConnections();
    QString threadConnectionName(const QThread* thread);
    QSqlError getThreadConnection(QSqlDatabase& connection);
    QSqlError makeNonBindQuery(QueryBuffer& resultBuffer, QSqlDatabase* database, const QString& queryCommand, const QString& sizeQueryCommand) const;

    // Init
    QSqlError checkDatabaseForRequiredTables(QSet<QString>& missingTablesBuffer);
    QSqlError checkDatabaseForRequiredColumns(QSet<QString>& missingColumsBuffer);
    QSqlError populateAvailableItems();
    QSqlError populateTags();
    QSqlError populateGameRedirects();

public:
    // Validity
    bool isValid();
    DbError error();

    // TODO: See if these query functions can be consolidated via by better filtration arguments

    // Queries - OFLIb
    DbError queryGamesByPlatform(QList<Db::QueryBuffer>& resultBuffer, const QStringList& platforms, const InclusionOptions& inclusionOptions,
                                   std::optional<const QList<QUuid>*> idInclusionFilter = std::nullopt);
    DbError queryAllAddApps(QueryBuffer& resultBuffer);
    DbError queryAllEntryTags(QueryBuffer& resultBuffer);

    // Queries - CLIFp
    DbError queryEntrys(QueryBuffer& resultBuffer, const EntryFilter& filter);
    DbError queryEntryDataById(QueryBuffer& resultBuffer, const QUuid& appId);
    DbError queryAllGameIds(QueryBuffer& resultBuffer, const LibraryFilter& filter);

    // Info
    QStringList platformNames() const;
    QMap<int, TagCategory> tags() const;

    // Checks
    DbError entryUsesDataPack(bool& resultBuffer, const QUuid& gameId);

    // Helper
    DbError getEntry(Entry& entry, const QUuid& entryId);
    DbError getGameData(GameData& data, const QUuid& gameId);
    DbError getGameTags(GameTags& tags, const QUuid& gameId);
    DbError updateGameDataOnDiskState(QList<int> packIds, bool onDisk);
    QUuid handleGameRedirects(const QUuid& gameId);

//-Slots ------------------------------------------------------------------------------------------------------
private:
    void connectedThreadDestroyed(QObject* thread);
};

}



#endif // FLASHPOINT_DB_H
