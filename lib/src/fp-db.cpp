// Unit Includes
#include "fp/fp-db.h"

// Qx Includes
#include <qx/core/qx-string.h>
#include <qx/core/qx-regularexpression.h>

namespace Fp
{

//===============================================================================================================
// DbError
//===============================================================================================================

//-Constructor------------------------------------------------------------------------------------------------
//Private:
DbError::DbError(Type t, const QString& c, const QString& d):
    mType(t),
    mCause(c),
    mDetails(d)
{}

//Public:
DbError::DbError() :
    mType(NoError)
{}

//-Class Functions---------------------------------------------------------------
//Private:
DbError DbError::fromSqlError(const QSqlError& e)
{
    return e.isValid() ? DbError(SqlError, e.text()) : DbError();
}

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
    QObject(),
    mValid(false), // Instance is invalid until proven otherwise
    mDatabaseName(databaseName)
{
    QScopeGuard validityGuard([this](){ nullify(); }); // Automatically nullify on fail

    // Error tracker
    QSqlError databaseError;

    // Ensure required database tables are present
    QSet<QString> missingTables;
    if((databaseError = checkDatabaseForRequiredTables(missingTables)).isValid())
    {
        mError = DbError(DbError::SqlError, databaseError.text());
        return;
    }

    // Check if tables are missing
    if(!missingTables.isEmpty())
    {
        mError = DbError(DbError::InvalidSchema, ERR_MISSING_TABLE,
                         QStringList(missingTables.begin(), missingTables.end()).join(u"\n"_s));
        return;
    }

    // Ensure the database contains the required columns
    QSet<QString> missingColumns;
    if((databaseError = checkDatabaseForRequiredColumns(missingColumns)).isValid())
    {
        mError = DbError(DbError::SqlError, databaseError.text());
        return;
    }

    // Check if columns are missing
    if(!missingColumns.isEmpty())
    {
        mError = DbError(DbError::InvalidSchema, ERR_MISSING_TABLE,
                         QStringList(missingColumns.begin(), missingColumns.end()).join(u"\n"_s));
        return;
    }

    // Populate item members
    if((databaseError = populateAvailableItems()).isValid())
    {
        mError = DbError(DbError::SqlError, databaseError.text());
        return;
    }

    if((databaseError = populateTags()).isValid())
    {
        mError = DbError(DbError::SqlError, databaseError.text());
        return;
    }

    // Give the ok
    mValid = true;
    validityGuard.dismiss();
}

//-Destructor------------------------------------------------------------------------------------------------
//Public:
Db::~Db()
{
    closeAllConnections();
}

//-Class Functions--------------------------------------------------------------------------------------------
//Private:
QString Db::threadConnectionName(const QThread* thread)
{
    // Important to also salt using instance "id" so that
    // different instances don't use the same connection
   return DATABASE_CONNECTION_NAME +
           u"_i"_s + QString::number((quint64)this, 16) +
           u"_t"_s + QString::number((quint64)thread, 16);
}


//-Instance Functions------------------------------------------------------------------------------------------------
//Private:
void Db::nullify()
{
   mPlatformNames.clear();
    mPlaylistList.clear();
    mTagMap.clear();
}

void Db::closeConnection(const QThread* thread)
{
    if(mConnectedThreads.contains(thread))
    {
        QString tcn = threadConnectionName(thread);

        {
            /* Scoped because the following QSqlDatabase instance must not exist when the database connection
             * is removed, (as all connection instances must be deleted before using removeDatabase()), or else
             * Qt will post a warning since any instances that remain then have a stale reference to the database.
             */
            QSqlDatabase connection = QSqlDatabase::database(tcn, false);
            connection.close();
        }

        QSqlDatabase::removeDatabase(tcn);
        mConnectedThreads.remove(thread);
    }
}

void Db::closeAllConnections()
{
    QSet<const QThread*>::iterator i;
    while(i != mConnectedThreads.end())
    {
        closeConnection(*i);
        i = mConnectedThreads.erase(i);
    }
}

QSqlError Db::getThreadConnection(QSqlDatabase& connection)
{
    QThread* thread = QThread::currentThread();
    QString tcn = threadConnectionName(thread);

    if(mConnectedThreads.contains(thread))
    {
        connection = QSqlDatabase::database(tcn, false);
        return QSqlError();
    }
    else
    {
        connection = QSqlDatabase::addDatabase(u"QSQLITE"_s, tcn);
        //connection.setConnectOptions("QSQLITE_OPEN_READONLY"); Lib features some DB writing now
        connection.setDatabaseName(mDatabaseName);

        if(connection.open())
        {
            mConnectedThreads.insert(thread);
            connect(thread, &QThread::destroyed, this, &Db::connectedThreadDestroyed);
            return QSqlError();
        }
        else
        {
            /* Grab error first because I'm not sure if the QSqlDatabase instance
             * is completely valid once its underlying connection is removed
             */
            QSqlError openError = connection.lastError();
            QSqlDatabase::removeDatabase(tcn);
            connection = QSqlDatabase();
            return openError;
        }
    }
}

QSqlError Db::makeNonBindQuery(QueryBuffer& resultBuffer, QSqlDatabase* database, const QString& queryCommand, const QString& sizeQueryCommand) const
{
    // Create main query
    QSqlQuery mainQuery(*database);
    mainQuery.setForwardOnly(true);
    mainQuery.prepare(queryCommand);

    // Execute query and return if error occurs
    if(!mainQuery.exec())
        return mainQuery.lastError();

    // Create size query
    QSqlQuery sizeQuery(*database);
    sizeQuery.setForwardOnly(true);
    sizeQuery.prepare(sizeQueryCommand);

    // Execute query and return if error occurs
    if(!sizeQuery.exec())
        return sizeQuery.lastError();

    // Get query size
    sizeQuery.next();
    int querySize = sizeQuery.value(0).toInt();

    // Set buffer instance to result
    resultBuffer.result = mainQuery;
    resultBuffer.size = querySize;

    // Return invalid SqlError
    return QSqlError();
}

//Public:
bool Db::isValid() { return mValid; }
DbError Db::error() { return mError; }

QSqlError Db::checkDatabaseForRequiredTables(QSet<QString>& missingTablesReturnBuffer)
{
    // Prep return buffer
    missingTablesReturnBuffer.clear();

    for(const TableSpecs& tableAndColumns : DATABASE_SPECS_LIST)
        missingTablesReturnBuffer.insert(tableAndColumns.name);

    // Get database
    QSqlDatabase fpDb;
    QSqlError dbError = getThreadConnection(fpDb);
    if(dbError.isValid())
        return dbError;

    QStringList existingTables = fpDb.tables();

    // Return if DB error occurred
    if(fpDb.lastError().isValid())
        return fpDb.lastError();

    for(const QString& table : existingTables)
        missingTablesReturnBuffer.remove(table);

    // Return an invalid error
    return  QSqlError();
}

QSqlError Db::checkDatabaseForRequiredColumns(QSet<QString>& missingColumsReturnBuffer)
{

    // Ensure return buffer starts empty
    missingColumsReturnBuffer.clear();

    // Get database
    QSqlDatabase fpDb;
    QSqlError dbError = getThreadConnection(fpDb);
    if(dbError.isValid())
        return dbError;

    // Ensure each table has the required columns
    QSet<QString> existingColumns;

    for(const TableSpecs& tableAndColumns : DATABASE_SPECS_LIST)
    {
        // Clear previous data
        existingColumns.clear();

        // Make column name query
        QSqlQuery columnQuery(u"PRAGMA table_info("_s + tableAndColumns.name + u")"_s, fpDb);

        // Return if error occurs
        if(columnQuery.lastError().isValid())
            return columnQuery.lastError();

        // Parse query
        while(columnQuery.next())
            existingColumns.insert(columnQuery.value(u"name"_s).toString());

        // Check for missing columns
        for(const QString& column : tableAndColumns.columns)
            if(!existingColumns.contains(column))
                missingColumsReturnBuffer.insert(tableAndColumns.name + u": "_s + column);
    }


    // Return invalid SqlError
    return QSqlError();
}

QSqlError Db::populateAvailableItems()
{
    // Get database
    QSqlDatabase fpDb;
    QSqlError dbError = getThreadConnection(fpDb);
    if(dbError.isValid())
        return dbError;

    // Ensure lists are reset
    mPlatformNames.clear();
    mPlaylistList.clear();

    // Make platform query
    QSqlQuery platformQuery(u"SELECT DISTINCT "_s + Table_Game::COL_PLATFORM_NAME + u" FROM "_s + Table_Game::NAME, fpDb);

    // Return if error occurs
    if(platformQuery.lastError().isValid())
        return platformQuery.lastError();

    // Parse query
    while(platformQuery.next())
        mPlatformNames.append(platformQuery.value(Table_Game::COL_PLATFORM_NAME).toString());

    // Sort list
    mPlatformNames.sort();

    // Return invalid SqlError
    return QSqlError();
}

QSqlError Db::populateTags()
{
    // Get database
    QSqlDatabase fpDb;
    QSqlError dbError = getThreadConnection(fpDb);
    if(dbError.isValid())
        return dbError;

    // Ensure list is reset
    mTagMap.clear();

    // Temporary id map
    QHash<int, QString> primaryAliases;

    // Make tag category query
    QSqlQuery categoryQuery(u"SELECT `"_s + Table_Tag_Category::COLUMN_LIST.join(u"`,`"_s) + u"` FROM "_s + Table_Tag_Category::NAME, fpDb);

    // Return if error occurs
    if(categoryQuery.lastError().isValid())
        return categoryQuery.lastError();

    // Parse query
    while(categoryQuery.next())
    {
        TagCategory tc;
        tc.name = categoryQuery.value(Table_Tag_Category::COL_NAME).toString();
        tc.color = QColor(categoryQuery.value(Table_Tag_Category::COL_COLOR).toString());

        mTagMap[categoryQuery.value(Table_Tag_Category::COL_ID).toInt()] = tc;
    }

    // Make tag alias query
    QSqlQuery aliasQuery(u"SELECT `"_s + Table_Tag_Alias::COLUMN_LIST.join(u"`,`"_s) + u"` FROM "_s + Table_Tag_Alias::NAME, fpDb);

    // Return if error occurs
    if(aliasQuery.lastError().isValid())
        return aliasQuery.lastError();

    // Parse query
    while(aliasQuery.next())
        primaryAliases[aliasQuery.value(Table_Tag_Alias::COL_ID).toInt()] = aliasQuery.value(Table_Tag_Alias::COL_NAME).toString();

    // Make tag query
    QSqlQuery tagQuery(u"SELECT `"_s + Table_Tag::COLUMN_LIST.join(u"`,`"_s) + u"` FROM "_s + Table_Tag::NAME, fpDb);

    // Return if error occurs
    if(tagQuery.lastError().isValid())
        return tagQuery.lastError();

    // Parse query
    while(tagQuery.next())
    {
        Tag tag;
        tag.id = tagQuery.value(Table_Tag::COL_ID).toInt();
        tag.primaryAlias = primaryAliases.value(tagQuery.value(Table_Tag::COL_PRIMARY_ALIAS_ID).toInt());

        mTagMap[tagQuery.value(Table_Tag::COL_CATEGORY_ID).toInt()].tags[tag.id] = tag;
    }

    // Return invalid SqlError
    return QSqlError();
}

DbError Db::queryGamesByPlatform(QList<QueryBuffer>& resultBuffer, const QStringList& platforms, const InclusionOptions& inclusionOptions,
                                   std::optional<const QList<QUuid>*> idInclusionFilter)
{
    // Ensure return buffer is reset
    resultBuffer.clear();

    // Empty shortcuts
    if(platforms.isEmpty() || (idInclusionFilter.has_value() && idInclusionFilter.value()->isEmpty()))
        return DbError();

    // Get database
    QSqlDatabase fpDb;
    QSqlError dbError = getThreadConnection(fpDb);
    if(dbError.isValid())
        return DbError::fromSqlError(dbError);

    // Determine game exclusion filter from tag exclusions if applicable
    QSet<QUuid> idExclusionFilter;
    if(!inclusionOptions.excludedTagIds.isEmpty())
    {
        // Make game tag sets query
        QString tagIdCSV = Qx::String::join(inclusionOptions.excludedTagIds, [](int tagId){return QString::number(tagId);}, u"','"_s);
        QSqlQuery tagQuery(u"SELECT `"_s + Table_Game_Tags_Tag::COL_GAME_ID + u"` FROM "_s + Table_Game_Tags_Tag::NAME +
                           u" WHERE "_s + Table_Game_Tags_Tag::COL_TAG_ID + u" IN('"_s + tagIdCSV + u"')"_s, fpDb);

        QSqlError tagQueryError = tagQuery.lastError();
        if(tagQueryError.isValid())
            return DbError::fromSqlError(tagQueryError);

        // Populate exclusion filter
        while(tagQuery.next())
            idExclusionFilter.insert(tagQuery.value(Table_Game_Tags_Tag::COL_GAME_ID).toUuid());
    }

    for(const QString& platform : platforms)
    {
        // Create platform query string
        QString placeholder = u":platform"_s;
        QString baseQueryCommand = u"SELECT %1 FROM "_s + Table_Game::NAME + u" WHERE "_s +
                                   Table_Game::COL_PLATFORM_NAME + u" = "_s + placeholder + u" AND "_s;

        // Handle filtering
        QString filteredQueryCommand = baseQueryCommand.append(inclusionOptions.includeAnimations ? GAME_AND_ANIM_FILTER : GAME_ONLY_FILTER);

        if(!idExclusionFilter.isEmpty())
        {
            QString gameIdCSV = Qx::String::join(idExclusionFilter, [](QUuid id){return id.toString(QUuid::WithoutBraces);}, u"','"_s);
            filteredQueryCommand += u" AND "_s + Table_Game::COL_ID + u" NOT IN('"_s + gameIdCSV + u"')"_s;
        }

        if(idInclusionFilter.has_value())
        {
            QString gameIdCSV = Qx::String::join(*idInclusionFilter.value(), [](QUuid id){return id.toString(QUuid::WithoutBraces);}, u"','"_s);
            filteredQueryCommand += u" AND "_s + Table_Game::COL_ID + u" IN('"_s + gameIdCSV + u"')"_s;
        }

        // Create final command strings
        QString mainQueryCommand = filteredQueryCommand.arg(u"`"_s + Table_Game::COLUMN_LIST.join(u"`,`"_s) + u"`"_s);
        QString sizeQueryCommand = filteredQueryCommand.arg(GENERAL_QUERY_SIZE_COMMAND);

        // Create main query and bind current platform
        QSqlQuery initialQuery(fpDb);
        initialQuery.setForwardOnly(true);
        initialQuery.prepare(mainQueryCommand);
        initialQuery.bindValue(placeholder, platform);

        // Execute query and return if error occurs
        if(!initialQuery.exec())
            return DbError::fromSqlError(initialQuery.lastError());

        // Create size query and bind current platform
        QSqlQuery sizeQuery(fpDb);
        sizeQuery.prepare(sizeQueryCommand);
        sizeQuery.bindValue(placeholder, platform);

        // Execute query and return if error occurs
        if(!sizeQuery.exec())
            return DbError::fromSqlError(sizeQuery.lastError());

        // Get query size
        sizeQuery.next();
        int querySize = sizeQuery.value(0).toInt();

        // Add result to buffer if there were any hits
        if(querySize > 0)
            resultBuffer.append({platform, initialQuery, querySize});
    }

    // Return invalid error
    return DbError();
}

DbError Db::queryAllAddApps(QueryBuffer& resultBuffer)
{
    // Ensure return buffer is effectively null
    resultBuffer = QueryBuffer();

    // Get database
    QSqlDatabase fpDb;
    QSqlError dbError = getThreadConnection(fpDb);
    if(dbError.isValid())
        return DbError::fromSqlError(dbError);

    // Make query
    QString baseQueryCommand = u"SELECT %1 FROM "_s + Table_Add_App::NAME;
    QString mainQueryCommand = baseQueryCommand.arg(u"`"_s + Table_Add_App::COLUMN_LIST.join(u"`,`"_s) + u"`"_s);
    QString sizeQueryCommand = baseQueryCommand.arg(GENERAL_QUERY_SIZE_COMMAND);

    resultBuffer.source = Table_Add_App::NAME;
    return DbError::fromSqlError(makeNonBindQuery(resultBuffer, &fpDb, mainQueryCommand, sizeQueryCommand));
}

DbError Db::queryEntrys(QueryBuffer& resultBuffer, const EntryFilter& filter)
{
    // Ensure return buffer is effectively null
    resultBuffer = QueryBuffer();

    // Query Constants
    const QString where = u" WHERE "_s;
    const QString nd = u" AND "_s;
    const QString likeTempl = u" LIKE '%%1%' ESCAPE '\\'"_s;

    // Get database
    QSqlDatabase fpDb;
    QSqlError dbError = getThreadConnection(fpDb);
    if(dbError.isValid())
        return DbError::fromSqlError(dbError);

    // Check for entry as a game first
    if(filter.type != EntryType::AddApp)
    {
        // Assemble Base Query Command
        QString baseQueryCommand = u"SELECT %1 FROM "_s + Table_Game::NAME;

        if(!filter.parent.isNull() || !filter.id.isNull() || !filter.name.isNull())
        {
            baseQueryCommand += where;

            if(!filter.id.isNull())
                baseQueryCommand += Table_Game::COL_ID + u" == '"_s + filter.id.toString(QUuid::WithoutBraces) + u"'"_s + nd;
            if(!filter.parent.isNull())
                baseQueryCommand += Table_Game::COL_PARENT_ID + u" == '"_s + filter.parent.toString(QUuid::WithoutBraces) + u"'"_s + nd;
            if(!filter.name.isNull())
            {
                if(filter.exactName)
                    baseQueryCommand += Table_Game::COL_TITLE + u" == '"_s + filter.name + u"'"_s + nd;
                else
                {
                    // Escape name to account for SQL LITE %
                    QString escapedName = filter.name;
                    escapedName.replace(uR"(\)"_s, uR"(\\)"_s); // Have to escape the escape char
                    escapedName.replace(uR"(%)"_s, uR"(\%)"_s);

                    baseQueryCommand += Table_Game::COL_TITLE + likeTempl.arg(escapedName) + nd;
                }
            }

            // Remove trailing AND
            baseQueryCommand.chop(nd.size());
        }

        // Assemble final query commands
        QString mainQueryCommand = baseQueryCommand.arg(u"`"_s + Table_Game::COLUMN_LIST.join(u"`,`"_s) + u"`"_s);
        QString sizeQueryCommand = baseQueryCommand.arg(GENERAL_QUERY_SIZE_COMMAND);

        // Make query
        QSqlError queryError;
        resultBuffer.source = Table_Game::NAME;

        if((queryError = makeNonBindQuery(resultBuffer, &fpDb, mainQueryCommand, sizeQueryCommand)).isValid())
            return DbError::fromSqlError(queryError);

        // Return result if one or more results were found (receiver handles situation in latter case)
        if(resultBuffer.size >= 1)
            return DbError();
    }

    // Check for entry as an additional app second
    if(filter.type != EntryType::Primary)
    {
        // Assemble Base Query Command
        QString baseQueryCommand = u"SELECT %1 FROM "_s + Table_Add_App::NAME;

        if(!filter.parent.isNull() || !filter.id.isNull() || !filter.name.isNull())
        {
            baseQueryCommand += where;

            if(!filter.id.isNull())
                baseQueryCommand += Table_Add_App::COL_ID + u" == '"_s + filter.id.toString(QUuid::WithoutBraces) + u"'"_s + nd;
            if(!filter.parent.isNull())
                baseQueryCommand += Table_Add_App::COL_PARENT_ID + u" == '"_s + filter.parent.toString(QUuid::WithoutBraces) + u"'"_s + nd;
            if(!filter.name.isNull())
            {
                if(filter.exactName)
                    baseQueryCommand += Table_Add_App::COL_NAME + u" == '"_s + filter.name + u"'"_s + nd;
                else
                {
                    // Escape name to account for SQL LITE %
                    QString escapedName = filter.name;
                    escapedName.replace(uR"(\)"_s, uR"(\\)"_s); // Have to escape the escape char
                    escapedName.replace(uR"(%)"_s, uR"(\%)"_s);

                    baseQueryCommand += Table_Add_App::COL_NAME + likeTempl.arg(escapedName) + nd;
                }
            }
            if(filter.playableOnly)
            {
                baseQueryCommand += Table_Add_App::COL_APP_PATH + u" NOT IN ('"_s + Table_Add_App::ENTRY_EXTRAS + u"','"_s +
                                    Table_Add_App::ENTRY_MESSAGE + u"') AND "_s + Table_Add_App::COL_AUTORUN + u" != 1"_s +
                                    nd;
            }

            // Remove trailing AND
            baseQueryCommand.chop(nd.size());
        }

        // Assemble final query commands
        QString mainQueryCommand = baseQueryCommand.arg(u"`"_s + Table_Add_App::COLUMN_LIST.join(u"`,`"_s) + u"`"_s);
        QString sizeQueryCommand = baseQueryCommand.arg(GENERAL_QUERY_SIZE_COMMAND);

        // Make query
        QSqlError queryError;
        resultBuffer.source = Table_Add_App::NAME;

        if((queryError = makeNonBindQuery(resultBuffer, &fpDb, mainQueryCommand, sizeQueryCommand)).isValid())
            return DbError::fromSqlError(queryError);

        // Return result if one or more results were found (receiver handles situation in latter case)
        if(resultBuffer.size >= 1)
            return DbError();
    }

    // No result found, return
    return DbError();
}

DbError Db::queryEntryDataById(QueryBuffer& resultBuffer, const QUuid& appId)
{
    // A few entries have more than one data pack. The results are sorted
    // by most to least recently added

    // Ensure return buffer is effectively null
    resultBuffer = QueryBuffer();

    // Get database
    QSqlDatabase fpDb;
    QSqlError dbError = getThreadConnection(fpDb);
    if(dbError.isValid())
        return DbError::fromSqlError(dbError);

    // Setup ID query
    QString baseQueryCommand = u"SELECT %1 FROM "_s + Table_Game_Data::NAME + u" WHERE "_s +
            Table_Game_Data::COL_GAME_ID + u" == '"_s + appId.toString(QUuid::WithoutBraces) + u"' "_s +
                               u"ORDER BY "_s + Table_Game_Data::COL_DATE_ADDED + u" DESC"_s;
    QString mainQueryCommand = baseQueryCommand.arg(u"`"_s + Table_Game_Data::COLUMN_LIST.join(u"`,`"_s) + u"`"_s);
    QString sizeQueryCommand = baseQueryCommand.arg(GENERAL_QUERY_SIZE_COMMAND);

    // Make query
    QSqlError queryError;
    resultBuffer.source = Table_Game_Data::NAME;

    return DbError::fromSqlError(makeNonBindQuery(resultBuffer, &fpDb, mainQueryCommand, sizeQueryCommand));
}

DbError Db::queryAllGameIds(QueryBuffer& resultBuffer, const LibraryFilter& includeFilter)
{
    // Ensure return buffer is effectively null
    resultBuffer = QueryBuffer();

    // Get database
    QSqlDatabase fpDb;
    QSqlError dbError = getThreadConnection(fpDb);
    if(dbError.isValid())
        return DbError::fromSqlError(dbError);

    // Make query
    QString baseQueryCommand = u"SELECT %2 FROM "_s + Table_Game::NAME + u" WHERE "_s +
                               Table_Game::COL_STATUS + u" != '"_s + Table_Game::ENTRY_NOT_WORK + u"'%1"_s;
    baseQueryCommand = baseQueryCommand.arg(includeFilter == LibraryFilter::Game ? u" AND "_s + GAME_ONLY_FILTER : (includeFilter == LibraryFilter::Anim ? u" AND "_s + ANIM_ONLY_FILTER : u""_s));
    QString mainQueryCommand = baseQueryCommand.arg(u"`"_s + Table_Game::COL_ID + u"`"_s);
    QString sizeQueryCommand = baseQueryCommand.arg(GENERAL_QUERY_SIZE_COMMAND);

    resultBuffer.source = Table_Game::NAME;
    return DbError::fromSqlError(makeNonBindQuery(resultBuffer, &fpDb, mainQueryCommand, sizeQueryCommand));
}

QStringList Db::platformNames() const { return mPlatformNames; } //TODO: Probably should use RAII for this.
QMap<int, Db::TagCategory> Db::tags() const { return mTagMap; }

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

