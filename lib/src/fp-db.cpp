// Unit Includes
#include "fp/fp-db.h"

// Qt Includes
#include <QSqlError>

// Qx Includes
#include <qx/core/qx-string.h>
#include <qx/core/qx-regularexpression.h>
#include <qx/core/qx-error.h>
#include <qx/core/qx-algorithm.h>
#include <qx/sql/qx-sqlquery.h>
#include <qx/sql/qx-sqlinlines.h>

namespace
{

struct GameRedirectSql
{
    QUuid id;
    QUuid sourceId;
};

struct GameTagsTagSql
{
    QUuid gameId;
    int tagId;
};

struct TagSql
{
    int id;
    int primaryAliasId;
    int categoryId;
};

struct TagAliasSql
{
    int id;
    int tagId;
    QString name;
};

struct TagCategorySql
{
    int id;
    QString name;
    QColor color;
};

}

QX_SQL_STRUCT_OUTSIDE_FULL(Fp::Game::Sql, "game", GameSqlQ,
    id, title, series, developer, publisher, dateAdded,
    dateModified, broken, playMode, status, notes, source,
    appPath, launchCommand, releaseDate, version,
    originalDescription, language, orderTitle, library,
    platformName, ruffleSupport
);

QX_SQL_STRUCT_OUTSIDE_FULL(Fp::GameData::Sql, "game_data", GameDataSqlQ,
    id, gameId, title, dateAdded, sha256, crc32, presentOnDisk, path,
    size, rawParameters, appPath, launchCommand
);

QX_SQL_MEMBER_OVERRIDE(Fp::GameData::Sql, dateAdded,
    static Qx::SqlError fromSql(QDateTime& dateAdded, const QVariant& v)
    {
        /* Have to bodge fractional time because of: https://github.com/qt/qtbase/blob/4e7f5c43a3be609502ccc15861319503dc2c842b/src/corelib/time/qdatetime.cpp#L2515
        * It seems that the converter was supposed to floor to the previous MS (which is what JS's Date() does), but qRound() was accidentally used,
        * and then afterwards additional functionality/tests were built around the fact that the conversion can round up, so a change being accepted
        * is potentially unlikely
        */

        QString cleanDate = v.toString();
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
        dateAdded = QDateTime::fromString(cleanDate, Qt::ISODateWithMs);

        return {};
    }

    static QVariant toSql(const QDateTime& dateAdded); // UNIMPLEMENTED
)

QX_SQL_STRUCT_OUTSIDE_FULL(Fp::AddApp::Sql, "additional_app", AddAppSqlQ,
    id, appPath, autorunBefore, launchCommand, name,
    waitExit, parentId,
);

QX_SQL_STRUCT_OUTSIDE_FULL(GameRedirectSql, "game_redirect", GameRedirectSqlQ,
    id, sourceId
);

QX_SQL_STRUCT_OUTSIDE_FULL(GameTagsTagSql, "game_tags_tag", GameTagsTagSqlQ,
    gameId, tagId
);

QX_SQL_STRUCT_OUTSIDE_FULL(TagSql, "tag", TagSqlQ,
    id, primaryAliasId, categoryId
);

QX_SQL_STRUCT_OUTSIDE_FULL(TagAliasSql, "tag_alias", TagAliasSqlQ,
    id, tagId, name
);

QX_SQL_STRUCT_OUTSIDE_FULL(TagCategorySql, "tag_category", TagCategorySqlQ,
    id, name, color
);

namespace QxSql
{
    // Some date-times in DB do not adhere to the standard
    template<>
    struct Converter<QDateTime>
    {
        static Qx::SqlError fromSql(QDateTime& dt, const QVariant& vValue)
        {
            QString dts = vValue.toString();

            static const QString DEFAULT_MONTH = u"-01"_s;
            static const QString DEFAULT_DAY = u"-01"_s;

            if(Qx::String::isOnlyNumbers(dts) && dts.length() == 4) // Year only
                dt = QDateTime::fromString(dts + DEFAULT_MONTH + DEFAULT_DAY, Qt::ISODateWithMs);
            else if(Qx::String::isOnlyNumbers(dts.left(4)) &&
                    Qx::String::isOnlyNumbers(dts.mid(5,2)) &&
                    dts.at(4) == '-' && dts.length() == 7) // Year and month only
                dt = QDateTime::fromString(dts + DEFAULT_DAY, Qt::ISODateWithMs);
            else if(Qx::String::isOnlyNumbers(dts.left(4)) &&
                    Qx::String::isOnlyNumbers(dts.mid(5,2)) &&
                    Qx::String::isOnlyNumbers(dts.mid(8,2)) &&
                    dts.at(4) == '-' && dts.at(7) == '-' && dts.length() == 10) // Year month and day
                dt = QDateTime::fromString(dts, Qt::ISODateWithMs);
            else
                dt = QDateTime(); // Invalid date provided

            return {};
        }

