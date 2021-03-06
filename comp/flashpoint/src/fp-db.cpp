// Unit Includes
#include "fp/flashpoint/fp-db.h"

// Qx Includes
#include <qx/core/qx-string.h>

namespace Fp
{

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
Db::Db(QString databaseName, const Key&) :
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
        mError = Qx::GenericError(Qx::GenericError::Critical, ERR_DATABASE, databaseError.text());
        return;
    }

    // Check if tables are missing
    if(!missingTables.isEmpty())
    {
        mError = Qx::GenericError(Qx::GenericError::Critical, ERR_MISSING_TABLE, QString(),
                         QStringList(missingTables.begin(), missingTables.end()).join("\n"));
        return;
    }

    // Ensure the database contains the required columns
    QSet<QString> missingColumns;
    if((databaseError = checkDatabaseForRequiredColumns(missingColumns)).isValid())
    {
        mError = Qx::GenericError(Qx::GenericError::Critical, ERR_DATABASE, databaseError.text());
        return;
    }

    // Check if columns are missing
    if(!missingColumns.isEmpty())
    {
        mError = Qx::GenericError(Qx::GenericError::Critical, ERR_MISSING_TABLE, QString(),
                         QStringList(missingColumns.begin(), missingColumns.end()).join("\n"));
        return;
    }

    // Populate item members
    if((databaseError = populateAvailableItems()).isValid())
    {
        mError = Qx::GenericError(Qx::GenericError::Critical, ERR_DATABASE, databaseError.text());
        return;
    }

    if((databaseError = populateTags()).isValid())
    {
        mError = Qx::GenericError(Qx::GenericError::Critical, ERR_DATABASE, databaseError.text());
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
           "_i" + QString::number((quint64)this, 16) +
           "_t" + QString::number((quint64)thread, 16);
}


//-Instance Functions------------------------------------------------------------------------------------------------
//Private:
void Db::nullify()
{
    mPlatformList.clear();
    mPlaylistList.clear();
    mTagMap.clear();
}