    // Get database
    QSqlDatabase fpDb;
    QSqlError dbError = getThreadConnection(fpDb);
    if(dbError.isValid())
        return DbError::fromSqlError(dbError);

    // Make query
    QString packCheckQueryCommand = u"SELECT "_s + GENERAL_QUERY_SIZE_COMMAND + u" FROM "_s + Table_Game_Data::NAME + u" WHERE "_s +
                                   Table_Game_Data::COL_GAME_ID + u" == '"_s + gameId.toString(QUuid::WithoutBraces) + u"'"_s;

    QSqlQuery packCheckQuery(fpDb);
    packCheckQuery.setForwardOnly(true);
    packCheckQuery.prepare(packCheckQueryCommand);

    // Execute query and return if error occurs
    if(!packCheckQuery.exec())
        return DbError::fromSqlError(packCheckQuery.lastError());

    // Set buffer based on result
    packCheckQuery.next();
    resultBuffer = packCheckQuery.value(0).toInt() > 0;

    // Return invalid error
    return DbError();
}

DbError Db::getEntry(std::variant<Game, AddApp>& entry, const QUuid& entryId)
{
    // Find title
    Db::EntryFilter mainFilter{.type = Fp::Db::EntryType::PrimaryThenAddApp, .id = entryId};

    Fp::Db::QueryBuffer searchResult;
    DbError searchError = queryEntrys(searchResult, mainFilter);
    if(searchError.isValid())
        return searchError;

    // Check if ID was found and that only one instance was found
    if(searchResult.size == 0)
        return DbError(DbError::IncompleteSearch, ERR_ID_NOT_FOUND);
    else if(searchResult.size > 1)
        return DbError(DbError::IdCollision, ERR_ID_DUPLICATE_ENTRY);

    // Advance result to only record
    searchResult.result.next();

    // Fill variant
    if(searchResult.source == Db::Table_Add_App::NAME)
    {
        AddApp::Builder fpAab;
        fpAab.wId(searchResult.result.value(Fp::Db::Table_Add_App::COL_ID).toString());
        fpAab.wAppPath(searchResult.result.value(Fp::Db::Table_Add_App::COL_APP_PATH).toString());
        fpAab.wAutorunBefore(searchResult.result.value(Fp::Db::Table_Add_App::COL_AUTORUN).toString());
        fpAab.wLaunchCommand(searchResult.result.value(Fp::Db::Table_Add_App::COL_LAUNCH_COMMAND).toString());
        fpAab.wName(searchResult.result.value(Fp::Db::Table_Add_App::COL_NAME).toString().remove(Qx::RegularExpression::LINE_BREAKS));
        fpAab.wWaitExit(searchResult.result.value(Fp::Db::Table_Add_App::COL_WAIT_EXIT).toString());
        fpAab.wParentId(searchResult.result.value(Fp::Db::Table_Add_App::COL_PARENT_ID).toString());

        entry = fpAab.build();
    }
    else if(searchResult.source == Db::Table_Game::NAME)
    {
        Game::Builder fpGb;
        fpGb.wId(searchResult.result.value(Fp::Db::Table_Game::COL_ID).toString());
        fpGb.wTitle(searchResult.result.value(Fp::Db::Table_Game::COL_TITLE).toString().remove(Qx::RegularExpression::LINE_BREAKS));
        fpGb.wSeries(searchResult.result.value(Fp::Db::Table_Game::COL_SERIES).toString().remove(Qx::RegularExpression::LINE_BREAKS));
        fpGb.wDeveloper(searchResult.result.value(Fp::Db::Table_Game::COL_DEVELOPER).toString().remove(Qx::RegularExpression::LINE_BREAKS));
        fpGb.wPublisher(searchResult.result.value(Fp::Db::Table_Game::COL_PUBLISHER).toString().remove(Qx::RegularExpression::LINE_BREAKS));
        fpGb.wDateAdded(searchResult.result.value(Fp::Db::Table_Game::COL_DATE_ADDED).toString());
        fpGb.wDateModified(searchResult.result.value(Fp::Db::Table_Game::COL_DATE_MODIFIED).toString());
        fpGb.wBroken(searchResult.result.value(Fp::Db::Table_Game::COL_BROKEN).toString());
        fpGb.wPlayMode(searchResult.result.value(Fp::Db::Table_Game::COL_PLAY_MODE).toString());
        fpGb.wStatus(searchResult.result.value(Fp::Db::Table_Game::COL_STATUS).toString());
        fpGb.wNotes(searchResult.result.value(Fp::Db::Table_Game::COL_NOTES).toString());
        fpGb.wSource(searchResult.result.value(Fp::Db::Table_Game::COL_SOURCE).toString().remove(Qx::RegularExpression::LINE_BREAKS));
        fpGb.wAppPath(searchResult.result.value(Fp::Db::Table_Game::COL_APP_PATH).toString());
        fpGb.wLaunchCommand(searchResult.result.value(Fp::Db::Table_Game::COL_LAUNCH_COMMAND).toString());
        fpGb.wReleaseDate(searchResult.result.value(Fp::Db::Table_Game::COL_RELEASE_DATE).toString());
        fpGb.wVersion(searchResult.result.value(Fp::Db::Table_Game::COL_VERSION).toString().remove(Qx::RegularExpression::LINE_BREAKS));
        fpGb.wOriginalDescription(searchResult.result.value(Fp::Db::Table_Game::COL_ORIGINAL_DESC).toString());
        fpGb.wLanguage(searchResult.result.value(Fp::Db::Table_Game::COL_LANGUAGE).toString().remove(Qx::RegularExpression::LINE_BREAKS));
        fpGb.wOrderTitle(searchResult.result.value(Fp::Db::Table_Game::COL_ORDER_TITLE).toString().remove(Qx::RegularExpression::LINE_BREAKS));
        fpGb.wLibrary(searchResult.result.value(Fp::Db::Table_Game::COL_LIBRARY).toString());
        fpGb.wPlatformName(searchResult.result.value(Fp::Db::Table_Game::COL_PLATFORM_NAME).toString());

        entry = fpGb.build();
    }
    else
        qFatal("Entry search result source must be 'game' or 'additional_app'");

    return DbError();
}