        static QVariant toSql(const QDateTime& value); // UNIMPLEMENTED
    };
}

namespace
{



template<typename T, typename F>
    requires std::same_as<std::invoke_result_t<F>, Fp::DbError>
Fp::DbError invokeWithCleanBuffer(T& buffer, F&& func)
{
    buffer = {};
    Fp::DbError err = func();
    if(err)
        buffer = {};

    return err;
}

void andWhere(Qx::SqlString& where, const Qx::SqlString& nd)
{
    if(where.isEmpty())
        where = nd;
    else
        where = where && nd;
}

}

namespace Fp
{

//===============================================================================================================
// DbError
//===============================================================================================================

//-Class Types-----------------------------------------------------------------------------------------------
template<>
struct Db::search_traits<Fp::Db::GameFilter>
{
    static inline const Qx::SqlString idString = GameSqlQ::id;
    using buffer_type = Fp::Game::Sql;
};

template<>
struct Db::search_traits<Fp::Db::AddAppFilter>
{
    static inline const Qx::SqlString idString = AddAppSqlQ::id;
    using buffer_type = Fp::AddApp::Sql;
};

//-Constructor------------------------------------------------------------------------------------------------
//Private:
DbError::DbError(Type t, const QString& c, const QString& d):
    mType(t),
    mCause(c),
    mDetails(d)
{}

DbError::DbError(const Qx::SqlError& e) :
    DbError(e.isValid() ? SqlError : NoError, e.cause(), e.query())
{}

DbError::DbError(const Qx::SqlSchemaReport& sr)
{
    if(!sr.hasDefects())
        mType = NoError;
    else
    {
        // "Cheat" with Qx::Error
        Qx::Error err(sr);
        mType = InvalidSchema;
        mCause = err.secondary();
        mDetails = err.details();
    }
}

//Public:
DbError::DbError() :
    mType(NoError)
{}

//-Instance Functions------------------------------------------------------------------------------------------------
//Private:
Qx::Severity DbError::deriveSeverity() const { return Qx::Critical; }
quint32 DbError::deriveValue() const { return mType; }
QString DbError::derivePrimary() const { return ERR_STRINGS.value(mType); }
QString DbError::deriveSecondary() const { return mCause; }
QString DbError::deriveDetails() const { return mDetails; }

//Public:
bool DbError::isValid() const { return mType != NoError; }
DbError::Type DbError::type() const { return mType; }
QString DbError::cause() const { return mCause; }
QString DbError::details() const { return mDetails; }


//===============================================================================================================
// DB::TAG_CATEGORY
//===============================================================================================================

//-Operators----------------------------------------------------------------------------------------------------
//Public:
bool operator< (const Db::TagCategory& lhs, const Db::TagCategory& rhs) noexcept { return lhs.name < rhs.name; }

//===============================================================================================================
// DB
//===============================================================================================================

//-Constructor------------------------------------------------------------------------------------------------
//Public:
Db::Db(const QString& databaseName, const Key&) :
    mValid(false), // Instance is invalid until proven otherwise
    mDatabase(databaseName, u"QSQLITE"_s)
{
    QScopeGuard validityGuard([this](){ nullify(); }); // Automatically nullify on fail

    // Error tracker
    Qx::SqlError databaseError;

    // Validate schema
    Qx::SqlSchemaReport schRep;
    if(databaseError = mDatabase.checkSchema<
            Game::Sql,
            GameData::Sql,
            AddApp::Sql,
            GameRedirectSql,
            GameTagsTagSql,
            TagSql,
            TagAliasSql,
            TagCategorySql
        >(schRep); databaseError.isValid())
    {
        mError = databaseError;
        return;
    }
    if(schRep.hasDefects())
    {
        mError = schRep;
        return;
    }

    // Populate item members
    if((databaseError = populateAvailablePlatforms()).isValid())
    {
        mError = databaseError;
        return;
    }

    if((databaseError = populateTags()).isValid())
    {
        mError = databaseError;
        return;
    }

    // Populate game redirects
    if((databaseError = populateGameRedirects()).isValid())
    {
        mError = databaseError;
        return;
    }

    // Give the ok
    mValid = true;
    validityGuard.dismiss();
}

//-Destructor------------------------------------------------------------------------------------------------
//Public:
Db::~Db() {}

//-Instance Functions------------------------------------------------------------------------------------------------
//Private:
void Db::nullify()
{
    mPlatformNames.clear();
    mTagDirectory.clear();
}

Qx::SqlError Db::populateAvailablePlatforms()
{   
    // Query
    auto err = mDatabase.SELECT_DISTINCT(GameSqlQ::platformName)
                        .FROM<Game::Sql>()
                        .execute(mPlatformNames);
    if(err)
        return err;

    // Sort list
    mPlatformNames.sort();

    // Return invalid SqlError
    return Qx::SqlError();
}

Qx::SqlError Db::populateTags()
{
    // Ensure directory is reset
    mTagDirectory.clear();

    QMap<int, QString> tagAliasMap; // Tag Alias ID -> Tag Alias Name

    // Make tag category query
    auto catQuery = mDatabase.SELECT<TagCategorySql>()
                             .FROM<TagCategorySql>();
    Qx::SqlResult<TagCategorySql> catQueryRes;
    if(auto err = catQuery.execute(catQueryRes); err.isValid())
        return err;

    // Parse query
    while(catQueryRes.next())
    {
        TagCategorySql sql;
        if(auto err = catQueryRes.value(sql); err.isValid())
            return err;

        TagCategory tc{
            .name = sql.name,
            .color = sql.color,
            .tags = {}
        };
        mTagDirectory[sql.id] = tc;
    }

    // Make tag alias query
    auto aliasQuery = mDatabase.SELECT<TagAliasSql>()
                               .FROM<TagAliasSql>();
    Qx::SqlResult<TagAliasSql> aliasQueryRes;
    if(auto err = aliasQuery.execute(aliasQueryRes); err.isValid())
        return err;

    // Parse query
    while(aliasQueryRes.next())
    {
        TagAliasSql sql;
        if(auto err = aliasQueryRes.value(sql); err.isValid())
            return err;

        tagAliasMap[sql.id] = sql.name;
    }

    // Make tag query
    auto tagQuery = mDatabase.SELECT<TagSql>()
                               .FROM<TagSql>();
    Qx::SqlResult<TagSql> tagQueryRes;
    if(auto err = tagQuery.execute(tagQueryRes); err.isValid())
        return err;

    // Parse query
    while(tagQueryRes.next())
    {
        TagSql sql;
        if(auto err = tagQueryRes.value(sql); err.isValid())
            return err;

        // Create Tag
        Tag tag{
            .id = sql.id,
            .primaryAlias = tagAliasMap.value(sql.primaryAliasId),
            .category = {}
        };
        int catId = sql.categoryId;
        Q_ASSERT(mTagDirectory.contains(catId));
        TagCategory& tc = mTagDirectory[catId];
        tag.category = tc.name; // CoW reduces overhead

        // Insert and add pointer to tag map
        Tag& insertedTag = tc.tags.insert(tag.id, tag).value();
        mTagMap[insertedTag.id] = &insertedTag;
    }

    // Return invalid SqlError
    return Qx::SqlError();
}

Qx::SqlError Db::populateGameRedirects()
{
    // Ensure map is reset
    mGameRedirects.clear();

    // Make redirect query
    auto redirectQuery = mDatabase.SELECT<GameRedirectSql>()
                                  .FROM<GameRedirectSql>();
    Qx::SqlResult<GameRedirectSql> redirectQueryRes;
    if(auto err = redirectQuery.execute(redirectQueryRes); err.isValid())
        return err;

    // Parse query
    while(redirectQueryRes.next())
    {
        GameRedirectSql sql;
        if(auto err = redirectQueryRes.value(sql); err.isValid())
            return err;

        if(sql.sourceId.isNull())
            continue;

        if(sql.id.isNull())
            continue;

        mGameRedirects[sql.sourceId] = sql.id;
    }

    // Return invalid SqlError
    return Qx::SqlError();
}

void Db::prepareSearchQuery(Qx::SqlDqlQuery& query, const Fp::Db::GameFilter& filter)
{
    using namespace QxSql;

    // Starts after SELECT...
    query.FROM<Game::Sql>();

    if(filter.title.isNull() && filter.platforms.isEmpty() && filter.excludedTagIds.isEmpty() &&
       filter.includedIds.isEmpty() && filter.includeAnimations)
        return;

    // Build WHERE
    Qx::SqlString where;
    if(!filter.title.isNull())
    {
        if(filter.exactName)
            andWhere(where, GameSqlQ::title == sqs(filter.title));
        else
        {
            // Escape name to account for SQL LITE %
            QString escapedName = filter.title;
            escapedName.replace(uR"(\)"_s, uR"(\\)"_s); // Have to escape the escape char
            escapedName.replace(uR"(%)"_s, uR"(\%)"_s);

            // Make LIKE param
            auto like = sqs(u"%%1%"_s.arg(filter.title));

            andWhere(where, GameSqlQ::title |= LIKE(like) |= ESCAPE(u"\\"_sqs));
        }
    }
    if(!filter.platforms.isEmpty())
        andWhere(where, GameSqlQ::platformName |= IN(filter.platforms));
    if(!filter.excludedTagIds.isEmpty())
    {
        // Use subquery to exclude game ids that match the tag ids.
        Qx::SqlDqlQuery tagSubQuery;
        tagSubQuery.SELECT(GameTagsTagSqlQ::gameId)
          .FROM<GameTagsTagSql>()
          .WHERE(GameTagsTagSqlQ::tagId |= IN(filter.excludedTagIds));

        andWhere(where, GameSqlQ::id |= !IN(tagSubQuery));
    }
    if(!filter.includedIds.isEmpty())
        andWhere(where, GameSqlQ::id |= IN(filter.includedIds));
    if(!filter.includeAnimations)
        andWhere(where, GameSqlQ::library != sqs(Game::Sql::ENTRY_ANIM_LIBRARY));

    query.WHERE(where);
}

void Db::prepareSearchQuery(Qx::SqlDqlQuery& query, const Fp::Db::AddAppFilter& filter)
{
    // Starts after SELECT...
    using namespace QxSql;

    // Starts after SELECT...
    query.FROM<AddApp::Sql>();

    if(filter.name.isNull() && filter.parent.isNull() && !filter.playableOnly)
        return;

    // Build WHERE
    Qx::SqlString where;
    if(!filter.name.isNull())
    {
        if(filter.exactName)
            andWhere(where, AddAppSqlQ::name == sqs(filter.name));
        else
        {
            // Escape name to account for SQL LITE %
            QString escapedName = filter.name;
            escapedName.replace(uR"(\)"_s, uR"(\\)"_s); // Have to escape the escape char
            escapedName.replace(uR"(%)"_s, uR"(\%)"_s);

            // Make LIKE param
            auto like = sqs(u"%%1%"_s.arg(filter.name));

            andWhere(where, AddAppSqlQ::name |= LIKE(like) |= ESCAPE(u"\\"_sqs));
        }
    }
    if(!filter.parent.isNull())
        andWhere(where, AddAppSqlQ::parentId == filter.parent);
    if(filter.playableOnly)
        andWhere(where, AddAppSqlQ::autorunBefore != true && AddAppSqlQ::appPath |= !IN(sqs(AddApp::Sql::ENTRY_EXTRAS), sqs(AddApp::Sql::ENTRY_MESSAGE)));

    query.WHERE(where);
}


template<typename T, typename F>
    requires Qx::any_of<T, Game, AddApp, Entry>
Fp::DbError Db::searchImpl(QList<T>& buffer, const F& filter)
{
    // Clear buffer
    buffer.clear();

    // Make query
    using buffer_t = typename search_traits<F>::buffer_type;
    auto query = mDatabase.SELECT<buffer_t>();
    prepareSearchQuery(query, filter);
    Qx::SqlResult<buffer_t> queryRes;
    if(auto err = query.execute(queryRes); err.isValid())
        return err;

    // Parse query
    QList<T> wipBuffer;
    while(queryRes.next())
    {
        buffer_t sql;
        if(auto err = queryRes.value(sql); err.isValid())
            return err;

        wipBuffer.append(std::move(sql));
    }

    buffer = wipBuffer;
    return DbError();
}

template<typename F>
Fp::DbError Db::searchImpl(QList<QUuid>& buffer, const F& filter)
{
    using trs = search_traits<F>;
    auto query = mDatabase.SELECT(trs::idString);
    prepareSearchQuery(query, filter);
    return query.execute(buffer);
}

template<typename T>
    requires Qx::any_of<T, Entry, QUuid>
DbError Db::searchEntryImpl(QList<T>& buffer, const EntryFilter& filter)
{
    auto checkGames = [&]{
        return filter.type == EntryType::Game || filter.type == EntryType::GameThenAddApp || filter.type == EntryType::GameAndAddApp;
    };
    auto checkAddApps = [&]{
        return filter.type == EntryType::GameAndAddApp || (filter.type == EntryType::GameThenAddApp && buffer.isEmpty());
    };

    if(checkGames())
    {
        GameFilter gf{.title = filter.name, .exactName = filter.exactName, .platforms = filter.platforms, .excludedTagIds = filter.excludedTagIds,
                      .includedIds = filter.includedIds, .includeAnimations = filter.includeAnimations};
        if(auto err = searchImpl(buffer, gf); err.isValid())
            return err;
    }

    if(checkAddApps())
    {
        AddAppFilter aaf{.name = filter.name, .exactName = filter.exactName, .parent = filter.parent, .playableOnly = filter.playableOnly};
        if(auto err = searchImpl(buffer, aaf); err.isValid())
            return err;
    }

    return {};
}

template<typename T>
    requires Qx::any_of<T, Game, AddApp>
DbError Db::acquireImpl(T& buffer, const QUuid& id)
{
    auto sqlId = std::same_as<T, Game> ? GameSqlQ::id : AddAppSqlQ::id;
    using buffer_t = typename T::Sql;

    // Make query
    auto query = mDatabase.SELECT<buffer_t>()
                          .template FROM<buffer_t>()
                          .WHERE(sqlId == id);

    QList<buffer_t> queryRes;
    if(auto err = query.execute(queryRes); err.isValid())
        return err;

    // Check if ID was found and that only one instance was found
    if(queryRes.size() == 0)
        return DbError(DbError::IncompleteSearch, ERR_ID_NOT_FOUND);
    else if(queryRes.size() > 1)
        return DbError(DbError::IdCollision, ERR_ID_DUPLICATE_ENTRY);

    buffer = std::move(queryRes.first());
    return {};
}

//Public:
bool Db::isValid() { return mValid; }
DbError Db::error() { return mError; }

DbError Db::searchGames(QList<Game>& games, const GameFilter& filter)
{
    return invokeWithCleanBuffer(games, [&]{ return searchImpl(games, filter); });
}

DbError Db::searchGameIds(QList<QUuid>& gameIds, const GameFilter& filter)
{
    return invokeWithCleanBuffer(gameIds, [&]{ return searchImpl(gameIds, filter); });
}

DbError Db::searchAddApps(QList<AddApp>& addApps, const AddAppFilter& filter)
{
    return invokeWithCleanBuffer(addApps, [&]{ return searchImpl(addApps, filter); });
}

DbError Db::searchAddAppIds(QList<QUuid>& addAppIds, const AddAppFilter& filter)
{
    return invokeWithCleanBuffer(addAppIds, [&]{ return searchImpl(addAppIds, filter); });
}

DbError Db::searchEntries(QList<Entry>& entries, const EntryFilter& filter)
{
    return invokeWithCleanBuffer(entries, [&]{ return searchEntryImpl(entries, filter); });
}


DbError Db::searchEntryIds(QList<QUuid>& entryIds, const EntryFilter& filter)
{
    return invokeWithCleanBuffer(entryIds, [&]{ return searchEntryImpl(entryIds, filter); });
}

QStringList Db::platformNames() const { return mPlatformNames; } //TODO: Probably should use RAII for this.
QMap<int, Db::TagCategory> Db::tags() const { return mTagDirectory; }

DbError Db::entryUsesDataPack(bool& resultBuffer, const QUuid& gameId)
{
    /* NOTE: The launcher performs this check and other data pack tasks by checking if the `activeDataId` column
     * of the `game` table has a value, and if it does, then matching that to the `id` column in the `game_data`
     * table to get the game's data pack info. This requires slightly less processing, but the way it's done here
     * is ultimately fine and technically handles typos/errors in the database slightly better since it's a more
     * direct check. Ultimately this should be switched over to the official method though.
     *
     * Also not sure what the `activeDataOnDisk` (`game`) and `presentOnDisk` (`game_data`) columns are for. At
     * first glance they seem to keep track on if the data pack for a given game is available (has been downloaded),
     * as the Launcher's setup for removing data packs manipulates these fields
     * (see https://github.com/FlashpointProject/launcher/blob/9937201594ace7aeccea6e511127d91f6deefd4e/src/back/responses.ts#L570)m
     * but this doesn't make entire sense given that Infinity comes with no data packs installed and yet many entries
     * in the database have these values set to 1 by default.
     */

    // Default return buffer to false
    resultBuffer = false;

    // Make query
    using namespace QxSql;
    auto packCheckQuery = mDatabase.SELECT(COUNT(1))
                                   .FROM<GameData::Sql>()
                                   .WHERE(GameDataSqlQ::gameId == gameId);
    int packCheckQueryRes;
    if(auto err = packCheckQuery.execute(packCheckQueryRes); err.isValid())
        return err;

    resultBuffer = packCheckQueryRes > 0;

    // Return invalid error
    return DbError();
}

DbError Db::getGame(Game& game, const QUuid& gameId)
{
    return invokeWithCleanBuffer(game, [&]{ return acquireImpl(game, gameId); } );
}

DbError Db::getAddApp(AddApp& addApp, const QUuid& addAppId)
{
    return invokeWithCleanBuffer(addApp, [&]{ return acquireImpl(addApp, addAppId); } );
}

DbError Db::getEntry(Entry& entry, const QUuid& entryId)
{
    /* Game, then AddApp
     * TODO: Improve the "no result" check here
     */

    entry = {};

    Game g;
    if(auto err = acquireImpl(g, entryId); err.isValid())
    {
        if(err.type() != DbError::IncompleteSearch) // Error that isn't "no result"
            return err;
    }
    else
    {
        entry = g;
        return {};
    }

    AddApp aa;
    if(auto err = acquireImpl(aa, entryId); err.isValid())
        return err;

    entry = aa;
    return {};
}

DbError Db::getGameData(GameData& data, const QUuid& gameId)
{
    // Clear buffer
    data = GameData();

    // Make query
    using namespace QxSql;
    auto packCheckQuery = mDatabase.SELECT<GameData::Sql>()
                                   .FROM<GameData::Sql>()
                                   .WHERE(GameDataSqlQ::gameId == gameId)
                                   .ORDER_BY(GameDataSqlQ::dateAdded |= DESC());
    QList<GameData::Sql> packCheckQueryRes;
    if(auto err = packCheckQuery.execute(packCheckQueryRes); err.isValid())
        return err;

    // Check if ID was found and if so that only one instance was found
    if(packCheckQueryRes.size() == 0)
        return DbError(); // Game doesn't have data pack
    else if(packCheckQueryRes.size() > 1)
        qWarning("Entry %s has more than one data pack, using most recent.", qPrintable(gameId.toString(QUuid::WithoutBraces)));

    data = GameData(packCheckQueryRes.takeFirst());

    return DbError();
}

DbError Db::getGameTags(GameTags& tags, const QUuid& gameId)
{
    // Clear buffer
    tags = GameTags();

    // Make query
    using namespace QxSql;
    auto tagQuery = mDatabase.SELECT<GameTagsTagSql>()
                             .FROM<GameTagsTagSql>()
                             .WHERE(GameTagsTagSqlQ::gameId == gameId);
    Qx::SqlResult<GameTagsTagSql> tagQueryRes;
    if(auto err = tagQuery.execute(tagQueryRes); err.isValid())
        return err;

    // Parse query
    GameTags wipTags;
    while(tagQueryRes.next())
    {
        GameTagsTagSql sql;
        if(auto err = tagQueryRes.value(sql); err.isValid())
            return err;

        int tagId = sql.tagId;
        auto tagItr = mTagMap.constFind(tagId);
        if(tagItr != mTagMap.constEnd())
        {
            auto tag = *tagItr;
            wipTags.addTag(tag->category, tag->primaryAlias);
        }
        else
            qWarning("Table %s contains invalid tag ID %d for game %s", qPrintable(GameTagsTagSqlQ::_.toString()), tagId, qPrintable(gameId.toString()));
    }
    tags = wipTags;

    return DbError();
}

DbError Db::getAllGameIds(QList<QUuid>& ids, const LibraryFilter& filter)
{
    // Clear buffer
    ids = {};

    // Make query
    using namespace QxSql;
    auto sqlFilter = GameSqlQ::status != sqi(Game::Sql::ENTRY_NOT_WORK);
    if(filter == LibraryFilter::Game)
        sqlFilter = sqlFilter && (GameSqlQ::library == sqs(Game::Sql::ENTRY_GAME_LIBRARY));
    else if(filter == LibraryFilter::Anim)
        sqlFilter = sqlFilter && (GameSqlQ::library == sqs(Game::Sql::ENTRY_ANIM_LIBRARY));

    return mDatabase.SELECT(GameSqlQ::id)
                    .FROM<Game::Sql>()
                    .WHERE(std::as_const(sqlFilter))
                    .execute(ids);
}

DbError Db::getAllAddApps(QList<AddApp>& addApps)
{
    // Clear buffer
    addApps = {};

    // Make query
    //using namespace QxSql;
    auto addAppsQuery = mDatabase.SELECT<AddApp::Sql>()
                                 .FROM<AddApp::Sql>();
    Qx::SqlResult<AddApp::Sql> addAppsQueryRes;
    if(auto err = addAppsQuery.execute(addAppsQueryRes); err.isValid())
        return err;

    // Parse query
    while(addAppsQueryRes.next())
    {
        AddApp::Sql sql;
        if(auto err = addAppsQueryRes.value(sql); err.isValid())
            return err;

        addApps.append(std::move(sql));
    }

    return DbError();
}

DbError Db::updateGameDataOnDiskState(QList<int> packIds, bool onDisk)
{
    // Make query
    auto updateQuery = mDatabase.UPDATE<GameData::Sql>()
                                .SET(GameDataSqlQ::presentOnDisk == onDisk)
                                .WHERE(GameDataSqlQ::id).IN(packIds);
    int tagQueryAffected;
    if(auto err = updateQuery.execute(tagQueryAffected); err.isValid())
        return err;

    // Check that expected count was affected
    int expected = packIds.size();
    if(tagQueryAffected != expected)
        return DbError(DbError::UpdateRowMismatch, GameDataSqlQ::_.toString() + u" SET "_s + GameDataSqlQ::presentOnDisk.toString(), u"%1 instead of %2"_s.arg(tagQueryAffected, expected));

    return DbError();
}

/* TODO: Technically this is a shortcut. The regular launcher will check for Game Redirects in all cases where an ID is searched
 * for, often using coalesce (see https://github.com/FlashpointProject/FPA-Rust/blob/03a4ddc4af9ae0b2773c5f678268cb9c944d893f/crates/flashpoint-archive/src/game/mod.rs#L323).
 * This makes sense if anyone is using this lib for any reason (which although that is the intention, currently no one is); but in the case of CLIFp/FIL, where
 * IDs are only sought out directly, or in bulk, we can just swap the target ID (if a redirect is present) before even hitting the database with it.
 * Just keep in mind the ideal long term thing to do is have the redirects considered whenever checking the database for a game ID at all. This could
 * also be an issue if a source ID is still used somewhere else in the DB, for example add_app or game_data, but that does not seem to be the case currently.
 */
QUuid Db::handleGameRedirects(const QUuid& gameId) { return mGameRedirects.value(gameId, gameId); }

}