void Db::closeConnection(const QThread* thread)
{
    if(mConnectedThreads.contains(thread))
    {
        QString tcn = threadConnectionName(thread);
        QSqlDatabase connection = QSqlDatabase::database(tcn, false);
        connection.close();
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
        connection = QSqlDatabase::addDatabase("QSQLITE", tcn);
        connection.setConnectOptions("QSQLITE_OPEN_READONLY");
        connection.setDatabaseName(mDatabaseName);

        if(connection.open())
        {
            mConnectedThreads.insert(thread);
            connect(thread, &QThread::finished, this, &Db::connectedThreadFinished);
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

QSqlError Db::makeNonBindQuery(QueryBuffer& resultBuffer, QSqlDatabase* database, QString queryCommand, QString sizeQueryCommand) const
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
Qx::GenericError Db::error() { return mError; }

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

QSqlError Db::checkDatabaseForRequiredColumns(QSet<QString> &missingColumsReturnBuffer)
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
        QSqlQuery columnQuery("PRAGMA table_info(" + tableAndColumns.name + ")", fpDb);

        // Return if error occurs
        if(columnQuery.lastError().isValid())
            return columnQuery.lastError();

        // Parse query
        while(columnQuery.next())
            existingColumns.insert(columnQuery.value("name").toString());

        // Check for missing columns
        for(const QString& column : tableAndColumns.columns)
            if(!existingColumns.contains(column))
                missingColumsReturnBuffer.insert(tableAndColumns.name + ": " + column);
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
    mPlatformList.clear();
    mPlaylistList.clear();

    // Make platform query
    QSqlQuery platformQuery("SELECT DISTINCT " + Table_Game::COL_PLATFORM + " FROM " + Table_Game::NAME, fpDb);

    // Return if error occurs
    if(platformQuery.lastError().isValid())
        return platformQuery.lastError();

    // Parse query
    while(platformQuery.next())
        mPlatformList.append(platformQuery.value(Table_Game::COL_PLATFORM).toString());

    // Sort list
    mPlatformList.sort();

    // Make playlist query
    QSqlQuery playlistQuery("SELECT DISTINCT " + Table_Playlist::COL_TITLE + " FROM " + Table_Playlist::NAME, fpDb);

    // Return if error occurs
    if(playlistQuery.lastError().isValid())
        return playlistQuery.lastError();

    // Parse query
    while(playlistQuery.next())
        mPlaylistList.append(playlistQuery.value(Db::Table_Playlist::COL_TITLE).toString());

    // Sort list
    mPlaylistList.sort();

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
    QSqlQuery categoryQuery("SELECT `" + Table_Tag_Category::COLUMN_LIST.join("`,`") + "` FROM " + Table_Tag_Category::NAME, fpDb);

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
    QSqlQuery aliasQuery("SELECT `" + Table_Tag_Alias::COLUMN_LIST.join("`,`") + "` FROM " + Table_Tag_Alias::NAME, fpDb);

    // Return if error occurs
    if(aliasQuery.lastError().isValid())
        return aliasQuery.lastError();

    // Parse query
    while(aliasQuery.next())
        primaryAliases[aliasQuery.value(Table_Tag_Alias::COL_ID).toInt()] = aliasQuery.value(Table_Tag_Alias::COL_NAME).toString();

    // Make tag query
    QSqlQuery tagQuery("SELECT `" + Table_Tag::COLUMN_LIST.join("`,`") + "` FROM " + Table_Tag::NAME, fpDb);

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

QSqlError Db::queryGamesByPlatform(QList<QueryBuffer>& resultBuffer, QStringList platforms, InclusionOptions inclusionOptions,
                                        const QList<QUuid>& idInclusionFilter)
{
    // Ensure return buffer is reset
    resultBuffer.clear();

    // Get database
    QSqlDatabase fpDb;
    QSqlError dbError = getThreadConnection(fpDb);
    if(dbError.isValid())
        return dbError;

    // Determine game exclusion filter from tag exclusions if applicable
    QSet<QUuid> idExclusionFilter;
    if(!inclusionOptions.excludedTagIds.isEmpty())
    {
        // Make game tag sets query
        QString tagIdCSV = Qx::String::join(inclusionOptions.excludedTagIds, [](int tagId){return QString::number(tagId);}, "','");
        QSqlQuery tagQuery("SELECT `" + Table_Game_Tags_Tag::COL_GAME_ID + "` FROM " + Table_Game_Tags_Tag::NAME +
                           " WHERE " + Table_Game_Tags_Tag::COL_TAG_ID + " IN('" + tagIdCSV + "')", fpDb);

        QSqlError tagQueryError = tagQuery.lastError();
        if(tagQueryError.isValid())
            return tagQueryError;

        // Populate exclusion filter
        while(tagQuery.next())
            idExclusionFilter.insert(tagQuery.value(Table_Game_Tags_Tag::COL_GAME_ID).toUuid());
    }

    for(const QString& platform : platforms) // Naturally returns empty list if no platforms are selected
    {
        // Create platform query string
        QString placeholder = ":platform";
        QString baseQueryCommand = "SELECT %1 FROM " + Table_Game::NAME + " WHERE " +
                                   Table_Game::COL_PLATFORM + " = " + placeholder + " AND ";

        // Handle filtering
        QString filteredQueryCommand = baseQueryCommand.append(inclusionOptions.includeAnimations ? GAME_AND_ANIM_FILTER : GAME_ONLY_FILTER);

        if(!idExclusionFilter.isEmpty())
        {
            QString gameIdCSV = Qx::String::join(idExclusionFilter, [](QUuid id){return id.toString(QUuid::WithoutBraces);}, "','");
            filteredQueryCommand += " AND " + Table_Game::COL_ID + " NOT IN('" + gameIdCSV + "')";
        }

        if(!idInclusionFilter.isEmpty())
        {
            QString gameIdCSV = Qx::String::join(idInclusionFilter, [](QUuid id){return id.toString(QUuid::WithoutBraces);}, "','");
            filteredQueryCommand += " AND " + Table_Game::COL_ID + " IN('" + gameIdCSV + "')";
        }

        // Create final command strings
        QString mainQueryCommand = filteredQueryCommand.arg("`" + Table_Game::COLUMN_LIST.join("`,`") + "`");
        QString sizeQueryCommand = filteredQueryCommand.arg(GENERAL_QUERY_SIZE_COMMAND);

        // Create main query and bind current platform
        QSqlQuery initialQuery(fpDb);
        initialQuery.setForwardOnly(true);
        initialQuery.prepare(mainQueryCommand);
        initialQuery.bindValue(placeholder, platform);

        // Execute query and return if error occurs
        if(!initialQuery.exec())
            return initialQuery.lastError();

        // Create size query and bind current platform
        QSqlQuery sizeQuery(fpDb);
        sizeQuery.prepare(sizeQueryCommand);
        sizeQuery.bindValue(placeholder, platform);

        // Execute query and return if error occurs
        if(!sizeQuery.exec())
            return sizeQuery.lastError();

        // Get query size
        sizeQuery.next();
        int querySize = sizeQuery.value(0).toInt();

        // Add result to buffer if there were any hits
        if(querySize > 0)
            resultBuffer.append({platform, initialQuery, querySize});
    }

    // Return invalid SqlError
    return QSqlError();
}

QSqlError Db::queryAllAddApps(QueryBuffer& resultBuffer)
{
    // Ensure return buffer is effectively null
    resultBuffer = QueryBuffer();

    // Get database
    QSqlDatabase fpDb;
    QSqlError dbError = getThreadConnection(fpDb);
    if(dbError.isValid())
        return dbError;

    // Make query
    QString baseQueryCommand = "SELECT %1 FROM " + Table_Add_App::NAME;
    QString mainQueryCommand = baseQueryCommand.arg("`" + Table_Add_App::COLUMN_LIST.join("`,`") + "`");
    QString sizeQueryCommand = baseQueryCommand.arg(GENERAL_QUERY_SIZE_COMMAND);

    resultBuffer.source = Table_Add_App::NAME;
    return makeNonBindQuery(resultBuffer, &fpDb, mainQueryCommand, sizeQueryCommand);
}

QSqlError Db::queryPlaylistsByName(QueryBuffer& resultBuffer, QStringList playlists)
{
    // Return blank result if no playlists are selected
    if(playlists.isEmpty())
    {
        resultBuffer.source = QString();
        resultBuffer.result = QSqlQuery();
        resultBuffer.size = 0;

        return QSqlError();
    }
    else
    {
        // Ensure return buffer is effectively null
        resultBuffer = QueryBuffer();

        // Get database
        QSqlDatabase fpDb;
        QSqlError dbError = getThreadConnection(fpDb);
        if(dbError.isValid())
            return dbError;

        // Create selected playlists query string
        QString placeHolders = QString("?,").repeated(playlists.size());
        placeHolders.chop(1); // Remove trailing ?
        QString baseQueryCommand = "SELECT %1 FROM " + Table_Playlist::NAME + " WHERE " +
                Table_Playlist::COL_TITLE + " IN (" + placeHolders + ") AND " +
                Table_Playlist::COL_LIBRARY + " = '" + Table_Playlist::ENTRY_GAME_LIBRARY + "'";
        QString mainQueryCommand = baseQueryCommand.arg("`" + Table_Playlist::COLUMN_LIST.join("`,`") + "`");
        QString sizeQueryCommand = baseQueryCommand.arg(GENERAL_QUERY_SIZE_COMMAND);

        // Create main query and bind selected playlists
        QSqlQuery mainQuery(fpDb);
        mainQuery.setForwardOnly(true);
        mainQuery.prepare(mainQueryCommand);
        for(const QString& playlist : playlists)
            mainQuery.addBindValue(playlist);

        // Execute query and return if error occurs
        if(!mainQuery.exec())
            return mainQuery.lastError();

        // Create size query and bind selected playlists
        QSqlQuery sizeQuery(fpDb);
        sizeQuery.setForwardOnly(true);
        sizeQuery.prepare(sizeQueryCommand);
        for(const QString& playlist : playlists)
            sizeQuery.addBindValue(playlist);

        // Execute query and return if error occurs
        if(!sizeQuery.exec())
            return sizeQuery.lastError();

        // Get query size
        sizeQuery.next();
        int querySize = sizeQuery.value(0).toInt();

        // Set buffer instance to result
        resultBuffer.source = Table_Playlist::NAME;
        resultBuffer.result = mainQuery;
        resultBuffer.size = querySize;

        // Return invalid SqlError
        return QSqlError();
    }
}

QSqlError Db::queryPlaylistGamesByPlaylist(QList<QueryBuffer>& resultBuffer, const QList<QUuid>& playlistIds)
{
    // Ensure return buffer is empty
    resultBuffer.clear();

    // Get database
    QSqlDatabase fpDb;
    QSqlError dbError = getThreadConnection(fpDb);
    if(dbError.isValid())
        return dbError;

    for(QUuid playlistId : playlistIds) // Naturally returns empty list if no playlists are selected
    {
        // Query all games for the current playlist
        QString baseQueryCommand = "SELECT %1 FROM " + Table_Playlist_Game::NAME + " WHERE " +
                Table_Playlist_Game::COL_PLAYLIST_ID + " = '" + playlistId.toString(QUuid::WithoutBraces) + "'";
        QString mainQueryCommand = baseQueryCommand.arg("`" + Table_Playlist_Game::COLUMN_LIST.join("`,`") + "`");
        QString sizeQueryCommand = baseQueryCommand.arg(GENERAL_QUERY_SIZE_COMMAND);

        // Make query
        QSqlError queryError;
        QueryBuffer queryResult;
        queryResult.source = playlistId.toString();

        if((queryError = makeNonBindQuery(queryResult, &fpDb, mainQueryCommand, sizeQueryCommand)).isValid())
            return queryError;

        // Add result to buffer if there were any hits
        if(queryResult.size > 0)
            resultBuffer.append(queryResult);
    }

    // Return invalid SqlError
    return QSqlError();
}

QSqlError Db::queryPlaylistGameIds(QueryBuffer& resultBuffer, const QList<QUuid>& playlistIds)
{
    // Ensure return buffer is empty
    resultBuffer = QueryBuffer();

    // Get database
    QSqlDatabase fpDb;
    QSqlError dbError = getThreadConnection(fpDb);
    if(dbError.isValid())
        return dbError;

    // Create playlist ID query string
    QString idCSV = Qx::String::join(playlistIds, [](QUuid id){return id.toString(QUuid::WithoutBraces);}, "','");

    // Query all game IDs that fall under given the playlists
    QString baseQueryCommand = "SELECT %1 FROM " + Table_Playlist_Game::NAME + " WHERE " +
            Table_Playlist_Game::COL_PLAYLIST_ID + " IN('" + idCSV + "')";
    QString mainQueryCommand = baseQueryCommand.arg("`" + Table_Playlist_Game::COL_GAME_ID + "`");
    QString sizeQueryCommand = baseQueryCommand.arg(GENERAL_QUERY_SIZE_COMMAND);

    // Make query
    QSqlError queryError;
    resultBuffer.source = Table_Playlist_Game::NAME;

    if((queryError = makeNonBindQuery(resultBuffer, &fpDb, mainQueryCommand, sizeQueryCommand)).isValid())
        return queryError;

    // Return invalid SqlError
    return QSqlError();

}

QSqlError Db::queryAllEntryTags(QueryBuffer& resultBuffer)
{
    // Ensure return buffer is empty
    resultBuffer = QueryBuffer();

    // Get database
    QSqlDatabase fpDb;
    QSqlError dbError = getThreadConnection(fpDb);
    if(dbError.isValid())
        return dbError;

    // Query all tags tied to game IDs
    QString baseQueryCommand = "SELECT %1 FROM " + Table_Game_Tags_Tag::NAME;
    QString mainQueryCommand = baseQueryCommand.arg("`" + Table_Game_Tags_Tag::COL_GAME_ID + "`");
    QString sizeQueryCommand = baseQueryCommand.arg(GENERAL_QUERY_SIZE_COMMAND);

    // Make query
    QSqlError queryError;
    resultBuffer.source = Table_Playlist_Game::NAME;

    // Return invalid SqlError
    return makeNonBindQuery(resultBuffer, &fpDb, mainQueryCommand, sizeQueryCommand);
}

QSqlError Db::queryEntryById(QueryBuffer& resultBuffer, QUuid appId)
{
    // Ensure return buffer is effectively null
    resultBuffer = QueryBuffer();

    // Get database
    QSqlDatabase fpDb;
    QSqlError dbError = getThreadConnection(fpDb);
    if(dbError.isValid())
        return dbError;

    // Check for entry as a game first
    QString baseQueryCommand = "SELECT %1 FROM " + Table_Game::NAME + " WHERE " +
            Table_Game::COL_ID + " == '" + appId.toString(QUuid::WithoutBraces) + "'";
    QString mainQueryCommand = baseQueryCommand.arg("`" + Table_Game::COLUMN_LIST.join("`,`") + "`");
    QString sizeQueryCommand = baseQueryCommand.arg(GENERAL_QUERY_SIZE_COMMAND);

    // Make query
    QSqlError queryError;
    resultBuffer.source = Table_Game::NAME;

    if((queryError = makeNonBindQuery(resultBuffer, &fpDb, mainQueryCommand, sizeQueryCommand)).isValid())
        return queryError;

    // Return result if one or more results were found (receiver handles situation in latter case)
    if(resultBuffer.size >= 1)
        return QSqlError();

    // Check for entry as an additional app second
    baseQueryCommand = "SELECT %1 FROM " + Table_Add_App::NAME + " WHERE " +
        Table_Add_App::COL_ID + " == '" + appId.toString(QUuid::WithoutBraces) + "'";
    mainQueryCommand = baseQueryCommand.arg("`" + Table_Add_App::COLUMN_LIST.join("`,`") + "`");
    sizeQueryCommand = baseQueryCommand.arg(GENERAL_QUERY_SIZE_COMMAND);

    // Make query and return result regardless of outcome
    resultBuffer.source = Table_Add_App::NAME;
    return makeNonBindQuery(resultBuffer, &fpDb, mainQueryCommand, sizeQueryCommand);
}

QSqlError Db::queryEntriesByTitle(QueryBuffer& resultBuffer, QString title)
{
    // Ensure return buffer is effectively null
    resultBuffer = QueryBuffer();

    // Escape title
    title.replace(R"(')", R"('')");

    // Get database
    QSqlDatabase fpDb;
    QSqlError dbError = getThreadConnection(fpDb);
    if(dbError.isValid())
        return dbError;

    // Check for entry as a game first
    QString baseQueryCommand = "SELECT %1 FROM " + Table_Game::NAME + " WHERE " +
            Table_Game::COL_TITLE + " == '" + title + "'";
    QString mainQueryCommand = baseQueryCommand.arg("`" + Table_Game::COLUMN_LIST.join("`,`") + "`");
    QString sizeQueryCommand = baseQueryCommand.arg(GENERAL_QUERY_SIZE_COMMAND);

    // Make query
    QSqlError queryError;
    resultBuffer.source = Table_Game::NAME;

    return makeNonBindQuery(resultBuffer, &fpDb, mainQueryCommand, sizeQueryCommand);
}

QSqlError Db::queryEntryDataById(QueryBuffer& resultBuffer, QUuid appId)
{
    // Ensure return buffer is effectively null
    resultBuffer = QueryBuffer();

    // Get database
    QSqlDatabase fpDb;
    QSqlError dbError = getThreadConnection(fpDb);
    if(dbError.isValid())
        return dbError;

    // Setup ID query
    QString baseQueryCommand = "SELECT %1 FROM " + Table_Game_Data::NAME + " WHERE " +
            Table_Game_Data::COL_GAME_ID + " == '" + appId.toString(QUuid::WithoutBraces) + "'";
    QString mainQueryCommand = baseQueryCommand.arg("`" + Table_Game_Data::COLUMN_LIST.join("`,`") + "`");
    QString sizeQueryCommand = baseQueryCommand.arg(GENERAL_QUERY_SIZE_COMMAND);

    // Make query
    QSqlError queryError;
    resultBuffer.source = Table_Game_Data::NAME;

    return makeNonBindQuery(resultBuffer, &fpDb, mainQueryCommand, sizeQueryCommand);
}

QSqlError Db::queryEntryAddApps(QueryBuffer& resultBuffer, QUuid appId, bool playableOnly)
{
    // Ensure return buffer is effectively null
    resultBuffer = QueryBuffer();

    // Get database
    QSqlDatabase fpDb;
    QSqlError dbError = getThreadConnection(fpDb);
    if(dbError.isValid())
        return dbError;

    // Make query
    QString baseQueryCommand = "SELECT %1 FROM " + Table_Add_App::NAME + " WHERE " +
            Table_Add_App::COL_PARENT_ID + " == '" + appId.toString(QUuid::WithoutBraces) + "'";
    if(playableOnly)
        baseQueryCommand += " AND " + Table_Add_App::COL_APP_PATH + " NOT IN ('" + Table_Add_App::ENTRY_EXTRAS +
                            "','" + Table_Add_App::ENTRY_MESSAGE + "') AND " + Table_Add_App::COL_AUTORUN +
                            " != 1";
    QString mainQueryCommand = baseQueryCommand.arg("`" + Table_Add_App::COLUMN_LIST.join("`,`") + "`");
    QString sizeQueryCommand = baseQueryCommand.arg(GENERAL_QUERY_SIZE_COMMAND);

    resultBuffer.source = Table_Add_App::NAME;
    return makeNonBindQuery(resultBuffer, &fpDb, mainQueryCommand, sizeQueryCommand);
}

QSqlError Db::queryDataPackSource(QueryBuffer& resultBuffer)
{
    // Ensure return buffer is effectively null
    resultBuffer = QueryBuffer();

    // Get database
    QSqlDatabase fpDb;
    QSqlError dbError = getThreadConnection(fpDb);
    if(dbError.isValid())
        return dbError;

    // Setup ID query
    QString baseQueryCommand = "SELECT %1 FROM " + Table_Source::NAME;
    QString mainQueryCommand = baseQueryCommand.arg("`" + Table_Source::COLUMN_LIST.join("`,`") + "`");
    QString sizeQueryCommand = baseQueryCommand.arg(GENERAL_QUERY_SIZE_COMMAND);

    // Make query
    QSqlError queryError;
    resultBuffer.source = Table_Source::NAME;

    return makeNonBindQuery(resultBuffer, &fpDb, mainQueryCommand, sizeQueryCommand);
}

QSqlError Db::queryEntrySourceData(QueryBuffer& resultBuffer, QString appSha256Hex)
{
    // Ensure return buffer is effectively null
    resultBuffer = QueryBuffer();

    // Get database
    QSqlDatabase fpDb;
    QSqlError dbError = getThreadConnection(fpDb);
    if(dbError.isValid())
        return dbError;

    // Setup ID query
    QString baseQueryCommand = "SELECT %1 FROM " + Table_Source_Data::NAME + " WHERE " +
            Table_Source_Data::COL_SHA256 + " == '" + appSha256Hex + "'";
    QString mainQueryCommand = baseQueryCommand.arg("`" + Table_Source_Data::COLUMN_LIST.join("`,`") + "`");
    QString sizeQueryCommand = baseQueryCommand.arg(GENERAL_QUERY_SIZE_COMMAND);

    // Make query
    QSqlError queryError;
    resultBuffer.source = Table_Source_Data::NAME;

    return makeNonBindQuery(resultBuffer, &fpDb, mainQueryCommand, sizeQueryCommand);
}

QSqlError Db::queryAllGameIds(QueryBuffer& resultBuffer, LibraryFilter includeFilter)
{
    // Ensure return buffer is effectively null
    resultBuffer = QueryBuffer();

    // Get database
    QSqlDatabase fpDb;
    QSqlError dbError = getThreadConnection(fpDb);
    if(dbError.isValid())
        return dbError;

    // Make query
    QString baseQueryCommand = "SELECT %2 FROM " + Table_Game::NAME + " WHERE " +
                               Table_Game::COL_STATUS + " != '" + Table_Game::ENTRY_NOT_WORK + "'%1";
    baseQueryCommand = baseQueryCommand.arg(includeFilter == LibraryFilter::Game ? " AND " + GAME_ONLY_FILTER : (includeFilter == LibraryFilter::Anim ? " AND " + ANIM_ONLY_FILTER : ""));
    QString mainQueryCommand = baseQueryCommand.arg("`" + Table_Game::COL_ID + "`");
    QString sizeQueryCommand = baseQueryCommand.arg(GENERAL_QUERY_SIZE_COMMAND);

    resultBuffer.source = Table_Game::NAME;
    return makeNonBindQuery(resultBuffer, &fpDb, mainQueryCommand, sizeQueryCommand);
}

QStringList Db::platformList() const { return mPlatformList; }
QStringList Db::playlistList() const { return mPlaylistList; }
QMap<int, Db::TagCategory> Db::tags() const { return mTagMap; }

QSqlError Db::entryUsesDataPack(bool& resultBuffer, QUuid gameId)
{
    // Default return buffer to false
    resultBuffer = false;

    // Get database
    QSqlDatabase fpDb;
    QSqlError dbError = getThreadConnection(fpDb);
    if(dbError.isValid())
        return dbError;

    // Make query
    QString packCheckQueryCommand = "SELECT " + GENERAL_QUERY_SIZE_COMMAND + " FROM " + Table_Game_Data::NAME + " WHERE " +
                                   Table_Game_Data::COL_GAME_ID + " == '" + gameId.toString(QUuid::WithoutBraces) + "'";

    QSqlQuery packCheckQuery(fpDb);
    packCheckQuery.setForwardOnly(true);
    packCheckQuery.prepare(packCheckQueryCommand);

    // Execute query and return if error occurs
    if(!packCheckQuery.exec())
        return packCheckQuery.lastError();

    // Set buffer based on result
    packCheckQuery.next();
    resultBuffer = packCheckQuery.value(0).toInt() > 0;

    // Return invalid SqlError
    return QSqlError();
}

//-Slots ------------------------------------------------------------------------------------------------------
//Private:
void Db::connectedThreadFinished()
{
    // Get the object that called this slot
    QThread* senderThread = qobject_cast<QThread*>(sender());

    // Ensure the signal that triggered this slot belongs to the above class by checking for null pointer
    if(senderThread == nullptr)
        throw std::runtime_error("Pointer conversion to thread failed");

    // Close connection
    closeConnection(senderThread);
}

}
