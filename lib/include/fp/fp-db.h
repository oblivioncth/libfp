#ifndef FLASHPOINT_DB_H
#define FLASHPOINT_DB_H

// Shared Lib Support
#include "fp/fp_export.h"

// Qt Includes
#include <QStringList>
#include <QColor>

// Qx Includes
#include <qx/core/qx-abstracterror.h>
#include <qx/sql/qx-sqldatabase.h>

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

//-Constructor-------------------------------------------------------------
private:
    DbError(Type t, const QString& c, const QString& d = {});
    DbError(const Qx::SqlError& e);
    DbError(const Qx::SqlSchemaReport& sr);

public:
    DbError();

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

class FP_FP_EXPORT Db
{
//-Inner Classes-------------------------------------------------------------------------------------------------
public:
    class Key
    {
        friend class Install;
    private:
        Key() {};
        Key(const Key&) = default;
    };

//-Class Types---------------------------------------------------------------------------------------------------
private:
    template<typename F>
    struct search_traits;

public:
    enum class EntryType{ Game, AddApp, GameThenAddApp, GameAndAddApp };

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

    struct GameFilter
    {
        QString title = {};
        bool exactName = true;
        //bool playableOnly = false; // TODO Implement, though 99.9% of games are playable so meh
        QStringList platforms = {};
        QList<int> excludedTagIds = {};
        QList<QUuid> includedIds = {}; // Explicit set of Game IDs to consider
        bool includeAnimations = {};
    };

    struct AddAppFilter
    {
        QString name = {};
        bool exactName = true;
        QUuid parent = {};
        bool playableOnly = false;
    };

    struct EntryFilter
    {
        // Specific
        EntryType type = EntryType::GameThenAddApp;

        // Common
        QString name = {};
        bool exactName = true;
        bool playableOnly = false;

        // Game
        QStringList platforms = {};
        QList<int> excludedTagIds = {};
        QList<QUuid> includedIds = {}; // Explicit set of Game IDs to consider
        bool includeAnimations = {};

        // Add App
        QUuid parent = {};
    };

//-Class Variables-----------------------------------------------------------------------------------------------
private:
    // Error
    static inline const QString ERR_ID_NOT_FOUND = u"An entry matching the specified ID could not be found in the Flashpoint database."_s;
    static inline const QString ERR_ID_DUPLICATE_ENTRY = u"This should not be possible and may indicate an error within the Flashpoint database"_s;

//-Instance Variables-----------------------------------------------------------------------------------------------
private:
    bool mValid;
    DbError mError;

    // Database information
    Qx::SqlDatabase mDatabase;
    QStringList mPlatformNames;
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

    // Init
    Qx::SqlError populateAvailablePlatforms();
    Qx::SqlError populateTags();
    Qx::SqlError populateGameRedirects();

    // Query
    void prepareSearchQuery(Qx::SqlDqlQuery& query, const Fp::Db::GameFilter& filter);
    void prepareSearchQuery(Qx::SqlDqlQuery& query, const Fp::Db::AddAppFilter& filter);

    template<typename T, typename F>
        requires Qx::any_of<T, Game, AddApp, Entry>
    DbError searchImpl(QList<T>& buffer, const F& filter);

    template<typename F>
    DbError searchImpl(QList<QUuid>& buffer, const F& filter);

    template<typename T>
        requires Qx::any_of<T, Entry, QUuid>
    DbError searchEntryImpl(QList<T>& buffer, const EntryFilter& filter);

    template<typename T>
        requires Qx::any_of<T, Game, AddApp>
    DbError acquireImpl(T& buffer, const QUuid& id);

public:
    // Validity
    bool isValid();
    DbError error();

    // Info
    QStringList platformNames() const;
    QMap<int, TagCategory> tags() const;

    // Read
    DbError searchGames(QList<Game>& games, const GameFilter& filter);
    DbError searchGameIds(QList<QUuid>& gameIds, const GameFilter& filter);
    DbError searchAddApps(QList<AddApp>& addApps, const AddAppFilter& filter);
    DbError searchAddAppIds(QList<QUuid>& addAppIds, const AddAppFilter& filter);
    DbError searchEntries(QList<Entry>& entries, const EntryFilter& filter);
    DbError searchEntryIds(QList<QUuid>& entryIds, const EntryFilter& filter);

    DbError getGame(Game& game, const QUuid& gameId);
    DbError getAddApp(AddApp& addApp, const QUuid& addAppId);
    DbError getEntry(Entry& entry, const QUuid& entryId);
    DbError getGameData(GameData& data, const QUuid& gameId);
    DbError getGameTags(GameTags& tags, const QUuid& gameId);
    DbError getAllGameIds(QList<QUuid>& ids, const Libraries& filter);
    DbError getAllAddApps(QList<AddApp>& addApps);
    DbError getAllEntryTags();

    // Write
    DbError updateGameDataOnDiskState(QList<int> packIds, bool onDisk);
    DbError updateGamePlayRecords(const QUuid& gameId, qint64 additionalSeconds);

    // Helper
    DbError entryUsesDataPack(bool& resultBuffer, const QUuid& gameId);
    QUuid handleGameRedirects(const QUuid& gameId);
};

}



#endif // FLASHPOINT_DB_H