DbError Db::getGameData(GameData& data, const QUuid& gameId)
{
    // Clear buffer
    data = GameData();

    // Get entry data
    DbError searchError;
    Fp::Db::QueryBuffer searchResult;

    if((searchError = queryEntryDataById(searchResult, gameId)).isValid())
        return searchError;

    // Check if ID was found and if so that only one instance was found
    if(searchResult.size == 0)
        return DbError(); // Game doesn't have data pack
    else if(searchResult.size > 1)
        qWarning("Entry with more than one data pack, using most recent.");

    // Advance result to first record
    searchResult.result.next();

    // Fill buffer
    GameData::Builder fpGdb;
    fpGdb.wId(searchResult.result.value(Fp::Db::Table_Game_Data::COL_ID).toString());
    fpGdb.wGameId(searchResult.result.value(Fp::Db::Table_Game_Data::COL_GAME_ID).toString());
    fpGdb.wTitle(searchResult.result.value(Fp::Db::Table_Game_Data::COL_TITLE).toString());
    fpGdb.wDateAdded(searchResult.result.value(Fp::Db::Table_Game_Data::COL_DATE_ADDED).toString());
    fpGdb.wSha256(searchResult.result.value(Fp::Db::Table_Game_Data::COL_SHA256).toString());
    fpGdb.wCrc32(searchResult.result.value(Fp::Db::Table_Game_Data::COL_CRC32).toString());
    fpGdb.wPresentOnDisk(searchResult.result.value(Fp::Db::Table_Game_Data::COL_PRES_ON_DISK).toString());
    fpGdb.wPath(searchResult.result.value(Fp::Db::Table_Game_Data::COL_PATH).toString());
    fpGdb.wSize(searchResult.result.value(Fp::Db::Table_Game_Data::COL_SIZE).toString());
    fpGdb.wParameters(searchResult.result.value(Fp::Db::Table_Game_Data::COL_PARAM).toString());
    fpGdb.wAppPath(searchResult.result.value(Fp::Db::Table_Game_Data::COL_APP_PATH).toString());
    fpGdb.wLaunchCommand(searchResult.result.value(Fp::Db::Table_Game_Data::COL_LAUNCH_COMMAND).toString());

    data = fpGdb.build();

    return DbError();
}

DbError Db::updateGameDataOnDiskState(QList<int> packIds, bool onDisk)
{
    // Get database
    QSqlDatabase fpDb;
    QSqlError dbError = getThreadConnection(fpDb);
    if(dbError.isValid())
        return DbError::fromSqlError(dbError);

    // Make query
    QString filter = Qx::String::join(packIds, [](int i){ return QString::number(i); }, u","_s);
    QString dataUpdateCommand = u"UPDATE "_s + Table_Game_Data::NAME + u" SET "_s + Table_Game_Data::COL_PRES_ON_DISK + u" = "_s + QString::number(onDisk) +
                                u" WHERE "_s + Table_Game_Data::COL_ID + u" IN ("_s + filter + ')';

    QSqlQuery packUpdateQuery(fpDb);
    packUpdateQuery.setForwardOnly(true);
    packUpdateQuery.prepare(dataUpdateCommand);

    // Execute query and return if error occurs
    if(!packUpdateQuery.exec())
        return DbError::fromSqlError(packUpdateQuery.lastError());

    // Check that expected count was affected
    int expected = packIds.size();
    int affected = packUpdateQuery.numRowsAffected();
    if(affected != expected)
        return DbError(DbError::UpdateRowMismatch, Table_Game_Data::NAME + u" SET "_s + Table_Game_Data::COL_PRES_ON_DISK, u"%1 instead of %2"_s.arg(affected, expected));

    return DbError();
}

//-Slots ------------------------------------------------------------------------------------------------------
//Private:
void Db::connectedThreadDestroyed(QObject* thread)
{
    QThread* pThread = qobject_cast<QThread*>(thread);

    // Ensure the signal that triggered this slot belongs to the above class by checking for null pointer
    if(pThread == nullptr)
        qFatal("Pointer conversion to thread failed");

    // Close connection
    closeConnection(pThread);
}

}
